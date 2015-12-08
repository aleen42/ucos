/*
*********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*                         (c) Copyright 1992-2001, Jean J. Labrosse, Weston, FL
*                                          All Rights Reserved
*
*
*                                       80x86/80x88 Specific code
*                                          LARGE MEMORY MODEL
*
*                                          Borland C/C++ V4.51
*
* File         : OS_CPU_C.C
* By           : Jean J. Labrosse
*********************************************************************************************************
*/
#include  "includes.h"
#include  <string.h>


#define  OS_CPU_GLOBALS

#define FP_OFF(fp)((unsigned)(fp))
#define FP_SEG(fp)((unsigned)((unsigned long)(fp) >> 16))

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              pdata         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then 
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : Interrupts are enabled when your task starts executing. You can change this by setting the
*              PSW to 0x0002 instead.  In this case, interrupts would be disabled upon task startup.  The
*              application code would be responsible for enabling interrupts at the beginning of the task
*              code.  You will need to modify OSTaskIdle() and OSTaskStat() so that they enable 
*              interrupts.  Failure to do this will make your system crash!
*********************************************************************************************************
*/

void TaskBucket()
{
	while (1)
		;
}



OS_STK *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
#if 0
    INT16U *stk;
   

    opt    = opt;                           /* 'opt' is not used, prevent warning                      */
    stk    = (INT16U *)ptos;                /* Load stack pointer                                      */
    *stk-- = (INT16U)FP_SEG(pdata);         /* Simulate call to function with argument                 */
    *stk-- = (INT16U)FP_OFF(pdata);         
    *stk-- = (INT16U)FP_SEG(task);
    *stk-- = (INT16U)FP_OFF(task);
    *stk-- = (INT16U)0x0202;                /* SW = Interrupts enabled                                 */
    *stk-- = (INT16U)FP_SEG(task);          /* Put pointer to task   on top of stack                   */
    *stk-- = (INT16U)FP_OFF(task);
    *stk-- = (INT16U)0xAAAA;                /* AX = 0xAAAA                                             */
    *stk-- = (INT16U)0xCCCC;                /* CX = 0xCCCC                                             */
    *stk-- = (INT16U)0xDDDD;                /* DX = 0xDDDD                                             */
    *stk-- = (INT16U)0xBBBB;                /* BX = 0xBBBB                                             */
    *stk-- = (INT16U)0x0000;                /* SP = 0x0000                                             */
    *stk-- = (INT16U)0x1111;                /* BP = 0x1111                                             */
    *stk-- = (INT16U)0x2222;                /* SI = 0x2222                                             */
    *stk-- = (INT16U)0x3333;                /* DI = 0x3333                                             */
    *stk-- = (INT16U)0x4444;                /* ES = 0x4444                                             */
    *stk   = _DS;                           /* DS = Current value of DS                                */
    return ((OS_STK *)stk);

#endif

	OS_STK*	stk;

	stk = (OS_STK *) ptos;			// Load stack pointer

	*--stk = (INT32U) pdata;		// Simulate a function call (to pass the parameter)
	*--stk = (INT32U) TaskBucket;			// Return address in case the task exits.

	*--stk = 0x00000202;			// Eflags (interrupt flag enabled)
	*--stk = 0x18;					// CS: define in TA
	*--stk = (INT32U) task;			// Entry point

	*--stk = 0;						// EAX
	*--stk = 0;						// ECX
	*--stk = 0;						// EDX
	*--stk = 0;						// EBX
	*--stk = 0;						// ESP (unused)
	*--stk = 0;						// EBP
	*--stk = 0;						// ESI
	*--stk = 0;						// EDI

    return stk;
        


}

/*$PAGE*/
/*
*********************************************************************************************************
*                        INITIALIZE A TASK'S STACK FOR FLOATING POINT EMULATION
*
* Description: This function MUST be called BEFORE calling either OSTaskCreate() or OSTaskCreateExt() in
*              order to initialize the task's stack to allow the task to use the Borland floating-point 
*              emulation.  The returned pointer MUST be used in the task creation call.
*
*              Ex.:   OS_STK TaskStk[1000];
*
*
*                     void main (void)
*                     {
*                         OS_STK *ptos;
*                         OS_STK *pbos;
*                         INT32U  size;
*
*
*                         OSInit();
*                         .
*                         .
*                         ptos  = &TaskStk[999];
*                         pbos  = &TaskStk[0];
*                         psize = 1000;
*                         OSTaskStkInit_FPE_x86(&ptos, &pbos, &size);
*                         OSTaskCreate(Task, (void *)0, ptos, 10);
*                         .
*                         .
*                         OSStart();
*                     }
*
* Arguments  : pptos         is the pointer to the task's top-of-stack pointer which would be passed to 
*                            OSTaskCreate() or OSTaskCreateExt().
*
*              ppbos         is the pointer to the new bottom of stack pointer which would be passed to
*                            OSTaskCreateExt().
*
*              psize         is a pointer to the size of the stack (in number of stack elements).  You 
*                            MUST allocate sufficient stack space to leave at least 384 bytes for the 
*                            floating-point emulation.
*
* Returns    : The new size of the stack once memory is allocated to the floating-point emulation.
*
* Note(s)    : 1) _SS  is a Borland 'pseudoregister' and returns the contents of the Stack Segment (SS)
*              2) The pointer to the top-of-stack (pptos) will be modified so that it points to the new
*                 top-of-stack.
*              3) The pointer to the bottom-of-stack (ppbos) will be modified so that it points to the new
*                 bottom-of-stack.
*              4) The new size of the stack is adjusted to reflect the fact that memory was reserved on
*                 the stack for the floating-point emulation.
*********************************************************************************************************
*/


/*$PAGE*/
#if OS_CPU_HOOKS_EN
/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookBegin (void)
{
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookEnd (void)
{
}
#endif


/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the 
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void OSTaskSwHook (void)
{
}

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your 
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
void OSTaskStatHook (void)
{
}

/*
*********************************************************************************************************
*                                           OSTCBInit() HOOK
*
* Description: This function is called by OSTCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
void OSTimeTickHook (void)
{
}


/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do  
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION >= 251
void OSTaskIdleHook (void)
{
}
#endif
#endif
