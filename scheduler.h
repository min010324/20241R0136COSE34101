#ifndef INC_20241R0136COSE4101_SCHEDULER_H
#define INC_20241R0136COSE4101_SCHEDULER_H
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "process.h"

void schedulerEval(ProcessPtr processPtr, int size);
void scheduleFCFS(ProcessPtr process_ptr, int size);
void scheduleSJF(ProcessPtr process_ptr, int size);
void schedulePRSJF(ProcessPtr process_ptr, int size);
void schedulePRIORITY(ProcessPtr process_ptr, int size);
void schedulePRPRIORITY(ProcessPtr process_ptr, int size);
void scheduleRR(ProcessPtr process_ptr, int size, int time_quantum);
void scheduleMultiLevel(ProcessPtr process_ptr, int size, int time_quantum);
void scheduleMultiLevelFeedBack(ProcessPtr process_ptr, int size, int queue_number);

#endif //INC_20241R0136COSE4101_SCHEDULER_H
