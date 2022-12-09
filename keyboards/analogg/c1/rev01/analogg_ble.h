// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"
#include "analogg_ble_protocol.h"

#ifndef BLE_TUNNEL_NUM
#   define BLE_TUNNEL_NUM        5
#endif

/* --------------------------------- GPIO ------------------------------------ */
#ifndef BLE_RST
#    define BLE_RST         A2
#endif

#ifndef IS_BLE
#   define IS_BLE            B8
#endif
#ifndef IS_CHRG
#   define IS_CHRG             A6  // 1 = full 0 change  1 0  no battery
#endif
#ifndef PIO11_WAKEUP
#   define PIO11_WAKEUP        A5  //input(wakeup control): use it with (AT+SLEEPMODE!=0)
#endif

#define EE_ANALOGG_LINK_ID  (uint8_t *)128

extern uint8_t tunnel;
extern uint8_t last_save_tunnel;

extern bool    is_rgb_enabled;

typedef enum {
    TX_IDLE     = 0,
    TX_START    = 1,
    TX_TIMEOUT  = 100,
    TX_RESTART  = 255
} _ble_send_state;
extern _ble_send_state ble_send_state;

typedef enum {
    INPUT_MODE = 0,
    WAIT_INPUT_MODE,
    CONFIG_MODE,
    WAIT_CONFIG_MODE
} _ble_work_state;
extern _ble_work_state ble_work_state;

typedef enum {
    IDLE = 0,
    ADV_WAIT_CONNECTING,
    ADV_WAIT_CONNECTING_ACTIVE,
    ADV_WAIT_CONNECTING_INACTIVE,
    CONNECTED,
    CONNECTED_AND_ACTIVE,
} ble_state;

typedef struct{
    uint8_t     tunnel;
    ble_state   list[BLE_TUNNEL_NUM];
}_ble_tunnel_state;
extern _ble_tunnel_state ble_tunnel_state;

_ble_work_state is_ble_work_state(void);
bool is_ble_work_state_input(void);
void set_ble_work_state(_ble_work_state state);

void analogg_ble_send_cmd(uint8_t type);
void analogg_ble_send_cmd_by_id(uint8_t type, uint8_t tunnel_id);
void analogg_ble_send_cmd_by_val(uint8_t type, uint8_t val);

void analogg_ble_reset_leds(void);
void analogg_ble_keyboard(void);
void analogg_ble_mouse(void);
void analogg_ble_system(void);
void analogg_ble_consumer(void);

void analogg_ble_startup(void);
void analogg_ble_disconnect(void);
void analogg_ble_stop(void);
