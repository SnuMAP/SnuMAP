CC=gcc
CFLAGS=-Wall -shared -fPIC

all:
	$(CC) $(CFLAGS) -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux libSnuMap.c -o libSnuMap.so

clean:
	rm libSnuMap.so
	
