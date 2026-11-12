#include <stdint.h>

// Absolute address definitions based on RTC_BASE = 0xA0710000
#define REG_RTC_BBPU        (*(volatile uint16_t *)(0xA0710000))
#define REG_RTC_POWERKEY1   (*(volatile uint16_t *)(0xA0710050))
#define REG_RTC_POWERKEY2   (*(volatile uint16_t *)(0xA0710054))
#define REG_RTC_PROT        (*(volatile uint16_t *)(0xA0710068))
#define REG_RTC_WRTGR       (*(volatile uint16_t *)(0xA0710074))

// Macro bitmasks and magic authorization keys
#define KEY_BBPU            0x4300
#define PWREN               (1 << 0)
#define BBPU                (1 << 2)
#define AUTO                (1 << 3)
#define CBUSY               (1 << 6)
#define WRTGR               (1 << 0)

#define PROT1               0x586A
#define PROT2               0x9136
#define POWERKEY1           0xA357
#define POWERKEY2           0x67D2

// Low-level hardware synchronization helper
static void Inline_RTC_UpdateContext(void)
{
    REG_RTC_WRTGR = WRTGR;
    while (REG_RTC_BBPU & CBUSY) {
        __asm__("nop"); // Wait for transaction completion
    }
}

void Force_Battery_Power_Hold(void)
{
    // 1. Clear the Master Write Protection (RTC_Unprotect)
    REG_RTC_PROT = PROT1;
    Inline_RTC_UpdateContext();
    REG_RTC_PROT = PROT2;
    Inline_RTC_UpdateContext();

    // 2. Set the authorized Power Keys (RTC_SetPowerkey)
    REG_RTC_POWERKEY1 = POWERKEY1;
    REG_RTC_POWERKEY2 = POWERKEY2;
    Inline_RTC_UpdateContext();

    // 3. Force the hardware latch configuration (KEY_BBPU | AUTO | BBPU | PWREN -> 0x430D)
    REG_RTC_BBPU = KEY_BBPU | AUTO | ((REG_RTC_BBPU | BBPU | PWREN) & 0xFF);
    Inline_RTC_UpdateContext();
}
