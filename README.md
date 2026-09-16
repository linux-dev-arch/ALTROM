# ALTROM
This is an alternative operating system for the nokia 130 music (MT6261).

## Current Status
* Custom Code execution.
* Bootup from download agent payload.
* Keypad and Keypad backlights.
* runs completely from ram!.
* Reboot by using watchdog timer reset.
 
##  What Does It DO?
* Rn the Payload boots up and toggles the keypad backlight using the LEFT and RIGHT keys
* Reboots when # key is pressed.
* Powers off when usb cable is disconnected.

## How to Run?
* A gnu/linux based operating system.
* Download payload.bin from releases or build it using build instructions.
* Download mediatek-flash tool from <a href="https://github.com/ilyakurdyukov/mediatek_flash">here</a>.
* Copy the payload.bin file to the top folder of the flash tool repo
* run the following command
```
sudo ./mtk_dump connect simple_da payload.bin 0x70008000
```
* Once it loads, you can toggle the Keypad backlight using the LEFT and RIGHT keys.

## How To Build From Source
* Download this Github Repo.
* Download and install Android NDK (eg:android-ndk-r26c works well).
* Edit the NDK path in build.sh to your NDK path
* run the following command to build payload.bin
```
 bash build.sh
```

## Future Plans
* Get hardware like LCD,SD card,USB and power management working.
* Implement a simple GUI library
* Get custom apps (like elf and bin) running from Micro SD!

## Supported devices
* Nokia 130 music (TA-1576)

## Credits
* https://github.com/MediatekInfo/DZ09 - Some of the mt6261 reverse engineered
* https://github.com/ilyakurdyukov/mediatek_flash - flash tool and loader
* https://github.com/isogashii/fernly/ - Hardware level reverse engineering OS
