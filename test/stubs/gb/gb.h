#include <stdint.h>
#define J_LEFT 2
#define J_RIGHT 1
#define J_UP 4
#define J_A 16
#define J_B 32
#define J_START 128
extern uint8_t NR10_REG,NR11_REG,NR12_REG,NR13_REG,NR14_REG,NR41_REG,NR42_REG,NR43_REG,NR44_REG,NR50_REG,NR51_REG,NR52_REG,BGP_REG,OBP0_REG;
#define SHOW_BKG
#define SHOW_SPRITES
#define DISPLAY_ON
#define DISPLAY_OFF
void wait_vbl_done(void); uint8_t joypad(void);
void set_bkg_data(uint8_t,uint8_t,const unsigned char*); void set_sprite_data(uint8_t,uint8_t,const unsigned char*);
void set_bkg_tiles(uint8_t,uint8_t,uint8_t,uint8_t,const uint8_t*);
void set_sprite_tile(uint8_t,uint8_t); void move_sprite(uint8_t,uint8_t,uint8_t);
