
#ifndef DEFINES_H
#define DEFINES_H

//KEYPAD STUFF

#define KP_BL (*(volatile uint32_t *) (0xA0700000 + 0x0C80))
#define KP_BL_ON 3
#define KP_BL_OFF 0
#define KP_STATUS (*(volatile uint32_t *) ( 0xA00D0000))

#define KP_MEM1 (*(volatile uint32_t *) (0xa00d0004))
#define KP_MEM2 (*(volatile uint32_t *) (0xa00d0008))
#define KP_MEM3 (*(volatile uint32_t *) (0xa00d000C))
#define KP_MEM4 (*(volatile uint32_t *) (0xa00d0010))
#define KP_MEM5 (*(volatile uint32_t *) (0xa00d0014))

#define KP_STAR (KP_MEM1&1)
#define KP_ZERO ((KP_MEM1>>12)&0x1)
#define KP_HASH ((KP_MEM3 >> 10)&0x1)
#define KP_TAKE ((KP_MEM3 >> 4)&0x1)
#define KP_RIGHT ((KP_MEM3 >> 1)&0x1)
#define KP_LEFT ((KP_MEM2 >> 14)&0x1)
#define KP_OK ((KP_MEM4 >> 9)&0x1)
#define KP_CENTER_UP ((KP_MEM3 >> 13)&0x1)
#define KP_CENTER_DOWN ((KP_MEM4)&0x1)
#define KP_CENTER_LEFT ((KP_MEM4 >> 3)&0x1)
#define KP_CENTER_RIGHT ((KP_MEM4 >> 6)&0x1)
#define KP_1 ((KP_MEM1 >> 15)&0x1)
#define KP_2 ((KP_MEM2 >> 2)&0x1)
#define KP_3 ((KP_MEM2 >> 5)&0x1)
#define KP_4 ((KP_MEM2 >> 8)&0x1)
#define KP_5 ((KP_MEM2 >> 11)&0x1)
#define KP_6 ((KP_MEM3 >> 7)&0x1)
#define KP_7 ((KP_MEM1 >> 9)&0x1)
#define KP_8 ((KP_MEM1 >> 3)&0x1)
#define KP_9 ((KP_MEM1 >> 6)&0x1)

//END OF KEYPAD STUFF
//wdt timer
#define WDT_TIMER (*(volatile uint32_t *) (0xa003001c))
#define WDT_CTRL (*(volatile uint32_t *) (0xa0030000))
#define WDT_DISABLE 0x2200
#define WDT_RST 0x1209
//END OF WDT TIMER
//PMIC
#define PMIC_ADDR 0xa0700a00
#define PMIC_CTRL0 (*(volatile uint32_t *) (PMIC_ADDR + 0x00))
#define PMIC_CTRL1 (*(volatile uint32_t *)(PMIC_ADDR + 0x04))
#define PMIC_CTRL2 (*(volatile uint32_t *)(PMIC_ADDR + 0x08))
#define PMIC_CTRL3 (*(volatile uint32_t *)(PMIC_ADDR + 0x0c))
#define PMIC_CTRL4 (*(volatile uint32_t *)(PMIC_ADDR + 0x10))
#define PMIC_CTRL5 (*(volatile uint32_t *)(PMIC_ADDR + 0x14))
#define PMIC_CTRL6 (*(volatile uint32_t *)(PMIC_ADDR + 0x18))
#define PMIC_CTRL7 (*(volatile uint32_t *)(PMIC_ADDR + 0x1c))
#define PMIC_CTRL8 (*(volatile uint32_t *)(PMIC_ADDR + 0x20))
#define PMIC_CTRL9 (*(volatile uint32_t *)(PMIC_ADDR + 0x24))
#define PMIC_CTRL10 (*(volatile uint32_t *)(PMIC_ADDR + 0x28))
//END OF PMIC
//PMU
#define PMU_BASE  0xA0700000
//END OF PMU
//VIBRATION MOTOR
#define VIBR_CON0 (*(volatile uint16_t *)(PMU_BASE + 0x01B0))
#define RG_VIBR_EN                  (1 << 0)
#define VIBR_ON_SEL                 (1 << 1)
#define RG_VIBR_VOSEL(v)            (((v) & 0x03) << 4)
//CHARGER STUFF
#define CHR_CON10                   (*(volatile uint16_t *)(PMU_BASE + 0x0A28))
#define RG_USBDL_EN                 (1 << 4)
#define RG_USBDL_RST                (1 << 14)                                                       // Force leave USBDL_MODE
#define RG_USBDL_SET                (1 << 15)
#define STRUP_CON0                  (*(volatile uint16_t *)(PMU_BASE + 0x0A80))

#define CHR_CON9                    (*(volatile uint16_t *)(PMU_BASE + 0x0A24))
#define RG_CHRWDT_WR                (1 << 5)
#define RG_CHRWDT_EN                (1 << 4)
#define RGS_CHRWDT_OUT              (1 << 15)
#define CHRWDT_FLAG_WR              (1 << 9)
//end of CHARGER AND VIBR
//USTIMER

#define TOPSM_BASE                  0xA0180000
#define USC_FREQUENCY               1000000

#define USCNT_CON                   (*(volatile uint32_t *)(TOPSM_BASE + 0x0080))
#define USC_START                   (1 << 0)
#define USC_STOP                    (0 << 0)
#define USC_KEY                     (0x1153 << 16)
#define F32K_FM                     (*(volatile uint32_t *)(TOPSM_BASE + 0x0084))
#define USCNTI_VAL                  (*(volatile uint32_t *)(TOPSM_BASE + 0x0088))

#define USCNT_START_CODE            (USC_KEY | USC_START)
#define USCNT_STOP_CODE             (USC_KEY | USC_STOP)
//end of ustimer
//PLLS adopted from the fernly project
#define PLL_CTRL_ADDR 0xa0170000
#define PLL_CLK_ADDR 0x80000100

#define PLL_CTRL_XOSC_CON0 (PLL_CTRL_ADDR + 0x00)
#define PLL_CTRL_XOSC_CON1 (PLL_CTRL_ADDR + 0x04)

#define PLL_CTRL_CLKSQ_CON0 (PLL_CTRL_ADDR + 0x20)
#define PLL_CTRL_CLKSQ_CON1 (PLL_CTRL_ADDR + 0x24)
#define PLL_CTRL_CLKSQ_CON2 (PLL_CTRL_ADDR + 0x28)

#define PLL_CTRL_CON0 (PLL_CTRL_ADDR + 0x40)
#define PLL_CTRL_CON1 (PLL_CTRL_ADDR + 0x44)
#define PLL_CTRL_CON2 (PLL_CTRL_ADDR + 0x48)
#define PLL_CTRL_CON3 (PLL_CTRL_ADDR + 0x4c)
#define PLL_CTRL_CON4 (PLL_CTRL_ADDR + 0x50)
#define PLL_CTRL_CON5 (PLL_CTRL_ADDR + 0x54)
#define PLL_CTRL_CON6 (PLL_CTRL_ADDR + 0x58)
#define PLL_CTRL_CON7 (PLL_CTRL_ADDR + 0x5c)
#define PLL_CTRL_CON8 (PLL_CTRL_ADDR + 0x5c)
#define PLL_CTRL_CON9 (PLL_CTRL_ADDR + 0x5c)
#define PLL_CTRL_CON10 (PLL_CTRL_ADDR + 0x5c)
#define PLL_CTRL_CON11 (PLL_CTRL_ADDR + 0x5c)

#define PLL_CTRL_DPM_CON0 (PLL_CTRL_ADDR + 0x90)
#define PLL_CTRL_DPM_CON1 (PLL_CTRL_ADDR + 0x94)
#define PLL_CTRL_DPM_CON2 (PLL_CTRL_ADDR + 0x98)

#define PLL_CTRL_MPLL_CON0 (PLL_CTRL_ADDR + 0x100)
#define PLL_CTRL_MPLL_CON1 (PLL_CTRL_ADDR + 0x104)
#define PLL_CTRL_MPLL_CON2 (PLL_CTRL_ADDR + 0x108)

#define PLL_CTRL_UPLL_CON0 (PLL_CTRL_ADDR + 0x140)
#define PLL_CTRL_UPLL_CON1 (PLL_CTRL_ADDR + 0x144)
#define PLL_CTRL_UPLL_CON2 (PLL_CTRL_ADDR + 0x148)

#define PLL_CTRL_EPLL_CON0 (PLL_CTRL_ADDR + 0x180)
#define PLL_CTRL_EPLL_CON1 (PLL_CTRL_ADDR + 0x184)
#define PLL_CTRL_EPLL_CON2 (PLL_CTRL_ADDR + 0x188)

#define PLL_CTRL_FH_CON0 (PLL_CTRL_ADDR + 0x500)
#define PLL_CTRL_FH_CON1 (PLL_CTRL_ADDR + 0x504)
#define PLL_CTRL_FH_CON2 (PLL_CTRL_ADDR + 0x508)
#define PLL_CTRL_FH_CON3 (PLL_CTRL_ADDR + 0x50c)
#define PLL_CTRL_FH_CON4 (PLL_CTRL_ADDR + 0x510)

#define PLL_CTRL_MDDS_CON0 (PLL_CTRL_ADDR + 0x640)
#define PLL_CTRL_MDDS_CON1 (PLL_CTRL_ADDR + 0x644)
#define PLL_CTRL_MDDS_CON2 (PLL_CTRL_ADDR + 0x648)

#define PLL_CTRL_EDDS_CON0 (PLL_CTRL_ADDR + 0x680)
#define PLL_CTRL_EDDS_CON1 (PLL_CTRL_ADDR + 0x684)
#define PLL_CTRL_EDDS_CON2 (PLL_CTRL_ADDR + 0x688)

#define PLL_CTRL_CLK_CONDA (PLL_CLK_ADDR + 0x00)
#define PLL_CTRL_CLK_CONDB (PLL_CLK_ADDR + 0x04)
#define PLL_CTRL_CLK_CONDC (PLL_CLK_ADDR + 0x08)
#define PLL_CTRL_CLK_CONDD (PLL_CLK_ADDR + 0x0c)
#define PLL_CTRL_CLK_CONDE (PLL_CLK_ADDR + 0x10)
#define PLL_CTRL_CLK_CONDF (PLL_CLK_ADDR + 0x14)
#define PLL_CTRL_CLK_CONDG (PLL_CLK_ADDR + 0x18)
#define PLL_CTRL_CLK_CONDH (PLL_CLK_ADDR + 0x1c)
//end PLLS
//EMI also adopted from Fernly project
#define EMI_CTRL_ADDR 0xa0050000
#define EMI_CTRL_CONA (EMI_CTRL_ADDR + 0x00) /* Bank 0 control (low) */
#define EMI_CTRL_CONB (EMI_CTRL_ADDR + 0x08) /* Bank 1 control (low) */
#define EMI_CTRL_CONC (EMI_CTRL_ADDR + 0x10) /* Bank 2 control (low) */
#define EMI_CTRL_COND (EMI_CTRL_ADDR + 0x18) /* Bank 3 control (low) */
#define EMI_CTRL_CONE (EMI_CTRL_ADDR + 0x20) /* Bank 0 control (high) */
#define EMI_CTRL_CONF (EMI_CTRL_ADDR + 0x28) /* Bank 1 control (high) */
#define EMI_CTRL_CONG (EMI_CTRL_ADDR + 0x30) /* Bank 2 control (high) */
#define EMI_CTRL_CONH (EMI_CTRL_ADDR + 0x38) /* Bank 3 control (high) */
#define EMI_CTRL_CONI (EMI_CTRL_ADDR + 0x40) /* Bank 0 control for mDDR */
#define EMI_CTRL_CONJ (EMI_CTRL_ADDR + 0x48) /* Bank 1 control for mDDR */
#define EMI_CTRL_CONK (EMI_CTRL_ADDR + 0x50) /* Bank 2 control for mDDR */
#define EMI_CTRL_CONL (EMI_CTRL_ADDR + 0x58) /* Bank 3 control for mDDR */
#define EMI_CTRL_CONM (EMI_CTRL_ADDR + 0x60) /* */
#define EMI_CTRL_GENA (EMI_CTRL_ADDR + 0x70) /* General control register 0 */
#define EMI_CTRL_GENB (EMI_CTRL_ADDR + 0x78) /* General control register 1 */
#define EMI_CTRL_ADMUX (EMI_CTRL_ADDR + 0x80) /* Something involving GPIO mux */
#define EMI_CTRL_RDCT (EMI_CTRL_ADDR + 0x88)
#define EMI_CTRL_DLLV (EMI_CTRL_ADDR + 0x90)
#define EMI_CTRL_IDLA (EMI_CTRL_ADDR + 0xc0)
#define EMI_CTRL_IDLB (EMI_CTRL_ADDR + 0xc8)
#define EMI_CTRL_IDLC (EMI_CTRL_ADDR + 0xd0)
#define EMI_CTRL_IDLD (EMI_CTRL_ADDR + 0xd8)
#define EMI_CTRL_IDLE (EMI_CTRL_ADDR + 0xe0)
#define EMI_CTRL_ODLA (EMI_CTRL_ADDR + 0xe8)
#define EMI_CTRL_ODLB (EMI_CTRL_ADDR + 0xf0)
#define EMI_CTRL_ODLC (EMI_CTRL_ADDR + 0xf8)
#define EMI_CTRL_ODLD (EMI_CTRL_ADDR + 0x100)
#define EMI_CTRL_ODLE (EMI_CTRL_ADDR + 0x108)
#define EMI_CTRL_ODLF (EMI_CTRL_ADDR + 0x110)
#define EMI_CTRL_IOA (EMI_CTRL_ADDR + 0x130)
#define EMI_CTRL_IOB (EMI_CTRL_ADDR + 0x138)
#define EMI_CTRL_DSRAM (EMI_CTRL_ADDR + 0x150)
#define EMI_CTRL_ARBA (EMI_CTRL_ADDR + 0x170)
#define EMI_CTRL_ARBB (EMI_CTRL_ADDR + 0x178)
#define EMI_CTRL_ARBC (EMI_CTRL_ADDR + 0x180)
#define EMI_CTRL_SLCT (EMI_CTRL_ADDR + 0x198)
#define EMI_CTRL_ABCT (EMI_CTRL_ADDR + 0x1a0)
#define EMI_CTRL_BMEN (EMI_CTRL_ADDR + 0x200)
#define EMI_CTRL_BCNT (EMI_CTRL_ADDR + 0x208)
#define EMI_CTRL_TACT (EMI_CTRL_ADDR + 0x210)
#define EMI_CTRL_TSCT (EMI_CTRL_ADDR + 0x218)
#define EMI_CTRL_WACT (EMI_CTRL_ADDR + 0x220)
#define EMI_CTRL_WSCT (EMI_CTRL_ADDR + 0x228)
#define EMI_CTRL_BACT (EMI_CTRL_ADDR + 0x230)
#define EMI_CTRL_BSCT0 (EMI_CTRL_ADDR + 0x238)
#define EMI_CTRL_BSCT1 (EMI_CTRL_ADDR + 0x240)
#define EMI_CTRL_TTYPE1 (EMI_CTRL_ADDR + 0x280)
#define EMI_CTRL_TTYPE2 (EMI_CTRL_ADDR + 0x288)
#define EMI_CTRL_TTYPE3 (EMI_CTRL_ADDR + 0x290)
#define EMI_CTRL_TTYPE4 (EMI_CTRL_ADDR + 0x298)
#define EMI_CTRL_TTYPE5 (EMI_CTRL_ADDR + 0x2a0)
#define EMI_CTRL_TTYPE6 (EMI_CTRL_ADDR + 0x2a8)
#define EMI_CTRL_TTYPE7 (EMI_CTRL_ADDR + 0x2b0)
#define EMI_CTRL_TTYPE8 (EMI_CTRL_ADDR + 0x2b8)
#define EMI_CTRL_TTYPE9 (EMI_CTRL_ADDR + 0x2c0)
#define EMI_CTRL_TTYPE10 (EMI_CTRL_ADDR + 0x2c8)
#define EMI_CTRL_TTYPE11 (EMI_CTRL_ADDR + 0x2d0)
#define EMI_CTRL_TTYPE12 (EMI_CTRL_ADDR + 0x2d8)
#define EMI_CTRL_TTYPE13 (EMI_CTRL_ADDR + 0x2e0)
#define EMI_CTRL_TTYPE14 (EMI_CTRL_ADDR + 0x2e8)
#define EMI_CTRL_TTYPE15 (EMI_CTRL_ADDR + 0x2f0)
#define EMI_CTRL_TTYPE16 (EMI_CTRL_ADDR + 0x2f8)

#define EMI_CTRL_MBISTA (EMI_CTRL_ADDR + 0x300)
#define EMI_CTRL_MBISTA_PATTERN_SHIFT 16
#define EMI_CTRL_MBISTA_PATTERN_MASK 0xffff0000

#define EMI_CTRL_MBISTB (EMI_CTRL_ADDR + 0x308)
#define EMI_CTRL_MBISTB_START_KEY 0x325
#define EMI_CTRL_MBISTB_RANDOMIZE (1 << 12)
#define EMI_CTRL_MBISTB_INVERT (1 << 13)

#define EMI_CTRL_MBISTC (EMI_CTRL_ADDR + 0x310)

#define EMI_CTRL_MBISTD (EMI_CTRL_ADDR + 0x318)
#define EMI_CTRL_MBISTD_FAILURE (1 << 0)
#define EMI_CTRL_MBISTD_FINISHED (1 << 1)



#define EMI_CTRL_TEST (EMI_CTRL_ADDR + 0x330)

#define EMI_CTRL_REMAP (0xA0510000)
//end of EMI
//funcs
extern void PMU_DisableUSBDLMode(void);
void PMU_SetChargerWDTEnabled(int Enabled);
void PMU_EnableUSBDLMode(void);
void sleep(unsigned int clk);
void reboot(void);
void KP_BL_set(int i);

void write_32(uint32_t addr, uint32_t value);
void write_16(uint32_t addr, uint16_t value);
void write_16_masked(uint32_t addr, uint16_t value, uint16_t mask);
void write_32_masked(uint32_t addr, uint32_t value, uint32_t mask);
uint32_t read_32(uint32_t addr);
uint16_t read_16(uint32_t addr);
void set_plls(void);
void enable_psram(void);
#endif
