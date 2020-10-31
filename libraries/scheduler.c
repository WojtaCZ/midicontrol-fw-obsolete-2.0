#include "scheduler.h"
#include "oled.h"


#include <libopencm3/stm32/usart.h>

extern void menu_scroll_callback();
Scheduler sched_menu_scroll = {500, 0, &menu_scroll_callback, SCHEDULER_PERIODICAL};

extern void menu_update();
Scheduler sched_menu_update = {30, 0, &menu_update, SCHEDULER_PERIODICAL | SCHEDULER_ON};

extern void led_dev_process_pending_status();
Scheduler sched_led_process = {50, 0, &led_dev_process_pending_status, SCHEDULER_PERIODICAL | SCHEDULER_ON};

extern void io_keypress_callback();
Scheduler sched_io_keypress = {10, 0, &io_keypress_callback, SCHEDULER_PERIODICAL | SCHEDULER_ON};

extern void oled_sleep_callback();
Scheduler sched_oled_sleep = {OLED_SLEEP_INTERVAL, 0, &oled_sleep_callback, SCHEDULER_ON};


void scheduler_check(Scheduler * sched){
    if(((sched->counter) >= (sched->interval)) && !(sched->flags & SCHEDULER_READY) && (sched->flags & SCHEDULER_ON)){
       sched->flags |= SCHEDULER_READY;
       sched->counter = 0;
    }else{
        sched->counter++;
    }
}

void scheduler_process(Scheduler * sched){
    if(sched->flags & SCHEDULER_READY){
        (*sched->callback)(NULL);
        
        if(!(sched->flags & SCHEDULER_PERIODICAL)){
            sched->flags &= ~SCHEDULER_READY;
            sched->flags |= SCHEDULER_COMPLETE;
        }

        sched->flags &= ~SCHEDULER_READY;
    }
}