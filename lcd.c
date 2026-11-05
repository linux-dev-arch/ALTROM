/*
 * lcd.c - LCD bring-up for Nokia 130 Music (TA-1576, MT6261), ST7789S panel.
 *
 * Included from entry.c like the other .c files (needs defines.h + helpers.c).
 * Sequence and values come from the stock firmware's ST7789S path:
 *   interface setup  : stock 0x2e3f4
 *   panel init       : stock 0x31210
 * LCD controller base / register layout match the DZ09 headers.
 */

#ifndef LCDIF_BASE
#define LCDIF_BASE 0xA0450000
#endif

/* command / data ports (not in defines.h yet) */
#define LCDIF_SCMD0 (*(volatile uint32_t *)(LCDIF_BASE + 0x0F80))
#define LCDIF_SDAT0 (*(volatile uint32_t *)(LCDIF_BASE + 0x0F90))

/* power-down clear register: write 1 = power the block up */
#define CNFG_PDN_CON0_CLR (*(volatile uint16_t *)(0xA0010320))
#define PD_LCD   (1 << 0)
#define PD_SLCD  (1 << 7)

#define LCD_CLK_CONDH 0xA001011C /* bits 6:4 = LCD clock mux, 0 = MPLL/4 */

/* sleep() is a busy loop of unknown length; tune this if the delays look off */
#define LCD_MS(x) sleep((x) * 200)

#define LCD_W 240
#define LCD_H 320

/* GPIO mode: 8 pins per register (0xA0020C00 + pin/8*0x10), 4 bits per pin */
static void lcd_pin_mode(unsigned pin, unsigned mode)
{
	uint32_t addr = 0xA0020C00 + (pin >> 3) * 0x10;
	unsigned s = (pin & 7) * 4;
	write_32_masked(addr, mode << s, 0xFu << s);
}

static void lcd_cmd(uint8_t c) { LCDIF_SCMD0 = c; }
static void lcd_dat(uint8_t d) { LCDIF_SDAT0 = d; }

/* ST7789S init table from stock: cmd, param count, params... ; 0 = end */
static const uint8_t st7789_init[] = {
	0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,   /* PORCTRL            */
	0x35, 1, 0x00,                           /* TEON               */
	0x44, 2, 0x00, 0x48,                     /* SETTSCAN           */
	0x36, 1, 0x00,                           /* MADCTL             */
	0x3A, 1, 0x05,                           /* COLMOD: RGB565     */
	0xB7, 1, 0x35,                           /* GCTRL              */
	0xBB, 1, 0x20,                           /* VCOMS              */
	0xC0, 1, 0x2C,                           /* LCMCTRL            */
	0xC2, 1, 0x01,                           /* VDVVRHEN           */
	0xC3, 1, 0x00,                           /* VRHS               */
	0xC6, 1, 0x0F,                           /* FRCTRL2            */
	0xD0, 1, 0xA7,                           /* PWCTRL1 (1st)      */
	0xD0, 2, 0xA4, 0xA1,                     /* PWCTRL1 (2nd)      */
	0xD6, 1, 0xA5,
	0xE0, 14, 0xD0, 0x00, 0x02, 0x09, 0x0B, 0x2B, 0x29,
	          0x54, 0x3E, 0x3F, 0x18, 0x11, 0x10, 0x14,   /* PVGAMCTRL */
	0xE1, 14, 0xD0, 0x00, 0x00, 0x03, 0x00, 0x20, 0x29,
	          0x55, 0x3F, 0x07, 0x14, 0x13, 0x13, 0x19,   /* NVGAMCTRL */
	0x29, 0,                                 /* DISPON             */
	0
};

static void lcd_hw_init(void)
{
	/* 1. pins 45..48 = reset, chip select, clock, data -> LCD interface */
	for (unsigned p = 45; p <= 48; p++)
		lcd_pin_mode(p, 1);

	/* 2. power up LCD controller + serial LCD interface, clock = MPLL/4 */
	CNFG_PDN_CON0_CLR = PD_LCD | PD_SLCD;
	write_32_masked(LCD_CLK_CONDH, 0, 0x70);

	/* 3. controller internal reset, interrupts off */
	LCDIF_START = LCDIF_INT_RESET;
	LCDIF_START = 0;
	LCDIF_INTEN = 0;

	/* 4. serial interface config (stock ST7789S setup) */
	{
		uint16_t pix = read_16(LCDIF_BASE + 0x18);
		pix |= 0x10;
		pix &= ~0x07;
		pix |= 0x02;
		pix &= ~0xC0;
		write_16(LCDIF_BASE + 0x18, pix);
	}
	/* timing: CSS=2 CSH=2 RD_1ST=15 RD_2ND=15 WR_1ST=1 WR_2ND=1 */
	write_32_masked(LCDIF_BASE + 0x1C, 0x0022FF11, 0x00FFFFFF);
	/* 3-wire, hardware chip select, 8-bit transfers */
	{
		uint32_t con = LCDIF_SIF_CON;
		con |= LCDIF_SIF0_3WIRE;
		con &= ~LCDIF_SIF0_SDI;
		con |= LCDIF_SIF0_HW_CS;
		con &= ~0x07u;
		LCDIF_SIF_CON = con;
	}
}

void lcd_init(void)
{
	const uint8_t *t = st7789_init;

	lcd_hw_init();

	/* panel hardware reset: high 200ms, low 50ms, high 120ms (as stock) */
	LCDIF_RSTB = LCDIF_RESET1; LCD_MS(200);
	LCDIF_RSTB = LCDIF_RESET0; LCD_MS(50);
	LCDIF_RSTB = LCDIF_RESET1; LCD_MS(120);

	lcd_cmd(0x11);                 /* SLPOUT */
	LCD_MS(120);

	while (*t) {
		uint8_t cmd = *t++;
		uint8_t n = *t++;
		lcd_cmd(cmd);
		while (n--)
			lcd_dat(*t++);
	}
}

/* quick visual test: fill the whole RAM window with one RGB565 colour */
void lcd_fill(uint16_t color)
{
	lcd_cmd(0x2A);                 /* CASET 0..239 */
	lcd_dat(0); lcd_dat(0); lcd_dat((LCD_W - 1) >> 8); lcd_dat((LCD_W - 1) & 0xFF);
	lcd_cmd(0x2B);                 /* RASET 0..319 */
	lcd_dat(0); lcd_dat(0); lcd_dat((LCD_H - 1) >> 8); lcd_dat((LCD_H - 1) & 0xFF);
	lcd_cmd(0x2C);                 /* RAMWR */
	for (uint32_t i = 0; i < (uint32_t)LCD_W * LCD_H; i++) {
		lcd_dat(color >> 8);
		lcd_dat(color & 0xFF);
	}
}
