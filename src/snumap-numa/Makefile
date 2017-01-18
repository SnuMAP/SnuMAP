CC=g++
CCFLAGS=-std=c++0x -g -O3 -pthread

PROFILER_SRC=.
OBJ_DIR=.

PROFILER_DEPS=numa_event.h \
				 pmu_interface.h \
				 profiler.h \
				 comm.h
PROFILER=profiler.cpp \
				 pmu_interface.cpp \
				 comm.cpp

PROFILER_DEPS_=$(patsubst %,$(PROFILER_SRC)/%,$(PROFILER_DEPS))
OBJ_PROFILER=$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(PROFILER))

.PHONY: clean doc

$(OBJ_DIR)/%.o: $(PROFILER_SRC)/%.cpp $(PROFILER_DEPS_)
	$(CC) $(CCFLAGS) -c -o $@ $<

all: snumap-numa

snumap-numa: $(OBJ_DIR)/main.o $(OBJ_PROFILER)
	mkdir -p $(SNUMAP_ROOT)/bin/
	$(CC) $(CCFLAGS) -o $(SNUMAP_ROOT)/bin/$@ $(OBJ_DIR)/main.o $(OBJ_PROFILER) -lnuma

clean:
	rm -rf $(OBJ_DIR)/*.o $(SNUMAP_ROOT)/bin/snumap-numa
