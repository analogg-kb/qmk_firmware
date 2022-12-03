RGB_MATRIX_DRIVER = IS31FL3733
# and disable RGB_DISABLE_WHEN_USB_SUSPENDED
NO_USB_STARTUP_CHECK = yes
# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
