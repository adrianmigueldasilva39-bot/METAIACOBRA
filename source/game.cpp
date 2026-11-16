bool Game::init() {
    // 1. Inicia apenas o vídeo no SDL (sem SDL_INIT_JOYSTICK para não conflitar com ioPad)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    // 2. Inicia o controle nativo do PS3
    ioPadInit(7);

    // 3. Usa SDL_SWSURFACE (mais seguro e compatível com o RSX do PS3)
    // Dica: 1280x720 roda muito mais rápido no PS3 quando se desenha pixel por pixel (setPixel)
    screen = SDL_SetVideoMode(1280, 720, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    if (!screen) {
        screen = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    }
    if (!screen) {
        screen = SDL_SetVideoMode(0, 0, 32, SDL_SWSURFACE | SDL_FULLSCREEN);
        if (!screen) return false;
    }

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
