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
int playerSize = 80;

Vector2 worldPos = {400, 225};
Vector2 startingPos = {400, 225};

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
    private:
        Vector2 offset; // Offset from player's position
        Vector2 pos;
        float rotation;
        float size;     

    public:
        Weapon(float _size, Vector2 _offset){
            size = _size;
            offset = _offset;
        }

        void update(Vector2 playerPos, float playerRotation) {
            // Rotate the offset according to the player's rotation
            Vector2 rotatedOffset = Vector2Rotate(offset, playerRotation * DEG2RAD);

            // Set the weapon position relative to the player's position
            // Add the rotated offset to the player's position
            // You may need to adjust the offset further to fine-tune the weapon's position
            pos = Vector2Add(playerPos, rotatedOffset);
            
            // Set the weapon rotation to match player rotation
            rotation = playerRotation;
        }

        void draw() {
            // Draw the weapon at its position
            DrawTexturePro(playerTex, Rectangle {0, 0, size, size}, {pos.x, pos.y, size, size}, {size / 2, size / 2}, rotation, RAYWHITE);
        }
};
class Entity
{
    protected:
    float speed = 1.0f;
    float size = 40.0f;
    Vector2 pos;

    public:
    float getPosX() { return pos.x; }
    float getPosY() { return pos.y; }
    float getSize() { return size; }
    Vector2 getPos() { return pos; }

    virtual void update() = 0;
    virtual void draw() = 0;
};

class Player : public Entity
{
    private:
    int sprintEnergy = 900;
    float sprintSpeed = 5.0f;
    float normalSpeed = 1.0f;
    bool isSprinting = false;
    float rotation = 0.0f;
    float size = 80.0f;

    public:
    Player(Vector2 _pos) {
        pos = _pos;
    }

    int getSprintEnergy() { return sprintEnergy; }

    float getRotation() { return rotation; }

    void update() override
    {
        // Player Sprint mode
        // Increase movement speed for a certain period of time
        if((IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) && !isSprinting && sprintEnergy >= 900)
        {
            isSprinting = true;
        }
        
        if(isSprinting && sprintEnergy > 0)
        {
            speed = sprintSpeed;
            sprintEnergy -= 6;
        }
        else
        {
            isSprinting = false;

            speed = normalSpeed;

            if(sprintEnergy < 900)
            {
                sprintEnergy += 1;
            }
        }

        // Player Movements
        // Left Right Movement
        if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            pos.x -= speed;
            worldPos.x -= speed;
        }
        else if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            pos.x += speed;
            worldPos.x += speed;
        }
        
        // Up Down Movement
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            pos.y -= speed;
            worldPos.y -= speed;
        }
        else if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            pos.y += speed;
            worldPos.x += speed;
        }

        // Player facing and rotation
        Vector2 mousePos = GetMousePosition();
        Vector2 dir = { mousePos.x - startingPos.x, mousePos.y - startingPos.y };
        rotation = atan2f(dir.y, dir.x) * RAD2DEG;
    }

    void draw() override
    {
        DrawCircle(pos.x, pos.y, size / 2, RED);
        DrawTexturePro(playerTex, Rectangle {0, 0, size, size}, {pos.x, pos.y, size, size}, {size / 2, size / 2}, rotation, RAYWHITE);
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
    //displayWidth = GetMonitorWidth(display);
    //displayHeight = GetMonitorHeight(display);
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
    ImageResize(&playerImg, playerSize, playerSize);
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
    //ToggleFullscreen();

    // Texture Setup
    TextureSetup();

    // Set Game FPS (frame per second)
    SetTargetFPS(60);

    Player player({ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f });
    Weapon weapon(80.0f, {player.getSize(), player.getSize()});
    int cnt = 0;

    // Game loop
    while (!WindowShouldClose())        // While the window is still open
    {
        ClearBackground(RAYWHITE);

        // Update player
        player.update();

        // Update weapon
        weapon.update(player.getPos(), player.getRotation());

        Vector2 mousePos = GetMousePosition();
        if(cnt == 60) {
            std::cout << "Mouse : " << mousePos.x << ' ' << mousePos.y << '\n';
            std::cout << "Player: " << player.getPosX() << ' ' << player.getPosY() << '\n';
            std::cout << "World : " << worldPos.x << ' ' << worldPos.y << '\n';
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

            DrawRectangle(GetDisplayWidth() / 2, GetDisplayHeight() / 2, 10, 10, RED);

            player.draw();

            weapon.draw();

            EndMode2D();
            //-----------------------------------------------------------------------------------------------

        DrawText("FURINA BEST GIRLLLLLLLL", 0, 0, 30, BLUE);

        DrawText(TextFormat("Energy: %d%", player.getSprintEnergy() / 9), 0, 30, 30, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
