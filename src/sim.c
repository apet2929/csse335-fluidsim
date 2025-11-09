
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include <stdio.h>
#include <omp.h>
#include <time.h>

const int BLOCK_SIZE = 64;

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
    #pragma omp parallel for schedule(guided)
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
            state->currentFrame[gridIndex(col, row, state)] = amp;
            state->lastFrame[gridIndex(col, row, state)] = amp;
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

    int num_blocks = nx / BLOCK_SIZE + (nx % BLOCK_SIZE != 0 ? 1 : 0);
    
    State foo = {
        currentFrame, lastFrame, nextFrame, nx, ny, num_blocks, alpha2
    };
    droplet(&foo, 5, 5, 0.5, 0.5, 3);
    droplet(&foo, 5, 5, 0.1, 0.5, 10);
    
    return foo;
}


int blockedIndex(int bx, int by, int num_blocks, int row, int col) {
    return (BLOCK_SIZE * BLOCK_SIZE) * (by*num_blocks + bx) + // origin position of block
            (col + row * BLOCK_SIZE);                          // position within block
}

// x,y -> bi,bj,row,col
int gridIndex(int x, int y, State* state) {
    int bx = x / BLOCK_SIZE;
    int col = x % BLOCK_SIZE;
    int by = y / BLOCK_SIZE;
    int row = y % BLOCK_SIZE;
    return blockedIndex(bx, by, state->numBlocks, row, col);
}


int main(void)
{
    const int nx = 128;
    const int ny = 128;
    State state = initState(nx,ny,0.2,1,0.25);

    long num_steps = 5E3;
    
    int thread_counts[] = {1,2,4,6,8};
    double avg_times[5];
    /*
    Todo: get chunky
    BOOM BOOM BOOM BOOM BOOM
    Check larger grid sizes
    Check for correctness- record total state for 10 iters, check for consistency with that +/- some fp epsillon
    Plot (python)
    */

    
    srand (time (NULL)); // define a seed for the random number generator
    const char ALLOWED[] = "abcdefghijklmnopqrstuvwxyz1234567890";
    char random[10 + 1];
    int i = 0;
    int c = 0;
    int nbAllowed = sizeof(ALLOWED)-1;
    for(i=0;i<10;i++) {
        c = rand() % nbAllowed ;
        random[i] = ALLOWED[c];
    }
    random[10] = '\0';
    char ftiming[100] = { '\0' };
    char fspeedup[100] = {'\0'};
    strcat(ftiming, "timings/");
    strcat(fspeedup, "timings/");
    strcat(ftiming, random);
    strcat(fspeedup, random);
    strcat(ftiming, "-timing.csv");
    strcat(fspeedup, "-speedup.csv");
    
    FILE* timing = fopen(ftiming, "w");
    fprintf(timing, "num_threads,avg_time_per_tick\n");
    for(int i = 4; i >= 0; i--) {
        omp_set_num_threads(thread_counts[i]);
        double starttime = omp_get_wtime();
        for(long i = 0; i < num_steps; i++) {
            simulate_tick(&state); // parallel
        }
    
        double endtime = omp_get_wtime();
        double elapsed = endtime - starttime;
        double avg = elapsed / num_steps;
        fprintf(timing, "%d,%f\n", thread_counts[i], avg);
        avg_times[i] = avg;
    }
    fclose(timing);

    FILE* speedup = fopen(fspeedup, "w");
    fprintf(speedup, "num_threads,speedup\n");
    for(int i = 1; i < 5; i++) {
        double speedu = avg_times[0] / avg_times[i];
        fprintf(speedup, "%d,%f\n", thread_counts[i], speedu);
    }
    fclose(speedup);

    return 0;
}