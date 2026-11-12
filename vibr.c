/*
 * vibr.c - vibration motor for ALTROM (MT6261 VIBR LDO, 1.8 / 2.8 / 3.0 V, up to 100 mA)
 *
 * The motor hangs directly on the chip's VIBR LDO output; one register,
 * VIBR_CON0 (defines.h), controls it:
 *   bit 0    RG_VIBR_EN      1 = LDO on
 *   bit 1    VIBR_ON_SEL     1 = software control (always set, as in DZ09)
 *   bits 5:4 RG_VIBR_VOSEL   0 = 1.8 V, 2 = 2.8 V, 3 = 3.0 V
 *   bit 15   status (read-only)
 *
 *   vibr_on();  vibr_off();
 *   vibr_pulse(ms);                  buzz for about ms milliseconds
 *   vibr_pattern(on_ms, off_ms, n);  n buzzes
 */

#define VIBR_V18  0
#define VIBR_V28  2
#define VIBR_V30  3

#ifndef VIBR_VOLT
#define VIBR_VOLT VIBR_V28      /* pick the lowest voltage that spins the motor well */
#endif

/* sleep() is a busy loop of unknown length: tune this by timing a pulse */
#define VIBR_MS(x) sleep((x) * 200)

void vibr_on(void)
{
	VIBR_CON0 = RG_VIBR_VOSEL(VIBR_VOLT) | VIBR_ON_SEL | RG_VIBR_EN;
}

void vibr_off(void)
{
	VIBR_CON0 = RG_VIBR_VOSEL(VIBR_VOLT) | VIBR_ON_SEL;
}

void vibr_pulse(unsigned ms)
{
	vibr_on();
	VIBR_MS(ms);
	vibr_off();
}

void vibr_pattern(unsigned on_ms, unsigned off_ms, unsigned n)
{
	while (n--) {
		vibr_pulse(on_ms);
		VIBR_MS(off_ms);
	}
}
