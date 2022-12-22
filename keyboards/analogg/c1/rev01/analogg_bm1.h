// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include QMK_KEYBOARD_H

#include <stdint.h>
#include <report.h>
#include "quantum.h"

// #define EE_ANALOGG_LINK_ID (uint8_t *)128

#ifndef BLE_TUNNEL_NUM
#    define BLE_TUNNEL_NUM 5
#endif

#ifndef BLE_RST
#    define BLE_RST A2
#endif

#ifndef IS_BLE_PIN
#    define IS_BLE_PIN B8
#endif
#define IS_BLE_DIP_ON() (readPin(IS_BLE_PIN) != 0)
#define IS_USB_DIP_ON() (readPin(IS_BLE_PIN) == 0)

#ifndef PIO11_WAKEUP
#    define PIO11_WAKEUP A5    // input(wakeup control): use it with (AT+SLEEPMODE!=0)
#endif

// DATA DIRECTION
#define DATA_DIRECTION_QMK_BLE 0x09
#define DATA_DIRECTION_BLE_QMK 0x89

// PROTOCOL VERSION
#define PROTOCOL_VERSION 0x01

// DATA TYPE
#define DATA_TYPE_DEFAULT_KEY 0x00
#define DATA_TYPE_KEY 0x01
#define DATA_TYPE_CONSUMER_KEY 0x02
#define DATA_TYPE_SYSTEM_CONTROL 0x03
#define DATA_TYPE_BATTERY_LEVEL 0x04
#define DATA_TYPE_STATE 0x10
#define DATA_TYPE_GET_TUNNEL_ID 0x11
#define DATA_TYPE_SWITCH 0x12
#define DATA_TYPE_GET_AUTH 0x13
#define DATA_TYPE_SET_AUTH 0x14
#define DATA_TYPE_GET_PSWD 0x15
#define DATA_TYPE_SET_PSWD 0x16
#define DATA_TYPE_GET_NAME 0x17
#define DATA_TYPE_SET_NAME 0x18
#define DATA_TYPE_PAIR 0x19
#define DATA_TYPE_UNPLUG 0x1A
#define DATA_TYPE_GET_BAUD 0x50
#define DATA_TYPE_SET_BAUD 0x51
#define DATA_TYPE_HD_VERSION 0x52
#define DATA_TYPE_DFU 0x53
#define DATA_TYPE_RESET 0x54
#define DATA_TYPE_DEFAULT 0x55
#define DATA_TYPE_GET_SLEEP 0x56
#define DATA_TYPE_SET_SLEEP 0x57

#define PROTOCOL_BUFFER_SIZE 512
#define CMD_BUFFER_SIZE 160

typedef struct {
    uint8_t  type;
    uint16_t value;
    bool     pressed;
} protocol_cmd;

typedef enum { TX_IDLE = 0, TX_START = 1, TX_TIMEOUT = 100 } _ble_send_state;

typedef enum { INPUT_MODE = 0, WAIT_INPUT_MODE, CONFIG_MODE, WAIT_CONFIG_MODE } _ble_work_state;

typedef enum {
    IDLE = 0,
    ADV_WAIT_CONNECTING,
    ADV_WAIT_CONNECTING_ACTIVE,
    ADV_WAIT_CONNECTING_INACTIVE,
    CONNECTED,
    CONNECTED_AND_ACTIVE,
} ble_state;

typedef struct {
    uint8_t   activity_tunnel;
    ble_state list[BLE_TUNNEL_NUM];
} _ble_tunnel_state;

bool            ble_send(SerialDriver *sdp, const uint8_t *source, const size_t size);
uint8_t         get_op_tunnel(void);
uint8_t         get_activity_tunnel(void);
_ble_work_state is_ble_work_state(void);
uint16_t        get_ble_send_state(void);
void            ble_send_state_tick(void);
void            set_ble_send_state(_ble_send_state state);

bool            is_ble_work_state_input(void);
void            set_ble_work_state(_ble_work_state state);
uint8_t         get_ble_activity_tunnel_state(void);
uint8_t         get_ble_tunnel_state_to(uint8_t tunnel);

void            analogg_ble_send_key(uint8_t type, uint16_t keycode, bool pressed);
void            analogg_ble_send_cmd(uint8_t type);
void            analogg_ble_send_cmd_by_id(uint8_t type, uint8_t tunnel_id);
void            analogg_ble_send_cmd_by_val(uint8_t type, uint8_t val);

bool            is_tx_idle(void);
uint16_t        get_buffer_size(void);
void            bm1_clear_buffer(void);
uint8_t         bufferPop(protocol_cmd *_buf);
void            bm1_reset(void);

bool            analogg_ble_cmd_handle(void);
void            analogg_ble_cmd_handle_timeout(void);
void            analogg_ble_resolve_protocol(uint8_t byte);
void            analogg_ble_cmd_tx(uint8_t seqId);
bool            analogg_ble_config_handle(protocol_cmd _protocol_cmd);
bool            analogg_ble_keycode_handle(protocol_cmd _protocol_cmd);

void            analogg_bm1_init(void);
void            analogg_bm1_task(void);
void            analogg_bm1_send_keyboard(report_keyboard_t *report);
void            analogg_bm1_send_mouse(report_mouse_t *report);
void            analogg_bm1_send_consumer(uint16_t usage);