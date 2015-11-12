CC=g++
CCFLAGS=-std=c++0x -g -O3 -pthread

PROFILER_SRC=.
OBJ_DIR=.

PROFILER_DEPS=common.h \
				 event.h \
				 pmu_interface.h \
				 profiler.h
PROFILER=profiler.cpp \
				 pmu_interface.cpp

PROFILER_DEPS_=$(patsubst %,$(PROFILER_SRC)/%,$(PROFILER_DEPS))
OBJ_PROFILER=$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(PROFILER))

.PHONY: clean doc

$(OBJ_DIR)/%.o: $(PROFILER_SRC)/%.cpp $(PROFILER_DEPS_)
	$(CC) $(CCFLAGS) -c -o $@ $<

all: PMU_monitor

PMU_monitor: $(OBJ_DIR)/monitor.o $(OBJ_PROFILER)
	$(CC) $(CCFLAGS) -o $@ $(OBJ_DIR)/monitor.o $(OBJ_PROFILER)

clean:
	rm -rf $(OBJ_DIR)/*.o PMU_monitor
