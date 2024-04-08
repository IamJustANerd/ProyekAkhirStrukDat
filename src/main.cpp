/*
Nama  : Michael
NRP   : 5024231022
Kelas : Pemrograman Lanjut A

Project Black Friday
*/

#include "../include/raylib.h"
#include "../include/raymath.h"
#include <stddef.h>                 // Null Reference (for debugging purpose)
#include <stdio.h>                  // Standard Input-Output
#include <iostream>

// Variables
const float phi = 3.14;
int display;                  // current window display
int displayWidth;                 // max window width
int displayHeight;                // max window height
Camera2D camera;

// Classes
class Entity
{

};

class Player : public Entity
{
};

class Zombie : public Entity
{
};

// Functions

void WindowSetup()
{
    display = GetCurrentMonitor();
    displayWidth = GetMonitorWidth(display);
    displayHeight = GetMonitorHeight(display);
}

int GetDisplayWidth()
{
    display = GetCurrentMonitor();
    
    return GetMonitorWidth(display);
}

int GetDisplayHeight()
{
    display = GetCurrentMonitor();

    return GetMonitorHeight(display);
}

void CameraSetup()
{
    camera = { 0 };
}

void LoadAllImage()
{
}

void ResizeAllImage()
{
}

void LoadAllTexture()
{
}

void UnloadAllImage()
{
}

void TextureSetup()
{
    // Set image yang dipakai
    // 1. Load all images
    LoadAllImage();

    // 2. Resize all images
    ResizeAllImage();

    // 3. Load all textures from images
    LoadAllTexture();

    // 4. Unload all images since they've been chenged into textures
    UnloadAllImage();
}

int main()
{
    // Window setup
    WindowSetup();

    // Camera setup
    CameraSetup();

    // Starting window size
    InitWindow(displayWidth, displayHeight, "Raylib - Black Friday");
    ToggleFullscreen();

    // Texture Setup
    TextureSetup();

    // Set Game FPS (frame per second)
    SetTargetFPS(60);

    // Game loop
    while (!WindowShouldClose())        // While the window is still open
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Tes", GetDisplayWidth() / 2, GetDisplayHeight() / 2, 20, LIGHTGRAY);
        DrawRectangle(0, 0, 100, 100, RED);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
