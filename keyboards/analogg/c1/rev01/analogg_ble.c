// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "analogg.h"
#include "analogg_ble.h"
#include "analogg_ble_protocol.h"
#include "eeprom.h"
#include "rgb.h"

uint8_t tunnel           = 1;
uint8_t last_save_tunnel = 1;

_ble_work_state   ble_work_state = INPUT_MODE;
_ble_send_state   ble_send_state = TX_IDLE;
_ble_tunnel_state ble_tunnel_state;

void set_ble_work_state(_ble_work_state state) {
    ble_work_state = state;
}

_ble_work_state is_ble_work_state(void) {
    return ble_work_state;
}

bool is_ble_work_state_input(void) {
    if (ble_work_state == INPUT_MODE || ble_work_state == WAIT_INPUT_MODE) {
        return true;
    } else {
        return false;
    }
}

/* -------------------- Public Function Implementation ---------------------- */
void analogg_ble_stop(void) {
    ble_send_state = IDLE;
}

void analogg_ble_send_cmd(uint8_t type) {
    push_cmd(type, 0, false);
}

void analogg_ble_send_cmd_by_id(uint8_t type, uint8_t tunnel_id) {
    rgb_matrix_indicator = BLE_LED_KEY_ONE;
    tunnel               = tunnel_id;
    push_cmd(type, tunnel_id, false);
}

void analogg_ble_send_cmd_by_val(uint8_t type, uint8_t val) {
    push_cmd(type, val, false);
}

void analogg_ble_disconnect(void) {
    clear_keyboard();
}

void analogg_ble_reset_leds() {
    if (is_ble_work_state() == WAIT_CONFIG_MODE) {
        for (uint8_t i = 0; i < BLE_TUNNEL_NUM; i++)
            ble_tunnel_state.list[i] = IDLE; // reset leds state
    }
}
