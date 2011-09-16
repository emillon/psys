#ifndef _TIMER_H
#define _TIMER_H 1

#define TIMER_INPUT_FREQ  0x1234DD  /* 1 193 181 Hz */

#define TIMER_CHANNEL0  0x40
#define TIMER_CHANNEL1  0x41
#define TIMER_CHANNEL2  0x42
#define TIMER_CONTROL   0x43

/* Control word */
#define TIMER_COUNTER(x)        ((x & 3) << 6)
    /* Read/load types */
#define TIMER_COUNTER_LATCH     0 
#define TIMER_LSB_ONLY          1
#define TIMER_MSB_ONLY          2
#define TIMER_LSB_MSB           3
    /* Modes */
#define TIMER_MODE(x)           ((x&7)<<1)
    /* BCD mode */
#define TIMER_BCD               1

extern int rate;
extern unsigned long nb_interrupt;

void wait_clock(unsigned long clock);
void update_timer(void);

void timer_init(void);
#endif /* timer.h */
