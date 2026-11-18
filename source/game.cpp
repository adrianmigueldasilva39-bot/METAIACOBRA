#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <cstdio>
#include <ctime> // <--- ADICIONADO AQUI PARA O TIME() FUNCIONAR

Game::Game() {
    screen = NULL;
    running = false;
    gameState = 1;
    cameraX = 0; cameraY = 0;
    zoom = 1.0f; targetZoom = 1.0f;
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
        spawnSnake(snakes[i], i == 0, cosf(a) * 1000.0f, sinf(a) * 1000.0f);
    }
    // Cores: Jogador é verde neon, bots são rosa/roxo/amarelo
    snakes[0].r = 50; snakes[0].g = 255; snakes[0].b = 100; 
    for(int i=1; i<MAX_SNAKES; i++) {
        snakes[i].r = 255; snakes[i].g = rand()%100 + 50; snakes[i].b = 200;
    }
    spawnSnake(snakes[0], true, 0, 0);
    cameraX = 0; cameraY = 0;
}

void Game::spawnSnake(Snake& s, bool isPlayer, float x, float y) {
    s.isPlayer = isPlayer;
    s.alive = true;
    s.angle = (float)rand() / RAND_MAX * 6.28318f;
    s.speed = isPlayer ? 250.0f : 200.0f; // Jogador um pouco mais rápido
    s.radius = 15.0f;
    s.score = 0;
    s.length = 20;
    s.aiTimer = 0;
    s.aiTarget = Vec2(x, y);
    
    s.body.clear();
    for (int i = 0; i < s.length; i++) {
        s.body.push_back(Vec2(x - cosf(s.angle) * i * (s.radius * 0.6f), y - sinf(s.angle) * i * (s.radius * 0.6f)));
    }
}

void Game::spawnFood(int index) {
    float a = (float)rand() / RAND_MAX * 6.28318f;
    float d = (float)rand() / RAND_MAX * WORLD_RADIUS;
    foods[index].pos = Vec2(cosf(a) * d, sinf(a) * d);
    foods[index].radius = 5.0f;
    foods[index].r = rand() % 255;
    foods[index].g = rand() % 255;
    foods[index].b = rand() % 255;
    foods[index].active = true;
}

void Game::updatePlayer(Snake& s, float dt, float ax, float ay, bool boost) {
    if (ax != 0 || ay != 0) {
        float targetAngle = atan2f(ay, ax);
        float diff = normalizeAngle(targetAngle - s.angle);
        s.angle += diff * 5.0f * dt;
    }
    s.speed = boost ? 400.0f : 250.0f;
}

void Game::updateBot(Snake& s, float dt) {
    if (s.body.empty()) return;
    Vec2 head = s.body.front();
    
    s.aiTimer -= dt;
    if (s.aiTimer <= 0) {
        s.aiTimer = 0.5f;
        // Acha a comida mais próxima
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
    
    // Se o alvo estiver exatamente atrás, vira pro lado pra não travar
    if (fabsf(fabsf(diff) - 3.14159f) < 0.2f) diff = 1.0f;

    s.angle += diff * 4.0f * dt;
    s.speed = 200.0f;
}

void Game::update(float dt) {
    // Move todas as cobras
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        
        Vec2 head = snakes[i].body.front();
        float nx = head.x + cosf(snakes[i].angle) * snakes[i].speed * dt;
        float ny = head.y + sinf(snakes[i].angle) * snakes[i].speed * dt;

        // Bateu na borda? Morre (ou reseta, se for o player)
        if (dist(nx, ny, 0, 0) > WORLD_RADIUS) {
            if (snakes[i].isPlayer) { snakes[i].alive = false; gameState = 2; }
            else { spawnSnake(snakes[i], false, 0, 0); }
            continue;
        }

        // Adiciona a nova cabeça na frente e remove o rabo (se não comeu)
        snakes[i].body.push_front(Vec2(nx, ny));
        while (snakes[i].body.size() > (size_t)snakes[i].length) {
            snakes[i].body.pop_back();
        }
    }
    
    // Colisão com comida
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Vec2 head = snakes[i].body.front();
        for (int f = 0; f < MAX_FOOD; f++) {
            if (!foods[f].active) continue;
            if (dist(head.x, head.y, foods[f].pos.x, foods[f].pos.y) < snakes[i].radius + foods[f].radius) {
                foods[f].active = false;
                snakes[i].length += 2;
                snakes[i].score += 10;
                spawnFood(f); // Respawna em outro lugar
            }
        }
    }
}

void Game::checkCollisions() {
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Vec2 head = snakes[i].body.front();
        
        // Bateu em outra cobra?
        for (int j = 0; j < MAX_SNAKES; j++) {
            if (i == j || !snakes[j].alive) continue;
            // Pula a própria cabeça
            for (size_t k = 2; k < snakes[j].body.size(); k++) {
                if (dist(head.x, head.y, snakes[j].body[k].x, snakes[j].body[k].y) < snakes[i].radius) {
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
            if (abs((int)(ax*100)) < 20) ax = 0; // Corrigido pra não usar abs float
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
            
            // Câmera segue o jogador
            if (snakes[0].alive) {
                Vec2 head = snakes[0].body.front();
                cameraX += (head.x - cameraX) * dt * 5.0f;
                cameraY += (head.y - cameraY) * dt * 5.0f;
            }
        } else {
            // Tela de Game Over (aperta X pra voltar)
            if (paddata.BTN_CROSS) {
                initWorld();
                gameState = 1;
            }
        }

        render();
    }
}