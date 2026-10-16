/* Dumps the background map + sprite list for one frame so tools/preview.py can draw it. Usage: dump title|play */
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "scene.h"
#ifdef PLAT_GB
#define COLS 20
#define ROWS 18
#else
#define COLS 32
#define ROWS 24
#endif
static int n; static int sx[64], sy[64], st[64];
static void add(int16_t x, int16_t y, uint8_t t) { if (n < 64) { sx[n] = x; sy[n] = y; st[n] = t; n++; } }
int main(int argc, char **argv) {
    static uint8_t map[COLS * ROWS]; char hud[COLS + 1]; int i, title = argc > 1 && !strcmp(argv[1], "title");
    game_init(); G.hi = 4200;
    if (!title) {
        game_step(IN_F); game_step(0);
        G.lv = 3; G.score = 2400; G.rk_n = 2; G.rk_fuel = 0; G.at = 60000;
        G.it[0].on = 0; G.it[0].type = 0; G.it[0].on = 1; G.it[0].seg = 2;
        G.it[0].x = (PLAT[0].x + 8) * 16; G.it[0].y = (PLAT[0].y - 16) * 16; G.it[0].vy = 0;
        const int ax[4] = { 30, 70, 110, 140 }, ay[4] = { 28, 70, GY - 40, 44 };
        for (i = 0; i < 4; i++) { G.al[i].on = 1; G.al[i].k = i; G.al[i].x = ax[i] * 16 * (SCR_W / 160 ? 1 : 1) ; G.al[i].y = ay[i] * 16; G.al[i].t = 0; }
        G.p.x = (RX + 24) * 16; G.p.y = (GY - 70) * 16; G.p.dir = -1; G.p.inv = 0; G.carry = -1;
        for (i = 0; i < 3; i++) game_step(IN_F | IN_U | IN_L);
        G.p.inv = 0;
    }
    scene_map(map, COLS, ROWS, title);
    if (title) scene_text(map, COLS, 0, ROWS - 1, "HI SCORE 004200"), scene_text(map, COLS, COLS == 20 ? 5 : 10, 9, COLS == 20 ? "PRESS START" : "PRESS BUTTON 1");
    else { scene_hud(hud, COLS); scene_text(map, COLS, 0, 0, hud); }
    scene_sprites(add);
    printf("%d %d\n", COLS, ROWS);
    for (i = 0; i < COLS * ROWS; i++) printf("%d ", map[i]);
    printf("\n%d\n", n);
    for (i = 0; i < n; i++) printf("%d %d %d\n", sx[i], sy[i], st[i]);
    return 0;
}
