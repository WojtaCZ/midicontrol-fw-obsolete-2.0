#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "global.h"

#define SCHEDULER_ON                0x0001
#define SCHEDULER_READY             0x0002
#define SCHEDULER_PERIODICAL        0x0004
#define SCHEDULER_COMPLETE          0x0008

typedef struct SScheduler {
    //Interval in ms
    int interval;
    //Counter
    int counter;
    //Callback to a function
    void (*callback)(void);
    //Flags
    uint16_t flags;
} Scheduler;


void scheduler_check(Scheduler * sched);
void scheduler_process(Scheduler * sched);

#endif
