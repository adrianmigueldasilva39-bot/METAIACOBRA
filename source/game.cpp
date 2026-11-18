#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <cstdio>
#include <ctime>

Game::Game() {
    screen = NULL;
    running = false;
    gameState = 1;
    score = 0;
    record = 15; // Recorde inicial igual ao print
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

Vec2 Game::worldToScreen(float wx, float wy) {
    // Centraliza o grid na tela e aplica um zoom para ficar 3D
    float centerX = (GRID_SIZE * CELL_SIZE) / 2.0f;
    float centerY = (GRID_SIZE * CELL_SIZE) / 2.0f;
    
    // Inclinação 3D fake (isométrico)
    float isoX = (wx - wy) * 1.2f;
    float isoY = (wx + wy) * 0.6f;
    
    float zoom = 1.2f;
    return Vec2(
        isoX * zoom + screen->w / 2,
        isoY * zoom + screen->h / 2 + 100 // Joga um pouco pra baixo
    );
}

void Game::spawnApple() {
    bool valid = false;
    while (!valid) {
        apple.x = rand() % GRID_SIZE;
        apple.y = rand() % GRID_SIZE;
        valid = true;
        // Não deixa a maçã nascer dentro do player
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
    player.moveSpeed = 0.15f; // Segundos por movimento

    for (int i = 0; i < MAX_SNAKES-1; i++) {
        bots[i].alive = false; // Desativado por enquanto para focar no Player
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

        // Bateu na parede (borda do mapa)?
        if (newHead.x < 0 || newHead.x >= GRID_SIZE || newHead.y < 0 || newHead.y >= GRID_SIZE) {
            s.alive = false;
            if (isPlayer) gameState = 2;
            return;
        }

        // Bateu no próprio corpo?
        for (size_t i = 0; i < s.body.size() - 1; i++) {
            if (s.body[i] == newHead) {
                s.alive = false;
                if (isPlayer) gameState = 2;
                return;
            }
        }

        s.body.insert(s.body.begin(), newHead);

        // Comeu a maçã?
        if (newHead == apple) {
            if (isPlayer) {
                score++;
                if (score > record) record = score;
                spawnApple();
            }
        } else {
            s.body.pop_back(); // Se não comeu, o rabo anda
        }
    }
}

void Game::checkCollisions() {
    // Se o player morreu, o jogo já para no updateSnake
}

void Game::run() {
    padInfo padinfo;
    padData paddata;
    Uint32 lastTime = SDL_GetTicks();
    bool lastCross = false;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;
        if (dt > 0.1f) dt = 0.1f; // Evita saltos se travar

        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        
        bool cross = false, start = false, select = false;

        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            
            // Controles
            if (gameState == 1) {
                if (paddata.BTN_UP && player.dirY == 0) { player.dirX = 0; player.dirY = -1; }
                if (paddata.BTN_DOWN && player.dirY == 0) { player.dirX = 0; player.dirY = 1; }
                if (paddata.BTN_LEFT && player.dirX == 0) { player.dirX = -1; player.dirY = 0; }
                if (paddata.BTN_RIGHT && player.dirX == 0) { player.dirX = 1; player.dirY = 0; }
            }

            if (paddata.BTN_CROSS) cross = true;
            if (paddata.BTN_START) start = true;
            if (paddata.BTN_SELECT) select = true;
        }

        if (select) break;

        // Game Over -> Aperta X pra reiniciar
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