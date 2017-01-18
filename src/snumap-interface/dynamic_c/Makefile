all: SnuMAPComm

SnuMAPComm:
	mkdir -p $(SNUMAP_ROOT)/lib
	gcc -DRUNTIME -shared -fPIC -o $(SNUMAP_ROOT)/lib/libSnuMAPComm.so snumap_comm.c -ldl

clean:
	rm -rf $(SNUMAP_ROOT)/lib/libSnuMAPComm.so
