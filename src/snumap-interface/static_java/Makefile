CC=gcc
CFLAGS=-Wall -shared -fPIC

all:
	mkdir -p $(SNUMAP_ROOT)/lib
	$(CC) $(CFLAGS) -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux snumap_comm_java.c -o $(SNUMAP_ROOT)/lib/libSnuMapJava.so
	mkdir -p $(SNUMAP_ROOT)/inc
	cp snumap_comm_java.h $(SNUMAP_ROOT)/inc/snumap_comm_java.h


clean:
	rm -rf *.o
	rm -rf $(SNUMAP_ROOT)/lib/libSnuMapJava.so
	rm -rf $(SNUMAP_ROOT)/inc/snumap_comm_java.h
