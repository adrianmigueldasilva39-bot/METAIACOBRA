#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <cstdio>

Game::Game() {
    screen = NULL;
    running = false;
    gameState = 1;
    score = 0;
    record = 15;
}

Game::~Game() {}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("ERRO: SDL_Init falhou!\n");
        return false;
    }
    
    ioPadInit(7);
    
    // No PS3, as vezes 32 bits estoura a memoria. Vamos usar 16 bits (mais leve)
    // e garantir que a flag é SWSURFACE.
    screen = SDL_SetVideoMode(1280, 720, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
    if (!screen) {
        printf("ERRO: SDL_SetVideoMode falhou!\n");
        return false;
    }
    
    SDL_ShowCursor(SDL_DISABLE);
    srand(12345); 
    
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

Vec2 Game::worldToScreen(float wx, float wy) {
    float isoX = (wx - wy) * 1.2f;
    float isoY = (wx + wy) * 0.6f;
    float zoom = 1.2f;
    return Vec2(
        isoX * zoom + screen->w / 2,
        isoY * zoom + screen->h / 2 + 100
    );
}

void Game::spawnApple() {
    bool valid = false;
    while (!valid) {
        apple.x = rand() % GRID_SIZE;
        apple.y = rand() % GRID_SIZE;
        valid = true;
        for (size_t i = 0; i < player.body.size(); i++) {
            if (player.body[i] == apple) { valid = false; break; }
        }
    }
}

void Game::initWorld() {
    player.isPlayer = true;
    player.alive = true;
    player.body.clear();
    player.body.push_back({10, 10});
    player.body.push_back({10, 11});
    player.body.push_back({10, 12});
    player.dirX = 0; player.dirY = -1;
    player.moveTimer = 0;
    player.moveSpeed = 0.15f;

    for (int i = 0; i < MAX_SNAKES-1; i++) {
        bots[i].alive = false;
    }

    score = 0;
    spawnApple();
    gameState = 1;
}

void Game::update(float dt) {
    if (gameState != 1) return;
    updateSnake(player, dt, true);
    checkCollisions();
}

void Game::updateSnake(Snake& s, float dt, bool isPlayer) {
    if (!s.alive) return;

    s.moveTimer += dt;
    if (s.moveTimer >= s.moveSpeed) {
        s.moveTimer = 0;

        GridPos newHead = s.body.front();
        newHead.x += s.dirX;
        newHead.y += s.dirY;

        if (newHead.x < 0 || newHead.x >= GRID_SIZE || newHead.y < 0 || newHead.y >= GRID_SIZE) {
            s.alive = false;
            if (isPlayer) gameState = 2;
            return;
        }

        for (size_t i = 0; i < s.body.size() - 1; i++) {
            if (s.body[i] == newHead) {
                s.alive = false;
                if (isPlayer) gameState = 2;
                return;
            }
        }

        s.body.insert(s.body.begin(), newHead);

        if (newHead == apple) {
            if (isPlayer) {
                score++;
                if (score > record) record = score;
                spawnApple();
            }
        } else {
            s.body.pop_back();
        }
    }
}

void Game::checkCollisions() {}

void Game::run() {
    padInfo padinfo;
    padData paddata;
    Uint32 lastTime = SDL_GetTicks();
    bool lastCross = false;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;
        if (dt > 0.1f) dt = 0.1f;

        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        
        bool cross = false, select = false;

        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            
            if (gameState == 1) {
                if (paddata.BTN_UP && player.dirY == 0) { player.dirX = 0; player.dirY = -1; }
                if (paddata.BTN_DOWN && player.dirY == 0) { player.dirX = 0; player.dirY = 1; }
                if (paddata.BTN_LEFT && player.dirX == 0) { player.dirX = -1; player.dirY = 0; }
                if (paddata.BTN_RIGHT && player.dirX == 0) { player.dirX = 1; player.dirY = 0; }
            }

            if (paddata.BTN_CROSS) cross = true;
            if (paddata.BTN_SELECT) select = true;
        }

        if (select) break;

        if (gameState == 2) {
            if (cross && !lastCross) {
                initWorld();
            }
        }
        lastCross = cross;

        if (gameState == 1) {
            update(dt);
        }

        render();
    }
}