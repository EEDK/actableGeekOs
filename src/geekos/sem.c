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

#define MAX_SEM_SIZE 30
#define MAX_NAME_SIZE 30

struct Semaphore
{
    int SID;
    int count;
    int num_users;

    char *name;
    struct Thread_Queue waitQueue;
};

static struct Semaphore semlist[MAX_SEM_SIZE];

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

    int i = 0;
    int length = state->ecx;
    int ival = state->edx;

    if (length > MAX_NAME_SIZE)
    {
        return ENAMETOOLONG;
    }

    char *name = Malloc(length + 1);
    Copy_From_User(name, state->ebx, length + 1);

    //Search semlist
    while (semlist[i].name != 0)
    {
        if (strcmp(semlist[i].name, name) == 0)
        {
            semlist[i].num_users++;
            Free(name);
            return semlist[i].SID;
        }

        i += 1;
        if (i == MAX_SEM_SIZE)
        {
            Free(name);
            return ENOSPACE;
        }
    }

    //Create
    semlist[i].SID = i;
    semlist[i].count = ival;
    semlist[i].num_users = 1;

    semlist[i].name = name;
    Clear_Thread_Queue(&semlist[i].waitQueue);

    return i;
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

    if (SID < 0 || SID >= MAX_SEM_SIZE || semlist[SID].name == 0)
        return EINVALID;

    bool iflag = Begin_Int_Atomic();

    semlist[SID].count -= 1;

    if (semlist[SID].count < 0)
    {
        Wait(&semlist[SID].waitQueue);
    }

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

    if (SID < 0 || SID >= MAX_SEM_SIZE || semlist[SID].name == 0)
        return EINVALID;

    bool iflag = Begin_Int_Atomic();

    semlist[SID].count += 1;

    if (!Is_Thread_Queue_Empty(&semlist[SID].waitQueue))
    {
        Wake_Up_One(&semlist[SID].waitQueue);
    }

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
    if (SID < 0 || SID >= MAX_SEM_SIZE || semlist[SID].name == 0)
        return -1;

    bool iflag = Begin_Int_Atomic();

    semlist[SID].num_users--;

    if (semlist[SID].num_users == 0)
    {
        Free(semlist[SID].name);
        semlist[SID].name = 0;
    }

    End_Int_Atomic(iflag);

    return 0;
}