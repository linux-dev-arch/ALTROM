#include <stdint.h>
#include "defines.h"
#include "divmod.c"
#include "init_hw.c"
#include "helpers.c"
#include "lcd.c"
#include "fb.c"
#include "fonts.c"
#include "font.h"
#include "menu.c"
#include "tests.c"
//#include "battery.c"
//#include "power.c"
//#include "power_debug.c"
void entry_main(void){
        int brightness=3;
        init();
        lcd_init();
        backlight(brightness);
	//power_debug();
	//power_hold();
	//battery_app();
	fb_clear(COL_BLACK);
	psram_test();
	fb_flush();
        menu();
}
