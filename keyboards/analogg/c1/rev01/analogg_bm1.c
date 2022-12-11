#include "analogg_bm1.h"
#include "analogg.h"

static const SerialConfig ble_uart_config = {
    .speed = 115200  //921600
};

bool ble_send(SerialDriver *sdp, const uint8_t* source, const size_t size) {
    bool success = (size_t)sdWriteTimeout(sdp, source, size, TIME_MS2I(100)) == size;
    return success;
}

// bool ble_receive(SerialDriver *sdp, uint8_t* destination, const size_t size) {
//     bool success = (size_t)sdReadTimeout(sdp, destination, size, TIME_MS2I(100)) == size;
//     return success;
// }

void analogg_bm1_init(void) {
#ifdef PIO11_WAKEUP
    setPinOutput(PIO11_WAKEUP);
    writePinLow(PIO11_WAKEUP);
#endif

#ifdef BLE_RST
    setPinOutput(BLE_RST);
    writePinHigh(BLE_RST);   //reset the ble moudle
    wait_ms(100);
    writePinLow(BLE_RST);
#endif
    // Start BLE UART
    sdStart(&SD1, &ble_uart_config);
    // Give the send uart thread some time to
    // send out the queue before we read back
    wait_ms(100);
    // loop to clear out receive buffer from ble wakeup
    while (!sdGetWouldBlock(&SD1)) sdGet(&SD1);
}

void analogg_bm1_task(void) {

}

void analogg_bm1_send_keyboard(report_keyboard_t *report) {

}

void analogg_bm1_send_mouse(report_mouse_t *report) {

}

void analogg_bm1_send_consumer(uint16_t usage) {

}
