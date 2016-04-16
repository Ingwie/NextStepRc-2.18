/**
 *******************************************************************************
 * @file       mutex.c
 * @version    V1.1.6    
 * @date       2014.05.23
 * @brief      Mutex management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met: 
 *  
 *      * Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *      * Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution. 
 *      * Neither the name of the <ORGANIZATION> nor the names of its 
 *  contributors may be used to endorse or promote products derived 
 *  from this software without specific prior written permission. 
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * <h2><center>&copy; COPYRIGHT 2014 CooCox </center></h2>
 *******************************************************************************
 */ 



/*---------------------------- Include ---------------------------------------*/
#include <coocox.h>


/*---------------------------- Variable Define -------------------------------*/
#if CFG_MUTEX_EN > 0

OS_MutexID MutexFreeID = 0;               /*!< Point to next vliad mutex ID.  */
MUTEX      MutexTbl[CFG_MAX_MUTEX] = {{0}}; /*!< Mutex struct array             */
	


/**
 *******************************************************************************
 * @brief      Create a mutex	 
 * @param[in]  None	 	 
 * @param[out] None  
 * @retval     E_CREATE_FAIL  Create mutex fail.
 * @retval     others         Create mutex successful.		 
 *
 * @par Description					  
 * @details    This function is called to create a mutex. 
 * @note  		
 *******************************************************************************
 */
OS_MutexID CoCreateMutex(void)
{
    OS_MutexID id;
    P_MUTEX pMutex;
    OsSchedLock();
    
    /* Assign a free mutex control block */
    if(MutexFreeID < CFG_MAX_MUTEX )
    {
        id  = MutexFreeID++;
        OsSchedUnlock();
        pMutex = &MutexTbl[id];
        pMutex->hipriTaskID  = INVALID_ID;
        pMutex->originalPrio = 0xff;
        pMutex->mutexFlag    = MUTEX_FREE;  /* Mutex is free,not was occupied */
        pMutex->taskID       = INVALID_ID;
        pMutex->waittingList = Co_NULL;
        return id;                      /* Return mutex ID                    */			
    }	
    
    OsSchedUnlock();	 
    return E_CREATE_FAIL;               /* No free mutex control block        */	
}



/**	
 *******************************************************************************		 	
 * @brief      Enter a critical area  
 * @param[in]  mutexID    Specify mutex. 	 
 * @param[out] None   
 * @retval     E_INVALID_ID  Invalid mutex id. 	
 * @retval     E_CALL        Error call in ISR.
 * @retval     E_OK          Enter critical area successful.
 *
 * @par Description
 * @details    This function is called when entering a critical area.	 
 * @note 
 *******************************************************************************
 */
StatusType CoEnterMutexSection(OS_MutexID mutexID)
{
    P_OSTCB ptcb,pCurTcb;
    P_MUTEX pMutex;

#if CFG_EVENT_EN >0
    P_ECB pecb;
#endif

    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        return E_CALL;
    }
    if(OSSchedLock != 0)                /* Is OS lock?                        */
    {								 
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }	

#if CFG_PAR_CHECKOUT_EN >0
    if(mutexID >= MutexFreeID)          /* Invalid 'mutexID'                  */
    {
        return E_INVALID_ID;	
    }
#endif

    OsSchedLock();
    pCurTcb = TCBRunning;
    pMutex  = &MutexTbl[mutexID];
    
    pCurTcb->mutexID = mutexID;
    if(pMutex->mutexFlag == MUTEX_FREE)       /* If mutex is available        */	 
    {
        pMutex->originalPrio = pCurTcb->prio; /* Save priority of owning task */   
        pMutex->taskID       = pCurTcb->taskID;   /* Acquire the resource     */
        pMutex->hipriTaskID  = pCurTcb->taskID;
        pMutex->mutexFlag    = MUTEX_OCCUPY;      /* Occupy the mutex resource*/
    }
    /* If the mutex resource had been occupied                                */
    else if(pMutex->mutexFlag == MUTEX_OCCUPY)	 	
    {	
		ptcb = &TCBTbl[pMutex->taskID];
        if(ptcb->prio > pCurTcb->prio)  /* Need to promote priority of owner? */
        {
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
			DeleteTaskPri(ptcb->prio);
			ActiveTaskPri(pCurTcb->prio);
#endif	
            ptcb->prio = pCurTcb->prio;	    /* Promote prio of owner          */
            
            /* Upgarde the highest priority about the mutex                   */
            pMutex->hipriTaskID	= pCurTcb->taskID;	
            if(ptcb->state == TASK_READY)   /* If the task is ready to run    */
            {
                RemoveFromTCBRdyList(ptcb); /* Remove the task from READY list*/
                InsertToTCBRdyList(ptcb);   /* Insert the task into READY list*/
            }
#if CFG_EVENT_EN >0
            /* If the task is waiting on a event                              */
            else if(ptcb->eventID != INVALID_ID) 
            {
                pecb = &EventTbl[ptcb->eventID];
                
                /* If the event waiting type is preemptive Priority           */
                if(pecb->eventSortType == EVENT_SORT_TYPE_PRIO)	
                {
                    /* Remove the task from event waiting list                */
                    RemoveEventWaittingList(ptcb);
                    
                    /* Insert the task into event waiting list                */ 	
                    EventTaskToWait(pecb,ptcb);		
                }	
            }
#endif	
        }
        
        pCurTcb->state   = TASK_WAITING;    /* Block current task             */
		TaskSchedReq     = Co_TRUE;
        pCurTcb->TCBnext = Co_NULL;
        pCurTcb->TCBprev = Co_NULL;
        
        ptcb = pMutex->waittingList;
        if(ptcb == Co_NULL)               /* If the event waiting list is empty  */
        {
            pMutex->waittingList = pCurTcb; /* Insert the task to head        */
        }
        else                        /* If the event waiting list is not empty */
        {            	
            while(ptcb->TCBnext != Co_NULL)    /* Insert the task to tail        */
            {
                ptcb = ptcb->TCBnext;		
            }
            ptcb->TCBnext    = pCurTcb;
            pCurTcb->TCBprev = ptcb;
            pCurTcb->TCBnext = Co_NULL;
        }
    }
    OsSchedUnlock();
    return E_OK;			
}


/**
 *******************************************************************************
 * @brief      Leave from a critical area	 
 * @param[in]  mutexID 	Specify mutex id.	 
 * @param[out] None 
 * @retval     E_INVALID_ID  Invalid mutex id.
 * @retval     E_CALL        Error call in ISR.
 * @retval     E_OK          Exit a critical area successful.
 *
 * @par Description		 
 * @details    This function must be called when exiting from a critical area.	
 * @note 
 *******************************************************************************
 */
StatusType CoLeaveMutexSection(OS_MutexID mutexID)
{
    P_OSTCB ptcb;
    P_MUTEX pMutex;
    U8      prio;
    U8      taskID;
    
    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        return E_CALL;
    }

#if CFG_PAR_CHECKOUT_EN >0
    if(mutexID >= MutexFreeID)
    {
        return E_INVALID_ID;            /* Invalid mutex id, return error     */
    }
#endif	
    OsSchedLock();
    pMutex = &MutexTbl[mutexID];        /* Obtain point of mutex control block*/   
    ptcb = &TCBTbl[pMutex->taskID];
	ptcb->mutexID = INVALID_ID;
	if(pMutex->waittingList == Co_NULL)    /* If the mutex waiting list is empty */
    {
        pMutex->mutexFlag = MUTEX_FREE; /* The mutex resource is available    */
        pMutex->taskID    = INVALID_ID;
        OsSchedUnlock();
    }	
    else              /* If there is at least one task waitting for the mutex */
    { 
        taskID = pMutex->taskID;        /* Get task ID of mutex owner         */
        
                                /* we havn't promoted current task's priority */
        if(pMutex->hipriTaskID == taskID)   
        {
            ptcb = pMutex->waittingList;/* Point to mutex first waiting task  */		
            prio = ptcb->prio; 
            while(ptcb != Co_NULL)         /* Find the highest priority task     */
            {
                if(ptcb->prio < prio)  		
                {
                    prio = ptcb->prio;
                    pMutex->hipriTaskID = ptcb->taskID;
                }
                ptcb = ptcb->TCBnext;					
            }
        }
        else                     /* we have promoted current task's priority  */
        {
			prio = TCBTbl[taskID].prio;
        }
        
        /* Reset the task priority */
		pMutex->taskID = INVALID_ID;	
		CoSetPriority(taskID,pMutex->originalPrio);
        
        /* Find first task in waiting list ready to run  */	
        ptcb                 = pMutex->waittingList; 		
        pMutex->waittingList = ptcb->TCBnext;	
        pMutex->originalPrio = ptcb->prio;
        pMutex->taskID       = ptcb->taskID;

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		if(prio != ptcb->prio)
		{
			DeleteTaskPri(ptcb->prio);
			ActiveTaskPri(prio);			
		}
#endif	

        ptcb->prio           = prio;    /* Raise the task's priority          */       
        				   
        /* Insert the task which acquire the mutex into ready list.           */
        ptcb->TCBnext = Co_NULL;
        ptcb->TCBprev = Co_NULL;

		InsertToTCBRdyList(ptcb);     /* Insert the task into the READY list  */
        OsSchedUnlock();
    }
    return E_OK;			
}

/**
 *******************************************************************************
 * @brief      Remove a task from mutex waiting list	   
 * @param[in]  ptcb   TCB which will remove out.	 
 * @param[out] None 	 
 * @retval     None
 *
 * @par Description		 
 * @details   This function be called when delete a task.	
 * @note 
 *******************************************************************************
 */
void RemoveMutexList(P_OSTCB ptcb)
{
    U8 prio;
	OS_TID taskID;
    P_MUTEX pMutex;
    pMutex = &MutexTbl[ptcb->mutexID];
    
    /* If only one task waiting on mutex                                      */	
    if((ptcb->TCBnext ==Co_NULL) && (ptcb->TCBprev == Co_NULL))
    {
        pMutex->waittingList = Co_NULL;     /* Waiting list is empty             */
    }
    else if(ptcb->TCBnext == Co_NULL)  /* If the task is the last of waiting list*/
    {
        /* Remove task from mutex waiting list                                */
        ptcb->TCBprev->TCBnext = Co_NULL;
        ptcb->TCBprev = Co_NULL;
    }	
    else if(ptcb->TCBprev ==  Co_NULL)/* If the task is the first of waiting list*/
    {
        /* Remove task from waiting list                                      */
        ptcb->TCBnext->TCBprev = Co_NULL;
        ptcb->TCBnext = Co_NULL;
    }
    else                      /* If the task is in the middle of waiting list */
    {
        /* Remove task from wait list */
        ptcb->TCBnext->TCBprev = ptcb->TCBprev;
        ptcb->TCBprev->TCBnext = ptcb->TCBnext;
        ptcb->TCBprev          = Co_NULL;
        ptcb->TCBnext          = Co_NULL;
    }
    
    ptcb->mutexID = INVALID_ID;
    
    /* If the task have highest priority in mutex waiting list                */	
    if(pMutex->hipriTaskID == ptcb->taskID)						
    {
        ptcb = pMutex->waittingList;
        prio = pMutex->originalPrio; 
        pMutex->hipriTaskID = pMutex->taskID;
        while(ptcb != Co_NULL)           /* Find task ID of highest priority task*/
        {
            if(ptcb->prio < prio)
            {
                prio = ptcb->prio;
                pMutex->hipriTaskID = ptcb->taskID;
            }
            ptcb = ptcb->TCBnext;			
        }
		taskID = pMutex->taskID;
		pMutex->taskID = INVALID_ID;
		CoSetPriority(taskID,prio);         /* Reset the mutex ower priority  */
		pMutex->taskID = taskID;
    }
}

#endif
