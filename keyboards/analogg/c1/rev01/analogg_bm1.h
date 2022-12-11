// Copyright 2021 GuangJun.Wei (@wgj600)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include <report.h>

bool ble_send(SerialDriver *sdp, const uint8_t *source, const size_t size);
// bool ble_receive(SerialDriver *sdp, uint8_t* destination, const size_t size);

void analogg_bm1_init(void);
void analogg_bm1_task(void);
void analogg_bm1_send_keyboard(report_keyboard_t *report);
void analogg_bm1_send_mouse(report_mouse_t *report);
void analogg_bm1_send_consumer(uint16_t usage);
