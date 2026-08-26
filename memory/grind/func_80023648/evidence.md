# Evidence bank — func_80023648

## [s1 2026-08-26 — recon] Baseline, chassis recovery, residual classified RA

- **Function identity:** `func_80023648(u8 *arg0)` in src/code6cac.c, 159 target
  insns. Pre-migration commit `36a7ca33` named it "replay camera frame rate /
  speed". Kind gate `{0x13, 0x1B, 0x30}` on `*(u16*)(arg0+0x6A)`, then a
  flag-bit table lookup (`D_8008EB40` 2x3 s16 table) feeding `func_8001F860`,
  then a clamp/decay block updating `+0x14E` and applying sin/cos
  (`Judge` table) increments to `+0xD8`/`+0xE0`.
- **Chassis recovery (this session):** main carried `INCLUDE_ASM` (asm-until-
  matched). The pre-migration C body was recovered from `git show
  2339fa22~1:src/code6cac.c` (lines 2757-2855) and re-applied to
  src/code6cac.c. All externs it needs already exist (Judge, D_8008EB40 in-file;
  D_8008DA08, D_800A310C in include/code6cac.h; D_800A38BA visible).
- **Honest floor measured THIS session: 30** (`sandbox func_80023648 --disable
  all` → score 30, build_insns 159 == target_insns 159). Matches the
  migration pin. Equal insn counts ⇒ no missing/extra instructions.
- **The 30 retired regfix rules (migration commit `2339fa22`) are ALL
  register-rename substs** — no inserts, no reorders. Rule map by maspsx idx:
  - idx 35–75: our `$3`(v1) → target `$6`(a2) — the table-value region
  - idx 77–95: our `$4`(a0) → `$3`(v1), our `$5`(a1) → `$4`(a0) — abs/clamp region
  - idx 98–139: our `$6`(a2) → `$7`(a3) — post-join arithmetic
  - idx 121–136: our `$5`(a1) → `$6`(a2)
- **Solver classification (object-level, `goal_from_tgt.py classify code6cac
  func_80023648`): FIRST DIVERGENCE = RA.** 30 renamed pairs, 0 skeleton
  diffs. Substitution census: `$v1→$a2 x13, $a2→$a3 x8, $a0→$v1 x7,
  $a1→$a0 x5, $a1→$a2 x3`.
  - CAUTION for successors: `inverse_compose.py classify` (text-stream path
    via `mkasm_honest.sh`) returns a FALSE "IDENTICAL" here — its `.tgt.s` is
    built from current src + rules, and this function has ZERO rules, so
    tgt==hon trivially. Use the OBJECT-LEVEL path (`goal_from_tgt.py
    classify/goal`) for this function, as mkasm_honest.sh's own header says.
- **Root-flip site identified:** first divergence is the table-entry load
  (C: `a2 = row[a1]`). Target: `lh $a2, 0x0($v1)` (asm/funcs/func_80023648.s
  line ~13EDC) — a FRESH register $a2, while the address register $v1 dies.
  Ours: `lh $v1, 0($v1)` — reuses the dying address seat. Target's value then
  dies feeding `$a1` before the `jal func_8001F860`
  (`addu $a1, $a2, $zero` in the bgez delay slot at 13F00).
- **Global-seat anomaly:** across the mid-region target's allocation SKIPS
  $a1 and reaches into $a3; ours packs {v1,a0,a1,a2}. Suggests either an
  extra/longer-lived value occupying a seat early in the original, or
  different allocno priorities (nrefs/live-length) on the index/row/table
  intermediates.
- **Pseudo attribution (extract.py model, `tmp/ra_solver_work/
  func_80023648.model.json`, 18 global pseudos):** `$v1→$a2` ambiguous over 14
  v1-holding pseudos; `$a0→$v1` ambiguous (a0 pseudos [85,129,135,177]);
  `$a1→$a0` ambiguous (a1 pseudos [83,109,110,113,126,165]); **`$a2→$a3` has
  NO global pseudo holding $a2 — it is a LOCAL-alloc quantity** → the local
  backend (`local_extract.py` / `local_alloc.py`) owns that cluster.
- **cc1 -da dumps generated** to `tmp/grind/func_80023648/dumps/` (code6cac.*).
  NOTE: the dump run printed `parse error before 'GameObj'` at src lines
  680/996 (also `cc1(hon) rc=33` in mkasm_honest.sh) yet emitted full dumps;
  the sandbox pipeline builds the same TU cleanly (159/159), so the parse
  errors come from the dump/solver cpp flag set, not the build pipeline —
  treat dump content for functions near those lines with suspicion, but
  func_80023648 (line ~2484) parsed fine (present in .greg at line 25851).
- **No duplicate/sibling leads:** `tmp/duplicates_leads.txt` has no entry for
  this function.
- **Cross-knowledge (main's ledger, evidence.md:341, :959):** func_80023648 is
  one of 12 rule-carrying functions flagged as CANDIDATES for main's reorg
  redundancy-thread-skip / branch-retarget class. **This session's measurement
  says that class is NOT this function's residual**: insn counts are equal and
  the divergence is pure RA renames — no branch/label/delay-slot geometry in
  the diff. The class-candidacy is effectively KILLED for func_80023648
  (it was only ever a "needs per-function bytework" flag, and the bytework
  now shows plain RA).
- Artifacts: `tmp/grind/func_80023648/s1/` (diffseats.sh, ours.txt,
  target.txt, seatdiff.txt — note the naive text diff there is noisy;
  trust the solver census), `tmp/grind/func_80023648/dumps/`,
  `tmp/ra_solver_work/func_80023648.model.json`.

- [s1] Chassis: pre-migration C body recovered from 2339fa22~1 and applied to src/code6cac.c; sandbox --disable all = 30 THIS session, build_insns 159 == target_insns 159.

- [s1] All 30 retired regfix rules (migration commit 2339fa22) are register-rename substs: v1->a2 (idx 35-75), a0->v1 + a1->a0 (idx 77-95), a2->a3 (idx 98-139), a1->a2 (idx 121-136). No inserts/reorders.

- [s1] Object-level solver classify: FIRST DIVERGENCE = RA; substitution census $v1->$a2 x13, $a2->$a3 x8, $a0->$v1 x7, $a1->$a0 x5, $a1->$a2 x3.

- [s1] TRAP for successors: inverse_compose.py classify (text-stream path) returns FALSE 'IDENTICAL' for this function — its .tgt.s is current-src+rules and this function has zero rules; use goal_from_tgt.py classify/goal instead.

- [s1] $a2->$a3 cluster has NO global pseudo holding $a2 — it is a local-alloc quantity; the local backend (local_extract.py/local_alloc.py) owns it.

- [s1] Target's mid-region allocation skips $a1 and reaches $a3 while ours packs {v1,a0,a1,a2} — priority/used-set difference, mechanism-consistent with a single upstream flip cascading.

- [s1] RA model extracted: tmp/ra_solver_work/func_80023648.model.json (18 global pseudos, 68 dispositions).

- [s1] No duplicate/sibling leads in tmp/duplicates_leads.txt for this function.

- [s1] Lookup-region respell axis measured dead: 38/44/43 vs 30 (forms banked in memory/grind/func_80023648/rejected/).

## [s2 2026-08-26 — structural] Root flip localised to global.c find_reg pass 0; structural axis KILLED

- **Chassis re-measured this session: floor 30**, 159/159 insns, with s1's
  candidate body re-applied to src/code6cac.c. `src/code6cac.c` was restored to
  `INCLUDE_ASM("asm/funcs", func_80023648);` before the session ended (asm-until-
  matched); the body lives only in `memory/grind/func_80023648/candidate.c`.

- **Full 159-line aligned ours-vs-target listing built** (artifact
  `tmp/grind/func_80023648/s2/diffseats.sh` + `ours.txt`/`target.txt`; note our
  objdump listing carries ONE leading header line, so the correct alignment is
  `ours[1:160]` against `target[0:159]` — s1's naive `diff` was noisy for
  exactly this reason). Every one of the 159 slots is the same mnemonic with the
  same operand shape; the ONLY differences are register names. Value->seat map:

  | value | ours | target |
  |---|---|---|
  | D_8008EB40 table entry (C `a2 = row[a1]`) | `$v1` | `$a2` |
  | `abs_val` (clamped `*(s16*)(arg0+0x150)`) | `$v1` | `$a2` |
  | `div16` (`*(s16*)(arg0+0x1A)` >>4 chain) | `$a0` | `$v1` |
  | `sub_result` (`0x14E - abs_val`, and the 0x1A re-read) | `$a1` | `$a0` |
  | mflo temps (`mult_res`, `speed_prod`) | `$a2` | `$a3` |
  | `speed` (`speed_prod >> 12`) | `$a1` | `$a2` |
  | `new_14e`, `limit` | `$v1`,`$a0` | `$v1`,`$a0` (SAME) |

  Reading: exactly TWO values want to move up to `$a2` (the table entry and
  `abs_val`); everything else in the table is the downstream cascade of those
  two seats being vacated/occupied. Our build never uses `$a3` at all; target's
  `$a3` is just "the mflo temp displaced by `speed` taking `$a2`".

- **Pseudo identification (from the cc1 `.lreg` RTL, artifact
  `tmp/grind/func_80023648/s2/lreg_fn.txt`)** — this replaces s1's "ambiguous
  among 14 v1-holders" attribution with exact ids:
  - **86** = the table entry (`insn 83: (set (reg 86) (sign_extend (mem (reg 97))))`)
  - **122** = `abs_val` (`insn 171`, negated at 177, clamped at 187)
  - **127** = `div16`, **126** = `sub_result`, **81** = `bits`, **75** = `kind`,
    **83**/**85** = the C `a1`/`a0` flag indices, **74** = `&D_8008EB40`,
    **97** = the final row-cell address, **137** = `new_14e`, **131** = `limit`,
    **165** = `speed`, **130**/**158**/**176**/**192** = the mult results.

- **global.c allocation order recovered from the `.greg` dump header:**
  `;; 18 regs to allocate: 127 122 85 81 72 130 176 75 86 126 153 83 110 131 192 107 158 173`.
  This is `allocno_compare`'s priority order, `floor_log2(n_refs)*n_refs /
  live_length`; recomputing it from the `.lreg` "used N times across M insns"
  lines reproduces the printed order exactly (127: 14/8=1.75; 122: 14/9=1.556;
  85: 10/7=1.429; 81: 12/11=1.091; 72: 120/112=1.07; 130/176: 2/2=1.0;
  75/86/126: 10/11=0.909 tied, broken by allocno index; 153: 3/4=0.75;
  83: 8/15=0.533; 110/131/192: 0.5; 107/158: 0.333; 173: 2/20=0.1).
  Successors can now predict the effect of any refs/live-length edit
  ARITHMETICALLY before spending a build.

- **ROOT-FLIP MECHANISM MEASURED, NOT HYPOTHESISED** (instrumented cc1
  `tools/gcc-2.7.2/cc1` + its `BB2_FINDREG_DEBUG=<pseudo>` hook; harness
  `tmp/grind/func_80023648/s2/findreg.sh`, logs `findreg_86.log` / `findreg_122.log`):

      FINDREGDBG func=func_80023648 pseudo=86 alt=0 acc=0 retry=0
        conflicts: 2 4 5 16 29
        someone_prefers: <empty>
        used_so_far: 0 1 2 3 4 5 6 7 8 ...
        pass0_used: 0 1 2 4 5 16 17..23 26..31
        own_full_prefs: <empty>

      FINDREGDBG func=func_80023648 pseudo=122 alt=0 acc=0 retry=0
        conflicts: 2 4 29
        someone_prefers: <empty>
        pass0_used: 0 1 2 4 16 17..23 26..31
        own_full_prefs: 3

  `find_reg` pass 0 excludes `used1 | ~regs_used_so_far | regs_someone_prefers`
  and then takes the LOWEST-numbered survivor (MIPS defines no `REG_ALLOC_ORDER`
  in `tools/gcc-2.7.2/config/mips/mips.h`, so the scan is plain ascending 0..31).
  - For **86**: 3 (`$v1`) is in NEITHER `hard_reg_conflicts` NOR
    `regs_someone_prefers`, and 3 IS in `regs_used_so_far`, so pass 0 returns 3.
    6 (`$a2`) is already in `regs_used_so_far`, so **the instant 3 enters either
    exclusion set, pass 0 returns 6 — target's seat — with no other change.**
    That is a one-bit gap, fully characterised.
  - For **122**: BOTH 3 (`$v1`) and 5 (`$a1`) would have to be excluded (it does
    not even conflict with `$a1`), and it additionally carries its OWN
    `hard_reg_full_preferences = {3}`. `abs_val` is therefore a STRICTLY HARDER
    seat than the table entry — a successor should attack 86 first.

- **`regs_someone_prefers` is built in `prune_preferences` as the union of
  `hard_reg_full_preferences` over LOWER-priority CONFLICTING allocnos.** 86's
  conflicting allocnos are 72 (`$s0`), 107 and 110, and the measurement shows
  that union is empty. So the two (and only two) honest levers on 86 are:
  (a) make some `$v1`-resident value LIVE ACROSS insn 83, or
  (b) give 107 or 110 a `hard_reg_full_preferences` bit for `$v1`.
  Both are DATA-FLOW facts, not spelling facts.

- **Why the structural modality cannot reach either lever (the kill):** the
  159/159 insn multiset pins the data flow. The `$v1` occupants around insn 83
  are pseudo 74 (`&D_8008EB40`) and pseudo 97 (the row-cell address), both
  local-alloc'd to `$v1`, and both provably dead AT insn 83 in target too —
  target's idx 27-34 address chain is register-for-register IDENTICAL to ours
  (`lui $v1 / addiu $v1 / sll $v0,$a0,1 / addu $v0,$v0,$a0 / sll $v0,$v0,1 /
  addu $v0,$v0,$v1 / sll $v1,$a1,1 / addu $v1,$v1,$v0`), and target's `$v1` is
  then untouched until idx 80. `bits` (81, `$v1`) dies at idx 23 in both. So no
  reordering / rescoping / renaming of the EXISTING values can put a live `$v1`
  value across insn 83 without adding an instruction, and adding one breaks the
  159/159 multiset.

- **Eight byte-neutral structural respellings measured, ALL exactly 30** (i.e.
  zero effect on allocation): declmove, sib (the matched sibling
  `func_800233AC`'s `s16 *tbl = &D_8008EB40;` nested-block idiom), abs_outer,
  merge_a2abs, flatten, declorder, scope_lookup, clamp_gt. Two non-neutral
  forms banked to rejected/: declinit -> 38, `s16 a2` narrowing -> 39 (161/159).

- **merge_a2abs is the informative null:** collapsing the table entry and
  `abs_val` into ONE C local — the shape suggested by both landing in `$a2` in
  target — changed NOTHING (still 30), because our build ALREADY co-seats both
  in `$v1`. The two-values-one-seat observation is therefore NOT evidence for a
  reused C variable in the original.

- **Matched-sibling precedent located:** `func_800233AC` (src/code6cac.c:2387,
  zero rules, same TU, same `D_8008EB40` 2x3 table, same `bits` flag-index
  decode) spells the lookup `s16 *tbl = &D_8008EB40; row = tbl + a0_idx * 3;
  a1_val = row[a1_idx];` inside a nested block. Adopting that exact idiom here
  (variant `sib`) is byte-neutral at 30 — so the sibling's shape is NOT the
  differentiator, and successors should stop treating it as an untried lead.

- Artifacts: `tmp/grind/func_80023648/s2/` (diffseats.sh, ours.txt, target.txt,
  lreg_fn.txt, findreg.sh, findreg_86.log, findreg_122.log, sweep.ps1, apply.py,
  var/*.c), `tmp/grind/func_80023648/dumps/` (regenerated .lreg/.greg/etc).

- [s2] Floor unchanged at 30. Eight byte-neutral structural respellings all measured exactly 30; the structural axis is KILLED with a mechanism-level reason, not just by enumeration.

- [s2] Exact pseudo ids recovered: 86 = table entry, 122 = abs_val, 127 = div16, 126 = sub_result, 81 = bits, 75 = kind, 74 = &D_8008EB40, 97 = row-cell address, 165 = speed. Replaces s1's ambiguous 14-v1-holder attribution.

- [s2] global.c priority order for the 18 allocnos is `127 122 85 81 72 130 176 75 86 126 153 83 110 131 192 107 158 173` and is reproduced exactly by floor_log2(n_refs)*n_refs/live_length from the .lreg stats — refs/live-length edits can now be evaluated arithmetically before a build.

- [s2] MEASURED (BB2_FINDREG_DEBUG, not hypothesised): pseudo 86 pass-0 state is conflicts={2,4,5,16,29}, someone_prefers={}, own_full_prefs={}, and 6($a2) is already in regs_used_so_far. $v1(3) is excluded by nothing, so pass 0 takes it. Adding 3 to hard_reg_conflicts[86] OR regs_someone_prefers[86] yields $a2 with no other change.

- [s2] MEASURED: pseudo 122 (abs_val) needs BOTH 3 and 5 excluded (it does not conflict with $a1) and carries its own hard_reg_full_preferences={3}. abs_val is a strictly harder seat than the table entry; attack 86 first.

- [s2] MIPS defines no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so find_reg's scan is plain ascending 0..31 — "lowest free hard reg wins" is exact here, and $a2 is the next free after {2,4,5} for pseudo 86.

- [s2] The matched sibling func_800233AC (src/code6cac.c:2387, zero rules, same table) uses the nested `s16 *tbl = &D_8008EB40;` idiom; adopting it verbatim is byte-neutral at 30. Not the differentiator — stop treating it as an untried lead.

- [s2] merge_a2abs null result: collapsing the table entry and abs_val into ONE C local scored 30 (no change), because our build already co-seats both in $v1. Two-values-one-seat in target is NOT evidence of a reused C variable in the original.

- [s2] The instrumented cc1 harness for this TU is tmp/grind/func_80023648/s2/findreg.sh (`bash tools/wsl.sh 'bash tmp/grind/func_80023648/s2/findreg.sh <pseudo>'`); it returns rc 33 from the pre-existing `parse error before GameObj` at src/code6cac.c:680/996 (a dump-only cpp flag-set artifact, see s1) yet still emits the FINDREGDBG lines for func_80023648. rc 33 is EXPECTED — do not treat it as a broken harness.

- [s2] Chassis re-measured this session with s1's candidate body applied: sandbox --disable all = 30, build_insns 159 == target_insns 159. src/code6cac.c was restored to INCLUDE_ASM("asm/funcs", func_80023648); before the session ended (asm-until-matched); the body lives only in memory/grind/func_80023648/candidate.c.

- [s2] Full 159-line aligned ours-vs-target listing built. Correct alignment is ours[1:160] vs target[0:159] (our objdump listing carries one leading header line) — s1's naive diff was noisy for exactly this reason. Every slot is the same mnemonic with the same operand shape; only register names differ.

- [s2] Value->seat map: table entry ours $v1 / target $a2; abs_val ours $v1 / target $a2; div16 ours $a0 / target $v1; sub_result ours $a1 / target $a0; mflo temps ours $a2 / target $a3; speed ours $a1 / target $a2; new_14e and limit are the SAME in both. Exactly two values want to move up to $a2; the rest is cascade. Our build never uses $a3 at all.

- [s2] Exact pseudo ids recovered from the cc1 .lreg RTL, replacing s1's ambiguous 14-v1-holder attribution: 86 = table entry (insn 83), 122 = abs_val (insn 171), 127 = div16, 126 = sub_result, 81 = bits, 75 = kind, 74 = &D_8008EB40, 97 = row-cell address, 137 = new_14e, 131 = limit, 165 = speed, 130/158/176/192 = mult results.

- [s2] global.c allocation order recovered from the .greg header: `;; 18 regs to allocate: 127 122 85 81 72 130 176 75 86 126 153 83 110 131 192 107 158 173`, and it is reproduced exactly by allocno_compare's floor_log2(n_refs)*n_refs/live_length using the .lreg 'used N times across M insns' stats (127: 1.75, 122: 1.556, 85: 1.429, 81: 1.091, 72: 1.07, 130/176: 1.0, 75/86/126: 0.909 tied and broken by allocno index, 153: 0.75, 83: 0.533, 110/131/192: 0.5, 107/158: 0.333, 173: 0.1). Successors can evaluate any refs/live-length edit arithmetically before spending a build.

- [s2] MEASURED with the instrumented cc1's BB2_FINDREG_DEBUG hook (not hypothesised): pseudo 86 pass-0 state is conflicts={2,4,5,16,29}, someone_prefers={}, own_full_prefs={}, used_so_far includes both 3 and 6. Adding hard reg 3 to hard_reg_conflicts[86] OR regs_someone_prefers[86] yields $a2 with no other change.

- [s2] MEASURED: pseudo 122 (abs_val) has conflicts={2,4,29}, someone_prefers={}, own_full_prefs={3}. It needs BOTH $v1 and $a1 excluded plus its own preference removed — a strictly harder seat than the table entry. Attack 86 first.

- [s2] MEASURED: find_reg is never called for pseudo 127 in this function (BB2_FINDREG_DEBUG=127 produced no record for func_80023648). Its $a0 is already assigned when global_alloc's find_reg loop runs. Corollary for successors: the .greg `;; N regs to allocate:` list is NOT the set of pseudos that actually reach pass 0 — verify with the debug hook, never assume.

- [s2] MIPS defines no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so find_reg's scan is plain ascending 0..31; 'lowest free hard reg wins' is exact here, and $a2 is the next free after {2,4,5} for pseudo 86.

- [s2] regs_someone_prefers is built in prune_preferences as the union of hard_reg_full_preferences over LOWER-priority CONFLICTING allocnos. 86's conflicting allocnos are 72($s0), 107 and 110, and that union measures empty — so the only two honest levers on 86 are (a) a $v1-resident value live across insn 83, or (b) a $v1 full-preference on allocno 107 or 110. Both are data-flow facts, not spelling facts.

- [s2] Why structural cannot reach those levers: target's idx 27-34 address chain is register-for-register IDENTICAL to ours (lui $v1 / addiu $v1 / sll $v0,$a0,1 / addu $v0,$v0,$a0 / sll $v0,$v0,1 / addu $v0,$v0,$v1 / sll $v1,$a1,1 / addu $v1,$v1,$v0), target's $v1 is then untouched until idx 80, and bits(81,$v1) dies at idx 23 in both. No reordering or rescoping of existing values can put a live $v1 value across insn 83 without adding an instruction, which would break the 159/159 multiset.

- [s2] Eight byte-neutral structural respellings measured at exactly 30 (declmove, sib, abs_outer, merge_a2abs, flatten, declorder, scope_lookup, clamp_gt); two non-neutral ones banked to rejected/: declinit -> 38, s16-narrowed table value -> 39 at 161/159 insns.

- [s2] Matched-sibling precedent located and measured: func_800233AC (src/code6cac.c:2387, zero rules, same TU, same D_8008EB40 2x3 table, same bits flag decode) spells the lookup `s16 *tbl = &D_8008EB40; row = tbl + a0_idx * 3; a1_val = row[a1_idx];` inside a nested block; transplanting it verbatim is byte-neutral at 30.

- [s2] Reusable harness for successors: tmp/grind/func_80023648/s2/findreg.sh <pseudo>, run as `bash tools/wsl.sh 'bash tmp/grind/func_80023648/s2/findreg.sh 86'`. It returns rc 33 from the pre-existing `parse error before GameObj` at src/code6cac.c:680/996 (a dump-only cpp flag-set artifact, see s1) yet still emits the FINDREGDBG lines — rc 33 is EXPECTED, not a broken harness. It requires the candidate body to be applied to src first (tmp/grind/func_80023648/s2/apply.py).

- [s2] Reusable sweep harness: tmp/grind/func_80023648/s2/sweep.ps1 -Names <variant...> applies each tmp/grind/func_80023648/s2/var/<name>.c to src and prints score + build_insns, about 30s per variant.

## [s3 2026-08-26 — structural] ROOT RE-LOCATED: the first divergence is allocno ord=0 (pseudo 129, `div16`), and its gap is a PREFERENCE bit, not a conflict bit

- **Chassis re-measured this session: floor 30**, 159/159 insns, with the s1/s2
  candidate body re-applied to src/code6cac.c via
  `tmp/grind/func_80023648/s3/apply.py`. src restored to
  `INCLUDE_ASM("asm/funcs", func_80023648);` before the session ended.

- **NEW INSTRUMENTATION USED: `BB2_ALLOC_DEBUG=1`.** The instrumented cc1
  (`tools/gcc-2.7.2/cc1`, global.c:601-618) has a second, previously unused hook
  that dumps, for EVERY allocno in allocation order:
  `ord / pseudo / hardreg / nrefs / livelen / priority`, plus the initial
  `regs_used_so_far` seed. Harness: `tmp/grind/func_80023648/s3/allocdbg.sh <pseudo>`
  (a one-line env addition to s2's findreg.sh; the FINDREGDBG output lands in the
  same `findreg_<pseudo>.log`). **This is strictly better than reading the `.greg`
  `;; N regs to allocate:` header and successors should use it by default.**

- **s2's allocno table is SUPERSEDED and was WRONG in its pseudo ids.** s2 read
  the `.greg` header list `127 122 85 81 72 130 176 75 86 126 153 83 110 131 192
  107 158 173`. The measured ALLOCDBG order for this function is:

      ord pseudo hardreg nrefs livelen pri
       0   129     4($a0)   7   8   17500
       1   122     3($v1)   7   9   15555
       2    85     4($a0)   5   7   14285
       3    81     3($v1)   6  11   10909
       4    72    16($s0)  30 112   10714
       5   134    65(LO)    2   2   10000
       6   176    65(LO)    2   2   10000
       7    75     3($v1)   5  11    9090
       8    86     3($v1)   5  11    9090
       9   126     5($a1)   5  11    9090
      10   153     3($v1)   3   4    7500
      11    83     5($a1)   4  15    5333
      12   110     5($a1)   3   6    5000
      13   135     4($a0)   3   6    5000
      14   192    65(LO)    2   4    5000
      15   107     2($v0)   2   6    3333
      16   158    65(LO)    2   6    3333
      17   173    -1 (no hard reg)  2  20  1000

  Differences that matter: `div16` is pseudo **129**, not 127 (127 is not an
  allocno at all — that is the real reason s2's `BB2_FINDREG_DEBUG=127` produced
  "NO HIT", re-confirmed this session; s2 mis-read that null as "127 never
  reaches pass 0"). `130` is really `134`; `131` is really `135`. The mflo temps
  (134/176/192/158) sit in hard reg **65 = LO**, not in a GPR — s2's
  "mflo temps ours $a2 / target $a3" seat-map row describes the *post-mflo copy*,
  not these allocnos. Pseudo **173 gets NO hard register** (hardreg = -1).

- **THE ROOT DIVERGENCE IS ord=0, pseudo 129 (`div16` = `*(s16 *)(arg0 + 0x1A)`),
  NOT pseudo 86.** Ours seats it in `$a0`; target seats `div16` in `$v1`
  (s2's value->seat map row "div16 ours $a0 / target $v1" — that row is correct).
  129 is the FIRST allocno global.c allocates, so every other seat in this
  function is decided downstream of it.

- **MEASURED pass-0 state of 129 (`BB2_FINDREG_DEBUG=129`, log
  `tmp/grind/func_80023648/s3/findreg_129.log`):**

      FINDREGDBG func=func_80023648 pseudo=129 alt=0 acc=0 retry=0
        conflicts: 2 29
        someone_prefers: 3
        used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 24 25 26 27 28 29 31
        pass0_used: 0 1 2 3 16..23 26..31
        own_copy_prefs: <empty>
        own_full_prefs: <empty>

  `$v1` (3) is **NOT in `hard_reg_conflicts[129]`**. It is excluded from pass 0
  **only by `regs_someone_prefers[129]`**. Remove that one bit and pass 0's
  ascending scan returns 3 = `$v1` = target's seat, with nothing else changed.
  This is a strictly BETTER lever than s2's pseudo-86 gap: `regs_someone_prefers`
  is built from `hard_reg_full_preferences` of *other* allocnos, and those are
  written by `set_preference()` off the COPY STRUCTURE of the RTL — which C-level
  spelling demonstrably does move (see the swapadd measurement below). s2's
  86 gap needed a `hard_reg_conflicts` bit, which the 159-insn multiset pins.

- **Who is putting the 3 there:** `prune_preferences` (global.c:882-931) unions
  `hard_reg_full_preferences[j]` over every LOWER-priority allocno j that
  CONFLICTS with 129. The only allocno measured to carry
  `own_full_prefs = {3}` in this function is **122 (`abs_val`)** (s2's
  measurement, still current), and 122 is ord=1, i.e. exactly one step lower in
  priority than 129. So the prime suspect chain is **122's `$v1` full-preference
  -> `regs_someone_prefers[129]` -> 129 loses `$v1` -> takes `$a0` -> the whole
  downstream cascade.** It is NOT yet proven that CONFLICTP(129,122) holds, nor
  that 122 is the only contributor — that is the next session's first
  measurement.

- **Corollary that rewrites s2's frontier:** s2's H6 ("attack pseudo 86 and only
  86") is now the WRONG target. 86 is ord=8, eight allocations downstream of the
  root, and its `$v1` exclusion would have to come from a conflict bit, which s2
  itself proved the multiset forbids and which s3's `andcond` diagnostic
  independently kills. Attack **129** instead, via **122's preference**, which is
  a preference-structure lever.

- **Structural probes measured this session (all at 159/159 insns):**

  | variant | what | score |
  |---|---|---|
  | `swapadd`  | `a2 / 4 + *(s16 *)(arg0 + 0x1CA)` (call-arg operand order swapped) | **33** |
  | `swapelse` | `a2 + *(s16 *)(arg0 + 0x1D8)` (else-arm operand order swapped) | 30 (neutral) |
  | `swapboth` | both of the above | 33 |
  | `andcond`  | `(D_800A38BA != 0) & (*(s16 *)(arg0+6) == 0)` — non-short-circuit | **47** |
  | `condswap` | `*(s16 *)(arg0+6) == 0 && D_800A38BA != 0` (condition order) | 36 |

  All banked to `memory/grind/func_80023648/rejected/s3-*.c`.

- **`swapadd` is the load-bearing POSITIVE result, not just a rejection.** It
  scores 33 at an unchanged 159/159 insn count, and the disassembly shows the
  `lh $v0, 0x1CA($s0)` load MOVED (from before the `bgez` to after it) and seats
  changed. Mechanism: `set_preference` (global.c:1671) unwraps an 'e'-format
  SET_SRC exactly once, so for `(set (reg 5 a1) (plus (reg X) (reg Y)))` it is
  **X — the FIRST plus operand — that receives the `$a1` preference**. Swapping
  the C operand order re-targets that preference bit to Y. **So C-level operand
  order IS a live, measurable lever on `hard_reg_full_preferences`, hence on
  `regs_someone_prefers`, hence on pass-0 seats.** The residual is NOT
  preference-frozen — s2's "spelling cannot reach the allocator" conclusion is
  true only of the CONFLICT bits and must not be generalised to preference bits.

- **`andcond` is the decisive diagnostic for the 86 conflict theory (KILL).**
  Making the two guard conditions non-short-circuit puts BOTH guard values live
  simultaneously across the table-entry load (asm: `sltu $v1,$zero,$v1 /
  sltiu $v0,$v0,1 / and $v1,$v1,$v0` before `lh $v0,0($a0)`). The table entry
  then took **`$v0`**, not `$a2`. This confirms find_reg pass 0 is pure
  lowest-free-survivor: adding live values just walks the entry down the
  ascending list. To land `$a2` (6) by conflicts alone, 2,3,4,5 must ALL be
  excluded from 86 — an unreachable four-bit ask inside a fixed 159-insn
  multiset. **Lever (A) from s2 (a `$v1`-live value across insn 83) is KILLED
  as a route to `$a2`.**

- **Three further global.c mechanism facts established by reading the source
  (`tools/gcc-2.7.2/global.c`), each of which closes an axis:**
  1. **`regs_used_so_far` is seeded with every `call_used_regs[i]`** (global.c:
     363-367), so `$v0`-`$a3`/`$t0`-`$t9` are ALWAYS in it before the first
     allocation. Therefore *no* allocno-priority reordering can make pass 0 skip
     `$v1` via the "never allocate a register for the first time in pass 0" rule.
     **Priority-reordering as a route to freeing `$v1` is KILLED.**
  2. **After pass 0/1 pick `best_reg`, find_reg OVERRIDES it** with any free
     same-class bit in `hard_reg_copy_preferences[allocno]`, then in
     `hard_reg_preferences[allocno]` (global.c:1086-1160). So a pseudo with an
     `$a2` copy-preference would get `$a2` regardless of the ascending scan.
     For 86 this is unreachable: `$a2` never appears as a hard reg or a
     local-alloc'd seat paired with 86, because this function's only call,
     `func_8001F860`, takes two arguments, so `$a2` is not an argument register
     in this frame.
  3. **107 and 110 can never seat `$v1`**: both already carry 3 in
     `hard_reg_conflicts` (measured this session — logs
     `s3/findreg_107.log`, `s3/findreg_110.log`), and 110 carries
     `own_full_prefs = {5}` while 107's `{5}` was pruned away by its own `$a1`
     conflict. **s2's H4 (route `regs_someone_prefers[86]` through 107/110's
     local seats) is KILLED.**

- Artifacts: `tmp/grind/func_80023648/s3/` (apply.py, sweep.ps1, diffseats.sh,
  allocdbg.sh, findreg.sh, findreg_86.log — which now also carries the ALLOCDBG
  table — findreg_107.log, findreg_110.log, findreg_129.log, ours.txt,
  target.txt, seatdiff.txt, var/*.c).

- [s3] Chassis re-measured: sandbox --disable all = 30, 159/159, with candidate.c applied. src restored to INCLUDE_ASM before session end.

- [s3] NEW TOOL: BB2_ALLOC_DEBUG=1 (global.c:601-618) dumps ord/pseudo/hardreg/nrefs/livelen/priority for every allocno plus the regs_used_so_far seed. Harness tmp/grind/func_80023648/s3/allocdbg.sh. Use this instead of reading the .greg header — the header list is not the allocation order and its pseudo ids misled s2.

- [s3] s2's allocno table is SUPERSEDED: div16 is pseudo 129 (not 127; 127 is not an allocno at all, which is the real reason BB2_FINDREG_DEBUG=127 shows NO HIT), 130->134, 131->135; the mflo temps 134/176/192/158 live in hard reg 65 = LO, not in a GPR; pseudo 173 gets no hard register at all.

- [s3] ROOT RE-LOCATED: the first-allocated allocno (ord=0) is pseudo 129 = div16 = *(s16*)(arg0+0x1A). Ours seats it $a0; target seats div16 in $v1. Every other seat in the function is decided downstream of it. Pseudo 86 (ord=8) is cascade, not root.

- [s3] MEASURED (BB2_FINDREG_DEBUG=129): conflicts={2,29}, someone_prefers={3}, own prefs empty, pass0_used={0,1,2,3,16-23,26-31}. $v1 is excluded ONLY by regs_someone_prefers — a PREFERENCE bit, not a conflict bit. Clear that one bit and pass 0 returns $v1, target's seat, with nothing else changed.

- [s3] The only allocno in this function measured to carry own_full_prefs={3} is 122 (abs_val), at ord=1 — exactly one step below 129. Prime suspect chain: 122's $v1 full-preference -> regs_someone_prefers[129] -> 129 loses $v1. NOT yet proven that CONFLICTP(129,122) holds or that 122 is the only contributor; that is the next measurement.

- [s3] MEASURED POSITIVE LEVER: swapping the call-arg operand order (a2/4 + *(s16*)(arg0+0x1CA)) moved the score to 33 at an unchanged 159/159. Mechanism: set_preference (global.c:1671) unwraps an 'e'-format SET_SRC exactly once, so in `(set (reg 5 a1) (plus X Y))` it is X, the FIRST operand, that gets the $a1 preference; swapping the C operand order retargets it. C operand order IS a live lever on hard_reg_full_preferences and therefore on regs_someone_prefers. The residual is NOT preference-frozen.

- [s3] KILLED (andcond diagnostic, 159/159, score 47): making both guards non-short-circuit puts two values live across the table-entry load; the entry then took $v0, not $a2. find_reg pass 0 is pure lowest-free-survivor, so landing $a2 by conflicts alone needs 2,3,4,5 all excluded from 86 — unreachable in a fixed 159-insn multiset. s2's lever (A) is dead as a route to $a2.

- [s3] KILLED: allocno-priority reordering cannot free $v1 for a pass-0 skip. regs_used_so_far is seeded with every call_used_regs[i] (global.c:363-367), so $v0-$a3/$t0-$t9 are in it before the first allocation.

- [s3] KILLED (s2's H4): 107 and 110 can never seat $v1 — both already carry 3 in hard_reg_conflicts (measured, s3/findreg_107.log and s3/findreg_110.log), 110 carries own_full_prefs={5}, and 107's {5} was pruned away by its own $a1 conflict.

- [s3] MECHANISM: after pass 0/1 pick best_reg, find_reg OVERRIDES it with any free same-class bit in hard_reg_copy_preferences[allocno], then hard_reg_preferences[allocno] (global.c:1086-1160). Unreachable for 86 here: $a2 never appears as a hard reg or a local-alloc seat paired with 86, because this function's only call (func_8001F860) takes two arguments so $a2 is not an argument register in this frame.

- [s3] Structural probes, all 159/159: swapadd 33, swapelse 30 (neutral), swapboth 33, andcond 47, condswap 36. Banked to memory/grind/func_80023648/rejected/s3-*.c.

- [s3] Chassis re-measured this session with the s1/s2 candidate body applied: sandbox func_80023648 --disable all = 30, build_insns 159 == target_insns 159. src/code6cac.c restored to INCLUDE_ASM("asm/funcs", func_80023648); before the session ended.

- [s3] NEW TOOL for all successors: BB2_ALLOC_DEBUG=1 (instrumented cc1, global.c:601-618) dumps ord / pseudo / hardreg / nrefs / livelen / priority for EVERY allocno in allocation order, plus the initial regs_used_so_far seed. Harness: tmp/grind/func_80023648/s3/allocdbg.sh <pseudo>. Use it instead of reading the .greg ';; N regs to allocate:' header - that header is not the allocation order and its pseudo ids misled s2 into a false KILL.

- [s3] Measured allocation order and seats for func_80023648: ord0=129 $a0, ord1=122 $v1, ord2=85 $a0, ord3=81 $v1, ord4=72 $s0, ord5=134 LO, ord6=176 LO, ord7=75 $v1, ord8=86 $v1, ord9=126 $a1, ord10=153 $v1, ord11=83 $a1, ord12=110 $a1, ord13=135 $a0, ord14=192 LO, ord15=107 $v0, ord16=158 LO, ord17=173 no hard reg.

- [s3] ROOT RE-LOCATED: the first-allocated allocno is pseudo 129 = div16 = *(s16 *)(arg0 + 0x1A). Ours seats it $a0; target seats div16 in $v1. Pseudo 86 (the table entry, ord=8) is downstream cascade, not the root.

- [s3] MEASURED (BB2_FINDREG_DEBUG=129): conflicts={2,29}, someone_prefers={3}, own prefs empty. $v1 is excluded from 129's pass 0 ONLY by regs_someone_prefers - a preference bit, not a conflict bit - so clearing it returns target's seat with nothing else changed.

- [s3] The only allocno in this function measured to carry own_full_prefs={3} is 122 (abs_val), at ord=1, exactly one step below 129. Prime suspect chain for the frontier: 122's $v1 full-preference -> regs_someone_prefers[129] -> 129 loses $v1 -> takes $a0 -> whole cascade. CONFLICTP(129,122) is NOT yet proven and must be measured before any C is spent.

- [s3] MEASURED POSITIVE LEVER: swapping the call-arg operand order (a2 / 4 + *(s16 *)(arg0 + 0x1CA)) moved the score to 33 at an unchanged 159/159, with a visibly different schedule and seats. C operand order retargets set_preference's full-preference bit (global.c:1671 unwraps an 'e'-format SET_SRC exactly once, so the FIRST plus operand gets the bit). The residual is NOT preference-frozen.

- [s3] KILLED (andcond diagnostic, 159/159, score 47): forcing both guards live across the table-entry load made the entry take $v0, not $a2. find_reg pass 0 is pure lowest-free-survivor, so landing $a2 by conflicts alone needs 2,3,4,5 all excluded from 86 - unreachable in a fixed 159-insn multiset. s2's lever (A) is dead.

- [s3] KILLED: allocno-priority reordering cannot free $v1 for a pass-0 skip - regs_used_so_far is seeded with every call_used_regs[i] (global.c:363-367), so $v0-$a3/$t0-$t9 are in it before the first allocation.

- [s3] KILLED (s2's H4): 107 and 110 can never seat $v1 - both already carry hard reg 3 in hard_reg_conflicts (measured), 110 carries own_full_prefs={5}, and 107's {5} was pruned away by its own $a1 conflict.

- [s3] MECHANISM (newly established, useful project-wide): after pass 0/1 pick best_reg, find_reg OVERRIDES it with any free same-class bit in hard_reg_copy_preferences[allocno], then in hard_reg_preferences[allocno] (global.c:1086-1160). Unreachable for 86 here, because this function's only call (func_8001F860) takes two arguments, so $a2 never appears as a hard reg or a local-alloc seat paired with 86.

- [s3] Structural probes measured this session, all at 159/159 insns: swapadd 33, swapelse 30 (byte-neutral), swapboth 33, andcond 47, condswap 36. Banked to memory/grind/func_80023648/rejected/s3-swapadd-operand-order-33.c, s3-andcond-nonshortcircuit-47.c, s3-condswap-36.c.

- [s3] candidate.c is unchanged as code (still the 30-floor body) but its header comment now carries the s3 correction so the next session does not act on s2's superseded 'attack pseudo 86' conclusion.

## s4 (permuter, 2026-08-26)
- Chassis at session start, re-measured with the s3 candidate applied: score 30, target_insns 159, build_insns 159. Matches the ledger.
- A standalone permuter workspace for this function is chassis-faithful and cheap to rebuild: tmp/perm_23648_s4/{base.c,compile.sh,settings.toml,target.o}. compile.sh reproduces the CURRENT code6cac pipeline (cpp -> cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w **-mel** -> prologue_fix -> maspsx 2.34 -> multu_pad); note the older tmp/perm_22F34 workspace in this repo is STALE — it lacks -mel and still pipes through the retired tools/fix_lwl.py. base.c needs only s8/u8/s16/u16/s32/u32 typedefs and six externs (D_8008DA08, D_800A310C, D_800A38BA, D_8008EB40, Judge, func_8001F860). Validation before launch: base.o vs target.o = 159/159 insns, 30 mismatching lines — identical to the sandbox residual.
- FLOOR MOVED 30 -> 15 this session, at an unchanged 159/159, on two ordinary-C respellings found by the permuter and hand-verified one at a time:
  * `div16` local reused to carry `(s16)new_14e` for the `limit <` comparison: 30 -> 22.
  * `ent = &row[a1]; a2 = *ent;` (naming the table-element address) stacked on top: 22 -> 15.
- Permuter campaign telemetry: vanilla base_score 180, 14 finds in ~12 min (best 120); varA reseed base_score 120, 10 finds in ~21 min (best 80); varL reseed base_score 95, 17032 iterations, ZERO finds. The dry third campaign is the fresh-seed stopping signal — a plain reseed off the 15-floor body will not pay.
- The permuter's own weighted score tracks the sandbox score directionally but NOT proportionally, and its best find is not the best sandbox form: vanilla's output-120-1 carried TWO deltas, only one of which (div16 reuse) helped; the other (a pointer local for the tail 0x14E store) was exactly neutral at 30 and is a dead construct. Always decompose a find and measure each delta separately.
- Competing-not-additive result (load-bearing for the next session): the pointer-intermediate levers contend for the same seat. `argp = (s16 *)arg0` alone scores 17; `ent = &row[a1]` alone scores 15; both together score 24. There is one allocation decision here with many spellings, not a stack of independent knobs.
- Neutral-on-this-chassis deltas (measured, 159/159, no floor movement): `a1 = a2` reuse in the else arm (22), fused `*(s16 *)(arg0+0x14E) = (new_14e = sub_result)` (22), pointer local for the tail store (30).
- Regressions from stacking (measured, 159/159): `abs_val` reused for the 0xD8 accumulate temp 23; `tbl_val = speed` 24; both 24; argp+ent 24.
- Methodological correction to s2/s3: s2's conclusion that "no declaration/scope/type/statement-order change can reach the allocator here" is FALSE as stated. It held only for the small hand-enumerated neighbourhood s2/s3 explored, and only for hard_reg_conflicts bits. s3 had already flagged this (its operand-order finding), and s4 confirms it with 15 points of floor. The s1-s3 seat map and the H9/H10 allocno ids (129/122/86) are now STALE — they were measured against the 30-floor body, and this session's edits sit directly on the region they describe.

- [s4] Chassis re-measured at session start with the s3 candidate applied: score 30, target_insns 159, build_insns 159 - matches the ledger's recorded floor.

- [s4] FLOOR IS NOW 15, measured with the new body in place in src/code6cac.c at 159/159 insns (down from 30, flat across s1-s3).

- [s4] A chassis-faithful standalone permuter workspace for this function is cheap to rebuild and is banked at tmp/perm_23648_s4/{base.c,compile.sh,settings.toml,target.o,valid.sh}. It needs only the s8/u8/s16/u16/s32/u32 typedefs plus six externs (D_8008DA08, D_800A310C, D_800A38BA, D_8008EB40, Judge, func_8001F860), and its pre-launch validation (base.o vs target.o = 159/159, 30 mismatching lines) reproduced the sandbox residual exactly.

- [s4] WARNING for future sessions: the pre-existing tmp/perm_22F34 workspace in this repo is STALE as a template - it omits cc1's -mel flag and still pipes through the retired tools/fix_lwl.py. Copying it verbatim yields a workspace that does not model the current chassis.

- [s4] The permuter's weighted score tracks the sandbox score directionally but not proportionally, and its best find is NOT the best sandbox form: vanilla's output-120-1 carried two deltas, only one of which helped; the other was exactly neutral. Always decompose a find and measure each delta on its own.

- [s4] Competing-not-additive: argp alone 17, ent alone 15, argp+ent 24. One allocation decision, many spellings.

- [s4] Neutral deltas measured at 159/159 (banked in rejected/): a1 = a2 reuse in the else arm (22), fused *(s16 *)(arg0+0x14E) = (new_14e = sub_result) (22), pointer local for the tail 0x14E store (30).

- [s4] Regressions measured at 159/159 (banked in rejected/): abs_val reuse for the 0xD8 accumulate temp 23, tbl_val = speed 24, both 24, argp+ent 24.

- [s4] s2's conclusion that 'no declaration/scope/type/statement-order change can reach the allocator here' is FALSE as stated - it held only for the small hand-enumerated neighbourhood and only for hard_reg_conflicts bits. 15 points of floor movement on two ordinary-C respellings disproves the generalisation.

- [s4] The s1-s3 seat map and the H9/H10/H11 allocno ids (129 = div16, 122 = abs_val, 86 = the table entry, ord list 129 122 85 81 72 134 ...) are now STALE: they were measured against the 30-floor body, and this session's two edits sit directly on the abs/clamp/div16 region those hypotheses describe.

- [s4] All three campaigns were harvested with --stop inside the session; permuter_campaign.py status reports 0 live campaigns and 0 stale registry entries.
