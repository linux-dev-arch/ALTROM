/*
 * fb.c - software framebuffer for ALTROM (RGB565, 240x320)
 *
 * Include after lcd.c (uses lcd_cmd/lcd_dat/LCD_W/LCD_H from it).
 *
 * The buffer lives in PSRAM, NOT in a C array: the 8 KB image limit in
 * simple.ld counts .bss, so a 150 KB array would overflow it.
 * 0x003DA800 is clear of psram_test(), which only touches the first 12 bytes.
 */

//#define FB_ADDR   0x00100000
#define FB_ADDR   0x003DA800
#define FB        ((volatile uint16_t *)FB_ADDR)

/* 8-bit r,g,b -> RGB565 */
#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

#define COL_BLACK 0x0000
#define COL_WHITE 0xFFFF
#define COL_RED   0xF800
#define COL_GREEN 0x07E0
#define COL_BLUE  0x001F
#define COL_YELLOW 0xffc0
#define COL_VIOLET 0xa816
#define COL_CYAN 0x07ff
#define COL_TEAL 0x07f3
/* set the panel's RAM write window and start a RAMWR */
static void lcd_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	lcd_cmd(0x2A);
	lcd_dat(x0 >> 8); lcd_dat(x0 & 0xFF); lcd_dat(x1 >> 8); lcd_dat(x1 & 0xFF);
	lcd_cmd(0x2B);
	lcd_dat(y0 >> 8); lcd_dat(y0 & 0xFF); lcd_dat(y1 >> 8); lcd_dat(y1 & 0xFF);
	lcd_cmd(0x2C);
}

/* ---- framebuffer (RAM only, nothing shows until fb_flush) ---- */

void fb_clear(uint16_t color)
{
	for (uint32_t i = 0; i < (uint32_t)LCD_W * LCD_H; i++)
		FB[i] = color;
}

void fb_pixel(int x, int y, uint16_t color)
{
	if ((unsigned)x < LCD_W && (unsigned)y < LCD_H)
		FB[y * LCD_W + x] = color;
}

uint16_t fb_get(int x, int y)
{
	if ((unsigned)x < LCD_W && (unsigned)y < LCD_H)
		return FB[y * LCD_W + x];
	return 0;
}

/* push a rectangle of the framebuffer to the panel */
void fb_flush_rect(int x, int y, int w, int h)
{
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > LCD_W) w = LCD_W - x;
	if (y + h > LCD_H) h = LCD_H - y;
	if (w <= 0 || h <= 0) return;

	lcd_window(x, y, x + w - 1, y + h - 1);
	for (int row = 0; row < h; row++) {
		volatile uint16_t *p = &FB[(y + row) * LCD_W + x];
		for (int col = 0; col < w; col++) {
			uint16_t c = p[col];
			lcd_dat(c >> 8);
			lcd_dat(c & 0xFF);
		}
	}
}

void fb_flush(void)
{
	fb_flush_rect(0, 0, LCD_W, LCD_H);
}

/* ---- direct write: one pixel straight to the panel, bypasses the buffer ---- */

void lcd_pixel(int x, int y, uint16_t color)
{
	if ((unsigned)x >= LCD_W || (unsigned)y >= LCD_H)
		return;
	lcd_window(x, y, x, y);
	lcd_dat(color >> 8);
	lcd_dat(color & 0xFF);
}
void fb_rect(int x, int y, int width, int height, uint16_t color)
{
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            fb_pixel(x + px, y + py, color);
        }
    }
}

void fb_rect_outline(int x, int y, int width, int height,
                     uint16_t color)
{
    for (int px = 0; px < width; px++) {
        fb_pixel(x + px, y, color);
        fb_pixel(x + px, y + height - 1, color);
    }

    for (int py = 0; py < height; py++) {
        fb_pixel(x, y + py, color);
        fb_pixel(x + width - 1, y + py, color);
    }
}
