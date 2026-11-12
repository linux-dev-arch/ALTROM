
#include <stdint.h>

#define RGU_BASE 0xA0030000

#define WDT_MODE \
    (*(volatile uint16_t *)(RGU_BASE + 0x00))

#define WDT_LENGTH \
    (*(volatile uint16_t *)(RGU_BASE + 0x04))

#define WDT_RESTART \
    (*(volatile uint16_t *)(RGU_BASE + 0x08))

#define WDT_STA \
    (*(volatile uint16_t *)(RGU_BASE + 0x0C))

#define WDT_SWRST \
    (*(volatile uint16_t *)(RGU_BASE + 0x1C))

#define WDMKEY  0x2200
#define WDLKEY  0x0008
#define WDTRKEY 0x1971
#define WDSWRKEY 0x1209
#define WDT_ENABLE 0x0001

#define WDT_MAX_SECONDS 31

void watchdog_feed(void)
{
    WDT_RESTART = WDTRKEY;
}

void watchdog_init(uint8_t seconds)
{
    if (seconds > WDT_MAX_SECONDS)
        seconds = WDT_MAX_SECONDS;

    WDT_MODE = WDMKEY;

    WDT_LENGTH = WDLKEY |
        (((seconds * 32768u / 512u) & 0x07FFu) << 5);

    watchdog_feed();

    WDT_MODE = WDMKEY | WDT_ENABLE;
}

void watchdog_reset(void)
{
    WDT_SWRST = WDSWRKEY;

    for (;;);
}
void watchdog_disable(void)
{
    WDT_MODE = WDMKEY;  // Disable watchdog
}
