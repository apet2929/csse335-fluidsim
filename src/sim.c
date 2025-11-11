
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include <stdio.h>
#include <omp.h>
#include <time.h>
#define PERF
const int BLOCK_SIZE = 16;



int left_neighbor_index(int blockOrigin, int row) {
    int leftBlockOrigin = blockOrigin - (BLOCK_SIZE * BLOCK_SIZE);
    return leftBlockOrigin + (BLOCK_SIZE-1) + (BLOCK_SIZE*row); // + (BLOCK_SIZE-1) moves to the right edge of the block
}

int right_neighbor_index(int blockOrigin, int row) {
    int rightBlockOrigin = blockOrigin + (BLOCK_SIZE * BLOCK_SIZE);
    return rightBlockOrigin + (BLOCK_SIZE*row);
}

int top_neighbor_index(State* state, int blockOrigin, int col) {
    int topBlockOrigin = blockOrigin - (BLOCK_SIZE * BLOCK_SIZE * state->numBlocks);
    return topBlockOrigin + (BLOCK_SIZE * (BLOCK_SIZE-1)) + col;
}

int bottom_neighbor_index(State* state, int blockOrigin, int col) {
    int bottomBlockOrigin = blockOrigin + (BLOCK_SIZE * BLOCK_SIZE * state->numBlocks);
    return bottomBlockOrigin + col;
}

int getBlockOrigin(State* state, int bx, int by) {
    return (BLOCK_SIZE * BLOCK_SIZE) * ((by*state->numBlocks) + bx);
}

double update_amplitude_given_indices(State *state, int index, int above_index, int left_index, int right_index, int below_index) {
    double above = state->currentFrame[above_index];
    double below = state->currentFrame[below_index];
    double left = state->currentFrame[left_index];
    double right = state->currentFrame[right_index];

    double current = state->currentFrame[index];
    double last = state->lastFrame[index];

    state->nextFrame[index] = state->alpha2 * (above + below + left + right - 4*current) + 2*current - last;
}


void update_left_edge(State* state, int bx, int by) {
    if(bx == 0) return;
    int blockOrigin = (BLOCK_SIZE * BLOCK_SIZE) * ((by*state->numBlocks) + bx);
    int index = blockOrigin + BLOCK_SIZE;
    int leftNeighbor = left_neighbor_index(blockOrigin, 1);

    for(int row = 1; row < BLOCK_SIZE-1; row++ ) {
        int above_index = index - BLOCK_SIZE;
        int left_index = leftNeighbor;
        int right_index = index + 1;
        int below_index = index + BLOCK_SIZE;
        update_amplitude_given_indices(state, index, above_index, left_index, right_index, below_index);
        index += BLOCK_SIZE;
        leftNeighbor += BLOCK_SIZE;
    }
}

void update_right_edge(State* state, int bx, int by) {
    if(bx == state->numBlocks) return;
    int blockOrigin = (BLOCK_SIZE * BLOCK_SIZE) * ((by*state->numBlocks) + bx);
    int index = blockOrigin + BLOCK_SIZE;
    int rightNeighbor = right_neighbor_index(blockOrigin, 1);

    for(int row = 1; row < BLOCK_SIZE-1; row++ ) {
        int above_index = index - BLOCK_SIZE;
        int right_index = rightNeighbor;
        int left_index = index - 1;
        int below_index = index + BLOCK_SIZE;
        update_amplitude_given_indices(state, index, above_index, left_index, right_index, below_index);
        index += BLOCK_SIZE;
        rightNeighbor += BLOCK_SIZE;
    }
}

void update_top_edge(State* state, int bx, int by) {
    if(bx == state->numBlocks) return;
    int blockOrigin = (BLOCK_SIZE * BLOCK_SIZE) * ((by*state->numBlocks) + bx);
    int index = blockOrigin + 1;
    int upperNeighbor = top_neighbor_index(state, blockOrigin, 1);

    for(int col = 1; col < BLOCK_SIZE-1; col++ ) {
        int above_index = upperNeighbor;
        int right_index = index + 1;
        int left_index = index - 1;
        int below_index = index + BLOCK_SIZE;
        update_amplitude_given_indices(state, index, above_index, left_index, right_index, below_index);
        index += 1;
        upperNeighbor += 1;
    }
}

void update_bottom_edge(State* state, int bx, int by) {
    if(bx == state->numBlocks) return;
    int blockOrigin = (BLOCK_SIZE * BLOCK_SIZE) * ((by*state->numBlocks) + bx);
    int index = blockOrigin + 1;
    int lowerNeighbor = bottom_neighbor_index(state, blockOrigin, 1);

    for(int col = 1; col < BLOCK_SIZE-1; col++ ) {
        int above_index = index - BLOCK_SIZE;
        int right_index = index + 1;
        int left_index = index - 1;
        int below_index = lowerNeighbor;
        update_amplitude_given_indices(state, index, above_index, left_index, right_index, below_index);
        index += 1;
        lowerNeighbor += 1;
    }
}


void update_corners(State* state, int bx, int by) {
    int blockOrigin = (BLOCK_SIZE * BLOCK_SIZE) * ((by*state->numBlocks) + bx);
    
    int left_neighbor = left_neighbor_index(blockOrigin, 0);
    int above_neighbor = top_neighbor_index(state, blockOrigin, 0); 
    int below_neighbor = bottom_neighbor_index(state, blockOrigin, 0); 
    int right_neighbor = right_neighbor_index(blockOrigin, 0);
    int lower_left_neighbor = left_neighbor + (BLOCK_SIZE * (BLOCK_SIZE-1)); // move down to bottom row
    int lower_right_neighbor = right_neighbor + (BLOCK_SIZE * (BLOCK_SIZE-1));
    int righter_above_neighbor = above_neighbor + (BLOCK_SIZE-1); // move to rightmost column
    int righter_below_neighbor = below_neighbor + (BLOCK_SIZE-1);
    // top left
    if(bx != 0 && by != 0) {
        int index = blockOrigin;
        int right_index = index + 1;
        int below_index = index + BLOCK_SIZE;
        update_amplitude_given_indices(state, index, above_neighbor, left_neighbor, right_index, below_index);
    }

    // bottom left
    if(bx != 0 && by != state->numBlocks) {
        int col = 0;
        int row = BLOCK_SIZE-1;

        int index = blockOrigin + BLOCK_SIZE;
        int right_index = index + 1;
        int above_index = index - BLOCK_SIZE;
        
        update_amplitude_given_indices(state, index, above_index, lower_left_neighbor, right_index, below_neighbor);
    }

    
    // bottom right
    if(bx != state->numBlocks && by != state->numBlocks) {
        int col = 0;
        int row = BLOCK_SIZE-1;

        int index = blockOrigin + BLOCK_SIZE;
        int rightBlockOrigin = blockOrigin + (BLOCK_SIZE * BLOCK_SIZE);
        int right_neighbor = rightBlockOrigin + (BLOCK_SIZE*row);
        int above_index = index - BLOCK_SIZE;
        int left_index = index - 1;
        int below_neighbor = blockedIndex(bx, by+1, state->numBlocks, BLOCK_SIZE-1, col); // TODO: Steal from update bottom edge

        update_amplitude_given_indices(state, index, above_index, left_index, lower_right_neighbor, righter_below_neighbor);
    }

    // top right
    if(bx != state->numBlocks && by != 0) {
        int col = 0;
        int row = 0;

        int index = blockOrigin + BLOCK_SIZE;
        int rightBlockOrigin = blockOrigin + (BLOCK_SIZE * BLOCK_SIZE);
        int right_neighbor = rightBlockOrigin + BLOCK_SIZE-1 + BLOCK_SIZE;
        int below_index = index + BLOCK_SIZE;
        int left_index = index-1;
        int above_neighbor = blockedIndex(bx, by-1, state->numBlocks, BLOCK_SIZE-1, col); // TODO: Steal from update top edge

        update_amplitude_given_indices(state, index, righter_above_neighbor, left_index, right_neighbor, below_index);
    }
}


double update_amplitude_at_complex(State *state, int bx, int by, int col, int row) {
    // edges of the grid
    if((bx == 0 && col == 0) || (bx == state->numBlocks-1 && col == BLOCK_SIZE-1)) return 0.0;
    if((by == 0 && row == 0) || (by == state->numBlocks-1 && row == BLOCK_SIZE-1)) return 0.0;
    
    int index = blockedIndex(bx, by, state->numBlocks, row, col);

    int above_index;
    if(row == 0) above_index = blockedIndex(bx, by-1, state->numBlocks, BLOCK_SIZE-1, col);
    else above_index = index - BLOCK_SIZE;
    
    int left_index;
    if(col == 0) left_index = blockedIndex(bx-1, by, state->numBlocks, row, BLOCK_SIZE-1);
    else left_index = index - 1;
    
    int right_index;
    if(col == BLOCK_SIZE-1) right_index = blockedIndex(bx+1, by, state->numBlocks, row, 0);
    else right_index = index + 1;
    
    int below_index;
    if(row == BLOCK_SIZE-1) below_index = blockedIndex(bx, by+1, state->numBlocks, 0, col);
    else below_index = index + BLOCK_SIZE;

    double above = state->currentFrame[above_index];
    double below = state->currentFrame[below_index];
    double left = state->currentFrame[left_index];
    double right = state->currentFrame[right_index];

    double current = state->currentFrame[index];
    double last = state->lastFrame[index];

    state->nextFrame[index] = state->alpha2 * (above + below + left + right - 4*current) + 2*current - last;
}

double update_amplitude_at_simple(State *state, int index) {
    int above_index = index - BLOCK_SIZE;
    int left_index = index - 1;
    int right_index = index + 1;
    int below_index = index + BLOCK_SIZE;

    double above = state->currentFrame[above_index];
    double below = state->currentFrame[below_index];
    double left = state->currentFrame[left_index];
    double right = state->currentFrame[right_index];

    double current = state->currentFrame[index];
    double last = state->lastFrame[index];

    state->nextFrame[index] = state->alpha2 * (above + below + left + right - 4*current) + 2*current - last;
}
// double calculate_amplitude_at(State *state, int bx, int by, int col, int row) { // TODO: Optimize this, reduce calls to gridIndex where possible
    
//     double above = state->currentFrame[gridIndex(i, j-1, state)];
//     double below = state->currentFrame[gridIndex(i, j+1, state)];
//     double left = state->currentFrame[gridIndex(i-1, j, state)];
//     double right = state->currentFrame[gridIndex(i+1, j, state)];

//     double current = state->currentFrame[gridIndex(i,j,state)];
//     double last = state->lastFrame[gridIndex(i,j,state)];

//     return state->alpha2 * (above + below + left + right - 4*current) + 2*current - last;
// }

void simulate_tick(State *state) {    
    const int default_horizontal_offset = 1; // horizontal
    const int default_vertical_offset = BLOCK_SIZE; // horizontal
    const int block_edge_h_offset = (BLOCK_SIZE*BLOCK_SIZE) - (BLOCK_SIZE-1);
    const int block_edge_v_offset = (BLOCK_SIZE*BLOCK_SIZE*state->numBlocks) - ((BLOCK_SIZE-2) * BLOCK_SIZE) - 1;

    int chunk = BLOCK_SIZE * BLOCK_SIZE;

    #pragma omp parallel for
    for(int block_index = 0; block_index < state->numBlocks * state->numBlocks; block_index++) {
        int bx = block_index % state->numBlocks;
        int by = block_index / state->numBlocks;
        
        // for(int row = 1; row < BLOCK_SIZE - 1; row++) {
        //     for(int col = 1; col < BLOCK_SIZE - 1; col++) {
        //         update_amplitude_at_simple(state, blockedIndex(bx,by,state->numBlocks,row,col));
        //     }
        // }

        for(int row = 0; row < BLOCK_SIZE; row++) {
            for(int col = 1; col < BLOCK_SIZE; col++) {
                update_amplitude_at_complex(state, bx, by, col, row);
            }
        }

        update_left_edge(state, bx, by);
        update_right_edge(state,bx,by);
        update_bottom_edge(state,bx,by);
        update_top_edge(state,bx,by);
        update_corners(state, bx, by);

        // 1 2 3    10 11 12    ... x num_blocks
        // 4 5 6    13 14 15    ... x num_blocks
        // 7 8 9    ...
        // num_blocks * BLOCK_SIZE
        // a b c
        // d e f
        // g h i

        // | bar .   |
        // | foo baz |

        // int foo = blockedIndex(0, 1, state->numBlocks, 0, BLOCK_SIZE-1); // top right of block [0,1]
        // int bar = blockedIndex(0, 0, state->numBlocks, BLOCK_SIZE-1, 0); // above foo
        // int baz = blockedIndex(1, 1, state->numBlocks, 0, 0);            // right of foo

        // int vert_offset = (BLOCK_SIZE*BLOCK_SIZE*state->numBlocks) - ((BLOCK_SIZE-2) * BLOCK_SIZE) - 1;
        // int hor_offset = (BLOCK_SIZE*BLOCK_SIZE) - (BLOCK_SIZE-1);
        
        // {
        // if(by != 0){ // first row
        //     int row = 1;
        //     for(int col = 1 + (bx == 0); col < BLOCK_SIZE-(1 + (bx == state->numBlocks)); col++) {
        //         update_amplitude_at_complex(state, bx, by, col, row);
        //         // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //         // update_amplitude_given_indices(state, index, index-block_edge_v_offset, index-default_horizontal_offset, index+default_horizontal_offset, index+default_vertical_offset);
        //     }
        // }

        // if(by != state->numBlocks-1){ // last row
        //     int row = state->ny;
        //     for(int col = 1 + (bx == 0); col < BLOCK_SIZE-(1 + (bx == state->numBlocks)); col++) {
        //         update_amplitude_at_complex(state, bx, by, col, row);
        //         // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //         // update_amplitude_given_indices(state, index, index-default_vertical_offset, index-default_horizontal_offset, index+default_horizontal_offset, index+block_edge_v_offset);
        //     }
        // }

        //     if(bx != 0){ // first col of block
        //         int col = 0;
        //         for(int row = 1 + (by == 0); row < BLOCK_SIZE-(1 + (by == state->numBlocks)); row++) {
        //             update_amplitude_at_complex(state, bx, by, col, row);
        //             // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //             // update_amplitude_given_indices(state, index, index-default_vertical_offset, index-block_edge_h_offset, index+default_horizontal_offset, index+default_vertical_offset);
        //         }
        //     }

        //     if(bx != state->numBlocks-1){ // last col of block
        //         int col = BLOCK_SIZE-1;
        //         for(int row = 1 + (by == 0); row < BLOCK_SIZE-(1 + (by == state->numBlocks)); row++) {
        //             update_amplitude_at_complex(state, bx, by, col, row);
        //             // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //             // update_amplitude_given_indices(state, index, index-default_vertical_offset, index-default_horizontal_offset, index+block_edge_h_offset, index+default_vertical_offset);
        //         }
        //     }
        // }
        
        // { // corners
        //     if(bx != 0 && by != 0) { // top left
        //         int col = 0;
        //         int row = 0;
        //         update_amplitude_at_complex(state, bx, by, col, row);
        //         // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //         // update_amplitude_given_indices(state, index, index-block_edge_v_offset, index-block_edge_h_offset, index+default_horizontal_offset, index+default_vertical_offset);
        //     }

        //     if(by != 0 && bx != state->numBlocks-1) { // top right
        //         int col = BLOCK_SIZE-1;
        //         int row = 0;
        //         update_amplitude_at_complex(state, bx, by, col, row);
        //         // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //         // update_amplitude_given_indices(state, index, index-block_edge_v_offset, index-default_horizontal_offset, index+block_edge_h_offset, index+default_horizontal_offset);
        //     }

        //     if(bx != 0 && by != state->numBlocks-1) { // bottom left
        //         int col = 0;
        //         int row = BLOCK_SIZE-1;
        //         update_amplitude_at_complex(state, bx, by, col, row);
        //         // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //         // update_amplitude_given_indices(state, index, index-default_vertical_offset, index-block_edge_h_offset, index+default_horizontal_offset, index+block_edge_v_offset);
        //     }

        //     if(bx != state->numBlocks-1 && by != state->numBlocks-1) { // bottom right
        //         int col = BLOCK_SIZE-1;
        //         int row = BLOCK_SIZE-1;
        //         update_amplitude_at_complex(state, bx, by, col, row);
        //         // int index = blockedIndex(bx, by, state->numBlocks, row, col);
        //         // update_amplitude_given_indices(state, index, index-default_vertical_offset, index-default_horizontal_offset, index+block_edge_h_offset, index+block_edge_v_offset);
        //     }
        // }


        // if(by != 0) { // skip over top row
            // for(int col = 1; ...)
        // }
    }
    // for (int i = 1; i < state->nx - 1; i++) {
    //     for (int j = 1; j < state->ny - 1; j++) {
    //         int curIndex = gridIndex(i, j, state);
    //         state->nextFrame[curIndex] = calculate_amplitude_at(i, j, state);
    //     }
    // }

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
    // droplet(&foo, 5, 5, 0.1, 0.1, 10);
    
    return foo;
}


int blockedIndex(int bx, int by, int num_blocks, int row, int col) {
    return (BLOCK_SIZE * BLOCK_SIZE) * ((by*num_blocks) + bx) + // origin position of block
            (col + (row * BLOCK_SIZE));                          // position within block
}

// x,y -> bi,bj,row,col
int gridIndex(int x, int y, State* state) {
    int bx = x / BLOCK_SIZE;
    int col = x % BLOCK_SIZE;
    int by = y / BLOCK_SIZE;
    int row = y % BLOCK_SIZE;
    return blockedIndex(bx, by, state->numBlocks, row, col);
}


int main_perf(void)
{
    const int nx = 1024;
    const int ny = 1024;
    State state = initState(nx,ny,0.2,1,0.25);

    long num_steps = 200;
    
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