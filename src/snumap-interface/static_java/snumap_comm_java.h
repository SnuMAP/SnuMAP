/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// ioctl definition
#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

int is_opened;
int fd;

#ifndef _Included_Task
#define _Included_Task
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_org_apache_hadoop_examples_WordCount_snumapInit 
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_org_apache_hadoop_examples_WordCount_snumapStart
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_org_apache_hadoop_examples_WordCount_snumapStop
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_org_apache_hadoop_examples_WordCount_snumapDumpResults
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_org_apache_hadoop_examples_WordCount_snumapCleanup
  (JNIEnv *, jobject);
#ifdef __cplusplus
}
#endif
#endif