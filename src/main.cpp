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

// Objects
int playerSize = 80;
int zombieSize = 80;
int sateliteSize = 60;
int sateliteRotationSpeed = 5;
int sateliteDist = 40;
int weaponWidth = 80;
int weaponHeight = 20;
int bgWidth = 6000;
int bgHeight = 4000;

bool inGame = true;
int waveNum = 1;
int zombiesNum = 5;

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
Image sateliteImg;
Image bgImg;
Texture2D playerTex;
Texture2D zombieTex[10];
Texture2D weaponTex[20];
Texture2D sateliteTex;
Texture2D bgTex;
Sound weaponSound[20];

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
            DrawCircleV(pos, size, BROWN);
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
        int maxAmmo = 30;
        int curAmmo = 30;
        float projectileSpeed = 3.0f;
        bool isReloading = false;
        int reloadInterval = 0;
        int reloadIntervalDuration = 120;
        int shootInterval = 0;
        int shootIntervalDuration = 30;

    public:
        Weapon(float _width, float _height, Vector2 _offset)
        {
            width = _width;
            height = _height;
            offset = _offset;
        }

        int getMaxAmmo() { return maxAmmo; }
        int getCurAmmo() { return curAmmo; }
        bool getIsReloading() { return isReloading; }

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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isReloading && curAmmo > 0 && shootInterval >= shootIntervalDuration)
            {
                // Calculate the direction of the projectile based on the weapon's rotation
                float projectileDirection = rotation;

                // Create the projectile object with its position set to the front end of the weapon
                // and its velocity determined by the direction of fire
                projectiles.push_back(Projectile(frontEndPos, projectileDirection, projectileSpeed, 3));

                // Reduce ammo by 1
                curAmmo -= 1;

                // Play weapon sound effect
                PlaySound(weaponSound[0]);

                // Restart the shoot interval
                shootInterval = 0;
            }

            shootInterval += 1;

            // Reload ammo when R button is pressed
            if (IsKeyPressed(KEY_R) && curAmmo < maxAmmo)
            {
                isReloading = true;
                curAmmo = 0;
            }

            // If it is reloading, then start increasing the ammo
            if (isReloading)
            {
                if(reloadInterval >= reloadIntervalDuration)
                {
                    reloadInterval = 0;
                    curAmmo = maxAmmo;
                    isReloading = false;
                }
                else
                {
                    reloadInterval += 1;
                }
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
    float normalSpeed = 25.0f;
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
            if(pos.x > -6000.0f)
            {
                pos.x -= speed;
                worldPos.x -= speed;
            }
        }
        else if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            if(pos.x < 6000.0f)
            {
                pos.x += speed;
                worldPos.x += speed;
            }
        }
        
        // Up Down Movement
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            if(pos.y > -4000.0f)
            {   
                pos.y -= speed;
                worldPos.y -= speed;
            }
        }
        else if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            if(pos.y < 4000.0f)
            {
                pos.y += speed;
                worldPos.x += speed;
            }
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

class Satelite {
    private:
        int speedX, speedY;
        float radius = 60.0f;
        Vector2 position;
        Color color;
        Player* parent;
        int dist;
        int angle = 0;

    public:
        Satelite(int _speedX, int _speedY, Player* _parent){
            speedX = _speedX;
            speedY = _speedY;
            parent = _parent;
            position = parent->getPos();
            dist = abs(radius + sateliteDist + _parent->getSize());
        }
        
        float getRadius() { return radius; }

        void update() {
            // Satelite rotation movement
            position.x = parent->getPosX() + dist*cos(angle * PI / 180);
            position.y = parent->getPosY() - dist*sin(angle * PI / 180);
            angle += 2;
            if(angle >= 360)
            {
                angle = 0;
            }

            // Check for collision with zombies
            auto zombieIt = zombies.begin();
            Circle sateliteCircle = { position, radius / 2 };
            while(zombieIt != zombies.end())
            {
                Circle zombieCircle = zombieIt->getCircle();
                if (CheckCollisionCircles(sateliteCircle.pos, sateliteCircle.radius, zombieCircle.pos, zombieCircle.radius))
                {
                    zombieIt = zombies.erase(zombieIt);
                    std::cout << "HIT" << '\n';
                }
                else
                {
                    ++zombieIt;
                }
            }
        }

        void draw()
        {
            //DrawCircle(position.x, position.y, radius / 2, WHITE);
            DrawTexture(sateliteTex, position.x - radius / 2, position.y - radius / 2, WHITE);
        }
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

void LoadAllSound()
{
    weaponSound[0] = LoadSound("../audios/weapon0.wav");
}

void LoadAllImage()
{
    playerImg = LoadImage("../graphics/earth.png");
    zombieImg[0] = LoadImage("../graphics/moon.png");
    weaponImg[0] = LoadImage("../graphics/baguette.png");
    sateliteImg = LoadImage("../graphics/moon.png");
    bgImg = LoadImage("../graphics/spaceBg.png");
}

void ResizeAllImage()
{
    ImageResize(&playerImg, playerSize, playerSize);
    ImageResize(&zombieImg[0], zombieSize, zombieSize);
    ImageResize(&weaponImg[0], weaponWidth, weaponHeight);
    ImageResize(&sateliteImg, sateliteSize, sateliteSize);
    ImageResize(&bgImg, bgWidth, bgHeight);
}

void LoadAllTexture()
{
    playerTex = LoadTextureFromImage(playerImg);
    zombieTex[0] = LoadTextureFromImage(zombieImg[0]);
    weaponTex[0] = LoadTextureFromImage(weaponImg[0]);
    sateliteTex = LoadTextureFromImage(sateliteImg);
    bgTex = LoadTextureFromImage(bgImg);
}

void UnloadAllImage()
{
    UnloadImage(playerImg);
    UnloadImage(zombieImg[0]);
    UnloadImage(weaponImg[0]);
    UnloadImage(bgImg);
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
    while (projectileIt!= projectiles.end()) {
        auto zombieIt = zombies.begin();
        while (zombieIt!= zombies.end()) {
            Circle projectileCircle = { projectileIt->getPosition(), projectileIt->getSize() / 2 };
            Circle zombieCircle = zombieIt->getCircle();
            if (CheckCollisionCircles(projectileCircle.pos, projectileCircle.radius, zombieCircle.pos, zombieCircle.radius))
            {
                zombieIt = zombies.erase(zombieIt);
                projectileIt = std::find_if(projectiles.begin(), projectiles.end(), [projectileIt](const Projectile& p) { return &p == projectileIt.operator->(); });
                if (projectileIt != projectiles.end()) {
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

Vector2 GenerateRandomPos(Vector2 playerPos) {
    Vector2 pos;
    
    // Random decider for left or right
    int lr = GetRandomValue(0, 1);
    if(lr == 0) // Left
    {
        pos.x = float(GetRandomValue(playerPos.x - 2000, playerPos.x - 600));
    }
    else        // Right
    {
        pos.x = float(GetRandomValue(playerPos.x + 600, playerPos.x + 2000));
    }

    // Random decider for top or bottom
    int tb = GetRandomValue(0, 1);
    if(tb == 0) // Top
    {
        pos.y = float(GetRandomValue(playerPos.y - 2000, playerPos.y - 600));
    }
    else        // Bottom
    {
        pos.y = float(GetRandomValue(playerPos.y + 600, playerPos.y + 2000));
    }

    return pos;
}

void GameManager(Vector2 playerPos)
{
    // If all current zombies eliminated, start a new wave
    if(zombies.size() == 0)
    {
        waveNum += 1;
        zombiesNum += 5;

        // Spawn zombies in random location near player
        for(int i = 0; i < zombiesNum; i++)
        {
            Vector2 randomPos = GenerateRandomPos(playerPos);
            float randomSize = float(GetRandomValue(60, 80));
            Zombie zombie(randomPos, randomSize);
            zombies.push_back(zombie);
        }
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

    // Initialize audio device
    InitAudioDevice();

    // World Position
    //std::cout << displayWidth << ' ' << displayHeight << '\n';
    worldPos = Vector2{displayWidth / 2.0f, displayHeight / 2.0f};
    //worldPos = Vector2{float(GetDisplayWidth()) / 2, float(GetDisplayHeight()) / 2};
    startingPos = worldPos;

    // Texture Setup
    TextureSetup();

    // Load all audios
    LoadAllSound();

    // Set Game FPS (frame per second)
    SetTargetFPS(60);

    Player player({ 0, 0 });
    
    Zombie zombie1({ GetScreenWidth() / 2.0f + 80.0f, GetScreenHeight() / 2.0f + 80.0f}, 80.0f);
    Zombie zombie2({ GetScreenWidth() / 2.0f - 80.0f, GetScreenHeight() / 2.0f - 80.0f}, 80.0f);
    Zombie zombie3({ GetScreenWidth() / 2.0f + 80.0f, GetScreenHeight() / 2.0f - 80.0f}, 80.0f);
    zombies.push_back(zombie1);
    zombies.push_back(zombie2);
    zombies.push_back(zombie3);
    
    Weapon weapon(80.0f, 20.0f, {player.getSize(), player.getSize()});
    Weapon weapon1(80.0f, 20.0f, {player.getSize(), -player.getSize()});
    Satelite satelite(sateliteRotationSpeed, sateliteRotationSpeed, &player);
    int cnt = 0;

    // Game loop
    while (!WindowShouldClose())        // While the window is still open
    {
        ClearBackground(BLACK);

        // Manage the wave
        GameManager(player.getPos());

        // Update player
        player.update();

        // Update satelite
        satelite.update();

        // Update weapon
        weapon.update(player.getPos(), player.getRotation());
        weapon1.update(player.getPos(), player.getRotation());

        // Update zombie based on player position
        ZombiesUpdate(player);

        // Handle projectile
        ProjectilesHandling();

        Vector2 mousePos = GetMousePosition();
        // Debugging purpose
        if(cnt == 60) {
            std::cout << "Mouse : " << mousePos.x << ' ' << mousePos.y << '\n';
            std::cout << "Player: " << player.getPosX() << ' ' << player.getPosY() << '\n';
            std::cout << "World : " << worldPos.x << ' ' << worldPos.y << '\n';

            auto it = zombies.begin();
            while(it != zombies.end())
            {
                std::cout << "Zombie: " << it->getPosX() << ' ' << it->getPosY() << '\n';
                ++it;
            }

            auto at = projectiles.begin();
            while(at != projectiles.end())
            {
                std::cout << "Projectile: " << at->getPosition().x << ' ' << at->getPosition().y << '\n';
                ++at;
            }
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

            // Draw background
            // Top left corner
            DrawTexture(bgTex, -6000, 0, GRAY);
            // Bottom left corner
            DrawTexture(bgTex, -6000, -4000, GRAY);
            // Top right corner
            DrawTexture(bgTex, 0, 0, GRAY);
            // Bottom right corner
            DrawTexture(bgTex, 0, -4000, GRAY);

            DrawRectangle(0, 0, 100, 100, RED);

            player.draw();

            satelite.draw();

            weapon.draw();
            weapon1.draw();

            ZombiesDrawing();

            EndMode2D();
            //-----------------------------------------------------------------------------------------------

        DrawText(TextFormat("Wave: %d%", waveNum), 0, 0, 30, WHITE);

        DrawText(TextFormat("Energy: %d%", player.getSprintEnergy() / 9), 0, 30, 30, WHITE);

        DrawText(TextFormat("Ammo: %d/%d%", weapon.getCurAmmo(), weapon.getMaxAmmo()), 0, 60, 30, WHITE);

        if(weapon.getIsReloading())
        {
            DrawText("RELOADING", 0, 90, 30, WHITE);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
