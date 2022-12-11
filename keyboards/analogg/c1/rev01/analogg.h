// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

#define LOG_Q_DEBUGF(fmt, args...) dprintf("%05d Q " #fmt "\n", log_time, ##args)
#define LOG_B_DEBUGF(fmt, args...) dprintf("%05d B " #fmt "\n", log_time, ##args)
#define LOG_Q_INFO(fmt, args...) uprintf("%05d Q " #fmt "\n", log_time, ##args)
#define LOG_B_INFO(fmt, args...) uprintf("%05d B " #fmt "\n", log_time, ##args)


// KEYCODES
enum keyboard_keycodes {
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

#define BT_TN1 BT_TUNNEL1
#define BT_TN2 BT_TUNNEL2
#define BT_TN3 BT_TUNNEL3
#define BT_TN4 BT_TUNNEL4
#define BT_TN5 BT_TUNNEL5
// #define  BT_TN6   BT_TUNNEL6
// #define  BT_TN7   BT_TUNNEL7
// #define  BT_TN8   BT_TUNNEL8
#define BT_FTY BT_DEFAULT

/** battery
    25%=3.72(1.86)  %50=3.79(1.895)  75%=3.93(1.965)  100%=4.2(2.1)
    x = (4.2/2)/3.3 * 1024 = 652
*/
#define BATTERY_RSOC_0 555
#define BATTERY_RSOC_AREA 97.00f

#define TIMER_BASE_TIME 10 // TIMER_BASE_TIME
#define T100MS 10          // TIMER_DELAY*BLE_INDICATOR_100MS
#define T200MS 20          // TIMER_DELAY*BLE_TUNNEL_LED_250MS
#define T400MS 40          // TIMER_DELAY*BLE_TUNNEL_LED_500MS
#define T1S 100            // TIMER_DELAY*BLE_TUNNEL_LED_1S
#define T2S 200            // TIMER_DELAY*BLE_TUNNEL_LED_2S

#define LONG_PRESSED_TIME 3000

extern uint16_t log_time;
void pressed_timeout_turn_off_led(void);
void key_pressed_rgb_enabled(void);

void uart_rx_data_handle(uint8_t byte);
bool uart_tx_data_handle(void);
void long_pressed_event(void);

uint16_t get_timer_count(void);
