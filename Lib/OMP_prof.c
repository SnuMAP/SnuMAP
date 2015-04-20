//------------------------------------------------------------------------------
/// @brief OpenMP profiler library implementation
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2015, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription OMP_prof.c
/// 
//------------------------------------------------------------------------------

#include "OMP_prof.h"

int omp_init_profiling(void)
{
	char buf[4096];
	char* env = getenv("OMP_PROFILER_ROOT");
	int i = 0;

	if (env == NULL) {
		perror("getenv(OMP_PROFILER_ROOT)");
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

	/* must initiate worker threads before setting start_profiling flag */

}

int omp_start_profiling(void)
{
	if (is_opened) {
		if (ioctl(fd, IOCTL_COMMAND_1, NULL) <= 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int omp_stop_profiling(void)
{
	if (is_opened) {
		if (ioctl(fd, IOCTL_COMMAND_2, NULL) <= 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int omp_dump_proflie_result(void)
{
	if (is_opened) {
		if (ioctl(fd, IOCTL_COMMAND_3, NULL) <= 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int omp_cleanup_profiling(void)
{
	if (is_opened) {
		close(fd);
	}
}
