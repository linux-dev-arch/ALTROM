void reboot(void){
	WDT_TIMER=WDT_RST;
}
void sleep(unsigned int clk)
{	//do nothing for clk cycles
    volatile unsigned int i;

    while (clk--) {
        for (i = 0; i < 100; i++)
            __asm__ volatile ("nop");
    }
}

/*void jump(uint32_t addr){
	((void (*)(void))addr)();
}
*/
void jump(uint32_t addr)
{
    __asm__ volatile(
        "bx %0"
        :
        : "r"(addr)
        : "memory"
    );
}
void exec_app(){
	jump(0x00000000);
}
void PMU_DisableUSBDLMode(void)
{
    STRUP_CON0 &= ~RG_USBDL_EN;
    CHR_CON10 = (CHR_CON10 & ~RG_USBDL_SET) | RG_USBDL_RST;
}

void PMU_EnableUSBDLMode(void)
{
    CHR_CON10 &= ~(RG_USBDL_SET | RG_USBDL_RST);
    STRUP_CON0 |= RG_USBDL_EN;
}

void PMU_SetChargerWDTEnabled(int Enabled)
{
    CHR_CON9 |= RG_CHRWDT_WR;
    if (Enabled == 1) CHR_CON9 |= RG_CHRWDT_EN;
    else CHR_CON9 &= ~RG_CHRWDT_EN;

    while(CHR_CON9 & RGS_CHRWDT_OUT) {}

    CHR_CON9 |= CHRWDT_FLAG_WR;
}

void KP_BL_set(int i){
	KP_BL=i;
}
void write_32(uint32_t addr,uint32_t value){
	(*(volatile uint32_t *)(addr))=value;
}
void write_16(uint32_t addr,uint16_t value){
	(*(volatile uint16_t *)(addr)) = value;
}

uint32_t read_32(uint32_t addr)
{
    return (*(volatile uint32_t *)(addr));
}

uint16_t read_16(uint32_t addr)
{
    return (*(volatile uint16_t *)(addr));
}

void write_16_masked(uint32_t addr, uint16_t value, uint16_t mask)
{
    uint16_t tmp = read_16(addr);

    tmp &= ~mask;
    tmp |= value & mask;

    write_16(addr, tmp);
}
void write_32_masked(uint32_t addr, uint32_t value, uint32_t mask)
{
    uint32_t tmp = read_32(addr);

    tmp &= ~mask;
    tmp |= value & mask;

    write_32(addr, tmp);
}
/*
void backlight(uint16_t level){
	//level can be set from 0 - 7 where 7 is max brightness.
	ISINKS_CON0 |= 0x0001;
	ISINK0_CON0 |= ((level << 4) +0x9);
}
*/
void backlight(uint16_t level)
{
    if (level > 7)
        level = 7;

    ISINKS_CON0 |= 0x0001;

    ISINK0_CON0 = (ISINK0_CON0 & ~0x007F) | ((level << 4) | 0x9);
}

void backlight_off(void)
{
    ISINK0_CON0 &= ~0x007F;  // Clear current level and configuration bits
    ISINKS_CON0 &= ~0x0001;  // Disable ISINK0
}

int kp_read_int(void)
{
    int value = 0;
    int digit;

    for (;;) {
        digit = -1;

        if (KP_0 == 0) digit = 0;
        if (KP_1 == 0) digit = 1;
        if (KP_2 == 0) digit = 2;
        if (KP_3 == 0) digit = 3;
        if (KP_4 == 0) digit = 4;
        if (KP_5 == 0) digit = 5;
        if (KP_6 == 0) digit = 6;
        if (KP_7 == 0) digit = 7;
        if (KP_8 == 0) digit = 8;
        if (KP_9 == 0) digit = 9;

        if (digit >= 0) {
            value = value * 10 + digit;

            while (
                KP_0 == 0 || KP_1 == 0 ||
                KP_2 == 0 || KP_3 == 0 ||
                KP_4 == 0 || KP_5 == 0 ||
                KP_6 == 0 || KP_7 == 0 ||
                KP_8 == 0 || KP_9 == 0
            );
        }

        if (KP_OK == 0)
            return value;
	watchdog_feed();
    }
}
