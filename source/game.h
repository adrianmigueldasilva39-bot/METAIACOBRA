// game.h
#pragma once
#include <vector>
#include <string>
using namespace std;

struct Pos { int x, y; };
struct Snake {
    vector<Pos> corpo;
    int dir;
    int speed;
    bool vivo;
    bool isPlayer;
    int pontos;
};
struct Food { int x, y; };

class Game {
public:
    bool init();
    void run();
private:
    vector<Snake> cobras;
    vector<Food> comidas;
    Snake player;
    int sua_posicao;
    int game_tick;
    void init_game();
    void update_game();
    void render_game();
    void update_bot(Snake &bot);
    Food comida_mais_perta(Pos p);
    bool vai_bater_parede(Snake &s);
    bool vai_bater_corpo(Snake &s);
    int virar_90(Snake &s);
    void matar_bot(int idx);
    void spawn_comida(int x, int y);
    int calcular_rank();
    void mover_cobra(Snake &s);
    void draw_rect(int x, int y, int w, int h, int cor);
    void draw_text(int x, int y, const char* texto);
};