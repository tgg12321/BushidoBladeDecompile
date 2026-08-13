# CdControlF — hypothesis frontier

Floor history: **23 (s1 start) → 11 (s1 end)**. Structure is exact (75/75
instructions, same order); the residual is register assignment only.

## KILLED in session 1 (do not re-run)

- **H-s1-a: declaration order steers allocation.** KILLED. All 120 declaration
  permutations score identically (14) with the init order held fixed.
  `sweep2_results.json`.
- **H-s1-b: the inherited `new_var`/`new_var2` DImode chain is load-bearing.**
  KILLED. The 11-scoring candidate has no such chain.
- **H-s1-c: narrowing spellings (`(u8)a0`, `elem[0]`, byte-offset pointer math)
  move the allocation.** KILLED — all neutral at 14. `sweep3_results.json`.
- **H-s1-d: initialising `result` late helps.** KILLED, and it is actively
  harmful (24–31). `result` must be initialised first-or-early.

## CONFIRMED in session 1

- **H-s1-e: init-statement order is the allocation lever.** CONFIRMED — the
  same 5 statements span 14…31 by ordering alone.
- **H-s1-f: CdControlB's `base = g_cd_sector_buf; elem = base + idx;` two-step
  is the right address spelling.** CONFIRMED — fixes the v0/v1 scratch pair and
  drops 14 → 11.

## KILLED in session 2 (do not re-run)

- **H-s2-a: a real loop (`do/while`, `for`, `while(1)+break`) instead of the
  goto chain reshapes the allocation favourably.** KILLED — 18 / 22 / 18
  against a base of 11. The flat goto chain is target's shape.
  `variants_a.json`; form banked at
  `rejected/real-loop-forms-do-while-for-while1-regress-18-22.c`.
- **H-s2-b: subscripting the base pointer in the loop (`base[idx]`, no `elem`
  local) lets loop.c create a better-shaped pseudo.** KILLED — 26.
- **H-s2-c: the masking spelling matters once the param is `u8`.** KILLED —
  `idx = a0` vs `idx = a0 & 0xFF`, `CD_cw(a0, ...)` vs `CD_cw(a0 & 0xFF, ...)`,
  and `u8 *a1` are all exactly 9. `s8`/`u16` com regress to 16; a `u8` LOCAL
  `idx` regresses to 27. `variants_b.json`.
- **H-s2-d: block-local scoping / nested-if flattening / loop-test
  re-association move the result/elem live ranges.** KILLED — seven spellings,
  every one leaves `result` at nrefs=3/livelen=39 and `elem` at
  nrefs=2/livelen=31, all still 9. `variants_e.json`.
- **H-s2-e: the init-order axis still has room on the new (u8) base.** KILLED —
  all 240 legal orders re-swept, floor flat at 9. `sweep_c.json`.
- **H-s2-f: `elem` can win by shrinking its live range (2 refs).** KILLED
  ARITHMETICALLY — it would need livelen 25; `elem` is live across the loop
  back edge so its live range is bounded below by the loop body, and 31 is the
  minimum observed in any spelling.
- **H-s2-g: `result` can lose by stretching its live range (3 refs).** KILLED
  ARITHMETICALLY — it would need livelen ≥ 47; the entire function is ~40 RTL
  insns and `result`'s 39 is already the longest live range in it.
- **H-s2-h: reusing `elem` as its own base (`elem = g_cd_sector_buf; elem =
  elem + idx;`) is the ref lift.** KILLED — that is a FOUR-reference lift
  (`floor_log2(4)*4 = 8`), pri 2352, elem takes s0, score 27.
  `rejected/elem-self-two-step-nrefs4-regress-27.c`.
- **H-s2-i (correction, not a kill): s1's "GCC creates the pseudo at first use,
  not at the declaration".** WRONG. `expand_decl` numbers pseudos in
  DECLARATION order; reordering the declarations renumbers them with every
  nrefs/livelen/pri unchanged. Declaration order is score-inert only because no
  TIE currently exists for it to break — it is a real lever the moment one does.

## CONFIRMED in session 2

- **H-s2-j: the Sony libcd `u_char com` parameter type is worth points.**
  CONFIRMED — 11 → 9, the only floor movement this session. The forward
  `extern` at `src/system.c:1012` was updated to match and measured
  codegen-neutral (all five callers stay at 0).
- **H-s2-k: the closing condition is a single, exactly-specified inequality.**
  CONFIRMED by `BB2_ALLOC_DEBUG=1` on the instrumented `tools/gcc-2.7.2/cc1`.
  Measured priorities: count 2285, a1 2162, idx 882, a0 810, saved 810,
  **result 769, elem 645**. First five rows already match target. Closing
  requires `pri(elem) ∈ (769, 810)` or `pri(result) < 645`, with
  `pri = floor_log2(nrefs)*nrefs/livelen*10000` and ties broken by pseudo
  (= declaration) number.
- **H-s2-l: a 2-reference `result` does produce target's register layout.**
  CONFIRMED by probe — with `return 0;` in the failure tail, `result` drops out
  of the allocno list and `elem` takes s5 (target's register). The probe is not
  a candidate: it also deletes the `li s6,-1 / addiu v0,s6,1` tail pair.

## Live frontier (session 3+)

### F0 — a byte-neutral THIRD reference to `elem`, livelen ~38
**Statement.** The measured window is exact: `elem` needs
`pri = 30000/livelen ∈ (769, 810)`, i.e. **nrefs = 3 and livelen = 38**
(nrefs=2 is arithmetically impossible, nrefs=4 overshoots by ~3×). Every other
allocno is already correct, so this single change closes the function.
**Mechanism.** `global.c:635 allocno_compare`; `reg_n_refs`/`reg_live_length`
are set by flow.c's life analysis, which runs after cse/loop/combine — so a
second `*elem` read that a LATER pass folds still counts, while one that cse
folds before flow does not. `elem` currently has livelen 31; a spelling that
also lengthens its live range to 38 (initialising it earlier raises it to 34,
measured) is required simultaneously.
**Next probe.** Enumerate spellings that read `elem` a second time WITHOUT
adding an instruction, and read the ALLOCDBG table for each rather than the
score — `tmp/grind/CdControlF/s2/variants_c.py` already has the harness
(`splice` + `score` + `allocdbg()`), so a new batch is a list of bodies. Start
with reads that sit on already-emitted control-flow edges. NOTE: if the only
surviving shape turns out to be a duplicated `*elem` read across the arms of
the `a1` test, that is the SANCTIONED `duplicated-statement-into-arms` family
(byte-neutral reg_n_refs lift) — but it requires demonstrated lever exhaustion,
a `/* FAKE */` annotation and layer-1+2 review, so it must NOT be reached for
before the honest spellings above are enumerated and killed.

### F1 — a 2-reference `result` that still emits target's tail
**Statement.** `pri(result) = 20000/39 = 512 < 645` would put `result` last
outright — a much wider target than F0's 2-unit window. The probe confirms the
allocation swaps. The obstacle is semantic: `result = 0`, `result = -1` and
`return result + 1` are all three needed to emit target's
`move s6,zero` / `li s6,-1` / `addiu v0,s6,1` ×2.
**Mechanism.** Same `allocno_compare`. `floor_log2(2)*2 = 2` vs
`floor_log2(3)*3 = 3` — dropping one reference cuts the priority by a third.
**Next probe.** Look for a spelling where one of the two constant stores
belongs to a DIFFERENT pseudo whose live range does not overlap `result`'s, so
that local_alloc (not global_alloc) handles it and it never becomes an allocno
— e.g. a second named local for the failure value that is copied into the
return expression. `base` already demonstrates the shape: it is a real local
that never appears in the global allocno list (local_alloc gives it v1).

### F2 — port the winning form to the sibling `CdControl`
**Statement.** `CdControl` (queue-active, distance 25, still carrying a
`register s32 result asm("s7")` pin) is the same function with a third
argument, and it has the same `u_char com` PsyQ prototype.
**Mechanism.** Same three-way family, same allocno-ordering question, one extra
live argument. The `u8` param lever alone is likely worth points there.
**Next probe.** Out of scope for a CdControlF session — replay the u8-param
change plus the sweep-4 init-order sweep against CdControl once this closes.

### F1-OLD — swap the `result` / `elem` allocno order (superseded by F0/F1 above)
**Statement.** `result` is currently allocated before `elem` (s5 vs s6); target
has `elem`=s5, `result`=s6. Every one of the 11 remaining differing instructions
mentions one of those two values, so a single swap closes the function.
**Mechanism.** GCC 2.7.2 `global.c:635 allocno_compare` ranks by
`floor_log2(n_refs)*n_refs / live_length`. Our `result` scores 3/L, our `elem`
scores 2/L. The original's C must give `elem` the higher rank — a spelling with
one more genuine `elem` reference, or one that lengthens `result`'s live range
(live_length is the divisor, so a longer-lived `result` sinks below `elem`).
**Next probe.** Take a `cc1 -O2 ... -da` dump of the candidate
(`tmp/grind/CdControlF/s1/dump.sh` already builds `system.i`; add `-da` and read
the `.greg` file's `;; Register dispositions` + allocno priority list) and read
the ACTUAL n_refs / live_length pair for the two pseudos instead of the
hand-count above. Then sweep honest respellings that raise `elem`'s reference
count — e.g. walking the pointer (`if (*elem != 0)` becoming a form that reads
`elem` in both the guard and the CD_cw argument path), or hoisting the loop's
`g_cd_callback_a` stores in a way that extends `result`'s range across the loop.
No dead stores, no holders — this is a genuine-reference question.

### F2 — the PsyQ prototype question (`u_char com, u_char *param`)
**Statement.** The real Sony libcd signature is
`int CdControlF(u_char com, u_char *param)`; ours is `s32 CdControlF(s32, s32)`.
A pointer-typed `param` and a `u_char`-typed `com` change what CD_cw's argument
setup looks like and could change the liveness of the `a0` copy (s3), which in
turn changes the allocno ordering `result`/`elem` sits inside.
**Mechanism.** Parameter types drive the incoming-arg copy pseudos and their
`reg_n_refs`; the s3 raw-`a0` copy exists precisely because `a0 & 0xFF` is needed
twice. A `u_char` param would fold one of those.
**Next probe.** Re-type the params (and `CD_cw`'s declared prototype, which is
already `s32 CD_cw(s32, void *, void *, s32)`) to the PsyQ shape and re-run the
sweep-4 init-order sweep over it. Cross-check against sotn-decomp's
`src/main/psxsdk/libcd/sys.c`, which the file's existing comments already cite as
the C reference for CdSync/CdReady in this same TU.

### F3 — port the winning form to the sibling `CdControl`
**Statement.** `CdControl` (queue-active, distance 25, still carrying a
`register s32 result asm("s7")` pin) is the same function with a third argument.
Whatever closes CdControlF should close it.
**Mechanism.** Same allocno-ordering question, one extra live value.
**Next probe.** After CdControlF closes, replay sweep-1/sweep-4 against
CdControl with the winning spelling as the seed. (Out of scope for a CdControlF
session — recorded so the work is not re-derived.)

## [s1] The residual is not structural — target and build emit the same 75 instructions and differ only in which callee-saved register each value occupies.
- mechanism: Register allocation only: GCC 2.7.2 global.c assigns the same RTL to different hard regs; no insn count, branch, frame-size or jal-site difference exists.
- probe: objdump -d tmp/sandbox/CdControlF/system.o vs asm/funcs/CdControlF.s, instruction-by-instruction
- result: 75 vs 75 instructions, identical opcode sequence; every differing instruction differs only in an s-register name. Target: s0=count s1=a1 s2=idx s3=a0 s4=saved s5=elem s6=result; session-start build had result at s2 and everything else shifted up one.
- verdict: CONFIRMED

## [s1] Init-statement order steers the callee-saved allocation.
- mechanism: Allocno creation/priority order in global.c follows first-use order in the RTL, which follows initialiser statement order.
- probe: Full 60-permutation sweep of the five initialiser statements (constraint idx-before-elem), each scored with sandbox --disable all (tmp/grind/CdControlF/s1/sweep_results.json)
- result: Scores span 14..31. Eight orders reach 14, all with `result` initialised first-or-early and `elem` initialised last. Baseline order scored 23.
- verdict: CONFIRMED

## [s1] Declaration order steers the allocation too.
- mechanism: Proposed: declaration order biases pseudo numbering / LUID.
- probe: All 120 declaration permutations with the init order pinned to a 14-scoring winner (tmp/grind/CdControlF/s1/sweep2_results.json)
- result: Every one of the 120 scored exactly 14. Declaration order is completely inert for these scalars — GCC 2.7.2 creates the pseudo at first use, not at declaration.
- verdict: KILLED

## [s1] The matched sibling CdControlB's two-step address spelling (`base = g_cd_sector_buf; elem = base + idx;`) is the right form for elem.
- mechanism: The one-step `&g_cd_sector_buf[idx]` emits `sll v1,s2,2 / lui v0 / addiu v0 / addu elem,v1,v0`; target emits `lui v1 / addiu v1 / sll v0,s2,2 / addu elem,v0,v1`. The named base pointer materialises the symbol into v1 and the scaled index into v0, matching target's scratch-register pairing.
- probe: Variant sweep A_base_two_step then a 240-permutation init-order re-sweep over the resulting 6-local form (sweep3_results.json, sweep4_results.json)
- result: 14 -> 11. v0/v1 now match target exactly. Confirmed floor of the two-step form is 11 across all 240 legal init orders.
- verdict: CONFIRMED

## [s1] The inherited `int new_var = 3; unsigned long long new_var2 = new_var; count = new_var2;` DImode chain, and narrowing respellings ((u8)a0, elem[0], byte-offset pointer math), carry allocation signal.
- mechanism: Proposed: DImode widening / narrowing casts change the pseudo's mode and therefore its allocno size and rank.
- probe: Variants v2 (plain `count = 3;`), F_u8_param, C_elem_deref_local, D_byte_offset, each swept (sweep_results.json, sweep3_results.json)
- result: All neutral — the 11-scoring candidate reaches its floor with a plain `count = 3;` and no casts. The DImode chain is not load-bearing and is deleted from the candidate.
- verdict: KILLED

## [s1] The `register s32 result asm("s6")` pin at HEAD was buying score.
- mechanism: The sandbox strips cheat-asm before scoring, so a pin is score-inert by construction.
- probe: Pin-free variant v1 (identical body otherwise) scored against the pinned HEAD form
- result: Both score 23. The pin bought nothing; the candidate deletes it, removing a cheat-asm construct from src/system.c.
- verdict: KILLED

## [s2] Typing the first parameter u8 (the Sony libcd `int CdControlF(u_char com, u_char *param)` shape flagged as frontier F2 by session 1) lowers the honest floor.
- mechanism: The s32 form has to spell the two `& 0xFF` masks explicitly; with a u8 parameter the narrowing is the parameter's own type and GCC re-materialises the two `andi` at exactly target's two sites, while the incoming-arg copy pseudo keeps the raw a0 value in s3 as target does.
- probe: Batch A: 9 structural variants over the session-1 candidate, each scored with `sandbox CdControlF --disable all` (tmp/grind/CdControlF/s2/variants_a.json). Batch B: 10 spellings around the winner (variants_b.json).
- result: 11 -> 9. The masking spelling is then irrelevant: idx = a0 / idx = a0 & 0xFF, CD_cw(a0,...) / CD_cw(a0 & 0xFF,...), and u8 *a1 all score exactly 9. s8 com and u16 com regress to 16; a u8 LOCAL idx regresses to 27.
- verdict: CONFIRMED

## [s2] The closing condition for the remaining residual is a single, exactly-specified inequality on GCC's allocno priority, not an open search.
- mechanism: global.c:635 allocno_compare sorts by pri = floor_log2(n_refs)*n_refs / live_length * 10000 * size and assigns hard registers in that order, ties broken by allocno index (= pseudo number).
- probe: BB2_ALLOC_DEBUG=1 on the instrumented tools/gcc-2.7.2/cc1 (driver tmp/grind/CdControlF/s2/allocdbg.sh) printing `ALLOCDBG ord/pseudo/hardreg/nrefs/livelen/pri` for every allocno, on the floor-9 form.
- result: count 2285 (s0), a1 2162 (s1), idx 882 (s2), a0 810 (s3), saved 810 (s4), result 769 (s5), elem 645 (s6). The first five rows already match target exactly; target wants elem at s5 and result at s6. Closing therefore requires pri(elem) strictly inside (769, 810), or pri(result) strictly below 645. Nothing else in the function is wrong.
- verdict: CONFIRMED

## [s2] elem can be made to outrank result by shrinking its live range while keeping 2 references.
- mechanism: pri = 20000/livelen for a 2-reference allocno; it would have to reach livelen 25 to enter the (769, 810) window.
- probe: ALLOCDBG livelen readings for elem across every spelling measured this session (variants_c/d/e.json, sweep_c.json over all 240 legal init orders).
- result: KILLED arithmetically. elem is live across the loop back edge, so its live range is bounded below by the loop body (~28 RTL insns); the smallest livelen observed in any spelling is 31, giving a ceiling of pri 714 < result's 769. Initialising elem earlier moves it the WRONG way (livelen 34, pri 588).
- verdict: KILLED

## [s2] result can be made to fall below elem by stretching its live range while keeping 3 references.
- mechanism: pri = 30000/livelen for a 3-reference allocno; it would have to reach livelen 47 to fall below elem's 645.
- probe: ALLOCDBG livelen readings for result and for every other pseudo in the function, across all of batches C, D and E.
- result: KILLED arithmetically. The whole function is ~40 RTL insns and result's livelen of 39 (set by the first statement, read by the last) is already the longest live range in it. 47 is not reachable without adding instructions, which breaks the 75/75 structure.
- verdict: KILLED

## [s2] Writing the retry loop as a real loop (do/while, for, while(1)+break) instead of the goto chain reshapes the allocation favourably. Every prior sweep held the goto-chain body fixed, so this axis had never been touched.
- mechanism: Loop notes (NOTE_INSN_LOOP_BEG/CONT/END) change loop.c's rotation and LICM behaviour and therefore both the pseudo set and the live ranges.
- probe: Batch A variants E1/E1b/E2/E3, each scored (variants_a.json).
- result: KILLED. do/while + continue 18, do/while + explicit decrement 18, while(1)+break 18, for-loop 22, against a base of 11. Target's shape is the flat goto chain. Banked at rejected/real-loop-forms-do-while-for-while1-regress-18-22.c.
- verdict: KILLED

## [s2] Block-local variable splits, declaration scoping, nested-if flattening and loop-test re-association move result's or elem's reference count or live length.
- mechanism: The structural catalog for this modality: NOTE_INSN_BLOCK_BEG/END placement is what flow.c walks when accumulating REG_LIVE_LENGTH, and condition flattening moves the refs themselves.
- probe: Batch E: inner-block scoping of the loop locals; inner-block scoping of base/elem alone; `idx != 1 && (g_cd_mode & 0x10)`; `a1 != 0 && *elem != 0 && CD_cw(...) != 0`; `if (--count != -1)`; plus batch C's elem[0], deref-into-local and combined-guard forms. Each measured with score AND the full ALLOCDBG table (variants_c.json, variants_e.json).
- result: KILLED. All seven leave result at nrefs=3/livelen=39 and elem at nrefs=2/livelen=31, and all still score 9. The (nrefs, livelen) pair is invariant under the entire structural catalog.
- verdict: KILLED

## [s2] Reusing elem as its own base (elem = g_cd_sector_buf; elem = elem + idx;) supplies the missing elem reference.
- mechanism: A read-modify-write of elem adds two references rather than one.
- probe: Batch C variant C3, with ALLOCDBG (variants_c.json).
- result: KILLED. That is a FOUR-reference lift: floor_log2(4)*4 = 8, so elem's pri jumps 645 -> 2352 and it takes s0 outright. Score 27. The useful negative: elem needs EXACTLY 3 references, not 4 — a fourth overshoots by ~3x. Banked at rejected/elem-self-two-step-nrefs4-regress-27.c.
- verdict: KILLED

## [s2] The init-statement-order axis, which was session 1's floor lever, still has room on the new u8 base.
- mechanism: Init order sets each pseudo's first set and therefore its live length; session 1 measured a 14..31 span over this space on the s32 form.
- probe: Full re-sweep of all 240 legal init orders (idx and base before elem) over the u8 form (sweep_c.json).
- result: KILLED for further gain. Floor flat at 9, reached by every order with result first-or-second and base,elem last; result-last scores 20-24. Init order does move livelens (elem 31 -> 34 when initialised early, result 39 -> 35), but both in the wrong direction.
- verdict: KILLED

## [s2] A 2-reference result really would hand elem target's register (model validation, not a candidate).
- mechanism: pri(result) would become 20000/39 = 512, below elem's 645, putting result last in the priority order and elem at s5.
- probe: Batch D probes D1 (`return 0;` in the failure tail) and D2 (`return 1;` on the success exit), each with ALLOCDBG (variants_d.json).
- result: CONFIRMED. In both probes result drops out of the allocno list entirely and elem takes hardreg 21 = s5, target's register. Neither is a candidate: they also delete the `li s6,-1 / addiu v0,s6,1` tail pair that target emits, because result = 0, result = -1 and return result + 1 are all three semantically required. The probe establishes the mechanism and points the next session at 'find a 2-reference result that still emits the tail'.
- verdict: CONFIRMED

## [s2] Session 1's conclusion that GCC creates a local's pseudo at first USE rather than at its declaration (its explanation for declaration order being score-inert).
- mechanism: expand_decl assigns DECL_RTL, and therefore the pseudo number, when the declaration is processed at block entry.
- probe: Batch D variant D4: declarations reordered to put base/elem first, init order untouched, ALLOCDBG compared against the control.
- result: KILLED (correction). The pseudos are renumbered exactly as declaration order predicts (elem 80 -> 76, result 75 -> 77) with every nrefs/livelen/pri unchanged and the score still 9. Declaration order is score-inert only because no priority TIE currently exists for it to break — allocno_compare's tiebreak is `return *v1 - *v2;` on the allocno index, so declaration order becomes a real lever the moment a spelling makes pri(elem) == pri(result).
- verdict: KILLED
