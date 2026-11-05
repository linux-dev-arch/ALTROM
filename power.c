/*
 * power.c - keep the phone powered from the battery
 *
 * On MT6261 the PMIC only keeps the system up while something is holding it:
 * USB/charger, the power key, or the RTC "BBPU" latch.  A payload started over
 * USB download mode has no BBPU latch set, so the phone dies the moment USB
 * is unplugged.  Setting BBPU (as the DZ09 project's RTC_LockBBPower does)
 * holds power from the battery.
 *
 * Include early (after defines.h); call power_hold() right after init().
 */

#define RTC_BASE_ADDR  0xA0710000
#define RTC16(off)     (*(volatile uint16_t *)(RTC_BASE_ADDR + (off)))
#define RTC_BBPU_REG   RTC16(0x0000)
#define RTC_WRTGR_REG  RTC16(0x0074)

#define BBPU_KEY       0x4300
#define BBPU_PWREN     (1 << 0)
#define BBPU_BBPU      (1 << 2)   /* baseband power hold */
#define BBPU_AUTO      (1 << 3)
#define BBPU_CBUSY     (1 << 6)

static void rtc_commit(void)
{
	RTC_WRTGR_REG = 1;
	for (uint32_t i = 0; i < 1000000 && (RTC_BBPU_REG & BBPU_CBUSY); i++) { }
}

/* returns 1 if the hold bit reads back set */
int power_hold(void)
{
	RTC_BBPU_REG = BBPU_KEY | BBPU_AUTO | ((RTC_BBPU_REG | BBPU_BBPU) & 0xFF);
	rtc_commit();
	return (RTC_BBPU_REG & BBPU_BBPU) ? 1 : 0;
}

/* release the hold: the phone powers off if nothing else (USB, key) holds it */
void power_release(void)
{
	RTC_BBPU_REG = BBPU_KEY | ((RTC_BBPU_REG & ~BBPU_BBPU) & 0xFF);
	rtc_commit();
}
