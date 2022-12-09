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
    LED_TYPE led = {.r=g,.g=r,.b=b};   //rgb.gbr.brg...
#ifdef RGBLIGHT_LIMIT_VAL
    if (r==255){
        led.g = RGBLIGHT_LIMIT_VAL;    //rgb.gbr.brg...
    }
    if (g==255){
        led.r = RGBLIGHT_LIMIT_VAL;
    }
    if (b==255){
        led.b = RGBLIGHT_LIMIT_VAL;
    }
#endif
    return led;
}


