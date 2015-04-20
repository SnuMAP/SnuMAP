gcc -c -fpic OMP_prof.c
gcc -shared -o libOMPProf.so OMP_prof.o
