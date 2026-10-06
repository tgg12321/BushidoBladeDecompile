#ifndef PSXSDK_LIBGPU_H
#define PSXSDK_LIBGPU_H

/* PsyQ LIBGPU types, macros and entry points (Sony's libgpu.h; SOTN include/psxsdk/libgpu.h).
 * Each prototype agrees with its C definition in src/main/psxsdk/libgpu/; where that differs
 * from PsyQ's LIBGPU.H spelling the entry carries a PsyQ: note. */

#include "common.h"

/* PS1 ordering-table / primitive tag word -- the head word of PsyQ
 * LIBGPU.H's P_TAG (`unsigned addr:24; unsigned len:8;`): next-packet
 * address in the LOW 24 bits, packet word count in the HIGH 8.
 *
 * FIELD ORDER NOTE (updated 2026-08-04, -mel adoption): cc1psx (PsyQ
 * GCC 2.7.2.SN) allocates the FIRST-declared bitfield at the LOW bits,
 * so PsyQ's addr-first declaration puts addr low. Before 2026-08-04 our
 * fork ran with a big-endian target default and allocated HIGH-first,
 * which forced a reversed (len-first) declaration as compensation
 * (probe-verified 2026-06-11). With -mel in CC_FLAGS the fork allocates
 * LOW-first exactly like cc1psx, so the ORIGINAL PsyQ field order is
 * restored below; see pre-slim-2026-10-01:.claude/rules/bitfield-direction-divergence.md. */
typedef struct {
    u32 addr : 24; /* LOW 24 bits: next-packet address */
    u32 len : 8;   /* HIGH 8 bits: packet word count */
} OTag;

/* PsyQ LIBGPU.H P_TAG: the primitive header -- the OT tag word (next-packet address low 24
 * bits, packet word count high 8) and the first colour word with the GPU command code. */
typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

/* PsyQ LIBGPU.H primitive handling macros (verbatim SDK definitions). */
#define setlen(p, _len) (((P_TAG *)(p))->len = (u8)(_len))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))
#define getcode(p) (u8)(((P_TAG *)(p))->code)
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))

#define setPolyF3(p) setlen(p, 4), setcode(p, 0x20)
#define setPolyFT3(p) setlen(p, 7), setcode(p, 0x24)
#define setPolyG3(p) setlen(p, 6), setcode(p, 0x30)
#define setPolyGT3(p) setlen(p, 9), setcode(p, 0x34)
#define setPolyF4(p) setlen(p, 5), setcode(p, 0x28)
#define setPolyFT4(p) setlen(p, 9), setcode(p, 0x2c)
#define setPolyG4(p) setlen(p, 8), setcode(p, 0x38)
#define setPolyGT4(p) setlen(p, 12), setcode(p, 0x3c)
#define setSprt8(p) setlen(p, 3), setcode(p, 0x74)
#define setSprt16(p) setlen(p, 3), setcode(p, 0x7c)
#define setSprt(p) setlen(p, 4), setcode(p, 0x64)
#define setTile1(p) setlen(p, 2), setcode(p, 0x68)
#define setTile8(p) setlen(p, 2), setcode(p, 0x70)
#define setTile16(p) setlen(p, 2), setcode(p, 0x78)
#define setTile(p) setlen(p, 3), setcode(p, 0x60)
#define setLineF2(p) setlen(p, 3), setcode(p, 0x40)
#define setLineG2(p) setlen(p, 4), setcode(p, 0x50)
#define setLineF3(p) setlen(p, 5), setcode(p, 0x48), (p)->pad = 0x55555555
#define setLineG3(p) setlen(p, 7), setcode(p, 0x58), (p)->pad = 0x55555555
#define setLineF4(p) setlen(p, 6), setcode(p, 0x4c), (p)->pad = 0x55555555
#define setLineG4(p) setlen(p, 9), setcode(p, 0x5c), (p)->pad = 0x55555555
#define setSemiTrans(p, abe)     ((abe) ? setcode(p, getcode(p) | 0x02) : setcode(p, getcode(p) & ~0x02))
#define setShadeTex(p, tge)     ((tge) ? setcode(p, getcode(p) | 0x01) : setcode(p, getcode(p) & ~0x01))

/* PsyQ LIBGPU.H environment types (public header layouts). */
typedef struct {
    s16 x, y, w, h;
} RECT;

typedef struct {
    u32 tag;
    u32 code[15];
} DR_ENV; /* 0x40 */

typedef struct {
    RECT clip;     /* +0x00 */
    s16 ofs[2];    /* +0x08 */
    RECT tw;       /* +0x0C */
    u16 tpage;     /* +0x14 */
    u8 dtd;        /* +0x16 */
    u8 dfe;        /* +0x17 */
    u8 isbg;       /* +0x18 */
    u8 r0, g0, b0; /* +0x19 */
    DR_ENV dr_env; /* +0x1C */
} DRAWENV; /* 0x5C */

/* PsyQ LIBGPU.H primitive/environment colour macro (verbatim SDK definition). */
#define setRGB0(p, _r0, _g0, _b0) (p)->r0 = _r0, (p)->g0 = _g0, (p)->b0 = _b0

typedef struct {
    RECT disp;   /* +0x00 */
    RECT screen; /* +0x08 */
    u8 isinter;  /* +0x10 */
    u8 isrgb24;  /* +0x11 */
    u8 pad0;     /* +0x12 */
    u8 pad1;     /* +0x13 */
} DISPENV; /* 0x14 */

/* PsyQ LIBGPU.H drawing-environment primitives: the OT tag word, then two GPU command words. */
typedef struct { u32 tag; u32 code[2]; } DR_MODE;   /* Drawing Mode */
typedef struct { u32 tag; u32 code[2]; } DR_TWIN;   /* Texture Window */
typedef struct { u32 tag; u32 code[2]; } DR_AREA;   /* Drawing Area */
typedef struct { u32 tag; u32 code[2]; } DR_OFFSET; /* Drawing Offset */

typedef struct { u32 tag; u32 code[2]; } DR_PRIO;   /* Mask Priority */

/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */
typedef struct { u32 tag; u32 code[5]; } DR_MOVE;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

/* PsyQ LIBGPU.H POLY_FT3 (0x20 bytes), a flat textured triangle: u16 clut at +0xE, u16 tpage
 * at +0x16, v0/v1/v2 at +0xD/+0x15/+0x1D. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} POLY_FT3;

/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes), a flat textured quad: u16 clut at +0xE, u16 tpage at
 * +0x16, v0/v1/v2/v3 at +0xD/+0x15/+0x1D/+0x25. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;

/* PsyQ LIBGPU.H POLY_GT3 (0x28 bytes), a gouraud textured triangle: u16 clut at +0xE, u16 tpage
 * at +0x1A, v0/v1/v2 at +0xD/+0x19/+0x25. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
} POLY_GT3;

/* PsyQ LIBGPU.H POLY_GT4 (0x34 bytes), a gouraud textured quad: u16 clut at +0xE, u16 tpage at
 * +0x1A, v0/v1/v2/v3 at +0xD/+0x19/+0x25/+0x31. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
    u8 r3, g3, b3, p3;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad3;
} POLY_GT4;

/* PsyQ LIBGPU.H primitive layouts (the SDK's members; tag = the OT tag word). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
} POLY_F3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} POLY_G3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} POLY_F4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} SPRT_8;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} SPRT_16;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
} TILE_1;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
} TILE_8;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
} TILE_16;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
} LINE_F2;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
} LINE_G2;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    u32 pad;
} LINE_F3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u32 pad;
} LINE_G3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
    u32 pad;
} LINE_F4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 r3, g3, b3, p3;
    s16 x3, y3;
    u32 pad;
} LINE_G4;

/* LIBGPU's debug-print hook (a .data word; the LIBGPU modules call through it). */
extern void (*GPU_printf)(); /* PsyQ: int (*GPU_printf)(char *, ...) */

extern void SetDispMask(s32);
extern void DrawSync(s32); /* PsyQ: int DrawSync(int) */
extern void DrawOTag(u32 *);
extern u32 *ClearOTagR(u32 *, s32);
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern DISPENV *PutDispEnv(DISPENV *);
extern s32 MoveImage(RECT *, s32, s32);
extern u32 GetTPage(s32, s32, s32, s32); /* PsyQ: u_short GetTPage(int, int, int, int) */
extern u16 GetClut(s32, s32);
extern void SetSemiTrans(void *, s32);
extern void SetShadeTex(void *, s32);
extern void SetPolyF3(POLY_F3 *);
extern void SetPolyFT3(POLY_FT3 *);
extern void SetPolyG3(POLY_G3 *);
extern void SetPolyGT3(POLY_GT3 *);
extern void SetPolyF4(POLY_F4 *);
extern void SetPolyFT4(POLY_FT4 *);
extern void SetPolyG4(POLY_G4 *);
extern void SetPolyGT4(POLY_GT4 *);
extern void SetSprt8(SPRT_8 *);
extern void SetSprt16(SPRT_16 *);
extern void SetSprt(SPRT *);
extern void SetTile1(TILE_1 *);
extern void SetTile8(TILE_8 *);
extern void SetTile16(TILE_16 *);
extern void SetTile(TILE *);
extern void SetLineF2(LINE_F2 *);
extern void SetLineG2(LINE_G2 *);
extern void SetLineF3(LINE_F3 *);
extern void SetLineG3(LINE_G3 *);
extern void SetLineF4(LINE_F4 *);
extern void SetLineG4(LINE_G4 *);
extern void SetTexWindow(DR_TWIN *, RECT *);
extern void SetDrawArea(DR_AREA *, RECT *);
extern void SetDrawMode(DR_MODE *, s32, s32, s32, RECT *);
extern void SetDrawOffset(DR_OFFSET *, s16 *); /* PsyQ: u_short *ofs (sys.c's definition loads them signed) */
extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32); /* PsyQ: (DR_MOVE *, RECT *, int, int) */

#endif /* PSXSDK_LIBGPU_H */
