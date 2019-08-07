#include "h8-3069-iodef.h"
#include "h8-3069-int.h"

#include "ad.h"
#include "lcd.h"
#include "random.h"
#include "sci2.h"
#include "timer.h"

#define true 1
#define false 0

/* タイマ割り込みの時間間隔[μs] */
#define TIMER0 1000   //1/1000秒

/* 割り込み処理で各処理を行う頻度を決める定数 */
#define DISPTIME 100
#define KEYTIME 1
#define ADTIME  2
#define PWMTIME 1
#define CONTROLTIME 10
#define THRESHOLD 150

//LCD表示関連
//1段に表示できる文字数
#define LCDDISPSIZE 8

//PWM制御関連
//制御周期を決める定数
#define MAXPWMCOUNT 255
#define HALFPWMCOUNT MAXPWMCOUNT/2

//A/D変換関連
//A/D変換のチャネル数とバッファサイズ
#define ADCHNUM   4
#define ADBUFSIZE 8
//平均化するときのデータ個数
#define ADAVRNUM 4
//チャネル指定エラー時に返す値
#define ADCHNONE -1

//割り込み処理に必要な変数は大域変数にとる
volatile int disp_time, ad_time, pwm_time, control_time;

//LCD関係
volatile int disp_flag;
volatile char lcd_str_upper[LCDDISPSIZE+1];
volatile char lcd_str_lower[LCDDISPSIZE+1];

//モータ制御関係
volatile int pwm_count;

//A/D変換関係
volatile unsigned char adbuf[ADCHNUM][ADBUFSIZE];
volatile int adbufdp;

//各値
volatile unsigned char Lval, Rval, Vval;


void port_init();
void create_text();
void pwm_proc();
int ad_read(int ch);
void control_proc();

int main(){
  int start_flag;
  ROMEMU();           /* ROMエミュレーションをON */

  //割り込みで使用する大域変数の初期化
  pwm_time = pwm_count = 0;     /* PWM制御関連 */
  disp_time = 0; disp_flag = 1; /* 表示関連 */
  ad_time = 0;                  /* A/D変換関連 */
  control_time = 0;             /* 制御関連 */
  adbufdp = 0;         /* A/D変換データバッファポインタの初期化 */
  port_init();         /* ポートの初期化 */
  lcd_init();          /* LCD表示器の初期化 */
  ad_init();           /* A/Dの初期化 */
  timer_init();        /* タイマの初期化 */
  timer_set(0,TIMER0); /* タイマ0の時間間隔をセット */
  timer_start(0);      /* タイマ0スタート */
  
  lcd_clear();

  lcd_str_upper[LCDDISPSIZE] = '\0';
  lcd_str_upper[LCDDISPSIZE] = '\0';

  while(P6DR & 0x01 == 0x01){
    
  }
  ENINT();             /* 全割り込み受付可 */
  while(1){
    if(disp_flag == 1){
      create_text();
      lcd_cursor(0,0);
      lcd_printstr(lcd_str_upper);
      lcd_cursor(0,1);
      lcd_printstr(lcd_str_lower);
      disp_flag = 0;
    }
  }
}

//各ポート初期化
void port_init(){
  //モータ使用のポート
  PBDDR = 0x0f;
  //LCD使用のポート
  P4DDR = 0xff;
  PADDR = 0x70;
  //スイッチ使用のポート
  P6DDR = 0x00;//0000 0000
}

//文字列の生成
void create_text(){
  int i, num;
  char off_str[] = "OFF LINE";
  char on_str[] = "ON LINE";
  char sensor_str[] = "L   R   ";

  //配列のコピー
  for(i = 0;i < LCDDISPSIZE;i++){
    if(Lval < 50 && Rval < 50){
      lcd_str_upper[i] = on_str[i];
    }else {
      lcd_str_upper[i] = off_str[i];
    }
    lcd_str_lower[i] = sensor_str[i];
  }

  //右センサーの値
  if(Rval < 10){
    lcd_str_lower[5] = Rval + '0';
    lcd_str_lower[6] = ' ';
    lcd_str_lower[7] = ' ';
  }else if(Rval < 100){
    lcd_str_lower[5] = (Rval / 10) + '0';
    lcd_str_lower[6] = (Rval % 10) + '0';
    lcd_str_lower[7] = ' ';
  }else{
    lcd_str_lower[5] = (Rval / 100) + '0';
    num = Rval - ((lcd_str_lower[5] - '0') * 100);
    num /= 10;
    lcd_str_lower[6] = num + '0';
    lcd_str_lower[7] = (Rval % 10) + '0';
  }

  //左センサーの値
  if(Lval < 10){
    lcd_str_lower[1] = Lval + '0';
    lcd_str_lower[2] = ' ';
    lcd_str_lower[3] = ' ';
  }else if(Rval < 100){
    lcd_str_lower[1] = (Lval / 10) + '0';
    lcd_str_lower[2] = (Lval % 10) + '0';
    lcd_str_lower[3] = ' ';
  }else{
    lcd_str_lower[1] = (Lval / 100) + '0';
    num = Lval - ((lcd_str_lower[1] - '0') * 100);
    num /= 10;
    lcd_str_lower[2] = num + '0';
    lcd_str_lower[3] = (Lval % 10) + '0';
  }
  
}

//割り込み制御の関数
#pragma interrupt
void int_imia0(void){

  //LCDの表示処理
  disp_time++;
  if (disp_time >= DISPTIME){
    disp_flag = 1;
    disp_time = 0;
  }

  //PWMの処理
  pwm_time++;
  if(pwm_time >= PWMTIME){
    pwm_proc();
    pwm_time = 0;
  }

  //A/D変換開始の処理
  ad_time++;
  if(ad_time >= ADTIME){
    ad_scan(0, 1);
    ad_time = 0;
  }

  //制御処理
  control_time++;
  if(control_time >= CONTROLTIME){
    control_proc();
    control_time = 0;

  }
  
  timer_intflag_reset(0); /* 割り込みフラグをクリア */
  ENINT();                /* CPUを割り込み許可状態に */
}

//PWM処理
void pwm_proc(){
  int r_ct, l_ct;
  
  pwm_count++;
  pwm_count %= MAXPWMCOUNT;//範囲：0 ~ 255
  
  if(Rval > THRESHOLD && Lval > THRESHOLD){//直進
    r_ct = MAXPWMCOUNT;
    l_ct = 0;
  }else if(Rval > THRESHOLD){//左旋回
    l_ct = 100;
    r_ct = 200;
    //r_ct = 100;
  }else if(Lval > THRESHOLD){//右旋回
    l_ct = 200;
    //l_ct = 100;
    r_ct = 100;
  }else {//直進
    r_ct = l_ct = MAXPWMCOUNT;
  }
  //r_ct = l_ct = 255;

  //右モータのPWM処理
  if(pwm_count < r_ct){
    PBDR |= 0x01;//前進 0000 0001
  }else{
    PBDR &= 0xfe;//ストップ 1111 1110
  }

  //左モータのPWM処理
  if(pwm_count < l_ct){
    PBDR |= 0x04;//前進 0000 0100
  }else{
    PBDR &= 0xfb;//ストップ 1111 1011
  }
  

}

//A/D変換終了の割り込みハンドラ
#pragma interrupt
void int_adi(void){
  ad_stop();    //A/D変換の停止と変換終了フラグのクリア

  //バッファポインタの更新を行う
  adbufdp++;
  adbufdp %= ADBUFSIZE;//adbufの範囲：0~7
  
  //バッファにA/Dの各チャネルの変換データを入れる
  //スキャングループ 0 を指定した場合は
  //A/D ch0〜3 (信号線ではAN0〜3)の値が ADDRAH〜ADDRDH に格納される
  
  adbuf[0][adbufdp] = ADDRAH;//AN0
  adbuf[1][adbufdp] = ADDRBH;//AN1
  adbuf[2][adbufdp] = ADDRCH;//AN2

  ENINT();      //割り込みの許可
}


//指定チャンネルの平均化した値を返す
int ad_read(int ch){
  int i, ad, bp;
  
  if ((ch > ADCHNUM) || (ch < 0)){ //チャネル範囲のチェック 
    ad = ADCHNONE;
  }else {
    bp = adbufdp + 1;
    ad = 0;
    for (i = 0;i < ADAVRNUM;i++){
      bp--;
      if(bp < 0)bp = 7;
      ad += adbuf[ch][bp];
    }
    ad /= ADAVRNUM;
  }
  return ad;
}

//制御を行う関数
void control_proc(){
  Lval = ad_read(1);
  Rval = ad_read(2);
  //Mval = ad_read(3);
}

