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

void foo(){
	int my_rank = omp_get_thread_num();
    int thread_count = omp_get_num_threads();
	printf("Hello from thread %d. We have %d threads", my_rank, thread_count);
}
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

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    const int nx = 128;
    const int ny = 128;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap rendering");

    // Define our custom camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 18.0f, 21.0f, 18.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };           // Define model position

    State state = initState(nx,ny,0.2,1,1);

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
	Mesh newMesh = GenMeshHeightmap(heightMap, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
	Model newModel = LoadModelFromMesh(newMesh);                  // Load model from generated mesh
	newModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightTexture; // Set map diffuse texture

    // UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM
	// UnloadImage(heightMap);             // Unload heightmap image from RAM, already uploaded to VRAM

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsKeyDown(KEY_ENTER))  {
            printf("Foo bar!");
        }
        simulate_tick(&state);
        GenHeightMapImageState(&state, &heightMap);    
        UpdateTexture(heightTexture, heightMap.data);
        // heightTexture = LoadTextureFromImage(heightMap);        // Convert image to texture (VRAM)
        newMesh = GenMeshHeightmap(heightMap, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
        // UpdateMeshBuffer() 
        newModel = LoadModelFromMesh(newMesh);                  // Load model from generated mesh
    	newModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightTexture; // Set map diffuse texture


        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

			DrawModel(newModel, mapPosition, 1.0f, WHITE);
                

                DrawGrid(20, 1.0f);

            EndMode3D();

            DrawTexture(heightTexture, screenWidth - heightTexture.width - 20, 20, WHITE);
            DrawRectangleLines(screenWidth - heightTexture.width - 20, 20, heightTexture.width, heightTexture.height, GREEN);

            DrawFPS(10, 10);

        EndDrawing();

        UnloadModel(newModel);
        // UnloadMesh(newMesh);
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(heightTexture);     // Unload texture
    // UnloadModel(newModel);         // Unload model

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