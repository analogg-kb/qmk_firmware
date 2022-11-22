/* Copyright 2022 Jaypei
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

#define xxx KC_NO

enum layers{
    WIN_BASE,
    WIN_FN,
    MAC_BASE,
    MAC_FN,
};

// KEYCODES
enum keyboard_keycodes   {
    KC_00 = SAFE_RANGE,
    NEW_SAFE_RANGE,
    BT_TUNNEL1,
    BT_TUNNEL2,
    BT_TUNNEL3,
    BT_TUNNEL4,
    BT_TUNNEL5,
    BT_TUNNEL6,
    BT_TUNNEL7,
    BT_TUNNEL8,
    BT_STATE,
    BT_DEFAULT,
};

#define  BT_TN1   BT_TUNNEL1
#define  BT_TN2   BT_TUNNEL2
#define  BT_TN3   BT_TUNNEL3
#define  BT_TN4   BT_TUNNEL4
#define  BT_TN5   BT_TUNNEL5
#define  BT_FTY   BT_DEFAULT
