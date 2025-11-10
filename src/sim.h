#pragma once


typedef double* Grid;

// double**[double*->[]]
// row [1,2,3,4,5,56,]
struct State {
    Grid currentFrame;
    Grid lastFrame;
    Grid nextFrame;
    int nx, ny;
    int numBlocks;
    double alpha2;
};

typedef struct State State;

int gridIndex(int x, int y, State *state);

State initState(int nx, int ny, double c, double h, double dt);
double calculate_amplitude_at(int i, int j, State *state);
void simulate_tick(State *state);
int blockedIndex(int bx, int by, int num_blocks, int row, int col);