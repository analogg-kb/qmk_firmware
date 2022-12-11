// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "led_indicator.h"
#include "ws2812.c"

_led_indicator  led_indicator               = {.power={RGB_BLACK},.ble={RGB_BLACK},.caps_lock={RGB_BLACK},.battery_level={RGB_BLACK}};

void led_indicator_set_ble_to(uint8_t tunnel){
    switch (tunnel){
        case 0:  led_indicator.ble = indicator_set_rgb(RGB_BLUE);       break;
        case 1:  led_indicator.ble = indicator_set_rgb(RGB_GREEN);      break;
        case 2:  led_indicator.ble = indicator_set_rgb(RGB_YELLOW);     break;
        case 3:  led_indicator.ble = indicator_set_rgb(RGB_MAGENTA);    break;
        case 4:  led_indicator.ble = indicator_set_rgb(RGB_RED);        break;
        default:break;
    }
}

void led_indicator_set_black(void){
    led_indicator.power         = indicator_set_rgb(RGB_WHITE);
    led_indicator.ble           = indicator_set_rgb(RGB_BLACK);
    led_indicator.caps_lock     = indicator_set_rgb(RGB_BLACK);
    led_indicator.battery_level = indicator_set_rgb(RGB_BLACK);
    rgblite_setrgb(led_indicator);
}

static uint8_t rgb_pwm_inc=0, rgb_pwm_dec=0;
void led_indicator_set_power_pwm(bool isChrg){
    if (isChrg){
        if (rgb_pwm_inc<RGBLIGHT_LIMIT_VAL){
            rgb_pwm_inc+=2;
            rgb_pwm_dec=rgb_pwm_inc;
            led_indicator_set_power(rgb_pwm_inc,rgb_pwm_inc,rgb_pwm_inc);
        }else{
            if (rgb_pwm_dec>0){
                rgb_pwm_dec-=2;
            }else{
                rgb_pwm_dec=0;
                rgb_pwm_inc=0;
            }
            led_indicator_set_power(rgb_pwm_dec,rgb_pwm_dec,rgb_pwm_dec);
        }
    }else{
        led_indicator_set_power(RGB_WHITE);
    }
}

void led_indicator_set_power(uint8_t r, uint8_t g, uint8_t b){
    led_indicator.power = indicator_set_rgb(r,g,b);
}

void led_indicator_set_ble(uint8_t r, uint8_t g, uint8_t b){
    led_indicator.ble = indicator_set_rgb(r,g,b);
}

void led_indicator_set_caps_lock(uint8_t r, uint8_t g, uint8_t b){
    led_indicator.caps_lock = indicator_set_rgb(r,g,b);
}

void led_indicator_set_battery_level(uint8_t r, uint8_t g, uint8_t b){
    led_indicator.battery_level = indicator_set_rgb(r,g,b);
}

void led_indicator_show(void){
    rgblite_setrgb(led_indicator);
}

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
