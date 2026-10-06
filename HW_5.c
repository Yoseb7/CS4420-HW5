#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 100

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int end_time;
    int waiting_time;
    int first_start_time;
    int is_completed;
} Task;

//function declarations
void simulate_FCFS(Task tasks[], int count);
void simulate_SJF(Task tasks[], int count);
void simulate_RR(Task tasks[], int count, int quantum);
int read_tasks(const char *filename, Task tasks[]);

int main(int argc, char *argv[]) {
    const char *filename = argv[1];
    const char *algorithm = argv[2];

    //initialize time quantum if selected algorithm is round robin
    int time_quantum = 0;
    if (strcmp(algorithm, "RR") == 0) {
        if (argc < 4) {
            printf("Error: Round Robin requires a time quantum parameter.\n");
            return 1;
        }
        time_quantum = atoi(argv[3]);
    }

    //read tasks from file and run the corresponding simulation
    Task tasks[MAX_TASKS];
    int count = read_tasks(filename, tasks);
    if (strcmp(algorithm, "FCFS") == 0) {
        simulate_FCFS(tasks, count);
    } else if (strcmp(algorithm, "SJF") == 0) {
        simulate_SJF(tasks, count);
    } else if (strcmp(algorithm, "RR") == 0) {
        simulate_RR(tasks, count, time_quantum);
    } else {
        printf("Error: Not one of the 3 algorithms\n");
        return 1;
    }
}

//function to read number of tasks from a file and initialize variables in list of tasks
int read_tasks(const char *filename, Task tasks[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file\n");
        return 1;
    }
    int count = 0;
    //read the number of tasks from the file
    fscanf(file, "%d", &count);

    for (int i = 0; i < count; i++) {
        //read and assign PID, arrival time, and burst time for each task
        (fscanf(file, "%d %d %d", &tasks[i].pid, &tasks[i].arrival_time, &tasks[i].burst_time) != 3);
    }

    //initialize task list variables
    for (int i = 0; i < count; i++) {
        tasks[i].remaining_time = tasks[i].burst_time;
        tasks[i].start_time = -1;
        tasks[i].end_time = -1;
        tasks[i].waiting_time = 0;
        tasks[i].first_start_time = -1;
        tasks[i].is_completed = 0;
    }

    fclose(file);
    return count;
}

//function to simulate first come first serve
void simulate_FCFS(Task tasks[], int count) {

    //sort tasks by arrival time and PID
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            // check if current task's arrival time is greater than the next task's arrival time, or if arrival times are equal then check PID order to break the tie
            if (tasks[j].arrival_time > tasks[j + 1].arrival_time || (tasks[j].arrival_time == tasks[j + 1].arrival_time && tasks[j].pid > tasks[j + 1].pid)) {
                //swap tasks
                Task temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    int current_time = 0;
    float total_waiting_time = 0;

    printf("FCFS:\n");
    printf("----------------------------------------------------------------------------\n");
    printf("| PID | Arrival Time | Start Time | End Time | Running Time | Waiting Time |\n");
    printf("----------------------------------------------------------------------------\n");

    //fcfs simulation loop, iterate through each task in order
    for (int i = 0; i < count; i++) {
        //skip to arrival of the current task
        if (current_time < tasks[i].arrival_time) {
            current_time = tasks[i].arrival_time;
        }
        tasks[i].start_time = current_time;

        //calculate end time, waiting time, and update total waiting time
        tasks[i].end_time = current_time + tasks[i].burst_time;
        tasks[i].waiting_time = tasks[i].start_time - tasks[i].arrival_time;
        total_waiting_time += tasks[i].waiting_time;

        //update current time to the end time of the current task (when CPU is free again)
        current_time = tasks[i].end_time;

        //print the current task's details
        printf("| %3d | %12d | %10d | %8d | %12d | %12d |\n",
               tasks[i].pid, tasks[i].arrival_time, tasks[i].start_time,
               tasks[i].end_time, tasks[i].burst_time, tasks[i].waiting_time);
    }
    printf("----------------------------------------------------------------------------\n");
    printf("\nAverage Waiting Time: %.2f\n", total_waiting_time / count);
}

//function to simulate shortest job first
void simulate_SJF(Task tasks[], int count) {
    int current_time = 0;
    int completed = 0;
    float total_waiting_time = 0;

    //track execution order for printing output table
    Task executed_tasks[MAX_TASKS];
    int exec_count = 0;

    while (completed < count) {
        //initialize index of chosen task to none and min burst to a large value
        int idx = -1;
        int min_burst = 100000;

        //find the arrived task with the shortest burst time
        for (int i = 0; i < count; i++) {
            //check if the task has arrived and is not completed
            if (tasks[i].arrival_time <= current_time && !tasks[i].is_completed) {
                //check if this task has a shorter burst time than the current minimum and update the chosen task index and min burst if so
                if (tasks[i].burst_time < min_burst) {
                    min_burst = tasks[i].burst_time;
                    idx = i;
                } else if (tasks[i].burst_time == min_burst) {
                    //for tie breaker, use earlier arrival time, then smaller PID
                    if (tasks[i].arrival_time < tasks[idx].arrival_time || (tasks[i].arrival_time == tasks[idx].arrival_time && tasks[i].pid < tasks[idx].pid)) {
                        idx = i;
                    }
                }
            }
        }

        if (idx != -1) {
            tasks[idx].start_time = current_time;

            //calculate end time, waiting time, and mark the task as completed
            tasks[idx].end_time = current_time + tasks[idx].burst_time;
            tasks[idx].waiting_time = tasks[idx].start_time - tasks[idx].arrival_time;
            tasks[idx].is_completed = 1;

            //update total waiting time and current time
            total_waiting_time += tasks[idx].waiting_time;
            current_time = tasks[idx].end_time;

            //save the completed task for printing later, increment count and total number of completed tasks
            executed_tasks[exec_count++] = tasks[idx];
            completed++;
        } else {
            //increment current time if no task is available
            current_time++;
        }
    }

    //print the summary table for SJF
    printf("SJF:\n");
    printf("----------------------------------------------------------------------------\n");
    printf("| PID | Arrival Time | Start Time | End Time | Running Time | Waiting Time |\n");
    printf("----------------------------------------------------------------------------\n");

    for (int i = 0; i < exec_count; i++) {
        printf("| %3d | %12d | %10d | %8d | %12d | %12d |\n",
               executed_tasks[i].pid, executed_tasks[i].arrival_time, executed_tasks[i].start_time,
               executed_tasks[i].end_time, executed_tasks[i].burst_time, executed_tasks[i].waiting_time);
    }
    printf("----------------------------------------------------------------------------\n");
    printf("\nAverage Waiting Time: %.2f\n", total_waiting_time / count);
}

//function to simulate round robin 
void simulate_RR(Task tasks[], int count, int quantum) {
    //initialize ready queue
    int queue[MAX_TASKS * 10];
    int front = 0;
    int rear = 0;
    int visited[MAX_TASKS] = {0};

    int current_time = 0;
    int completed = 0;

    //track execution segments for top RR table
    typedef struct {
        int pid;
        int start_time;
        int end_time;
        int running_time;
    } ExecutionSegment;

    ExecutionSegment segments[MAX_TASKS * 20];
    int seg_count = 0;

    //sort initially by arrival time
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            //check if current task arrives later than the next task
            if (tasks[j].arrival_time > tasks[j + 1].arrival_time) {
                //swap tasks
                Task temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    //add tasks arriving at time 0
    for (int i = 0; i < count; i++) {
        if (tasks[i].arrival_time == 0) {
            queue[rear++] = i;
            visited[i] = 1;
        }
    }
    while (completed < count) {
        //if ready queue is empty then advance time to the next task arrival
        if (front == rear) {
            int next_arrival = 100000;
            for (int i = 0; i < count; i++) {
                //check that the task is unfinished and arrives after the current time
                if (!tasks[i].is_completed && tasks[i].arrival_time > current_time) {
                    //check whether this is the earliest upcoming arrival and store it as next arrival
                    if (tasks[i].arrival_time < next_arrival) {
                        next_arrival = tasks[i].arrival_time;
                    }
                }
            }
            //advance the CPU time to the next task's arrival time
            current_time = next_arrival;

            for (int i = 0; i < count; i++) {
                //add newly arrived, unvisited, unfinished tasks to the queue
                if (tasks[i].arrival_time <= current_time && !visited[i] && !tasks[i].is_completed) {
                    queue[rear++] = i;
                    //mark as visited so it is not added again
                    visited[i] = 1;
                }
            }
        }

        int idx = queue[front++];

        //determine how long the task will run for
        int run_time = (tasks[idx].remaining_time < quantum) ? tasks[idx].remaining_time : quantum;

        //store the PID in the execution segment, starting time, ending time, and running time
        segments[seg_count].pid = tasks[idx].pid;
        segments[seg_count].start_time = current_time;
        segments[seg_count].end_time = current_time + run_time;
        segments[seg_count].running_time = run_time;

        //move to the next execution segment
        seg_count++;

        //advance the current CPU time by the amount of time the task ran
        current_time += run_time;

        //subtract the time used from the task's remaining CPU burst time
        tasks[idx].remaining_time -= run_time;

        //check for tasks that arrived while the current task was executing
        for (int i = 0; i < count; i++) {
            //check whether the task has arrived, has not been visited, and is unfinished
            if (tasks[i].arrival_time <= current_time && !visited[i] && !tasks[i].is_completed) {
                //add the newly arrived task to the ready queue
                queue[rear++] = i;
                //mark the task as visited
                visited[i] = 1;
            }
        }

        //check whether the current task still has CPU time remaining
        if (tasks[idx].remaining_time > 0) {
            //place the current task at the back of the queue so it can run again later
            queue[rear++] = idx;
        //if task has finished
        } else {
            //mark the task as completed, store end time, and calculate waiting time
            tasks[idx].is_completed = 1;
            tasks[idx].end_time = current_time;
            tasks[idx].waiting_time = tasks[idx].end_time - tasks[idx].arrival_time - tasks[idx].burst_time;

            //increase the completed task counter
            completed++;
        }
    }

    //print execution segments
    printf("RR (Time quantum = %d):\n\n", quantum);
    printf("----------------------------------------------\n");
    printf("| PID | Start Time | End Time | Running Time |\n");
    printf("----------------------------------------------\n");
    for (int i = 0; i < seg_count; i++) {
        printf("| %3d | %10d | %8d | %12d |\n",
               segments[i].pid, segments[i].start_time,
               segments[i].end_time, segments[i].running_time);
    }
    printf("----------------------------------------------\n\n");

    //print summary table
    float total_waiting_time = 0;
    printf("---------------------------------------------------------------\n");
    printf("| PID | Arrival Time | Running Time | End Time | Waiting Time |\n");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("| %3d | %12d | %12d | %8d | %12d |\n",
               tasks[i].pid, tasks[i].arrival_time, tasks[i].burst_time,
               tasks[i].end_time, tasks[i].waiting_time);
        total_waiting_time += tasks[i].waiting_time;
    }
    printf("---------------------------------------------------------------\n");
    printf("\nAverage Waiting Time: %.2f\n", total_waiting_time / count);
}
