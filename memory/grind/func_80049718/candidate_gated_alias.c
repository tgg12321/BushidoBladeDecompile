/* func_80049718 — GATED close to floor 0 (sandbox distance 0, byte-match modulo
 * relocation). NOT committable as-is: layer-1 cheat-reviewer FAILed it (s1).
 *
 * The ONLY delta vs the clean floor-4 candidate.c is the prologue address comp:
 *     s16 *tbl = D_800EF980;
 *     p_anim = tbl + arg0;         // replaces  p_anim = &D_800EF980[arg0];
 *
 * WHY IT CLOSES CLUSTER A:
 *   RTL (text1b.i.greg): with `&D_800EF980[arg0]`, `fold` canonicalizes the
 *   address PLUS so the symbol_ref (address constant) is operand-2, so the INDEX
 *   (ashift) insn is emitted before the BASE (symbol_ref) insn; local-alloc then
 *   gives index->$v0, base->$v1 and emits `addu s0,v0,v1`. Staging the base into
 *   a live pointer register (`tbl`) keeps base as PLUS operand-1, so the base
 *   insn is emitted first -> base->$v0, index->$v1, `addu s0,v1,v0` == target.
 *
 * WHY IT'S GATED (cheat-reviewer verdict, s1): `tbl` is behaviorally identical
 * to `&D_800EF980[arg0]`, used exactly once; it carries NO observable effect and
 * its only purpose is to steer register birth order (pure test-3 GCC-internals
 * justification). That places it in the [[pointer-alias-fake-exception]] family
 * (narrow last-resort carve-out), NOT the unconditional named-intermediate
 * family. To become committable it needs ALL of:
 *   (a) documented full pure-C lever-exhaustion (register-alloc-pure-c A/B/C on
 *       the prologue addr comp + a directed permuter sweep from floor-4 targeting
 *       the addu $s0 operand-order swap) — NOT yet done (only 2 forms tried),
 *   (b) the named GCC-pass mechanism (have it — fold PLUS-operand canonicalization
 *       -> local-alloc birth order; see above),
 *   (c) a /* FAKE: <mechanism> */ annotation on the `tbl` declaration.
 * Pursue (a) FIRST; the alias is the fallback if the clean levers are exhausted. */
