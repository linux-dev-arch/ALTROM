# ALTROM
This is an alternative operating system for the nokia 130 music (MT6261).

## Current Status
* Custom Code execution.
* Bootup from download agent payload.
* Keypad and Keypad backlights.
* 4MB of internal PSRAM! (cool stuff :D)
* runs completely from ram!.
* Reboot by using watchdog timer reset.
* LCD and a simple graphics driver!
 
##  What Does It DO?
* The Payload boots up and blinks the keypad backlight at regular intervals.
* toggle the keypad backlight using the D pad up  and D pad down keys ,cycles the apps using left and right keys.
* Reboots when # key is pressed.
* Powers off when USB cable is disconnected.

## How to Run?
* You will need a gnu/linux based operating system.
* Download altrom.bin from <a href="https://github.com/linux-dev-arch/ALTROM/releases">releases</a> or build it using <a href=https://github.com/linux-dev-arch/ALTROM#how-to-run>build instructions</a>.
* Download mediatek-flash tool from <a href="https://github.com/ilyakurdyukov/mediatek_flash">here</a>.
* Copy the altrom.bin file to the top folder of the flash tool repo
* Poweroff the phone and plug it in while holding the * key.
* run the following command
```
sudo ./mtk_dump connect simple_da altrom.bin 0x70008000
```
* Once it loads, the display lights up and shows a menu and on bootup will attempt to test psram.

## How To Build From Source
* Download this Github Repo.
* Note:You must have arm-none-eabi toolchain and clang installed!
* run the following command to build altrom.bin
```
 make
```

## Future Plans
* Get hardware like SD card,USB and power management working.
* Get custom apps (like elf and bin) running from Micro SD!

## Supported devices
* Nokia 130 music (TA-1576)
* Any feature phone with MT6261 chipset (might require custom drivers).

## Credits
* https://github.com/MediatekInfo/DZ09 - Some of the mt6261 reverse engineered
* https://github.com/ilyakurdyukov/mediatek_flash - flash tool and DA loader
* https://github.com/isogashii/fernly/ - Hardware level reverse engineering OS
* <a href="https://drive.google.com/file/d/1_u99xIKyY8CJ5ilEQIDJ67I0Cq0AjG8g/view?usp=sharing">reference datasheet</a> [ALTROM does not redistribute the datasheet.]
