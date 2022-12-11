// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include QMK_KEYBOARD_H

typedef enum {
    BLE_LED_KEY_ONE = 0,
    BLE_LED_KEY_ALL,
    RGB_MATRIX_ANIMATION,
} _rgb_matrix_indicator;

extern _rgb_matrix_indicator rgb_matrix_indicator;

void rgb_init(void);
void rgb_wakeup(void);
void rgb_sleep(void);
void rgb_key_press_task(void);
void rgb_sleep_timer_task(void);
