#define SERVER "fifo_server"
#define CLIENT "fifo_client"

typedef struct OngoingTask{
    int type; // 0 -> exec, 1 -> status, 2 -> stop
    int time;
    int pid;
    char prog[20];
    char args[300];
    int argsSize;
    char taskID[20];
    time_t start_time;
    time_t finish_time;
    struct OngoingTask *next;
} OngoingTask;

typedef struct FinishedTask{
    int pid;
    char taskID[20];
    char prog[20];
    time_t exec_time;
} FinishedTask;

typedef struct TaskQueue{
    OngoingTask *front;
    OngoingTask *rear;
} TaskQueue;
