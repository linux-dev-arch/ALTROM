void keypad_mux_init(){
	//fixing gpio mux
	*(volatile unsigned int *)0xA0020C10 = 0x11111100;
	*(volatile unsigned int *)0xA0020C20 = 0x11111111;
}
void init(void){
        // Disable the digital watchdogs
        WDT_CTRL = WDT_DISABLE;
        PMIC_CTRL9 = 0x2;
	PMU_DisableUSBDLMode();
        PMU_SetChargerWDTEnabled(0);
	keypad_mux_init();
	set_plls();
	enable_psram();
}

void set_plls(void){
    write_16(PLL_CTRL_CON2, 0);
    write_16(PLL_CTRL_CON3, 0);
    write_16(PLL_CTRL_CON0, 0);

    sleep(3);

    write_16_masked(PLL_CTRL_UPLL_CON0, 1, 1);

    write_16(PLL_CTRL_EPLL_CON0, 0x1840);
    write_16_masked(PLL_CTRL_EPLL_CON1, 0x100, 0x100);

    write_16(PLL_CTRL_MDDS_CON0, 1);
    write_16_masked(PLL_CTRL_MPLL_CON0, 1, 1);

    sleep(1);

    write_16(PLL_CTRL_EDDS_CON0, 1);
    write_16_masked(PLL_CTRL_EPLL_CON0, 1, 1);

    sleep(70);

    write_16_masked(PLL_CTRL_CLK_CONDB, 0x4000, 0x4000);

    sleep(1);

    write_32(PLL_CTRL_CLK_CONDC, 0x8048);
    write_32(PLL_CTRL_CLK_CONDH, 0xd002);

    write_32(PLL_CTRL_CLK_CONDC, 0xb6a0);
}

void enable_psram(void)
{
    /* Unclear what these do, but required on some chips */
    write_32(EMI_CTRL_CONB, 0x39320d61);
    write_32_masked(EMI_CTRL_CONF, 0x400, 0x400);

    write_32(EMI_CTRL_GENA, 0x401);

    /* Remap EMI to 0x10000000, and SPI to 0x00000000 */
    write_32(EMI_CTRL_REMAP, 2);

    /* Memory configuration */
    read_16(0x1ffffffe);
    sleep(50);

    read_16(0x1ffffffe);
    sleep(50);

    write_16(0x1ffffffe, 1);
    sleep(50);

    write_16(0x1ffffffe, 0x2b13);
    sleep(50);

    read_16(0x1ffffffe);
    sleep(50);

    read_16(0x1ffffffe);
    sleep(50);

    write_16(0x1ffffffe, 0);
    sleep(50);

    write_16(0x1ffffffe, 0x10);
    sleep(50);

    write_32(EMI_CTRL_CONB, 0xa0000000);
    write_32(EMI_CTRL_CONF, 0xb2024419);
    sleep(50);

    write_32_masked(EMI_CTRL_GENA, 0x400, 0x400);

    /* Now map EMI back to 0x00000000,
       and SPI to 0x10000000 */
    write_32(EMI_CTRL_REMAP, 3);
    sleep(50);

    write_32(EMI_CTRL_RDCT, 0x20004001);
    write_32(EMI_CTRL_DSRAM, 0x5111);

    write_32(EMI_CTRL_IDLA, 0);
    write_32(EMI_CTRL_IDLB, 0);
    write_32(EMI_CTRL_IDLC, 0);
    write_32(EMI_CTRL_IDLD, 0);
    write_32(EMI_CTRL_IDLE, 0);

    write_32(EMI_CTRL_ODLA, 0);
    write_32(EMI_CTRL_ODLB, 0);
    write_32(EMI_CTRL_ODLC, 0);
    write_32(EMI_CTRL_ODLD, 0);
    write_32(EMI_CTRL_ODLE, 0);

    write_32(EMI_CTRL_IOA, 0x00010001);
    write_32(EMI_CTRL_IOB, 0x00010001);

    sleep(50);

    /* Calibrate DQ in delay */

    write_32(EMI_CTRL_DLLV, 0x300f0000);

    /* Wait until bit 7 becomes 1 */
    while ((read_32(EMI_CTRL_DLLV) & 0x80) != 0x80)
        ;

    write_32(EMI_CTRL_DLLV, 0x700f0000);

    /* Wait until bit 7 becomes 0 */
    while ((read_32(EMI_CTRL_DLLV) & 0x80) != 0)
        ;

    write_32(EMI_CTRL_DLLV, 0x100f0000);

    write_32(EMI_CTRL_ARBA, 0x5426);
    write_32(EMI_CTRL_ARBB, 0x5009);
    write_32(EMI_CTRL_ARBC, 0x5051);
}
