#ifndef GAME_H
#define GAME_H
#include <stdint.h>
#include "config.h"

/* input bits */
#define IN_L 1
#define IN_R 2
#define IN_U 4
#define IN_F 8
#define IN_S 16
/* events (for sound) */
#define EV_FIRE 1
#define EV_KILL 2
#define EV_PICK 4
#define EV_DELIVER 8
#define EV_DIE 16
#define EV_LAUNCH 32
#define EV_LEVEL 64
#define EV_OVER 128

enum { ST_TITLE, ST_PLAY, ST_DEAD, ST_LAUNCH, ST_OVER };
#define MAXAL 5
#define MAXIT 3
#define MAXFX 3

/* all positions are pixels*16 (4 fractional bits) */
typedef struct { int16_t x, y, w; } Plat;
typedef struct { int16_t x, y, vx, vy; int8_t dir; uint8_t gr, th, inv, cd; } Pl;
typedef struct { uint8_t on, k, st, t; int16_t x, y, vx, vy; } Al;
typedef struct { uint8_t on, type, seg, held; int16_t x, y, vy; } It; /* type 0=rocket section 1=fuel pod */
typedef struct { uint8_t t; int16_t x, y; } Fx;                      /* x,y in whole pixels */
typedef struct { uint8_t t; int16_t x0, x1, y; } Bm;                 /* whole pixels */

typedef struct {
    uint8_t state, lv, lives, rk_n, rk_fuel, ev, tick, prev_in;
    uint16_t timer, at, ft, rng;
    int16_t rk_ly, rk_v;
    uint32_t score, hi;
    int8_t carry;
    Pl p; Al al[MAXAL]; It it[MAXIT]; Fx fx[MAXFX]; Bm beam;
} Game;

extern Game G;
extern const Plat PLAT[NPLAT];
void game_init(void);
void game_step(uint8_t in);
#endif
