all: snumap-main

snumap-main:
	mkdir -p $(SNUMAP_ROOT)/bin/
	g++ -o $(SNUMAP_ROOT)/bin/$@ main.cpp

clean:
	rm -rf $(OBJ_DIR)/*.o $(SNUMAP_ROOT)/bin/snumap-main
