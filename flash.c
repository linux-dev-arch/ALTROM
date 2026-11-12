/*
 * flash.c - minimal, conservative SPI NOR flash driver for ALTROM (MT6261 SFI @ 0xA0140000)
 *
 * This touches the same flash chip that holds the bootloader and MAUI. A mistake here
 * can brick the phone, so this driver is deliberately narrow:
 *   - Reads always go through the normal memory-mapped (XIP) window at 0x10000000: this
 *     is how the CPU already executes code from flash, so it carries no extra risk.
 *   - Erase is ALWAYS exactly one 4 KB sector, address-aligned. No 32K/64K block erase,
 *     no capacity auto-detection, no partial-sector preserve-and-restore logic: the
 *     caller is responsible for supplying a full, already-assembled 4 KB of new content
 *     (patch_bl.py's sector outputs are exactly this).
 *   - Every erase is followed by a blank check; every program is followed by a byte-for-
 *     byte read-back verification. flash_write_sector() reports failure rather than
 *     silently leaving flash in a mixed state.
 *   - Nothing here decides *which* sector to touch - the caller always passes an address.
 *
 * Register layout (MAC_CTL / GPRAM / OUTL / INL) confirmed two independent ways:
 *   - matches the DZ09 project's sfi.c / sf.c for the MT6261
 *   - matches the stock bootloader's OWN flash-read routine, disassembled at file offset
 *     0x9060-0x9110 of the Nokia 130 Music dump, which uses these exact register offsets
 * The GPRAM command+address word packing (command in byte0, address high-to-low in
 * bytes 1-3) was derived from that disassembly's calling convention, not copied blind.
 */

#ifndef FLASH_BASE
#define FLASH_BASE      0xA0140000u
#endif
#ifndef FLASH_XIP_BASE
#define FLASH_XIP_BASE  0x10000000u      /* flash, memory-mapped for reading / code fetch */
#endif

/* each of these can be predefined (e.g. by a host test harness) before including this file */
#ifndef F_MAC_CTL
#define F_MAC_CTL   (*(volatile uint32_t *)(FLASH_BASE + 0x00))
#endif
#ifndef F_OUTL
#define F_OUTL      (*(volatile uint32_t *)(FLASH_BASE + 0x10))
#endif
#ifndef F_INL
#define F_INL       (*(volatile uint32_t *)(FLASH_BASE + 0x14))
#endif
#ifndef F_GPRAM
#define F_GPRAM(i)  (*(volatile uint32_t *)(FLASH_BASE + 0x800 + 4 * (i)))
#endif

#define F_WIP        (1u << 0)
#define F_WIP_READY  (1u << 1)
#define F_TRIG       (1u << 2)
#define F_MAC_EN     (1u << 3)

#define CMD_PAGE_PROG     0x02
#define CMD_READ_SR       0x05
#define CMD_WREN          0x06
#define CMD_ERASE_SECTOR  0x20
#define CMD_READ_ID       0x9F

#define FLASH_SECTOR  0x1000u   /* 4 KB: the only erase granularity this driver uses */
#define FLASH_PAGE    256u      /* conservative chunk size; real page may be bigger, never smaller */
#define FLASH_TIMEOUT 2000000u  /* loop iterations before giving up on a stuck busy flag */

static int flash_wait_idle(void)       /* after a trigger: MAC_CTL's own transaction-done flag */
{
	uint32_t t;

	for (t = 0; t < FLASH_TIMEOUT && (F_MAC_CTL & F_WIP_READY); t++) { }
	if (F_MAC_CTL & F_WIP_READY) return -1;
	for (t = 0; t < FLASH_TIMEOUT && (F_MAC_CTL & F_WIP); t++) { }
	return (F_MAC_CTL & F_WIP) ? -1 : 0;
}

static int flash_run(void)             /* GPRAM/OUTL/INL already loaded: fire the transaction */
{
	F_MAC_CTL |= F_MAC_EN;
	F_MAC_CTL |= F_TRIG | F_MAC_EN;
	if (flash_wait_idle()) return -1;
	F_MAC_CTL &= ~(F_TRIG | F_MAC_EN);
	return 0;
}

static int flash_cmd(uint8_t cmd)      /* no address, no data: WREN, WRDIS, ... */
{
	F_GPRAM(0) = cmd;
	F_OUTL = 1;
	F_INL = 0;
	return flash_run();
}

static int flash_cmd_read(uint8_t cmd, uint8_t *buf, uint32_t n)   /* RDID, RDSR, ... */
{
	uint32_t i, w;

	if (n > 60) n = 60;                 /* GPRAM is small; this driver never needs more */
	F_GPRAM(0) = cmd;
	F_OUTL = 1;
	F_INL = n;
	if (flash_run()) return -1;
	for (i = 0; i < n; i++) {
		w = F_GPRAM((i + 1) >> 2);      /* word 0 holds the echoed command byte + 3 data bytes */
		buf[i] = (uint8_t)(w >> (8 * ((i + 1) & 3)));
	}
	return 0;
}

static uint32_t flash_addr_word(uint8_t cmd, uint32_t addr)
{
	return (uint32_t)cmd | (((addr >> 16) & 0xFFu) << 8) |
	       (((addr >> 8) & 0xFFu) << 16) | ((addr & 0xFFu) << 24);
}

/* command + 3-byte address, with up to 248 bytes of outgoing data (page program) */
static int flash_cmd_addr(uint8_t cmd, uint32_t addr, const uint8_t *data, uint32_t n)
{
	uint32_t i, w;

	if (n > 248) return -1;             /* caller must chunk; this driver never asks for more */
	F_GPRAM(0) = flash_addr_word(cmd, addr);
	for (i = 0; i < n; i += 4) {
		w = data[i];
		if (i + 1 < n) w |= (uint32_t)data[i + 1] << 8;
		if (i + 2 < n) w |= (uint32_t)data[i + 2] << 16;
		if (i + 3 < n) w |= (uint32_t)data[i + 3] << 24;
		F_GPRAM(1 + i / 4) = w;
	}
	F_OUTL = 4 + n;
	F_INL = 0;
	return flash_run();
}

static int flash_wait_not_busy(void)   /* poll the flash chip's own status register (bit 0) */
{
	uint8_t sr;
	uint32_t t;

	for (t = 0; t < FLASH_TIMEOUT; t++) {
		if (flash_cmd_read(CMD_READ_SR, &sr, 1)) return -1;
		if (!(sr & 1)) return 0;
	}
	return -1;
}

/* 3-byte JEDEC ID: manufacturer | (type << 8) | (capacity << 16). 0 on failure. */
uint32_t flash_id(void)
{
	uint8_t id[3] = { 0, 0, 0 };

	if (flash_cmd_read(CMD_READ_ID, id, 3)) return 0;
	return (uint32_t)id[0] | ((uint32_t)id[1] << 8) | ((uint32_t)id[2] << 16);
}

void flash_read(uint32_t addr, void *buf, uint32_t n)
{
	const uint8_t *src = (const uint8_t *)(FLASH_XIP_BASE + addr);
	uint8_t *dst = (uint8_t *)buf;

	while (n--) *dst++ = *src++;
}

static int flash_is_blank(uint32_t addr, uint32_t n)
{
	const uint8_t *p = (const uint8_t *)(FLASH_XIP_BASE + addr);

	while (n--)
		if (*p++ != 0xFF) return 0;
	return 1;
}

/*
 * Erase exactly one 4 KB sector. addr must be sector-aligned.
 * Returns: 0 ok, -1 bad address / transaction failure, -2 erased but did not read back blank.
 */
int flash_erase_sector(uint32_t addr)
{
	if (addr & (FLASH_SECTOR - 1)) return -1;
	if (flash_is_blank(addr, FLASH_SECTOR)) return 0;        /* nothing to do */
	if (flash_cmd(CMD_WREN)) return -1;
	if (flash_cmd_addr(CMD_ERASE_SECTOR, addr, 0, 0)) return -1;
	if (flash_wait_not_busy()) return -1;
	return flash_is_blank(addr, FLASH_SECTOR) ? 0 : -2;
}

/* program up to FLASH_PAGE bytes; caller must have already erased this region */
static int flash_program_page(uint32_t addr, const uint8_t *data, uint32_t n)
{
	uint32_t off = 0, chunk;

	while (off < n) {
		chunk = n - off;
		if (chunk > 128) chunk = 128;   /* stay well under the GPRAM / page-size limits */
		if (flash_cmd(CMD_WREN)) return -1;
		if (flash_cmd_addr(CMD_PAGE_PROG, addr + off, data + off, chunk)) return -1;
		if (flash_wait_not_busy()) return -1;
		off += chunk;
	}
	return 0;
}

/*
 * Write a WHOLE, already-erased 4 KB sector and verify it byte for byte against what was
 * asked for. len must be exactly FLASH_SECTOR; this never does a partial-sector write.
 * Returns: 0 ok, -1 bad arguments, -2 a program step failed, -3 verify mismatch (first
 * differing offset left in *fail_at if fail_at is non-NULL).
 */
int flash_write_sector(uint32_t addr, const uint8_t *data, uint32_t len, uint32_t *fail_at)
{
	uint32_t off, i;
	uint8_t check[32];

	if (!data || len != FLASH_SECTOR || (addr & (FLASH_SECTOR - 1))) return -1;

	for (off = 0; off < FLASH_SECTOR; off += FLASH_PAGE)
		if (flash_program_page(addr + off, data + off, FLASH_PAGE)) return -2;

	for (off = 0; off < FLASH_SECTOR; off += sizeof check) {
		flash_read(addr + off, check, sizeof check);
		for (i = 0; i < sizeof check; i++)
			if (check[i] != data[off + i]) {
				if (fail_at) *fail_at = off + i;
				return -3;
			}
	}
	return 0;
}
