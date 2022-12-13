// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"
#include "color.h"

#undef WS2812_BYTE_ORDER
#define WS2812_BYTE_ORDER WS2812_BYTE_ORDER_RGB

typedef struct {
    LED_TYPE power;
    LED_TYPE ble;
    LED_TYPE caps_lock;
    LED_TYPE battery_level;
} _led_indicator;

void led_indicator_set_power(uint8_t r, uint8_t g, uint8_t b);
void led_indicator_set_power_pwm(bool isChrg);
void led_indicator_set_ble(uint8_t r, uint8_t g, uint8_t b);
void led_indicator_set_caps_lock(uint8_t r, uint8_t g, uint8_t b);
void led_indicator_set_battery_level(uint8_t r, uint8_t g, uint8_t b);
void led_indicator_show(void);

void led_indicator_set_ble_to(uint8_t tunnel);
void led_indicator_set_sleep(void);
void led_indicator_usb_suspended(void);
void rgblite_setrgb(_led_indicator led_indicator);

LED_TYPE indicator_set_rgb(uint8_t r, uint8_t g, uint8_t b);
