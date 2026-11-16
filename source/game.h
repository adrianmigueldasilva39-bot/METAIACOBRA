#ifndef GAME_H
#define GAME_H
#include <SDL/SDL.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#define SCREEN_W 1280
#define SCREEN_H 720
#define WORLD_RADIUS 4000.0f
#define MAX_SNAKES 26
#define MAX_FOOD 400
#define MAX_PARTICLES 200
extern volatile bool g_exitRequested;
struct Vec2 { float x; float y; Vec2():x(0),y(0){} Vec2(float a,float b):x(a),y(b){} };
struct Food { Vec2 pos; float radius; Uint8 r,g,b; float pulse; bool active; };
struct Particle { Vec2 pos; Vec2 vel; float life; bool active; Uint8 r,g,b; float size; };
struct Snake { bool isPlayer; bool alive; std::vector<Vec2> path; std::vector<Vec2> body; float angle; float targetAngle; float speed; float baseSpeed; float boostSpeed; float radius; float length; int score; int kills; Uint8 cr,cg,cb; Uint8 cr2,cg2,cb2; float boostTimer; bool boosting; float turnSpeed; int aiTimer; Vec2 aiTarget; std::string name; };
class Game {
public: Game(); ~Game(); bool init(); void run(); void shutdown();
private:
SDL_Surface* screen; bool running; bool sdlUp; int gameState; float cameraX; float cameraY; float zoom; int frameCount;
Snake snakes[MAX_SNAKES]; Food foods[MAX_FOOD]; Particle particles[MAX_PARTICLES];
int rankOrder[MAX_SNAKES]; int rankCount; int playerRank; int bestScore; int deathScore; int deathLength; int deathKills; int deathRank;
void initWorld(); void resetPlayer(); void spawnSnake(Snake &s,bool isPlayer,float x,float y); void spawnFood(int i); void spawnParticle(float x,float y,Uint8 r,Uint8 g,Uint8 b);
void updatePlayer(Snake &s,float dt,float ax,float ay,bool boost); void updateBot(Snake &s,float dt); void updateFoods(float dt); void updateParticles(float dt); void updateCamera(float dt); void updateRanking(); void checkCollisions(); void eatFood(Snake &s,int idx); void killSnake(int idx,int killer); void addScore(Snake &s,int v);
void render(); void drawBackground(); void drawFood(); void drawSnake(Snake &s); void drawParticles(); void drawMinimap(); void drawHUD(); void drawRanking(); void drawMenu(); void drawDeath();
void drawFilledCircle(int cx,int cy,int rad,Uint8 r,Uint8 g,Uint8 b); void fillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b); void setPixel(int x,int y,Uint8 r,Uint8 g,Uint8 b);
void drawText(const char* t,int x,int y,int scale,Uint8 r,Uint8 g,Uint8 b); void drawTextCentered(const char* t,int y,int scale,Uint8 r,Uint8 g,Uint8 b); void drawChar(int x,int y,char c,int scale,Uint8 r,Uint8 g,Uint8 b); int textWidth(const char* t,int scale);
float dist(float x1,float y1,float x2,float y2); float angleTo(float x1,float y1,float x2,float y2); float normAngle(float a); Vec2 worldToScreen(float wx,float wy); bool insideWorld(float x,float y);
};
#endif