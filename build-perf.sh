# First: Rename "main-perf" function to "main" in sim.c
gcc -g -DPERF -Isrc -fopenmp -c -g src/sim.c -o perf.o
gcc -fopenmp -DPERF  perf.o -o perf
