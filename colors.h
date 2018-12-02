#ifndef COLORS_H
#define COLORS_H

#include <cstddef>
#include <stdint.h>

namespace nonogram
{

// Store some colors
typedef struct
{
    char const *token; // single letter color token
    char const *code;  // ANSI terminal color code (bg)
    uint32_t bitmask;  // Positional bitmask
} color_t;

// Main color table
extern color_t color_table[];

extern size_t color_table_size;

// Table lookup (exit on error)
int color_table_lookup(char const *key);

// Table lookup (false on error)
bool color_table_lookup(char const *key, int &idx);

char const *color_code_by_bitmask(uint32_t bitmask);

};

#endif
