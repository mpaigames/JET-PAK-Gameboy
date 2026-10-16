#include "game.h"
#include "tileids.h"
#include "scene.h"

static void spr4(AddFn add, int16_t x, int16_t y, uint8_t base) {
    add(x, y, base); add(x + 8, y, base + 1); add(x, y + 8, base + 2); add(x + 8, y + 8, base + 3);
}

void scene_sprites(AddFn add) {
    uint8_t i, n, t; int16_t x, y; Pl *p = &G.p;
    n = (G.state == ST_TITLE) ? 3 : G.rk_n;
    for (i = 0; i < n; i++) {
        y = GY - 16 * (i + 1) + ((G.state == ST_TITLE) ? 0 : (G.rk_ly >> 4));
        if (y > -16) spr4(add, RX - 8, y, S_SEG + i * 4);
    }
    if (G.state == ST_TITLE) return;
    if (G.state == ST_LAUNCH && G.timer > 30) add(RX - 4, GY + (G.rk_ly >> 4), S_FLAME0 + ((G.tick >> 1) & 1));
    for (i = 0; i < MAXIT; i++) {
        It *o = &G.it[i];
        if (!o->on) continue;
        x = o->x >> 4; y = o->y >> 4;
        if (o->type) add(x, y, S_POD); else spr4(add, x, y, S_SEG + o->seg * 4);
    }
    for (i = 0; i < MAXAL; i++) {
        Al *a = &G.al[i];
        if (a->on) add(a->x >> 4, a->y >> 4, S_AL + a->k * 2 + ((G.tick >> 3) & 1));
    }
    if (G.state == ST_PLAY && !(p->inv && (G.tick & 4))) {
        x = p->x >> 4; y = p->y >> 4;
        t = p->dir > 0 ? S_PR_T : S_PL_T;
        add(x, y, t); add(x, y + 8, t + 1);
        if (p->th) add(x, y + 16, S_FLAME0 + (G.tick & 1));
    }
    if (G.beam.t) for (x = G.beam.x0; x < G.beam.x1; x += 8) add(x, G.beam.y - 4, S_BEAM);
    for (i = 0; i < MAXFX; i++) if (G.fx[i].t) add(G.fx[i].x, G.fx[i].y, S_EXP0 + (G.fx[i].t > 7 ? 0 : 1));
}

uint8_t font_tile(char c) {
    if (c >= '0' && c <= '9') return B_FONT + (c - '0');
    if (c >= 'A' && c <= 'Z') return B_FONT + 10 + (c - 'A');
    return B_BLANK;
}
void scene_text(uint8_t *m, uint8_t cols, uint8_t x, uint8_t y, const char *s) {
    while (*s) { m[y * cols + x] = font_tile(*s); x++; s++; }
}

void scene_map(uint8_t *m, uint8_t cols, uint8_t rows, uint8_t title) {
    uint16_t i, seed = 12345; uint8_t c, r, j, s;
    static const uint8_t letters[6] = { 0, 1, 2, 3, 4, 5 };   /* J E T P A K */
    static const uint8_t lcol[6] = { 0, 2, 4, 7, 9, 11 };
    for (i = 0; i < (uint16_t)cols * rows; i++) m[i] = B_BLANK;
    for (i = 0; i < (uint16_t)(cols * rows) / 36; i++) {
        seed = seed * 25173 + 13849;
        c = (seed >> 4) % cols; r = 1 + ((seed >> 9) % (rows - 2));
        m[r * cols + c] = (seed & 0x100) ? B_STAR1 : B_STAR0;
    }
    for (c = 0; c < cols; c++) m[(GY / 8) * cols + c] = B_GROUND;
    if (!title) {
        for (j = 0; j < NPLAT; j++) {
            uint8_t c0 = PLAT[j].x / 8, c1 = (PLAT[j].x + PLAT[j].w) / 8;
            r = PLAT[j].y / 8;
            for (c = c0; c < c1; c++) m[r * cols + c] = (c == c0) ? B_PLL : ((c == c1 - 1) ? B_PLR : B_PLM);
        }
    } else {
        s = (cols - 13) / 2;
        for (j = 0; j < 6; j++) {
            c = s + lcol[j];
            m[4 * cols + c] = B_LOGO + letters[j] * 4;     m[4 * cols + c + 1] = B_LOGO + letters[j] * 4 + 1;
            m[5 * cols + c] = B_LOGO + letters[j] * 4 + 2; m[5 * cols + c + 1] = B_LOGO + letters[j] * 4 + 3;
        }
    }
}

static void num(char *s, uint32_t v, uint8_t digits) {
    uint8_t i;
    for (i = digits; i > 0; i--) { s[i - 1] = '0' + (v % 10); v /= 10; }
}
void scene_hud(char *s, uint8_t cols) {
    uint8_t i;
    for (i = 0; i < cols; i++) s[i] = ' ';
    s[0] = 'S'; s[1] = 'C'; num(s + 2, G.score, 6);
    s[9] = 'L'; s[10] = '0' + (G.lives % 10);
    s[12] = 'L'; s[13] = 'V'; num(s + 14, G.lv % 100, 2);
    s[17] = 'F'; s[18] = '0' + G.rk_fuel;
    if (cols >= 30) { s[21] = 'H'; s[22] = 'I'; num(s + 23, G.hi, 6); }
    s[cols] = 0;
}
