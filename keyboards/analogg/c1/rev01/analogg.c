// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "usb_main.h"

#include "analogg_bm1.h"
#include <ctype.h>
#include "analogg.h"
#include "led_indicator.h"
#include "rgb.h"
#include "battery_manager.h"

#include "chtime.h"
#include "config.h"
#include "keycode_config.h"

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

// Long press monitor
bool     custom_pressed           = false;
uint16_t custom_keycode           = KC_NO;
uint16_t last_keycode             = KC_NO;
uint16_t custom_pressed_long_time = 0;
// Log tick time
uint16_t log_time                 = 0;

protocol_cmd pop_protocol_cmd = {0};

bool timer_task_dip_ble_query(void);
bool timer_task_dip_ble_query(void);
void long_pressed_event(void);
bool uart_tx_data_handle(void);
void matrix_scan_uart_recv(void);
void matrix_scan_usb_state(void);
bool dip_switch_update_kb(uint8_t index, bool active);
void dip_ble_update_event(bool ble_active);
void timer_task_update_battery_level(void);

void battery_board_init(void) {
    setPinInput(IS_CHRG_PIN);
    writePinHigh(IS_CHRG_PIN);
}

/* --------------------------- qmk function------------------------------ */
void board_init(void) {
    AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_Msk;
    // disable JTAG and SWD
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_DISABLE_Msk;
    // init ble/usb switch
    setPinInput(IS_BLE_PIN);
    battery_board_init();
}

void keyboard_post_init_kb(void) {
    defer_exec(TIMER_BASE_TIME, timer_callback, NULL);
    rgb_init();
    keyboard_post_init_user();
}

void matrix_scan_kb(void) {
    charge_task_scan_state_and_tick();
    matrix_scan_uart_recv();
    matrix_scan_usb_state();
    matrix_scan_user();
}

bool bm1_process_record_key(uint16_t keycode, keyrecord_t *record) {
    rgb_ble_indicator_exit();
    push_cmd(DATA_TYPE_DEFAULT_KEY, keycode, record->event.pressed);
    return false;
}

bool bm1_process_record_function_key(uint16_t keycode, keyrecord_t *record) {
    custom_keycode = keycode;
    custom_pressed = record->event.pressed;
    if (record->event.pressed) {
        rgb_matrix_enable_noeeprom();
        switch (keycode) {
            case BT_TN1 ... BT_TN5:
            // case BT_TN1 ... BT_TN8:
                analogg_ble_send_cmd_by_id(DATA_TYPE_SWITCH, keycode - BT_TN1 + 1);
                break;
            case BT_STATE:
                rgb_ble_indicator_show_all();
                analogg_ble_send_cmd(DATA_TYPE_STATE);
                break;
            default:
                break;
        }
    }
    return false;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    LOG_Q_DEBUG("kc=%04x pressed=%d", keycode, record->event.pressed);
    rgb_sleep_activity();

    // Process key directly when USB is connected
    if (IS_USB_DIP_ON()) {
        return process_record_user(keycode, record);
    }
    // Process key when BLE is connected
    switch (keycode) {
        case KC_A ... 0xFF:
            return bm1_process_record_key(keycode, record);
        case BT_TN1 ... BT_FTY:
            return bm1_process_record_function_key(keycode, record);
        default:
            return process_record_user(keycode, record);
    }
}

static uint16_t bt_log_time_count = 0;

static bool is_ble_dip_on = false;

bool timer_task_dip_ble_query() {
    bool _is_ble_on = IS_BLE_DIP_ON();
    if (_is_ble_on != is_ble_dip_on) {
        is_ble_dip_on = _is_ble_on;
        dip_ble_update_event(_is_ble_on);
    }
    return _is_ble_on;
}

void dip_ble_update_event(bool ble_active) {
    LOG_Q_INFO("Switch BLE = %d", ble_active);
    if (!ble_active) {
        bm1_reset();
        rgb_ble_indicator_exit();
    }
}

void timer_usb_mode(void) {
    if (is_usb_suspended) {
        led_indicator_usb_suspended();
        rgb_ble_indicator_exit();
    }
}

void timer_task_update_ble_tunnel_indicator(void) {
    uint8_t state = ble_tunnel_state.list[ble_tunnel_state.current_tunnel];
    // ble led state
    if (state == CONNECTED || state == CONNECTED_AND_ACTIVE) {
        led_indicator_set_ble_to(ble_tunnel_state.current_tunnel);
    }
}

void timer_task_update_battery_level(void) {
    // BATTERY LEVEL LED state
    uint8_t chrg_rsoc = battery_query_level();
    if (chrg_rsoc < 10) {
        led_indicator_set_battery_level(RGB_RED);
    } else if (chrg_rsoc >= 10 && chrg_rsoc < 40) { // 10-40
        led_indicator_set_battery_level(RGB_ORANGE);
    } else if (chrg_rsoc >= 40 && chrg_rsoc < 70) { // 40-70
        led_indicator_set_battery_level(RGB_YELLOW);
    } else if (chrg_rsoc >= 70) {
        led_indicator_set_battery_level(RGB_GREEN);
    }
}

void timer_task_caps_lock(void) {
    // CAPS_LOCK LED state
    if (host_keyboard_led_state().caps_lock) {
        led_indicator_set_caps_lock(RGB_WHITE);
    } else {
        led_indicator_set_caps_lock(RGB_BLACK);
    }
}

void timer_task_ble_state_query(void) {
    if (is_tx_idle()) {
        // Send state query command
        analogg_ble_send_cmd(DATA_TYPE_STATE);
        // Update battery level every 1 minutes
        bt_log_time_count++;
        if (bt_log_time_count > UPDATE_BATTERY_TIME) {
            uint8_t chrg_rsoc = battery_query_level();
            bt_log_time_count = 0;
            analogg_ble_send_cmd_by_val(DATA_TYPE_BATTERY_LEVEL, chrg_rsoc);
        }
    }
}

void timer_task_ble_indicator_blink(void) {
    uint8_t state = ble_tunnel_state.list[ble_tunnel_state.current_tunnel];
    if (state == ADV_WAIT_CONNECTING_ACTIVE) {
        if (time_count % T200MS == 0) {
            led_indicator_set_ble_to(ble_tunnel_state.current_tunnel);
        } else if (time_count % T400MS == 0) {
            led_indicator_set_ble(RGB_BLACK);
        }
    } else if (state == IDLE || state == ADV_WAIT_CONNECTING || state == ADV_WAIT_CONNECTING_INACTIVE) {
        if (time_count % T1S == 0) {
            led_indicator_set_ble_to(ble_tunnel_state.current_tunnel);
        } else if (time_count % T2S == 0) {
            led_indicator_set_ble(RGB_BLACK);
        }
    }
}

void timer_task_flush_led_indicator(void) {
    if (is_tx_idle()) {
        led_indicator_show();
    }
}

void timer_ble_mode(void) {
    switch (ble_send_state) {
        case TX_IDLE:
            uart_tx_data_handle();
            break;
        case TX_TIMEOUT:
            analogg_ble_cmd_tx(++mSeqId);
            break;
        default:
            ble_send_state++;
            break;
    }
}

uint32_t timer_callback(uint32_t trigger_time, void *cb_arg) {
    bool is_ble_dip = timer_task_dip_ble_query();
    time_count++;
    if (time_count % T100MS == 0) {
        long_pressed_event();
        timer_task_update_battery_level();
        charge_task_delay_debounce_tick();
        led_indicator_set_power_pwm(charge_task_is_charging());
        timer_task_caps_lock();
        if (is_ble_dip) {
            timer_task_update_ble_tunnel_indicator();
            timer_task_ble_state_query();
        }else{
            timer_usb_mode();
        }
        rgb_ble_tick(100);
    }
    // if (time_count % T200MS == 0) { }
    // if (time_count % T400MS == 0) { }
    if (time_count % T1S == 0) {
        log_time++;
        // rgb sleep mode
        rgb_sleep_tick(1000);
    }
    if (time_count % T2S == 0) {
        time_count = 0;
    }
    // For whole time
    if (is_ble_dip) {
        timer_ble_mode();
    }
    if(rgb_sleep_is_sleep()) {
        led_indicator_set_sleep();
    } else {
        timer_task_flush_led_indicator();
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
            switch (custom_keycode) {
                // case BT_TN1 ... BT_TN8:
                case BT_TN1 ... BT_TN5:
                    analogg_ble_send_cmd_by_id(DATA_TYPE_UNPLUG, custom_keycode - BT_TN1 + 1);
                    break;
                case BT_FTY:
                    rgb_ble_indicator_show_all();
                    analogg_ble_send_cmd(DATA_TYPE_DEFAULT);
                    analogg_ble_send_cmd(DATA_TYPE_STATE);
                    LOG_Q_INFO("Q:Restore factory mode");
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

void matrix_scan_uart_recv(void) {
    while (!sdGetWouldBlock(&SD1)) {
        analogg_ble_resolve_protocol(sdGet(&SD1));
    }
}

void matrix_scan_usb_state(void) {
    // RGB disable when usb suspend
    if (USB_DRIVER.state == USB_SUSPENDED) {
        is_usb_suspended = true;
        if (IS_USB_DIP_ON() && !charge_task_is_charging()) {
            rgb_sleep_sleep();
        }
    } else if (USB_DRIVER.state == USB_ACTIVE) {
        if (is_usb_suspended) {
            rgb_sleep_wakeup();
            is_usb_suspended = false;
        }
    }
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
