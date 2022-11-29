/*
    Copyright (C) 2020 Yaotian Feng, Codetector<codetector@codetector.cn>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "analogg.h"
#include "analogg_ble.h"
#include "analogg_ble_protocol.h"
#include "eeprom.h"

uint8_t tunnel                      = 1;
uint8_t last_save_tunnel            = 1;

bool    is_rgb_enabled              = true;    // TODO TEST  

_ble_work_state     ble_work_state   = INPUT_MODE;
_ble_send_state     ble_send_state   = TX_IDLE;
_ble_tunnel_state   ble_tunnel_state;

void set_ble_work_state(_ble_work_state state){
    ble_work_state = state;
}

_ble_work_state is_ble_work_state(void){
    return ble_work_state;
}

bool is_ble_work_state_input(void){
    if (ble_work_state==INPUT_MODE || ble_work_state==WAIT_INPUT_MODE){
        return true;
    }else{
        return false;
    }
}

/* -------------------- Public Function Implementation ---------------------- */
void analogg_ble_stop(void){
    ble_send_state = IDLE;
}

void analogg_ble_send_cmd(uint8_t type){
    push_cmd(type,0,false);
}

void analogg_ble_send_cmd_by_id(uint8_t type, uint8_t tunnel_id){
    ble_state_led = BLE_LED_KEY_ONE;
    tunnel = tunnel_id;
    push_cmd(type,tunnel_id,false);
}

void analogg_ble_send_cmd_by_val(uint8_t type, uint8_t val){
    push_cmd(type,val,false);
}

void analogg_ble_startup(void) { 
    
    setPinInput(IS_BLE);

    setPinInput(IS_CHRG);
    writePinHigh(IS_CHRG);

    setPinOutput(PIO11_WAKEUP);
    writePinLow(PIO11_WAKEUP);

#ifdef BLE_RST
    setPinOutput(BLE_RST);
    writePinHigh(BLE_RST);   //reset the ble moudle
    wait_ms(100);
    writePinLow(BLE_RST);
#endif
}

void analogg_ble_disconnect(void) {
    clear_keyboard();
}

void analogg_ble_reset_leds(){
    if (is_ble_work_state()==WAIT_CONFIG_MODE){
        for (uint8_t i = 0; i < BLE_TUNNEL_NUM ; i++) ble_tunnel_state.list[i]=IDLE;  //reset leds state
        is_rgb_enabled = rgb_matrix_is_enabled();   // Save the current rgb state
        rgb_matrix_enable_noeeprom();               // Turn on the rgb light
    }
}

void analogg_ble_mouse(){
    
} 





