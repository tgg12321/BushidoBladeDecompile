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
