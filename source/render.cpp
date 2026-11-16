#include "game.h"
#include <cstdio>

static const Uint8 FONT_5x7[128][7] = {
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},
    {0x04,0x04,0x04,0x04,0x04,0x00,0x04},
    {0x0A,0x0A,0x0A,0x00,0x00,0x00,0x00},
    {0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x0A},
    {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},
    {0x18,0x19,0x02,0x04,0x08,0x13,0x18},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x08,0x04,0x02,0x02,0x02,0x04,0x08},
    {0x02,0x04,0x08,0x08,0x08,0x04,0x02},
    {0x00,0x04,0x15,0x0E,0x15,0x04,0x00},
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    {0x00,0x00,0x00,0x00,0x0C,0x04,0x08},
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C},
    {0x01,0x02,0x02,0x04,0x08,0x10,0x10},
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},
    {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E},
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    {0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00},
    {0x00,0x0C,0x0C,0x00,0x0C,0x04,0x08},
    {0x02,0x04,0x08,0x10,0x08,0x04,0x02},
    {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},
    {0x08,0x04,0x02,0x01,0x02,0x04,0x08},
    {0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
    {0x0E,0x11,0x01,0x0D,0x15,0x15,0x0E},
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F},
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    {0x01,0x01,0x01,0x01,0x11,0x11,0x0E},
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    {0x11,0x11,0x1B,0x15,0x11,0x11,0x11},
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x11,0x11,0x0A,0x04},
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0A},
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
    {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E},
    {0x10,0x08,0x04,0x02,0x01,0x01,0x01},
    {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E},
    {0x04,0x0A,0x11,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x1F},
    {0x08,0x04,0x02,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F},
    {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E},
    {0x00,0x00,0x0E,0x11,0x10,0x11,0x0E},
    {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F},
    {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E},
    {0x06,0x08,0x1E,0x08,0x08,0x08,0x08},
    {0x00,0x0F,0x11,0x11,0x0F,0x01,0x0E},
    {0x10,0x10,0x16,0x19,0x11,0x11,0x11},
    {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E},
    {0x02,0x00,0x06,0x02,0x02,0x12,0x0C},
    {0x10,0x10,0x12,0x14,0x18,0x14,0x12},
    {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E},
    {0x00,0x00,0x1A,0x15,0x15,0x11,0x11},
    {0x00,0x00,0x1E,0x11,0x11,0x11,0x11},
    {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E},
    {0x00,0x00,0x1E,0x11,0x11,0x1E,0x10},
    {0x00,0x00,0x0F,0x11,0x11,0x0F,0x01},
    {0x00,0x00,0x16,0x19,0x10,0x10,0x10},
    {0x00,0x00,0x0F,0x10,0x0E,0x01,0x1E},
    {0x08,0x08,0x1E,0x08,0x08,0x09,0x06},
    {0x00,0x00,0x11,0x11,0x11,0x13,0x0D},
    {0x00,0x00,0x11,0x11,0x11,0x0A,0x04},
    {0x00,0x00,0x11,0x15,0x15,0x15,0x0A},
    {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11},
    {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E},
    {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F},
    {0x06,0x08,0x08,0x0E,0x08,0x08,0x06},
    {0x04,0x04,0x04,0x04,0x04,0x04,0x04},
    {0x0C,0x02,0x02,0x0E,0x02,0x02,0x0C},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

void Game::drawFilledCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    if (radius <= 0) return;
    Uint32 c = getColor(r, g, b);
    for (int y = -radius; y <= radius; y++) {
        int w = (int)sqrtf((float)(radius * radius - y * y));
        SDL_Rect rc = {(Sint16)(cx - w), (Sint16)(cy + y), (Uint16)(w * 2), 1};
        SDL_FillRect(screen, &rc, c);
    }
}

void Game::drawCircleOutline(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    Uint32 c = getColor(r, g, b);
    for (int a = 0; a < 360; a += 3) {
        float rad = a * 3.14159f / 180.0f;
        SDL_Rect rc = {(Sint16)(cx + (int)(cosf(rad) * radius)), (Sint16)(cy + (int)(sinf(rad) * radius)), 2, 2};
        SDL_FillRect(screen, &rc, c);
    }
}

void Game::drawLine(int x0, int y0, int x1, int y1, Uint8 r, Uint8 g, Uint8 b) {
    Uint32 c = getColor(r, g, b);
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int x = x0;
    int y = y0;
    while (true) {
        if (x >= 0 && y >= 0 && x < screen->w && y < screen->h) {
            SDL_Rect rc = {(Sint16)x, (Sint16)y, 1, 1};
            SDL_FillRect(screen, &rc, c);
        }
        if (x == x1 && y == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

void Game::drawGlowCircle(int sx, int sy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    drawFilledCircle(sx, sy, radius + 7, r / 5, g / 5, b / 5);
    drawFilledCircle(sx, sy, radius + 2, r / 2, g / 2, b / 2);
    drawFilledCircle(sx, sy, radius, r, g, b);
    drawFilledCircle(sx, sy, radius / 2, 255, 255, 255);
}

void Game::drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, bool big) {
    int s = big ? 3 : 2;
    int cx = x;
    Uint32 c = getColor(r, g, b);
    for (int i = 0; text[i] != 0; i++) {
        unsigned char ch = (unsigned char)text[i];
        if (ch == ' ') {
            cx += (5 + 1) * s;
            continue;
        }
        if (ch < 32 || ch > 126) {
            cx += (5 + 1) * s;
            continue;
        }
        for (int row = 0; row < 7; row++) {
            Uint8 bits = FONT_5x7[ch][row];
            for (int col = 0; col < 5; col++) {
                if (bits & (0x10 >> col)) {
                    SDL_Rect rc = {(Sint16)(cx + col * s), (Sint16)(y + row * s), (Uint16)s, (Uint16)s};
                    SDL_FillRect(screen, &rc, c);
                }
            }
        }
        cx += (5 + 1) * s;
    }
}

void Game::drawBackground() {
    SDL_FillRect(screen, NULL, getColor(12, 14, 22));
    float hexR = 70.0f;
    float stepX = hexR * 1.5f;
    float stepY = hexR * 1.732f;
    float camLeft = cameraX - (screen->w / 2) / zoom - hexR * 2;
    float camTop = cameraY - (screen->h / 2) / zoom - hexR * 2;
    float camRight = cameraX + (screen->w / 2) / zoom + hexR * 2;
    float camBottom = cameraY + (screen->h / 2) / zoom + hexR * 2;
    int gxStart = (int)floorf(camLeft / stepX) - 1;
    int gxEnd = (int)floorf(camRight / stepX) + 1;
    int gyStart = (int)floorf(camTop / stepY) - 1;
    int gyEnd = (int)floorf(camBottom / stepY) + 1;
    for (int gy = gyStart; gy <= gyEnd; gy++) {
        for (int gx = gxStart; gx <= gxEnd; gx++) {
            float wx = gx * stepX + ((gy & 1) ? stepX / 2.0f : 0.0f);
            float wy = gy * stepY;
            Vec2 prev = worldToScreen(wx + cosf(0) * hexR, wy + sinf(0) * hexR);
            for (int k = 1; k <= 6; k++) {
                float a = k * 1.0471975f;
                Vec2 cur = worldToScreen(wx + cosf(a) * hexR, wy + sinf(a) * hexR);
                drawLine((int)prev.x, (int)prev.y, (int)cur.x, (int)cur.y, 38, 42, 58);
                prev = cur;
            }
        }
    }
}

void Game::drawBorder() {
    int W = screen->w;
    int H = screen->h;
    for (int deg = 0; deg < 360; deg++) {
        float a1 = deg * 3.14159f / 180.0f;
        float a2 = (deg + 1) * 3.14159f / 180.0f;
        Vec2 p1 = worldToScreen(cosf(a1) * WORLD_RADIUS, sinf(a1) * WORLD_RADIUS);
        Vec2 p2 = worldToScreen(cosf(a2) * WORLD_RADIUS, sinf(a2) * WORLD_RADIUS);
        if (p1.x < -50 && p2.x < -50) continue;
        if (p1.x > W + 50 && p2.x > W + 50) continue;
        if (p1.y < -50 && p2.y < -50) continue;
        if (p1.y > H + 50 && p2.y > H + 50) continue;
        drawLine((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, 255, 40, 40);
        Vec2 q1 = worldToScreen(cosf(a1) * (WORLD_RADIUS - 6), sinf(a1) * (WORLD_RADIUS - 6));
        Vec2 q2 = worldToScreen(cosf(a2) * (WORLD_RADIUS - 6), sinf(a2) * (WORLD_RADIUS - 6));
        drawLine((int)q1.x, (int)q1.y, (int)q2.x, (int)q2.y, 255, 80, 80);
    }
}

void Game::drawFood() {
    int W = screen->w;
    int H = screen->h;
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].active) continue;
        Vec2 s = worldToScreen(foods[i].pos.x, foods[i].pos.y);
        if (s.x < -30 || s.x > W + 30 || s.y < -30 || s.y > H + 30) continue;
        int rad = (int)(foods[i].radius * zoom * (1.0f + 0.22f * sinf(foods[i].pulse)));
        if (rad < 2) rad = 2;
        drawGlowCircle((int)s.x, (int)s.y, rad, foods[i].r, foods[i].g, foods[i].b);
    }
}

void Game::drawSnake(Snake& s) {
    if (!s.alive) return;
    int W = screen->w;
    int H = screen->h;
    for (int i = (int)s.body.size() - 1; i >= 0; i--) {
        Vec2 sc = worldToScreen(s.body[i].x, s.body[i].y);
        if (sc.x < -60 || sc.x > W + 60 || sc.y < -60 || sc.y > H + 60) continue;
        int rad = (int)(s.radius * zoom);
        if (rad < 4) rad = 4;
        int pat = (i / 6) % s.skinLen;
        Uint8 r = s.cols[pat][0];
        Uint8 g = s.cols[pat][1];
        Uint8 b = s.cols[pat][2];
        float t = (float)i / (float)(s.body.size() > 1 ? s.body.size() - 1 : 1);
        Uint8 fr = (Uint8)(r * (1.0f - t * 0.25f));
        Uint8 fg = (Uint8)(g * (1.0f - t * 0.25f));
        Uint8 fb = (Uint8)(b * (1.0f - t * 0.25f));
        drawFilledCircle((int)sc.x + 1, (int)sc.y + 2, rad, 8, 8, 12);
        drawFilledCircle((int)sc.x, (int)sc.y, rad, fr, fg, fb);
        drawFilledCircle((int)sc.x, (int)sc.y, rad - 2, (Uint8)(fr * 0.92f), (Uint8)(fg * 0.92f), (Uint8)(fb * 0.92f));
        drawFilledCircle((int)sc.x - rad / 3, (int)sc.y - rad / 3, rad / 3, 255, 255, 255);
    }
    if (!s.body.empty()) {
        Vec2 h = worldToScreen(s.body[0].x, s.body[0].y);
        int rad = (int)(s.radius * zoom * 1.15f);
        Uint8 r = s.cols[0][0];
        Uint8 g = s.cols[0][1];
        Uint8 b = s.cols[0][2];
        drawFilledCircle((int)h.x + 1, (int)h.y + 2, rad, 8, 8, 12);
        drawFilledCircle((int)h.x, (int)h.y, rad, r, g, b);
        drawFilledCircle((int)h.x, (int)h.y, rad - 2, (Uint8)(r * 0.92f), (Uint8)(g * 0.92f), (Uint8)(b * 0.92f));
        float px = -sinf(s.angle);
        float py = cosf(s.angle);
        float ex1 = h.x + cosf(s.angle) * rad * 0.65f + px * rad * 0.55f;
        float ey1 = h.y + sinf(s.angle) * rad * 0.65f + py * rad * 0.55f;
        float ex2 = h.x + cosf(s.angle) * rad * 0.65f - px * rad * 0.55f;
        float ey2 = h.y + sinf(s.angle) * rad * 0.65f - py * rad * 0.55f;
        drawFilledCircle((int)ex1, (int)ey1, rad / 2, r, g, b);
        drawFilledCircle((int)ex2, (int)ey2, rad / 2, r, g, b);
        drawFilledCircle((int)ex1, (int)ey1, rad / 3, 255, 255, 255);
        drawFilledCircle((int)ex2, (int)ey2, rad / 3, 255, 255, 255);
        drawFilledCircle((int)ex1 + 1, (int)ey1 + 1, rad / 5, 0, 0, 0);
        drawFilledCircle((int)ex2 + 1, (int)ey2 + 1, rad / 5, 0, 0, 0);
        drawFilledCircle((int)ex1 - 1, (int)ey1 - 1, 2, 255, 255, 255);
        drawFilledCircle((int)ex2 - 1, (int)ey2 - 1, 2, 255, 255, 255);
        drawFilledCircle((int)(h.x + cosf(s.angle) * rad * 0.8f), (int)(h.y + sinf(s.angle) * rad * 0.8f), 3, 0, 0, 0);
    }
}

void Game::drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        Vec2 s = worldToScreen(particles[i].pos.x, particles[i].pos.y);
        drawGlowCircle((int)s.x, (int)s.y, (int)particles[i].size, particles[i].r, particles[i].g, particles[i].b);
    }
}

void Game::drawMinimap() {
    int W = screen->w;
    int H = screen->h;
    int mx = W - 115;
    int my = H - 115;
    int mr = 95;
    drawFilledCircle(mx, my, mr, 18, 20, 32);
    drawCircleOutline(mx, my, mr, 255, 255, 255);
    drawCircleOutline(mx, my, mr - 2, 120, 130, 150);
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        if (snakes[i].body.empty()) continue;
        int px = mx + (int)(snakes[i].body[0].x / WORLD_RADIUS * mr);
        int py = my + (int)(snakes[i].body[0].y / WORLD_RADIUS * mr);
        Uint32 c = snakes[i].isPlayer ? getColor(0, 255, 120) : getColor(255, 255, 255);
        SDL_Rect rc = {(Sint16)(px - 3), (Sint16)(py - 3), 6, 6};
        SDL_FillRect(screen, &rc, c);
    }
}

void Game::drawLeaderboard() {
    int W = screen->w;
    int lx = W - 260;
    int ly = 10;
    drawText("LEADERBOARD", lx, ly, 255, 255, 255, false);
    int order[MAX_SNAKES];
    for (int i = 0; i < MAX_SNAKES; i++) order[i] = i;
    for (int i = 0; i < MAX_SNAKES; i++) {
        for (int j = i + 1; j < MAX_SNAKES; j++) {
            if (snakes[order[j]].score > snakes[order[i]].score) {
                int t = order[i];
                order[i] = order[j];
                order[j] = t;
            }
        }
    }
    for (int k = 0; k < 5 && k < MAX_SNAKES; k++) {
        int idx = order[k];
        if (!snakes[idx].alive) continue;
        char buf[64];
        snprintf(buf, sizeof(buf), "#%d %d", k + 1, snakes[idx].score);
        Uint8 r = snakes[idx].isPlayer ? 0 : 255;
        Uint8 g = 255;
        Uint8 b = snakes[idx].isPlayer ? 120 : 255;
        drawText(buf, lx, ly + 28 + k * 22, r, g, b, false);
    }
}

void Game::drawHUD() {
    char buf[64];
    snprintf(buf, sizeof(buf), "YOUR LENGTH: %d", snakes[0].score);
    drawText(buf, 20, screen->h - 70, 255, 255, 255, true);
    int rank = 1;
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (snakes[i].score > snakes[0].score) rank++;
    }
    snprintf(buf, sizeof(buf), "YOUR RANK: %d OF %d", rank, MAX_SNAKES);
    drawText(buf, 20, screen->h - 40, 255, 220, 0, false);
    drawText("BOOST: R1 / X", 20, 20, 255, 255, 255, false);
}

void Game::renderGameOver() {
    drawBackground();
    drawBorder();
    drawFood();
    for (int i = 0; i < MAX_SNAKES; i++) {
        drawSnake(snakes[i]);
    }
    drawParticles();
    drawMinimap();
    drawLeaderboard();
    SDL_Rect box = {(Sint16)(screen->w / 2 - 280), (Sint16)(screen->h / 2 - 60), 560, 100};
    SDL_FillRect(screen, &box, getColor(20, 5, 5));
    drawText("VOCE MORREU!", screen->w / 2 - 140, screen->h / 2 - 40, 255, 50, 50, true);
    drawText("APERTE X PARA VOLTAR", screen->w / 2 - 180, screen->h / 2 + 10, 255, 255, 255, true);
}

void Game::render() {
    if (gameState == 2) {
        renderGameOver();
        SDL_Flip(screen);
        return;
    }
    drawBackground();
    drawBorder();
    drawFood();
    for (int i = 0; i < MAX_SNAKES; i++) {
        drawSnake(snakes[i]);
    }
    drawParticles();
    drawMinimap();
    drawLeaderboard();
    drawHUD();
    SDL_Flip(screen);
}