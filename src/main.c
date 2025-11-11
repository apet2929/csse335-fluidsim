/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <omp.h>
#include <stdio.h>
#include "sim.h"
#include <math.h>


const int BLOCK_SIZE2 = 16;
/*******************************************************************************************
*
*   raylib [models] example - heightmap rendering
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 1.8, last time updated with raylib 3.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
static Mesh GenMeshCustom(void);    // Generate a simple triangle mesh from code
static Image* GenHeightMapImage(void);
static void GenHeightMapImageState(State *, Image *);

struct Neighbors {
    int index;
    int left;
    int right;
    int up;
    int down;
};


// int left_neighbor_index(int blockOrigin, int row) {
//     int leftBlockOrigin = blockOrigin - (BLOCK_SIZE2 * BLOCK_SIZE2);
//     return leftBlockOrigin + (BLOCK_SIZE2-1) + (BLOCK_SIZE2*row); // + (BLOCK_SIZE2-1) moves to the right edge of the block
// }

// int right_neighbor_index(int blockOrigin, int row) {
//     int rightBlockOrigin = blockOrigin + (BLOCK_SIZE2 * BLOCK_SIZE2);
//     return rightBlockOrigin + (BLOCK_SIZE2*row);
// }

// int top_neighbor_index(State* state, int blockOrigin, int col) {
//     int topBlockOrigin = blockOrigin - (BLOCK_SIZE2 * BLOCK_SIZE2 * state->numBlocks);
//     return topBlockOrigin + (BLOCK_SIZE2 * (BLOCK_SIZE2-1)) + col;
// }

// int bottom_neighbor_index(State* state, int blockOrigin, int col) {
//     int bottomBlockOrigin = blockOrigin + (BLOCK_SIZE2 * BLOCK_SIZE2 * state->numBlocks);
//     return bottomBlockOrigin + col;
// }


// int getBlockOrigin(State* state, int bx, int by) {
//     return (BLOCK_SIZE2 * BLOCK_SIZE2) * ((by*state->numBlocks) + bx);
// }


struct Neighbors getNeighbors(State* state, int bx, int by, int col, int row) {
    int blockOrigin = getBlockOrigin(state,bx,by);
    int index = blockedIndex(bx, by, state->numBlocks, row, col);
    int above_index;
    if(row == 0) above_index = blockedIndex(bx, by-1, state->numBlocks, BLOCK_SIZE2-1, col);
    else above_index = index - BLOCK_SIZE2;
    
    int left_index;
    if(col == 0) {
        left_index = left_neighbor_index(blockOrigin, row);
    }
    else left_index = index - 1;
    
    int right_index;
    if(col == BLOCK_SIZE2-1) right_index = right_neighbor_index(blockOrigin, row);
    else right_index = index + 1;
    
    int below_index;
    if(row == BLOCK_SIZE2-1) below_index = bottom_neighbor_index(state, blockOrigin, col);
    else below_index = index + BLOCK_SIZE2;
    return (struct Neighbors) { index, left_index, right_index, above_index, below_index };
}

void drawGrid(State* state) {
    for(int bx = 0; bx < state->numBlocks; bx++) {
        for(int by = 0; by < state->numBlocks; by++) {
            for(int col = 0; col < BLOCK_SIZE2; col++) {
                int i = blockedIndex(bx,by,state->numBlocks,0, col);
                state->currentFrame[i] = 0.5;
            }

            for(int col = 0; col < BLOCK_SIZE2; col++) {
                int i = blockedIndex(bx,by,state->numBlocks,BLOCK_SIZE2-1, col);
                state->currentFrame[i] = 0.5;
            }

            for(int row = 0; row < BLOCK_SIZE2; row++) {
                int i = blockedIndex(bx,by,state->numBlocks,row,0);
                state->currentFrame[i] = 0.5;
            }
            
            for(int row = 0; row < BLOCK_SIZE2; row++) {
                int i = blockedIndex(bx,by,state->numBlocks,row,BLOCK_SIZE2-1);
                state->currentFrame[i] = 0.5;
            }
        }
    }
}

void testNeighbors() {
    State state = initState(64,64,0.2,1,1.0);
    Color *pixels = (Color *)malloc(64*64*sizeof(Color)); 
    Image heightMap =  {
            .data = pixels,             // We can assign pixels directly to data
            .width = 64,
            .height = 64,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            .mipmaps = 1
    };
    struct Neighbors ns[] = { 
        getNeighbors(&state, 0,0,BLOCK_SIZE2-1,BLOCK_SIZE2-1), // bottom right of [0,0]
        // getNeighbors(&state, 2,1,0,0), // top left of block [2,1]
        // getNeighbors(&state, 2,1,0,BLOCK_SIZE2-1), // bottom left of block [2,1]
        // getNeighbors(&state, 2,1,BLOCK_SIZE2-1,0), // top right of block [2,1]
        // getNeighbors(&state, 2,1,BLOCK_SIZE2-5,BLOCK_SIZE2-5), // top right of block [2,1]
    };

    // drawGrid(&state);

    for(int nn = 0; nn < 1; nn++){
        struct Neighbors n = ns[nn];
        state.currentFrame[n.index] = 1.0;
        state.currentFrame[n.left] = 0.2;
        state.currentFrame[n.right] = 0.4;
        state.currentFrame[n.up] = 0.6;
        state.currentFrame[n.down] = 1.0;
    }
    // int bx = 1;
    // int by = 1;
    // int blockOrigin = (BLOCK_SIZE2 * BLOCK_SIZE2) * ((by*state.numBlocks) + bx);
    // int index = blockOrigin + BLOCK_SIZE2;
    // int leftBlockOrigin = (BLOCK_SIZE2 * BLOCK_SIZE2) * ((by*state.numBlocks) + (bx-1));
    // int leftNeighbor = leftBlockOrigin + BLOCK_SIZE2-1 + BLOCK_SIZE2;
    // int col = 0;
    // for(int row = 1; row < BLOCK_SIZE2-1; row++) {
    //     state.currentFrame[leftNeighbor] = 0.1;
    //     state.currentFrame[index] = 1.0;
    //     index += BLOCK_SIZE2;
    //     leftNeighbor += BLOCK_SIZE2;
    // }

    GenHeightMapImageState(&state, &heightMap);
    ExportImage(heightMap, "testNeighbors.png");
}

void testIndexing() {
    State state = initState(64,64,0.2,1,1.0);
    Color *pixels = (Color *)malloc(64*64*sizeof(Color)); 
    Image heightMap =  {
            .data = pixels,             // We can assign pixels directly to data
            .width = 64,
            .height = 64,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            .mipmaps = 1
    };
    drawGrid(&state);

    GenHeightMapImageState(&state, &heightMap);
    ExportImage(heightMap, "testIndexing.png");
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int nx = 128;
    const int ny = 128;

    int screenWidth = 800;
    int screenHeight = 450;
    screenWidth = screenWidth > nx ? screenWidth : nx;
    screenHeight = screenHeight > ny ? screenHeight : ny;

    const int drawMesh = 0;
    testIndexing();
    testNeighbors();
    return 0;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap rendering");

    // Define our custom camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 18.0f, 21.0f, 18.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    Vector3 mapPosition = { -8.0f, 10.0f, -8.0f };           // Define model position

    State state = initState(nx,ny,0.2,1,1.0);

    Color *pixels = (Color *)malloc(nx*ny*sizeof(Color)); 
    Image heightMap =  {
            .data = pixels,             // We can assign pixels directly to data
            .width = nx,
            .height = ny,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            .mipmaps = 1
        };
    
    GenHeightMapImageState(&state, &heightMap);
    // Image* heightMap = GenHeightMapImage();
    ExportImage(heightMap, "SimHeightMap.png");
	Texture2D heightTexture = LoadTextureFromImage(heightMap);        // Convert image to texture (VRAM)
    Mesh newMesh;
    Model newModel;
    if (drawMesh) {
        Mesh newMesh = GenMeshHeightmap(heightMap, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
        Model newModel = LoadModelFromMesh(newMesh);                  // Load model from generated mesh
        newModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightTexture; // Set map diffuse texture
    }


    // UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM
	// UnloadImage(heightMap);             // Unload heightmap image from RAM, already uploaded to VRAM

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    int cursorDisabled = 0;
    // Main game loop
    omp_set_num_threads(8);

    int total_fps = 0;
    int frames = 1;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsKeyDown(KEY_ENTER))  {
            printf("Foo bar!");
        }
        if (IsKeyPressed(KEY_H))
        {
            if (!cursorDisabled)
            {
                DisableCursor();                    // Limit cursor to relative movement inside the window();
            }
            else
            {
                EnableCursor();                    // Limit cursor to relative movement inside the window();
            }
            cursorDisabled = !cursorDisabled;
        }
        simulate_tick(&state);
        GenHeightMapImageState(&state, &heightMap);    
        UpdateTexture(heightTexture, heightMap.data);

        if (drawMesh) {
            newMesh = GenMeshHeightmap(heightMap, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
            newModel = LoadModelFromMesh(newMesh);                  // Load model from generated mesh
            newModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightTexture; // Set map diffuse texture
        }

        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);


            if (drawMesh) {
                BeginMode3D(camera);

                DrawModel(newModel, mapPosition, 0.25f,  Fade(BLUE, 0.5f));
                DrawGrid(20, 1.0f);

                EndMode3D();
                DrawTexture(heightTexture, screenWidth - heightTexture.width - 20, 20, BLUE);
                DrawRectangleLines(screenWidth - heightTexture.width - 20, 20, heightTexture.width, heightTexture.height, GREEN);
            }
            else {
                Vector2 pos = { screenWidth/2 - screenHeight/2, 0 };
                float scale = screenHeight / heightTexture.height;
                DrawTextureEx(heightTexture, pos, 0.0f, scale, BLUE);
            }

            DrawFPS(10, 10);

            Color color = LIME;
            total_fps += GetFPS();
            int average_fps = total_fps / frames;
            if ((average_fps < 30) && (average_fps >= 15)) color = ORANGE;
            else if (average_fps < 15) color = RED;
            DrawText(TextFormat("%2i Average FPS", average_fps), 10, 30, 20, color);
            frames++;

        EndDrawing();

        if (drawMesh) {
            UnloadModel(newModel);
        }
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(heightTexture);     // Unload texture

    free(state.currentFrame);
    free(state.lastFrame);
    free(state.nextFrame);

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static Image* GenHeightMapImage(void)
{
	int width = 128;
	int height = 128;
	// Dynamic memory allocation to store pixels data (Color type)
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (((x/32+y/32)/1)%2 == 0) pixels[y*width + x] = BLACK;
            else pixels[y*width + x] = GOLD;
        }
    }
    Image *image = malloc(sizeof(Image));
    
    // Load pixels data into an image structure and create texture
    Image checkedIm = {
        .data = pixels,             // We can assign pixels directly to data
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    *image = checkedIm;
	return image;
}


static void GenHeightMapImageState(State *state, Image *image)
{
	int width = state->nx;
	int height = state->ny;
	// Dynamic memory allocation to store pixels data (Color type)
    Color *pixels = image->data;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double amp = state->currentFrame[gridIndex(x,y,state)];
            double v = (tanh(amp) + 1) / 2.0; 
            // if(v != 0) printf("%f ", v);
            unsigned char val = (unsigned char) (v*255);
            // if(amp != 0) printf("%f %f %d \n", amp, v, val);

            pixels[y*width + x] = (Color) {val,val,val,255};
        }
    }
}

static Mesh GenMeshCustom(void)
{
    Mesh mesh = { 0 };
    mesh.triangleCount = 1;
    mesh.vertexCount = mesh.triangleCount*3;
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));    // 3 vertices, 3 coordinates each (x, y, z)
    mesh.texcoords = (float *)MemAlloc(mesh.vertexCount*2*sizeof(float));   // 3 vertices, 2 coordinates each (x, y)
    mesh.normals = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));     // 3 vertices, 3 coordinates each (x, y, z)

    // Vertex at (0, 0, 0)
    mesh.vertices[0] = 0;
    mesh.vertices[1] = 0;
    mesh.vertices[2] = 0;
    mesh.normals[0] = 0;
    mesh.normals[1] = 1;
    mesh.normals[2] = 0;
    mesh.texcoords[0] = 0;
    mesh.texcoords[1] = 0;

    // Vertex at (1, 0, 2)
    mesh.vertices[3] = 1;
    mesh.vertices[4] = 0;
    mesh.vertices[5] = 2;
    mesh.normals[3] = 0;
    mesh.normals[4] = 1;
    mesh.normals[5] = 0;
    mesh.texcoords[2] = 0.5f;
    mesh.texcoords[3] = 1.0f;

    // Vertex at (2, 0, 0)
    mesh.vertices[6] = 2;
    mesh.vertices[7] = 0;
    mesh.vertices[8] = 0;
    mesh.normals[6] = 0;
    mesh.normals[7] = 1;
    mesh.normals[8] = 0;
    mesh.texcoords[4] = 1;
    mesh.texcoords[5] =0;

    // Upload mesh data from CPU (RAM) to GPU (VRAM) memory
    UploadMesh(&mesh, false);

    return mesh;
}