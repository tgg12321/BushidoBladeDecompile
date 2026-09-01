# Evidence bank — func_800770B8

## Session 1 (recon, 2026-09-01)

### Function identity
- 175-insn init routine in src/text1b.c: `s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2)`.
  Clears an OT (`ClearOTagR(D_800A374C, 0x1008)`), stops sound, allocates/initializes
  the D_800A36A0 control block via `func_8006E49C(func_80076FF8(arg0+0x58), D_800A35D8)`,
  runs a 2×(5+10) init loop over per-player fields + the D_8009BCE4[20] byte table
  (bit i of arg2 sets the low bit and bumps a per-player u16 count on the stack),
  then derives byte 0x64 = min(count0,count1)-3 clamped (>=3 → 2), stores arg1 at +0,
  bytes 0x65/0x66/0x67, D_800A35DC=1, RETURNS 1 (target sets $v0=1 pre-epilogue —
  the function is NOT void; caller func_80077A80 ignores the result).
- Caller-side edits landed in src/text1b.c s1 (byte-neutral for the caller):
  prototype `s32 func_800770B8(s32, s32, s32);` (was `void (s32, s32*, s32)`), call
  arg `(s32)&D_8009BD24` (was `(s32 *)&D_8009BD24`).
- canonical verdict C, hand-coded tier LOW. No sibling/duplicate leads
  (tmp/duplicates_leads.txt has no entry for this function).

### Inheritance
- Retired chassis (memory/grind/func_800770B8/retired-chassis-2026-08/body.c, floor 32,
  25 rules): carried a `register asm("$16")` pin + memory barrier + dead `arg0 = 1`
  (all cheats, dropped). Its rules.txt is the map of the rule-era residual classes.
- Cross-knowledge: this function is in the insert_label device census (9 funcs /
  11 rules, main's ledger s5) — its rule-era residual included the synthetic-label
  beq-retarget device (rules @2200/2205/2206: insert_after `addu $2,$6,1` +
  insert_label + beq retarget). main's ledger proves that device serves the
  reorg.c redundancy/thread-skip mechanism THERE but device-sharing is shape
  evidence only.

### Floor history THIS session (all sandbox func_800770B8 --disable all, HEAD chassis)
- INCLUDE_ASM baseline: 175 (no C body).
- Clean chassis (chassis minus cheats + `return 1`): **26** (175/175).
- +mask named intermediate (`s32 mask = 1 << idx;` in inner loop 2): **20** (176/175).
  Kills GCC's single-bit fold `(x & (1<<k))!=0 → (x>>k)&1` (srav+andi → sllv+and,
  and the constant 1 becomes a hoisted loop invariant `$t1=1` exactly as target).
- +two-statement symbol materialization into the reused `ptr`
  (`ptr = (u8*)&D_800A35D0; ptr = (t0*4) + ptr;`): **15**. Defeats LICM of the
  `lui/addiu %hi/%lo(D_800A35D0)` pair: expand targets ptr's own (multi-set) pseudo,
  so scan_loop never admits it as a movable; the pair emits in-loop exactly as
  target (rows 49-51 clean). Single-statement `ptr = (u8*)&D_800A35D0 + t0*4`
  does NOT work (symbol materializes into a fresh single-set temp → hoisted, even
  though ptr itself is multi-set).
- +int-domain first ptr (`ptr = (u8*)((t0*2) + (s32)base);`): **14** (176/175).
  Fixes the row-43 addu operand order (target `addu v0,v0,a0` = offset,base).
  Note: the same swap in POINTER-domain (`(t0*2) + base`, int+ptr) is canonicalized
  and byte-neutral; the (s32) cast into int-domain is what preserves operand order.
- Killed forms (details in rejected/): q-split-handle (28, folds), tail cached-pointer
  groups (29, cse merges the reloads), row-pointer for p_6a/p_6e bases (38).

### Residual census at floor 14 (from tmp/grind/func_800770B8/s1/posdiff.py output)
1. **Prologue schedule (rows 7-12, ~5-6 diffs):** ours saves $s1 at slot 7 + computes
   `addiu $17,$16,88` early, `sw $ra` late; target saves $ra at 7, $s1 at 8, sets up
   ClearOTagR args a1-first (`addiu a1,0,0x1008` BEFORE `lui/lw a0`), `addiu s1,s0,0x58`
   at 12. Ours evaluates a0 first, a1 last. sched1 ordering cascade.
2. **Pre-loop cluster (rows 30, 33-37, ~4 diffs):** (a) ours schedules the pre-loop
   `a2 = 0` (`move $6,$0`) BEFORE the invariant constants; target has it AFTER the
   0x30/0x34 stores (row 37). (b) ours stores 0x30/0x34 through $17 (s1); target
   through $v0 (call result) while D_800A36A0/+4 go through $s1 — i.e. target keeps
   the raw call-result pseudo live for two stores. The naive second-handle spelling
   is KILLED (rejected/q-split-handle-call-result.c).
3. **p_6a/p_7e base coalesce (rows 62-64, ~3 diffs):** ours `addu $2,$2,$3` (dest =
   base's pseudo); target `addu $v1,$v1,$v0` (dest = offset's pseudo; the running
   5t0→10t0 variable absorbs the base). Association-order spelling is byte-neutral
   (measured); the whole-region `row` variable restructures and scores worse. This is
   an RA/coalesce question → read .lreg "dies in N places" for the two pseudos.
4. **reorg delay-slot fill choice (~1-2 diffs + phase):** both builds share ONE C-level
   `a2 = a2+1` join in inner loop 2. Ours: reorg fills the `beqz` slot from the
   FALL-THROUGH (`ori $2,$4,1`, dest dead on taken path) and branch targets the shared
   addiu. Target: reorg fills from the BRANCH-TARGET thread (`addiu v0,a2,1`),
   retargets past it, leaving the original addiu for the fall-through path — so the
   insn appears twice (this is the rule-era insert_label device residual). Pre-branch
   instruction streams are IDENTICAL (rows 96-103 clean), so this is purely reorg's
   fill selection. tmp/grind/func_800770B8/dumps/text1b.dbr is generated and unread.
5. Everything else — both inner loops' bodies, the D_8009BCE4 read-modify-write
   cluster, the min/clamp tail, the final store tail, the epilogue — is byte-clean
   at floor 14.

### Artifacts
- tmp/grind/func_800770B8/s1/posdiff.py — positional differ (ours .o vs target .s),
  reusable; run in WSL after any sandbox to refresh the census.
- tmp/grind/func_800770B8/dumps/text1b.{combine,cse,cse2,dbr,flow,greg,jump,jump2,loop,lreg,sched,sched2}
  — generated at the floor-14 chassis (dump.ps1 warnings about redefinitions are
  pre-existing TU noise, dumps are valid).

- [s1] sandbox func_800770B8 --disable all = 14 (176/175) with the s1 candidate in place in src/text1b.c THIS session

- [s1] canonical verdict C, hand_coded_tier LOW

- [s1] retired chassis floor 32 carried $16 pin + memory barrier + dead arg0=1 (all dropped); its 25 rules map the residual classes

- [s1] residual census at 14: prologue schedule (~6), a2=0 placement + 0x30/0x34 store handle (~4), p_6a base coalesce (~3), reorg delay-slot fill choice on inner-loop-2 beqz (~2, the rule-era insert_label device residual)

- [s1] pre-branch streams identical at the beqz — the fill divergence is purely reorg's selection (ours: fall-through ori steal; target: branch-target thread steal + retarget)

- [s1] caller-side byte-neutral edits landed: prototype s32(s32,s32,s32) + (s32)&D_8009BD24 cast

- [s1] full ledger: memory/grind/func_800770B8/evidence.md + hypotheses.md; candidate at memory/grind/func_800770B8/candidate.c; 3 rejected forms banked
