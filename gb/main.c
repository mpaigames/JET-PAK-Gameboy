/* Jet Pak - Game Boy front end (GBDK-2020).  Game logic lives in ../common. */
#include <gb/gb.h>
#include <stdint.h>
#include "game.h"
#include "scene.h"
#include "tiles_gb.h"

#define COLS 20
#define ROWS 18
static uint8_t map[COLS * ROWS];
static char hud[COLS + 1];
static uint8_t nspr, prev_nspr, cur_title = 255, over_drawn;
static const uint16_t J_DELIVER[] = { 0x706, 0x739, 0x759 };
static const uint16_t J_LEVEL[] = { 0x706, 0x739, 0x759, 0x783 };
static const uint16_t J_OVER[] = { 0x759, 0x739, 0x706, 0x6B2 };
static const uint16_t *jp; static uint8_t jn, jl, jt;

static void add(int16_t x, int16_t y, uint8_t t) {
    if (nspr >= 40 || x < -7 || x > 159 || y < -7 || y > 143) return;
    set_sprite_tile(nspr, t); move_sprite(nspr, (uint8_t)(x + 8), (uint8_t)(y + 16)); nspr++;
}
static void snd1(uint16_t f, uint8_t env, uint8_t sweep) {
    NR10_REG = sweep; NR11_REG = 0x80; NR12_REG = env; NR13_REG = f & 0xFF; NR14_REG = 0x80 | (f >> 8);
}
static void noise(uint8_t env, uint8_t poly) { NR41_REG = 0; NR42_REG = env; NR43_REG = poly; NR44_REG = 0x80; }
static void jingle(const uint16_t *p, uint8_t n) { jp = p; jn = 0; jl = n; jt = 0; }

static void put_row(uint8_t x, uint8_t y, const char *s) {
    uint8_t buf[COLS], n = 0;
    while (*s && n < COLS) buf[n++] = font_tile(*s++);
    set_bkg_tiles(x, y, n, 1, buf);
}
static void load_map(uint8_t title) {
    DISPLAY_OFF;
    scene_map(map, COLS, ROWS, title);
    if (title) { scene_text(map, COLS, 0, ROWS - 1, "HI SCORE"); }
    set_bkg_tiles(0, 0, COLS, ROWS, map);
    DISPLAY_ON;
    cur_title = title; over_drawn = 0;
}
static void put_hi(void) {
    char s[7]; uint32_t v = G.hi; uint8_t i;
    for (i = 6; i > 0; i--) { s[i - 1] = '0' + (v % 10); v /= 10; } s[6] = 0;
    put_row(9, ROWS - 1, s);
}

void main(void) {
    uint8_t k, in, i;
    NR52_REG = 0x80; NR51_REG = 0xFF; NR50_REG = 0x77;
    BGP_REG = 0x1B; OBP0_REG = 0x1B;                 /* index 0 = black (space), 3 = white */
    set_bkg_data(0, N_BKG, bkg_tiles); set_sprite_data(0, N_SPR, spr_tiles);
    SHOW_BKG; SHOW_SPRITES;
    game_init();
    for (;;) {
        wait_vbl_done();
        k = joypad(); in = 0;
        if (k & J_LEFT) in |= IN_L;
        if (k & J_RIGHT) in |= IN_R;
        if (k & (J_A | J_UP)) in |= IN_U;            /* A or Up = thrust */
        if (k & J_B) in |= IN_F;                     /* B = laser */
        if (k & J_START) in |= IN_S;
        game_step(in);

        if (cur_title != (G.state == ST_TITLE)) load_map(G.state == ST_TITLE);
        if (G.state == ST_TITLE) {
            if (!(G.tick & 31)) put_row(5, 9, (G.tick & 32) ? "           " : "PRESS START");
            if (!(G.tick & 15)) put_hi();
        } else {
            if (!(G.tick & 3)) { uint8_t b[COLS]; scene_hud(hud, COLS); for (i = 0; i < COLS; i++) b[i] = font_tile(hud[i]); set_bkg_tiles(0, 0, COLS, 1, b); }
            if (G.state == ST_OVER && !over_drawn) { put_row(5, 8, "GAME OVER"); over_drawn = 1; }
        }
        nspr = 0; scene_sprites(add);
        for (i = nspr; i < prev_nspr; i++) move_sprite(i, 0, 0);
        prev_nspr = nspr;

        if (G.ev & EV_FIRE) snd1(0x700, 0x81, 0x1B);
        if (G.ev & EV_KILL) noise(0xF2, 0x53);
        if (G.ev & EV_PICK) snd1(0x7A0, 0xA1, 0);
        if (G.ev & EV_DIE) noise(0xF7, 0x73);
        if (G.ev & EV_LAUNCH) noise(0xF8, 0x77);
        if (G.ev & EV_DELIVER) jingle(J_DELIVER, 3);
        if (G.ev & EV_LEVEL) jingle(J_LEVEL, 4);
        if (G.ev & EV_OVER) jingle(J_OVER, 4);
        if (jt) jt--; else if (jn < jl) { snd1(jp[jn++], 0xA2, 0); jt = 6; }
    }
}
