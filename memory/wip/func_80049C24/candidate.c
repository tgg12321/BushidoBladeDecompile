/* Candidate body for func_80049C24 — closes to sandbox 0 via two splits,
 * but cheat-reviewer returned NEEDS_USER on Change #1 (var_s7 split-init).
 * See meta.json + notes.md. Do NOT apply to src/ until user policy clears
 * the same-variable split-init-accumulation pattern.
 */
s32 func_80049C24(s32 arg0, s32 arg1) {
    s32 count;
    s32 temp_v0;
    s32 temp_a2;
    s32 var_s7;
    s32 v0;
    s32 var_fp;
    s32 var_s5;
    s32 var_s6;
    s32 var_s4;
    s32 var_s0;
    s32 var_s2;
    s32 var_s1;
    s32 var_s3;
    s32 v1;
    s32 hdr;
    s32 a0_arg;

    count = *(s32 *)arg0;
    var_s3 = arg1;
    temp_v0 = *(s32 *)(arg0 + (count * 4) + 4);
    temp_a2 = *(s32 *)(arg0 + 8);
    /* Change #1 — NEEDS_USER:
     *   HEAD: var_s7 = arg0 + temp_v0;
     *   CAND: var_s7 = arg0; var_s7 += temp_v0;
     * The split bumps pseudo 77's refs 3->5 in RTL, jumping its allocno
     * priority 329->1075 (formula log2(refs)*refs/livelen, global.c:611),
     * flipping var_s7 to $s7 / var_fp to $fp (matches target). Combine
     * folds back to one emitted insn. */
    var_s7 = arg0;
    var_s7 += temp_v0;
    v0 = *(s32 *)(arg0 + 4);
    var_fp = arg0 + v0;
    var_s5 = temp_a2 - v0;

    if (count >= 2) {
        var_s6 = arg0 + temp_a2;
        var_s4 = *(s32 *)(arg0 + 0xC) - temp_a2;
    } else {
        var_s6 = 0;
        var_s4 = 0;
    }

    var_s0 = D_800A33E8;
    var_s2 = D_800A33EA;
    var_s1 = var_s3 + 0xC;

    if (var_s0 == -1) {
        if (var_s2 == var_s0) {
            var_s0 = var_s2;
        } else {
            var_s2 = 0;
        }
    } else if (var_s2 == -1) {
        var_s0 = 0;
    } else if (var_s0 == var_s2) {
        var_s0 = 0;
        var_s2 = 0;
    } else if (var_s0 < var_s2) {
        var_s0 = 0;
        var_s2 = 1;
    } else if (var_s2 < var_s0) {
        var_s0 = 1;
        var_s2 = 0;
    } else {
        InitFadePanel();
    }

    /* Change #2 — reviewer PASS independently:
     *   HEAD: hdr = ((u32)~var_s0) >> 31;   (single statement, late)
     *   CAND: hoist hdr = ~var_s0;  early, then hdr = (u32)hdr >> 31; later.
     * Staged compound expression with live intermediate — gives sched1
     * room to place `nor` above `move v1,s3 / addiu s3,s3,4` (matches
     * target's .L80049D14 ordering). */
    hdr = ~var_s0;
    v1 = var_s3;
    var_s3 += 4;
    hdr = (u32)hdr >> 31;
    if (var_s2 >= 0) {
        hdr += 1;
    }
    *(s32 *)v1 = hdr;

    if (var_s0 >= 0) {
        *(s32 *)var_s3 = 2;
        var_s3 += 4;
        if (var_s0 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        saTan5TakeGetPos_80045230(a0_arg);
        var_s1 += tslGlobalMemFree_8005C2A8(var_s1, 2, var_s7);
    }

    if (var_s2 >= 0) {
        *(s32 *)var_s3 = 5;
        if (var_s2 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        saTan5TakeGetPos_80045230(a0_arg);
        var_s1 += tslGlobalMemFree_8005C2A8(var_s1, 5, var_s7);
    }
    return var_s1;
}
