#include "bluetooth.h"
#include "analogg_bm1.h"

void bluetooth_init(void) {
    analogg_bm1_init();
}

void bluetooth_task(void) {
    analogg_bm1_task();
}

void bluetooth_send_keyboard(report_keyboard_t *report) {
    analogg_bm1_send_keyboard(report);
}

void bluetooth_send_mouse(report_mouse_t *report) {
    analogg_bm1_send_mouse(report);
}

void bluetooth_send_consumer(uint16_t usage) {
    analogg_bm1_send_consumer(usage);
}
