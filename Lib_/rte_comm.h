#ifndef __RTE_COMM_H_
#define __RTE_COMM_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <dlfcn.h>
#include "comm_interface.h"

#define DEBUG_MODE    ///< debug mode shows debug messages

/* main initialization routine interpositioning */
int __libc_start_main(int (*main) (int, char **, char **),
    int argc,
    char ** ubp_av,
    void (*init) (void),
    void (*fini) (void),
    void (*rtld_fini) (void),
    void (*stack_end));

/* exit call interpositioning */
void exit(int status);

#endif // __RTE_COMM_H_

