# First: Rename "main-perf" function to "main" in sim.c
gcc -g -Isrc -fopenmp -c -g src/sim.c -o perf.o
gcc -fopenmp perf.o -o perf
