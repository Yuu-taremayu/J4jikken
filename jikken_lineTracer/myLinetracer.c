#include "h8-3069-iodef.h"
#include "h8-3052-int.h"

#include "lcd.h"
#include "ad.h"
#include "timer.h"

/* タイマ割り込みの時間間隔[μs] */
#define TIMER0 1000

/* 割り込み処理で各処理を行う頻度を決める定数 */
#define ADTIME  2
#define PWMTIME 1

/* PWM制御関連 */
/* 制御周期を決める定数 */
#define MAXPWMCOUNT 255

/* A/D変換関連 */
/* A/D変換のチャネル数とバッファサイズ */
#define ADCHNUM   4
#define ADBUFSIZE 8
/* 平均化するときのデータ個数 */
#define ADAVRNUM 4
/* チャネル指定エラー時に返す値 */
#define ADCHNONE -1

/* 割り込み処理に必要な変数は大域変数にとる */
volatile int ad_time, pwm_time;

/* モータ制御関係 */
volatile int pwm_count;

/* A/D変換関係 */
volatile unsigned char adbuf[ADCHNUM][ADBUFSIZE];
volatile int adbufdp;

int main(void)
{
  ROMEMU();

  /* 割り込みで使用する大域変数の初期化 */
  pwm_time = pwm_count = 0;     /* PWM制御関連 */
  ad_time = 0;                  /* A/D変換関連 */
  adbufdp = 0;         /* A/D変換データバッファポインタの初期化 */
  ad_init();           /* A/Dの初期化 */
  timer_init();        /* タイマの初期化 */
  timer_set(0,TIMER0); /* タイマ0の時間間隔をセット */
  timer_start(0);      /* タイマ0スタート */
  ENINT();             /* 全割り込み受付可 */
  
  PBDDR |= 0x0f;
  
  return 0;
}

#pragma interrupt
void int_imia0(void)
{
  pwm_time++;
  if (pwm_time >= PWMTIME) {
    pwm_proc();
    pwm_time = 0;
  }
  
  timer_intflag_reset(0); /* 割り込みフラグをクリア */
  ENINT();                /* CPUを割り込み許可状態に */
}

#pragma interrupt
void int_adi(void)
{
  ad_stop();    /* A/D変換の停止と変換終了フラグのクリア */

  adbufdp = (adbufdp + 1) % ADBUFSIZE;

  adbuf[0][adbufdp] = ADDRAH;
  adbuf[1][adbufdp] = ADDRBH;
  adbuf[2][adbufdp] = ADDRCH;
  adbuf[3][adbufdp] = ADDRDH;
  
  ENINT();      /* 割り込みの許可 */
}

void pwm_proc(void)
{
  if (pwm_count < 10);
  /* カウント数を更新し，MAXPWMCOUNTに等しくなったらリセットする */
  pwm_count++;
  if (pwm_count >= MAXPWMCOUNT) {
    pwm_count = 0;
  }
}
