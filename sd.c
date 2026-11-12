/*
 * sd.c - SD card driver for ALTROM (MT6261 MSDC0), polling, 1-bit bus.
 * Include after lcd.c and fb.c (the test screen uses fb_pixel/fb_flush_rect).
 *
 * The init sequence is the one proven by hand in fernly on a Nokia 130 Music:
 * pins 31..36 -> mode 1, PDN_CON1 bit 3 clear (MSDC clock), VMC_CON0 bit 0
 * (card supply), then CMD0 / CMD8 / ACMD41 / CMD2 / CMD3 / CMD7.
 * Register layout and reset values are from the MT6261M datasheet 3.16 and
 * the DZ09 project.  Command words:
 *   bits 5:0 index, 9:7 response type, 12:11 data type, 13 write, 14 stop.
 *
 *   int sd_init(void);                          0 = ok, else error code
 *   int sd_read_block(uint32_t lba, void *dst); 512 bytes, dst 4-byte aligned
 *   void sd_test(void);                         read + show sectors on the LCD
 */

#define SD_BASE      0xA0130000
#define SDR(off)     (*(volatile uint32_t *)(SD_BASE + (off)))
#define SD_CFG       SDR(0x00)
#define SD_STA       SDR(0x04)
#define SD_PS        SDR(0x0C)
#define SD_DAT       SDR(0x10)
#define SD_IOCON     SDR(0x14)
#define SD_IOCON1    SDR(0x18)
#define SD_SDCCFG    SDR(0x20)
#define SD_CMDREG    SDR(0x24)
#define SD_ARG       SDR(0x28)
#define SD_SDCSTA    SDR(0x2C)
#define SD_RESP0     SDR(0x30)
#define SD_RESP1     SDR(0x34)
#define SD_RESP2     SDR(0x38)
#define SD_RESP3     SDR(0x3C)
#define SD_CMDSTA    SDR(0x40)    /* read-to-clear */
#define SD_DATSTA    SDR(0x44)    /* read-to-clear */

#define SD_FIFOCLR   (1u << 14)   /* MSDC_STA */
#define SD_CMDRDY    (1u << 0)    /* SDC_CMDSTA */
#define SD_CMDTO     (1u << 1)
#define SD_RSPCRC    (1u << 2)
#define SD_BLKDONE   (1u << 0)    /* SDC_DATSTA */
#define SD_DATTO     (1u << 1)
#define SD_DATCRC    0x3FCu       /* bits 9:2 */
#define SD_BESDCBUSY (1u << 0)    /* SDC_STA */

#define SD_PDN1_CLR  (*(volatile uint32_t *)0xA0010324)  /* bit 3 = MSDC clock; 32-bit like the fernly session */
#define SD_VMC_CON0  (*(volatile uint32_t *)0xA07001C0)  /* bit 0 = card supply, bit 15 = status */

#define SD_RSP_NONE  0
#define SD_RSP_R1    1
#define SD_RSP_R2    2
#define SD_RSP_R3    3
#define SD_RSP_R6    6
#define SD_RSP_R1B   7
#define SD_CMDW(i, r) ((uint32_t)(i) | ((uint32_t)(r) << 7))
#define SD_DT_SINGLE (1u << 11)

#define SD_SLOW_SCLKF 80          /* ~300 kHz for card identification */
#define SD_FAST_SCLKF 4           /* ~6 MHz: fhost / (4 * SCLKF); lower = faster */

#define SD_CMD_TRIES  30000
#define SD_BUSY_TRIES 300000
#define SD_IDLE_TRIES 3000      /* short wait for the controller to go idle before a command */
#define SD_DATA_TRIES 3000000
#define SD_ACMD41_TRIES 5000

#ifdef LCD_MS
#define SD_MS(x) LCD_MS(x)
#else
#define SD_MS(x) sleep((x) * 200)
#endif

/* where the test screen keeps a sector: PSRAM, because a C array would count
 * against the 8 KB image limit */
#define SD_SECTOR_BUF 0x003DA000//0x00180000

enum {
	SD_OK = 0,
	SD_E_CMD0 = 1, SD_E_CMD55 = 2, SD_E_ACMD41 = 3, SD_E_NOTREADY = 4,
	SD_E_CMD2 = 5, SD_E_CMD3 = 6, SD_E_CMD7 = 7,
	SD_E_READCMD = 8, SD_E_DATTO = 9, SD_E_DATCRC = 10
};

uint32_t sd_cid[4];     /* CID, [0] = bits 127:96 (manufacturer in the top byte) */
uint32_t sd_rca;        /* relative card address (already shifted down) */
int      sd_ccs;        /* 1 = SDHC/SDXC: read args are sector numbers */

/* diagnostics shown on the test screen (ALTROM does not zero globals,
 * so sd_init() clears every one of these itself) */
uint32_t sd_dbg_st;     /* last non-zero SDC_CMDSTA seen by sd_cmd() */
uint32_t sd_dbg_ocr;    /* last ACMD41 response */
uint32_t sd_dbg_ok;     /* ACMD41 attempts that got a response */
uint32_t sd_dbg_f55;    /* CMD55 failures */
uint32_t sd_dbg_f41;    /* ACMD41 failures */
uint32_t sd_dbg_sta;    /* SDC_STA at the last failure */
uint32_t sd_dbg_msta;   /* MSDC_STA at the last failure */
uint32_t sd_dbg_ps;     /* MSDC_PS (pin levels) at the last failure */
int      sd_dbg_v2;     /* 1 = CMD8 answered */
int      sd_dbg_re;     /* 1 = CMD8 still answers after ACMD41 gave up */

static void sd_pin_mode(unsigned pin, unsigned mode)
{
	uint32_t addr = 0xA0020C00 + (pin >> 3) * 0x10;
	unsigned s = (pin & 7) * 4;
	write_32_masked(addr, mode << s, 0xFu << s);
}

static void sd_snap(void)
{
	sd_dbg_sta  = SD_SDCSTA;
	sd_dbg_msta = SD_STA;
	sd_dbg_ps   = SD_PS;
}

/* issue a command and wait for completion; 0 = ok */
static int sd_cmd(uint32_t word, uint32_t arg)
{
	(void)SD_CMDSTA;                 /* clear stale flags */
	(void)SD_DATSTA;
	sd_dbg_st = 0;
	for (uint32_t t = SD_IDLE_TRIES; t && (SD_SDCSTA & 0x1F); t--) { }   /* back-end/front-end busy bits */
	SD_ARG = arg;
	SD_CMDREG = word;

	for (uint32_t i = 0; i < SD_CMD_TRIES; i++) {
		uint32_t st = SD_CMDSTA;
		if (st) {
			sd_dbg_st = st;
			if (st & SD_CMDTO)  { sd_snap(); return -1; }
			if (st & SD_RSPCRC) { sd_snap(); return -2; }
			if (st & SD_CMDRDY) return 0;
		}
	}
	sd_snap();
	return -1;
}

/* pause, issue, retry a few times: the card/controller want a gap between commands */
static int sd_cmd_retry(uint32_t word, uint32_t arg, int tries)
{
	int r = -1;
	while (tries-- > 0) {
		SD_MS(2);
		r = sd_cmd(word, arg);
		if (!r) break;
	}
	return r;
}

int sd_init(void)
{
	sd_cid[0] = sd_cid[1] = sd_cid[2] = sd_cid[3] = 0;
	sd_rca = 0; sd_ccs = 0;
	sd_dbg_st = sd_dbg_ocr = sd_dbg_ok = 0;
	sd_dbg_f55 = sd_dbg_f41 = 0;
	sd_dbg_sta = sd_dbg_msta = sd_dbg_ps = 0;
	sd_dbg_v2 = sd_dbg_re = 0;

	/* pins 31..36 = clock, cmd, dat0..3 */
	for (unsigned p = 31; p <= 36; p++)
		sd_pin_mode(p, 1);

	/* MSDC clock gate open, card supply on */
	SD_PDN1_CLR = 1 << 3;
	SD_VMC_CON0 = 0x443;             /* exactly what worked in fernly: 3.3 V + enable */
	for (int i = 0; i < 100 && !(SD_VMC_CON0 & 0x8000); i++)
		SD_MS(1);
	SD_MS(50);

	/* controller reset and config (values from the fernly session) */
	SD_CFG = 0x2;
	SD_CFG = 0x0;
	SD_IOCON  = 0x012800C3;
	SD_IOCON1 = 0x00022222;
	SD_SDCCFG = 0x50018200;          /* timeout, serial if enabled, BLKLEN 512 */
	SD_CFG = 0x04000001 | (SD_SLOW_SCLKF << 8);
	SD_MS(20);

	/* CMD0 twice: the first can be lost if the card was mid-init */
	sd_cmd(SD_CMDW(0, SD_RSP_NONE), 0);
	SD_MS(5);
	if (sd_cmd(SD_CMDW(0, SD_RSP_NONE), 0)) return SD_E_CMD0;
	SD_MS(5);

	/* CMD8: only v2 cards answer; v1 cards time out */
	sd_dbg_v2 = 0;
	for (int i = 0; i < 3 && !sd_dbg_v2; i++)
		if (sd_cmd(SD_CMDW(8, SD_RSP_R1), 0x1AA) == 0 && (SD_RESP0 & 0xFFF) == 0x1AA)
			sd_dbg_v2 = 1;
	int v2 = sd_dbg_v2;

	/* ACMD41 until the card reports ready (bit 31); single failures are tolerated */
	uint32_t ocr = 0;
	for (int i = 0; i < SD_ACMD41_TRIES; i++) {
		SD_MS(2);                                       /* pace commands like the slow fernly run */
		if (sd_cmd(SD_CMDW(55, SD_RSP_R1), 0)) {
			sd_dbg_f55++;
		} else {
			SD_MS(2);
			if (sd_cmd(SD_CMDW(41, SD_RSP_R3), v2 ? 0x40FF8000u : 0x00FF8000u)) {
				sd_dbg_f41++;
			} else {
				sd_dbg_ok++;
				ocr = SD_RESP0;
				sd_dbg_ocr = ocr;
				if (ocr & 0x80000000u) break;
				continue;
			}
		}
		if (!sd_dbg_ok && sd_dbg_f55 + sd_dbg_f41 >= 50)   /* never answered: give up quickly */
			break;
	}
	if (!sd_dbg_ok) {
		/* is the controller still alive?  CMD0 + CMD8 again */
		sd_cmd(SD_CMDW(0, SD_RSP_NONE), 0);
		SD_MS(5);
		sd_dbg_re = (sd_cmd(SD_CMDW(8, SD_RSP_R1), 0x1AA) == 0 && (SD_RESP0 & 0xFFF) == 0x1AA);
		return SD_E_ACMD41;
	}
	if (!(ocr & 0x80000000u)) return SD_E_NOTREADY;
	sd_ccs = (ocr >> 30) & 1;

	/* CMD2: CID */
	if (sd_cmd_retry(SD_CMDW(2, SD_RSP_R2), 0, 3)) return SD_E_CMD2;
	sd_cid[0] = SD_RESP3;
	sd_cid[1] = SD_RESP2;
	sd_cid[2] = SD_RESP1;
	sd_cid[3] = SD_RESP0;

	/* CMD3: RCA */
	if (sd_cmd_retry(SD_CMDW(3, SD_RSP_R6), 0, 3)) return SD_E_CMD3;
	sd_rca = SD_RESP0 >> 16;

	/* CMD7: select the card (R1b, wait for not busy) */
	if (sd_cmd_retry(SD_CMDW(7, SD_RSP_R1B), sd_rca << 16, 3)) return SD_E_CMD7;
	for (uint32_t t = SD_BUSY_TRIES; t && (SD_SDCSTA & SD_BESDCBUSY); t--) { }

	/* standard-capacity cards need an explicit 512-byte block length */
	if (!sd_ccs)
		sd_cmd(SD_CMDW(16, SD_RSP_R1), 512);

	/* speed up now that identification is done */
	SD_CFG = (SD_CFG & ~0xFF00u) | (SD_FAST_SCLKF << 8);
	return SD_OK;
}

int sd_read_block(uint32_t lba, void *dst)
{
	uint32_t *p = (uint32_t *)dst;
	uint32_t t, st;

	SD_STA = SD_FIFOCLR;
	for (t = SD_CMD_TRIES; t && (SD_STA & SD_FIFOCLR); t--) { }

	/* CMD17: 0x891 = index 17 | R1 | single block read (same word the stock code uses) */
	if (sd_cmd(SD_CMDW(17, SD_RSP_R1) | SD_DT_SINGLE, sd_ccs ? lba : lba << 9))
		return SD_E_READCMD;

	for (int i = 0; i < 128; i++) {
		t = SD_DATA_TRIES;
		while (((SD_STA >> 4) & 0xF) == 0)           /* FIFOCNT */
			if (--t == 0) return SD_E_DATTO;
		p[i] = SD_DAT;
	}

	for (t = SD_CMD_TRIES; t; t--) {
		st = SD_DATSTA;
		if (st) {
			if (st & SD_DATTO)  return SD_E_DATTO;
			if (st & SD_DATCRC) return SD_E_DATCRC;
			if (st & SD_BLKDONE) return SD_OK;
		}
	}
	return SD_E_DATTO;
}

#ifndef SD_NO_TEST   /* define before including sd.c to drop the test screen and save space */

/* ---- test screen ---- */

static const uint16_t sd_font[16] = {
	0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7249,
	0x7BEF, 0x7BCF, 0x2BED, 0x6BAE, 0x3923, 0x6B6E, 0x79A7, 0x79A4
};

static void sd_hex(int x, int y, uint32_t v, int digits, uint16_t col)
{
	for (int k = 0; k < digits; k++) {
		uint16_t g = sd_font[(v >> (4 * (digits - 1 - k))) & 15];
		for (int r = 0; r < 5; r++)
			for (int c = 0; c < 3; c++) {
				uint16_t pc = (g & (1 << (14 - (r * 3 + c)))) ? col : COL_BLACK;
				for (int j = 0; j < 3; j++)
					for (int i = 0; i < 3; i++)
						fb_pixel(x + k * 12 + c * 3 + i, y + r * 3 + j, pc);
			}
	}
}

/*
 * Rows: init result | RCA | CCS
 *       CID word 0 (03 = SanDisk)
 *       LBA
 *       read result | bytes 510,511 (55AA on a partitioned card)
 *       first 16 bytes of the sector
 *       debug 1: CMD8 ok | CMD8 ok after failure | ACMD41 ok | CMD55 fails | ACMD41 fails
 *       debug 2: last CMDSTA | SDC_STA | MSDC_STA   (last two taken at the last failure)
 *       debug 3: MSDC_PS at failure | last ACMD41 OCR
 * Keys: OK = next sector, LEFT = previous, RIGHT = exit, # = reboot
 */
void sd_test(void)
{
	uint8_t *sec = (uint8_t *)SD_SECTOR_BUF;
	uint32_t lba = 0;

	/* wipe the panel's RAM first (it powers up with noise), show a busy marker */
	fb_clear(COL_BLACK);
	sd_hex(10, 10, 0xFFFF, 4, 0xFFE0);
	fb_flush();

	int init = sd_init();

	for (;;) {
		int rd = init;
		if (!init)
			rd = sd_read_block(lba, sec);

		fb_clear(COL_BLACK);
		sd_hex(10, 10, init, 4, init ? COL_RED : COL_GREEN);
		sd_hex(80, 10, sd_rca, 4, COL_WHITE);
		sd_hex(150, 10, sd_ccs, 1, COL_BLUE);
		sd_hex(10, 34, sd_cid[0], 8, COL_WHITE);
		sd_hex(10, 58, lba, 8, 0xFFE0);
		sd_hex(10, 82, rd, 4, rd ? COL_RED : COL_GREEN);
		if (!rd) {
			uint16_t sig = (sec[510] << 8) | sec[511];
			sd_hex(80, 82, sig, 4, sig == 0x55AA ? COL_GREEN : COL_RED);
			for (int row = 0; row < 4; row++) {
				uint32_t w = (uint32_t)sec[row * 4] << 24 | (uint32_t)sec[row * 4 + 1] << 16 |
				             (uint32_t)sec[row * 4 + 2] << 8 | sec[row * 4 + 3];
				sd_hex(10, 106 + row * 24, w, 8, COL_WHITE);
			}
		}
		sd_hex(10, 210, sd_dbg_v2, 1, COL_BLUE);        /* CMD8 answered              */
		sd_hex(26, 210, sd_dbg_re, 1, COL_BLUE);        /* CMD8 answers after failure */
		sd_hex(44, 210, sd_dbg_ok, 4, COL_WHITE);       /* ACMD41 ok                  */
		sd_hex(98, 210, sd_dbg_f55, 4, COL_RED);        /* CMD55 failures             */
		sd_hex(152, 210, sd_dbg_f41, 4, COL_RED);       /* ACMD41 failures            */
		sd_hex(10, 234, sd_dbg_st, 4, 0xFFE0);          /* last CMDSTA                */
		sd_hex(64, 234, sd_dbg_sta, 4, 0xFFE0);         /* SDC_STA at failure         */
		sd_hex(118, 234, sd_dbg_msta, 4, 0xFFE0);       /* MSDC_STA at failure        */
		sd_hex(10, 258, sd_dbg_ps, 8, COL_WHITE);       /* MSDC_PS at failure         */
		sd_hex(120, 258, sd_dbg_ocr, 8, COL_WHITE);     /* last OCR                   */
		fb_flush_rect(0, 0, LCD_W, 290);

		for (;;) {
			if (KP_OK == 0)    { lba++;                 while (KP_OK == 0) { } break; }
			if (KP_LEFT == 0)  { if (lba) lba--;        while (KP_LEFT == 0) { } break; }
			if (KP_HASH == 0)  reboot();
			if (KP_RIGHT == 0) return;
		}
	}
}

#endif /* SD_NO_TEST */
