void kp_blink(uint32_t cycles){
	for(;;){
		KP_BL_set(KP_BL_ON);
		sleep(cycles);
		KP_BL_set(KP_BL_OFF);
		sleep(cycles);
		if(KP_OK==0){
			break;
		}
	}
}
void psram_test(){
volatile uint32_t *psram = (volatile uint32_t *)0x00000000;
	psram[0] = 0x12345678;
	psram[1] = 0xdeadbeef;
	psram[2] = 0xbeefdead;
	if (psram[0] == 0x12345678 &&
    		psram[1] == 0xdeadbeef && psram[2]== 0xbeefdead) {
    		/* PSRAM works */
		KP_BL_set(KP_BL_ON);
		fb_clear(COL_BLACK);
		draw_string8(10, 120, "PSRAM TEST PASSED",COL_BLACK,COL_GREEN, 0, 2);
		fb_flush();
		sleep(100000);
	}
	else{
		fb_clear(COL_BLACK);
		draw_string8(10, 120, "PSRAM TEST FAILED",COL_BLACK,COL_GREEN, 0, 2);
		fb_flush();
		for(;;){}
}
}
void jump_rst(void){
	jump(0x70008000);
}
