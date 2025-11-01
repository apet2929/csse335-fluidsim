
#include <stdlib.h>
#include <string.h>
#include <sim.h>

double calculate_amplitude_at(int i, int j, State *state) {
    double above = state->currentFrame->grid[gridIndex(i, j-1, state)];
    double below = state->currentFrame->grid[gridIndex(i, j+1, state)];
    double left = state->currentFrame->grid[gridIndex(i-1, j, state)];
    double right = state->currentFrame->grid[gridIndex(i+1, j, state)];

    double current = state->currentFrame->grid[gridIndex(i,j,state)];
    double last = state->lastFrame->grid[gridIndex(i,j,state)];

    return state->alpha2 * (above + below + left + right - 4*current) + 2*current - last;
}

void simulate_tick(State *state) {
    for (int i = 1; i < state->nx - 1; i++) {
        for (int j = 1; j < state->ny - 1; j++) {
            int curIndex = gridIndex(i, j, state);
            state->nextFrame->grid[curIndex] = calculate_amplitude_at(i, j, state);
        }
    }

    state->lastFrame = state->currentFrame;
    state->currentFrame = state->nextFrame;
    state->nextFrame = state->lastFrame;
}

State initState(int nx, int ny, double c, double h, double dt) {
    double alpha = c * dt / h;
    double alpha2 = alpha * alpha;

    Grid* u0 = (Grid*) malloc(nx * ny * sizeof(double));
    Grid* u1 = (Grid*) malloc(nx * ny * sizeof(double));
    Grid* u2 = (Grid*) malloc(nx * ny * sizeof(double));

    memset(u0, 0, nx * ny * sizeof(double));
    memset(u1, 0, nx * ny * sizeof(double));
    memset(u2, 0, nx * ny * sizeof(double));

    State foo = {
        u0, u1, u2, nx, ny, alpha2
    };
    return foo;
}

int gridIndex(int x, int y, State *state) {
    return x + state->nx * y;
}