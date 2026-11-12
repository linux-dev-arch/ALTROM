#include <stdint.h>
//#include "defines.h"
//#include "init_hw.c"
//#include "helpers.c"
//#include "apps.c"
//#include "lcd.c"
//#include "fb.c"
//#include "fonts.c"
//#include "font.h"
#define APP_COUNT 5
#define MENU_COL COL_GREEN //COL_TEAL looks pretty cool :)
#define BG_COL COL_BLACK
const char *app_names[] = {
    "bounce",
    "snake",
    "calculator",
    "file manager",
    "execute app"
};

void (*apps[])(void) = {
    bounce,
    snake,//snake
    calc,//calc
    file_mgr,
    exec_app
};

void draw_bat_status(void){
	uint32_t mv  = battery_read_mv();
        int chg = charger_connected();
	if(!chg){
                draw_int8(10,300,mv,MENU_COL,BG_COL);
                draw_string8(50, 300, "mv",MENU_COL,BG_COL, 0, 2);
                charger_stop();
        }
        else{
                draw_int8(10,300,mv,COL_BLUE,BG_COL);
                draw_string8(50, 300, "mv",COL_BLUE,BG_COL, 0, 2);
                charger_start();
	}

}
void draw_apps_menu(int sel){
	fb_clear(BG_COL);
	draw_string12(80, 10,"ALTROM",BG_COL,MENU_COL,0,5);
	int line=1;
	for(int i=0; i<APP_COUNT;i++){
                if(app_names[i]){
                        if((sel+1)==line){
                                draw_string12(10,50+(15*line),app_names[i],BG_COL,MENU_COL,0,2);
                        }
                        else{
                                draw_string12(10,50+(15*line),app_names[i],MENU_COL,BG_COL,0,2);
                        }
                        line++;
                }
        }
}
void apps_menu(void){
	//char apps [20][20]={"bounce","snake","calculator"};
	//fb_clear(BG_COL);
	//draw_string12(80, 10,"ALTROM",BG_COL,MENU_COL,0,5);
	int sel=0;
	draw_apps_menu(sel);
	fb_flush();
	for(;;){
		if(KP_LEFT==0){
			break;
		}
		if(KP_CENTER_DOWN==0 && sel<(APP_COUNT-1)){
			sel++;
		}
		if(KP_CENTER_UP==0 && sel>0){
			sel--;
                }
		if(KP_OK==0){
			apps[sel]();
		}
		fb_clear(BG_COL);
		draw_apps_menu(sel);
		draw_bat_status();
		fb_flush();
		watchdog_feed();
	}
}

void menu(void){
	int sel=0;
	fb_clear(BG_COL);
	draw_string12(10, 120,"LOADING MENU...",MENU_COL,BG_COL,0,5);
	fb_flush();
	sleep(100000);
	fb_clear(BG_COL);
	draw_string12(80, 10,"ALTROM",BG_COL,MENU_COL,0,5);
	draw_string12(10, 50, "APPLICATIONS",MENU_COL,BG_COL, 0, 2);
	draw_string12(10, 70, "SETTINGS",MENU_COL,BG_COL, 0, 2);
	draw_string12(10, 90, "ABOUT",MENU_COL,BG_COL, 0, 2);
	draw_string12(10, 110, "REBOOT",MENU_COL,BG_COL, 0, 2);
	fb_flush();
	KP_BL=KP_BL_ON;
	for(;;){
		watchdog_feed();
		fb_clear(BG_COL);
		draw_string12(80, 10,"ALTROM",BG_COL,MENU_COL,0,5);
		draw_string12(10, 50, "APPLICATION",MENU_COL,BG_COL, 0, 2);
        	draw_string12(10, 70, "SETTINGS",MENU_COL,BG_COL, 0,2);
        	draw_string12(10, 90, "ABOUT",MENU_COL,BG_COL, 0, 2);
        	draw_string12(10, 110, "REBOOT",MENU_COL,BG_COL, 0, 2);
		draw_bat_status();
		if(KP_HASH == 0){
			fb_clear(BG_COL);
			fb_flush();
			backlight_off();
			KP_BL_set(KP_BL_OFF);
			delay_ms(500);
			for(;;){
				watchdog_feed();
				if(KP_HASH==0){
					backlight(4);
					KP_BL_set(KP_BL_ON);
					delay_ms(500);
					break;
				}
			}
		}
		if(KP_CENTER_DOWN==0 && sel <3){
			sel++;
		}
		if(KP_CENTER_UP ==0 && sel>0){
			sel--;
		}
		if(KP_OK==0){
			if(sel==0){
				apps_menu();
			}
			if(sel==1){
				brightness_change();
			}
			if(sel==2){
				fb_clear(BG_COL);
				draw_string8(40,10,"Welcome to ALTROM!",MENU_COL,BG_COL,0,5);
				draw_string8(40,30,"an open source os ",MENU_COL,BG_COL,0,5);
				draw_string8(70,50,"written by",MENU_COL,BG_COL,0,5);
				draw_string8(75,70,"linuxdev",MENU_COL,BG_COL,0,5);
				draw_string8(40,90," for Phones with",MENU_COL,BG_COL,0,5);
				draw_string8(35,110,"the MT6261 chipset",MENU_COL,BG_COL,0,5);
				draw_string8(10,150,__DATE__" "__TIME__,MENU_COL,BG_COL,1,2);
				draw_string8(0,180,"PRESS left key TO GO BACK",MENU_COL,BG_COL,0,5);
				fb_flush();
				for(;;){
					if(KP_LEFT==0){
						break;
					}
					watchdog_feed();
				}
			}
			if(sel==3){
				fb_clear(BG_COL);
				draw_string12(10, 10,"Rebooting...",MENU_COL,BG_COL,0,5);
				fb_flush();
				sleep(100000);
				reboot();
			}
		}
		if(sel==0){
			draw_string12(10, 50, "APPLICATION",BG_COL,MENU_COL, 0, 2);
		}
		if(sel==1){
        		draw_string12(10, 70, "SETTINGS",BG_COL,MENU_COL, 0, 3);
		}
		if(sel == 2){
			draw_string12(10, 90, "ABOUT",BG_COL,MENU_COL, 0, 3);
		}
		if(sel == 3){
			draw_string12(10, 110, "REBOOT",BG_COL,MENU_COL, 0, 3);
		}
		fb_flush();
	}
}
