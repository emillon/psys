/****************************************
 *                                      *
 *               timer.c                *
 *       timing-related routines        *
 *                                      *
 ****************************************/

#include "timer.h"
#include "cpu.h"
#include "irq.h"
#include "context.h"

#include "sys.h"

int rate = 0;

static void timer_setticks(int freq)
{
    rate = TIMER_INPUT_FREQ / freq;
    outb(TIMER_COUNTER(0) | TIMER_LSB_MSB | TIMER_MODE(2), TIMER_CONTROL);
    outb( rate & 0xff        , TIMER_CHANNEL0);
    outb((rate & 0xff00) >> 8, TIMER_CHANNEL0);
}

void wait_clock(unsigned long clock){
    tab_proc[pid_cour-1].state = STATE_ASLEEP;
    tab_proc[pid_cour-1].timer = clock;
    scheduler();
}

unsigned long nb_interrupt = 0;

void update_timer(){
    int i;
    for(i=0; i< NBPROC; i++){
        if (tab_proc[i].valide==1 && tab_proc[i].state == STATE_ASLEEP){
            if (tab_proc[i].timer <= nb_interrupt){
                wakeup_process(i+1);
            }
        }
    }
}

void clock_settings(unsigned long *quartz, unsigned long *ticks)
{
    quartz = check_pointer(quartz, SYSCALL_CLOCK_SETTINGS);
    ticks  = check_pointer(ticks,  SYSCALL_CLOCK_SETTINGS);
    if(quartz) *quartz = TIMER_INPUT_FREQ;
    if(ticks)  *ticks  = rate;
}

unsigned long current_clock(void)
{
    return nb_interrupt;
}

void usleep(unsigned long microseconds)
{
    unsigned long quartz, ticks;
    clock_settings(&quartz, &ticks);
    unsigned long now = current_clock();
    wait_clock(now + (microseconds * quartz * ticks)/(1000*1000));
}

void timer_init(void)
{
    timer_setticks(1);
    init_irqs(IRQ(0));
}
