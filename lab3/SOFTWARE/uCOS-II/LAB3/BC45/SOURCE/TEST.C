#include "includes.h"

#define TASK_STK_SIZE 512 // Size of each task's stacks (# of WORDs)
#define N_TASKS 3 // Number of identical tasks

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE]; // Tasks stacks
OS_STK TaskStartStk[TASK_STK_SIZE];
char   TaskData[N_TASKS]; // Parameters to pass to each task
OS_EVENT *RandomSem;
OS_EVENT *R1; // pointer to R1 ECB 
OS_EVENT *R2; // pointer to R2 ECB
INT8U R1_error; // R1 error message 
INT8U R2_error; // R2 error message

void Task1(void *pdata); // Function prototypes of Startup task
void Task2();  
void Task3();
void Tasks2_1(void *pdata);
void Tasks2_2();
void PrintMsgList();
void InitMsgList();

void main(void) {
  OSInit();
  PC_DOSSaveReturn();
  PC_VectSet(uCOS, OSCtxSw);

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
  int arrive = 8;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 6;
  int period = 30;
  int deadline = arrive + period;
  int useR1 = 0; // R1是否正在使用
  int useR2 = 0; // R2是否正在使用
  
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  // Some code for startup task
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

  OSTimeDly(arrive);
    
  while (1) {
    if (PC_GetKey(&key) == TRUE) {      
      if (key == 0x1B) {
        PC_DOSReturn();
      }
    }

    // 等待task執行結束
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 4 && !useR1) {
        OSMutexPend(R1, 5, &error);
        useR1 = 1;
      }
      else if (OSTCBCur->computeTime == 2 && !useR2) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1;
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
      OS_ENTER_CRITICAL();
      PrintMsgList();
      OS_EXIT_CRITICAL();
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
  int arrive = 4;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 6;
  int period = 30;
  int deadline = arrive + period;
  int useR2 = 0; // R2是否正在使用

#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;

  OSTimeDly(arrive);
    
  while (1) {
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 4 && !useR2) {
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
  int arrive = 0;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 9;
  int period = 30;
  int deadline = arrive + period;
  int useR1 = 0; // R1是否正在使用

#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;
    
  while (1) {
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 7 && !useR1) {
        OSMutexPend(R1, 5, &error);
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
  int arrive = 5;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 11;
  int period = 40;
  int deadline = arrive + period;
  int useR1 = 0; // R1是否正在使用
  int useR2 = 0; // R2是否正在使用
  
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  // Some code for startup task
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

  OSTimeDly(arrive);
    
  while (1) {
    if (PC_GetKey(&key) == TRUE) {
      if (key == 0x1B) {
        PC_DOSReturn();
      }
    }

    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 9 && !useR2) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1 ;
      }
      else if (OSTCBCur->computeTime == 6 && !useR1) { 
        OSMutexPend(R1, 5, &error);
        useR1 = 1;
      }
      else if (OSTCBCur->computeTime == 3 && useR1) {
        OSMutexPost(R1);
        useR1 = 0 ;
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
      OS_ENTER_CRITICAL();
      PrintMsgList();
      OS_EXIT_CRITICAL();
      OSTimeDly(toDelay);
    }
    // 將deadline增加至下一周期
    deadline += OSTCBCur->period;
  }

}

void Tasks2_2() {
  INT16S key;
  INT8U error;
  int prio = 4;
  int arrive = 0;
  int start = arrive;
  int end;
  int toDelay;
  int computeTime = 12;
  int period = 40;
  int deadline = arrive + period;
  int useR1 = 0; // R1是否正在使用
  int useR2 = 0; // R2是否正在使用

#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR  cpu_sr;
#endif

  OSTCBCur->computeTime = computeTime;
  OSTCBCur->period = period;
    
  while (1) {
    while (OSTCBCur->computeTime > 0) {
      if (OSTCBCur->computeTime == 10 && !useR1) { 
        OSMutexPend(R1, 5, &error);
        useR1 = 1;
      }
      else if (OSTCBCur->computeTime == 4 && !useR2) {
        OSMutexPend(R2, 5, &error);
        useR2 = 1; 
      }
      else if (OSTCBCur->computeTime == 2 && useR2) {
        OSMutexPost(R2);
        useR2 = 0;
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
      OSTimeDly(toDelay);
    }
    // 將deadline增加至下一周期
    deadline += OSTCBCur->period;
  }
}

void PrintMsgList() {
  while (msgList->next) {
    printf("%d\t", msgList->next->tick);
    switch (msgList->next->event) {
      case 0:
        printf("%s\t\t\t%d\t\t%d\n", "  Preemt", msgList->next->fromTaskId, msgList->next->toTaskId);
        break;
      case 1:
        printf("%s\t\t\t%d\t\t%d\n", "Complete", msgList->next->fromTaskId, msgList->next->toTaskId);
        break;
      case 2:
        printf( "%s\tR%d\t(Prio=%d changes to=%d)\n", "    Lock", msgList->next->resource, msgList->next->fromTaskId, msgList->next->toTaskId);
        break;
      case 3:
        printf( "%s\tR%d\t(Prio=%d changes to=%d)\n", "  Unlock", msgList->next->resource, msgList->next->fromTaskId, msgList->next->toTaskId);
        break;
      default:
        break; // never happen
    }
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