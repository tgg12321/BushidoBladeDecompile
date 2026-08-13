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
 *
 * =====================================================================
 * s4 (permuter modality) — form UNCHANGED, and now defended on two more axes
 * =====================================================================
 * s4 did not move the floor and did not change a character of this body.  What
 * it did was close the two axes that could have replaced it:
 *  - decomp-permuter ran on this function for the first time: 50,425 iterations
 *    across two structurally different chassis (this 51-insn `s32 val` form,
 *    34,003 iters / 30 finds; and the 49-insn `u8 val` form, 16,422 iters /
 *    67 finds).  Every find was re-scored with the honest sandbox; the best
 *    honest, semantically-correct find on either seed TIES this form at 18.
 *    The 49-insn chassis converged onto this one (its 18-point finds are
 *    51-insn `s32`-widened forms), so the two are a single basin.
 *  - The four spellings s3 listed as "untried" on frontier F1 are measured:
 *    per-block temporaries 30, copy loop through `ptr` 31, third block storing
 *    via `*ptr` 22, `(u8)`-cast staging 18 (neutral), block-1-read-first 18
 *    (neutral).  Banked in rejected/.
 * The two pointer results sharpen WHY lever 1 above is spelled the way it is:
 * giving `ptr` any use beyond the single read-modify-write keeps its address
 * pseudo live further down the function, cse then merges it, and the target's
 * per-block `lui`+`addiu` rematerialisations vanish.  ONE read-modify-write use
 * is load-bearing in both directions — two memory operands so `combine` cannot
 * fold `%lo`, and no third operand so the pseudo dies before block 2.  Do not
 * "clean up" that pointer, and do not give it another use.
 * NB (s4): the permuter produced exactly one sub-floor number, a 17, and it is
 * a MISCOMPILE (the third block's store hoisted inside the `if` arm) — see
 * rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c.  The sandbox
 * scores bytes, not behaviour.
 *
 * =====================================================================
 * s5 (permuter modality, DIRECTED) — form UNCHANGED; three of the levers
 * above are now known to be free, and two more are proven load-bearing
 * =====================================================================
 * s5 ran the permuter's OTHER mode (manual mutation via PERM_GENERAL, random
 * mutation disabled) as an exhaustive enumerator, and scored the same two
 * cross-products deterministically with the honest sandbox: 1,008 forms in
 * total (864 + 144), plus 1,152 permuter iterations across the same spaces.
 * Best = 18 in both waves. The only campaign output (permuter score 1290 vs a
 * base of 1300 — the permuter ranked it BETTER) sandbox-scores 18, a tie; see
 * rejected/directed-perm-mixed-condform-score18-TIE.c.
 * What changed in how this body must be READ:
 *  - Lever 2 above (condition BEFORE the flag-byte read) is NO LONGER A LEVER.
 *    All eight per-block orderings score 18. It was worth 1 point at floor 23
 *    and was absorbed by lever 4's `s32 val` widening. Spell it however reads
 *    best; do not defend it.
 *  - `p = func_80077D00();` BEFORE `ptr = &D_80106A73;` is load-bearing and was
 *    never previously probed: assigning the pointer first costs a MINIMUM of
 *    +11 (29 over all 432 such forms). Do not tidy that order.
 *  - Lever 1's `*ptr &= 0xF8;` may equally be spelled `*ptr = *ptr & 0xF8;`
 *    (both 18). What defeats combine's `%lo` fold is the address expression
 *    having TWO memory operands, not the compound assignment.
 *  - `s32 val` may equally be `u32 val` (both 18); `u8 val` is 20. The lever is
 *    WORD WIDTH (the zero_extend), not signedness. `u8 val2` and a single
 *    word-typed `c` are both required (s32 val2 / u32 val2 / u8 c / split
 *    c1,c2,c3 all cost 1-2).
 *  - The copy loop is exactly orthogonal: `+ 0x17 + i` costs exactly +2 in all
 *    432 pairings, delta set {2}.
 *
 * =====================================================================
 * s6 (forensics) — form UNCHANGED; two of the mechanisms quoted ABOVE are
 * WRONG and are corrected here (read this before trusting levers 1 and 4)
 * =====================================================================
 * The first cc1 `-da` dump taken of THIS body (s2's dumps were of a pre-lever
 * volatile variant) corrects the model this header has carried since s1:
 *  - "combine folds `%lo` into a single-use mem" is FALSE. The pre-cse dump
 *    already contains `(mem:QI (symbol_ref "D_80106A73"))` for every plain
 *    symbol access and `(mem:QI (reg 73))` for every `*ptr` access, straight
 *    out of RTL expansion — MIPS accepts a bare symbol_ref as an address and
 *    the `lui $at` / `%lo(...)($at)` pair is manufactured at assembly-output
 *    time. No optimizer pass is involved, so lever 1 works for a DIFFERENT
 *    reason than stated: `*ptr &= 0xF8;` is spelled through a pointer, so its
 *    two accesses are `(mem (reg))`, and the following SYMBOL-spelled read
 *    hashes differently and therefore survives as a real `lbu`.
 *  - the block-2/3 reloads die by an ORDINARY cse hash hit (identical
 *    `(mem:QI (symbol_ref))` rtx on store and load), not by the
 *    `cse.c:7329 src_elt == 0` path, which is volatile-only. The two
 *    `zero_extend`-from-register insns that replace them are exactly the two
 *    `andi a0,v1,0xff` truncations, i.e. the forward is ALSO the whole 51-vs-49
 *    instruction excess.
 *  - the `$v0`/`$v1` swap of `val2` is NOT a steerable allocation tie. `.greg`
 *    shows `75 conflicts: … 2 …` (hard reg $v0) while the condition pseudo
 *    carries `preferences: 2`; the hard reg was taken by `local-alloc` for the
 *    block-local `p[8]` loads before `global_alloc` ran. Six forms that alter
 *    the live ranges (if/else both arms 24, ternary 24, positive-sense if 23,
 *    store duplicated into arms 35, val2-split 24, val-split 23, both-split 29)
 *    all keep the conflict and all score worse. Banked in rejected/.
 * Net: all three residual defect classes are downstream of ONE decision —
 * pointer-vs-symbol spelling per access, fixed at expansion — so do not look
 * for a separate register or scheduling lever.
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
