/*
 * battery.c - battery reading, charger detect/enable, demo screen for ALTROM
 * Include after fb.c.  Register layout and constants from the DZ09 project
 * (AUXADC driver + PMU charger code).
 *
 *   battery_init();                 power up + reset the AUXADC
 *   battery_read_mv();              battery voltage in mV
 *   battery_percent(mv);            rough 0..100 from the voltage
 *   charger_connected();            1 if a charger/USB is plugged in
 *   charger_start() / _stop();      OPT-IN, see warning below
 *   battery_app();                  on-screen demo (voltage, %, charger)
 */

/* ---- AUXADC ---- */
#define ADCR(off)      (*(volatile uint16_t *)(0xA0790000 + (off)))
#define ADC_CON0       ADCR(0x00)
#define ADC_CON1       ADCR(0x04)   /* write 1 to a channel bit = start sample */
#define ADC_CON2       ADCR(0x08)
#define ADC_CON3       ADCR(0x0C)   /* bit0 = busy, bit7 = soft reset          */
#define ADC_DAT(ch)    ADCR(0x10 + 4 * (ch))
#define ADC_CH_VBAT    0

/* AUXADC bus clock lives in the ANALOG config block (DZ09: PCTL_PowerUp(0x32)
 * -> ACFG_CLK_CON_CLR bit 2), not in the digital power-down registers */
#define ACFG_CLK_CON_CLR (*(volatile uint16_t *)(0xA0730010))

/* ---- charger registers (PMU_BASE is in defines.h) ---- */
#define PMU16(off)     (*(volatile uint16_t *)(PMU_BASE + (off)))
#define BCHR_CON0      PMU16(0x0A00)
#define BCHR_CON1      PMU16(0x0A04)
#define BCHR_CON2      PMU16(0x0A08)
#define BCHR_CON4      PMU16(0x0A10)
#define BCHR_CON5      PMU16(0x0A14)
#define BCHR_CON10     PMU16(0x0A28)
#define BCHR_CON11     PMU16(0x0A2C)
#define BCHR_CON12     PMU16(0x0A30)
#define BCHR_CON15     PMU16(0x0A3C)

#define BCHR_DET       (1 << 5)     /* CHR_CON0: charger present */

/* charge current: 300 mA (0x0D). Conservative for a ~1000 mAh cell. */
#define BAT_CHARGE_CS  0x0D
#define BAT_CHARGE_USB 0x28         /* DZ09 value paired with 300 mA */

/* voltage -> percent mapping, tune for your cell */
#define BAT_EMPTY_MV   3300
#define BAT_FULL_MV    4150

static uint16_t adc_sample(unsigned ch)
{
	uint16_t m = 1u << ch;

	ADC_CON1 &= ~m;
	ADC_CON1 |= m;
	for (uint32_t i = 0; i < 200000 && (ADC_CON3 & 1); i++) { }
	return ADC_DAT(ch);
}

void battery_init(void)
{
	ACFG_CLK_CON_CLR = 1 << 2;        /* enable AUXADC clock */
	ADC_CON3 = 1 << 7;               /* soft reset      */
	ADC_CON3 &= ~(1 << 7);
	ADC_CON0 = 0;
	ADC_CON1 = 0;
	ADC_CON2 = 0;
	for (uint32_t i = 0; i < 200000 && (ADC_CON3 & 1); i++) { }

	BCHR_CON10 |= (1 << 4);          /* connect VBAT to the ADC input */
}

/* battery voltage in mV (average of 16 samples) */
uint32_t battery_read_mv(void)
{
	uint32_t sum = 0;

	for (int i = 0; i < 16; i++)
		sum += adc_sample(ADC_CH_VBAT);
	sum /= 16;
	return (5600 * sum) / 0x03FF;    /* 10-bit ADC, 5.6 V full scale (DZ09) */
}

int battery_percent(uint32_t mv)
{
	if (mv <= BAT_EMPTY_MV) return 0;
	if (mv >= BAT_FULL_MV)  return 100;
	return (int)((mv - BAT_EMPTY_MV) * 100 / (BAT_FULL_MV - BAT_EMPTY_MV));
}

int charger_connected(void)
{
	return (BCHR_CON0 & BCHR_DET) ? 1 : 0;
}

/*
 * WARNING: this turns on the Li-ion charger with DZ09's settings
 * (4.20 V CV, 300 mA, 4.25 V over-voltage cutoff). Nothing here monitors
 * battery temperature or charge time, so only use it with a healthy,
 * correct 3.7 V Li-ion cell, and watch the first runs.
 */
void charger_start(void)
{
	BCHR_CON15 = BAT_CHARGE_USB;
	BCHR_CON0  = (9 << 12) | (2 << 8);                    /* HV 6.0 V, LV 4.3 V */
	BCHR_CON1  = (0x10 << 8) | (0 << 4);                  /* CV 4.20 V, CC 3.3 V */
	BCHR_CON5  = (BCHR_CON5 & ~(7 << 4)) | (1 << 4) | (1 << 1) | (1 << 0); /* OV 4.25 V */
	BCHR_CON10 |= (1 << 4);
	BCHR_CON12 = (4 << 8) | (1 << 7) | (1 << 6) | (1 << 2);
	BCHR_CON11 = (1 << 11);
	PMU_SetChargerWDTEnabled(0);

	if (!charger_connected())
		return;

	BCHR_CON1  |= (1 << 1) | (1 << 0);                    /* CC + CV on */
	BCHR_CON2   = (1 << 8) | BAT_CHARGE_CS;               /* current sense on, 300 mA */
	BCHR_CON4   = (3 << 12);                              /* DZ09 ramp settings */
	BCHR_CON10 |= (1 << 6) | (1 << 5);
	BCHR_CON0  |= (1 << 4) | (1 << 3);                    /* CHR_EN + CSDAC_EN */
}

void charger_stop(void)
{
	BCHR_CON0  &= ~((1 << 4) | (1 << 3));
	BCHR_CON1  &= ~((1 << 1) | (1 << 0));
	BCHR_CON2  &= (1 << 8);
	BCHR_CON10 &= ~((1 << 6) | (1 << 5));
}

/* ---- demo screen ---- */

static void bat_rect(int x, int y, int w, int h, uint16_t c)
{
	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
			fb_pixel(x + i, y + j, c);
}

/* 3x5 digit font, 15 bits per digit, row-major, MSB first */
static const uint16_t font3x5[10] = {
	0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7249, 0x7BEF, 0x7BCF
};

static void draw_digit(int x, int y, int d, int s, uint16_t c)
{
	uint16_t g = font3x5[d];

	for (int r = 0; r < 5; r++)
		for (int k = 0; k < 3; k++)
			if (g & (1 << (14 - (r * 3 + k))))
				bat_rect(x + k * s, y + r * s, s, s, c);
}

/* print n with at least `digits` digits (leading zeros dropped) */
static void draw_number(int x, int y, uint32_t n, int s, uint16_t c)
{
	char buf[10];
	int len = 0;

	do { buf[len++] = n % 10; n /= 10; } while (n && len < 10);
	for (int i = 0; i < len; i++)
		draw_digit(x + i * (4 * s), y, buf[len - 1 - i], s, c);
}

void battery_app(void)
{
	battery_init();
	/* charger_start();   <- uncomment to also charge (read the warning above) */

	for (;;) {
		uint32_t mv  = battery_read_mv();
		int      pct = battery_percent(mv);
		int      chg = charger_connected();

		fb_clear(COL_BLACK);

		/* battery icon: outline, nub, fill bar */
		bat_rect(20, 30, 124, 54, COL_WHITE);
		bat_rect(24, 34, 116, 46, COL_BLACK);
		bat_rect(144, 45, 8, 24, COL_WHITE);
		uint16_t col = pct > 50 ? COL_GREEN : (pct > 20 ? 0xFFE0 : COL_RED);
		bat_rect(26, 36, pct * 112 / 100, 42, col);

		draw_number(20, 110, mv,  6, COL_WHITE);      /* mV    */
		draw_number(20, 160, pct, 6, col);            /* %     */
		if (chg)
			bat_rect(20, 210, 30, 30, COL_BLUE);      /* blue square = charger plugged in */

		fb_flush_rect(0, 20, LCD_W, 240);

		sleep(20000);

		if (KP_HASH == 0)  reboot();
		if (KP_RIGHT == 0) break;
	}
}
