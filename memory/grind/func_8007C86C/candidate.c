/* func_8007C86C — BEST LEGITIMATE FORM (banked s1 recon, 2026-08-10):
 * sandbox --disable all == 5, build_insns 50 vs target 51, 21 rules dropped,
 * cheat-asm stripped 153. Measured at HEAD 2d6270da THIS session
 * (tmp/grind/func_8007C86C/s1/ours_5form_50.txt vs target_51.s).
 *
 * This is the sibling func_8007C7A0's floor-5 chassis (its candidate.c,
 * re-banked there s11) with only the GPU command constant swapped
 * (0xE3000000 -> 0xE4000000). The twins are verbatim-linked from the same
 * Sony PsyQ 4.0 LIBGPU SYS module and move in exact lock-step: same score
 * (5), same build count (50 vs 51), and the identical residual.
 *
 * THE RESIDUAL IS THE SAME BANNED FAMILY AS THE SIBLING'S. The single
 * missing instruction + cascading masked diffs = the X-clamp three-arm JOIN
 * TEMP + writeback copy (target: addiu v0,a2,-1 / move v0,a3 / move v0,zero
 * then move a3,v0; ours: arms write $a3 directly, one insn fewer). Every C
 * dataflow that materializes that join routes a distinct temp back into a
 * live-initialized variable — the join-temp-writeback family banned by
 * layer-1 rulings on func_8007C7A0 (2026-08-10, docs/grind/decisions.md
 * ~L4177/L4249/L4253) under ANY spelling, including the x/tx param-alias,
 * the temp->param writeback, and the published-SOTN ternary self-read clamp.
 * Do NOT re-propose any of them here; do NOT invent a new spelling of the
 * same dataflow. func_8007C7A0 is OWNER-ESCALATED on exactly this question
 * (decisions.md 2026-08-10 17:31 ESCALATE entry); whatever the owner rules
 * there governs this twin identically.
 *
 * Y-clamp, dispatch (wide-mask andi 0xFFF in the dispatch delay slot via
 * cross-jump of the per-arm returns), and the shared OR tail all match
 * target 1:1 from this body. Only the X join is open.
 */

/* PsyQ 4.0 LIBGPU SYS: get_ce (static) — verbatim-linked Sony object
 * (census 2026-07-09); C ref: ground-up reconstruction (sibling get_cs
 * chassis; the published SOTN reference build clamps against constants
 * and lost when measured on the sibling — different library build). */
s32 func_8007C86C(s16 arg0, s16 arg1)
{
    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            arg0 = D_8009BE78 - 1;
        }
    } else {
        arg0 = 0;
    }

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    if ((u32)(D_8009BE74 - 1) < 2U) {
        s32 hi = arg1 & 0xFFF;
        s32 lo;
        hi = hi << 12;
        lo = arg0 & 0xFFF;
        lo = lo | 0xE4000000;
        return hi | lo;
    } else {
        s32 hi = arg1 & 0x3FF;
        s32 lo;
        hi = hi << 10;
        lo = arg0 & 0x3FF;
        lo = lo | 0xE4000000;
        return hi | lo;
    }
}
