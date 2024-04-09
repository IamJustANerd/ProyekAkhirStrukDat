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
#include <vector>
#include <algorithm>

// Variables
// Window display
int display;                      // current window display
int displayWidth;                 // max window width
int displayHeight;                // max window height
int playerSize = 80;
int zombieSize = 80;
int weaponWidth = 80;
int weaponHeight = 20;

Vector2 worldPos;
Vector2 startingPos;

// Camera
Camera2D camera;
Vector2 cameraPos;
Vector2 cameraTarget;

// Assets
Image playerImg;
Image zombieImg[10];
Image weaponImg[20];
Texture2D playerTex;
Texture2D zombieTex[10];
Texture2D weaponTex[20];

// Struct
struct Circle
{
    Vector2 pos;
    float radius;
};

// Classes
class Projectile
{
    private:
        Vector2 pos;
        Vector2 velocity;
        float speed;
        float size;
        int lifetime = 300;        // Projectile might be disappeared after a certain amount of time
        bool alive = true;

    public:
        Projectile(Vector2 _pos, float _direction, float _speed, float _size)
        {
            pos = _pos;
            speed = _speed;
            size = _size;

            // Calculate velocity based on direction and speed
            velocity.x = cosf(_direction * DEG2RAD) * _speed;
            velocity.y = sinf(_direction * DEG2RAD) * _speed;
        }

        bool IsAlive() { return alive; }

        void update()
        {
            // Update projectile position based on velocity
            pos = Vector2Add(pos, velocity);

            lifetime -= 1;

            if(lifetime <= 0)
                alive = false;
        }

        void draw() const
        {
            // Draw the projectile at its position
            DrawCircleV(pos, size, BLACK);
        }

        Vector2 getPosition() const
        {
            return pos;
        }

        float getSize() const
        { 
            return size;
        }
};

// Dynamic array to contain all existing projectiles in the game
std::vector<Projectile> projectiles;
class Weapon
{
    private:
        Vector2 offset; // Offset from player's position
        Vector2 pos;
        float rotation;
        float width, height;     

        float projectileSpeed = 3.0f;

    public:
        Weapon(float _width, float _height, Vector2 _offset)
        {
            width = _width;
            height = _height;
            offset = _offset;
        }

        void update(Vector2 playerPos, float playerRotation)
        {
            // Rotate the offset according to the player's rotation
            Vector2 rotatedOffset = Vector2Rotate(offset, playerRotation * DEG2RAD);

            // Set the weapon position relative to the player's position
            // then add the rotated offset to the player's position
            // Dev note: Offset adjustment might be required
            pos = Vector2Add(playerPos, rotatedOffset);
            
            // Set the weapon rotation to match player rotation
            rotation = playerRotation;

            // Calculate the front end of the weapon
            Vector2 frontEndOffset = {width / 2, 0};
            Vector2 frontEndPos = Vector2Add(pos, Vector2Rotate(frontEndOffset, rotation * DEG2RAD));

            // Shot projectile when mouse button is pressed
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                // Calculate the direction of the projectile based on the weapon's rotation
                float projectileDirection = rotation;

                // Create the projectile object with its position set to the front end of the weapon
                // and its velocity determined by the direction of fire
                projectiles.push_back(Projectile(frontEndPos, projectileDirection, projectileSpeed, 3));
            }

            // Update all projectiles
            auto projectileIt = projectiles.begin();
            while (projectileIt != projectiles.end())
            {
                projectileIt->update();
                if (!projectileIt->IsAlive())
                {
                    projectileIt = projectiles.erase(projectileIt); // Remove projectile if its lifetime has expired
                }
                else
                {
                    ++projectileIt;
                }
            }
        }

        void draw()
        {
            // Draw the weapon at its position
            DrawTexturePro(weaponTex[0], Rectangle {0, 0, width, height}, {pos.x, pos.y, width, height}, {width / 2, height / 2}, rotation, RAYWHITE);

            // Draw all projectiles
            for (const auto& projectile : projectiles) {
                projectile.draw();
            }
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
    float normalSpeed = 2.5f;
    bool isSprinting = false;
    float rotation = 0.0f;
    float size = 80.0f;

    public:
    Player(Vector2 _pos)
    {
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
    private:
    float speed = 1.0f;
    float rotation;
    Vector2 dir;
    Vector2 playerPos;

    public:
    Zombie(Vector2 _pos, float _size)
    {
        pos = _pos;
        size = _size;
    }

    void update() override
    {
        // Zombie facing and rotation
        Vector2 dir = { playerPos.x - pos.x, playerPos.y - pos.y };
        rotation = atan2f(dir.y, dir.x) * RAD2DEG;

        // Zombie movement
        dir.x = cosf(rotation * DEG2RAD) * speed;
        dir.y = sinf(rotation * DEG2RAD) * speed;

        pos = Vector2Add(pos, dir);
    }

    void draw() override
    {
        DrawTexturePro(zombieTex[0], Rectangle {0, 0, size, size}, {pos.x, pos.y, size, size}, {size / 2, size / 2}, rotation, RAYWHITE);
    }

    void setPlayerPos(Vector2 _playerPos)
    {
        playerPos = _playerPos;
    }

    Circle getCircle()
    {
        Circle c;
        c.pos = pos;
        c.radius = size / 2.0f;
        return c;
    }
};

// Dynamic array to contain all existing zombies in the game
std::vector<Zombie> zombies;

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
    zombieImg[0] = LoadImage("../graphics/moon.png");
    weaponImg[0] = LoadImage("../graphics/baguette.png");
}

void ResizeAllImage()
{
    ImageResize(&playerImg, playerSize, playerSize);
    ImageResize(&zombieImg[0], zombieSize, zombieSize);
    ImageResize(&weaponImg[0], weaponWidth, weaponHeight);
}

void LoadAllTexture()
{
    playerTex = LoadTextureFromImage(playerImg);
    zombieTex[0] = LoadTextureFromImage(zombieImg[0]);
    weaponTex[0] = LoadTextureFromImage(weaponImg[0]);
}

void UnloadAllImage()
{
    UnloadImage(playerImg);
    UnloadImage(zombieImg[0]);
    UnloadImage(weaponImg[0]);
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

void ProjectilesHandling()
{
    // Check projectiles collision with zombies
    auto projectileIt = projectiles.begin();
    auto zombieIt = zombies.begin();
    while (projectileIt!= projectiles.end()) {
        while (zombieIt!= zombies.end()) {
            Circle projectileCircle = { projectileIt->getPosition(), projectileIt->getSize() / 2.0f };
            Circle zombieCircle = zombieIt->getCircle();
            if (CheckCollisionCircles(projectileCircle.pos, projectileCircle.radius, zombieCircle.pos, zombieCircle.radius))
            {
                zombieIt = zombies.erase(zombieIt);
                projectileIt = std::find_if(projectiles.begin(), projectiles.end(), [projectileIt](const Projectile& p) { return &p == projectileIt.operator->(); });
                if (projectileIt!= projectiles.end()) {
                    projectiles.erase(projectileIt);
                }
                std::cout << "HIT" << '\n';
                break;
            }
            else
            {
                ++zombieIt;
            }
        }
        if (projectileIt!= projectiles.end()) {
            ++projectileIt;
        }
    }
}

void ZombiesUpdate(Player& player)
{
    auto zombieIt = zombies.begin();
    while (zombieIt != zombies.end())
    {
        zombieIt->setPlayerPos(player.getPos());
        zombieIt->update();

        // Check if the zombie is within the player's circle
        Circle zombieCircle = zombieIt->getCircle();
        Circle playerCircle = { player.getPos(), player.getSize() / 2.0f };
        if (CheckCollisionCircles(playerCircle.pos, playerCircle.radius, zombieCircle.pos, zombieCircle.radius))
        {
            // Game over
            std::cout << "Game Over" << std::endl;
        }

        ++zombieIt;
    }
}

void ZombiesDrawing()
{
    auto zombieIt = zombies.begin();
    while (zombieIt != zombies.end())
    {
        zombieIt->draw();
        ++zombieIt;
    }
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

    // World Position
    //std::cout << displayWidth << ' ' << displayHeight << '\n';
    worldPos = Vector2{displayWidth / 2.0f, displayHeight / 2.0f};
    //worldPos = Vector2{float(GetDisplayWidth()) / 2, float(GetDisplayHeight()) / 2};
    startingPos = worldPos;

    // Texture Setup
    TextureSetup();

    // Set Game FPS (frame per second)
    SetTargetFPS(60);

    Player player({ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f });
    Zombie zombie({ GetScreenWidth() / 2.0f + 80.0f, GetScreenHeight() / 2.0f + 80.0f}, 80.0f);
    zombies.push_back(zombie);
    Weapon weapon(80.0f, 20.0f, {player.getSize(), player.getSize()});
    int cnt = 0;

    // Game loop
    while (!WindowShouldClose())        // While the window is still open
    {
        ClearBackground(GREEN);

        // Update player
        player.update();

        // Update weapon
        weapon.update(player.getPos(), player.getRotation());

        // Update zombie based on player position
        ZombiesUpdate(player);

        // Handle projectile
        ProjectilesHandling();

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

            ZombiesDrawing();

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
