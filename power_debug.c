/*
 * power_debug.c - find out why power isn't being held
 * Include after fb.c and power.c.  Call power_debug() after lcd_init()
 * INSTEAD of battery_app().
 *
 * Screen (hex, 4 digits each, top to bottom):
 *   1. RTC_BBPU before power_hold()
 *   2. RTC_BBPU after  power_hold()      (bit 2 = 0004 should be set)
 *   3. PMU CHR_CON0   (bit 5 = 0020 = charger present)
 *   4. PMU STRUP_CON0
 *   5. live counter - keeps counting if the battery is holding the phone up
 *
 * Keys: OK    = try the alternative hold (PWREN|BBPU|AUTO)
 *       LEFT  = release the hold (phone should power off when USB is out)
 *       RIGHT = exit,  # = reboot
 */

#define BCHR_CON0_V (*(volatile uint16_t *)(PMU_BASE + 0x0A00))

static const uint16_t hexfont[16] = {
	0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7249,
	0x7BEF, 0x7BCF, 0x2BED, 0x6BAE, 0x3923, 0x6B6E, 0x79A7, 0x79A4
};

static void dbg_hexdigit(int x, int y, int d, int s, uint16_t c)
{
	uint16_t g = hexfont[d & 15];

	for (int r = 0; r < 5; r++)
		for (int k = 0; k < 3; k++) {
			uint16_t col = (g & (1 << (14 - (r * 3 + k)))) ? c : COL_BLACK;
			for (int j = 0; j < s; j++)
				for (int i = 0; i < s; i++)
					fb_pixel(x + k * s + i, y + r * s + j, col);
		}
}

static void dbg_hex16(int x, int y, uint16_t v, uint16_t c)
{
	for (int n = 0; n < 4; n++)
		dbg_hexdigit(x + n * 16, y, (v >> (12 - 4 * n)) & 15, 4, c);
}

void power_debug(void)
{
	uint16_t before = RTC_BBPU_REG;
	power_hold();
	uint16_t after = RTC_BBPU_REG;
	uint16_t count = 0;

	fb_clear(COL_BLACK);
	dbg_hex16(10,  20, before,       COL_WHITE);
	dbg_hex16(10,  60, after,        COL_GREEN);
	dbg_hex16(10, 100, BCHR_CON0_V,  COL_BLUE);
	dbg_hex16(10, 140, STRUP_CON0,   0xFFE0);
	fb_flush_rect(0, 0, LCD_W, 170);

	for (;;) {
		dbg_hex16(10, 190, count++, COL_RED);
		fb_flush_rect(0, 180, 100, 40);

		sleep(20000);

		if (KP_OK == 0) {
			/* alternative: also set PWREN */
			RTC_BBPU_REG = BBPU_KEY | BBPU_PWREN | BBPU_BBPU | BBPU_AUTO;
			rtc_commit();
			dbg_hex16(10, 60, RTC_BBPU_REG, COL_GREEN);
			fb_flush_rect(0, 50, 100, 30);
		}
		if (KP_LEFT == 0)  power_release();
		if (KP_HASH == 0)  reboot();
		if (KP_RIGHT == 0) break;
	}
}
