void brightness_change(void)
{
    int bright = 4;
    for (;;) {
	watchdog_feed();
	fb_clear(COL_BLACK);
	draw_string8(10, 130, "BRIGHTNESS", COL_GREEN, COL_BLACK, 0, 0);
	draw_string8(10, 160, "UP/DOWN TO CHANGE", COL_GREEN, COL_BLACK, 0, 0);
	draw_string8(10, 190, "RIGHT TO EXIT", COL_GREEN, COL_BLACK, 0, 0);
	draw_string8(10,220,"LEVEL:",COL_GREEN,COL_BLACK,0,0);
        draw_int(75, 220, bright, COL_GREEN, COL_BLACK);
        fb_flush();

        if (KP_CENTER_UP == 0) {
            if (bright < 7) {
                bright++;
                backlight(bright);
            }
            sleep(10000);
        }

        if (KP_CENTER_DOWN == 0) {
            if (bright > 0) {
                bright--;
                backlight(bright);
            }
            sleep(10000);
        }

        if (KP_LEFT == 0) {
            break;
        }
    }
}
