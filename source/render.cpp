#include "game.h"
#include <cstdio>

// Fonte para o HUD
static const Uint8 FONT[128][7] = {
    {0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},{0x04,0x04,0x04,0x04,0x04,0x00,0x04},{0x0A,0x0A,0x0A,0x00,0x00,0x00,0x00},{0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x0A},{0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},{0x18,0x19,0x02,0x04,0x08,0x13,0x18},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0x08,0x04,0x02,0x02,0x02,0x04,0x08},{0x02,0x04,0x08,0x08,0x08,0x04,0x02},{0x00,0x04,0x15,0x0E,0x15,0x04,0x00},{0x00,0x04,0x04,0x1F,0x04,0x04,0x00},{0,0,0,0,0x0C,0x04,0x08},{0,0,0,0x1F,0,0,0},{0,0,0,0,0,0x0C,0x0C},{0x01,0x02,0x02,0x04,0x08,0x10,0x10},
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},{0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},{0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},{0x1F,0x02,0x04,0x02,0x01,0x11,0x0E},{0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},{0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},{0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},{0x1F,0x01,0x02,0x04,0x08,0x08,0x08},{0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},{0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},{0,0x0C,0x0C,0,0x0C,0x0C,0},{0,0x0C,0x0C,0,0x0C,0x04,0x08},{0x02,0x04,0x08,0x10,0x08,0x04,0x02},{0,0,0x1F,0,0x1F,0,0},{0x08,0x04,0x02,0x01,0x02,0x04,0x08},{0x0E,0x11,0x01,0x02,0x04,0,0x04},
    {0x0E,0x11,0x01,0x0D,0x15,0x15,0x0E},{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},{0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},{0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},{0x1C,0x12,0x11,0x11,0x11,0x12,0x1C},{0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},{0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},{0x0E,0x11,0x10,0x17,0x11,0x11,0x0F},{0x11,0x11,0x11,0x1F,0x11,0x11,0x11},{0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},{0x01,0x01,0x01,0x01,0x11,0x11,0x0E},{0x11,0x12,0x14,0x18,0x14,0x12,0x11},{0x10,0x10,0x10,0x10,0x10,0x10,0x1F},{0x11,0x1B,0x15,0x15,0x11,0x11,0x11},{0x11,0x11,0x1B,0x15,0x11,0x11,0x11},{0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},{0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},{0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},{0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},{0x1F,0x04,0x04,0x04,0x04,0x04,0x04},{0x11,0x11,0x11,0x11,0x11,0x11,0x0E},{0x11,0x11,0x11,0x11,0x11,0x0A,0x04},{0x11,0x11,0x11,0x15,0x15,0x15,0x0A},{0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},{0x11,0x11,0x0A,0x04,0x04,0x04,0x04},{0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},{0x0E,0x08,0x08,0x08,0x08,0x08,0x0E},{0x10,0x08,0x04,0x02,0x01,0x01,0x01},{0x0E,0x02,0x02,0x02,0x02,0x02,0x0E},{0x04,0x0A,0x11,0,0,0,0},{0,0,0,0,0,0,0x1F},
    {0x08,0x04,0x02,0,0,0,0},{0,0,0x0E,0x01,0x0F,0x11,0x0F},{0x10,0x10,0x1E,0x11,0x11,0x11,0x1E},{0,0,0x0E,0x11,0x10,0x11,0x0E},{0x01,0x01,0x0F,0x11,0x11,0x11,0x0F},{0,0,0x0E,0x11,0x1F,0x10,0x0E},{0x06,0x08,0x1E,0x08,0x08,0x08,0x08},{0,0x0F,0x11,0x11,0x0F,0x01,0x0E},{0x10,0x10,0x16,0x19,0x11,0x11,0x11},{0x04,0,0x0C,0x04,0x04,0x04,0x0E},{0x02,0,0x06,0x02,0x02,0x12,0x0C},{0x10,0x10,0x12,0x14,0x18,0x14,0x12},{0x0C,0x04,0x04,0x04,0x04,0x04,0x0E},{0,0,0x1A,0x15,0x15,0x11,0x11},{0,0,0x1E,0x11,0x11,0x11,0x11},{0,0,0x0E,0x11,0x11,0x11,0x0E},
    {0,0,0x1E,0x11,0x11,0x1E,0x10},{0,0,0x0F,0x11,0x11,0x0F,0x01},{0,0,0x16,0x19,0x10,0x10,0x10},{0,0,0x0F,0x10,0x0E,0x01,0x1E},{0x08,0x08,0x1E,0x08,0x08,0x09,0x06},{0,0,0x11,0x11,0x11,0x13,0x0D},{0,0,0x11,0x11,0x11,0x0A,0x04},{0,0,0x11,0x15,0x15,0x15,0x0A},{0,0,0x11,0x0A,0x04,0x0A,0x11},{0,0,0x11,0x11,0x0F,0x01,0x0E},{0,0,0x1F,0x02,0x04,0x08,0x1F},{0x06,0x08,0x08,0x0E,0x08,0x08,0x06},{0x04,0x04,0x04,0x04,0x04,0x04,0x04},{0x0C,0x02,0x02,0x0E,0x02,0x02,0x0C},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}
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

// Efeito de brilho (Glow) para a comida e cobras
void Game::drawGlowCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    drawFilledCircle(cx, cy, radius + 5, r / 4, g / 4, b / 4);
    drawFilledCircle(cx, cy, radius + 2, r / 2, g / 2, b / 2);
    drawFilledCircle(cx, cy, radius, r, g, b);
}

void Game::drawText(const char* text, int x, int y, Uint8 r, Uint8 g, Uint8 b, int scale) {
    int cx = x;
    Uint32 c = getColor(r, g, b);
    for (int i = 0; text[i] != 0; i++) {
        unsigned char ch = (unsigned char)text[i];
        if (ch == ' ') { cx += 6 * scale; continue; }
        if (ch < 32 || ch > 126) { cx += 6 * scale; continue; }
        for (int row = 0; row < 7; row++) {
            Uint8 bits = FONT[ch][row];
            for (int col = 0; col < 5; col++) {
                if (bits & (0x10 >> col)) {
                    SDL_Rect rc = {(Sint16)(cx + col * scale), (Sint16)(y + row * scale), (Uint16)scale, (Uint16)scale};
                    SDL_FillRect(screen, &rc, c);
                }
            }
        }
        cx += 6 * scale;
    }
}

// Fundo escuro com padrão hexagonal (igual Slither.io)
void Game::drawBackground() {
    // Cor de fundo bem escura
    SDL_Rect bg = {0, 0, (Uint16)screen->w, (Uint16)screen->h};
    SDL_FillRect(screen, &bg, getColor(15, 15, 20));

    float hexR = 45.0f * zoom;
    float stepX = hexR * 1.5f;
    float stepY = hexR * 1.732f;
    
    int offX = (int)(-cameraX * zoom) % (int)stepX;
    int offY = (int)(-cameraY * zoom) % (int)stepY;
    if (offX > 0) offX -= (int)stepX;
    if (offY > 0) offY -= (int)stepY;

    Uint32 gridColor = getColor(30, 30, 40);

    for (int y = offY - (int)stepY; y < screen->h + stepY; y += stepY) {
        for (int x = offX - (int)stepX; x < screen->w + stepX; x += stepX) {
            int cx = x + ((y / (int)stepY) % 2 ? (int)(stepX/2) : 0);
            int cy = y;
            
            // Desenha um hexágono vazado usando pontos
            for (int k = 0; k < 6; k++) {
                float a1 = k * 1.047f;
                float a2 = (k + 1) * 1.047f;
                int x1 = cx + (int)(cosf(a1) * hexR);
                int y1 = cy + (int)(sinf(a1) * hexR);
                int x2 = cx + (int)(cosf(a2) * hexR);
                int y2 = cy + (int)(sinf(a2) * hexR);
                
                // Pinta uma linha reta (Bresenham simples para o grid)
                int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
                int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
                int err = dx + dy;
                while (1) {
                    SDL_Rect dot = {(Sint16)x1, (Sint16)y1, 2, 2};
                    SDL_FillRect(screen, &dot, gridColor);
                    if (x1 == x2 && y1 == y2) break;
                    int e2 = 2 * err;
                    if (e2 >= dy) { err += dy; x1 += sx; }
                    if (e2 <= dx) { err += dx; y1 += sy; }
                }
            }
        }
    }
}

void Game::drawBorder() {
    Vec2 center = worldToScreen(0, 0);
    int rad = (int)(WORLD_RADIUS * zoom);
    // Borda vermelha do limite do mapa
    for (int a = 0; a < 360; a += 1) {
        float ar = a * 3.14159f / 180.0f;
        int px = center.x + (int)(cosf(ar) * rad);
        int py = center.y + (int)(sinf(ar) * rad);
        SDL_Rect rc = {(Sint16)px, (Sint16)py, 4, 4};
        SDL_FillRect(screen, &rc, getColor(255, 0, 0));
    }
}

void Game::drawFood() {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].active) continue;
        Vec2 s = worldToScreen(foods[i].pos.x, foods[i].pos.y);
        int rad = (int)(foods[i].radius * zoom * (1.0f + 0.2f * sinf(foods[i].pulse)));
        if (rad < 2) rad = 2;
        drawGlowCircle((int)s.x, (int)s.y, rad, foods[i].r, foods[i].g, foods[i].b);
    }
}

// Desenha a cobra igual Slither.io (contínua com brilho e olhos)
void Game::drawSnake(Snake& s) {
    if (!s.alive || s.body.empty()) return;
    
    // Desenha o corpo (de trás pra frente para a cabeça ficar por cima)
    for (int i = (int)s.body.size() - 1; i >= 0; i--) {
        Vec2 sc = worldToScreen(s.body[i].x, s.body[i].y);
        
        // Raio diminui um pouco perto do rabo
        float sizeFactor = (i < 3) ? 1.0f : (1.0f - (float)i / (s.body.size() * 2.5f));
        int rad = (int)(s.radius * zoom * sizeFactor);
        if (rad < 2) rad = 2;

        // Brilho externo (Efeito Neon)
        drawFilledCircle((int)sc.x, (int)sc.y, rad + 4, s.r / 5, s.g / 5, s.b / 5);
        drawFilledCircle((int)sc.x, (int)sc.y, rad + 1, s.r / 2, s.g / 2, s.b / 2);
        
        // Corpo principal
        drawFilledCircle((int)sc.x, (int)sc.y, rad, s.r, s.g, s.b);
        
        // Detalhe de luz (Highlight) para dar volume 3D
        drawFilledCircle((int)sc.x - rad/3, (int)sc.y - rad/3, rad/3, 255, 255, 255);
    }

    // Desenha a Cabeça e os Olhos
    Vec2 h = worldToScreen(s.body.front().x, s.body.front().y);
    int headRad = (int)(s.radius * zoom * 1.1f);

    // Olhos (Bolinhas brancas com pupilas pretas)
    float px = -sinf(s.angle);
    float py = cosf(s.angle);
    float eyeDist = headRad * 0.6f;
    float eyeSpread = headRad * 0.45f;
    
    float ex1 = h.x + cosf(s.angle) * eyeDist + px * eyeSpread;
    float ey1 = h.y + sinf(s.angle) * eyeDist + py * eyeSpread;
    float ex2 = h.x + cosf(s.angle) * eyeDist - px * eyeSpread;
    float ey2 = h.y + sinf(s.angle) * eyeDist - py * eyeSpread;

    int eyeRad = headRad / 2;
    if (eyeRad < 2) eyeRad = 2;

    // Desenha olhos
    drawFilledCircle((int)ex1, (int)ey1, eyeRad, 255, 255, 255);
    drawFilledCircle((int)ex2, (int)ey2, eyeRad, 255, 255, 255);
    // Pupilas
    drawFilledCircle((int)ex1 + 1, (int)ey1 + 1, eyeRad/2, 0, 0, 0);
    drawFilledCircle((int)ex2 + 1, (int)ey2 + 1, eyeRad/2, 0, 0, 0);
}

void Game::drawMinimap() {
    int mSize = 120;
    int mx = screen->w - mSize - 20;
    int my = 20;
    
    SDL_Rect bg = {(Sint16)mx, (Sint16)my, (Uint16)mSize, (Uint16)mSize};
    SDL_FillRect(screen, &bg, getColor(20, 20, 30));
    
    for (int i = 0; i < MAX_SNAKES; i++) {
        if (!snakes[i].alive || snakes[i].body.empty()) continue;
        Vec2 head = snakes[i].body.front();
        
        int px = mx + mSize/2 + (int)(head.x / WORLD_RADIUS * (mSize/2));
        int py = my + mSize/2 + (int)(head.y / WORLD_RADIUS * (mSize/2));
        
        SDL_Rect dot = {(Sint16)(px-2), (Sint16)(py-2), 4, 4};
        SDL_FillRect(screen, &dot, getColor(snakes[i].r, snakes[i].g, snakes[i].b));
    }
}

void Game::drawHUD() {
    char buf[64];
    snprintf(buf, sizeof(buf), "TAMANHO: %d", snakes[0].length);
    drawText(buf, 20, 20, 255, 255, 255, 3);
    
    snprintf(buf, sizeof(buf), "PONTOS: %d", snakes[0].score);
    drawText(buf, 20, 50, 255, 255, 255, 2);
    
    if (!snakes[0].alive) {
        SDL_Rect box = {(Sint16)(screen->w/2 - 200), (Sint16)(screen->h/2 - 60), 400, 120};
        SDL_FillRect(screen, &box, getColor(10, 10, 15));
        drawText("VOCE MORREU!", screen->w/2 - 110, screen->h/2 - 30, 255, 50, 50, 4);
        drawText("APERTE X PARA VOLTAR", screen->w/2 - 120, screen->h/2 + 20, 255, 255, 255, 2);
    }
}

void Game::render() {
    drawBackground();
    drawBorder();
    drawFood();
    
    // Desenha bots primeiro, jogador por cima
    for (int i = 1; i < MAX_SNAKES; i++) drawSnake(snakes[i]);
    drawSnake(snakes[0]);
    
    drawMinimap();
    drawHUD();
    
    SDL_Flip(screen);
}