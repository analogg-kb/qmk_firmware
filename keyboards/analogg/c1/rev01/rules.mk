BLUETOOTH_DRIVER = RN42
RGB_MATRIX_DRIVER = IS31FL3733
# and disable RGB_DISABLE_WHEN_USB_SUSPENDED
NO_USB_STARTUP_CHECK = yes
# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE

SRC += analogg_bm1.c
SRC += rgb.c
SRC += led_indicator.c
SRC += analog.c
SRC += analogg.c
SRC += battery_manager.c
