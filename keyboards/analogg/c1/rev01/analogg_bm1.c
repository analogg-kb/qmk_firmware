// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ctype.h>
#include "analogg_bm1.h"
#include "analogg.h"
#include "eeprom.h"
#include "analog.h"
#include "rgb.h"

uint8_t tunnel           = 1;
uint8_t last_save_tunnel = 1;

_ble_work_state   ble_work_state = INPUT_MODE;
_ble_send_state   ble_send_state = TX_IDLE;
_ble_tunnel_state ble_tunnel_state;

static const SerialConfig ble_uart_config = {
    .speed = 115200 // 921600
};


bool is_kb_startup = false;

void bm1_clear_buffer(void);

bool ble_send(SerialDriver *sdp, const uint8_t *source, const size_t size) {
    bool success = (size_t)sdWriteTimeout(sdp, source, size, TIME_MS2I(100)) == size;
    return success;
}

// bool ble_receive(SerialDriver *sdp, uint8_t* destination, const size_t size) {
//     bool success = (size_t)sdReadTimeout(sdp, destination, size, TIME_MS2I(100)) == size;
//     return success;
// }

void analogg_bm1_init(void) {
#ifdef PIO11_WAKEUP
    setPinOutput(PIO11_WAKEUP);
    writePinLow(PIO11_WAKEUP);
#endif

#ifdef BLE_RST
    setPinOutput(BLE_RST);
    writePinHigh(BLE_RST); // reset the ble moudle
    wait_ms(100);
    writePinLow(BLE_RST);
#endif
    // Start BLE UART
    sdStart(&SD1, &ble_uart_config);
    // Give the send uart thread some time to
    // send out the queue before we read back
    wait_ms(100);
    // loop to clear out receive buffer from ble wakeup
    while (!sdGetWouldBlock(&SD1))
        sdGet(&SD1);
}

void analogg_bm1_task(void) {}

void analogg_bm1_send_keyboard(report_keyboard_t *report) {}

void analogg_bm1_send_mouse(report_mouse_t *report) {}

void analogg_bm1_send_consumer(uint16_t usage) {}


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

unsigned char memsets(unsigned char *buff, int value, int len) {
    while (len) {
        *buff = value;
        buff++;
        len--;
    }
    return TRUE;
}

void push_cmd(uint8_t type, uint16_t keycode, bool pressed) {
    if (type == DATA_TYPE_DEFAULT_KEY) {
        if (!is_ble_work_state_input()) {
            set_ble_work_state(WAIT_INPUT_MODE);
            bm1_clear_buffer();
        }
    } else {
        if (type != DATA_TYPE_STATE && is_ble_work_state() == INPUT_MODE) {
            set_ble_work_state(WAIT_CONFIG_MODE);
        }
    }

    protocol_cmd protocol_cmd_new = {type, keycode, pressed};
    bufferPush(protocol_cmd_new);
}


void bm1_reset(void) {
    bm1_clear_buffer();
    ble_send_state = TX_IDLE;
}

/**
 * @param dataType
 * type
    0x01：Key
    0x02：Consumer key
    0x03：System Control
    0x04：Battery level
    0x05：AT mode
 */
uint8_t cmdDataBuffer[160];
uint8_t cmdDataBufferSize = 8;
void    general_protocol_array_of_byte(uint8_t dataType, uint8_t dataSize, uint8_t *bleData) {
    uint8_t offset = 0, sum = 0;
    cmdDataBufferSize = 8;
    if (bleData != NULL) {
        cmdDataBufferSize += dataSize;
    }
    cmdDataBuffer[offset++] = 0x00;
    cmdDataBuffer[offset++] = 0x55;
    cmdDataBuffer[offset++] = DATA_DIRECTION_QMK_BLE;
    cmdDataBuffer[offset++] = dataType;
    cmdDataBuffer[offset++] = PROTOCOL_VERSION;
    mSeqId++;
    cmdDataBuffer[offset++] = mSeqId;
    cmdDataBuffer[offset++] = dataSize;
    uint8_t temp            = 0;
    while (dataSize--) {
        temp                    = *bleData++;
        cmdDataBuffer[offset++] = temp;
    }
    for (uint8_t i = 0; i < offset; i++)
        sum += cmdDataBuffer[i];
    cmdDataBuffer[offset++] = sum;
    analogg_ble_cmd_tx(mSeqId);
}

void analogg_ble_cmd_tx(uint8_t seqId) {
    if (cmdDataBufferSize < 8) return;
    ble_send_state = TX_START;

    uint8_t sum = 0, size = cmdDataBufferSize - 1;
    cmdDataBuffer[5] = seqId;
    for (uint8_t i = 0; i < size; i++) {
        sum += cmdDataBuffer[i];
    }
    cmdDataBuffer[size] = sum;
    ble_send(&SD1, cmdDataBuffer, cmdDataBufferSize);
    // sdWrite(&SD1, cmdDataBuffer, cmdDataBufferSize);
}

#define INDEX_RESET -1
#define PROTOCOL_DATA_SIZE 32
#define DATA_PARSE_MAX 255
uint8_t pos      = INDEX_RESET;
uint8_t dataLen  = 0;
uint8_t checkSum = 0;
uint8_t protocolData[PROTOCOL_DATA_SIZE];

#define LOG_SIZE 128
uint8_t log_buffer[LOG_SIZE];

void resetGetData(void) {
    dataLen  = 0;
    checkSum = 0;
    memsets(protocolData, 0, PROTOCOL_DATA_SIZE);
}

bool protocol_handle(uint8_t data_package[], uint8_t size) {
    uint8_t type = 0, version = 0, seqId = 0, dataLen = 0, errCode = 0;
    type    = data_package[3];
    version = data_package[4];
    seqId   = data_package[5];
    dataLen = data_package[6];
    if (dataLen == 0 || version != PROTOCOL_VERSION) return false;
    errCode = data_package[7];

    // LOG_Q_DEBUG("mId=%d Id=%d err=%d %d",mSeqId,seqId,errCode,timer_read());
    if (mSeqId != seqId || errCode != 0) {
        LOG_Q_INFO("errCode=%d\n", errCode);
        ble_send_state = TX_START;
        return false;
    }

    if (type == DATA_TYPE_KEY || type == DATA_TYPE_CONSUMER_KEY || type == DATA_TYPE_SYSTEM_CONTROL) {
        // if (is_ble_work_state()==CONFIG_MODE){
        // 	rgb_matrix_enable_noeeprom();            // Turn on the rgb light
        // }

        // else{
        // 	if (is_rgb_enabled){
        // 		rgb_matrix_enable_noeeprom();
        // 	}else{
        // 		rgb_matrix_disable_noeeprom();
        // 	}
        // }

        set_ble_work_state(INPUT_MODE);

        ble_send_state = TX_IDLE;
        return true;
    }

    switch (type) {
        case DATA_TYPE_BATTERY_LEVEL:
            break;
        case DATA_TYPE_STATE:
            /**
             * @brief
             * dataLen = 0x0A  ble state
             * dataLen > 0x0A  ble state + log
             */
            if (IS_BLE_DIP_ON()) {
                if (!is_kb_startup) {
                    is_kb_startup  = true;
                    uint8_t tunnel = eeprom_read_byte(EE_ANALOGG_LINK_ID);
                    if (tunnel == 0 || tunnel > BLE_TUNNEL_NUM) {
                        tunnel = 1;
                    }
                    analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, tunnel);
                }
            }

            ble_tunnel_state.current_tunnel = data_package[8];
            for (uint8_t i = 0; i < BLE_TUNNEL_NUM; i++) {
                ble_tunnel_state.list[i] = data_package[9 + i];
            }

            // log: indexstart=17
            if (dataLen > 0x0a) {
                uint8_t logSize = dataLen - 0x0a;
                if (logSize > LOG_SIZE) {
                    logSize = LOG_SIZE;
                }
                memsets(log_buffer, 0, LOG_SIZE);
                for (uint8_t i = 0; i < logSize; i++) {
                    log_buffer[i] = data_package[17 + i];
                }
                LOG_B_INFO("%s", log_buffer);
            }

            ble_send_state = TX_IDLE;
            return true;
        case DATA_TYPE_GET_TUNNEL_ID:
            break;
        case DATA_TYPE_SWITCH:
            analogg_ble_send_cmd(DATA_TYPE_STATE);
            break;
        case DATA_TYPE_GET_AUTH:
            break;
        case DATA_TYPE_SET_AUTH:
            break;
        case DATA_TYPE_GET_PSWD:
            break;
        case DATA_TYPE_SET_PSWD:
            break;
        case DATA_TYPE_GET_NAME:
            break;
        case DATA_TYPE_SET_NAME:
            break;
        case DATA_TYPE_PAIR:
            analogg_ble_send_cmd(DATA_TYPE_STATE);
            break;
        case DATA_TYPE_UNPLUG:
            analogg_ble_send_cmd_by_id(DATA_TYPE_PAIR, tunnel);
            break;
        case DATA_TYPE_GET_BAUD:
            break;
        case DATA_TYPE_SET_BAUD:
            break;
        case DATA_TYPE_HD_VERSION:
            break;
        case DATA_TYPE_DFU:
            break;
        case DATA_TYPE_RESET:
            break;
        case DATA_TYPE_DEFAULT:
            break;
        case DATA_TYPE_GET_SLEEP:
            break;
        case DATA_TYPE_SET_SLEEP:
            break;
        default:
            break;
    }
    set_ble_work_state(CONFIG_MODE);
    ble_send_state = TX_IDLE;
    return true;
}

void analogg_ble_resolve_protocol(uint8_t byte) {
    if (pos == INDEX_RESET) {
        resetGetData();
    }
    pos++;
    protocolData[pos] = byte;
    if (pos == 0) {
        if (byte != 0x00) pos = INDEX_RESET;
    } else if (pos == 1) {
        if (byte != 0x55) pos = INDEX_RESET;
    } else if (pos == 2) {
        if (byte != DATA_DIRECTION_BLE_QMK) pos = INDEX_RESET; // 89
    } else if (pos == 3) {
        // type
    } else if (pos == 4) {
        // version
    } else if (pos == 5) {
        if (byte != mSeqId) {
            LOG_Q_INFO("id error,id=%02x seqId=%02x", byte, mSeqId);
            pos = INDEX_RESET; // mSeqId
        }
    } else if (pos == 6) {
        dataLen = byte;
    } else if (pos > 6 && pos < (dataLen + 7)) {
        // getdata
    } else if (pos == (dataLen + 7)) {
        checkSum = 0;
        for (uint8_t i = 0; i < pos; i++)
            checkSum += protocolData[i];
        if (byte != checkSum)
            pos = INDEX_RESET;
        else {
            pos++;
            protocol_handle(&protocolData[0], pos);
            pos = INDEX_RESET;
        }
    } else if (pos >= DATA_PARSE_MAX) {
        pos = INDEX_RESET;
    } else {
        pos = INDEX_RESET;
    }
    // LOG_Q_DEBUG("pos=%d-%02x-checkSum=%02x",pos, byte, checkSum);
}

typedef struct _circle_buffer {
    uint16_t     head_pos; // 缓冲区头部位置
    uint16_t     tail_pos; // 缓冲区尾部位置
    protocol_cmd circle_buffer[PROTOCOL_BUFFER_SIZE];
} circle_buffer;

circle_buffer buffer;
uint16_t      bufferCount = 0;

bool is_tx_idle(void) {
    if (bufferCount == 0 && ble_send_state == TX_IDLE) {
        return true;
    }

    return false;
}

uint16_t get_buffer_size(void) {
    return bufferCount;
}

void bm1_clear_buffer(void) {
    buffer.head_pos = 0;
    buffer.tail_pos = 0;
    bufferCount     = 0;
}

unsigned char bufferPop(protocol_cmd *_buf) {
    if (buffer.head_pos == buffer.tail_pos) { // 如果头尾接触表示缓冲区为空
        return FALSE;
    } else {
        *_buf = buffer.circle_buffer[buffer.head_pos]; // 如果缓冲区非空则取头节点值并偏移头节点
        if (++buffer.head_pos >= PROTOCOL_BUFFER_SIZE) buffer.head_pos = 0;
        if (bufferCount > 0) bufferCount--;
        return TRUE;
    }
}

void bufferPush(protocol_cmd _buf) {
    bufferCount++;
    if (bufferCount >= PROTOCOL_BUFFER_SIZE) {
        bufferCount = PROTOCOL_BUFFER_SIZE;
    }

    buffer.circle_buffer[buffer.tail_pos] = _buf;    // 从尾部追加
    if (++buffer.tail_pos >= PROTOCOL_BUFFER_SIZE) { // 尾节点偏移
        buffer.tail_pos = 0;                         // 大于数组最大长度 制零 形成环形队列
        if (buffer.tail_pos == buffer.head_pos) {
            if (++buffer.head_pos >= PROTOCOL_BUFFER_SIZE) {
                buffer.head_pos = 0;
            }
        } // 如果尾部节点追到头部节点 则修改头节点偏移位置丢弃早期数据
    }
}

//-------------------------------------------------------------------------------------------------------------------------
uint8_t ble_protocol_payload_cmd[1] = {0x00};

// BYTE1=[ALT、SHIFT、CTRL、GUI state], BYTE3-BYTE8=[6 keycodes]
static uint8_t  ble_protocol_key[8]            = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t  ble_protocol_consumer[4]       = {0x00, 0x00, 0x00, 0x00};
static uint8_t  ble_protocol_system[1]         = {0x00};
static uint8_t  ble_protocol_bitH0_7[8]        = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static uint16_t ble_protocol_consumer_byte5[8] = {KC_WWW_BACK, KC_WWW_FORWARD, KC_WWW_STOP, KC_WWW_REFRESH, KC_WWW_SEARCH, KC_WWW_FAVORITES, KC_WWW_HOME, KC_MAIL};

static uint16_t ble_protocol_consumer_byte6[8] = {KC_AUDIO_MUTE, KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_STOP, KC_MEDIA_PREV_TRACK, KC_MEDIA_NEXT_TRACK, KC_MEDIA_SELECT};

static uint16_t ble_protocol_consumer_byte7[8] = {AL_LOCAL_BROWSER,
                                                  KC_CALCULATOR,
                                                  AC_PROPERTIES,
                                                  0, // TODO bit3 AC_PROPERTIES = Record
                                                  KC_MEDIA_FAST_FORWARD,
                                                  KC_MEDIA_REWIND,
                                                  0,  // TODO bit6 Media_Select_Program_Guide,
                                                  0}; // TODO bit7 Microphone};

static uint16_t ble_protocol_consumer_byte8[8] = {KC_PWR,
                                                  0,
                                                  0,
                                                  0,
                                                  0, // TODO bit4 AL Screen Saver
                                                  KC_MEDIA_EJECT,
                                                  0,
                                                  0};

//-------------------------------------------------------------------------------------------------------------------------
void clear_keycode_buffer(void) {
    memsets(ble_protocol_key, 0, 8);
    memsets(ble_protocol_consumer, 0, 4);
    ble_protocol_system[0] = 0;
}

bool analogg_ble_config_handle(protocol_cmd _protocol_cmd) {
    if (is_ble_work_state() == INPUT_MODE) set_ble_work_state(WAIT_CONFIG_MODE);
    if (_protocol_cmd.type == DATA_TYPE_STATE) {
        general_protocol_array_of_byte(_protocol_cmd.type, 0, NULL);
        return true;
    }
    ble_protocol_payload_cmd[0] = _protocol_cmd.value;
    if (_protocol_cmd.type == DATA_TYPE_BATTERY_LEVEL) {
        general_protocol_array_of_byte(_protocol_cmd.type, sizeof(ble_protocol_payload_cmd), &ble_protocol_payload_cmd[0]);
        return true;
    }

    analogg_ble_reset_leds();

    switch (_protocol_cmd.type) {
        // case DATA_TYPE_BATTERY_LEVEL:
        // 	general_protocol_array_of_byte(_protocol_cmd.type,sizeof(ble_protocol_payload_cmd),&ble_protocol_payload_cmd[0]);
        // 	break;
        case DATA_TYPE_GET_TUNNEL_ID:
            break;
        case DATA_TYPE_SWITCH:
            clear_keycode_buffer();
            general_protocol_array_of_byte(_protocol_cmd.type, sizeof(ble_protocol_payload_cmd), &ble_protocol_payload_cmd[0]);
            return true;
        case DATA_TYPE_GET_AUTH:
            break;
        case DATA_TYPE_SET_AUTH:
            break;
        case DATA_TYPE_GET_PSWD:
            break;
        case DATA_TYPE_SET_PSWD:
            break;
        case DATA_TYPE_GET_NAME:
            break;
        case DATA_TYPE_SET_NAME:
            break;
        case DATA_TYPE_PAIR:
            clear_keycode_buffer();
            general_protocol_array_of_byte(_protocol_cmd.type, sizeof(ble_protocol_payload_cmd), &ble_protocol_payload_cmd[0]);
            return true;
        case DATA_TYPE_UNPLUG:
            general_protocol_array_of_byte(_protocol_cmd.type, sizeof(ble_protocol_payload_cmd), &ble_protocol_payload_cmd[0]);
            return true;
        case DATA_TYPE_GET_BAUD:
            break;
        case DATA_TYPE_SET_BAUD:
            general_protocol_array_of_byte(_protocol_cmd.type, sizeof(ble_protocol_payload_cmd), &ble_protocol_payload_cmd[0]);
            return true;
        case DATA_TYPE_HD_VERSION:
            break;
        case DATA_TYPE_DFU:
            break;
        case DATA_TYPE_RESET:
            break;
        case DATA_TYPE_DEFAULT:
            clear_keycode_buffer();
            general_protocol_array_of_byte(_protocol_cmd.type, sizeof(ble_protocol_payload_cmd), &ble_protocol_payload_cmd[0]);
            break;
        case DATA_TYPE_GET_SLEEP:
            break;
        case DATA_TYPE_SET_SLEEP:
            break;
        default:
            break;
    }
    return false;
}

// uint16_t getLastPressedKey(uint8_t bufferByte, uint8_t size, uint16_t baseKeycode){
//     uint8_t i;
//     if (size>8)return 0;
//     for(i=0; i<size; i++)if(bufferByte==ble_protocol_bitH0_7[i]) return baseKeycode+i;
//     return 0;
// }

bool analogg_ble_keycode_handle(protocol_cmd _protocol_cmd) {
    if (!is_ble_work_state_input()) set_ble_work_state(WAIT_INPUT_MODE);

    uint8_t  i = 0, j = 0;
    uint16_t keycode = _protocol_cmd.value;
    if (IS_KEY(keycode)) {
        bool isExist = false;
        for (i = 2; i < 8; i++) {
            if (_protocol_cmd.pressed) {
                if (!isExist && ble_protocol_key[i] == keycode) {
                    isExist = true;
                    continue;
                }

                if (isExist && ble_protocol_key[i] == keycode) { // if it exists, clear it;
                    ble_protocol_key[i] = 0;
                }

                if (ble_protocol_key[i] == 0) {
                    ble_protocol_key[i] = keycode;
                    break;
                }
            } else {
                if (ble_protocol_key[i] == keycode) {
                    ble_protocol_key[i] = 0;
                }
            }
        }
        general_protocol_array_of_byte(DATA_TYPE_KEY, sizeof(ble_protocol_key), &ble_protocol_key[0]);
    } else if (IS_MOD(keycode)) {
        if (_protocol_cmd.pressed) {
            ble_protocol_key[0] = ble_protocol_key[0] | ble_protocol_bitH0_7[keycode - KC_LEFT_CTRL];
        } else {
            ble_protocol_key[0] = ble_protocol_key[0] & (~ble_protocol_bitH0_7[keycode - KC_LEFT_CTRL]);
        }
        general_protocol_array_of_byte(DATA_TYPE_KEY, sizeof(ble_protocol_key), &ble_protocol_key[0]);
    } else if (IS_SYSTEM(keycode)) {
        if (_protocol_cmd.pressed) {
            ble_protocol_system[0] = ble_protocol_system[0] | ble_protocol_bitH0_7[keycode - KC_SYSTEM_POWER];
        } else {
            ble_protocol_system[0] = ble_protocol_system[0] & (~ble_protocol_bitH0_7[keycode - KC_SYSTEM_POWER]);
        }
        general_protocol_array_of_byte(DATA_TYPE_SYSTEM_CONTROL, sizeof(ble_protocol_system), &ble_protocol_system[0]);
    } else if (IS_CONSUMER(keycode)) {
        for (j = 0; j < 4; j++) {
            if (j == 0) {
                for (i = 0; i < 8; i++)
                    if (keycode == ble_protocol_consumer_byte5[i]) {
                        ble_protocol_consumer[j] = 0;
                        if (_protocol_cmd.pressed) ble_protocol_consumer[j] = ble_protocol_bitH0_7[i];
                    }
            } else if (j == 1) {
                for (i = 0; i < 8; i++)
                    if (keycode == ble_protocol_consumer_byte6[i]) {
                        ble_protocol_consumer[j] = 0;
                        if (_protocol_cmd.pressed) ble_protocol_consumer[j] = ble_protocol_bitH0_7[i];
                    }
            } else if (j == 2) {
                for (i = 0; i < 8; i++)
                    if (keycode == ble_protocol_consumer_byte7[i]) {
                        ble_protocol_consumer[j] = 0;
                        if (_protocol_cmd.pressed) ble_protocol_consumer[j] = ble_protocol_bitH0_7[i];
                    }
            } else if (j == 3) {
                for (i = 0; i < 8; i++)
                    if (keycode == ble_protocol_consumer_byte8[i]) {
                        ble_protocol_consumer[j] = 0;
                        if (_protocol_cmd.pressed) ble_protocol_consumer[j] = ble_protocol_bitH0_7[i];
                    }
            }
        }
        general_protocol_array_of_byte(DATA_TYPE_CONSUMER_KEY, sizeof(ble_protocol_consumer), &ble_protocol_consumer[0]);
    } else {
        return false;
    }
    return true;
}
