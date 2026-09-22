void kp_toggle(void){
//Simple KP BL test func
int bkl_state=0;
        for(;;){
                if((KP_CENTER_UP)==0){
                        bkl_state=1;
                }
                if(KP_HASH==0){
                        reboot();//reboot
                }
                if((KP_CENTER_DOWN) == 0){
                        bkl_state=0;
                }
		if((KP_LEFT)==0){
			break;//exit the app
		}
                if(bkl_state == 1){
                        KP_BL=KP_BL_ON;
                }
                else{
                        KP_BL=KP_BL_OFF;
                }
        }
}
void kp_blink(uint32_t cycles){
	for(;;){
		KP_BL_set(KP_BL_ON);
		sleep(cycles);
		KP_BL_set(KP_BL_OFF);
		sleep(cycles);
		if(KP_RIGHT ==0){
			break;
		}
		if(KP_HASH==0){
			reboot();
		}
	}
}
void psram_test(){
volatile uint32_t *psram = (volatile uint32_t *)0x00000000;
	psram[0] = 0x12345678;
	psram[1] = 0xdeadbeef;
	if (psram[0] == 0x12345678 &&
    		psram[1] == 0xdeadbeef) {
    		/* PSRAM works */
		sleep(100000);
		KP_BL_set(KP_BL_ON);
		sleep(100000);
	}
}
