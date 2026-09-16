#include <stdint.h>
#include "defines.c"
#include "init_hw.c"
#include "helpers.c"
void usleep(unsigned int usec)
{
    volatile unsigned int i;

    while (usec--) {
        for (i = 0; i < 100; i++)
            __asm__ volatile ("nop");
    }
}

int entry_main(void)
{	// *(volatile unsigned int *)0xA0020C20 = 0x11111111;
	//keypad_boot_key_mux = 0x11000001;
	keypad_mux_init();
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
