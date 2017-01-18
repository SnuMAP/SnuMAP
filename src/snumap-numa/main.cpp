//------------------------------------------------------------------------------
/// SnuMAP NUMA entry point
///

#include <thread>
#include <numa.h>
#include <iostream>
#include <fstream>
#include "profiler.h"
#include "comm.h"

static int num_nodes;
static int num_cores;
static int cores_in_node;

static int is_file_dump;

std::ofstream out;

void ProfilingRoutine(CProfiler* profiler, bool* profiler_running)
{
  unsigned long* l2_accesses = (unsigned long*)malloc(num_cores * sizeof(unsigned long));
  unsigned long* l2_misses = (unsigned long*)malloc(num_cores * sizeof(unsigned long));
  unsigned long* l3_accesses = (unsigned long*)malloc(num_nodes * sizeof(unsigned long));
  unsigned long* l3_misses = (unsigned long*)malloc(num_nodes * sizeof(unsigned long));
  unsigned long* local_accesses = (unsigned long*)malloc(num_nodes * sizeof(unsigned long));
  unsigned long* remote_accesses = (unsigned long*)malloc(num_nodes * sizeof(unsigned long));

  unsigned long total_local_accesses = 0;
  unsigned long total_remote_accesses = 0;

  const int option = 1;

  for (int core = 0; core < num_cores; core++) {
    l2_accesses[core] = 0;
    l2_misses[core] = 0;
  }

  for (int node = 0; node < num_nodes; node++) {
    l3_accesses[node] = 0;
    l3_misses[node] = 0;
    local_accesses[node] = 0;
    remote_accesses[node] = 0;
  }

  std::cout << "profiler start" << std::endl;
  profiler->InitPerfEventAttr();
  while (*profiler_running) {
    profiler->StartProfiling();
    /* periodically try to perform system-wide profiling */
    usleep(10000); // 10ms - current default sampling duration

    /* read jiffies */
    volatile unsigned long jiffies;
    read_jiffies(&jiffies);

    /* stop the profiling: update profiling results */
    //printf("jiffies: %lu\n", jiffies);
    if (profiler->StopProfiling(l2_accesses,
        l2_misses,
        l3_accesses,
        l3_misses,
        local_accesses,
        remote_accesses)) {
      for (int node = 0; node < num_nodes; node++) {
        total_local_accesses += local_accesses[node];
        total_remote_accesses += remote_accesses[node];
      }

      if (is_file_dump) {
        if (option == 1) {
          for (int node = 0; node < num_nodes; node++) {
            for (int core = node * cores_in_node; core < (node+1) * cores_in_node; core++) {
              out << core << "\t" << node << "\t" << jiffies
                << "\t" << l2_misses[core]
                << "\t" << l3_misses[node]
                << "\t" << local_accesses[node]
                << "\t" << remote_accesses[node]
                << "\t" << local_accesses[node] + remote_accesses[node]
                << "\t" << (double)local_accesses[node] / (double)(local_accesses[node] + remote_accesses[node])
                << std::endl;
            }
          }
        } else if (option == 0) {
          out << jiffies
            << "\t" << total_local_accesses
            << "\t" << total_remote_accesses
            << "\t" << total_local_accesses + total_remote_accesses
            << "\t" << (double)total_local_accesses / (double)(total_local_accesses + total_remote_accesses)
            << std::endl;
        }
      } else {
        fprintf(stdout, "total_local_accesses: %lu total_remote_accesses: %lu (%lf) total_accesses: %lu (%lf)\n\n",
            total_local_accesses, total_remote_accesses,
            (double)total_local_accesses/(double)total_remote_accesses,
            total_local_accesses + total_remote_accesses,
            (double)total_local_accesses / (double)(total_local_accesses + total_remote_accesses));
      }

      for (int core = 0; core < num_cores; core++) {
        l2_accesses[core] = 0;
        l2_misses[core] = 0;
      }

      for (int node = 0; node < num_nodes; node++) {
        l3_accesses[node] = 0;
        l3_misses[node] = 0;
        local_accesses[node] = 0;
        remote_accesses[node] = 0;
      }

      total_local_accesses = 0;
      total_remote_accesses = 0;
    }
  }

  profiler->InitPerfEventAttr();
  std::cout << "profiler end" << std::endl;

  profiler->DumpProfilingResults();
  return;
}

int main(int argc, char* argv[])
{
  num_nodes = numa_num_task_nodes();
  num_cores = numa_num_task_cpus();
  cores_in_node = num_cores / num_nodes;

  std::cout << "num_nodes: " << num_nodes
    << " num_cores: " << num_cores
    << " cores_in_node: " << cores_in_node << std::endl;

  CProfiler* profiler =
    new CSimpleNUMAProfiler(num_nodes, num_cores, cores_in_node);
    //new CNUMAProfiler(num_nodes, num_cores, cores_in_node);

  if (!argv[1]) {
    char buf_[4096] = { 0, };
    char command_[4096] = { 0, };
    char* env = getenv("SNUMAP_ROOT");
    strncat(buf_, env, strlen(env));
    strncat(buf_, "/log/numa.csv", 13);
    strncat(command_, "mkdir -p ", 9);
    strncat(command_, env, strlen(env));
    strncat(command_, "/log", 4);

    out.open(buf_);
    is_file_dump = 1;
    memset(buf_, 0x00, 4096);

    system(command_);

    init_profiling();

    bool profiler_running = true;
    std::thread profiler_thread(ProfilingRoutine, profiler, &profiler_running);

    while (true) { usleep(1000000); }

    cleanup_profiling();
    delete profiler;

  } else if (argv[1]) {
    char buf_[4096];
    strncat(buf_, argv[1], strlen(argv[1]));
    strncat(buf_, ".numa.csv", 9);
    strncat(buf_, "\0", 1);
    out.open(buf_);
    is_file_dump = 1;
    memset(buf_, 0x00, 4096);

    bool profiler_running = true;
    std::thread profiler_thread(ProfilingRoutine, profiler, &profiler_running);

    init_profiling();
    //start_profiling();

    char buf[4096];
    memset(buf, 0x00, 4096);
    strncat(buf, "LD_PRELOAD=", 13);
    char* env = getenv("SNUMAP_ROOT");
    strncat(buf, env, strlen(env));
    char* lib_path = "/lib/libSnuMAPComm.so ";
    strncat(buf, lib_path, strlen(lib_path));
    for (int i = 1; i < argc; i++) {
      strcat(buf, argv[i]);
      strcat(buf, " ");
    }
    strcat(buf, "\0");

    std::cout << "buf: " << buf << std::endl;
    system(buf);

    profiler_running = false;
    profiler_thread.join();

    //stop_profiling();
    //dump_profile_result();
    cleanup_profiling();

    delete profiler;
  }

  return 0;
}
