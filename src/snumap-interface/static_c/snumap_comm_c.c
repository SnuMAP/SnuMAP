//------------------------------------------------------------------------------
/// SnuMAP OpenMP (C/C++ applications) profiler library implementation
/// 

#include "snumap_comm_c.h"

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
	}
}

int omp_cleanup_profiling(void)
{
	//fprintf(stdout, "omp_cleanup_profiling called\n");
	if (is_opened) {
		close(fd);
	}
}
