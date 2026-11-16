#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_NAMES 24
static const char* BOT_NAMES[NUM_NAMES] = {
    "NEO", "ZICO", "LUNA", "RAIO", "KIRA", "TITA", "BRUNO", "PAULO", "ANA", "MIKO",
    "ZEUS", "NOVA", "LOBO", "FENIX", "DUDU", "MAGO", "THOR", "RUBI", "SOL", "VEGA",
    "KAIO", "ICARO", "BIA", "DANTE"
};

#define NUM_PAL 6
static const Uint8 PAL[NUM_PAL][6] = {
    {255, 60, 140, 255, 255, 255},
    {255, 200, 0,  255, 250, 200},
    {60, 200, 255, 220, 245, 255},
    {200, 80, 255, 245, 220, 255},
    {255, 120, 40, 255, 230, 200},
    {80, 255, 140, 220, 255, 230}
};

static const Uint8 FOOD_COL[6][3] = {
    {255, 60, 120}, {255, 220, 0}, {60, 255, 140},
    {60, 200, 255}, {210, 80, 255}, {255, 140, 40}
};

static float frand() { return (float)rand() / (float)RAND_MAX; }

static float dist(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static float angleTo(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

static float normalizeAngle(float a) {
    if (a != a || a > 1e6f || a < -1e6f) return 0.0f;
    const float PI_F = 3.14159265f;
    a = fmodf(a + PI_F, 2.0f * PI_F);
    if (a < 0) a += 2.0f * PI_F;
    return a - PI_F;
}

// ===== CONSTRUTOR / DESTRUTOR =====
Game::Game() {
    screen = NULL;
    running = false;
    sdlUp = false;
    gameState = 0;
    camX = 0; camY = 0; zoom = 1.0f;
    frame = 0;
    rankCount = 0;
    playerRank = 1;
    bestScore = 0;
    deathScore = 0; deathLen = 0; deathKills = 0; deathRank = 1;
    for (int i = 0; i < MAX_SNAKES; i++) { snakes[i].alive = false; snakes[i].isPlayer = false; }
    for (int i = 0; i < MAX_FOOD; i++) foods[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) parts[i].active = false;
}

Game::~Game() {
    shutdown();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) return false;
    sdlUp = true;

    int w = SCREEN_W;
    int h = SCREEN_H;
    SDL_Rect **modes = SDL_ListModes(NULL, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (modes && modes != (SDL_Rect **)-1 && modes[0]) {
        w = modes[0]->w;
        h = modes[0]->h;
    }

    screen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    if (!screen) {
        screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    }
    if (!screen) return false;

    SDL_ShowCursor(SDL_DISABLE);
    ioPadInit(7);
    srand(SDL_GetTicks());
    initWorld();
    running = true;
    return true;
}

void Game::shutdown() {
    if (!sdlUp) return;
    sdlUp = false;
    running = false;
    ioPadEnd();
    SDL_Quit();
    screen = NULL;
}

// ===== MUNDO =====
void Game::initWorld() {
    for (int i = 0; i < MAX_PARTICLES; i++) parts[i].active = false;
    spawnSnake(0, true, 0, 0);
    for (int i = 1; i < MAX_SNAKES; i++) {
        float a = frand() * 6.2831f;
        float d = 400 + frand() * 2400;
        spawnSnake(i, false, cosf(a) * d, sinf(a) * d);
    }
    for (int i = 0; i < MAX_FOOD; i++) spawnFood(i);
    zoom = 1.0f;
    camX = snakes[0].body[0].x - (screen->w * 0.5f) / zoom;
    camY = snakes[0].body[0].y - (screen->h * 0.5f) / zoom;
    updateRanking();
    gameState = 0;
}

void Game::startGame() {
    initWorld();
    gameState = 1;
}

void Game::spawnSnake(int i, bool isPlayer, float x, float y) {
    Snake &s = snakes[i];
    s.alive = true;
    s.isPlayer = isPlayer;
    s.boosting = false;
    s.angle = frand() * 6.2831f;
    s.speed = 2.6f;
    s.turnRate = 0.07f;
    s.growPending = 0;
    s.boostAcc = 0;
    s.score = 0;
    s.kills = 0;
    s.aiTimer = 0;
    s.aiTarget = Vec2(x, y);

    int len = START_LEN;
    if (isPlayer) {
        s.name = "VOCE";
        s.r1 = 40;  s.g1 = 255; s.b1 = 120;
        s.r2 = 150; s.g2 = 255; s.b2 = 190;
    } else {
        int p = rand() % NUM_PAL;
        s.name = BOT_NAMES[rand() % NUM_NAMES];
        s.r1 = PAL[p][0]; s.g1 = PAL[p][1]; s.b1 = PAL[p][2];
        s.r2 = PAL[p][3]; s.g2 = PAL[p][4]; s.b2 = PAL[p][5];
        len = 12 + rand() % 40;
    }

    s.body.clear();
    for (int k = 0; k < len; k++) {
        s.body.push_back(Vec2(x - cosf(s.angle) * k * SEG_DIST, y - sinf(s.angle) * k * SEG_DIST));
    }
    s.radius = 9;
}

void Game::spawnFood(int i) {
    float a = frand() * 6.2831f;
    float d = sqrtf(frand()) * (WORLD_RADIUS - 80);
    foods[i].pos = Vec2(cosf(a) * d, sinf(a) * d);
    foods[i].radius = 5 + (float)(rand() % 5);
    if (rand() % 12 == 0) foods[i].radius = 11;
    int c = rand() % 6;
    foods[i].r = FOOD_COL[c][0];
    foods[i].g = FOOD_COL[c][1];
    foods[i].b = FOOD_COL[c][2];
    foods[i].active = true;
}

void Game::dropFood(Vec2 p, float radius, Uint8 r, Uint8 g, Uint8 b) {
    int slot = -1;
    for (int f = 0; f < MAX_FOOD; f++) {
        if (!foods[f].active) { slot = f; break; }
    }
    if (slot < 0) slot = rand() % MAX_FOOD;
    foods[slot].pos = p;
    foods[slot].radius = radius;
    foods[slot].r = r;
    foods[slot].g = g;
    foods[slot].b = b;
    foods[slot].active = true;
}

void Game::spawnParticle(float x, float y, Uint8 r, Uint8 g, Uint8 b) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!parts[i].active) {
            parts[i].pos = Vec2(x, y);
            float a = frand() * 6.2831f;
            float s = 1.0f + frand() * 2.5f;
            parts[i].vel = Vec2(cosf(a) * s, sinf(a) * s);
            parts[i].life = 1.0f;
            parts[i].r = r; parts[i].g = g; parts[i].b = b;
            parts[i].size = 2 + (float)(rand() % 3);
            parts[i].active = true;
            return;
        }
    }
}

// ===== MOVIMENTO =====
void Game::moveHead(Snake &s, float dt, float targetAngle, bool boost) {
    float diff = normalizeAngle(targetAngle - s.angle);
    float maxT = s.turnRate * dt;
    if (diff > maxT) diff = maxT;
    else if (diff < -maxT) diff = -maxT;
    s.angle = normalizeAngle(s.angle + diff);
    float sp = s.speed * (boost ? 1.9f : 1.0f);
    s.body[0].x += cosf(s.angle) * sp * dt;
    s.body[0].y += sinf(s.angle) * sp * dt;
}

// Cada segmento segue o anterior mantendo distância fixa
void Game::followBody(Snake &s) {
    for (size_t k = 1; k < s.body.size(); k++) {
        Vec2 prev = s.body[k - 1];
        Vec2 &cur = s.body[k];
        float dx = cur.x - prev.x;
        float dy = cur.y - prev.y;
        float d = sqrtf(dx * dx + dy * dy);
        if (d > SEG_DIST && d > 0.0001f) {
            float f = SEG_DIST / d;
            cur.x = prev.x + dx * f;
            cur.y = prev.y + dy * f;
        }
    }
}

void Game::updatePlayer(float dt, float ax, float ay, bool boost) {
    Snake &s = snakes[0];
    if (!s.alive) return;
    float target = s.angle;
    if (ax != 0 || ay != 0) target = atan2f(ay, ax);
    bool canBoost = boost && s.body.size() > 12;
    moveHead(s, dt, target, canBoost);
    s.boosting = canBoost;
}

void Game::updateBot(int i, float dt) {
    Snake &s = snakes[i];
    Vec2 h = s.body[0];

    s.aiTimer--;
    if (s.aiTimer <= 0) {
        float best = 1e18f;
        int bi = -1;
        for (int f = 0; f < MAX_FOOD; f++) {
            if (!foods[f].active) continue;
            float dx = foods[f].pos.x - h.x;
            float dy = foods[f].pos.y - h.y;
            float d2 = dx * dx + dy * dy;
            if (d2 < best) { best = d2; bi = f; }
        }
        if (bi >= 0) s.aiTarget = foods[bi].pos;
        else s.aiTarget = Vec2(0, 0);
        s.aiTimer = 30 + rand() % 40;
    }

    float target = angleTo(h.x, h.y, s.aiTarget.x, s.aiTarget.y);
    if (dist(h.x, h.y, 0, 0) > WORLD_RADIUS - 350) {
        target = angleTo(h.x, h.y, 0, 0);
    }

    // Foge se tiver corpo de outra cobra logo à frente
    float lx = h.x + cosf(s.angle) * 80;
    float ly = h.y + sinf(s.angle) * 80;
    bool danger = false;
    for (int j = 0; j < MAX_SNAKES && !danger; j++) {
        if (j == i || !snakes[j].alive) continue;
        const std::vector<Vec2> &bj = snakes[j].body;
        for (size_t k = 0; k < bj.size(); k += 2) {
            if (dist(lx, ly, bj[k].x, bj[k].y) < 36) {
                target = angleTo(bj[k].x, bj[k].y, h.x, h.y);
                danger = true;
                break;
            }
        }
    }

    moveHead(s, dt, target, false);
}

void Game::updateWorld(float dt, float ax, float ay, bool boost) {
    updatePlayer(dt, ax, ay, boost);
    for (int i = 1; i < MAX_SNAKES; i++) {
        if (snakes[i].alive) updateBot(i, dt);
    }

    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Snake &s = snakes[i];

        followBody(s);

        while (s.growPending >= 1.0f) {
            Vec2 t = s.body.back();
            s.body.push_back(t);
            s.growPending -= 1.0f;
        }

        if (s.boosting) {
            s.boostAcc += dt;
            if (s.boostAcc >= 10.0f && s.body.size() > 12) {
                s.boostAcc = 0;
                Vec2 tail = s.body.back();
                s.body.pop_back();
                dropFood(tail, 5, s.r1, s.g1, s.b1);
            }
            if (s.isPlayer && frame % 3 == 0) {
                spawnParticle(s.body[0].x, s.body[0].y, s.r1, s.g1, s.b1);
            }
        }

        float nr = 9 + (float)s.body.size() / 30.0f;
        if (nr > 17) nr = 17;
        s.radius = nr;

        if (dist(s.body[0].x, s.body[0].y, 0, 0) > WORLD_RADIUS) {
            killSnake(i, -1);
        }
    }

    checkCollisions();
    updateParticles(dt);
    updateCamera(dt);
    updateRanking();
}

// ===== COLISÕES =====
void Game::checkCollisions() {
    // Comida
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Vec2 h = snakes[i].body[0];
        for (int f = 0; f < MAX_FOOD; f++) {
            if (!foods[f].active) continue;
            float d = dist(h.x, h.y, foods[f].pos.x, foods[f].pos.y);
            if (d < snakes[i].radius + foods[f].radius) {
                snakes[i].growPending += 1.0f;
                snakes[i].score += 1 + (int)foods[f].radius;
                spawnFood(f);
            }
        }
    }

    // Cabeça contra corpo (inclusive o próprio, a partir do 5º segmento)
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        Vec2 h = snakes[i].body[0];
        float ri = snakes[i].radius;
        for (int j = 0; j < MAX_SNAKES && snakes[i].alive; j++) {
            if (!snakes[j].alive) continue;
            float lim = (ri + snakes[j].radius) * 0.5f;
            size_t start = (j == i) ? 5 : 0;
            const std::vector<Vec2> &bj = snakes[j].body;
            bool hit = false;
            bool headOnHead = false;
            for (size_t k = start; k < bj.size(); k++) {
                if (dist(h.x, h.y, bj[k].x, bj[k].y) < lim) {
                    hit = true;
                    if (k == 0 && j != i) headOnHead = true;
                    break;
                }
            }
            if (!hit) continue;
            if (headOnHead) {
                killSnake(i, j);
                killSnake(j, i);
            } else {
                killSnake(i, (j == i) ? -1 : j);
            }
        }
    }
}

void Game::killSnake(int idx, int killer) {
    if (idx < 0 || idx >= MAX_SNAKES) return;
    Snake &s = snakes[idx];
    if (!s.alive) return;
    s.alive = false;

    for (size_t k = 0; k < s.body.size(); k += 2) {
        dropFood(s.body[k], 7, s.r1, s.g1, s.b1);
    }
    for (int p = 0; p < 16; p++) {
        spawnParticle(s.body[0].x, s.body[0].y, s.r1, s.g1, s.b1);
    }

    if (killer >= 0 && killer < MAX_SNAKES && killer != idx && snakes[killer].alive) {
        snakes[killer].kills++;
        snakes[killer].score += 50 + (int)s.body.size();
    }

    if (!s.isPlayer) {
        float a = frand() * 6.2831f;
        float d = 500 + frand() * 2400;
        spawnSnake(idx, false, cosf(a) * d, sinf(a) * d);
    }
}

// ===== EFEITOS / CÂMERA / RANKING =====
void Game::updateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!parts[i].active) continue;
        parts[i].pos.x += parts[i].vel.x * dt;
        parts[i].pos.y += parts[i].vel.y * dt;
        parts[i].life -= 0.04f * dt;
        if (parts[i].life <= 0) parts[i].active = false;
    }
}

void Game::updateCamera(float dt) {
    if (!snakes[0].alive) return;
    Vec2 h = snakes[0].body[0];
    float tz = 1.0f - (float)snakes[0].body.size() / 600.0f;
    if (tz < 0.55f) tz = 0.55f;
    zoom += (tz - zoom) * 0.03f * dt;
    float tx = h.x - (screen->w * 0.5f) / zoom;
    float ty = h.y - (screen->h * 0.5f) / zoom;
    camX += (tx - camX) * 0.12f * dt;
    camY += (ty - camY) * 0.12f * dt;
}

void Game::updateRanking() {
    rankCount = 0;
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (snakes[i].alive) rankOrder[rankCount++] = i;
    }
    // Ordena por tamanho, maior primeiro (insertion sort)
    for (int a = 1; a < rankCount; a++) {
        int key = rankOrder[a];
        int b = a - 1;
        while (b >= 0 && snakes[rankOrder[b]].body.size() < snakes[key].body.size()) {
            rankOrder[b + 1] = rankOrder[b];
            b--;
        }
        rankOrder[b + 1] = key;
    }
    for (int a = 0; a < rankCount; a++) {
        if (rankOrder[a] == 0) { playerRank = a + 1; break; }
    }
}

// ===== LOOP PRINCIPAL =====
void Game::run() {
    padInfo padinfo;
    padData paddata;
    Uint32 lastTime = SDL_GetTicks();
    bool startPrev = false;

    while (running) {
        if (g_exitRequested) break;

        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 16.666f;
        if (dt > 3.0f) dt = 3.0f;
        if (dt < 0.1f) dt = 0.1f;
        lastTime = now;

        sysUtilCheckCallback();

        float ax = 0, ay = 0;
        bool boost = false;
        bool start = false;
        ioPadGetInfo(&padinfo);
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            int rx = paddata.ANA_L_H - 128;
            int ry = paddata.ANA_L_V - 128;
            if (rx < -12 || rx > 12 || ry < -12 || ry > 12) {
                ax = rx / 110.0f;
                ay = ry / 110.0f;
                if (ax > 1) ax = 1;
                if (ax < -1) ax = -1;
                if (ay > 1) ay = 1;
                if (ay < -1) ay = -1;
            }
            if (paddata.BTN_CROSS || paddata.BTN_R2) boost = true;
            if (paddata.BTN_START) start = true;
        }
        bool startPressed = start && !startPrev;
        startPrev = start;

        if (gameState == 0) {
            if (startPressed) startGame();
        } else if (gameState == 1) {
            updateWorld(dt, ax, ay, boost);
            if (!snakes[0].alive) {
                deathScore = snakes[0].score;
                deathLen = (int)snakes[0].body.size();
                deathKills = snakes[0].kills;
                deathRank = playerRank;
                if (deathScore > bestScore) bestScore = deathScore;
                gameState = 2;
            }
        } else {
            if (startPressed) startGame();
        }

        render();
        frame++;

        Uint32 el = SDL_GetTicks() - now;
        if (el < 16) SDL_Delay(16 - el);
    }
}