/* REJECTED (grind session 9c, PERMUTER modality, 2026-08-13) — a KILLED axis,
 * not a policy rejection.  Nothing here is a cheat; it simply does not work.
 *
 * THE QUESTION.  Session 9c's JD-chassis campaign proved the JD body is one
 * empty loop note away from byte-exact (see
 * rejected/jd-basin-zeros-are-all-loop-note-wrappers.c), and that every zero
 * the permuter finds spells that note as a banned bare wrapper.  The obvious
 * follow-up: is the closing effect available from an ORDINARY, semantically
 * motivated construct that puts a real lexical BLOCK boundary
 * (NOTE_INSN_BLOCK_BEG, emitted by stmts.c/function.c for a scope) at the same
 * point?  Declaring a variable in the innermost scope where it is used is
 * exactly such a construct: a human programmer writes it for scoping reasons,
 * it needs no GCC-internals justification, and it is not a wrapper.
 *
 * THE MEASUREMENT (tmp/grind/func_800645B0/s9c/sweep32.py, honest
 * `sandbox --disable all` on each variant, JD chassis throughout):
 *
 *   XA  JD control, all locals at function scope ................  3 / 78
 *   XB  `s32 idx = i + j;` declared in the inner for-body .......  3 / 78
 *   XC  `idx` AND `val` both declared in the inner for-body .....  3 / 78
 *   XD  `{ s32 one = 1; mask = one << idx; }` inner block ....... 12 / 80
 *   XE  idx/val/mask/last ALL declared in the inner for-body ....  3 / 78
 *   XF  val/mask/last inner, idx at function scope .............. 48 / 41 (!)
 *   XG  no const-1 carrier at all (`mask = 1 << idx;`) ......... 12 / 80
 *   XH  bare `{ ... }` braces around the whole inner-loop body ..  3 / 78
 *
 * WHAT IT KILLS.  A lexical block boundary — whether it carries declarations
 * (XB/XC/XE) or not (XH) — is completely CODEGEN-INERT here: byte-identical to
 * the control, 3 / 78, not one instruction moved.  GCC 2.7.2 at -O2 without -g
 * gives a scope no RTL presence that the first-pass scheduler can see, so
 * "declare at point of use" cannot substitute for the loop note.  The closing
 * effect specifically requires NOTE_INSN_LOOP_BEG/END, which only a LOOP
 * STATEMENT emits — and there is no loop that belongs at that point of this
 * function semantically.  This axis is dead; do not re-measure it.
 *
 * TWO SIDE FACTS worth keeping.  XD/XG (12 / 80) re-confirm that the const-1
 * carrier `val = 1;` is load-bearing: both the block-scoped `one` and the
 * literal `1 << idx` cost two instructions and lose the mask shift's shape.
 * XF (48 / 41) is a degenerate build, listed only so the row is not re-run.
 *
 * Artifact: tmp/grind/func_800645B0/s9c/sweep32.py (variant text + harness).
 */
#if 0
    /* XB — the representative dead variant: ordinary C, zero effect. */
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            s32 idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                /* ... JD body verbatim ... */
            }
        }
    }
#endif
