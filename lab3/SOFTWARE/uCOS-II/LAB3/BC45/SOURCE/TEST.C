#include "includes.h"

// CONSTANTS
#define TASK_STK_SIZE 512 // Size of each task's stacks (# of WORDs)
#define N_TASKS 3 // Number of identical tasks

// VARIABLES
OS_STK TaskStk[N_TASKS][TASK_STK_SIZE]; // Tasks stacks
OS_STK TaskStartStk[TASK_STK_SIZE];
char   TaskData[N_TASKS]; // Parameters to pass to each task
OS_EVENT *RandomSem;
OS_EVENT *R1; // pointer to R1 ECB 
OS_EVENT *R2; // pointer to R2 ECB
INT8U R1_error; // R1 error message 
INT8U R2_error; // R2 error message
int _useR1 = 0;
int _useR2 = 0;

// FUNCTION PROTOTYPES
void BaseTask(int _taskId, int _computeTime, int _period, int _isPrint);
void Task1(void *pdata); // Function prototypes of Startup task
void Task2();  
void Task3();
void PrintMsgList();
void InitMsgList();
void AddMsgList();

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
  OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE-1], 3);
  // OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE-1], 2);
  // OSTaskCreate(Task3, (void *)0, &TaskStk[2][TASK_STK_SIZE-1], 3);
  // Create mutexs
  R1 = OSMutexCreate (1, &R1_error);
  R2 = OSMutexCreate (2, &R2_error);
  // Initialize message list
  InitMsgList();
  // Start multitasking
  OSStart();
}

// void BaseTask(int _taskId, int _computeTime, int _period, int _isPrint) {
//   INT16S key;
//   INT8U *error;
//   int start, end, toDelay, deadline;
//   OSTCBCur->computeTime = _computeTime;
//   OSTCBCur->period = _period;
//   deadline = _period;
    
//   while (1) {
//     // See if key has been pressed
//     if (PC_GetKey(&key) == TRUE) {                     
//       // Yes, see if it's the ESCAPE key
//       if (key == 0x1B) {
//         // Return to DOS
//         PC_DOSReturn();
//       }
//     }

//     // 取得開始時間
//     start = OSTimeGet();
//     // 等待task執行結束
//     while (OSTCBCur->computeTime > 0) {
//       if ( OSTCBCur->computeTime == 4 && _useR1 == 0 ) { 
//         OSMutexPend(R1, 5, error) ;
//         _useR1 = 1 ;
//       } // if   
//       // if ( OSTCBCur->computeTime == 2 && _useR2 == 0 ) {
//       //   OSMutexPend(R2, 5, error) ;
//       //   _useR2 = 1 ;
//       // } // if   
//     } // while    
//     // Release mutex
//     OSMutexPost(R1) ;
//     _useR1 = 0 ;
//     // OSMutexPost(R2) ;
//     _useR2 = 0 ;
//     // 取得結束時間
//     end = OSTimeGet();
//     // 計算完成時間與期望時間的差 -> 期望花的時間:period, 實際花的時間:end-start 
//     toDelay = OSTCBCur->period - (end - start);
//     // 計算下一輪開始時間
//     start += OSTCBCur->period;
//     // 重製執行時間
//     OSTCBCur->computeTime = _computeTime;
//     // 檢查此task是否超時
//     if (toDelay < 0) { // 超時
//       OS_ENTER_CRITICAL();
//       printf("%d\tTask%d Deadline!\n", deadline, _taskId);
//       OS_EXIT_CRITICAL();
//     }
//     else { // 未超時
//       if (_isPrint) {
//         OS_ENTER_CRITICAL();
//         PrintMsgList();
//         OS_EXIT_CRITICAL();
//       }
//       OSTimeDly(toDelay);
//     }
//     // 將deadline增加至下一周期
//     deadline += _period;
//   }
// }

// Task1 (STARTUP TASK)
void Task1(void *pdata) {
  INT16S key;
  INT8U error;
  int prio = 3;
  int arrive = 8;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 6;
  int period = 30;
  int deadline = arrive + period;
  int useR1 = 0; // R1是否正在使用
  int useR2 = 0; // R2是否正在使用
  
  // Some code for startup task
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif
  pdata = pdata;
  OS_ENTER_CRITICAL();
  PC_VectSet(0x08, OSTickISR);
  PC_SetTickRate(OS_TICKS_PER_SEC);
  OS_EXIT_CRITICAL();
  // OSStatInit函數似乎會造成tick計算異常，因此選擇註解掉
  // OSStatInit();
  
  // 在做完start up task所需額外做的事之後，將tick歸零  
  OSTimeSet(0);
  
  // if ( !R1 )
  //   printf("r1 invalid\n") ;
  // else 
  //   printf("r1 valid\n") ;
  // if ( !R2 ) {
  //   printf("r2 invalid\n") ;
  //   printf("%d\n", *R2_error) ;
  // } // if 
  // else
  //   printf("r2 valid\n") ;
  // BaseTask(1, 6, 10, 1);

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;

  OSTimeDly(8);
    
  while (1) {
    // See if key has been pressed
    if (PC_GetKey(&key) == TRUE) {                     
      // Yes, see if it's the ESCAPE key
      if (key == 0x1B) {
        // Return to DOS
        PC_DOSReturn();
      }
    }

    OS_ENTER_CRITICAL();
    AddMsgList(OSTimeGet(), 3, 100, 100);
    OS_EXIT_CRITICAL();

    // 取得開始時間
    // start = OSTimeGet();
    // 等待task執行結束
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 4 && !useR1) { 
        OSMutexPend(R1, 5, &error);
        useR1 = 1;
      }
      else if (OSTCBCur->computeTime == 2 && !useR2) {
        // OSMutexPend(R2, 5, &error);
        // useR2 = 1 ;
      } 
    }  
    // Release mutex
    OSMutexPost(R2);
    useR2 = 0;
    OSMutexPost(R1);
    useR1 = 0;
    
    // 取得結束時間
    end = OSTimeGet();
    // 計算完成時間與期望時間的差 -> 期望花的時間:period, 實際花的時間:end-start 
    toDelay = OSTCBCur->period - (end - start);
    // 計算下一輪開始時間
    start += OSTCBCur->period;
    // 重製執行時間
    OSTCBCur->computeTime = computeTime;
    // 檢查此task是否超時
    if (toDelay < 0) { // 超時
      OS_ENTER_CRITICAL();
      printf("%d\tTask%d Deadline!\n", deadline, prio);
      OS_EXIT_CRITICAL();
    }
    else { // 未超時
      if (1) {
        OS_ENTER_CRITICAL();
        PrintMsgList();
        OS_EXIT_CRITICAL();
      }
      OSTimeDly(toDelay);
    }
    // 將deadline增加至下一周期
    deadline += OSTCBCur->period;
  }
}

// // Task2
// void Task2() {
//   BaseTask(2, 3, 6, 0);
// }

// // Task3
// void Task3() {
//   BaseTask(3, 4, 9, 0);
// }

void PrintMsgList() {
  while (msgList->next) {
    // 印出訊息佇列節點訊息
    // printf("%d\t%s\t%d\t%d\n", 
    //   msgList->next->tick,
    //   (msgList->next->event ? "Complete" : "Preemt  "),
    //   msgList->next->fromTaskId,
    //   msgList->next->toTaskId
    // );
    printf( "%d\t", msgList->next->tick) ;
    if ( msgList->next->event == 0 )
      printf( "%s\t", "Complete") ;
    else if ( msgList->next->event == 1 )
      printf( "%s\t", "Preemt  ") ;
    else if ( msgList->next->event == 2 )
      printf( "%s\t", "lock    ") ;
    else if ( msgList->next->event == 3 )
      printf( "%s\t", "unlock  ") ;
    printf( "%d\t", msgList->next->fromTaskId) ;
    printf( "%d\n", msgList->next->toTaskId) ;  
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

static  void  AddMsgList(int _tick, int _event, int _fromTaskId, int _toTaskId) {
    /* 尋找訊息佇列尾端 */
    msgTemp = msgList;
    while (msgTemp->next)
        msgTemp = msgTemp->next;
    /* 增加一個節點到訊息佇列 */
    msgTemp->next = (msg*)malloc(sizeof(msg));
    msgTemp->next->tick = _tick;
    msgTemp->next->event = _event;
    msgTemp->next->fromTaskId = _fromTaskId;
    msgTemp->next->toTaskId = _toTaskId;
    msgTemp->next->next = (msg*)0;
}
