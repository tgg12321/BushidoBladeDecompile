/* KILLED s3 (structural, 2026-07-20) — H4 phantom-slot mechanism.
 *
 * Hypothesis (from s2 frontier): pinpoint via cc1 -da the exact reload/alter_reg
 * trigger conditions in a MATCHING BUILD of func_80049A2C (dummy[2] in place,
 * sandbox=0). Look for pseudos with reg_n_refs>0 && reg_renumber<0 that receive
 * no hard reg — that IS the phantom firing. If it's already firing cheat-freely,
 * replicate its entry condition without dummy[2].
 *
 * MEASUREMENT (tmp/grind/func_80049A2C/s3/da/*.f80049A2C):
 *
 *   With dummy[2] IN (baseline, sandbox=0):
 *     .frame $sp,48 # vars=8, regs=5/0, args=16
 *     7 pseudos: 89,123,133,141,94,73,81 — ALL get hard regs
 *     Register dispositions: 73→17(s1), 81→19(s3), 89→3(v1), 94→3(v1),
 *                            123→65($f1/64+1), 133→65, 141→65, etc.
 *     Only spill: "Spilling reg 7" (hard reg $a3 spill for insn 135 —
 *     outgoing-arg passing, NOT a frame slot for a pseudo).
 *     No pseudo denied a hard reg. No reload/alter_reg phantom firing.
 *
 *   With dummy[2] OUT (sandbox=12, frame-collapse baseline):
 *     .frame $sp,40 # vars=0, regs=5/0, args=16
 *     Same 7 pseudos, IDENTICAL greg dispositions (73→17, 81→19, 89→3,
 *     94→3, 123→65, 133→65, 141→65), identical conflicts, identical
 *     "Spilling reg 7" for insn 135.
 *
 *   DELTA between the two greg dumps: ZERO. The greg pass produces the same
 *   register allocation whether dummy[2] is present or not. The 8-byte frame
 *   slot in the target's prologue comes SOLELY from dummy[2]'s aggregate
 *   declaration (mips.c compute_frame_size reserves get_frame_size()=8 for
 *   an s32[2] aggregate unconditionally at declaration time).
 *
 * MECHANISM: phantom-frame-slots-gcc272 describes reload/alter_reg reserving
 * stack slots for pseudos with stale reg_n_refs>0 that no longer receive a
 * hard reg. In func_80049A2C's ACTUAL greg pass, that condition NEVER fires:
 * every pseudo gets a hard reg (7 of 7), and no reload-time reservation is
 * emitted. The "phantom firing on live code" hypothesis is measurably absent.
 *
 * CONSEQUENCE: there is no phantom-slot entry condition to replicate in a
 * different C shape — the mechanism simply does not activate for this
 * function's register pressure. Combined with s2's H1/H2 kills (dead-HImode-
 * bitwise DCE'd before combine; scalar widening does not trigger a slot; s2's
 * scalar_dummy no-frame kill), every "get the +8 slot via a non-aggregate
 * lever" avenue on the phantom axis is now measured dead.
 *
 * The +8 frame slot in target is reachable ONLY via an aggregate declaration
 * (array or struct) — either dummy[2] as it stands (owner-gated H5), or a
 * semantically-loaded aggregate that emits no target-diverging bytes (no such
 * construction exists for this shape: any aggregate whose stores are
 * unconditionally DCE'd IS dummy[2] with different lipstick; any aggregate
 * whose stores survive DCE emits sw instructions the target does not have).
 *
 * ARTIFACTS:
 *   tmp/grind/func_80049A2C/s3/build_da.sh
 *   tmp/grind/func_80049A2C/s3/slice.py
 *   tmp/grind/func_80049A2C/s3/da/text1b.i.greg.f80049A2C  (dummy-OUT, last-written)
 *   (dummy-IN dump was overwritten by the counterfactual re-run; re-derivable
 *    from build_da.sh with dummy[2] restored, deterministic under project cc1 flags)
 */
