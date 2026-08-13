/*
 * func_80034F88 — best form as of grind session s2 (structural).
 * Honest sandbox floor (`sandbox func_80034F88 --disable all`): **12**
 *   (s1 best was 23 with 50 build insns; the committed baseline was 24 with
 *    three `asm volatile("" ::: "memory")` scheduling barriers.)
 * Build: **49 insns vs target 49** — the instruction SEQUENCE now matches the
 * target 1:1 for the whole flag section; every residual point of distance is
 * register assignment, not shape.
 *
 * =====================================================================
 * WHAT THIS FORM IS, AND THE ONE OPEN QUESTION ABOUT IT
 * =====================================================================
 * The three levers, each measured this session:
 *
 * 1. `volatile u8 *pbit` (pointer-to-volatile local).  This is what makes the
 *    four `lbu` reloads of D_80106A73 survive.  GCC 2.7.2's `cse.c` refuses to
 *    hash a volatile MEM (canon_hash sets `do_not_record`), so `sets[i].src_elt`
 *    is 0 for the store and the dest-recording block at cse.c:7329 skips it —
 *    the stored value is never entered in the value table and the next read of
 *    the byte survives as a real load.  Without volatile, cse forwards every
 *    store into the following load and only ONE `lbu` remains in the whole
 *    flag section (measured repeatedly: scores 23–30, 43–50 insns).
 *
 * 2. Condition-before-read ordering (`c = p[8] & K;` BEFORE `val = *pbit;`).
 *    The scheduler may not move a non-volatile load across a volatile one, so
 *    source order fixes the `lw`/`lbu` order.  Writing the volatile read first
 *    strands a `nop` in each `lw v0,0x20(a1)` load-delay slot (3 wasted insns);
 *    writing the condition first lets the `lbu` fill it, exactly as target does.
 *
 * 3. THREE separate pointer locals (`pbit`, `pbit2`, `pbit3`), each assigned
 *    `&D_80106A73` immediately before the PREVIOUS block's store.  Target
 *    rematerialises the `lui`+`addiu` address base three times (once for
 *    {&=0xF8, bit-1}, once for bit-2, once for bit-4) rather than keeping one
 *    live base.  A single pointer local gives one base and 45 insns (score 15);
 *    three give the target's three bases and 49 insns (score 12).  Placing the
 *    NEXT block's assignment before the CURRENT block's store is what puts the
 *    `lui`/`addiu` pair ahead of the `sb`, as target has it.
 *
 * >>> OPEN CLASSIFICATION QUESTION — READ BEFORE SUBMITTING ANYTHING <<<
 * `volatile u8 *pbit = &D_80106A73;` adds a `volatile` qualifier to a global
 * that is declared plain `extern u8`.  The engine's `volatile_cheats` detector
 * does NOT strip it (proved: the score moves, so the qualifier reaches cc1;
 * by contrast s1 proved `extern volatile u8 D_80106A73;` IS stripped), and the
 * sibling function in this same file already spells it exactly this way
 * (`src/code6cac_b.c:4030, volatile u8 *pbit;`).  But the forbidden-family
 * catalog lists "volatile-coercion ... by cast", and a pointer-to-volatile
 * local is arguably the same intent in a different spelling.  This form is
 * banked as the best MEASURED floor.  It is NOT cleared for submission — the
 * classification must be ruled on first.  See hypotheses.md F1.
 *
 * Remaining gap at 12: register assignment only.  Target uses base v1 for the
 * first two blocks and a0 for the last two, with the loaded byte in a0/a0/v1/v1;
 * this build uses base a0 throughout with the byte in v1.
 */
void func_80034F88(void) {
    s32 *p;
    volatile u8 *pbit;
    volatile u8 *pbit2;
    volatile u8 *pbit3;
    u8 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    pbit = &D_80106A73;
    *pbit &= 0xF8;

    c = p[8] & 1;
    val = *pbit;
    val2 = c ? (u8)(val | 1) : val;
    pbit2 = &D_80106A73;
    *pbit = val2;

    c = p[8] & 2;
    val = *pbit2;
    val2 = c ? (u8)(val | 2) : val;
    pbit3 = &D_80106A73;
    *pbit2 = val2;

    c = p[8] & 4;
    val = *pbit3;
    val2 = c ? (u8)(val | 4) : val;
    *pbit3 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
