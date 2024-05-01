#define SERVER "fifo_server"
#define CLIENT "fifo_client"

typedef struct OngoingTask{
    int time;
    int pid;
    char prog[20];
    char args[300];
    int argsSize;
    char taskID[20];
} OngoingTask;

typedef struct FinishedTask{
    int pid;
    char taskID[20];
    time_t exec_time;
} FinishedTask;