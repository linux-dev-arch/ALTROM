void reboot(void){
	WDT=0x1209;
}
void sleep(unsigned int clk)
{
    volatile unsigned int i;

    while (clk--) {
        for (i = 0; i < 100; i++)
            __asm__ volatile ("nop");
    }
}
