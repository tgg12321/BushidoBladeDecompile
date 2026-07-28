/* func_800600C8 — BEST FORM s1 (recon): sandbox --disable all = 6, 121/121 insns.
 * 100% cheat-free (both source cheats removed: empty-if dead-read of hi, 0&0xFFFFu mask).
 * APPLIED IN src/text1b.c as of s1 end.
 *
 * Key levers that got 13 -> 6:
 *  1. Per-arm s.width stores (NO width local) at BOTH call sites -> jump2
 *     cross-jump merges the identical `sw v0,0x30(sp)` suffixes -> restores the
 *     `j`+delay-slot li diamond, width lands in $v0, sw before addiu a0
 *     (kills 2 regfix-swap clusters + the v0/v1 renames + 1 missing insn).
 *  2. d1/d0 init order `s.d1 = hi; s.d0 = hi;` (4A then 48, target order) with
 *     `s.p0 = &D_8009B6FC;` moved AFTER the pair: flow.c's single-slot
 *     last_mem_set dead-store tracker needs the p0 store between `s.d0 = hi`
 *     and `s.d0 = arg0%10` to evict the tracked slot, else d0-init is deleted
 *     (flow.c:1740 insn_dead_p / 1988 mark_set_1).
 *  3. hi=arg0 as pair carrier -> restores `move v0,s1` copy + cur_tex copy
 *     appears in prologue (addu t1,s2) + t0/t1/t2 all land on target regs.
 *
 * REMAINING 6 diffs = ONE block transposition:
 *   ours:   [move v0,s1; sh v0,0x4A; sh v0,0x48] then [lui/addiu v0,B6FC; sw v0,0x18]
 *   target: [lui/addiu v0,B6FC; sw v0,0x18] then [addu v0,s1; sh 0x4A; sh 0x48]
 * sched1 tie: both chains priority-tied, rank_for_schedule falls to INSN_LUID
 * (source order) — but flow REQUIRES pair-before-p0 in source. See hypotheses.md.
 */
typedef struct {
    s32 *p0;
    s32 *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg2;
    s32 width;
    s32 zero1C;
    s32 pad20;
    s32 pad24;
    s8 byte28;
    s8 padpad[7];
    s16 d0;
    s16 d1;
} S60C8;
s32 func_800600C8(s32 arg0, s32 arg1, s32 arg2)
{
    S60C8 s;
    s32 dist_off = arg1 + 0xB4;
    s32 end_off = arg1 + 0xC0;
    s32 cur_tex = arg1;
    s32 i;
    s16 hi;

    s.p0 = &D_8009B6F0;
    s.byte28 = 0;
    s.zero10 = 0;
    s.zero1C = 0;
    s.arg2 = arg2;
    if (arg0 < 0xA) {
        s.width = 0x93;
    } else {
        s.width = 0xA3;
    }
    s.p1 = &D_8009B758;
    s.in_tex = cur_tex;
    cur_tex = func_8007352C((s32)&s);
    hi = arg0;
    s.d1 = hi;
    s.d0 = hi;
    s.p0 = &D_8009B6FC;
    s.d0 = ((s16)arg0) % 10;
    hi = ((s16)arg0) / 10;
    s.d1 = hi % 10;
    i = 0;
loop_60C8:
    s.p1 = (s32 *)((s32)&D_8009B708 + ((&s.d0)[i] * 8));
    if (arg0 < 0xA) {
        s.width = 0x64;
    } else {
        s.width = (((1 - i) << 2) << 3) + 0x54;
    }
    s.in_tex = cur_tex;
    cur_tex = func_8007352C((s32)&s);
    if (s.d1 != 0) {
        i += 1;
        if (i < 2) goto loop_60C8;
    }
    initTexPage(dist_off, 1, 0, saMotionSet((s32 *)&D_8009B6F0, 0), 0);
    ot_Link(D_800A374C + (arg2 * 4), dist_off);
    return end_off - arg1;
}
