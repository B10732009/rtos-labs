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
void Tasks2_1(void *pdata);
void Tasks2_2();
void PrintMsgList();
void InitMsgList();
void testAddMsgList(int _tick, int _event, int _fromTaskId, int _toTaskId);

// MAIN
void main(void) {
  // Initialize uC/OS-II
  OSInit();
  // Save environment to return to DOS
  PC_DOSSaveReturn();
  // Install uC/OS-II's context switch vector
  PC_VectSet(uCOS, OSCtxSw);
  // Random number semaphore
  // RandomSem = OSSemCreate(1);

  // Create tasks S1
  // OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE-1], 3);
  // OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE-1], 4);
  // OSTaskCreate(Task3, (void *)0, &TaskStk[2][TASK_STK_SIZE-1], 5);

  // Create tasks S2
  OSTaskCreate(Tasks2_1, (void *)0, &TaskStk[0][TASK_STK_SIZE-1], 3);
  OSTaskCreate(Tasks2_2, (void *)0, &TaskStk[1][TASK_STK_SIZE-1], 4);
  // Create mutexs
  R1 = OSMutexCreate (1, &R1_error);
  R2 = OSMutexCreate (2, &R2_error);
  // Initialize message list
  InitMsgList();
  // Start multitasking
  OSStart();
}

void Task1(void *pdata) {
  INT16S key;
  INT8U error;
  int prio = 3;
  int tempPrio;
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

    // OS_ENTER_CRITICAL();
    // testAddMsgList(OSTimeGet(), 3, 100, 100);
    // OS_EXIT_CRITICAL();

    // 取得開始時間
    // start = OSTimeGet();
    // 等待task執行結束
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 4 && !useR1) { 
        tempPrio = OSTCBCur->OSTCBPrio;
        OSMutexPend(R1, 5, &error);
        // OS_ENTER_CRITICAL();
        // testAddMsgList(OSTimeGet(), 2, 100 + tempPrio, 100 + OSTCBCur->OSTCBPrio);
        // OS_EXIT_CRITICAL();
        useR1 = 1;
      }
      else if (OSTCBCur->computeTime == 2 && !useR2) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1 ;
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

void Task2() {
  INT16S key;
  INT8U error;
  int prio = 4;
  int tempPrio;
  int arrive = 4;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 6;
  int period = 30;
  int deadline = arrive + period;
  int useR2 = 0; // R2是否正在使用
  
  // Some code for startup task
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;

  OSTimeDly(4);
    
  while (1) {
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 4 && !useR2) { 
        tempPrio = OSTCBCur->OSTCBPrio;
        OSMutexPend(R2, 5, &error);
        useR2 = 1;
      }
    }  
    // Release mutex
    OSMutexPost(R2);
    useR2 = 0;
    
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

void Task3() {
  INT16S key;
  INT8U error;
  int prio = 5;
  int tempPrio;
  int arrive = 0;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 9;
  int period = 30;
  int deadline = arrive + period;
  int useR1 = 0; // R2是否正在使用
  
  // Some code for startup task
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;
    
  while (1) {
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 7 && !useR1) { 
        // tempPrio = OSTCBCur->OSTCBPrio;
        OSMutexPend(R1, 5, &error);
        tempPrio = OSTCBCur->OSTCBPrio;
        useR1 = 1;
      }
    }  
    // Release mutex
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

void Tasks2_1(void *pdata) {
  INT16S key;
  INT8U error;
  int prio = 3;
  int tempPrio;
  int arrive = 5;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 11;
  int period = 40;
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

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;

  OSTimeDly(5);
    
  while (1) {
    // See if key has been pressed
    if (PC_GetKey(&key) == TRUE) {                     
      // Yes, see if it's the ESCAPE key
      if (key == 0x1B) {
        // Return to DOS
        PC_DOSReturn();
      }
    }

    while (OSTCBCur->computeTime > 0) {
      if ( OSTCBCur->computeTime == 9 && !useR2 ) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1 ;
      } // if
      if ( OSTCBCur->computeTime == 6 && useR2 ) {
        OSMutexPost(R2);
        useR2 = 0 ;
      } // if 
      if ( OSTCBCur->computeTime == 6 && !useR1 ) { 
        OSMutexPend(R1, 5, &error);
        useR1 = 1;
      } // if 
      if ( OSTCBCur->computeTime == 3 && useR1 ) {
        OSMutexPost(R1);
        useR1 = 0 ;
      } // else if
      if ( OSTCBCur->computeTime == 3 && !useR2 ) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1 ;
      } // if
       
    } // while 
    // Release mutex
    OSMutexPost(R2);
    useR2 = 0;
    
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

} // Tasks2_1()

void Tasks2_2() {
  INT16S key;
  INT8U error;
  int prio = 4;
  int tempPrio;
  int arrive = 0;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 12;
  int period = 40;
  int deadline = arrive + period;
  int useR1 = 0 ;
  int useR2 = 0; // R2是否正在使用
  
  // Some code for startup task
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;
    
  while (1) {
    while (OSTCBCur->computeTime > 0) {
      if ( OSTCBCur->computeTime == 10 && !useR1) { 
        OSMutexPend(R1, 5, &error);
        useR1 = 1;
      } // if 
      if ( OSTCBCur->computeTime == 4 && useR1 ) {
        OSMutexPost(R1);
        useR1 = 0 ;
      } // else if 
      if ( OSTCBCur->computeTime == 4 && !useR2 ) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1; 
      } // else if
      if ( OSTCBCur->computeTime == 2 && useR2 ) {
        OSMutexPost(R2);
        useR2 = 0 ;
      } // else if 
      if ( OSTCBCur->computeTime == 2 && !useR1 ) {
        OSMutexPend(R1, 5, &error);
        useR1 = 1; 
      } // else if 
    }  
    // Release mutex
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

} // Tasks2_2()

void PrintMsgList() {
  while (msgList->next) {
    printf( "%d\t", msgList->next->tick) ;
    if ( msgList->next->event == 1 )
      printf( "%s\t", "Complete") ;
    else if ( msgList->next->event == 0 )
      printf( "%s\t", "  Preemt") ;
    else if ( msgList->next->event == 2 )
      printf( "%s\t", "    lock") ;
    else if ( msgList->next->event == 3 )
      printf( "%s\t", "  unlock") ;
    if ( msgList->next->resource == 1 || msgList->next->resource == 2 ) {
      if ( msgList->next->resource == 1 )
        printf( "%s\t", "R1") ;
      else 
        printf( "%s\t", "R2") ;
      printf( "(Prio =%d changes to=%d)\n", msgList->next->fromTaskId, msgList->next->toTaskId) ;
    } // if
    else { 
      printf( "\t\t%d\t\t", msgList->next->fromTaskId) ;
      printf( "%d\n", msgList->next->toTaskId) ;
    } // else     
    // 將印過的節點刪掉
    msgTemp = msgList;
    msgList = msgList->next;
    free(msgTemp);
  }
} // Tasks2_2() 

void InitMsgList() {
  // 新增dummy節點(簡化串列操作)
  msgList = (msg*)malloc(sizeof(msg));
  msgList->next = (msg*)0;
}

void testAddMsgList(int _tick, int _event, int _fromTaskId, int _toTaskId) {
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
