#include <stdio.h>
#include <stdlib.h>

typedef struct process {
    int PID;
    int cpu_burst;
    int waiting_time;
}Process;

int FCFS(Process * ps);
int SJF(Process * ps);
int RR(Process * ps);

void enter_process(Process * ps);	// 프로세스 입력
void create_readyqueue(Process * ps, Process * ps2);	// 각 스케쥴링에 쓸 레디 큐 생성

void print_process(Process * ps);	// 각 프로세스 내용 출력
void print_gantt_waiting(Process * ps);	// 스케쥴링 결과 gantt chart와 waiting time 출력

void sort_process_arrival(Process * ps);	// 도착순서(PID)대로 정렬
void sort_process_cpuburst(Process * ps);	// CPU burst 순 대로 정렬

int main() {
    Process ps[5];
    Process ready_queue[5];
    int scheduling_num;

    enter_process(ps);

    while (1) {
        printf("\nCPU Scheduling 알고리즘을 선택해주세요.\n");
        printf("1. FCFS\n");
        printf("2. SJF\n");
        printf("3. RR\n");
        printf("4. Quit\n");
        printf("번호 입력: ");
        scanf("%d", &scheduling_num);

        create_readyqueue(ps, ready_queue);

        // FCFS
        if (scheduling_num == 1) {
            FCFS(ready_queue);
            printf("\n-------------------------------------- FCFS ----------------------------------------");
            print_gantt_waiting(ready_queue);
        }
            // SJF
        else if (scheduling_num == 2) {
            SJF(ready_queue);
            printf("\n-------------------------------------- SJF -----------------------------------------");
            print_gantt_waiting(ready_queue);
        }

            // RR
        else if (scheduling_num == 3) {
            printf("\n--------------------------------------- RR -----------------------------------------");
            RR(ready_queue);
        }
        else {
            break;
        }
    }

    return 0;
}

void enter_process(Process * ps) {
    int i;

    printf("Enter CPU burst...\n");

    for (i = 0; i < 5; i++)
    {
        printf("P%d: ", i+1);
        ps[i].PID = i + 1;
        scanf("%d", &ps[i].cpu_burst);
    }
}

void print_process(Process * ps) {
    int i;
    printf("--------------\n");
    printf("PID |CPU burst\n");

    for (i = 0; i < 5; i++)
        printf("P%d   |%2d\n", ps[i].PID, ps[i].cpu_burst);

    printf("---------------\n");
}

int FCFS(Process * ps) {
    sort_process_arrival(ps);
    int i, j = 0;

    ps[0].waiting_time = 0;

    //calculate waiting time
    for (i = 1; i < 5; i++)
    {
        ps[i].waiting_time = 0;
        for (j = 0; j < i; j++)
            ps[i].waiting_time += ps[j].cpu_burst;
    }
    return 0;
}

int SJF(Process * ps) {
    sort_process_cpuburst(ps);
    int i, j;

    ps[0].waiting_time = 0;

    //calculate waiting time
    for (i = 1; i < 5; i++)
    {
        ps[i].waiting_time = 0;
        for (j = 0; j < i; j++)
            ps[i].waiting_time += ps[j].cpu_burst;
    }

    return 0;
}

int RR(Process * ps) {
    int time_quantum, i, index = 0;
    int order_queue[500];				// RR에서 다음으로 할당 될 프로세스의 PID를 담은 큐
    int order[5] = { 1,2,3,4,5 };		// 큐에 저장할 PID
    int head = 0, tail = 0;				// 큐의 가장 처음(head)과 맨 끝(tail)
    int process_remain_time[6];
    int total_remain_time = 0, total_run_time = 0, avg_waiting_time = 0;
    int arrival_time[5];
    int tmp_cpu_burst = 0;

    sort_process_arrival(ps);

    // 남은 시간을 확인하기 위해 process_remain_time을 입력받은 cpu_burst로 초기화
    process_remain_time[0] = 0;
    for (i = 0; i < 5; i++) {
        process_remain_time[i+1] = ps[i].cpu_burst;
        total_remain_time += ps[i].cpu_burst;
    }

    for (i = 0; i < 5; i++)
    {
        ps[i].waiting_time = 0;
    }

    // QUEUE 초기화
    for (i = 0; i < 5; i++) {
        order_queue[tail++] = order[i];
    }

    printf("\nEnter Time Quantum: ");
    scanf("%d", &time_quantum);

    // 도착시간
    arrival_time[0] = 0;
    for (i = 1; i < 5; i++) {
        if (ps[i-1].cpu_burst <= time_quantum) {
            arrival_time[i] = arrival_time[i - 1] + ps[i-1].cpu_burst;
        }
        else {
            arrival_time[i] = arrival_time[i - 1] + time_quantum;
        }
    }

    //printf("\n%d\n", total_remain_time);
    printf("\nGantt chart\n");
    while (1) {
        index = order_queue[head];
        //printf("index: %d ", index);
        //printf("process remain time: %d\n", process_remain_time[index]);
        //printf("%d", total_remain_time);

        if (index <= 0) {
            break;
        }

        if (process_remain_time[index] <= time_quantum && process_remain_time[index] > 0 && index) {

            //cpu burst만큼 해당 프로세스 출력

            printf("| ");
            for (i = 0; i < process_remain_time[index]; i++) {
                printf("P%d ", ps[index-1].PID);
                total_run_time += 1;
            }

            //프로세스남은시간 0으로 초기화
            process_remain_time[index] = 0;

            //큐 헤드 += 1
            head += 1;

            total_remain_time -= process_remain_time[index];
        }
        else {
            //타임 퀀텀만큼 해당 프로세스 출력
            printf("| ");
            for (i = 0; i < time_quantum; i++) {
                printf("P%d ", ps[index-1].PID);
                total_run_time += 1;
            }

            //프로세스 남은시간 -= time quantum
            process_remain_time[index] -= time_quantum;

            //큐 헤드 += 1, 큐 테일에 해당 프로세스 번호 삽입
            order_queue[tail] = index;
            head += 1;
            tail += 1;

            total_remain_time -= time_quantum;

        }

        if (total_remain_time <= 0) {
            break;
        }
    }

    printf("\n");

    return 0;
}

void sort_process_arrival(Process * ps) {
    Process tmp_process;
    int i, j;

    for (i = 4; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (ps[j].PID > ps[j + 1].PID) {
                tmp_process = ps[j + 1];
                ps[j + 1] = ps[j];
                ps[j] = tmp_process;
            }
        }
    }
}

void sort_process_cpuburst(Process * ps) {
    Process tmp_process;
    int i, j;

    for (i = 4; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (ps[j].cpu_burst > ps[j + 1].cpu_burst) {
                tmp_process = ps[j + 1];
                ps[j + 1] = ps[j];
                ps[j] = tmp_process;
            }
        }
    }
}

void print_gantt_waiting(Process * ps) {
    printf("\n\nGantt chart\n");
    int total_time = 0;
    int i, avg_waiting_time = 0;
    int a[500];
    int idx = 0;

    for (i = 0; i < 5; i++) {
        total_time = total_time + ps[i].cpu_burst;
    }

    printf("\n| ");

    for (i = 0; i < total_time; i++) {
        a[i] = ps[idx].PID;
        printf("P%d ", a[i]);
        ps[idx].cpu_burst -= 1;
        if (ps[idx].cpu_burst == 0) {
            idx += 1;
            printf("| ");
        }
    }

    sort_process_arrival(ps);
    printf("\n");

    printf("\n");
    printf("waiting time: ");

    for (i = 0; i < 5; i++) {
        printf(" P%d  %2d(ms)  ", i+1, ps[i].waiting_time);
        avg_waiting_time += ps[i].waiting_time;
    }

    printf("\n");
    printf("average waiting time:  %d(ms)\n\n", avg_waiting_time / 5);
}

void create_readyqueue(Process * ps, Process * ps2) {

    int i;

    for (i = 0; i < 5; i++)
    {
        ps2[i].PID = ps[i].PID;
        ps2[i].cpu_burst = ps[i].cpu_burst;
        ps2[i].waiting_time = ps[i].waiting_time;
    }
}