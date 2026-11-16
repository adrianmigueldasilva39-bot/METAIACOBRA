#include "game.h"
#include <cstdio>

static Uint32* g_buf = NULL;
static int g_pitch = 0;
static int g_w = 0;
static int g_h = 0;

static inline void putPixel(int x, int y, Uint32 c) {
    if (x < 0 || y < 0 || x >= g_w || y >= g_h) return;
    g_buf[y * g_pitch + x] = c;
}

static inline void fillHLine(int x0, int x1, int y, Uint32 c) {
    if (y < 0 || y >= g_h) return;
    if (x0 > x1) {
        int t = x0;
        x0 = x1;
        x1 = t;
    }
    if (x1 < 0 || x0 >= g_w) return;
    if (x0 < 0) x0 = 0;
    if (x1 >= g_w) x1 = g_w - 1;
    Uint32* row = g_buf + y * g_pitch + x0;
    for (int x = x0; x <= x1; x++) {
        *row++ = c;
    }
}

static inline void fillRectFast(int x, int y, int w, int h, Uint32 c) {
    if (w <= 0 || h <= 0) return;
    for (int yy = y; yy < y + h; yy++) {
        fillHLine(x, x + w - 1, yy, c);
    }
}

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
    if (radius <= 0 || g_buf == NULL) return;
    Uint32 c = getColor(r, g, b);
    int r2 = radius * radius;
    int w = radius;
    for (int y = -radius; y <= radius; y++) {
        int y2 = y * y;
        while (w >= 0 && w * w + y2 > r2) w--;
        while ((w + 1) * (w + 1) + y2 <= r2) w++;
        fillHLine(cx - w, cx + w, cy + y, c);
    }
}

void Game::drawCircleOutline(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    if (radius <= 0 || g_buf == NULL) return;
    Uint32 c = getColor(r, g, b);
    int x = radius;
    int y = 0;
    int err = 1 - radius;
    while (x >= y) {
        putPixel(cx + x, cy + y, c);
        putPixel(cx - x, cy + y, c);
        putPixel(cx + x, cy - y, c);
        putPixel(cx - x, cy - y, c);
        putPixel(cx + y, cy + x, c);
        putPixel(cx - y, cy + x, c);
        putPixel(cx + y, cy - x, c);
        putPixel(cx - y, cy - x, c);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void Game::drawLine(int x0, int y0, int x1, int y1, Uint8 r, Uint8 g, Uint8 b) {
    if (g_buf == NULL) return;
    Uint32 c = getColor(r, g, b);
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int x = x0;
    int y = y0;
    while (true) {
        putPixel(x, y, c);
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
    drawFilledCircle(sx, sy, radius + 6, r / 5, g / 5, b / 5);
    drawFilledCircle(sx, sy, radius, r, g, b);
}

void Game::drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, bool big) {
    if (g_buf == NULL) return;
    int s = big ? 3 : 2;
    int cx = x;
    Uint32 c = getColor(r, g, b);
    for (int i = 0; text[i] != 0; i++) {
        unsigned char ch = (unsigned char)text[i];
        if (ch == ' ') {
            cx += 6 * s;
            continue;
        }
        if (ch < 32 || ch > 126) {
            cx += 6 * s;
            continue;
        }
        for (int row = 0; row < 7; row++) {
            Uint8 bits = FONT_5x7[ch][row];
            for (int col = 0; col < 5; col++) {
                if (bits & (0x10 >> col)) {
                    fillRectFast(cx + col * s, y + row * s, s, s, c);
                }
            }
        }
        cx += 6 * s;
    }
}

void Game::drawBackground() {
    Uint32 bg = getColor(12, 14, 22);
    for (int y = 0; y < g_h; y++) {
        Uint32* row = g_buf + y * g_pitch;
        for (int x = 0; x < g_w; x++) row[x] = bg;
    }
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
            Vec2 prev = worldToScreen(wx + hexR, wy);
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
        drawFilledCircle((int)sc.x, (int)sc.y, rad, 8, 8, 12);
        drawFilledCircle((int)sc.x, (int)sc.y, rad - 2, fr, fg, fb);
    }
    if (!s.body.empty()) {
        Vec2 h = worldToScreen(s.body[0].x, s.body[0].y);
        int rad = (int)(s.radius * zoom * 1.15f);
        Uint8 r = s.cols[0][0];
        Uint8 g = s.cols[0][1];
        Uint8 b = s.cols[0][2];
        drawFilledCircle((int)h.x, (int)h.y, rad, 8, 8, 12);
        drawFilledCircle((int)h.x, (int)h.y, rad - 2, r, g, b);
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
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive) continue;
        if (snakes[i].body.empty()) continue;
        int px = mx + (int)(snakes[i].body[0].x / WORLD_RADIUS * mr);
        int py = my + (int)(snakes[i].body[0].y / WORLD_RADIUS * mr);
        Uint32 c = snakes[i].isPlayer ? getColor(0, 255, 120) : getColor(255, 255, 255);
        fillRectFast(px - 3, py - 3, 6, 6, c);
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
    Uint32 boxColor = getColor(20, 5, 5);
    fillRectFast(screen->w / 2 - 280, screen->h / 2 - 60, 560, 100, boxColor);
    drawText("VOCE MORREU!", screen->w / 2 - 140, screen->h / 2 - 40, 255, 50, 50, true);
    drawText("APERTE X PARA VOLTAR", screen->w / 2 - 180, screen->h / 2 + 10, 255, 255, 255, true);
}

void Game::render() {
    if (SDL_MUSTLOCK(screen)) {
        SDL_LockSurface(screen);
    }
    g_buf = (Uint32*)screen->pixels;
    g_pitch = screen->pitch / 4;
    g_w = screen->w;
    g_h = screen->h;
    if (gameState == 2) {
        renderGameOver();
    } else {
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
    }
    if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
    }
    g_buf = NULL;
    SDL_Flip(screen);
}