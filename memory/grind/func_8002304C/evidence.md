# Evidence bank — func_8002304C

## s1 (2026-08-26, recon) — floor 29 -> 1 in one session; single opcode residual

- [s1-E1] **Baseline reproduced.** Recovered the pre-migration C body from git
  (`3a5882b2^:src/code6cac.c`, function `tanren_CameraControl`), de-cheated it
  (deleted the 3 register-asm pins on s4/s5/s6), applied to src/code6cac.c:
  `sandbox --disable all` == **29** (214/216 insns) — exactly the queue's
  migration-pin floor. Artifact: tmp/grind/func_8002304C/s1/normdiff.py +
  target_norm.s/build_norm.s (opcode-aligned differ; sra "10 vs 0xa" lines in
  its output are a formatting artifact, not diffs).
- [s1-E2] **Named 0x1F8002B8 constant = -10.** Declaring `s32 lim = 0x1F8002B8;`
  and passing it at both func_8005344C sites (instead of the literal) hoists
  the constant into a callee-save in the prologue exactly as target does
  (target s7), instead of lui/ori rematerialization into the compare-branch
  holes. 29 -> 19. This was the old chassis's rule family A (6 of 16 rules).
- [s1-E3] **Callee-save seats are DECLARATION-POSITION-mapped, and the map is
  stable across contents.** Three measurements (s1) with different var-to-
  position bindings all produced position->seat: pos2->s5, pos3->s7, pos4->s6,
  pos5->s4 (pos1 = scratch -> s0 always). Target seats (count=s5, lim=s7,
  scratch_d=s6, scratch_c=s4) are reached with decl order:
  scratch, count, lim, scratch_d, scratch_c. 19 -> 12 (with [s1-E4]'s retype).
- [s1-E4] **mode/m retype (s32 mode = *(u16*)...; u16 m = mode;) fixes the
  register split** — target keeps raw lhu in $a0 (feeds `addiu $v0,$a0,-0x17`)
  and a separate masked copy in $v1 (feeds the three beq tests). The wide->
  narrow direction materializes the distinct $v1 insn; both narrow-first
  spellings collapse it (score 3, see rejected/u16-mode-with-masked-or-copied-m.c).
- [s1-E5] **Prologue emission order follows INIT-statement order, seats follow
  DECL order — they are separable.** Declaring `lim` and `scratch_d`
  uninitialized (positions 3-4) and assigning them AFTER the initialized
  decls (`lim = ...; scratch_d = ...;` following `scratch_c`'s init) produces
  target's prologue order (s5,s4,s7,s6 init sequence) while keeping the
  positional seats of [s1-E3]. 12 -> 6.
- [s1-E6] **Inline ternary closes the whole 2nd-EBDC cluster.** Replacing the
  `s32 step; if/else; call(..., step)` block with
  `func_8002EBDC(..., (*(u16*)(obj+0x6A) == 0x15) ? 0x80 : 0x100)` gives:
  step-temp lands in $v0 (was $t0), `sw $v0,0x10(sp)` emitted BEFORE the
  a0-a2 moves, `addu $a3,$zero,$zero` in the jal delay slot — all matching
  target. 6 -> **1**. Mechanism (dump-read, .lreg): the named `step` local's
  const-0x100 birth (insn 309) was hoisted by sched1 ABOVE the 0x15 compare,
  widening its range into the compare temps' ($v0/$v1) ranges, forcing $t0 and
  freeing the sw to sink into the delay slot; the ternary births the value in
  the bne delay slot (after the compare temps die), so local RA gives $v0 and
  the store stays put. Old chassis rule families B+C (7 rules) — closed.
- [s1-E7] **Sole residual (floor 1): target 0x232F4 `andi $v1,$a0,0xffff` vs
  our `addu $v1,$a0,$zero`.** Same position, same registers, opcode-only.
  This is the exact insn the old chassis injected via
  `insert "andi $3,$4,0xFFFF" @ 168` (a lost-codegen-insert cheat, retired).
- [s1-E8] **Fold mechanism READ from compiler source (not guessed).**
  combine.c:6885-6891 (nonzero_bits REG case): a pseudo whose last set is
  current (`reg_n_sets==1 || reg_last_set_label==label_tick`) returns
  reg_last_set_nonzero_bits — the lhu (zero_extend:SI(mem:HI)) proves
  <=0xFFFF, so `zero_extend(subreg:HI(mode))` simplifies to a copy. The
  full-mask escape (combine.c:6920-6923 `else return nonzero;`) is reachable
  ONLY when: multi-set pseudo AND last set in a different label region AND
  reg_nonzero_bits never recorded (set_nonzero_bits_and_sign_copies gate,
  combine.c:724-732, requires reg_n_sets>1 && multi-BB && !live-at-start; two
  lhu sets OR to 0xFFFF anyway, so even recorded multi-set double-reads fold).
  The lhu and the mask use here are 2 insns apart in one BB with no label
  between — every same-BB honest spelling reaches the current-value path.
- [s1-E9] **Precedent trail for this exact shape:** decisions.md:2124-2140
  (func_800871D4, 2026-07-28) — same `lhu; nop; andi 0xFFFF` fold, ruled
  terminal for the u16-GLOBAL variant after census: zero COMPLETED-C sibling
  produces the shape; func_8002304C is NAMED there as one of 5 siblings. BUT
  the cases differ: 871D4's andi insns were MISSING entirely (fold-to-nothing);
  here the insn EXISTS as an addu copy — the residual is opcode-only, and the
  source is a struct-field read through a u8* base, not a splat u16 global.
  Also decisions.md sibling note: saTan0Main (COMPLETED-C) resolved a
  same-family byte case via narrow `char b` (memory/reference/
  redundant-byte-andi-satan0main.md) — the width lever direction that worked
  there is the one measured DEAD here ([s1-E4]'s rejected variants).
- [s1-E10] cc1psx does NOT fold this pattern (recorded calibration,
  decisions.md:2124 — 871D4's WIP notes, 2026-06-16). Informational only
  per no-compiler-divergence; our fork provably reproduces the full oracle,
  so a C spelling reaching andi in OUR cc1 exists or the original C reached
  it through structure we have not found yet (difficult-is-not-impossible).
- [s1-E11] Old chassis fully superseded: all 16 regfix rules + 3 pins +
  29-line prologue_config splice (removed at 86a18a14) map to honest levers
  [s1-E2..E6]; nothing from retired-chassis-2026-08/ is needed.

- [s1] Floor 1 measured THIS session with candidate applied in src/code6cac.c: sandbox --disable all == 1, 216/216 insns, zero rules dropped for this function, zero pins/FAKE constructs

- [s1] Sole residual: target 0x232F4 andi $v1,$a0,0xffff vs our addu $v1,$a0,$zero — same position, same registers, opcode-only; this is the exact insn the retired chassis injected via insert 'andi $3,$4,0xFFFF' @ 168

- [s1] All 20 retired cheat artifacts (16 regfix rules, 3 pins, 29-line prologue_config splice removed at 86a18a14) are superseded by honest levers; nothing from retired-chassis-2026-08/ is needed

- [s1] Precedent trail: decisions.md:2124-2140 (func_800871D4) ruled the u16-GLOBAL variant of this fold terminal and names func_8002304C as a shape-sibling — but there the andi insns were MISSING entirely; here the insn exists with wrong opcode, and the source is a struct-field read via u8* base, so the endgame equivalence is NOT established

- [s1] cc1psx does not fold this pattern (2026-06-16 calibration recorded in decisions.md:2124) — informational only per no-compiler-divergence

- [s1] Full session ledger in memory/grind/func_8002304C/evidence.md [s1-E1..E11] + hypotheses.md; rejected spellings banked in rejected/u16-mode-with-masked-or-copied-m.c
