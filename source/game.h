#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <vector>
#include <string>

#define SCREEN_W 1280
#define SCREEN_H 720
#define WORLD_RADIUS 3200.0f
#define MAX_SNAKES 14
#define MAX_FOOD 360
#define MAX_PARTICLES 160
#define SEG_DIST 8.0f
#define RANK_SHOW 8
#define START_LEN 20

extern volatile bool g_exitRequested;

struct Vec2 {
    float x; float y;
    Vec2() : x(0), y(0) {}
    Vec2(float a, float b) : x(a), y(b) {}
};

struct Food {
    Vec2 pos; float radius; Uint8 r, g, b; bool active;
};

struct Particle {
    Vec2 pos; Vec2 vel; float life; Uint8 r, g, b; float size; bool active;
};

struct Snake {
    bool alive;
    bool isPlayer;
    bool boosting;
    std::vector<Vec2> body;
    float angle;
    float speed;
    float turnRate;
    float radius;
    float growPending;
    float boostAcc;
    int score;
    int kills;
    int aiTimer;
    Vec2 aiTarget;
    Uint8 r1, g1, b1, r2, g2, b2;
    std::string name;
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
    bool sdlUp;
    int gameState;
    float camX, camY, zoom;
    int frame;
    Snake snakes[MAX_SNAKES];
    Food foods[MAX_FOOD];
    Particle parts[MAX_PARTICLES];
    int rankOrder[MAX_SNAKES];
    int rankCount;
    int playerRank;
    int bestScore;
    int deathScore, deathLen, deathKills, deathRank;

    void startGame();
    void initWorld();
    void spawnSnake(int i, bool isPlayer, float x, float y);
    void spawnFood(int i);
    void dropFood(Vec2 p, float radius, Uint8 r, Uint8 g, Uint8 b);
    void spawnParticle(float x, float y, Uint8 r, Uint8 g, Uint8 b);
    void moveHead(Snake &s, float dt, float targetAngle, bool boost);
    void followBody(Snake &s);
    void updatePlayer(float dt, float ax, float ay, bool boost);
    void updateBot(int i, float dt);
    void updateWorld(float dt, float ax, float ay, bool boost);
    void killSnake(int idx, int killer);
    void checkCollisions();
    void updateParticles(float dt);
    void updateCamera(float dt);
    void updateRanking();

    void render();
    void renderMenu();
    void drawWorld();
    void drawBackground();
    void drawBorder();
    void drawFood();
    void drawSnake(Snake &s);
    void drawParticles();
    void drawMinimap();
    void drawRanking();
    void drawRankRow(int x, int y, int boxW, int pos, Snake &s);
    void drawHUD();
    void drawDeathPanel();
    void fillRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
    void fillCircle(int cx, int cy, int r, Uint8 R, Uint8 G, Uint8 B);
    void drawChar(int x, int y, char c, int scale, Uint8 r, Uint8 g, Uint8 b);
    void drawText(const char* text, int x, int y, int scale, Uint8 r, Uint8 g, Uint8 b);
    void drawTextCentered(const char* text, int y, int scale, Uint8 r, Uint8 g, Uint8 b);
    int textWidth(const char* text, int scale);
    Vec2 worldToScreen(float wx, float wy);
};
#endif