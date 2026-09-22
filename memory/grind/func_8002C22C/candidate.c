/* func_8002C22C (PutRobShadow) — s5 candidate (enumerate, 2026-09-21).
 * sandbox --disable all = 173 (target 252 insns, build 240 insns). NOT a match.
 * Down from s3/s4's floor 196. Full-diff read (`sandbox --diff`) on the s4
 * chassis showed the second if/else accumulate block's C statement order
 * ALREADY matched the target's read/add/store sequence for 10 of 12 field
 * updates (t0[0xA8/0xAC/0xB0/0xB8] += ...  literally 1:1), but the deferred
 * pair (t0[0xBC], t0[0xC0]) diverged: BC's deferred sum already read
 * t0[0xBC] INSIDE the arm (`sum_v0_2 = t0[0xBC/4] + d_tbl[0x244/4];`), but
 * C0's deferred sum only captured the table read
 * (`sum_a1_2 = d_tbl[0x248/4];`) and left the read of t0[0xC0] itself to the
 * POST-JOIN store statement (`t0[0xC0/4] += sum_a1_2;`). Ground-truth asm
 * (asm/funcs/func_8002C22C.s, .L8002C41C..8002C4E8 arm) shows the compiler
 * reads t0[0xC0] EARLY inside the arm (right after the round-1 add for
 * 0x23C) and carries that value in a register across the whole of round 2,
 * only adding the round-2 table term at the join (.L8002C5B8) — mirroring
 * exactly what the BC term already did. Because our C put t0[0xC0]'s read
 * textually AFTER the if/else join, cse1's basic-block boundary
 * (cse_end_of_basic_block, cse.c:8102-8184) could not hoist that load back
 * into the arm the way it does for a same-block reference — a second face
 * of the SAME mechanism the s2 zero-init lever already exploited
 * ([[cse-block-extension-controls-fold-span]]). Fix: read t0[0xC0] inside
 * each arm too (`sum_a1_2 = t0[0xC0/4] + d_tbl[0x248/4];`) and store the
 * plain sum at the join (`t0[0xC0/4] = sum_a1_2;` instead of `+=`). This is
 * ordinary C (named-intermediate pattern already established for BC in this
 * same function, no FAKE, no new construct) — measured 196 -> 173 this
 * session, confirmed stable across an operand-order swap probe (173 both
 * ways, so the residual left is NOT this term's operand order).
 *
 * REJECTED same session: moving the trailing unconditional stores
 * (0x1F800368/374/378, currently after the first if/else join) into each
 * arm, duplicating them using that arm's own already-computed a2/d_v1/d_a0/
 * d_v0/d_a1 (the natural next step of the SAME cse-block-extension
 * mechanism, per the s4 frontier's second item) — measured WORSE, 173 -> 214
 * (target_insns 252, build grew to 250: the duplication this time defeated
 * folding the compiler was otherwise doing across that join, net negative).
 * See rejected/dup-trailing-stores-into-arms-214.c. The frontier item this
 * disproves: the mechanism does not generalize to every join-adjacent
 * store — it depends on whether the value is later RE-READ across the join
 * (BC/C0 are; the a2/d_v1..d_a1 trailing stores are not re-read at all, so
 * duplicating them just doubles emitted stores with no CSE-defeat payoff).
 *
 * Residual 173 is still dominated by source-level hunks (29 of 36) per the
 * post-fix `sandbox --diff` — largely in the FIRST if/else block (the
 * 0x1F800360-378 zero-init/read/store region, hunks 1-12) and in remaining
 * accumulate-block scheduling artifacts (hunks 13-36, several now
 * operand-only where they were source-level before this session's fix).
 * See hypotheses.md s5 for the hunk-by-hunk read and the un-tried next
 * probe (apply the same "read the shared-join value early, inside the arm"
 * pattern to whichever OTHER post-join reads remain, if any — none
 * obviously remain after this fix; the frontier below names the actual
 * next candidate region).
 */
extern s32 D_80102314; /* record 1 of a 2-elem table, stride 0x44C from D_80101EC8 (func_8002C61C's s1+0x44C);
                         * fields accessed base+offset here, unlike record 0's individually-named scalars.
                         * TODO once scope-granted: promote to include/code6cac.h (out of this session's
                         * scope_allow surface, so declared file-local for now). */
void func_8002C22C(void) {
    s32 *t0 = (s32 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;
    s32 v1, v0, a2;
    s32 d_v1, d_a0, d_v0, d_a1;
    s32 sum_v0_2, sum_a1_2;

    if (D_800A3824 & 1) {
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        v1 = *(s32 *)0x1F80004C;
        v0 = *(s32 *)0x1F800048;
        a2 = *(s32 *)0x1F800050;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + *(s32 *)0x1F800058;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + *(s32 *)0x1F800054;
        *(s32 *)0x1F800360 = v0;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_80102100;
        d_a0 = D_80102104;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + *(s32 *)0x1F80005C;
        *(s32 *)0x1F800370 = D_801020FC;
        *(s32 *)0x1F800370 = D_801020FC + D_80102108;
        d_v0 = D_8010210C;
        d_a1 = D_80102110;
    } else {
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        v1 = *(s32 *)0x1F800004;
        v0 = *(s32 *)0x1F800000;
        a2 = *(s32 *)0x1F800008;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + *(s32 *)0x1F800010;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + *(s32 *)0x1F80000C;
        *(s32 *)0x1F800360 = v0;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_801020DC;
        d_a0 = D_801020E0;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + *(s32 *)0x1F800014;
        *(s32 *)0x1F800370 = D_801020D8;
        *(s32 *)0x1F800370 = D_801020D8 + D_801020E4;
        d_v0 = D_801020E8;
        d_a1 = D_801020EC;
    }
    *(s32 *)0x1F800368 = a2;
    *(s32 *)0x1F800374 = d_v1;
    *(s32 *)0x1F800378 = d_a0;
    *(s32 *)0x1F800374 = d_v1 + d_v0;
    *(s32 *)0x1F800378 = d_a0 + d_a1;

    if (D_800A3824 & 2) {
        t0[0xA8/4] += *(s32 *)0x1F800060;
        t0[0xAC/4] += *(s32 *)0x1F800064;
        t0[0xB0/4] += *(s32 *)0x1F800068;
        t0[0xB8/4] += d_tbl[0x234/4];
        t0[0xBC/4] += d_tbl[0x238/4];
        t0[0xC0/4] += d_tbl[0x23C/4];
        t0[0xA8/4] += *(s32 *)0x1F80006C;
        t0[0xAC/4] += *(s32 *)0x1F800070;
        t0[0xB0/4] += *(s32 *)0x1F800074;
        t0[0xB8/4] += d_tbl[0x240/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x244/4];
        sum_a1_2 = t0[0xC0/4] + d_tbl[0x248/4];
    } else {
        t0[0xA8/4] += *(s32 *)0x1F800024;
        t0[0xAC/4] += *(s32 *)0x1F800028;
        t0[0xB0/4] += *(s32 *)0x1F80002C;
        t0[0xB8/4] += d_tbl[0x210/4];
        t0[0xBC/4] += d_tbl[0x214/4];
        t0[0xC0/4] += d_tbl[0x218/4];
        t0[0xA8/4] += *(s32 *)0x1F800030;
        t0[0xAC/4] += *(s32 *)0x1F800034;
        t0[0xB0/4] += *(s32 *)0x1F800038;
        t0[0xB8/4] += d_tbl[0x21C/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x220/4];
        sum_a1_2 = t0[0xC0/4] + d_tbl[0x224/4];
    }
    t0[0xBC/4] = sum_v0_2;
    t0[0xC0/4] = sum_a1_2;
    t0[0x13C/4] = ((t0[0xA8/4] * 3) + t0[0xB8/4]) >> 4;
    t0[0x140/4] = ((t0[0xAC/4] * 3) + t0[0xBC/4]) >> 4;
    t0[0x144/4] = ((t0[0xB0/4] * 3) + t0[0xC0/4]) >> 4;
}
