bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    ioPadInit(7);
    // HWSURFACE é o certo no PS3, sem SWSURFACE
    screen = SDL_SetVideoMode(1280, 720, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        screen = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    }
    if (!screen) return false;
    SDL_ShowCursor(SDL_DISABLE);
    srand(12345);
    initWorld();
    running = true;
    return true;
}

void Game::shutdown() {
    ioPadEnd();
    SDL_Quit();
    screen = NULL;
}