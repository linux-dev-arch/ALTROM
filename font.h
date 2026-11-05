#ifndef ALTROM_FONT_H
#define ALTROM_FONT_H

#include <stdint.h>

void draw_char12(int x, int y, char c,
                 uint16_t fg, uint16_t bg,
                 int transparent);

void draw_char8(int x, int y, char c,
                uint16_t fg, uint16_t bg,
                int transparent);

void draw_string12(int x, int y, const char *str,
                   uint16_t fg, uint16_t bg,
                   int transparent, int padding);

void draw_string8(int x, int y, const char *str,
                  uint16_t fg, uint16_t bg,
                  int transparent, int padding);

void draw_int(int x, int y, int value,
              uint16_t fg, uint16_t bg);

#endif
