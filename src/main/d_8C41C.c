/* Data-only file: .data 0x8009BC1C..0x8009BCD4 (ROM 0x8C41C), the SEL.BIN
 * menu's tables, read only by 5ED34.c (that menu's -G8 unit).
 * Owner ruling Q122 (data-only-no-sdata):
 * 1. One run: 596CC.o(.data) ends at 0x8009BC1C and 5ED34.o(.data) is empty;
 *    the next .data is 63D2C.c's D_8009BCD4. No uninitialized objects.
 * 2. Small objects in .data: D_8009BC38 (u8[6], indexed by D_800A3590[i],
 *    0..5 like D_8009BC94[i]; the 2 zero bytes after it are the word
 *    alignment of D_8009BC40, another type with other readers) and
 *    D_8009BCD0 (s16[2], scroll state menu_UpdateChoiceGrid writes; the
 *    run's end). Under -G8 the original compiler puts both in .sdata, so no -G8
 *    file defined them.
 * 3. Neighbours in .data link order: 596CC.c (its own .sdata, gp accesses)
 *    and 5ED34.c (its own .sdata / .sbss, gp accesses), both -G8 code
 *    modules with adjacent .text, so neither owns these objects.
 * Readers: D_8009BC1C (func_8006EACC), D_8009BC38 (menu_UpdateRowCursors),
 * D_8009BC40 (func_8006E534, func_8006ECF4, menu_UpdateEntryGrid,
 * menu_UpdateEntryGridCursors), D_8009BC7C (those, func_8006F100,
 * menu_UpdateRowCursors, func_80071C20, func_80071C4C), D_8009BC94
 * (func_8006F100, func_80071C4C), D_8009BCC4 / D_8009BCD0
 * (menu_UpdateChoiceGrid). */
#include "common.h"
#include "bb2.h"

void menu_UpdateEntryGrid(Unk8006EACCRec *);
void menu_UpdateRow(Unk8006EACCRec *);
void func_8006F100(Unk8006EACCRec *);
void func_80071C4C(Unk8006EACCRec *);
void func_80072084(Unk8006EACCRec *);
void func_800720AC(Unk8006EACCRec *);
void func_800720D4(Unk8006EACCRec *);

/* the SEL.BIN menu's per-mode update handlers, indexed by D_800A3580 */
void (*D_8009BC1C[7])(Unk8006EACCRec *) = {
    menu_UpdateEntryGrid, menu_UpdateRow, func_8006F100, func_80071C4C,
    func_80072084,        func_800720AC,  func_800720D4,
};

u8 D_8009BC38[6] = {4, 3, 0, 2, 1, 5};

SelectEntryE534 D_8009BC40[5][6] = {
    {{0x00, 0x00},
     {0x09, 0x09},
     {0x0A, 0x0C},
     {0x11, 0x13},
     {0x15, 0x1A},
     {0x14, 0x18}},
    {{0x02, 0x02},
     {0x07, 0x07},
     {0x0B, 0x0D},
     {0x0D, 0x0F},
     {0x15, 0x1A},
     {0x14, 0x18}},
    {{0x01, 0x01},
     {0x08, 0x08},
     {0x0C, 0x0E},
     {0x12, 0x14},
     {0x15, 0x1A},
     {0x14, 0x18}},
    {{0x03, 0x03},
     {0x06, 0x06},
     {0x10, 0x12},
     {0x0F, 0x11},
     {0x15, 0x1A},
     {0x14, 0x18}},
    {{0x04, 0x04},
     {0x05, 0x05},
     {0x13, 0x15},
     {0x0E, 0x10},
     {0x15, 0x1A},
     {0x14, 0x18}},
};

u8 D_8009BC7C[24] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0};

Unk8009BC94Record D_8009BC94[2][6] = {
    {{-0x38, -0x1F},
     {-0x06, -0x0E},
     {-0x1A, -0x10},
     {-0x44, -0x20},
     {-0x22, -0x20},
     {-0x38, -0x1F}},
    {{0x38, -0x1F},
     {0x06, -0x0E},
     {0x1A, -0x10},
     {0x44, -0x20},
     {0x22, -0x20},
     {0x38, -0x1F}},
};

s16 D_8009BCC4[3][2] = {{0x00, 0x17}, {0x99, 0x03}, {0xA0, 0x24}};

s16 D_8009BCD0[2] = {0, 0};
