// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "battery_manager.h"
#include "analog.h"
#include "analogg.h"
#include "analogg_bm1.h"

#define CHARGE_DEBOUNCE_TICK_VAL             30
#define DEBOUNCE_TIMEOUT                     100
#define DEBOUNCE_TIMEOUT_BATTERY_LEVEL       100

typedef struct {
    bool is_charging;
    uint16_t debounce_timeout;
    uint16_t debounce_tick;
} _charge_info;

_charge_info charge_info = {false,0,0};

typedef struct {
    bool is_timeout;
    uint8_t value;
    uint8_t last_value;
    uint32_t sum;
    uint16_t debounce_tick;
} _battery_level;

_battery_level battery_level = {false,0,0,0};

bool battery_level_task_delay_debounce_tick(uint8_t value) {

    if (battery_level.value == 0) {
        battery_level.value = value;    //first
        return true;
    }
    
    battery_level.debounce_tick++;
    battery_level.sum += value;
    if (battery_level.debounce_tick >= DEBOUNCE_TIMEOUT_BATTERY_LEVEL) {
        battery_level.value = battery_level.sum / battery_level.debounce_tick;
        if (battery_level.last_value != battery_level.value) {
            battery_level.last_value = battery_level.value;
        }
        
        if (!charge_info.is_charging && battery_level.value > battery_level.last_value) {
            battery_level.value = battery_level.last_value;
        } 
        
        // LOG_Q_INFO("battery level a=%d  b=%d,", value, battery_level.value); 

        battery_level.sum = 0;
        battery_level.debounce_tick = 0;
        return true;
    } 
    return false;
}

uint8_t get_average_battery_level(void){
    return battery_level.value;
}

void charge_task_scan_state_and_tick(void) {
    if (IS_CHARGE_STATE()) {
        charge_info.debounce_tick++;
    } else {
        charge_info.is_charging = false;
        charge_info.debounce_tick = 0; 
    }
}

// TODO: Led1 should turn off when the usb is unplugged
void charge_task_delay_debounce_tick(void) {
    // Ensure that the charging light is on for more than 30 times
    if (charge_info.debounce_tick > CHARGE_DEBOUNCE_TICK_VAL) {
        charge_info.debounce_timeout++;
        if (charge_info.debounce_timeout > DEBOUNCE_TIMEOUT){
            charge_info.debounce_timeout = 0;
            charge_info.is_charging = true;
        }
    } else {
        charge_info.debounce_timeout = 0;
    }
    charge_info.debounce_tick = 0; 
}

bool charge_task_is_charging(void){
    return charge_info.is_charging;
}

uint8_t battery_query_level(void) {
    uint16_t bl = analogReadPin(BATTERY_LEVEL_PIN);
    uint8_t chrg_rsoc = (uint8_t)(((float)(bl - BATTERY_RSOC_0) / BATTERY_RSOC_AREA) * 100);
    if (chrg_rsoc > 100) {
        chrg_rsoc = 100;
    }
    if (chrg_rsoc < 0) {
        chrg_rsoc = 0;
    }

    return chrg_rsoc;
}