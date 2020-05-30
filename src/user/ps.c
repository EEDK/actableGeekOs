/*
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
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

#include <conio.h>
#include <process.h>

char swichingAffinity(int affi){
	char ret;
	switch(affi){
		case -1 :
		    ret = 'A';
		    break;
		default:
		    ret = affi + '0';
		    break; 		
		}
	return ret;
}

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
	int count , length;
	char stat , core , affinity;
	
	struct Process_Info procInfo[50];

	length = PS(procInfo, 50) - 1;

	Print("PID PPID PRIO STAT AFF TIME COMMAND\n");
	for(count = 0 ; count < length; count++){	
		switch(procInfo[count].status){
		case 0 :
		    core = procInfo[count].currCore + '0';
		    stat = 'R';
		    break;
		case 1 :
		    core = ' ';
		    stat = 'B';
		    break;	
		default:
		    core = ' ';
		    stat = 'Z';
		    break; 		
		}
		affinity = swichingAffinity(procInfo[count].affinity);
		Print("%3d %4d %4d %2c%2c %3c %4d %s\n" ,
			procInfo[count].pid,
			procInfo[count].parent_pid,
			procInfo[count].priority,
			core,
			stat,
			affinity,
			procInfo[count].totalTime,
			procInfo[count].name
		);
	}
}


