// render.cpp
#include "game.h"
#include <stdio.h>

void Game::draw_rect(int x, int y, int w, int h, int cor) {
    (void)x; (void)y; (void)w; (void)h; (void)cor;
}

void Game::draw_text(int x, int y, const char* texto) {
    (void)x; (void)y; (void)texto;
}

void Game::render_game() {
    draw_rect(0, 0, 1280, 720, 0xFF000000);
    for (size_t i = 0; i < comidas.size(); i++) {
        draw_rect(comidas[i].x * 10, comidas[i].y * 10, 8, 8, 0xFFFF0000);
    }
    for (size_t i = 0; i < player.corpo.size(); i++) {
        draw_rect(player.corpo[i].x * 10, player.corpo[i].y * 10, 9, 9, 0xFF00FF00);
    }
    for (size_t i = 0; i < cobras.size(); i++) {
        if (!cobras[i].vivo) continue;
        for (size_t j = 0; j < cobras[i].corpo.size(); j++) {
            draw_rect(cobras[i].corpo[j].x * 10, cobras[i].corpo[j].y * 10, 9, 9, 0xFFFFAA00);
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