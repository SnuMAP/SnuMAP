#include "snumap_comm.h"

int init_profiling(void)
{
	char buf[4096];
	char* env = getenv("SNUMAP_ROOT");
	int i = 0;

	if (env == NULL) {
		perror("getenv(SNUMAP_ROOT)");
		abort();
	}

	for (i = 0; i < 4096; i++) {
		buf[i] = 0;
	}

	strncat(buf, env, strlen(env));
	strncat(buf, "/", 1);
	strncat(buf, DEVICE_FILE_NAME, strlen(DEVICE_FILE_NAME));

	fd = open(buf, 0);
	if (fd < 0) {
		fprintf(stderr, "can't open the device file: %s\n", buf);
		exit(-1);
	}

	is_opened = 1;
}

int start_profiling(void)
{
  if (is_opened) {
	while (1) {
		if (ioctl(fd, IOCTL_START_PROFILING, NULL) < 0) {
			if (errno == EBUSY) { // Try again
				usleep(1000);
				continue;
			}
			fprintf(stderr, "errno : %d\n", errno);
			fprintf(stderr, "ioctl error\n");
		}

		is_started = 1;
		break;
	}
  }
}

int stop_profiling(void)
{
  if (is_opened) {
	while (1) {
		if (ioctl(fd, IOCTL_STOP_PROFILING, NULL) < 0) {
			if (errno == EBUSY) { // Try again
				usleep(1000);
				continue;
			}
			fprintf(stderr, "errno : %d\n", errno);
			fprintf(stderr, "ioctl error\n");
		}

		is_started = 0;
		break;
	}
  }
}

int dump_profile_result(void)
{
	int cnt = 0;
	if (is_opened) {
		while (1) {
			if (ioctl(fd, IOCTL_DUMP_PROFILED_RESULT, NULL) < 0) {
				if (errno == EBUSY) { // Try again
					if (cnt == 0) usleep(100000);
					else if (cnt == 1) usleep(250000);
					else if (cnt == 2) usleep(500000);
					else sleep(1);
					cnt++;
					continue;
				}
				fprintf(stderr, "errno : %d\n", errno);
				fprintf(stderr, "ioctl error\n");
			}
			
			break;
		}
	}
}

int cleanup_profiling(void)
{
	if (is_opened) {
		close(fd);
	}
}

int read_jiffies(volatile unsigned long* jiffies)
{
  if (is_opened) {
    if (ioctl(fd, IOCTL_GET_JIFFIES, jiffies)) {
      fprintf(stderr, "ioctl error\n");
    }
  }
}

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

  init_profiling();
  start_profiling();

  char buf__[4096] = { 0, };
  char* env = getenv("PWD");
  strncat(buf__, env, strlen(env));
  strncat(buf__, "/", 1);
  strncat(buf__, ubp_av[0], strlen(ubp_av[0]));
  strncat(buf__, ".call_info.csv", 14);

  call_func_id = 0;
  call_info = fopen(buf__, "w");
  if (is_started) {
    unsigned long jiffies;
    read_jiffies(&jiffies);

    fprintf(call_info, "%10d\t%10x\t%10lu\t%10lu\t",
        call_func_id++, 0, 0, jiffies);
  }

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

  if (is_started) {
    unsigned long jiffies;
    read_jiffies(&jiffies);

    fprintf(call_info, "%10lu\n", jiffies);
  }

  stop_profiling();
  dump_profile_result();
  cleanup_profiling();

  if (!exit_p) {
    exit_p = dlsym(RTLD_NEXT, "exit");
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  exit_p(status);
}

void GOMP_parallel_start(void (*fn) (void *), void *data, unsigned num_threads)
{
  static void (*GOMP_parallel_start_p) (void (*fn) (void *),
      void *data, unsigned num_threads);
  char* error;
  void* ptr;

  if (is_started) {
    unsigned long jiffies;
    read_jiffies(&jiffies);

    fprintf(call_info, "%10lu\n", jiffies);
    fprintf(call_info, "%10d\t%10x\t%10lu\t%10lu\t",
        call_func_id++, fn, (unsigned long)fn, jiffies);
  }

  if (!GOMP_parallel_start_p) {
    GOMP_parallel_start_p = dlsym(RTLD_NEXT, "GOMP_parallel_start");
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  GOMP_parallel_start_p(fn, data, num_threads);
}

void GOMP_parallel_end(void)
{
  static void (*GOMP_parallel_end_p) (void);
  char* error;
  void* ptr;

  if (is_started) {
    unsigned long jiffies;
    read_jiffies(&jiffies);

    fprintf(call_info, "%10lu\n", jiffies);
    fprintf(call_info, "%10d\t%10x\t%10lu\t%10lu\t",
        0, 0, 0, jiffies);
  }

  if (!GOMP_parallel_end_p) {
    GOMP_parallel_end_p = dlsym(RTLD_NEXT, "GOMP_parallel_end");
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  GOMP_parallel_end_p();
}

void GOMP_parallel(void (*fn) (void *), void *data, unsigned num_threads, unsigned int flags)
{
    static void (*GOMP_parallel_p) (void (*fn) (void *),
            void *data, unsigned num_threads, unsigned int flags);
    char* error;
    void* ptr;

    if (is_started) {
        unsigned long jiffies;
        read_jiffies(&jiffies);

        fprintf(call_info, "%10lu\n", jiffies);
        fprintf(call_info, "%10d\t%10x\t%10lu\t%10lu\t",
                call_func_id++, fn, (unsigned long)fn, jiffies);
    }

    if (!GOMP_parallel_p) {
        GOMP_parallel_p = dlsym(RTLD_NEXT, "GOMP_parallel");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(1);
        }
    }

    GOMP_parallel_p(fn, data, num_threads, flags);
}
