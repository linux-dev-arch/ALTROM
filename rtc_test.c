#include "rtc.h"

void rtc_test(void)
{
    RTC_DATETIME dt;
    bool valid;

    char line[32];
    int last_second = -1;
    int last_valid = -1;

    fb_clear(0x0000);

    draw_string12(10, 10, "ALTROM RTC TEST",
                  0xFFFF, 0x0000, 0, 0);

    fb_flush();

    while (1)
    {
        watchdog_feed();

        if (KP_RIGHT == 0)
            break;

        valid = RTC_IsValidState();

        if (valid)
        {
            dt = RTC_GetDateTime();

            if (dt.Time.Sec != last_second || last_valid != 1)
            {
                last_second = dt.Time.Sec;
                last_valid = 1;

                fb_clear(0x0000);

                draw_string12(10, 10, "ALTROM RTC TEST",
                              0xFFFF, 0x0000, 0, 0);

                draw_string8(10, 55, "RTC STATUS:",
                             0xFFFF, 0x0000, 0, 0);

                draw_string8(10, 75, "VALID",
                             0x07E0, 0x0000, 0, 0);

                /* Date */
                line[0] = 'D';
                line[1] = 'A';
                line[2] = 'T';
                line[3] = 'E';
                line[4] = ':';
                line[5] = ' ';

                line[6]  = '0' + dt.Date.Day / 10;
                line[7]  = '0' + dt.Date.Day % 10;
                line[8]  = '/';
                line[9]  = '0' + dt.Date.Month / 10;
                line[10] = '0' + dt.Date.Month % 10;
                line[11] = '/';

                line[12] = '0' + (dt.Date.Year / 1000) % 10;
                line[13] = '0' + (dt.Date.Year / 100) % 10;
                line[14] = '0' + (dt.Date.Year / 10) % 10;
                line[15] = '0' + dt.Date.Year % 10;
                line[16] = '\0';

                draw_string12(10, 115, line,
                              0xFFFF, 0x0000, 0, 0);

                /* Time */
                line[0] = 'T';
                line[1] = 'I';
                line[2] = 'M';
                line[3] = 'E';
                line[4] = ':';
                line[5] = ' ';

                line[6]  = '0' + dt.Time.Hour / 10;
                line[7]  = '0' + dt.Time.Hour % 10;
                line[8]  = ':';
                line[9]  = '0' + dt.Time.Min / 10;
                line[10] = '0' + dt.Time.Min % 10;
                line[11] = ':';
                line[12] = '0' + dt.Time.Sec / 10;
                line[13] = '0' + dt.Time.Sec % 10;
                line[14] = '\0';

                draw_string12(10, 145, line,
                              0xFFFF, 0x0000, 0, 0);

                fb_flush();
            }
        }
        else
        {
            if (last_valid != 0)
            {
                last_valid = 0;
                last_second = -1;

                fb_clear(0x0000);

                draw_string12(10, 10, "ALTROM RTC TEST",
                              0xFFFF, 0x0000, 0, 0);

                draw_string12(10, 75, "RTC INVALID!",
                              0xF800, 0x0000, 0, 0);

                draw_string8(10, 105, "Check power keys",
                             0xFFFF, 0x0000, 0, 0);

                draw_string8(10, 125, "and low-power status",
                             0xFFFF, 0x0000, 0, 0);

                fb_flush();
            }
        }
    }

    fb_clear(0x0000);
    fb_flush();
}

