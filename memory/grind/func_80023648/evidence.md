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
