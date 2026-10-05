/* PsyQ 4.0 LIBGPU PRIM: the primitive and environment helpers (GetTPage .. DumpDispEnv). .text
 * 0x8007A788..0x8007AE7C, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include <psxsdk/libgpu.h>
#include "psx.h"

/* .rodata 0x80015D58..0x80015E28: this module's strings (moved from src/text1a_b_post_rodata.c, Q106 D4:
 * every reader is in this file, in link order). */

/* D_80015D58: 1 string(s), 24B @ 0x80015D58 */
const char D_80015D58[24] =
    "tpage: (%d,%d,%d,%d)\n\0\0\0"
    ;

/* D_80015D70: 1 string(s), 16B @ 0x80015D70 */
const char D_80015D70[16] =
    "clut: (%d,%d)\n\0\0"
    ;

/* D_80015D80: 1 string(s), 24B @ 0x80015D80 */
const char D_80015D80[24] =
    "clip (%3d,%3d)-(%d,%d)\n\0"
    ;

/* D_80015D98: 1 string(s), 16B @ 0x80015D98 */
const char D_80015D98[16] =
    "ofs  (%3d,%3d)\n\0"
    ;

/* D_80015DA8: 1 string(s), 24B @ 0x80015DA8 */
const char D_80015DA8[24] =
    "tw   (%d,%d)-(%d,%d)\n\0\0\0"
    ;

/* D_80015DC0: 1 string(s), 12B @ 0x80015DC0 */
const char D_80015DC0[12] =
    "dtd   %d\n\0\0\0"
    ;

/* D_80015DCC: 1 string(s), 12B @ 0x80015DCC */
const char D_80015DCC[12] =
    "dfe   %d\n\0\0\0"
    ;

/* D_80015DD8: 1 string(s), 28B @ 0x80015DD8 */
const char D_80015DD8[28] =
    "disp   (%3d,%3d)-(%d,%d)\n\0\0\0"
    ;

/* D_80015DF4: 1 string(s), 28B @ 0x80015DF4 */
const char D_80015DF4[28] =
    "screen (%3d,%3d)-(%d,%d)\n\0\0\0"
    ;

/* D_80015E10: 1 string(s), 12B @ 0x80015E10 */
const char D_80015E10[12] =
    "isinter %d\n\0"
    ;

/* D_80015E1C: 1 string(s), 12B @ 0x80015E1C */
const char D_80015E1C[12] =
    "isrgb24 %d\n\0"
    ;

u32 GetTPage(s32 a0, s32 a1, s32 a2, s32 a3) {
    return ((a0 & 3) << 7) | ((a1 & 3) << 5) | ((a3 & 0x100) >> 4) | ((a2 & 0x3FF) >> 6) | ((a3 & 0x200) << 2);
}
u16 GetClut(s32 a0, s32 a1) {
    return (a1 << 6) | ((a0 >> 4) & 0x3F);
}
void DumpTPage(s32 a0) {
    u32 val = a0 & 0xFFFF;
    GPU_printf(&D_80015D58, (val >> 7) & 3, (val >> 5) & 3, (val << 6) & 0x7C0,
               ((val << 4) & 0x100) + ((val >> 2) & 0x200));
}
void DumpClut(s32 a0) {
    GPU_printf(&D_80015D70, (a0 & 0x3F) << 4, (a0 & 0xFFFF) >> 6);
}
u32 NextPrim(u32 *a0) {
    return (*a0 & OT_ADDR_MASK) | OT_TAG_BASE;
}

u32 IsEndPrim(u32 *a0) {
    return (*a0 & OT_ADDR_MASK) == OT_ADDR_MASK;
}
void AddPrim(void *ot, void *p) {
    OTag *a0 = ot;
    OTag *a1 = p;

    a1->addr = a0->addr;
    a0->addr = (u32)a1;
}
void AddPrims(OTag *a0, u32 a1, OTag *a2) {
    a2->addr = a0->addr;
    a0->addr = a1;
}
void CatPrim(u32 *a0, u32 a1) {
    *a0 = (*a0 & OT_TAG_MASK) | (a1 & OT_ADDR_MASK);
}
void TermPrim(u32 *a0) {
    *a0 |= OT_ADDR_MASK;
}
void SetSemiTrans(void *p, s32 abe) {
    setSemiTrans(p, abe);
}

void SetShadeTex(void *p, s32 tge) {
    setShadeTex(p, tge);
}

void SetPolyF3(POLY_F3 *p) {
    setPolyF3(p);
}

void SetPolyFT3(POLY_FT3 *p) {
    setPolyFT3(p);
}

void SetPolyG3(POLY_G3 *p) {
    setPolyG3(p);
}

void SetPolyGT3(POLY_GT3 *p) {
    setPolyGT3(p);
}

void SetPolyF4(POLY_F4 *p) {
    setPolyF4(p);
}

void SetPolyFT4(POLY_FT4 *p) {
    setPolyFT4(p);
}

void SetPolyG4(POLY_G4 *p) {
    setPolyG4(p);
}

void SetPolyGT4(POLY_GT4 *p) {
    setPolyGT4(p);
}

void SetSprt8(SPRT_8 *p) {
    setSprt8(p);
}

void SetSprt16(SPRT_16 *p) {
    setSprt16(p);
}

void SetSprt(SPRT *p) {
    setSprt(p);
}

void SetTile1(TILE_1 *p) {
    setTile1(p);
}

void SetTile8(TILE_8 *p) {
    setTile8(p);
}

void SetTile16(TILE_16 *p) {
    setTile16(p);
}

void SetTile(TILE *p) {
    setTile(p);
}

void SetLineF2(LINE_F2 *p) {
    setLineF2(p);
}

void SetLineG2(LINE_G2 *p) {
    setLineG2(p);
}

void SetLineF3(LINE_F3 *p) {
    setLineF3(p);
}

void SetLineG3(LINE_G3 *p) {
    setLineG3(p);
}

void SetLineF4(LINE_F4 *p) {
    setLineF4(p);
}

void SetLineG4(LINE_G4 *p) {
    setLineG4(p);
}

void SetDrawTPage(u8 *a0, s32 a1, s32 a2, u32 a3) {
    u32 cmd;
    u32 val;
    a0[3] = 1;
    cmd = GP0_DRAW_MODE;
    if (a2) {
        cmd = (GP0_DRAW_MODE | GPU_DRAW_MODE_DITHER);
    }
    if (a1) {
        /* FAKE: `a3 & MASK` duplicated into both arms (duplicated-statement-into-arms,
         * owner ruling) instead of the compound `val = a3 & MASK; if (a1)
         * val |= TEXOFF;`. The single-def compound form ties val's andi dest into dying
         * $a3 via GCC 2.7.2 local-alloc combine_regs; the two-arm spelling
         * keeps the masked temp in $v0 so the final IOR reproduces target's
         * cmd=$v1/val=$v0 `or v0,v1,v0`. Byte-neutral: one andi (delay slot), 11 insns. */
        val = (a3 & GPU_DRAW_MODE_MASK) | GPU_DRAW_MODE_TEXOFF;
    } else {
        val = a3 & GPU_DRAW_MODE_MASK;
    }
    *(u32 *)(a0 + 4) = cmd | val;
}
/* Copy packet: tag followed by five GPU command words. */
void SetDrawMove(DR_MOVE *a0, RECT *a1, u32 a2, u32 a3) {
    s32 size = 5;
    if (a1->w == 0 || a1->h == 0) {
        size = 0;
    }
    a0->code[0] = OT_TERMINATOR;
    a0->code[1] = OT_TAG_BASE;
    /* FAKE: SDK setlen view of the packet's tag word; the explicit mask-and-or on a0->tag scores 13. */
    /* SOTN: include/psxsdk/libgpu.h:87 @db41b28eee52969244a52cc269c8163d1ed8826a (PS1 use: src/main/psxsdk/libgpu/sys.c:287) */
    ((OTag *)a0)->len = size;
    /* FAKE: packed RECT word read follows matched Sony-library precedent; (y << 16) | (u16)x scores 16. */
    /* SOTN: src/main/psxsdk/libgpu/sys.c:275 @db41b28eee52969244a52cc269c8163d1ed8826a */
    a0->code[2] = *(s32 *)&a1->x;
    a0->code[3] = (a3 << 16) | (a2 & 0xFFFF);
    /* FAKE: packed RECT word read follows matched Sony-library precedent; (h << 16) | (u16)w scores 4. */
    /* SOTN: src/main/psxsdk/libgpu/sys.c:277 @db41b28eee52969244a52cc269c8163d1ed8826a */
    a0->code[4] = *(s32 *)&a1->w;
}

void SetDrawLoad(u32 *a0, s16 *a1) {
    u32 nwords;
    s32 size;
    u32 *end;
    nwords = (a1[2] * a1[3] + 1) / 2;
    size = nwords + 4;
    if (nwords >= 13) {
        size = 0;
    }
    ((u8 *)a0)[3] = size;
    a0[1] = GP0_COPY_RECT_C2V;
    a0[2] = *(u32 *)&a1[0];
    a0[3] = *(u32 *)&a1[2];
    end = a0 + size;
    *end = OT_TERMINATOR;
}
s32 MargePrim(u8 *a0, u32 *a1) {
    s32 size;
    size = a0[3] + ((u8 *)a1)[3] + 1;
    if (size >= 17) {
        return -1;
    }
    a0[3] = size;
    *a1 = 0;
    return 0;
}
void DumpDrawEnv(s16 *a0) {
    u32 val;
    GPU_printf(&D_80015D80, a0[0], a0[1], a0[2], a0[3]);
    GPU_printf(&D_80015D98, a0[4], a0[5]);
    GPU_printf(&D_80015DA8, a0[6], a0[7], a0[8], a0[9]);
    GPU_printf(&D_80015DC0, ((u8 *)a0)[0x16]);
    GPU_printf(&D_80015DCC, ((u8 *)a0)[0x17]);
    val = ((u16 *)a0)[0xA];
    GPU_printf(&D_80015D58, (val >> 7) & 3, (val >> 5) & 3, (val << 6) & 0x7C0,
               ((val << 4) & 0x100) + ((val >> 2) & 0x200));
}
void DumpDispEnv(s16 *a0) {
    GPU_printf(&D_80015DD8, a0[0], a0[1], a0[2], a0[3]);
    GPU_printf(&D_80015DF4, a0[4], a0[5], a0[6], a0[7]);
    GPU_printf(&D_80015E10, ((u8 *)a0)[0x10]);
    GPU_printf(&D_80015E1C, ((u8 *)a0)[0x11]);
}
