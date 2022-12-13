// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include QMK_KEYBOARD_H

void rgb_init(void);
void rgb_sleep_wakeup(void);
void rgb_sleep_sleep(void);
void rgb_sleep_activity(void);
void rgb_sleep_tick(uint16_t ms);
void rgb_ble_tick(uint16_t ms);
void rgb_ble_indicator_single_tunnel(uint8_t tunnel);
void rgb_ble_indicator_show_all(void);
void rgb_ble_indicator_exit(void);
bool rgb_sleep_is_sleep(void);