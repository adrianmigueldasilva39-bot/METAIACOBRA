#include "game.h"
#include "render.h"
#include <SDL/SDL.h>

void Game::run(SDL_Surface *screen) {
    int running = 1;
    int x = 640, y = 360;
    int vx = 4, vy = 0;
    SDL_Event ev;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = 0;
            if (ev.type == SDL_JOYBUTTONDOWN) {
                if (ev.jbutton.button == 10) running = 0;
                if (ev.jbutton.button == 4) { vx = 0; vy = -4; }
                if (ev.jbutton.button == 6) { vx = 0; vy = 4; }
                if (ev.jbutton.button == 7) { vx = -4; vy = 0; }
                if (ev.jbutton.button == 5) { vx = 4; vy = 0; }
            }
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) running = 0;
                if (ev.key.keysym.sym == SDLK_LEFT) { vx = -4; vy = 0; }
                if (ev.key.keysym.sym == SDLK_RIGHT) { vx = 4; vy = 0; }
                if (ev.key.keysym.sym == SDLK_UP) { vx = 0; vy = -4; }
                if (ev.key.keysym.sym == SDLK_DOWN) { vx = 0; vy = 4; }
            }
        }
        x += vx; y += vy;
        if (x < 0) x = 1280;
        if (x > 1280) x = 0;
        if (y < 0) y = 720;
        if (y > 720) y = 0;
        render_frame(screen, x, y);
        SDL_Flip(screen);
        SDL_Delay(16);
    }
}