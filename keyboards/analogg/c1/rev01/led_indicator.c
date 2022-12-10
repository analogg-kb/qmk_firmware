// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "led_indicator.h"
#include "ws2812.c"

void rgblite_setrgb(_led_indicator led_indicator) {
    LED_TYPE leds[RGBLED_NUM] = {
        led_indicator.caps_lock,
        led_indicator.ble,
        led_indicator.battery_level,
        led_indicator.power};
    ws2812_setleds(leds, RGBLED_NUM);
}

LED_TYPE indicator_set_rgb(uint8_t r, uint8_t g, uint8_t b){
#ifdef RGBLIGHT_LIMIT_VAL
    r = r / 255 * RGBLIGHT_LIMIT_VAL;
    g = g / 255 * RGBLIGHT_LIMIT_VAL;
    b = b / 255 * RGBLIGHT_LIMIT_VAL;
#endif
#if (RGB_INDICATOR_ORDER == RGB_INDICATOR_ORDER_GRB)
    uint8_t tmp;
    tmp = r;
    r = g;
    g = tmp;
#elif (RGB_INDICATOR_ORDER == RGB_INDICATOR_ORDER_GRB)
    uint8_t tmp;
    tmp = r;
    r = b;
    b = tmp;
#endif
    return (LED_TYPE){ .r=r, .g=g, .b=b };
}
