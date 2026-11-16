#include "game.h"
#include <sysutil/sysutil.h>
#include <sys/process.h>
#include <cstdio>

static void sysutilExitCallback(u64 status, u64 param, void* userdata) {
    (void)param;
    (void)userdata;
    if (status == SYSUTIL_EXIT_GAME) {
        sysProcessExit(0);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    sysUtilRegisterCallback(0, sysutilExitCallback, NULL);
    Game game;
    if (!game.init()) {
        game.shutdown();
        sysProcessExit(0);
        return 0;
    }
    game.run();
    game.shutdown();
    sysProcessExit(0);
    return 0;
}