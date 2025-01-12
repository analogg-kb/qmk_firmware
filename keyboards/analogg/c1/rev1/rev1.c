#include "usb_main.h"
#include "analogg.h"

static bool turn_off_is_rgb_enabled = false;
static bool is_usb_suspended = false;

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
    setPinInput(IS_BLE_PIN);
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    keyboard_post_init_user();
}

void matrix_scan_kb(void) {
    /**********RGB_DISABLE_WHEN_USB_SUSPENDED***********/
    if (USB_DRIVER.state==USB_SUSPENDED){
        is_usb_suspended = true;
        turn_off_is_rgb_enabled = rgb_matrix_is_enabled();
        if(SWITCH_USB_MODE() && turn_off_is_rgb_enabled){
            rgb_matrix_disable_noeeprom();
        }
    }else if (USB_DRIVER.state==USB_ACTIVE){
        if (is_usb_suspended && !turn_off_is_rgb_enabled){
            rgb_matrix_enable_noeeprom();
            turn_off_is_rgb_enabled = true;
            is_usb_suspended = false;
        }
    }

    matrix_scan_user();
}


#ifdef DIP_SWITCH_ENABLE
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) { return false;}
    if (index == 0) {
    #ifdef CONSOLE_ENABLE
        uprintf("dip_switch: index=%d, active= %d\n", (index), active);
    #endif
        default_layer_set(1UL << (active ? 2 : 0));
    }
    return true;
}
#endif

/* RGB Matrix variables */
#ifdef RGB_MATRIX_ENABLE
const is31_led PROGMEM g_is31_leds[DRIVER_LED_TOTAL] = {
/* Refer to IS31 manual for these locations
 *   driver
 *   |  R location
 *   |  |       G location
 *   |  |       |       B location
 *   |  |       |       | */
    {0, G_1,    H_1,    I_1},
    {0, G_2,    H_2,    I_2},
    {0, G_3,    H_3,    I_3},
    {0, G_4,    H_4,    I_4},
    {0, G_5,    H_5,    I_5},
    {0, G_6,    H_6,    I_6},
    {0, G_7,    H_7,    I_7},
    {0, G_8,    H_8,    I_8},
    {0, G_9,    H_9,    I_9},
    {0, G_10,   H_10,   I_10},
    {0, G_11,   H_11,   I_11},
    {0, G_12,   H_12,   I_12},
    {0, G_13,   H_13,   I_13},
    {0, G_14,   H_14,   I_14},
    {0, G_15,   H_15,   I_15},

    {0, D_1,    E_1,    F_1},
    {0, D_2,    E_2,    F_2},
    {0, D_3,    E_3,    F_3},
    {0, D_4,    E_4,    F_4},
    {0, D_5,    E_5,    F_5},
    {0, D_6,    E_6,    F_6},
    {0, D_7,    E_7,    F_7},
    {0, D_8,    E_8,    F_8},
    {0, D_9,    E_9,    F_9},
    {0, D_10,   E_10,   F_10},
    {0, D_11,   E_11,   F_11},
    {0, D_12,   E_12,   F_12},
    {0, D_13,   E_13,   F_13},
    {0, D_14,   E_14,   F_14},
    {0, D_15,   E_15,   F_15},

     {0, A_1,    B_1,    C_1},
    {0, A_2,    B_2,    C_2},
    {0, A_3,    B_3,    C_3},
    {0, A_4,    B_4,    C_4},
    {0, A_5,    B_5,    C_5},
    {0, A_6,    B_6,    C_6},
    {0, A_7,    B_7,    C_7},
    {0, A_8,    B_8,    C_8},
    {0, A_9,    B_9,    C_9},
    {0, A_10,   B_10,   C_10},
    {0, A_11,   B_11,   C_11},
    {0, A_12,   B_12,   C_12},
    {0, A_13,   B_13,   C_13},
    {0, A_14,   B_14,   C_14},
    {0, A_15,   B_15,   C_15},

    {1, G_1,    H_1,    I_1},
    {1, G_2,    H_2,    I_2},
    {1, G_3,    H_3,    I_3},
    {1, G_4,    H_4,    I_4},
    {1, G_5,    H_5,    I_5},
    {1, G_6,    H_6,    I_6},
    {1, G_7,    H_7,    I_7},
    {1, G_8,    H_8,    I_8},
    {1, G_9,    H_9,    I_9},
    {1, G_10,   H_10,   I_10},
    {1, G_11,   H_11,   I_11},
    {1, G_12,   H_12,   I_12},
    {1, G_13,   H_13,   I_13},
    {1, G_14,   H_14,   I_14},
    {1, G_15,   H_15,   I_15},

    {1, D_1,    E_1,    F_1},
    {1, D_2,    E_2,    F_2},
    {1, D_3,    E_3,    F_3},
    {1, D_4,    E_4,    F_4},
    {1, D_5,    E_5,    F_5},
    {1, D_6,    E_6,    F_6},
    {1, D_7,    E_7,    F_7},
    {1, D_8,    E_8,    F_8},
    {1, D_9,    E_9,    F_9},
    {1, D_10,   E_10,   F_10},
    {1, D_11,   E_11,   F_11},
    {1, D_12,   E_12,   F_12},
    {1, D_13,   E_13,   F_13},
    {1, D_14,   E_14,   F_14},
    {1, D_15,   E_15,   F_15},

    {1, A_1,    B_1,    C_1},
    {1, A_2,    B_2,    C_2},
    {1, A_3,    B_3,    C_3},
    {1, A_4,    B_4,    C_4},
    {1, A_5,    B_5,    C_5},
    {1, A_6,    B_6,    C_6},
    {1, A_7,    B_7,    C_7},
    {1, A_8,    B_8,    C_8},
    {1, A_9,    B_9,    C_9},
    {1, A_10,   B_10,   C_10},
    {1, A_11,   B_11,   C_11},
    {1, A_12,   B_12,   C_12},
    {1, A_13,   B_13,   C_13},
    {1, A_14,   B_14,   C_14},
    {1, A_15,   B_15,   C_15},

};


led_config_t g_led_config = {
    {
        {      0,      1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,      12,     13,     14 },
        {     15,     16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,      27,     28,     29 },
        {     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,      42,     43,     44 },
        {     45,     46,     47,     48,     49,     50,     51,     52,     53,     54,     55,     56,      57, NO_LED,     59 },
        {     60,     61,     62,     63,     64,     65,     66,     67,     68,     69,     70,     71,  NO_LED,     73, NO_LED },
        {     75,     76,     77, NO_LED, NO_LED, NO_LED,     81, NO_LED, NO_LED,     84,     85,     86,      87,     88,     89 }
    },
    {
        {0,0},  {16, 0}, {32, 0}, {48, 0}, {64, 0}, {80, 0}, {96, 0}, {112, 0}, {128, 0}, {144, 0}, {160, 0}, {176, 0}, {192, 0}, {208, 0}, {224, 0},
        {0,13}, {16,13}, {32,13}, {48,13}, {64,13}, {80,13}, {96,13}, {112,13}, {128,13}, {144,13}, {160,13}, {176,13}, {192,13}, {208,13}, {224,13},
        {0,26}, {16,26}, {32,26}, {48,26}, {64,26}, {80,26}, {96,26}, {112,26}, {128,26}, {144,26}, {160,26}, {176,26}, {192,26}, {208,26}, {224,26},
        {0,38}, {16,38}, {32,38}, {48,38}, {64,38}, {80,38}, {96,38}, {112,38}, {128,38}, {144,38}, {160,38}, {176,38}, {192,38}, {208,38}, {224,38},
        {0,51}, {16,51}, {32,51}, {48,51}, {64,51}, {80,51}, {96,51}, {112,51}, {128,51}, {144,51}, {160,51}, {176,51}, {192,51}, {208,51}, {224,51},
        {0,64}, {16,64}, {32,64}, {48,64}, {64,64}, {80,64}, {96,64}, {112,64}, {128,64}, {144,64}, {160,64}, {176,64}, {192,64}, {208,64}, {224,64},
    },
    {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    }
};
#endif
