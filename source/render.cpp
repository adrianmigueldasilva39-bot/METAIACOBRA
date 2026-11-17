// render.cpp
#include "game.h"
#include <stdio.h>

extern void draw_rect(int x, int y, int w, int h, int cor);
extern void draw_text(int x, int y, const char* texto);

void render_game() {
    draw_rect(0, 0, 1280, 720, 0xFF000000);
    for (auto &f : comidas) {
        draw_rect(f.x * 10, f.y * 10, 8, 8, 0xFFFF0000);
    }
    for (auto &p : player.corpo) {
        draw_rect(p.x * 10, p.y * 10, 9, 9, 0xFF00FF00);
    }
    for (auto &b : cobras) {
        if (!b.vivo) continue;
        for (auto &p : b.corpo) {
            draw_rect(p.x * 10, p.y * 10, 9, 9, 0xFFFFAA00);
        }
    }
    char buf[64];
    sprintf(buf, "SUA POSICAO: #%d", sua_posicao);
    draw_text(20, 20, buf);
    sprintf(buf, "PONTOS: %d", player.pontos);
    draw_text(20, 50, buf);
    sprintf(buf, "COBRAS VIVAS: %d", (int)cobras.size());
    draw_text(20, 80, buf);
}