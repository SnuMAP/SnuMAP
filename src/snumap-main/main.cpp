//------------------------------------------------------------------------------
/// SnuMAP main entry point
///
//------------------------------------------------------------------------------

#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
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

  return 0;
}
