// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include QMK_KEYBOARD_H

#ifndef IS_CHRG_PIN
#    define IS_CHRG_PIN A6    // 1 = full 0 change  1 0  no battery
#endif

#ifndef BATTERY_LEVEL_PIN
#    define BATTERY_LEVEL_PIN A4 
#endif

#define BATTERY_RSOC_0      555
#define BATTERY_RSOC_AREA   97.00f
#define UPDATE_BATTERY_TIME 600  //60S    

#define IS_CHARGE_STATE() (readPin(IS_CHRG_PIN) == 0)
#define IS_UNCHARGED() (readPin(IS_CHRG_PIN) != 0)

void charge_task_scan_state_and_tick(void);
void charge_task_delay_debounce_tick(void);
bool charge_task_is_charging(void);

uint8_t battery_query_level(void);
