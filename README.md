# Real-time Operating System Labs #

## Lab1 : Periodic Task Emulation ##

### Objectives ###

- To implement periodic tasks
- To observe the scheduling behaviors 
    - Fixed priority scheduling

### Task Sets ###

Two sets of periodic tasks.
- Task set 1 = { t1(1, 3), t2(3, 6) }
- Task set 2 = { t1(1, 3), t2(3, 6), t3(4, 9) }
- Tasks all arrive at the same time.
- Show context switch behaviors.
- Show deadline violations if there is any.

### Implementation ###

Create Tasks
- Create task function (in `TEST.C`)
    - The while loop part is for simulating the periodic task, so we separate it to a independent function `BaseTask()` , and let `Task1()` , `Task2()` , `Task3()` call it.
    - Note that the `Task1()` is *start-up* task, which includes extra code.
- Use `OSTaskCreate()` to add a task to OS. (in `TEST.C`)
    - Because we are using `RM`, assign higher priority to the task which has shorter period.

Count Executing Time
- Add `computeTime` , `period` to TCB (in `uCOS_II.H`)
    - Use these 2 attritubes to store the **remaining computing time** and period of each task.
- Subtract 1 from `computeTime` every tick (in `OS_CORE.C`)
    - In function `OSTimeTick()` , subtract 1 from `computeTime` of running task's TCB.

Build Message List
- Message list is implemented in linked list.
- Declare message list node structure & head pointer (in `uCOS_II.H`)
    - Add `OS_EXT` to make the variables can be shared between files.
- Create `InitMsgList()` function to intialize the list. (in `TEST.C`)
    - This function should be called in the `main()` of `TEST.C`.
- Create `addMsgList()` function to insert a message. (in `OS_CORE.C`)
    - Add node at tail of the list.
- Create `printMsgList()` function to print messages. (in `TEST.C`)
    - After print, the message node will be delete.
    - Print until there is no node in the list.

Add Message When Event Occur
- In `OSStart()` 
    - *prempt* (from `Idle` to `Task1`)
- In `OSIntExit()`
    - *prempt*
- In `OS_Sched()`
    - *complete*

### Issues ###

- The deadline violation message should show its deadline rather than the time we know it.
    - For example, in the set1, the task3 should violate at 9 tick, but we can only know it at 2x tick. In this case, we need to print `9` rather than `2x`.
- The demo environment doesn't support DOS-related functions, so we need to comment out all DOS-related code.
- The default task getting OS's statistics seems to cause some bug to tick counting of OS.
    - We choose to comment out the `OSStatInit()`.
- The demo environment can run the program without `OSInit()` (I don't know why?), if there is any problem about tick counting, try comment out `OSInit()`.