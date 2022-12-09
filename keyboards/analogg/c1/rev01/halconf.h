// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define HAL_USE_SERIAL TRUE
#define HAL_USE_I2C TRUE
#define HAL_USE_ADC TRUE
#define SERIAL_USB_BUFFERS_SIZE 256

#include_next <halconf.h>
