/* Jet Pak - Sega Master System front end (devkitSMS / SMSlib).  Game logic lives in ../common. */
#include "SMSlib.h"
#include <stdint.h>
#include "game.h"
#include "scene.h"
#include "tiles_sms.h"

SMS_EMBED_SEGA_ROM_HEADER(9999, 0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1, 0, "Jet Pak", "Jet Pak", "Rocket-building platform shooter");

#ifdef STUB
static uint8_t PSGPort;
#else
__sfr __at 0x7F PSGPort;
#endif

#define COLS 32
#define ROWS 24
static uint8_t map[COLS * ROWS];
static unsigned int map16[COLS * ROWS];
static char hud[COLS + 1];
static uint8_t cur_title = 255, over_drawn, att1 = 15, att3 = 15;
static uint16_t n1;
static const uint16_t J_DELIVER[] = { 213, 170, 143 };
static const uint16_t J_LEVEL[] = { 213, 170, 143, 107 };
static const uint16_t J_OVER[] = { 143, 170, 213, 254 };
static const uint16_t *jp; static uint8_t jn, jl, jt;

static const unsigned char bgpal[16] = { 0x00, 0x14, 0x2A, 0x3F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const unsigned char sprpal[16] = { 0x00, 0x03, 0x3C, 0x3F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void tone(uint8_t ch, uint16_t n, uint8_t vol) {
    PSGPort = 0x80 | (ch << 5) | (n & 15); PSGPort = (n >> 4) & 63; PSGPort = 0x90 | (ch << 5) | vol;
}
static void add(int16_t x, int16_t y, uint8_t t) {
    if (x < 0 || x > 248 || y < 0 || y > 183) return;
    SMS_addSprite(x, y, N_BKG + t);
}
static void jingle(const uint16_t *p, uint8_t n) { jp = p; jn = 0; jl = n; jt = 0; }
static void put_row(uint8_t x, uint8_t y, const char *s) {
    while (*s) { SMS_setTileatXY(x++, y, font_tile(*s)); s++; }
}
static void load_map(uint8_t title) {
    uint16_t i;
    SMS_displayOff();
    scene_map(map, COLS, ROWS, title);
    if (title) scene_text(map, COLS, 0, ROWS - 1, "HI SCORE");
    for (i = 0; i < COLS * ROWS; i++) map16[i] = map[i];
    SMS_loadTileMap(0, 0, map16, sizeof(map16));
    SMS_displayOn();
    cur_title = title; over_drawn = 0;
}
static void put_hi(void) {
    char s[7]; uint32_t v = G.hi; uint8_t i;
    for (i = 6; i > 0; i--) { s[i - 1] = '0' + (v % 10); v /= 10; } s[6] = 0;
    put_row(9, ROWS - 1, s);
}

void main(void) {
    unsigned int k; uint8_t in;
    SMS_init();
    SMS_setSpriteMode(SPRITEMODE_NORMAL);
    SMS_useFirstHalfTilesforSprites(1);              /* sprite tiles live after the background tiles */
    SMS_loadBGPalette(bgpal); SMS_loadSpritePalette(sprpal);
    SMS_loadTiles(bkg_tiles, 0, sizeof(bkg_tiles));
    SMS_loadTiles(spr_tiles, N_BKG, sizeof(spr_tiles));
    SMS_displayOn();
    game_init();
    for (;;) {
        SMS_initSprites(); scene_sprites(add); SMS_finalizeSprites();
        SMS_waitForVBlank(); SMS_copySpritestoSAT();

        k = SMS_getKeysStatus(); in = 0;
        if (k & PORT_A_KEY_LEFT) in |= IN_L;
        if (k & PORT_A_KEY_RIGHT) in |= IN_R;
        if (k & (PORT_A_KEY_UP | PORT_A_KEY_2)) in |= IN_U;   /* Up or button 2 = thrust */
        if (k & PORT_A_KEY_1) in |= IN_F;                    /* button 1 = laser / start */
        game_step(in);

        if (cur_title != (G.state == ST_TITLE)) load_map(G.state == ST_TITLE);
        if (G.state == ST_TITLE) {
            if (!(G.tick & 31)) put_row(10, 9, (G.tick & 32) ? "              " : "PRESS BUTTON 1");
            if (!(G.tick & 15)) put_hi();
        } else {
            if (!(G.tick & 3)) { uint8_t i; scene_hud(hud, COLS); for (i = 0; i < COLS; i++) SMS_setTileatXY(i, 0, font_tile(hud[i])); }
            if (G.state == ST_OVER && !over_drawn) { put_row(11, 11, "GAME OVER"); over_drawn = 1; }
        }

        /* PSG: ch0 jingles, ch1 effects, ch3 noise (explosions + thruster) */
        if (G.ev & EV_FIRE) { n1 = 70; att1 = 0; }
        if (G.ev & EV_PICK) { n1 = 120; att1 = 0; }
        if (G.ev & (EV_KILL | EV_DIE | EV_LAUNCH)) { att3 = 0; }
        if (G.ev & EV_DELIVER) jingle(J_DELIVER, 3);
        if (G.ev & EV_LEVEL) jingle(J_LEVEL, 4);
        if (G.ev & EV_OVER) jingle(J_OVER, 4);
        if (att1 < 15) { tone(1, n1, att1); n1 += 6; att1++; } else tone(1, 1, 15);
        if (att3 < 15) { PSGPort = 0xE4; PSGPort = 0xF0 | att3; if (!(G.tick & 1)) att3++; }
        else if (G.state == ST_PLAY && G.p.th) { PSGPort = 0xE5; PSGPort = 0xF0 | 11; }
        else PSGPort = 0xFF;
        if (jt) jt--; else if (jn < jl) { tone(0, jp[jn++], 2); jt = 6; } else tone(0, 1, 15);
    }
}
