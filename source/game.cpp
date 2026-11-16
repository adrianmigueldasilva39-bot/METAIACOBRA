#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <cstdio>

Game::Game() : screen(NULL), running(false), gameState(0), cameraX(0), cameraY(0), zoom(1.0f), targetZoom(1.0f), frameCount(0) {}
Game::~Game() {}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    ioPadInit(7);
    screen = SDL_SetVideoMode(1280, 720, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) screen = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) return false;
    SDL_ShowCursor(SDL_DISABLE);
    srand(12345);
    initWorld();
    running = true;
    return true;
}

void Game::shutdown() { ioPadEnd(); SDL_Quit(); screen = NULL; }

float Game::dist(float x1, float y1, float x2, float y2) { float dx=x1-x2, dy=y1-y2; return sqrtf(dx*dx+dy*dy); }
float Game::angleTo(float x1, float y1, float x2, float y2) { return atan2f(y2-y1, x2-x1); }
float Game::normalizeAngle(float a) { while(a>3.14159f) a-=6.28318f; while(a<-3.14159f) a+=6.28318f; return a; }
bool Game::isInsideWorld(float x, float y) { return dist(x,y,0,0) < WORLD_RADIUS; }
Vec2 Game::worldToScreen(float wx, float wy) { return Vec2((wx - cameraX)*zoom + screen->w/2, (wy - cameraY)*zoom + screen->h/2); }
Uint32 Game::getColor(Uint8 r, Uint8 g, Uint8 b) { return SDL_MapRGB(screen->format, r,g,b); }
void Game::setPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b) { if(x<0||y<0||x>=screen->w||y>=screen->h) return; SDL_Rect rect={(Sint16)x,(Sint16)y,1,1}; SDL_FillRect(screen,&rect,getColor(r,g,b)); }

void Game::spawnSnake(Snake &s, bool isPlayer, float x, float y) {
    s.isPlayer=isPlayer; s.alive=true; s.angle=0; s.targetAngle=0; s.baseSpeed=120; s.boostSpeed=220; s.speed=120;
    s.radius=14; s.length=20; s.score=0; s.kills=0; s.boostTimer=0; s.boosting=false; s.turnSpeed=4.5f; s.aiTimer=0; s.aiFlee=false;
    s.colorR=rand()%200+55; s.colorG=rand()%200+55; s.colorB=rand()%200+55;
    s.colorR2=255-s.colorR; s.colorG2=255-s.colorG; s.colorB2=255-s.colorB;
    if(isPlayer){ s.colorR=0; s.colorG=255; s.colorB=120; s.colorR2=0; s.colorG2=200; s.colorB2=255; s.name="VOCE"; }
    else s.name="BOT";
    s.body.clear(); s.path.clear();
    for(int i=0;i<20;i++) s.body.push_back(Vec2(x - i*10, y));
}

void Game::spawnFood(int index, bool anywhere) {
    float a = (float)rand()/RAND_MAX * 6.28318f;
    float d = (float)rand()/RAND_MAX * (anywhere ? WORLD_RADIUS-100 : 800);
    if(anywhere){ foods[index].pos = Vec2(cos(a)*d, sin(a)*d); }
    else { foods[index].pos = Vec2(snakes[0].body[0].x + cos(a)*d, snakes[0].body[0].y + sin(a)*d); }
    foods[index].radius = 5 + rand()%5;
    foods[index].r = rand()%255; foods[index].g = rand()%255; foods[index].b = rand()%255;
    foods[index].pulse = (float)rand(); foods[index].active = true;
}

void Game::spawnParticle(float x, float y, Uint8 r, Uint8 g, Uint8 b) {
    for(int i=0;i<MAX_PARTICLES;i++) if(!particles[i].active){
        particles[i].active=true; particles[i].pos=Vec2(x,y);
        particles[i].vel=Vec2((rand()%100-50), (rand()%100-50));
        particles[i].life=1; particles[i].maxLife=1; particles[i].r=r; particles[i].g=g; particles[i].b=b; particles[i].size=3+rand()%4;
        break;
    }
}

void Game::initWorld() {
    for(int i=0;i<MAX_FOOD;i++) spawnFood(i,true);
    for(int i=0;i<MAX_PARTICLES;i++) particles[i].active=false;
    for(int i=0;i<MAX_SNAKES;i++){
        float a = (float)i/MAX_SNAKES*6.28318f;
        float d = 500 + (rand()%2000);
        spawnSnake(snakes[i], i==0, cos(a)*d, sin(a)*d);
    }
    cameraX = snakes[0].body[0].x; cameraY = snakes[0].body[0].y; gameState=0;
}

void Game::resetPlayer() {
    spawnSnake(snakes[0], true, 0, 0);
    cameraX=0; cameraY=0;
}

void Game::addScore(Snake &s, int amount){ s.score+=amount; s.length+=amount/10; if(s.length>200) s.length=200; }
void Game::eatFood(Snake &s, int foodIndex){ addScore(s,10); spawnParticle(foods[foodIndex].pos.x, foods[foodIndex].pos.y, foods[foodIndex].r, foods[foodIndex].g, foods[foodIndex].b); spawnFood(foodIndex,false); }
void Game::killSnake(int index, int killerIndex){
    if(!snakes[index].alive) return;
    snakes[index].alive=false;
    for(size_t i=0;i<snakes[index].body.size();i+=3) spawnParticle(snakes[index].body[i].x, snakes[index].body[i].y, snakes[index].colorR, snakes[index].colorG, snakes[index].colorB);
    if(killerIndex>=0 && killerIndex<MAX_SNAKES){ snakes[killerIndex].kills++; addScore(snakes[killerIndex],100); }
    if(index!=0){
        float a=(float)rand()/RAND_MAX*6.28318f; float d=500+(rand()%2500);
        spawnSnake(snakes[index], false, cos(a)*d, sin(a)*d);
    }
}

void Game::updatePlayer(Snake &s, float dt, float analogX, float analogY, bool boostBtn) {
    if(analogX!=0 || analogY!=0) s.targetAngle = atan2f(analogY, analogX);
    float diff = normalizeAngle(s.targetAngle - s.angle);
    s.angle += diff * s.turnSpeed * dt;
    s.boosting = boostBtn && s.boostTimer<=0;
    s.speed = s.boosting ? s.boostSpeed : s.baseSpeed;
    if(s.boosting){ s.boostTimer=0.1f; s.length-=0.05f; }
    if(s.boostTimer>0) s.boostTimer-=dt;
    float nx = s.body[0].x + cos(s.angle)*s.speed*dt;
    float ny = s.body[0].y + sin(s.angle)*s.speed*dt;
    if(!isInsideWorld(nx,ny)){ killSnake(0,-1); return; }
    s.body.insert(s.body.begin(), Vec2(nx,ny));
    while((int)s.body.size() > (int)s.length) s.body.pop_back();
}

void Game::updateBot(Snake &s, float dt) {
    s.aiTimer-=dt;
    if(s.aiTimer<=0){
        s.aiTimer=1+ (rand()%3);
        float a=(float)rand()/RAND_MAX*6.28318f; float d=300+(rand()%1000);
        s.aiTarget=Vec2(s.body[0].x + cos(a)*d, s.body[0].y + sin(a)*d);
        if(!isInsideWorld(s.aiTarget.x,s.aiTarget.y)) s.aiTarget=Vec2(0,0);
    }
    s.targetAngle = angleTo(s.body[0].x, s.body[0].y, s.aiTarget.x, s.aiTarget.y);
    float diff=normalizeAngle(s.targetAngle - s.angle);
    s.angle += diff * s.turnSpeed * dt;
    s.speed=s.baseSpeed;
    float nx=s.body[0].x + cos(s.angle)*s.speed*dt;
    float ny=s.body[0].y + sin(s.angle)*s.speed*dt;
    if(!isInsideWorld(nx,ny)){ s.aiTarget=Vec2(0,0); return; }
    s.body.insert(s.body.begin(), Vec2(nx,ny));
    while((int)s.body.size() > (int)s.length) s.body.pop_back();
}

void Game::updateFoods(float dt){ for(int i=0;i<MAX_FOOD;i++) if(foods[i].active) foods[i].pulse+=dt*5; }
void Game::updateParticles(float dt){ for(int i=0;i<MAX_PARTICLES;i++) if(particles[i].active){ particles[i].pos.x+=particles[i].vel.x*dt; particles[i].pos.y+=particles[i].vel.y*dt; particles[i].life-=dt; if(particles[i].life<=0) particles[i].active=false; } }
void Game::updateCamera(float dt){ if(snakes[0].body.empty()) return; cameraX += (snakes[0].body[0].x - cameraX)*dt*5; cameraY += (snakes[0].body[0].y - cameraY)*dt*5; zoom += (targetZoom - zoom)*dt*3; }

void Game::checkCollisions() {
    for(int i=0;i<MAX_SNAKES;i++) if(snakes[i].alive && !snakes[i].body.empty()){
        // Comida
        for(int f=0;f<MAX_FOOD;f++) if(foods[f].active){
            if(dist(snakes[i].body[0].x, snakes[i].body[0].y, foods[f].pos.x, foods[f].pos.y) < snakes[i].radius + foods[f].radius) eatFood(snakes[i], f);
        }
        // Bateu em outra cobra
        for(int j=0;j<MAX_SNAKES;j++) if(i!=j && snakes[j].alive){
            for(size_t k=1;k<snakes[j].body.size();k+=2){
                if(dist(snakes[i].body[0].x, snakes[i].body[0].y, snakes[j].body[k].x, snakes[j].body[k].y) < snakes[i].radius){
                    killSnake(i,j); break;
                }
            }
        }
    }
}

void Game::update(float dt) {
    for(int i=1;i<MAX_SNAKES;i++) if(snakes[i].alive) updateBot(snakes[i], dt);
    updateFoods(dt); updateParticles(dt); updateCamera(dt); checkCollisions();
}

void Game::run() {
    padInfo padinfo; padData paddata;
    bool startLast=false;
    while(running){
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        float ax=0, ay=0; bool boost=false, start=false, select=false;
        if(padinfo.status[0]){
            ioPadGetData(0,&paddata);
            ax=((float)paddata.ANA_L_H-128)/128.0f; ay=((float)paddata.ANA_L_V-128)/128.0f;
            if(ax>-0.2f && ax<0.2f) ax=0; if(ay>-0.2f && ay<0.2f) ay=0;
            if(ax==0 && ay==0){ if(paddata.BTN_LEFT) ax=-1; if(paddata.BTN_RIGHT) ax=1; if(paddata.BTN_UP) ay=-1; if(paddata.BTN_DOWN) ay=1; }
            if(paddata.BTN_R1 || paddata.BTN_CROSS) boost=true;
            if(paddata.BTN_START) start=true;
            if(paddata.BTN_SELECT) select=true;
        }
        if(select) break;
        if(gameState==0){ if(start && !startLast){ resetPlayer(); gameState=1; } }
        else if(gameState==2){ if(start && !startLast){ initWorld(); gameState=1; } }
        else if(gameState==1){
            float dt=1.0f/60.0f;
            if(snakes[0].alive) updatePlayer(snakes[0], dt, ax, ay, boost);
            update(dt);
            if(!snakes[0].alive) gameState=2;
        }
        startLast=start;
        render();
        frameCount++;
        SDL_Delay(16);
    }
}