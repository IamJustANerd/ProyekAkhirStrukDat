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

const float phi = 3.14;
Image earthImg;
Image moonImg;
Texture2D earthTex;
Texture2D moonTex;

class Benda {
    protected:
        int x, y;
        int speedX, speedY;
        float radius;
    public:
    Benda(int _x, int _y, int _speedX, int _speedY, float _radius) {
        x = _x;
        y = _y;
        speedX = _speedX;
        speedY = _speedY;
        radius = _radius;
    }

    virtual void update() = 0;
    virtual void draw() = 0;
    int getX() { return x; }
    int getY() { return y; }
    float getRadius() { return radius; }
};

class Bola : public Benda {
    private:
        Vector2 position;
        Color color;
    
    public:
        Bola(int  _x, int _y, int _speedX, int _speedY, int _radius) : Benda(_x, _y, _speedX, _speedY, _radius) {
            x = _x;
            y = _y;
            speedX = _speedX;
            speedY = _speedY;
            radius = _radius;
        }

        void update() override {
            x += speedX;
            y += speedY;

            if (x < radius || x > GetScreenWidth() - radius) {
                speedX = -speedX;
            }
            if (y < radius || y > GetScreenHeight() - radius) {
                speedY = -speedY;
            }
        }

        void draw() override {
            position.x = x;
            position.y = y;
            //DrawCircle(position.x, position.y, radius, color);
            DrawTexture(earthTex, position.x - radius, position.y - radius, WHITE);
        }

        void set_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
            color = {r, g, b, a};
        }

        
};

class Satelit : public Benda {
    private:
        Vector2 position;
        Color color;
        Bola* parent;
        int dist;
        int angle = 0;

    public:
        Satelit(int  _x, int _y, int _speedX, int _speedY, int _radius, Bola* _parent) : Benda(_x, _y, _speedX, _speedY, _radius) {
            x = _x;
            y = _y;
            speedX = _speedX;
            speedY = _speedY;
            radius = _radius;
            parent = _parent;
            dist = abs(radius * 2 + _parent->getRadius());
        }

        void update() override {
            // Buat satelit mengorbit parent / induknya
            position.x = parent->getX() + dist*cos(angle * phi / 180);
            position.y = parent->getY() - dist*sin(angle * phi / 180);
            angle += 2;
            if(angle >= 360)
            {
                angle = 0;
            }
        }

        void draw() override {
            //DrawCircle(position.x, position.y, radius, WHITE);
            DrawTexture(moonTex, position.x - radius, position.y - radius, WHITE);
        }
};

void LoadAllImage() {
    earthImg = LoadImage("../graphics/earth.png");
    moonImg = LoadImage("../graphics/moon.png");
}

void ResizeAllImage() {
    ImageResize(&earthImg, 80, 80);
    ImageResize(&moonImg, 40, 40);
}

void LoadAllTexture() {
    earthTex = LoadTextureFromImage(earthImg);
    moonTex = LoadTextureFromImage(moonImg);
}

void UnloadAllImage() {
    UnloadImage(earthImg);
    UnloadImage(moonImg);
}

void Setup()
{
    // Set image yang dipakai
    // 1. Load all images
    LoadAllImage();

    // 2. Resize all images
    ResizeAllImage();

    // 3. Load all textures from images
    LoadAllTexture();
}

int main() {
    // Inisialisasi jendela Raylib
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Raylib Bola Pantul");

    // Setup
    Setup();

    // Membuat objek Bola dan Satelit
    Bola earth(400, 225, 5, 4, 40);
    Satelit moon(480, 305, 5, 4, 20, &earth);

    // Ubah earth menjadi hijau
    earth.set_color(0, 228, 48, 255);

    // Mengatur FPS (frame per second)
    SetTargetFPS(60);

    // Game loop
    while (!WindowShouldClose()) { // Deteksi tombol close window atau ESC key
        // Update
        earth.update();
        moon.update();

        // Draw
        BeginDrawing();
            ClearBackground(BLACK);
            earth.draw();
            moon.draw();
        EndDrawing();
    }

    // De-Initialization
    CloseWindow(); // Close window and OpenGL context
    
    UnloadAllImage();

    return 0;
}
