#include "scheduler.h"


extern void menu_scroll_callback();
Scheduler sched_menu_scroll = {500, &menu_scroll_callback, SCHEDULER_PERIODICAL};

extern void menu_update();
Scheduler sched_menu_update = {30, &menu_update, SCHEDULER_PERIODICAL | SCHEDULER_ON};

extern void led_dev_process_pending_status();
Scheduler sched_led_process = {50, &led_dev_process_pending_status, SCHEDULER_PERIODICAL | SCHEDULER_ON};

extern void io_keypress_callback();
Scheduler sched_io_keypress = {10, &io_keypress_callback, SCHEDULER_PERIODICAL | SCHEDULER_ON};




void scheduler_check(int time, Scheduler * sched){
    if(!(time % sched->interval) && !(sched->flags & SCHEDULER_READY) && (sched->flags & SCHEDULER_ON)){
       sched->flags |= SCHEDULER_READY;
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