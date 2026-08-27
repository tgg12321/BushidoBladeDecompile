/* func_800485EC — candidate at sandbox --disable all == 0 (session s2, 2026-08-27).
 * APPLIED IN src/text1b.c as of this session (replaces the INCLUDE_ASM at the
 * former line 346). sandbox 0 at 68/68 insns; matched caller func_80048530
 * re-verified sandbox 0 with the coupled extern edits in place.
 *
 * SUPERSEDES the s1 candidate whose tail `tim += 2; p = tim;` was layer-1
 * FAILed (decisions.md 2026-08-27 14:00) and is BANNED for this function.
 * This form uses the natural tail def `p = tim + 2;` (the spelling layer-1
 * itself named as natural) plus an F6-sanctioned exact cancellation pair
 * `tim++; tim--;` between the def and the reads.
 *
 * WHY the pair is needed (dump-proven, tmp/grind/func_800485EC/s2/):
 * cse1's fold_rtx PLUS-association (cse.c:5589-5666; applied UNCOSTED to
 * memory addresses via find_best_addr, cse.c:2663-2665) rewrites the pixel
 * reads (plus p 2)/(reg p) onto tim whenever p's table equivalent
 * (plus tim 8) passes exp_equiv_p validity; even the bare-reg lw 0(p) is
 * hijacked by find_best_addr's equal-ADDRESS_COST / higher-rtx_cost
 * preference (cse.c:2610-2618, 2719-2721). The pair bumps reg_tick(tim),
 * invalidating the equivalence at the reads, so the emitted tail is target's
 * addiu v1,s1,8; lhu 2(v1); lw 0(v1); addiu v1,v1,4. The pair itself is
 * byte-free: present through cse1/cse2 (insns 126/129 in text1b.cse/.cse2),
 * deleted by flow.c dead-store elimination (NOTE_INSN_DELETED in .flow)
 * because tim is dead after the join — nothing reaches RA/emission.
 * Family: "Semantically-null fabricated statement pairs (cancellation-pair)"
 * — .claude/rules/no-new-park-categories.md:370-382, owner ruling 2026-08-18,
 * F6 ESTABLISHED, exact `i++; i--;` shape (`+= 2 / -= 2` also measured 0 but
 * the exact ++/-- exhibit shape is used; the banned `+= 2 / -= 1` NET-NONZERO
 * lineage is a different, still-banned construct).
 *
 * COUPLED EDITS the integrator must keep (all in src/text1b.c, all verified
 * codegen-neutral):
 *  1. line ~311: `extern void func_800485EC();` (was a (s32 x6) prototype).
 *     K&R definition + unprototyped extern keep the matched caller
 *     func_80048530's default-promotion codegen byte-identical.
 *  2. former line 5947: `extern u32 GetClut(s32, s32);` (was `extern s32
 *     GetClut(u16, u16);`, zero call sites). Promoted-arg prototypes for
 *     GetClut/GetTPage above the function let the lh-fed args pass unmasked.
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
        p = tim + 2;
        /* !FAKE: cancellation pair (sanctioned family: semantically-null
         * fabricated statement pair, .claude/rules/no-new-park-categories.md:370-382,
         * owner ruling 2026-08-18, F6 ESTABLISHED — exact `i++; i--;` shape).
         * What: net-zero adjacent same-variable inc/dec of tim, byte-free
         * (survives cse1/cse2, then flow.c dead-store elimination deletes both:
         * NOTE_INSN_DELETED in .flow dump, tmp/grind/func_800485EC/dumps/).
         * Mechanism: cse.c fold_rtx PLUS-association (cse.c:5589-5666, applied
         * uncosted to addresses via find_best_addr, cse.c:2663) rewrites the
         * pixel-block reads onto tim whenever p's recorded equivalent
         * (plus tim 8) is valid; the pair bumps reg_tick(tim) so exp_equiv_p
         * invalidates that equivalence and the reads keep p as base, matching
         * target's addiu v1,s1,8 + lhu 2(v1)/lw 0(v1)/addiu v1,v1,4.
         * Lever-exhaustion: memory/grind/func_800485EC/hypotheses.md s1-s2 —
         * natural fresh-def folds (7), tim-walker misallocates to s1 (39),
         * live tim->pixdata routing cascades (22), def-in-arms leaves two
         * unmergeable addius (3), full-tail duplication into arms (16); the
         * cse.c mechanism proof shows every join-local p==tim+K chain folds. */
        tim++;
        tim--;
        spr->x = x;
        spr->y = y;
        spr->h = ((u16 *)p)[1];
        spr->w = *p++;
        spr->pixdata = p;
        spr->tpage = GetTPage(spr->mode, 0, spr->x & 0xFFC0, spr->y & 0xFF00);
    }
}
