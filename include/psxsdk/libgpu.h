#ifndef PSXSDK_LIBGPU_H
#define PSXSDK_LIBGPU_H

/* PsyQ LIBGPU types and entry points (Sony's libgpu.h; SOTN include/psxsdk/libgpu.h), spelled as
 * BB2's code already spells them: the types as the game and library code declared them, the
 * prototypes as the C definitions in src/main/psxsdk/libgpu/ and every caller's declaration
 * agree. */

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

/* LIBGPU's debug-print hook (a .data word; the LIBGPU modules call through it). */
extern void (*GPU_printf)();

extern void SetDispMask(s32);
extern void DrawSync(s32);
extern void DrawOTag(u32 *);
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern DISPENV *PutDispEnv(DISPENV *);
extern s32 MoveImage(RECT *, s32, s32);
extern u32 GetTPage(s32, s32, s32, s32);
extern u16 GetClut(s32, s32);
extern void SetPolyF4(u8 *);
extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32);

#endif /* PSXSDK_LIBGPU_H */
