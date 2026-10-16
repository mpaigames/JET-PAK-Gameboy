#ifndef SCENE_H
#define SCENE_H
#include <stdint.h>
typedef void (*AddFn)(int16_t x, int16_t y, uint8_t tile);
void scene_sprites(AddFn add);                                   /* emit every visible sprite for this frame */
void scene_map(uint8_t *m, uint8_t cols, uint8_t rows, uint8_t title); /* static background (platforms, ground, stars, logo) */
void scene_text(uint8_t *m, uint8_t cols, uint8_t x, uint8_t y, const char *s);
void scene_hud(char *s, uint8_t cols);                           /* builds the top-row HUD text */
uint8_t font_tile(char c);
#endif
