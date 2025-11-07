# First: Rename "main-perf" function to "main" in sim.c
gcc -Isrc -fopenmp -o sim src/sim.c