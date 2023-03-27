/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        3       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *RandomSem;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/
        void  Task1(void *data);       /* Function prototypes of Startup task           */
        void  Task2();  
        void  Task3();
        void  PrintMsg();
        void  InitMsg();

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    RandomSem   = OSSemCreate(1);                          /* Random number semaphore                  */

    OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE-1], 1);
    OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE-1], 2);
    OSTaskCreate(Task3, (void *)0, &TaskStk[2][TASK_STK_SIZE-1], 3);
    
    InitMsg();

    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  Task1 (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;
    int start ;
    int end ;
    int todelay ;
    int count = 1 ;
    OSTCBCur->computime = 1 ;
    OSTCBCur->period = 3 ;
    pdata = pdata;                                         /* Prevent compiler warning                 */
    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();
    OSStatInit();  
    
    start = OSTimeGet() ;                                        /* Initialize uC/OS-II's statistics         */
    while(1) {
        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            } // if 
        } // if 
        
        while( OSTCBCur->computime > 0 )
          ; // Busywaiting 
        end = OSTimeGet() ;
        todelay = OSTCBCur->period - (end-start) ;
        start += OSTCBCur->period;
        OSTCBCur->computime = 1 ; // task1的計算時間是1
        if ( todelay < 0 ) {
          OS_ENTER_CRITICAL();
          printf("Task1 Deadline!\n") ;
          OS_EXIT_CRITICAL();
        } // if   
        else {
          //sprintf(str, "Time Ticks:%d Task1 finish!", OSTimeGet()) ; 
          //PC_DispStr( 0, count, str, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
          
          
          
          
          OS_ENTER_CRITICAL();
          PrintMsg();
          OS_EXIT_CRITICAL();
          
          
          
          
          count = count + 3 ;
          OSTimeDly(todelay) ;
        } // else  
    } // while
}
/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  Task2() {
    int start ;
    int end ;
    int todelay ;
    int count = 2 ;
    OSTCBCur->computime = 3 ;
    OSTCBCur->period = 6 ;
    start = OSTimeGet() ;
    while(1) {
      while( OSTCBCur->computime > 0 )
        ; // Busywaiting 
      end = OSTimeGet() ;
      todelay = OSTCBCur->period - (end-start) ;
      start = start + OSTCBCur->period ; 
      OSTCBCur->computime = 3 ; // task2的計算時間是3
      if ( todelay < 0 ) {
        OS_ENTER_CRITICAL();
        printf("Task2 Deadline!\n") ;
        OS_EXIT_CRITICAL();
      } // if   
      else {
        //sprintf(str, "Time Ticks:%d Task2 finish!", OSTimeGet()) ; 
        //PC_DispStr( 0, count, str, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        //count = count + 3 ;
        OSTimeDly(todelay) ;
      } // else       
      
    } // while 

} // Task2()

void  Task3() {
    int start ;
    int end ;
    int todelay ;
    int count = 3 ;
    OSTCBCur->computime = 4 ;
    OSTCBCur->period = 9 ;
    start = OSTimeGet() ;
    while(1) {
      while( OSTCBCur->computime > 0 )
        ; // Busywaiting 
      end = OSTimeGet() ;
      todelay = OSTCBCur->period - (end-start) ;
      start = start + OSTCBCur->period ;
      OSTCBCur->computime = 4 ; // task3的計算時間是4
      if ( todelay < 0 ) {
        OS_ENTER_CRITICAL();
        printf("Task3 Deadline!\n") ;
        OS_EXIT_CRITICAL();
      } // if   
      else {
        //sprintf(str, "Time Ticks:%d Task3 finish!", OSTimeGet()) ; 
        //PC_DispStr( 0, count, str, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        //count = count + 3 ;
        OSTimeDly(todelay) ;
      } // else       
      
    } // while 

} // Task2()

void PrintMsg() {
  //return;
  while (msgList->next) {
    /* 印出訊息佇列節點訊息 */
    // char str[128];
    // sprintf(str, "%d %s %d %d", msgList->next->tick,
    //   (msgList->next->event ? "Complete" : "Preemt"),
    //  msgList->next->fromTaskId,
    //    msgList->next->toTaskId
    //  ); 
    // PC_DispStr( 0, 0, str, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    printf("%d\t%s\t%d\t%d\n", 
      msgList->next->tick,
      (msgList->next->event ? "Complete" : "Preemt  "),
      msgList->next->fromTaskId,
      msgList->next->toTaskId
    );
    /* 將印過的節點刪掉 */
    msgTemp = msgList;
    msgList = msgList->next;
    free(msgTemp);
  }
}

void InitMsg() {
  /* 新增dummy節點(簡化串列操作) */
  msgList = (msg*)malloc(sizeof(msg));
  msgList->next = (msg*)0;
}