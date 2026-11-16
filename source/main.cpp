#include "game.h"
#include <stdio.h>
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <sys/process.h>
volatile bool g_exitRequested = false;
static void cb(uint64_t s,uint64_t p,void* u){(void)p;(void)u; if(s==SYSUTIL_EXIT_GAME) g_exitRequested=true;}
int main(int argc,char* argv[]){(void)argc;(void)argv; sysUtilRegisterCallback(0,cb,NULL); { Game g; if(!g.init()){printf("fail\n"); return 1;} g.run(); g.shutdown(); } sysProcessExit(0); return 0;}