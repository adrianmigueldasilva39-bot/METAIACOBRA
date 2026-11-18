#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <deque>

#define WORLD_RADIUS 2500.0f
#define MAX_SNAKES 15
#define MAX_FOOD 600

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

struct Food {
    Vec2 pos;
    float radius;
    Uint8 r, g, b;
    float pulse;
    bool active;
};

struct Snake {
    bool isPlayer;
    bool alive;
    std::deque<Vec2> body;
    float angle;
    float speed;
    float radius;
    int score;
    int length;
    Uint8 r, g, b;
    float aiTimer;
    Vec2 aiTarget;
};

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();
    void shutdown();

private:
    SDL_Surface* screen;
    bool running;
    int gameState;
    float cameraX, cameraY;
    float zoom, targetZoom;

    Snake snakes[MAX_SNAKES];
    Food foods[MAX_FOOD];

    void initWorld();
    void spawnSnake(Snake& s, bool isPlayer, float x, float y);
    void spawnFood(int index);
    void update(float dt);
    void updatePlayer(Snake& s, float dt, float ax, float ay, bool boost);
    void updateBot(Snake& s, float dt);
    void checkCollisions();

    void render();
    void drawBackground();
    void drawBorder();
    void drawFood();
    void drawSnake(Snake& s);
    void drawMinimap();
    void drawHUD();
    
    void drawFilledCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b);
    void drawGlowCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b);
    void drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, int scale);
    Vec2 worldToScreen(float wx, float wy);
    float dist(float x1, float y1, float x2, float y2);
    float angleTo(float x1, float y1, float x2, float y2);
    float normalizeAngle(float a);
    Uint32 getColor(Uint8 r, Uint8 g, Uint8 b);
};

#endif