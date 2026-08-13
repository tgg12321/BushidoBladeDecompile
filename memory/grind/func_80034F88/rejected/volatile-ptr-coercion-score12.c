/*
 * REJECTED (s3) — s2's score-12 form.  Scores 12 at 49 insns (the best number
 * ever measured on this function) and is NOT usable: the construct that buys
 * every one of those points is a forbidden volatile coercion.
 *
 * THE CONSTRUCT: `volatile u8 *pbit = &D_80106A73;`, where the global is
 * declared `extern u8 D_80106A73;` (src/code6cac_b.c:127).  The local pointer
 * adds a `volatile` qualifier the object does not have, purely so that
 * `canon_hash` sets `do_not_record` on the MEM and `cse.c:7329` skips recording
 * the store — which is what makes the target's four `lbu` reloads survive.
 *
 * WHY IT IS A CHEAT (s3 ruling, self-resolved with citations rather than
 * escalated, because the project's own policy artifacts already answer it):
 *  - `engine/volatile_cheats.py` pattern 2, verbatim from its docstring:
 *    "Inline `*(volatile T *)&D_globalsym` casts on game-RAM symbols.  Treats a
 *    plain game-state global as volatile at the access site, defeating CSE/
 *    scheduling."  `volatile u8 *pbit = &D_80106A73; ... *pbit` is that cast
 *    spelled in two statements instead of one; the pointee, the object, the
 *    qualifier added and the CSE effect are identical.
 *  - The detector does not STRIP it (measured: the score moves), but its
 *    docstring's carve-out for pointer-to-volatile is explicitly about
 *    "`extern volatile T *name;` — the VARIABLE is a pointer ... typically a
 *    hardware register address loaded at startup", not about a local pointer
 *    initialised with the address of a plain game-RAM global.  A regex gap is
 *    not a sanction (cheat checklist T4/T5: "it's different because it's spelled
 *    with X instead of Y" is exactly the loophole the policy forbids).
 *  - The sibling usage at src/code6cac_b.c:4030 is not a precedent either: it is
 *    unreviewed pre-existing source, not a ruling.
 *
 * WHY THE SANCTIONED SPELLING IS ALSO UNAVAILABLE.  `extern volatile u8
 * D_80106A73;` + an entry in `volatile_extern_allowlist.txt` is the one
 * sanctioned volatile family for a `D_`-named global.  Both prongs of
 * `.claude/rules/legitimate-volatile-interrupt-touched.md` were researched in s3
 * and both FAIL:
 *   prong 1 (IRQ/MMIO writer, must be cited): the byte is at 0x80106A73, KSEG0
 *     game RAM, not the 0x1F8xxxxx MMIO range; the six functions that reference
 *     it are func_800167AC/BC/D4/EC, func_80034708, func_80034F88, func_80035280
 *     — none is installed via InterruptCallback / VSyncCallback /
 *     irq_EnableInterrupts / irq_AcknowledgeVblank anywhere in src/, and the one
 *     external storer (func_800167EC) is called synchronously from
 *     src/ings.c:414.
 *   prong 2 (use-site shape, "exact list, not illustrative"): the shape here is
 *     store-then-readback inside straight-line code — not spin-wait, not
 *     double-read-across-sequence-point, not IRQ-mutated-loop-bound.
 *
 * Consequence banked for the next session: the target's flag-section bytes are
 * only explainable by volatile QImode semantics on D_80106A73, and no sanctioned
 * volatile spelling is available under current policy.  Do NOT re-measure this
 * form; the number is known and the number is not the problem.
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
