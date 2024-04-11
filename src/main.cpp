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
// Player
int playerSize = 80;
int score = 0;

// Zombie
int zombieSize = 80;

// Satelite
int sateliteSize = 60;
int sateliteRotationSpeed = 2;
int sateliteDist = 40;
int sateliteLevel = 0;

// Weapon
int weaponWidth[20] = {40, 40, 80, 50, 60}, weaponHeight[20] = {10, 10, 20, 15, 20};
int weaponLevel = 1;
int weaponTier = 1;

// Buff
int buffSize = 60;
bool isBuffActive[5];
int buffCnt[5] = {0, 0, 0, 0, 0};
// 60 = 1 second
int buffDuration[5] = {1800, 1800, 1800, 1800, 1800};

// Background
int bgWidth = 6000;
int bgHeight = 4000;

// Game manager
bool inGame = false;
int waveNum = 0;
int zombiesNum = 0;
int zombiesLevel = 1;

// Debugging purpose
Vector2 worldPos;
Vector2 startingPos;

// Camera
Camera2D camera;
Vector2 cameraPos;
Vector2 cameraTarget;

// Assets
Image playerImg;
Image zombieImg[20];
Image weaponImg[20];
Image sateliteImg;
Image bgImg;
Image buffImg[20];
Image iconImg[20];
Texture2D playerTex;
Texture2D zombieTex[20];
Texture2D weaponTex[20];
Texture2D sateliteTex;
Texture2D bgTex;
Texture2D buffTex[20];
Texture2D iconTex[20];
Sound weaponSound[20];

// Level based on color (for weapons, satelite, and enemies)
Color transparent = {0, 0, 0, 0};
Color level[11] = {
    transparent,// 0
    RAYWHITE,   // 1
    RED,        // 2
    ORANGE,     // 3
    YELLOW,     // 4
    GREEN,      // 5
    DARKGREEN,  // 6
    DARKBLUE,   // 7
    BLUE,       // 8
    PURPLE,     // 9
    GOLD        // 10
};

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
        int damage;
        int lifetime = 300;        // Projectile might be disappeared after a certain amount of time
        bool alive = true;

    public:
        Projectile(Vector2 _pos, float _direction, float _speed, int _damage, float _size)
        {
            pos = _pos;
            speed = _speed;
            size = _size;
            damage = _damage;

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

        int getDamage() const
        {
            return damage;
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
        int curAmmo = 0;
        float projectileSpeed = 10.0f;
        bool isReloading = false;
        int reloadInterval = 0;
        int reloadIntervalDuration = 120;
        int shootInterval = 0;
        int shootIntervalDuration = 30;
        int damage;

    public:
        Weapon(float _width, float _height, int _maxAmmo, int _shootIntervalDuration, int _damage, Vector2 _offset)
        {
            width = _width;
            height = _height;
            maxAmmo = _maxAmmo;
            curAmmo = maxAmmo;
            shootIntervalDuration = _shootIntervalDuration;
            damage = _damage;
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

            // Shot projectile when mouse button is pressed (for tier 2 and below)
            if (weaponTier <= 2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isReloading && curAmmo > 0 && shootInterval >= shootIntervalDuration)
            {
                // Calculate the direction of the projectile based on the weapon's rotation
                float projectileDirection = rotation;

                // Create the projectile object with its position set to the front end of the weapon
                // and its velocity determined by the direction of fire
                projectiles.push_back(Projectile(frontEndPos, projectileDirection, projectileSpeed, damage + (2 * (weaponLevel - 1)), 3));

                // Reduce ammo by 1
                if(!isBuffActive[0])
                {
                    curAmmo -= 1;
                }

                // Play weapon sound effect
                PlaySound(weaponSound[0]);

                // Restart the shoot interval
                shootInterval = 0;
            }

            // Shot projectile when mouse button is down (for tier 3 and above)
            if (weaponTier > 2 && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !isReloading && curAmmo > 0 && shootInterval >= shootIntervalDuration)
            {
                // Calculate the direction of the projectile based on the weapon's rotation
                float projectileDirection = rotation;

                // Create the projectile object with its position set to the front end of the weapon
                // and its velocity determined by the direction of fire
                projectiles.push_back(Projectile(frontEndPos, projectileDirection, projectileSpeed, damage * weaponLevel, 3));

                // Reduce ammo by 1
                if(!isBuffActive[0])
                {
                    curAmmo -= 1;
                }

                // Play weapon sound effect
                PlaySound(weaponSound[0]);

                // Restart the shoot interval
                shootInterval = 0;
            }

            shootInterval += 1;

            // Reload ammo when R button is pressed or when ran out of ammo
            if ((IsKeyPressed(KEY_R) && curAmmo < maxAmmo) || curAmmo == 0)
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
            DrawTexturePro(weaponTex[weaponTier - 1], Rectangle {0, 0, width, height}, {pos.x, pos.y, width, height}, {width / 2, height / 2}, rotation, level[weaponLevel]);

            // Draw all projectiles
            for (const auto& projectile : projectiles) {
                projectile.draw();
            }
        }
};

Weapon weaponList[7]
{
    // Tier 1 - Short baguette
    {float(weaponWidth[0]), float(weaponHeight[0]), 16, 60, 4, {float(playerSize) / 2, float(playerSize) / 2}},
    // Tier 2 - Double short baguette
    {float(weaponWidth[1]), float(weaponHeight[1]), 16, 60, 4, {float(playerSize) / 2, float(playerSize) / 2}},
    {float(weaponWidth[1]), float(weaponHeight[1]), 16, 60, 4, {float(playerSize) / 2, -float(playerSize) / 2}},
    // Tier 3 - Baguette rifle
    {float(weaponWidth[2]), float(weaponHeight[2]), 30, 20, 6, {float(playerSize) / 2, float(playerSize) / 2}},
    // Tier 4 - Double baguette uzis
    {float(weaponWidth[3]), float(weaponHeight[3]), 60, 10, 3, {float(playerSize) / 2, float(playerSize) / 2}},
    {float(weaponWidth[3]), float(weaponHeight[3]), 60, 10, 3, {float(playerSize) / 2, -float(playerSize) / 2}},
    // Tier 5 - Baguette machine gun
    {float(weaponWidth[4]), float(weaponHeight[4]), 500, 0, 1, {float(playerSize) / 2, float(playerSize) / 2}}
};

// Player current weapon
std::vector<Weapon> curWeapon;

class LootBox
{
    private:
    int item;
    /*
    Item list:
    0 - (Fire icon) - Infinity ammo and double damage, lasts for 30 seconds.
    1 - (Moon icon) - Gain satelite. Only one satelite can exist at the same time. Gaining more will increase its level
        by 1, up to level 10 (golden colour). Each level increase its damage by 2. Taking another buff will only increase its speed
    2 - (Baguette bucket icon) Upgrade current weapon level by 1. Each level increase its damage by 2. Reaching level 10 will
        unlock a new weapon (from tier 1 up to tier 5). Won't appear if already maxed.
        Tier 1: short baguette, no auto
        Tier 2: double short baguette, no auto
        Tier 3: baguette rifle, semi auto
        Tier 4: double baguette uzis, semi auto
        Tier 5: laser, auto, full auto
    3 - (Snowflake icon) Slow enemies movement speed by 0.5x for 30 seconds
    4 - (Lightning icon) Gain unlimited sprint for the next 30 seconds.
    */
    float size = 60.0f;
    Vector2 pos;

    public:
    LootBox(Vector2 _pos, int _item)
    {
        pos = _pos;
        item = _item;
    }

    Vector2 getPos() { return pos; }
    float getSize() { return size; }

    void draw()
    {
        DrawTexturePro(buffTex[item], Rectangle {0, 0, size, size}, {pos.x, pos.y, size, size}, {size / 2, size / 2}, 0, RAYWHITE);
    }

    Circle getCircle()
    {
        Circle c;
        c.pos = pos;
        c.radius = size / 2.0f;
        return c;
    }

    int getType() { return item; }
};

std::vector<LootBox> lootBoxes;

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
    float sprintSpeed = 10.0f;
    float normalSpeed = 5.0f;
    bool isSprinting = false;
    float rotation = 0.0f;
    float size = 80.0f;

    public:
    Player(Vector2 _pos)
    {
        pos = _pos;
    }

    bool getIsSprinting() { return isSprinting; }

    int getSprintEnergy() { return sprintEnergy; }

    float getRotation() { return rotation; }

    void update() override
    {
        // Only update when the game started
        if(!inGame)
        {
            sprintEnergy = 900;
            return;
        }
        // Player Sprint mode
        // Increase movement speed for a certain period of time
        if((IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) && !isSprinting && sprintEnergy >= 900)
        {
            isSprinting = true;
        }

        if(isSprinting && sprintEnergy > 0)
        {
            speed = sprintSpeed;
            
            if(!isBuffActive[4])
            {
                sprintEnergy -= 6;
            }   
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
        // Only draw player when the game started
        if(!inGame)
        {
            return;
        }
        //DrawCircle(pos.x, pos.y, size / 2, RED);
        DrawTexturePro(playerTex, Rectangle {0, 0, size, size}, {pos.x, pos.y, size, size}, {size / 2, size / 2}, rotation, RAYWHITE);
    }
};

class Zombie : public Entity
{
    private:
    float speed = 2.0f + (0.1f * zombiesLevel);
    float rotation;
    Vector2 dir;
    Vector2 playerPos;
    int maxHealth;
    int sateliteHitInterval = 120;
    int type;
    Color color;

    public:
    int sateliteHitCd = 120;
    int health;
    
    Zombie(Vector2 _pos, float _size, int _type)
    {
        pos = _pos;
        size = _size;
        type = _type;

        if(type == 0) // Normal zombie
        {
            health = 5 * zombiesLevel;
            maxHealth = health;
        }
        else if(type == 1) // Lootbox zombie
        {
            health = 10 * zombiesLevel;
            maxHealth = health;
        }
        else if(type == 2) // Boss zombie
        {
            health = 100;
            maxHealth = health;
        }
        else if(type == 3) // Big zombie
        {
            health = 50;
            maxHealth = health;
            size *= 2;
        }
    }

    void update() override
    {
        // Zombie facing and rotation
        Vector2 dir = { playerPos.x - pos.x, playerPos.y - pos.y };
        rotation = atan2f(dir.y, dir.x) * RAD2DEG;

        // Zombie movement
        dir.x = cosf(rotation * DEG2RAD) * speed;
        dir.y = sinf(rotation * DEG2RAD) * speed;

        // If slow debuff is active, reduce movement speed by 50%
        if(isBuffActive[3])
        {
            dir.x /= 2;
            dir.y /= 2;
        }

        pos = Vector2Add(pos, dir);

        // Satelite hit cooldown
        if(sateliteHitCd < sateliteHitInterval)
        {
            sateliteHitCd += 1;
        }
    }

    void draw() override
    {
        // Draw zombie
        DrawTexturePro(zombieTex[type], Rectangle {0, 0, size, size}, {pos.x, pos.y, size, size}, {size / 2, size / 2}, rotation, level[zombiesLevel]);

        // Draw healthbar
        DrawRectangle(pos.x - size / 2, pos.y + size / 2, zombieSize * health / maxHealth, 10, RED);
        DrawRectangleLines(pos.x - size / 2, pos.y + size / 2, zombieSize, 10, WHITE);
    }

    void setPlayerPos(Vector2 _playerPos)
    {
        playerPos = _playerPos;
    }

    int getType() { return type; }

    int getSateliteHitInterval() { return sateliteHitInterval; }

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

// Zombie drop handling
void ZombieDrop(Vector2 zombiePos, int type)
{
    if(type == 1) // Zombie lootbox will drop lootbox upon death
    {
        // Generate random buff
        int item;
        while(true)
        {
            item = GetRandomValue(0, 4);
            // If satelite upgrade is maxed, change buff
            if(item == 1 && sateliteLevel >= 10)
            {
                continue;
            }
            // If baguette upgrade is maxed, change buff
            if(item == 2 && (weaponLevel >= 10 && weaponTier >= 5))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        LootBox lootBox(zombiePos, item);

        lootBoxes.push_back(lootBox);
    }
}

class Satelite {
    private:
        float radius = 60.0f;
        Vector2 position;
        Color color;
        Player* parent;
        int dist;
        int angle = 0;
        int damage = 2;

    public:
        Satelite(Player* _parent){
            parent = _parent;
            position = parent->getPos();
            dist = abs(radius + sateliteDist + _parent->getSize());
        }
        
        float getRadius() { return radius; }

        void update() {
            // Satelite rotation movement
            position.x = parent->getPosX() + dist*cos(angle * PI / 180);
            position.y = parent->getPosY() - dist*sin(angle * PI / 180);
            angle += (sateliteRotationSpeed + sateliteLevel);
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
                // If hit and currently no in the satelite hit cooldown, reduce zombie health
                if (CheckCollisionCircles(sateliteCircle.pos, sateliteCircle.radius, zombieCircle.pos, zombieCircle.radius) &&
                    zombieIt->sateliteHitCd >= zombieIt->getSateliteHitInterval())
                {
                    // Reduce health
                    if(isBuffActive[0])
                    {
                        zombieIt->health -= damage * sateliteLevel * 2;
                    }
                    else
                    {   
                        zombieIt->health -= damage * sateliteLevel;
                    }

                    // Enter satelite hit cooldown
                    zombieIt->sateliteHitCd = 0;

                    // If zombie health drops below zero, kill it
                    if(zombieIt->health <= 0)
                    {
                        // If a loot zombie died, spawn a lootbow
                        if(zombieIt->getType() == 1)
                        {
                            ZombieDrop(zombieIt->getPos(), zombieIt->getType());
                        }
                        
                        // Gain score
                        int point = 0;
                        if(zombieIt->getType() == 0)
                        {
                            point = 1;
                        }
                        else if(zombieIt->getType() == 1)
                        {
                            point = 2;
                        }
                        else if(zombieIt->getType() == 2)
                        {
                            point = 50;
                        }
                        else if(zombieIt->getType() == 3)
                        {
                            point = 20;
                        }
                        score += point * zombiesLevel;

                        // Delete zombie
                        zombieIt = zombies.erase(zombieIt);
                    }
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
            DrawTexture(sateliteTex, position.x - radius / 2, position.y - radius / 2, level[sateliteLevel]);
        }
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

void LoadAllSound()
{
    weaponSound[0] = LoadSound("../audios/weapon0.wav");
}

void LoadAllImage()
{
    playerImg = LoadImage("../graphics/earth.png");

    zombieImg[0] = LoadImage("../graphics/asteroid.png");
    zombieImg[1] = LoadImage("../graphics/lootbox.png");
    zombieImg[2] = LoadImage("../graphics/UFO.png");
    zombieImg[3] = LoadImage("../graphics/asteroid.png");
    
    weaponImg[0] = LoadImage("../graphics/baguette.png");
    weaponImg[1] = LoadImage("../graphics/baguette.png");
    weaponImg[2] = LoadImage("../graphics/baguette.png");
    weaponImg[3] = LoadImage("../graphics/baguette.png");
    weaponImg[4] = LoadImage("../graphics/baguette.png");
    
    sateliteImg = LoadImage("../graphics/moon.png");
    
    bgImg = LoadImage("../graphics/spaceBg.png");

    buffImg[0] = LoadImage("../graphics/fireBuff.png");
    buffImg[1] = LoadImage("../graphics/moon.png");
    buffImg[2] = LoadImage("../graphics/baguetteUpgrade.png");
    buffImg[3] = LoadImage("../graphics/slowDebuff.png");
    buffImg[4] = LoadImage("../graphics/electricBuff.png");

    iconImg[0] = LoadImage("../graphics/fireBuff.png");
    iconImg[1] = LoadImage("../graphics/moon.png");
    iconImg[2] = LoadImage("../graphics/baguetteUpgrade.png");
    iconImg[3] = LoadImage("../graphics/slowDebuff.png");
    iconImg[4] = LoadImage("../graphics/electricBuff.png");
}

void ResizeAllImage()
{
    ImageResize(&playerImg, playerSize, playerSize);

    ImageResize(&zombieImg[0], zombieSize, zombieSize);
    ImageResize(&zombieImg[1], zombieSize, zombieSize);
    ImageResize(&zombieImg[2], zombieSize, zombieSize);
    ImageResize(&zombieImg[3], zombieSize * 4, zombieSize * 4);

    ImageResize(&weaponImg[0], weaponWidth[0], weaponHeight[0]);
    ImageResize(&weaponImg[1], weaponWidth[1], weaponHeight[1]);
    ImageResize(&weaponImg[2], weaponWidth[2], weaponHeight[2]);
    ImageResize(&weaponImg[3], weaponWidth[3], weaponHeight[3]);
    ImageResize(&weaponImg[4], weaponWidth[4], weaponHeight[4]);

    ImageResize(&sateliteImg, sateliteSize, sateliteSize);

    ImageResize(&bgImg, bgWidth, bgHeight);

    ImageResize(&buffImg[0], buffSize, buffSize);
    ImageResize(&buffImg[1], buffSize, buffSize);
    ImageResize(&buffImg[2], buffSize, buffSize);
    ImageResize(&buffImg[3], buffSize, buffSize);
    ImageResize(&buffImg[4], buffSize, buffSize);

    ImageResize(&iconImg[0], buffSize / 2, buffSize / 2);
    ImageResize(&iconImg[1], buffSize / 2, buffSize / 2);
    ImageResize(&iconImg[2], buffSize / 2, buffSize / 2);
    ImageResize(&iconImg[3], buffSize / 2, buffSize / 2);
    ImageResize(&iconImg[4], buffSize / 2, buffSize / 2);
}

void LoadAllTexture()
{
    playerTex = LoadTextureFromImage(playerImg);

    for(int i = 0; i < 4; i++) 
    {
        zombieTex[i] = LoadTextureFromImage(zombieImg[i]);
    }  

    for(int i = 0; i < 5; i++)
    {
        weaponTex[i] = LoadTextureFromImage(weaponImg[i]);
    }

    sateliteTex = LoadTextureFromImage(sateliteImg);

    bgTex = LoadTextureFromImage(bgImg);

    for(int i = 0; i < 5; i++)
    {
        buffTex[i] = LoadTextureFromImage(buffImg[i]);
    }

    for(int i = 0; i < 5; i++)
    {
        iconTex[i] = LoadTextureFromImage(iconImg[i]);
    }
}

void UnloadAllImage()
{
    UnloadImage(playerImg);
    
    for(int i = 0; i < 4; i++)
    {
        UnloadImage(zombieImg[i]);
    }

    UnloadImage(weaponImg[0]);
    
    UnloadImage(sateliteImg);

    UnloadImage(bgImg);

    for(int i = 0; i < 5; i++)
    {
        UnloadImage(buffImg[i]);
    }
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

            // If hit, then reduce zombie health
            if (CheckCollisionCircles(projectileCircle.pos, projectileCircle.radius, zombieCircle.pos, zombieCircle.radius))
            {
                if(isBuffActive[0])
                {
                    zombieIt->health -= projectileIt->getDamage() * 2;
                }
                else
                {   
                    zombieIt->health -= projectileIt->getDamage();
                }

                // If health below zero, kill zombie
                if(zombieIt->health <= 0)
                {
                    // If a loot zombie died, spawn a lootbow
                    if(zombieIt->getType() == 1)
                    {
                        ZombieDrop(zombieIt->getPos(), zombieIt->getType());
                    }

                    // Gain score
                    int point = 0;
                    if(zombieIt->getType() == 0)
                    {
                        point = 1;
                    }
                    else if(zombieIt->getType() == 1)
                    {
                        point = 2;
                    }
                    else if(zombieIt->getType() == 2)
                    {
                        point = 50;
                    }
                    else if(zombieIt->getType() == 3)
                    {
                        point = 20;
                    }
                    score += point * zombiesLevel;
                
                    // Delete zombie
                    zombieIt = zombies.erase(zombieIt);
                }

                // Remove bullet
                projectileIt = std::find_if(projectiles.begin(), projectiles.end(), [projectileIt](const Projectile& p) { return &p == projectileIt.operator->(); });
                if (projectileIt != projectiles.end()) {
                    projectiles.erase(projectileIt);
                }

                // Debugging purpose
                // std::cout << "HIT" << '\n';
                
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

void WeaponUpdate(Vector2 playerPos, float rotation)
{
    auto it = curWeapon.begin();
    while(it != curWeapon.end())
    {
        it->update(playerPos, rotation);
        ++it;
    }
}

void WeaponDraw()
{
    auto it = curWeapon.begin();
    while(it != curWeapon.end())
    {
        it->draw();
        ++it;
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
            inGame = false;
            break;
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

void BuffesUpdate(Vector2 playerPos, float playerRadius)
{
    // Check for collision with player
    auto buffIt = lootBoxes.begin();
    Circle playerCircle = { playerPos, playerRadius / 2 };
    while(buffIt != lootBoxes.end())
    {
        Circle buffCircle = buffIt->getCircle();
        // If player collide with buff, take the buff
        if (CheckCollisionCircles(playerCircle.pos, playerCircle.radius, buffCircle.pos, buffCircle.radius))
        {
            // Activate the buff
            isBuffActive[buffIt->getType()] = true;

            // Turn on the counter
            buffCnt[buffIt->getType()] = 0;

            // Remove the buff
            buffIt = lootBoxes.erase(buffIt);
        }
        else
        {
            ++buffIt;
        }
    }

    // Activate buff
    // 0 - Infinity ammo buff
    if(isBuffActive[0])
    {
        buffCnt[0] += 1;
        if(buffCnt[0] >= buffDuration[0])
        {
            isBuffActive[0] = false;        
        }
    }
    // 1 - Satelite Upgrade
    if(isBuffActive[1])
    {
        // Upgrade satelite level by 1
        if(sateliteLevel < 10)
        {
            sateliteLevel += 1;
        }

        isBuffActive[1] = false;
    }
    // 2 - Weapon Upgrade
    if(isBuffActive[2])
    {
        // Increase weapon level by 1
        if(weaponLevel < 10)
        {
            weaponLevel += 1;
        }

        // If reaches level 10 and weapon tier is not maxed, upgrade weapon tier
        // and reset weapon level to 1
        if(weaponLevel >= 10 && weaponTier < 5)
        {
            // Reset weapon level and increase weapon tier
            weaponLevel = 1;
            weaponTier += 1;

            // Remove all current weapon
            while(curWeapon.size() > 0)
            {
                curWeapon.erase(curWeapon.begin());
            }
            
            // Tier 2 - Double short baguette
            if(weaponTier == 2)
            {
                curWeapon.push_back(weaponList[1]);
                curWeapon.push_back(weaponList[2]);
            }
            // Tier 3 - Baguette rifle
            else if(weaponTier == 3)
            {
                curWeapon.push_back(weaponList[3]);
            }
            // Tier 4 - Double baguette uzis
            else if(weaponTier == 4)
            {
                curWeapon.push_back(weaponList[4]);
                curWeapon.push_back(weaponList[5]);
            }
            // Tier 5 - Baguette machine gun
            else if(weaponTier == 5)
            {
                curWeapon.push_back(weaponList[6]);
            }
        }

        isBuffActive[2] = false;
    }
    // 3 - Slow Debuff
    if(isBuffActive[3])
    {
        buffCnt[3] += 1;
        if(buffCnt[3] >= buffDuration[3])
        {
            isBuffActive[3] = false;
        }
    }
    // 4 - Speed buff
    if(isBuffActive[4])
    {
        buffCnt[4] += 1;
        if(buffCnt[4] >= buffDuration[4])
        {
            isBuffActive[4] = false;
        }
    }
}

void BuffesDrawing()
{
    auto lootBoxIt = lootBoxes.begin();
    while (lootBoxIt != lootBoxes.end())
    {
        lootBoxIt->draw();
        ++lootBoxIt;
    }
}

void GameStart()
{
    // Mark the game as started
    inGame = true;

    // Clear all previous objects
    zombies.clear();
    curWeapon.clear();
    lootBoxes.clear();

    // Reset score
    score = 0;

    // Reset wave
    waveNum = 0;
    zombiesNum = 0;
    zombiesLevel = 1;

    // Reset weapon stat
    weaponTier = 1;
    weaponLevel = 1;

    // Reset satelite stat
    sateliteLevel = 0;

    // Give tier 1 weapon to player
    //curWeapon.push_back(weaponList[weaponTier]);
    curWeapon.push_back(weaponList[0]);
}

void GameOver()
{
    // Mark the game as finished
    inGame = false;

    // Clear all objects
    zombies.clear();
    curWeapon.clear();
    lootBoxes.clear();
    sateliteLevel = 0;

    // Clear all buff
    for(int i = 0; i < 5; i++)
    {
        isBuffActive[i] = false;
    }

    // Stay while enter key has yet to be pressed
    while(!IsKeyPressed(KEY_ENTER))
    {
        BeginDrawing();
            // Show game over title along with total score
            ClearBackground(BLACK);
            DrawText("GAME OVER", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 - 40, 60, WHITE);
            DrawText(TextFormat("LAST SCORE: %d", score), GetScreenHeight() / 2 - 150, GetScreenHeight() / 2 + 80, 30, WHITE);    
            DrawText("PRESS ENTER TO CONTINUE", GetScreenHeight() / 2 - 150, GetScreenHeight() / 2 + 120, 30, WHITE);    
        EndDrawing();
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
    // Only happen when the game started
    if(!inGame)
    {
        return;
    }

    // If all current zombies eliminated, start a new wave
    if(zombies.size() == 0)
    {
        waveNum += 1;
        zombiesNum += 5;

        // Increase enemies level by 1 every 50 waves. Up to level 10
        if(waveNum % 50 == 0 && zombiesLevel < 10)
        {
            zombiesLevel += 1;
        }

        // Spawn zombies in random location near player
        for(int i = 0; i < zombiesNum; i++)
        {
            Vector2 randomPos = GenerateRandomPos(playerPos);
            Zombie zombie(randomPos, zombieSize, 0);
            
            zombies.push_back(zombie);
        }

        // Spawn lootbox zombies based on current waves
        for(int i = 0; i < waveNum; i++)
        {
            Vector2 randomPos = GenerateRandomPos(playerPos);
            Zombie zombie(randomPos, zombieSize, 1);
            
            zombies.push_back(zombie);
        }

        // Spawn large zombie every five waves based on current waves
        if(waveNum % 5 == 0)
        {
            for(int i = 0; i < waveNum / 5; i++)
            {
                Vector2 randomPos = GenerateRandomPos(playerPos);
                Zombie zombie(randomPos, zombieSize * 2, 3);
            
                zombies.push_back(zombie);
            }
        }

        // Spawn boss every ten waves based on current waves
        if(waveNum % 10 == 0)
        {
            for(int i = 0; i < waveNum / 10; i++)
            {
                Vector2 randomPos = GenerateRandomPos(playerPos);
                Zombie zombie(randomPos, zombieSize, 2);
            
                zombies.push_back(zombie);
            }
        }
    }
}

void InGame(Player *player, Satelite *satelite)
{   
    // Manage the wave
    GameManager(player->getPos());

    // Update player
    player->update();

    // Update satelite
    satelite->update();

    // Update weapon
    WeaponUpdate(player->getPos(), player->getRotation());

    // Update zombie based on player position
    ZombiesUpdate(*player);

    // If player is dead, stop all actions and return
    if(!inGame)
    {
        return;
    }

    // Update buffes
    BuffesUpdate(player->getPos(), player->getSize());

    // Handle projectile
    ProjectilesHandling();

    // Update camera
    UpdateCamera(*player);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
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
        // Draw border line
        DrawRectangleLines(-6000, -4000, 12000, 8000, WHITE);

        BuffesDrawing();

        player->draw();

        WeaponDraw();

        satelite->draw();

        ZombiesDrawing();

        EndMode2D();
        //-----------------------------------------------------------------------------------------------

    /*
    if(!inGame)
    {
        continue;
    }
    */

    // Draw UI
    // Draw current score
    DrawText(TextFormat("Score: %d", score), 0, 0, 30, WHITE);

    // Draw wave status
    DrawText(TextFormat("Wave: %d", waveNum), 0, 40, 30, WHITE);

    // Draw enemies status
    DrawText(TextFormat("Remaining enemies: %d", zombies.size()), 0, 80, 30, WHITE);

    // Draw energy status
    DrawText(TextFormat("Energy: %d%", player->getSprintEnergy() / 9), 0, 120, 30, WHITE);
    if(player->getSprintEnergy() < 900 && !player->getIsSprinting())
    {
        DrawText(TextFormat("Recharging", player->getSprintEnergy() / 9), 220, 120, 30, WHITE);
    }
    else if(!player->getIsSprinting())
    {
        DrawText(TextFormat("Ready", player->getSprintEnergy() / 9), 220, 120, 30, WHITE);
    }

    // Draw weapon status
    auto weaponIt = curWeapon.begin();
    DrawText(TextFormat("Ammo: %d/%d%", weaponIt->getCurAmmo(), weaponIt->getMaxAmmo()), 0, 160, 30, WHITE);

    if(weaponIt->getIsReloading())
    {
        DrawText("RELOADING", 0, 200, 30, WHITE);
    }

    // Draw buff status
    // Unlimited ammo buff
    if(isBuffActive[0])
    {
        DrawTexturePro(buffTex[0], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {40, GetScreenHeight() - 40.0f, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText(TextFormat("%ds", (buffDuration[0] - buffCnt[0]) / 60), 25, GetScreenHeight() - 40, 15, WHITE);
    }

    // Slow debuff
    if(isBuffActive[3])
    {
        DrawTexturePro(buffTex[3], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {100, GetScreenHeight() - 40.0f, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText(TextFormat("%ds", (buffDuration[3] - buffCnt[3]) / 60), 85, GetScreenHeight() - 40, 15, WHITE);
    }

    // Unlimited energy buff
    if(isBuffActive[4])
    {
        DrawTexturePro(buffTex[4], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {160, GetScreenHeight() - 40.0f, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText(TextFormat("%ds", (buffDuration[4] - buffCnt[4]) / 60), 145, GetScreenHeight() - 40, 15, WHITE);
    }


    EndDrawing();
    //----------------------------------------------------------------------------------
}

void PrintTutorial()
{
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("BASIC", 10, 10, 60, WHITE); 
        DrawText("MOVEMENTS: WASD / ARROW KEYS", 10, 80, 40, WHITE); 
        DrawText("SPRINT: LEFT SHIFT / RIGHT SHIFT", 10, 150, 40, WHITE);    
        DrawText("SHOOT: LEFT MOUSE BUTTON", 10, 220, 40, WHITE); 
        DrawText("RELOAD: R", 10, 290, 40, WHITE); 
        DrawText("PRESS ENTER TO CONTINUE", 10, 360, 40, WHITE); 
    EndDrawing();
}

void PrintBuffList()
{
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("BUFF LIST", 10, 10, 60, WHITE);  
        DrawTexturePro(buffTex[0], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {50, 150, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText("INFINITY AMMO & DOUBLE DAMAGE", 100, 130, 40, WHITE);              
        DrawTexturePro(buffTex[1], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {50, 290, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText("ORBIT THE PLAYER, DEAL DAMAGE TO ENEMIES", 100, 270, 40, WHITE);             
        DrawTexturePro(buffTex[2], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {50, 430, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText("UPGRADE WEAPON LEVEL, CHANGE WEAPON EVERY TEN LEVELS", 100, 410, 40, WHITE);  
        DrawTexturePro(buffTex[3], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {50, 570, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText("SLOW ENEMIES MOVEMENT SPEED BY 50%", 100, 550, 40, WHITE);  
        DrawTexturePro(buffTex[4], Rectangle {0, 0, float(buffSize), float(buffSize)},
                       {50, 710, float(buffSize), float(buffSize)},
                       {float(buffSize / 2), float(buffSize / 2)}, 0, RAYWHITE);
        DrawText("SPRINT WITHOUT USING ENERGY", 100, 690, 40, WHITE);  
        DrawText("PRESS ENTER TO CONTINUE", 10, 800, 40, WHITE); 
    EndDrawing();
}

void PrintLevelList()
{
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("COLOR LIST FOR WEAPONS AND ENEMIES LEVELS", 10, 10, 60, WHITE); 
        DrawText("LEVEL 1 - NORMAL"                         , 10, 80, 40, level[1]); 
        DrawText("LEVEL 2 - RED"                            , 10, 150, 40, level[2]); 
        DrawText("LEVEL 3 - ORANGE"                         , 10, 220, 40, level[3]); 
        DrawText("LEVEL 4 - YELLOW"                         , 10, 290, 40, level[4]); 
        DrawText("LEVEL 5 - GREEN"                          , 10, 360, 40, level[5]); 
        DrawText("LEVEL 6 - DARK GREEN"                     , 10, 430, 40, level[6]); 
        DrawText("LEVEL 7 - DARK BLUE"                      , 10, 500, 40, level[7]); 
        DrawText("LEVEL 8 - BLUE"                           , 10, 570, 40, level[8]); 
        DrawText("LEVEL 9 - PURPLE"                         , 10, 640, 40, level[9]); 
        DrawText("LEVEL 10 - GOLD"                          , 10, 710, 40, level[10]);
        DrawText("PRESS ENTER TO RETURN"                    , 10, 780, 40, WHITE); 
    EndDrawing();
}

void PrintCredit()
{
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("DIBUAT OLEH:", GetScreenHeight() / 2 - 150, GetScreenHeight() / 2 - 80, 30, WHITE);    
        DrawText("MICHAEL / 5024231022 / PEMROGRAMAN LANJUT A", GetScreenHeight() / 2 - 150, GetScreenHeight() / 2 - 40, 30, WHITE);  
        DrawText("DOSEN PENGAJAR:", GetScreenHeight() / 2 - 150, GetScreenHeight() / 2, 30, WHITE);  
        DrawText("DR. ARIEF KURNIAWAN, S.T., M.T.", GetScreenHeight() / 2 - 150, GetScreenHeight() / 2 + 40, 30, WHITE);  
        DrawText("PRESS ENTER TO RETURN", GetScreenHeight() / 2 - 150, GetScreenHeight() / 2 + 80, 30, WHITE);
    EndDrawing();
}

enum TutorialState {
    TUTORIAL,
    BUFF_LIST,
    LEVEL_LIST,
    FINISHED
};

void HandleTutorial() {
    enum TutorialState currentState = TUTORIAL;

    while (currentState != FINISHED) {
        // Depending on the current state, print the corresponding tutorial
        switch (currentState) {
            case TUTORIAL:
                PrintTutorial();
                break;
            case BUFF_LIST:
                PrintBuffList();
                break;
            case LEVEL_LIST:
                PrintLevelList();
                break;
            case FINISHED:
                break;
        }

        // Check if the Enter key is pressed to advance to the next state
        if (IsKeyPressed(KEY_ENTER)) {
            switch (currentState) {
                case TUTORIAL:
                    currentState = BUFF_LIST;
                    break;
                case BUFF_LIST:
                    currentState = LEVEL_LIST;
                    break;
                case LEVEL_LIST:
                    currentState = FINISHED;
                    break;
                case FINISHED:
                    break;
            }
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
    ToggleFullscreen();

    // Initialize audio device
    InitAudioDevice();

    // World Position
    //std::cout << displayWidth << ' ' << displayHeight << '\n';
    //worldPos = Vector2{displayWidth / 2.0f, displayHeight / 2.0f};
    worldPos = Vector2{float(GetDisplayWidth()) / 2, float(GetDisplayHeight()) / 2};
    startingPos = worldPos;

    // Texture Setup
    TextureSetup();

    // Load all audios
    LoadAllSound();

    // Set Game FPS (frame per second)
    SetTargetFPS(60);

    // Spawn player
    Player player({ 0, 0 });
    
    // Spawn satelite
    Satelite satelite(&player);

    // Game loop
    while (!WindowShouldClose())        // While the window is still open
    {
        // Main menu
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("BLACK FRIDAY", GetScreenWidth() / 2 - 225, GetScreenHeight() / 2 - 80, 60, WHITE);
            DrawText("PRESS ENTER TO PLAY", GetScreenHeight() / 2 - 50, GetScreenHeight() / 2, 30, WHITE);    
            DrawText("PRESS T FOR TUTORIAL", GetScreenHeight() / 2 - 50, GetScreenHeight() / 2 + 40, 30, WHITE);  
            DrawText("PRESS C FOR CREDIT", GetScreenHeight() / 2 - 50, GetScreenHeight() / 2 + 80, 30, WHITE);  
        EndDrawing();

        // If player press enter button
        if(IsKeyPressed(KEY_ENTER))
        {
            // Setup the game
            GameStart();

            // Loop while the game is still going
            while(inGame)
            {
                InGame(&player, &satelite);
            }
            std::cout << "pass\n";

            // Print game over message
            GameOver();
        }

        // If player press T button
        if(IsKeyPressed(KEY_T))
        {
            HandleTutorial();
        }

        // If player press C button
        if(IsKeyPressed(KEY_C))
        {
            while(!IsKeyPressed(KEY_ENTER))
            {
                // Print credit
                PrintCredit();
            }
        }
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
