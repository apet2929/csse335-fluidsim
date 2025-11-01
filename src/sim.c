#include <stdlib.h>
struct Grid;
struct State;

typedef struct Grid Grid;
typedef struct State State;

void simulate(State *state) {}

struct Grid{
    double** grid;
};

struct State {
    struct Grid* currentFrame; // u[1]
    struct Grid* lastFrame; // u[2]
    struct Grid* nextFrame; // u[0]
    int nx, ny;
    // double c, h, dt;
    double alpha2;
};

State initState(int nx, int ny, double c, double h, double dt) {
    double alpha = c * dt / h;
    double alpha2 = alpha * alpha;

    Grid* u0 = (Grid*) malloc(nx * ny * sizeof(double));
    Grid* u1 = (Grid*) malloc(nx * ny * sizeof(double));
    Grid* u2 = (Grid*) malloc(nx * ny * sizeof(double));
    State foo = {
        u0, u1, u2, nx, ny, alpha2
    };
    return foo;
}

