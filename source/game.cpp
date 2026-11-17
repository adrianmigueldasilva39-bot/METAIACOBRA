// game.cpp
#include "game.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define LARGURA 60
#define ALTURA 40

bool Game::init() {
    init_game();
    return true;
}

void Game::spawn_comida(int x, int y) {
    Food f; f.x = x; f.y = y;
    comidas.push_back(f);
}

Food Game::comida_mais_perta(Pos p) {
    Food melhor = comidas[0];
    int menor = 999999;
    for (size_t i = 0; i < comidas.size(); i++) {
        int d = abs(comidas[i].x - p.x) + abs(comidas[i].y - p.y);
        if (d < menor) { menor = d; melhor = comidas[i]; }
    }
    return melhor;
}

bool Game::vai_bater_parede(Snake &s) {
    Pos prox = s.corpo[0];
    if (s.dir == 0) prox.y--;
    if (s.dir == 1) prox.y++;
    if (s.dir == 2) prox.x--;
    if (s.dir == 3) prox.x++;
    if (prox.x < 0 || prox.x >= LARGURA || prox.y < 0 || prox.y >= ALTURA) return true;
    return false;
}

bool Game::vai_bater_corpo(Snake &s) {
    Pos prox = s.corpo[0];
    if (s.dir == 0) prox.y--;
    if (s.dir == 1) prox.y++;
    if (s.dir == 2) prox.x--;
    if (s.dir == 3) prox.x++;
    for (size_t i = 1; i < s.corpo.size(); i++) {
        if (prox.x == s.corpo[i].x && prox.y == s.corpo[i].y) return true;
    }
    return false;
}

int Game::virar_90(Snake &s) {
    if (s.dir == 0 || s.dir == 1) return (rand() % 2 == 0) ? 2 : 3;
    else return (rand() % 2 == 0) ? 0 : 1;
}

void Game::update_bot(Snake &bot) {
    if (!bot.vivo || comidas.empty()) return;
    Food alvo = comida_mais_perta(bot.corpo[0]);
    int dx = alvo.x - bot.corpo[0].x;
    int dy = alvo.y - bot.corpo[0].y;
    if (abs(dx) > abs(dy)) {
        int nova = (dx > 0) ? 3 : 2;
        if (!((bot.dir == 3 && nova == 2) || (bot.dir == 2 && nova == 3)))
            bot.dir = nova;
    } else {
        int nova = (dy > 0) ? 1 : 0;
        if (!((bot.dir == 1 && nova == 0) || (bot.dir == 0 && nova == 1)))
            bot.dir = nova;
    }
    if (vai_bater_parede(bot) || vai_bater_corpo(bot)) {
        bot.dir = virar_90(bot);
    }
}

void Game::matar_bot(int idx) {
    for (size_t i = 0; i < cobras[idx].corpo.size(); i++) {
        spawn_comida(cobras[idx].corpo[i].x, cobras[idx].corpo[i].y);
    }
    cobras[idx].vivo = false;
    cobras[idx].corpo.clear();
}

int Game::calcular_rank() {
    int rank = 1;
    for (size_t i = 0; i < cobras.size(); i++) {
        if (cobras[i].isPlayer) continue;
        if (cobras[i].vivo && cobras[i].pontos > player.pontos) rank++;
    }
    return rank;
}

void Game::mover_cobra(Snake &s) {
    if (s.corpo.empty()) return;
    Pos nova = s.corpo[0];
    if (s.dir == 0) nova.y--;
    if (s.dir == 1) nova.y++;
    if (s.dir == 2) nova.x--;
    if (s.dir == 3) nova.x++;
    s.corpo.insert(s.corpo.begin(), nova);
    bool comeu = false;
    for (size_t i = 0; i < comidas.size(); i++) {
        if (comidas[i].x == nova.x && comidas[i].y == nova.y) {
            comidas.erase(comidas.begin() + i);
            s.pontos += 10;
            spawn_comida(rand() % LARGURA, rand() % ALTURA);
            comeu = true;
            break;
        }
    }
    if (!comeu) s.corpo.pop_back();
}

void Game::update_game() {
    game_tick++;
    if (game_tick % 6 != 0) return;
    for (size_t i = 0; i < cobras.size(); i++) {
        if (!cobras[i].isPlayer && cobras[i].vivo) update_bot(cobras[i]);
    }
    for (size_t i = 0; i < cobras.size(); i++) {
        if (cobras[i].vivo) mover_cobra(cobras[i]);
    }
    if (player.vivo) mover_cobra(player);
    for (size_t i = 0; i < cobras.size(); i++) {
        if (!cobras[i].vivo) continue;
        for (size_t j = 0; j < cobras.size(); j++) {
            if (i == j || !cobras[j].vivo) continue;
            if (cobras[i].corpo.empty() || cobras[j].corpo.empty()) continue;
            Pos cabeca = cobras[i].corpo[0];
            for (size_t k = 1; k < cobras[j].corpo.size(); k++) {
                if (cabeca.x == cobras[j].corpo[k].x && cabeca.y == cobras[j].corpo[k].y) {
                    matar_bot(i);
                    break;
                }
            }
            if (!cobras[i].vivo) break;
        }
    }
    for (size_t i = 0; i < cobras.size(); i++) {
        if (!cobras[i].vivo || cobras[i].isPlayer) continue;
        if (cobras[i].corpo.empty()) continue;
        Pos cabeca = cobras[i].corpo[0];
        for (size_t k = 1; k < player.corpo.size(); k++) {
            if (cabeca.x == player.corpo[k].x && cabeca.y == player.corpo[k].y) {
                matar_bot(i);
                player.pontos += 50;
                break;
            }
        }
    }
    if (player.vivo && !player.corpo.empty()) {
        Pos cabeca = player.corpo[0];
        for (size_t i = 0; i < cobras.size(); i++) {
            if (cobras[i].isPlayer || !cobras[i].vivo) continue;
            for (size_t k = 0; k < cobras[i].corpo.size(); k++) {
                if (cabeca.x == cobras[i].corpo[k].x && cabeca.y == cobras[i].corpo[k].y) player.vivo = false;
            }
        }
    }
    sua_posicao = calcular_rank();
}

void Game::init_game() {
    cobras.clear();
    comidas.clear();
    player.corpo.clear();
    sua_posicao = 1;
    game_tick = 0;
    player.corpo.push_back({30, 20});
    player.corpo.push_back({29, 20});
    player.corpo.push_back({28, 20});
    player.dir = 3;
    player.speed = 6;
    player.vivo = true;
    player.isPlayer = true;
    player.pontos = 0;
    for (int i = 0; i < 8; i++) {
        Snake b;
        int sx = rand() % 50 + 5;
        int sy = rand() % 30 + 5;
        b.corpo.push_back({sx, sy});
        b.corpo.push_back({sx-1, sy});
        b.corpo.push_back({sx-2, sy});
        b.dir = rand() % 4;
        b.speed = 5;
        b.vivo = true;
        b.isPlayer = false;
        b.pontos = 0;
        cobras.push_back(b);
    }
    for (int i = 0; i < 30; i++) {
        spawn_comida(rand() % LARGURA, rand() % ALTURA);
    }
}

void Game::run() {
    while (1) {
        update_game();
        render_game();
    }
}