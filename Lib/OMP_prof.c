//------------------------------------------------------------------------------
/// @brief OpenMP profiler library implementation
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created
/// 2015/10 Heesik Shin apply IOCTL number macro.
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
	
	//fprintf(stdout, "omp_init_profileing called\n");

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
}

int omp_start_profiling(void)
{
	//fprintf(stdout, "omp_start_profiling called\n");
	if (is_opened) {
		if (ioctl(fd, IOCTL_START_PROFILING, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int omp_stop_profiling(void)
{
	//fprintf(stdout, "omp_stop_profiling called\n");
	if (is_opened) {
		if (ioctl(fd, IOCTL_STOP_PROFILING, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int omp_dump_profile_result(void)
{
	//fprintf(stdout, "omp_dump_profile_result called\n");
	if (is_opened) {
		if (ioctl(fd, IOCTL_DUMP_PROFILED_RESULT, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}


//		int i = 0, j = 0, k = 0;
//		struct taskprofile_data data;
//
//		if (ioctl(fd, 3, &data) < 0) {
//			fprintf(stderr, "ioctl error\n");
//		}
//
//
//		for (i=0; i<1; i++) {
//			fprintf(stdout, "thread: %d\n", i);
//
//			for (j = 0; j < 8; j++) {
//				fprintf(stdout, ">> cpu: %d resume_cnt: %d suspend_cnt: %d\n",
//						j, data.resume_cnt[j], data.suspend_cnt[j]);
//
//				for (k = 0; k < data.resume_cnt[j]; k++) {
//					fprintf(stdout, ">>>> cnt: %d resume_time: %lu suspend_time: %lu\n",
//							k, data.resume_time[j][k], data.suspend_time[j][k]);
//				}
//			}
//		}
	}
}

int omp_cleanup_profiling(void)
{
	//fprintf(stdout, "omp_cleanup_profiling called\n");
	if (is_opened) {
		close(fd);
	}
}
