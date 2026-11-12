/*
 * fat.c - read-only FAT32 / FAT16 for ALTROM, on top of sd_read_block().
 * Include after sd.c (and after fb.c if you want the demo screen).
 *
 *   int fat_mount(void);                                 0 = ok
 *   int fat_opendir(fat_dir_t *d, const char *path);     "/" or "/DIR/SUB"
 *   int fat_dir_next(fat_dir_t *d, fat_ent_t *e);        0 = entry, 1 = end, <0 = error
 *   int fat_open(fat_file_t *f, const char *path);       "/DIR/FILE.TXT"
 *   int fat_read(fat_file_t *f, void *dst, uint32_t n);  bytes read (0 = EOF), <0 = error
 *   int fat_seek(fat_file_t *f, uint32_t pos);
 *   void fat_test(void);                                 LCD demo: list root, view first file
 *
 * Handles an MBR-partitioned card or a bare FAT volume, 512-byte sectors.
 * Names are the short 8.3 names (case-insensitive); long-name entries are
 * skipped, so a file called "My Document.txt" is reached as "MYDOCU~1.TXT".
 * FAT12 and writing are not supported.
 *
 * Buffers live in PSRAM, not in C arrays: .bss counts against the 8 KB image.
 */

#ifndef FAT_SECBUF
#define FAT_SECBUF ((uint8_t *)0x003DA200)   /* file data sector       */
#define FAT_FATBUF ((uint8_t *)0x003DA600)   /* FAT sector cache       */
#define FAT_DIRBUF ((uint8_t *)0x003DA400)   /* directory sector cache */
#endif
//0x00190200
enum {
	FAT_OK = 0, FAT_E_IO = -1, FAT_E_BADFS = -2, FAT_E_NOPART = -3,
	FAT_E_FAT12 = -4, FAT_E_NOTFOUND = -5, FAT_E_ISDIR = -6, FAT_E_NOTDIR = -7
};

#define FAT_ATTR_DIR 0x10

typedef struct {
	char     name[13];      /* "NAME.EXT", NUL terminated */
	uint8_t  attr;
	uint32_t cluster;       /* first cluster (0 = none / root) */
	uint32_t size;
} fat_ent_t;

typedef struct {
	uint32_t cluster;       /* current directory cluster; 0 = FAT16 fixed root */
	uint32_t sec;           /* sector within the cluster (or the root area) */
	uint16_t idx;           /* entry within the sector, 0..15 */
	uint8_t  done;
} fat_dir_t;

typedef struct {
	uint32_t start, size, pos;
	uint32_t cluster;       /* cluster holding byte `pos` */
	uint32_t coff;          /* byte offset of `pos` inside that cluster */
} fat_file_t;

/* volume geometry, all set by fat_mount() (ALTROM does not zero globals) */
static uint8_t  fat_t32;          /* 1 = FAT32, 0 = FAT16 */
static uint8_t  fat_shift;        /* log2(sectors per cluster) */
static uint32_t fat_fat;          /* LBA of the first FAT */
static uint32_t fat_data;         /* LBA of cluster 2 */
static uint32_t fat_root;         /* FAT32 root cluster */
static uint32_t fat_root_lba;     /* FAT16 root directory LBA */
static uint32_t fat_root_secs;    /* FAT16 root directory sectors */
static uint32_t fat_fcache;       /* LBA currently in FAT_FATBUF */
static uint32_t fat_dcache;       /* LBA currently in FAT_DIRBUF */

static uint16_t fat_le16(const uint8_t *p) { return p[0] | (p[1] << 8); }
static uint32_t fat_le32(const uint8_t *p)
{
	return p[0] | (p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* is the sector a plausible FAT boot sector?  fills the geometry if so */
static int fat_bpb(const uint8_t *b, uint32_t part)
{
	uint8_t  spc = b[13], nf = b[16];
	uint16_t rsv = fat_le16(b + 14);
	uint32_t fatsz = fat_le16(b + 22) ? fat_le16(b + 22) : fat_le32(b + 36);
	uint32_t tot = fat_le16(b + 19) ? fat_le16(b + 19) : fat_le32(b + 32);
	uint32_t rootsecs, clusters;
	uint8_t s;

	if (fat_le16(b + 11) != 512 || !spc || (spc & (spc - 1)) || !rsv ||
	    (nf != 1 && nf != 2) || !fatsz || !tot)
		return FAT_E_BADFS;

	for (s = 0; (1u << s) != spc; s++) { }
	fat_shift = s;
	fat_t32 = (fat_le16(b + 22) == 0);
	fat_fat = part + rsv;

	if (fat_t32) {
		fat_root = fat_le32(b + 44);
		fat_root_secs = 0;
		fat_data = fat_fat + nf * fatsz;
	} else {
		rootsecs = ((uint32_t)fat_le16(b + 17) * 32 + 511) >> 9;
		fat_root_lba = fat_fat + nf * fatsz;
		fat_root_secs = rootsecs;
		fat_data = fat_root_lba + rootsecs;
	}

	clusters = (tot - (fat_data - part)) >> s;
	if (!fat_t32 && clusters < 4085)
		return FAT_E_FAT12;
	return FAT_OK;
}

/*
int fat_mount(void)
{
	uint8_t *b = FAT_SECBUF;
	uint32_t part = 0;
	int r, i;

	fat_fcache = fat_dcache = 0xFFFFFFFFu;

	if (sd_read_block(0, b)) return FAT_E_IO;
	if (fat_le16(b + 510) != 0xAA55) return FAT_E_BADFS;

	r = fat_bpb(b, 0);                 // bare FAT volume?
	if (r == FAT_OK || r == FAT_E_FAT12) return r;

	for (i = 0; i < 4; i++) {          // otherwise: first FAT partition in the MBR 
		uint8_t t = b[446 + i * 16 + 4];
		if (t == 0x0B || t == 0x0C || t == 0x04 || t == 0x06 || t == 0x0E) break;
	}
	if (i == 4) return FAT_E_NOPART;
	part = fat_le32(b + 446 + i * 16 + 8);

	if (sd_read_block(part, b)) return FAT_E_IO;
	if (fat_le16(b + 510) != 0xAA55) return FAT_E_BADFS;
	return fat_bpb(b, part);
}
*/

int fat_mount(void)
{
    uint8_t *b = FAT_SECBUF;
    uint32_t part = 0;
    int r, i;

    /* Invalidate old filesystem state */
    fat_t32 = 0;
    fat_shift = 0;
    fat_fat = 0;
    fat_data = 0;
    fat_root = 0;
    fat_root_lba = 0;
    fat_root_secs = 0;

    fat_fcache = 0xFFFFFFFFu;
    fat_dcache = 0xFFFFFFFFu;

    if (sd_read_block(0, b))
        return FAT_E_IO;

    if (fat_le16(b + 510) != 0xAA55)
        return FAT_E_BADFS;

    r = fat_bpb(b, 0);

    if (r == FAT_OK || r == FAT_E_FAT12)
        return r;

    for (i = 0; i < 4; i++) {
        uint8_t t = b[446 + i * 16 + 4];

        if (t == 0x0B || t == 0x0C ||
            t == 0x04 || t == 0x06 || t == 0x0E)
            break;
    }

    if (i == 4)
        return FAT_E_NOPART;

    part = fat_le32(b + 446 + i * 16 + 8);

    if (sd_read_block(part, b))
        return FAT_E_IO;

    if (fat_le16(b + 510) != 0xAA55)
        return FAT_E_BADFS;

    return fat_bpb(b, part);
}
static uint32_t fat_clus_lba(uint32_t c) { return fat_data + ((c - 2) << fat_shift); }

static int fat_valid(uint32_t c) { return c >= 2 && c < (fat_t32 ? 0x0FFFFFF0u : 0xFFF0u); }

/* next cluster in the chain; 0 on I/O error (which reads as "end") */
static uint32_t fat_next(uint32_t c)
{
	uint32_t off = fat_t32 ? c << 2 : c << 1;
	uint32_t lba = fat_fat + (off >> 9);

	if (lba != fat_fcache) {
		if (sd_read_block(lba, FAT_FATBUF)) return 0;
		fat_fcache = lba;
	}
	return fat_t32 ? fat_le32(FAT_FATBUF + (off & 511)) & 0x0FFFFFFFu
	               : fat_le16(FAT_FATBUF + (off & 511));
}

/* ---- directories ---- */

static void fat_dir_open(fat_dir_t *d, uint32_t cluster)
{
	if (cluster == 0 && fat_t32) cluster = fat_root;   /* FAT16 root stays 0 */
	d->cluster = cluster;
	d->sec = 0;
	d->idx = 0;
	d->done = 0;
}

int fat_dir_next(fat_dir_t *d, fat_ent_t *e)
{
	for (;;) {
		const uint8_t *p;
		uint32_t lba;
		uint8_t a;
		int i, n;

		if (d->done) return 1;

		lba = d->cluster ? fat_clus_lba(d->cluster) + d->sec : fat_root_lba + d->sec;
		if (lba != fat_dcache) {
			if (sd_read_block(lba, FAT_DIRBUF)) return FAT_E_IO;
			fat_dcache = lba;
		}
		p = FAT_DIRBUF + (d->idx << 5);

		if (++d->idx == 16) {                 /* step to the next sector */
			d->idx = 0;
			d->sec++;
			if (d->cluster == 0) {
				if (d->sec >= fat_root_secs) d->done = 1;
			} else if (d->sec == (1u << fat_shift)) {
				d->sec = 0;
				d->cluster = fat_next(d->cluster);
				if (!fat_valid(d->cluster)) d->done = 1;
			}
		}

		if (p[0] == 0x00) { d->done = 1; return 1; }   /* end of directory */
		if (p[0] == 0xE5) continue;                    /* deleted */
		a = p[11];
		if ((a & 0x3F) == 0x0F) continue;              /* long-name piece */
		if (a & 0x08) continue;                        /* volume label */

		n = 0;
		for (i = 0; i < 8 && p[i] != ' '; i++) e->name[n++] = p[i];
		if (p[8] != ' ') {
			e->name[n++] = '.';
			for (i = 8; i < 11 && p[i] != ' '; i++) e->name[n++] = p[i];
		}
		e->name[n] = 0;
		e->attr = a;
		e->cluster = ((fat_t32 ? (uint32_t)fat_le16(p + 20) : 0) << 16) | fat_le16(p + 26);
		e->size = fat_le32(p + 28);
		return 0;
	}
}

static int fat_eq(const char *a, const char *b, int n)
{
	for (int i = 0; i < n; i++) {
		char x = a[i], y = b[i];
		if (!x) return 0;
		if (x >= 'a' && x <= 'z') x -= 32;
		if (y >= 'a' && y <= 'z') y -= 32;
		if (x != y) return 0;
	}
	return a[n] == 0;
}

/* resolve a path to its directory entry; the root becomes a fake directory entry */
static int fat_walk(const char *path, fat_ent_t *out)
{
	out->name[0] = 0;
	out->attr = FAT_ATTR_DIR;
	out->cluster = 0;
	out->size = 0;

	for (;;) {
		fat_dir_t d;
		int len = 0, r;

		while (*path == '/') path++;
		if (!*path) return FAT_OK;
		if (!(out->attr & FAT_ATTR_DIR)) return FAT_E_NOTDIR;

		while (path[len] && path[len] != '/') len++;

		fat_dir_open(&d, out->cluster);
		for (;;) {
			r = fat_dir_next(&d, out);
			if (r < 0) return r;
			if (r > 0) return FAT_E_NOTFOUND;
			if (fat_eq(out->name, path, len)) break;
		}
		path += len;
	}
}

int fat_opendir(fat_dir_t *d, const char *path)
{
	fat_ent_t e;
	int r = fat_walk(path, &e);

	if (r) return r;
	if (!(e.attr & FAT_ATTR_DIR)) return FAT_E_NOTDIR;
	fat_dir_open(d, e.cluster);
	return FAT_OK;
}

/* ---- files ---- */

int fat_seek(fat_file_t *f, uint32_t pos)
{
	uint32_t sh = 9 + fat_shift;
	uint32_t n, c = f->start;

	if (pos > f->size) pos = f->size;
	for (n = pos >> sh; n && fat_valid(c); n--)
		c = fat_next(c);
	f->cluster = c;
	f->coff = pos & ((1u << sh) - 1);
	f->pos = pos;
	return FAT_OK;
}

int fat_open(fat_file_t *f, const char *path)
{
	fat_ent_t e;
	int r = fat_walk(path, &e);

	if (r) return r;
	if (e.attr & FAT_ATTR_DIR) return FAT_E_ISDIR;
	f->start = e.cluster;
	f->size = e.size;
	return fat_seek(f, 0);
}

int fat_read(fat_file_t *f, void *dst, uint32_t len)
{
	uint8_t *out = (uint8_t *)dst;
	uint32_t csize = 512u << fat_shift, done = 0;

	if (len > f->size - f->pos) len = f->size - f->pos;

	while (len) {
		uint32_t so, n, i;

		if (f->coff == csize) {
			f->cluster = fat_next(f->cluster);
			f->coff = 0;
		}
		if (!fat_valid(f->cluster)) break;

		if (sd_read_block(fat_clus_lba(f->cluster) + (f->coff >> 9), FAT_SECBUF))
			return FAT_E_IO;

		so = f->coff & 511;
		n = 512 - so;
		if (n > len) n = len;
		for (i = 0; i < n; i++)
			out[i] = FAT_SECBUF[so + i];

		out += n; len -= n; done += n;
		f->coff += n; f->pos += n;
	}
	return (int)done;
}

/* ---- demo screen: list the root directory, view the first file ---- */

#ifndef FAT_NO_UI

/* 3x5 font: 0-9, A-Z, then . _ - ~ / : */
static const uint16_t fat_font[42] = {
	0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7249, 0x7BEF, 0x7BCF,
	0x2BED, 0x6BAE, 0x3923, 0x6B6E, 0x79A7, 0x79A4, 0x396B, 0x5BED, 0x7497, 0x126A,
	0x5BAD, 0x4927, 0x5FED, 0x6B6D, 0x2B6A, 0x6BA4, 0x2B7B, 0x6BAD, 0x388E, 0x7492,
	0x5B6F, 0x5B6A, 0x5BFD, 0x5AAD, 0x5A92, 0x72A7,
	0x0002, 0x0007, 0x01C0, 0x0780, 0x12A4, 0x0410
};

static void fat_char(int x, int y, char c, uint16_t col)
{
	uint16_t g = 0;

	if (c >= 'a' && c <= 'z') c -= 32;
	if (c >= '0' && c <= '9') g = fat_font[c - '0'];
	else if (c >= 'A' && c <= 'Z') g = fat_font[10 + c - 'A'];
	else if (c == '.') g = fat_font[36];
	else if (c == '_') g = fat_font[37];
	else if (c == '-') g = fat_font[38];
	else if (c == '~') g = fat_font[39];
	else if (c == '/') g = fat_font[40];
	else if (c == ':') g = fat_font[41];

	for (int r = 0; r < 5; r++)
		for (int k = 0; k < 3; k++) {
			uint16_t pc = (g & (1 << (14 - (r * 3 + k)))) ? col : COL_BLACK;
			for (int j = 0; j < 2; j++)
				for (int i = 0; i < 2; i++)
					fb_pixel(x + k * 2 + i, y + r * 2 + j, pc);
		}
}

static void fat_text(int x, int y, const char *s, uint16_t col)
{
	for (; *s; s++, x += 8)
		fat_char(x, y, *s, col);
}

static void fat_hex(int x, int y, uint32_t v, int digits, uint16_t col)
{
	for (int k = 0; k < digits; k++) {
		int nib = (v >> (4 * (digits - 1 - k))) & 15;
		fat_char(x + k * 8, y, nib < 10 ? '0' + nib : 'A' + nib - 10, col);
	}
}

void fat_test(void)
{
	int r = sd_init();

	for (;;) {
		fat_dir_t d;
		fat_ent_t e;
		uint32_t fc = 0, fs = 0;
		int have = 0, row = 0;

		fb_clear(COL_BLACK);
		if (!r) r = fat_mount();
		fat_text(4, 4, "SD", 0xFFE0);
		fat_hex(24, 4, r < 0 ? (uint32_t)-r : 0, 2, r ? COL_RED : COL_GREEN);
		if (!r) fat_text(52, 4, fat_t32 ? "FAT32" : "FAT16", COL_WHITE);

		if (!r && !fat_opendir(&d, "/")) {
			while (row < 22 && fat_dir_next(&d, &e) == 0) {
				fat_text(4, 20 + row * 12, e.name, (e.attr & FAT_ATTR_DIR) ? COL_BLUE : COL_WHITE);
				if (e.attr & FAT_ATTR_DIR) fat_char(4 + 8 * 12, 20 + row * 12, '/', COL_BLUE);
				else fat_hex(120, 20 + row * 12, e.size, 8, 0xFFE0);
				if (!have && !(e.attr & FAT_ATTR_DIR)) { have = 1; fc = e.cluster; fs = e.size; }
				row++;
			}
		}
		fb_flush();

		for (;;) {                          /* keys: OK = view first file, RIGHT = exit */
			if (KP_HASH == 0)  reboot();
			if (KP_RIGHT == 0) return;
			if (KP_OK == 0 && have) {
				fat_file_t f;
				char buf[180];
				int n, i;

				while (KP_OK == 0) { }
				f.start = fc; f.size = fs;
				fat_seek(&f, 0);
				n = fat_read(&f, buf, sizeof buf);

				fb_clear(COL_BLACK);
				for (i = 0; i < n; i++) {
					char c = buf[i];
					if (c < ' ' || c > '~') c = '.';
					fat_char(4 + (i % 29) * 8, 8 + (i / 29) * 12, c, COL_WHITE);
				}
				fb_flush();
				while (KP_LEFT != 0 && KP_RIGHT != 0 && KP_HASH != 0) { }
				if (KP_HASH == 0)  reboot();
				if (KP_RIGHT == 0) return;
				while (KP_LEFT == 0) { }
				break;                      /* LEFT: back to the listing */
			}
		}
	}
}

#endif /* FAT_NO_UI */
