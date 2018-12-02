#include <iostream>
#include <cstdlib>
#include <cstring>
#include "colors.h"
using namespace std;

namespace nonogram
{

// Main color table
color_t color_table[] =
{
    { "",   "107", 0x0001 }, // White (empty)
    { "K",   "40", 0x0002 }, // Black (default fill)
    { "R",   "41", 0x0004 }, // Red
    { "G",   "42", 0x0008 }, // Green
    { "Y",   "43", 0x0010 }, // Yellow
    { "B",   "44", 0x0020 }, // Blue
    { "M",   "45", 0x0040 }, // Magenta
    { "C",   "46", 0x0080 }, // Cyan
    { "Gr", "100", 0x0100 }, // Grey
};

// Color table size
size_t color_table_size = sizeof(color_table) / sizeof(color_t);

// Table lookup (exit on error)
int color_table_lookup(char const *token)
{
    int idx;
    if (!color_table_lookup(token, idx))
    {
        cerr << "Color token " << token << " not found" << endl;
        exit(1);
    }
    return idx;
}

// Table lookup (false on error)
bool color_table_lookup(char const *token, int &idx)
{
    // Look for matching token
    for (size_t i = 0; i < color_table_size; i++)
    {
        if (strcmp(token, color_table[i].token) == 0)
        {
            idx = i;
            return true;
        }
    }

    // Not found
    return false;
}

char const *color_code_by_bitmask(uint32_t bitmask)
{
    // Look for matching token
    for (size_t i = 0; i < color_table_size; i++)
    {
        if (bitmask == color_table[i].bitmask)
        {
            return color_table[i].code;
        }
    }

    // Not found
    cerr << "Color bitmask " << (int)bitmask << " not found" << endl;
    exit(1);
}

};
