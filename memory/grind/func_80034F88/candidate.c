/*
 * func_80034F88 — best HONEST form as of grind session s3 (structural).
 * Honest sandbox floor (`sandbox func_80034F88 --disable all`): **18**
 *   (s1 best 23 / 50 insns; s2's 12 is DISCARDED — see "the volatile ruling"
 *    below; the committed baseline is 24 and carries three forbidden
 *    `asm volatile("" ::: "memory")` scheduling barriers.)
 * Build: 51 insns against a 49-insn target.  The 49-insn spelling of this same
 * form (`u8 val` instead of `s32 val`) scores 20 — fewer instructions, worse
 * distance; the score is what counts.
 *
 * =====================================================================
 * WHY THIS FORM (three measured levers, all volatile-free)
 * =====================================================================
 * 1. `u8 *ptr = &D_80106A73; *ptr &= 0xF8;` — a C-level pointer alias to a
 *    global (FROZEN SOTN-accepted family).  The read-modify-write gives the
 *    address expression TWO memory uses, so `combine` cannot fold `%lo` into
 *    either one and the target's unfolded `lui`+`addiu` base survives; it also
 *    mismatches the following symbol read of the same byte, so the first `lbu`
 *    reload materialises.  Spelling the mask as plain `D_80106A73 &= 0xF8;`
 *    costs 9 points (29 at the same insn count) because the base is then folded
 *    away and the reload disappears with it.
 * 2. Condition BEFORE the flag-byte read (`c = p[8] & K;` then
 *    `val = D_80106A73;`).  Worth 1 point and 1 insn over the s1 ordering
 *    (23/50 -> 22/49): the `lw v0,0x20(a1)` issues first, so its load-delay slot
 *    can be filled instead of stranding work behind it.  This lever was found in
 *    s2 on the volatile form and had never been applied to the non-volatile one.
 * 3. Copy loop spelled `*((u8 *)p + i + 0x17)` — the induction variable BEFORE
 *    the constant displacement.  Worth 2 points (22 -> 20): it emits
 *    `addu v0,a1,v1` (p + i) in target's operand order, where
 *    `*((u8 *)p + 0x17 + i)` emits `addu v0,v1,a1`.  The pair appears twice (the
 *    loop body and the branch delay slot), hence 2 points.  s1/s2 recorded the
 *    copy loop as "already matching in every form measured" — that was wrong.
 * 4. `s32 val` for the loaded byte (with `u8 val2` for the selected value).
 *    Worth 2 more points (22 -> 20 -> 18) at the cost of 2 instructions: the
 *    read becomes a `(zero_extend:SI (mem:QI))` and an explicit
 *    `andi a0,v1,0xff` truncation appears before each store, which lengthens the
 *    byte's live range across the block boundary and changes which value each
 *    block's `ori`/`move` pair reads.  s1 killed "int-typed temporaries" at 28,
 *    but that was measured before any of levers 1-3 existed; the type of the
 *    loaded byte only pays once the address and ordering shape are right.
 *    Making `val2` `s32` as well gives the gain straight back (20).
 *
 * =====================================================================
 * THE REMAINING 18 POINTS, AND THE VOLATILE RULING THAT GATES THEM
 * =====================================================================
 * All 18 are in the three flag blocks.  The target reads the flag byte back
 * from memory FOUR times, each `lbu`/`sb` pair sharing one unfolded base
 * register (`lbu a0,0(v1)` immediately after `sb a0,0(v1)` — it even reloads
 * into the register that already holds the stored value).  s3 established from
 * the compiler source that this is not reachable without a volatile access:
 *   - `cse.c:7329` skips recording a SET_DEST when `sets[i].src_elt == 0`, and
 *     `canon_hash` (cse.c:1941-1947) sets `do_not_record` for a MEM only when
 *     `MEM_VOLATILE_P` is set.  The other `do_not_record` triggers (PRE/POST
 *     inc-dec, PC, CC0, CALL, UNSPEC_VOLATILE, volatile ASM_OPERANDS, and
 *     hard-reg under SMALL_REGISTER_CLASSES) are unreachable from C here.
 *   - Two C address expressions for the same byte either hash the same (cse
 *     merges the pseudos -> one base -> the store forwards into the load and the
 *     reload dies) or hash differently (no forward, but two `lui`s and `%lo`
 *     folded into each single-use mem).  Aliasing expressions such as
 *     `(&D_80106A70)[3]` would additionally be treated as a distinct object by
 *     GCC's alias analysis and MISCOMPILE the read-back, so they cannot be what
 *     the original source did either.
 * s2's floor of 12 came from `volatile u8 *pbit = &D_80106A73;` on a global
 * declared plain `extern u8`.  s3 rules that construct OUT: it is
 * `engine/volatile_cheats.py` pattern 2 (`*(volatile T *)&D_globalsym` on a
 * game-RAM global) respelled in two steps, and the detector's silence is a
 * regex gap, not a sanction.  The sanctioned spelling —
 * `extern volatile u8 D_80106A73;` plus the `volatile_extern_allowlist.txt`
 * grant — was researched and FAILS BOTH PRONGS of
 * `.claude/rules/legitimate-volatile-interrupt-touched.md`: no IRQ/VSync writer
 * exists (all six functions that touch the byte are ordinary synchronous game
 * logic; the only one that stores it, func_800167EC, is called from
 * src/ings.c:414, not installed as a callback), and the use-site shape
 * (store-then-readback) is not on the rule's exact three-shape list.
 * See `rejected/volatile-ptr-coercion-score12.c` and hypotheses.md s3-H1..H4.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
