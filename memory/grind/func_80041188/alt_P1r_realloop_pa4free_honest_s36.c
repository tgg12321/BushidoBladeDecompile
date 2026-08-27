/* alt_P1r_realloop_pa4free_honest_s36.c -- s36 (forensics, 2026-08-27).
 *
 * SANDBOX --disable all = 2 at 132 build / 132 target insns. ZERO constructs: no
 * FAKE, no wrap, no dead store, no alias, no reuse. This is the BEST HONEST
 * (construct-free) form ever recorded for func_80041188 -- previous best honest was
 * W4 = 3 (s19) and Q1 = 7 (s21, and Q1 carried a banned block-0 wrap).
 *
 * WHAT IT IS: s35's W4c (the REAL-LOOP chassis carrying target's own honest block-2
 * `out3 = (s32 *)(((u8 *)a4) + 0x20);` rather than `out3 = out2;`) with the `pa4`
 * param-alias local DELETED, i.e. the fifth parameter `a4` used directly at every
 * site. W4c measures 11; deleting pa4 takes it to 2.
 *
 * WHY IT WORKS (measured, tmp/grind/func_80041188/s36/P1r/fr_86.err). Deleting the
 * pa4 alias merges pa4's references onto the stack parameter, which local-alloc.c:1064
 * DOUBLES the live length of (REG_EQUIV, the s30 mechanism). a4 lands at 9 refs /
 * live 188 / pri 1436 -- above a3 (5/99/1010) because 9 references cross floor_log2's
 * 8-step -- while out2 keeps 5 refs / live 41 / pri 2439 and therefore outranks BOTH.
 * The allocation order is then exactly target's:
 *     out2 2439 -> $s6   a4 1436 -> $s7   a3 1010 -> $fp
 *     out3 638 -> $s3    tbl 2978 -> $s5  i 3402 -> $s4   stptr2 2500 -> $s0
 * ALL-TARGET SEATS, with target's block-2 `addiu $s3,$s7,0x20`, at 132 insns, with no
 * construct of any kind. s30 killed the pa4-free family on the GOTO chassis (there a3
 * 808 outranks a4 736 unconditionally); the REAL-LOOP chassis inverts that because
 * loop-depth weighting doubles a4's two loop1 references, taking it from 7 to 9.
 *
 * ITS ENTIRE RESIDUAL (2 diffs, and s36 proved it is STRUCTURAL -- see evidence.md
 * E-s36-3/4/5): loop1's `*((s16 *)(ents + i * 0x68 + 6)) = 2;` materialises the
 * constant in a HImode temp; loop.c move_movables HOISTS that temp to block 0
 * (red.i.loop: "Insn 152: regno 121 (life 1), move-insn savings 1  moved to 312");
 * hoisted, it is live 92 insns across 7 calls, every callee-saved register is already
 * taken by a target-hosted pseudo, so reload spills it and rematerialises the constant
 * at the use -- correct POSITION, but into $t0, because reload's forbidden_regs
 * excludes $v0/$v1/$a0-$a3 (all explicitly used by the calls) and MIPS has no
 * REG_ALLOC_ORDER, so the first legal spill register is $t0. Target has
 * `addiu $v0,$zero,0x2`, which is what local-alloc gives an UNHOISTED, block-local,
 * call-free temp -- exactly what loop2 (a goto loop, no loop notes, no LICM) already
 * produces here and matches.
 *
 * The hoist is UNCONDITIONAL: loop.c:1631 moves when
 * `threshold * savings * lifetime >= insn_count`; threshold = (loop_has_call ? 1 : 2)
 * * (1 + n_non_fixed_regs) = 1 * (1 + 60) = 61 (68 regs, 8 fixed in mips.h:1188),
 * savings >= 1, lifetime >= 1, and loop1 is 48 real insns. 61 >= 48 for every
 * spelling. Five source spellings measured identical at 2 (see rejected/).
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, a4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, out2);
        func_800523E0(a4, out2, a3, ents + i * 0x68 + 0x38);
        *((s16 *) (ents + i * 0x68 + 6)) = 2;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
    out3 = (s32 *) (((u8 *) a4) + 0x20);
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(a4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
