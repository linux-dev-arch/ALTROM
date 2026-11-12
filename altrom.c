#include <stdint.h>
#include "defines.h"
#include "watchdog.c"
#include "divmod.c"
#include "init_hw.c"
#include "gpt.c"
#include "helpers.c"
#include "lcd.c"
#include "fb.c"
#include "fonts.c"
#include "font.h"
#include "tests.c"
#include "sd.c"
#include "fat.c"
#include "battery.c"
#include "settings.c"
#include "apps.c"
#include "menu.c"
#include "bat.c"
//#include "flash.c"
//#include "flash_test.c"


//#include "power.c"
//#include "power_debug.c"
void entry_main(void){
        int brightness=4;
        init();
	watchdog_init(5);
	timer_init();
        lcd_init();
	sd_init();
	battery_init();
        backlight(brightness);
	//flash_rehearsal();
	if(KP_ZERO!=0){
		Force_Battery_Power_Hold();
	}
	fb_clear(COL_BLACK);
	//psram_test();
	fb_flush();
	//file_mgr();
	//vibr_on();
        menu();
}
