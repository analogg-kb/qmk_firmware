// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

#ifdef CONSOLE_ENABLE
#    define LOG_Q_DEBUG(fmt, args...) dprintf("%05d Q " fmt "\n", log_time, ##args)
#    define LOG_B_DEBUG(fmt, args...) dprintf("%05d B " fmt "\n", log_time, ##args)
#    define LOG_Q_INFO(fmt, args...) uprintf("%05d Q " fmt "\n", log_time, ##args)
#    define LOG_B_INFO(fmt, args...) uprintf("%05d B " fmt "\n", log_time, ##args)
#else
#    define LOG_Q_DEBUG(fmt, args...)
#    define LOG_B_DEBUG(fmt, args...)
#    define LOG_Q_INFO(fmt, args...)
#    define LOG_B_INFO(fmt, args...)
#endif // CONSOLE_ENABLE

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

#define TIMER_BASE_TIME 10 // TIMER_BASE_TIME
#define T50MS 5            // 50MS = T100MS*TIMER_BASE_TIME
#define T100MS 10          // 100MS
#define T200MS 20          // 200MS
#define T400MS 40          // 400MS
#define T1S 100            // 1S
#define T2S 200            // 2S

#define LONG_PRESSED_TIME 3000

extern uint16_t log_time;
void pressed_timeout_turn_off_led(void);
void key_pressed_rgb_enabled(void);

void uart_rx_data_handle(uint8_t byte);
bool uart_tx_data_handle(void);
void long_pressed_event(void);

uint16_t get_timer_count(void);
