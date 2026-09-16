#include <stdint.h>
#include "defines.c"
#include "init_hw.c"
#include "helpers.c"
int entry_main(void)
{	keypad_mux_init();
	int bkl_state=0;
	for(;;){
		if((KP_LEFT)==0){
			bkl_state=1;
		}
		if(KP_HASH==0){
			reboot();
		}
		if((KP_RIGHT) == 0){
			bkl_state=0;
		}
		if(bkl_state == 1){
			keypad_backlight=3;
		}
		else{
			keypad_backlight=0;
		}
	}
}
