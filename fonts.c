
#include <stdint.h>

/*
 * ALTROM
 * Nokia 130 Music
 * Bitmap Font Renderer
 *
 * Font sizes: 12x12 and 8x8
 * Base glyph: 5x7
 * Colour: RGB565
 */

extern void fb_pixel(int x, int y, uint16_t color);

/* -------------------------------------------------
 * FONT SETTINGS
 * ------------------------------------------------- */

#define FONT12_WIDTH  12
#define FONT12_HEIGHT 12

#define FONT8_WIDTH   8
#define FONT8_HEIGHT  8

#define CHAR12_SPACING 13
#define LINE12_SPACING 14

#define CHAR8_SPACING  9
#define LINE8_SPACING  10

/* -------------------------------------------------
 * FONT DATA
 *
 * 0-25 : A-Z
 * 26-35: 0-9
 * 36-52: Punctuation
 * 53-54: < >
 * ------------------------------------------------- */

static const uint8_t font5x7[55][7] = {

    /* A-Z */

    {14,17,17,31,17,17,17}, /* A */
    {30,17,17,30,17,17,30}, /* B */
    {14,17,16,16,16,17,14}, /* C */
    {30,17,17,17,17,17,30}, /* D */
    {31,16,16,30,16,16,31}, /* E */
    {31,16,16,30,16,16,16}, /* F */
    {14,17,16,23,17,17,15}, /* G */
    {17,17,17,31,17,17,17}, /* H */
    {31,4,4,4,4,4,31},      /* I */
    {7,2,2,2,18,18,12},     /* J */
    {17,18,20,24,20,18,17}, /* K */
    {16,16,16,16,16,16,31}, /* L */
    {17,27,21,21,17,17,17}, /* M */
    {17,25,21,19,17,17,17}, /* N */
    {14,17,17,17,17,17,14}, /* O */
    {30,17,17,30,16,16,16}, /* P */
    {14,17,17,17,21,18,13}, /* Q */
    {30,17,17,30,20,18,17}, /* R */
    {15,16,16,14,1,1,30},   /* S */
    {31,4,4,4,4,4,4},       /* T */
    {17,17,17,17,17,17,14}, /* U */
    {17,17,17,17,17,10,4},  /* V */
    {17,17,17,21,21,21,10}, /* W */
    {17,17,10,4,10,17,17},  /* X */
    {17,17,10,4,4,4,4},     /* Y */
    {31,1,2,4,8,16,31},     /* Z */

    /* 0-9 */

    {14,17,19,21,25,17,14}, /* 0 */
    {4,12,4,4,4,4,14},      /* 1 */
    {14,17,1,2,4,8,31},     /* 2 */
    {30,1,1,14,1,1,30},     /* 3 */
    {2,6,10,18,31,2,2},     /* 4 */
    {31,16,16,30,1,1,30},   /* 5 */
    {14,16,16,30,17,17,14}, /* 6 */
    {31,1,2,4,8,8,8},       /* 7 */
    {14,17,17,14,17,17,14}, /* 8 */
    {14,17,17,15,1,1,14},   /* 9 */

    /* Punctuation */

    {4,4,4,4,4,0,4},        /* ! */
    {14,17,1,2,4,0,4},      /* ? */
    {0,0,0,0,0,0,4},        /* . */
    {0,0,0,0,0,4,8},        /* , */
    {0,4,4,0,4,4,0},        /* : */
    {0,4,4,0,4,4,8},        /* ; */
    {0,0,0,31,0,0,0},       /* - */
    {0,4,4,31,4,4,0},       /* + */
    {1,2,2,4,8,8,16},       /* / */
    {2,4,8,8,8,4,2},        /* ( */
    {8,4,2,2,2,4,8},        /* ) */
    {14,8,8,8,8,8,14},      /* [ */
    {14,2,2,2,2,2,14},      /* ] */
    {4,4,8,0,0,0,0},        /* ' */
    {10,10,0,0,0,0,0},      /* " */
    {0,0,0,0,0,0,31},       /* _ */
    {0,0,31,0,31,0,0},      /* = */

    /* Angled brackets */

    {2,4,8,16,8,4,2},       /* < */
    {8,4,2,1,2,4,8}         /* > */
};

/* -------------------------------------------------
 * BLANK GLYPH
 * ------------------------------------------------- */

static const uint8_t blank_glyph[7] = {
    0,0,0,0,0,0,0
};

/* -------------------------------------------------
 * CHARACTER LOOKUP
 * ------------------------------------------------- */

static const uint8_t *get_glyph(char c)
{
    if (c >= 'a' && c <= 'z')
        c -= 32;

    if (c >= 'A' && c <= 'Z')
        return font5x7[c - 'A'];

    if (c >= '0' && c <= '9')
        return font5x7[26 + c - '0'];

    switch (c) {

        case '!': return font5x7[36];
        case '?': return font5x7[37];
        case '.': return font5x7[38];
        case ',': return font5x7[39];
        case ':': return font5x7[40];
        case ';': return font5x7[41];
        case '-': return font5x7[42];
        case '+': return font5x7[43];
        case '/': return font5x7[44];
        case '(': return font5x7[45];
        case ')': return font5x7[46];
        case '[': return font5x7[47];
        case ']': return font5x7[48];
        case '\'': return font5x7[49];
        case '"': return font5x7[50];
        case '_': return font5x7[51];
        case '=': return font5x7[52];
        case '<': return font5x7[53];
        case '>': return font5x7[54];

        case ' ':
        default:
            return blank_glyph;
    }
}

/* -------------------------------------------------
 * SCALING MAPS
 * ------------------------------------------------- */

/* 12x12 -> 5x7 */

static const uint8_t map12to5[12] = {
    0,0,0,1,1,2,2,2,3,3,4,4
};

static const uint8_t map12to7[12] = {
    0,0,1,1,2,2,3,4,4,5,5,6
};

/* 8x8 -> 5x7 */

static const uint8_t map8to5[8] = {
    0,0,1,2,2,3,4,4
};

static const uint8_t map8to7[8] = {
    0,1,2,3,4,5,6,6
};

/* -------------------------------------------------
 * RECTANGLE DRAWING
 * ------------------------------------------------- */

static void draw_rect(int x, int y,
                      int width, int height,
                      uint16_t color)
{
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            fb_pixel(x + px, y + py, color);
        }
    }
}

/* -------------------------------------------------
 * 12x12 CHARACTER
 * ------------------------------------------------- */

void draw_char12(int x, int y, char c,
                 uint16_t fg,
                 uint16_t bg,
                 int transparent)
{
    const uint8_t *glyph = get_glyph(c);

    for (int py = 0; py < FONT12_HEIGHT; py++) {
        for (int px = 0; px < FONT12_WIDTH; px++) {

            uint8_t sx = map12to5[px];
            uint8_t sy = map12to7[py];

            if (glyph[sy] & (1u << (4 - sx))) {
                fb_pixel(x + px, y + py, fg);
            }
            else if (!transparent) {
                fb_pixel(x + px, y + py, bg);
            }
        }
    }
}

/* -------------------------------------------------
 * 8x8 CHARACTER
 * ------------------------------------------------- */

void draw_char8(int x, int y, char c,
                uint16_t fg,
                uint16_t bg,
                int transparent)
{
    const uint8_t *glyph = get_glyph(c);

    for (int py = 0; py < FONT8_HEIGHT; py++) {
        for (int px = 0; px < FONT8_WIDTH; px++) {

            uint8_t sx = map8to5[px];
            uint8_t sy = map8to7[py];

            if (glyph[sy] & (1u << (4 - sx))) {
                fb_pixel(x + px, y + py, fg);
            }
            else if (!transparent) {
                fb_pixel(x + px, y + py, bg);
            }
        }
    }
}

/* -------------------------------------------------
 * 12x12 STRING
 * ------------------------------------------------- */

void draw_string12(int x, int y, const char *str,
                   uint16_t fg,
                   uint16_t bg,
                   int transparent,
                   int padding)
{
    int start_x = x;

    while (*str) {

        const char *p = str;
        int length = 0;

        while (*p && *p != '\n') {
            length++;
            p++;
        }

        int width = 0;

        if (length > 0)
            width = (length - 1) * CHAR12_SPACING
                    + FONT12_WIDTH;

        if (!transparent && width > 0) {
            draw_rect(
                x - padding,
                y - padding,
                width + padding * 2,
                FONT12_HEIGHT + padding * 2,
                bg
            );
        }

        for (int i = 0; i < length; i++) {

            draw_char12(
                x, y, str[i],
                fg, bg, 1
            );

            x += CHAR12_SPACING;
        }

        if (*p == '\n') {
            str = p + 1;
            x = start_x;
            y += LINE12_SPACING;
        }
        else {
            str = p;
        }
    }
}

/* -------------------------------------------------
 * 8x8 STRING
 * ------------------------------------------------- */

void draw_string8(int x, int y, const char *str,
                  uint16_t fg,
                  uint16_t bg,
                  int transparent,
                  int padding)
{
    int start_x = x;

    while (*str) {

        const char *p = str;
        int length = 0;

        while (*p && *p != '\n') {
            length++;
            p++;
        }

        int width = 0;

        if (length > 0)
            width = (length - 1) * CHAR8_SPACING
                    + FONT8_WIDTH;

        if (!transparent && width > 0) {
            draw_rect(
                x - padding,
                y - padding,
                width + padding * 2,
                FONT8_HEIGHT + padding * 2,
                bg
            );
        }

        for (int i = 0; i < length; i++) {

            draw_char8(
                x, y, str[i],
                fg, bg, 1
            );

            x += CHAR8_SPACING;
        }

        if (*p == '\n') {
            str = p + 1;
            x = start_x;
            y += LINE8_SPACING;
        }
        else {
            str = p;
        }
    }
}

void draw_int(int x, int y, int value,
              uint16_t fg, uint16_t bg)
{
    char buf[12];
    int i = 0;
    unsigned int n;
    int negative = 0;

    if (value < 0) {
        negative = 1;
        n = (unsigned int)(-(value + 1)) + 1;
    } else {
        n = (unsigned int)value;
    }

    static const unsigned int powers[] = {
        1000000000u,
        100000000u,
        10000000u,
        1000000u,
        100000u,
        10000u,
        1000u,
        100u,
        10u,
        1u
    };

    int started = 0;

    for (int p = 0; p < 10; p++) {
        unsigned int digit = 0;

        while (n >= powers[p]) {
            n -= powers[p];
            digit++;
        }

        if (digit || started || p == 9) {
            buf[i++] = '0' + digit;
            started = 1;
        }
    }

    if (negative)
        buf[i++] = '-';

    buf[i] = '\0';

    // Reverse
    /*for (int a = 0, b = i - 1; a < b; a++, b--) {
        char t = buf[a];
        buf[a] = buf[b];
        buf[b] = t;
    }*/

    draw_string12(x, y, buf, fg, bg, 1, 0);
}



void draw_int8(int x, int y, int value,
              uint16_t fg, uint16_t bg)
{
    char buf[12];
    int i = 0;
    unsigned int n;
    int negative = 0;

    if (value < 0) {
        negative = 1;
        n = (unsigned int)(-(value + 1)) + 1;
    } else {
        n = (unsigned int)value;
    }

    static const unsigned int powers[] = {
        1000000000u,
        100000000u,
        10000000u,
        1000000u,
        100000u,
        10000u,
        1000u,
        100u,
        10u,
        1u
    };

    int started = 0;

    for (int p = 0; p < 10; p++) {
        unsigned int digit = 0;

        while (n >= powers[p]) {
            n -= powers[p];
            digit++;
        }

        if (digit || started || p == 9) {
            buf[i++] = '0' + digit;
            started = 1;
        }
    }

    if (negative)
        buf[i++] = '-';

    buf[i] = '\0';

    // Reverse
    /*for (int a = 0, b = i - 1; a < b; a++, b--) {
        char t = buf[a];
        buf[a] = buf[b];
        buf[b] = t;
    }*/

    draw_string8(x, y, buf, fg, bg, 1, 0);
}
