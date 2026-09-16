/* REJECTED (s33, 2026-09-16). KILLED (instance), not a cheat.
 *
 * MOTIVATION (s31/s32-named frontier item #1): two SEPARATELY-named locals
 * that `combine_givs` (loop.c:5502-5512) might still merge into one giv
 * (same biv/mult/add triple, merge condition does not require identical
 * source expressions) could let global_alloc/reload treat them as two
 * short individual-lifetime pseudos rather than one pseudo forced to live
 * across the whole ratan2+func_80053614 call span -- potentially flipping
 * loop.c:3823's giv-worth decision without paying the s32-measured
 * shared-idx-local cost (lifetime 42, forced 9th call-spanning resident).
 *
 * SPELLING (applied to the s22-s32-banked 38/204 body, changed nothing
 * else):
 *
 *   s32 idxB;
 *   ...
 *   flags = (&D_8009A821)[i * 2] << 8;   // unchanged, own i*2
 *   idxB = i * 2;                         // new, same source position
 *   ...
 *   scale = (&D_8009A820)[idxB] << 8;    // was (&D_8009A820)[i * 2]
 *
 * MEASURED on the s33 chassis (func_80053614 s32-return prerequisite +
 * header externs restored, no FAKE constructs): sandbox
 * func_80056CB8 --disable all: score 38 -> 51/204 (WORSE), build_insns
 * 198 -> 200 (+2 real instructions). Reverted immediately; re-confirmed
 * 38/204 exactly reproduces after revert.
 *
 * DUMP EVIDENCE: fresh `pwsh tools/grinder/dump.ps1 func_80056CB8` did not
 * emit a `.loop`/`.combine` pass file for this build (no promotion decision
 * logged); `grep -n "giv\|strength" tmp/grind/func_80056CB8/s33/dumps/text1b.lreg`
 * returned nothing -- no giv-promotion trace, unlike s32's successful trace
 * on the DIFFERENT shared-single-idx form (lifetime 42, giv promoted,
 * reduced to reg 217). The +2 insn delta reads as ordinary extra register
 * pressure from carrying a second live pseudo with the same value as the
 * first (split-then-recombine cost), not a giv-promotion event.
 *
 * CONCLUSION: combine_givs either did not treat the two separately-named-
 * but-identical-value locals as a mergeable strength-reduction candidate,
 * or did and the extra pseudo's register-pressure cost exceeded any
 * benefit. This closes the "two separately-named locals" spelling of the
 * shared-i*2-index family -- the sixth and (per s33's audit) apparently
 * final untried spelling of "how the doubled index is NAMED or CARRIED"
 * (int-fresh, int-loop-carried, pointer-fresh, pointer-loop-carried,
 * single-shared-index, two-separately-named-index -- all six now measured,
 * all six flat-or-worse). Do not re-propose any further respelling of how
 * i*2's VALUE IDENTITY is carried for this residual; per s31/s32/s33 the
 * remaining levers are (a) shrinking the loop body's real-insn count below
 * 124 without touching the index expressions at all, or (b) restructuring
 * one of the other 8 residents' conflict footprint.
 * ------------------------------------------------------------------- */
