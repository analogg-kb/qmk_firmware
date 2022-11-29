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

// DATA DIRECTION
#define DATA_DIRECTION_QMK_BLE      0x09 
#define DATA_DIRECTION_BLE_QMK      0x89 

//PROTOCOL VERSION
#define PROTOCOL_VERSION            0x01   

//DATA TYPE
#define DATA_TYPE_DEFAULT_KEY       0x00
#define DATA_TYPE_KEY               0x01
#define DATA_TYPE_CONSUMER_KEY      0x02
#define DATA_TYPE_SYSTEM_CONTROL    0x03
#define DATA_TYPE_BATTERY_LEVEL     0x04
#define DATA_TYPE_STATE             0x10
#define DATA_TYPE_GET_TUNNEL_ID     0x11
#define DATA_TYPE_SWITCH            0x12
#define DATA_TYPE_GET_AUTH          0x13
#define DATA_TYPE_SET_AUTH          0x14
#define DATA_TYPE_GET_PSWD          0x15
#define DATA_TYPE_SET_PSWD          0x16
#define DATA_TYPE_GET_NAME          0x17
#define DATA_TYPE_SET_NAME          0x18
#define DATA_TYPE_PAIR              0x19
#define DATA_TYPE_UNPLUG            0x1A
#define DATA_TYPE_GET_BAUD          0x50
#define DATA_TYPE_SET_BAUD          0x51
#define DATA_TYPE_HD_VERSION        0x52
#define DATA_TYPE_DFU               0x53
#define DATA_TYPE_RESET             0x54
#define DATA_TYPE_DEFAULT           0x55
#define DATA_TYPE_GET_SLEEP         0x56
#define DATA_TYPE_SET_SLEEP         0x57

#define PROTOCOL_BUFFER_SIZE        512

volatile uint8_t mSeqId;  //0-255
extern uint8_t ble_protocol_payload_cmd[1];

typedef struct{
    uint8_t  type;
    uint16_t value;
    bool pressed;
}protocol_cmd;

uint8_t memsets(uint8_t *buff,int value,int len);

bool is_tx_idle(void);
uint8_t bufferPop(protocol_cmd *_buf);
void bufferPush(protocol_cmd _buf);
uint16_t get_buffer_size(void);
void resetGetData(void);
void clear_buffer(void);
void push_cmd(uint8_t type, uint16_t keycode, bool pressed);

void general_protocol_array_of_byte(uint8_t dataType, uint8_t dataSize, uint8_t *bleData);
void analogg_ble_resolve_protocol(uint8_t byte);
void analogg_ble_cmd_tx(uint8_t seqId);

bool analogg_ble_config_handle(protocol_cmd _protocol_cmd);
bool analogg_ble_keycode_handle(protocol_cmd _protocol_cmd);
