#ifndef MAIN_H
#define MAIN_H

#include "scheduler.h"

void enc_inc(void);
void enc_dec(void);
void enc_psh(void);

extern struct Input encoder;

extern Scheduler sched_menu_scroll;
extern Scheduler sched_menu_update;
extern Scheduler sched_led_process;
extern Scheduler sched_io_keypress;
extern Scheduler sched_oled_sleep;

#endif
