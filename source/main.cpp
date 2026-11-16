#include "game.h"
#include <stdio.h>
#include <io/pad.h>
#include <sysutil/sysutil.h>
static void sysutil_exit_callback(uint64_t status, uint64_t param, void* userdata) {
    (void)param; (void)userdata;
    if (status == SYSUTIL_EXIT_GAME) exit(0);
}
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);
    Game game;
    if (!game.init()) { printf("Erro ao iniciar o jogo!\n"); return 1; }
    game.run();
    game.shutdown();
    return 0;
}