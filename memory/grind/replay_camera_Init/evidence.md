
## s4 (permuter, 2026-07-30) — floor unchanged at 13; two families closed

- [s4] The floor did NOT move this session. `candidate.c` re-measured with
  `sandbox replay_camera_Init --disable all` at **13 / build_insns 38 /
  target_insns 39**, unchanged from s3, and nothing found this session beats it.
  `src/code6cac_b2_post.c` is returned to the exact `candidate.c` body at
  session end.

- [s4] PERMUTER OBJECTIVE IS ANTI-CORRELATED WITH THE SANDBOX HERE — the single
  most important operational fact this session produced. The permuter's default
  scorer weights registers × 5, reorderings × 60, insertions/deletions × 100
  (memory/reference/scoring-systems.md), while the engine sandbox counts every
  differing instruction at full weight. This function's entire remaining residue
  IS register naming, so the permuter prices our only defect at 5 points each
  and pays 100-point structural damage to shave it. Measured: the ws4 campaign's
  BEST find `output-235-1` sandboxes at **21** (the worst number in this grind),
  `output-315-1` at 15, `output-360-1` at 14, `output-385-1` at 15 — while
  `candidate.c`, the score-13 floor, is rated **650** by the permuter (read
  directly as the ws5 campaign's `base_score`). Chasing a low permuter score on
  this function walks AWAY from the floor. The permuter remains useful here only
  as a STRUCTURE generator (s3's winning pointer re-read came from a novel
  construct, not from a low score): triage finds by construct novelty, ignore
  the ranking, sandbox every one. The scorer cannot be reweighted from a grind
  session (`tools/` is outside the allowed surface).

- [s4] ws4 campaign (seeded from the 39-instruction `v_f`, label
  `s4-vf-39insn`, -j 6, --stop-on-zero): ~23,100 iterations, 160 output dirs,
  best permuter score 235, harvested and STOPPED in-turn. Directly kills s3's
  frontier item 2 ("seed from the 39-insn form and the search becomes purely
  register allocation"): the search does become purely register allocation, and
  the permuter cannot see register allocation.

- [s4] The `addu $a3,$a1,$zero` parameter-home copy is produced by making the
  TWO LOADS ADJACENT, not by target's statement order (s3's attribution was
  wrong). Ten-form hybridisation sweep between `v_d` (= candidate.c) and `v_f`,
  one statement moved at a time: every form whose loads are separated by a store
  builds 38 instructions (h1, h2 — both score 13), every form whose loads are
  adjacent builds 39 (h3-h10), regardless of where `D_80101E7C = a1;` sits. The
  mechanism is s2's H7 exactly — two simultaneously-live values force one into
  hard reg `$a1`, so the home copy cannot become a self-move — but supplied here
  by the function's OWN honest values, with no dead temporaries. s2 recorded
  that route as closed; it is not.

- [s4] Best member of the 39-instruction family is `h8` at **17 / 39**
  (`*pe62 = 2` before `D_80101E7C = a1`, and the `D_8008EC38` load issued BEFORE
  the `SpecialCam` load). Issuing the EC38 load first is worth -2 and fixes one
  naming defect outright: it puts `cam_val` in `$v1`, matching target's
  `lw v1,%lo(SpecialCam)`. Still 4 points above the 38-instruction floor.

- [s4] THE 39-INSTRUCTION FAMILY IS SEMANTICALLY WRONG — do not re-open it.
  Disassembly of h8 (tmp/grind/replay_camera_Init/s4/diffform.sh) shows the
  `D_80101E70` re-read emitted BEFORE the store it is meant to observe:
  `lw v1`/`lw a1`, `sw v1`(E6C), `lui a0; lw a0,0(a0)` (the re-read), `li v1,2`,
  `sh v1,0(a3)`, … and only at the very end `lui at; sw a1,0(at)` (the E70
  store). `D_80101E78` is therefore computed from the STALE value of
  `D_80101E70`. GCC 2.7.2 is free to sink the store past the load because the
  read `(mem (reg))` and the store `(mem (symbol_ref))` do not alias to its
  disambiguator — the same property that makes the re-read survive at all.
  `candidate.c` does NOT have this defect (its object emits the E70 store first,
  then `lui v1; lw v1,0(v1)`), so the floor form is semantically faithful. Any
  future session that moves the re-read earlier must re-check the EMITTED order,
  not just the score.

- [s4] REGISTER RESIDUE, restated precisely from this session's disassemblies.
  Target uses `$v1` (cam_val), `$a0` (ec_val), `$a3` (the a1 home copy) and
  `$t0` (the D_80101E62 address) and leaves `$a1` and `$a2` COMPLETELY UNUSED.
  `candidate.c` (38 insns) uses `$v1`, `$v1` again, and `$a2` for the address.
  `h8` (39 insns) uses `$v1`, `$a1`, `$a2`, `$a3`. Every one of our allocations
  sits one or two hard registers BELOW target's. s2 proved `find_reg` takes the
  lowest free hard reg (no `REG_ALLOC_ORDER` for MIPS in this tree), so target's
  compile must have carried genuine `hard_reg_conflicts` on `$a1` AND `$a2` at
  both allocation points, which nothing in the honest value set supplies. This
  is the one unexplained fact left and it is a PROVENANCE question, not an
  ordering question — the ordering axis is now measured dead in BOTH the 38- and
  39-instruction families.

- [s4] No layer-2 cheat-reviewer verdict was obtained on `candidate.c`'s two
  `/* FAKE */` pointer aliases this session (the session was constrained from
  spawning agents). It remains the highest-leverage open question in the ledger:
  a FAIL reverts the floor from 13 to 17 and changes the whole frontier.

- [s4] Both campaigns were harvested and STOPPED in-turn; no background work was
  orphaned. Artifacts under tmp/grind/replay_camera_Init/s4/.

- [s4] ws5 campaign (seeded from the score-13 candidate.c chassis, label
  s4-candidate13-reseed): permuter base_score 650, ~44,900 iterations, best
  permuter score 295; de-permuted finds sandbox at 16 / 37 (output-295-1) and
  14 / 39 (output-380-1's `short a0`). Harvested and STOPPED in-turn.

- [s4] `short a0` (the one structurally novel idea either campaign produced)
  reaches 39 instructions at score 14 with the re-read still correctly AFTER
  the E70 store — but the 39th instruction is `move v1,a0`, a truncation home
  copy of the FIRST parameter, not target's `move a3,a1`. Target's own asm
  disproves the narrowing: it stores the parameter with `sh a0,0(at)` straight
  from the incoming register, with no truncating copy, which is what an
  un-narrowed s32 parameter produces.

- [s4] The floor did NOT move: candidate.c re-measures at score 13 / build_insns 38 / target_insns 39 with `sandbox replay_camera_Init --disable all`, unchanged from s3, and nothing found this session beats it. src/code6cac_b2_post.c is left holding the exact candidate.c body and re-verified at 13.

- [s4] PERMUTER OBJECTIVE IS ANTI-CORRELATED WITH THE SANDBOX ON THIS FUNCTION — the key operational fact of the session. Permuter scorer = registers x 5, reorderings x 60, ins/del x 100; the sandbox counts every differing instruction at full weight; this function's whole residue is register naming. Measured: ws4's best find (permuter 235) sandboxes at 21 (worst in the grind); output-315-1 at 15; output-360-1 at 14; output-385-1 at 15; ws5's best (permuter 295) at 16; while candidate.c, the score-13 floor, is rated 650 by the permuter (read as ws5's base_score). Chasing a low permuter score here walks AWAY from the floor.

- [s4] ws4 campaign (seeded from the 39-instruction v_f, label s4-vf-39insn, -j 6, --stop-on-zero): ~23,100 iterations, 160 output dirs, best permuter score 235; harvested and STOPPED in-turn. This directly kills s3's frontier item 2 — the search does become purely register allocation, and the permuter cannot see register allocation.

- [s4] ws5 campaign (seeded from the score-13 candidate.c chassis, label s4-candidate13-reseed): base_score 650, ~44,900 iterations, best permuter score 295; harvested and STOPPED in-turn. `permuter_campaign.py status` reports 0 alive at session end; no background work orphaned.

- [s4] The `addu $a3,$a1,$zero` parameter-home copy is produced by making the TWO LOADS ADJACENT, not by target's statement order (s3's attribution corrected). Ten-form hybrid sweep between v_d and v_f, one statement at a time: loads separated by a store -> 38 insns (h1, h2, both score 13); loads adjacent -> 39 insns (h3-h10, scores 17-19). Mechanism is s2's H7, but supplied by the function's own honest values with no dead temporaries — s2 recorded that route as closed and it is not.

- [s4] Best member of the 39-instruction family is h8 at 17/39 (`*pe62 = 2` before `D_80101E7C = a1`, and the D_8008EC38 load issued BEFORE the SpecialCam load). Issuing the EC38 load first is worth -2 and fixes one naming defect outright: it puts cam_val in $v1, matching target's `lw v1,%lo(SpecialCam)`.

- [s4] THE 39-INSTRUCTION FAMILY IS SEMANTICALLY WRONG — do not re-open it. h8's disassembly emits the D_80101E70 re-read BEFORE the store it is meant to observe, so D_80101E78 is computed from a stale value. GCC may sink the store past the load because (mem (reg)) and (mem (symbol_ref)) do not alias to its disambiguator — the same property that makes the re-read survive. candidate.c does not have this defect.

- [s4] `short a0` narrowing (ws5's only novel idea) reaches 39 instructions at score 14 with the re-read still correctly AFTER the E70 store — but the 39th instruction is `move v1,a0`, a truncation home copy of the FIRST parameter, not target's `move a3,a1`. Target's own asm disproves the narrowing: it stores the parameter with `sh a0,0(at)` straight from the incoming register, with no truncating copy.

- [s4] REGISTER RESIDUE restated from this session's disassemblies: target uses $v1 (cam_val), $a0 (ec_val), $a3 (the a1 home copy) and $t0 (the D_80101E62 address) and leaves $a1 and $a2 COMPLETELY UNUSED; candidate.c (38 insns) uses $v1 and $a2; h8 (39 insns) uses $v1, $a1, $a2, $a3. Every one of our allocations sits one or two hard registers BELOW target's. s2 proved find_reg takes the lowest free hard reg (no REG_ALLOC_ORDER for MIPS in this tree), so target's compile carried genuine hard_reg_conflicts on $a1 AND $a2 at both allocation points, which nothing in the honest value set supplies. This is now the one unexplained fact and it is a PROVENANCE question, not an ordering question.

- [s4] The ordering axis is now measured dead in BOTH families — s2 killed it at 36/38 instructions, s4 killed it at 39 — and the permuter axis is measured dead for the reason in H12. The untried modalities are forensics / rederive / synthesis.

- [s4] No layer-2 cheat-reviewer verdict was obtained on candidate.c's two /* FAKE */ pointer aliases (this session was constrained from spawning agents; the driver's Judge or the operator must supply it). It remains the highest-leverage open question: a FAIL reverts the floor from 13 to 17 and changes the entire frontier.

- [s4] Reusable and validated this session: tmp/grind/replay_camera_Init/s4/setup_ws4.sh + setup_ws5.sh (workspace build), s4/sweep.py (apply-form -> sandbox -> restore, appends to sweep_results.json), s4/diffform.sh (re-preprocess the sandbox copy, compile, objdump side-by-side diff against target), s4/watch_ws5.sh (loop `permuter_campaign.py wait` for ~12 min in one call, because `wait` returns on EVERY novel output and this function produces them constantly).

## s5 (permuter, 2026-07-30) — measured facts

- **Ordering is now dead in the 38-instruction family too.** `candidate.c` = 13 / 38.
  Issuing the `D_8008EC38` load first with the `D_80101E70` store between the loads
  = 19 / 39; with `D_80101E7C = a1` between = 19 / 38; with BOTH stores between
  = 18 / 38. The load-order swap that is worth −2 inside the 39-instruction family
  is worth +5/+6 inside the 38-instruction one. (s5 H17; sweep in
  `tmp/grind/replay_camera_Init/s5/sweep_results.json`.)
- **s4 H14's 38-vs-39 predicate is too coarse.** p1 keeps a store between the two
  loads and STILL emits 39 instructions. Textual adjacency of the loads is a proxy
  for the real condition (the two loaded values being simultaneously live), not the
  condition itself. H15's semantic kill of the 39-instruction family is independent
  of this and still stands.
- **The expression-SPELLING axis is inert.** Twelve one-change, semantics-preserving
  re-spellings of `candidate.c` (sval as `((a0<<16)>>16)*8` / `(s32)(s16)a0 << 3`;
  `sval + base` index operand order; implicit guard; `pe70[0]` array syntax;
  declaration order swapped; cast placement in the E78 expression; an extra `u8 *`
  address local; `u32 reloaded`; `>> 11` written `/ 0x800`) ALL score 13 / 38 —
  identical to the floor. cc1 2.7.2 canonicalises them to the same RTL. (s5 H18.)
- **The `pe62` address cache is worth exactly −1 and only when the guard read AND
  the `*pe62 = 2` store both go through the pointer.** Splitting them (guard on the
  symbol, store on the pointer, or vice versa) scores 14 / 38 both ways. This is a
  direct confirmation of the "one materialised address reused across the branch"
  justification in candidate.c's reviewer notice — target's `$t0`.
- **The permuter mutates ONLY the named function.** `randomizer.py:2469`
  `def randomize(self, ast, fn_name)` → `ast_util.extract_fn(ast, fn_name)[0]`,
  with `fn_name` threaded from `settings.toml`'s `func_name` (`src/main.py:344-379`).
  The 1071-line preprocessed TU used as `base.c` in s3/s4/s5 is context only, so the
  ~33k (s3) + ~68k (s4) + s5 iterations were all genuine mutations of
  `replay_camera_Init`. The "diluted search" alternative explanation for the dead
  permuter axis is eliminated; s4 H12 (anti-correlated scorer) stands as the cause.
- **Third permuter basin, fresh seed, nothing new.** ws6 seeded from p3
  (EC38-load-first chassis, sandbox 18 / 38, permuter `base_score` 1080). Best finds
  triaged by construct rather than score; the only construct they contain is the
  already-known semantically-broken `sval` staging (`sval` reused to hold `cam_val`
  and then used as the `D_8008EC38` index), identical in kind to s4's ws5
  `output-295-1`. No novel construct in three campaigns across two sessions.

- [s5] Floor unchanged at 13 / 38 instructions (target 39). candidate.c remains the strict optimum of every axis measured in five sessions.

- [s5] 38-instruction ordering axis closed: candidate.c 13/38; EC38 load first with the E70 store between the loads 19/39; with D_80101E7C=a1 between 19/38; with both stores between 18/38.

- [s5] s4 H14's predicate is too coarse and must not be relied on: p1 keeps a store between the two loads and still emits 39 instructions. Simultaneous liveness of the two loaded values, not textual adjacency, is the real condition.

- [s5] Expression spelling is inert: 12 one-change semantics-preserving re-spellings of candidate.c all score 13/38 except the two that split the pe62 pointer, which score 14/38.

- [s5] The pe62 pointer alias is worth exactly -1 and only when the guard read AND the *pe62 = 2 store both go through it — i.e. it reproduces target's one materialised $t0 address reused across the branch and nothing else. (Supporting fact for the pending layer-2 cheat-reviewer.)

- [s5] decomp-permuter mutates ONLY the function named in settings.toml: randomizer.py:2469 randomize(ast, fn_name) -> ast_util.extract_fn(ast, fn_name)[0], fn_name threaded from src/main.py:344-379. The 1071-line preprocessed TU is context only, so all prior iteration counts are genuine.

- [s5] `short a1` scores 18 / 40 — GCC widens the narrowed parameter back to SImode for the 32-bit `sw` into D_80101E7C, costing two instructions and overshooting target's 39.

- [s5] Target's own encoding disproves both parameter narrowings: `sh $a0, %lo(D_80101E60)($at)` and `sw $a3, %lo(D_80101E7C)($at)`, both straight from un-narrowed 32-bit registers, with the home copy `addu $a3,$a1,$zero` a plain move in the bnez delay slot.

- [s5] ws6 campaign (fresh seed, EC38-first basin, base_score 1080): 40,900 iterations, 266 finds, best score 505, harvested and STOPPED in-turn before the outcome was written.

- [s5] candidate.c's two /* FAKE */ pointer aliases STILL carry no layer-2 cheat-reviewer verdict — s3 could not obtain one, s4 and s5 were both constrained from spawning agents. A FAIL reverts the floor 13 -> 17 and invalidates the frontier.
