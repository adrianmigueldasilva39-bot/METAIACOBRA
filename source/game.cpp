#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <cstdio>

Game::Game()
    : screen(NULL), running(false), gameState(1), cameraX(0), cameraY(0),
      zoom(0.95f), targetZoom(0.95f), frameCount(0), deathTimer(0), xPressedLast(false) {
}

Game::~Game() {
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    ioPadInit(7);
    screen = SDL_SetVideoMode(1280, 720, 32, SDL_SWSURFACE | SDL_FULLSCREEN);
    if (!screen) {
        return false;
    }
    SDL_ShowCursor(SDL_DISABLE);
    srand(12345);
    initWorld();
    running = true;
    gameState = 1;
    return true;
}

void Game::shutdown() {
    ioPadEnd();
    SDL_Quit();
    screen = NULL;
}

float Game::dist(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx * dx + dy * dy);
}

float Game::angleTo(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

float Game::normalizeAngle(float a) {
    while (a > 3.14159f) a -= 6.28318f;
    while (a < -3.14159f) a += 6.28318f;
    return a;
}

bool Game::isInsideWorld(float x, float y) {
    return dist(x, y, 0, 0) < WORLD_RADIUS;
}

Vec2 Game::worldToScreen(float wx, float wy) {
    return Vec2((wx - cameraX) * zoom + screen->w / 2, (wy - cameraY) * zoom + screen->h / 2);
}

Uint32 Game::getColor(Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(screen->format, r, g, b);
}

void Game::setSkin(Snake& s, int skin) {
    static const Uint8 pal[8][3][3] = {
        {{0, 255, 140}, {0, 200, 110}, {0, 255, 180}},
        {{255, 255, 255}, {180, 180, 180}, {255, 255, 255}},
        {{0, 220, 90}, {255, 255, 255}, {0, 220, 90}},
        {{255, 80, 180}, {80, 200, 255}, {255, 220, 0}},
        {{255, 50, 50}, {255, 255, 255}, {255, 50, 50}},
        {{255, 120, 0}, {255, 220, 0}, {255, 0, 180}},
        {{120, 80, 255}, {255, 255, 255}, {0, 200, 255}},
        {{255, 200, 0}, {255, 80, 0}, {255, 255, 0}}
    };
    static const Uint8 rainbow[6][3] = {
        {255, 0, 0}, {255, 140, 0}, {255, 255, 0},
        {0, 255, 0}, {0, 100, 255}, {150, 0, 255}
    };
    if (skin < 0 || skin > 7) skin = 0;
    s.skin = skin;
    if (s.isPlayer) {
        skin = 0;
        s.skin = 0;
    }
    if (skin == 5) {
        s.skinLen = 6;
        for (int i = 0; i < 10; i++) {
            s.cols[i][0] = rainbow[i % 6][0];
            s.cols[i][1] = rainbow[i % 6][1];
            s.cols[i][2] = rainbow[i % 6][2];
        }
    } else {
        s.skinLen = 3;
        for (int i = 0; i < 10; i++) {
            s.cols[i][0] = pal[skin][i % 3][0];
            s.cols[i][1] = pal[skin][i % 3][1];
            s.cols[i][2] = pal[skin][i % 3][2];
        }
    }
}

void Game::spawnSnake(Snake& s, bool isPlayer, float x, float y) {
    s.isPlayer = isPlayer;
    s.alive = true;
    s.angle = (float)rand() / (float)RAND_MAX * 6.28318f;
    s.targetAngle = s.angle;
    s.baseSpeed = 310.0f;
    s.boostSpeed = 580.0f;
    s.speed = 310.0f;
    s.radius = 18.0f;
    s.length = 28;
    s.score = 100;
    s.kills = 0;
    s.boostTimer = 0;
    s.boosting = false;
    s.turnSpeed = 6.0f;
    s.aiTimer = 0;
    s.aiTarget = Vec2(x, y);
    s.name = isPlayer ? "VOCE" : "BOT";
    setSkin(s, rand() % 8);
    s.body.clear();
    s.path.clear();
    float spacing = s.radius * 0.65f;
    for (int i = 0; i < 400; i++) {
        float px = x - cosf(s.angle) * spacing * i;
        float py = y - sinf(s.angle) * spacing * i;
        s.path.push_back(Vec2(px, py));
    }
    rebuildBodyFromPath(s);
}

void Game::rebuildBodyFromPath(Snake& s) {
    float spacing = s.radius * 0.65f;
    s.body.clear();
    if (s.path.empty()) return;
    s.body.push_back(s.path[0]);
    Vec2 last = s.path[0];
    float acc = 0;
    for (size_t i = 1; i < s.path.size() && (int)s.body.size() < (int)s.length; i++) {
        float d = dist(last.x, last.y, s.path[i].x, s.path[i].y);
        acc += d;
        last = s.path[i];
        if (acc >= spacing) {
            s.body.push_back(s.path[i]);
            acc = 0;
        }
    }
    float maxPathLen = spacing * (s.length + 20);
    float total = 0;
    size_t cut = s.path.size();
    for (size_t i = 1; i < s.path.size(); i++) {
        total += dist(s.path[i - 1].x, s.path[i - 1].y, s.path[i].x, s.path[i].y);
        if (total > maxPathLen) {
            cut = i;
            break;
        }
    }
    if (cut < s.path.size()) {
        s.path.erase(s.path.begin() + (int)cut, s.path.end());
    }
}

void Game::spawnFood(int index, bool anywhere) {
    float a = (float)rand() / (float)RAND_MAX * 6.28318f;
    float d = (float)rand() / (float)RAND_MAX * (anywhere ? WORLD_RADIUS - 200.0f : 1200.0f);
    Vec2 p;
    if (anywhere) {
        p = Vec2(cosf(a) * d, sinf(a) * d);
    } else {
        if (!snakes[0].body.empty()) {
            p = Vec2(snakes[0].body[0].x + cosf(a) * d, snakes[0].body[0].y + sinf(a) * d);
        } else {
            p = Vec2(cosf(a) * d, sinf(a) * d);
        }
        if (!isInsideWorld(p.x, p.y)) {
            p = Vec2(cosf(a) * 500.0f, sinf(a) * 500.0f);
        }
    }
    static const Uint8 fc[8][3] = {
        {255, 80, 180}, {80, 255, 180}, {255, 220, 0}, {80, 200, 255},
        {255, 120, 0}, {180, 80, 255}, {255, 255, 255}, {255, 0, 80}
    };
    int c = rand() % 8;
    foods[index].pos = p;
    foods[index].big = (rand() % 12 == 0);
    foods[index].radius = foods[index].big ? 10.0f + (rand() % 5) : 4.0f + (rand() % 4);
    foods[index].r = fc[c][0];
    foods[index].g = fc[c][1];
    foods[index].b = fc[c][2];
    foods[index].pulse = (float)rand();
    foods[index].active = true;
}

void Game::spawnParticle(float x, float y, Uint8 r, Uint8 g, Uint8 b) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].active = true;
            particles[i].pos = Vec2(x, y);
            particles[i].vel = Vec2((float)(rand() % 160 - 80), (float)(rand() % 160 - 80));
            particles[i].life = 0.6f;
            particles[i].maxLife = 0.6f;
            particles[i].r = r;
            particles[i].g = g;
            particles[i].b = b;
            particles[i].size = 4.0f + (rand() % 5);
            break;
        }
    }
}

void Game::spawnKillOrbs(Snake& s) {
    for (size_t i = 0; i < s.body.size(); i++) {
        int idx = rand() % MAX_FOOD;
        int pat = (int)((i / 4) % s.skinLen);
        foods[idx].pos = Vec2(s.body[i].x + (rand() % 40 - 20), s.body[i].y + (rand() % 40 - 20));
        foods[idx].radius = 7.0f + (rand() % 5);
        foods[idx].r = s.cols[pat][0];
        foods[idx].g = s.cols[pat][1];
        foods[idx].b = s.cols[pat][2];
        foods[idx].active = true;
        foods[idx].big = true;
        foods[idx].pulse = (float)rand();
    }
}

void Game::initWorld() {
    for (int i = 0; i < MAX_FOOD; i++) {
        spawnFood(i, true);
    }
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    for (int i = 0; i < MAX_SNAKES; i++) {
        float a = (float)i / (float)MAX_SNAKES * 6.28318f;
        float d = 900.0f + (rand() % 3000);
        spawnSnake(snakes[i], i == 0, cosf(a) * d, sinf(a) * d);
    }
    spawnSnake(snakes[0], true, 0, 0);
    cameraX = 0;
    cameraY = 0;
    targetZoom = 0.95f;
    zoom = 0.95f;
    deathTimer = 0;
    gameState = 1;
}

void Game::resetPlayer() {
    spawnSnake(snakes[0], true, 0, 0);
    cameraX = 0;
    cameraY = 0;
    targetZoom = 0.95f;
    zoom = 0.95f;
    deathTimer = 0;
    gameState = 1;
}

void Game::addScore(Snake& s, int amount) {
    s.score += amount;
    s.length += amount / 18;
    if (s.length > 420) s.length = 420;
    if (s.isPlayer) {
        targetZoom = 0.95f - (snakes[0].length / 420.0f) * 0.45f;
        if (targetZoom < 0.50f) targetZoom = 0.50f;
    }
}

void Game::eatFood(Snake& s, int index) {
    addScore(s, foods[index].big ? 25 : 8);
    for (int i = 0; i < 3; i++) {
        spawnParticle(foods[index].pos.x, foods[index].pos.y, foods[index].r, foods[index].g, foods[index].b);
    }
    spawnFood(index, false);
}

void Game::killSnake(int index, int killerIndex) {
    if (!snakes[index].alive) return;
    snakes[index].alive = false;
    spawnKillOrbs(snakes[index]);
    for (size_t i = 0; i < snakes[index].body.size(); i += 3) {
        spawnParticle(snakes[index].body[i].x, snakes[index].body[i].y,
                      snakes[index].cols[0][0], snakes[index].cols[0][1], snakes[index].cols[0][2]);
    }
    if (killerIndex >= 0 && killerIndex < MAX_SNAKES) {
        snakes[killerIndex].kills++;
        addScore(snakes[killerIndex], 150);
    }
    if (index == 0) {
        deathTimer = 2.0f;
        gameState = 2;
        return;
    }
    float a = (float)rand() / (float)RAND_MAX * 6.28318f;
    float d = 900.0f + (rand() % 4000);
    spawnSnake(snakes[index], false, cosf(a) * d, sinf(a) * d);
}

void Game::updatePlayer(Snake& s, float dt, float ax, float ay, bool boostBtn) {
    if (ax != 0 || ay != 0) {
        s.targetAngle = atan2f(ay, ax);
    }
    float diff = normalizeAngle(s.targetAngle - s.angle);
    s.angle += diff * s.turnSpeed * dt;
    s.boosting = boostBtn && s.length > 30;
    s.speed = s.boosting ? s.boostSpeed : s.baseSpeed;
    if (s.boosting) {
        s.boostTimer += dt;
        if (s.boostTimer > 0.25f) {
            s.boostTimer = 0;
            s.length -= 1;
            if (s.length < 10) s.length = 10;
            int idx = rand() % MAX_FOOD;
            if (!s.path.empty()) {
                foods[idx].pos = Vec2(s.path.back().x, s.path.back().y);
            }
            foods[idx].radius = 6;
            foods[idx].r = s.cols[0][0];
            foods[idx].g = s.cols[0][1];
            foods[idx].b = s.cols[0][2];
            foods[idx].active = true;
            foods[idx].big = false;
            foods[idx].pulse = 0;
        }
    }
    Vec2 head = s.path.empty() ? Vec2(0, 0) : s.path[0];
    float nx = head.x + cosf(s.angle) * s.speed * dt;
    float ny = head.y + sinf(s.angle) * s.speed * dt;
    if (!isInsideWorld(nx, ny)) {
        killSnake(0, -1);
        return;
    }
    s.path.insert(s.path.begin(), Vec2(nx, ny));
    rebuildBodyFromPath(s);
}

void Game::updateBot(Snake& s, float dt) {
    s.baseSpeed = 285.0f;
    s.boostSpeed = 540.0f;

    if (s.body.empty()) return;

    Vec2 head = s.body[0];
    s.aiTimer -= dt;

    if (s.aiTimer <= 0) {
        s.aiTimer = 0.6f + (rand() % 7) / 10.0f;

        float centerDist = dist(head.x, head.y, 0, 0);
        if (centerDist > WORLD_RADIUS - 700.0f) {
            s.aiTarget = Vec2(0, 0);
            s.boosting = false;
        } else {
            float bestDist = 1e9f;
            Vec2 bestFood = head;
            for (int i = 0; i < MAX_FOOD; i++) {
                if (!foods[i].active) continue;
                float dd = dist(head.x, head.y, foods[i].pos.x, foods[i].pos.y);
                if (dd < bestDist) {
                    bestDist = dd;
                    bestFood = foods[i].pos;
                }
            }

            int nearest = -1;
            float nearestDist = 1e9f;
            for (int j = 0; j < MAX_SNAKES; j++) {
                if (&snakes[j] == &s) continue;
                if (!snakes[j].alive || snakes[j].body.empty()) continue;
                if (snakes[j].score < s.score + 50) continue;
                float dd = dist(head.x, head.y, snakes[j].body[0].x, snakes[j].body[0].y);
                if (dd < nearestDist) {
                    nearestDist = dd;
                    nearest = j;
                }
            }

            bool wantCut = false;
            if (nearest >= 0 && nearestDist < 450.0f && bestDist > 200.0f) {
                Snake& target = snakes[nearest];
                s.aiTarget = Vec2(target.body[0].x + cosf(target.angle) * 160.0f,
                                  target.body[0].y + sinf(target.angle) * 160.0f);
                wantCut = true;
            } else {
                s.aiTarget = bestFood;
            }

            if (bestDist > 900.0f && rand() % 100 < 30) {
                float a = (float)rand() / (float)RAND_MAX * 6.28318f;
                float d = 400.0f + (rand() % 800);
                Vec2 p(head.x + cosf(a) * d, head.y + sinf(a) * d);
                if (isInsideWorld(p.x, p.y)) s.aiTarget = p;
            }

            s.boosting = wantCut && s.length > 40;
        }
    }

    float avoidX = 0, avoidY = 0;
    int dangerCount = 0;
    for (int j = 0; j < MAX_SNAKES; j++) {
        if (&snakes[j] == &s) continue;
        if (!snakes[j].alive || snakes[j].body.empty()) continue;
        for (size_t k = 3; k < snakes[j].body.size(); k += 2) {
            float dd = dist(head.x, head.y, snakes[j].body[k].x, snakes[j].body[k].y);
            if (dd < 140.0f) {
                float w = 1.0f - (dd / 140.0f);
                avoidX += (head.x - snakes[j].body[k].x) * w;
                avoidY += (head.y - snakes[j].body[k].y) * w;
                dangerCount++;
            }
        }
    }
    float cd = dist(head.x, head.y, 0, 0);
    if (cd > WORLD_RADIUS - 400.0f) {
        float w = (cd - (WORLD_RADIUS - 400.0f)) / 400.0f;
        avoidX += (0.0f - head.x) * w * 0.5f;
        avoidY += (0.0f - head.y) * w * 0.5f;
        dangerCount++;
    }

    Vec2 goal = s.aiTarget;
    if (dangerCount > 0) {
        float len = sqrtf(avoidX * avoidX + avoidY * avoidY);
        if (len > 0.0001f) {
            avoidX /= len;
            avoidY /= len;
            goal = Vec2(head.x + avoidX * 220.0f + (s.aiTarget.x - head.x) * 0.25f,
                        head.y + avoidY * 220.0f + (s.aiTarget.y - head.y) * 0.25f);
        }
        s.boosting = false;
    }

    float desired = angleTo(head.x, head.y, goal.x, goal.y);
    float diff = normalizeAngle(desired - s.angle);

    if (fabsf(diff) > 0.12f) {
        float turn = (dangerCount > 0) ? 9.0f : 5.0f;
        s.angle += diff * turn * dt;
    }
    while (s.angle >  3.14159f) s.angle -= 6.28318f;
    while (s.angle < -3.14159f) s.angle += 6.28318f;

    s.speed = (s.boosting && s.length > 30) ? s.boostSpeed : s.baseSpeed;
    float nx = head.x + cosf(s.angle) * s.speed * dt;
    float ny = head.y + sinf(s.angle) * s.speed * dt;
    if (!isInsideWorld(nx, ny)) {
        s.aiTarget = Vec2(0, 0);
        s.angle = angleTo(head.x, head.y, 0, 0);
        return;
    }
    s.path.insert(s.path.begin(), Vec2(nx, ny));
    rebuildBodyFromPath(s);
}

void Game::updateFoods(float dt) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foods[i].active) foods[i].pulse += dt * 4.0f;
    }
}

void Game::updateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].pos.x += particles[i].vel.x * dt;
        particles[i].pos.y += particles[i].vel.y * dt;
        particles[i].life -= dt;
        if (particles[i].life <= 0) particles[i].active = false;
    }
}

void Game::updateCamera(float dt) {
    if (snakes[0].body.empty()) return;
    cameraX += (snakes[0].body[0].x - cameraX) * dt * 6.0f;
    cameraY += (snakes[0].body[0].y - cameraY) * dt * 6.0f;
    zoom += (targetZoom - zoom) * dt * 2.0f;
}

void Game::checkCollisions() {
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive || snakes[i].body.empty()) continue;
        for (int f = 0; f < MAX_FOOD; f++) {
            if (!foods[f].active) continue;
            if (dist(snakes[i].body[0].x, snakes[i].body[0].y, foods[f].pos.x, foods[f].pos.y) < snakes[i].radius + foods[f].radius) {
                eatFood(snakes[i], f);
            }
        }
        for (int j = 0; j < MAX_SNAKES; j++) {
            if (i == j) continue;
            if (!snakes[j].alive) continue;
            float hitDist = (snakes[i].radius + snakes[j].radius) * 0.85f;
            for (size_t k = 4; k < snakes[j].body.size(); k++) {
                if (dist(snakes[i].body[0].x, snakes[i].body[0].y, snakes[j].body[k].x, snakes[j].body[k].y) < hitDist) {
                    killSnake(i, j);
                    break;
                }
            }
            if (!snakes[i].alive) break;
        }
    }
}

void Game::update(float dt) {
    for (int i = 1; i < MAX_SNAKES; i++) {
        if (snakes[i].alive) updateBot(snakes[i], dt);
    }
    updateFoods(dt);
    updateParticles(dt);
    updateCamera(dt);
    checkCollisions();
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
        if (dt < 0.0f) dt = 0.0f;
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        float ax = 0, ay = 0;
        bool boost = false;
        bool xBtn = false;
        bool select = false;
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            ax = ((float)paddata.ANA_L_H - 128) / 128.0f;
            ay = ((float)paddata.ANA_L_V - 128) / 128.0f;
            if (ax > -0.2f && ax < 0.2f) ax = 0;
            if (ay > -0.2f && ay < 0.2f) ay = 0;
            if (ax == 0 && ay == 0) {
                if (paddata.BTN_LEFT) ax = -1;
                if (paddata.BTN_RIGHT) ax = 1;
                if (paddata.BTN_UP) ay = -1;
                if (paddata.BTN_DOWN) ay = 1;
            }
            if (paddata.BTN_R1 || paddata.BTN_CROSS || paddata.BTN_L1) boost = true;
            if (paddata.BTN_CROSS) xBtn = true;
            if (paddata.BTN_SELECT) select = true;
        }
        if (select) break;
        if (gameState == 1) {
            if (snakes[0].alive) {
                updatePlayer(snakes[0], dt, ax, ay, boost);
            }
            update(dt);
        } else if (gameState == 2) {
            updateFoods(dt);
            updateParticles(dt);
            for (int i = 1; i < MAX_SNAKES; i++) {
                if (snakes[i].alive) updateBot(snakes[i], dt);
            }
            deathTimer -= dt;
            bool xJust = xBtn && !xPressedLast;
            if (xJust || deathTimer <= 0) {
                resetPlayer();
            }
        }
        xPressedLast = xBtn;
        render();
        frameCount++;
    }
}