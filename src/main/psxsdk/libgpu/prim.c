/* PsyQ 4.0 LIBGPU PRIM: the primitive and environment helpers (GetTPage .. DumpDispEnv). .text
 * 0x8007A788..0x8007AE7C, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include "gpu.h"
#include "psx.h"

/* Externs for globals */
extern s32 D_80015D58;
extern s32 D_80015D70;
extern s32 D_80015DD8;
extern s32 D_80015DF4;
extern s32 D_80015E10;
extern s32 D_80015E1C;
extern s32 D_80015D80;
extern s32 D_80015D98;
extern s32 D_80015DA8;
extern s32 D_80015DC0;
extern s32 D_80015DCC;

u32 GetTPage(s32 a0, s32 a1, s32 a2, s32 a3) {
    return ((a0 & 3) << 7) | ((a1 & 3) << 5) | ((a3 & 0x100) >> 4) | ((a2 & 0x3FF) >> 6) | ((a3 & 0x200) << 2);
}
u32 GetClut(s32 a0, s32 a1) {
    return ((a1 << 6) | ((a0 >> 4) & 0x3F)) & 0xFFFF;
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
void AddPrim(OTag *a0, OTag *a1) {
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
void SetSemiTrans(u8 *a0, s32 a1) {
    if (a1) {
        a0[7] |= 2;
    } else {
        a0[7] &= ~2;
    }
}

void SetShadeTex(u8 *a0, s32 a1) {
    if (a1) {
        a0[7] |= 1;
    } else {
        a0[7] &= ~1;
    }
}

void SetPolyF3(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x20;
}

void SetPolyFT3(u8 *p) {
    p[3] = 0x7;
    p[7] = 0x24;
}

void SetPolyG3(u8 *p) {
    p[3] = 0x6;
    p[7] = 0x30;
}

void SetPolyGT3(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x34;
}

void SetPolyF4(u8 *p) {
    p[3] = 0x5;
    p[7] = 0x28;
}

void SetPolyFT4(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x2C;
}

void SetPolyG4(u8 *p) {
    p[3] = 0x8;
    p[7] = 0x38;
}

void SetPolyGT4(u8 *p) {
    p[3] = 0xC;
    p[7] = 0x3C;
}

void SetSprt8(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x74;
}

void SetSprt16(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x7C;
}

void SetSprt(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x64;
}

void SetTile1(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x68;
}

void SetTile8(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x70;
}

void SetTile16(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x78;
}

void SetTile(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x60;
}

void SetLineF2(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x40;
}

void SetLineG2(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x50;
}

void SetLineF3(u8 *p) {
    p[3] = 0x5;
    p[7] = 0x48;
    *(u32 *)(p + 0x14) = GPU_DITHER_PATTERN;
}

void SetLineG3(u8 *p) {
    p[3] = 0x7;
    p[7] = 0x58;
    *(u32 *)(p + 0x1C) = GPU_DITHER_PATTERN;
}

void SetLineF4(u8 *p) {
    p[3] = 0x6;
    p[7] = 0x4C;
    *(u32 *)(p + 0x18) = GPU_DITHER_PATTERN;
}

void SetLineG4(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x5C;
    *(u32 *)(p + 0x24) = GPU_DITHER_PATTERN;
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
