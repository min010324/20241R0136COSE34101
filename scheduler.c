#include "scheduler.h"

int compareArrivalTime(const void *a, const void *b) {
    ProcessPtr procA = *(ProcessPtr*)a;
    ProcessPtr procB = *(ProcessPtr*)b;
    return procA->arrival_time - procB->arrival_time;
}

int comparePriority(const void *a, const void *b) {
    ProcessPtr procA = *(ProcessPtr*)a;
    ProcessPtr procB = *(ProcessPtr*)b;
    return procA->priority - procB->priority;
}

int compareRemainCpuBurst(const void *a, const void *b) {
    ProcessPtr procA = *(ProcessPtr*)a;
    ProcessPtr procB = *(ProcessPtr*)b;
    return procA->cpu_burst_remain_time - procB->cpu_burst_remain_time;
}

int compareRemainIOBurst(const void *a, const void *b) {
    ProcessPtr procA = *(ProcessPtr*)a;
    ProcessPtr procB = *(ProcessPtr*)b;
    return procA->io_burst_remain_time - procB->io_burst_remain_time;
}

void printQueue(Queue *queue){
    Node *node;
    node = queue->front;

    ProcessPtr process;
    while (node != NULL){
        process = (ProcessPtr)node->dataPtr;
        showProcessInfo(process);
        node = node->next;
    }
}

void schedulerEval(ProcessPtr processPtr, int size){
    float avg_waiting = 0;
    float avg_turnaround = 0;
    printf("\n\n#### Scheduler Evaluation:\n");
    for (int i = 0; i < size; i++) {
        printf("PID:%3d    Waiting time:%3d    Turnaround time:%3d\n", processPtr[i].pid, processPtr[i].waiting_time, processPtr[i].turnaround_time);
        avg_waiting += processPtr[i].waiting_time;
        avg_turnaround += processPtr[i].turnaround_time;
    }
    printf("\n### Average waiting time:%.4f    Average turnaround time:%.4f\n\n\n\n", avg_waiting / size, avg_turnaround / size);
}

void config(Queue** job_queue, Queue** ready_queue, Queue** waiting_queue){
    *job_queue = createQueue();
    *ready_queue = createQueue();
    *waiting_queue = createQueue();
}

void scheduleFCFS(ProcessPtr process_ptr, int size){
    Queue* job_queue;
    Queue* ready_queue;
    Queue* waiting_queue;
    config(&job_queue, &ready_queue, &waiting_queue);

    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### FCFS init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue(ready_queue, deletedData);
            current_job_node = current_job_node->next;
        }

        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            if(ready_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }

                dequeue(ready_queue, (void**)&running_process);
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                if(ready_queue->count){ // ready queue -> running
                    dequeue(ready_queue, (void **)&running_process);
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_process->io_burst_remain_time > 0 && rand() % 2 == 0){ // IO Interrupt 랜덤으로 발생
                    printf("%d ] IO interrupt : %d\n", current_time, running_process->io_burst_remain_time); // Interrupt 발생
                    enqueue(waiting_queue, running_process);
                    running_process = NULL;

                    if(ready_queue->count){
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    } else{
                        idle_status = true;
                        printf("### <IDLE> time : [ %d ~ ", current_time);
                    }
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
        }
        processWaitingTime(ready_queue); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue);
    deleteQueue(waiting_queue);
}

void scheduleSJF(ProcessPtr process_ptr, int size){
    Queue* job_queue;
    Queue* ready_queue;
    Queue* waiting_queue;
    config(&job_queue, &ready_queue, &waiting_queue);

    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### Non-Preempt SFJ init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue(ready_queue, deletedData);
            current_job_node = current_job_node->next;
        }

        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            if(ready_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                sortQueue(ready_queue, compareRemainCpuBurst);
                dequeue(ready_queue, (void**)&running_process);
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                if(ready_queue->count){ // ready queue -> running
                    sortQueue(ready_queue, compareRemainCpuBurst);
                    dequeue(ready_queue, (void **)&running_process);
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_process->io_burst_remain_time > 0 && rand() % 2 == 0){ // IO Interrupt 랜덤으로 발생
                    printf("%d ] IO interrupt : %d\n", current_time, running_process->io_burst_remain_time); // Interrupt 발생
                    enqueue(waiting_queue, running_process);
                    running_process = NULL;

                    if(ready_queue->count){
                        sortQueue(ready_queue, compareRemainCpuBurst);
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    } else{
                        idle_status = true;
                        printf("### <IDLE> time : [ %d ~ ", current_time);
                    }
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
        }
        processWaitingTime(ready_queue); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue);
    deleteQueue(waiting_queue);
}


void schedulePRSJF(ProcessPtr process_ptr, int size){
    Queue* job_queue;
    Queue* ready_queue;
    Queue* waiting_queue;
    config(&job_queue, &ready_queue, &waiting_queue);

    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### Preempt SFJ init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue(ready_queue, deletedData);
            current_job_node = current_job_node->next;
        }

        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            if(ready_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                sortQueue(ready_queue, compareRemainCpuBurst);
                dequeue(ready_queue, (void**)&running_process);
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                if(ready_queue->count){ // ready queue -> running
                    sortQueue(ready_queue, compareRemainCpuBurst);
                    dequeue(ready_queue, (void **)&running_process);
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_process->io_burst_remain_time > 0 && rand() % 2 == 0){ // IO Interrupt 랜덤으로 발생
                    printf("%d ] IO interrupt : %d\n", current_time, running_process->io_burst_remain_time); // Interrupt 발생
                    enqueue(waiting_queue, running_process);
                    running_process = NULL;

                    if(ready_queue->count){
                        sortQueue(ready_queue, compareRemainCpuBurst);
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    } else{
                        idle_status = true;
                        printf("### <IDLE> time : [ %d ~ ", current_time);
                    }
                } else{ //for preempt
                    sortQueue(ready_queue, compareRemainCpuBurst);
                    if(ready_queue->count > 0 && ready_queue->front->dataPtr->cpu_burst_remain_time < running_process->cpu_burst_remain_time){
                        printf("%d ] preempted\n", current_time); // running process 완료 처리
                        enqueue(ready_queue, running_process);
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    }
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
        }
        processWaitingTime(ready_queue); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue);
    deleteQueue(waiting_queue);
}


void schedulePRIORITY(ProcessPtr process_ptr, int size){
    Queue* job_queue;
    Queue* ready_queue;
    Queue* waiting_queue;
    config(&job_queue, &ready_queue, &waiting_queue);

    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### Non-Preempt Priority init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue(ready_queue, deletedData);
            current_job_node = current_job_node->next;
        }

        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            if(ready_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                sortQueue(ready_queue, comparePriority);
                dequeue(ready_queue, (void**)&running_process);
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                if(ready_queue->count){ // ready queue -> running
                    sortQueue(ready_queue, comparePriority);
                    dequeue(ready_queue, (void **)&running_process);
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_process->io_burst_remain_time > 0 && rand() % 2 == 0){ // IO Interrupt 랜덤으로 발생
                    printf("%d ] IO interrupt : %d\n", current_time, running_process->io_burst_remain_time); // Interrupt 발생
                    enqueue(waiting_queue, running_process);
                    running_process = NULL;

                    if(ready_queue->count){
                        sortQueue(ready_queue, comparePriority);
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    } else{
                        idle_status = true;
                        printf("### <IDLE> time : [ %d ~ ", current_time);
                    }
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
        }
        processWaitingTime(ready_queue); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue);
    deleteQueue(waiting_queue);
}

void schedulePRPRIORITY(ProcessPtr process_ptr, int size){
    Queue* job_queue;
    Queue* ready_queue;
    Queue* waiting_queue;
    config(&job_queue, &ready_queue, &waiting_queue);

    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### Preempt Priority init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue(ready_queue, deletedData);
            current_job_node = current_job_node->next;
        }

        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            if(ready_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                sortQueue(ready_queue, comparePriority);
                dequeue(ready_queue, (void**)&running_process);
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                if(ready_queue->count){ // ready queue -> running
                    sortQueue(ready_queue, comparePriority);
                    dequeue(ready_queue, (void **)&running_process);
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_process->io_burst_remain_time > 0 && rand() % 2 == 0){ // IO Interrupt 랜덤으로 발생
                    printf("%d ] IO interrupt : %d\n", current_time, running_process->io_burst_remain_time); // Interrupt 발생
                    enqueue(waiting_queue, running_process);
                    running_process = NULL;

                    if(ready_queue->count){
                        sortQueue(ready_queue, comparePriority);
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    } else{
                        idle_status = true;
                        printf("### <IDLE> time : [ %d ~ ", current_time);
                    }
                } else{ //for preempt
                    sortQueue(ready_queue, comparePriority);
                    if(ready_queue->count > 0 && ready_queue->front->dataPtr->priority < running_process->priority){
                        printf("%d ] preempted\n", current_time); // running process 완료 처리
                        enqueue(ready_queue, running_process);
                        dequeue(ready_queue, (void **)&running_process);
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    }
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
        }
        processWaitingTime(ready_queue); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue);
    deleteQueue(waiting_queue);
}

void scheduleRR(ProcessPtr process_ptr, int size, int time_quantum){
    Queue* job_queue;
    Queue* ready_queue;
    Queue* waiting_queue;
    config(&job_queue, &ready_queue, &waiting_queue);

    int current_time = 0;
    int running_quantum = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### Round Robin init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue(ready_queue, deletedData);
            current_job_node = current_job_node->next;
        }

        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            if(ready_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                dequeue(ready_queue, (void**)&running_process);
                running_quantum = 0;
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                if(ready_queue->count){ // ready queue -> running
                    dequeue(ready_queue, (void **)&running_process);
                    running_quantum = 0;
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_process->io_burst_remain_time > 0 && rand() % 2 == 0){ // IO Interrupt 랜덤으로 발생
                    printf("%d ] IO interrupt : %d\n", current_time, running_process->io_burst_remain_time); // Interrupt 발생
                    enqueue(waiting_queue, running_process);
                    running_process = NULL;

                    if(ready_queue->count){
                        dequeue(ready_queue, (void **)&running_process);
                        running_quantum = 0;
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    } else{
                        idle_status = true;
                        printf("### <IDLE> time : [ %d ~ ", current_time);
                    }
                } else{ //for preempt
                    if(running_quantum == time_quantum){
                        printf("%d ] preempted\n", current_time); // running process 완료 처리
                        enqueue(ready_queue, running_process);
                        dequeue(ready_queue, (void **)&running_process);
                        running_quantum = 0;
                        printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                    }
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
            running_quantum++;
        }
        processWaitingTime(ready_queue); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue);
    deleteQueue(waiting_queue);
}

void selectMode(Queue **target_queue, Queue **ready_queue_foreground, Queue **ready_queue_background){
    if(rand() % 10 > 7){ //cpu를 할당할 ready queue 선택
        *target_queue = *ready_queue_background;
        if((*target_queue)->count == 0) {
            *target_queue = *ready_queue_foreground;
        }
    } else {
        *target_queue = *ready_queue_foreground;
        if((*target_queue)->count == 0) {
            *target_queue = *ready_queue_background;
        }
    }
}

void scheduleMultiLevel(ProcessPtr process_ptr, int size, int time_quantum){
    Queue* job_queue = createQueue();
    Queue* ready_queue_foreground = createQueue();
    Queue* ready_queue_background = createQueue();
    Queue* target_queue = createQueue();
    Queue* waiting_queue = createQueue();
    int running_quantum = 0;

    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        process_ptr[i].io_burst_remain_time = 0; // ignore IO
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### Multilevel init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && ready_queue_foreground->count == 0 && ready_queue_background->count == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);

            if(deletedData->priority > 10){
                enqueue(ready_queue_background, deletedData);
            } else{
                enqueue(ready_queue_foreground, deletedData);
            }
            current_job_node = current_job_node->next;
        }
        if(current_time == 0){
            printf("\n### Ready Queue foreground\n");
            printQueue(ready_queue_foreground);
            printf("\n### Ready Queue background\n");
            printQueue(ready_queue_background);
        }


//        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    running_quantum = 0;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        if(current_waiting->priority > 10){
                            enqueue(ready_queue_background, current_waiting);
                        } else{
                            enqueue(ready_queue_foreground, current_waiting);
                        }
//                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            selectMode(&target_queue, &ready_queue_foreground, &ready_queue_background);
            if(target_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                running_quantum = 0;
                dequeue(target_queue, (void**)&running_process);
                if(target_queue == ready_queue_foreground) running_quantum = 0;
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                selectMode(&target_queue, &ready_queue_foreground, &ready_queue_background);
                if(target_queue->count){ // ready queue -> running
                    running_quantum = 0;
                    dequeue(target_queue, (void **)&running_process);
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_quantum == time_quantum && running_process->priority <= 10){
                    printf("%d ] preempted\n", current_time); // running process 완료 처리
                    enqueue(target_queue, running_process);

                    selectMode(&target_queue, &ready_queue_foreground, &ready_queue_background);
                    dequeue(target_queue, (void **)&running_process);
                    running_quantum = 0;
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
            running_quantum++;
        }
        processWaitingTime(ready_queue_foreground); // ready queue에 있는 process의 waiting time 증가
        processWaitingTime(ready_queue_background); // ready queue에 있는 process의 waiting time 증가
        current_time++;
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    deleteQueue(ready_queue_foreground);
    deleteQueue(ready_queue_background);
    deleteQueue(waiting_queue);
}


void scheduleMultiLevelFeedBack(ProcessPtr process_ptr, int size, int queue_number){
    Queue* job_queue = createQueue();
    Queue** ready_queue_list = (Queue **) malloc(sizeof(Queue*) * queue_number);
    Queue* waiting_queue = createQueue();
    Queue* target_queue;
    for(int i =0; i< queue_number; i++){
        (ready_queue_list)[i] = createQueue();
    }

    int current_queue = 0;
    int running_quantum = 0;
    int total_ready_cnt = 0;
    int current_time = 0;
    ProcessPtr running_process = NULL;
    bool idle_status = false;

    for(int i=0; i<size; i++){
        process_ptr[i].io_burst_remain_time = 0; // ignore IO
        enqueue(job_queue,&process_ptr[i]);
    }

    sortQueue(job_queue, compareArrivalTime); // 도착순으로 정렬
    printf("### MultiLevel Feedback init Job Queue\n");
    printQueue(job_queue);

    printf("\n### Gantt Chart ###\n");
    while (!(job_queue->count == 0 && total_ready_cnt == 0 && waiting_queue->count == 0 && running_process == NULL)){ //scheduler 시작
        int time_quantum = (current_queue + 1) * 4;

        Node *current_job_node = job_queue->front;
        while (current_job_node != NULL){ // job queue -> ready queue
            if(current_job_node->dataPtr->arrival_time != current_time) {
                break;
            }

            ProcessPtr deletedData;
            dequeue(job_queue, (void **)&deletedData);
            enqueue((ready_queue_list)[0], deletedData);
            current_job_node = current_job_node->next;
        }
        if(current_time == 0){
            for(int i =0; i< queue_number; i++){
                printf("\nReady Queue Index : %d\n", i);
                printQueue((ready_queue_list)[i]);
            }
            printf("\n\n");
        }


//        processIOWorking(waiting_queue); // 기존에 끝난 waiting queue의 작업을 이동시킨 뒤 IO operation 수행

        if(waiting_queue->count > 0){ // waiting queue -> ready queue
            sortQueue(waiting_queue, compareRemainIOBurst);
            Node *current_waiting_node = waiting_queue->front;
            while (current_waiting_node != NULL){
                if(current_waiting_node->dataPtr->io_burst_remain_time <= 0){ // IO 종료
                    ProcessPtr current_waiting;
                    running_quantum = 0;
                    dequeue(waiting_queue, (void **)&current_waiting);

                    if (current_waiting->cpu_burst_remain_time > 0){ // 아직 끝나지 않은 process인 경우에만 ready queue로 이동
                        enqueue((ready_queue_list)[current_queue+1], current_waiting);
//                        enqueue(ready_queue, current_waiting);
                    }
                }
                current_waiting_node = current_waiting_node->next;
            }
        }

        if(running_process == NULL){ //CPU 사용 가능
            target_queue = (ready_queue_list)[current_queue];
            if(target_queue->count){ // ready queue에서 running으로 process 이동
                if(idle_status){ // 기존에 CPU가 IDLE 상태였는지 확인
                    idle_status = false;
                    printf("%d ]\n", current_time);
                }
                running_quantum = 0;
                dequeue(target_queue, (void**)&running_process);
                if((ready_queue_list)[current_queue]->count == 0) current_queue++;
//                if(target_queue == ready_queue_foreground) running_quantum = 0;
                printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);

            } else{ // ready queue 비어있는 idle 상황
                if(!idle_status){
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            }
        } else {
            if(running_process->cpu_burst_remain_time <= 0){ //남은 cpu burst 없음
                if(running_process->io_burst_remain_time <= 0){ // 남은 IO 없음
                    printf("%d ]\n", current_time); // running process 완료 처리
                } else { // IO interrupt 발생
                    printf("%d ] IO interrupt : %d, Actually Finish : %d\n", current_time, running_process->io_burst_remain_time, current_time + running_process->io_burst_remain_time); // running process 완료 처리
//                    printf("%d ] IO interrupt : %d, Actually Finish \n", current_time, running_process->io_burst_remain_time); // running process 완료 처리
                    enqueue(waiting_queue, running_process);
                }

                running_process->turnaround_time = current_time - running_process->arrival_time + running_process->io_burst_remain_time;
                running_process = NULL;

                target_queue = (ready_queue_list)[current_queue];
                if(target_queue != NULL && target_queue->count){ // ready queue -> running
                    running_quantum = 0;
                    dequeue(target_queue, (void **)&running_process);
                    if((ready_queue_list)[current_queue]->count == 0) current_queue++;
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                } else { // ready queue is empty! IDLE
                    idle_status = true;
                    printf("### <IDLE> time : [ %d ~ ", current_time);
                }
            } else{ // running process 동작
                if(running_quantum == time_quantum && current_queue != (queue_number -1)){
                    printf("%d ] preempted\n", current_time); // running process 완료 처리
                    enqueue((ready_queue_list)[current_queue+1], running_process);

                    target_queue = (ready_queue_list)[current_queue];
                    dequeue(target_queue, (void **)&running_process);
                    if((ready_queue_list)[current_queue]->count == 0) current_queue++;
                    running_quantum = 0;
                    printf("<PID : %d> Process running [ %d ~ ", running_process->pid, current_time);
                }
            }
        }

        if(running_process){
            (running_process->cpu_burst_remain_time)--; //동작중인 process 처리
            running_quantum++;
        }

        for(int i =0; i< queue_number; i++){
            processWaitingTime((ready_queue_list)[i]); // ready queue에 있는 process의 waiting time 증가
        }
        current_time++;

        total_ready_cnt = 0;
        for(int i =0; i< queue_number; i++){
            total_ready_cnt += (ready_queue_list)[i]->count;
        }
    }

    printf("]\n");
    schedulerEval(process_ptr, size);
    deleteQueue(job_queue);
    for(int i =0; i< queue_number; i++){
        deleteQueue((ready_queue_list)[i]);
    }
    deleteQueue(waiting_queue);
    free(ready_queue_list);
}