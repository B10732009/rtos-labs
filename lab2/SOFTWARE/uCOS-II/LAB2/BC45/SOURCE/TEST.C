#include "includes.h"

// CONSTANTS
#define TASK_STK_SIZE 512 // Size of each task's stacks (# of WORDs)
#define N_TASKS 3 // Number of identical tasks

// VARIABLES
OS_STK TaskStk[N_TASKS][TASK_STK_SIZE]; // Tasks stacks
OS_STK TaskStartStk[TASK_STK_SIZE];
char   TaskData[N_TASKS]; // Parameters to pass to each task
OS_EVENT *RandomSem;

// FUNCTION PROTOTYPES
void BaseTask(int _taskId, int _computeTime, int _period, int _isPrint);
void Task1(void *pdata); // Function prototypes of Startup task
void Task2();  
void Task3();
void PrintMsgList();
void InitMsgList();

// MAIN
void main(void) {
  // Initialize uC/OS-II
  OSInit();
  // Save environment to return to DOS
  PC_DOSSaveReturn();
  // Install uC/OS-II's context switch vector
  PC_VectSet(uCOS, OSCtxSw);
  // Random number semaphore
  RandomSem = OSSemCreate(1);
  // Create tasks
  OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE-1], 1);
  OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE-1], 2);
  //OSTaskCreate(Task3, (void *)0, &TaskStk[2][TASK_STK_SIZE-1], 3);
  // Initialize message list
  InitMsgList();
  // Start multitasking
  OSStart();
}

void BaseTask(int _taskId, int _computeTime, int _period, int _isPrint) {
  INT16S key;
  int start, end, toDelay, deadline;
  OSTCBCur->computeTime = _computeTime;
  OSTCBCur->period = _period;
  OSTCBCur->deadline = _period;
  start = 0 ;
  while (1) {
    // See if key has been pressed
    if (PC_GetKey(&key) == TRUE) {                     
      // Yes, see if it's the ESCAPE key
      if (key == 0x1B) {
        // Return to DOS
        PC_DOSReturn();
      }
    }

    // 等待task執行結束
    while (OSTCBCur->computeTime > 0)
      ; // Busywaiting
    // 取得結束時間
    end = OSTimeGet();
    // 計算完成時間與期望時間的差 -> 期望花的時間:period, 實際花的時間:end-start 
    toDelay = OSTCBCur->period - (end - start);
    // 計算下一輪開始時間
    start += OSTCBCur->period;
    // 重製執行時間
    OSTCBCur->computeTime = _computeTime;
    // 將deadline增加至下一周期
    OSTCBCur->deadline = OSTCBCur->deadline + _period ;
    // 檢查此task是否超時
    if (toDelay < 0) { // 超時
      OS_ENTER_CRITICAL();
      printf("%d\tTask%d Deadline!\n", OSTCBCur->deadline, _taskId);
      OS_EXIT_CRITICAL();
    }
    else { // 未超時
      OSTimeDly(toDelay);
      if (_isPrint) {
        OS_ENTER_CRITICAL();
        PrintMsgList();
        OS_EXIT_CRITICAL();
      }

      //OSTimeDly(toDelay);
    }
  }
}

// Task1 (STARTUP TASK)
void Task1(void *pdata) {
  // Allocate storage for CPU status register
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif
  // Prevent compiler warning
  pdata = pdata;

  OS_ENTER_CRITICAL();
  // Install uC/OS-II's clock tick ISR
  PC_VectSet(0x08, OSTickISR);
  // Reprogram tick rate
  PC_SetTickRate(OS_TICKS_PER_SEC);
  OS_EXIT_CRITICAL();
  
  // Initialize uC/OS-II's statistics
  // OSStatInit函數似乎會造成tick計算異常，因此選擇註解掉
  // OSStatInit();
  
  // 在做完start up task所需額外做的事之後，將tick歸零  
  OSTimeSet(0);
  BaseTask(1, 1, 3, 1);
}

// Task2
void Task2() {
  BaseTask(2, 3, 5, 0);
}

// Task3
void Task3() {
  BaseTask(3, 2, 10, 0);
}

void PrintMsgList() {
  while (msgList->next) {
    // 印出訊息佇列節點訊息
    printf("%d\t%s\t%d\t%d\n", 
      msgList->next->tick,
      (msgList->next->event ? "Complete" : "Preemt  "),
      msgList->next->fromTaskId,
      msgList->next->toTaskId
    );
    // 將印過的節點刪掉
    msgTemp = msgList;
    msgList = msgList->next;
    free(msgTemp);
  }
}

void InitMsgList() {
  // 新增dummy節點(簡化串列操作)
  msgList = (msg*)malloc(sizeof(msg));
  msgList->next = (msg*)0;
}
