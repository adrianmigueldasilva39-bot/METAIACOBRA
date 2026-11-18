#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <vector>
#include <cmath>
#include <cstdlib>

#define GRID_SIZE 20       // Tamanho do tabuleiro (20x20 blocos)
#define CELL_SIZE 20.0f    // Tamanho de cada quadrado no mundo
#define MAX_SNAKES 4

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

struct GridPos {
    int x, y;
    bool operator==(const GridPos& o) const { return x == o.x && y == o.y; }
};

struct Snake {
    bool isPlayer;
    bool alive;
    std::vector<GridPos> body;
    int dirX, dirY;
    float moveTimer;
    float moveSpeed;
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
    int gameState; // 1 = Jogando, 2 = Game Over
    
    Snake player;
    Snake bots[MAX_SNAKES-1];
    GridPos apple;
    
    int score;
    int record;

    void initWorld();
    void update(float dt);
    void updateSnake(Snake& s, float dt, bool isPlayer);
    void checkCollisions();
    void spawnApple();

    void render();
    void drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, int scale);
    Uint32 getColor(Uint8 r, Uint8 g, Uint8 b);
    Vec2 worldToScreen(float wx, float wy);
};

#endif