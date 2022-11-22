#include "analogg.h"


void enable_swdp(void){
#ifdef ENABLE_SWDP
    AFIO->MAPR&=0XF8FFFFFF;     //disable JTAG enable SWD
    AFIO->MAPR|=0X02000000;
#else
    AFIO->MAPR&=0XF8FFFFFF;     //disable JTAG and SWD
    AFIO->MAPR|=0X04000000;
#endif
}

void board_init(void){
    enable_swdp();
}

void keyboard_pre_init_kb(void) {
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    keyboard_post_init_user();
}
