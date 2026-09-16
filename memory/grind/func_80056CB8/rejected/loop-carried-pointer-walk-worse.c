/* REJECTED (s12, 2026-09-16). KILLED (instance), not a cheat.
 *
 * MOTIVATION: tools/ra_solver/inverse_compose.py classify (object-level,
 * text1b func_80056CB8, target-object build/src/text1b.o vs the s11-banked
 * floor-48 sandbox .o) confirmed PRE-RA divergence with an explicit
 * instruction-multiset diff: target carries `addiu s8,s8,2` (the known
 * $fp/$s8 i*2 accumulator) plus `addu #,#,s8` / `beqz` / `bltz` / `j` that
 * our build lacks; our build carries `move #,s8` / `move s8,#` /
 * `lw s8,168(#)` plus a bigger frame (`addiu sp,sp,-176` vs target's
 * `-168` -- exactly one extra 8-byte spill slot) that target lacks. This
 * reads as: our compile spills something to the stack and shuffles it
 * through $s8, while target keeps a genuine i*2 accumulator resident in
 * $s8 across the whole loop body without ever spilling.
 *
 * s6/s7/s10 had already killed sharing i*2 as one C value via (a) a fresh
 * per-iteration int local, (b) a genuine loop-carried INT induction
 * variable, and (c) per-table POINTER locals RECOMPUTED fresh each
 * iteration (not loop-carried). The one untried spelling implied by the
 * classify output was a per-table POINTER as a genuine LOOP-CARRIED
 * induction variable (incremented by 2 in the for-statement's increment
 * clause, mirroring target's actual $fp/i*2 accumulator structure at the
 * POINTER level instead of the integer level):
 *
 *   u8 *flags_p = &D_8009A821 + start * 2;
 *   u8 *scale_p = &D_8009A820 + start * 2;
 *   for (i = start; i < start + 2; i++, flags_p += 2, scale_p += 2) {
 *       ...
 *       flags = *flags_p << 8;
 *       ...
 *       scale = *scale_p << 8;
 *   }
 *
 * MEASURED on the s11/s12 floor-48 chassis (func_80053614 s32-return fix +
 * s7 flags/ang/code merge + s11 r1/r2 merge, unmodified otherwise):
 * sandbox func_80056CB8 --disable all: score 48 -> 78 (WORSE),
 * build_insns 198 -> 209 (MORE real instructions, not fewer -- two
 * separate pointer inductions cost more than the array-index recompute
 * they replaced). Reverted immediately; re-confirmed floor 48 exactly
 * reproduces after revert.
 *
 * CONCLUSION: this closes the last untried "share i*2 as one C value"
 * spelling (int-fresh, int-loop-carried, pointer-fresh, pointer-loop-
 * carried -- all four now measured and all four WORSE than plain
 * `(&D_x)[i*2]` array indexing on this chassis). The classify-confirmed
 * PRE-RA multiset gap (missing $fp accumulator + 8-byte frame delta) is
 * real, but is NOT reached by any C-level re-spelling of how the doubled
 * index is NAMED or CARRIED -- consistent with s11's loop.c strength-
 * reduction-threshold root cause (the giv rejection is a function of the
 * loop body's insn_count, not of which C construct expresses the index).
 * Do not re-propose any spelling of "i*2 lives in one shared C handle"
 * for this residual; the remaining lever (if any) is shrinking/reshaping
 * the loop body's insn_count enough to cross the strength-reduction
 * benefit threshold, per s11's frontier item 3 -- not touching the index
 * expression itself.
 * ------------------------------------------------------------------- */
