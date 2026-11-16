#include "game.h"
#include <math.h>
#include <cstdio>
#include <cstring>

// Fonte 5x7 em pixels. Só maiúsculas, números e alguns símbolos.
static const char FONT_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:-!.";
static const Uint8 FONT[40][7] = {
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 0
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, // 1
    {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}, // 2
    {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}, // 3
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, // 4
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, // 5
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, // 6
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, // 7
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 8
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, // 9
    {0x0E,0x11,0x11,0x11,0x1F,0x11,0x11}, // A
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // B
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // C
    {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C}, // D
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // E
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}, // G
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // H
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, // I
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C}, // J
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // L
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, // M
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11}, // N
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // O
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, // Q
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
    {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}, // S
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // T
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // U
    {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}, // V
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0A}, // W
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // X
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, // Y
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}, // Z
    {0x00,0x04,0x04,0x00,0x04,0x04,0x00}, // :
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // -
    {0x04,0x04,0x04,0x04,0x04,0x00,0x04}, // !
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C}  // .
};

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

void Game::fillRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Rect rc = { (Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h };
    SDL_FillRect(screen, &rc, getColor(r, g, b));
}

void Game::drawChar(int x, int y, char c, int scale, Uint8 r, Uint8 g, Uint8 b) {
    if (c == 0 || c == ' ') return;
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    const char* p = strchr(FONT_CHARS, c);
    if (!p) return;
    int idx = (int)(p - FONT_CHARS);
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (FONT[idx][row] & (1 << (4 - col))) {
                fillRect(x + col * scale, y + row * scale, scale, scale, r, g, b);
            }
        }
    }
}

int Game::textWidth(const char* text, int scale) {
    int n = (int)strlen(text);
    if (n == 0) return 0;
    return n * 6 * scale - scale;
}

void Game::drawText(const char* text, int x, int y, int scale, Uint8 r, Uint8 g, Uint8 b) {
    for (int i = 0; text[i] != 0; i++) {
        drawChar(x + i * 6 * scale, y, text[i], scale, r, g, b);
    }
}

void Game::drawTextCentered(const char* text, int y, int scale, Uint8 r, Uint8 g, Uint8 b) {
    int x = (screen->w - textWidth(text, scale)) / 2;
    drawText(text, x, y, scale, r, g, b);
}

void Game::drawBackground() {
    fillRect(0, 0, screen->w, screen->h, 12, 12, 22);
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
            fillRect(x, yy, 2, 2, 35, 35, 55);
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
    int W = screen->w; int H = screen->h;
    int mx = W - 110; int my = H - 110; int mr = 90;
    drawFilledCircle(mx, my, mr, 20, 20, 35);
    drawCircleOutline(mx, my, mr, 255, 255, 255);
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        if (snakes[i].body.empty()) continue;
        int px = mx + (int)(snakes[i].body[0].x / WORLD_RADIUS * mr);
        int py = my + (int)(snakes[i].body[0].y / WORLD_RADIUS * mr);
        Uint8 r = snakes[i].isPlayer ? 0 : 255;
        Uint8 g = 255;
        Uint8 b = snakes[i].isPlayer ? 0 : 255;
        fillRect(px-2, py-2, 4, 4, r, g, b);
    }
}

// ===== RANKING (canto superior direito) =====
void Game::drawRankRow(int x, int y, int boxW, int pos, Snake &s) {
    int scale = 2;
    Uint8 cr = 255, cg = 255, cb = 255;
    if (s.isPlayer) { cr = 0; cg = 255; cb = 120; }
    else if (pos == 1) { cr = 255; cg = 220; cb = 0; }

    char num[8];
    snprintf(num, sizeof(num), "%d", pos);
    char len[12];
    snprintf(len, sizeof(len), "%d", (int)s.length);

    drawText(num, x + 12, y, scale, cr, cg, cb);
    drawText(s.name.c_str(), x + 12 + 3 * 6 * scale, y, scale, cr, cg, cb);
    drawText(len, x + boxW - 12 - textWidth(len, scale), y, scale, cr, cg, cb);
}

void Game::drawRanking() {
    int scale = 2;
    int rowH = 8 * scale + 8;
    int boxW = 230;
    int rows = RANK_SHOW + 3;
    int boxH = rows * rowH + 24;
    int x = screen->w - boxW - 16;
    int y = 16;

    fillRect(x, y, boxW, boxH, 255, 255, 255);
    fillRect(x + 2, y + 2, boxW - 4, boxH - 4, 10, 10, 25);
    drawText("RANKING", x + 12, y + 10, scale, 255, 220, 0);

    int shown = rankCount < RANK_SHOW ? rankCount : RANK_SHOW;
    for (int i = 0; i < shown; i++) {
        drawRankRow(x, y + 12 + (i + 1) * rowH, boxW, i + 1, snakes[rankOrder[i]]);
    }
    // Se o jogador estiver fora do top 8, mostra a linha dele embaixo
    if (snakes[0].alive && playerRank > shown) {
        drawRankRow(x, y + 12 + (RANK_SHOW + 2) * rowH, boxW, playerRank, snakes[0]);
    }
}

// ===== HUD =====
void Game::drawHUD() {
    char buf[64];
    snprintf(buf, sizeof(buf), "SCORE %d", snakes[0].score);
    drawText(buf, 20, 20, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "TAMANHO %d", (int)snakes[0].length);
    drawText(buf, 20, 66, 2, 0, 255, 120);

    snprintf(buf, sizeof(buf), "POSICAO %d DE %d", playerRank, rankCount);
    drawText(buf, 20, 92, 2, 255, 255, 255);

    if (snakes[0].boosting) {
        drawText("TURBO", 20, 118, 2, 255, 220, 0);
    }
}

// ===== TELAS =====
void Game::renderMenu() {
    fillRect(0, 0, screen->w, screen->h, 10, 10, 20);
    int cy = screen->h / 2;
    drawTextCentered("SLIPS3", cy - 230, 12, 0, 255, 150);
    drawTextCentered("ANALOGICO ESQUERDO: MOVER", cy - 60, 3, 255, 255, 255);
    drawTextCentered("X OU R2: TURBO", cy - 20, 3, 255, 255, 255);
    drawTextCentered("COMA COMIDA E CRESCA. NAO BATA NOS OUTROS!", cy + 20, 3, 255, 255, 255);

    if (bestScore > 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "MELHOR PONTUACAO %d", bestScore);
        drawTextCentered(buf, cy + 80, 3, 255, 220, 0);
    }
    if ((frameCount / 30) % 2 == 0) {
        drawTextCentered("PRESSIONE START PARA JOGAR", cy + 160, 3, 255, 255, 255);
    }
}

void Game::drawDeathPanel() {
    int pw = 760;
    int ph = 480;
    int px = (screen->w - pw) / 2;
    int py = (screen->h - ph) / 2;
    fillRect(px, py, pw, ph, 255, 80, 80);
    fillRect(px + 4, py + 4, pw - 8, ph - 8, 20, 6, 12);

    char buf[64];
    drawTextCentered("VOCE BATEU!", py + 30, 6, 255, 80, 80);

    snprintf(buf, sizeof(buf), "PONTUACAO %d", deathScore);
    drawTextCentered(buf, py + 160, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "TAMANHO %d", deathLength);
    drawTextCentered(buf, py + 210, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "ABATES %d", deathKills);
    drawTextCentered(buf, py + 260, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "POSICAO %d", deathRank);
    drawTextCentered(buf, py + 310, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "MELHOR %d", bestScore);
    drawTextCentered(buf, py + 360, 3, 255, 220, 0);

    if ((frameCount / 30) % 2 == 0) {
        drawTextCentered("PRESSIONE START PARA JOGAR", py + 420, 3, 255, 255, 255);
    }
}

void Game::drawWorld() {
    drawBackground();
    drawBorder();
    drawFood();
    for (int i = 0; i < MAX_SNAKES; i++) drawSnake(snakes[i]);
    drawParticles();
    drawMinimap();
    drawRanking();
}

void Game::render() {
    if (gameState == 0) {
        renderMenu();
        return;
    }
    drawWorld();
    if (gameState == 2) {
        drawDeathPanel();
    } else {
        drawHUD();
    }
}