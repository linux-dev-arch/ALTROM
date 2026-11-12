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
* SD Card Drive and a simple READ ONLY FAT implementation.
* watchdog timer to help with crashes.
* The BATTERY works!
##  What Does It DO?
* The Payload boots up and shows you the altrom menu interface.
* the left key acts as universal back button.
* press # to put to sleep and press it again to wake it up.
* settings lets you change brightness
* the file manager is navigated by typing the number of the selection and pressing "OK"
* exit file manager by holding "right key" and "OK"
* you can load apps by selecting file and holding right.
* you can execute apps by running "execute app" from the menu.
* DO NOT run "execute app" IF YOU HAVE NOT LOADED AN APP.DOING SO CAN CAUSE THE SYSTEM TO CRASH.

## How to Run?
* You will need a gnu/linux based operating system.
* Download altrom.bin from <a href="https://github.com/linux-dev-arch/ALTROM/releases">releases</a> or build it using <a href=https://github.com/linux-dev-arch/ALTROM#how-to-run>build instructions</a>.
* Download mediatek-flash tool from <a href="https://github.com/ilyakurdyukov/mediatek_flash">here</a>.
* Copy the altrom repo to the top folder of the flash tool repo.
* Poweroff the phone and plug it in while holding the * key.
* run the following command
```
 make NAME=altrom run
```
* Once it loads, the display lights up and shows a menu and on bootup will attempt to test psram.

## How To Build From Source
* Download this Github Repo.
* Note:You must have arm-none-eabi toolchain and clang installed!
* run the following command to build altrom.bin
```
 make NAME=altrom
```

## Future Plans
* Get hardware like USB,torch,audio and VIBR working.

## Supported devices
* Nokia 130 music (TA-1576) [tested]
* Any feature phone with MT6261 chipset (might require custom drivers).

## Credits
* https://github.com/MediatekInfo/DZ09 - Some of the mt6261 reverse engineered
* https://github.com/ilyakurdyukov/mediatek_flash - flash tool and DA loader
* https://github.com/isogashii/fernly/ - Hardware level reverse engineering OS
* <a href="https://drive.google.com/file/d/1_u99xIKyY8CJ5ilEQIDJ67I0Cq0AjG8g/view?usp=sharing">reference datasheet</a> [ALTROM does not redistribute the datasheet.]
