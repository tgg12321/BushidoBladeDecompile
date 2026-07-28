/* s4 H4d/H4e — replace then-arm mask compute with sll/srl shift form:
 * `(a0 & 0xFFFF0000) | ((a0 << 19) >> 19)` (case-1 only H4d, both cases H4e).
 * Intent: emit `andi/sll/srl/or` instead of `lui/ori/and` so then-arm no
 * longer needs a lui-hoistable first-insn candidate; reorg.c would then
 * fall back to else-arm's `lui $v1` for the dead-branch delay-slot fill.
 * Result: score 6 -> 7 (case-1 only) / 6 -> 8 (both cases). REGRESSED.
 * combine.c folds the shift form back to lui/ori/and OR emits enough
 * extra insns that the fill-priority calculus doesn't flip — the
 * pattern shortfall persists AND per-arm mask insn count grew.
 */
