#include "comm.h"

static int is_opened;
static int fd;

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
		if (ioctl(fd, IOCTL_START_PROFILING, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int stop_profiling(void)
{
	if (is_opened) {
		if (ioctl(fd, IOCTL_STOP_PROFILING, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}
}

int dump_profile_result(void)
{
	if (is_opened) {
		if (ioctl(fd, IOCTL_DUMP_PROFILED_RESULT, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
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

