#include "game.h"
#include <cstdio>

Vec2 Game::worldToScreen(float wx, float wy) {
    Vec2 s;
    s.x = (wx - cameraX) * zoom + SCREEN_W / 2;
    s.y = (wy - cameraY) * zoom + SCREEN_H / 2;
    return s;
}
Uint32 Game::getColor(Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(screen->format, r, g, b);
}
void Game::setPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    Uint32 c = getColor(r,g,b);
    Uint32 *p = (Uint32*)screen->pixels + y * screen->pitch / 4 + x;
    *p = c;
}
void Game::drawFilledCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    if (radius <= 0) return;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) setPixel(cx+x, cy+y, r, g, b);
        }
    }
}
void Game::drawCircleOutline(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    for (int a = 0; a < 360; a++) {
        float rad = a * 3.14159f / 180.0f;
        setPixel(cx + (int)(cos(rad)*radius), cy + (int)(sin(rad)*radius), r, g, b);
    }
}
void Game::drawGlowCircle(int sx, int sy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {
    (void)alpha;
    drawFilledCircle(sx, sy, radius, r, g, b);
    drawFilledCircle(sx, sy, radius/2, 255, 255, 255);
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
    int grid = (int)(60 * zoom);
    if (grid < 20) grid = 20;
    int offX = (int)((cameraX * zoom) ) % grid;
    int offY = (int)((cameraY * zoom) ) % grid;
    for (int x = -offX; x < SCREEN_W; x += grid) {
        for (int yy = -offY; yy < SCREEN_H; yy += grid) {
            SDL_Rect r = { (Sint16)x, (Sint16)yy, (Uint16)2, (Uint16)2 };
            SDL_FillRect(screen, &r, getColor(35, 35, 55));
        }
    }
}
void Game::drawBorder() {
    Vec2 c = worldToScreen(0, 0);
    int rad = (int)(WORLD_RADIUS * zoom);
    drawCircleOutline((int)c.x, (int)c.y, rad, 255, 40, 40);
    drawCircleOutline((int)c.x, (int)c.y, rad-2, 255, 80, 80);
}
void Game::drawFood() {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].active) continue;
        Vec2 s = worldToScreen(foods[i].pos.x, foods[i].pos.y);
        if (s.x < -20 || s.x > SCREEN_W+20 || s.y < -20 || s.y > SCREEN_H+20) continue;
        int rad = (int)(foods[i].radius * zoom * (1.0f + 0.15f * sin(foods[i].pulse)));
        if (rad < 2) rad = 2;
        drawGlowCircle((int)s.x, (int)s.y, rad, foods[i].r, foods[i].g, foods[i].b, 255);
    }
}
void Game::drawSnake(Snake &s) {
    if (!s.alive) return;
    for (int i = (int)s.body.size()-1; i >= 0; i--) {
        Vec2 sc = worldToScreen(s.body[i].x, s.body[i].y);
        if (sc.x < -40 || sc.x > SCREEN_W+40 || sc.y < -40 || sc.y > SCREEN_H+40) continue;
        int rad = (int)(s.radius * zoom);
        if (rad < 3) rad = 3;
        Uint8 r = (i % 2 == 0) ? s.colorR : s.colorR2;
        Uint8 g = (i % 2 == 0) ? s.colorG : s.colorG2;
        Uint8 b = (i % 2 == 0) ? s.colorB : s.colorB2;
        drawFilledCircle((int)sc.x, (int)sc.y, rad, r, g, b);
        drawFilledCircle((int)sc.x-2, (int)sc.y-2, rad/2, 255, 255, 255);
    }
    if (s.body.size() > 0) {
        Vec2 h = worldToScreen(s.body[0].x, s.body[0].y);
        float ex = cos(s.angle) * s.radius * 0.5f * zoom;
        float ey = sin(s.angle) * s.radius * 0.5f * zoom;
        drawFilledCircle((int)(h.x+ex-4), (int)(h.y+ey-4), 4, 255, 255, 255);
        drawFilledCircle((int)(h.x+ex+4), (int)(h.y+ey-4), 4, 255, 255, 255);
        drawFilledCircle((int)(h.x+ex-4), (int)(h.y+ey-4), 2, 0, 0, 0);
        drawFilledCircle((int)(h.x+ex+4), (int)(h.y+ey-4), 2, 0, 0, 0);
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
    int mx = SCREEN_W - 110; int my = SCREEN_H - 110; int mr = 90;
    drawFilledCircle(mx, my, mr, 20, 20, 35);
    drawCircleOutline(mx, my, mr, 255, 255, 255);
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
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
    drawText("SLIPS3", 500, 250, 0, 255, 150, true);
    drawText("PRESSIONE START PARA JOGAR", 380, 380, 255, 255, 255, true);
}
void Game::renderGameOver() {
    SDL_FillRect(screen, NULL, getColor(20,5,5));
    drawText("VOCE BATEU!", 500, 250, 255, 50, 50, true);
    drawText("PRESSIONE START DE NOVO", 380, 380, 255, 255, 255, true);
}
void Game::render() {
    if (gameState == 0) { renderMenu(); return; }
    if (gameState == 2) { renderGameOver(); return; }
    drawBackground();
    drawBorder();
    drawFood();
    for (int i = 0; i < MAX_SNAKES; i++) drawSnake(snakes[i]);
    drawParticles();
    drawMinimap();
    drawHUD();
}