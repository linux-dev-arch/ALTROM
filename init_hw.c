void keypad_mux_init(){
	//fixing gpio mux
	*(volatile unsigned int *)0xA0020C10 = 0x11111100;
	*(volatile unsigned int *)0xA0020C20 = 0x11111111;
}
