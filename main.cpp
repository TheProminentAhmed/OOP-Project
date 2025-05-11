#include <iostream>
#include "raylib.h"
#include <vector>
#include <algorithm>
#include <cstdlib>   
#include <ctime>     
using namespace std;
class Ball {
public:
    float x, y;
    int radius;
    float speed_x, speed_y;
    Color color;
    int fireEffectTimer;
    float rotation;

    RenderTexture2D renderTexture;
    Texture2D ballTexture;

    Ball() : radius(20), speed_x(0), speed_y(0), color(WHITE), fireEffectTimer(0), rotation(0.0f) {}

    void Init() {
     
        renderTexture = LoadRenderTexture(radius * 2, radius * 2);
        
        BeginTextureMode(renderTexture);
        ClearBackground(BLANK);

       
        DrawCircle(radius, radius, radius, WHITE);

        
        int stripeWidth = radius / 2; 
        int stripeX = radius - stripeWidth / 2; 

        DrawRectangle(stripeX, 0, stripeWidth, radius * 2, BLACK);

        EndTextureMode();

        
        ballTexture = renderTexture.texture;
    }

    void Draw() {
      
        Vector2 pos = { x - radius, y - radius };
        DrawTexture(ballTexture, pos.x, pos.y, color);
    }

    void Update() {
        x += speed_x;
        y += speed_y;
        if (y < 0 || y > GetScreenHeight())
            speed_y *= -1;

        if (fireEffectTimer > 0) {
            fireEffectTimer--;
            if (fireEffectTimer == 0) {
                color = WHITE;
            }
        }
    }

    void Reset() {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;
        speed_x = (GetRandomValue(0, 1) == 0 ? -1 : 1) * 7;
        speed_y = (GetRandomValue(0, 1) == 0 ? -1 : 1) * 5;
        color = WHITE;
        fireEffectTimer = 0;
        rotation = 0.0f;
    }

    void ActivateFireEffect() {
        color = ORANGE;
        fireEffectTimer = 5 * 60;
    }

    void Unload() {
        UnloadRenderTexture(renderTexture);
    }
};

class Paddle {
public:
    float x, y;
    float width, height;
    int speed;
    Color color;

    Paddle() : x(0), y(0), width(25), height(120), speed(8), color(WHITE) {}

    void Draw() {
        DrawRectangle(x, y, width, height, color);
    }

    void Update(int upKey, int downKey) {
        if (IsKeyDown(upKey)) y -= speed;
        if (IsKeyDown(downKey)) y += speed;
        if (y < 0) y = 0;
        if (y + height > GetScreenHeight()) y = GetScreenHeight() - height;
    }

    void CPUUpdate(Ball ball) {
        if (ball.y < y + height / 2) y -= speed;
        if (ball.y > y + height / 2) y += speed;
        if (y < 0) y = 0;
        if (y + height > GetScreenHeight()) y = GetScreenHeight() - height;
    }
};

class PowerUp {
public:
    float x, y;
    int width, height;
    int speed_x, speed_y;
    int type;

    PowerUp() : width(40), height(40), speed_x(1), speed_y(1), type(0) {}

    void Draw() {
        Color c = (type == 0) ? RED : ORANGE;
        DrawRectangle(x, y, width, height, c);
    }

    void Update() {
        x += speed_x;
        y += speed_y;
        if (y <= 0 || y + height >= GetScreenHeight()) speed_y *= -1;
        if (x <= 0 || x + width >= GetScreenWidth()) speed_x *= -1;
    }
};

Ball ball;
Paddle player1, player2;
vector<PowerUp> powerUps;
int player1_score = 0, player2_score = 0;
int powerUpSpawnCooldown = 8 * 60;
bool isPowerUpActiveP1 = false, isPowerUpActiveP2 = false;
int powerUpEffectTimerP1 = 0, powerUpEffectTimerP2 = 0;
bool isTwoPlayer = false;

void DrawFootballPitch() {
    ClearBackground((Color){0, 128, 0, 255});

    int margin = 20;
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    DrawRectangleLines(margin, margin, w - 2 * margin, h - 2 * margin, WHITE);
    DrawLine(w / 2, margin, w / 2, h - margin, WHITE);
    DrawCircle(w / 2, h / 2, 60, WHITE);
    DrawCircle(w / 2, h / 2, 5, WHITE);

    DrawRectangleLines(margin, h / 2 - 100, 60, 200, WHITE);
    DrawRectangleLines(w - margin - 60, h / 2 - 100, 60, 200, WHITE);
    DrawRectangleLines(margin, h / 2 - 40, 20, 80, WHITE);
    DrawRectangleLines(w - margin - 20, h / 2 - 40, 20, 80, WHITE);
}

void DrawMenu() {
    DrawFootballPitch();
    DrawText("PONG MENU", 300, 100, 40, WHITE);
    DrawText("1. Single Player", 300, 200, 30, LIGHTGRAY);
    DrawText("2. Two Player", 300, 250, 30, LIGHTGRAY);
    DrawText("Press 1 or 2 to start", 300, 350, 20, GRAY);
}

void DrawGame() {
    DrawFootballPitch();
    ball.Draw();
    player1.Draw();
    player2.Draw();

    for (auto& p : powerUps) p.Draw();

    DrawText(TextFormat("%i", player1_score), GetScreenWidth() / 4, 20, 80, WHITE);
    DrawText(TextFormat("%i", player2_score), 3 * GetScreenWidth() / 4, 20, 80, WHITE);
}

void UpdateGame() {
    ball.Update();

    player1.Update(KEY_UP, KEY_DOWN);

    if (isTwoPlayer)
        player2.Update(KEY_W, KEY_S);
    else
        player2.CPUUpdate(ball);

    Rectangle p1Rect = { player1.x, player1.y, player1.width, player1.height };
    Rectangle p2Rect = { player2.x, player2.y, player2.width, player2.height };

    if (CheckCollisionCircleRec({ ball.x, ball.y }, ball.radius, p1Rect)) {
        ball.speed_x *= -1;
        ball.x = player1.x - ball.radius;

       
        int nudge = GetRandomValue(-3, 3);
        ball.speed_y += nudge;
 
        if (ball.speed_y > -1 && ball.speed_y < 1) {
            ball.speed_y = (ball.speed_y < 0) ? -2 : 2;
        }
    }

    if (CheckCollisionCircleRec({ ball.x, ball.y }, ball.radius, p2Rect)) {
        ball.speed_x *= -1;
        ball.x = player2.x + player2.width + ball.radius;

        int nudge = GetRandomValue(-3, 3);
        ball.speed_y += nudge;
        if (ball.speed_y > -1 && ball.speed_y < 1) {
            ball.speed_y = (ball.speed_y < 0) ? -2 : 2;
        }
    }

    if (ball.x < 0) {
        player2_score++;
        ball.Reset();
    }

    if (ball.x > GetScreenWidth()) {
        player1_score++;
        ball.Reset();
    }

    for (auto& p : powerUps) p.Update();

    if (powerUpSpawnCooldown <= 0 && powerUps.empty()) {
        PowerUp p;
        p.width = 40;
        p.height = 40; 
        p.x = GetRandomValue(100, GetScreenWidth() - 100 - p.width);
        p.y = GetRandomValue(50, GetScreenHeight() - 50 - p.height);
        p.speed_x = GetRandomValue(-2, 2);
        if (p.speed_x == 0) p.speed_x = 1;
        p.speed_y = GetRandomValue(-2, 2);
        if (p.speed_y == 0) p.speed_y = 1;
        p.type = 0;
        powerUps.push_back(p);
        powerUpSpawnCooldown = 8 * 60;
    }
    else {
        powerUpSpawnCooldown--;
    }

    for (auto it = powerUps.begin(); it != powerUps.end();) {
        bool collected = false;
        if (CheckCollisionRecs({ player1.x, player1.y, player1.width, player1.height },
            { it->x, it->y, it->width, it->height }) && !isPowerUpActiveP1) {
            player1.height *= 1.5f;
            isPowerUpActiveP1 = true;
            powerUpEffectTimerP1 = 5 * 60;
            collected = true;
        }
        if (CheckCollisionRecs({ player2.x, player2.y, player2.width, player2.height },
            { it->x, it->y, it->width, it->height }) && !isPowerUpActiveP2) {
            player2.height *= 1.5f;
            isPowerUpActiveP2 = true;
            powerUpEffectTimerP2 = 5 * 60;
            collected = true;
        }
        if (collected) it = powerUps.erase(it);
        else ++it;
    }

    for (auto it = powerUps.begin(); it != powerUps.end();) {
        bool collected = false;
        if (CheckCollisionCircleRec({ ball.x, ball.y }, ball.radius,
            { it->x, it->y, it->width, it->height })) {
            const float speedIncrement = 4.0f;
            const float speedMax = 30.0f;

            if (ball.speed_x < 0)
                ball.speed_x = max(ball.speed_x - speedIncrement, -speedMax);
            else
                ball.speed_x = min(ball.speed_x + speedIncrement, speedMax);

            if (ball.speed_y < 0)
                ball.speed_y = max(ball.speed_y - speedIncrement, -speedMax);
            else
                ball.speed_y = min(ball.speed_y + speedIncrement, speedMax);

            ball.ActivateFireEffect();

            collected = true;
        }
        if (collected) it = powerUps.erase(it);
        else ++it;
    }

    if (isPowerUpActiveP1 && --powerUpEffectTimerP1 <= 0) {
        player1.height /= 1.5f;
        isPowerUpActiveP1 = false;
    }
    if (isPowerUpActiveP2 && --powerUpEffectTimerP2 <= 0) {
        player2.height /= 1.5f;
        isPowerUpActiveP2 = false;
    }
}

int main() {
    InitWindow(800, 600, "Football Themed Pong with PowerUps");
    ball.Init();

    SetTargetFPS(60);

    ball.Reset();

    player1.width = player2.width = 25;
    player1.height = player2.height = 120;

    player1.x = GetScreenWidth() - player1.width - 10;
    player2.x = 10;

    player1.y = player2.y = GetScreenHeight() / 2 - player1.height / 2;
    player1.speed = player2.speed = 8;

    player1.color = BLUE;
    player2.color = RED;

    bool gameStarted = false;

    srand(time(nullptr));

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (!gameStarted) {
            DrawFootballPitch();
            DrawText("PONG MENU", 300, 100, 40, WHITE);
            DrawText("1. Single Player", 300, 200, 30, LIGHTGRAY);
            DrawText("2. Two Player", 300, 250, 30, LIGHTGRAY);
            DrawText("Press 1 or 2 to start", 300, 350, 20, GRAY);

            if (IsKeyPressed(KEY_ONE)) {
                isTwoPlayer = false;
                gameStarted = true;
            }
            if (IsKeyPressed(KEY_TWO)) {
                isTwoPlayer = true;
                gameStarted = true;
            }
        }
        else {
            UpdateGame();
            DrawGame();
        }

        EndDrawing();
    }
    ball.Unload();

    CloseWindow();
    return 0;
}
