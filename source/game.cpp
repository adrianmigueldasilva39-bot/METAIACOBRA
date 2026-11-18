#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <cstdio>
#include <ctime>

Game::Game() {
    screen = NULL;
    running = false;
    gameState = 1;
    cameraX = 0; cameraY = 0;
    zoom = 0.8f; targetZoom = 0.8f;
}

Game::~Game() {}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    ioPadInit(7);
    screen = SDL_SetVideoMode(1280, 720, 32, SDL_SWSURFACE | SDL_FULLSCREEN);
    if (!screen) return false;
    
    SDL_ShowCursor(SDL_DISABLE);
    srand(time(NULL));
    initWorld();
    running = true;
    return true;
}

void Game::shutdown() {
    ioPadEnd();
    SDL_Quit();
    screen = NULL;
}

Uint32 Game::getColor(Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(screen->format, r, g, b);
}

float Game::dist(float x1, float y1, float x2, float y2) {
    return sqrtf((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

float Game::angleTo(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

float Game::normalizeAngle(float a) {
    while (a > 3.14159f) a -= 6.28318f;
    while (a < -3.14159f) a += 6.28318f;
    return a;
}

Vec2 Game::worldToScreen(float wx, float wy) {
    return Vec2((wx - cameraX) * zoom + screen->w / 2, (wy - cameraY) * zoom + screen->h / 2);
}

void Game::initWorld() {
    for (int i = 0; i < MAX_FOOD; i++) spawnFood(i);
    for (int i = 0; i < MAX_SNAKES; i++) {
        float a = (float)i / MAX_SNAKES * 6.28318f;
        spawnSnake(snakes[i], i == 0, cosf(a) * 1500.0f, sinf(a) * 1500.0f);
        
        // Cores estilo Slither.io
        if (i == 0) {
            snakes[i].r = 50; snakes[i].g = 255; snakes[i].b = 100; // Player verde
        } else {
            int c = rand()%4;
            if(c==0){ snakes[i].r = 255; snakes[i].g = 50; snakes[i].b = 50; } // Vermelho
            if(c==1){ snakes[i].r = 255; snakes[i].g = 255; snakes[i].b = 50; } // Amarelo
            if(c==2){ snakes[i].r = 255; snakes[i].g = 50; snakes[i].b = 255; } // Rosa/Magenta
            if(c==3){ snakes[i].r = 50; snakes[i].g = 150; snakes[i].b = 255; } // Azul
        }
    }
    spawnSnake(snakes[0], true, 0, 0);
    cameraX = 0; cameraY = 0;
}

void Game::spawnSnake(Snake& s, bool isPlayer, float x, float y) {
    s.isPlayer = isPlayer;
    s.alive = true;
    s.angle = (float)rand() / RAND_MAX * 6.28318f;
    s.speed = isPlayer ? 220.0f : 180.0f;
    s.radius = 12.0f;
    s.score = 0;
    s.length = 15;
    s.aiTimer = 0;
    s.aiTarget = Vec2(x, y);
    
    s.body.clear();
    for (int i = 0; i < s.length; i++) {
        s.body.push_back(Vec2(x - cosf(s.angle) * i * (s.radius * 0.4f), y - sinf(s.angle) * i * (s.radius * 0.4f)));
    }
}

void Game::spawnFood(int index) {
    float a = (float)rand() / RAND_MAX * 6.28318f;
    float d = (float)rand() / RAND_MAX * WORLD_RADIUS;
    foods[index].pos = Vec2(cosf(a) * d, sinf(a) * d);
    foods[index].radius = 6.0f;
    
    // Cores neon para a comida
    static const Uint8 colors[6][3] = {
        {255, 100, 100}, {100, 255, 100}, {100, 100, 255},
        {255, 255, 100}, {255, 100, 255}, {100, 255, 255}
    };
    int c = rand() % 6;
    foods[index].r = colors[c][0];
    foods[index].g = colors[c][1];
    foods[index].b = colors[c][2];
    foods[index].pulse = (float)rand();
    foods[index].active = true;
}

void Game::updatePlayer(Snake& s, float dt, float ax, float ay, bool boost) {
    if (ax != 0 || ay != 0) {
        float targetAngle = atan2f(ay, ax);
        float diff = normalizeAngle(targetAngle - s.angle);
        s.angle += diff * 4.5f * dt;
    }
    s.speed = boost ? 380.0f : 220.0f;
}

void Game::updateBot(Snake& s, float dt) {
    if (s.body.empty()) return;
    Vec2 head = s.body.front();
    
    s.aiTimer -= dt;
    if (s.aiTimer <= 0) {
        s.aiTimer = 0.5f;
        float bestDist = 999999.0f;
        for (int i = 0; i < MAX_FOOD; i++) {
            if (!foods[i].active) continue;
            float d = dist(head.x, head.y, foods[i].pos.x, foods[i].pos.y);
            if (d < bestDist) {
                bestDist = d;
                s.aiTarget = foods[i].pos;
            }
        }
    }
    
    float targetAngle = angleTo(head.x, head.y, s.aiTarget.x, s.aiTarget.y);
    float diff = normalizeAngle(targetAngle - s.angle);
    if (fabsf(fabsf(diff) - 3.14159f) < 0.2f) diff = 1.0f;
    
    s.angle += diff * 3.5f * dt;
    s.speed = 180.0f;
}

void Game::update(float dt) {
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        
        Vec2 head = snakes[i].body.front();
        float nx = head.x + cosf(snakes[i].angle) * snakes[i].speed * dt;
        float ny = head.y + sinf(snakes[i].angle) * snakes[i].speed * dt;

        if (dist(nx, ny, 0, 0) > WORLD_RADIUS) {
            if (snakes[i].isPlayer) { snakes[i].alive = false; gameState = 2; }
            else { spawnSnake(snakes[i], false, 0, 0); }
            continue;
        }

        snakes[i].body.push_front(Vec2(nx, ny));
        
        // O corpo encolhe se não comer (como no Slither.io ao dar boost)
        while (snakes[i].body.size() > (size_t)snakes[i].length) {
            snakes[i].body.pop_back();
        }
    }
    
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Vec2 head = snakes[i].body.front();
        for (int f = 0; f < MAX_FOOD; f++) {
            if (!foods[f].active) continue;
            if (dist(head.x, head.y, foods[f].pos.x, foods[f].pos.y) < snakes[i].radius + foods[f].radius) {
                foods[f].active = false;
                snakes[i].length += 1;
                snakes[i].score += 10;
                // Aumenta o raio da cabeça levemente conforme cresce (igual slither)
                snakes[i].radius = 12.0f + (snakes[i].length * 0.05f);
                spawnFood(f);
            }
        }
    }
    
    // Atualiza animação da comida
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foods[i].active) foods[i].pulse += dt * 5.0f;
    }
}

void Game::checkCollisions() {
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Vec2 head = snakes[i].body.front();
        
        for (int j = 0; j < MAX_SNAKES; j++) {
            if (i == j || !snakes[j].alive) continue;
            for (size_t k = 4; k < snakes[j].body.size(); k++) { // Pula o pescoço
                if (dist(head.x, head.y, snakes[j].body[k].x, snakes[j].body[k].y) < snakes[i].radius * 0.8f) {
                    if (snakes[i].isPlayer) { snakes[i].alive = false; gameState = 2; }
                    else { spawnSnake(snakes[i], false, 0, 0); }
                    break;
                }
            }
        }
    }
}

void Game::run() {
    padInfo padinfo;
    padData paddata;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;
        if (dt > 0.05f) dt = 0.05f;
        
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        
        float ax = 0, ay = 0;
        bool boost = false, select = false;
        
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            ax = ((float)paddata.ANA_L_H - 128) / 128.0f;
            ay = ((float)paddata.ANA_L_V - 128) / 128.0f;
            if (abs((int)(ax*100)) < 20) ax = 0;
            if (abs((int)(ay*100)) < 20) ay = 0;
            if (paddata.BTN_CROSS || paddata.BTN_R1) boost = true;
            if (paddata.BTN_SELECT) select = true;
        }
        
        if (select) break;

        if (gameState == 1) {
            if (snakes[0].alive) updatePlayer(snakes[0], dt, ax, ay, boost);
            for (int i = 1; i < MAX_SNAKES; i++) if (snakes[i].alive) updateBot(snakes[i], dt);
            
            update(dt);
            checkCollisions();
            
            if (snakes[0].alive) {
                Vec2 head = snakes[0].body.front();
                cameraX += (head.x - cameraX) * dt * 8.0f;
                cameraY += (head.y - cameraY) * dt * 8.0f;
                // Zoom out conforme a cobra cresce
                targetZoom = 0.8f - (snakes[0].length * 0.002f);
                if (targetZoom < 0.3f) targetZoom = 0.3f;
                zoom += (targetZoom - zoom) * dt * 2.0f;
            }
        } else {
            if (paddata.BTN_CROSS) {
                initWorld();
                gameState = 1;
                zoom = 0.8f;
            }
        }

        render();
    }
}