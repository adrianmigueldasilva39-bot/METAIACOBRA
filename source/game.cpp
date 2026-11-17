// game.cpp
#include "game.h"
#include <stdlib.h>
#include <math.h>

vector<Snake> cobras;
vector<Food> comidas;
Snake player;
int sua_posicao = 1;
int game_tick = 0;

#define LARGURA 60
#define ALTURA 40

void spawn_comida(int x, int y) {
    Food f; f.x = x; f.y = y;
    comidas.push_back(f);
}

Food comida_mais_perta(Pos p) {
    Food melhor = comidas[0];
    int menor = 999999;
    for (auto &f : comidas) {
        int d = abs(f.x - p.x) + abs(f.y - p.y);
        if (d < menor) { menor = d; melhor = f; }
    }
    return melhor;
}

bool vai_bater_parede(Snake &s) {
    Pos prox = s.corpo[0];
    if (s.dir == 0) prox.y--;
    if (s.dir == 1) prox.y++;
    if (s.dir == 2) prox.x--;
    if (s.dir == 3) prox.x++;
    if (prox.x < 0 || prox.x >= LARGURA || prox.y < 0 || prox.y >= ALTURA) return true;
    return false;
}

bool vai_bater_corpo(Snake &s) {
    Pos prox = s.corpo[0];
    if (s.dir == 0) prox.y--;
    if (s.dir == 1) prox.y++;
    if (s.dir == 2) prox.x--;
    if (s.dir == 3) prox.x++;
    for (int i = 1; i < (int)s.corpo.size(); i++) {
        if (prox.x == s.corpo[i].x && prox.y == s.corpo[i].y) return true;
    }
    return false;
}

int virar_90(Snake &s) {
    if (s.dir == 0 || s.dir == 1) return (rand() % 2 == 0) ? 2 : 3;
    else return (rand() % 2 == 0) ? 0 : 1;
}

void update_bot(Snake &bot) {
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

void matar_bot(int idx) {
    for (auto &p : cobras[idx].corpo) {
        spawn_comida(p.x, p.y);
    }
    cobras[idx].vivo = false;
    cobras[idx].corpo.clear();
}

int calcular_rank() {
    int rank = 1;
    for (auto &b : cobras) {
        if (b.isPlayer) continue;
        if (b.vivo && b.pontos > player.pontos) rank++;
    }
    return rank;
}

void mover_cobra(Snake &s) {
    Pos nova = s.corpo[0];
    if (s.dir == 0) nova.y--;
    if (s.dir == 1) nova.y++;
    if (s.dir == 2) nova.x--;
    if (s.dir == 3) nova.x++;
    s.corpo.insert(s.corpo.begin(), nova);
    bool comeu = false;
    for (int i = 0; i < (int)comidas.size(); i++) {
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

void update_game() {
    game_tick++;
    if (game_tick % 6 != 0) return;
    for (auto &b : cobras) {
        if (!b.isPlayer && b.vivo) {
            update_bot(b);
        }
    }
    for (auto &b : cobras) {
        if (b.vivo) mover_cobra(b);
    }
    if (player.vivo) mover_cobra(player);
    for (int i = 0; i < (int)cobras.size(); i++) {
        if (!cobras[i].vivo) continue;
        for (int j = 0; j < (int)cobras.size(); j++) {
            if (i == j || !cobras[j].vivo) continue;
            Pos cabeca = cobras[i].corpo[0];
            for (int k = 1; k < (int)cobras[j].corpo.size(); k++) {
                if (cabeca.x == cobras[j].corpo[k].x && cabeca.y == cobras[j].corpo[k].y) {
                    matar_bot(i);
                    break;
                }
            }
        }
    }
    for (int i = 0; i < (int)cobras.size(); i++) {
        if (!cobras[i].vivo || cobras[i].isPlayer) continue;
        Pos cabeca = cobras[i].corpo[0];
        for (int k = 1; k < (int)player.corpo.size(); k++) {
            if (cabeca.x == player.corpo[k].x && cabeca.y == player.corpo[k].y) {
                matar_bot(i);
                player.pontos += 50;
                break;
            }
        }
    }
    if (player.vivo) {
        Pos cabeca = player.corpo[0];
        for (auto &b : cobras) {
            if (b.isPlayer || !b.vivo) continue;
            for (auto &p : b.corpo) {
                if (cabeca.x == p.x && cabeca.y == p.y) player.vivo = false;
            }
        }
    }
    sua_posicao = calcular_rank();
}

void init_game() {
    cobras.clear();
    comidas.clear();
    player.corpo.clear();
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