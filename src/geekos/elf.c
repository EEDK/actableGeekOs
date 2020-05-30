/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.31 $
 *
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h>      /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/fileio.h>
#include <geekos/elf.h>

#include <geekos/paging.h>

int elfDebug = 0;

/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */
/*int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
                         struct Exe_Format *exeFormat) {
	TODO("Parse an ELF executable image");
}*/

int Parse_ELF_Executable(char* exeFileData, ulong_t exeFileLength,
    struct Exe_Format* exeFormat) {

    elfHeader* ehdr;
    ehdr = (elfHeader*)exeFileData;

    programHeader* phdr;
    phdr = (programHeader*)(exeFileData + ehdr->phoff);



    if (exeFileData == 0) {
        //Print("exeFileData == 0\n");
        return ENOTFOUND;
    }

    if (ehdr->ident[0] != 0x7F || ehdr->ident[1] != 'E' || ehdr->ident[2] != 'L' || ehdr->ident[3] != 'F') {
        Print("ehdr -> ident is not ELF");
        return -1;
    }

    exeFormat->numSegments = EXE_MAX_SEGMENTS;
    exeFormat->entryAddr = ehdr->entry;

    struct Exe_Segment* segment;

    for (int i = 0; i < EXE_MAX_SEGMENTS; i++) {
        segment = &exeFormat->segmentList[i];
        segment->offsetInFile = phdr->offset;
        segment->lengthInFile = phdr->fileSize;
        segment->startAddress = phdr->vaddr;
        segment->sizeInMemory = phdr->memSize;
        segment->protFlags = phdr->flags;

        phdr = (programHeader*)((char*)phdr + ehdr->phentsize);
    }


    return 0;
    //TODO("Parse an ELF executable image");
}
