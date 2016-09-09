//------------------------------------------------------------------------------
/// @brief Dynamic RTE communication library
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/02 Younghyun Cho created
/// 2015/04 Younghyun Cho Dynamic RTE communication library
/// 2015/05 Younghyun Cho communication points interpositioning
///
/// @section license_section Licence
/// Copyright (c) 2015, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription rte_comm.c
/// Dynamic RTE communication library implementation
/// 
//------------------------------------------------------------------------------

#include "rte_comm.h"

int __libc_start_main(int (*main) (int, char **, char **),
    int argc,
    char ** ubp_av,
    void (*init) (void),
    void (*fini) (void),
    void (*rtld_fini) (void),
    void (*stack_end))
{
  static void (*main_p) (int (*main) (int, char **, char **),
      int argc,
      char ** ubp_av,
      void (*init) (void),
      void (*fini) (void),
      void (*rtld_fini) (void),
      void (*stack_end));
  char* error;

  if (!main_p) {
    main_p = dlsym(RTLD_NEXT, "__libc_start_main");
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  main_p(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

void exit(int status)
{
  static void (*exit_p) (int status);
  char* error;

  if (RTE_FinishApplication()) {
    fprintf(stderr, "RTE FinishApplication error\n");
  }

  if (!exit_p) {
    exit_p = dlsym(RTLD_NEXT, "exit");
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  exit_p(status);
}

