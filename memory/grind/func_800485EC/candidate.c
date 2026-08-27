/* func_800485EC — candidate at sandbox --disable all == 0 (session s1, 2026-08-27).
 * APPLIED IN src/text1b.c as of this session (replaces the INCLUDE_ASM at the
 * former line 346). Byte-compare vs asm/funcs/func_800485EC.s: 68/68 words
 * identical except the two jal relocations and the internal `j` absolute
 * address (same relative offset 0xAC) — link-resolved.
 *
 * TIM-header parser: checks the 0x10 magic byte, fills a 0x20-byte sprite
 * header struct (mode/x/y/w/h + clut rect + tpage/clut ids + data pointers),
 * calling GetClut/GetTPage. No FAKE constructs; every statement live.
 *
 * COUPLED EDITS the integrator must keep (all in src/text1b.c, all verified
 * codegen-neutral this session):
 *  1. line ~311: `extern void func_800485EC();` (was a (s32 x6) prototype).
 *     The definition is K&R with s16/u16 narrow params; the unprototyped
 *     extern keeps the matched caller func_80048530's default-promotion
 *     codegen byte-identical (re-verified sandbox func_80048530 == 0).
 *     ABI evidence: caller sign-extends all four u16 loads (sll/sra 16) and
 *     stores full words for args 4/5, while the callee reads args 4/5 with
 *     lhu at +0x38/+0x3C — only K&R default-promotion semantics produce both.
 *  2. line ~5947: `extern u32 GetClut(s32, s32);` (was `extern s32
 *     GetClut(u16, u16);`). Zero call sites existed in this TU for the old
 *     decl (grep-verified); mirrors the real definition in src/gpu.c:329.
 *     Required because the u16 prototype would force andi masks at this
 *     function's lh-fed call (target has none) and conflicting decls in one
 *     TU are a cc1 error.
 */
typedef struct {
    /* 0x00 */ s16 mode;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 w;
    /* 0x08 */ s16 h;
    /* 0x0A */ s16 cx;
    /* 0x0C */ s16 cy;
    /* 0x0E */ s16 cw;
    /* 0x10 */ s16 ch;
    /* 0x12 */ u16 tpage;
    /* 0x14 */ u16 clut;
    /* 0x18 */ u32 *pixdata;
    /* 0x1C */ u32 *clutdata;
} TimHdr485;
extern u32 GetTPage(s32, s32, s32, s32);
extern u32 GetClut(s32, s32);
void func_800485EC(tim, spr, x, y, cx, cy)
u32 *tim;
TimHdr485 *spr;
s16 x, y;
u16 cx, cy;
{
    u32 flag;
    u32 *p;

    if (*(u8 *)tim++ == 0x10) {
        flag = *tim++;
        spr->mode = flag & 7;
        if (flag & 8) {
            u32 bnum;

            p = tim;
            bnum = *p;
            spr->cx = cx;
            spr->cy = cy;
            tim = p + (bnum >> 2);
            p += 2;
            spr->ch = ((u16 *)p)[1];
            spr->cw = *p++;
            spr->clutdata = p;
            spr->clut = GetClut(spr->cx, spr->cy);
        } else {
            spr->clut = 0;
        }
        tim += 2;
        p = tim;
        spr->x = x;
        spr->y = y;
        spr->h = ((u16 *)p)[1];
        spr->w = *p++;
        spr->pixdata = p;
        spr->tpage = GetTPage(spr->mode, 0, spr->x & 0xFFC0, spr->y & 0xFF00);
    }
}
