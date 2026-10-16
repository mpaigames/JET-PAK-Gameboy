#include "game.h"
Game G;
const Plat PLAT[NPLAT] = PLAT_DATA;
#define PL(v) ((v) >> 4)

static uint16_t rnd(void) { G.rng ^= G.rng << 7; G.rng ^= G.rng >> 9; G.rng ^= G.rng << 8; return G.rng; }
static int ov(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
static int sgn(int v) { return v > 0 ? 1 : (v < 0 ? -1 : 0); }
static void add_fx(int x, int y) {
    uint8_t i;
    for (i = 0; i < MAXFX; i++) if (!G.fx[i].t) { G.fx[i].t = 14; G.fx[i].x = x; G.fx[i].y = y; return; }
}
static It *add_item(uint8_t type, uint8_t seg, int x, int y) {
    uint8_t i;
    for (i = 0; i < MAXIT; i++) if (!G.it[i].on) {
        It *o = &G.it[i];
        o->on = 1; o->type = type; o->seg = seg; o->held = 0; o->x = x * 16; o->y = y * 16; o->vy = 0;
        return o;
    }
    return 0;
}
static void spawn_part(void) {
    const Plat *q = &PLAT[G.rk_n == 1 ? PART_PLAT_1 : PART_PLAT_2];
    add_item(0, G.rk_n, q->x + q->w / 2 - 8, -16);
}
static void reset_pl(void) {
    Pl *p = &G.p;
    p->x = 40 * 16; p->y = (GY - 16) * 16; p->vx = p->vy = 0; p->dir = 1;
    p->gr = 1; p->th = 0; p->inv = 100; p->cd = 0; G.carry = -1;
}
static void start_level(void) {
    uint8_t i;
    G.rk_n = 1; G.rk_fuel = 0; G.rk_ly = 0; G.rk_v = 0;
    for (i = 0; i < MAXIT; i++) G.it[i].on = 0;
    for (i = 0; i < MAXAL; i++) G.al[i].on = 0;
    for (i = 0; i < MAXFX; i++) G.fx[i].t = 0;
    G.beam.t = 0; G.at = 50; G.ft = 60;
    spawn_part(); reset_pl();
}
static void new_game(void) { G.lv = 1; G.score = 0; G.lives = 4; start_level(); G.state = ST_PLAY; }

void game_init(void) {
    G.hi = 0; G.rng = 0xACE1; G.state = ST_TITLE; G.tick = 0; G.prev_in = 0; G.ev = 0; G.carry = -1;
    G.beam.t = 0;
}

static void fall(It *o) {
    int w = o->type ? 8 : 16, i, py = PL(o->y) + w, ny;
    o->vy += 2; if (o->vy > 34) o->vy = 34;
    o->y += o->vy; ny = PL(o->y) + w;
    for (i = 0; i < NPLAT; i++) {
        const Plat *q = &PLAT[i];
        if (PL(o->x) + w > q->x && PL(o->x) < q->x + q->w && ny >= q->y && py <= q->y + 1) { o->y = (q->y - w) * 16; o->vy = 0; }
    }
    if (PL(o->y) + w >= GY) { o->y = (GY - w) * 16; o->vy = 0; }
}

static void kill_al(Al *a, int score) {
    add_fx(PL(a->x), PL(a->y)); a->on = 0; G.ev |= EV_KILL;
    if (score) G.score += 100;
}
static void die(void) {
    uint8_t i;
    G.lives--; G.ev |= EV_DIE;
    if (G.carry >= 0) { G.it[G.carry].held = 0; G.carry = -1; }
    add_fx(PL(G.p.x), PL(G.p.y)); add_fx(PL(G.p.x), PL(G.p.y) + 8);
    for (i = 0; i < MAXAL; i++) G.al[i].on = 0;
    G.beam.t = 0; G.state = ST_DEAD; G.timer = 70;
}

static void upd_player(uint8_t in) {
    Pl *p = &G.p; int i, top;
    if ((in & IN_L) && !(in & IN_R)) { p->vx -= 2; if (p->vx < -27) p->vx = -27; p->dir = -1; }
    else if ((in & IN_R) && !(in & IN_L)) { p->vx += 2; if (p->vx > 27) p->vx = 27; p->dir = 1; }
    else if (p->gr) p->vx = p->vx / 2;
    else p->vx -= p->vx / 64;
    if (in & IN_U) { p->vy -= 3; if (p->vy < -30) p->vy = -30; p->th = 1; }
    else { p->vy += (G.tick & 1) ? 2 : 1; if (p->vy > 38) p->vy = 38; p->th = 0; }
    p->x += p->vx;
    if (p->x < -8 * 16) p->x = SCR_W * 16; else if (p->x > SCR_W * 16) p->x = -8 * 16;
    for (i = 0; i < NPLAT; i++) {
        const Plat *q = &PLAT[i];
        if (ov(PL(p->x), PL(p->y), 8, 16, q->x, q->y, q->w, 8)) {
            if (p->vx > 0) p->x = (q->x - 8) * 16; else if (p->vx < 0) p->x = (q->x + q->w) * 16;
            p->vx = 0;
        }
    }
    p->gr = 0; p->y += p->vy;
    top = TOP * 16 + (G.carry >= 0 ? (G.it[G.carry].type ? 9 : 17) * 16 : 0);
    if (p->y < top) { p->y = top; if (p->vy < 0) p->vy = 0; }
    for (i = 0; i < NPLAT; i++) {
        const Plat *q = &PLAT[i];
        if (ov(PL(p->x), PL(p->y), 8, 16, q->x, q->y, q->w, 8)) {
            if (p->vy >= 0) { p->y = (q->y - 16) * 16; p->vy = 0; p->gr = 1; }
            else { p->y = (q->y + 8) * 16; p->vy = 6; }
        }
    }
    if (PL(p->y) + 16 >= GY) { p->y = (GY - 16) * 16; p->vy = 0; p->gr = 1; }
    if (p->inv) p->inv--;
}

static void upd_fire(uint8_t in) {
    Pl *p = &G.p; int by, x0, x1, i;
    if (p->cd) p->cd--;
    if ((in & IN_F) && !p->cd) {
        p->cd = 12; by = PL(p->y) + 6;
        if (p->dir > 0) { x0 = PL(p->x) + 8; x1 = x0 + BEAM_LEN;
            for (i = 0; i < NPLAT; i++) if (by >= PLAT[i].y && by < PLAT[i].y + 8 && PLAT[i].x >= x0 && PLAT[i].x < x1) x1 = PLAT[i].x; }
        else { x1 = PL(p->x); x0 = x1 - BEAM_LEN;
            for (i = 0; i < NPLAT; i++) if (by >= PLAT[i].y && by < PLAT[i].y + 8 && PLAT[i].x + PLAT[i].w <= x1 && PLAT[i].x + PLAT[i].w > x0) x0 = PLAT[i].x + PLAT[i].w; }
        G.beam.t = 5; G.beam.x0 = x0; G.beam.x1 = x1; G.beam.y = by; G.ev |= EV_FIRE;
        for (i = 0; i < MAXAL; i++) {
            Al *a = &G.al[i];
            if (a->on && PL(a->y) < by + 2 && PL(a->y) + 8 > by - 2 && PL(a->x) < x1 && PL(a->x) + 8 > x0) kill_al(a, 1);
        }
    }
    if (G.beam.t) G.beam.t--;
}

static void upd_items(void) {
    Pl *p = &G.p; uint8_t i, n = 0;
    for (i = 0; i < MAXIT; i++) if (G.it[i].on && G.carry != i) fall(&G.it[i]);
    if (G.carry < 0)
        for (i = 0; i < MAXIT; i++) {
            It *o = &G.it[i]; int w = o->type ? 8 : 16;
            if (o->on && ov(PL(p->x), PL(p->y), 8, 16, PL(o->x), PL(o->y), w, w)) { G.carry = i; o->held = 1; G.ev |= EV_PICK; break; }
        }
    if (G.carry >= 0) {
        It *o = &G.it[G.carry]; int w = o->type ? 8 : 16, cx = PL(p->x) + 4;
        o->x = p->x + (8 - w) * 8; o->y = p->y - (w + 1) * 16; o->vy = 0;
        if (cx > RX - 9 && cx < RX + 9) {
            o->on = 0; o->held = 0; G.carry = -1; G.score += 100; G.ev |= EV_DELIVER;
            if (o->type == 0) { G.rk_n++; if (G.rk_n < 3) spawn_part(); } else G.rk_fuel++;
        }
    }
    if (G.rk_n == 3) {
        for (i = 0; i < MAXIT; i++) if (G.it[i].on && G.it[i].type) n++;
        if (G.rk_fuel + n < 6 && G.ft) G.ft--;
        if (G.rk_fuel + n < 6 && !G.ft) { add_item(1, 0, 16 + (rnd() % (SCR_W - 40)), -8); G.ft = 110; }
    }
    if (G.rk_n == 3 && G.rk_fuel >= 6 && ov(PL(p->x), PL(p->y), 8, 16, RX - 8, GY - 48, 16, 48)) {
        for (i = 0; i < MAXAL; i++) G.al[i].on = 0;
        if (G.carry >= 0) { G.it[G.carry].on = 0; G.carry = -1; }
        G.state = ST_LAUNCH; G.timer = 0; G.rk_v = 0; G.ev |= EV_LAUNCH;
    }
}

static void upd_aliens(void) {
    Pl *p = &G.p; uint8_t i, cnt = 0, lvc = G.lv > 10 ? 10 : G.lv;
    int sp = 8 + lvc * 4 / 3, maxn = 2 + G.lv, pool = 2 + G.lv, j;
    if (maxn > MAXAL) maxn = MAXAL;
    if (pool > 4) pool = 4;
    for (i = 0; i < MAXAL; i++) if (G.al[i].on) cnt++;
    if (G.at) G.at--;
    if (!G.at && cnt < maxn) {
        for (i = 0; i < MAXAL; i++) if (!G.al[i].on) {
            Al *a = &G.al[i]; int left = rnd() & 1;
            a->on = 1; a->k = rnd() % pool; a->st = 0; a->t = rnd() & 63;
            a->x = left ? -8 * 16 : SCR_W * 16; a->y = (TOP + 8 + rnd() % (GY - TOP - 32)) * 16;
            a->vx = left ? sp : -sp; a->vy = a->k == 2 ? 16 : 0;
            break;
        }
        G.at = 70 - G.lv * 6; if (G.at < 25) G.at = 25;
    }
    for (i = 0; i < MAXAL; i++) {
        Al *a = &G.al[i]; int hit = 0;
        if (!a->on) continue;
        a->t++;
        switch (a->k) {
        case 0: a->x += a->vx; a->y += (a->t & 32) ? 5 : -5; break;
        case 1: a->x += sgn(p->x - a->x) * sp * 3 / 4; a->y += sgn(p->y - a->y) * sp * 3 / 4; break;
        case 2: a->x += a->vx; a->vy += 1; a->y += a->vy; if (PL(a->y) + 8 >= GY) { a->y = (GY - 8) * 16; a->vy = -36; } break;
        default: a->x += a->vx; a->y += (a->t & 16) ? sp : -sp; break;
        }
        if (a->x < -9 * 16) a->x = SCR_W * 16; else if (a->x > SCR_W * 16 + 16) a->x = -8 * 16;
        if (PL(a->y) < TOP) { a->y = TOP * 16; if (a->vy < 0) a->vy = 0; }
        if (PL(a->y) + 8 > GY) a->y = (GY - 8) * 16;
        for (j = 0; j < NPLAT; j++) {
            const Plat *q = &PLAT[j];
            if (ov(PL(a->x), PL(a->y), 8, 8, q->x, q->y, q->w, 8)) {
                if (a->k == 2) { if (a->vy > 0) { a->y = (q->y - 8) * 16; a->vy = -36; } else { a->y = (q->y + 8) * 16; a->vy = 16; } }
                else hit = 1;
            }
        }
        if (hit) { kill_al(a, 0); continue; }
        if (!p->inv && ov(PL(a->x) + 1, PL(a->y) + 1, 6, 6, PL(p->x), PL(p->y), 8, 16)) { kill_al(a, 0); die(); return; }
    }
}

void game_step(uint8_t in) {
    uint8_t edge = in & ~G.prev_in, i;
    G.prev_in = in; G.ev = 0; G.tick++;
    for (i = 0; i < MAXFX; i++) if (G.fx[i].t) G.fx[i].t--;
    switch (G.state) {
    case ST_TITLE: if (edge & (IN_F | IN_S)) new_game(); break;
    case ST_PLAY:
        upd_player(in); upd_fire(in); upd_items();
        if (G.state == ST_PLAY) upd_aliens();
        break;
    case ST_DEAD:
        if (--G.timer == 0) {
            if (G.lives) { reset_pl(); G.state = ST_PLAY; }
            else { if (G.score > G.hi) G.hi = G.score; G.state = ST_OVER; G.timer = 70; G.ev |= EV_OVER; }
        }
        break;
    case ST_LAUNCH:
        G.timer++; G.rk_v++; if (G.timer > 30) G.rk_ly -= G.rk_v;
        if (G.timer > 160) { G.lv++; G.score += 500; if (G.score > G.hi) G.hi = G.score; start_level(); G.state = ST_PLAY; G.ev |= EV_LEVEL; }
        break;
    case ST_OVER:
        if (G.timer) G.timer--; else if (edge & (IN_F | IN_S)) G.state = ST_TITLE;
        break;
    }
}
