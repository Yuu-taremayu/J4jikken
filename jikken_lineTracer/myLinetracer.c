#include "h8-3069-iodef.h"
#include "h8-3069-int.h"

#include "lcd.h"
#include "ad.h"
#include "timer.h"
#include "random.h"
#include "sci2.h"

/* タイマ割り込みの時間間隔[μs] */
#define TIMER0 1000

/* 割り込み処理で各処理を行う頻度を決める定数 */
#define DISPTIME 100
#define ADTIME 2
#define PWMTIME 1
#define CONTROLTIME 10

/* LCD表示関連 */
/* 1段に表示できる文字数 */
#define LCDDISPSIZE 8

/* PWM制御関連 */
/* 制御周期を決める定数 */
#define MAXPWMCOUNT 255

/* ライントレーサーの移動方向を決めるビット位置 */
#define STOP 0x00
#define GO 0x05
#define BACK 0x0a
#define RIGHTTURN 0x01
#define LEFTTURN 0x04
#define BRAKE 0x0f

#define RIGHTSENSOR 2
#define LEFTSENSOR 1

#define THRESHOLD 150

#define STATEBUFSIZE 10
#define BLACK 0
#define WHITE 1
#define NONE -1

#define WAITTIME 100

/* A/D変換関連 */
/* A/D変換のチャネル数とバッファサイズ */
#define ADCHNUM   4
#define ADBUFSIZE 8
/* 平均化するときのデータ個数 */
#define ADAVRNUM 4
/* チャネル指定エラー時に返す値 */
#define ADCHNONE -1

/* 割り込み処理に必要な変数は大域変数にとる */
volatile int disp_time, ad_time, control_time, pwm_time, wait_time;

/* LCD関係 */
volatile int disp_flag;
volatile char lcd_str_upper[LCDDISPSIZE+1];
volatile char lcd_str_lower[LCDDISPSIZE+1] = "L   R   ";

/* モータ制御関係 */
volatile int pwm_count;

/* A/D変換関係 */
volatile unsigned char adbuf[ADCHNUM][ADBUFSIZE];
volatile int adbufdp;

volatile int ad_right_sensor;
volatile int ad_left_sensor;

volatile int oldstatebuf[2][STATEBUFSIZE];
volatile int oldstatebufdp;
volatile int l_ct, r_ct;


//volatile int switch_flag;
//volatile unsigned char switch_old_state;

int main(void);
void all_init(void);
void int_imia0(void);
void int_adi(void);
int ad_read(int ch);
void pwm_proc(void);
void control_proc(void);

int main(void)
{
  all_init();
  
  timer_start(0);
  ENINT();

  lcd_clear();

  while (1) {
    if (disp_flag == 1) {
      lcd_cursor(0, 0);
      lcd_printstr(lcd_str_upper);
      lcd_cursor(0, 1);
      lcd_printstr(lcd_str_lower);
      disp_flag = 0;
    }
  }
      
  return 0;
}

/* 全ての初期化を行う関数 */
void all_init(void)
{
  /* ROMエミュレーションをON */
  ROMEMU();
  
  /* モータ用のポートの初期化 */
  /* PB0~PB3を出力に設定 */
  PBDDR |= 0x0f;

  /* スイッチ用のポートの初期化 */
  /* P60,P61を入力に設定 */
  P6DDR &= ~0x02;

  /* モータは最初はSTOPにしておく */
  PBDR = STOP;

  /* 大域変数の初期化 */
  disp_time = 0;
  disp_flag = 1;
  ad_time = 0;
  control_time = 0;
  pwm_time = 0;
  pwm_count = 0;
  wait_time = 0;
  adbufdp = 0;
  oldstatebufdp = 0;

  /* A/D変換，LCD，タイマーの初期化 */
  ad_init();
  lcd_init();
  timer_init();
  timer_set(0, TIMER0);
}

/* タイマ0の割り込みハンドラ */
/* タイマ割り込みで各処理を呼び出す */
/* 頻度は上記の DISPTIME, ADTIME, CONTROLTIME, PWMTIMEで決まる */
/* wait_timeは貫通電流を起こさないための待ち時間の処理に使う */
#pragma interrupt
void int_imia0(void)
{
  disp_time++;
  if (disp_time >= DISPTIME) {
    disp_flag = 1;
    disp_time = 0;
  }

  ad_time++;
  if (ad_time >= ADTIME) {
    ad_scan(0, 1);
    ad_time = 0;
  }

  control_time++;
  if (control_time >= CONTROLTIME) {
    control_proc();
    control_time = 0;
  }

  pwm_time++;
  if (pwm_time >= PWMTIME) {
    pwm_proc();
    pwm_time = 0;
  }

  wait_time++;
  
  timer_intflag_reset(0);
  ENINT();
}

/* A/D変換終了の割り込みハンドラ */
/* A/D変換の停止，終了，変換データの格納をしている */
#pragma interrupt
void int_adi(void)
{
  ad_stop();

  oldstatebufdp = (oldstatebufdp + 1) % STATEBUFSIZE;

  adbufdp = (adbufdp + 1) % ADBUFSIZE;
  
  adbuf[0][adbufdp] = ADDRAH;
  adbuf[1][adbufdp] = ADDRBH;
  adbuf[2][adbufdp] = ADDRCH;
  adbuf[3][adbufdp] = ADDRDH;
  
  ENINT();
}

/* A/Dのチャネル番号を引数で与えると，指定チャネルの平均化した値を返す */
int ad_read(int ch)
{
  int i, ad, bp;

  if ((ch > ADCHNUM) || (ch < 0)) {
    ad = ADCHNONE;
  }
  else {
    ad = 0;
    bp = adbufdp - (ADAVRNUM - 1);

    if (bp < 0) bp += ADBUFSIZE;

    for (i = 0; i < ADAVRNUM; i++) {
      ad += (int)adbuf[ch][(bp + i) % ADBUFSIZE];
    }
    ad /= ADAVRNUM;
  }

  return ad;
}

/* PWM制御を行う関数 */
/* 与えられた値 / MAXPWMCOUNTがデューティ比となる */
void pwm_proc(void)
{

  switch (oldstatebuf[LEFTSENSOR - 1][oldstatebufdp]) {
    
  case WHITE:
    if (oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] == WHITE) {
      l_ct = MAXPWMCOUNT;
      r_ct = MAXPWMCOUNT;
    }
    else if (oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] == BLACK) {
      l_ct = 50;
      r_ct = 200;
    }
    break;
    
  case BLACK:
    if (oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] == WHITE) {
      l_ct = 200;
      r_ct = 50;
    }
    else if (oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] == BLACK) {
      l_ct = 0;
      r_ct = 0;
    }
    break;
    
  case NONE:
    break;
  }

  if (pwm_count < l_ct) {
    PBDR |= LEFTTURN;
  }
  else {
    PBDR &= ~LEFTTURN;
  }

  if (pwm_count < r_ct) {
    PBDR |= RIGHTTURN;
  }
  else {
    PBDR &= ~RIGHTTURN;
  }
  
  pwm_count++;
  if (pwm_count >= MAXPWMCOUNT) {
    pwm_count = 0;
  }
}

/* 表示する文字列の更新，進行方向を制御する関数 */
void control_proc(void)
{
  int i;
  int tmp;
  char str_online[LCDDISPSIZE + 1] = "ON LINE!";
  char str_offline[LCDDISPSIZE + 1] = " OFFLINE";
  char str_error[LCDDISPSIZE + 1] = "ERROR...";
  
  ad_left_sensor = ad_read(LEFTSENSOR);
  ad_right_sensor = ad_read(RIGHTSENSOR);

  /* A/D変換値の表示の更新 */
  lcd_str_lower[1] = ad_right_sensor / 100 + '0';
  lcd_str_lower[2] = (ad_right_sensor / 10) % 10 + '0';
  lcd_str_lower[3] = ad_right_sensor % 10 + '0';

  lcd_str_lower[5] = ad_left_sensor / 100 + '0';
  lcd_str_lower[6] = (ad_left_sensor / 10) % 10 + '0';
  lcd_str_lower[7] = ad_left_sensor % 10 + '0';

  /*if (old_state[0] == ) {
    for (tmp = wait_time; wait_time >= tmp + 100;);
    wait_time = 0;
  }
  */
  
  
  if (ad_right_sensor < THRESHOLD && ad_left_sensor < THRESHOLD) {
    oldstatebuf[LEFTSENSOR - 1][oldstatebufdp] = WHITE;
    oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] = WHITE;
    for (i = 0; i < LCDDISPSIZE; i++) {
      lcd_str_upper[i] = str_online[i];
    }
  }
  else if (ad_right_sensor < THRESHOLD && ad_left_sensor > THRESHOLD) {
    oldstatebuf[LEFTSENSOR - 1][oldstatebufdp] = BLACK;
    oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] = WHITE;
    lcd_str_upper[0] = 'L';
    for (i = 1; i < LCDDISPSIZE; i++) {
      lcd_str_upper[i] = str_offline[i];
    }
  }
  else if (ad_right_sensor > THRESHOLD && ad_left_sensor < THRESHOLD) {
    oldstatebuf[LEFTSENSOR - 1][oldstatebufdp] = WHITE;
    oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] = BLACK;
    lcd_str_upper[0] = 'R';
    for (i = 1; i < LCDDISPSIZE; i++) {
      lcd_str_upper[i] = str_offline[i];
    }
  }
  else if (ad_right_sensor > THRESHOLD && ad_left_sensor > THRESHOLD) {
    oldstatebuf[LEFTSENSOR - 1][oldstatebufdp] = BLACK;
    oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] = BLACK;
    lcd_str_upper[0] = 'W';
    for (i = 0; i < LCDDISPSIZE; i++) {
      lcd_str_upper[i] = str_offline[i];
    }
  }
  /*else {
    oldstatebuf[LEFTSENSOR - 1][oldstatebufdp] = NONE;
    oldstatebuf[RIGHTSENSOR - 1][oldstatebufdp] = NONE;
    for (i = 0; i < LCDDISPSIZE; i++) {
      lcd_str_upper[i] = str_error[i];
    }
  }
  */

}
