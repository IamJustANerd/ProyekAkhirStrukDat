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
// Window display
int display;                      // current window display
int displayWidth;                 // max window width
int displayHeight;                // max window height

// Camera
Camera2D camera;
Vector2 cameraPos;
Vector2 cameraTarget;

// Assets
Image playerImg;
Texture2D playerTex;

// Classes
class Weapon
{

};
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
    private:
    int sprintEnergy = 300;
    float sprintSpeed = 5.0f;
    float normalSpeed = 1.0f;
    bool isSprinting = false;
    float rotation = 0.0f;

    public:
    Player(Vector2 _pos) {
        pos = _pos;
    }

    void update() override
    {
        // Player Sprint mode
        // Increase movement speed for a certain period of time
        if((IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) && !isSprinting && sprintEnergy >= 300)
        {
            isSprinting = true;
        }
        
        if(isSprinting && sprintEnergy > 0)
        {
            speed = sprintSpeed;
            sprintEnergy -= 1;
        }
        else
        {
            isSprinting = false;

            speed = normalSpeed;

            if(sprintEnergy < 300)
            {
                sprintEnergy += 1;
            }
        }

        // Player Movements
        // Left Right Movement
        if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            pos.x -= speed;
        }
        else if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            pos.x += speed;
        }
        
        // Up Down Movement
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            pos.y -= speed;
        }
        else if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            pos.y += speed;
        }

        // Player facing and rotation
        Vector2 mousePos = GetMousePosition();
        Vector2 dir = { mousePos.x - pos.x, mousePos.y - pos.y };
        rotation = atan2f(dir.y, dir.x) * 180.0f / PI;
    }

    void draw() override
    {
        DrawTexturePro(playerTex, Rectangle {0, 0, 20, 20}, {pos.x, pos.y, 20, 20}, {20 / 2, 20 / 2}, rotation, RAYWHITE);
        DrawText(TextFormat("Energy: %d%", sprintEnergy / 3), pos.x, pos.y, 20, BLACK);
    }
};

class Zombie : public Entity
{
};

// Functions

void WindowSetup()
{
    display = GetCurrentMonitor();
    displayWidth = 800;
    displayHeight = 450;
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
    playerImg = LoadImage("../graphics/earth.png");
}

void ResizeAllImage()
{
    ImageResize(&playerImg, 20, 20);
}

void LoadAllTexture()
{
    playerTex = LoadTextureFromImage(playerImg);
}

void UnloadAllImage()
{
    UnloadImage(playerImg);
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
