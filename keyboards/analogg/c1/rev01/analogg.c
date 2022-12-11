// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "usb_main.h"

#include "analogg_ble_protocol.h"
#include "analogg_ble.h"
#include <ctype.h>
#include "analog.h"
#include "analogg.h"
#include "led_indicator.h"
#include "rgb.h"

#include "chtime.h"
#include "config.h"
#include "keycode_config.h"

extern bool last_rgb_enabled;
static uint16_t time_count = 0;
uint32_t timer_callback(uint32_t trigger_time, void *cb_arg);

uint16_t get_timer_count(void) {
    return time_count;
}

// clang-format off
// Disable the DIP switch key
const matrix_row_t matrix_mask[] = {
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b111111111011111,
};
// clang-format on

bool     is_usb_suspended         = false;
bool     is_chrg                  = false;
// Charging light readed count
uint8_t  chrg_count               = 0;
// Remaining capacity
float    chrg_rsoc                = 0;
// Long press monitor
bool     custom_pressed           = false;
uint16_t custom_keycode           = KC_NO;
uint16_t last_keycode             = KC_NO;
uint16_t custom_pressed_long_time = 0;
// Log tick time
uint16_t log_time                 = 0;

protocol_cmd pop_protocol_cmd = {0};


void battery_board_init(void) {
    setPinInput(IS_CHRG);
    writePinHigh(IS_CHRG);
}


/* --------------------------- qmk function------------------------------ */
void board_init(void) {
    AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_Msk;
    // disable JTAG and SWD
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_DISABLE_Msk;
    // init ble/usb switch
    setPinInput(IS_BLE);
    battery_board_init();
}

void keyboard_post_init_kb(void) {
    defer_exec(TIMER_BASE_TIME, timer_callback, NULL);
    rgb_init();
    keyboard_post_init_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode == RGB_TOG) {
        last_rgb_enabled = !rgb_matrix_is_enabled();
    }

    if (!readPin(IS_BLE)) {
#ifdef CONSOLE_ENABLE
        uprintf("%5d Q:cable,kc=%04x pressed=%d\n", log_time, keycode, record->event.pressed);
#endif
        return process_record_user(keycode, record);
    }

    // BLE mode
    if (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) { // fn
        return process_record_user(keycode, record);
    }
    custom_keycode = keycode;
    custom_pressed = record->event.pressed;
    if (keycode >= BT_TN1 && keycode <= BT_FTY) { // costom key
        if (record->event.pressed) {
            rgb_matrix_enable_noeeprom();
            uprintf("rgb_matrix_enable_noeeprom=3\n");
            switch (keycode) {
                case BT_TN1:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 1);
                    break;
                case BT_TN2:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 2);
                    break;
                case BT_TN3:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 3);
                    break;
                case BT_TN4:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 4);
                    break;
                case BT_TN5:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 5);
                    break;
                // case BT_TN6: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 6);break;
                // case BT_TN7: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 7);break;
                // case BT_TN8: analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, 8);break;
                case BT_STATE:
                    rgb_matrix_indicator = BLE_LED_KEY_ALL;
                    analogg_ble_send_cmd(DATA_TYPE_STATE);
                    break;
                default:
                    break;
            }
        }
        return process_record_user(keycode, record);
    }
    if (IS_ANY(keycode)) {
        rgb_key_press_task();
        rgb_matrix_indicator = RGB_MATRIX_ANIMATION;
        push_cmd(DATA_TYPE_DEFAULT_KEY, keycode, record->event.pressed);
        return false;
    } else {
        return process_record_user(keycode, record);
    }
}

static uint16_t bt_log_time_count = 0;

uint32_t timer_callback(uint32_t trigger_time, void *cb_arg) {
    if (!readPin(IS_BLE)) {
        if (is_usb_suspended) {
            led_indicator_set_black();
            rgb_matrix_indicator = RGB_MATRIX_ANIMATION; // Turn on RGB matrix
        }
        return TIMER_BASE_TIME * 100;
    } else {
        long_pressed_event();

        switch (ble_send_state) {
            case TX_IDLE:
                uart_tx_data_handle();
                break;
            case TX_TIMEOUT:
                analogg_ble_cmd_tx(++mSeqId);
                break;
            case TX_RESTART:
                analogg_ble_cmd_tx(++mSeqId);
                break;
            default:
                ble_send_state++;
                break;
        }

        uint8_t state = ble_tunnel_state.list[ble_tunnel_state.tunnel];
        time_count++;
        if (time_count % T100MS == 0) {
            // ble led state
            if (state == CONNECTED || state == CONNECTED_AND_ACTIVE) led_indicator_set_ble_to(ble_tunnel_state.tunnel);

            // POWER LED state
            //  uprintf("chrg_count=%d %b\n",chrg_count,is_chrg);
            if (chrg_count > 30) is_chrg = true;
            chrg_count = 0;
            led_indicator_set_power_pwm(is_chrg);

            // CAPS_LOCK LED state
            if (host_keyboard_led_state().caps_lock) {
                led_indicator_set_caps_lock(RGB_WHITE);
            } else {
                led_indicator_set_caps_lock(RGB_BLACK);
            }
            // get ble
            if (is_tx_idle()) {
                analogg_ble_send_cmd(DATA_TYPE_STATE);
                bt_log_time_count++;
                if (bt_log_time_count > 600) {
                    bt_log_time_count = 0;
                    analogg_ble_send_cmd_by_val(DATA_TYPE_BATTERY_LEVEL, (uint8_t)((int16_t)chrg_rsoc));
                }
            }
        }
        if (time_count % T200MS == 0)
            if (state == ADV_WAIT_CONNECTING_ACTIVE) led_indicator_set_ble_to(ble_tunnel_state.tunnel);
        if (time_count % T400MS == 0)
            if (state == ADV_WAIT_CONNECTING_ACTIVE) led_indicator_set_ble(RGB_BLACK);

        if (time_count % T1S == 0) {
            log_time++;
            // BATTERY LEVEL LED state
            uint16_t bl = analogReadPin(BATTERY_LEVEL_PORT);
            chrg_rsoc        = ((bl - BATTERY_RSOC_0) / BATTERY_RSOC_AREA) * 100.00f;
            if (chrg_rsoc > 100) {
                chrg_rsoc = 100;
            }
            if (chrg_rsoc < 0) {
                chrg_rsoc = 0;
            }
            if (chrg_rsoc < 10) {
                led_indicator_set_battery_level(RGB_RED);
            } else if (chrg_rsoc >= 10 && chrg_rsoc < 40) { // 10-40
                led_indicator_set_battery_level(RGB_ORANGE);
            } else if (chrg_rsoc >= 40 && chrg_rsoc < 70) { // 40-70
                led_indicator_set_battery_level(RGB_YELLOW);
            } else if (chrg_rsoc >= 70) {
                led_indicator_set_battery_level(RGB_GREEN);
            }
            // rgb
            rgb_sleep_timer_task();

            if (state == IDLE || state == ADV_WAIT_CONNECTING || state == ADV_WAIT_CONNECTING_INACTIVE) {
                led_indicator_set_ble_to(ble_tunnel_state.tunnel);
            }
        }
        if (time_count % T2S == 0) {
            if (state == IDLE || state == ADV_WAIT_CONNECTING || state == ADV_WAIT_CONNECTING_INACTIVE) {
                led_indicator_set_ble(RGB_BLACK);
            }
            time_count = 0;
        }
        if (is_tx_idle()) {
            led_indicator_show();
        }
    }
    return TIMER_BASE_TIME;
}


void long_pressed_event(void) {
    uint16_t time = 0;
    if (custom_keycode >= BT_TN1 && custom_keycode <= BT_FTY) {
        if (!custom_pressed) {
            last_keycode = KC_NO;
            return;
        }

        if (last_keycode != custom_keycode) {
            last_keycode             = custom_keycode;
            custom_pressed_long_time = timer_read();
            return;
        }

        time = timer_read() > custom_pressed_long_time ? (timer_read() - custom_pressed_long_time) : (65535 - custom_pressed_long_time) + timer_read();
        if (time >= LONG_PRESSED_TIME && last_keycode != KC_NO) {
            // uprintf(" BLE time---: %d  0x%04X\n", time,custom_keycode);
            switch (custom_keycode) {
                case BT_TN1:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 1);
                    break;
                case BT_TN2:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 2);
                    break;
                case BT_TN3:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 3);
                    break;
                case BT_TN4:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 4);
                    break;
                case BT_TN5:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 5);
                    break;
                // case BT_TN6: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 6);break;
                // case BT_TN7: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 7);break;
                // case BT_TN8: analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, 8);break;
                case BT_FTY:
                    analogg_ble_send_cmd(DATA_TYPE_DEFAULT);
                    rgb_matrix_indicator = BLE_LED_KEY_ALL;
                    analogg_ble_send_cmd(DATA_TYPE_STATE);
                    uprintf("%5d Q:Restore factory mode\n", log_time);
                    break;
                default:
                    break;
            }
            custom_pressed = false;
            last_keycode   = KC_NO;
        }
    }
}


bool uart_tx_data_handle(void) {
    if (ble_send_state == TX_IDLE && bufferPop(&pop_protocol_cmd)) {
        ble_send_state = TX_START;
        if (pop_protocol_cmd.type == DATA_TYPE_DEFAULT_KEY) {
            analogg_ble_keycode_handle(pop_protocol_cmd);
        } else {
            analogg_ble_config_handle(pop_protocol_cmd);
        }
        return true;
    }
    return false;
}

void matrix_scan_kb(void) {
    if (!readPin(IS_CHRG)) {
        is_chrg = true;
        chrg_count++;
    } else {
        is_chrg    = false;
        chrg_count = 0;
    }
    while (!sdGetWouldBlock(&SD1)) {
        analogg_ble_resolve_protocol(sdGet(&SD1));
    }

    /**********RGB_DISABLE_WHEN_USB_SUSPENDED***********/
    if (USB_DRIVER.state == USB_SUSPENDED) {
        is_usb_suspended = true;
        if (!readPin(IS_BLE) && last_rgb_enabled) {
            if (!is_chrg) {
                rgb_sleep();
            }
        }
    } else if (USB_DRIVER.state == USB_ACTIVE) {
        if (is_usb_suspended) {
            if (last_rgb_enabled) {
                rgb_wakeup();
            } else {
                rgb_sleep();
            }
            is_usb_suspended = false;
        }
    }

    matrix_scan_user();
}

#ifdef DIP_SWITCH_MATRIX_GRID
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0) {
        default_layer_set(1UL << (active ? 2 : 0));
    }
    return true;
}
#endif

#ifdef ENCODER_ENABLE
bool encoder_update_kb(uint8_t index, bool clockwise) {
    if (!encoder_update_user(index, clockwise)) {
        return false;
    }
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    return true;
}
#endif
