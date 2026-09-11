# Hypothesis ledger -- _spu_gcSPU

## s1 (2026-09-11, recon)

### H1 -- DECLARATION FIX (mandated first probe: both DATA MODEL symbols flagged
`decl: NONE in include/*.h`). **KILLED (instance) -- no declaration change is
needed; the existing shapes are correct.**
`_spu_AllocLastNum` as `extern s32` (main.c:57) and `_spu_memList` as
`extern s32` viewed through `#define _spu_memList ((SpuMemRec *)_spu_memList)`
(main.c:58 + :2107) both measure **0** unchanged. See the `OBJECT MODEL:` entry
in evidence.md. The only declaration defect present was a redundant, and in the
`_spu_memList` case macro-broken, re-declaration pair sitting below the macro;
removing it was measured byte-neutral (22 before, 22 after).

### H2 -- the psyz `s_m_int.c` body is the right chassis. **CONFIRMED.**
Transplanting Xeeynamo/psyz `decomp/src/libspu/s_m_int.c` verbatim (5 phases,
plain `for`, array indexing, loop bound as a live global read) takes the floor
from 194 to 22 in one compile, with phases 2-5 instruction-exact. The
catalogued cc1 ICE (structured pointer-walk) did NOT reproduce on the
array-indexed form.

### H3 -- phase 1's inner scan must exit by `goto`, not `break`. **CONFIRMED
(22 -> 2, phase 1 becomes instruction-identical).**
Mechanism (read out of `tools/gcc-2.7.2/stmt.c expand_end_loop` +
`tools/gcc-2.7.2/jump.c:2163 duplicate_loop_exit_test`, not inferred):
`expand_end_loop` rolls a *leading* conditional exit to the bottom of the loop
only when that exit jumps to the loop's own `end_label`/`alt_end_label`. A
`break` is exactly such a jump, so the loop is rotated; the rotation creates
`NOTE_INSN_LOOP_BEG` + simplejump, which is the trigger `jump.c:620` uses to
call `duplicate_loop_exit_test`, peeling a guard copy of the test above the
loop (+8 insns, plus a re-loaded base and a re-materialized 0x2FFFFFFF).
A `goto` to a user label after the loop does not target `end_label`, so
`last_test_insn` stays 0 and no rotation (and hence no peel) happens -- which is
the target's shape. This is an ordinary-C mixed-exit form
(`.claude/rules/cross-jump-store-tail-merge.md`), no FAKE.

### H4 -- the last 2 are the maspsx `.L`-label load-delay blind spot, fixable
only via the per-function gate. **CONFIRMED (2 -> 0, full-build SHA1 ==
oracle).**
Appending `_spu_gcSPU` to `maspsx_label_nop_funcs.txt` restores both dropped
`nop`s. This is the sanctioned pure-C retirement path
(`.claude/rules/maspsx-label-nop-gate.md`), already used by two siblings in this
same TU (`SpuFree`, `_spu_init`). The file is outside a grind session's
one-build-input scope, so the completion is an INTEGRATION HANDOFF.

## Live frontier for the next session
Nothing is open on the codegen side: the function is at **0** with zero cheat
constructs and a verified full-build SHA1 match. The only remaining step is the
operator-side scope grant; see docs/grind/decisions.md (2026-09-11 entry).

## s1 RE-RUN (2026-09-11, recon) -- re-measured after the prior run was discarded

- **H1 (declaration fix)** -- re-affirmed by construction: the floor-2 body uses
  `extern s32 _spu_AllocLastNum;` (main.c:57) and `extern s32 _spu_memList;`
  (main.c:58) viewed through `#define _spu_memList ((SpuMemRec *)_spu_memList)`
  (main.c:2107) **unchanged**. No declaration edit is in the candidate. KILLED
  (instance) on this chassis.
- **H2 (psyz s_m_int.c chassis)** -- CONFIRMED, re-measured: 194 -> 2 in one
  compile with the `goto` exit already in place. Phases 2-5 instruction-exact.
- **H3 (`goto` not `break` in phase 1's inner scan)** -- carried CONFIRMED from
  the discarded run; the floor-2 body contains the `goto` form and its
  instruction stream matches the target's unrotated loop shape (no peeled guard,
  no re-materialized 0x2FFFFFFF) at target indices 14-25 of
  `tmp/grind/_spu_gcSPU/s1/diff2.txt`. The 22-insn `break` variant was not
  re-compiled this session; treat the *number* 22 as inherited, the *shape*
  conclusion as verified.
- **H4 (the last 2 are the maspsx `.L`-label blind spot)** -- CONFIRMED
  independently, and this time WITHOUT editing the tracked gate file: a scratch
  copy of the list passed via `--label-nop-funcs=` takes the same cc1 output
  from 192 to 194 insns, word-identical to the target modulo unrelocated
  hi/lo + branch displacements. See evidence.md.

## Live frontier
Nothing open on the codegen side. The only remaining step is the operator-side
scope grant (`docs/grind/decisions.md`, 2026-09-11 `_spu_gcSPU` entry), or a
`tools/grinder/scope_allow.txt` line
`_spu_gcSPU maspsx_label_nop_funcs.txt` after which a routine
`candidate-ready` session closes the function.

## [s1] Neither DATA MODEL-flagged symbol needs a declaration change: _spu_AllocLastNum as `extern s32` (src/main.c:57) and _spu_memList as `extern s32` viewed through `#define _spu_memList ((SpuMemRec *)_spu_memList)` (src/main.c:58 + :2107) are the shapes that produce the target's %hi/%lo word reads and sll/addu index arithmetic; changing either is unnecessary on this chassis.
- mechanism: Object-model check: the three libspu allocator words (0x800A2D38 _spu_AllocBlockNum, 0x800A2D3C _spu_AllocLastNum, 0x800A2D40 _spu_memList) are Sony's three separate file-scope objects in s_m_init.c / s_m_m.c / s_m_f.c, so there is no aggregate to merge; the already-matched siblings SpuInitMalloc (src/main.c:2085) and SpuMalloc (src/main.c:2108) in this same TU use exactly these declarations.
- probe: Applied memory/grind/_spu_gcSPU/candidate.c to src/main.c with both declarations UNCHANGED and ran `sandbox _spu_gcSPU --disable all`; then compared the emitted stream instruction-by-instruction against asm/funcs/_spu_gcSPU.s.
- result: score 2 (build_insns 192, target_insns 194, rules_dropped 0) with the declarations untouched, and the aligned diff (tmp/grind/_spu_gcSPU/s1/diff2.txt) shows every %hi/%lo access and every index computation already agreeing with the target. The full pipeline replay with the maspsx label-nop gate reaches 194/194 word-identical with these same declarations. No declaration edit is present in the candidate.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-11, src/main.c with candidate.c body applied, zero FAKE constructs present, zero rules; sandbox --disable all = 2.

## [s1] The psyz PsyQ 4.0 libspu/s_m_int.c body (five array-indexed `for` phases, loop bound read as a live global in every condition, two induction locals) is the right chassis for _spu_gcSPU.
- mechanism: Version-correct Sony source: BB2 links the same PsyQ libspu object (memory/closer/psyq-library-census.md:173 maps 0x800896A0-0x800899A8 to LIBSPU/S_M_INT), so the original C is that file; array indexing rather than a hand pointer walk is what emits the target's sll/addu addressing.
- probe: `sandbox _spu_gcSPU --disable all` on the dispatch state, then with the transcribed body in place.
- result: 194 (no_c_body) -> 2. Phases 2-5 are instruction-exact; the entire residual is two dropped nops. The cc1 ICE catalogued for this function in memory/closer/phase3-progress.md (a structured pointer-walk scan) does not reproduce on the array-indexed form -- cc1 compiled it cleanly here.
- verdict: CONFIRMED

## [s1] Phase 1's inner scan must exit by `goto scanned;` rather than `break;` to emit the target's unrotated loop (test at top, unconditional `j` back-edge with `j++` in its delay slot) instead of a peeled guard plus rotated do-while.
- mechanism: tools/gcc-2.7.2/stmt.c expand_end_loop rolls a leading conditional exit to the bottom of the loop only when that exit jumps to the loop's own end_label/alt_end_label -- which is exactly what a C `break` compiles to. The rotation leaves NOTE_INSN_LOOP_BEG followed by a simplejump, the trigger jump.c:620 uses to call duplicate_loop_exit_test (jump.c:2163), which copies the test above the loop (+8 insns, plus a re-loaded %hi/%lo base and a re-materialized 0x2FFFFFFF). A `goto` to a user label placed after the loop targets neither end_label nor alt_end_label, so last_test_insn stays 0 and neither the rotation nor the peel happens.
- probe: The floor-2 body carries the `goto` form; its emitted stream at target indices 14-25 was compared against asm/funcs/_spu_gcSPU.s (tmp/grind/_spu_gcSPU/s1/diff2.txt).
- result: Phase 1 is instruction-identical to the target with the `goto` form: no peeled guard, no duplicated sll/addu/lw/nop/bne test, no re-materialized 0x2FFFFFFF preheader. The 22-insn measurement of the `break` variant is inherited from the discarded prior run and was not re-compiled this session; the SHAPE conclusion is verified here. This is an ordinary-C mixed-exit form (.claude/rules/cross-jump-store-tail-merge.md), not a carve-out -- no FAKE annotation is involved.
- verdict: CONFIRMED

## [s1] The residual 2 are two load-delay `nop`s dropped by the maspsx `.L`-label blind spot, and the per-function gate maspsx_label_nop_funcs.txt restores both, taking the build to 194/194 word-identical with the target.
- mechanism: tools/maspsx/maspsx.py is_label() recognises only $L-prefix local labels while this GCC fork emits .L-prefix, so a load whose destination is consumed across a .L merge label is not seen as a hazard and loses its nop (both the branch-consumer and load-consumer cases). The per-function gate re-enables the correction for one function without shifting other functions' maspsx output indices.
- probe: tmp/grind/_spu_gcSPU/s1/pipeline.sh -- the exact Makefile stage order (cpp | cc1 | prologue_fix | maspsx | sed align | multu_pad | as, flags copied verbatim from Makefile:154 and engine/buildconfig.py:43) run TWICE on the same cc1 output, differing only in the list handed to --label-nop-funcs: the untouched tracked file vs a SCRATCH COPY (tmp/grind/_spu_gcSPU/s1/label_nop_plus.txt) with the single line `_spu_gcSPU` appended. No tracked build input was modified.
- result: 192 insns with the HEAD list, 194 with the scratch copy. The 194-insn build compares word-for-word against the 194 target instructions of asm/funcs/_spu_gcSPU.s: every opcode and every register field matches; the only 37 differing 32-bit words are unrelocated %hi/%lo immediates and intra-function branch/jump displacements in an unlinked .o -- exactly the fields engine/score.py masks. The two restored nops are at target indices 56 (lw _spu_AllocLastNum -> bltz) and 64 (lw _spu_memList -> lw 0x4($v1)). .claude/rules/maspsx-label-nop-gate.md calls this gate 'a pure-C RETIREMENT path, not a park', and two siblings in this same TU are already listed (SpuFree, _spu_init).
- verdict: CONFIRMED
