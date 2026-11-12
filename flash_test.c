/*
 * flash_test.c - on-device rehearsal for flash.c.
 * Include after lcd.c, fb.c and flash.c.  Call flash_rehearsal() from your menu.
 *
 * Deliberately narrow: this ONLY ever touches 0x300000, a fixed address confirmed
 * erased (0xFF) in the factory dump and far from SF_BOOT/BL1/EXT_BL/MAUI/the resource
 * file. There is no "enter an address" screen here - that is the whole point: nothing
 * typed or mis-pressed can make this test touch a sector that matters.
 *
 * Requires OK to be pressed twice (with a release in between) before anything is
 * erased, so it can't fire from a stray or stuck key.
 */

#define FLASH_TEST_ADDR 0x300000u

/* small self-contained 3x5 font: 0-9, A-Z, space, + ( ) . : -   kept independent of fat.c/menu.c
 * on purpose, so this file never silently depends on which other files happen to be linked in */
static const uint16_t ft_font[42] = {
	0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7249, 0x7BEF, 0x7BCF,
	0x2BED, 0x6BAE, 0x3923, 0x6B6E, 0x79A7, 0x79A4, 0x396B, 0x5BED, 0x7497, 0x126A,
	0x5BAD, 0x4927, 0x5FED, 0x6B6D, 0x2B6A, 0x6BA4, 0x2B7B, 0x6BAD, 0x388E, 0x7492,
	0x5B6F, 0x5B6A, 0x5BFD, 0x5AAD, 0x5A92, 0x72A7,
	0x0000, 0x05D0, 0x1491, 0x4494, 0x0002, 0x01C0
};
static const char ft_sym[] = " +().-";

static void ft_char(int x, int y, char c, uint16_t col)
{
	uint16_t g = 0;
	int r, k, i, j;

	if (c >= 'a' && c <= 'z') c -= 32;
	if (c >= '0' && c <= '9') g = ft_font[c - '0'];
	else if (c >= 'A' && c <= 'Z') g = ft_font[10 + c - 'A'];
	else
		for (i = 0; ft_sym[i]; i++)
			if (ft_sym[i] == c) { g = ft_font[36 + i]; break; }

	for (r = 0; r < 5; r++)
		for (k = 0; k < 3; k++) {
			uint16_t pc = (g & (1 << (14 - (r * 3 + k)))) ? col : COL_BLACK;
			for (j = 0; j < 2; j++)
				for (i = 0; i < 2; i++)
					fb_pixel(x + k * 2 + i, y + r * 2 + j, pc);
		}
}

static void ft_line(int y, const char *s, uint16_t col)
{
	int x = 6;
	for (; *s; s++, x += 8)
		ft_char(x, y, *s, col);
}

static void FT_HEX(int x, int y, uint32_t v, int digits, uint16_t col)
{
	int k, nib;
	for (k = 0; k < digits; k++) {
		nib = (v >> (4 * (digits - 1 - k))) & 15;
		ft_char(x + k * 8, y, nib < 10 ? (char)('0' + nib) : (char)('A' + nib - 10), col);
	}
}

void flash_rehearsal(void)
{
	uint32_t id, fail_at;
	uint8_t pat[FLASH_SECTOR];
	int i, r;

	fb_clear(COL_BLACK);
	ft_line(6, "FLASH REHEARSAL", 0xFFE0);
	ft_line(24, "ADDR 0x300000 ONLY", COL_WHITE);
	fb_flush_rect(0, 0, LCD_W, 40);

	id = flash_id();
	fb_clear(COL_BLACK);
	ft_line(6, "FLASH REHEARSAL", 0xFFE0);
	ft_line(28, "JEDEC ID", COL_WHITE);
	FT_HEX(90, 28, id, 6, id ? COL_GREEN : COL_RED);
	ft_line(50, "OK TWICE TO ERASE+", COL_WHITE);
	ft_line(64, "WRITE+VERIFY 0x300000", COL_WHITE);
	ft_line(84, "RIGHT TO CANCEL", COL_WHITE);
	fb_flush_rect(0, 0, LCD_W, 100);

	if (!id) {
		ft_line(110, "NO ID. STOPPING HERE", COL_RED);
		fb_flush_rect(0, 100, LCD_W, 20);
		while (KP_RIGHT != 0) { }
		return;
	}

	for (;;) {
		if (KP_RIGHT == 0) return;
		if (KP_OK == 0) break;
	}
	while (KP_OK == 0) { }
	ft_line(110, "PRESS OK AGAIN", 0xFFE0);
	fb_flush_rect(0, 100, LCD_W, 20);
	for (;;) {
		if (KP_RIGHT == 0) return;
		if (KP_OK == 0) break;
	}
	while (KP_OK == 0) { }

	fb_clear(COL_BLACK);
	ft_line(6, "ERASING...", 0xFFE0);
	fb_flush_rect(0, 0, LCD_W, 20);
	r = flash_erase_sector(FLASH_TEST_ADDR);
	fb_clear(COL_BLACK);
	ft_line(6, "ERASE RESULT", 0xFFE0);
	FT_HEX(90, 6, (uint32_t)(r < 0 ? -r : 0), 2, r ? COL_RED : COL_GREEN);
	fb_flush_rect(0, 0, LCD_W, 20);
	if (r) { ft_line(28, "STOPPED: ERASE FAILED", COL_RED); fb_flush_rect(0, 20, LCD_W, 20); while (KP_RIGHT != 0) { } return; }

	for (i = 0; i < (int)FLASH_SECTOR; i++)
		pat[i] = (uint8_t)((i * 37 + 11) ^ (i >> 5));     /* arbitrary, not all-0xFF, not all-0x00 */

	ft_line(28, "PROGRAMMING...", 0xFFE0);
	fb_flush_rect(0, 20, LCD_W, 20);
	fail_at = 0xFFFFFFFFu;
	r = flash_write_sector(FLASH_TEST_ADDR, pat, FLASH_SECTOR, &fail_at);

	fb_clear(COL_BLACK);
	ft_line(6, "WRITE+VERIFY RESULT", 0xFFE0);
	FT_HEX(140, 6, (uint32_t)(r < 0 ? -r : 0), 2, r ? COL_RED : COL_GREEN);
	if (r == -3) { ft_line(24, "MISMATCH AT OFFSET", COL_RED); FT_HEX(6, 38, fail_at, 8, COL_RED); }
	else if (r)  { ft_line(24, "PROGRAM STEP FAILED", COL_RED); }
	else {
		ft_line(24, "PASS. READBACK MATCHES", COL_GREEN);
		/* leave the test sector erased again, since nothing should depend on its content */
		flash_erase_sector(FLASH_TEST_ADDR);
		ft_line(40, "SECTOR ERASED (CLEANUP)", COL_WHITE);
	}
	fb_flush_rect(0, 0, LCD_W, 56);
	while (KP_RIGHT != 0) { }
}
