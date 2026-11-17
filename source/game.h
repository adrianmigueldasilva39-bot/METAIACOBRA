#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <string>

#define WORLD_RADIUS 6000.0f
#define MAX_SNAKES 21
#define MAX_FOOD 900
#define MAX_PARTICLES 400

struct Vec2 {
    float x;
    float y;
    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

struct Food {
    Vec2 pos;
    float radius;
    Uint8 r, g, b;
    float pulse;
    bool active;
    bool big;
};

struct Particle {
    Vec2 pos;
    Vec2 vel;
    float life;
    float maxLife;
    Uint8 r, g, b;
    float size;
    bool active;
};

struct Snake {
    bool isPlayer;
    bool alive;
    std::vector<Vec2> body;
    float angle;
    float targetAngle;
    float speed;
    float baseSpeed;
    float boostSpeed;
    float radius;
    float length;
    int score;
    int kills;
    int skin;
    int skinLen;
    Uint8 cols[10][3];
    float boostTimer;
    bool boosting;
    float turnSpeed;
    float aiTimer;
    Vec2 aiTarget;
    std::string name;
};

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();
    void shutdown(); // <-- ADICIONADO AQUI

private:
    SDL_Surface* screen;
    bool running;
    int gameState;
    float cameraX;
    float cameraY;
    float zoom;
    float targetZoom;
    int frameCount;
    float deathTimer;
    bool xPressedLast;

    Snake snakes[MAX_SNAKES];
    Food foods[MAX_FOOD];
    Particle particles[MAX_PARTICLES];

    void initWorld();
    void resetPlayer();
    void setSkin(Snake& s, int skin);
    void spawnSnake(Snake& s, bool isPlayer, float x, float y);
    void spawnFood(int index, bool anywhere);
    void spawnParticle(float x, float y, Uint8 r, Uint8 g, Uint8 b);
    void spawnKillOrbs(Snake& s);
    void update(float dt);
    void updatePlayer(Snake& s, float dt, float ax, float ay, bool boostBtn);
    void updateBot(Snake& s, float dt);
    void updateFoods(float dt);
    void updateParticles(float dt);
    void updateCamera(float dt);
    void checkCollisions();
    void eatFood(Snake& s, int index);
    void killSnake(int index, int killerIndex);
    void addScore(Snake& s, int amount);
    
    // Faltava declarar essa função aqui também (usada no spawn e no update)
    void rebuildBodyFromPath(Snake& s);

    void render();
    void renderGameOver();
    void drawBackground();
    void drawBorder();
    void drawFood();
    void drawSnake(Snake& s);
    void drawParticles();
    void drawMinimap();
    void drawHUD();
    void drawLeaderboard();
    void drawGlowCircle(int sx, int sy, int radius, Uint8 r, Uint8 g, Uint8 b);
    void drawFilledCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b);
    void drawCircleOutline(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b);
    void drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, bool big);

    float dist(float x1, float y1, float x2, float y2);
    float angleTo(float x1, float y1, float x2, float y2);
    float normalizeAngle(float a);
    Vec2 worldToScreen(float wx, float wy);
    bool isInsideWorld(float x, float y);
    Uint32 getColor(Uint8 r, Uint8 g, Uint8 b);
};

#endif