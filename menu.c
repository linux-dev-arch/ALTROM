#include <stdint.h>
//#include "defines.h"
//#include "init_hw.c"
//#include "helpers.c"
//#include "apps.c"
//#include "lcd.c"
//#include "fb.c"
//#include "fonts.c"
//#include "font.h"
void menu(void){
	int sel=0;
	fb_clear(COL_BLACK);
	draw_string12(10, 120,"LOADING MENU...",COL_GREEN,COL_BLACK,0,5);
	fb_flush();
	sleep(100000);
	fb_clear(COL_BLACK);
	draw_string12(80, 10,"ALTROM",COL_BLACK,COL_GREEN,0,5);
	draw_string12(10, 50, "APPLICATIONS",COL_GREEN,COL_BLACK, 0, 2);
	draw_string12(10, 70, "SETTINGS",COL_GREEN,COL_BLACK, 0, 2);
	draw_string12(10, 90, "ABOUT",COL_GREEN,COL_BLACK, 0, 2);
	draw_string12(10, 110, "REBOOT",COL_GREEN,COL_BLACK, 0, 2);
	fb_flush();
	KP_BL=KP_BL_ON;
	for(;;){
		fb_clear(COL_BLACK);
		draw_string12(80, 10,"ALTROM",COL_BLACK,COL_GREEN,0,5);
		draw_string12(10, 50, "APPLICATION",COL_GREEN,COL_BLACK, 0, 2);
        	draw_string12(10, 70, "SETTINGS",COL_GREEN,COL_BLACK, 0,2);
        	draw_string12(10, 90, "ABOUT",COL_GREEN,COL_BLACK, 0, 2);
        	draw_string12(10, 110, "REBOOT",COL_GREEN,COL_BLACK, 0, 2);
		if(KP_CENTER_DOWN==0 && sel <3){
			sel++;
		}
		if(KP_CENTER_UP ==0 && sel>0){
			sel--;
		}
		if(KP_OK==0){
			if(sel==2){
				fb_clear(COL_BLACK);
				draw_string8(40,10,"Welcome to ALTROM!",COL_GREEN,COL_BLACK,0,5);
				draw_string8(40,30,"an open source os ",COL_GREEN,COL_BLACK,0,5);
				draw_string8(70,50,"written by",COL_GREEN,COL_BLACK,0,5);
				draw_string8(75,70,"linuxdev",COL_GREEN,COL_BLACK,0,5);
				draw_string8(40,90," for Phones with",COL_GREEN,COL_BLACK,0,5);
				draw_string8(35,110,"the MT6261 chipset",COL_GREEN,COL_BLACK,0,5);
				draw_string8(10,150,__DATE__" "__TIME__,COL_GREEN,COL_BLACK,1,2);
				draw_string8(10,180,"...PRESS OK TO GO BACK...",COL_GREEN,COL_BLACK,0,5);
				fb_flush();
				for(;;){
					if(KP_OK==0){
						sleep(50000);
						break;
					}
				}
			}
			if(sel==3){
				fb_clear(COL_BLACK);
				draw_string12(10, 10,"Rebooting...",COL_GREEN,COL_BLACK,0,5);
				fb_flush();
				sleep(100000);
				reboot();
			}
		}
		if(sel==0){
			draw_string12(10, 50, "APPLICATION",COL_BLACK,COL_GREEN, 0, 2);
		}
		if(sel==1){
        		draw_string12(10, 70, "SETTINGS",COL_BLACK,COL_GREEN, 0, 3);
		}
		if(sel == 2){
			draw_string12(10, 90, "ABOUT",COL_BLACK,COL_GREEN, 0, 3);
		}
		if(sel == 3){
			draw_string12(10, 110, "REBOOT",COL_BLACK,COL_GREEN, 0, 3);
		}
		fb_flush();
	}
}
