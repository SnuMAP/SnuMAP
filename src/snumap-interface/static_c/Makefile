all: SnuMAP-static

SnuMAP-static:
	mkdir -p $(SNUMAP_ROOT)/lib
	gcc -c -fPIC snumap_comm_c.c
	gcc -shared -o $(SNUMAP_ROOT)/lib/libSnuMAP.so snumap_comm_c.o
	mkdir -p $(SNUMAP_ROOT)/inc
	cp snumap_comm_c.h $(SNUMAP_ROOT)/inc/snumap_comm_c.h

clean:
	rm -rf *.o
	rm -rf $(SNUMAP_ROOT)/lib/libSnuMAP.so
	rm -rf $(SNUMAP_ROOT)/inc/snumap_comm_c.h
