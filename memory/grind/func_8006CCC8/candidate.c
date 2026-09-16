/* func_8006CCC8 — src/text1b.c — session s2 (structural)
 * Sandbox --disable all floor at end of s2: 77 (target_insns=189, build_insns=185).
 * s1 ended at 94 (no-C-body was 189). This candidate IS applied to src/text1b.c
 * as of end of s2 (2026-09-16) — it is the resident body, not a draft. NOTE the
 * s1 candidate.c was NEVER actually applied to HEAD (HEAD was still
 * INCLUDE_ASM at s2 dispatch — a STALE HEAD CLAIM the s2 consistency audit
 * flagged); this file replaces that stale claim with a real, re-verified
 * chassis.
 *
 * s2 changes from the s1 body (both measured this session, same chassis):
 *   1. H2 CONFIRMED + FIXED: added a `s32 t;` scratch local, multi-set
 *      non-consecutively within the `for (i...)` loop:
 *        t = arg2;                         (top of loop, feeds `lim`)
 *        ... (unchanged body) ...
 *        t = i;  func_8006CBD4(t, *arg1);  (the field28>=0 tail arm)
 *      This defeats loop.c's `move_movables` invariant-hoist classification
 *      for the `(s32)arg2` sign-extension pseudo (see hypotheses.md H2 for
 *      the dump-confirmed mechanism) — floor 94 -> 91.
 *   2. `i` widened from `s16` to `s32`: floor 91 -> 77 (build_insns 193 ->
 *      185, now UNDER target's 189 — see H3 in hypotheses.md). Target's
 *      $s1 (the `i` register) is used directly as a call argument
 *      (`addu $a0,$s1,$zero`, asm/funcs/func_8006CCC8.s:117) and as a raw
 *      32-bit shift-amount operand with no HI->SI sign-extension anywhere
 *      in its lifetime — strong evidence the original `i` was declared as
 *      a plain (32-bit) int, not a narrow type. `lim` was ALSO tried as
 *      s32 (neutral, no score change) and reverted to s16 to keep the
 *      m2c-reconstructed type where it costs nothing to do so.
 *
 * KNOWN REMAINING GAP (s2 evidence, not yet fixed — floor 77, 187 insns
 * still differ per triage; ~4-6 real diffs by hand-count of the disassembly
 * walked this session): the `for (j...)` inner loop (the two `D_800A3524`
 * record-update arms, field28==3 and field28==4) does not yet structurally
 * match target. Target (asm/funcs/func_8006CCC8.s:132-160, the
 * `.L8006CEAC` loop) loads BOTH the +0x17 and +0x1A/+0x1D bytes
 * unconditionally every iteration and combines with a mask computed ONCE
 * before the loop (`sllv $a2,$s6,$s5` = `0xF << fade`), selecting the
 * i==0-vs-else half via the AND source register ($a0 vs bare $v0/$v1),
 * whereas our build (tmp/grind/func_8006CCC8/dumps/text1b.s:16694-16716)
 * branches on `i==0` at the TOP of the loop body into two near-duplicate
 * lbu/andi/lbu sequences (0xF0 vs 0x0F masks) that both fall through to a
 * shared `.L1030` combine+store tail. Same fold precomputed
 * (`sll $6,$22,$21` before the loop matches target's `sllv` fold), but the
 * i==0 dispatch shape differs. NOT yet root-caused this session — next
 * session's first move should read `.combine`/`.cse` for this inner loop
 * (see hypotheses.md H4 frontier) before trying restructures by hand.
 *
 * Structure derived from m2c --valid-syntax over asm/funcs/func_8006CCC8.s
 * (s1 provenance, unchanged this session). Field layout cross-checked
 * against sibling func_8006CBD4 (same TU, same D_800A34FC / D_800A3524
 * struct idiom).
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 mask;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 t;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    fade = 0;
    for (i = 0, shift = 0, mask = 0; i < 2; i++, shift += 0x10, mask += 2) {
        t = arg2;
        lim = ((t >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) <= 0) {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = lim;
            } else {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = (s16)(*(s16 *)((u8 *)D_800A34FC + mask + 0x28) - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= lim) {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = 0;
            } else {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = (s16)(*(s16 *)((u8 *)D_800A34FC + mask + 0x28) + 1);
            }
        }

        if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) == 3) {
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + (*(rec + 0x1A) & (0xF << fade)));
                    } else {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + (*(rec + 0x1A) & (0xF << fade)));
                    }
                }
            }
        } else if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= 4) {
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) == 4) {
                if (*arg1 & (0x40 << shift)) {
                    func_8005C650(1, 0x7F, 0x7F);
                    for (j = 0; j < 3; j++) {
                        rec = (u8 *)D_800A3524 + j;
                        if (i == 0) {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + (*(rec + 0x1D) & (0xF << fade)));
                        } else {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + (*(rec + 0x1D) & (0xF << fade)));
                        }
                    }
                }
            }
        } else if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= 0 && (*arg1 & (0xF0 << shift))) {
            func_8005C650(0, 0x7F, 0x7F);
            t = i;
            func_8006CBD4(t, *arg1);
        }
        fade += 4;
    }
    return ret;
}
