#include "game.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

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

// ===== PRIMITIVAS =====
void Game::fillRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w;
    int y1 = y + h;
    if (x1 > screen->w) x1 = screen->w;
    if (y1 > screen->h) y1 = screen->h;
    if (x1 <= x0 || y1 <= y0) return;
    SDL_Rect rc;
    rc.x = (Sint16)x0;
    rc.y = (Sint16)y0;
    rc.w = (Uint16)(x1 - x0);
    rc.h = (Uint16)(y1 - y0);
    SDL_FillRect(screen, &rc, SDL_MapRGB(screen->format, r, g, b));
}

void Game::fillCircle(int cx, int cy, int r, Uint8 R, Uint8 G, Uint8 B) {
    if (r <= 0) return;
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)sqrtf((float)(r * r - dy * dy));
        fillRect(cx - dx, cy + dy, dx * 2 + 1, 1, R, G, B);
    }
}

Vec2 Game::worldToScreen(float wx, float wy) {
    return Vec2((wx - camX) * zoom, (wy - camY) * zoom);
}

// ===== TEXTO =====
void Game::drawChar(int x, int y, char c, int scale, Uint8 r, Uint8 g, Uint8 b) {
    if (c == 0) return;
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
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

// ===== MUNDO =====
void Game::drawBackground() {
    int W = screen->w;
    int H = screen->h;
    fillRect(0, 0, W, H, 30, 80, 230);

    float G = 90.0f;
    int k0 = (int)floorf(camX / G);
    for (int k = k0; ; k++) {
        int sx = (int)((k * G - camX) * zoom);
        if (sx > W) break;
        if (sx >= 0) fillRect(sx, 0, 2, H, 140, 180, 255);
    }
    int m0 = (int)floorf(camY / G);
    for (int m = m0; ; m++) {
        int sy = (int)((m * G - camY) * zoom);
        if (sy > H) break;
        if (sy >= 0) fillRect(0, sy, W, 2, 140, 180, 255);
    }
}

void Game::drawBorder() {
    Vec2 c = worldToScreen(0, 0);
    float R = WORLD_RADIUS * zoom;
    int n = (int)(6.2831f * R / 5.0f);
    if (n > 1800) n = 1800;
    if (n < 120) n = 120;
    for (int i = 0; i < n; i++) {
        float a = i * 6.2831f / n;
        int px = (int)(c.x + cosf(a) * R);
        int py = (int)(c.y + sinf(a) * R);
        fillRect(px - 2, py - 2, 5, 5, 255, 60, 60);
    }
}

void Game::drawFood() {
    int W = screen->w;
    int H = screen->h;
    for (int f = 0; f < MAX_FOOD; f++) {
        if (!foods[f].active) continue;
        Vec2 s = worldToScreen(foods[f].pos.x, foods[f].pos.y);
        if (s.x < -30 || s.x > W + 30 || s.y < -30 || s.y > H + 30) continue;
        int R = (int)(foods[f].radius * zoom);
        if (R < 2) R = 2;
        fillCircle((int)s.x, (int)s.y, R, foods[f].r, foods[f].g, foods[f].b);
        if (R >= 5) fillCircle((int)s.x, (int)s.y, R / 3, 255, 255, 255);
    }
}

void Game::drawSnake(Snake &s) {
    if (!s.alive || s.body.empty()) return;
    int W = screen->w;
    int H = screen->h;
    int R = (int)(s.radius * zoom);
    if (R < 3) R = 3;

    for (int k = (int)s.body.size() - 1; k >= 0; k--) {
        Vec2 p = worldToScreen(s.body[k].x, s.body[k].y);
        if (p.x < -R - 4 || p.x > W + R + 4 || p.y < -R - 4 || p.y > H + R + 4) continue;
        bool even = (k % 2) == 0;
        Uint8 cr = even ? s.r1 : s.r2;
        Uint8 cg = even ? s.g1 : s.g2;
        Uint8 cb = even ? s.b1 : s.b2;
        fillCircle((int)p.x, (int)p.y, R, cr, cg, cb);
    }

    // Olhos da cabeça
    Vec2 h = worldToScreen(s.body[0].x, s.body[0].y);
    float fx = cosf(s.angle), fy = sinf(s.angle);
    float px = -fy, py = fx;
    int er = R / 3; if (er < 2) er = 2;
    int pr = er / 2; if (pr < 1) pr = 1;
    float off = R * 0.45f;
    int e1x = (int)(h.x + fx * off + px * off);
    int e1y = (int)(h.y + fy * off + py * off);
    int e2x = (int)(h.x + fx * off - px * off);
    int e2y = (int)(h.y + fy * off - py * off);
    fillCircle(e1x, e1y, er, 255, 255, 255);
    fillCircle(e2x, e2y, er, 255, 255, 255);
    fillCircle((int)(e1x + fx * pr), (int)(e1y + fy * pr), pr, 0, 0, 0);
    fillCircle((int)(e2x + fx * pr), (int)(e2y + fy * pr), pr, 0, 0, 0);
}

void Game::drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!parts[i].active) continue;
        Vec2 s = worldToScreen(parts[i].pos.x, parts[i].pos.y);
        fillCircle((int)s.x, (int)s.y, (int)parts[i].size, parts[i].r, parts[i].g, parts[i].b);
    }
}

void Game::drawMinimap() {
    int W = screen->w;
    int H = screen->h;
    int mx = W - 120;
    int my = H - 120;
    int mr = 90;
    fillCircle(mx, my, mr, 10, 30, 110);
    for (int a = 0; a < 360; a += 3) {
        float rad = a * 3.14159f / 180.0f;
        fillRect(mx + (int)(cosf(rad) * mr) - 1, my + (int)(sinf(rad) * mr) - 1, 2, 2, 255, 255, 255);
    }
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive || snakes[i].body.empty()) continue;
        int px = mx + (int)(snakes[i].body[0].x / WORLD_RADIUS * mr);
        int py = my + (int)(snakes[i].body[0].y / WORLD_RADIUS * mr);
        if (snakes[i].isPlayer) fillRect(px - 2, py - 2, 5, 5, 0, 255, 120);
        else fillRect(px - 2, py - 2, 4, 4, 255, 255, 255);
    }
}

// ===== RANKING =====
void Game::drawRankRow(int x, int y, int boxW, int pos, Snake &s) {
    const int sc = 2;
    Uint8 cr = 255, cg = 255, cb = 255;
    if (s.isPlayer) { cr = 0; cg = 255; cb = 120; }
    else if (pos == 1) { cr = 255; cg = 220; cb = 0; }

    char num[8];
    snprintf(num, sizeof(num), "%d", pos);
    char len[12];
    snprintf(len, sizeof(len), "%d", (int)s.body.size());

    drawText(num, x + 12, y, sc, cr, cg, cb);
    drawText(s.name.c_str(), x + 12 + 4 * 6 * sc, y, sc, cr, cg, cb);
    drawText(len, x + boxW - 12 - textWidth(len, sc), y, sc, cr, cg, cb);
}

void Game::drawRanking() {
    const int sc = 2;
    const int rowH = 24;
    const int boxW = 240;
    int rows = RANK_SHOW + 3;
    int boxH = rows * rowH + 24;
    int x = screen->w - boxW - 16;
    int y = 16;

    fillRect(x, y, boxW, boxH, 255, 255, 255);
    fillRect(x + 3, y + 3, boxW - 6, boxH - 6, 10, 10, 28);
    drawText("RANKING", x + 12, y + 10, sc, 255, 220, 0);

    int shown = rankCount < RANK_SHOW ? rankCount : RANK_SHOW;
    for (int i = 0; i < shown; i++) {
        drawRankRow(x, y + 12 + (i + 1) * rowH, boxW, i + 1, snakes[rankOrder[i]]);
    }
    if (snakes[0].alive && playerRank > shown) {
        drawRankRow(x, y + 12 + (RANK_SHOW + 2) * rowH, boxW, playerRank, snakes[0]);
    }
}

// ===== HUD / TELAS =====
void Game::drawHUD() {
    char buf[64];
    snprintf(buf, sizeof(buf), "SCORE %d", snakes[0].score);
    drawText(buf, 20, 20, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "TAMANHO %d", (int)snakes[0].body.size());
    drawText(buf, 20, 66, 2, 0, 255, 120);

    snprintf(buf, sizeof(buf), "POSICAO %d DE %d", playerRank, rankCount);
    drawText(buf, 20, 92, 2, 255, 255, 255);

    if (snakes[0].boosting) drawText("TURBO", 20, 118, 2, 255, 220, 0);
}

void Game::renderMenu() {
    fillRect(0, 0, screen->w, screen->h, 30, 80, 230);
    int cy = screen->h / 2;
    drawTextCentered("SLIPS3", cy - 230, 12, 40, 255, 120);
    drawTextCentered("ANALOGICO ESQUERDO: MOVER", cy - 60, 3, 255, 255, 255);
    drawTextCentered("X OU R2: TURBO", cy - 20, 3, 255, 255, 255);
    drawTextCentered("COMA COMIDA PARA CRESCER. NAO BATA NOS OUTROS!", cy + 20, 3, 255, 255, 255);

    if (bestScore > 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "MELHOR PONTUACAO %d", bestScore);
        drawTextCentered(buf, cy + 80, 3, 255, 220, 0);
    }
    if ((frame / 30) % 2 == 0) {
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

    snprintf(buf, sizeof(buf), "TAMANHO %d", deathLen);
    drawTextCentered(buf, py + 210, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "ABATES %d", deathKills);
    drawTextCentered(buf, py + 260, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "POSICAO %d", deathRank);
    drawTextCentered(buf, py + 310, 3, 255, 255, 255);

    snprintf(buf, sizeof(buf), "MELHOR %d", bestScore);
    drawTextCentered(buf, py + 360, 3, 255, 220, 0);

    if ((frame / 30) % 2 == 0) {
        drawTextCentered("PRESSIONE START PARA JOGAR", py + 420, 3, 255, 255, 255);
    }
}

void Game::drawWorld() {
    drawBackground();
    drawBorder();
    drawFood();
    for (int i = 1; i < MAX_SNAKES; i++) drawSnake(snakes[i]);
    drawSnake(snakes[0]);
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
    if (gameState == 2) drawDeathPanel();
    else drawHUD();
}