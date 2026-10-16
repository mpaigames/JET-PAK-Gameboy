/* Headless tests of the game core: a bot plays with enemies off (mechanics check),
   then random-input fuzzing with enemies on (invariants + sanitizers). */
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#define PXL(v) ((v) >> 4)

static uint8_t bot(void) {
    uint8_t in = 0; int tx, ty, px = PXL(G.p.x) + 4, py = PXL(G.p.y) + 8, i;
    if (G.state != ST_PLAY) return IN_F;
    if (G.carry >= 0) { tx = RX; ty = py; }
    else if (G.rk_n == 3 && G.rk_fuel >= 6) { tx = RX; ty = GY - 24; }
    else { tx = -1; ty = 0;
        for (i = 0; i < MAXIT; i++) if (G.it[i].on) { int w = G.it[i].type ? 8 : 16; tx = PXL(G.it[i].x) + w / 2; ty = PXL(G.it[i].y) + w / 2; break; }
        if (tx < 0) { tx = RX; ty = GY - 90; } }
    /* waypoint: if a platform sits between us (below it) and a target above it, go round the edge first */
    for (i = 0; i < NPLAT; i++) {
        const Plat *q = &PLAT[i];
        if (tx > q->x - 6 && tx < q->x + q->w + 6 && ty < q->y && py > q->y - 4) {
            tx = (px < q->x + q->w / 2) ? q->x - 10 : q->x + q->w + 10; ty = q->y - 30; break;
        }
    }
    for (i = 0; i < NPLAT; i++) {          /* target below a platform we are above: go round the edge */
        const Plat *q = &PLAT[i];
        if (px > q->x - 6 && px < q->x + q->w + 6 && tx > q->x - 6 && tx < q->x + q->w + 6 && py < q->y + 4 && ty > q->y + 8) {
            tx = (px < q->x + q->w / 2) ? q->x - 12 : q->x + q->w + 12; ty = py; break;
        }
    }
    px += PXL(G.p.vx * 10);                 /* look-ahead so the bot brakes instead of orbiting */
    if (px < tx - 3) in |= IN_R; else if (px > tx + 3) in |= IN_L;
    /* keep above any platform we need to clear when far away horizontally */
    if (py > ty + 2 || (abs(px - tx) > 12 && py > ty - 30)) in |= IN_U;
    return in;
}

int main(void) {
    long f; int maxlv = 0; long t_lv2 = -1, t_lv3 = -1;
    game_init(); game_step(IN_F); game_step(0);
    if (G.state != ST_PLAY) { printf("FAIL: game did not start\n"); return 1; }
    for (f = 0; f < 60000 && G.lv < 4; f++) {
        G.at = 65000;                       /* no enemies */
        game_step(bot());
        if (getenv("DBG") && f > 3000 && f % 400 == 0) { printf("f=%ld p=(%d,%d) v=(%d,%d) gr=%d carry=%d state=%d n=%d fuel=%d items:", f, PXL(G.p.x), PXL(G.p.y), G.p.vx, G.p.vy, G.p.gr, G.carry, G.state, G.rk_n, G.rk_fuel); for (int k=0;k<MAXIT;k++) if (G.it[k].on) printf(" [t%d (%d,%d)]", G.it[k].type, PXL(G.it[k].x), PXL(G.it[k].y)); printf("\n"); }
        if (G.lv > maxlv) { maxlv = G.lv; if (G.lv == 2) t_lv2 = f; if (G.lv == 3) t_lv3 = f; }
    }
    printf("bot (no enemies): reached level %d, L2 at frame %ld, L3 at frame %ld, score %lu\n", G.lv, t_lv2, t_lv3, (unsigned long)G.score);
    if (G.lv < 3) { printf("FAIL: bot could not complete levels (state=%d rk_n=%d fuel=%d carry=%d)\n", G.state, G.rk_n, G.rk_fuel, G.carry); return 1; }

    /* fuzz with enemies */
    srand(12345); game_init(); game_step(IN_F); game_step(0);
    uint8_t in = 0; long deaths = 0, kills = 0; uint8_t prevstate = G.state;
    for (f = 0; f < 400000; f++) {
        if (f % 7 == 0) in = (uint8_t)(rand() & 31) & ~IN_S;
        if (G.state == ST_OVER || G.state == ST_TITLE) in = (f & 1) ? IN_F : 0;
        game_step(in);
        if (G.ev & EV_KILL) kills++;
        if (G.state == ST_DEAD && prevstate != ST_DEAD) deaths++;
        prevstate = G.state;
        if (G.state == ST_PLAY || G.state == ST_DEAD) {
            if (G.p.x < -8 * 16 - 1 || G.p.x > SCR_W * 16 + 1 || G.p.y < TOP * 16 || G.p.y > (GY - 16) * 16) { printf("FAIL: player out of bounds at %ld (%d,%d)\n", f, G.p.x, G.p.y); return 1; }
            for (int i = 0; i < MAXAL; i++) if (G.al[i].on && (PXL(G.al[i].y) < TOP - 1 || PXL(G.al[i].y) > GY)) { printf("FAIL: alien y out of range %d\n", PXL(G.al[i].y)); return 1; }
            if (G.rk_n < 1 || G.rk_n > 3 || G.rk_fuel > 6) { printf("FAIL: rocket state corrupt\n"); return 1; }
        }
    }
    printf("fuzz: 400000 frames OK, kills=%ld deaths=%ld, final lv=%d hi=%lu\n", kills, deaths, G.lv, (unsigned long)G.hi);
    printf("ALL TESTS PASSED\n");
    return 0;
}
