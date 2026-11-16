#include <SDL/SDL.h>
#include <stdio.h>
#include "game.h"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYPAD) < 0) {
        printf("SDL_Init falhou: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Surface *screen = SDL_SetVideoMode(1280, 720, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        printf("SetVideoMode falhou: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_JoystickOpen(0);
    Game game;
    game.run(screen);
    SDL_Quit();
    return 0;
}