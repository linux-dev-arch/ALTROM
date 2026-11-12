/*
 * menu.c - test-function menu for ALTROM.
 * Include after lcd.c and fb.c (uses fb_*, lcd_fill, the KP_* macros and sleep()).
 *
 *   typedef struct { const char *name; void (*fn)(void); } menu_item_t;
 *   void menu_run(const menu_item_t *items, int n, const char *title);
 *
 * Build your menu as a const table in entry.c and hand it to menu_run():
 *
 *   static const menu_item_t items[] = {
 *       { "KEYPAD",    test_keys      },
 *       { "COLORS",    test_colors    },
 *       { "BACKLIGHT", test_backlight },
 *       { "VIBRATION", test_vibr      },
 *       { "SD / FAT",  fat_test       },      // any void f(void) works
 *       { "REBOOT",    reboot         },
 *   };
 *   menu_run(items, sizeof items / sizeof items[0], "ALTROM TESTS");
 *
 * Keys in the menu: nav up/down = move, OK = run, right soft key = leave, # = reboot.
 * The built-in tests are static: any you don't list in the table cost no space.
 * All text is 3x5 glyphs drawn 2x: 0-9, A-Z (lowercase shows as uppercase) and  . - _ / : ( ) + * # < >
 */

typedef struct {
	const char *name;
	void (*fn)(void);
} menu_item_t;

#define MN_UNUSED static __attribute__((unused))
#define MN_MS(x)  sleep((x) * 200)      /* sleep() is a busy loop of unknown length: tune by eye */

#define MN_ROW 14
#define MN_TOP 22
#define MN_VIS 20

/* ---- text ---- */

static const uint16_t mn_font[36] = {
	0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7249, 0x7BEF, 0x7BCF,
	0x2BED, 0x6BAE, 0x3923, 0x6B6E, 0x79A7, 0x79A4, 0x396B, 0x5BED, 0x7497, 0x126A,
	0x5BAD, 0x4927, 0x5FED, 0x6B6D, 0x2B6A, 0x6BA4, 0x2B7B, 0x6BAD, 0x388E, 0x7492,
	0x5B6F, 0x5B6A, 0x5BFD, 0x5AAD, 0x5A92, 0x72A7
};
static const char     mn_sym[]  = ".-_/:()+*#<>";
static const uint16_t mn_symg[] = { 0x0002, 0x01C0, 0x0007, 0x12A4, 0x0410, 0x1491,
                                    0x4494, 0x05D0, 0x0AA8, 0x5F7D, 0x1511, 0x4454 };

static void mn_char(int x, int y, char c, uint16_t fg, uint16_t bg)
{
	uint16_t g = 0;

	if (c >= 'a' && c <= 'z') c -= 32;
	if (c >= '0' && c <= '9') g = mn_font[c - '0'];
	else if (c >= 'A' && c <= 'Z') g = mn_font[10 + c - 'A'];
	else
		for (int i = 0; mn_sym[i]; i++)
			if (mn_sym[i] == c) g = mn_symg[i];

	for (int r = 0; r < 5; r++)
		for (int k = 0; k < 3; k++) {
			uint16_t pc = (g & (1 << (14 - (r * 3 + k)))) ? fg : bg;
			for (int j = 0; j < 2; j++)
				for (int i = 0; i < 2; i++)
					fb_pixel(x + k * 2 + i, y + r * 2 + j, pc);
		}
}

static void mn_str(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
	for (; *s; s++, x += 8)
		mn_char(x, y, *s, fg, bg);
}

static void mn_hex(int x, int y, uint32_t v, int digits, uint16_t fg, uint16_t bg)
{
	for (int k = 0; k < digits; k++) {
		int nib = (v >> (4 * (digits - 1 - k))) & 15;
		mn_char(x + k * 8, y, nib < 10 ? '0' + nib : 'A' + nib - 10, fg, bg);
	}
}

static void mn_rect(int x, int y, int w, int h, uint16_t col)
{
	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
			fb_pixel(x + i, y + j, col);
}

/* wait until no navigation key is down */
static void mn_release(void)
{
	while (KP_CENTER_UP == 0 || KP_CENTER_DOWN == 0 || KP_OK == 0 ||
	       KP_RIGHT == 0 || KP_LEFT == 0) { }
}

/* ---- the menu ---- */

static void mn_row(const menu_item_t *it, int row, int sel)
{
	int y = MN_TOP + row * MN_ROW;

	mn_rect(0, y, LCD_W, MN_ROW, sel ? COL_GREEN : COL_BLACK);
	mn_str(6, y + 2, it->name, sel ? COL_BLACK : COL_WHITE, sel ? COL_GREEN : COL_BLACK);
}

void menu_run(const menu_item_t *it, int n, const char *title)
{
	int sel = 0, top = 0, full = 1;

	for (;;) {
		if (full) {
			int rows = n - top < MN_VIS ? n - top : MN_VIS;

			fb_clear(COL_BLACK);
			mn_str(6, 6, title, 0xFFE0, COL_BLACK);
			for (int i = 0; i < rows; i++)
				mn_row(&it[top + i], i, top + i == sel);
			fb_flush();
			full = 0;
		}

		for (;;) {
			int old = sel;

			if (KP_HASH == 0) reboot();
			if (KP_RIGHT == 0) { mn_release(); return; }
			if (KP_OK == 0)    { mn_release(); it[sel].fn(); full = 1; break; }

			if (KP_CENTER_UP == 0)        { mn_release(); if (sel) sel--; }
			else if (KP_CENTER_DOWN == 0) { mn_release(); if (sel < n - 1) sel++; }
			else continue;

			if (sel == old) continue;
			if (sel < top)                 { top = sel; full = 1; }
			else if (sel >= top + MN_VIS)  { top = sel - MN_VIS + 1; full = 1; }
			else {                          /* only two rows changed: redraw just those */
				mn_row(&it[old], old - top, 0);
				mn_row(&it[sel], sel - top, 1);
				fb_flush_rect(0, MN_TOP + (old - top) * MN_ROW, LCD_W, MN_ROW);
				fb_flush_rect(0, MN_TOP + (sel - top) * MN_ROW, LCD_W, MN_ROW);
				continue;
			}
			break;
		}
	}
}

/* ---- built-in tests ---- */

/* raw keypad registers plus the names of every key defined in defines.h that is held down;
 * press * and # together to leave.  An undefined key shows up as a changed bit in the raw rows. */
MN_UNUSED void test_keys(void)
{
	static const struct { const char *n; uint8_t r, b; } k[] = {
		{ "STAR", 1, 0 },  { "0", 1, 12 },   { "HASH", 3, 10 }, { "TAKE", 3, 4 },
		{ "SOFT-R", 3, 1 }, { "SOFT-L", 2, 14 }, { "OK", 4, 9 },
		{ "NAV-UP", 3, 13 }, { "NAV-DN", 4, 0 }, { "NAV-L", 4, 3 }, { "NAV-R", 4, 6 },
		{ "1", 1, 15 }, { "2", 2, 2 }, { "3", 2, 5 }, { "4", 2, 8 }, { "5", 2, 11 },
		{ "6", 3, 7 },  { "7", 1, 9 }, { "8", 1, 3 }, { "9", 1, 6 }
	};
	uint32_t m[5], old[5] = { 0, 0, 0, 0, 0 };
	int first = 1;

	for (;;) {
		int ch = first, x = 6, y = 106;

		m[0] = KP_MEM1; m[1] = KP_MEM2; m[2] = KP_MEM3; m[3] = KP_MEM4; m[4] = KP_MEM5;
		for (int i = 0; i < 5; i++) {
			if (m[i] != old[i]) ch = 1;
			old[i] = m[i];
		}

		if (ch) {
			first = 0;
			fb_clear(COL_BLACK);
			mn_str(6, 6, "KEYPAD", 0xFFE0, COL_BLACK);
			for (int i = 0; i < 5; i++) {
				mn_char(6, 24 + i * 14, 'M', COL_WHITE, COL_BLACK);
				mn_char(14, 24 + i * 14, '1' + i, COL_WHITE, COL_BLACK);
				mn_hex(34, 24 + i * 14, m[i], 8, COL_GREEN, COL_BLACK);
			}
			for (unsigned i = 0; i < sizeof k / sizeof k[0]; i++) {
				if ((m[k[i].r - 1] >> k[i].b) & 1) continue;      /* pressed = bit clear */
				int len = 0;
				while (k[i].n[len]) len++;
				if (x + len * 8 > LCD_W - 8) { x = 6; y += 14; }
				mn_str(x, y, k[i].n, COL_WHITE, COL_BLACK);
				x += (len + 1) * 8;
			}
			mn_str(6, 200, "STAR+HASH EXIT", COL_BLUE, COL_BLACK);
			fb_flush_rect(0, 0, LCD_W, 216);
		}
		if (!(m[0] & 1) && !((m[2] >> 10) & 1)) break;
	}
	while (KP_STAR == 0 || KP_HASH == 0) { }
}

/* full-screen colors: OK = next, right soft key = leave */
MN_UNUSED void test_colors(void)
{
	static const uint16_t c[] = { COL_RED, COL_GREEN, COL_BLUE, COL_WHITE, COL_BLACK };
	int i = 0;

	for (;;) {
		lcd_fill(c[i]);
		while (KP_OK != 0 && KP_RIGHT != 0) { }
		if (KP_RIGHT == 0) { mn_release(); return; }
		mn_release();
		i = (i + 1) % 5;
	}
}

/* backlight level 0-7 or off, written straight to the register (backlight() can only go up) */
MN_UNUSED void test_backlight(void)
{
	int lv = 7;

	for (;;) {
		ISINKS_CON0 |= 0x0001;
		ISINK0_CON0 = lv < 8 ? (uint16_t)((lv << 4) | 0x9) : 0;

		fb_clear(COL_BLACK);
		mn_str(6, 6, "BACKLIGHT", 0xFFE0, COL_BLACK);
		if (lv < 8) mn_hex(6, 30, lv, 1, COL_GREEN, COL_BLACK);
		else        mn_str(6, 30, "OFF", COL_RED, COL_BLACK);
		mn_str(6, 60, "NAV UP/DN  LEVEL", COL_WHITE, COL_BLACK);
		mn_str(6, 74, "SOFT-R     EXIT", COL_WHITE, COL_BLACK);
		fb_flush_rect(0, 0, LCD_W, 90);

		for (;;) {
			if (KP_CENTER_UP == 0)   { mn_release(); if (lv < 8) lv++; break; }
			if (KP_CENTER_DOWN == 0) { mn_release(); if (lv > 0) lv--; break; }
			if (KP_RIGHT == 0) {
				mn_release();
				ISINK0_CON0 = (7 << 4) | 0x9;          /* leave it bright */
				return;
			}
		}
	}
}

/* vibration motor: pick the supply voltage, OK = half-second buzz.  Shows VIBR_CON0 as read
 * back while the motor is on and after it is off: bit 15 (8000) is the LDO status flag. */
MN_UNUSED void test_vibr(void)
{
	static const uint8_t vs[3] = { 0, 2, 3 };
	static const char *const vn[3] = { "1.8V", "2.8V", "3.0V" };
	int v = 1;
	uint16_t on = 0, off = 0;

	for (;;) {
		fb_clear(COL_BLACK);
		mn_str(6, 6, "VIBRATION", 0xFFE0, COL_BLACK);
		mn_str(6, 28, "VOLT", COL_WHITE, COL_BLACK);
		mn_str(54, 28, vn[v], COL_GREEN, COL_BLACK);
		mn_str(6, 48, "ON ", COL_WHITE, COL_BLACK);
		mn_hex(38, 48, on, 4, COL_BLUE, COL_BLACK);
		mn_str(6, 62, "OFF", COL_WHITE, COL_BLACK);
		mn_hex(38, 62, off, 4, COL_BLUE, COL_BLACK);
		mn_str(6, 92, "NAV UP/DN  VOLT", COL_WHITE, COL_BLACK);
		mn_str(6, 106, "OK         BUZZ", COL_WHITE, COL_BLACK);
		mn_str(6, 120, "SOFT-R     EXIT", COL_WHITE, COL_BLACK);
		fb_flush_rect(0, 0, LCD_W, 136);

		for (;;) {
			if (KP_CENTER_UP == 0)   { mn_release(); if (v < 2) v++; break; }
			if (KP_CENTER_DOWN == 0) { mn_release(); if (v > 0) v--; break; }
			if (KP_OK == 0) {
				mn_release();
				VIBR_CON0 = RG_VIBR_VOSEL(vs[v]) | VIBR_ON_SEL | RG_VIBR_EN;
				MN_MS(20);
				on = VIBR_CON0;
				MN_MS(500);
				VIBR_CON0 = RG_VIBR_VOSEL(vs[v]) | VIBR_ON_SEL;
				MN_MS(20);
				off = VIBR_CON0;
				break;
			}
			if (KP_RIGHT == 0) { mn_release(); return; }
		}
	}
}
