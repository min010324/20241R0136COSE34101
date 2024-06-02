#include "process.h"

ProcessPtr createProcess(int size){
    ProcessPtr process_prt = (ProcessPtr) calloc(sizeof(Process), size);
    srand((unsigned int)time(NULL));

    for(int i =0; i<size; i++){
        process_prt[i].pid = i;
        process_prt[i].priority = rand() % 20 +1;
        process_prt[i].arrival_time = rand() % 20;
//        process_prt[i].arrival_time = 0; // test for non-*
//        process_prt[i].arrival_time = i; // test for preempt-*
        process_prt[i].cpu_burst_time = rand() % 20 +1;
        process_prt[i].cpu_burst_remain_time = process_prt[i].cpu_burst_time;
        process_prt[i].io_burst_time = 0;
        if(rand() % 2){
            process_prt[i].io_burst_time = rand() % 20 +1;
        }
        process_prt[i].io_burst_remain_time = process_prt[i].io_burst_time;
        process_prt[i].is_complete = 0;
        process_prt[i].waiting_time = 0;
        process_prt[i].turnaround_time = 0;
    }

    printf("\nsuccessfully create process!\n\n");
    printf("\nProcess List\n");
    for(int i =0; i<size; i++){
        showProcessInfo(&process_prt[i]);
    }

    return process_prt;


}

void resetProcess(ProcessPtr process_prt, int size){
    for(int i =0; i<size; i++){
        process_prt[i].cpu_burst_remain_time = process_prt[i].cpu_burst_time;
        process_prt[i].io_burst_remain_time = process_prt[i].io_burst_time;
        process_prt[i].is_complete = 0;
        process_prt[i].waiting_time = 0;
        process_prt[i].turnaround_time = 0;
    }
    printf("\nsuccessfully reset process!\n\n");

}

void showProcessInfo(ProcessPtr process_prt){
    printf("PID:%d\tpriority:%d\tarrival time:%d\tcpu burst time:%d\tio burst time:%d\t\n", process_prt->pid, process_prt->priority, process_prt->arrival_time, process_prt->cpu_burst_time, process_prt->io_burst_time);
}

Queue* createQueue(){
    Queue* queue;
    queue = (Queue *) malloc(sizeof(Queue));
    if(queue){
        queue->front = NULL;
        queue->rear = NULL;
        queue->count = 0;
    }
    return queue;
}

void deleteQueue(Queue* queue){
    if (queue->count){
        ProcessPtr process_ptr;
        while (queue->count > 0){
            dequeue(queue, (void **)&process_ptr);
        }
    }
    free(queue);
}

void enqueue(Queue* queue, ProcessPtr vp){
    Node *newNode;
    newNode = (Node *) malloc(sizeof(Node));
    if(!newNode) return;
    if(vp == NULL) {
        printf("NULL!!!\n");
        return;
    }
    newNode->dataPtr = vp;
    newNode->next = NULL;
    if(queue->count == 0) {
        queue->front = newNode;
    } else{
        queue->rear->next = newNode;
    }
    (queue->count)++;
    queue->rear = newNode;
}
void dequeue(Queue* queue, void** item){
    Node *deleteLoc;
    if(!queue->count) return;

    *item = queue->front->dataPtr;
    if(*item == NULL) {
        printf("NULL!!!\n");
        return;
    }
    deleteLoc = queue->front;
    if(queue->count == 1) {
        queue->rear = queue->front = NULL;
    } else {
        queue->front = queue->front->next;
    }
    (queue->count)--;
//    free(deleteLoc);
}

void sortQueue(Queue *queue, int compareProcesses(const void *a, const void *b)){
    if (queue->count <= 1) return;

    ProcessPtr* process_arr = (ProcessPtr *) malloc(queue->count * sizeof(ProcessPtr));
    if(process_arr == NULL) return;

    Node *current_node = queue->front;
    for (int i = 0; i < queue->count; i++) {
        process_arr[i] = (ProcessPtr)current_node->dataPtr;
        current_node = current_node->next;
    }

    qsort(process_arr, queue->count, sizeof(ProcessPtr), compareProcesses);

    current_node = queue->front;
    for (int i = 0; i < queue->count; i++) {
        current_node->dataPtr = process_arr[i];
        if (current_node->next != NULL) {
            current_node = current_node->next;
        }
    }
    free(process_arr);
}

void processWaitingTime(Queue* ready){
    if(ready->count == 0) return;

    Node *current = ready->front;
    while (current != NULL){
        (current->dataPtr->waiting_time)++;
        current = current->next;
    }
}

void processIOWorking(Queue* waiting){
    if(waiting->count == 0) return;

    Node *current = waiting->front;
    while (current != NULL){
        if(current->dataPtr->io_burst_remain_time > 0){
            (current->dataPtr->io_burst_remain_time)--;
        }
        current = current->next;
    }
}
