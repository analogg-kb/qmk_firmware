/*
 * Copyright (c) 2018 Yaotian Feng
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "quantum.h"

#define xxx KC_NO

enum layers{
    WIN_BASE,
    WIN_FN,
    MAC_BASE,
    MAC_FN,          
};


// KEYCODES
enum keyboard_keycodes   {
    #ifdef VIA_ENABLE
        BT_TUNNEL1 = USER00,
    #else
        BT_TUNNEL1 = SAFE_RANGE,
    #endif
    BT_TUNNEL2,     
    BT_TUNNEL3,       
    BT_TUNNEL4,       
    BT_TUNNEL5,    
    // BT_TUNNEL6,    
    // BT_TUNNEL7,    
    // BT_TUNNEL8,    
    BT_STATE,  
    BT_DEFAULT,
};

#define  BT_TN1   BT_TUNNEL1
#define  BT_TN2   BT_TUNNEL2
#define  BT_TN3   BT_TUNNEL3
#define  BT_TN4   BT_TUNNEL4
#define  BT_TN5   BT_TUNNEL5
// #define  BT_TN6   BT_TUNNEL6
// #define  BT_TN7   BT_TUNNEL7
// #define  BT_TN8   BT_TUNNEL8
#define  BT_FTY   BT_DEFAULT

/** battery
    25%=3.72(1.86)  %50=3.79(1.895)  75%=3.93(1.965)  100%=4.2(2.1)
    x = (4.2/2)/3.3 * 1024 = 652
*/
#define BATTERY_RSOC_0              555
#define BATTERY_RSOC_AREA           97.00f

#define TIMER_BASE_TIME             10  // TIMER_BASE_TIME
#define BLE_INDICATOR_100MS         10  // TIMER_DELAY*BLE_INDICATOR_100MS
#define BLE_INDICATOR_200MS         20  // TIMER_DELAY*BLE_TUNNEL_LED_250MS
#define BLE_INDICATOR_400MS         40  // TIMER_DELAY*BLE_TUNNEL_LED_500MS
#define BLE_INDICATOR_1S            100 // TIMER_DELAY*BLE_TUNNEL_LED_1S
#define BLE_INDICATOR_2S            200 // TIMER_DELAY*BLE_TUNNEL_LED_2S

#define LONG_PRESSED_TIME           3000


typedef enum {
    BLE_LED_KEY_ONE= 0,
    BLE_LED_KEY_ALL,  
    BLE_LED_INDICATOR, 
} _ble_state_led;
extern _ble_state_led ble_state_led;

void pressed_timeout_turn_off_led(void);
void pressed_turn_on_led(void);

bool send(SerialDriver *sdp, const uint8_t* source, const size_t size);
bool receive(SerialDriver *sdp, uint8_t* destination, const size_t size);

uint32_t my_callback(uint32_t trigger_time, void *cb_arg);
void uart_rx_data_handle(uint8_t byte);
bool uart_tx_data_handle(void);
void long_pressed_event(void);
/*
#define LAYOUT_82_ansi( \
        K00, K01,  K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K0E, \
        K0F, K10,  K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, \
        K1E, K1F,  K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C, \
        K2D, K2E,  K2F, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39,      K3B, \
        K3C, K3D,  K3E, K3F, K40, K41, K42, K43, K44, K45, K46, K47,      K49,      \
        K4B, K4D,  K4E,                K52,           K55, K56, K57, K58, K59, K5A  \
    ) { \
        {K00, K01,  K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K0E}, \
        {K0F, K10,  K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D}, \
        {K1E, K1F,  K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C}, \
        {K2D, K2E,  K2F, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, xxx, K3B}, \
        {K3C, K3D,  K3E, K3F, K40, K41, K42, K43, K44, K45, K46, K47, xxx, K49, xxx}, \
        {K4B, K4D,  K4E, xxx, xxx, xxx, K52, xxx, xxx, K55, K56, K57, K58, K59, K5A}  \
    }

#define LAYOUT_all( \
        K00, K01,  K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K4F, K0E, K50, \
        K0F, K10,  K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, \
        K1E, K1F,  K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C, \
        K2D, K2E,  K2F, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39,      K3B, \
        K3C, K3D,  K3E, K3F, K40, K41, K42, K43, K44, K45, K46, K47,      K49,      \
        K4B, K4D,  K4E,                K52,           K55, K56, K57, K58, K59, K5A  \
    ) { \
        {K00, K01,  K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K4F, K0E, K50},\
        {K0F, K10,  K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D}, \
        {K1E, K1F,  K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C}, \
        {K2D, K2E,  K2F, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, xxx, K3B}, \
        {K3C, K3D,  K3E, K3F, K40, K41, K42, K43, K44, K45, K46, K47, xxx, K49, xxx}, \
        {K4B, K4D,  K4E, xxx, xxx, xxx, K52, xxx, xxx, K55, K56, K57, K58, K59, K5A}  \
    }
*/

