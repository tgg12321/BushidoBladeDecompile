/* Data-only file: .data 0x8009BCE4..0x8009BD68 (ROM 0x8C4E4), the character
 * select tables and the match settings record.
 * Owner ruling Q122 (data-only-no-sdata):
 * 1. One run: 63D2C.c's D_8009BCD4 ends at 0x8009BCE4, 64FD8.o(.data) is
 *    empty, and LIBAPI COUNTER's .data starts at 0x8009BD68 (the libapi
 *    modules between have none). No uninitialized objects.
 * 2. Small object in .data: g_sel_sheet_code_pairs (u8[2][2];
 * g_sel_grid_page_cells before it ends at 0x8009BD20 by its [2][10] shape,
 * g_working_record after it is another type with other readers). Under -G8 the
 * original compiler puts it in .sdata, so no -G8 file defined it.
 * 3. Neighbours in .data link order: 63D2C.c (gp accesses) and 64FD8.c (its
 *    own .sdata / .sbss, gp accesses), both -G8 code modules with adjacent
 *    .text, then Sony's LIBAPI; so no code module owns g_sel_sheet_code_pairs.
 * Readers: g_sel_entry_availability_and_claims / g_sel_grid_page_cells /
 * D_8009BD58 (64FD8.c), g_sel_sheet_code_pairs (63D2C.c, 64FD8.c),
 * g_working_record (4D0A8.c, 51268.c, 64FD8.c). */
#include "common.h"
#include "bb2.h"

u8 g_sel_entry_availability_and_claims[20] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

Unk8009BCF8Record g_sel_grid_page_cells[2][10] = {
    {{0x00, 0x00},
     {0x02, 0x01},
     {0x01, 0x02},
     {0x03, 0x03},
     {0x04, 0x04},
     {0x09, 0x05},
     {0x07, 0x06},
     {0x08, 0x07},
     {0x06, 0x08},
     {0x05, 0x09}},
    {{0x0A, 0x0C},
     {0x0B, 0x0D},
     {0x0C, 0x0E},
     {0x10, 0x0F},
     {0x13, 0x10},
     {0x11, 0x11},
     {0x0D, 0x12},
     {0x12, 0x13},
     {0x0F, 0x14},
     {0x0E, 0x15}},
};

u8 g_sel_sheet_code_pairs[2][2] = {{1, 2}, {3, 4}};

Unk8009BD24Block g_working_record = {
    {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
     {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0x11, 0x44, 0x88},
    {0x11, 0x44, 0x88},
    {0x11, 0x44, 0x88},
    0,
    1,
    0,
    0,
    0,
    {{0x3B, 0, 0, 0}, {0x3B, 0, 0, 0}, {0x3B, 0, 0, 0}},
    {0, 0, 0},
    0,
};

u8 D_8009BD58[8][2] = {
    {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 1}, {5, 1}, {6, 1}, {7, 0}};
