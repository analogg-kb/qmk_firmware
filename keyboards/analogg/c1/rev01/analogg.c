// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "usb_main.h"

#include "analogg_ble_protocol.h"
#include "analogg_ble.h"
#include <ctype.h>
#include "analog.h"
#include "analogg.h"
#include "led_indicator.h"

#include "chtime.h"
#include "config.h"
#include "eeprom.h"
#include "keycode_config.h"

// Disable the DIP switch key
const matrix_row_t matrix_mask[] = {
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b111111111011111,
};

bool            is_rgb_enabled              = true;
bool            is_usb_suspended            = false;

bool            isChrg                      = false;
uint8_t         chrgCount                   = 0;
float           rsoc                        = 0;

bool            custom_pressed              = false;
uint16_t        custom_keycode              = KC_NO;
uint16_t        last_keycode                = KC_NO;
uint16_t        custom_pressed_long_time    = 0;
uint16_t        log_time                    = 0;

#define RGB_SLEEP_TIMEOUT                   10
uint16_t        rgb_sleep_time              = 0;

protocol_cmd    pop_protocol_cmd            = {0};

_rgb_matrix_indicator  rgb_matrix_indicator               = RGB_MATRIX_ANIMATION;
// _led_indicator  led_indicator               = {.power={RGB_BLACK},.ble={RGB_BLACK},.caps_lock={RGB_BLACK},.battery_level={RGB_BLACK}};

/* --------------------------- qmk function------------------------------ */
void board_init(void){
    AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_Msk;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_DISABLE_Msk; //disable JTAG and SWD
}

void keyboard_pre_init_kb(void) {
    setPinInput(IS_BLE);

    setPinInput(IS_CHRG);
    writePinHigh(IS_CHRG);
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    defer_exec(TIMER_BASE_TIME, my_callback, NULL);
    keyboard_post_init_user();

    if (rgb_matrix_is_enabled()){
        is_rgb_enabled = true;
    }else{
        is_rgb_enabled = false;
    }
}

static uint16_t time_count = 0;
static uint16_t bt_log_time_count = 0;
uint32_t my_callback(uint32_t trigger_time, void *cb_arg) {
    if (!readPin(IS_BLE)){
        if (is_usb_suspended){
            led_indicator_set_black();
            rgb_matrix_indicator = RGB_MATRIX_ANIMATION;  //Turn on RGB matrix
        }
        return TIMER_BASE_TIME*100;
    }else{
        long_pressed_event();

        switch (ble_send_state){
            case TX_IDLE:     uart_tx_data_handle();        break;
            case TX_TIMEOUT:  analogg_ble_cmd_tx(++mSeqId); break;
            case TX_RESTART:  analogg_ble_cmd_tx(++mSeqId); break;
            default:          ble_send_state++;             break;
        }

        uint8_t state = ble_tunnel_state.list[ble_tunnel_state.tunnel];
        time_count++;
        if (time_count%T100MS==0){
            //ble led state
            if (state==CONNECTED || state==CONNECTED_AND_ACTIVE) 
                led_indicator_set_ble_to(ble_tunnel_state.tunnel);

            //POWER LED state
            // uprintf("chrgCount=%d %b\n",chrgCount,isChrg);
            if (chrgCount>30)isChrg = true;
            chrgCount = 0;
            led_indicator_set_power_pwm(isChrg);

            //CAPS_LOCK LED state
            if (host_keyboard_led_state().caps_lock) {
                led_indicator_set_caps_lock(RGB_WHITE);
            } else {
                led_indicator_set_caps_lock(RGB_BLACK);
            }
            // get ble
            if (is_tx_idle()){
                analogg_ble_send_cmd(DATA_TYPE_STATE);
                bt_log_time_count++;
                if (bt_log_time_count>600){
                    bt_log_time_count=0;
                    analogg_ble_send_cmd_by_val(DATA_TYPE_BATTERY_LEVEL,(uint8_t)((int16_t)rsoc));
                }
            }
        }
        if (time_count%T200MS==0)if (state==ADV_WAIT_CONNECTING_ACTIVE)led_indicator_set_ble_to(ble_tunnel_state.tunnel);
        if (time_count%T400MS==0)if (state==ADV_WAIT_CONNECTING_ACTIVE)led_indicator_set_ble(RGB_BLACK);
        
        if (time_count%T1S==0){
            log_time++;
            //BATTERY LEVEL LED state
            uint16_t bl = analogReadPin(BATTERY_LEVEL_PORT);
            rsoc = ((bl-BATTERY_RSOC_0)/BATTERY_RSOC_AREA) * 100.00f;
            if (rsoc>100)rsoc=100;
            if (rsoc<0)rsoc=0;
            if (rsoc<10){
                led_indicator_set_battery_level(RGB_RED);
            }else if (rsoc>=10 && rsoc<40){  //10-40
                led_indicator_set_battery_level(RGB_ORANGE);
            }else if (rsoc>=40 && rsoc<70){   //40-70
                led_indicator_set_battery_level(RGB_YELLOW);
            }else if (rsoc>=70){
                led_indicator_set_battery_level(RGB_GREEN);
            }
            //rgb
            if (rgb_sleep_time<RGB_SLEEP_TIMEOUT){
                rgb_sleep_time++;
                if (rgb_sleep_time==RGB_SLEEP_TIMEOUT){
                    rgb_matrix_disable_noeeprom();
                }
            }

            if (state==IDLE || state==ADV_WAIT_CONNECTING || state==ADV_WAIT_CONNECTING_INACTIVE)led_indicator_set_ble_to(ble_tunnel_state.tunnel);
        }
        if (time_count%T2S==0){
            if (state==IDLE || state==ADV_WAIT_CONNECTING || state==ADV_WAIT_CONNECTING_INACTIVE)led_indicator_set_ble(RGB_BLACK);
            time_count = 0;
        }
        if (is_tx_idle())led_indicator_show();
    }
    return TIMER_BASE_TIME;
}

void key_pressed_rgb_enabled(void){
    if(is_rgb_enabled){
        rgb_matrix_enable_noeeprom();
        uprintf("rgb_matrix_enable_noeeprom=2\n");
    }else {
        rgb_matrix_disable_noeeprom();
    }  
    rgb_sleep_time = 0;
}

void ble_state_show_by_rgb_matrix(uint8_t index,uint8_t state){
    index+=31;
    switch (state){
        case IDLE:      //always bright
            rgb_matrix_set_color(index, RGB_RED);
            break;
        case ADV_WAIT_CONNECTING: //slow blink
            if (time_count%T2S<T1S){
                rgb_matrix_set_color(index, RGB_YELLOW);
            }else{
                rgb_matrix_set_color(index, RGB_BLACK);
            }
            break;
        case ADV_WAIT_CONNECTING_ACTIVE:   //quick blink
            if (time_count%T400MS<T200MS){
                rgb_matrix_set_color(index, RGB_YELLOW);
            }else{
                rgb_matrix_set_color(index, RGB_BLACK);
            }
        break;
        case ADV_WAIT_CONNECTING_INACTIVE:      //slow blink
            if (time_count%T2S<T1S){
                rgb_matrix_set_color(index, RGB_YELLOW);
            }else{
                rgb_matrix_set_color(index, RGB_BLACK);
            }
        break;
        case CONNECTED:  //always bright
                rgb_matrix_set_color(index, RGB_YELLOW);
        break;
        case CONNECTED_AND_ACTIVE:
            rgb_matrix_set_color(index, RGB_BLUE);

            if (last_save_tunnel!=tunnel){
                last_save_tunnel = tunnel;
                eeprom_write_byte(EE_ANALOGG_LINK_ID, tunnel);
            }
        break;
        default:break;
    }
}

bool rgb_matrix_indicators_kb(void) {
     if (is_ble_work_state_input())
        return false;

     if (rgb_matrix_indicator==RGB_MATRIX_ANIMATION)
        return false;

    rgb_matrix_set_color_all(0,0,0);
    if(rgb_matrix_indicator==BLE_LED_KEY_ONE){
        uint8_t index = tunnel-1;
        ble_state_show_by_rgb_matrix(index,ble_tunnel_state.list[index]);
    }else if (rgb_matrix_indicator==BLE_LED_KEY_ALL){
        for (uint8_t i = 0; i < BLE_TUNNEL_NUM; i++)ble_state_show_by_rgb_matrix(i,ble_tunnel_state.list[i]);     
    }
    return true;
}

void long_pressed_event(void){
    uint16_t time = 0;
    if (custom_keycode>=BT_TN1 && custom_keycode<=BT_FTY){
        if (!custom_pressed){
            last_keycode = KC_NO;
            return;
        }

        if (last_keycode!=custom_keycode){
            last_keycode = custom_keycode;
            custom_pressed_long_time = timer_read();
            return;
        }

        time = timer_read()>custom_pressed_long_time ? (timer_read()-custom_pressed_long_time) : (65535-custom_pressed_long_time)+timer_read();
        if (time>=LONG_PRESSED_TIME && last_keycode != KC_NO){
            //uprintf(" BLE time---: %d  0x%04X\n", time,custom_keycode);
            switch (custom_keycode){
                case BT_TN1: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 1);break;
                case BT_TN2: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 2);break;
                case BT_TN3: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 3);break;
                case BT_TN4: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 4);break;
                case BT_TN5: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 5);break;
                // case BT_TN6: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 6);break;
                // case BT_TN7: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 7);break;
                // case BT_TN8: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 8);break;
                case BT_FTY:
                    analogg_ble_send_cmd(DATA_TYPE_DEFAULT);
                    rgb_matrix_indicator = BLE_LED_KEY_ALL;
                    analogg_ble_send_cmd(DATA_TYPE_STATE);
                    uprintf("%5d Q:Restore factory mode\n",log_time);
                break;
                default: break;
            }
            custom_pressed = false;
            last_keycode = KC_NO;
        }
    }
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode==RGB_TOG){
        if (rgb_matrix_is_enabled()){
            is_rgb_enabled = false;
        }else{
            is_rgb_enabled = true;
        }
        uprintf("rgb_matrix_enable_noeeprom4=%d\n",is_rgb_enabled);
    }

    if (!readPin(IS_BLE)){
        #ifdef CONSOLE_ENABLE
            uprintf("%5d Q:cable,kc=%04x pressed=%d\n",log_time,keycode,record->event.pressed);
        #endif
        return process_record_user(keycode, record);
    }

    //BLE mode
    if (keycode>=QK_MOMENTARY && keycode<=QK_MOMENTARY_MAX){   //fn
        return process_record_user(keycode, record);
    }
    custom_keycode = keycode;
    custom_pressed = record->event.pressed;
    if (keycode>=BT_TN1 && keycode<=BT_FTY){    // costom key
        if (record->event.pressed){
            rgb_matrix_enable_noeeprom();
            uprintf("rgb_matrix_enable_noeeprom=3\n");
            switch (keycode){
                case BT_TN1: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 1);break;
                case BT_TN2: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 2);break;
                case BT_TN3: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 3);break;
                case BT_TN4: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 4);break;
                case BT_TN5: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 5);break;
                // case BT_TN6: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 6);break;
                // case BT_TN7: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 7);break;
                // case BT_TN8: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 8);break;
                case BT_STATE:
                    rgb_matrix_indicator = BLE_LED_KEY_ALL;
                    analogg_ble_send_cmd(DATA_TYPE_STATE);break;
                default:break;
            }
        }
        return process_record_user(keycode, record);
    }
    if (IS_ANY(keycode)){
        key_pressed_rgb_enabled();
        rgb_matrix_indicator = RGB_MATRIX_ANIMATION;
        push_cmd(DATA_TYPE_DEFAULT_KEY,keycode,record->event.pressed);
        return false;
    }else{
        return process_record_user(keycode, record);
    }
}

bool uart_tx_data_handle(void){
    if (ble_send_state==TX_IDLE && bufferPop(&pop_protocol_cmd)){
        ble_send_state=TX_START;
        if (pop_protocol_cmd.type==DATA_TYPE_DEFAULT_KEY){
            analogg_ble_keycode_handle(pop_protocol_cmd);
        }else{
            analogg_ble_config_handle(pop_protocol_cmd);
        }
        return true;
    }
    return false;
}

void matrix_scan_kb(void) {
    if (!readPin(IS_CHRG)){
        isChrg = true;
        chrgCount++;
    }else{
        isChrg = false;
        chrgCount=0;
    }
    while (!sdGetWouldBlock(&SD1)) {
        analogg_ble_resolve_protocol(sdGet(&SD1));
    }
 
    /**********RGB_DISABLE_WHEN_USB_SUSPENDED***********/
    if (USB_DRIVER.state==USB_SUSPENDED){
        is_usb_suspended = true;
        if(!readPin(IS_BLE) && is_rgb_enabled){
            if (!isChrg){
                rgb_matrix_disable_noeeprom();
            }
        }
    }else if (USB_DRIVER.state==USB_ACTIVE){
    	if (is_usb_suspended){
            if(is_rgb_enabled){
                rgb_matrix_enable_noeeprom();
            }else{
                rgb_matrix_disable_noeeprom();
            }
            rgb_sleep_time = 0;
            uprintf("rgb_matrix_enable_noeeprom=1\n");
            is_usb_suspended = false;
        }
    }

    matrix_scan_user();
}

#ifdef DIP_SWITCH_MATRIX_GRID
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) { return false;}
    if (index == 0) {
        default_layer_set(1UL << (active ? 2 : 0));
    }
    return true;
}
#endif

#ifdef ENCODER_ENABLE
bool encoder_update_kb(uint8_t index, bool clockwise) {
    if (!encoder_update_user(index, clockwise)) { return false; }
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    return true;
}
#endif

#ifdef RGB_MATRIX_ENABLE
const is31_led PROGMEM g_is31_leds[RGB_MATRIX_LED_COUNT] = {
/* Refer to IS31 manual for these locations
 *   driver
 *   |  G location
 *   |  |       R location
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
		{  0,       1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,      13,     14      },
		{  15,     16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,      27,     28,     29      },
		{  30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,      42,     43,     44      },
		{  45,     46,     47,     48,     49,     50,     51,     52,     53,     54,     55,     56,      57,     NO_LED, 59      },
		{  60,     61,     62,     63,     64,     65,     66,     67,     68,     69,     70,     71,      NO_LED, 73,     NO_LED  },
		{  75,     76,     77,     NO_LED, NO_LED, NO_LED, 81,     NO_LED, NO_LED, 84,     85,     86,      87,     88,     89      }
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
		4,	  4,	4,	  4,    4,	  4,    4,    4,    4,    4,	4,	  4,    4,	  4,    4,
		4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,    4,
		4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,    4,
		4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,    4,
		4,	  4,    4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,	  4,	4,    4,    4,
		4,	  4,	4,	  4,    4,	  4,    4,	  4,	4,    4,    4,    4,	4,    4,	4,

	}
};
#endif
