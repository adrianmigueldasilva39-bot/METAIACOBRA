.#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <psl1ght/lv2.h>
#include <io/pad.h>
#include <tiny3d.h>
#include <sysutil/sysutil.h>

int main() {
    sysUtilRegisterCallback(0, NULL, NULL);

    tiny3d_Init(1024 * 1024);
    tiny3d_Project2D();

    padInfo padinfo;
    padData paddata;

    float x = 640, y = 360;
    float vx = 3, vy = 2;

    while (1) {
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            if (paddata.BTN_SELECT) break;
            if (paddata.BTN_LEFT) { vx = -4; vy = 0; }
            if (paddata.BTN_RIGHT) { vx = 4; vy = 0; }
            if (paddata.BTN_UP) { vx = 0; vy = -4; }
            if (paddata.BTN_DOWN) { vx = 0; vy = 4; }
        }

        x += vx;
        y += vy;
        if (x < 0) x = 1280;
        if (x > 1280) x = 0;
        if (y < 0) y = 720;
        if (y > 720) y = 0;

        tiny3d_Clear(0xFF0A141E, TINY3D_CLEAR_ALL);

        for (int i = 0; i < 10; i++) {
            float cx = x - i * 20;
            float cy = y + sin(i * 0.5f) * 20;
            tiny3d_SetPolygon(TINY3D_QUADS);
            tiny3d_VertexPos(cx - 15, cy - 15, 0);
            tiny3d_VertexColor(0xFF66FF66);
            tiny3d_VertexPos(cx + 15, cy - 15, 0);
            tiny3d_VertexPos(cx + 15, cy + 15, 0);
            tiny3d_VertexPos(cx - 15, cy + 15, 0);
            tiny3d_End();
        }

        tiny3d_Flip();
    }

    tiny3d_Close();
    return 0;
}