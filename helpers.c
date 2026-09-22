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
