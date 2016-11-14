 #include <stdio.h>
 #include "SnuMap.h"

JNIEXPORT void JNICALL
Java_org_apache_hadoop_examples_WordCount_snumapInit(JNIEnv *env, jobject obj)
{
  printf("[SnuMap] Init Profiling Called!\n");
 
	char buf[4096];
	char* snumap_env = getenv("OMP_PROFILER_ROOT");
	int i = 0;

	//fprintf(stdout, "omp_init_profileing called\n");

	if (snumap_env == NULL) {
		perror("getenv(OMP_PROFILER_ROOT)");
		abort();
	}

	for (i = 0; i < 4096; i++) {
		buf[i] = 0;
	}

	strncat(buf, snumap_env, strlen(snumap_env));
	strncat(buf, "/", 1);
	strncat(buf, DEVICE_FILE_NAME, strlen(DEVICE_FILE_NAME));

	fd = open(buf, 0);
	if (fd < 0) {
		fprintf(stderr, "can't open the device file: %s\n", buf);
		exit(-1);
	}

	is_opened = 1;
	return; 
}

JNIEXPORT void JNICALL
Java_org_apache_hadoop_examples_WordCount_snumapStart(JNIEnv *env, jobject obj)
{
	printf("[SnuMap] Start Profiling Called!\n");
	if (is_opened) {
		if (ioctl(fd, IOCTL_START_PROFILING, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}

	return;
 }

 JNIEXPORT void JNICALL
Java_org_apache_hadoop_examples_WordCount_snumapStop(JNIEnv *env, jobject obj)
{
	printf("[SnuMap] Stop Profiling Called!\n");
	if (is_opened) {
		if (ioctl(fd, IOCTL_STOP_PROFILING, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}

	return;
 }

 JNIEXPORT void JNICALL
Java_org_apache_hadoop_examples_WordCount_snumapCleanup(JNIEnv *env, jobject obj)
{
	printf("[SnuMap] Cleanup Profiling Called!\n");
	if (is_opened) {
		close(fd);
	}

	return;
 }

JNIEXPORT void JNICALL
Java_org_apache_hadoop_examples_WordCount_snumapDumpResults(JNIEnv *env, jobject obj)
{
	printf("[SnuMap] Dump Results Called!\n");
	if (is_opened) {
		if (ioctl(fd, IOCTL_DUMP_PROFILED_RESULT, NULL) < 0) {
			fprintf(stderr, "ioctl error\n");
		}
	}

	return;
 }
