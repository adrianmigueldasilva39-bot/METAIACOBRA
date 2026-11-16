#include "game.h"
#include <cstdio>

void Game::drawFilledCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    if (radius <= 0) return;
    // Usa FillRect, não setPixel - 100x mais rápido, não trava
    SDL_Rect rect = { (Sint16)(cx - radius), (Sint16)(cy - radius), (Uint16)(radius*2), (Uint16)(radius*2) };
    SDL_FillRect(screen, &rect, getColor(r,g,b));
}

void Game::drawCircleOutline(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    // Só 4 pontos pra não travar, não 360 setPixel
    SDL_Rect rect;
    Uint32 c = getColor(r,g,b);
    rect = { (Sint16)(cx - radius), (Sint16)(cy - 1), (Uint16)(radius*2), 2 }; SDL_FillRect(screen, &rect, c);
    rect = { (Sint16)(cx - 1), (Sint16)(cy - radius), 2, (Uint16)(radius*2) }; SDL_FillRect(screen, &rect, c);
}

void Game::drawGlowCircle(int sx, int sy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {
    (void)alpha;
    drawFilledCircle(sx, sy, radius, r, g, b);
}

void Game::drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, bool big) {
    int s = big ? 4 : 2;
    int cx = x;
    for (int i = 0; text[i] != 0; i++) {
        if (text[i] == ' ') { cx += 10*s; continue; }
        SDL_Rect rect = { (Sint16)cx, (Sint16)y, (Uint16)(6*s), (Uint16)(8*s) };
        SDL_FillRect(screen, &rect, getColor(r,g,b));
        cx += 10*s;
    }
}

void Game::drawBackground() {
    SDL_FillRect(screen, NULL, getColor(12, 12, 22));
    int W = screen->w;
    int H = screen->h;
    int grid = (int)(60 * zoom);
    if (grid < 20) grid = 20;
    int offX = (int)(cameraX * zoom) % grid;
    if (offX < 0) offX += grid;
    int offY = (int)(cameraY * zoom) % grid;
    if (offY < 0) offY += grid;
    for (int x = -offX; x < W; x += grid) {
        for (int yy = -offY; yy < H; yy += grid) {
            SDL_Rect r = { (Sint16)x, (Sint16)yy, (Uint16)2, (Uint16)2 };
            SDL_FillRect(screen, &r, getColor(35, 35, 55));
        }
    }
}

void Game::drawBorder() {
    Vec2 c = worldToScreen(0, 0);
    int rad = (int)(WORLD_RADIUS * zoom);
    drawCircleOutline((int)c.x, (int)c.y, rad, 255, 40, 40);
}

void Game::drawFood() {
    int W = screen->w; int H = screen->h;
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].active) continue;
        Vec2 s = worldToScreen(foods[i].pos.x, foods[i].pos.y);
        if (s.x < -20 || s.x > W+20 || s.y < -20 || s.y > H+20) continue;
        int rad = (int)(foods[i].radius * zoom * (1.0f + 0.15f * sin(foods[i].pulse)));
        if (rad < 2) rad = 2;
        drawGlowCircle((int)s.x, (int)s.y, rad, foods[i].r, foods[i].g, foods[i].b, 255);
    }
}

void Game::drawSnake(Snake &s) {
    if (!s.alive) return;
    int W = screen->w; int H = screen->h;
    for (int i = (int)s.body.size()-1; i >= 0; i--) {
        Vec2 sc = worldToScreen(s.body[i].x, s.body[i].y);
        if (sc.x < -40 || sc.x > W+40 || sc.y < -40 || sc.y > H+40) continue;
        int rad = (int)(s.radius * zoom);
        if (rad < 3) rad = 3;
        Uint8 r = (i % 2 == 0) ? s.colorR : s.colorR2;
        Uint8 g = (i % 2 == 0) ? s.colorG : s.colorG2;
        Uint8 b = (i % 2 == 0) ? s.colorB : s.colorB2;
        drawFilledCircle((int)sc.x, (int)sc.y, rad, r, g, b);
    }
    if (s.body.size() > 0) {
        Vec2 h = worldToScreen(s.body[0].x, s.body[0].y);
        float ex = cos(s.angle) * s.radius * 0.5f * zoom;
        float ey = sin(s.angle) * s.radius * 0.5f * zoom;
        drawFilledCircle((int)(h.x+ex-4), (int)(h.y+ey-4), 4, 255, 255, 255);
        drawFilledCircle((int)(h.x+ex+4), (int)(h.y+ey-4), 4, 255, 255, 255);
    }
}

void Game::drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        Vec2 s = worldToScreen(particles[i].pos.x, particles[i].pos.y);
        drawFilledCircle((int)s.x, (int)s.y, (int)particles[i].size, particles[i].r, particles[i].g, particles[i].b);
    }
}

void Game::drawMinimap() {
    int W = screen->w; int H = screen->h;
    int mx = W - 110; int my = H - 110; int mr = 90;
    drawFilledCircle(mx, my, mr, 20, 20, 35);
    drawCircleOutline(mx, my, mr, 255, 255, 255);
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        if (snakes[i].body.empty()) continue;
        int px = mx + (int)(snakes[i].body[0].x / WORLD_RADIUS * mr);
        int py = my + (int)(snakes[i].body[0].y / WORLD_RADIUS * mr);
        Uint32 c = snakes[i].isPlayer ? getColor(0,255,0) : getColor(255,255,255);
        SDL_Rect r = { (Sint16)(px-2), (Sint16)(py-2), 4, 4 };
        SDL_FillRect(screen, &r, c);
    }
}

void Game::drawHUD() {
    char buf[64];
    snprintf(buf, sizeof(buf), "SCORE %d", snakes[0].score);
    drawText(buf, 20, 20, 255, 255, 255, true);
}

void Game::renderMenu() {
    SDL_FillRect(screen, NULL, getColor(10,10,20));
    drawText("SLIPS3", screen->w/2 - 80, screen->h/2 - 80, 0, 255, 150, true);
    drawText("PRESSIONE START PARA JOGAR", screen->w/2 - 180, screen->h/2 + 20, 255, 255, 255, true);
}

void Game::renderGameOver() {
    SDL_FillRect(screen, NULL, getColor(20,5,5));
    drawText("VOCE BATEU!", screen->w/2 - 80, screen->h/2 - 80, 255, 50, 50, true);
    drawText("PRESSIONE START DE NOVO", screen->w/2 - 180, screen->h/2 + 20, 255, 255, 255, true);
}

void Game::render() {
    if (gameState == 0) { renderMenu(); }
    else if (gameState == 2) { renderGameOver(); }
    else {
        drawBackground();
        drawBorder();
        drawFood();
        for (int i = 0; i < MAX_SNAKES; i++) drawSnake(snakes[i]);
        drawParticles();
        drawMinimap();
        drawHUD();
    }
    // ISSO AQUI FALTAVA - sem isso fica tela preta pra sempre
    SDL_Flip(screen);
}