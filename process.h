#ifndef INC_20241R0136COSE4101_PROCESS_H
#define INC_20241R0136COSE4101_PROCESS_H
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
typedef struct Process {
    int pid;
    int cpu_burst_time;
    int cpu_burst_remain_time;
    int io_burst_time;
    int io_burst_remain_time;
    int arrival_time;
    int priority;
    int waiting_time;
    int turnaround_time;
    int is_complete;
}Process;
typedef Process* ProcessPtr;

typedef struct Node {
    ProcessPtr dataPtr;
    struct Node* next;
}Node;

typedef struct Queue {
    Node *front;
    Node *rear;
    int count;
}Queue;

ProcessPtr createProcess(int size);
void resetProcess(ProcessPtr process_prt, int size);
void showProcessInfo(ProcessPtr process_prt);

Queue* createQueue();
void deleteQueue(Queue* queue);
void enqueue(Queue* queue, ProcessPtr vp);
void dequeue(Queue* queue, void** item);

void sortQueue(Queue *queue, int compareProcesses(const void *a, const void *b));
void processWaitingTime(Queue* ready);
void processIOWorking(Queue* waiting);

#endif //INC_20241R0136COSE4101_PROCESS_H
