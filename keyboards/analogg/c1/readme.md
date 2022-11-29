# analogg_dbg103/rev3

## General information

![analogg_dbg103/rev3](https://i.imgur.com/dlwcDbSh.jpg)

*Analogg, Aluminum Body Wireless QMK&VIA Mechanical Keyboard*

* Keyboard Maintainer: [Jaypei](https://github.com/jaypei), [GuangJun Wei](https://github.com/wgj600)
* Hardware Supported: *Analogg C1, STM32F103*
* Hardware Availability: *[Analogg](https://theanalogg.com)*

## Build and flash firmware

Make example for this keyboard (after setting up your build environment):

```sh
# for minimal keymap
make analogg/c1/rev01:default
# for via keymap
make analogg/c1/rev01:via
```

Flashing example for this keyboard:

```sh
make analogg/c1/rev01:default:flash
```

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 2 ways:

* **Physical reset button**: Briefly press the button under the spacebar. It can be accessed by removing the spacebar keycap
* **Keycode in layout**: Press the key mapped to `RESET` if it is available
