
AUDIO_ENABLE = no           # Audio output 
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality

DEFERRED_EXEC_ENABLE = yes

DIP_SWITCH_ENABLE = yes
RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = IS31FL3733
ENCODER_ENABLE = yes    # Enable Encoder

NO_USB_STARTUP_CHECK = yes  #and disable RGB_DISABLE_WHEN_USB_SUSPENDED 
# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
