#pragma once

struct Grid{
    double* grid;
};

struct State {
    struct Grid* currentFrame;
    struct Grid* lastFrame;
    struct Grid* nextFrame;
    int nx, ny;
    double alpha2;
};

typedef struct Grid Grid;
typedef struct State State;

int gridIndex(int x, int y, State *state);

State initState(int nx, int ny, double c, double h, double dt);
double calculate_amplitude_at(int i, int j, State *state);
void simulate_tick(State *state);