// game.h
#pragma once
#include <vector>
using namespace std;

struct Pos { int x, y; };
struct Snake {
    vector<Pos> corpo;
    int dir;
    int speed;
    bool vivo;
    bool isPlayer;
    int pontos;
    int cor;
};
struct Food { int x, y; };

extern vector<Snake> cobras;
extern vector<Food> comidas;
extern Snake player;
extern int sua_posicao;

void init_game();
void update_game();
void update_bot(Snake &bot);
Food comida_mais_perta(Pos p);
bool vai_bater_parede(Snake &s);
bool vai_bater_corpo(Snake &s);
void matar_bot(int idx);
void spawn_comida(int x, int y);
int calcular_rank();