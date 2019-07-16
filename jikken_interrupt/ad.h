#define ADSTATUS() (ADCSR & 0x80)
#define ADREAD() ADDRAH

extern void ad_init();
extern void ad_start(unsigned char ch, unsigned char int_sw);
extern void ad_scan(unsigned char ch_grp, unsigned char int_sw);
extern void ad_stop(void);
