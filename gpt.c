/*
 * timer.c - general-purpose timers for ALTROM (MT6261 GPT block at 0xA00C0000).
 *
 *   timer_init();                    once, early: opens the GPT clock gate, starts GPT4
 *   ticks();                         GPT4 free-running counter, 26 MHz, 32-bit (wraps every ~165 s)
 *   delay_us(us); delay_ms(ms);      real-time busy delays (replace sleep())
 *   millis();                        milliseconds since timer_init() (call at least every 165 s;
 *                                    needs divmod.c, enable with #define TIMER_MILLIS 1)
 *   gpt_start(t, count, rate, repeat);  GPT1/GPT2: 16-bit count-down on the 32 kHz clock
 *   gpt_expired(t);                  poll: 1 once per timeout
 *   gpt_stop(t);
 *
 * No interrupts: everything is polled.  GPT4 is documented in the datasheet (section 3.5);
 * the clock gate is code PD_GPT (0x22) from the DZ09 project: bit 2 of PDN_CON2.
 * GPT4's counter stops if the 26 MHz clock is gated in sleep mode (ALTROM never sleeps).
 */

#define TIMER_PRESENT 1

#ifndef GPT_BASE_ADDR
#define GPT_BASE_ADDR 0xA00C0000
#endif
#ifndef GPT4_DAT
#define GPT_REG(off) (*(volatile uint32_t *)(GPT_BASE_ADDR + (off)))
#define GPT1_CON   GPT_REG(0x00)
#define GPT1_DAT   GPT_REG(0x04)
#define GPT2_CON   GPT_REG(0x08)
#define GPT2_DAT   GPT_REG(0x0C)
#define GPT_STA    GPT_REG(0x10)      /* bit0 GPT1, bit1 GPT2; read-to-clear BOTH */
#define GPT1_PS    GPT_REG(0x14)
#define GPT2_PS    GPT_REG(0x18)
#define GPT4_CON   GPT_REG(0x28)
#define GPT4_DAT   GPT_REG(0x2C)
#define GPT_PDN_CLR (*(volatile uint32_t *)0xA0010328)   /* PDN_CON2 clear; bit 2 = GPT clock */
#endif

#define GPT_EN      (1u << 15)
#define GPT_REPEAT  (1u << 14)
#define GPT4_EN     (1u << 0)
#define GPT4_LOCK   (1u << 1)
#define TICKS_PER_US 26u              /* GPT4 runs at 26 MHz */

static uint32_t gpt_sta;              /* shadow of the status flags (the register clears on read) */

void timer_init(void)
{
	GPT_PDN_CLR = 1 << 2;             /* clock gate open */
	GPT1_CON = 0;
	GPT2_CON = 0;
	if (!(GPT4_CON & GPT4_LOCK))
		GPT4_CON |= GPT4_EN;          /* a locked counter is already running */
	gpt_sta = 0;
	(void)GPT_STA;                    /* drop any stale flags */
}

uint32_t ticks(void)
{
	return GPT4_DAT;
}

void delay_us(uint32_t us)            /* up to about 165 000 000 us (165 s) in one call */
{
	uint32_t t0, n = us * TICKS_PER_US;

	if (!(GPT4_CON & GPT4_EN))        /* forgot timer_init()?  a stopped counter would hang here */
		timer_init();
	t0 = ticks();
	while (ticks() - t0 < n) { watchdog_feed();}
}

void delay_ms(uint32_t ms)
{
	while (ms--)
		delay_us(1000);
}

#if defined(TIMER_MILLIS) && TIMER_MILLIS
uint32_t millis(void)                 /* wrap-safe if called at least every ~165 s */
{
	static uint32_t last, rem, ms, started;
	uint32_t now = ticks();

	if (!started) { started = 1; last = now; rem = ms = 0; }
	rem += now - last;
	last = now;
	ms += rem / 26000u;
	rem %= 26000u;
	return ms;
}
#endif

/* ---- GPT1 / GPT2: 16-bit count-down timers on the 32 kHz clock ---- */

/*
 * t = 1 or 2.  count 1..65535 ticks at rate: 0=16384 Hz 1=8192 2=4096 3=2048 4=1024
 * 5=512 6=256 7=128 Hz (so 16384 Hz gives up to 4 s, 128 Hz up to 512 s).
 * repeat = 1: auto-reload and flag again every period; 0: one shot.
 */
void gpt_start(int t, uint16_t count, uint8_t rate, int repeat)
{
	volatile uint32_t *con = t == 1 ? &GPT1_CON : &GPT2_CON;
	volatile uint32_t *dat = t == 1 ? &GPT1_DAT : &GPT2_DAT;
	volatile uint32_t *ps  = t == 1 ? &GPT1_PS  : &GPT2_PS;

	*con = 0;
	delay_us(120);                    /* datasheet: let it settle for 3+ cycles of the 32 kHz clock */
	*dat = count;
	*ps = rate & 7;
	gpt_sta |= GPT_STA;               /* read (and clear) any stale hardware flag, keep the other timer's */
	gpt_sta &= ~(1u << (t - 1));
	*con = GPT_EN | (repeat ? GPT_REPEAT : 0);
}

void gpt_stop(int t)
{
	if (t == 1) GPT1_CON = 0;
	else        GPT2_CON = 0;
}

/* 1 once per timeout.  GPT_STA clears both flags when read, so the other timer's flag is kept. */
int gpt_expired(int t)
{
	uint32_t bit = 1u << (t - 1);

	gpt_sta |= GPT_STA;
	if (!(gpt_sta & bit)) return 0;
	gpt_sta &= ~bit;
	return 1;
}
