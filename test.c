#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PROCESS_COUNT 5
#define IO_OPERATION_OCCUR 2000
#define IO_OPERATION_TIME 5

static int process_pid_count = 1;
static int cpu_time = 0;
static struct process_* process_list[PROCESS_COUNT];
//static struct process_* dup_process_list[PROCESS_COUNT];

static int cpu_mutex = 0; // 0은 CPU 사용 x, 1은 CPU 사용 중

typedef struct process_ { // 프로세스 구조체
    int ProcessID;
    int CPU_burst_time;
    int CPU_bursted_time;
    int I_O_burst_time;
    int Arrival_time;
    int Priority;
    struct process_* next;
} process;

typedef struct Queue_ { // 프로세스 큐 구조체
    process* front;
    process* rear;
} Queue;

typedef struct result_ {
    int avr_waiting_time;
    int avr_turnaround_time;
} result;

result schedule_result[6]; //스케쥴러 값 저장

//간트차트를 위한 배열
int gant_chart[6][150]; //1부터 차례로 fcfs non_preemp~~

Queue* create_queue();
void enqueue(Queue* q, process* p);
process* dequeue(Queue* q);
void swap_node(Queue* q, process* a, process* b);
process* create_process(int CPU_burst_time, int I_O_burst_time, int Arrival_time, int Priority);
void config(Queue** ready_queue, Queue** waiting_queue);
void schedule();
void evaluation(result, result, result, result, result, result);
void CPU_Scheduling_Simulator();
void create_random_processes(process* process_list[PROCESS_COUNT]);
//void create_dup_procesess(process* process_list[PROCESS_COUNT], process* dup_process_list[PROCESS_COUNT]);
int dispatch_cpu(process* process, int burst_time, int* time);
int is_IO_occur(int time);
void update_waiting_queue(Queue* waiting_queue, Queue* ready_queue, int current_time);
void enqueue_by_arrive_time(Queue* ready_queue, process* process_list[PROCESS_COUNT], int time);
void fcfs_scheduler(Queue* ready_queue, Queue* waiting_queue, result* result);
void non_preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res);
void preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res);
void non_preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res);
void preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res);
void round_robin(Queue* ready_queue, Queue* waiting_queue, result* res);
void init();
void clear_screen();
int result_view();
void reset_queue(Queue** ready_queue, Queue** waiting_queue);
void reset_process(process* process_list[PROCESS_COUNT]);
void print_gantt_chart(int, int);

Queue** ready_queue;
Queue** waiting_queue;

int main(void) {
    CPU_Scheduling_Simulator(); // 시뮬레이터 실행
    return 0;
}



void CPU_Scheduling_Simulator() {
    while (1) {
        init();
        config(&ready_queue, &waiting_queue); // 시스템 환경 설정
        create_random_processes(process_list); // 랜덤한 방법으로 평가를 위한 프로세스 생성
        // create_dup_procesess(process_list, dup_process_list); //프로세스 복사본 만듬
        schedule(); // 스케줄링 시작
        int n=  result_view();
        if (n == -1) {
            break;
        }
    }
}

Queue* create_queue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, process* p) {
    p->next = NULL; // 새로 추가되는 프로세스의 next 포인터를 NULL로 설정

    if (q->rear == NULL) {
        // 큐가 비어 있는 경우
        q->front = q->rear = p;
    }
    else {
        // 큐가 비어 있지 않은 경우
        q->rear->next = p;
        q->rear = p;
    }
}

process* dequeue(Queue* q) {
    if (q == NULL || q->front == NULL) {
        return NULL; // 큐가 비어 있거나 NULL인 경우
    }

    process* temp = q->front;
    q->front = q->front->next; // front를 다음 노드로 업데이트

    if (q->front == NULL) { // 큐에 원소가 하나만 있었던 경우
        q->rear = NULL; // rear도 NULL로 설정
    }

    temp->next = NULL; // 반환된 노드의 다음 노드를 NULL로 설정하여 큐와의 연결 끊기
    return temp;
}

process* create_process(int CPU_burst_time, int I_O_burst_time, int Arrival_time, int Priority) {
    process* new_process = (process*)malloc(sizeof(process));

    // PID 값 할당
    if (process_pid_count != 32769) {
        new_process->ProcessID = process_pid_count++;
    }
    else {
        new_process->ProcessID = 1;
    }
    // 나머지 값 할당
    new_process->CPU_burst_time = CPU_burst_time;
    new_process->CPU_bursted_time = 0;
    new_process->I_O_burst_time = I_O_burst_time;
    new_process->Arrival_time = Arrival_time;
    new_process->Priority = Priority;
    new_process->next = NULL;

    return new_process;
}

// waiting 큐와 ready 큐 환경설정
void config(Queue** ready_queue, Queue** waiting_queue) {
    // 시뮬레이터를 위한 큐 생성
    *ready_queue = create_queue();
    *waiting_queue = create_queue();

    //간트차트 초기화
    for (int i = 0; i < 6; i++) {
        for (int k = 0; k < 150; k++) {
            gant_chart[i][k] = -1;
        }
    }
}

// 랜덤한 방법으로 평가를 위한 프로세스 임의의 개수만큼 생성
void create_random_processes(process* process_list[PROCESS_COUNT] ) {
    srand(time(NULL)); // 난수 초기화

    // 프로세스 생성, 값은 랜덤
    for (int i = 0; i < PROCESS_COUNT; i++) {
        int cpu_burst_time = rand() % 20 + 1; // 1과 20 사이
        int I_O_burst_time = rand() % 20 + 1; // 1과 20 사이
        int Arrival_time = rand() % 50 + 1; // 1과 50 사이
        int Priority = rand() % 100 + 1; // 1과 10 사이

        process* temp;
        //process* temp2;

        // 프로세스 생성 후 리스트에 집어 넣는다
        temp = create_process(cpu_burst_time, I_O_burst_time, Arrival_time, Priority);
        //temp2 = create_process(cpu_burst_time, I_O_burst_time, Arrival_time, Priority);
        process_list[i] = temp;
        //dup_process_list[i] = temp2;

    }

}

// 스케쥴링 & 평가단계
void schedule() {


    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result fcfs_result;
    fcfs_scheduler(ready_queue, waiting_queue, &fcfs_result);
    printf("FCFS scheduling complete.\n");



    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result non_preemptive_sjf_result;
    non_preemptive_sjf(ready_queue, waiting_queue, &non_preemptive_sjf_result);
    printf("Non-preemptive SJF scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result preemptive_sjf_result;
    preemptive_sjf(ready_queue, waiting_queue, &preemptive_sjf_result);
    printf("Preemptive SJF scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result non_preemptive_priority_result;
    non_preemptive_priority(ready_queue, waiting_queue, &non_preemptive_priority_result);
    printf("Non-preemptive Priority scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result preemptive_priority_result;
    preemptive_priority(ready_queue, waiting_queue, &preemptive_priority_result);
    printf("Preemptive Priority scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result round_robin_result;
    round_robin(ready_queue, waiting_queue, &round_robin_result);
    printf("Round Robin scheduling complete.\n");

    evaluation(fcfs_result, non_preemptive_sjf_result, preemptive_sjf_result, non_preemptive_priority_result, preemptive_priority_result, round_robin_result); // 평가
}

void fcfs_scheduler(Queue* ready_queue, Queue* waiting_queue, result* result) {
    cpu_time = 0;
    int turnaround_time;
    int waiting_time;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;

    while (cpu_time < 150) {

        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {

            process* current = ready_queue->front;
            process* earliest_process = current;
            while (current != NULL) {
                if (current->Arrival_time < earliest_process->Arrival_time) {
                    earliest_process = current;
                }
                current = current->next;
            }


            if (earliest_process != ready_queue->front) {
                swap_node(ready_queue, ready_queue->front, earliest_process);
            }

            process* execute_process = ready_queue->front;

            cpu_mutex = 1;
            for (int i = 0; i < execute_process->CPU_burst_time; i++) {
                gant_chart[0][cpu_time] = execute_process->ProcessID;
                cpu_time++;

                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    execute_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    dequeue(ready_queue);
                    enqueue(waiting_queue, execute_process);
                    cpu_mutex = 0;
                    break;
                }
                else {
                    execute_process->CPU_bursted_time += 1;
                }
            }

            cpu_mutex = 0;
            if (execute_process->CPU_bursted_time == execute_process->CPU_burst_time) {
                turnaround_time = cpu_time - execute_process->Arrival_time;
                waiting_time = turnaround_time - execute_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;

                dequeue(ready_queue);
            }

        }
        else {
            gant_chart[0][cpu_time] = -1; //-1은 아무것도 실행하지않았다는 뜻
            cpu_time++;
        }
    }

    result->avr_turnaround_time = turnaround_time_sum / count;
    result->avr_waiting_time = waiting_time_sum / count;
}

void non_preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res) {
    cpu_time = 0;
    int turnaround_time;
    int waiting_time;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    cpu_mutex = 0;

    while (cpu_time < 150) {

        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
            process* shortest_process = ready_queue->front;
            process* current = ready_queue->front;
            process* previous = NULL;
            process* shortest_previous = NULL;

            // Find the process with the shortest CPU burst time
            while (current != NULL) {
                if (current->CPU_burst_time < shortest_process->CPU_burst_time) {
                    shortest_process = current;
                    shortest_previous = previous;
                }
                previous = current;
                current = current->next;
            }

            cpu_mutex = 1;
            int burst_time = shortest_process->CPU_burst_time - shortest_process->CPU_bursted_time;
            for (int i = 0; i < burst_time; i++) {
                gant_chart[1][cpu_time] = shortest_process->ProcessID;
                cpu_time++;
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
                update_waiting_queue(waiting_queue, ready_queue, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    shortest_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    if (shortest_previous == NULL) {
                        ready_queue->front = shortest_process->next;
                    }
                    else {
                        shortest_previous->next = shortest_process->next;
                    }
                    enqueue(waiting_queue, shortest_process);
                    cpu_mutex = 0;
                    break;
                }
                else {
                    shortest_process->CPU_bursted_time += 1;
                }
            }
            cpu_mutex = 0;
            if (shortest_process->CPU_bursted_time >= shortest_process->CPU_burst_time) {
                turnaround_time = cpu_time - shortest_process->Arrival_time;
                waiting_time = turnaround_time - shortest_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                cpu_mutex = 0;
                if (shortest_previous == NULL) {
                    ready_queue->front = shortest_process->next;
                }
                else {
                    shortest_previous->next = shortest_process->next;
                }
            }
        }
        else {
            gant_chart[1][cpu_time] = -1;
            cpu_time++;
            cpu_mutex = 0;
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}

void preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res) {
    cpu_time = 0;
    int turnaround_time;
    int waiting_time;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    cpu_mutex = 0;

    while (cpu_time < 150) {
        // 현재 시간에 도착한 프로세스를 준비 큐에 추가
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

        // 대기 큐의 프로세스를 업데이트하고 필요한 경우 준비 큐로 이동
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if (ready_queue->front != NULL) {
            process* shortest_process = ready_queue->front;
            process* current = ready_queue->front;

            // Find the process with the shortest remaining burst time
            while (current != NULL) {
                if ((current->CPU_burst_time - current->CPU_bursted_time) < (shortest_process->CPU_burst_time - shortest_process->CPU_bursted_time)) {
                    shortest_process = current;
                }
                current = current->next;
            }

            // 간트 차트에 현재 시간에 실행 중인 프로세스를 기록
            gant_chart[2][cpu_time] = shortest_process->ProcessID;
            shortest_process->CPU_bursted_time += 1;
            cpu_time++;

            // Check if the shortest process needs I/O
            if (shortest_process->CPU_bursted_time == IO_OPERATION_OCCUR) {
                shortest_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                // Remove shortest_process from ready_queue and add to waiting_queue
                if (shortest_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = shortest_process->next;
                }
                enqueue(waiting_queue, shortest_process);
            }
            else if (shortest_process->CPU_bursted_time == shortest_process->CPU_burst_time) {
                // Process has completed its execution
                turnaround_time = cpu_time - shortest_process->Arrival_time;
                waiting_time = turnaround_time - shortest_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                // Remove shortest_process from ready_queue
                if (shortest_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = shortest_process->next;
                }
            }
        }
        else {
            gant_chart[2][cpu_time] = -1; // No process is executing
            cpu_time++;
            cpu_mutex = 0;
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}
void non_preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res) {
    cpu_time = 0;
    int turnaround_time;
    int waiting_time;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    cpu_mutex = 0;

    while (cpu_time < 150) {
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
            process* highest_priority_process = ready_queue->front;
            process* current = ready_queue->front;
            while (current != NULL) {
                if (current->Priority < highest_priority_process->Priority) {
                    highest_priority_process = current;
                }
                current = current->next;
            }

            cpu_mutex = 1;
            for (int i = 0; i < highest_priority_process->CPU_burst_time; i++) {
                gant_chart[3][cpu_time] = highest_priority_process->ProcessID;
                cpu_time++;
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    highest_priority_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    dequeue(ready_queue);
                    enqueue(waiting_queue, highest_priority_process);
                    break;
                }
                else {
                    highest_priority_process->CPU_bursted_time += 1;
                }
            }

            if (highest_priority_process->CPU_bursted_time == highest_priority_process->CPU_burst_time) {
                turnaround_time = cpu_time - highest_priority_process->Arrival_time;
                waiting_time = turnaround_time - highest_priority_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                cpu_mutex = 0;
                dequeue(ready_queue);
            }
        }
        else {
            gant_chart[3][cpu_time] = -1;
            cpu_time++;
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}

void preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res) {
    cpu_time = 0;
    int turnaround_time;
    int waiting_time;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    cpu_mutex = 0;


    while (cpu_time < 150) {
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
            process* highest_priority_process = ready_queue->front;
            process* current = ready_queue->front;
            while (current != NULL) {
                if (current->Priority < highest_priority_process->Priority) {
                    highest_priority_process = current;
                }
                current = current->next;
            }

            cpu_mutex = 1;
            while (highest_priority_process->CPU_bursted_time < highest_priority_process->CPU_burst_time) {
                gant_chart[4][cpu_time] = highest_priority_process->ProcessID;
                cpu_time++;
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    highest_priority_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    dequeue(ready_queue);
                    enqueue(waiting_queue, highest_priority_process);
                    break;
                }
                else {
                    highest_priority_process->CPU_bursted_time += 1;
                }

                process* new_process = ready_queue->front;
                while (new_process != NULL) {
                    if (new_process->Priority < highest_priority_process->Priority) {
                        highest_priority_process = new_process;
                        break;
                    }
                    new_process = new_process->next;
                }
            }

            if (highest_priority_process->CPU_bursted_time == highest_priority_process->CPU_burst_time) {
                turnaround_time = cpu_time - highest_priority_process->Arrival_time;
                waiting_time = turnaround_time - highest_priority_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                cpu_mutex = 0;
                dequeue(ready_queue);
            }
        }
        else {
            gant_chart[4][cpu_time] = -1;
            cpu_time++;
        }
    }


    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;


}

void round_robin(Queue* ready_queue, Queue* waiting_queue, result* res) {

    int time_quantum = 4;
    cpu_time = 0;
    int turnaround_time;
    int waiting_time;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;

    while (cpu_time < 150) {

        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if (ready_queue->front != NULL) {
            process* execute_process = dequeue(ready_queue);
            int executed_time = 0;


            while (executed_time < time_quantum && execute_process->CPU_bursted_time < execute_process->CPU_burst_time) {
                gant_chart[5][cpu_time] = execute_process->ProcessID;
                cpu_time++;
                executed_time++;
                execute_process->CPU_bursted_time++;


                if (execute_process->CPU_bursted_time == IO_OPERATION_OCCUR) {
                    execute_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    enqueue(waiting_queue, execute_process);
                    break;
                }


                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
            }


            if (execute_process->CPU_bursted_time == execute_process->CPU_burst_time) {
                turnaround_time = cpu_time - execute_process->Arrival_time;
                waiting_time = turnaround_time - execute_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                free(execute_process);
            }
            else if (executed_time == time_quantum) {

                enqueue(ready_queue, execute_process);
            }
        }
        else {
            gant_chart[5][cpu_time] = -1; // No process is executing
            cpu_time++;
        }
    }


    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;

}

void evaluation(result fcfs_result, result non_preemptive_sjf_result, result preemptive_sjf_result, result non_preemptive_priority_result, result preemptive_priority_result, result round_robin_result) {
    schedule_result[0] = fcfs_result;
    schedule_result[1] = non_preemptive_sjf_result;
    schedule_result[2] = preemptive_sjf_result;
    schedule_result[3] = non_preemptive_priority_result;
    schedule_result[4] = preemptive_priority_result;
    schedule_result[5] = round_robin_result;
}

// 큐 갯수 세는 함수
int queue_count(Queue* q) {
    return PROCESS_COUNT;
}

// 큐에서 노드를 스왑한다
void swap_node(Queue* q, process* a, process* b) {
    if (a == b) return;
    process* prev_a = NULL;
    process* prev_b = NULL;
    process* current = q->front;

    while (current != NULL && (prev_a == NULL || prev_b == NULL)) {
        if (current->next == a) prev_a = current;
        if (current->next == b) prev_b = current;
        current = current->next;
    }

    if (q->front == a) q->front = b;
    else if (q->front == b) q->front = a;

    if (q->rear == a) q->rear = b;
    else if (q->rear == b) q->rear = a;

    process* temp = a->next;
    a->next = b->next;
    b->next = temp;

    if (prev_a != NULL) prev_a->next = b;
    if (prev_b != NULL) prev_b->next = a;
}

int is_IO_occur(int time) {
    if (time == IO_OPERATION_OCCUR) { // I/O 고정적 발생
        return 1;
    }
    return 0;
}

void update_waiting_queue(Queue* waiting_queue, Queue* ready_queue, int current_time) {
    process* current = waiting_queue->front;
    process* prev = NULL;

    while (current != NULL) {
        // 현재 시각이 프로세스의 IO 버스트 시간을 초과하면 준비 큐로 이동
        if (current->I_O_burst_time <= current_time) {
            if (prev == NULL) {
                dequeue(waiting_queue);
                enqueue(ready_queue, current);
                current = waiting_queue->front;
            }
            else {
                prev->next = current->next;
                enqueue(ready_queue, current);
                current = prev->next;
            }
        }
        else {
            prev = current;
            current = current->next;
        }
    }
}

// 가상 프로세스 생성기, 시간에 따라 정해진 프로세스를 큐에 넣는다
void enqueue_by_arrive_time(Queue* ready_queue, process* process_list[PROCESS_COUNT], int cpu_time) {
    for (int i = 0; i < PROCESS_COUNT; i++) {

        if (process_list[i]->Arrival_time == cpu_time) {
            enqueue(ready_queue, process_list[i]);
        }
    }
}

void init() {
    clear_screen();

    printf("-----------------------------------------------------------------------------------------------------------------------\n");
    printf("|  CPU Scheduling Simulator                                                                                           |\n");
    printf("|                                                                                                                     |\n");
    printf("|  * %d번을 입력하면 스케쥴링이 진행됩니다.                                                                            |\n",1);
    printf("|                                                                                                                     |\n");
    printf("-----------------------------------------------------------------------------------------------------------------------\n");


    int n;
    do{
        scanf("%d", &n);
    } while (n != 1);

}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#elif __linux__
    system("clear");

#endif
}

int result_view() {
    int n;

    while (1) {
        clear_screen();
        printf("-----------------------------------------------------------------------------------------------------------------------\n");
        printf("| 스케쥴링 완료.                                                                                                       |\n");
        printf("| 다시 시작하려면 %d를 입력                                                                                             |\n", 0);
        printf("| 스케줄러별 Gantt Chart를 확인하려면, 해당 스케줄러의 번호를 입력                                                     |\n");
        printf("| 프로그램을 종료하려면 %d 입력                                                                                        |\n", -1);
        printf("-----------------------------------------------------------------------------------------------------------------------\n");

        printf("\n");


        for (int i = 0; i < 6; i++) {
            switch (i)
            {
                case 0:
                    printf("%d : fcfs_schedule ", 1);
                    break;
                case 1:
                    printf("%d : non_preemptive_sjf ", 2);
                    break;
                case 2:
                    printf("%d : preemptive_sjf ", 3);
                    break;
                case 3:
                    printf("%d : non_preemptive_priorit ", 4);
                    break;
                case 4:
                    printf("%d : preemptive_priority ", 5);
                    break;
                case 5:
                    printf("%d : round_robin ", 6);
                    break;
                default:
                    break;
            }


            printf("(avr_turnaround_time : %d, avr_wating_time : %d)\n", schedule_result[i].avr_turnaround_time, schedule_result[i].avr_waiting_time);
        }

        printf("\n");


        scanf("%d", &n);
        if (n == 0 || n == -1) {
            break;
        }
        else if (1 <= n && n <= 6) {
            print_gantt_chart(n-1, 150);
        }
    }

    if (n == -1) {
        return -1;
    }
    return 0;


}

/*
void made_virtual_queue_state() {


    for (int i = 0; i < PROCESS_COUNT; i++) {
        process* temp = process_list[i];
;
        enqueue(ready_queue, temp);
    }

}
*/

void reset_queue(Queue** ready_queue, Queue** waiting_queue) {
    *ready_queue = create_queue();
    *waiting_queue = create_queue();


}

void create_dup_procesess(process* process_list[PROCESS_COUNT], process* dup_process_list[PROCESS_COUNT]) {
    process* temp;
    process* original;

    for (int i = 0; i < PROCESS_COUNT; i++) {
        original = process_list[i];

        // 프로세스 생성 후 리스트에 집어 넣는다
        temp = create_process(original->CPU_burst_time, original->I_O_burst_time, original->Arrival_time, original->Priority);

        dup_process_list[i] = temp;
    }

}

void reset_process(process* process_list[PROCESS_COUNT]) {
    process* original;


    for (int i = 0; i < PROCESS_COUNT; i++) {
        original = process_list[i];
        original->CPU_bursted_time = 0;
        original->next = NULL;

    }

}

void print_gantt_chart(int algorithm_index, int time_units) {
    clear_screen();
    const char* algorithm_names[] = {
            "FCFS", "Non-Preemptive SJF", "Preemptive SJF",
            "Non-Preemptive Priority", "Preemptive Priority", "Round Robin"
    };
    printf("Gantt Chart for %s:\n", algorithm_names[algorithm_index]);

    // 상단 라인을 출력합니다.
    printf("Time:  ");
    for (int t = 0; t < time_units; t++) {
        printf("%4d", t);
    }
    printf("\n");

    // 간트 차트의 프로세스 ID를 출력합니다.
    printf("PIDs:  ");
    for (int t = 0; t < time_units; t++) {
        int pid = gant_chart[algorithm_index][t];
        if (pid == -1) {
            printf("   -");
        }
        else {
            printf("%4d", pid);
        }
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("되돌아가려면 %d 입력", 1);

    int num;
    do {
        scanf("%d", &num);


    } while (num!=1);


}