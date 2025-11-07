
#include <stdlib.h>
#include <string.h>
#include "sim.h"
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

void droplet(State* state, int width, int height, double posx, double posy, double amp) { // posx, posy center position of drop in (0, 1) as % width, % height
    int centerx = posx * state->nx;
    int centery = posy * state->ny;

    for(int row = centerx - width/2; row < state->nx && row <centerx + width/2; row++) {
        for(int col = centery - height/2; col < state->ny && col < centery + height/2; col++) {
            state->currentFrame[col + row * state->nx] = amp;
            state->lastFrame[col + row * state->nx] = amp;
        }   
    }
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

    
    State foo = {
        currentFrame, lastFrame, nextFrame, nx, ny, alpha2
    };
    droplet(&foo, 5, 5, 0.5, 0.5, 3);
    droplet(&foo, 5, 5, 0.1, 0.5, 10);
    
    return foo;
}

int gridIndex(int x, int y, State *state) {
    return x + state->nx * y;
}

int main_perf(void)
{
    const int nx = 128;
    const int ny = 128;
    State state = initState(nx,ny,0.2,1,0.25);

    // while(1) {
    //     simulate_tick(&state);
    // }
    printf("Hello world!\n");
    // return 0;
    return 0;
}