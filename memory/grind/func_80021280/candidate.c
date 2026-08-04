/*
 * func_80021280 — BYTES PROVEN: sandbox --disable all = 0 (72/72), s2
 * (2026-08-04). Acceptance PENDING an owner ruling (s2 outcome =
 * ruling-request): the closing construct duplicates the loop tail
 * (control-transfer statements) into the if (a0 == 0) arm, which layer-1
 * cheat-reviewer ruled an EXTENSION of [[duplicated-statement-into-arms]]
 * (whose SOTN evidence base is assignment statements only).
 *
 * Structure vs the s1 form C:
 *  - Preamble in TARGET textual order: a1 = 0 FIRST, then t1, t4, t3, t2,
 *    mode, t0. (s1 measured this order alone = 19: the counter loses $a1.)
 *  - `a3` (s32) reused for `mode` — target keeps mode in $a3; lhu vs lh
 *    fall out of the assignment types. Floor-neutral, faithful spelling.
 *  - THE CLOSING LEVER: /* FAKE * / loop-tail duplication into the a0==0
 *    arm. Mechanism (ALLOCDBG-measured, tmp/grind/func_80021280/s2/):
 *    global.c allocno_compare pri = floor_log2(nrefs)*nrefs/livelen*10000;
 *    with a1 first the counter (12 refs / len 50 = 7200) loses $a1 to the
 *    pointer (11 refs / len 45 = 7333). The duplicate lifts counter refs
 *    to 15 (pri 9000) pre-RA; jump2 cross-jump re-merges it to IDENTICAL
 *    bytes (emitted branch is exactly target's beqz a0,.L80021388; single
 *    shared tail; lhu/nop/sh delay nop preserved).
 *  - Placement is load-bearing: the same duplicate in the store5
 *    fall-through arm leaves build 73 (sched1 hoists addiu into the lhu
 *    load-delay slot, breaking the cross-jump suffix) — the arm must
 *    contain no loads. See rejected/tail-dup-store5-arm-sched1-hoist.c.
 *
 * If the ruling refuses the construct: fall back to s1 form C (floor 2,
 * git history of this file) and the post-RA-scheduling forensics frontier.
 */
void func_80021280(s32 a0) {
    s32 a1 = 0;
    u8 *a2 = (u8 *)&D_80101EC8 + a0 * 1100;
    s32 a3 = *(u16 *)(a2 + 0x48);
    u16 *v1 = (u16 *)&D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = *(u16 *)(a2 + 0x48);
        *(s16 *)(a2 + 0x4A) = a1;
        *(s16 *)(a2 + 0x4C) = 0;

        if ((u32)(val >> 12) < 2) {
            u16 t1;
            s32 t4;
            s32 t3;
            s32 t2;
            u8 t0;

            a1 = 0;
            t1 = val;
            t4 = 4;
            t3 = 3;
            t2 = 1;
            a3 = D_800A38DC;
            t0 = D_800A384C;
        loop2_21280:
            {
                u16 nibble = (t1 >> (a1 << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                *(s16 *)(a2 + 0x88) = a1;
                if (a3 != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                *(u16 *)(a2 + 0x8A) = *(u16 *)(a2 + 0x26C);
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                *(s16 *)(a2 + 0x8E) = a1;
                if (a3 != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) {
                    /* FAKE: loop tail duplicated into this arm (cross-jump
                       re-merges to identical bytes; lifts the counter's
                       reg_n_refs so it beats the pointer for $a1) */
                    a1++;
                    if (a1 < 3) goto loop2_21280;
                    return;
                }
            store5_21280:
                *(u16 *)(a2 + 0x90) = *(u16 *)(a2 + 0x26C);
            }
        next_21280:
            a1++;
            if (a1 < 3) goto loop2_21280;
        }
    }
}
