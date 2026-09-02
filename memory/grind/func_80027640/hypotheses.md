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
