/*
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */

#include <geekos/syscall.h>
#include <geekos/errno.h>
#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/elf.h>
#include <geekos/malloc.h>
#include <geekos/screen.h>
#include <geekos/keyboard.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/timer.h>
#include <geekos/vfs.h>
#include <geekos/signal.h>
#include <geekos/sem.h>
#include <geekos/projects.h>
#include <geekos/smp.h>

#define MAX_SEM_SIZE 25
#define MAX_NAME_SIZE 25

struct Semaphore
{
    char *name;
    int SID;
    int value;
    int num_users;
    struct Thread_Queue blockQueue;
};

static struct Semaphore *sem_list[MAX_SEM_SIZE] = {0};

/*
 * Create or find a semaphore.
 * Params:
 *   state->ebx - user address of name of semaphore
 *   state->ecx - length of semaphore name
 *   state->edx - initial semaphore count
 * Returns: the global semaphore id
 */
int Sys_Open_Semaphore(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.
    /*TODO_P(PROJECT_SEMAPHORES, "Open_Semaphore system call");
    return EUNSUPPORTED;*/
    int i;

    int length = state->ecx;
    int ival = state->edx;

    if (length > MAX_NAME_SIZE)
    {
        return ENAMETOOLONG;
    }

    // Sys_PrintString Part
    /*   state->ebx - user pointer of string to be printed
     *   state->ecx - number of characters to print
     *   Copy_User_String(state->ebx, length, 1023, (char **)&buf))
     */

    char *name = Malloc(length + 1);
    Copy_From_User(name, state->ebx, length + 1);

    int count = 0;
    while (sem_list[count] != 0)
    {
        if (strncmp(sem_list[count]->name, name, length) == 0)
        {
            sem_list[count]->num_users++;
            Free(name);
            return sem_list[count]->SID;
        }

        count++;

        if (count == MAX_SEM_SIZE)
        {
            Free(name);
            return ENOSPACE;
        }
    }

    struct Semaphore *new_sem = Malloc(sizeof(struct Semaphore));

    Clear_Thread_Queue(&new_sem->blockQueue);
    new_sem->name = name;
    new_sem->SID = count;
    new_sem->value = ival;
    new_sem->num_users = 1;
    sem_list[count] = new_sem;

    return count;
}

/*
 * Acquire a semaphore.
 * Assume that the process has permission to access the semaphore,
 * the call will block until the semaphore count is >= 0.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
int Sys_P(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.
    /*TODO_P(PROJECT_SEMAPHORES, "P (semaphore acquire) system call");
    return EUNSUPPORTED;*/

    int SID = state->ebx;

    if (SID < 0 || SID >= MAX_SEM_SIZE || sem_list[SID] == 0)
        return EINVALID;

    bool iflag = Begin_Int_Atomic();

    if (sem_list[SID]->value > 0)
    {
        sem_list[SID]->value--;
        End_Int_Atomic(iflag);
        return 0;
    }

    Wait(&sem_list[SID]->blockQueue);
    iflag = Begin_Int_Atomic();
    sem_list[SID]->value--;
    End_Int_Atomic(iflag);

    return 0;
}

/*
 * Release a semaphore.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
int Sys_V(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.
    /*TODO_P(PROJECT_SEMAPHORES, "V (semaphore release) system call");
    return EUNSUPPORTED;*/

    int SID = state->ebx;

    if (SID < 0 || SID >= MAX_SEM_SIZE || sem_list[SID] == 0)
        return EINVALID;

    bool iflag = Begin_Int_Atomic();

    sem_list[SID]->value++;
    if (!Is_Thread_Queue_Empty(&sem_list[SID]->blockQueue))
        Wake_Up_One(&sem_list[SID]->blockQueue);

    End_Int_Atomic(iflag);

    return 0;
}

/*
 * Destroy our reference to a semaphore.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
int Sys_Close_Semaphore(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.
    /*TODO_P(PROJECT_SEMAPHORES, "Close_Semaphore system call");
    return EUNSUPPORTED;*/

    int SID = state->ebx;
    if (SID < 0 || SID >= MAX_SEM_SIZE || sem_list[SID] == 0)
        return EINVALID;

    sem_list[SID]->num_users--;
    if (sem_list[SID]->num_users == 0)
    {
        Free(sem_list[SID]->name);
        Free(sem_list[SID]);
        sem_list[SID] = 0;
    }

    return 0;
}