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

static Mesh GenMeshCustom(void);    // Generate a simple triangle mesh from code
static Image* GenHeightMapImage(void);
static void GenFluidHeightMap(State *, Image *);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int nx = 176;
    const int ny = 176;

    int screenWidth = 700;
    int screenHeight = 700;
    screenWidth = screenWidth > nx ? screenWidth : nx;
    screenHeight = screenHeight > ny ? screenHeight : ny;

    const int drawMesh = 1;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap rendering");

    // Define our custom camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 18.0f, 10.0f, 18.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    Vector3 mapPosition = { 0.0f, 5.0f, 0.0f };           // Define model position

    State state = initState(nx,ny,0.2,1,1.0);

    Color *pixels = (Color *)malloc(nx*ny*sizeof(Color)); 
    Image heightMap =  {
            .data = pixels,             // We can assign pixels directly to data
            .width = nx,
            .height = ny,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            .mipmaps = 1
        };
    
    GenFluidHeightMap(&state, &heightMap);
    ExportImage(heightMap, "SimHeightMap.png");
	Texture2D heightTexture = LoadTextureFromImage(heightMap);        // Convert image to texture (VRAM)
    Mesh newMesh;
    Model newModel;
    if (drawMesh) {
        Mesh newMesh = GenMeshHeightmap(heightMap, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
        Model newModel = LoadModelFromMesh(newMesh);                  // Load model from generated mesh
        newModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightTexture; // Set map diffuse texture
    }

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
        GenFluidHeightMap(&state, &heightMap);    
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

                DrawModel(newModel, mapPosition, 0.5f,  Fade(BLUE, 0.5f));
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


static void GenFluidHeightMap(State *state, Image *image)
{
	int width = state->nx;
	int height = state->ny;
    Color *pixels = image->data;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double amp = state->currentFrame[gridIndex(x,y,state)];
            double v = (tanh(amp) + 1) / 2.0; 
            unsigned char val = (unsigned char) (v*255);

            pixels[y*width + x] = (Color) {val,val,val,255};
        }
    }
}
