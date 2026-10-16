#include <stdint.h>
#define SMS_EMBED_SEGA_ROM_HEADER(a,b)
#define SMS_EMBED_SDSC_HEADER_AUTO_DATE(a,b,c,d,e)
#define __sfr
#define __at
#define SPRITEMODE_NORMAL 0
#define PORT_A_KEY_LEFT 4
#define PORT_A_KEY_RIGHT 8
#define PORT_A_KEY_UP 1
#define PORT_A_KEY_1 16
#define PORT_A_KEY_2 32
void SMS_init(void);void SMS_setSpriteMode(int);void SMS_useFirstHalfTilesforSprites(unsigned char);
void SMS_loadBGPalette(const void*);void SMS_loadSpritePalette(const void*);void SMS_loadTiles(const void*,unsigned int,unsigned int);
void SMS_displayOn(void);void SMS_displayOff(void);void SMS_initSprites(void);signed char SMS_addSprite(unsigned char,unsigned int,unsigned char);
void SMS_finalizeSprites(void);void SMS_waitForVBlank(void);void SMS_copySpritestoSAT(void);unsigned int SMS_getKeysStatus(void);
void SMS_setTileatXY(unsigned char,unsigned char,unsigned int);void SMS_loadTileMap(unsigned char,unsigned char,const void*,unsigned int);
