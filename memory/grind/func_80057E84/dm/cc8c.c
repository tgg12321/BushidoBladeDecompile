void func_80057CC8(NavPoly *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 *p;
    s32 pi;
    u16 cy;

    prev_idx = arg1 - 1;
    cx = arg0->vtx[arg1][0];
    cy = arg0->vtx[arg1][1];

    if ((s16) prev_idx < 0) {
        prev_idx = arg0->nvtx - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0->nvtx) {
            next_idx = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(arg0->vtx[pi][0] - (s16) cx,
                      arg0->vtx[pi][1] - (s16) cy) & 0xFFF;
    /* The next vertex's address stays the integer sum, index first, that this line held before
     * arg0 was typed: every pointer spelling (vtx[k], *(k + vtx), &vtx[k][0], (u8 *)vtx + k * 4,
     * *(vtx + k), vtx[(s32)(k << 16) >> 16]) expands base first, and local-alloc ties the sum to
     * the dying table load (lw a1 / addu a1,a1,v1) instead of the shifted index (target lw a0 /
     * addu v1,v1,a0 at 0x80057D80 / 0x80057D88): score 4 each,
     * memory/grind/func_80057E84/dm/README.md. */
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)arg0->vtx);
    ang_next = ratan2(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        /* FAKE: `base` and `half` are fresh once-written/once-read named
         * intermediates for the antipode of ang_prev and half the angular gap
         * (named-intermediate family, pre-slim-2026-10-01:.claude/rules/no-new-park-categories.md:204
         * + the 2026-08-17 clarification at :208-229; both values are real and
         * appear in the target's own bytes, build_insns == target_insns == 111).
         * mechanism: local-alloc.c block_alloc -- they become BLOCK-LOCAL allocnos
         * (pseudos 82 and 83, "in block 5", tmp/grind/func_80057CC8/dumps/text1b.lreg
         * at the func_80057CC8 heading) that local-alloc seats before global.c runs;
         * collapsing them into one expression instead yields a single combine-folded
         * tree whose scratch is allocated globally and measures score 6.
         * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions);
         * both collapse spellings banked in
         * rejected/s46-collapse-base-half-splitinit-score6.c. */
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0->margin * 40;
    *arg2 = cx + ((scale * (s32)Judge[ang_mid & 0xFFF]) >> 12);
    *arg3 = cy + ((scale * (s32)Judge[((s16)ang_mid + 0x400) & 0xFFF]) >> 12);
}
