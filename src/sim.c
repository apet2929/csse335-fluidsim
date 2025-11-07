
#include <stdlib.h>
#include <string.h>
#include <sim.h>
#include <stdio.h>

double calculate_amplitude_at(int i, int j, State *state) {
    double above = state->currentFrame[gridIndex(i, j-1, state)];
    double below = state->currentFrame[gridIndex(i, j+1, state)];
    double left = state->currentFrame[gridIndex(i-1, j, state)];
    double right = state->currentFrame[gridIndex(i+1, j, state)];

    double current = state->currentFrame[gridIndex(i,j,state)];
    double last = state->lastFrame[gridIndex(i,j,state)];

    return state->alpha2 * (above + below + left + right - 4*current) + 2*current - last;
}

void simulate_tick(State *state) {
    for (int i = 1; i < state->nx - 1; i++) {
        for (int j = 1; j < state->ny - 1; j++) {
            int curIndex = gridIndex(i, j, state);
            state->nextFrame[curIndex] = calculate_amplitude_at(i, j, state);
        }
    }

    state->lastFrame = state->currentFrame;
    state->currentFrame = state->nextFrame;
    state->nextFrame = state->lastFrame;
}

State initState(int nx, int ny, double c, double h, double dt) {
    double alpha = c * dt / h;
    double alpha2 = alpha * alpha;

    Grid currentFrame = (Grid) malloc(nx * ny * sizeof(double));
    Grid lastFrame = (Grid) malloc(nx * ny * sizeof(double));
    Grid nextFrame = (Grid) malloc(nx * ny * sizeof(double));

    memset(currentFrame, 0, nx * ny * sizeof(double));
    memset(lastFrame, 0, nx * ny * sizeof(double));
    memset(nextFrame, 0, nx * ny * sizeof(double));

    for(int row = nx * 0.4; row < nx * 0.6; row++) {
        printf("%d ", row);
        for(int col = ny * 0.4; col < ny * 0.6; col++) {
            currentFrame[col + row * nx] = 3;
            lastFrame[col + row * nx] = 3;
        }   
    }

    State foo = {
        currentFrame, lastFrame, nextFrame, nx, ny, alpha2
    };
    return foo;
}

int gridIndex(int x, int y, State *state) {
    return x + state->nx * y;
}