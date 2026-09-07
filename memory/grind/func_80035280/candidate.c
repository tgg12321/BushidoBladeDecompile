/*
 * CANDIDATE -- func_80035280 (src/code6cac_b.c) -- s3 (2026-09-07, structural)
 *
 * MEASURED THIS SESSION on the HEAD chassis (INCLUDE_ASM baseline, target 108):
 *     s2 candidate (single reused `v` accumulator)   sandbox = 39  (build 109)
 *     v declared first / v = p[8] hoisted            sandbox = 39  (build 109)
 *     `f` dropped, src = &D_80106A73 - 3 directly    sandbox = 44  (build 111)
 *     THREE-WAY SPLIT accumulator (flags/0/1/2)      sandbox = 18  (build 109)
 *     two alternating accumulators (fA/fB)           sandbox = 28  (build 109)
 *     THIS body = three-way split + `i = 0;` hoisted sandbox = 16  (build 109)
 *     same, but `s32 i = 0;` initialiser             sandbox = 29  (build 110)
 *     same, but `i = 0;` just before loop 1          sandbox = 18  (build 109)
 * 16 is the session floor and supersedes the s2 floor of 39.
 *
 * WHY THIS SPELLING WINS -- the mechanism, read out of the compiler source and
 * confirmed in the dumps, not guessed.
 * local-alloc.c:472 makes a pseudo eligible for LOCAL allocation only when
 *     reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1
 * With ONE reused accumulator `v` the pseudo is set four times, so
 * reg_n_deaths == 4 (`Register 77 ... dies in 4 places` in the s2 .lreg dump),
 * reg_qty is left at -1, and the accumulator falls through to global-alloc.
 * By then local-alloc has already handed $v0/$v1/$a0 to the short per-arm
 * temps (QTYDBG blk=0 shows exactly seven block-0 quantities and the
 * accumulator is not among them), so the accumulator gets whatever is left --
 * $a1 -- and the whole flag block comes out as the 4-cycle permutation
 * (addr $a0, acc $a1, mask $v1, byte $v0) against the target's
 * (addr $a1, acc $v0, mask $a0, byte $v1).
 * Splitting the accumulator into one local per merged bit gives every one of
 * them reg_n_deaths == 1, so they are local quantities. Their local-alloc
 * priority is log2(refs)*refs*size / (death - birth) (local-alloc.c
 * qty_compare), which puts the accumulator chain ahead of the per-arm temps,
 * and find_free_reg hands it $v0 first. The emitted flag block then matches
 * asm/funcs/func_80035280.s:25AA4-25AEC instruction for instruction, register
 * for register.
 * Hoisting `i = 0;` to immediately after the `func_80077D00()` call (and
 * spelling loop 1 `for (; i < 3; i++)`) puts the counter zeroing where the
 * target has it -- the first insn after the jal's delay slot -- and takes the
 * last two points off the prologue. Position matters: the same statement placed
 * just before loop 1, or as a `s32 i = 0;` declaration initialiser, measures 18
 * and 29 respectively.
 *
 * WHAT IS LEFT, EXACTLY: build insns 1..38 are now IDENTICAL to the target
 * (prologue, the whole flag block, the whole of loop 1). The entire remaining
 * score of 16 is the single loop.c:1631 LICM decision s1 identified and s2
 * measured: the build hoists the 0x91A2B3C5 (/1800) magic into the loop-2
 * preheader (+2 insns) and then needs a `nop` in the `lw 0x4($a2)` load-delay
 * slot, where the target materialises the constant inside the loop and lets the
 * `ori` fill that slot; that also displaces the mfhi temp from $t2 to $t3.
 * Net build 109 vs target 108. Loop 2 is otherwise register-identical.
 * The gate needs loop-2 RTL insn_count >= 123 (measured in s2); this body's
 * loop 2 is 55 real insns (`Loop from 110 to 274: 55 real insns.` in the .loop
 * dump, both constants "moved to"). Do NOT re-derive this -- see evidence.md
 * FACT 8/9 and the -msoft-float configuration finding in FACT 11.
 *
 * ADMISSIBILITY NOTE for whoever submits this body. Two constructs need a call:
 * (1) the split accumulators flags/flags0/flags1/flags2 -- each is written once
 *     and holds a real value that IS stored to p[8] and consumed by the next
 *     arm, so nothing is dead; but the only reason not to reuse one `v` is
 *     codegen, and flags0/flags1 are read TWICE (the store and the next arm),
 *     so this is not the once-written/once-read named-intermediate shape.
 * (2) the `i = 0;` hoist -- a real initialisation of a real loop counter, moved
 *     eight statements earlier for a scheduling slot (worth 2 points).
 * If either is ruled inadmissible, memory/grind/func_80035280/rejected/
 * s3-single-accumulator-global-allocno-score39.c is the un-split fallback at 39
 * and tmp/grind/func_80035280/s3/v/v4.c is the split-only body at 18.
 */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i;
    s32 flags;
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    i = 0;
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
    for (; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = *src;
        ((u8 *)p)[0x1D + i] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[i * 4 + 0x21] = *(s32 *)(base + i * 8 + 4) / 1800;
        ((u8 *)p)[i * 4 + 0x22] = (*(s32 *)(base + i * 8 + 4) / 30) % 60;
        ((u8 *)p)[i * 4 + 0x23] = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;
        ((u8 *)p)[i * 4 + 0x24] = base[i * 8];
    }
}
