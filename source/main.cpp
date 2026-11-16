#include <SDL/SDL.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>

static volatile bool g_exit = false;

static void sysutil_cb(uint64_t status, uint64_t param, void* userdata) {
    (void)param; (void)userdata;
    if (status == SYSUTIL_EXIT_GAME) g_exit = true;
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    sysUtilRegisterCallback(0, sysutil_cb, NULL);
    ioPadInit(7);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    SDL_Surface* screen = SDL_SetVideoMode(1280, 720, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    if (!screen) { SDL_Quit(); return 2; }

    Uint32 start = SDL_GetTicks();
    while (!g_exit && SDL_GetTicks() - start < 10000) {
        sysUtilCheckCallback();
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 0, 0));
        SDL_Flip(screen);
        SDL_Delay(16);
    }

    ioPadEnd();
    SDL_Quit();
    return 0;
}