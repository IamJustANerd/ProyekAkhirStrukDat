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

// Window display
int display;                      // current window display
int displayWidth;                 // max window width
int displayHeight;                // max window height

// Camera
Camera2D camera;
Vector2 cameraPos;
Vector2 cameraTarget;

// Classes
class Entity
{
    protected:
    float speed = 1.0f;
    float size = 20.0f;
    Vector2 pos;

    public:
    float getPosX() { return pos.x; }
    float getPosY() { return pos.y; }
    Vector2 getPos() { return pos; }

    virtual void update() = 0;
    virtual void draw() = 0;
};

class Player : public Entity
{
    public:
    Player(Vector2 _pos) {
        pos = _pos;
    }

    void update() override
    {
        pos.x += speed;
        pos.y += speed;
    }

    void draw() override
    {
        DrawCircle(pos.x, pos.y, size, BLUE);
    }
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

    // Set initial camera position and target
    camera.target = { 0 };
    camera.offset = { 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateCamera(Player player)
{
    // Update camera target based on player position
    camera.target = player.getPos();

    // Update camera offset based on player position
    camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
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

    Player player({ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f });

    int cnt = 0;

    // Game loop
    while (!WindowShouldClose())        // While the window is still open
    {
        ClearBackground(RAYWHITE);

        // Update player
        player.update();

        if(cnt == 60) {
            std::cout << player.getPosX() << ' ' << player.getPosY() << '\n';
            cnt = 0;
        }
        cnt += 1;

        // Update camera
        UpdateCamera(player);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            // These will be the only objects that are moving from camera perspective,
            // while the others outside of this will remain static

            // Begin 2D mode with camera
            //-----------------------------------------------------------------------------------------------
            BeginMode2D(camera);

            player.draw();

            DrawRectangle(GetDisplayWidth() / 2, GetDisplayHeight() / 2, 10, 10, RED);

            EndMode2D();
            //-----------------------------------------------------------------------------------------------

        DrawText("FURINA BEST GIRLLLLLLLL", 0, 0, 30, BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
