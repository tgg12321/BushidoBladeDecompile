# Evidence bank — replay_camera_Init

- WIP rejected_form: *(volatile s32 *)&D_80101E70 inline cast at the final read site -- forces the reload but is the forbidden case-2 CSE-defeat cast pattern (inline-asm-injection / legitimate-volatile-interrupt-touched); engine detector flags it as cheat_asm_stripped delta, reviewer would FAIL the family check.

- WIP rejected_form: volatile s32 *ecp = &D_80101E70; *ecp = ec_val; ... *ecp + 0x7FF ... -- pointer-indirection variant of the same forbidden CSE-defeat spirit (defeats CSE via volatile aliasing with no IRQ-writer citation); DOES force a genuine reload instruction (confirmed empirically) but is the same forbidden family by spirit even though the mechanical detector's exact regex may not match the pointer-variable spelling. Do NOT resurrect without a genuine two-pronged IRQ-touched justification (none exists for D_80101E70 here) -- see .claude/rules/legitimate-volatile-interrupt-touched.md.

- WIP rejected_form: register s32 saved_a1 asm("$7") = a1; (diagnostic-only, confirmed via instrumented sandbox --keep-cheat-asm run to be the ONLY thing that reproduces target's exact instruction ORDER: with just this pin, disable=none/keep-cheat-asm scored 11 with build_insns=39 matching target count and the volatile reload appeared naturally at the correct position). This CONFIRMS the pin is diagnostically correct but per register-asm-pins.md it is diagnostic-only / never committable. No pure-C structure found yet that makes GCC choose this allocation without the pin.

- WIP rejected_form: a1 = *(s32*)(SpecialCam+sval); D_80101E6C = a1; (param-reuse lever, param-reuse-base-copy-cse-canon.md style) -- did NOT force GCC to allocate a1 hardware register to cam_val; GCC still chose v1 and treated the a1 reassignment as an ordinary reg-to-reg copy candidate, not a hard binding.

- WIP rejected_form: s32 saved_a1 = a1; as a plain (non-register) local referenced once at D_80101E7C -- always coalesced/folded back to a1 by GCC (dead-copy elimination), regardless of declaration order (tried before AND after `s16 *s0` decl).

- WIP rejected_form: func_80036FD4 sibling's entry[0]/entry[1] shared-pointer trick (SpecialCam and D_8008EC38 are adjacent globals, SpecialCam=0x8008EC34, D_8008EC38=0x8008EC38, exactly +4) -- does NOT apply here: target's actual asm/funcs/replay_camera_Init.s emits TWO SEPARATE symbol relocations (R_MIPS_HI16 SpecialCam and R_MIPS_HI16 D_8008EC38, confirmed via objdump -dr), unlike func_80036FD4 which computes both loads through ONE base+offset. Ruled out by direct relocation evidence, not guesswork.

- == imported from memory/wip notes.md ==
# replay_camera_Init — WIP TL;DR

**Status:** honest pure-C distance 17 (down from HEAD's 18), 0 cheat constructs, 1 residual regfix rule (`fill_delay @ 26 <- 15`). NOT closable to 0 yet.

## What HEAD had (and why this is on the queue)
HEAD's committed body byte-matches the oracle but ONLY via 2 register-asm pins
(`saved_a1 asm("$7")`, `s0 asm("$8")`) + a bare `__asm__ volatile("":::"memory")`
scheduling barrier — all diagnostic-only/forbidden (register-asm-pins.md,
inline-asm-policy.md). That's why it's INCOMPLETE despite SHA1==oracle at HEAD.

## What I found
1. Removing the pins/barrier alone keeps distance at 18 (same as before) — the
   pins/barrier were score-inert for the honest metric anyway (as expected).
2. Restoring `s16 *s0 = &D_80101E62;` (a plain, non-register pointer cache,
   matching sibling `func_80036FD4`'s idiom) makes GCC cache the address once
   and reuse it for both the initial `*s0 != 0` check and the final `*s0 = 2`
   store — matching target's use of ONE register ($t0 in target) throughout.
3. **The core remaining gap is a register-rotation/scheduling wall.** Target's
   compiled form:
   - preserves the incoming `a1` into a fresh register (`$a3`) in the branch's
     delay slot, unconditionally, right at function entry
   - this frees `$a1`'s hardware register for the `SpecialCam` load (`cam_val`)
   - which in turn means the store to the (legitimately volatile, pre-existing)
     `D_80101E70` global is followed by a GENUINE re-load instruction (not
     folded), because by the time of the read, the register holding the
     just-stored value has already been reused for `D_8008EC38` doesn't apply — see the raw asm; the reload is real and uses a DIFFERENT register than the store).
4. I confirmed (2) is the load-bearing mechanism by DIAGNOSTIC-ONLY use of
   `register s32 saved_a1 asm("$7") = a1;` (never committed) via
   `sandbox --disable none --keep-cheat-asm`: with JUST that pin, build_insns
   became 39 (== target) and the reload appeared naturally, score dropped to 11.
   This PROVES the pin's target register assignment is what target's original
   compile produced — but per register-asm-pins.md this is diagnostic-only.
5. I could NOT find a pure-C structure that makes GCC choose this allocation
   without the pin. Tried: param-reuse (`a1 = ...; D_80101E6C = a1;`), plain
   `saved_a1` locals (before/after other decls), the sibling's shared-pointer
   trick (ruled out — target uses two SEPARATE symbol relocations for
   SpecialCam/D_8008EC38, confirmed via `objdump -dr`, so the shared-pointer
   shape doesn't apply here even though the two globals ARE adjacent in memory).
6. One reordering DID help: moving `D_80101E7C = a1;` to sit texually between
   the two loads let GCC's scheduler place it (using `a1` directly, no pin
   needed) at almost the right spot, dropping distance 18 -> 17. This is
   captured in `candidate.c`.

## Cheat-reviewer history
- Pass 1 (2026-07-05) FAILed an earlier version of this candidate that kept
  `s16 *s0 = &D_80101E62;` as a plain pointer-caching idiom (matching sibling
  `func_80036FD4`'s shape). Reviewer classified it as an unannotated
  pointer-alias-fake-exception shape (no lever-exhaustion doc, no named
  mechanism, no `/* FAKE */` annotation) and FAILed on that ground alone —
  everything else passed.
- Fix: removed the alias entirely, replaced with two direct `D_80101E62`
  accesses. Re-measured: distance UNCHANGED at 17 — the alias was not
  load-bearing for the score, so there was no reason to justify/annotate it.
  `candidate.c` now reflects this simpler, alias-free form.
- Pass 2 re-review requested on the corrected candidate; see meta.json for
  the recorded verdict.

## Rejected forms (do not re-derive)
See `meta.json.rejected_forms` for the full list with reasoning. Highlights:
- `*(volatile s32 *)&D_80101E70` cast at the read site — FORBIDDEN (case-2
  CSE-defeat cast, matches inline-asm-injection / legitimate-volatile-interrupt-touched
  ban; D_80101E70 has no IRQ-writer citation so doesn't qualify for the narrow carve-out).
- `volatile s32 *ecp = &D_80101E70;` pointer-indirection variant — empirically
  DOES force the reload, but is the same forbidden-by-spirit CSE-defeat family
  (no IRQ justification). Do not resurrect without a genuine two-pronged citation.

## Next steps for whoever resumes this
- The remaining ~17-point gap is almost entirely the a1/a3 register-rotation
  wall + the consequent lack of a real D_80101E70 reload. This matches the
  "register rotation" wall class documented in register-alloc-pure-c.md
  (Confirmed limits section) — consider a proper decomp-permuter run (not
  attempted this session — infra setup was deprioritized in favor of manual
  structural analysis) before declaring this un-closable in pure C.
- Do NOT re-try the rejected forms above.
- Apply `candidate.c`, confirm floor with `sandbox --disable all` (expect 17),
  then continue from there.


## == s1 (recon, 2026-07-30) ==

- EXACT structural gap measured: target 39 insns, honest build 36. The three
  missing instructions are (a) `addu $a3,$a1,$zero` — the incoming-a1 parameter
  copy, which target puts in the `bnez` delay slot; (b)+(c) `lui $v1,%hi(D_80101E70)`
  + `lw $v1,%lo(D_80101E70)($v1)` — a genuine RE-LOAD of D_80101E70 immediately
  after the store of the same global. Everything else is register naming.

- The reload is produced solely by the PRE-EXISTING `extern volatile s32
  D_80101E70;` at src/code6cac_b2_post.c:45, which the cheat-invisible sandbox
  STRIPS. Proof: the sandbox's own source copy
  (tmp/sandbox/replay_camera_Init/src/code6cac_b2_post.c:45) reads
  `extern s32 D_80101E70;` and `grep -c volatile` on it returns 0 vs 2 on the real
  file. Mechanism: cse.c:7329 skips inserting a store's destination MEM into the
  equivalence table when `sets[i].src_elt == 0`, which is exactly the volatile
  case; without volatile the insert at cse.c:7358 forwards the stored register to
  the later read. Confirmed in the RTL: with volatile present, `.rtl` already has
  `(mem/v:SI (symbol_ref "D_80101E70"))` on both the store and the read and both
  survive every pass through `.greg`.

- MEASURED FLOOR SPLIT (this is the key number for planning):
    `sandbox --disable all`                  -> 17, build_insns 36  (reload absent)
    `sandbox --disable all --keep-cheat-asm`  -> 14, build_insns 38  (reload present)
  So 3 of the 17 honest points / 2 of the 3 missing instructions are gated on the
  volatile POLICY question, not on a C-structure search. The residual after the
  reload is 14, dominated by the missing a1->a3 parameter copy plus renames.

- OVERTURNS the s0 ledger claim that the `s16 *s0 = &D_80101E62;` alias is not
  load-bearing. It is inert at the stripped floor (17 with and without) but worth
  -1 once the reload exists (14 without alias vs 13 with alias, both at 38 insns).
  s0 measured only the stripped regime and generalised. The alias still is not
  committable as-is (unannotated pointer-alias-fake-exception, reviewer-FAILed in
  s0) but it must be re-measured, not assumed dead, by whoever resolves the
  volatile question.

- rejected_form (KILLED s1): `s16 *s0 = &D_80101E62;` as a CSE-invalidation lever
  for the D_80101E70 reload. Reproduces target's single-materialized-address shape
  (`lui;addiu;lh 0(reg)` ... `sh 0(reg)`, matching target's $t0 usage) but produces
  NO reload: score 17 / 36 insns, identical to the alias-free form. Reason:
  cse.c:7539 `note_mem_written` only sets `writes_ptr->all = 1` (which would kill
  every memory equivalence) when the store's address satisfies
  `cse_rtx_addr_varies_p`; cse_insn folds the destination address first, and the
  address pseudo carries `REG_EQUIV (symbol_ref "D_80101E62")` (visible in .greg
  insn 13), so it folds to a constant and does not vary. ANY C pointer whose
  initializer GCC can constant-fold fails identically — this closes the whole
  "use a pointer store to invalidate CSE" family, not just this spelling.

- rejected_form (KILLED s1): extra declared parameter
  `replay_camera_Init(s32 a0, s32 a1, s32 a2)` with a2 unused, as a way to make
  the incoming-argument hard registers live at entry and shift RA toward target's
  $a3/$t0. Score 17 / 36 insns, completely unchanged — `flow` deletes the dead
  parameter-copy insn before global.c builds the conflict graph.

- RA FACTS from this session's `.greg` dump (banked at
  tmp/grind/replay_camera_Init/s1/rtl/base.i.greg): exactly 3 global allocnos.
  `72 preferences: 4` -> $a0; `73 preferences: 5` -> $a1 (this is the a1 parameter
  pseudo — target needs it in $a3=7); `75` no preference -> $a2 (the address
  pseudo — target has it in $t0=8). Conflict sets: `73 conflicts: 72 73 75 2 3 29`,
  `75 conflicts: 72 73 75 2 3 4 29`. Target's allocation is a uniform +2 shift,
  which means hard regs $a1 AND $a2 were excluded for allocno 73 in the original
  compile. This tree defines NO `REG_ALLOC_ORDER` for MIPS (grep over
  tools/gcc-2.7.2/ hits only ChangeLogs), so global.c `find_reg` masks
  conflicts+preferences (global.c:1093) and then takes the lowest-numbered free
  hard reg — hence our 4/5/6. Producing target's 4/7/8 requires real conflicts on
  $a1 and $a2, i.e. a C shape keeping those hard regs live past the pseudo's birth.

- Permuter has still NEVER been run on this function (s0 deprioritized the infra;
  s1 was recon-modality). That search space is entirely unexplored.

- [s1] Exact structural gap: target 39 insns, honest build 36. The three missing instructions are (a) `addu $a3,$a1,$zero` — the incoming-a1 parameter copy, which target places in the bnez delay slot; (b)+(c) `lui $v1,%hi(D_80101E70)` + `lw $v1,%lo(D_80101E70)($v1)` — a genuine re-load of D_80101E70 immediately after the store of the same global. Everything else in the 17 is register naming.

- [s1] MEASURED FLOOR SPLIT (the key planning number): `sandbox --disable all` = 17 with build_insns 36 (reload absent); `sandbox --disable all --keep-cheat-asm` = 14 with build_insns 38 (reload present). So 3 of the 17 honest points are gated on the volatile policy question and the residual C-structure problem is 14, dominated by the single missing parameter copy plus renames.

- [s1] `extern volatile s32 D_80101E70;` at src/code6cac_b2_post.c:45 pre-dates this grind and is also used by a second function in the same file (line 325). The sandbox strips it: its own source copy at tmp/sandbox/replay_camera_Init/src/code6cac_b2_post.c:45 reads `extern s32 D_80101E70;`, grep -c volatile = 0 on the sandbox copy vs 2 on the real file.

- [s1] cse.c mechanism, named and line-cited: cse.c:7329 skips inserting a store's destination MEM into the equivalence table when sets[i].src_elt == 0 (the volatile case); cse.c:7358 performs the insertion otherwise, which is what folds our read. cse.c:7539-7578 note_mem_written is the only route to invalidating memory equivalences without volatile, and it requires cse_rtx_addr_varies_p to hold on the store address.

- [s1] RA facts from this session's .greg dump: exactly 3 global allocnos. `72 preferences: 4` -> $a0; `73 preferences: 5` -> $a1 (the a1 parameter pseudo — target needs it in $a3 = reg 7); `75` with no preference -> $a2 (the address pseudo — target has it in $t0 = reg 8). Conflict sets: `73 conflicts: 72 73 75 2 3 29` and `75 conflicts: 72 73 75 2 3 4 29`. Producing target's allocation requires real conflicts on BOTH $a1 and $a2 for allocno 73.

- [s1] The alias reproduces target's address-materialization shape exactly (one `lui;addiu` address in a register, reused for both the pre-branch `lh` and the post-branch `sh`), which is a structural match to target's $t0 usage — so that half of the shape is solved and only its register number differs.

- [s1] Permuter has still NEVER been run on this function: s0 explicitly deprioritized the infra setup and s1 was recon modality. That search space is entirely unexplored.

- [s1] The engine's canonical gate re-confirms verdict C (pure-C target, 39 total insns, distance 17); diagnose classifies it LARGE (d22) with 18 differing insns.

- [s1] src/code6cac_b2_post.c was returned to the exact candidate.c body at end of session and re-verified at score 17 / build_insns 36. No other tracked file was modified.

## == s2 (structural, 2026-07-30) ==

- [s2] VOLATILE CENSUS — NEGATIVE, GATE CLOSED. `D_80101E70` has exactly two
  writers in the whole tree, both synchronous: `replay_camera_Init` itself
  (src/code6cac_b2_post.c:263) and `func_80036FD4`
  (src/code6cac_b2_post.c:329), the latter called synchronously from
  src/code6cac_b2_post.c:362 and src/code6cac_c2.c:327. All other mentions are
  the seven `extern` declarations across the code6cac* TUs. No IRQ / callback /
  VSync handler writes it: every callback registration in the tree installs a
  different address (`&D_80080014` / `&D_8008003C` at src/display.c:3743-3744,
  `&D_80082050` at src/system.c:1146, `&g_snd_irq_data` at src/main.c:1617), and
  `marionation_camera_Init_80036064` (src/code6cac_b2_post.c:205) — the
  replay-camera path s1's frontier suspected — calls `cdrom_SetCallbackB(0)`,
  i.e. it DEREGISTERS. Prong (1) of legitimate-volatile-interrupt-touched FAILS;
  the `extern volatile s32 D_80101E70;` at line 45 is not carve-out-eligible and
  the honest floor of 17 stands.

- [s2] Our build's register NAMING already matches target for the three value
  pseudos: `sval` = $v0, `cam_val` = $v1, `ec_val` = $a0, identical to target's
  `sll/sra $v0`, `lw $v1,%lo(SpecialCam)`, `lw $a0,%lo(D_8008EC38)`. Only two
  register-level residuals remain: the `$t0` cached address of `D_80101E62`
  (target materialises `lui;addiu` once and uses `lh 0($t0)` / `sh $a0,0($t0)`;
  we emit two separate `lui`s) and the `$a3` copy of the `a1` parameter. The
  "17" is therefore NOT a broad rename cluster, contrary to how s1 summarised it.

- [s2] ORDERING AXIS MEASURED DEAD (8 forms, both regimes). candidate.c is a
  strict local optimum at 17/36 stripped and 14/38 with the reload; every
  neighbour is worse. Full table in hypotheses.md H6. The counter-intuitive
  result worth remembering: writing the statements in TARGET'S OWN EXECUTION
  ORDER scores 18/37 and 23/40 — worse in both regimes than candidate.c's odd
  placement of `D_80101E7C = a1;` between the two loads. Do not "clean up" that
  placement.

- [s2] CONFIRMED by measurement, not inference: the `a1` parameter-home copy
  materialises as a real instruction as soon as another live value occupies hard
  reg $a1 across its live range. Diagnostic form with three extra live temps
  produced `3b0: move v1,a1` at exactly target's copy position while a temp took
  $a1 (`3e4: lw a1,8(at)`). It lands in $v1, not target's $a3, and the extra
  temps are dead-value cheats — so the route is closed, but the mechanism is now
  proven.

- [s2] RA facts for the CURRENT candidate (2 global allocnos, not s1's 3 — s1
  measured with the pointer alias in place): `;; 72 conflicts: 72 73 2 5 29`,
  `;; 72 preferences: 4`; `;; 73 conflicts: 72 73 2 3 29`,
  `;; 73 preferences: 5`; `Register dispositions: 72 in 4  73 in 5`. Note that
  the a0-home pseudo (72) DOES conflict with hard reg 5 — $a1 is still live when
  it is born — while the a1-home pseudo (73) does not, because $a1 is the source
  of its own copy. Dump banked at tmp/grind/replay_camera_Init/s2/rtl/base.i.greg.

- [s2] global.c mechanism, line-cited: `prune_preferences` (global.c:893-895)
  refuses to place into `regs_someone_prefers[A]` any register A itself prefers
  (the same-size `AND_COMPL_HARD_REG_SET (temp, hard_reg_full_preferences[allocno])`
  clause), so $a1 can never be denied to allocno 73 by the preference machinery;
  and `find_reg` seeds `regs_used_so_far` with ALL `call_used_regs`
  (global.c:352-355), so the pass-0 "never allocate a register for the first
  time" rule never protects $a1..$a3 / $t0... The only route to denying 73 hard
  reg 5 is a genuine `hard_reg_conflicts` entry, i.e. $a1 occupied by an
  overlapping live value.

- [s2] CALLER EVIDENCE (asm/funcs/special_camera_check_pos_outside_ground_80036E34.s):
  the only in-tree caller sets up NOTHING beyond the incoming $a0/$a1 before
  `jal replay_camera_Init` — it moves its own $a2/$a3 into $s1/$s2 first
  (`addu $s1,$a2,$zero` / `addu $s2,$a3,$zero`) and fills the jal delay slot with
  `sw $s0,0x10($sp)`. So a wider declared signature for replay_camera_Init is
  not supported from the call side, independently of s1's H4 (which killed the
  dead-extra-parameter form on the callee side).

- [s2] OPEN CONTRADICTION worth stating plainly for the next session: target
  allocates the a1 value to $a3 (7) and the D_80101E62 address to $t0 (8) while
  leaving hard regs $a1 (5) and $a2 (6) COMPLETELY UNUSED in the emitted body.
  Under this tree's `find_reg` (lowest free hard reg, no REG_ALLOC_ORDER for
  MIPS) that is not reachable from a two-allocno shape like ours. The original
  compile's allocno set must differ from ours in a way that has not yet been
  identified — that, not statement order, is where the remaining structural
  search should go.

- [s2] Artifacts: sweep harness + variants + both result JSONs, the diagnostic
  form, and a fresh full `cc1 -da` dump set under
  tmp/grind/replay_camera_Init/s2/. src/code6cac_b2_post.c was returned to the
  exact candidate.c body at end of session and re-verified at score 17 /
  build_insns 36. No other tracked file was modified.

- [s2] VOLATILE CENSUS NEGATIVE — GATE CLOSED. D_80101E70 has exactly two writers in the whole tree, both synchronous: replay_camera_Init itself (src/code6cac_b2_post.c:263) and func_80036FD4 (src/code6cac_b2_post.c:329), which is called synchronously from src/code6cac_b2_post.c:362 and src/code6cac_c2.c:327. No IRQ/callback/VSync handler writes it — every callback registration installs a different address (&D_80080014 / &D_8008003C at src/display.c:3743-3744, &D_80082050 at src/system.c:1146, &g_snd_irq_data at src/main.c:1617), and marionation_camera_Init_80036064 (src/code6cac_b2_post.c:205) calls cdrom_SetCallbackB(0) — it deregisters. Prong (1) of legitimate-volatile-interrupt-touched FAILS.

- [s2] Our build's register NAMING already matches target for all three value pseudos: sval = $v0, cam_val = $v1, ec_val = $a0, identical to target's sll/sra $v0, lw $v1,%lo(SpecialCam), lw $a0,%lo(D_8008EC38). The honest 17 is therefore NOT a broad rename cluster (contrary to how s1 summarised it) — only two register-level residuals remain: the $t0 cached address of D_80101E62, and the $a3 copy of the a1 parameter.

- [s2] ORDERING AXIS MEASURED DEAD across 8 forms in both regimes; candidate.c is a strict local optimum at 17/36 stripped and 14/38 with the reload. Writing the statements in target's own execution order scores 18/37 and 23/40 — worse in both regimes. The placement of `D_80101E7C = a1;` between the SpecialCam load and the D_8008EC38 load is load-bearing.

- [s2] PROVEN BY MEASUREMENT (not inference): the a1 parameter-home copy materialises as a real instruction the moment another live value occupies hard reg $a1 across its range. The diagnostic form emitted `move v1,a1` at exactly target's copy position while a temp took $a1. It lands in $v1 rather than target's $a3, and the temps are dead-value cheats, so the route is closed — but the mechanism is now established fact.

- [s2] RA facts for the CURRENT candidate (2 global allocnos, not s1's 3 — s1 measured with the pointer alias in place): `;; 72 conflicts: 72 73 2 5 29` / `;; 72 preferences: 4`; `;; 73 conflicts: 72 73 2 3 29` / `;; 73 preferences: 5`; `Register dispositions: 72 in 4  73 in 5`. The a0-home pseudo DOES conflict with hard reg 5 ($a1 is still live when it is born); the a1-home pseudo does not, because $a1 is the source of its own copy.

- [s2] global.c mechanism, line-cited: prune_preferences (global.c:893-895) refuses to place into regs_someone_prefers[A] any register A itself prefers, so $a1 can never be denied to allocno 73 by the preference machinery; find_reg seeds regs_used_so_far with ALL call_used_regs (global.c:352-355), so pass 0's 'never allocate a register for the first time' rule never protects $a1..$a3 / $t0..; no REG_ALLOC_ORDER is defined for MIPS in this tree.

- [s2] CALLER EVIDENCE (asm/funcs/special_camera_check_pos_outside_ground_80036E34.s): the only in-tree caller sets up nothing beyond the incoming $a0/$a1 before `jal replay_camera_Init` — it moves its own $a2/$a3 into $s1/$s2 first (addu $s1,$a2,$zero / addu $s2,$a3,$zero) and fills the jal delay slot with sw $s0,0x10($sp). A wider declared signature is unsupported from the call side, independently of s1's H4 which killed the dead-extra-parameter form on the callee side.

- [s2] OPEN CONTRADICTION for the next session: target allocates the a1 value to $a3 (7) and the D_80101E62 address to $t0 (8) while leaving hard regs $a1 (5) and $a2 (6) COMPLETELY UNUSED in the emitted body. Under this tree's find_reg (lowest free hard reg, no REG_ALLOC_ORDER) that is unreachable from a two-allocno shape like ours. The original compile's allocno set must differ from ours in a way neither s1 nor s2 has identified.

- [s2] src/code6cac_b2_post.c was returned to the exact candidate.c body at end of session and re-verified: score 17, build_insns 36, target_insns 39, rules_dropped 1, cheat_asm_stripped 13. No other tracked file was modified (git status shows only the memory/grind ledger files, metrics/events.jsonl, and src/code6cac_b2_post.c).
