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

## Session 2 (structural, 2026-09-01) - floor 14 -> 12 -> 10

### Chassis re-measurement
- The s1 candidate was NOT in src/text1b.c at dispatch (HEAD carried
  INCLUDE_ASM("asm/funcs", func_800770B8); and the ORIGINAL caller prototype
  `void func_800770B8(s32, s32 *, s32);` + `(s32 *)&D_8009BD24` call arg). s2
  re-applied the s1 body AND both caller-side edits (tmp/grind/func_800770B8/s2/apply.py)
  and re-measured **14** - the s1 floor is chassis-valid on today's HEAD.

### Positional-differ upgrade (tooling)
- tmp/grind/func_800770B8/s1/posdiff.py now aligns with difflib (SequenceMatcher over
  normalized insns) instead of index-by-index, so it survives an insn-count mismatch
  and prints insert/delete/replace hunks. This immediately exposed a residual class
  s1's index-by-index census had folded into "everything else is clean" (see below).
- tmp/grind/func_800770B8/s2/side.py - raw side-by-side listing over an index range
  (`side.py <lo> <hi>`), for reading a region without normalization.
- GOTCHA banked: side.py/posdiff.py read tmp/sandbox/func_800770B8/text1b.o, which is
  only refreshed by a `sandbox` run. After editing src/ you MUST re-run sandbox before
  diffing, or you will analyse the PREVIOUS probe's object (cost s2 one wrong analysis).
- FALSE DIFF: row 50 `addiu $2,$2,0` vs `addiu $v0,$v0,%lo(D_800A35D0)` is a normalizer
  gap (posdiff only rewrites `lui $N,0` into the reloc form, not `addiu`). Rows 49-51 are
  byte-clean. Do not chase it.

### Floor moves THIS session (all sandbox func_800770B8 --disable all)
- **14 -> 12: tail store grouping.** Target reads D_800A36A0 once for the `+0` (arg1)
  and `+0x65` stores; our per-statement global derefs reloaded it between them (a
  pointer store may alias the global), costing `lw` + a load-delay `nop`. The fix is
  ordinary C - one local holding the global's value for that pair:
  `{ u8 *q = D_800A36A0; *(s32 *)q = arg1; *(s8 *)(q + 0x65) = 0; }`.
  Target's tail grouping is exactly {+0, +0x65} | {+0x67} | {lbu +0x67, sb +0x66} -
  ONLY that pair groups, which is why s1's K2 (group the whole tail) measured 29.
- **12 -> 10: `a2 = 0;` relocated into the outer loop body.** Was: one `a2 = 0;` in the
  outer-loop preheader plus a duplicate at the outer-loop tail. Now: a single
  `a2 = 0;` as the FIRST statement of the do-body. This sinks `addu $a2,$zero,$zero`
  from sched1 slot 30 to slot 37 (after the 0x30/0x34 store cluster) exactly as
  target, and rows 30-34 + 37 become byte-clean. Pure statement-placement lever.

### Residual census at floor 10 (174 ours / 175 target)
A. **Prologue rows 7-12 (~3 rows).** ours: `sw $s1`@7, `addiu $s1,$s0,0x58`@8,
   `lui/lw a0`@9-10, `li a1`@11, `sw $ra`@12. target: `sw $ra`@7, `sw $s1`@8,
   `addiu a1,0x1008`@9, `lui/lw a0`@10-11, `addiu $s1,$s0,0x58`@12. Two sub-parts:
   (i) the addiu-$s1 slot follows C statement order (PROVEN - see
   rejected/s2-p_old-computed-after-clearotagr.c: moving the statement moves the insn
   from slot 8 to slot 11), and (ii) target evaluates the CONSTANT arg a1 BEFORE the
   a0 global load, ours does a0 first because its lui->lw chain carries the higher
   sched1 priority. (ii) is the real wall and is untouched.
B. **Rows 35-36 (2 rows).** ours stores 0x30/0x34 through $17/$s1 (p_old); target
   through $v0, the raw call-result pseudo, while $s1 (the copy) carries the
   D_800A36A0 gp store and the +4 store. .lreg confirms our call-result temp is
   "Register 80 used 2 times across 9 insns in block 0" - set + the copy into p_old,
   i.e. it dies at the copy; target needs it used 4 times. TWO independent spellings
   of a second C handle are now KILLED (K1 s1, K4 s2) - both collapse the build to
   169-171 insns because cse forwards the handle and deletes the D_800A35D0
   loop-preheader reload. Also measured byte-NEUTRAL: writing the last two stores as
   `*(s32 *)(D_800A36A0 + 0x30) = 0;` (a CSE'd load of the global) - cse resolves the
   load to p_old's pseudo, not to the call-result temp.
C. **Rows 62-64 (3 rows).** `addu` dest for the p_6a/p_7e base: ours `addu $2,$2,$3`,
   target `addu $v1,$v1,$v0` (dest = the offset pseudo, which absorbs the base).
   s1's K3 killed the row-pointer restructure (38) and measured association order
   byte-neutral; s2 additionally killed the int-domain address form (34, see
   rejected/s2-p6a-int-domain-address.c - the +0x6A/+0x7E constants fold into the int
   expression and destroy the shared (t0*10) CSE). Both directions dead => this is the
   local-alloc dest-coalesce / reg_n_deaths decision s1 flagged, NOT a spelling
   question. Next probe stays: .lreg "dies in N places" for the two pseudos.
D. **Row 104 (1 row + the entire 174-vs-175 count gap). MECHANISM PROVEN - below.**

### Class D: reorg delay-slot fill - mechanism nailed with the instrumented cc1
The instrumented cc1 (tools/gcc-2.7.2/cc1, NOT build/cc1) carries env-gated reorg
hooks: BB2_DBR_DEBUG=1 (decision trace), BB2_NO_FT_STEAL (skip fill_eager's
fall-through fill entirely) and BB2_ALLLIVE_LABEL=<uid[,uid...]> (force
mark_target_live_regs to its conservative everything-live answer for those target
uids, i.e. simulate find_basic_block()==-1). Driver scripts:
tmp/grind/func_800770B8/s2/run_idump.sh (instrumented -> s2/idumps/) and
s2/run_wdump.sh (what-if -> s2/wdumps/); trace in s2/dbr.err.

- The inner-loop-2 beqz is **jump_insn 314** (`eq (reg:SI 2 v0) 0`, label_ref 352) in
  s2/idumps/text1b.sched2. Its trace:
    DBRDBG thr insn=314 thread=327 opp=355 own=1 likely=0 tif=0 oppregs=200c0fe0_00000000 oppmem=1
    DBRDBG thr insn=314 trial=327 refset=0 setset=0 setneed=0 setsopp=0 trap=0
    DBRDBG thr WINNER insn=314 trial=327 annul=0
  Reading: likely=0 => mostly_true_jump returned 0 (reorg.c: an EQ test with equal
  rare_destination on both sides hits `case EQ: return 0`), so fill_eager_delay_slots
  takes its ELSE arm and tries the FALL-THROUGH thread FIRST. own=1 (own_fallthrough:
  no CODE_LABEL between the branch and the if-body). trial 327 is the if-body's first
  insn `ori $2,$4,1`; setsopp=0 means its dest $v0 is NOT in opposite_needed (bit 2
  clear in oppregs=0x200c0fe0) because the join's first insn `addu $2,$6,1` SETS $v0
  without reading it. So our steal is legal and wins.
- **What-if (decisive):** BB2_ALLLIVE_LABEL=355 reproduces the TARGET's shape exactly:
    ours   (normal) : beq $2,$0,.L901 / delay `ori $2,$4,0x0001`
    ours (all-live) : beq $2,$0,.L908 / delay `addu $2,$6,1` / then `ori $2,$4,0x0001`
    target          : beqz $v0,.L80077280 / delay `addiu $v0,$a2,0x1` / then `ori`
  and the insn count goes 174 -> 175, closing the count gap. See s2/wdumps/text1b.s.
- **Conclusion:** the target build's fall-through steal was BLOCKED. Per reorg.c only
  two things can block it for this shape:
    (a) own_fallthrough == 0 - a CODE_LABEL sits between the beqz and the if-body's
        first active insn (own_thread_p returns 0 on ANY code label when label==0).
        The target .s shows no surviving label there, but an `if (A || B)` C shape is
        the canonical way GCC materialises a label at a then-arm head; or
    (b) setsopp == 1 - the trial's dest is live at the branch target, which for this
        shape means mark_target_live_regs answering conservatively
        (find_basic_block()==-1: scan back from the join to the previous BARRIER, then
        fail to find a basic_block_head among the labels that follow it).
  Ordinary re-spellings cannot reach (b) directly; (a) is the C-reachable half and is
  NOT yet tried. This is the rule-era insert_label device residual.

- [s2] sandbox func_800770B8 --disable all = 10 (174/175) with the s2 candidate in place in src/text1b.c THIS session (14 at session start with the s1 body re-applied)

- [s2] the s1 candidate was NOT on HEAD at dispatch - re-applying it (body + BOTH caller-side edits) is required before any measurement; apply script tmp/grind/func_800770B8/s2/apply.py

- [s2] tail: target reads D_800A36A0 ONCE for the {+0, +0x65} store pair only; grouping exactly that pair through a local u8 *q is worth 2 insns (14 -> 12) and does not re-trigger s1's K2

- [s2] a2's initialiser belongs at the TOP of the outer loop body, not in the preheader plus a duplicate at the tail: 12 -> 10, sched1 slot 30 -> 37, rows 30-34/37 byte-clean

- [s2] class D mechanism PROVEN: BB2_ALLLIVE_LABEL=355 on the instrumented cc1 reproduces the target's beqz/delay-slot shape and closes the 174->175 count gap; ours wins the fall-through steal with likely=0 own=1 setsopp=0 (DBRDBG thr insn=314)

- [s2] second-handle spellings for the call result are dead in BOTH directions (K1 s1, K4 s2, both collapse to 169-171 insns); a CSE'd `D_800A36A0 + 0x30` load is byte-neutral

- [s2] int-domain address form KILLS rows 55-70 (34) - the s1 H4 int-domain trick does NOT generalise from row 43 to rows 62-64

- [s2] [s2] sandbox func_800770B8 --disable all = 10 (174 ours / 175 target) with the s2 candidate in place in src/text1b.c THIS session; the session opened at 14 after re-applying the s1 body.

- [s2] [s2] CHASSIS NOTE: the s1 candidate was NOT on HEAD at dispatch - src/text1b.c carried INCLUDE_ASM plus the ORIGINAL caller prototype `void func_800770B8(s32, s32 *, s32);` and a `(s32 *)&D_8009BD24` call arg. Re-applying the body AND both caller-side edits is a prerequisite for any measurement; script at tmp/grind/func_800770B8/s2/apply.py.

- [s2] [s2] Residual census at floor 10, four classes: (A) prologue rows 7-12, of which the addiu-$s1 slot is statement-order-controlled but the ClearOTagR a1-first argument evaluation is untouched; (B) rows 35-36, the 0x30/0x34 stores through $s1 (ours) vs $v0, the raw call-result pseudo (target); (C) rows 62-64, the p_6a/p_7e base addu destination register; (D) row 104, reorg's delay-slot fill choice, which is also the entire 174-vs-175 insn-count gap.

- [s2] [s2] .lreg says our call-result temp is `Register 80 used 2 times across 9 insns in block 0` - set plus the copy into p_old, so it dies at the copy. The target needs that pseudo used four times. Both second-handle spellings (K1 s1, K4 s2) collapse the build to 169-171 insns, so class B is an allocator question now, not a spelling question.

- [s2] [s2] Class D mechanism PROVEN, not inferred: BB2_ALLLIVE_LABEL=355 on the instrumented cc1 reproduces the target's beqz/delay-slot shape byte-for-byte in that region and closes the count gap; the DBRDBG trace shows our build wins the fall-through steal with likely=0, own=1, setsopp=0 on trial 327 (the `ori`).

- [s2] [s2] TOOLING: posdiff.py now aligns with difflib so it survives an insn-count mismatch (it previously crashed and mis-reported); s2/side.py gives a raw side-by-side over an index range. GOTCHA: both read tmp/sandbox/func_800770B8/text1b.o, which only a `sandbox` run refreshes - diffing after a src edit without re-running sandbox analyses the previous probe's object (this cost s2 one wrong analysis). FALSE DIFF: row 50 `addiu $2,$2,0` vs `addiu $v0,$v0,%lo(D_800A35D0)` is a posdiff normalizer gap; rows 49-51 are byte-clean.

- [s2] [s2] The instrumented cc1's reorg hooks are BB2_DBR_DEBUG (decision trace), BB2_NO_FT_STEAL (skip fill_eager's fall-through fill) and BB2_ALLLIVE_LABEL=<uid,...> (force mark_target_live_regs conservative). Runner scripts: tmp/grind/func_800770B8/s2/run_idump.sh and run_wdump.sh.

## Session 3 (structural, 2026-09-01) - floor 10 -> 9, insn count now EXACT (175/175)

### Chassis re-measurement
- The s2 candidate was NOT in src/text1b.c at dispatch (HEAD carried
  INCLUDE_ASM("asm/funcs", func_800770B8) plus the ORIGINAL caller prototype). s3
  re-applied the s2 body and both caller-side edits and re-measured **10**, so the
  s2 floor is chassis-valid on today's HEAD. Apply script for s3 is
  tmp/grind/func_800770B8/s3/try.py - unlike s2's apply.py it always starts from a
  pristine `git show HEAD:src/text1b.c` snapshot, which makes back-to-back variant
  measurements order-independent (s2's version mutated whatever was already in the
  tree).

### THE WIN: class D closed by a `for`-loop spelling of inner loop 2 (10 -> 9)
- s2 PROVED the class-D mechanism (reorg.c fill_eager_delay_slots steals the `ori`
  from the fall-through thread, whereas the target build copied
  `addiu $v0,$a2,1` out of the branch-target thread and redirected past it) and
  left two candidate blockers: own_fallthrough==0 (C-reachable) or an all-live
  mark_target_live_regs answer (not C-reachable).
- s3 first re-confirmed sufficiency mechanically from the OTHER knob: compiling the
  s2 body with the instrumented cc1 under BB2_NO_FT_STEAL=1 (reorg.c:3817, skips
  the fall-through fill entirely) reproduces the target region EXACTLY -
  `beq $2,$0,.L915` with delay `addu $2,$6,1`, then the `ori $2,$4,0x0001`, then
  the second `addu $2,$6,1`. So ANY blocker of the steal is worth the last insn.
- The C form that does it is ordinary: write inner loop 2 as
  `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) { ... }` in place of a preheader
  `a2 = 0;` plus `do { ... a2 = (s16)(a2 + 1); } while (a2 < 0xA);`.
  Measured 10 -> 9 and 174 -> 175 insns. Rows 103-105 and 113 become byte-clean and
  OUR INSN COUNT NOW EQUALS THE TARGET'S (175) for the first time in the grind.
- This does not contradict s2's H6. H6 concerns the OUTER loop body's `a2 = 0;`
  statement feeding inner loop 1, which remains the first statement of the outer
  do-body; the `for` header supplies the equivalent RTL position for the SECOND
  inner loop only. E1/E2/E3 (making the outer loop and/or inner loop 1 `for` loops
  as well) all measured byte-NEUTRAL at 9, so the do-while spelling is kept there
  because it is the smaller diff from the s2 form.

### PASS-ATTRIBUTION CORRECTION: class A is SCHED2, not sched1
- s2 recorded class A as a sched1 block-0 ordering question. That is wrong and it
  matters for how s4 attacks it. tmp/grind/func_800770B8/s3/idumps/text1b.sched
  shows sched1's block 0 still operating on PSEUDOS with no register-save insns at
  all; the `sw $s0/$s1/$s2/$s3/$ra` prologue stores are emitted by
  save_restore_insns after reload and first appear in text1b.sched2 as insns
  546..556.
- In sched2, block 0 is "from 546 to 81" and EVERY insn in the contested cluster is
  priority 1: the sp adjust (546), all five saves (548 ra, 550 s3, 552 s2, 554 s1,
  556 s0), the three incoming-arg copies (4, 6, 8), the p_old addiu (15), the
  ClearOTagR a0 load (26), the a1 constant (28) and the two `sh` zero stores
  (18, 21). With every priority equal, the cluster is settled entirely by
  rank_for_schedule's tie-breaks (dependence class vs last_scheduled_insn, then
  INSN_LUID) and by the "greater potential hazard" reordering - not by anything a C
  statement addresses directly.
- Emission order, ours vs target, after the three save/copy pairs at rows 1-6:
    ours   554(sw s1) 15(addiu s1) 26(lui/lw a0) 28(li a1)   548(sw ra)
    target 548(sw ra) 554(sw s1)   28(addiu a1)  26(lui/lw a0) 15(addiu s1)
  The target's order is ours exactly REVERSED. sched1's output order for the
  C-controllable subset is [15, 26, 28, 18, 21] and no statement permutation
  measured this session changed it.

### Class A: twelve statement orderings measured, all neutral or worse
- On the s2 chassis (base 10): A2 (sp inits, ClearOTagR, p_old, global) = 11;
  A3 (p_old, ClearOTagR, sp, global) = 12; A4 (ClearOTagR, p_old, sp, global) = 12;
  A5 (sp, p_old, ClearOTagR, global) = 10, NEUTRAL; A6 (named local
  `s32 ot = D_800A374C;` for the OT argument) = 11; A7/A8/A9 (the
  `D_800A35D8 = arg0;` store moved BEFORE ClearOTagR in three placements) =
  31/32/32 with the build collapsing to 172 insns.
- On the s3 chassis (base 9): A2 = 10, A5 = 9 (NEUTRAL), A6 = 10, A10 (p_old
  assigned after snd_StopAll) = 10, A11 (p_old assigned between the global store
  and snd_StopAll) = 10, A12 (sp inits moved last) = 13.
- The A7/A8/A9 collapse is itself a durable fact: moving the `D_800A35D8 = arg0;`
  store ahead of the ClearOTagR call lets cse forward $s0 and deletes three insns,
  so that store must stay AFTER the call in any future form.
- Statement ORDER around the ClearOTagR call is therefore a spent axis for class A.

### Class B: byte-neutral in every restatement (axis closed five ways)
- B1 (0x30/0x34 stores emitted BEFORE the `D_800A36A0 = p_old` and `+4` stores) =
  12 on the s2 chassis - it reorders the OUTPUT too and breaks rows 33-36 wholesale.
- B2 (stores spelled through `(u8 *)p_old` instead of through the global) = 10 on
  the s2 chassis and 9 on the s3 chassis: exactly NEUTRAL, cse resolves both
  spellings to the same pseudo.
- B3 (the `+4` store also spelled through the global) = 9, NEUTRAL.
- B4 (the 0x30 store spelled as the array form `p_old[12] = 0;`) = 9, NEUTRAL.
- With s1's K1 and s2's K4 (both directions of a second C handle) class B now has
  five measured spellings and no C-level lever: the target's $v0 use at rows 35-36
  requires the raw call-result pseudo to stay live past the copy into p_old, and
  every C form that names it twice makes cse forward the handle and DELETE the
  D_800A35D0 loop-preheader reload (a 5-insn collapse).

### Class C: six more address spellings measured; it is a local-alloc dest-coalesce
- The single decision is the dest of the p_6a/p_7e base add. Rows 60 and 61 are
  already byte-identical (`lw $v0,%gp_rel(D_800A36A0)` and `sll $v1,$v1,1`); only
  row 62 differs - ours `addu $2,$2,$3` (dest tied to the lw pseudo, operands
  (lw, sll)), target `addu $v1,$v1,$v0` (dest tied to the sll pseudo, operands
  (sll, lw)). Both input pseudos die at that insn.
- C1 - the source-order swap in the POINTER domain,
  `(t0 * 10) + D_800A36A0 + 0x6A` - measured NEUTRAL on both chassis (10 / 9).
  That positively CONFIRMS s1's H4 in its negative direction: a pointer-domain PLUS
  is canonicalized, so C source order cannot set the RTL operand order here.
- C2 (`s32 rowb = (t0*10) + (s32)D_800A36A0;` named int base) = 34 / 33;
  C3 (the same as a `u8 *row`) = 34 / 33; C4 (mixed int/pointer) = 34;
  C6 (constant folded onto the base first, `D_800A36A0 + 0x6A + (t0*10)`) = 13 with
  176 insns; C7 (`&((s16 *)(D_800A36A0 + 0x6A))[t0 * 5]`) = 13 with 176 insns.
- With s1's K3 and s2's K5 that is eight measured address shapes. Row 62 is not
  address-shaped; it is which dying input operand local-alloc ties the output to.

### Tooling
- tmp/grind/func_800770B8/s3/try.py - applies any candidate body to src/text1b.c
  from a pristine HEAD snapshot (tmp/grind/func_800770B8/s3/base_text1b.c).
- tmp/grind/func_800770B8/s3/m.ps1 - one PowerShell call: apply + sandbox, printing
  score and build_insns. Use it for variant sweeps; it costs one turn per batch.
- tmp/grind/func_800770B8/s3/v/*.c - every variant measured this session, kept so a
  future chassis can re-measure them without re-deriving the shapes.
- tmp/grind/func_800770B8/s3/idumps/ - full cc1 -da dump set for the floor-9 form.


- [s3] sandbox func_800770B8 --disable all = 9 (175 ours / 175 target) with the s3 candidate in place in src/text1b.c THIS session; the session opened at 10 after re-applying the s2 body plus both caller-side edits.

- [s3] THE INSN-COUNT GAP IS CLOSED: ours is 175 insns, the target is 175. Every remaining diff is a register-choice or emission-order diff, not a missing or extra instruction.

- [s3] Class D is CLOSED by ordinary C: writing inner loop 2 as `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1))` blocks reorg's fall-through delay-slot steal, so the beqz is filled from the branch-target thread by copy+redirect exactly as the target. 10 -> 9, 174 -> 175 insns, rows 103-105 and 113 byte-clean.

- [s3] BB2_NO_FT_STEAL=1 on the instrumented cc1 reproduces the same region independently of BB2_ALLLIVE_LABEL, so ANY blocker of the fall-through steal suffices - s2's two candidate routes were equivalent in effect.

- [s3] PASS-ATTRIBUTION CORRECTION: class A is a SCHED2 (post-reload) decision, not sched1. The prologue register saves are emitted by save_restore_insns after reload and do not exist in sched1's RTL at all; text1b.sched2 block 0 is 'from 546 to 81' and every insn in the contested cluster has priority 1.

- [s3] Target's block-0 emission order for the contested cluster is ours exactly REVERSED: target 548(sw ra), 554(sw s1), 28(li a1), 26(lui/lw a0), 15(addiu s1) vs ours 554, 15, 26, 28, 548.

- [s3] Class A statement-order axis is SPENT: twelve orderings measured across two chassis, best is byte-neutral (A5). The `D_800A35D8 = arg0;` store must stay AFTER the ClearOTagR call - moving it earlier lets cse forward $s0 and collapses the build by 3 insns (31/32/32).

- [s3] Class B is closed at the C level with five measured spellings (K1 s1, K4 s2, B1/B2/B3/B4 s3): B2/B3/B4 are exactly byte-NEUTRAL, so no restatement of the store expressions reaches the raw call-result pseudo.

- [s3] Class C is closed at the address-shape level with eight measured shapes (K3 s1, K5 s2, C1/C2/C3/C4/C6/C7 s3). C1 (pointer-domain source-order swap) is NEUTRAL, positively confirming that pointer-domain PLUS is canonicalized; the row-62 diff is a local-alloc dest-coalesce between two dying input pseudos.

- [s3] KILLED as class-D alternatives: the short-circuit `||` label device (32, 176 insns - the second test is really emitted), the `== mask` NE-branch device aimed at mostly_true_jump's prediction path (29), and goto-into-the-arm (byte-neutral - jump.c folds the pair and the label does not survive to reorg).

- [s3] TOOLING: tmp/grind/func_800770B8/s3/try.py applies a candidate from a pristine `git show HEAD:src/text1b.c` snapshot (s2's apply.py mutated the live tree, making back-to-back variant measurements order-dependent); m.ps1 does apply+sandbox in one PowerShell call; every measured variant is kept under s3/v/.

- [s3] sandbox func_800770B8 --disable all = 9 (175 ours / 175 target) with the s3 candidate in place in src/text1b.c THIS session; the session opened at 10 after re-applying the s2 body plus both caller-side edits (HEAD carried INCLUDE_ASM and the original caller prototype, same as at s2 dispatch).

- [s3] THE INSN-COUNT GAP IS CLOSED: ours is 175 insns and the target is 175. Every one of the 9 remaining diffs is a register-choice or emission-order diff, not a missing or extra instruction.

- [s3] Class D is CLOSED by ordinary C: `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1))` for inner loop 2 blocks reorg's fall-through delay-slot steal, so the beqz is filled from the branch-target thread by copy+redirect exactly as the target (rows 103-105 and 113 byte-clean).

- [s3] BB2_NO_FT_STEAL=1 on the instrumented cc1 reproduces the same region independently of s2's BB2_ALLLIVE_LABEL what-if, so the two candidate routes were equivalent in effect and any blocker of the fall-through steal suffices.

- [s3] PASS-ATTRIBUTION CORRECTION (s2 got this wrong): class A is a SCHED2 (post-reload) decision. The prologue register saves are emitted by save_restore_insns after reload and are absent from sched1's RTL; text1b.sched2 block 0 is 'from 546 to 81' with every contested insn at priority 1 (546 sp-adjust, 548 sw ra, 550 sw s3, 552 sw s2, 554 sw s1, 556 sw s0, 4/6/8 arg copies, 15 addiu s1, 26 a0 load, 28 a1 const, 18/21 sh zero stores).

- [s3] The target's block-0 emission order for the contested cluster is ours exactly REVERSED: target 548, 554, 28, 26, 15 vs ours 554, 15, 26, 28, 548.

- [s3] Class A statement-order axis is SPENT: twelve orderings measured across two chassis, best byte-neutral. The `D_800A35D8 = arg0;` store must stay AFTER the ClearOTagR call - moving it earlier lets cse forward $s0 and collapses the build by 3 insns (scores 31/32/32).

- [s3] Class B is closed at the C level with five measured spellings (K1 s1, K4 s2, B1-B4 s3); B2/B3/B4 are exactly byte-neutral, so no restatement of the store expressions reaches the raw call-result pseudo.

- [s3] Class C is closed at the address-shape level with eight measured shapes (K3 s1, K5 s2, C1/C2/C3/C4/C6/C7 s3). C1 being neutral positively confirms that a pointer-domain PLUS is canonicalized, so C source order cannot set the RTL operand order at row 62.

- [s3] Sibling evidence: src/text1b_b.c:922 (func_800784E4, already matched) makes the same `ClearOTagR(D_800A374C, 0x1008);` call and its target asm emits the a0 lui/lw chain BEFORE the a1 constant - i.e. our order, not func_800770B8's. So the a1-first order in this target is not a property of the call, it is a property of this block's sched2 tie-breaks.

- [s3] TOOLING: tmp/grind/func_800770B8/s3/try.py applies a candidate from a pristine `git show HEAD:src/text1b.c` snapshot (s2's apply.py mutated the live tree, which made back-to-back variant measurements order-dependent); tmp/grind/func_800770B8/s3/m.ps1 does apply+sandbox in one PowerShell call; all 24 measured variants are kept under tmp/grind/func_800770B8/s3/v/.

## [s4] permuter modality — the class-A sched2 fence and the p_old/ClearOTagR coupling

Chassis re-measured at session start: the s3 candidate.c form measures **9** on
today's chassis (175/175 insns), identical to the s3 ledger entry. Nothing drifted.

### Workspace (first permuter run this function has ever had)
`tools/decomp-permuter/nonmatchings/func_800770B8_s4/` (and `_s4B/`) — hand-built,
NOT `import.py`. base.c is a standalone TU: the six typedefs plus the seven
`extern` decls that already sit above the function in src/text1b.c (lines
6396-6407), then the function body verbatim. compile.sh mirrors the engine's
non-GP pipeline (cc1 `-O2 -G0 ... -mel` → prologue_fix → maspsx with the seven
standard flag groups → multu_pad → as). target.o is built from
`asm/funcs/func_800770B8.s` with a `gp=64`-stripped prelude so the function sits
at offset 0.

GOTCHA worth reusing: `compile.sh` must NOT pass a `realpath`'d input to cpp.
The repo path contains a space, so cc1 emits `.file 1 "/mnt/c/.../Bushido Blade 2
Decompile/..."` and maspsx dies with `too many values to unpack (expected 3)`
while printing only `MASPSX: An exception occurred:` — the object silently comes
out empty (0 insns). Fix: `sed 's|^\t\.file.*|\t.file\t1 "base.c"|'` between cc1
and prologue_fix. Validated: base 175 insns vs target 175, and the objdump diff
is exactly the ledger's classes A (3 rows) + B (2 rows) + C (3 rows).

### The finds
Campaign A (label `s4-random-floor9`, base permuter score 372, 16166 iters, 7
finds): the dominant, repeatedly-rediscovered lever is a bare
`do { } while (0);` inserted in the prologue, and it pays MORE the earlier it
sits — score 155 immediately before `p_old = (s32*)(arg0+0x58);`, 249 after it,
309 after the `sp[]` inits.

Sandbox measurements of that lever (honest metric, not permuter score):

| form | sandbox | insns |
|---|---|---|
| s3 candidate (baseline) | 9 | 175 |
| + `do { } while (0);` immediately before the `p_old` assign (V1) | **7** | 175 |
| + `do { } while (0);` after the `p_old` assign (V4) | 11 | 175 |
| real `do { p_old=…; sp[0]=0; sp[1]=0; } while (0);` (V2) | 11 | 175 |
| real `do { p_old=…; } while (0);` (V3) | 9 | 175 |
| inner brace scope holding `p_old`/`r` (V7) | 9 | 175 |
| two nested inner brace scopes (V8) | 9 | 175 |

V1 closes the `sw $ra,56($sp)` row: with it, sched2 emits all five
register-save stores contiguously and only the `addiu $s1,$s0,88` / `li $a1,4104`
pair remains swapped (objdump diff 14 half-rows → the sandbox's 9 → 7).

### Pass attribution (dumps, not a guess)
`tmp/grind/func_800770B8/s4/dumps/{cand,v1}/in.i.*` (full cc1 `-da` set for both).
- The loop notes are present from `.rtl` onward and survive to `.dbr` (+6 lines
  at every stage). Basic-block structure is IDENTICAL in both (14 blocks, block 0
  is `558→81` vs `570→93`), so the notes do **not** split a scheduling region.
- `in.i.sched2` block 0, ours: `… insn 566 (sw $s2) / insn 15 (addiu $s1) /
  insn 26 (a0) / insn 28 (a1) / insn 560 (sw $ra) / NOTE_INSN_PROLOGUE_END`.
  With V1: `… insn 578 (sw $s2) / insn 568 (sw $ra) / NOTE_INSN_PROLOGUE_END /
  NOTE_INSN_LOOP_BEG 583 / NOTE_INSN_LOOP_END 582 / insn 27 (addiu $s1) /
  insn 38 (a0) / insn 40 (a1)`.
  i.e. the note pair sits BETWEEN the reload-emitted saves and the first three
  body insns and stops sched2 from interleaving them.
- NOTE_INSN_BLOCK_BEG/END notes do NOT do this: in both dumps every block note
  has migrated to the very top of the function ahead of the first prologue insn,
  which is why V7/V8 (real inner scopes, ordinary C) are exactly byte-neutral.
  Only LOOP notes stay anchored mid-block (sched.c reattaches them via REG_NOTES
  in `reemit_notes`).

### The coupling — why s3's twelve statement orderings all measured dead
s3 measured A2-A12 (orderings of the `p_old` assign / `sp[]` inits / ClearOTagR
call / `D_800A35D8` store / `snd_StopAll` call) on an UNFENCED chassis and got
neutral-or-worse everywhere. Campaign B (seeded on the V1 chassis, base score
155) found score **35** by combining the fence with exactly one of those
orderings — `p_old = (s32*)(arg0+0x58);` moved to AFTER the ClearOTagR call.
Measured both ways:

| form | sandbox |
|---|---|
| honest s3 chassis + `p_old` moved after ClearOTagR (V9) | **10** (worse) |
| V1 fence + `p_old` moved after ClearOTagR (V10) | **5** |

So the statement-order axis for class A is **NOT dead** — it was MASKED. The
reorder is only profitable once sched2's prologue window is fenced; unfenced it
costs a row. s3's "twelve orderings measured dead" conclusion is hereby scoped
to the unfenced chassis only, and the whole ordering set is worth re-running on
any chassis that carries a legitimate fence.

### Two further measurements bounding the fence's shape
- `do { <entire remaining function body> } while (0);` + the `p_old` move (V11):
  **39**, 176 insns — a whole-body wrap is far worse; the note must sit at the
  prologue boundary specifically, not merely at function scope.
- real `do { sp[0]=0; sp[1]=0; ClearOTagR(D_800A374C,0x1008); } while (0);` on
  the `p_old`-moved chassis (V12): **8**, 174 insns. This is the only
  non-empty-bodied wrap that beats the honest floor, and it is one insn SHORT of
  the target's 175, so it is not on the path to 0 as written.

### Disposition of the winning forms
V1/V10 (floor 7 / floor 5) are CHEATS and are banked as rejected/, not as
candidate.c. An empty-bodied `do { } while (0);` whose entire measured effect is
to stop sched2 from interleaving reload-emitted save stores with body insns is a
scheduling barrier by construction: it fails T1 (no observable effect on the
function's output), T2 (no human writes an empty do-while at a function's top),
T3 (the mechanism IS a named GCC pass's behaviour and nothing else) and T6.
`.claude/rules/do-while-zero-exception.md` does not cover it either — that
carve-out is scoped to the LABEL_OUTSIDE_LOOP_P / reorg.c interaction, and this
is sched2. First reach of an unsanctioned family ⇒ not submittable.
candidate.c therefore remains the s3 form at floor 9.

- [s4] Chassis check: the s3 candidate.c form re-measures 9 (175/175 insns) on today's chassis, identical to the s3 ledger entry - nothing drifted.

- [s4] This is the FIRST permuter run this function has ever had; the ledger contained no permuter evidence at all before s4.

- [s4] Workspaces tools/decomp-permuter/nonmatchings/func_800770B8_s4 and _s4B are hand-built (not import.py): base.c is a standalone TU of the six typedefs plus the seven extern decls already sitting above the function in src/text1b.c, then the body verbatim; compile.sh mirrors the engine's non-GP pipeline (cc1 -O2 -G0 ... -mel, prologue_fix, maspsx with the seven standard flag groups, multu_pad, as); target.o is built from asm/funcs/func_800770B8.s with a gp=64-stripped prelude so the function sits at offset 0. Validated: 175 base insns vs 175 target, and the objdump diff is exactly the ledger's classes A (3 rows) + B (2 rows) + C (3 rows).

- [s4] REUSABLE TOOLING GOTCHA: a permuter compile.sh for this repo must NOT hand cpp a realpath'd input. The repo path contains a space, so cc1 emits a .file directive containing spaces, maspsx dies with 'too many values to unpack (expected 3)' while printing only 'MASPSX: An exception occurred:', and the .o comes out silently EMPTY (0 insns) while the pipeline still exits 0. Fix: sed the .file line to a constant between cc1 and prologue_fix.

- [s4] Campaign A telemetry: base permuter score 372, 16166 iterations, 7 finds (155/249/275x2/309/355x2), best 155; the do-while(0) construct was rediscovered from three independent seeds and pays MORE the earlier it sits (155 before the p_old assign, 249 after it, 309 after the sp[] inits).

- [s4] Campaign B telemetry (seeded on the fenced chassis): base permuter score 155, 17760 iterations, 2 finds (95, 35), best 35 = the fence plus p_old moved after ClearOTagR plus two further dead constructs the permuter piled on top.

- [s4] Sandbox measurement table (honest metric, all 175 insns unless noted): s3 candidate 9 | V1 empty do-while(0) before the p_old assign 7 | V4 same after the p_old assign 11 | V2 real wrap of the first three statements 11 | V3 real wrap of the p_old assign only 9 | V7 inner brace scope 9 | V8 two nested inner scopes 9 | V9 p_old moved after ClearOTagR unfenced 10 | V10 fence + p_old moved 5 | V11 whole-body do-while(0) wrap + p_old moved 39 (176 insns) | V12 real wrap of sp[0]/sp[1]/ClearOTagR on the moved chassis 8 (174 insns).

- [s4] A measured honest distance of 5 exists for this function's C (V10), which is direct evidence that classes A and B are not jointly foreclosed - the residual at 5 is smaller than any class-partition argument previously banked.

- [s4] Every s4 form that beats the honest floor is banked under memory/grind/func_800770B8/rejected/ with a name that states why it is dead; candidate.c is byte-identical to the s3 body and still measures 9. src/text1b.c was reverted to HEAD (INCLUDE_ASM) at session end.

- [s4] Both campaigns were harvested with --stop before the session ended; pgrep confirms no surviving permuter process.

## [s5] synthesis modality — the loop-note anchor is killed, and the residual is re-attributed to RA

Chassis re-measured at session start with candidate.c applied to src/text1b.c:
**9** (175 ours / 175 target). Unchanged from s3 and s4; nothing drifted. The
dispatch brief's "measurement unavailable" was a driver-side gap, not drift.

### 1. The s4 frontier's #1 hypothesis (an ORDINARY-C loop-note anchor) is KILLED
s4 proved that only NOTE_INSN_LOOP_BEG/END notes stay anchored mid-block at
sched2 (block notes all migrate to the top of the function), so an honest fence
had to be a REAL loop placed as the function's first statement. Four spellings
were built and measured (all kept in tmp/grind/func_800770B8/s5/v/):

| form | sandbox | insns |
|---|---|---|
| candidate.c (baseline) | 9 | 175 |
| P1 `for (t0=0;t0<2;t0++) sp[t0]=0;` first, then p_old, then ClearOTagR | 42 | 186 |
| P2 same loop first, ClearOTagR, then p_old (the V10 pairing) | 41 | 186 |
| P3 p_old first, then the loop | 42 | 186 |
| P4 `while` spelling of the same loop | 42 | 186 |

MECHANISM HALF CONFIRMED — a real first-statement loop DOES anchor its note pair
at exactly the V1 fence position. tmp/grind/func_800770B8/s5/dumps/p1/in.i.sched2:

    (note 599 611 16 "" NOTE_INSN_PROLOGUE_END)
    (note  16 599 25 "" NOTE_INSN_LOOP_BEG)

versus the baseline dump (tmp/grind/func_800770B8/s5/dumps/cand/in.i.sched2),
where the first LOOP_BEG is at line 682, far past the prologue. Ordinary C CAN
reach the anchor.

COST HALF FATAL — it cannot pay for it. A genuine loop over `sp[]` forces the two
`sh $zero` stores onto a computed stack address (`addu $3,$sp,16` plus a sll/sra
of the index) and adds loop control: **+11 insns, 175 -> 186**. Our insn count
has EQUALLED the target's 175 since s3, so any construct that adds instructions
is off the path to 0 by construction. The only zero-insn loop is an empty one,
which is precisely the s4 cheat.
Banked: rejected/s5-real-loop-note-anchor-insn-cost.c

### 2. THE DECISIVE COROLLARY — the target has no fence, so the fence is a red herring
asm/funcs/func_800770B8.s rows 1-14 are straight-line: five saves interleaved
with three arg copies, then `addiu $a1,$zero,0x1008`, `lui/lw $a0`,
`addiu $s1,$s0,0x58`, two `sh $zero`, then the ClearOTagR jal. There is no
branch, no label, and therefore NO LOOP anywhere near the prologue in the
original. The target's contiguous save emission is consequently NOT produced by
a loop-note fence. The `do { } while (0);` of s4 is a COINCIDENTAL route to the
same emission order, not the original mechanism — a second, independent reason
(beyond the cheat ruling) not to spend further sessions respelling it. It also
retires the s4 frontier item that proposed a ruling-request on extending the
do-while(0) carve-out from reorg.c to sched2: there is nothing to extend the
carve-out TO, because the construct does not reconstruct what the original
source did. That ruling-request is withdrawn before it was ever filed.

### 3. Frame layout is identical, which narrows class A to pure emission order
Target `addiu $sp,$sp,-0x40` with saves at 0x28/0x2C/0x30/0x34/0x38
(s0/s1/s2/s3/ra). Ours `.frame $sp,64,$31 # vars=24, regs=5/0, args=16` with
`sw $16,40 / $17,44 / $18,48 / $19,52 / $31,56`. Byte-for-byte the same frame,
save set and slots. Class A has no frame-shape component at all:
  target: sw $ra | sw $s1 | addiu $a1 | lui/lw $a0 | addiu $s1,$s0,0x58
  ours:   sw $s1 | addiu $s1,$s0,0x58 | lw $a0 | li $a1 | sw $ra

### 4. THE RE-ATTRIBUTION — the object-level classifier says the residual is RA
`tools/ra_solver/goal_from_tgt.py classify text1b func_800770B8`:

    func_800770B8 (text1b): ours 175 insns, target 175 insns
      [object-level: replace_with_asmfile-safe]
    FIRST DIVERGENCE: RA
      $v0 -> $v1   x4
      $s1 -> $v0   x2
      $v1 -> $v0   x1

5 renamed pairs, 2 pairs skipped as "skeleton differs — reloc/immediate, not a
rename" (those two are the class-A ordering rows). SEVEN of the residual
half-rows are register naming, not scheduling. Three sessions of sched2 work
were aimed at the smaller half of the residual.

Corroborating, from the other solver: `tools/sched_solver/perturb.py --pass 2
--goal-from-target text1b --target-object build/src/text1b.o --ours-object
tmp/sandbox/func_800770B8/text1b.o --atoms luid,luid_move --depth 2` prints
NOTHING for func_800770B8 — i.e. no block in pass 2 has a goal that differs from
our emission order — while `--self-check` reports "priority recomputation:
14250/14250 exact (100.00%)", so the model is live and the silence is a real
verdict, not a crash. CAVEAT to inherit honestly: goalmap could not align the
two "skeleton differs" rows as renames and holds them at OUR positions, so this
is evidence that the ordering residual is not independently expressible as a
sched2 goal, NOT a proof that the emission order is identical.

### 5. CLASS B IS MECHANICALLY FORECLOSED AT THE C LEVEL (the session's hard result)
Attribution onto pseudos (`goal_from_tgt.py goal ... --model
tmp/ra_solver_work/func_800770B8.model.json`) resolves the `$s1 -> $v0 x2`
substitution UNIQUELY to pseudo 75 — p_old — giving `goal: {"75": 2}`. Fed to
the inverse solver:

    python3 tools/ra_solver/inverse.py global \
        tmp/ra_solver_work/func_800770B8.model.json --goal '{"75": 2}' \
        --depth 2 --top 8

    FORECLOSED — 3 preference atom(s) NOT emitted (mechanically unreachable from C):
      pseudo 75: preference for $v0
          pseudo 75 crosses 4 call(s) and $v0 is call-used, so
          prune_preferences (global.c:897) strips it from this allocno's
          preferences before find_reg ever sees it.
    NEGATIVE RESULT: no perturbation of any modelled input, up to depth 2,
    reaches the target assignment.
    (161 single perturbations over 6 classes; refs delta +12/-6, live length +/-2,4,8.)

Read semantically: the target's 0x30/0x34 stores are based on a pseudo whose life
ENDS before the next call, so it is eligible for the call-clobbered $v0. Our
`p_old` is one C variable spanning the whole function and crossing four calls, so
global.c's `prune_preferences` deletes $v0 from its preference list before
`find_reg` runs. No amount of refs / live-span / birth-order / conflict /
preference / calls-crossed perturbation can give it $v0. This is not a spelling
that has not been found yet; it is a spelling that cannot exist for THIS pseudo.

The only route to the target's bytes is a SECOND pseudo holding the raw
`func_8006E49C` result across the two stores and dying before the loop — exactly
the "second handle" family, now SIX measured spellings deep and dead every time
(s1 K1 = 28 with an insn collapse, s2 K4, s3 B1 = 12, B2/B3/B4 byte-neutral):
cse forwards the handle and deletes the D_800A35D0 loop-preheader reload.

Note precisely what the solver did and did not say. It classified the seat as
unreachable BY PERTURBING THE MODELLED INPUTS OF PSEUDO 75'S ALLOCATION. It
explicitly names the mechanisms still outside the model — the local-alloc
SUGGESTED-REGISTER pass (`qty_phys_copy_sugg` / `qty_phys_sugg`, reported but not
scored today), `qty_size` for DImode, and reload's spill-retry — and says the
next move there is INSTRUMENTATION, not another spelling search. That is the
honest boundary of this result.

### 6. What this leaves
- Class A (2 half-rows, sched2 emission order): the only honest fence would be a
  real loop; real loops cost >= 11 insns here; the target has no loop; and the
  sched solver reports no differing pass-2 goal. No ordinary-C lever is left
  unmeasured on this axis.
- Class B (2 half-rows, pseudo 75 / $v0): FORECLOSED by prune_preferences on the
  rename reading; the second-pseudo reading is six measured C spellings dead.
- Class C (the $v0 <-> $v1 substitutions, rows 62-64): still a local-alloc
  dest-coalesce question and the ONE typed-verdict axis still unspent. It was NOT
  run this session: `inverse.py local` needs `<stem>.local.json` from
  `tools/ra_solver/local_extract.py`, which was not built here. This is the
  correct first move for s6, and its inputs are now all in place.

### Artifacts
- tmp/grind/func_800770B8/s5/v/P1..P4.c — the four real-loop forms
- tmp/grind/func_800770B8/s5/dumps/{cand,p1}/ — full cc1 -da dump sets
- tmp/grind/func_800770B8/s5/{hdr.c,mkdump.py,dump.sh} — standalone-TU dump
  harness rebuilt this session. NOTE: s4/dump.sh no longer works — it depends on
  the s4 permuter workspaces (gone; permuter dirs are gitignored) and must be run
  under WSL, not Git Bash. Use
  `bash tools/wsl.sh 'bash tmp/grind/func_800770B8/s5/dump.sh <cand.c> <tag>'`.
- tmp/grind/func_800770B8/s5/sched_solver/ — scratch COPY of tools/sched_solver
  carrying a one-function patch (below). tools/ is not an editable surface for a
  grind session, so the fix lives in scratch.
- tmp/ra_solver_work/func_800770B8.model.json — the RA model (21 pseudos, 78
  dispositions)

### REUSABLE TOOLING GOTCHA (worth a permanent fix by the operator)
`tools/sched_solver/goalmap.py::_macro_expand_counts` under-counts the honest text
stream for any function containing `<mem> $r,SYM($base)` — a bare symbol WITH an
index register. GNU as assembles that to THREE insns (lui $at,%hi /
addu $at,$at,$base / <mem> $r,%lo($at)); the helper scores it 1 because the last
operand contains a `(`. func_800770B8 has two such lines (`sb $4,D_8009BCE4($3)`
and `sb $2,D_8009BCE4($3)`), so the same-source checksum failed with "honest
object has 175 insns but text1b.hon.s body has 169 lines (macro-expanded estimate
171)" — a message that blames the user for a stale build when the tree is in fact
consistent. The scratch copy in tmp/grind/func_800770B8/s5/sched_solver/goalmap.py
carries the three-insn case and makes the object-level goal path work.

- [s5] Chassis check: candidate.c re-measures 9 (175/175) on today's chassis; no drift from s3/s4.
- [s5] KILLED the s4 frontier's #1 hypothesis: four real first-statement loops (P1-P4) measure 41-42 at 186 insns. A real loop DOES anchor NOTE_INSN_LOOP_BEG at the V1 fence position (dumped and read), but costs +11 insns, and our count already equals the target's 175, so no real loop is on the path to 0.
- [s5] DECISIVE: the TARGET has no loop, label or branch anywhere in its prologue (asm/funcs/func_800770B8.s rows 1-14 are straight-line), so its contiguous save emission is NOT produced by a loop-note fence. The s4 do-while(0) is a coincidental route to the same order, not the original mechanism — which independently retires the proposed ruling-request on extending the do-while(0) carve-out to sched2.
- [s5] Target and ours have byte-identical frames (0x40; saves s0/s1/s2/s3/ra at 0x28/0x2C/0x30/0x34/0x38), so class A has no frame-shape component; it is purely the emission order of five insns.
- [s5] RE-ATTRIBUTION: goal_from_tgt.py classify reports FIRST DIVERGENCE = RA with $v0->$v1 x4, $s1->$v0 x2, $v1->$v0 x1 — 5 renamed pairs plus 2 skipped as non-renames (the class-A order rows). Seven of the residual half-rows are register naming, not scheduling.
- [s5] CLASS B IS FORECLOSED at the C level: the $s1->$v0 x2 substitution attributes uniquely to pseudo 75 (p_old), and inverse.py global returns FORECLOSED — pseudo 75 crosses 4 calls and $v0 is call-used, so prune_preferences (global.c:897) strips the $v0 preference before find_reg runs. 161 single perturbations over 6 classes, depth 2, NEGATIVE.
- [s5] The solver names what is still OUTSIDE its model for that seat — the local-alloc suggested-register pass (qty_phys_copy_sugg / qty_phys_sugg, reported-not-scored), qty_size for DImode, and reload's spill-retry — and says the next move there is instrumentation, not another spelling search.
- [s5] sched_solver perturb (pass 2, object-level goal) prints NO differing block for this function while --self-check reports 14250/14250 exact priority recomputation. Caveat: goalmap holds the two non-alignable rows at OUR positions, so this is evidence the ordering residual is not independently expressible as a sched2 goal, not proof the emission order matches.
- [s5] `inverse_compose.py classify` REFUSES on this function (zero-rule: the src-derived tgt.s cannot carry the target stream) and names `goal_from_tgt.py classify` as the object-level replacement. Future sessions should skip straight to goal_from_tgt.
- [s5] TOOLING BUG (scratch-patched; tools/ is not editable in a grind session): goalmap.py::_macro_expand_counts scores `<mem> $r,SYM($base)` as 1 insn where GNU as emits 3 (lui/addu/mem), so the same-source checksum falsely reports a stale build. func_800770B8 hits it twice via `sb $r,D_8009BCE4($3)`. Patched copy: tmp/grind/func_800770B8/s5/sched_solver/goalmap.py.

- [s5] Chassis check: candidate.c re-measures sandbox --disable all = 9 (175 ours / 175 target) on today's chassis, identical to s3 and s4. The dispatch brief's 'measurement unavailable' was a driver-side gap, not drift.

- [s5] Four real first-statement loops (P1-P4) measure 41/42 at 186 insns against the 9 / 175 baseline: a genuine loop over sp[] costs +11 insns here.

- [s5] A real first-statement loop DOES anchor its note pair at the V1 fence position - tmp/grind/func_800770B8/s5/dumps/p1/in.i.sched2 has NOTE_INSN_PROLOGUE_END immediately followed by NOTE_INSN_LOOP_BEG, where the baseline dump's first LOOP_BEG is 137 lines further on.

- [s5] DECISIVE: asm/funcs/func_800770B8.s rows 1-14 (the target's own prologue) are straight-line with no branch, no label and no loop, so the target's contiguous save emission is not produced by a loop-note fence. The s4 do-while(0) is a coincidental route to the same order, not the original mechanism.

- [s5] Target and ours have byte-identical frames (0x40; saves s0/s1/s2/s3/ra at 0x28/0x2C/0x30/0x34/0x38), so class A has no frame-shape component - it is purely the emission order of five insns.

- [s5] goal_from_tgt.py classify reports FIRST DIVERGENCE = RA with $v0->$v1 x4, $s1->$v0 x2, $v1->$v0 x1 (5 renamed pairs, 2 skipped as non-renames). Seven of the nine residual half-rows are register naming, not scheduling - three sessions of sched2 work were aimed at the smaller half.

- [s5] CLASS B FORECLOSED: the $s1->$v0 x2 substitution attributes uniquely to pseudo 75 (p_old); inverse.py global returns NEGATIVE over 161 single perturbations in 6 classes at depth 2, because pseudo 75 crosses 4 calls and $v0 is call-used, so prune_preferences (global.c:897) strips the $v0 preference before find_reg runs.

- [s5] The mechanisms still outside the RA model for that seat are named by the solver itself: the local-alloc suggested-register pass (qty_phys_copy_sugg / qty_phys_sugg, reported but not scored), qty_size for DImode, and reload's spill-retry. Its stated next move is instrumentation, not another spelling search.

- [s5] sched_solver perturb (pass 2, object-level goal) prints no differing block for func_800770B8 while --self-check reports 14250/14250 exact priority recomputation. Caveat: goalmap holds the two non-alignable rows at our positions, so this is evidence rather than proof.

- [s5] TOOLING: inverse_compose.py classify REFUSES on this function (zero-rule, so the src-derived tgt.s cannot carry the target stream) and names goal_from_tgt.py classify as the object-level replacement - future sessions should skip straight to goal_from_tgt.

- [s5] TOOLING BUG (scratch-patched; tools/ is not an editable surface here): goalmap.py::_macro_expand_counts scores `<mem> $r,SYM($base)` as 1 insn where GNU as emits 3 (lui $at,%hi / addu $at,$at,$base / mem $r,%lo($at)), so the same-source checksum falsely reports a stale build. func_800770B8 hits it twice via `sb $r,D_8009BCE4($3)` (169 text lines, estimate 171, object 175). Patched copy at tmp/grind/func_800770B8/s5/sched_solver/goalmap.py; the operator may want the fix upstream.

- [s5] TOOLING: tmp/grind/func_800770B8/s4/dump.sh no longer works - it depends on the s4 permuter workspaces (gone; permuter dirs are gitignored) and must run under WSL, not Git Bash. Replacement harness: tmp/grind/func_800770B8/s5/{hdr.c,mkdump.py,dump.sh}, invoked as `bash tools/wsl.sh 'bash tmp/grind/func_800770B8/s5/dump.sh <cand.c> <tag>'`.

- [s5] src/text1b.c was reverted to HEAD (INCLUDE_ASM) at session end; the tree carries only ledger edits plus metrics/events.jsonl. No permuter or solver process survives the session.

## Session 6 (synthesis, 2026-09-01) — floor 9 (unchanged); class C RE-ATTRIBUTED

Chassis re-measured at session start with candidate.c applied: score 9,
build_insns 175, target_insns 175. Identical to the s3/s4/s5 chassis, so every
banked spelling conclusion below is still chassis-current.

### The headline: class C is NOT a register-allocation question
s5 handed s6 a frontier whose item 1 said "class C is a local-alloc
dest-coalesce decision; run inverse.py local". That framing is WRONG, and this
session disproves it from the RTL rather than from a model. The correct
attribution is **RTL plus operand order at insn 173** (an expand/fold-level
property of the C expression), and the register naming at rows 62-64 is a
downstream CONSEQUENCE, not the cause.

Proof, from tmp/grind/func_800770B8/dumps/text1b.lreg (regenerated this session
off the floor-9 body) and the positional diff:

    rows 60/61 are BYTE-IDENTICAL on both sides:
        60  lw   $v0, %gp_rel(D_800A36A0)($gp)     (insn 171, reg 109)
        61  sll  $v1, $v1, 1                       (insn 169, reg 108)
    row 62 differs only in the plus's operand order:
        ours    addu $2,$2,$3     = (plus 109 108)  -> dest ties to reg 109 ($v0)
        target  addu $v1,$v1,$v0  = (plus 108 109)  -> dest ties to reg 108 ($v1)

Both input pseudos already carry the TARGET's hard registers (109 -> $v0,
108 -> $v1). There is therefore nothing for local-alloc to fix: the allocator is
already doing what the target's allocator did. The mips addsi3_internal template
is `addu %0,%1,%2` and in BOTH builds the dest ties to operand 1 (the first
source), so the entire 3-row class collapses to a single question — which of
reg 108 / reg 109 is operand 0 of the plus in insn 173.

inverse.py local was therefore NOT run: running an allocator solver on a seat
where the allocator already agrees with the target would have produced a
meaningless verdict. local_extract.py text1b --func func_800770B8 --suggest WAS
run and its outputs are banked (tmp/ra_solver_work/text1b.local.json,
text1b.sugg.json) for the class-B instrumentation item, which is untouched.

### The operand order IS C-controllable — and every flip costs the double read
Thirteen further address spellings were built and sandbox-measured (bringing the
project total for this seat to nineteen). The finding is a clean dichotomy:

  * POINTER-domain `D_800A36A0 + (t0*10)` -> canonicalized to (base, offset).
    This is the floor-9 form; it also produces the target's SECOND lw of
    D_800A36A0 at row 60 (the loop-top `u8 *base = D_800A36A0;` local supplies
    the first read at row 40, and the global is genuinely re-loaded here).
  * INT-domain `(t0*10) + (s32)D_800A36A0` -> preserves written order, i.e. it
    DOES flip insn 173 to the target's (108, 109). Confirmed in D2's output:
    rows 55-57 compute t0*5, sll 1, then `addu $2,$2,$3` with the shift as
    operand 0 — the target's shape.
  * But every int-domain spelling ALSO collapses the double read: the value of
    D_800A36A0 already sits in `base`'s register, and once the expression is
    int-typed cse forwards it, deleting the row-60 lw and re-floating the whole
    p_6a/p_7e computation ahead of the 0x42/0x40/0x68 stores. Score 33-36.

So the residual C question is now sharply stated for the first time:
**flip insn 173's plus to (shift, load) while keeping the row-60 lw a distinct
in-place memory read.** Every spelling measured so far buys one and loses the
other.

### Measured this session (all on the floor-9 chassis, score / build_insns)
  D1 two-statement multi-set int rowb (H3 anti-hoist + H4 int order)    33 / 175
  D2 symmetric int-domain, unnamed                                      33 / 175
  D3 multi-set pointer, int add written first                           12 / 174
  D4 int rowb built off the loop-top base local                         45 / 174
  E1 shift as the pointer operand, (s32)D_800A36A0                      33 / 175
  E2 constant folded onto the shift side                                20 / 176
  E3 multi-set u8 *row, shift as pointer base                           33 / 175
  E4 (u8*)(t0*10) + (s32)(D_800A36A0 + 0x6A)                            13 / 176
  F2 shared row, global carries 0x6A, p_7e = row + 0x14                 35 / 175
  F4 same value spelled through p_6a                                    35 / 175
  F5 named pointer handle g = D_800A36A0 + 0x6A, int add                15 / 175
  G0 loop-top base local removed (control)                              34 / 176
  G1 G0 + int-domain p_6a                                               36 / 176

E4 is the most informative loser: it is the ONLY form that flips the order AND
keeps the row-60 re-read, and it costs exactly one insn (176) because the
+0x6A / +0x7E constants fold onto the shift side BEFORE the add, so the
single addu + two addiu shape becomes two addus. G0/G1 prove the loop-top
`base` local is load-bearing: removing it costs an insn outright.

### A residual row is a SCORING ARTIFACT, not a byte difference
Row 50 (`addiu $2,$2,0` vs target `addiu $v0,$v0,%lo(D_800A35D0)`) is NOT a real
divergence. `mipsel-linux-gnu-objdump -dr` on tmp/sandbox/func_800770B8/text1b.o
shows R_MIPS_HI16 / R_MIPS_LO16 against D_800A35D0 at 0x8bc8 / 0x8bcc with a zero
addend; the engine's scorer masks the HI16 immediate (row 49 is not flagged) but
not the LO16 addend, so an unlinked-relocation row is counted as a difference.
The linker resolves it to exactly the target's bytes. See the auto-memory note
[[sandbox-lo16-text-addend-false-distance]]. **The honest residual is therefore
8 real rows, not 9**, distributed entirely across classes A (4), B (2) and C (3).
Any future exhaustion record for this function must state the floor as "9 as
scored, 8 real, one unrelocated-LO16 artifact".

## [s6] Class C (rows 62-64) is a local-alloc dest-coalesce decision between two dying input pseudos (the s5 frontier's item 1).
- mechanism: s5 read the object-level classifier's $v0->$v1 x4 output as register renaming and concluded the seat belonged to local-alloc's block_alloc, which ties a dest quantity to a dying source quantity.
- probe: regenerated the full cc1 -da dump set off the floor-9 body (tmp/grind/func_800770B8/run_dump.sh) and read insns 168/171/169/173/175/190 out of text1b.lreg; cross-read the positional diff rows 60-64. Also ran local_extract.py text1b --func func_800770B8 --suggest to have the local-alloc ground truth in hand.
- result: KILLED as an attribution. insn 173 is (set (reg 110) (plus (reg 109) (reg 108))), reg 109 = the lw D_800A36A0 (insn 171), reg 108 = the sll ...,1 (insn 169). Rows 60 and 61 are byte-identical to the target, i.e. BOTH input pseudos already receive the target's hard registers ($v0 and $v1 respectively). The allocator is already in agreement with the target; in both builds the dest ties to operand 1 of addu %0,%1,%2. The only difference is the ORDER of the plus's two register operands in the RTL, which is settled at expand/fold time from the C expression's type domain, long before local-alloc runs. inverse.py local was deliberately not run on this seat: an allocator solver cannot return anything meaningful about a seat where the allocator already matches.
- verdict: KILLED (re-attributed: RTL plus operand order, not RA)

## [s6] The row-62 plus operand order can be flipped from C without collateral damage.
- mechanism: s1's H4 established that a pointer-domain PLUS is canonicalized to (base, offset) while an int-domain PLUS preserves written source order. Applying H4's own lever to the p_6a/p_7e base add should therefore produce the target's (shift, load) order.
- probe: thirteen spellings built and sandbox-measured on the floor-9 chassis — D1-D4 (int row bases, multi-set and single-set, off the global and off the base local), E1-E4 (the shift cast to u8 * so it becomes the canonical pointer operand, with and without the constants folded onto it), F2/F4/F5 (a named pointer handle carrying the 0x6A so the global read stays pointer-typed), G0/G1 (the loop-top base local removed, as a control).
- result: HALF-CONFIRMED, HALF-KILLED. The flip itself is REACHABLE: D2's output shows addu $2,$2,$3 with the shift as operand 0, exactly the target's shape. But every int-domain spelling simultaneously deletes the target's second lw of D_800A36A0 at row 60 — the value is already live in the loop-top base local's register, and once the expression is int-typed cse forwards it and re-floats the whole p_6a/p_7e computation ahead of the 0x42/0x40/0x68 stores. Scores 33-36 at 175 insns (no insn-count change: it is pure reordering + renaming cascade). E4 is the sole form that keeps the re-read AND flips the order, and it costs exactly one insn (176) because the +0x6A/+0x7E constants fold onto the shift side before the add. G0/G1 show the base local itself is load-bearing (removing it costs an insn: 176). Nineteen address spellings are now measured across s1/s2/s3/s6; the naive axis is exhausted, but the question is now sharply stated rather than merely dead.
- verdict: KILLED (as a spelling search) / CONFIRMED (as a mechanism: the order IS C-controllable)

## [s6] Row 50 of the residual is a real byte divergence.
- mechanism: the positional diff shows addiu $2,$2,0 against the target's addiu $v0,$v0,%lo(D_800A35D0), which looks like a missing low-half materialisation.
- probe: mipsel-linux-gnu-objdump -dr -Mreg-names=numeric tmp/sandbox/func_800770B8/text1b.o, grepping for D_800A35D0.
- result: KILLED. The object carries R_MIPS_HI16 and R_MIPS_LO16 relocations against D_800A35D0 at 0x8bc8 / 0x8bcc with a zero in-place addend. The engine's scorer masks the HI16 immediate (row 49 is correctly not flagged) but does not mask the LO16 addend, so the unrelocated row is counted as a difference that the linker resolves to the target's exact bytes. Matches the auto-memory note [[sandbox-lo16-text-addend-false-distance]]. The honest residual is 8 real rows, not 9.
- verdict: KILLED (the row is a scoring artifact)

## Live frontier (for s7) — reset by the s6 synthesis

1. **Class C, restated precisely: flip insn 173's plus to (shift, load) while keeping the row-60 lw a distinct in-place memory read.**
   - mechanism: pointer-domain PLUS canonicalizes to (base, offset) and keeps the
     re-read; int-domain PLUS preserves written order and lets cse forward the
     loop-top base local's register, deleting the re-read. The two properties
     have never been obtained together. E4 comes closest (re-read kept, order
     flipped, +1 insn from the constants folding onto the shift side).
   - next probe: attack the CONSTANT FOLD, not the operand order. E4's only defect
     is that 0x6A/0x7E land on the shift operand before the add. Spellings that
     keep the shift as the canonical pointer operand while forcing the constants to
     stay on the RESULT: index the row through a struct/array element type so the
     +0x6A is a member offset rather than a folded addend, or give the two
     inner-loop pointers a common (u8 *)(t0*10) + (s32)D_800A36A0 row whose two
     users each add their own constant. Read text1b.rtl (pre-optimization) for E4
     and for the baseline side by side to see exactly where the fold happens — the
     baseline dumps are already in tmp/grind/func_800770B8/dumps/.

2. **Class B's suggested-register instrumentation (unspent, and now the ONLY item with a solver-named mechanism).**
   - mechanism: s5's inverse.py global foreclosure on pseudo 75 explicitly named
     qty_phys_copy_sugg / qty_phys_sugg as reported-but-not-scored. The
     instrumented cc1 already emits SUGGDBG-QTY / SUGGDBG-FFR lines and this
     session banked a full extraction for text1b:
     tmp/ra_solver_work/text1b.sugg.json (2.7 MB) and text1b.local.json.
     Nobody has read them yet.
   - next probe: read the SUGGDBG rows for func_800770B8's block containing the
     addu $s1,$v0,$zero copy out of text1b.sugg.json and check whether the
     target's assignment is explicable as a copy-suggestion our RTL does not offer.
     The data is already on disk; this is a read, not a build.

3. **Do NOT re-open: the prologue fence, the A2-A12 ordering set, the do-while(0) ruling, OR inverse.py local on the row-62 seat.**
   - mechanism: the first three are closed by s5 (the target has no loop in its
     prologue and a real loop costs +11 insns). The fourth is closed by s6: both
     input pseudos at insn 173 already carry the target's hard registers, so the
     allocator has nothing to fix and a local-alloc solver run on that seat cannot
     return a meaningful verdict.
   - next probe: none. Any session that finds itself running an RA solver on rows
     62-64, or respelling a prologue fence, should stop and go to item 1 or 2.

- [s6] Chassis re-measured at session start with candidate.c applied to src/text1b.c: score 9, build_insns 175, target_insns 175 - identical to the s3/s4/s5 chassis, so every banked spelling conclusion remains chassis-current. Restored and re-verified at 9/175 at session end.

- [s6] text1b.lreg insn 173 = (set (reg 110) (plus (reg 109) (reg 108))); insn 171 = (set (reg 109) (mem (symbol_ref D_800A36A0))); insn 169 = (set (reg 108) (ashift (reg 107) (const_int 1))) with REG_EQUAL (mult (reg 86) (const_int 10)). Both 109 and 108 carry REG_DEAD at insn 173. Insns 175/190 add const_int 106 / 126 to reg 110.

- [s6] Positional diff rows 60/61 are byte-identical on both sides (lw $v0,%gp_rel(D_800A36A0)($gp) then sll $v1,$v1,1), so the two input pseudos already receive the target's hard registers; only row 62's plus operand order differs (ours (109,108) -> dest $v0, target (108,109) -> dest $v1). Class C is therefore an expand/fold-level operand-order question, not an RA question.

- [s6] Int-domain spellings of the p_6a/p_7e base address DO flip insn 173 to the target's operand order (D2 rows 55-57), proving the order is C-controllable - but all of them delete the target's second lw of D_800A36A0 at row 60 because the value is already live in the loop-top `u8 *base = D_800A36A0;` local and cse forwards it once the expression is int-typed.

- [s6] E4 ((u8*)(t0*10) + (s32)(D_800A36A0 + 0x6A)) is the only measured form that BOTH flips the operand order AND keeps the row-60 re-read; it scores 13 at 176 insns, the single extra insn coming from 0x6A/0x7E folding onto the shift operand before the add instead of staying on the result. This makes the constant fold, not the operand order, the next lever.

- [s6] G0/G1 (loop-top `base` local removed) measure 34/176 and 36/176: the `base` local is load-bearing for the insn count and must stay.

- [s6] tools/ra_solver/local_extract.py text1b --func func_800770B8 --suggest ran clean and banked tmp/ra_solver_work/text1b.local.json (282 KB) plus tmp/ra_solver_work/text1b.sugg.json (2.7 MB, the BB2_SUGG_DEBUG SUGGDBG-QTY/SUGGDBG-FFR ground truth). The suggested-register data that s5's frontier item 2 called for is now ON DISK and unread - s7 can read it without a build.

- [s6] Residual row 50 is a scoring artifact: R_MIPS_HI16/R_MIPS_LO16 against D_800A35D0 at 0x8bc8/0x8bcc with zero addend; the scorer masks HI16 but not LO16. Honest residual = 8 real rows (class A 4, class B 2, class C 3, minus the artifact), and any future exhaustion record must state the floor as '9 as scored, 8 real'.

- [s6] Four disproven forms banked to memory/grind/func_800770B8/rejected/: s6-int-domain-flips-plus-order-but-collapses-double-read.c (D2), s6-shift-as-pointer-const-folds-wrong-side-176insn.c (E4), s6-named-global-handle-breaks-a2-init-slot.c (F5), s6-base-local-removed-costs-an-insn.c (G0). The rejected bank now holds 24 forms.

## s7 (solver) — 2026-09-01

Chassis re-measured at session start with the committed candidate applied:
**score 9, build_insns 175, target_insns 175** — unchanged.

`inverse_compose.py classify` refuses this function (zero-rule / rules-to-zero
guard, it would report a fictitious PRE-RA verdict). The sanctioned substitute
`goal_from_tgt.py classify text1b func_800770B8` reports
`FIRST DIVERGENCE: RA` with the register deltas `$v0 -> $v1 x4`,
`$s1 -> $v0 x2`, `$v1 -> $v0 x1` — i.e. class C (4 rows incl. the two addiu
consumers), class B (2 rows), and one more. Note the classifier's "RA" label is
a heuristic on the object streams; s6 already re-attributed class C to RTL
operand order and s7 re-attributes class B to cse pseudo identity, so the label
should not be taken as a pass attribution.

### Frontier item 2 (the SUGGDBG read) — ANSWERED, class B closed at the RA layer
`tmp/ra_solver_work/text1b.sugg.json` sliced for func_800770B8: 34 local-alloc
quantities across 14 blocks. Pseudo 75 (p_old) is **not among them** — local-alloc
never forms a quantity for it, which is exactly what s5's global-alloc
attribution implies. The whole function carries exactly ONE suggestion of any
kind: blk0 qty0 (reg1=72, birth 2, death 20, refs 3, calls 1) with
`ncopysugg=1, copysugg=[4]` ($a0). qty_phys_copy_sugg / qty_phys_sugg — the two
mechanisms s5's negative report named as reported-but-not-scored — therefore
have nothing to say about the $s1-vs-$v0 residual. **Every RA-layer mechanism
for class B is now spent.**

### Class B re-attributed to cse pseudo identity (pre-RA), and MEASURED
Residual rows 35/36 are `sw $0,48($17)` / `sh $0,52($17)` against the target's
`sw $zero,0x30($v0)` / `sh $zero,0x34($v0)`. The four consecutive stores after
the `func_8006E49C` call divide 2+2 in the target ($s1, $s1, $v0, $v0) and 4+0
in ours ($s1 x4).

Reading `tools/gcc-2.7.2/cse.c`:
* `canon_reg()` — *"Never replace a hard reg, because hard regs can appear in
  more than one machine mode…"* — returns hard regs untouched, so `$v0` can never
  be substituted INTO a use. The target's `0x30($v0)` cannot be hard-reg
  forwarding; it must be a second pseudo seated in $v0 by the allocator.
* `make_regs_eqv()` — *"Prefer fixed hard registers to anything. Prefer pseudo
  regs to other hard regs. Among pseudos, if NEW will live longer than any other
  reg of the same qty, and that is beyond the current basic block, make it the
  new canonical replacement for this qty."* $v0 is not fixed, so the pseudo the
  call result is copied into becomes `qty_first_reg` and every later reference in
  the extended basic block canonicalises onto it. A later-joining pseudo takes
  over as canonical only when
  `uid_cuid[regno_last_uid[new]] > cse_basic_block_end` or
  `uid_cuid[regno_first_uid[new]] < cse_basic_block_start`.

Twelve spellings measured on the floor-9 chassis (all ordinary C, all in
`tmp/grind/func_800770B8/s7/v/`):

| id  | shape | score / insns |
|-----|-------|---------------|
| H1  | raw call result gets its own local `pn`; `p_old = pn`; 0x30/0x34 via `pn` | 23 / 170 |
| H2  | mirror: `p_old` takes the call, `pn = p_old`, long-lived uses via `pn`     | 9 / 175 |
| H3  | H1 with the 0x30/0x34 stores still spelled through the global             | 23 / 170 |
| H4  | H1 with a `u8 *` short-lived handle                                       | 23 / 170 |
| H5  | 0x4 store via the global re-read, 0x30/0x34 via `p_old` (spelling swap)   | 9 / 175 |
| H6  | all four stores spelled through `p_old`                                   | 9 / 175 |
| H7  | all four stores spelled through the `D_800A36A0` re-read                  | 9 / 175 |
| H8  | the existing `r` local reused to hold the raw result                      | 23 / 170 |
| H10 | `u8 *pp` hoisted to fn scope, set from `p_old` AFTER the global+0x4 stores, used for 0x30/0x34, REUSED in the tail block | 12 / 176 |
| H11 | H10 but `pp` is block-local (control for the canonicality test)           | 9 / 175 |
| H12 | H10 with `pp` fed from the global re-read instead of from `p_old`         | 12 / 176 |
| H15 | H10 composed with s6's D3 address form                                    | 15 / 176 |
| H16 | s6's D3 address form alone, re-measured                                   | 12 / **175** |
| H17 | H10 composed with s6's E4 address form                                    | 16 / 177 |

Four durable facts fall out:

1. **Store-base spelling is byte-neutral.** H5/H6/H7 are all 9 / 175: whichever
   of `p_old` / the global re-read each store names, cse collapses all four onto
   one canonical pseudo. Nineteen-plus spellings of this seat are now dead.
2. **Naming the raw result first is strictly wrong.** H1/H3/H4/H8 make that pseudo
   canonical, route ALL FOUR stores through it, delete the `move $s1,$v0` copy
   and free a whole callee-saved register (the frame shrinks 0x40 → 0x38 and
   `p_old` shares $s0 with arg0): 170 insns, score 23.
3. **The target's split IS reachable, and the mechanism is exactly the
   make_regs_eqv canonicality test.** H10's `text1b.cse` reads:
   `(insn 68 (set (mem:SI (symbol_ref "D_800A36A0")) (reg/v:SI 75)))`,
   `(insn 71 (set (mem:SI (plus:SI (reg/v:SI 75) (const_int 4))) (reg/v:SI 81)))`,
   `(insn 74 (set (reg/v:SI 76) (reg/v:SI 75)))`,
   `(insn 77 (set (mem:SI (plus:SI (reg/v:SI 76) (const_int 48))) (const_int 0)))`,
   `(insn 80 (set (mem:HI (plus:SI (reg/v:SI 76) (const_int 52))) (const_int 0)))`
   — two stores on pseudo 75 and two on pseudo 76, structurally the target's
   $s1/$s1/$v0/$v0. H11 (identical but with `pp` block-local, so its
   `regno_last_uid` stays inside the cse block) measures 9 / 175, confirming the
   uid-vs-block-boundary test is what turns the split on.
4. **The split costs exactly one instruction and the cost is structural.** insn 74,
   the copy that makes pseudo 76 join the quantity, is a real insn (176 total).
   It disappears only if 75 and 76 get the same hard register — which destroys the
   split. The alternative, having 76 born free from `(set 76 (reg:SI 2 v0))` so
   that seating it in $v0 deletes the copy, is unavailable: C offers exactly one
   assignment of a call result and whichever pseudo takes it becomes canonical for
   the entire block (fact 2).

### The 174-insn credit does not exist
s6 recorded D3 at 12 / 174 — one insn under target — which would have paid for
the class-B split copy. Re-measured against the committed candidate on this
chassis, D3 is 12 / **175** (H16); s6's 174 came from its own working base, not
the ledger form. Composites confirm there is no credit: H15 (split + D3) is
176 and H17 (split + E4) is 177. **No form on record is under 175 insns while
keeping the floor-9 structure**, so the class-B split currently has nothing to
pay with.

- [s7] Chassis re-measured at session start with the committed candidate applied: score 9, build_insns 175, target_insns 175 - unchanged from the ledger.

- [s7] inverse_compose.py classify refuses func_800770B8 (rules-to-zero guard: with no regfix/asmfix rules the src-derived tgt.s cannot carry the target's stream and it would report a fictitious PRE-RA verdict). The sanctioned substitute is goal_from_tgt.py classify, which reports FIRST DIVERGENCE: RA with deltas $v0->$v1 x4, $s1->$v0 x2, $v1->$v0 x1. That label is an object-stream heuristic, not a pass attribution: s6 re-attributed class C to RTL operand order and s7 re-attributes class B to cse pseudo identity.

- [s7] func_800770B8 has 34 local-alloc quantities across 14 blocks and pseudo 75 (p_old) is not one of them; the only suggestion in the entire function is blk0 qty0 copysugg=[$a0]. Every RA-layer mechanism for class B is now spent (global: s5 prune_preferences; local: s7 suggested registers).

- [s7] GCC 2.7.2 cse.c canon_reg() returns hard registers untouched ('Never replace a hard reg'), so a non-fixed hard reg such as $v0 can never be canonicalized INTO a use - the target's 0x30($v0)/0x34($v0) stores are a second pseudo, not $v0 forwarding.

- [s7] GCC 2.7.2 cse.c make_regs_eqv() prefers a pseudo to a non-fixed hard reg as qty_first_reg, and promotes a later-joining pseudo to canonical only when uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start. That single test is what turns the 2+2 store split on and off: H10 (pp reused in the tail, uid past the block end) splits; H11 (identical but block-local) does not.

- [s7] Measured on the floor-9 chassis: H1 23/170, H2 9/175, H3 23/170, H4 23/170, H5 9/175, H6 9/175, H7 9/175, H8 23/170, H10 12/176, H11 9/175, H12 12/176, H15 15/176, H16 12/175, H17 16/177.

- [s7] Store-base SPELLING (p_old vs the D_800A36A0 re-read, in every permutation across the four post-call stores) is completely byte-neutral - cse collapses all four bases onto one canonical pseudo. This seat's naive spelling axis is exhausted.

- [s7] Giving the raw call result its own C name FIRST is strictly wrong, not merely neutral: it makes that pseudo canonical, sends all four stores through it, deletes the move $s1,$v0 copy and drops a callee-saved register (frame 0x40 -> 0x38, p_old shares $s0 with arg0), landing at 170 insns / score 23.

- [s7] s6's D3 address form is 175 insns on the committed chassis, not the 174 s6 recorded; no form on record is under 175 insns while keeping the floor-9 structure, so there is no insn credit available to pay for the class-B split copy.

## s8 (forensics) — 2026-09-01

Chassis re-measured at session start with the committed candidate applied:
**score 9, build_insns 175, target_insns 175** — unchanged; restored and
re-verified at 9 / 175 at session end.

### The target's class-B shape, read off the target asm (not inferred)
`asm/funcs/func_800770B8.s` lines 29-38, immediately after `jal func_8006E49C`:

    addu  $v1, $s1, $zero        ; prev = the OLD p_old (arg0+0x58)
    addu  $s1, $v0, $zero        ; p_old = the call result   <-- the ONE copy
    ...
    sw    $s1, %gp_rel(D_800A36A0)($gp)
    sw    $v1, 0x4($s1)
    sw    $zero, 0x30($v0)       ; <-- raw call-result register, still live
    sh    $zero, 0x34($v0)

`$s1` has exactly two lives (`addiu $s1,$s0,0x58` then `addu $s1,$v0,$zero`) and is
dead after line 36 (a grep for $s1 shows only the save/restore afterwards), so the
target carries **two pseudos holding the same value with a single copy between
them** — and that costs the same insn count as our one-pseudo form, because when
the raw-result pseudo is seated in $v0 its `(set Q (reg:SI 2 v0))` birth insn is a
no-op and is deleted, leaving the `(set P (reg Q))` copy as the only move. **The
class-B split is insn-count FREE in principle**; s7's "the split costs one insn"
conclusion was a property of s7's H10 spelling (two long-lived pseudos), not of
the split itself.

### PASS ATTRIBUTION (the point of this session): cse pass 1 CAN produce the split; cse pass 2 destroys it
Probe W1 = H1's shape (`pn = call(); p_old = pn;` then the two p_old stores, then
the two pn stores) with a `do { } while (0);` between the copy and the stores,
built with the instrumented cc1 (`tmp/grind/func_800770B8/s8/dW1/`). Dumps read in
pass order:

* `text1b.rtl` — insns 83/86 on `(reg/v:SI 75)` (p_old), insns 89/92 on
  `(reg/v:SI 81)` (pn), copy at insn 68. As written.
* `text1b.cse` — **the split SURVIVES cse1 exactly in the target's shape**:
  83/86 still `reg 75`, 89/92 still `reg 81`. Mechanism named:
  `cse.c: cse_end_of_basic_block()` contains
  `if (! after_loop && GET_CODE (p) == NOTE && NOTE_LINE_NUMBER (p) == NOTE_INSN_LOOP_END) break;`
  — the empty loop's LOOP_END note ends the extended basic block, `reg_qty` is
  reset, and the two pseudos are never made equivalent.
* `text1b.loop` — unchanged (split still present).
* `text1b.cse2` — **the split is GONE**: 83/86 rewritten to `reg 81`. cse pass 2
  runs `cse_main` with `after_loop = 1`, so the `! after_loop` guard above no longer
  fires, the LOOP_END note is ignored, the block re-merges, `make_regs_eqv` puts 75
  and 81 in one quantity and `canon_reg` rewrites every use onto `qty_first_reg`.
* `text1b.flow` — insn 68 becomes `NOTE_INSN_DELETED`: with no uses left the copy
  is dead and flow.c removes it.
* `text1b.greg` — insn 65 is `(set (reg:SI 2 v0) (reg:SI 2 v0))`; all four stores
  on $v0; p_old's callee-saved life is gone. Result 170 insns / score 23.

This retires an entire family of proposals: **a NOTE-based (do-while(0)-style)
basic-block fence cannot buy class B at all** — it is undone by cse2 two passes
before RA. s4's "fence measures 5/7" effect is a scheduling artifact elsewhere in
the function, not a class-B mechanism.

### The C-level invariant, measured seven ways
| id | shape | score / insns |
|----|-------|---------------|
| H1 (re-measured) | `pn = call(); p_old = pn;` stores 2+2 as target | 23 / 170 |
| V1 | H1 with the pn stores moved BEFORE the copy | 25 / 170 |
| V2 | V1 with the +4 store spelled through the global | 25 / 170 |
| W1 | H1 + `do{}while(0)` between the copy and the stores | 23 / 170 |
| W2 | H1 + `do{}while(0)` before the copy (control) | 23 / 170 |
| V3 | V1 + `p_old` reused in the tail block (ordinary C) | 20 / **175** |
| V4 | H1 + `p_old` reused in the tail block | 18 / **175** |

Reading `cse.c: make_regs_eqv()` against these: within ONE extended basic block a
copy between two pseudos ALWAYS collapses. Uses that precede the copy take the
pre-copy canonical register; uses that follow it take the post-copy canonical.
Whichever pseudo wins, the other's uses are rewritten onto it and the copy dies
(H1/V1/V2/W1/W2 → 170) or the copy survives with everything on the long-lived
pseudo (V3/V4 → 175, still `sw $0,48($17)`). **The target needs P (the callee-saved
long-lived pseudo) canonical for the two stores that FOLLOW the copy and Q (the raw
call result) canonical for the two stores that also follow it — mutually exclusive
under make_regs_eqv.** The positional diff for V3 and V4 confirms rows 35/36 stay
$17-based in both.

### The promotion lever is real, free in insn count, and still not enough
`make_regs_eqv`'s canonicality test is
`(uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`.
In H1 the long-lived `p_old` fails it (it dies at the +4 store, inside the block),
so the call-result pseudo stays canonical and everything collapses onto $v0.
Giving `p_old` a genuine later use — reusing the variable for the tail block's
`D_800A36A0` re-read, ordinary C, the same `u8 *p` role — flips the canonical and
costs **zero insns** (V3/V4 are both 175). But it collapses onto $s1 instead of
splitting, and it damages the tail: the positional diff for V4 shows rows 126-137
turn into `lw $17,0($28)` / `sw $0,32($17)` where the target re-loads D_800A36A0
into $v0/$a0 per statement. Score 18 vs 9.

### Why a hard-register address (the last free route) is not C-reachable
The one rtx `canon_reg` refuses to touch is a hard register ("Never replace a hard
reg..."), so `(mem (plus (reg:SI 2 v0) 48))` would survive cse, cse2 and RA and would
reproduce the target exactly at 175 insns. `calls.c: expand_call()` forecloses it:
when the call feeds an assignment the caller passes a TARGET and expand emits
`emit_move_insn (target, valreg)` returning the pseudo (calls.c:2039); when it does
not, the tail is `else target = copy_to_reg (valreg);` (calls.c:2114). The hard
return register never escapes into a later address rtx, and C offers exactly one
assignment of a call result. No spelling reaches this shape.

- [s8] Chassis re-measured with the committed candidate: 9 / 175 / 175 at session start AND at session end.

- [s8] The target's class-B split is insn-count FREE, not +1: target rows 29-38 show ONE copy (addu $s1,$v0,$zero) with the raw call result still live in $v0 for the 0x30/0x34 stores, and $s1 dead after row 36. s7's "+1 insn" was a property of H10's two-long-lived-pseudo spelling, not of the split.

- [s8] PROVEN with instrumented-cc1 dumps (tmp/grind/func_800770B8/s8/dW1): cse PASS 1 keeps the target's exact 2+2 pseudo split when a NOTE_INSN_LOOP_END breaks the extended basic block (cse.c cse_end_of_basic_block: `if (! after_loop && ... NOTE_INSN_LOOP_END) break;`), and cse PASS 2 destroys it because it runs with after_loop=1 and ignores that note; flow.c then deletes the dead copy. A NOTE-based fence therefore cannot buy class B under any spelling - it is undone two passes before RA.

- [s8] Measured on the floor-9 chassis: H1 23/170, V1 25/170, V2 25/170, W1 23/170, W2 23/170, V3 20/175, V4 18/175. Within one cse extended basic block a pseudo-to-pseudo copy ALWAYS collapses (make_regs_eqv + canon_reg); uses before the copy take the pre-copy canonical, uses after take the post-copy canonical, so the target's requirement (long-lived pseudo canonical for two post-copy stores AND raw result canonical for two other post-copy stores) is mutually exclusive.

- [s8] The make_regs_eqv canonicality promotion IS C-controllable at ZERO insn cost: reusing `p_old` for the tail block's D_800A36A0 re-read (ordinary variable reuse) gives it a last_uid beyond cse_basic_block_end and flips the canonical from the call-result pseudo to p_old - V3 20/175 and V4 18/175 (vs 170 without it). It collapses onto $s1 rather than splitting, and the tail rows 126-137 become $17-based where the target re-loads D_800A36A0 per statement.

- [s8] A hard-register address (the only rtx canon_reg will not rewrite) would reproduce the target at 175 insns but is not C-reachable: calls.c:2039 emits `emit_move_insn (target, valreg)` when the call feeds an assignment and calls.c:2114 does `target = copy_to_reg (valreg)` otherwise, so a call's hard return register never escapes into a later address rtx.

- [s8] Four disproven forms banked: s8-cse1-block-break-undone-by-cse2-170insn.c (W1), s8-promotion-collapses-onto-p_old-tail-cost-175insn.c (V4), s8-stores-before-copy-reorders-only-no-split-175insn.c (V3), s8-h1-order-flip-still-collapses-170insn.c (V1). The rejected bank now holds 33 forms.

### Live frontier (for s9)
1. **Class B is one measurement short of foreclosed.** Every route except one is
   now measured or read out of the compiler source: NOTE fences (undone by cse2),
   both canonical choices (collapse), both store orderings (collapse), the
   promotion lever (free but collapses to $s1 and damages the tail), and hard-reg
   addressing (unreachable from C per calls.c). The single untested route is a
   REAL control-flow boundary between the copy and the two stores — a label/jump
   that survives to cse2 — which the target's straight-line asm does not contain
   and which cannot be free in insns. Next session should spend at most one probe
   confirming that and then treat class B as a closed axis.
2. **Class C (rows 62-64) is unchanged and is now the highest-value open axis** —
   see the s6 frontier: attack the CONSTANT FOLD (index the row through a struct
   or array element type so +0x6A is a member offset rather than an addend folded
   onto the shift operand before the add). E4 remains the only form that flips the
   operand order AND keeps the row-60 re-read, at +1 insn.
3. **Class A (prologue rows 7-12, 4 rows) has not been attacked since s3/s5** and
   is now the largest untouched residual. s5 closed the fence hypothesis; the
   sched2 ordering of the reload-emitted save stores has never been read out of a
   `.sched2` dump against the target's order. That dump is one build away.

- [s8] [s8] Chassis re-measured with the committed candidate applied to src/text1b.c: score 9, build_insns 175, target_insns 175 - at session start AND at session end.

- [s8] [s8] The target's class-B split is insn-count FREE, not +1: asm/funcs/func_800770B8.s rows 29-38 show ONE copy (addu $s1,$v0,$zero) with the raw call result still live in $v0 for the 0x30/0x34 stores, and $s1 dead after row 36 (only the epilogue restore follows). This retires the 's7: no insn credit exists to pay for the split' blocker.

- [s8] [s8] PASS ATTRIBUTION, proven with instrumented-cc1 dumps in tmp/grind/func_800770B8/s8/dW1: cse PASS 1 keeps the target's exact 2+2 pseudo split when a NOTE_INSN_LOOP_END breaks the extended basic block (cse.c cse_end_of_basic_block: 'if (! after_loop && ... NOTE_INSN_LOOP_END) break;'), and cse PASS 2 destroys it because it runs with after_loop=1 and ignores that note; flow.c then deletes the dead copy insn 68.

- [s8] [s8] Consequence: a NOTE-based (do-while(0)-style) basic-block fence cannot buy class B under ANY spelling - it is undone two passes before RA. s4's fenced 5/7 measurements are a scheduling artifact elsewhere in the function, not a class-B mechanism.

- [s8] [s8] Measured on the floor-9 chassis: H1 23/170, V1 25/170, V2 25/170, W1 23/170, W2 23/170, V3 20/175, V4 18/175. Within one cse extended basic block a pseudo-to-pseudo copy ALWAYS collapses: uses before the copy take the pre-copy canonical register and uses after take the post-copy canonical, so the target's requirement is mutually exclusive.

- [s8] [s8] The make_regs_eqv canonicality promotion IS C-controllable at ZERO insn cost: reusing p_old for the tail block's D_800A36A0 re-read gives it a last_uid beyond cse_basic_block_end and flips the canonical away from the call-result pseudo - V3 175 insns and V4 175 insns, against 170 for the same forms without the reuse. It collapses onto $s1 rather than splitting and regresses the tail (rows 126-137).

- [s8] [s8] A hard-register address would reproduce the target at 175 insns but is not C-reachable: calls.c:2039 emits emit_move_insn (target, valreg) when the call feeds an assignment, calls.c:2114 does target = copy_to_reg (valreg) otherwise.

- [s8] [s8] Four disproven forms banked to memory/grind/func_800770B8/rejected/ (bank now holds 33): s8-cse1-block-break-undone-by-cse2-170insn.c, s8-promotion-collapses-onto-p_old-tail-cost-175insn.c, s8-stores-before-copy-reorders-only-no-split-175insn.c, s8-h1-order-flip-still-collapses-170insn.c.
