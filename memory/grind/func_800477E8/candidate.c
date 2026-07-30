/* func_800477E8 — best known form as of grind session s3 (2026-07-30, structural).
 *
 *   honest floor (sandbox --disable all): 5      (s1/s2 floor was 17)
 *   build_insns == target_insns == 170
 *
 * The residual 5 is EXACTLY tie A and nothing else.  Verified instruction by
 * instruction against build/src/sound.o with tmp/grind/func_800477E8/s1/diff.py
 * (pass build/src/sound.o as argv[1]; the default reference build/asm/6CAC.o no
 * longer contains this function).  Branch-offset noise removed, the whole diff is
 *   26  move t2,v0  | move t1,v0
 *   39  sh t1,0(s0) | sh t2,0(s0)
 *   55  sh t1,0(s0) | sh t2,0(s0)
 *   57  sh t2,0(s0) | sh t1,0(s0)
 * plus the `li` of 0x2C00 landing in $t1 instead of $t2.
 *
 * The ENTIRE loop1 and loop2 register seating now equals target, verified on the
 * cc1 -da `.greg` dispositions, not inferred:
 *   loop1  v0->$v0(2) v1->$v1(3) a0->$a0(4) a1->$a1(5) a2->$a2(6) a3->$a3(7)
 *          t0->$t0(8) s0->$s0 s1val/s2val/s3val->$s1/$s2/$s3
 *   loop2  p(inner walking ptr, compiler-made)->$v0(2)  val->$v1(3)
 *          a0->$a0(4)  row base (compiler-made)->$a1(5)  w->$a2(6)  a3->$a3(7)
 *   loop3  ptr->$v1(3)  a0->$a0(4)
 * The ONLY wrong dispositions left are tie A: t1val->$t2(10) and t2->$t1(9),
 * where target wants t1val->$t1(9) and t2->$t2(10).
 *
 * WHY THIS FORM (the s3 findings, each measured on the .greg dump):
 *
 * 1. GCC 2.7.2 weights every register reference by the loop_depth flow.c sees,
 *    and loop_depth comes from NOTE_INSN_LOOP_BEG notes, which the front end
 *    emits only for real loop constructs — a `goto`-driven loop gets none.  So
 *    m2c's `goto inner2:` shape silently starved every loop2 pseudo of reference
 *    weight and pinned the allocation order.  Writing loop2 as a real
 *    `do { ... for (...) ... } while (...)` nest is what unlocked it.
 *
 * 2. loop2's inner walking pointer and its row base are NOT source variables in
 *    the original: indexing the row (`p[a0] = val`) makes loop.c strength-reduce
 *    the row address into the $a1 induction variable and the inner index into the
 *    $v0 walking pointer, and only then do the emitted preheader order
 *    (`a3=0; w=0;` BEFORE the row pointer's lui/addiu) and the delay-slot
 *    placement of `addiu $a1,$a1,0x44` match target.  A source-level row pointer
 *    always lands in the accumulator's slot and vice-versa, because loop.c
 *    inserts iv initialisations AFTER the source statements in the preheader.
 *
 * 3. loop2's row counter MUST be loop1's `a3`, the same C variable.  Its loop1
 *    references drag the allocno's priority down to 0.84 (18 refs / 86 insns),
 *    which is what makes it the LAST of the loop2 group to be allocated and so
 *    take $a3.  A fresh local scores 1.17 and is allocated third, which rotates
 *    the whole loop2 seating (measured: 14 instead of 6).
 *
 * 4. `a3` must also be USED inside the loop2 body, which the `a3 * 0x11` row
 *    index supplies.  With the counter unused in the body, cc1's check_dbra_loop
 *    REVERSES the loop into a countdown and drops target's `slti $v0,$a3,0x9`
 *    (build_insns 169, score 11 — see rejected/loop2-counter-only-loop-reverses.c).
 *
 * 5. The stored value must be an explicit local copied from the accumulator
 *    (`val = w;`).  Letting LICM manufacture it from `a3 * 0x7D0` also works
 *    mechanically but leaves the accumulator compiler-made, which puts its init
 *    after the row pointer's and swaps $a1/$a2 (score 14).
 *
 * 6. The three pre-loop initialisations are in TARGET's own order,
 *    `a3 = 0; t2 = 0x2C00; a0 = 0;` (target's preheader is `addu $a3,$zero,$zero;
 *    addiu $t2,$zero,0x2C00; addu $a0,$zero,$zero`), which is also HEAD's order.
 *    s2 rejected that order and used `t2; a3; a0;` — correctly for s2's body,
 *    where loop2's row counter was a fresh local — but with a3 shared into loop2
 *    the profile changed and target's order is now worth exactly one instruction
 *    (score 6 -> 5).  This retires s2's "fragile arbitrary statement order"
 *    caveat: the ordering the candidate uses is the one target itself emits.
 *
 * TIE A — mechanism PROVEN, cost not yet paid.  t1val has 3 weighted refs over a
 * 76-insn range (0.0395) and t2 has 5 over 150 (0.0667), so t2 is allocated first
 * and takes the lower $t1.  t2's range is double t1val's because t2 is used in
 * BOTH arms and so stays live on both paths.  Giving t2 a SINGLE in-loop
 * reference flips it exactly: the `ta-factor-store` / `ta-factor-cached` variants
 * (shared `*s0 = t2;` lifted out of the two arms) produce t1val->$t1(9) and
 * t2->$t2(10) with every other disposition still correct — but factoring needs a
 * second test of `a3 >= 5` and costs one instruction (build_insns 171).  The open
 * question for the next session is a spelling of "t2 read once per iteration"
 * that keeps 170 insns; equalising from the other side needs a 4th reference to
 * t1val, and no honest one exists in this body.
 *
 * KNOWN-DEAD, do not re-derive (s1 + s2 + s3, all measured):
 *   - splitting `a0` off any of the three loops (17 -> 36/40 in s2; 32 in s3).
 *   - a fresh local for loop2's row counter (score 14, not 6).
 *   - a fresh local for loop2's inner counter (compiler-made p then loses $v0).
 *   - declaration order anywhere (inert: global.c reaches the allocno-number
 *     tiebreak only on EXACTLY equal priority).
 *   - hoisting `t2 = 0x2C00` above gpu_CalcClut: +2 insns (callee-save pair).
 *   - inlining the 0x2C00 literal in both arms: CSE rebuilds one pseudo, seating
 *     unchanged (score 6, same as this form).
 *   - converting loop1's inner `goto inner` into a real do-while/for loop: cc1
 *     creates TWO extra pseudos that take $t1/$t2 and push t1val/t2 out to
 *     $t3/$t4 (score 10 / 13).  Loop shape in loop1 is NOT neutral — same
 *     failure mode s2 saw with a full nested-for rewrite.
 *   - inverting the arms' branch sense: score 18.
 *   - routing both arms' t2 store through the v0 scratch: score 39.
 *   - `s0[1] = t2;` hoisted ahead of the branch with `s0 += 2` in the arms:
 *     build_insns 167, score 10.
 *
 * OUTSTANDING JUDGE SURFACE: the `do { v0 = v1 | a1; } while (0);` below is
 * inherited from HEAD and is NOT `/* FAKE *\/`-annotated. Any session that
 * reaches distance 0 must annotate it per .claude/rules/do-while-zero-exception
 * or eliminate it before proposing the candidate.
 */

s32 func_800477E8(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    s32 s1val;
    s32 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;
    s32 w;
    s32 val;

    s0 = D_800A33D0;
    s3val = gpu_CalcTPage(0, 0, 0x2C0, 0x1C0);
    s2val = gpu_CalcTPage(0, 0, 0x2C0, 0x180);
    s1val = gpu_CalcClut(0x10, 0x1E0);
    t1val = gpu_CalcClut(0x10, 0x1E0);
    a3 = 0;
    t2 = 0x2C00;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        a1 = 0;
        v1 = 1;
inner:
        if (a3 >= 5) {
            *s0 = s3val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = s1val;
            s0 += 1;
            v0 = -0xC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x3FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4000;
        } else {
            *s0 = s2val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = t1val;
            s0 += 1;
            v0 = -0x40C1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x7FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x8000;
        }
        *s0 = v0;
        s0 += 1;
        if (a3 & 1) {
            v0 = a2 | t0;
            *s0 = v0;
            s0 += 1;
            do { v0 = v1 | a1; } while (0);
        } else {
            v0 = v1 | a1;
            *s0 = v0;
            s0 += 1;
            v0 = a2 | t0;
        }
        *s0 = v0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        v0 = -0x1000;
        *s0 = v0;
        s0 += 1;
        t0 += 0x100;
        a2 += 1;
        a1 += 0x100;
        a0 += 1;
        v1 += 1;
        if (a0 < 0x10) goto inner;
        a3 += 1;
        a0 = 0;
    } while (a3 < 8);

    {
        s32 *a0p;
        a0p = (s32 *)&D_800EF070;
        *(s8 *)a0p = 0xE;
        D_800EF07A = 4;
        D_800EF0BC = -0x2EE0;
        D_800EF071 = 0;
        D_800EF0C0 = 0;
        D_800EF0C4 = -0xFA0;
        D_800EF080 = 0;
        D_800EF082 = 0;
        D_800EF084 = 0;
        D_800EF078 = 0;
        D_800EF07C = 0;
        D_800EF076 = 0;
        func_800417D0(a0p);
    }

    a3 = 0;
    w = 0;
    do {
        val = w;
        p = &D_800EF59C[a3 * 0x11];
        for (a0 = 0x10; a0 >= 0; a0--) {
            p[a0] = val;
        }
        w += 0x7D0;
        a3 += 1;
    } while (a3 < 9);

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0) goto loop3;

    return (s32)s0 - (s32)D_800A33D0;
}
