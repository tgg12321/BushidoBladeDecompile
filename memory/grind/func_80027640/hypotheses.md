# Hypothesis ledger -- func_80027640

## s1 (2026-09-01, recon)

| # | Hypothesis | Mechanism | Probe | Result | Verdict |
|---|---|---|---|---|---|
| H1 | The four 8-stride sp slots are two local `VECTOR`s (tgt @0x10, dir @0x20), not eight volatile scalars | GCC 2.7.2 keeps aggregate locals in memory (no SRA); VECTOR is 16 bytes so two fill 0x10-0x2F exactly | form1..form4, sandbox --disable all | frame, every sp round-trip and slot assignment match; 35 (chassis) -> 21 (form1) -> 0 | CONFIRMED |
| H2 | The direction constants must be computed into a scalar then stored (`v = A; if (c) v = B; dir.v = v;`) so the multiply consumes the register | conditional overwrite keeps the value in a pseudo; storing in both arms cross-jumps the store and cse loses the value -> reload | form1 (21) vs form2 (0) | CONFIRMED |
| H3 | Ternary spelling of the same constants matches | `?:` expands with the inverted branch | form3 = 6 (beqz/bgtz vs bnez/blez) | KILLED |
| H4 | Reading `dir.vx`/`dir.vz` back from the struct in the blend is byte-neutral vs the scalar temps | cse resolves the struct-field load to the just-stored pseudo | form4 = 0 | CONFIRMED (adopted) |
| H5 | The lone missing nop after `.L800277A4` is the maspsx .L-label load-delay blind spot, retired by the per-function fidelity gate | maspsx is_label() matches `$L` not `.L`; the gate emits the nop for listed functions | added func_80027640 to maspsx_label_nop_funcs.txt -> sandbox 0 | CONFIRMED |

Frontier: none -- candidate at 0. Remaining step is the driver's byte re-verification with the
gate entry present (commit needs `[infra-rule: maspsx-label-nop]` + site citation).

## s2 (2026-09-01, recon -- re-dispatch)

| # | Hypothesis | Mechanism | Probe | Result | Verdict |
|---|---|---|---|---|---|
| H6 | With the gate entry the candidate is byte-identical through the FULL build, not just the sandbox object | the gate only adds the one hazard nop; nothing else in code6cac_b shifts | s1/build_gated.sh (scratch list in tmp) + s1/link_gated.sh (tmp relink) | 160/160 words (8 reloc-only diffs); relinked SHA1 == oracle | CONFIRMED |
| H7 | The residual 1 is reachable from C | -- | none: adjudicated assembler-fidelity gap (maspsx-gate-lists.md); cc1 stream already identical | not probed by design | FORECLOSED-BY-RULE (not a C question) |

Frontier: none for C. Next step is operator integration per decisions.md:20004.

## [s1] With func_80027640 in the label-nop gate list the candidate is byte-identical through the FULL build, not just the sandbox object
- mechanism: maspsx per-function gate emits the one store-value-consumer load-delay nop across .L800277A4; nothing else in code6cac_b shifts
- probe: tmp/grind/func_80027640/s1/build_gated.sh (Makefile pipeline with --label-nop-funcs pointed at a scratch list copy) + compare_words.py (objdump -dz) + link_gated.sh (tmp relink of the oracle-matching build/ tree with gated.o swapped in)
- result: 160/160 words, 8 differing words all unresolved pre-link relocations; relinked bb2.exe SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
- verdict: CONFIRMED

## [s1] The residual 1 instruction (the nop) is reachable by respelling the C
- mechanism: none -- cc1's instruction stream is already identical to target; the nop is inserted by the assembler (ASPSX) and dropped by maspsx across .L labels; .claude/rules/maspsx-gate-lists.md adjudicates it a FIDELITY gap ('No C spelling can emit an assembler hazard nop')
- probe: not probed by design (a C change can only move away from 0); sandbox re-measure on HEAD chassis = 1 with tdiff.py isolating the single nop diff
- result: residual is assembler-fidelity, not compiler codegen
- verdict: KILLED

## s3 (2026-09-01, structural)

| # | Hypothesis | Mechanism | Probe | Result | Verdict |
|---|---|---|---|---|---|
| H8 | Some structural respelling (decl order, block-local split, type narrowing, re-association) can move the `lh`/`sw` seam off the `.L` merge label and let maspsx emit the load-delay nop | maspsx's `is_label()` (`^\$L\d+:$`) misses `.L`, so the nop is only lost when a `.L` label sits between load and consumer; if no label sat there the ordinary path would emit it | byte-level argument over the fixed target words + the cc1 dump (tmp/grind/func_80027640/s2/cc1.s:526-531) | the target fixes `lh`@0x800277A0, `nop`@0x800277A4, `sw`@0x800277A8 AND `j 0x800277A4`@0x80027770, so ANY byte-correct C form must define a basic-block label at the nop's address, textually between the load and its consumer. Invariant under every structural lever | KILLED (proved impossible for all C forms, not merely unfound) |
| H9 | cc1 is the divergence and could be steered to emit the hazard nop itself | GCC 2.7.2 emits `#nop` load-delay hints | read cc1 output for the function | cc1 emits `#nop` for the `lh $2,0($4)`/`sw $2,16($sp)` seam but not across `.L75:` -- and its instruction stream is otherwise already the target's. The divergence is 100% maspsx-side | KILLED (compiler exonerated; not a C question) |
| H10 | The `.L` blind spot is a broad hazard needing per-function scoping (the gate list's premise) | listed-function opt-in avoids index cascades | project-wide census: compile every src/*.c with real Makefile flags, read maspsx's own DEBUG lines (scan_blindspots.sh + analyze_sites.py) | 33 sites; 28 non-consuming; 2 false positives (post-label load redefines the reg); only 3 real seams, and 2 of those are already oracle-matching because their consumer's `%hi/%lo` `lui $at` expansion fills the delay. **Exactly ONE site project-wide (func_80027640) has a genuinely unfilled delay** | KILLED (the per-function premise is empirically unnecessary) |
| H11 | A single general maspsx fidelity fix -- apply the existing `$at`/`$gp` delay-fill test inside the `.L`-label branch -- closes func_80027640 and is byte-neutral project-wide | it mirrors the ordinary path's `uses_at`/`_uses_gp`/`nop_at_expansion` logic, so `%hi/%lo` consumers (whose `lui $at` fills the slot) keep getting no nop, while an sp-relative consumer gets one -- which is exactly ASPSX's behaviour | patched COPY of maspsx under tmp/ + full pipeline rebuild of all 32 C objects + word compare + full relink (build_patched.sh, compare_words.py, link_atfix.sh) | all 31 unrelated objects byte-identical to the oracle build; func_80027640 160/160 words (8 reloc-only diffs); full-link SHA1 == oracle; and with an EMPTY gate list everything still matches, i.e. the fix SUBSUMES and retires the 5-entry list | CONFIRMED |
| H12 | Widening `is_label` globally to `[$.]L\d+:` is the fix | one-line regex repair | same harness | breaks code6cac_c2.o (mflo/mfhi-with-label div expansion reorders) | KILLED |
| H13 | Simply un-gating the load-consumer branch (no `$at` test) is the fix | drop `label_nop_func_set` | same harness | breaks code6cac_c_ab.o (func_8003ACB8 gains a nop the target lacks) | KILLED |

Frontier for C: **empty, and provably so** -- H8 closes the structural axis by construction and H9
exonerates cc1. The remaining work is not a C question: it is the 4-line maspsx fidelity repair of
H11, an operator surface (`tools/`), filed as an INTEGRATION HANDOFF distinct from the gate-list
route the Judge foreclosed (that route is NOT re-proposed; the fix makes the gate list dead code).

## [s2] Some structural respelling (declaration order, block-local split, type narrowing, statement re-association) can move the lh/sw seam off the .L merge label so maspsx emits the load-delay nop
- mechanism: maspsx is_label() is ^\$L(b|e)?\d+:$ and this GCC fork emits .L, so the nop is lost only when a .L label sits between a load and its consumer; with no label there the ordinary path emits it
- probe: Byte-level argument over the fixed target words (lh@0x800277A0, nop@0x800277A4, sw@0x800277A8, and j 0x800277A4 @0x80027770 encoded 0x08009DE9) plus the cc1 dump tmp/grind/func_80027640/s2/cc1.s:526-531
- result: Any byte-correct C form must define a basic-block label at the nop's address (GCC always labels a jump destination), textually between the load and its consumer. The seam is invariant under every structural lever
- verdict: KILLED

## [s2] cc1 is the divergence and could be steered to emit the hazard nop itself
- mechanism: GCC 2.7.2 emits #nop load-delay hints in its assembly output
- probe: Read cc1 output for the function (tmp/grind/func_80027640/s2/cc1.s)
- result: cc1 emits #nop for the lh $2,0($4) / sw $2,16($sp) seam but not across .L75:, and its instruction stream is otherwise already the target's; the divergence is entirely maspsx-side. maspsx also strips all .set reorder/noreorder so GAS cannot recover it
- verdict: KILLED

## [s2] The .L blind spot is a broad hazard that genuinely needs per-function scoping (the gate list's premise)
- mechanism: per-function opt-in avoids maspsx index cascades into index-anchored regfix/asmfix rules
- probe: Project-wide census: compile every src/*.c with the real Makefile flags and parse maspsx's own DEBUG lines (tmp/grind/func_80027640/s2/scan_blindspots.sh + analyze_sites.py)
- result: 33 sites; 28 non-consuming; 2 false positives (the post-label load redefines the register: D_800832F8 in ings2, _spu_Fw1ts in main); only 3 real load->label->consumer seams, and 2 of those already match the oracle without a nop because their %hi/%lo consumer's lui $at fills the delay. Exactly ONE site project-wide (func_80027640, sp-relative store) has a genuinely unfilled delay
- verdict: KILLED

## [s2] A single general maspsx fidelity repair -- apply maspsx's own $at/$gp delay-fill test inside the .L-label branch -- closes func_80027640 and is byte-neutral project-wide
- mechanism: mirrors the ordinary path's uses_at/_uses_gp/nop_at_expansion test, so %hi/%lo consumers keep getting no nop while an sp-relative consumer gets one -- which is ASPSX's actual behaviour
- probe: Patched COPY of maspsx under tmp/ (tools/maspsx untouched) + full pipeline rebuild of all 32 C objects (build_patched.sh, per-file -G8/--expand-lb/align2) + compare_words.py + full relink (link_atfix.sh); a stock-maspsx control run through the same harness reproduces build/src/*.o byte-identically
- result: All 31 unrelated objects byte-identical to the oracle build; func_80027640 160/160 words (8 reloc-only diffs); full-link SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle; and with an EMPTY --label-nop-funcs list every object is still identical, so the repair subsumes and retires the 5-entry gate list
- verdict: CONFIRMED

## [s2] Widening is_label globally to [$.]L\d+: is the right fix
- mechanism: one-line regex repair of the $L-vs-.L prefix mismatch
- probe: Same patched-copy harness (maspsx_p)
- result: Breaks code6cac_c2.o -- the mflo/mfhi-with-label path starts seeing the label and reorders the div expansion. Not byte-neutral
- verdict: KILLED

## [s2] Simply un-gating the load-consumer label branch (drop label_nop_func_set, no $at test) is the right fix
- mechanism: make the load-consumer variant unconditional like the always-on jalr variant
- probe: Same patched-copy harness (maspsx_u)
- result: Breaks code6cac_c_ab.o -- func_8003ACB8 gains a nop its target does not have (its %hi/%lo consumer's lui $at already fills the delay). Not byte-neutral
- verdict: KILLED

## s4 (2026-09-01, structural, 2nd dispatch)

- H-s4-1 **KILLED (and now mechanism-grounded): "some C spelling makes cc1 emit the load-delay nop
  at 0x800277A4."** Mechanism: `mips_fill_delay_slot` (tools/gcc-2.7.2/config/mips/mips.c:673,
  the single common path for every load via mips.c:1242/1591 and mips.md:2950) explicitly zeroes
  `dslots_number_nops`/`mips_load_reg` when `GET_CODE (next_insn) == CODE_LABEL` -- comment
  "Make sure that we don't put nop's after labels." `final_prescan_insn` (mips.c:4104) is the only
  emitter of `#nop` and is gated on that counter. Probe: read the source + the real dump; s2/cc1.s
  shows the same lh/sw pair WITH a `#nop` 4 words earlier where no label intervenes and WITHOUT one
  across `.L75:`. Verdict: no C input can change this; the label adjacency is forced by the target
  bytes (`j` at 0x80027770 encodes 0x800277A4). Do not re-open.

- H-s4-2 **CONFIRMED: the floor is 1 on the stock chassis with the final candidate.**
  Probe: apply candidate.c, `sandbox func_80027640 --disable all` -> score 1 (158/157,
  rules_dropped 0). tmp/grind/func_80027640/s3/sandbox_s3.txt.

- REMAINING FRONTIER after s4: none on the C axis. The only known route to 0 is the assembler-layer
  repair (s3's measured `$at`-aware maspsx fix, SHA1 == oracle, byte-neutral across all 31 other
  objects, subsumes and retires maspsx_label_nop_funcs.txt) -- foreclosed for grind sessions by the
  Judge's binding no-build-surface-change constraint. The next productive modality is NOT another
  C-spelling search; it is forensics/rederive to look for a chassis-independent reading of the seam,
  or the escalation record once the driver declares exhaustion.

## [s3] Some C spelling (declaration order, block-local split, type narrowing, statement re-association) makes cc1 emit the missing load-delay nop at 0x800277A4.
- mechanism: GCC 2.7.2 mips backend: every load routes its delay bookkeeping through mips_fill_delay_slot (tools/gcc-2.7.2/config/mips/mips.c:673, reached from mips.c:1242 and mips.c:1591 and mips.md:2950). That function zeroes dslots_number_nops / mips_load_reg when GET_CODE(next_insn) == CODE_LABEL (skipping NOTEs only) -- its own comment is 'Make sure that we do not put nop's after labels.' final_prescan_insn (mips.c:4104) is the only emitter of '#nop' and is gated on that counter, and is never called for a CODE_LABEL.
- probe: Read the two source regions (banked as tmp/grind/func_80027640/s3/mips_fill_delay_slot.excerpt.txt and final_prescan_insn.excerpt.txt) and cross-checked against the real cc1 dump: tmp/grind/func_80027640/s3/cc1_seam.txt shows the SAME lh/sw pair four words earlier receiving cc1's '#nop' where no label intervenes, and receiving none across '.L75:'. The label placement is forced by the target: j at 0x80027770 encodes 0x08009DE9 -> 0x800277A4, so the merge label sits on the nop's address with lh $v0,0x4($a0) at label-4 (tmp/grind/func_80027640/s3/target_seam.txt).
- result: cc1 provably cannot emit the nop for any C input, because the suppression depends solely on RTL adjacency load -> CODE_LABEL and that adjacency is pinned by the target bytes. The only slack in the code is a NOTE-skipping loop, and NOTEs emit no words, so nothing writable in C can break the adjacency while preserving the bytes.
- verdict: KILLED

## [s3] The honest floor on the stock chassis with the final candidate is still 1.
- mechanism: n/a -- direct measurement of the pinned chassis, required because the driver's dispatch measurement was unavailable.
- probe: python3 tmp/grind/func_80027640/s1/apply.py memory/grind/func_80027640/candidate.c ; & tools/wteng.ps1 main sandbox func_80027640 --disable all
- result: score 1, target_insns 158, build_insns 157, rules_dropped 0, cheat_asm_stripped 35 (tmp/grind/func_80027640/s3/sandbox_s3.txt). src/code6cac_b.c restored to HEAD afterwards.
- verdict: CONFIRMED

## [s4] A machine search over C spellings (decomp-permuter) finds a form that reaches score 0.
- mechanism: the permuter's move set is C-source rewriting (statement/expression permutation,
  declaration reordering, temp introduction, type changes, branch restructuring). If the residual
  were any ordinary codegen-shape divergence -- a register seat, an emission order, a fold that
  did or did not survive -- random C rewriting is the standard way to hit the basin. This session
  tested whether the func_80027640 residual is of that kind.
- probe: built the FIRST faithful stand-alone permuter workspace for this function
  (tmp/grind/func_80027640/s4/perm/), reproducing engine/pipeline.py's code6cac_b command exactly
  (-mel, --expand-lb, the .align 3 -> 2 rodata sed, prologue_fix, multu_pad) and assembling
  target.o from asm/funcs/func_80027640.s (160 words) via s4/mktarget.py. Fidelity proved before
  launching: the reduced TU compiles to 159 words differing from the target ONLY by the single
  missing nop at word 90 = 0x800277A4 (diff = "89a90 > nop"), and the permuter's own base_score is
  100 = exactly one insertion, zero register/reordering penalty. Then two campaigns via
  tools/permuter_campaign.py: campaign 1 on the final candidate chassis (56,275 iterations,
  28.6 min, -j 8, --stop-on-zero) and campaign 2 on a structurally different scalar-join chassis
  (58,381 iterations, 29.9 min) that compiles to 155 words with different register assignment
  throughout. Both harvested with --stop; status confirms 0 live campaigns.
- result: 114,656 iterations, ZERO forms at score 0. Campaign 1 produced no output at all -- from
  a base of 100 the permuter has no scoring move, because every C rewrite it can make leaves the
  one-word insertion in place. Campaign 2 descended 2188 -> 1737 -> 818 -> 750 and then sat flat
  at 750 for its final 18 minutes, never even returning to chassis 1's 100. A fresh cc1 dump taken
  from this session's own reduced TU (tmp/grind/func_80027640/s4/cc1_raw.s:145-149) re-confirms
  the mechanism first-hand and chassis-independently: the lh/sw pair with no intervening label
  gets cc1's "#nop" hint, the pair across the cross-jump merge label ".L8" gets none.
- verdict: KILLED

## [s4] The residual might be a codegen-shape divergence that a different chassis exposes.
- mechanism: if the missing word were, say, a scheduling artifact or a spill, restructuring the
  if/else join (scalars instead of VECTOR member stores inside the arms, single store after the
  join) would move it or expose a neighbouring basin.
- probe: chassis 2 above (tmp/grind/func_80027640/s4/chassis2_body.c, banked as
  rejected/scalar-join-chassis-perm-basin.c) -- compiled and word-diffed against the target before
  seeding, then given a full ~30-minute campaign.
- result: the restructured chassis is strictly worse (155 words vs 160; register assignment
  diverges across the whole body; branch target shifts) and its best permuted descendant is 750,
  7.5x chassis 1's base. The residual does not move, split, or change character under
  restructuring -- consistent with it being an assembler-inserted hazard word rather than any
  compiler-shape artifact.
- verdict: KILLED

- REMAINING FRONTIER after s4: still none on the C axis, and now the machine-search axis is closed
  empirically as well as analytically. The C in candidate.c is final. The only known route to 0
  remains s3's assembler-layer repair, foreclosed for grind sessions by the Judge's binding
  no-build-surface-change constraint. Do not dispatch a second permuter session (grindlib's own
  zero-yield rule applies), and do not dispatch another C-spelling modality against this residual;
  the productive next step is the escalation record once the driver declares exhaustion.

## [s4] A machine search over C spellings (decomp-permuter) finds a form for func_80027640 that reaches score 0.
- mechanism: The permuter's move set is C-source rewriting (statement/expression permutation, declaration reordering, temp introduction, type changes, branch restructuring). If the residual were an ordinary codegen-shape divergence -- a register seat, an emission order, a spill, a fold that did or did not survive -- random C rewriting is the standard way to reach the basin. The probe tests whether func_80027640's one-word residual is of that kind at all.
- probe: Built the FIRST faithful stand-alone permuter workspace for this function (tmp/grind/func_80027640/s4/perm). compile.sh reproduces engine/pipeline.py:c_pipeline_cmd("code6cac_b") exactly -- cpp with the project CPP_DEFS, tools/gcc-2.7.2/build/cc1 with -mel, prologue_fix, maspsx with MASPSX_FLAGS + --expand-lb (code6cac_b is in EXPAND_LB_FILES), the '.align 3 -> .align 2' sed (RODATA_ALIGN2_FILES), multu_pad, as. target.o assembled from asm/funcs/func_80027640.s by s4/mktarget.py = 160 words. base.c is the reduced TU pre-preprocessed with mipsel cpp -P (the permuter preprocesses base.c itself with bare 'cpp -P -nostdinc', which cannot find include/ and kills a naive workspace at launch). FIDELITY PROVED BEFORE LAUNCH: the reduced TU compiles to 159 words differing from the target ONLY by the missing nop at word 90 = 0x800277A4 (diff = '89a90 > nop'), no register or ordering drift. Then two campaigns via tools/permuter_campaign.py launch/wait/harvest --stop, -j 8, --stop-on-zero, --stack-diffs (default).
- result: Campaign 1 (label s4-vector-chassis, seed = the final candidate): base_score 100 -- exactly one insertion under the standard weights, zero register and zero reordering penalty -- 56,275 iterations over 28.6 min across three fresh-seed windows, and ZERO outputs of any score. Not merely no zero-score find: from a base of 100 the permuter has no scoring move at all, because every C rewrite it can make leaves the one-word insertion in place. Campaign 2 (label s4-scalar-join-chassis, a deliberately different basin) 58,381 iterations over 29.9 min, 190 outputs, descending 2188 -> 1737 -> 818 -> 750 and then flat at 750 for its final 18 minutes -- never returning to chassis 1's 100, never 0. 114,656 iterations total, zero score-0 forms. Both campaigns harvested with --stop; 'permuter_campaign.py status' reports 0 live campaigns, 0 stale registry entries.
- verdict: KILLED

## [s4] The residual is a codegen-shape divergence that a structurally different chassis would expose or relocate.
- mechanism: If the missing word were a scheduling artifact, a spill, or a merge that happened to fire, restructuring the if/else join -- plain s32 scalars in both arms with a single pair of tgt.vx/tgt.vz stores after the join, instead of storing into the VECTOR members inside the arms -- would move it, split it, or open a neighbouring basin.
- probe: tmp/grind/func_80027640/s4/chassis2_body.c, compiled and word-diffed against the 160-word target before seeding, then given a full ~30-minute / 58,381-iteration campaign in tmp/grind/func_80027640/s4/perm2. Banked as memory/grind/func_80027640/rejected/scalar-join-chassis-perm-basin.c.
- result: The restructured chassis is strictly worse: 155 words vs 160, register assignment diverging across the whole body (a1/a3, a0/a2, v1 swaps), and a shifted branch target. Its best permuted descendant is 750 -- 7.5x chassis 1's base of 100 -- and it plateaued there. The residual does not move, split, or change character under restructuring, which is what an assembler-inserted hazard word behaves like and what a compiler-shape artifact does not.
- verdict: KILLED

## [s4] The honest floor on the stock chassis with the final candidate applied is still 1 (the driver's dispatch measurement was reported unavailable).
- mechanism: n/a -- direct chassis re-measure, required because the brief's CHASSIS CHECK said 'measurement unavailable' and every banked conclusion is chassis-relative.
- probe: python3 tmp/grind/func_80027640/s1/apply.py memory/grind/func_80027640/candidate.c ; & tools/wteng.ps1 main sandbox func_80027640 --disable all
- result: score 1, target_insns 158, build_insns 157, rules_dropped 0, cheat_asm_stripped 35 (tmp/grind/func_80027640/s4/sandbox_s4.txt). src/code6cac_b.c restored to HEAD (INCLUDE_ASM) afterwards.
- verdict: CONFIRMED

## [s4] s3's compiler-source mechanism (GCC 2.7.2 mips_fill_delay_slot suppresses the load-delay nop hint whenever the next insn is a CODE_LABEL) is chassis-independent and reproduces in a dump this session produced itself.
- mechanism: mips.c:673 mips_fill_delay_slot zeroes dslots_number_nops / mips_load_reg at a CODE_LABEL ('Make sure that we don't put nop's after labels.'); mips.c:4104 final_prescan_insn is the only emitter of '#nop' and is gated on that counter. If true, cc1 emits the hint for a load/store pair with no intervening label and withholds it for an otherwise identical pair behind a label -- in ANY translation unit, not just the full code6cac_b.c context s3 examined.
- probe: Fresh raw cc1 dump of this session's REDUCED TU (a context s3 never compiled): tmp/grind/func_80027640/s4/cc1_raw.s, 21 '#nop' hints in the function; read lines 145-149.
- result: Verbatim: 'lh $2,0($4)' / '#nop' / 'sw $2,16($sp)' / 'lh $2,4($4)' / '.L8:' / 'sw $2,24($sp)'. Two structurally identical load/store pairs four words apart. The first, with no label between load and consumer, gets the '#nop' hint. The second, whose consumer sits behind the cross-jump merge label .L8, gets no hint at all. Same C statement shape, opposite outcome, decided purely by the intervening label -- s3's source reading confirmed first-hand and independent of translation-unit context.
- verdict: CONFIRMED
