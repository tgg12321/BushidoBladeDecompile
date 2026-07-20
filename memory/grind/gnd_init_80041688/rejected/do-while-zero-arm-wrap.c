/* REJECTED [s14] — do-while(0) arm wrappers (three variants), all INERT.
 *
 * Motivation: per [[do-while-zero-exception]] (2026-07-06 final ruling),
 * `do { ... } while(0);` IS sanctioned for ANY codegen effect. This is the
 * only sanctioned wrapper construct not yet measured on the FALSE-arm
 * pseudo-78-fusion / sched1 hazard-tag axis. Tested three placements:
 *
 *   Variant A — do{...}while(0) around FALSE-arm r/g/b + gnd_load_tex:
 *     } else {
 *         do {
 *             r = ...0x18; g = ...0x19; b = ...0x1A;
 *             gnd_load_tex(b | ((r<<16) | (g<<8)));
 *         } while (0);
 *     }
 *   Result: sandbox --disable all -> score=2, build_insns=82. INERT.
 *
 *   Variant B — do{...}while(0) around TRUE-arm only (asymmetric):
 *     if (func_800486FC()) {
 *         do {
 *             r=...; g=...; b=...; v=func_8004881C(b,g,r);
 *             gnd_load_tex((v<<16)|(v<<8)|v);
 *         } while (0);
 *     } else { ...baseline FALSE arm... }
 *   Result: sandbox --disable all -> score=2, build_insns=82. INERT.
 *
 *   Variant C — do{...}while(0) around FALSE-arm color triple ONLY,
 *     with gnd_load_tex OUTSIDE the wrap:
 *     } else {
 *         do { r=...; g=...; b=...; } while (0);
 *         gnd_load_tex(b | ((r<<16) | (g<<8)));
 *     }
 *   Result: sandbox --disable all -> score=2, build_insns=82. INERT.
 *
 * Mechanism killed: GCC 2.7.2's c-parse.y lowers `do{...}while(0)` early —
 * jump.c/cfg passes strip the trivial back-branch before flow.c sees any
 * loop structure. NOTE_INSN_LOOP_BEG/END markers are dropped ahead of
 * sched1, so the wrapper produces byte-identical RTL to the unwrapped
 * form. Corroborates s11's label-placement inertia: GCC 2.7.2 aggressively
 * removes redundant control flow before the schedule-visible passes.
 *
 * Implication for the ledger: the do-while(0) axis was the last un-tested
 * sanctioned wrapper construct on the sched1 hazard-tag lever surface for
 * this function. Its measurement confirms the s10 case-exhaustion synthesis
 * empirically at one additional independent axis. The pseudo-78-fusion
 * frontier is now case-exhausted across four CFG sites (loop1, pre-branch,
 * intra-BB init, do-while(0) wrap) plus the three permuter chassis (s4/s5
 * chassis-1, s13 chassis-2, s13 chassis-3).
 */
