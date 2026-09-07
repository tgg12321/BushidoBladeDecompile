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

- [s9] [s9] Session opened by re-applying candidate.c to src/text1b.c (tmp/grind/func_800770B8/s3/try.py) and re-measuring on today's chassis: sandbox --disable all = **9**, target_insns 175 / build_insns 175. The chassis has NOT moved since s8; every s3-s8 spelling conclusion is still chassis-valid.

- [s9] [s9] CLASS A IS NOW FULLY READ OUT OF THE SCHEDULER, at insn granularity. Instrumented-cc1 dumps in tmp/grind/func_800770B8/s9/d/ (text1b.sched2 basic block 0, ";; Function func_800770B8" at line 75962). Block 0 runs 558..81; its first 15 emitted insns map 1:1 onto the asm prologue as: 558 `subu $sp,64`, 568 `sw $s0,40`, 4 `move $s0,$a0`, 562 `sw $s3,52`, 6 `move $s3,$a1`, 564 `sw $s2,48`, 8 `move $s2,$a2`, 566 `sw $s1,44`, 15 `addu $s1,$s0,88`, 26 `lw $a0,D_800A374C`, 28 `li $a1,0x1008`, 560 `sw $ra,56`, 18 `sh $0,16($sp)`, 21 `sh $0,18($sp)`, 30 `jal ClearOTagR`. The TARGET's first 15 (asm/funcs/func_800770B8.s rows 1-15) are the SAME 15 insns with the window at positions 8-12 permuted: ours 566,15,26,28,560 vs target 560,566,28,26,15. Rows 1-7 and 13-15 are already byte-identical, so class A is exactly a 5-insn intra-block permutation, not a missing/extra insn.

- [s9] [s9] The scheduler's decision that produces our order is named exactly. Reload emits the five frame saves in the order ra,s3,s2,s1,s0 (UIDs 560,562,564,566,568), so sched2 assigns them LUIDs 1,2,3,4,5 - below every source-derived insn (15/26/28/18/21 carry LUIDs 18/19/20/21/22, i.e. POST-SCHED1 chain order, not source order). Each save is released (backwards) only by the insn that clobbers its register: 568 by insn 4, 562 by 6, 564 by 8, 566 by insn 15 (`addu $s1,$s0,88`), and 560 by insn 30 (the first `jal`, which sets $ra). Because insn 30 is emitted AFTER insn 15, it is PICKED BEFORE it in the backward pass, so **560 is always released strictly earlier than 566**. At the pick where the priority-1 group is {28,15,8,560}, 560 is the only function-unit insn present, and sched.c:2708-2721 (`schedule_select`, the "select among those that remain for the one with the largest potential hazard" loop, printed as ";; insn 560 has a greater potential hazard") promotes it to the head of the ready list and it is taken immediately. Consequence: `sw $s1` is ALWAYS emitted before `sw $ra`; the target's opposite order requires 560 to be picked after 566, which the release order forbids.

- [s9] [s9] Typed verdict, sched_solver: **class A is FORECLOSED to statement order.** tools/sched_solver/extract.py text1b re-extracted with parity=True (486 funcs / 1752 blocks / 14250 picks); the simulator reproduces block 0 exactly (`base match True`). An EXHAUSTIVE depth-1 sweep of all 3234 input perturbations (add_dep, del_dep, luid swap, luid_move, insn cost) against the goal emission 558,568,4,562,6,564,8,560,566,28,26,15 returns **0 hits** (tmp/grind/func_800770B8/s9/probe2.py). Decomposing the goal, only the sub-goal "560 emitted before 566" is even individually reachable, by 32 atoms: 26 of them are `add_dep X <- 560`, i.e. inventing a dependence ON A RELOAD-EMITTED SAVE STORE (not expressible in C); 1 is `del_dep 15 <- 566`, i.e. p_old NOT living in $s1 (an RA outcome, and it still does not yield the target order); 5 are `cost` atoms (`cost 4 := 2/3/12`, `cost 39 := 3/12`), i.e. turning the parameter move `move $s0,$a0` into a multi-cycle load - instruction selection, not spellable for a register parameter. **NO `luid` or `luid_move` atom - the two classes the toolkit designates as spellable source-statement moves - flips the pair.**

- [s9] [s9] The specific statement move the ledger flagged for class A (hoist `p_old = arg0 + 0x58` past the ClearOTagR argument setup) is simulated and dead: `luid_move 15 -> before 18/21/30` and `luid swap 15 <-> 28` all give emission 558,568,4,562,6,564,8,**26,28,566,15,560**,18,21,30 - a different wrong permutation, still with `sw $s1` before `sw $ra`. This is the model's explanation of s4's measured 10 for that move.

- [s9] [s9] CLASS B'S LAST OPEN AXIS IS CLOSED BY MEASUREMENT. The s8 frontier reserved one probe for "a REAL control-flow boundary (a label/jump surviving to cse pass 2) between the copy and the 0x30/0x34 stores". Spelled as `if (p_old != 0) { *(s32*)(D_800A36A0+0x30)=0; *(s16*)(D_800A36A0+0x34)=0; }`: **176 insns, score 16** (baseline 175/9). The boundary costs an instruction exactly as predicted (the target's asm is straight-line there) and the extra branch also perturbs the surrounding allocation. Banked as rejected/s9-classB-real-branch-boundary-176insn-score16.c. Class B now has no untested mechanism on record.

- [s9] [s9] CLASS C RE-ATTRIBUTED AGAIN, with the C-front-end line that decides it. Insn 173 in text1b.rtl is `(set (reg 110) (plus (reg 109 = lw D_800A36A0) (reg 108 = sll)))` - the operand order is fixed at EXPAND, and it is fixed by the C front end, not by fold or combine: `build_binary_op`'s PLUS_EXPR case rewrites `int + pointer` to `pointer_int_sum (PLUS_EXPR, op1, op0)` (c-typeck.c:1988) and `pointer_int_sum` ends `result = build (resultcode, result_type, ptrop, intop);` (c-typeck.c:2695) - **the pointer operand is unconditionally operand 0.** So no way of writing `D_800A36A0 + (t0*10)` in the POINTER domain can ever put the shift first. The separate constant-fold hazard the s6 frontier described is also named: c-typeck.c:2654-2678 applies the distributive law and moves a constant term out of `intop` ONTO `ptrop` whenever intop is a non-constant PLUS/MINUS with a constant second operand - that is why `p + (t0*10 + 0x6A)` shapes fold 0x6A onto the wrong side.

- [s9] [s9] **The class-C operand flip is C-reachable at ZERO insn cost** (s6/s7 had it at 176). Making the SHIFT the ptrop and the global the intop - `(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A)` - measures 175 insns and produces `lw $3,D_800A36A0; sll $2,$2,1; addu $2,$2,$3; addu $7,$2,106; addu $5,$2,126`: the addu's operand order is now the TARGET's (shift first, dest tied to the shift pseudo), and the 0x6A/0x7E constants stay off the shift (the outer `+ 0x6A` has a CONSTANT intop, so the c-typeck.c:2654 distributive law does not fire). What remains at rows 62-64 is no longer an operand-order question but a two-register SEAT question: target puts the lw in $v0 and the sll in $v1, ours the reverse. The form is NOT a win overall - whole-function score 33 (vs 9), because the inner loop's whole addressing chain re-schedules and re-allocates around it. Banked as rejected/s9-classC-shift-as-ptrop-flips-order-free-but-score33.c (and the named-local variant s9-classC-shift-as-ptrop-named-row-local-score33.c, also 175/33).

- [s9] Chassis re-measured at session start AND end with the s3-s8 candidate applied to src/text1b.c: sandbox --disable all = 9, target_insns 175, build_insns 175. The chassis has not moved since s8.

- [s9] Block 0's 15 prologue insns are mapped 1:1 to UIDs in both streams: 558 subu sp / 568 sw s0 / 4 move s0 / 562 sw s3 / 6 move s3 / 564 sw s2 / 8 move s2 / 566 sw s1 / 15 addu s1,s0,88 / 26 lw a0,D_800A374C / 28 li a1,0x1008 / 560 sw ra / 18 sh 16(sp) / 21 sh 18(sp) / 30 jal ClearOTagR. Positions 1-7 and 13-15 already byte-match the target; class A is EXACTLY the 5-insn permutation ours 566,15,26,28,560 vs target 560,566,28,26,15 - no missing or extra instruction.

- [s9] Reload emits the five frame saves in the order ra,s3,s2,s1,s0 (UIDs 560,562,564,566,568), so sched2 gives them LUIDs 1..5, below every source-derived insn. The frame layout itself already matches the target exactly (ra@0x38, s3@0x34, s2@0x30, s1@0x2C, s0@0x28).

- [s9] sched2 LUIDs are POST-SCHED1 chain order, not source order: the two `sh $0,1x($sp)` stores are written before the ClearOTagR call in C yet carry LUIDs 21/22, above the call-setup insns' 19/20. Any pass-2 LUID reasoning must go through sched1.

- [s9] tools/sched_solver reproduces func_800770B8 block 0 exactly on today's tree (extract.py text1b: parity=True, 486 funcs / 1752 blocks / 14250 picks; simulate: base match True), so its negative verdicts are model-exact, not heuristic.

- [s9] Exhaustive depth-1 perturbation sweep (all 3234 atoms) against the target emission order returns 0 hits. Decomposed, only 'sw $ra before sw $s1' is reachable and only by unspellable atoms (26 add_dep onto a reload save store, 1 del_dep removing p_old from $s1, 5 cost atoms making a register-parameter move cost like a load).

- [s9] The class-B reserved probe measures 176 insns / score 16 - a real control-flow boundary costs an instruction the function does not have to spend.

- [s9] c-typeck.c:1988 + c-typeck.c:2695 fix the PLUS operand order by pointer-typedness, and c-typeck.c:2654-2678 is the distributive law that moved 0x6A onto the wrong side in the s6 attempts. Both are now named lines, not hypotheses.

- [s9] The class-C flip is reachable at 175 insns; three disproven forms banked this session (bank now holds 36).

- [s10] Chassis re-measured at session start: the s3 candidate body is 9 / 175 insns, unchanged.

- [s10] A full STRUCT-TYPED rederive of the D_800A36A0 block is NOT the original shape: 178 insns / score 22. The evidence-backed layout (s16 unk08/unk0C/unk10/unk14/unk3C[2] at stride 2, s16 unk40[2][2] and the D_800A35D0 pair at stride 4, u8 unk68[2], s16 unk6A[2][5] and unk7E[2][5] at stride 10) is certainly the real object model, but writing it as C ARRAY_REFs makes GCC 2.7.2 (a) hoist `&D_800A35D0 + 2` into a loop-invariant register in the outer preheader (rows 30-32, +3 insns; the target has no such hoist) and (b) fold the 0x6A/0x7E constant onto the INDEX side (`addiu $2,$3,106` emitted BEFORE the base addu) instead of leaving it as a trailing addiu off the base. Banked rejected/s10-struct-typed-rederive-licm-hoists-D3-178insn.c.

- [s10] A fresh m2c decompile (tools/m2c, --target mipsel-gcc-c) of asm/funcs/func_800770B8.s corroborates the current body statement-for-statement and adds no new lever. Its only two structural readings not already in the ledger are that the 6A/7E base is `(var_t0 * 0xA) + D_800A36A0` (shift-first, i.e. the class-C flip) and that the sp[] slot address is hoisted into a pointer before inner loop 2 - both already covered.

- [s10] Two further structurally different shapes measure BYTE-NEUTRAL (9 / 175 insns): writing the OUTER loop as `for (t0 = 0; t0 < 2; t0 = (s16)(t0 + 1))` instead of the do-while, and deleting the `r` local by nesting the call as `func_8006E49C(func_80076FF8(p_old), D_800A35D8)` (m2c's literal shape). Banked as rejected/s10-outer-for-loop-byte-neutral.c and rejected/s10-nested-call-no-r-local-byte-neutral.c. Statement-level restructuring outside the three residual classes is inert.

- [s10] The class-C collateral is a property of the flipped TREE, not of any spelling. Four distinct flip spellings all measure EXACTLY 175 insns / score 33 with a byte-identical positional diff: (1) `(u8 *)(t0*10) + (s32)D_800A36A0 + 0x6A` (s9's form), (2) the same with the operands written in the other textual order, (3) the pure int-domain `(t0*10) + (s32)D_800A36A0 + 0x6A`, (4) a named `s32 row10 = t0 * 10;` intermediate (and a named `s32 i4 = t0 * 4;` index variant). NOTE this also corrects s6: on today's chassis the pure int-domain form KEEPS the second lw of D_800A36A0 (s6 recorded that it deleted it), so "int domain deletes the re-read" is chassis-stale and must not be re-quoted.

- [s10] LOCAL-ALLOC GROUND TRUTH for the class-C seat (tools/ra_solver/local_extract.py, BB2_QTY_DEBUG QTYDBG rows, block 1 = the outer-loop body); artifacts tmp/grind/func_800770B8/s10/text1b.local.BASE.json and text1b.local.FLIPPED.json:
    BASE (score 9)   ord0 qty2 r89  [12,14) refs4  -> $v0
                     ord1 qty4 r110 [48,56) refs10 -> $v0
                     ord2 qty3 r108 [28,52) refs16 -> $v1
                     ord3 qty1 r100 [10,44) refs12 -> $a0
                     ord4 qty0 r86  [6,46)  refs14 -> $a1
    FLIPPED (33)     ord0 qty2 r89  [12,14) refs4  -> $v0
                     ord1 qty3 r110 [28,56) refs22 -> $v0
                     ord2 qty4 r109 [48,52) refs4  -> $v1
                     ord3 qty1 r100 [10,44) refs12 -> $v1
                     ord4 qty0 r86  [6,46)  refs14 -> $a0
  The addu's dest ties to operand 0, so unflipped it merges into the SHORT lw quantity (span 8, refs 10, qty_compare pri 37500) and flipped it merges into the LONG sll chain (span 28, refs 22, pri 31428). In BOTH cases the merged quantity is ranked ord1 and takes $v0 - which is why the flip alone cannot win. The target needs the merged quantity in $v1 and the bare lw in $v0.

- [s10] inverse.py local --swap 3,4 --depth 2 (392-atom space) on the FLIPPED model returns REACHABLE with 30 minimal single-atom vectors in exactly three families, all C-unreachable here: 13x live_shrink on qty0 (r86 = the sign-extension of t0; born >=33 instead of 6), 11x live_shrink on qty1 (r100 = the FIRST D_800A36A0 read; born >=33 instead of 10), and 4x live_shrink + 2x refs_up on qty4 (the SECOND D_800A36A0 read; span 4->2, or refs 4->7/8). Families (a) and (b) contradict the target's own emission, whose sext and first lw ARE the first insns of the loop body (rows 38-41). Family (c) was probed directly: naming the shift in its own statement re-extracts to byte-identical QTYDBG rows (qty4 still [48,52) refs 4), so GCC emits the global read BEFORE the final `sll ...,1` regardless of spelling, and refs 7 would need the second read used seven times where the target uses it twice.

- [s10] Closed-form statement of what any future class-C lever must achieve, so the next session can test against it without re-deriving: with the flip in place, the merged dest quantity's qty_compare priority must fall below the bare lw's 20000, i.e. floor_log2(refs)*refs/span*10000 < 20000 - span > 44 (currently 28) or refs <= 15 (currently 22).

- [s10] Chassis re-measured at session start and again at session end: the s3 candidate body is 9 / 175 insns, unchanged; the residual is still rows 7-12 (class A), 35-36 (class B), 62-64 (class C), plus the row-50 LO16 scorer artifact.

- [s10] A full struct-typed rederive of the D_800A36A0 block measures 178 insns / score 22: GCC 2.7.2 LICM-hoists `&D_800A35D0 + 2` into the outer-loop preheader and folds the 0x6A/0x7E constant onto the index side. Struct typing is not the original shape even though the aggregate evidence (base register + strides 1/2/4/10) is real.

- [s10] A fresh m2c decompile corroborates the current body statement-for-statement and adds no new lever; its only structural readings beyond the ledger are the shift-first 6A/7E base (the known class-C flip) and a hoisted sp[] slot pointer.

- [s10] Two more structurally different shapes are byte-neutral at 9 / 175: the outer loop as a `for`, and the `r` local deleted by nesting func_8006E49C(func_80076FF8(p_old), D_800A35D8).

- [s10] Four distinct spellings of the class-C operand flip all measure exactly 175 insns / score 33 with a byte-identical positional diff, so the collateral is a property of the flipped tree, not of any spelling.

- [s10] CORRECTION to s6: on today's chassis the pure int-domain 6A/7E form KEEPS the second lw of D_800A36A0; 'int domain deletes the re-read' is chassis-stale and must not be re-quoted.

- [s10] Local-alloc ground truth (QTYDBG, block 1 = outer-loop body). BASE (score 9): ord0 qty2 r89 [12,14) refs4 -> $v0; ord1 qty4 r110 [48,56) refs10 -> $v0; ord2 qty3 r108 [28,52) refs16 -> $v1; ord3 qty1 r100 [10,44) refs12 -> $a0; ord4 qty0 r86 [6,46) refs14 -> $a1. FLIPPED (score 33): ord0 qty2 r89 [12,14) refs4 -> $v0; ord1 qty3 r110 [28,56) refs22 -> $v0; ord2 qty4 r109 [48,52) refs4 -> $v1; ord3 qty1 r100 [10,44) refs12 -> $v1; ord4 qty0 r86 [6,46) refs14 -> $a0.

- [s10] The addu's dest ties to operand 0: unflipped it merges into the SHORT lw quantity (pri 37500), flipped into the LONG sll chain (pri 31428). In BOTH cases the merged quantity ranks ord1 and takes $v0, which is exactly why the flip alone cannot win - the target needs the merged quantity in $v1 and the bare lw in $v0.

- [s10] inverse.py local --swap 3,4 --depth 2 (392 atoms) returns REACHABLE with 30 minimal single-atom vectors in exactly three families: 13x live_shrink qty0 (sext of t0 born >=33 instead of 6), 11x live_shrink qty1 (first D_800A36A0 read born >=33 instead of 10), 4x live_shrink + 2x refs_up qty4 (second D_800A36A0 read, span 4->2 or refs 4->7/8).

- [s10] Families 1 and 2 are excluded by the target itself: its sign-extension ($a1, rows 38-39) and its first lw (row 41) are the first insns of the loop body, so 'born at >=33' describes a different function. Family 3 was measured directly - naming the shift in its own statement re-extracts to byte-identical QTYDBG rows (qty4 still [48,52) refs 4), so GCC emits the global read before the final `sll ...,1` regardless of spelling.

- [s10] Closed-form target for any future class-C lever, so it need not be re-derived: with the flip in place the merged dest quantity's qty_compare priority must fall below the bare lw's 20000, i.e. floor_log2(refs)*refs/span*10000 < 20000 - span > 44 (currently 28) or refs <= 15 (currently 22).

- [s10] src/text1b.c was restored to its pristine INCLUDE_ASM state at session end; no build files are left modified.

## [s11] escalation modality — the do-while(0) rule re-read, and the floor drops 9 -> 5

- [s11] Chassis re-measured at session start with the s3 candidate applied: **9** (175/175). Unchanged since s8, so every s3-s10 spelling conclusion was still chassis-valid at the start of this session.

- [s11] **THE FLOOR IS 5, NOT 9.** The s4 form banked as `rejected/s4-dw0-fence-plus-pold-move-FLOOR5-CHEAT.c` (an empty `do { } while (0);` as the first statement, plus `p_old = (s32 *)(arg0 + 0x58);` moved after the ClearOTagR call) re-measures **5 / 175 insns** on today's chassis. It was rejected in s4 as a "scheduling barrier" cheat. That rejection was made under the **2026-06-04 mechanism-scoping**, which `.claude/rules/do-while-zero-exception.md` (owner ruling **2026-07-06**) explicitly **abolishes**: its scope sentence reads "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification." The rule's body says "The former scoping to the reorg.c label-note mechanism is abolished", and its **Confirmed applications** section records `marionation_Exec` (2026-07-06) as precisely this situation - a wrap-based candidate reviewer-FAILed under the old scoping, measured honest under the construct line, and reinstated by the owner ruling. The s4 rejection is therefore STALE, not wrong-at-the-time.

- [s11] **DOCUMENTATION CONFLICT, unresolved, flagged for a ruling before any submission.** `.claude/rules/no-new-park-categories.md:256-271` still carries the OLD summary ("**Narrowly sanctioned, last-resort only** ... applies only to the LABEL_OUTSIDE_LOOP_P / reorg.c interaction, NOT a precedent for other wrappers ... User policy 2026-06-04"), and the grind role-prompt's frozen-family table quotes that stale summary verbatim. The stale entry itself designates the dedicated rule as the authority ("The dedicated rule [[do-while-zero-exception]] enumerates the strict prerequisites"), and the dedicated rule is the later document (2026-07-06 vs 2026-06-04) - but the two texts disagree on their face. This session did NOT self-approve the question: the outcome is `progress`, not `candidate-ready`, and the `ruling_question` field carries it.

- [s11] **CLASS A IS CLOSED BY THE WRAP.** Positional diff at floor 5 (`tmp/grind/func_800770B8/s3/posdiff.py`, ours 175 vs target 175) is exactly: rows 35-36 (class B), row 50 (the known LO16 scorer artifact, [[sandbox-lo16-text-addend-false-distance]]), rows 62-64 (class C). The prologue rows 7-12 that s3/s5/s9 spent three sessions on now match ROW FOR ROW. s9's sched_solver verdict ("class A is FORECLOSED to statement order", 3234-atom exhaustive sweep, 0 hits) stands and is not contradicted: the wrap is not a statement ORDER change, it inserts NOTE_INSN_LOOP_BEG/END, which is outside the solver's perturbation alphabet.

- [s11] **EXHAUSTIVE SINGLE-WRAP POSITION SWEEP, 63 positions, all measured.** An empty `do { } while (0);` was inserted at every legal statement position of the floor-5 body (every line ending in `;` or `{` that is not a declaration - preceding a declaration is illegal C89). Generator + manifest: `tmp/grind/func_800770B8/s11/sw/manifest.tsv`; results `tmp/grind/func_800770B8/s11/sweep.log`. **Minimum 5**; no second wrap position beats the one already in the form. Notable: P001/P002 (before `sp[0]=0` / `sp[1]=0`) regress to 10/11; P003/P008/P009 drop to 174 insns and score 8; the whole outer-loop-body region P025-P051 is byte-neutral at 5.

- [s11] **NESTED WRAPS DO NOT MOVE THE CONTESTED QUANTITIES.** Depth-2 (`do { do { } while (0); } while (0);`) and depth-3 wraps at 9 positions in and around the outer-loop body and the p_6a/p_7e inner loop: best 5, and **depth 3 measures byte-identically to depth 2 at every position** (8/8, 42/42, 7/7, 60/60, 5/5). This is a direct measurement against the s10 frontier's hope that loop-note ref weighting could raise the bare-lw quantity's `qty_compare` priority: extra nesting levels do not change the class-C seat at all. Log `tmp/grind/func_800770B8/s11/nsweep.log`.

- [s11] **CLASS C ON THE FLOOR-5 CHASSIS.** The s9/s10 operand flip (`(u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A`) measures **29** on the floor-5 chassis (it measured 33 on the floor-9 chassis), i.e. the +24 collateral is chassis-independent, confirming s10's "the collateral is a property of the flipped TREE". Flip plus an adjacent wrap: 29 (unchanged). Flip plus a class-B wrap: 34. A wrap immediately before the flipped pointer decls: 60 / 163 insns (it fuses the two inner loops). Banked `rejected/s11-classC-flip-on-floor5-chassis-score29.c`, `s11-classB-wrap-plus-classC-flip-score34.c`.

- [s11] **CLASS B ON THE FLOOR-5 CHASSIS.** An empty wrap between the copy and the `0x30`/`0x34` stores measures 10, after the stores 11 - consistent with s8's dump-proven finding that a NOTE-based basic-block break is undone by cse pass 2 (`after_loop=1`) and that flow.c then deletes the copy. Class B remains foreclosed. Banked `rejected/s11-classB-wrap-before-stores-score10.c`.

- [s11] **ENDGAME-LOCK GATE (a), MEASURED THIS SESSION:** `python3 tools/scan_hand_coded.py --single func_800770B8` returns **tier=LOW score=0/8** ("no strong hand-coded indicators"; S1-S8 all unset; 175 insns, 5 spills, 14 distinct regs). The canonical-asm gate FAILS - this function is compiled C, not hand-written asm, and the canonical-asm grant path is not available to it.

- [s11] Disposition NOT filed. The driver dispatched `escalation` on a flat-at-9 floor; the floor is now 5, which resets the exhaustion counter, so per the escalation brief's option (1) this session returns `progress` with the lower floor rather than a foreclosure record. No entry was added to docs/grind/decisions.md.

- [s11] `src/text1b.c` restored to its pristine `INCLUDE_ASM` state at session end; no build files left modified.

- [s11] Chassis at session start with the s3 candidate applied: score 9, 175 build insns / 175 target insns - unchanged since s8.

- [s11] THE HONEST FLOOR IS 5, measured this session on today's chassis with the annotated candidate in place: sandbox func_800770B8 --disable all = 5, 175/175 insns.

- [s11] The floor-5 form is the s3 body plus exactly two edits: (1) an empty, FAKE-annotated 'do { } while (0);' as the first statement, (2) 'p_old = (s32 *)(arg0 + 0x58);' moved to after the ClearOTagR call. The two are coupled: the move alone measures 10 (WORSE), the wrap alone 7, both together 5.

- [s11] The s4 session measured this same form at 5 and banked it as a cheat (rejected/s4-dw0-fence-plus-pold-move-FLOOR5-CHEAT.c) under the 2026-06-04 mechanism-scoping. Owner ruling 2026-07-06 in .claude/rules/do-while-zero-exception.md supersedes that scoping: scope sentence 'SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): do { ... } while (0); (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification.' Body: 'The former scoping to the reorg.c label-note mechanism is abolished'. Confirmed applications: marionation_Exec (2026-07-06) - a wrap candidate reviewer-FAILed under the old scoping and reinstated by this ruling.

- [s11] DOCUMENTATION CONFLICT (unresolved, deliberately not self-approved): .claude/rules/no-new-park-categories.md:256-271 still carries the OLD 2026-06-04 summary ('Narrowly sanctioned, last-resort only ... applies only to the LABEL_OUTSIDE_LOOP_P / reorg.c interaction'), and the grind role-prompt's frozen-family table quotes that stale text. The stale entry itself designates do-while-zero-exception as the authority, and that rule is the later document, but the two disagree on their face. This session therefore returns progress, not candidate-ready, and carries the question in ruling_question.

- [s11] Positional diff at floor 5 (tmp/grind/func_800770B8/s3/posdiff.py, 175 vs 175) is exactly three hunks: rows 35-36 (class B: ours 'sw $0,48($17) / sh $0,52($17)'; target 'sw $zero,0x30($v0) / sh $zero,0x34($v0)'), row 50 (the known LO16 scorer artifact, not a byte diff), rows 62-64 (class C: ours 'addu $2,$2,$3 / addiu $7,$2,106 / addiu $5,$2,126'; target 'addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E'). CLASS A IS CLOSED.

- [s11] s9's sched_solver verdict ('class A FORECLOSED to statement order', exhaustive 3234-atom depth-1 sweep, 0 hits) is NOT contradicted: the wrap is not a statement-order change, it inserts NOTE_INSN_LOOP_BEG/END, which is outside the solver's perturbation alphabet. The transferable lesson is that the solver's alphabet bounds its negative verdicts.

- [s11] EXHAUSTIVE 63-position single-wrap sweep on the floor-5 body: minimum 5; no position beats the existing wrap. P001/P002 regress to 10/11; P003/P008/P009 drop to 174 insns at score 8; P025-P051 (the outer-loop body) are byte-neutral at 5.

- [s11] 18 nested-wrap variants (depth 2 and depth 3 at 9 positions): best 5, and depth 3 is byte-identical to depth 2 at every position - loop-note ref weighting does not move the class-C quantities, killing the s10 frontier's only remaining route to its closed-form priority target.

- [s11] The class-C flip measures 29 on the floor-5 chassis (33 on the floor-9 chassis) - a constant +24 collateral, confirming s10's finding that the collateral is a property of the flipped tree, not of a spelling or of the surrounding chassis.

- [s11] Endgame-lock gate (a) measured: scan_hand_coded --single func_800770B8 = tier LOW, score 0/8, all of S1-S8 unset. No canonical-asm grant path.

- [s11] No disposition entry was filed in docs/grind/decisions.md: the driver dispatched escalation on a flat-at-9 floor, and the floor moved to 5, which per the escalation brief's option (1) resets the exhaustion counter and mandates a progress outcome instead.

- [s11] src/text1b.c restored to its pristine INCLUDE_ASM state at session end (git checkout); the only tree changes are under memory/grind/func_800770B8/ plus untracked tmp/ scratch.

## [s12] structural, 2026-09-01 — floor 5 (unchanged); CLASS C IS REACHABLE AND HAS BEEN REACHED

Chassis re-measured at dispatch: the s11 candidate.c body applied to src/text1b.c measures
`sandbox func_800770B8 --disable all` = **5**, 175 build insns / 175 target insns. Positional
diff re-read (tmp/grind/func_800770B8/s3/posdiff.py): class B rows 35-36, the row-50 LO16
scorer artifact, class C rows 62-64. Class A absent. Identical to the s11 record.

### The headline: the class-C flip's "+24 intrinsic collateral" was a fixed-body artifact

s10 and s11 both varied the SPELLING of the class-C plus-operand flip while holding the rest
of the outer-loop body at its ABCD statement order, measured 175/33 (floor-9 chassis) and
175/29 (floor-5 chassis) for every spelling, and concluded the collateral was intrinsic to the
flipped tree. s12 inverted the experiment: hold the flip fixed (int-domain spelling) and
permute the four store groups of the outer loop body.

Groups, as they appear in the body:
  A  `ptr = (u8 *)((t0 * 2) + (s32)base);` + sh 0 at ptr+0x10, +0x8, +0xC, +0x14, +0x3C
  B  `ptr = (u8 *)&D_800A35D0; ptr = (t0 * 4) + ptr;` + sh 0 at ptr+2, ptr+0
  C  `ptr = base + (t0 * 4);` + sh 0 at ptr+0x42, ptr+0x40
  D  `*(u8 *)(base + t0 + 0x68) = (u8)t0;`

All 24 orders measured (tmp/grind/func_800770B8/s12/perm/, log perm.log), every one 175 insns:
  CABD 12 | CADB 13 | BACD 14 | BADC 16 | CBAD 16 | CBDA 17 | BCAD 22 | CDAB 22 | BCDA 23
  ABCD 29 | ABDC 29 | ACBD 29 | ACDB 29 | ADCB 29 | DABC 29 | DACB 29 | DCAB 35 | BDCA 37
  DBCA 37 | ADBC 38 | BDAC 38 | CDBA 38 | DBAC 38 | DCBA 38
ABCD - the order every prior session used, and the order the TARGET emits its stores in - is
among the worst. CABD is the best at 12.

**The CABD build closes residual class C.** Its objdump (rows 60-64, reg-names=numeric):
    lw    $2,0($28)        <- the second D_800A36A0 read       (target: lw $v0, %gp_rel)
    sll   $3,$3,0x1        <- t0*5 -> t0*10                    (target: sll $v1,$v1,1)
    addu  $3,$3,$2         <- chain FIRST, dest = chain        (target: addu $v1,$v1,$v0)
    addiu $7,$3,106                                            (target: addiu $a3,$v1,0x6A)
    addiu $5,$3,126                                            (target: addiu $a1,$v1,0x7E)
Register seats included: the merged quantity takes $v1 and the bare lw takes $v0, which is
exactly the seat assignment s10's ra_solver run typed as REACHABLE-in-model but FORECLOSED to
every C-spellable depth-2 input perturbation. It is not foreclosed; a statement-order change
outside the class-C expression reaches it. The CABD positional diff contains no rows 62-64
hunk at all — it is class B (2 rows) + the row-50 artifact + the displacement of the C store
group I introduced by moving it (an insert of tgt[40], a 4-row delete at ours[42:46], a 3-row
insert of tgt[52:55], and a 2-row replace at 57-58).

### What does NOT recover the flip's collateral (two exhaustive sweeps + 12 hoist variants)

1. **Wraps do not reach it.** Exhaustive second-`do { } while (0);` sweep at all 79 legal
   statement positions of the flipped ABCD body: minimum 29, and 42 of the 79 positions are
   byte-identical to no wrap at all. Log tmp/grind/func_800770B8/s12/wrapflip.log. Combined
   with s11's 63-position sweep on the unflipped body, the wrap lever on this function is now
   exhaustively characterised: it reaches the prologue save-store scheduling region (worth the
   4 rows of class A) and nothing else.
2. **Integer index hoists are inert.** `s32 i2 = t0 * 2;` / `s32 i4 = t0 * 4;` hoisted to the
   top of the loop body and substituted into A/B/C: 29 with both, 29 with either alone -
   byte-identical to the un-hoisted flipped body. On the UNFLIPPED floor-5 body the same
   hoists measure 5, i.e. they are byte-neutral there (a free structural degree of freedom for
   future sessions). Log r.log (R1/R2/R3/R6).
3. **Pointer hoists partially recover it, and one of them triggers LICM.** `u8 *pC = base +
   (t0*4);` hoisted: 25. pC + `u8 *pB = (u8 *)&D_800A35D0 + (t0*4);`: 20, and adding `u8 *pA`
   and/or `u8 *pD` changes nothing (R5/R7/R8/R9 all 20). pB ALONE regresses to 40: hoisting the
   &D_800A35D0 base into its own local makes LICM lift the `lui/addiu %hi/%lo(D_800A35D0)` pair
   out of the outer loop entirely (they appear at build rows 30-31, in the prologue), whereas
   the target computes them inside the loop at rows 49-50. This is the same LICM hazard s10's
   struct-typed rederive hit, reached by a completely different construct - so "any construct
   that gives &D_800A35D0 a loop-invariant name triggers the hoist" is now a two-witness fact.
   Log r.log/r2.log (R4/R5/R7-R12).

### The state of the two residuals after s12

Class C: **REACHABLE, reached, and now a statement-ORDER problem rather than an expression
problem.** Two basins exist and neither is 0:
  * unflipped + ABCD store order  = the target's loop-head emission rows 38-61 EXACTLY, and the
    wrong plus-operand order at row 62 -> floor 5 (3 class-C rows + 2 class-B rows).
  * flipped   + CABD store order  = the target's class-C rows 60-64 EXACTLY, and the store
    groups emitted in the wrong order -> 12.
The original C therefore produced BOTH, which means our loop body still differs from the
original somewhere OUTSIDE the class-C expression and OUTSIDE the four store groups' order.
Class B is untouched by everything in this session and remains where s8/s9 left it.

### Live frontier after s12

1. **Find the statement-level structure that makes the flip free.** The two basins prove the
   original C reaches both the target's loop-head emission AND the flipped addu. s12 has now
   eliminated: the class-C expression spelling (s10, 4 forms; s12 int-domain), the four store
   groups' ORDER (24/24 measured), the spelling of groups A/B/D (8 forms, six byte-identical),
   integer index hoists, pointer hoists, and wraps (79 positions in each basin). What remains
   untried at statement level: the SHAPE of the two loops themselves - the inner
   `do { p_6a[a2] = -1; p_7e[a2] = 0; a2++; } while (a2 < 5)` written as a `for`, as a
   pointer-walking loop (`*p_6a++ = -1`), or unrolled/split into two loops; and the position of
   the `a2 = 0;` initialisers relative to the store groups (s12's permutation always kept
   `a2 = 0;` first). Probe those on the FLIPPED body and score against 29 (ABCD) / 12 (CABD).
2. **Re-run the ra_solver inverse with the goal restricted to qty3 -> $v1.** The s11 frontier's
   probe is still unspent and is now better motivated: s12 has an actual build (CABD) in which
   the seat comes out right, so `tools/ra_solver/local_extract.py` QTYDBG on the CABD build vs
   the flipped-ABCD build gives a MEASURED pair of allocator states that differ only in the
   contested seat - a far stronger input to `inverse.py local` than the synthetic depth-2
   atom space s10 searched. Extract both, diff the qty rows, and read off which atom the store
   group order actually moved.
3. **Class B (2 rows) stays foreclosed.** Nothing in s12 touched it; s8's dumps and s9's
   measurement stand. Do not re-open it by spelling search.

- [s12] Chassis re-measured at dispatch: the s11 candidate body applied to src/text1b.c measures sandbox --disable all = 5, 175 build insns / 175 target insns, residual = class B rows 35-36 + the row-50 LO16 scorer artifact + class C rows 62-64. Class A absent. Matches the ledger.

- [s12] Class C is REACHABLE and has been REACHED in a real build. The CABD-store-group-order flipped body emits addu $3,$3,$2 / addiu $7,$3,106 / addiu $5,$3,126 after lw $2,0($28) / sll $3,$3,0x1 - byte-equal to the target's rows 60-64 (addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E). This overturns the s10 typing 'class C is REACHABLE-in-model but FORECLOSED to every C-spellable input perturbation'.

- [s12] The 24-way store-group permutation spread is 12..38 with the target's own emission order (ABCD) sitting at 29, near the worst. Full table in evidence.md [s12]. Statement ORDER, not expression spelling, is this function's dominant structural lever.

- [s12] The wrap lever on func_800770B8 is now exhaustively characterised across three sweeps (s11: 63 positions unflipped; s12: 79 positions flipped-ABCD, 79 positions flipped-CABD). It reaches the prologue save-store scheduling region (the 4 rows of class A) and, in the CABD basin, 2 rows of store-group displacement. It reaches nothing else - 42/79 and 38/79 positions are byte-identical to no wrap at all.

- [s12] Integer-valued hoists (s32 i2 = t0*2; s32 i4 = t0*4;) are byte-neutral on the floor-5 body and completely inert on the flipped body; pointer-valued hoists are what move this function's loop-head schedule. A hoisted &D_800A35D0 pointer local triggers a LICM lift of its lui/addiu out of the outer loop (score 40) - the same hazard s10's struct-typed rederive hit, now witnessed by a second, unrelated construct.

- [s12] The two basins do not compose: unflipped+ABCD gives the target's loop-head rows 38-61 exactly with the wrong operand order at row 62 (score 5); flipped+CABD gives the target's rows 60-64 exactly with the store groups displaced (score 12, 10 with a second wrap). The original C produced both, so our loop body still differs from the original somewhere outside the class-C expression, outside the group order, and outside the A/B/D spellings - all three of which s12 measured exhaustively.

- [s12] Class B (rows 35-36) was not touched by anything in s12; s8's instrumented-cc1 dumps and s9's 176-insn measurement stand unchanged.

## [s13] structural — 2026-09-01 — floor 5 (unchanged); the s12 loop-SHAPE frontier is CLOSED and class C is re-typed to a single RTL plus-operand-order token with a measured 24-point collateral cost

Chassis re-measured at session start: `sandbox func_800770B8 --disable all` on
`memory/grind/func_800770B8/candidate.c` = **score 5 / build_insns 175 / target 175**,
identical to the floor the s11/s12 ledger recorded. No chassis drift; every s12
conclusion below is spent against the same numbers it was banked with.

Positional diff of the floor-5 form re-run this session
(`tmp/grind/func_800770B8/s3/posdiff.py`, 175 vs 175) — unchanged from s11:
rows 35-36 (class B), row 50 (the known LO16 scorer artifact,
[[sandbox-lo16-text-addend-false-distance]]), rows 62-64 (class C).

### 1. The s12 frontier hypothesis — "a loop-SHAPE change makes the class-C flip free" — is KILLED, on all three loops

s12 left the loop shapes as the last unenumerated statement-level structure.
This session enumerated all three of them, on four bases in parallel
(F = the floor-5 unflipped-ABCD candidate; A = flipped ABCD, `s12/perm/Q00.c`,
baseline 29; B = flipped BACD, `s12/perm/Q06.c`, baseline 14; C = flipped CABD,
`s12/perm/Q12.c`, baseline 12).

**(a) Inner p_6a/p_7e loop shape — 48 builds, `tmp/grind/func_800770B8/s13/sh.log`.**

| shape | F (5) | A (29) | B (14) | C (12) | insns |
|---|---|---|---|---|---|
| `do { } while (a2 < 5)` (current) | 5 | 29 | 14 | 12 | 175 |
| `for (a2 = 0; a2 < 5; ...)` | 5 | 29 | 14 | 12 | 175 |
| `while (a2 < 5) { }` | 5 | 29 | 14 | 12 | 175 |
| pointer-walking `*p_6a++ = -1; *p_7e++ = 0;` | 26 | 48 | 37 | 35 | **173** |
| split into two separate loops | 26 | 50 | 37 | 35 | **186** |
| the two stores swapped (`p_7e` first) | 9 | 33 | 18 | 16 | 175 |

for / while / do-while are **byte-identical** on every base — GCC 2.7.2 lowers all
three to the same RTL here, so the inner loop's syntactic shape carries no
information at all. Pointer-walking is *insn-count disqualifying*: it elides two
instructions the target has (173 vs 175), which is positive evidence the original
source indexed `p_6a[a2]` rather than post-incrementing. Splitting the loop costs
eleven instructions. Swapping the two stores costs a uniform +4 on every base.

Also measured in the same sweep: moving the `a2 = 0;` initialiser from the top of
the outer-loop body down to immediately before the inner-loop block. Byte-neutral
on F, A and C; costs +2 on B (14 -> 16). This was s12's explicit "a2 = 0 always kept
first" caveat — it is now measured and inert on every basin of interest.
Banked: `rejected/s13-inner-loop-pointer-walk-173insn-score26.c`,
`rejected/s13-inner-loop-split-into-two-loops-186insn-score26.c`,
`rejected/s13-inner-loop-store-order-swapped-score9.c`.

**(b) Inner-loop POSITION — a degree of freedom s12 never had. 240 builds,
`tmp/grind/func_800770B8/s13/p5.log` + `s13/p5/manifest.tsv`.**

s12's exhaustive "24/24 store-group orders" permuted only the four store groups
A/B/C/D and *always left the p_6a/p_7e block after all four*. This session
promoted the inner-loop block to a fifth permutable element and measured all
5! = 120 orders x {unflipped, flipped} = 240 builds.

Result: **every one of the 96 orders that does not put I last is worse than the
corresponding I-last order.** Top of the table:

    5  ABCDI u   <- the floor-5 candidate (control; reproduces exactly)
    7  ABDCI u
   12  CABDI f   <- reproduces s12's flipped-CABD = 12 exactly (control)
   13  ACBDI u / CADBI f
   14  ACDBI u / BACDI f
   15  CABDI u

Not one variant with I in a non-final position appears in the top 24. Hoisting the
inner loop ahead of any store group is strictly harmful, and the two controls
(ABCDI u = 5, CABDI f = 12) reproduce s12's numbers to the point, which validates
the harness. Best non-baseline order banked as
`rejected/s13-inner-block-hoisted-before-groupC-score7-best-of-96.c`.
Side datum: the 96 non-I-last orders produce build_insns in {173, 174, 175} — some
of them lose instructions the target has, so they are disqualified on count alone.

**(c) Outer loop and third loop shape — 27 builds, `tmp/grind/func_800770B8/s13/lp.log`.**

Outer `t0` loop written three ways x third `a2 < 0xA` loop written three ways, on
F / A / C:

- **Outer loop shape is completely byte-neutral.** `t0 = 0; do { } while (t0 < 2)`,
  `for (t0 = 0; t0 < 2; t0 = (s16)(t0 + 1))`, and `t0 = 0; while (t0 < 2) { }` produce
  *identical bytes* on all three bases (F 5/5/5, A 29/29/29, C 12/12/12). Another
  zero-information axis.
- **The third loop's `for` shape is uniquely correct.** Rewriting it as
  `a2 = 0; do { ... } while (a2 < 0xA)` or `a2 = 0; while (a2 < 0xA) { ... }` drops
  build_insns from 175 to **174** and costs +1 score on every base (F 5->6,
  A 29->30, C 12->13). The target has 175 instructions, so the `for` spelling
  already in the candidate is confirmed against its two alternatives rather than
  merely assumed. Banked `rejected/s13-third-loop-as-dowhile-174insn-score6.c`.

**Verdict: the s12 frontier's "loop SHAPE" hypothesis is KILLED with 315
measurements.** All three loops are either byte-neutral under every alternative
spelling (inner, outer) or already at their unique optimum (third). There is no
loop-shape degree of freedom left in this function.

### 2. Class C re-typed: the residual is exactly ONE RTL plus-operand-order token, and its 24-point collateral cost is now isolated

Reading the two builds instruction-for-instruction (`tmp/grind/func_800770B8/s13/ours.py`)
produced a much sharper statement of class C than s6/s10/s12 had.

**Our floor-5 build already reproduces the target's entire address chain.** Rows
37-61 are identical to the target row for row, including the non-obvious part: the
target does **not** compute `t0 * 10` directly. It computes `t0 * 4` for the 0x40/0x42
store group (row 42 `sll $v1, $a1, 2`), then reuses it — row 56 `addu $v1, $v1, $a1`
makes `t0 * 5`, and row 61 `sll $v1, $v1, 1` makes `t0 * 10`. Our build emits exactly
that chain in exactly those registers (`addu $3,$3,$5` at 56, `sll $3,$3,0x1` at 61),
and rows 60-61 already put the reloaded `D_800A36A0` in `$v0`/`$2` and the shift chain
in `$v1`/`$3` — **the target's own seats.** The prior sessions' framing of class C as
a local-alloc seat question (s10's QTYDBG priority target) is therefore obsolete on
this form: the seats are already right.

What is left is one token:

    ours   62  addu $2,$2,$3        (= addu $v0, $v0, $v1  — reloaded global first)
    target 62  addu $v1, $v1, $v0   (                       — shift chain first)

Both are `addu rd, rs, rt` with `rd == rs`, so the printed operand order *is* the RTL
`(plus A B)` operand order, i.e. it is the source-level operand order surviving
`fold`. GCC 2.7.2's `fold()` only commutes a binary operand pair to move a *constant*
to the second position; it has no complexity-based swap, so the source order is
preserved verbatim. Confirming s6's attribution, and narrowing it: it is not "an RTL
order question at insn 173", it is *this one addend pair*.

**17 spellings of the inner-block address measured** (`tmp/grind/func_800770B8/s13/addr.log`,
`s13/addr/manifest.tsv`), all on the floor-5 base:

| spelling of `p_6a` | score | insns |
|---|---|---|
| `(s16 *)(D_800A36A0 + (t0 * 10) + 0x6A)` (current) | 5 | 175 |
| `(s16 *)((s32)D_800A36A0 + (t0 * 10) + 0x6A)` | **5** | 175 |
| `(s16 *)((t0 * 10) + D_800A36A0 + 0x6A)` (no cast) | **5** | 175 |
| `(s16 *)(D_800A36A0 + (t0 * 10)) + 0x35` | 5 | 175 |
| flip on `p_7e` only, `p_6a` unflipped | **5** | 175 |
| `(s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A)` (the flip) | 29 | 175 |
| flip on `p_6a` only, `p_7e` unflipped | **29** | 175 |
| `s32 off = t0 * 10;` + `(s16 *)(off + (s32)D_800A36A0 + 0x6A)` | 29 | 175 |
| `(s16 *)((t0 * 5) * 2 + (s32)D_800A36A0 + 0x6A)` | 29 | 175 |
| `(s16 *)((t0 * 10) + (s32)D_800A36A0) + 0x35` | 29 | 175 |
| `&((s16 *)(D_800A36A0 + 0x6A))[t0 * 5]` | 9 | 176 |
| `(s16 *)((t0 * 10) + (s32)(D_800A36A0 + 0x6A))` | 9 | 176 |
| `(s16 *)(&D_800A36A0[(t0 * 10) + 0x6A])` | 9 | 176 |
| `(s16 *)(0x6A + (t0 * 10) + (s32)D_800A36A0)` | 16 | 176 |
| `(s16 *)((t0 * 10) + 0x6A + (s32)D_800A36A0)` | 16 | 176 |
| `(s16 *)((t0 * 10) + (s32)base + 0x6A)` | 41 | **174** |
| `(s16 *)(base + (t0 * 10) + 0x6A)` | 41 | **174** |

Four results worth carrying forward:

1. **The `(s32)` cast is not the trigger; the operand ORDER is.** Writing the whole
   address as integer arithmetic with the global still *first*
   (`(s32)D_800A36A0 + (t0 * 10) + 0x6A`) is byte-identical to the current form (5).
   So class C is not a type/pointer-arithmetic artefact — the cost attaches solely
   to which addend the source names first.
2. **Writing the addend first without a cast does not work.** `(t0 * 10) + D_800A36A0`
   scores 5, i.e. the C front end's pointer-arithmetic lowering canonicalises the
   pointer back to the first position. The cast is the *only* way to express the flip,
   which is why every flipped spelling collapses onto the same 29.
3. **`p_7e`'s spelling is inert; `p_6a`'s spelling is everything.** Flipping only
   `p_7e` measures 5 (byte-identical to the unflipped form); flipping only `p_6a`
   measures 29 (byte-identical to flipping both). CSE derives `p_7e` from `p_6a`'s
   address, so the contested `addu` is emitted from `p_6a`'s expression alone. Any
   future probe of class C only has to vary `p_6a`.
   Banked `rejected/s13-classC-flip-p7e-only-inert-score5.c`.
4. **Routing the flip through the `base` local deletes the second `lw`** (174 insns,
   score 41) — the target reloads `D_800A36A0` from `$gp` at row 60, so the inner
   block must re-read the global and not reuse the outer-loop copy. That closes
   the "use `base` in the inner block" idea permanently.
   Banked `rejected/s13-classC-flip-through-base-local-drops-second-lw-174insn-score41.c`.

**The flip's collateral cost, read out.** Posdiff of flipped-ABCD (`s12/perm/Q00.c`,
29) against the target shows the damage is *not* in the inner block at all — rows
60-64 of the flipped build are `lw $3 / sll $2,$2,1 / addu $2,$2,$3 / addiu $7,$2,106 /
addiu $5,$2,126`, i.e. the operand order is now RIGHT (shift chain first) but the two
hard registers have *swapped* ($2 = shift, $3 = load, where the target has $v1 = $3 =
shift, $v0 = $2 = load), so the printed insn is `addu $2,$2,$3` again and the row still
mismatches. The 24 extra points all sit in rows 38-59: under the flip GCC hoists the
`lw` ahead of the `sll` (ours 40 `lw $3` / 41 `sll $2,$4,0x1` vs target 40 `sll $v0,$a1,1`
/ 41 `lw $a0`) and delays the `sll ...,2` from row 42 to row 50, which re-seats the whole
loop head.

So class C is a genuine two-part lock, and this session can state it precisely for
the first time: **the flip fixes the operand order but swaps the seats; s12's CABD
group order fixes the seats but breaks the store rows. Neither the flip nor the
group order can be applied without the other's damage, and the damage is emission
order in rows 38-59, not allocation.** Every remaining lever must move the *emission
order of the loop head under the flip*, not the inner block.

### 3. What this session did NOT touch (deliberately, per the brief)

Group order (s12, 24/24), group spelling (s12, 8 forms), pointer/index hoists (s12,
12 forms), wrap positions (s11 63-position + 18 nested; s12 79+79 second-wrap) were
all left alone — they are banked exhausted. Class B (rows 35-36) was not re-probed;
it remains foreclosed by s7/s8/s9 measurement + dumps. The do-while(0) scoping
conflict at `.claude/rules/no-new-park-categories.md:256-271` vs
`.claude/rules/do-while-zero-exception.md` is carried forward unresolved.

- [s13] Chassis re-measured at session start and again at session end: memory/grind/func_800770B8/candidate.c = score 5 / build_insns 175 / target 175. No drift from the s11/s12 ledger floor; every s12 conclusion is spent against the same numbers it was banked with. Positional diff unchanged: rows 35-36 (class B), row 50 (the known LO16 scorer artifact), rows 62-64 (class C).

- [s13] LOOP SHAPE IS FULLY EXHAUSTED (315 builds). Inner p_6a/p_7e loop as `do { } while (a2 < 5)`, `for (a2 = 0; a2 < 5; ...)` and `while (a2 < 5) { }` are byte-identical on all four bases tested. Outer t0 loop as do-while, for and while are byte-identical on all three bases tested. GCC 2.7.2 lowers all of them to the same RTL here.

- [s13] The third loop's `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1))` is uniquely correct: rewriting it as `a2 = 0; do { ... } while (a2 < 0xA)` or as a while-loop drops build_insns from 175 to 174 and costs +1 score on every base. The candidate's spelling is now confirmed against its alternatives rather than assumed.

- [s13] Pointer-walking the inner loop (`*p_6a++ = -1; *p_7e++ = 0;`) produces 173 build_insns - two FEWER than the target's 175. That is positive evidence the original source indexed `p_6a[a2]` rather than post-incrementing, and it disqualifies the whole post-increment family on count alone.

- [s13] The inner-loop BLOCK is a fifth permutable element that s12 never varied (s12 permuted only the four store groups and always left the block last). All 120 orders x {flipped, unflipped} = 240 builds measured: every one of the 96 non-final positions is worse than its final-position counterpart, best alternative ABDCI = 7 vs the ABCDI = 5 baseline, and several non-final orders produce 173/174 insns. Hoisting the inner loop ahead of any store group is strictly harmful.

- [s13] CLASS C IS ONE SOURCE-LEVEL OPERAND-ORDER TOKEN, NOT AN ALLOCATION RESIDUAL. The floor-5 build matches the target row for row from 37 to 61, including the target's t0*4 -> +t0 -> <<1 reuse chain for t0*10 (rows 42, 56, 61) and the target's exact seats at rows 60-61 ($v0 = reloaded global, $v1 = shift chain). The only mismatch is `addu $2,$2,$3` vs `addu $v1,$v1,$v0`. Since both are addu rd,rs,rt with rd==rs, the printed operand order is the RTL plus operand order, which is the source order verbatim: GCC 2.7.2's fold() commutes a binary pair only to move a CONSTANT to the second position and has no complexity-based swap.

- [s13] The (s32) cast is NOT the trigger - the order is. `(s16 *)((s32)D_800A36A0 + (t0 * 10) + 0x6A)` (fully integer arithmetic, pointer named first) is byte-identical to the candidate at 5/175. Conversely `(s16 *)((t0 * 10) + D_800A36A0 + 0x6A)` (addend first, no cast) is also 5, because the C front end canonicalises the pointer back to first position. The cast is therefore the ONLY way to express the flip, and all seven flipped spellings collapse onto exactly 29/175.

- [s13] Only p_6a carries degrees of freedom: flipping p_7e alone is inert (5, byte-identical to unflipped); flipping p_6a alone is byte-identical to flipping both (29). cse derives p_7e's address from p_6a's. This halves the class-C search space for every future session.

- [s13] The inner block MUST re-read D_800A36A0 from $gp. Routing its address through the live `base` local (either operand order) deletes the second lw, giving 174 build_insns / score 41 - and the target has that lw at row 60. The `base`-routed inner-block family is closed permanently.

- [s13] The flip's 24-point collateral cost is emission ORDER in the loop head, not allocation. Posdiff of flipped-ABCD shows rows 60-64 have the operand order RIGHT (shift chain first) but the two hard registers swapped ($2 = shift, $3 = load, against the target's $v1 = $3 = shift, $v0 = $2 = load), so the printed insn is `addu $2,$2,$3` again and the row still mismatches; all 24 extra points sit in rows 38-59, where GCC hoists the `lw` ahead of the `sll` (ours 40 `lw $3` / 41 `sll $2,$4,0x1` vs target 40 `sll $v0,$a1,1` / 41 `lw $a0`) and delays the `sll ...,2` from row 42 to row 50.

- [s13] Consolidated statement of the lock: the flip fixes the operand order but swaps the seats; s12's CABD group order fixes the seats but breaks the store rows. Neither can be applied without the other's damage, and the damage is loop-head emission order in rows 38-59.

- [s13] Not re-probed this session (banked exhausted by s11/s12, per the brief): group order 24/24, group spelling, pointer/index hoists, wrap positions (63-position single + 18 nested + 79+79 second-wrap). Class B (rows 35-36) remains foreclosed by s7/s8/s9. The do-while(0) scoping conflict between .claude/rules/no-new-park-categories.md:256-271 and .claude/rules/do-while-zero-exception.md is carried forward unresolved.

## [s14] synthesis — 2026-09-01 — FLOOR 5 -> 4; RESIDUAL CLASS C IS CLOSED for the first time in fourteen sessions

Chassis re-measured at session start: `sandbox func_800770B8 --disable all` on the
inherited s11 body = **score 5 / build_insns 175 / target 175**, identical to the
s11/s12/s13 ledger floor. No drift; every inherited conclusion is spent against the
same numbers it was banked with. (The dispatch brief's "measurement unavailable" is
a driver-side gap, not a chassis change.)

### 1. The s13 frontier's item 1 is CONFIRMED, and it closes class C

s13 left this as the strongest hypothesis: *under the p_6a operand flip, the loop
head's emission order (rows 38-59) can be restored by changing what the loop head
COMPUTES rather than how the inner block spells its address — the lever is an
additional early CONSUMER of `t0*4`.* s14 spelled that as eleven perturbations of the
loop head's first-demand order and measured them on three bases in one sweep
(`tmp/grind/func_800770B8/s14/sweep.log`, 33 builds; generator `s14/gen.py`;
F = the floor-5 candidate, A = flipped-ABCD `s12/perm/Q00.c` (29),
C = flipped-CABD `s12/perm/Q12.c` (12)):

| perturbation | F (5) | A (29) | C (12) | insns |
|---|---|---|---|---|
| P0 control | 5 | 29 | 12 | 175 |
| P1 group C into the integer domain, addend first | 6 | 29 | 12 | 175 |
| P2 group A into the pointer domain, base first | 6 | 29 | 13 | 175 |
| P3 group B in one step (`&D_800A35D0 + t0*4`) | 10 | 31 | 17 | 175 |
| P4 0x5C/0x60 pair hoisted to the top of the loop body | 50 | 48 | 57 | **173** |
| P5 0x5C/0x60 pair moved to the inner-loop boundary | 71 | 71 | 71 | **174** |
| P6 group D hoisted above every t0*N group | 40 | 29 | 35 | 175 |
| P7 group C's shift as `t0 << 2` | 5 | 29 | 12 | 175 |
| **P8 group A's index as `(t0 * 4) >> 1`** | 7 | **4** | 12 | 175 |
| P9 inner index as `(t0*4)*2 + t0*2` | 5 | 43 | 43 | **178** |
| P10 groups B and C both integer-domain, addend first | 11 | 31 | 17 | 175 |

**P8 on the flipped-ABCD base measures 4/175 — a new floor.** Positional diff
(`s3/posdiff.py`) of that build:

    replace ours[35:37] / tgt[35:37]      <- class B, unchanged
      O 35 sw $0,48($17)     T 35 sw $zero, 0x30($v0)
      O 36 sh $0,52($17)     T 36 sh $zero, 0x34($v0)
    insert tgt[40:42]        T 40 sll $v0, $a1, 1
                             T 41 lw $a0, %gp_rel(D_800A36A0)($gp)
    delete ours[41:43]       O 41 lw $4,0($28)
                             O 42 sra $2,$3,0x1
    replace ours[50:51]                   <- the known LO16 scorer artifact
      O 50 addiu $2,$2,0     T 50 addiu $v0, $v0, %lo(D_800A35D0)

Rows 43-64 now match the target row for row. **Residual class C (rows 62-64,
`addu $2,$2,$3` vs `addu $v1,$v1,$v0`) — the lock s6 through s13 could not break —
is GONE.** Class A was already gone at floor 5. What replaces class C is a smaller,
sharper residual, recorded below as **class D**.

### 2. Class D — the new 2-row residual, fully characterised

Our loop head (`s13/ours.py 36 52`) vs the target, same three slots:

    ours    40 sll $3,$5,0x2      (t0*4)      target 40 sll $v0,$a1,1   (t0*2)
    ours    41 lw  $4,0($28)      (global)    target 41 lw  $a0,...     (global)
    ours    42 sra $2,$3,0x1      (t0*2)      target 42 sll $v1,$a1,2   (t0*4)

Same slot count, same lw position, and the two quantities land in the target's own
registers by role ($3/$v1 carries t0*4, $2/$v0 carries t0*2). Only two things differ:
the ORDER of the two shifts and the opcode of the second — because our t0*2 is
*derived from* t0*4 (`sra`), whereas the target computes both shifts directly from
t0. That derivation is exactly the token that closes class C, so class D is the
price of the class-C fix, and the two are currently welded together.

### 3. The dependence DIRECTION is what matters, not the reference count

13 builds, `s14/sweep2.log` (generator `s14/gen2.py`), all on flipped-ABCD:

| variant | score | insns |
|---|---|---|
| D03 group A `(t0*4) >> 1` (= P8) | **4** | 175 |
| D06 group A `(u32)(t0*4) >> 1` (srl) | **4** | 175 |
| D09 named `s32 i4 = t0*4;` + `i4 >> 1`, C uses `i4` | **4** | 175 |
| D12 D03 + group C as `base + (t0 << 2)` | **4** | 175 |
| D04 `(s32)base + ((t0*4) >> 1)` (base named first) | 5 | 175 |
| D05 `base + ((t0*4) >> 1)` (pointer domain) | 5 | 175 |
| D11 D03 + group C into the integer domain | 5 | 175 |
| D08 `u8 *pC = base + t0*4;` hoisted, C uses pC | 25 | 175 |
| D01 group C as `base + ((t0*2) * 2)` | 29 | 175 |
| D02 group C as `base + ((t0*2) << 1)` | 29 | 175 |
| D07 group A as `(t0*4) / 2` | 29 | 175 |
| D10 named `s32 i4 = t0*4;`, C uses it, A unchanged | 29 | 175 |
| D14 D03 + the 0x5C/0x60 pair also `(t0*4) >> 1` | 14 | 176 |

Four facts fall out:

1. **The direction is asymmetric.** t0*2 derived from t0*4 = 4. t0*4 derived from
   t0*2 (`(t0*2)*2`, `(t0*2)<<1`) = 29, i.e. it does nothing at all.
2. **`/ 2` is folded away.** `(t0*4) / 2` measures exactly the unperturbed 29, so
   GCC 2.7.2's `fold()` collapses the exact division back to `t0*2` and the
   dependence never reaches RTL. Only a shift survives. (This is a genuinely useful
   toolchain datum: the mult/exact-div identity folds, the mult/shift identity does
   not.)
3. **Naming alone is inert.** `s32 i4 = t0*4;` used only by group C = 29; the same
   local additionally consumed by group A as `i4 >> 1` = 4. The local is not the
   lever; the consumption is.
4. **Spelling of the surrounding groups is second-order** — group C in the integer
   domain, `t0 << 2`, or base-first ordering move the result by at most 1.

### 4. Bumping the t0*4 reference count WITHOUT the dependence does not work

8 builds, `s14/sweep4.log` (generator `s14/gen4.py`), on flipped-ABCD. The idea was
to raise `reg_n_refs` on the t0*4 pseudo without paying class D's `sra`:

| variant | score | insns |
|---|---|---|
| E5 group C's two stores spelled with the index inline (2 refs, no `ptr`) | 25 | 175 |
| E6 the same on group B | 35 | 177 |
| E7 both | 12 | 177 |
| E8 group A's five stores spelled with the index inline | 29 | 175 |
| E1 the 0x5C/0x60 pair via `(t0*4) >> 1`, group A unchanged | 39 | **176** |
| E4 group D via `base + ((t0*4) >> 2)` | 7 | **176** |
| E9 P8 + group C's stores spelled inline | 5 | 175 |
| E10 group C's stores inline, group A back to `t0*2` | 25 | 175 |

Nothing reaches 4. Reference count alone moves the flipped base from 29 to 25 (E5)
and to 12 (E7, but at 177 insns — disqualified on count). **It is the dependence
edge, not the reference count, that reverses the shift birth order and re-seats the
contested plus.**

### 5. Class B re-confirmed dead ON THIS CHASSIS

s7/s8/s9 foreclosed class B (rows 35-36: the 0x30/0x34 stores go through the `p_old`
copy rather than the raw `func_8006E49C` result) on the floor-9 chassis. Re-measured
here on floor-4 with four fresh spellings of the whole post-call block
(`s14/sweep3.log`, generator `s14/gen3.py`):

| variant | score | insns |
|---|---|---|
| B1 fresh `u8 *nb` result local, stores first, `p_old`/global assigned after | 24 | **170** |
| B3 result assigned straight into `D_800A36A0`, `p_old` derived after | 22 | **170** |
| B4 fresh `u8 *nb`, global first, `p_old` last | 22 | **170** |
| B2 all four stores spelled through `(u8 *)p_old` | 6 | 175 |

Every spelling that actually reaches the raw call-result pseudo collapses the block
to **170 instructions — five FEWER than the target's 175** — which is the same
failure mode s7 recorded (the copy the target keeps is deleted by flow.c). The one
spelling that preserves 175 is byte-worse. Class B stays foreclosed, now on the
current chassis.

### 6. A second do-while(0) wrap is inert on the floor-4 chassis

79-position second-wrap sweep on the floor-4 body (`s14/wrap.log`, positions
generated by `s12/gen_wrap.py`): **minimum 4**, reached at 34 of the 79 positions,
all byte-identical to the no-second-wrap build. Nothing goes below 4; the harmful
positions are the familiar ones (inside the inner loop / around the call block,
down to 67-163 insns). The floor-4 form therefore carries exactly ONE wrap — the
s11 prologue fence — and s12's "the CABD basin needs a SECOND wrap" note no longer
applies to the live basin.

### 7. The open classification question (carried to the next session)

The floor-4 form differs from the floor-5 form in exactly two source tokens:
(1) `p_6a`/`p_7e` written flipped — ordinary C, and s13 proved this is the operand
order the original source must have had; (2) group A's index written `(t0 * 4) >> 1`
instead of `t0 * 2`. Token (2) is an arithmetic identity with no semantic purpose
(cheat-checklist T1) that no human would write from the specification (T2), and its
justification is a named GCC mechanism (T3). It is not obviously covered by any
frozen family: the sanctioned list carries "opaque arithmetic variables" while the
forbidden catalog carries "`s32 one = 1;` opaque variable to defeat single-bit
transform", and the two are in tension for this shape. **This is a first reach, so
the honest cheat-free floor remains 5 until a ruling says otherwise, and the s14
outcome is a ruling-request rather than a submission.** The STRUCTURAL finding — that
an early t0*4 dependence in the loop head closes class C — is independent of how
token (2) is ultimately spelled, and is the real inheritance from this session.

- [s14] FLOOR 5 -> 4 and RESIDUAL CLASS C IS CLOSED. Deriving group A's half-word index from the t0*4 pseudo (`(t0 * 4) >> 1` instead of `t0 * 2`) on the flipped-p_6a ABCD body measures 4/175 and emits the contested addition as `addu $v1,$v1,$v0` with the target's own seats; rows 43-64 match the target row for row. This confirms the s13 frontier item 1 exactly: the lever was what the LOOP HEAD COMPUTES, not how the inner block spells its address.

- [s14] The class-C fix costs a NEW 2-row residual (class D): ours 40 `sll $3,$5,0x2` / 41 `lw $4` / 42 `sra $2,$3,0x1` against target 40 `sll $v0,$a1,1` / 41 `lw $a0` / 42 `sll $v1,$a1,2`. Same slots, same lw position, same registers by role - only the order of the two shifts and the opcode of the second differ, because our t0*2 is derived from t0*4 while the target computes both from t0 directly.

- [s14] The dependence DIRECTION is asymmetric and load-bearing: t0*2 derived from t0*4 gives 4; t0*4 derived from t0*2 (`(t0*2)*2` or `(t0*2)<<1`) gives 29, i.e. is completely inert.

- [s14] GCC 2.7.2's fold() collapses `(t0 * 4) / 2` back to `t0 * 2` (measures exactly the unperturbed 29) but does NOT collapse `(t0 * 4) >> 1`. The mult/exact-division identity folds; the mult/shift identity survives to RTL. Reusable toolchain datum.

- [s14] Naming is not the lever: `s32 i4 = t0 * 4;` consumed only by group C measures 29; the same local additionally consumed by group A as `i4 >> 1` measures 4. `(u32)(t0*4) >> 1` (srl) also measures 4.

- [s14] Raising reg_n_refs on the t0*4 pseudo WITHOUT creating the dependence does not close class C: spelling group C's two stores with the index inline (two refs instead of one through `ptr`) gives 25, the same on group B gives 35/177, both together 12/177, group A's five stores inline gives 29. It is the dependence edge, not the reference count.

- [s14] Loop-head first-demand order is otherwise exhausted (33 builds, three bases x 11 perturbations): group C into the integer domain, group A into the pointer domain, group B in one step, `t0 << 2`, and group D hoisted are all neutral-or-worse everywhere; hoisting the 0x5C/0x60 pair to the loop top (173 insns) or to the inner-loop boundary (174 insns) is catastrophic on every base.

- [s14] CLASS B RE-CONFIRMED FORECLOSED ON THE FLOOR-4 CHASSIS (s7/s8/s9 measured it on the floor-9 chassis). Four fresh spellings of the post-call block: every one that reaches the raw `func_8006E49C` result pseudo collapses the function to 170 instructions - five FEWER than the target's 175 - and the one spelling that preserves 175 is byte-worse (6).

- [s14] A SECOND `do { } while (0);` is inert on the floor-4 chassis: 79-position sweep, minimum 4, reached at 34 of 79 positions all byte-identical to the unwrapped build, nothing below 4. The live form carries exactly one wrap (the s11 prologue fence).

- [s14] CLASSIFICATION OPEN: the floor-4 form's second token, `(t0 * 4) >> 1` for a half-word index, is an arithmetic identity with no semantic purpose (T1), which no human would write from the specification (T2), justified by a named GCC mechanism (T3). It is a first reach - the sanctioned "opaque arithmetic variables" entry and the forbidden "`s32 one = 1;` opaque variable" entry are in tension for this shape. Until a ruling lands, the honest cheat-free floor is 5 and the floor-4 form is NOT submittable.

## [s15] synthesis — 2026-09-01 — FLOOR 5 (re-verified on today's chassis); the Judge-suggested replacement path for class C is MEASURED DEAD, and the do-while(0) classification question is CLOSED BY CITATION

(Driver session index 14; ledger label [s15] because the previous session already
wrote [s14] — that session's `ruling-request` was answered at
docs/grind/decisions.md, `2026-09-01 07:23 — func_800770B8 — **FAIL**`.)

### 0. Chassis re-measurement (first action of the session)
The s11/s12/s13 body (unflipped ABCD, one prologue wrap) measures **5 / 175 insns /
0 rules dropped** on today's HEAD chassis, so every conclusion below is
chassis-current. The s14 floor-4 body (`(t0 * 4) >> 1`) is Judge-FAILED and has
been moved to `rejected/s14-classD-identity-detour-JUDGE-FAILED-2026-09-01.c`;
`candidate.c` is once again the floor-5 body, re-verified at 5 with the s15 header.

### 1. The Judge's own suggested replacement for the identity is DEAD (8 builds)
The FAIL entry closed with a concrete alternative: *"pursue the type-forced
derivation instead — a cursor really derived from group C's t0*4 pointer has a
semantic reading and would not emit a `sra` the target does not have."* s15 built
that literally. Group C is turned into a named cursor
`s16 *rowC = (s16 *)(base + (t0 * 4)); rowC[0x21] = 0; rowC[0x20] = 0;` hoisted to
the top of the outer-loop body, and group A's base is then derived from `rowC`
four different ways (`tmp/grind/func_800770B8/s14/b/gen6.py`, `b/w/`,
`b/sweep6.log`), on both the unflipped (5) and flipped-ABCD (29) bases:

| group A base, derived from `rowC` | flipped base | unflipped base |
|---|---|---|
| `(u8 *)base + (((u8 *)rowC - base) >> 1)` | 36 / **176** | 38 / **176** |
| `(u8 *)base + (rowC - (s16 *)base)` | 36 / **176** | 38 / **176** |
| `(u8 *)base + (((u8 *)rowC - base) / 2)` | 54 / **178** | 54 / **178** |
| `(u8 *)rowC - (t0 * 2)` | 51 / 175 | 51 / 175 |

**GCC 2.7.2 does not fold a pointer difference back into the shared shift.** Both
`>>1` forms materialise the `subu` (and, for the `s16 *` difference, its own
`sra`), costing exactly the instruction the Judge's condition forbids ("emits no
instruction the target lacks"); the `/2` form costs three; and subtracting `t0*2`
from the cursor merely re-introduces the very pseudo the derivation was supposed
to avoid computing. There is therefore **no truthful cursor derivation that
supplies the t0*4 -> t0*2 dependence edge for free**, and the sanctioned-path
door the FAIL entry left open is now measured shut.

### 2. Type-forced derivation (s14 frontier item 1c) is DEAD (20 builds)
`tmp/grind/func_800770B8/s14/b/gen5.py`, `b/v/`, `b/sweep5.log` — ten
perturbations x three bases (F = unflipped 5, A = flipped ABCD 29, C = flipped
CABD 12). s10 killed a WHOLE-BODY struct rewrite (178 insns); s15 applies types
only to groups A and C, which s14's frontier reserved as untried:

| perturbation | F (5) | A (29) | C (12) |
|---|---|---|---|
| group A as `*((s16 *)(base + 0x10) + t0)` x5 | 31 / 175 | **29 / 175** (byte-identical to base) | 34 / 175 |
| group C as `((s16 (*)[2])(base + 0x40))[t0]` | 18 / 176 | 15 / 176 | 39 / 176 |
| both | 35 / 176 | 33 / 176 | 37 / 176 |

Array-typing group A is *byte-neutral* under the flip and *worse* without it;
array-typing group C always costs an instruction (the `[2]` element type makes the
0x40 base a separate pointer temp). **Neither type creates the t0*4 -> t0*2
edge** — GCC computes each stride directly from `t0` regardless of the element
type, exactly as the target does. The edge is only ever created by writing one
stride as an expression over the other, i.e. by the banned identity.

### 3. The class-C flip is MECHANISM-INDEPENDENT (10 builds) — a new toolchain datum
s6 attributed the flip to `pointer_int_sum` (c-typeck.c:2695) placing the
POINTER-typed side in operand 0. s13 measured seven flip spellings, all reaching
the pointer slot through the `(s32)` cast on the global, and all collapsing onto
29/175. s15 reaches the same slot from the OTHER side — casting the *addend* to a
pointer — and gets the identical build:

| p_6a spelling | F | A | C |
|---|---|---|---|
| `(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A)` | **29 / 175** | 29 / 175 | 12 / 175 |
| `(s16 *)((u8 *)(t0 * 10) + ((s32)D_800A36A0 + 0x6A))` | 16 / 176 | 16 / 176 | 22 / 176 |
| `(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0) + 0x35` | 29 / 175 | 29 / 175 | 12 / 175 |
| flip on p_6a only (u8 * form) | 29 / 175 | 29 / 175 | 12 / 175 |
| `((s16 (*)[5])(D_800A36A0 + 0x6A))[t0]` | 9 / **176** | 9 / 176 | 19 / 176 |
| `((s16 (*)[5])((s32)D_800A36A0 + 0x6A))[t0]` | 9 / **176** | 9 / 176 | 19 / 176 |

Casting the addend to `u8 *` on the UNFLIPPED base produces a build byte-identical
to the `(s32)`-cast flip (29/175). So the flip is a property of *which side lands
in ptrop*, not of the cast that puts it there, and its +24-row loop-head
collateral is not an artefact of integer typing that a different spelling could
dodge. Eleven flip spellings across s6/s13/s15 now collapse onto exactly one
build. The genuinely array-typed row (`(s16 (*)[5])`) is the flip plus one
instruction (9/176) — the same 9/176 s13 found for
`&((s16 *)(D_800A36A0 + 0x6A))[t0 * 5]`, confirming those are one build under two
spellings.

### 4. The do-while(0) prologue fence: CLOSED BY CITATION, no ruling owed
Carried as an open ruling question by s11, s12, s13 and s14. s15 read both
documents end to end and the chain resolves without a Judge cycle:
`.claude/rules/no-new-park-categories.md:256-271` still prints the stale
2026-06-04 scoping, but the SAME paragraph designates the dedicated rule as the
authority on prerequisites ("**The dedicated rule [[do-while-zero-exception]]
enumerates the strict prerequisites**"); and the dedicated rule
`.claude/rules/do-while-zero-exception.md` — the LATER document, owner ruling
2026-07-06 — says in its scope sentence that the construct is "an allowed pure-C
match device for ANY codegen effect incl. register allocation, with mandatory
inline FAKE annotation", and in its body "The former scoping to the reorg.c
label-note mechanism is abolished". Its prerequisite 2 states in terms that
exhaustion "is no longer a hard gate for SINGLE-LEVEL wraps". The candidate uses
ONE level, annotated inline with what + mechanism + lever-exhaustion. **The honest
floor of func_800770B8 is therefore 5, not 9, and the class-A closure is
legitimate.** (The role-prompt's frozen-family table reproduces the same stale
"reorg.c interaction ONLY" scoping; that is a documentation-staleness artefact of
the same 2026-06-04 text, not a second independent restriction.)

### s15 facts for the next session
- [s15] Floor 5 re-verified on today's HEAD chassis (175/175, 0 rules dropped) with the s11/s12/s13 body; candidate.c restored to it after the s14 floor-4 body was Judge-FAILED.
- [s15] The Judge-suggested "cursor genuinely derived from group C's t0*4 pointer" path is MEASURED DEAD in all four spellings on both bases: GCC 2.7.2 materialises the pointer difference (`subu`, plus an `sra` for an `s16 *` difference) instead of folding it back to a shared shift, so every such cursor emits at least one instruction the target lacks (176 or 178 insns) and none scores better than 36.
- [s15] Array/struct typing applied ONLY to groups A and C (the axis s10's whole-body rewrite never isolated) does not create the t0*4 -> t0*2 dependence edge: group A array-typed is byte-identical to its base under the flip (29) and worse without it (31); group C array-typed always costs one instruction (15/18/39 at 176). GCC derives each stride directly from t0 regardless of element type.
- [s15] NEW TOOLCHAIN DATUM: the class-C operand flip is mechanism-independent. Casting the ADDEND to `u8 *` (reaching pointer_int_sum's ptrop slot from the opposite side) produces a build byte-identical to the `(s32)`-cast flip, 29/175, on both the unflipped and flipped bases. Eleven flip spellings across s6/s13/s15 collapse onto one build; the +24-row loop-head collateral is intrinsic to the operand order, not to the cast.
- [s15] `((s16 (*)[5])(D_800A36A0 + 0x6A))[t0]` — the honestly array-typed spelling of the 5-element rows — is the flip plus exactly one instruction (9/176), the same build as s13's `&((s16 *)(D_800A36A0 + 0x6A))[t0 * 5]`.
- [s15] The do-while(0) prologue fence needs NO ruling: no-new-park-categories.md:256-271 designates .claude/rules/do-while-zero-exception.md as the authority on prerequisites, and that rule (owner ruling 2026-07-06, the later document) abolishes the reorg.c-only scoping, sanctions any codegen effect, and drops exhaustion as a hard gate for single-level wraps. Four sessions carried this as an open question; it is closed by citation.
- [s15] With class B foreclosed (three times, two chassis), class C reachable only through a Judge-FAILED construct, and class A legitimately closed, NO sanctioned axis remains open on this function. The honest, defensible floor is 5 and the residual is 2 rows of class B plus 3 rows of class C; the next modality should be `escalation`.

- [s14] Floor 5 re-verified on today's HEAD chassis as the first action of the session: the s11/s12/s13 body measures score 5, build_insns 175 == target_insns 175, rules_dropped 0. candidate.c has been restored to that body (with a fresh s15 header) after the s14 floor-4 body was Judge-FAILED, and re-measured at 5 in place.

- [s14] The 2026-09-01 07:23 Judge FAIL ((t0 * 4) >> 1 is a byte-materializing chain-extender) is now reflected in the ledger: the floor-4 body is banked at rejected/s14-classD-identity-detour-JUDGE-FAILED-2026-09-01.c and the identity is not to be respelled in any form.

- [s14] The Judge's own suggested legal replacement -- group A addressed through a cursor genuinely derived from group C's t0*4 pointer -- is MEASURED DEAD in all four spellings on both bases. GCC 2.7.2 materialises the pointer difference (subu, plus an sra for an s16 * difference) instead of folding it back into the shared shift, so every such cursor emits at least one instruction the target lacks (176 or 178 insns) and none scores better than 36.

- [s14] Array/struct typing applied ONLY to groups A and C does not create the t0*4 -> t0*2 dependence edge: group A array-typed is byte-identical to its base under the flip (29/175) and worse without it (31/175); group C array-typed always costs one instruction (15/18/39 at 176 insns). The edge the banned identity created is not expressible as a type.

- [s14] NEW REUSABLE TOOLCHAIN DATUM: the class-C operand flip is mechanism-independent. Casting the ADDEND to u8 * -- reaching pointer_int_sum's ptrop slot from the opposite side to the (s32) cast -- produces a build byte-identical to the (s32)-cast flip, 29/175, on both the unflipped and the flipped base. The +24-row loop-head collateral is therefore intrinsic to the operand order, not an artefact of integer typing that some other spelling could dodge.

- [s14] ((s16 (*)[5])(D_800A36A0 + 0x6A))[t0], the honestly array-typed spelling of the two 5-element rows, is the flip plus exactly one instruction (9/176) -- the same build as s13's &((s16 *)(D_800A36A0 + 0x6A))[t0 * 5], which s13 had recorded as a separate result.

- [s14] The do-while(0) prologue fence needs NO ruling: no-new-park-categories.md:256-271 designates .claude/rules/do-while-zero-exception.md as the authority on prerequisites, and that rule (owner ruling 2026-07-06, the later document) abolishes the reorg.c-only scoping, sanctions any codegen effect, and drops exhaustion as a hard gate for single-level wraps. The candidate uses one annotated level. Four sessions carried this as an open question; it is closed by citation and the honest floor is 5, not 9.

- [s14] Endgame gate evidence is in hand and NEGATIVE on both prongs: canonical func_800770B8 re-run this session returns {verdict: C, asm_insns: 0, total: 175, distance: 5}, and s11's tools/scan_hand_coded.py --single func_800770B8 returned tier=LOW score=0/8 with S1-S8 all unset -- so the canonical-asm grant path is a measured FAIL, and the coercion-family prong has no SOTN-master precedent for an arithmetic-identity dependence edge (verified independently by the 2026-09-01 Judge).

- [s14] State of the residual at floor 5: class B (2 rows, 0x30/0x34 stores through the p_old copy instead of the raw call result) is foreclosed three times over on two chassis -- every spelling reaching the raw pseudo lets flow.c delete the copy and its four dependents, collapsing the function to 170 instructions against the target's 175; class C (3 rows, addu $2,$2,$3 vs addu $v1,$v1,$v0) is reachable only through the flip, whose collateral is repairable only by the Judge-FAILED identity; class A is closed and legitimate.

## [s16] synthesis — 2026-09-01 — FLOOR 5 (re-verified); the ledger's LAST named re-opening path for class B is killed by an exhaustive corpus census, and two brand-new axes (procedural factoring, declaration order) are swept dead

Chassis re-measured as the first action of the session, `candidate.c` applied to
`src/text1b.c` via `tmp/grind/func_800770B8/s3/try.py`: **score 5, build_insns 175,
target_insns 175** — identical to the s15 chassis, so every s3-s15 spelling
conclusion remains chassis-valid and none of them was re-spent. Restored and
re-verified at 5 / 175 at session end.

### 1. The class-B "2+2 split" is UNIQUE in the entire 1,435-function corpus — frontier item 2's only proposed re-opening path does not exist

The s15 frontier reserved exactly one way to re-open class B: *"find a sibling
function that performs the same allocate-then-initialise sequence and whose target
asm shows the same copy retained, which would NAME the source shape instead of
guessing it."* This session ran that census mechanically instead of by hand.

`tmp/grind/func_800770B8/s16/census.py` (log `census_callee_save_split.log`) and its
broadened twin `census_broad.py` (log `census_broad.log`) scan **all 1,435
`asm/funcs/*.s` files** for the class-B shape: a `addu $X, $v0, $zero` /
`move $X, $v0` copy of a call return value (a `jal` within the preceding 4 rows),
followed within 18 rows by **stores through BOTH the copy register and the raw
`$v0`**. Narrow form (callee-saved destination only): **1 hit — func_800770B8
itself.** Broadened form (any destination register): **2 hits — func_800770B8 and a
false positive in func_80068F70**, where the `addu $a0,$v0,$zero` sits *before* the
`jal` and copies a *different* call's result (func_80068F70 then stores through the
raw `$v0` only: `sw $s0,0x24($v0)` / `sw $v0,D_800A3500`, i.e. the ordinary
collapsed shape our body already produces).

Independently, the direct-caller census of `func_8006E49C` — the allocator whose
result class B is about — lists five call sites (func_80068F70, func_8006E534,
func_800784E4, func_80078824, func_800770B8). **All four siblings use the raw `$v0`
for every post-call store and carry no retained copy at all.** Two of them
(func_800784E4, func_80078824) store only to gp-rel globals; func_8006E534 stores
the raw result to D_800A356C and then works exclusively through gp-rel globals;
func_80068F70 stores through the raw `$v0` and then increments it.

So there is no sibling to learn the source shape from, matched or unmatched: the
target's 2+2 split is a one-of-one shape in this executable. **Frontier item 2 is
KILLED** — not "unattempted", but measured non-existent across the whole corpus.

### 2. Procedural factoring (`static inline` helpers) is byte-transparent — a genuinely new axis, dead

Nothing in s1-s15 ever varied the FUNCTION DECOMPOSITION of the body; every probe
kept one flat function. `static inline` is ordinary, in-project C (`src/main.c:2179`
already ships `static inline void _memcpy`), and GCC 2.7.2 inlines it at `-O2`, so
it is a legitimate structural degree of freedom — and one with real theoretical
reach for class B, since `integrate.c` copies the callee's RTL with its own pseudos
and block notes, which is exactly the kind of thing that could give the copy and the
stores different cse quantities.

Eight builds (`tmp/grind/func_800770B8/s16/gen_inline.py`, bodies in `s16/i/`, log
`s16/sweep_inline.log`), all on the floor-5 body, factoring the post-call header
initialisation four different ways x two argument routings:

| id | shape | score / insns |
|----|-------|---------------|
| I1 | `hdr_clear(u8 *p)` does 0x30/0x34, called with `p_old` | **5 / 175** |
| I2 | I1 called with `D_800A36A0` instead | **5 / 175** |
| I3 | `hdr_link(s32 *p, s32 prev)` does the global + 0x4 stores | **5 / 175** |
| I4 | `hdr_init(...)` does all four stores | **5 / 175** |
| I5 | `hdr_new(...)` takes the call result, returns it; 0x30/0x34 outside | 23 / 170 |
| I6 | `hdr_all(...)` takes the call result, does all four, returns it | 23 / 170 |
| I7 | second local `q`, `hdr_clear2(q)` BEFORE `p_old = q` | 25 / 170 |
| I8 | second local `q`, `hdr_clear2(q)` AFTER `p_old = q` | 23 / 170 |

Two clean readings. (a) I1-I4 are **byte-identical to the un-factored floor-5 body**:
GCC 2.7.2's inliner leaves no cse-visible boundary and creates no surviving extra
pseudo — the inlined statements land in the caller's extended basic block exactly as
if written inline, so factoring cannot buy class B. (b) I5-I8 reproduce the *same*
170-instruction collapse as s7's/s8's two-local forms (H1 23/170, V1 25/170), to the
identical scores: passing the call result through an inline parameter is, at RTL,
the same pseudo-to-pseudo copy `make_regs_eqv` collapses. The inline-parameter route
is therefore not a new mechanism, it is the banked one with new syntax.

### 3. Local DECLARATION ORDER is completely inert — 120/120 builds byte-identical

Also never swept in fifteen sessions, and a plausible lever on paper: s8's forecloser
for class B rests on `cse.c: make_regs_eqv`'s canonicality test, which compares
`uid_cuid[regno_first_uid]` / `regno_last_uid` and settles ties by `qty_first_reg`
(pseudo NUMBER), and `local-alloc.c` orders allocnos partly by pseudo number too. If
pseudo numbering followed declaration order, permuting the declarations would be a
free, ordinary-C way to move both the class-B canonical choice and the class-C seat.

`tmp/grind/func_800770B8/s16/gen_decl.py` emits **all 120 permutations** of the five
top-level locals (`u16 sp[2]; s32 *p_old; s32 r; s16 t0; s16 a2;`) into `s16/d/`,
each measured with `sandbox --disable all` (log `s16/sweep_decl.log`, manifest
`s16/d/manifest.tsv`). **Result: every one of the 120 builds scores 5 / 175 — a
single distinct build.** GCC 2.7.2 numbers pseudos at first RTL EMISSION (the order
`expand_expr` first materialises each `DECL_RTL`), not at declaration, so for a body
whose locals are all used in a fixed statement order the declaration list is
semantically and byte-wise inert. This retires the "named-intermediate declaration
order" family as a lever for THIS function without needing to argue about its
sanction status, and it also retires the tie-break half of the s8 canonicality
reading: the pseudo numbers cannot be moved from C here.

### Bank / artifacts
- `memory/grind/func_800770B8/rejected/s16-inline-helper-returns-ptr-collapses-170insn.c` (I6)
- `memory/grind/func_800770B8/rejected/s16-inline-helper-byte-neutral-175insn-score5.c` (I4)
- `memory/grind/func_800770B8/rejected/s16-decl-order-permutation-byte-neutral-all-120.c` (D001, representative of all 120)
- rejected bank now holds 79 forms.

- [s16] Chassis re-measured with candidate.c applied: score 5, build_insns 175, target_insns 175 — at session start AND at session end. Unchanged from s15.
- [s16] EXHAUSTIVE CORPUS CENSUS (1,435 asm/funcs/*.s scanned, tmp/grind/func_800770B8/s16/census.py + census_broad.py): the class-B "2+2 split" — a call-result copy whose copy register AND raw $v0 are BOTH used as store bases afterwards — occurs EXACTLY ONCE in the whole executable, in func_800770B8 itself. The only other hit, func_80068F70, is a false positive (the copy precedes the jal and belongs to a different call). The s15 frontier's sole reserved re-opening path for class B ("find a sibling whose target asm retains the copy") is therefore measured NON-EXISTENT, not merely untried.
- [s16] All four sibling call sites of func_8006E49C (func_80068F70, func_8006E534, func_800784E4, func_80078824) store through the RAW $v0 result and retain no copy — i.e. they emit exactly the collapsed shape our body produces. No sibling can name the target's source shape.
- [s16] NEW AXIS SWEPT DEAD — procedural factoring: 8 builds with `static inline` helpers for the post-call header init (tmp/grind/func_800770B8/s16/i/, sweep_inline.log). Helpers taking the ALREADY-ASSIGNED pointer (I1-I4) are BYTE-IDENTICAL to the un-factored body (5/175): GCC 2.7.2's integrate.c leaves no cse-visible block boundary and no surviving extra pseudo. Helpers taking the CALL RESULT as a parameter (I5-I8) collapse to 170 insns at scores 23/23/25/23 — the identical collapse s7/s8 measured for two-local forms, because an inline parameter is the same pseudo-to-pseudo copy make_regs_eqv eliminates.
- [s16] NEW AXIS SWEPT DEAD — local declaration order: all 120 permutations of the five top-level locals measure 5 / 175, one single distinct build (tmp/grind/func_800770B8/s16/d/, sweep_decl.log). GCC 2.7.2 numbers pseudos at first RTL emission, not at declaration, so declaration order is byte-inert for this body; the pseudo-number tie-break in cse.c make_regs_eqv / local-alloc allocno ordering is not C-controllable here.

- [s15] Chassis re-measured with candidate.c applied at session start AND at session end: score 5, build_insns 175, target_insns 175 - unchanged from s15, so every s3-s15 spelling conclusion stays chassis-valid and none was re-spent.

- [s15] EXHAUSTIVE CORPUS CENSUS (1,435 asm/funcs/*.s): the class-B '2+2 split' - a call-result copy whose copy register AND raw $v0 are both used as store bases afterwards - occurs EXACTLY ONCE in the whole executable, in func_800770B8 itself. The only other hit (func_80068F70) is a false positive: its copy precedes the jal and belongs to a different call, and it then stores through the raw $v0 only.

- [s15] All four sibling call sites of func_8006E49C (func_80068F70, func_8006E534, func_800784E4, func_80078824) store through the RAW $v0 result and retain no copy, i.e. they emit exactly the collapsed shape our body produces. No sibling in the game can name the target's source shape - the s15 frontier's sole reserved re-opening path for class B is measured non-existent, not merely untried.

- [s15] NEW AXIS SWEPT DEAD - procedural factoring: static inline helpers taking the already-assigned pointer are byte-identical to the unfactored body (I1-I4, 5/175); helpers taking the call result as a parameter or returning it collapse to 170 insns (I5 23, I6 23, I7 25, I8 23), the identical collapse s7/s8 banked for two-local forms. GCC 2.7.2's integrate.c leaves no cse-visible block boundary and no surviving extra pseudo, so factoring is new syntax for a banked mechanism.

- [s15] NEW AXIS SWEPT DEAD - local declaration order: all 120 permutations of the five top-level locals measure 5/175, one single distinct build. This retires the declaration-order lever for this function on MEASUREMENT rather than on sanction grounds, and retires the pseudo-number tie-break half of s8's cse.c canonicality reading.

- [s15] Three forms banked to memory/grind/func_800770B8/rejected/ (bank now holds 79): s16-inline-helper-returns-ptr-collapses-170insn.c, s16-inline-helper-byte-neutral-175insn-score5.c, s16-decl-order-permutation-byte-neutral-all-120.c.

- [s15] src/text1b.c was restored to its committed INCLUDE_ASM state at session end (asm-until-matched); the floor-5 body lives in memory/grind/func_800770B8/candidate.c with its s16 negative results recorded in the header.

## [s17] synthesis — 2026-09-01 — FLOOR 5 (re-verified); CLASS B's 2+2 SPLIT IS PRODUCED IN C FOR THE FIRST TIME IN SEVENTEEN SESSIONS, and its price is proven to be at least one instruction the function does not have

Chassis re-measured at session start with `candidate.c` applied:
**score 5, build_insns 175, target_insns 175** — unchanged. Restored and
re-verified at 5 / 175 / 175 at session end; `src/text1b.c` returned to HEAD.

### The source-level enumeration that made this session possible
`tools/gcc-2.7.2/cse.c:8038-8063` (`cse_end_of_basic_block`) — the scan loop is
`while (p && GET_CODE (p) != CODE_LABEL)` with exactly two early `break`s inside:

* `if (! after_loop && GET_CODE (p) == NOTE && NOTE_LINE_NUMBER (p) == NOTE_INSN_LOOP_END) break;`
* `if (GET_CODE (p) == NOTE && NOTE_LINE_NUMBER (p) == NOTE_INSN_SETJMP) break;`

cse pass 2 runs with `after_loop = 1`, so under cse2 **an extended basic block is
terminated by exactly two things: a `CODE_LABEL`, or a `NOTE_INSN_SETJMP`.**
NOTE_INSN_SETJMP is emitted only for a real `setjmp` call, which this function does
not and cannot contain. Therefore the ONLY C-reachable cse2 block break is a real
`CODE_LABEL`. This retires nine sessions of "find some fence" guessing: there is no
note, scope, wrap, loop or inline boundary that cse2 honours — s8's NOTE_INSN_LOOP_END
fence worked in cse1 for exactly the reason the `! after_loop` guard states, and cse2
ignores that note by construction.

### Probe 1 — a dead label does not survive to cse2 (8 builds, dump-attributed)
`tmp/grind/func_800770B8/s17/gen.py` -> `s17/b/`, log `s17/sweep.log`. The H1 split
shape (`pn = call(); p_old = pn;` with the 0x30/0x34 stores through `pn`), crossed
with three label spellings (`goto Lb; Lb: ;`, a bare `Lb: ;`, `do { break; } while (0);`)
and with/without the s8 make_regs_eqv canonical promotion (`p_old` reused for the tail
block's `D_800A36A0` re-read):

| id | shape | score / insns |
|----|-------|---------------|
| A1 | split, no label, no promotion (control) | 23 / 170 |
| A2 | split + promotion | 14 / **175** |
| A3 | split + promotion + `goto Lb; Lb: ;` | 14 / **175** (byte-identical to A2) |
| A4 | split + `goto Lb; Lb: ;`, no promotion | 23 / 170 |
| A5 | split + promotion + bare `Lb: ;` | 14 / 175 (byte-identical to A2) |
| A6 | split + promotion + `do { break; } while (0);` | 19 / 175 |
| A7 | split + bare label, no promotion | 23 / 170 |
| A8 | split + `do{break;}while(0)`, no promotion | 25 / 170 |

A3/A5 are byte-identical to A2: a label with no live reference is **free and inert**.
Pass attribution read from `tools/grinder/dump.ps1` dumps of A3
(`tmp/grind/func_800770B8/dumps/`): `text1b.rtl` carries
`(jump_insn 88) (barrier 89) (code_label 90 ... ("Lb"))`; by `text1b.jump` insn 90 is
already `(note 90 86 93 "" NOTE_INSN_DELETED_LABEL)` and the jump is gone — jump.c
pass 1 deletes a jump-to-next-insn and then the now-unreferenced label, converting it
to a NOTE. A NOTE is not a `CODE_LABEL`, so `cse_end_of_basic_block` walks straight
through it in cse1 AND cse2. **A zero-cost surviving label does not exist**; the
forbidden "dead-goto label-pad" family is not merely banned here, it is measurably
inert.

### Probe 2 — the label must sit BETWEEN the copy and the raw uses, and then the split appears (4 builds)
`s17/gen2.py` -> `s17/c/`, `s17/gen3.py` -> `s17/e/`; logs `s17/sweep_branch.log`,
`s17/sweep_join.log`. Rows read with `s17/rows.py` off the sandbox object.

| id | shape | score / insns | rows 29-37 |
|----|-------|---------------|------------|
| B1 | split + promotion + `if (r != 0) { raw stores }` | 23 / 177 | label lands AFTER the stores -> still `$17`-based |
| B2 | B1 without promotion | 19 / 174 | copy gone, all `$2` |
| C1 | split + promotion + duplicated-arm `if (r) { +4 store } else { +4 store }` join label, raw stores AFTER the join | 30 / **177** | **`move $17,$2` / `sw $17,0($28)` / `sw $18,4($17)` ... `sw $0,48($2)` / `sh $0,52($2)` — THE TARGET'S 2+2 SPLIT** |
| C2 | C1 without promotion | 14 / 173 | jump.c cross-jumps the arms, label gone, all `$2` |

**C1 is the first C body in seventeen sessions to emit the class-B split**: the copy
survives, the global and `+4` stores go through the copy register, and the 0x30/0x34
stores go through the raw call-result register — exactly `asm/funcs/func_800770B8.s`
rows 29-38. The recipe is now fully named: (i) two pseudos with one copy, (ii) the s8
make_regs_eqv canonical promotion so the long-lived pseudo wins the pre-label uses,
(iii) a `CODE_LABEL` that survives jump.c, placed between the copy-based uses and the
raw-based uses. B1 shows placement is part of the recipe: a conditional *jump* does not
end a cse block (the scan follows the fall-through), only the join LABEL does, so an
`if` wrapping the raw stores puts the label one statement too late.

### Why this CLOSES class B instead of opening it
Every surviving `CODE_LABEL` costs at least one instruction, and the function has no
spare instruction: ours and the target are both exactly 175.
* an unreferenced label is deleted (A3/A5, dump-proven) -> free but inert;
* a label kept alive by a conditional branch costs the branch (s9's `if (p_old != 0)`
  = 176 / score 16; B1 = 177);
* a label kept alive by a duplicated-arm `if/else` costs the branch plus the duplicate
  (C1 = 177), and if the arms are byte-identical jump.c cross-jumps them and deletes
  the label again (C2 = 173);
* the target's own control flow offers no label to reuse — `asm/funcs/func_800770B8.s`
  rows 24-40 are straight-line from the `jal func_8006E49C` to the loop top.

So class B is not "no mechanism found": it is **a known, demonstrated mechanism whose
minimum price is +1 insn on a function with zero slack**. That is a proof of
foreclosure, and it is strictly stronger than the previous "every reaching spelling
collapses to 170" exhaustion argument.

- [s17] Chassis re-verified: candidate.c = 5 / 175 / 175 at session start and at session end; src/text1b.c restored to HEAD.
- [s17] SOURCE-READ (tools/gcc-2.7.2/cse.c:8038-8063): under cse pass 2 (`after_loop = 1`) an extended basic block is terminated by exactly two things - a CODE_LABEL or a NOTE_INSN_SETJMP. The NOTE_INSN_LOOP_END break is guarded by `! after_loop`. NOTE_INSN_SETJMP requires a real setjmp call. Therefore the ONLY C-reachable cse2 block break is a real CODE_LABEL, and no note / scope / wrap / inline / loop device can ever serve as a class-B fence.
- [s17] DUMP-PROVEN (tmp/grind/func_800770B8/dumps, body A3): `goto Lb; Lb: ;` emits (jump_insn)(barrier)(code_label "Lb") in text1b.rtl, and text1b.jump already shows it as NOTE_INSN_DELETED_LABEL - jump.c pass 1 deletes the jump-to-next-insn and then the unreferenced label. A3/A5 measure byte-identical to A2 (14/175). A free-but-surviving label does not exist in GCC 2.7.2.
- [s17] FIRST EVER PRODUCTION OF THE CLASS-B 2+2 SPLIT IN C (body C1, rejected/s17-classB-split-DEMONSTRATED-join-label-177insn-score30.c): two pseudos + one copy, the s8 canonical promotion, and a duplicated-arm if/else join label placed between the copy-based stores and the raw-based stores emits `move $17,$2 / sw $17,0($28) / sw $18,4($17) ... sw $0,48($2) / sh $0,52($2)` - the target's rows 29-38 shape. Cost 177 insns / score 30.
- [s17] Placement is part of the recipe: a conditional JUMP does not end a cse block (the scan follows the fall-through); only the join LABEL does. B1 (`if (r != 0) { raw stores }`) puts the label after the stores and stays $17-based at 177 insns / score 23.
- [s17] The price of a surviving label is >= 1 insn under every spelling measured across s9/s17 (branch 176, branch+promotion 177, duplicated-arm join 177, cross-jumped arms 173 with the label deleted), and func_800770B8 has zero insn slack (175 = 175). Class B is FORECLOSED BY PRICE, with the mechanism demonstrated rather than merely unfound.
- [s17] Four forms banked: s17-classB-split-DEMONSTRATED-join-label-177insn-score30.c (C1), s17-classB-branch-after-copy-label-too-late-177insn.c (B1), s17-classB-dead-goto-label-deleted-by-jump-byte-inert.c (A3), s17-classB-promotion-only-collapses-onto-s1-175insn.c (A2). The rejected bank now holds 83 forms.

- [s16] Chassis re-verified this session: memory/grind/func_800770B8/candidate.c measures score 5 / build_insns 175 / target_insns 175 at session start and again at session end; src/text1b.c restored to HEAD (INCLUDE_ASM) before finishing.

- [s16] SOURCE-READ (tools/gcc-2.7.2/cse.c:8038-8063): under cse pass 2 (after_loop = 1) an extended basic block is terminated by exactly two things - a CODE_LABEL or a NOTE_INSN_SETJMP. The NOTE_INSN_LOOP_END break is guarded by `! after_loop`. Therefore no note, lexical scope, do-while(0) wrap, inline-function boundary or loop note can ever serve as a class-B fence; the only C-reachable cse2 block break is a real CODE_LABEL.

- [s16] DUMP-PROVEN (tmp/grind/func_800770B8/dumps, body A3): `goto Lb; Lb: ;` emits (jump_insn)(barrier)(code_label "Lb") in text1b.rtl, and text1b.jump already carries it as NOTE_INSN_DELETED_LABEL - jump.c pass 1 removes the jump-to-next-insn and demotes the unreferenced label. A3 and A5 measure byte-identical to the unlabelled A2 (14/175). A free-but-surviving CODE_LABEL does not exist in GCC 2.7.2.

- [s16] FIRST EVER PRODUCTION OF THE CLASS-B 2+2 SPLIT IN C (body C1, banked as rejected/s17-classB-split-DEMONSTRATED-join-label-177insn-score30.c): two pseudos with one copy + the s8 make_regs_eqv canonical promotion + a duplicated-arm if/else join label placed between the copy-based stores and the raw-based stores emits the target's rows 29-38 shape at 177 insns / score 30.

- [s16] Placement is part of the recipe: a conditional JUMP does not end a cse block (the scan follows the fall-through); only the join LABEL does. B1 (`if (r != 0) { raw stores }` + promotion) puts the label one statement too late and stays $17-based at 177 insns / score 23.

- [s16] The price of a surviving label is >= 1 insn under every spelling measured across s9 and s17 (real branch 176/16; branch+promotion 177/23; duplicated-arm join 177/30; cross-jumped identical arms 173/14 with the label deleted), and func_800770B8 has zero insn slack (build 175 = target 175). Class B is foreclosed BY PRICE, with the mechanism demonstrated rather than merely unfound.

- [s16] Re-measured on today's floor-5 chassis: the s8 canonical-promotion result reproduces exactly (A2 = 175 insns / score 14, rows 35-36 still `sw $0,48($17)` / `sh $0,52($17)`), so the promotion alone flips the canonical without splitting - as s8 concluded on the floor-9 chassis.

- [s16] Four forms banked to memory/grind/func_800770B8/rejected/ (bank now holds 83): s17-classB-split-DEMONSTRATED-join-label-177insn-score30.c, s17-classB-branch-after-copy-label-too-late-177insn.c, s17-classB-dead-goto-label-deleted-by-jump-byte-inert.c, s17-classB-promotion-only-collapses-onto-s1-175insn.c.

## [s18] solver — 2026-09-01 — THE WHOLE SOLVER SUITE IS NOW SPENT ON THIS FUNCTION WITH FULL-DISPOSITION GOALS; SCHED IS FORECLOSED IN BOTH PASSES AND GLOBAL-ALLOC AT DEPTH 3

(This session is the driver's "session 17"; the previous session labelled its own
artifacts `s17`, so this one banks under `s18` to avoid clobbering
`tmp/grind/func_800770B8/s17/`. Nothing in the s17 record is contradicted here.)

Chassis re-measured as the first action: `memory/grind/func_800770B8/candidate.c`
applied to `src/text1b.c` = **score 5 / build_insns 175 / target_insns 175**, and
re-measured again at session end after every probe = 5/175. No drift; `src/text1b.c`
was returned to HEAD (`INCLUDE_ASM`) before the outcome was written.

### 1. Object-level typing of the floor-5 residual (the mandated first step)

`inverse_compose.py classify` still refuses this function (rules-to-zero guard) and
names the object-level classifier, exactly as s5/s7 banked. `goal_from_tgt.py classify
text1b func_800770B8` on the floor-5 body:

    FIRST DIVERGENCE: RA      $v0 -> $v1 x4 | $s1 -> $v0 x2 | $v1 -> $v0 x1
    5 renamed pair(s), 0 pair(s) skipped (skeleton differs)

**Zero skipped pairs is the load-bearing number and it is new to the ledger.** It
means our 175-instruction stream and the target's 175-instruction stream have the
same skeleton at every position: the entire honest residual is five in-place
register-name replacements. There is no insn our build lacks, none it adds, and none
in the wrong slot.

### 2. FULL-DISPOSITION global-alloc inverse — NEGATIVE at depth 3 (supersedes s5)

The solver playbook's rule (3) says a subset goal voids the vectors. Every previous
ra_solver run on this function used a subset: s5 ran `--goal '{"75": 2}'` (class B
alone, depth 2) and s10 ran `local --swap 3,4` on the FLIPPED model. This session ran
the goal that states the whole target disposition.

`goal_from_tgt.py goal ... --model tmp/ra_solver_work/func_800770B8.model.json`
attributes `$s1->$v0 (x2)` uniquely to **pseudo 75** (p_old, class B) and reports the
other two substitutions AMBIGUOUS at register level (38 pseudos hold $v0, 13 hold
$v1). The class-C member is identified from the s6 dump read-out already in this
ledger: `.lreg` insn 173 = `(set (reg 110) (plus (reg 109) (reg 108)))`, so the
class-C goal is **pseudo 110 -> $v1 (3)**.

    inverse.py global <model> --goal '{"75": 2, "110": 3}' --depth 2  -> NEGATIVE
    inverse.py global <model> --goal '{"75": 2, "110": 3}' --depth 3  -> NEGATIVE
    inverse.py global <model> --goal '{"110": 3}'          --depth 3  -> NEGATIVE
    (search bounds: refs delta +12/-6, live length +/-2,4,8)

Same mechanism as s5 for the class-B half (pseudo 75 crosses 3 calls and $v0 is
call-used, so `prune_preferences`, global.c:897, strips the $v0 preference before
`find_reg` runs; pseudo 74 is stripped identically). The class-C half is now
independently NEGATIVE — no perturbation of refs / live span / birth order /
conflicts / preferences / calls-crossed reaches pseudo 110 -> $v1.
Artifact: `tmp/grind/func_800770B8/s18/inv_global_full.log`.

### 3. BOTH SCHEDULING PASSES ARE FORECLOSED — and pass 1 had never been checked

s9's exhaustive 3234-atom sweep was a **sched2** result aimed at class A. s5's
"no differing block" was **pass 2 only** and carried an explicit alignment caveat.
This session ran the object-level goal path on both passes of the floor-5 body,
using the s5-patched `goalmap` (the `_macro_expand_counts` bug still bites: the
stock tool aborts with "honest object has 175 insns but text1b.hon.s body has 169
lines"):

    perturb.py <sched.json> --func func_800770B8 --pass {1,2} \
      --goal-from-target text1b --target-object build/src/text1b.o \
      --ours-object tmp/sandbox/func_800770B8/text1b.o --atoms luid,luid_move --depth 2

    align honobj->tgtobj: |A|=175 |B|=175
        {'equal': 170, 'replace': 5, 'delete': 0, 'insert': 0, 'moved': 0}
    PASS 1: no differing block.   PASS 2: no differing block.

**`moved: 0` with a real target object.** The floor-5 build's instruction order is
target-exact in sched1 and in sched2. Scheduling is not a lever for any part of the
remaining 5 points, in either pass — this is the first time that has been stated for
sched1, and it retires "emission order" as a description of the floor-5 residual.
Artifact: `tmp/grind/func_800770B8/s18/perturb_base.log`.

### 4. TOOLING TRAP (transferable, costs a whole probe if unknown)

`tools/sched_solver/mkasm.sh` **ignores a `--target` argument** and unconditionally
does `cp <stem>.hon.s <stem>.tgt.s` (per the 2026-08-25 owner ruling baked into its
comments: for INCLUDE_ASM-routed functions the src-derived stream cannot carry the
target). So the solver-playbook instruction "pin the target with `--target
<stem>.tgt.head.s`" is **actively dangerous on this function**: `.tgt.head.s` copied
from `.tgt.s` is OUR OWN stream, and `goalmap.py --target` then reports
`align hon->tgt {equal: 169}` and "block N: GOAL == OURS (identity)" for every
block — a fictitious clean bill of health. This session hit it and caught it only
by `cmp`-ing the two files. The correct path for this function is
`--target-object build/src/<stem>.o --ours-object tmp/sandbox/<func>/<stem>.o`, and
`build/src/text1b.o` must be a HEAD (INCLUDE_ASM) build for that to be the target.

### 5. Local-alloc inverse on the BASE model: REACHABLE, one family, MEASURED DEAD in C

s10 ran `inverse.py local --swap 3,4` on the FLIPPED model only. On the BASE
(floor-5) model the block-1 quantities re-extract byte-identically to s10's BASE
table (chassis stable):

    ord0 qty2 r89  [12,14) refs4  -> $v0
    ord1 qty4 r110 [48,56) refs10 -> $v0     <- the sum / addiu chain; target wants $v1
    ord2 qty3 r108 [28,52) refs16 -> $v1     <- the `sll ...,2` (t0*4) chain
    ord3 qty1 r100 [10,44) refs12 -> $a0
    ord4 qty0 r86  [6,46)  refs14 -> $a1

`inverse.py local ... --block 1 --swap 3,4 --depth 2` (392-atom space, 7 classes)
returns **REACHABLE: 55 distinct minimal vectors, all one atom, all ONE family** —
`live_shrink qty 3: born later (28 -> 35..46)`. In words: if the t0*4 shift chain
were born later in the block, qty3 would lose $v1 to qty4 and the seats would swap
into the target's `addu $v1,$v1,$v0`. The tool prints its own standing caveat
(camera_set_zoom 2026-08-05): a birth/span vector is a claim about ALLOC-TIME order,
which is not known to equal emission order — NECESSARY, not SUFFICIENT.

**Spelled and measured — five builds, all worse** (`s18/sweep_v.log`, generator
`s18/gen.py`, bodies in `s18/v/`). The family says "delay the birth of the t0*4
quantity"; s12 (24/24 store-group orders) and s13 (120 five-element orders) already
relocated it by statement movement, so this session attacked it in place, by
changing which subexpression the two t0*4 consumers demand:

| variant | spelling | score | insns |
|---|---|---|---|
| V1 | group B via s16 index `*((s16 *)&D_800A35D0 + t0*2 + 1)` | 37 | 177 |
| V2 | group C via s16 index `*((s16 *)(base + 0x40) + t0*2 + 1)` | **6** | 175 |
| V3 | V1 + V2 | 15 | 177 |
| V4 | group B array-typed `((s16 (*)[2])&D_800A35D0)[t0][1]` | 37 | 177 |
| V5 | group B + group C array-typed | 15 | 177 |

V2 is the only one that stays at 175 insns, and its positional diff shows **class C
is completely unchanged** (rows 62-64 still `addu $2,$2,$3`); the extra point is a
NEW break at row 54 (`addu $2,$3,$4` vs target `addu $v0, $a0, $v1`). So the s16-index
spelling does not move qty3's birth in the direction the model needs — it only
re-orders a different plus. Banked:
`rejected/s18-qty3-birth-delay-groupC-s16-index-176row54-score6.c`,
`rejected/s18-qty3-birth-delay-groupB-s16-index-177insn-score37.c`,
`rejected/s18-qty3-birth-delay-both-array-typed-177insn-score15.c`.

The local family is also contradicted by the target's own emission, exactly as s10's
three families were: our build emits the `sll ...,2` at the target's own row 42 and
section 3 proves `moved: 0` in both passes, so "born at >= 35 instead of 28"
describes a function that emits its loop head in a different order from the target.

### 6. The FLIPPED basin, typed for the first time

Applying `s12/perm/Q00.c` (flipped-ABCD, 29/175) and classifying it object-level:

    9 renamed pair(s), 11 pair(s) skipped (skeleton differs)
    $a0->$a1 x4 | $a1->$v0 x3 | $v1->$a0 x3 | $s1->$v0 x2 | $v0->$v1 x2 | $a1->$v1 x1
    align honobj->tgtobj: {'equal': 147, 'replace': 15, 'delete': 8, 'insert': 8, 'moved': 5}

This SHARPENS s13's "the flip's damage is emission order in rows 38-59": only **5**
of the 28 damaged slots are moves; 15 are in-place replacements and 8+8 are genuine
insert/delete pairs. The flip does not merely re-order the loop head, it changes
which instructions the loop head contains. `perturb.py` cannot search it: block 1
(27 insns) reports `goal differs from ours` but is **SKIPPED — goal is not a
topological order (19 violations pass 1, 22 pass 2): the target alignment mis-paired
duplicate instruction text**. That is a tooling limit, not a verdict; recorded so no
future session re-runs it expecting an answer.
Artifacts: `s18/perturb_flip_p1.log`, `s18/perturb_flip_p2.log`,
`s18/goalmap_flip.log` (the FICTITIOUS identity run of section 4 — kept as the
evidence of the trap, NOT as a result).

- [s18] Chassis: candidate.c = score 5 / 175 build insns / 175 target insns at session start and again at session end. src/text1b.c returned to HEAD INCLUDE_ASM before the outcome was written.
- [s18] goal_from_tgt.py classify on the floor-5 body reports 5 renamed pairs and ZERO skipped pairs: our stream and the target's stream share the same skeleton at all 175 positions, so the entire honest residual is five in-place register-name replacements.
- [s18] inverse.py global with the FULL disposition goal {"75": 2, "110": 3} returns NEGATIVE at depth 2 AND depth 3 (bounds: refs +12/-6, live length +/-2,4,8). The class-C-only goal {"110": 3} is independently NEGATIVE at depth 3. This supersedes s5's subset goal {"75": 2} under solver rule (3); global allocation is foreclosed for BOTH residual classes, not just class B.
- [s18] SCHED IS FORECLOSED IN BOTH PASSES on the floor-5 body. perturb.py with the object-level goal (--target-object build/src/text1b.o --ours-object tmp/sandbox/func_800770B8/text1b.o) reports NO differing block for pass 1 and pass 2, with align honobj->tgtobj = {equal 170, replace 5, delete 0, insert 0, moved 0}. s9's sweep was sched2/class-A only; sched1 had never been checked on this function.
- [s18] TOOLING TRAP: tools/sched_solver/mkasm.sh ignores --target and always copies hon.s to tgt.s, so goalmap/perturb "--target tmp/sched_map/<stem>.tgt.head.s" (the solver playbook's own instruction) compares this function against ITSELF and prints "GOAL == OURS (identity)" for every block. Verify with cmp before trusting any sched goal on an INCLUDE_ASM-routed function; use --target-object/--ours-object instead.
- [s18] inverse.py local on the BASE model (--block 1 --swap 3,4 --depth 2, 392 atoms) is REACHABLE with 55 minimal single-atom vectors in exactly ONE family: live_shrink qty3 (r108, the t0*4 shift chain) born later (28 -> 35..46). Block-1 QTYDBG rows re-extract byte-identically to s10's BASE table.
- [s18] The live_shrink-qty3 family is MEASURED DEAD in C in place (5 builds): group B via s16 index 37/177, group C via s16 index 6/175, both 15/177, group B array-typed 37/177, both array-typed 15/177. The only 175-insn variant (group C via s16 index) leaves class C rows 62-64 completely unchanged and breaks row 54 instead. Statement-level relocation of the same quantity was already exhausted by s12 (24/24 group orders) and s13 (120 five-element orders).
- [s18] The FLIPPED-ABCD basin (29/175) typed object-level for the first time: 9 renamed pairs + 11 skeleton-differing pairs; align = {equal 147, replace 15, delete 8, insert 8, moved 5}. Only 5 of the 28 damaged slots are MOVES - the flip changes which instructions the loop head contains, it does not merely reorder them. This sharpens s13's "emission order" framing.
- [s18] perturb.py cannot search the flipped basin: block 1's goal is not a topological order (19 violations pass 1, 22 pass 2) because the target alignment mis-pairs duplicate instruction text. A tooling limit, not a verdict - do not re-run expecting vectors.

- [s17] Chassis re-measured at session start and again at session end: memory/grind/func_800770B8/candidate.c applied to src/text1b.c = score 5 / build_insns 175 / target_insns 175. No drift from the s11-s17 ledger floor. src/text1b.c was returned to HEAD (INCLUDE_ASM) before the outcome was written; the only tree changes are memory/grind/func_800770B8/* and tmp/.

- [s17] goal_from_tgt.py classify on the floor-5 body reports 5 renamed pairs and ZERO skipped (skeleton-differing) pairs - our 175-insn stream and the target's 175-insn stream share the same skeleton at every position, so the entire honest residual is five in-place register-name replacements. Substitutions: $v0->$v1 x4, $s1->$v0 x2, $v1->$v0 x1.

- [s17] inverse.py global with the FULL disposition goal {"75": 2, "110": 3} is NEGATIVE at depth 2 AND depth 3 (search bounds refs +12/-6, live length +/-2,4,8); the class-C-only goal {"110": 3} is independently NEGATIVE at depth 3. This supersedes s5's subset goal {"75": 2} under the solver playbook's rule (3) that a subset goal voids the vectors.

- [s17] SCHED IS FORECLOSED IN BOTH PASSES: perturb.py with the object-level goal reports no differing block for pass 1 and for pass 2, align honobj->tgtobj = {equal 170, replace 5, delete 0, insert 0, moved 0}. sched1 had never been checked on this function; s9's 3234-atom sweep was sched2/class-A only and s5's pass-2 run carried an explicit alignment caveat.

- [s17] TOOLING TRAP (transferable): tools/sched_solver/mkasm.sh ignores a --target argument and unconditionally copies <stem>.hon.s to <stem>.tgt.s, so following the solver playbook's 'pin the target with --target <stem>.tgt.head.s' on an INCLUDE_ASM-routed function compares the function against ITSELF and prints 'block N: GOAL == OURS (identity)' for every block - a fictitious clean bill of health. Caught here only by cmp-ing the two files. Correct path: --target-object build/src/<stem>.o --ours-object tmp/sandbox/<func>/<stem>.o, with build/src/<stem>.o from a HEAD build.

- [s17] The stock tools/sched_solver/goalmap.py still aborts on this function with '_macro_expand_counts' undercounting (honest object 175 insns vs hon.s 169 lines); the s5-patched scratch copy tmp/grind/func_800770B8/s5/sched_solver/perturb.py works and was used for every sched run this session.

- [s17] inverse.py local on the BASE model (--block 1 --swap 3,4 --depth 2, 392 atoms, 7 classes) is REACHABLE with 55 minimal single-atom vectors in exactly ONE family: live_shrink qty3 (r108, the t0*4 shift chain) born later (28 -> 35..46). Block-1 QTYDBG rows re-extract byte-identically to s10's BASE table (ord0 qty2 r89 [12,14) refs4 -> $v0; ord1 qty4 r110 [48,56) refs10 -> $v0; ord2 qty3 r108 [28,52) refs16 -> $v1; ord3 qty1 r100 [10,44) refs12 -> $a0; ord4 qty0 r86 [6,46) refs14 -> $a1).

- [s17] The live_shrink-qty3 family is measured dead in C in place: group B via s16 index 37/177, group C via s16 index 6/175, both 15/177, group B array-typed 37/177, both array-typed 15/177. Banked as rejected/s18-qty3-birth-delay-groupC-s16-index-176row54-score6.c, rejected/s18-qty3-birth-delay-groupB-s16-index-177insn-score37.c, rejected/s18-qty3-birth-delay-both-array-typed-177insn-score15.c.

- [s17] The FLIPPED-ABCD basin (s12/perm/Q00.c, re-measured 29/175) typed object-level for the first time: 9 renamed pairs + 11 skeleton-differing pairs; align = {equal 147, replace 15, delete 8, insert 8, moved 5}. Only 5 of the 28 damaged slots are moves.

- [s17] Net effect on the ledger: all three solver backends (global-alloc, local-alloc, both scheduler passes) have now been run against this function's residual with full-disposition goals, and every one returns either a NEGATIVE verdict or a single family that measurement kills. No new floor; no construct proposed; nothing in the s11-s17 record contradicted.

- [s18-forensics] Chassis re-measured at session start: memory/grind/func_800770B8/candidate.c applied to src/text1b.c = score 5 / build_insns 175 / target_insns 175. No drift. src/text1b.c returned to HEAD (INCLUDE_ASM) before the outcome was written. (Ledger numbering note: the previous session labelled itself s17 in hypotheses.md and s18 in evidence.md; this session's entries are tagged s18-forensics and its scratch lives in tmp/grind/func_800770B8/s18/fx/.)

- [s18-forensics] CLASS C IS NOW ATTRIBUTED TO A NAMED PASS AND A NAMED DECISION, replacing 17 sessions of "fold operand order" / "seat priority" framing. The divergent insn is RTL insn 185, `(set (reg:SI 110) (plus:SI (reg:SI 109) (reg:SI 108)))` where 109 = the second D_800A36A0 reload and 108 = the t0*10 shift, both carrying REG_DEAD notes (tmp/grind/func_800770B8/s18/lreg.txt, insn 185). The hard-register identity of 110 is NOT decided by find_free_reg seat priority: it is decided by local-alloc.c block_alloc's operand-tying loop (tools/gcc-2.7.2/local-alloc.c:1240-1298), which walks operands 1..n IN ORDER and calls combine_regs(operand_i, operand_0), breaking at the FIRST success. Operand 1 (reg 109) dies here and has a quantity, so 110 is merged into 109's quantity and inherits its seat -> `;; Register 109 in 2.` / `;; Register 110 in 2.` (lreg.txt:233-234) -> `addu $2,$2,$3`. The target wants 110 merged with operand 2 (the shift) -> `addu $v1,$v1,$v0`.

- [s18-forensics] THE COMBINE_REGS GATE SET IS ENUMERATED AND ALL TEN GATES ARE TYPED FOR C-REACHABILITY (tools/gcc-2.7.2/local-alloc.c:1784-1946). For the tie at operand 1 to FAIL, exactly one of these must hold: (1) reg_qty[ureg] < 0, i.e. the reload is not block-local or dies more than once (local-alloc.c:470-478); (2) no REG_DEAD note for ureg at this insn, i.e. the reload is still live afterwards (local-alloc.c:1917); (3) subreg size/offset mismatch; (4) an already-larger combined object; (5) reg_qty[sreg] != -2; (6) a REG_NO_CONFLICT note; (7) ureg == sreg; (8) two hard regs; (9) !MODES_TIEABLE_P; (10) !reg_meets_class_p(sreg, qty_min_class[qty(ureg)]). Gates 3,4,6,7,8,9 are structurally impossible for two SImode pointer pseudos in a plain addsi3; gate 10 is unreachable on MIPS where every candidate is GR_REGS; gate 5 removes the tie for BOTH operands (110 then goes to global-alloc, not to the shift's seat). Only gates 1 and 2 are C-reachable, and BOTH of them require the reload's value to be live past the add - i.e. an extra use, in another block (gate 1) or later in the same block (gate 2). Since the target reloads D_800A36A0 fresh at every later use (rows 84, 127, 141, 149, 153, 156) and the whole of rows 38-64 is ONE basic block whose only other reload (row 41) is separately consumed, any such extra use either deletes an existing load or adds an instruction. The floor-5 body has ZERO instruction slack (175 == 175).

- [s18-forensics] GATE 1 IS DEMONSTRATED IN C AND PRICED. Hoisting the second reload into a local `b2` that is also used for the post-inner-loop 0x5C/0x60 stores makes the reload cross a block boundary; the dump shows `Register 86 used 6 times across 26 insns` with NO `in block 1` marker (reg_basic_block < 0 -> reg_qty = -1), insn 186 becomes `(set (reg 110) (plus (reg/v 86) (reg 109)))` with a REG_DEAD note on 109 only, and local-alloc prints `;; Register 109 in 2.` / `;; Register 110 in 2.` - the dest is tied to the SHIFT, printing `addu $2,$8,$2`: the target's rows 62-64 tie topology reached for the first time WITHOUT the operand-order flip. It costs the third reload: 174 insns vs the target's 175, score 49. Artifacts tmp/grind/func_800770B8/s18/fx/p1.lreg.txt (insn 186) and rejected/s18fx-classC-blocklocal-reload-defeats-op1-tie-174insn-score49.c.

- [s18-forensics] THE FLOOR-5 BUILD ALREADY HAS THE TARGET'S SEATS FOR CLASS C AND ONLY THE TIE IS WRONG; THE FLIP HAS THE TIE RIGHT AND THE SEATS WRONG. Floor-5: row 60 `lw $2` (target `lw $v0`), row 61 `sll $3` (target `sll $v1`) - both seats correct - and row 62 `addu $2,$2,$3` vs `addu $v1,$v1,$v0`. Re-measuring the plain operand-order flip on today's chassis ((t0 * 10) + (s32)D_800A36A0 + 0x6A) gives 29/175 and prints row 60 `lw $3` / row 61 `sll $2` / row 62 `addu $2,$2,$3`: the tie is now to the shift (target topology) but the reload and the shift have SWAPPED hard registers relative to both the floor-5 build and the target. So the flip is not "class C plus collateral"; it is a different allocation of the same two values plus collateral. Banked rejected/s18fx-classC-flip-fixes-tie-but-swaps-seats-175insn-score29.c.

- [s18-forensics] THE PLAIN FLIP'S LOOP-HEAD COLLATERAL IS A PURE REORDERING (multiset-identical), UNLIKE s17's TYPED BASIN. Rows 38-59 of the plain-flip build and of the target contain exactly the same opcode multiset (3 sll, 1 sra, 1 lw, 5 addu, 9 sh, 1 lui, 1 addiu, 1 sb); the differences are that the t0*4 shift SINKS from row 42 to row 50 and the D_800A36A0 reload RISES from row 41 to row 40. s17 typed the FLIPPED-ABCD basin (s12/perm/Q00.c = flip PLUS a store-group permutation) and found delete 8 / insert 8, then concluded perturb.py cannot search it because the goal is not a topological order. That conclusion is about Q00, NOT about the plain flip: a moves-only alignment is exactly the input perturb.py needs. Running goalmap/perturb on the plain-flip build is an untried, well-posed solver question.

- [s18] Chassis re-verified at session start: candidate.c applied to src/text1b.c = score 5 / build_insns 175 / target_insns 175. src/text1b.c was returned to HEAD (INCLUDE_ASM) before this outcome was written; the only tree changes are memory/grind/func_800770B8/* and tmp/.

- [s18] The whole honest residual is still 5 in-place register names: rows 35-36 (class B, stores through the copy $17 instead of the raw call result $v0) and rows 62-64 (class C, the sum tied to the reload instead of the shift).

- [s18] CLASS C IS NOW ATTRIBUTED TO A NAMED PASS AND A NAMED DECISION: local-alloc.c block_alloc's operand-tying loop (tools/gcc-2.7.2/local-alloc.c:1240-1298) ties the destination of RTL insn 185 to the FIRST operand for which combine_regs succeeds. This supersedes 17 sessions of 'fold operand order' / 'seat priority' framing, and it means s17's local-alloc solver run (qty_compare / find_free_reg seat ordering, live_shrink qty3) was modelling the wrong decision for this residual.

- [s18] All ten combine_regs gates (local-alloc.c:1784-1946) are enumerated and typed for C-reachability in evidence.md. Six are structurally impossible for two SImode pointer pseudos in a plain addsi3; one (reg_meets_class_p) is unreachable on MIPS where every candidate is GR_REGS; one (reg_qty[sreg] != -2) detaches the dest from BOTH operands rather than handing it to the shift. Only two are C-reachable: reg_qty[ureg] < 0 (reload not block-local or dying more than once, local-alloc.c:470-478) and the absence of a REG_DEAD note for the reload at this insn (local-alloc.c:1917).

- [s18] GATE 1 IS DEMONSTRATED IN C AND PRICED: hoisting the outer loop's second D_800A36A0 read into a local that is also used for the post-inner-loop 0x5C/0x60 stores makes the reload's pseudo cross a block boundary; the .lreg dump shows `Register 86 used 6 times across 26 insns` with no `in block 1` marker, insn 186 `(set (reg 110) (plus (reg/v 86) (reg 109)))` with a REG_DEAD note on 109 only, and `;; Register 109 in 2.` / `;; Register 110 in 2.` - the dest tied to the SHIFT, printing `addu $2,$8,$2`. It costs the third reload: 174 insns vs the target's 175, score 49.

- [s18] Both C-reachable gates require the reload to be live past the add. The target reloads D_800A36A0 fresh at every later use (rows 84, 127, 141, 149, 153, 156) and rows 38-64 form a single basic block whose only other reload (row 41) is separately consumed, so any extra live use must delete an existing load or add one. The floor-5 body has zero instruction slack (175 == 175). Class C is therefore foreclosed at local-alloc for the unflipped operand order.

- [s18] THE FLIP IS RIGHT-TIE / WRONG-SEATS, NOT 'CLASS C PLUS COLLATERAL': re-measured plain flip = 29/175 printing `lw $3 / sll $2 / addu $2,$2,$3` against the target's `lw $v0 / sll $v1 / addu $v1,$v1,$v0`. The floor-5 build already has the target's seats and differs only in the tie.

- [s18] The plain flip's loop-head damage is a PURE REORDERING: rows 38-59 are multiset-identical to the target's (3 sll, 1 sra, 1 lw, 5 addu, 9 sh, 1 lui, 1 addiu, 1 sb), with the t0*4 shift sinking 42 -> 50 and the reload rising 41 -> 40. s17's 'perturb.py cannot search the flipped basin' verdict was measured on s12/perm/Q00.c (flip + store-group permutation, delete 8 / insert 8) and does NOT transfer to the plain flip.

- [s18] Nothing in the Judge constraints was approached: no identity detour, no dependence-edge construct, no new construct of any kind was proposed. The two builds banked this session are honest C shapes measured and rejected on bytes.

- [s18] Bookkeeping: tmp/grind/func_800770B8/dumps/ currently holds the GATE-1 VARIANT's dumps (the last dump.ps1 run), not the floor-5 body's. The floor-5 extractions are saved separately as tmp/grind/func_800770B8/s18/lreg.txt and s18/greg.txt; the gate-1 extraction is s18/fx/p1.lreg.txt. Re-run tools/grinder/dump.ps1 with candidate.c applied before reading dumps/ next session.

- [s19] DISPOSITION SESSION (escalation modality). Chassis re-measured at session start with memory/grind/func_800770B8/candidate.c applied to src/text1b.c: score 5 / build_insns 175 / target_insns 175 / rules_dropped 0. No drift from the s11-s18 ledger floor. src/text1b.c was returned to HEAD (INCLUDE_ASM) before the outcome was written; the only tree changes are docs/grind/decisions.md, memory/grind/func_800770B8/*, tmp/ and metrics/events.jsonl.

- [s19] NEW TOOL, REUSABLE: tmp/grind/func_800770B8/s19/rows.py normalises objdump numeric register names to symbolic ones AND compares on an (opcode, register-operand-tuple) key, so it prints ONLY genuine differences. The s18 diffonly.py compares raw text and therefore reports ~25 false rows per build that are pure objdump aliasing (`move $a2,$zero` vs `addu $a2,$zero,$zero`; `li $v0,2` vs `addiu $v0,$zero,0x2`). Any future session reading a row diff on this function must use the normalised form or it will chase aliases. Usage: `bash tools/wsl.sh 'python3 tmp/grind/func_800770B8/s19/rows.py <obj> [lo] [hi]'`.

- [s19] THE FLOOR-5 RESIDUAL IS RE-CONFIRMED AT EXACTLY FIVE ROWS on today's chassis, by the normalised diff: rows 35, 36 (class B: `sw $zero,0x30($s1)` / `sh $zero,0x34($s1)` vs the target's `sw $zero,0x30($v0)` / `sh $zero,0x34($v0)`) and rows 62, 63, 64 (class C: `addu $v0,$v0,$v1` / `addiu $a3,$v0,106` / `addiu $a1,$v0,126` vs the target's `addu $v1,$v1,$v0` / `addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E`). Rows 60 and 61 MATCH the target exactly in this build.

- [s19] THE LAST LIVE FRONTIER ITEM IS DEAD, AND DEAD ARITHMETICALLY. s18 left one well-posed untried question: run sched_solver perturb.py on the PLAIN operand-order flip, whose loop-head collateral is multiset-identical to the target's (so, unlike s17's Q00 basin, a topologically valid goal). s19 re-measured the plain flip (rejected/s18fx-classC-flip-fixes-tie-but-swaps-seats-175insn-score29.c) at score 29 / 175 / 175 and row-diffed it: under the flip, the class-C REGION rows 60-64 carry FIVE differing rows (row 60 `lw $v1,0($gp)` vs `lw $v0`; row 61 `sll $v0,$v0,0x1` vs `sll $v1,$v1,1`; rows 62/63/64 still wrong) against the floor-5 body's THREE. A scheduling perturbation reorders emissions; it does not undo a seat swap. So the flip basin's CEILING with all 24 collateral rows perfectly repaired is 2 (class B) + 5 (class C region) = 7, strictly worse than the standing floor of 5. Spending the solver run on it cannot drop the floor under any outcome. The item is CLOSED, not deferred -- no future session should re-open it.

- [s19] ENDGAME-LOCK GATE (a) FAILED: `python3 tools/scan_hand_coded.py --single func_800770B8` -> `tier=LOW score=0/8`, "no strong hand-coded indicators", every one of S1-S8 unset (0 multu/mflo pairs; no empty-body branches; 5 spills over 175 insns / 14 distinct regs; max load burst 3 in any 8-insn window; no high-similarity sibling, jaccard < 0.5; no BIOS jumptable pattern; all callee-save uses have an `$sp` save; no redundant mask-before-shift). Ordinary compiler output; the canonical-asm grant path is unavailable.

- [s19] ENDGAME-LOCK GATE (b) FAILED AND VACUOUS: docs/reference/sotn-construct-index.md (1,056 lines) returns ZERO PSX/GCC-2.7.2 hits for either construct shape the residual would need -- `operand order|operand-order|addend|tie|combine_regs|local-alloc|reload live` and `identity|detour|dependence|chain-extend|extra read|redundant read|reread|re-read`. Vacuous because there is no closing construct in hand at all after s18's forensics: class C's only two C-reachable combine_regs gates both need instruction slack a 175==175 body has none of, class B collapses to 170 insns under every spelling reaching the raw pseudo, and the one construct ever measured to repair the flipped basin was Judge-FAILED 2026-09-01.

- [s19] DISPOSITION FILED: docs/grind/decisions.md now carries `## 2026-09-01 - func_800770B8 - **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**`, stating both gate results, the residual attribution, the exhaustion pointers (19 sessions; >=5 modalities; 88 rejected forms; 33,926 permuter iterations across two telemetered campaigns with 9 finds and no match; ~1,900 enumerated sandbox builds; all three solver backends run with full-disposition goals) and the four re-activation triggers. Session outcome: owner-gated, floor 5, escalation_ref pointing at that entry. Per the 2026-08-31 ruling the driver forecloses silently; nothing is surfaced to the owner and nothing waits.

- [s19] candidate.c is UNCHANGED and remains the best known form (score 5, cheat-free apart from the one annotated single-level do-while(0) wrap whose classification s15 closed by citation to .claude/rules/do-while-zero-exception.md). No new rejected form was banked this session: the only two builds were re-measurements of the already-banked floor-5 body and the already-banked s18 plain flip.

- [s19b] FLOOR RE-VERIFIED AT 5 ON TODAY'S CHASSIS, with the reference object rebuilt from PRISTINE main source first: `sandbox func_800770B8 --disable all` = score 5, build_insns 175, target_insns 175, rules_dropped 0.

- [s19b] TOOLING TRAP, BANKED (cost this session ~4 turns): the sandbox scores against `build/src/<stem>.o`, and on main this function is `INCLUDE_ASM`, so build/src/text1b.o IS the target reference. Running `engine build-c text1b` while the candidate body is applied OVERWRITES that reference with our own build and the sandbox then prints a FALSE `score: 0`. Recovery: `git checkout -- src/text1b.c`, `build-c text1b` (regenerates the true reference), then re-apply the candidate and score. Never run build-c with candidate edits in place. Related second trap: extracting the candidate body by searching candidate.c for `s32 func_800770B8(` matches the PROSE mention in its header comment first (line 168, "the prototype becomes `s32 func_800770B8(s32, s32, s32);`") and silently splices comment text into src/; anchor on `\ns32 func_800770B8(s32 arg0`. Applying candidate.c also requires the two caller-side edits it documents (prototype -> `s32 func_800770B8(s32, s32, s32);`, call site -> `(s32)&D_8009BD24`); without them text1b.c compiles but the function is absent from the object and the sandbox reports "not found in tmp/sandbox/...".

- [s19b] CLASS C'S LAST FRONTIER QUESTION IS ANSWERED NO, with three fresh builds. Making an already-present instruction consume the D_800A36A0 reload after the add does not flip the combine_regs tie: deriving both inner cursors from the loop-top `base` local DELETES the reload (score 41, 174 insns -- banked as rejected/s19-classC-base-reuse-deletes-reload-174insn-score41.c), while deriving p_7e from p_6a, or routing both through a shared `pb = D_800A36A0 + t0*10` cursor, both collapse onto the byte-identical floor-5 body (score 5, 175 insns). An extra consumer that folds into the same addiu is not a separate live use, so gate 2 (local-alloc.c:1917, the REG_DEAD requirement) is untouched. Combined with the s19 flip-ceiling arithmetic (flip ceiling 7 > floor 5), class C has no remaining C-reachable attack in either operand order.

- [s19b] The s19 evidence line claiming "no new rejected form was banked" is superseded: s19b banks rejected/s19-classC-base-reuse-deletes-reload-174insn-score41.c (89 forms total).

- [s19] Floor re-verified at 5 on today's chassis with the reference object rebuilt from pristine main source: sandbox func_800770B8 --disable all = score 5, build_insns 175, target_insns 175, rules_dropped 0.

- [s19] TOOLING TRAP banked: the sandbox scores against build/src/<stem>.o, and on main this function is INCLUDE_ASM, so build/src/text1b.o IS the target reference. Running `engine build-c text1b` while the candidate body is applied overwrites that reference with our own build and the sandbox then prints a FALSE score 0. Recovery: git checkout -- src/text1b.c, build-c text1b, then re-apply the candidate and score.

- [s19] Second trap banked: extracting the candidate body by searching candidate.c for 's32 func_800770B8(' matches the PROSE mention in its header comment first (line 168) and silently splices comment text into src/; anchor on a newline followed by 's32 func_800770B8(s32 arg0'. Applying candidate.c also requires its two documented caller-side edits (prototype -> s32 func_800770B8(s32, s32, s32); call site -> (s32)&D_8009BD24) or the function is absent from the object and the sandbox reports 'not found in tmp/sandbox/...'.

- [s19] Class C's last frontier question is answered NO with three fresh builds: base-local reuse deletes the reload (score 41, 174 insns, banked as rejected/s19-classC-base-reuse-deletes-reload-174insn-score41.c); p_7e-from-p_6a and a shared pb cursor both collapse onto the byte-identical floor-5 body (score 5, 175 insns).

- [s19] Gate (a) FAILED: scan_hand_coded tier=LOW score=0/8, every signal unset.

- [s19] Gate (b) FAILED: zero PSX/GCC-2.7.2 hits in docs/reference/sotn-construct-index.md for either construct shape.

- [s19] Disposition RE-FILED at docs/grind/decisions.md:17443 with a header line carrying the literal token the driver's validator matches ('OWNER-ESCALATION') alongside the standing-ruling title - the previous session was discarded solely because its header omitted that token (driver check: tools/grinder/grind.ps1:1058 requires a line matching 'OWNER-ESCALATION|CANONICAL-ASM GRANT PATH' AND the function name).

- [s19] candidate.c is unchanged and remains the best known form (score 5, cheat-free apart from the one annotated single-level do-while(0) wrap whose classification s15 closed by citation to .claude/rules/do-while-zero-exception.md). Rejected bank now holds 89 forms. src/text1b.c was restored to HEAD before finishing.

## [s20] escalation — the owner's Ruling-A named probe, executed (36 fresh builds)

Chassis re-verified FIRST, on a reference `build/src/text1b.o` rebuilt from PRISTINE main
source this session (`engine build-c text1b`, sha1 55a39c6dbcf00ec4feda82df16f9325319e1e5a8):
`memory/grind/func_800770B8/candidate.c` applied to src/text1b.c (plus the two byte-neutral
caller-side edits it documents) measures **score 5 / build_insns 175 / target_insns 175 /
rules_dropped 0**. The floor is still 5; the ledger number and the chassis agree.

**The probe.** The 2026-09-01 FORECLOSED-BUCKET REVIEW (Ruling A) returned this function to
active on the ground that both class closures are PRICE arguments resting on "zero insn slack
(175 == 175)" while the ledger holds measured 173/174-insn spellings, and that the
fence-device x slack-spelling cross-product had never been built. This session built it:
`tmp/grind/func_800770B8/s20/gen.py` -> `s20/v/` (36 bodies), scored one at a time by
`s20/run.sh` (each build re-applies a body to a pristine src/text1b.c and runs
`sandbox func_800770B8 --disable all`); full log `s20/sweep.log`.

Axes:
- bases: **F** = candidate.c (unflipped ABCD, floor, 5/175); **C** = s12 `perm/Q12.c`
  (flipped CABD basin, 12/175 — the build whose rows 60-64 are the target's byte-exactly,
  register seats included).
- label devices (the only constructs ever measured to emit the target's class-B 2+2 split):
  **c** = s17 C1 duplicated-arm join label; **b** = s17 B1 post-copy branch.
- slack spellings: **D** = s13 third loop as `do/while` (-1 insn); **G** = s18fx class-C
  gate-1 block-local reload `b2` (-1); **E** = D+G composed; **P** = s13 inner-loop pointer
  walk `*p_6a++` (-2); **Q** = P+D (-3).

**Result 1 — the "zero slack" premise is FALSIFIED as stated, and the price argument survives
anyway, now measured instead of asserted.** Instruction counts compose EXACTLY additively:
every device costs +2 insns, every -1 slack spelling gives back exactly 1, and the two
compose. Eight builds this session are 175-insn bodies CARRYING A SURVIVING CODE_LABEL —
the object s17 declared unconstructible ("this function has ZERO insn slack"):

    FPb 175/43   FPc 175/51   FEb 175/56   FEc 175/65
    CPb 175/52   CPc 175/60   CEb 175/47   CEc 175/56

So a 175-insn label build EXISTS. It is 43 at best against a floor of 5.

**Result 2 — SCORE is additive too, and that is what forecloses the family.** Measured
deltas on the F base: device b = +18 rows, device c = +25; slack D = +1, P = +21, G = +36,
E = +37, Q = +22. Every combination lands within 2 of the sum of its parts (e.g. FDc = 31 =
5+25+1; FPc = 51 = 5+25+21; FPb = 43 = 5+18+21). Two consequences:
- the CHEAPEST slack (D, +1 row) only buys -1 insn, and the cheapest -2 slack in nineteen
  sessions of spellings costs +21 rows (P), so a device can only be brought back to 175 by
  paying ~+21 rows before the device's own collateral is counted;
- the device's own collateral (+18 / +25 rows) ALREADY exceeds by an order of magnitude the
  2 rows class B is worth. Even a hypothetical free -2 slack would leave the best label build
  at 23. Class B is foreclosed by the DEVICE'S price, not by the insn budget — the correction
  the ruling asked for strengthens the closure rather than reopening it.

**Result 3 — the flip ceiling recomputed over the CABD basin: 10, i.e. 5 worse than the
floor.** The CABD basin was never priced against slack/devices before. All twelve C-base
builds this session are >= 12 (C00 12, CD0 13, CG0 32, CE0 33, CP0 35, CQ0 36, plus the six
device builds 30..61). With s12's exhaustive 79-position second-wrap sweep on this exact base
(minimum 10 at W039/W040, which additionally costs a second FAKE-annotated wrap) the basin's
true ceiling is **10**. s19's "ceiling 7" figure came from the flipped-ABCD basin; the CABD
number is worse, not better. The only value below 5 ever recorded on any flipped basin is
s14's P8 = 4/175, which IS the `(t0 * 4) >> 1` arithmetic-identity detour the Judge FAILED on
2026-09-01 and which remains banned; nothing in this sweep uses it.

**Gate (a) re-run this session — FAILED.** `python3 tools/scan_hand_coded.py --single
func_800770B8` (log `tmp/grind/func_800770B8/s20/scan.log`): `tier=LOW score=0/8`, "no strong
hand-coded indicators", all of S1-S8 unset (0 multu/mflo pairs; no empty-body branches; 175
insns / 5 spills / 14 distinct regs; max load burst 3 per 8-insn window; no high-similarity
sibling; no BIOS jumptable pattern; every callee-save use paired with an `$sp` save; no
redundant mask-before-shift). Ordinary GCC 2.7.2 output; the canonical-asm grant path is not
available.

**Gate (b) re-run against the UNCAPPED index — FAILED.** The review's Correction 2 (census
ran against a cap-truncated artifact) is answered: `docs/reference/sotn-construct-index.md`
has since been rebuilt uncapped (2,746 lines, commit aa535002, generated 2026-09-01, vs the
1,056-line artifact s19 used). Re-censused on the new index: ZERO hits for
`combine_regs|local-alloc|reg_qty|operand[ -]order|register seat|swap operand`. The gate is
also vacuous for the same reason as in s19 — after this sweep there is still no closing
construct in hand to seek precedent FOR: the only constructs that emit either target row
group are the two label devices (>= +18 rows of collateral, measured above) and the banned
identity detour.

- [s20] Chassis re-verified on a PRISTINE reference: `engine build-c text1b` from untouched main source regenerated build/src/text1b.o (sha1 55a39c6dbcf00ec4feda82df16f9325319e1e5a8) BEFORE any probe; candidate.c then measures score 5 / build_insns 175 / target_insns 175 / rules_dropped 0. Floor flat at 5 since s11 (ten sessions, five modalities).

- [s20] The owner's Ruling-A named probe is EXECUTED IN FULL, not deferred: 36 builds spanning the exact cross-product the ruling specified, plus the two -2-insn slack spellings (E = D+G, P = pointer walk) needed to bring a +2-insn device back to 175.

- [s20] FALSIFIED: 'this function has zero insn slack (175 == 175)' as a constructibility claim. Eight 175-insn bodies carrying a surviving CODE_LABEL were built this session (FPb 43, FPc 51, FEb 56, FEc 65, CPb 52, CPc 60, CEb 47, CEc 56).

- [s20] CONFIRMED and now quantified: score composes additively on these axes. Device collateral on the floor base is +18 (B1) / +25 (C1) rows; slack collateral is +1 (D), +21 (P), +36 (G), +37 (E), +22 (Q). Class B is worth 2 rows, so the device's own price forecloses it by ~9x regardless of the instruction budget.

- [s20] CABD basin priced for the first time against slack and devices: minimum 12 across twelve builds, 10 including s12's second-wrap sweep — the basin's ceiling is 10, five worse than the floor and worse than the flipped-ABCD basin's 7. s19's 'ceiling 7' was not an under-estimate.

- [s20] Both endgame gates re-fail on freshly-run evidence: scan_hand_coded tier=LOW 0/8; uncapped sotn-construct-index census returns zero for every seat/operand-order shape.

- [s20] Four new rejected forms banked (93 total), including the first 175-insn surviving-label builds this ledger has ever held: rejected/s20-classB-B1-label-device-plus-pointer-walk-slack-175insn-score43.c, s20-classB-C1-joinlabel-plus-pointer-walk-slack-175insn-score51.c, s20-classB-C1-joinlabel-plus-dowhile-gate1-slack-175insn-score65.c, s20-CABD-basin-plus-B1-label-slack-175insn-score52.c.

- [s20] src/text1b.c was restored to its pristine committed state at the end of the sweep; the only tracked edits this session are the ledger, decisions.md, and the four rejected forms.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=20

### s21 (2026-09-05, rederive modality)

- [s21] **CHASSIS MOVED AND THE FLOOR SURVIVED IT.** `src/text1b.c` at HEAD is no longer the file s20 measured against: `func_80060A68` landed as matched C in the SAME translation unit (commit 27441fa5, +105 lines of C replacing its `INCLUDE_ASM`, and three `extern s32 D_800F119x;` file-scope decls removed). First action of the session was to re-measure: `memory/grind/func_800770B8/candidate.c` applied to the new pristine (`tmp/grind/func_800770B8/s21/pristine_text1b.c`, taken from `git show HEAD:src/text1b.c`) with the two documented byte-neutral caller-side edits gives `sandbox func_800770B8 --disable all` = **score 5, build_insns 175, target_insns 175**. The TU-mate landing is byte-neutral for this function; every chassis-relative conclusion in the ledger still holds.

- [s21] The residual is still exactly five rows and they are still the two known classes. Row-diff of the floor build against `asm/funcs/func_800770B8.s` (`tmp/grind/func_800770B8/s19/rows.py`, ignoring the tool's known `li`-vs-`addiu $zero` and `move`-vs-`addu $zero` false positives): rows **35, 36** (class B — ours `sw $zero,48($s1) / sh $zero,52($s1)`, target `sw $zero,0x30($v0) / sh $zero,0x34($v0)`) and rows **62, 63, 64** (class C — ours `addu $v0,$v0,$v1 / addiu $a3,$v0,106 / addiu $a1,$v0,126`, target `addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E`).

- [s21] **CLASS C's TIE AND ITS SEATS ARE ONE BIT OF C, NOT TWO — the s18-forensics frontier instruction ("start from this body and attack the tie, not from the flip and attack the collateral") is answered NEGATIVE.** Eleven fresh builds (`s21/gen.py`, `s21/gen2.py`, runner `s21/run.sh`, log `s21/sweep.log`, bodies in `s21/v/`) sweep every way of separating the two decisions:
  - staged reload: `s32 g2 = (s32)D_800A36A0;` then `(t0*10) + g2 + 0x6A` (v1) — **29/175**;
  - the same staged as a pointer, cast at the use site (v2) — **29/175**;
  - both operands staged, shift declared first (v3) — **29/175**;
  - staged reload with `p_7e = p_6a + 10` (v5) — **29/175**;
  - staged reload HOISTED above the `sb $t0,0x68` store group, so its pseudo is born several insns earlier (v6) — **29/175**;
  - staged shift `s32 sh = t0 * 10;` then `sh + (s32)D_800A36A0 + 0x6A` (v7) — **29/175**;
  - the same with `sh` hoisted to the outer-loop top (v8) — **29/175**;
  - staged shift with `p_7e = p_6a + 10` (v10) / `p_7e = (s16 *)((s32)p_6a + 0x14)` (v11) — **29/175**;
  - staged reload with POINTER-first tree order (v4) — **5/175**, byte-identical to the floor;
  - pointer-first written entirely in the integer domain, `(s32)D_800A36A0 + (t0*10) + 0x6A` (v12) — **5/175**, byte-identical to the floor.
  All eleven land on exactly TWO objects, partitioned by one thing only: **which operand the source names first**. Declaration staging, naming, the integer-vs-pointer domain of the global, the hoist position of either intermediate, and the derivation of `p_7e` are all byte-inert here. Together with s15's eleven flip spellings and s6/s9/s13's earlier ones, **at least twenty-two distinct C spellings of this address now collapse onto the same two builds.**

- [s21] **THE .lreg DUMPS NAME WHY THEY CANNOT BE SEPARATED.** `pwsh tools/grinder/dump.ps1 func_800770B8` on the floor body and on v1 (saved as `s21/dumps_base/text1b.lreg` and `s21/dumps_flip_v1_lreg`):
  - floor body, tree order = (reload, shift): `Register 108 used 4 times across 2 insns in block 1` (the `sll` = t0*10), `Register 109 used 4 times across 6 insns in block 1` (the `lw` of D_800A36A0), `Register 110 used 6 times across 3 insns` (the sum); allocation `;; Register 108 in 3. / ;; Register 109 in 2. / ;; Register 110 in 2.`
  - v1, tree order = (shift, reload): the spans swap — `Register 108 ... across 3 insns` is now the reload and `Register 109 ... across 2 insns` is the shift.
  So **expand numbers the two pseudos in the REVERSE of the source tree's operand order**, and `find_free_reg` then hands `$2` to the LATER-numbered quantity in both builds. The operand a body names FIRST is therefore always born SECOND and is always the one that ends up in `$2`; `block_alloc`'s ordered `combine_regs` loop (local-alloc.c:1240-1298) then ties the sum to that same first-named operand. The target needs the tie on the SHIFT (`addu $v1,$v1,...`) **and** the shift in `$3` — i.e. that single bit of C set both ways simultaneously. That is the mechanism behind s18fx's observation that the flip gives "right tie, WRONG SEATS", generalised from one build to the whole spelling space.

- [s21] Collateral read-out for the flip, for the record (row-diff of v1): the +24 rows are NOT a local perturbation of rows 60-64. Rows 38-59 are opcode-for-opcode the same work with a rotated register assignment plus one displacement — the `sll $v1,$a1,2` that computes `t0*4` sits at target row 42 and floor-body row 42, but at row **50** in every flipped build, because the flipped `t0*10` expansion changes where CSE materialises the shared `t0<<2`. Rows 60-64 in v1 print `lw $v1,0($gp) / sll $v0,$v0,0x1 / addu $v0,$v0,$v1 / addiu $a3,$v0,106 / addiu $a1,$v0,126` — the target's tie on the shift, with the reload and the shift holding each other's seats.

- [s21] **SIBLING SWEEP: the CD_ready pairing is documentary, not structural.** The dispatch brief flagged `CD_ready` (aka `marionation_Exec`, src/system.c, foreclosed at floor 2 since its s60, candidate rewritten 2026-09-04) as an UNSPENT sibling. Read `memory/grind/CD_ready/candidate.c` end-to-end and traced every mention of it in this ledger (evidence.md:1245, 1271; hypotheses.md:988, 1012). There is **no shared code window**: CD_ready is a marionation dispatch body with no call, global, loop or addressing idiom in common with func_800770B8, so there is nothing to transplant. Every citation of it here is a POLICY citation — it is the named "Confirmed application" inside `.claude/rules/do-while-zero-exception.md`, which s11 used to establish that this body's annotated single-level `do { } while (0);` prologue fence is sanctioned and that the honest floor is 5 rather than 9. Its s84 content (the `pri(x) = max over preds p of (pri(p) + insn_cost(p,kind,x) - 1)` recurrence; the correction that solver-json `ref` is INSN_REF_COUNT and not `reg_n_refs`; the edge-removal model bug that voided its own s83 sweeps) is transferable METHOD for any future sched_solver work on this function, but it names no construct this function can borrow. This pairing is now spent; no further sibling sweep is owed for it.

- [s21] REDERIVE-LADDER NOTE. The modality's fresh-m2c rung is already spent (s10, evidence.md:1194 — the m2c body corroborates the current one statement-for-statement and its only two structural readings are the class-C flip and the hoisted `sp[]` slot pointer, both banked). The remaining rederive rungs are the decomp.me corpus scrape (`tools/decomp_me_scrape.py`, never run for this function) and a Kengo/PS2 transplant (project memory `slog-kengo-dead-end` records that BB2 functions surveyed so far have no Kengo equivalent at function, sub-region or callee-signature level). Neither was reached this session; the class-C decoupling probe consumed it. Recorded here so the next rederive session starts on the corpus scrape rather than re-running m2c.

- [s21] CHASSIS MOVED AND THE FLOOR SURVIVED IT. src/text1b.c at HEAD is not the file s20 measured against - func_80060A68 landed as matched C in the SAME translation unit (commit 27441fa5, +105 lines replacing its INCLUDE_ASM, three file-scope extern s32 D_800F119x decls removed). candidate.c applied to the new pristine (tmp/grind/func_800770B8/s21/pristine_text1b.c from git show HEAD:src/text1b.c) with the two documented byte-neutral caller-side edits measures score 5, build_insns 175, target_insns 175. The TU-mate landing is byte-neutral here; every chassis-relative conclusion in the ledger still holds.

- [s21] The residual is still exactly five rows in the two known classes: rows 35-36 (class B, ours sw $zero,48($s1) / sh $zero,52($s1) vs target sw $zero,0x30($v0) / sh $zero,0x34($v0)) and rows 62-64 (class C, ours addu $v0,$v0,$v1 / addiu $a3,$v0,106 / addiu $a1,$v0,126 vs target addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E).

- [s21] Eleven fresh builds partition on ONE bit of C: shift-first tree order = 29/175, reload-first tree order = 5/175. v12 in particular proves the integer domain is not the lever - (s32)D_800A36A0 + (t0*10) + 0x6A is byte-identical to the pointer-first floor body - so the s6/s9/s13 'int-domain flip' results were measuring operand order, not the cast.

- [s21] The .lreg dumps pin the mechanism: expand numbers the reload and the shift pseudos in the REVERSE of the source tree's operand order, and find_free_reg gives $2 to the later-numbered quantity, so tie-target and seat-target are the same source decision with opposite senses. This generalises s18fx's single-build observation ('the flip fixes the tie but swaps the seats') to the whole spelling space.

- [s21] Flip collateral read-out (row-diff of v1): the +24 rows are not local to rows 60-64. Rows 38-59 are the same opcodes with a rotated register assignment plus one displacement - the sll computing t0*4 sits at row 42 in the target and in the floor body but at row 50 in every flipped build, because the flipped t0*10 expansion moves where CSE materialises the shared t0<<2.

- [s21] REDERIVE-LADDER STATE for the next session: the fresh-m2c rung is already spent (s10, evidence.md:1194 - the m2c body corroborates the current one statement-for-statement; its only structural readings are the class-C flip and the hoisted sp[] slot pointer, both banked). The two unspent rederive rungs are the decomp.me corpus scrape (tools/decomp_me_scrape.py, never run for this function) and a Kengo/PS2 transplant (project memory slog-kengo-dead-end records no Kengo equivalent for the BB2 functions surveyed so far). Start there rather than re-running m2c.

## [s22] structural — the class-C residual re-typed as a quantity-priority problem; frontier item #2 closed

Chassis re-verification. `sandbox func_800770B8 --disable all` on
`memory/grind/func_800770B8/candidate.c` applied to src/text1b.c (with the two
byte-neutral caller-side edits the candidate documents: prototype
`s32 func_800770B8(s32, s32, s32);`, call site `(s32)&D_8009BD24`) = **score 5,
build_insns 175, target_insns 175** at commit 768f08a4. The plain flipped base
scores 29/175. Both numbers were re-measured live before any probe.

Mandated FAKE kill re-audit. `tools/fake_ablate.py` reports ONE FAKE unit in the
candidate (the empty `do { } while (0);` prologue fence at line 206): keep-all 5,
drop-1 10. The fence is load-bearing, worth 5 rows, and lives in the prologue
(class A); it occupies none of the block-1 pseudos that class B and class C
live in, so no banked class-B/class-C kill on this chassis is a FAKE-masking
artefact.

Frontier item #2 (the ledger's standing next probe) is CLOSED NEGATIVE. The
s20 device x slack cross-product was re-run on the PLAIN flipped base, the one
base s20 never covered (it covered the floor base F and the flipped-CABD base C).
17 builds, using the s20 transform definitions re-executed verbatim so the
devices and slack spellings are bit-identical to the ones s20 priced:

    X0  29   X0b 47   X0c 54
    XD0 30   XDb 48   XDc 55
    XG0 41   XGb 55   XGc 64
    XE0 42   XEb 56   XEc 65
    XP0 48   XPb 65   XPc 73
    XQ0 49   XQb 66   XQc 74

Device B1 = +18 rows and C1 = +25 on this base exactly as on the F base; slack
D +1, G +12, E +13, P +19, Q +20. Nothing is refunded: the flip's collateral and
the device's collateral stack. Cheapest build of any kind on this base is 30,
best surviving-label 175-insn build is 56, against a floor of 5. The "class B and
class C are payable together" hypothesis is dead on all three bases now.

THE NEW RESULT — class C is a quantity-PRIORITY residual, and the sum always
takes $v0. 20 further builds in three families (priority levers w1-w9, sum
write-back / variable-reuse levers y1-y7, seat-competition levers z1-z4) land on
exactly four objects:

  (i)  5/175, byte-identical to the floor — y1, y6, z2, z4. Reload-first source
       tree, sum tied to the reload. Note z2 (`p_7e = p_6a + 10`) and z4
       (`p_7e = (s16 *)((s32)p_6a + 0x14)`) are free respellings of the second
       cursor that cost nothing, and y1/y6 show the shift can be staged into its
       own named local with no byte change as long as the sum stays reload-first.
  (ii) 29/175 — y2, y3, y4, y5, y7 and the plain flip X. Sum tied to the shift.
       Row diff of y2 at rows 60-64:
           ours   lw $v1,0($gp) / sll $v0,$v0,1 / addu $v0,$v0,$v1 /
                  addiu $a3,$v0,106 / addiu $a1,$v0,126
           target lw $v0,%gp_rel(D_800A36A0) / sll $v1,$v1,1 /
                  addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E
       The TIE is correct and the two SEATS are swapped, and the swap drags 24
       further rows of preheader rotation with it (rows 38-59 all reallocate).
  (iii) 25/171 — w1, w4, w9. Consuming one hoisted `s32 sh = t0 * 10;` in both
       the cursors and the for-loop's `s16 idx = (s16)(a2 + sh)` is a real CSE
       that DELETES four instructions the target keeps. The target computes the
       index loop's t0*10 separately (`sll $a3,$v0,1` in the for-loop preheader,
       target rows 78-82) and recomputes it for the cursors (target rows 64-65),
       so sharing is off-target by construction, whatever it does to the seats.
  (iv) worse: w2 41, w3 51, w5 64, w6 64, w7 71, w8 41, z1 26, z3 49.

Two findings fall out.

FIRST, a refinement of s21. s21 concluded "the only bit of C that reaches this
decision is which operand the source names first". That is not quite it: y2
(`sh = (s32)D_800A36A0 + sh;`) and y3 (`sh += (s32)D_800A36A0;`) both use a
RELOAD-FIRST source tree and still produce object (ii). What selects the object
is which QUANTITY the sum's pseudo gets merged into by block_alloc's combine_regs
(local-alloc.c:1240-1298), and C can express that merge either through operand
order or by assigning the sum into the shift's own variable. Variable reuse,
compound assignment, pointer-vs-integer domain, reusing the loop's existing `ptr`
scratch, and declaration position are all merely different spellings of the same
merge and give the same 29.

SECOND, the observation that re-types the residual. In ALL four objects the
cursor sum receives $v0 — the lower-numbered free register — whether it is tied
to the reload (object i), tied to the shift (object ii), or fed by a shared
loop-level shift sitting in $t1 (object iii). The target puts that sum in $v1
and the reload in $v0. So the open question is no longer "which operand does the
sum tie to" (C reaches both) but "why is the target's merged shift+sum quantity
NOT first in local-alloc's qty_order".

The arithmetic, from a fresh .lreg dump of the floor body
(tmp/grind/func_800770B8/dumps/text1b.lreg, ";; Function func_800770B8"):

    Register 108  the t0*10 shift    used 4 times across 2 insns in block 1
    Register 109  the D_800A36A0 reload  used 4 times across 6 insns in block 1
    Register 110  the sum            used 6 times across 3 insns in block 1

qty_compare (local-alloc.c:1630-1657) orders by
floor_log2(n_refs)*n_refs*size/(death-birth). Merged shift+sum is ~10 refs over
a ~3-insn span, priority ~10.0; the reload alone is 4 refs over 6 insns, ~1.33.
For find_free_reg to reach the reload FIRST (and hand it $2, leaving $3 for the
merged shift+sum, which is the target's disposition) the merged quantity's span
would have to grow past roughly 23 insns at the same reference count — i.e. the
shift's pseudo must be born about twenty insns before the addu while STILL being
merged with the sum. Note s21's v6/v8 hoisted the shift statement to the
outer-loop top and did not achieve this, so statement position alone does not
move the pseudo's birth that far; what is untested is a form where the shift's
VALUE is genuinely produced early because a target-present earlier statement
consumes it.

Sibling sweep, both remaining unspent pairings, closed. CD_datasync
(saEft01Init) and CD_sync (cpu_side_move_dir_4), both src/system.c, both
foreclosed at floor 2, have ZERO global overlap with this function (they touch
D_800161B8/C8, D_80016240, D_800A11B4/B8/D5/DC, D_800A125C, D_800A147C,
D_800A1494, D_800F19A0/A8/B8/BC/C0 against this function's D_8009BCE4,
D_8009BD21, D_800A35D0/D8/DC/E8, D_800A36A0, D_800A374C) and neither names
func_800770B8 or text1b anywhere. No shared window, nothing to transplant; with
s21's CD_ready result all three sibling pairings are now spent. The one thing
worth carrying is a TECHNIQUE pointer: CD_sync's candidate.c:232 carries the
sanctioned F1 combine-foldable chain-extender FAKE — a link-constant SYMBOL_REF
difference that combine.c folds to zero emitted bytes after flow.c has already
recorded the extra reg_n_refs. That is the only byte-neutral reg_n_refs lever
known in this project and it is the right SHAPE for a qty_order residual. It is
not proposed here: the priority arithmetic above wants a ~20-insn SPAN change,
not a reference-count change, and a chain-extender moves refs, not span.

Artifacts: tmp/grind/func_800770B8/s22/ (gen_cross.py, gen_prio.py, gen_reuse.py,
gen_z.py, mkbase.py, apply.py, run.sh, rowdiff.sh, v/ = the 33 bodies,
sweep.log), tmp/grind/func_800770B8/dumps/text1b.lreg (fresh this session).
Banked rejected forms: rejected/s22-sum-written-into-shift-variable-seats-swap-29.c,
rejected/s22-shared-t0x10-cse-loses-4-target-insns-25of171.c,
rejected/s22-named-reload-reused-post-loop-kills-target-reload-49.c,
rejected/s22-flipbase-B1-label-device-additive-47.c.

- [s22] Floor re-verified live on the current chassis (commit 768f08a4, func_80060A68 already matched in the same TU): candidate.c = score 5, build_insns 175, target_insns 175. Plain flipped base = 29/175.

- [s22] Mandated FAKE kill re-audit: fake_ablate finds one FAKE unit (the do-while(0) prologue fence); keep-all 5, drop-1 10. Load-bearing, prologue-scoped, touches no class-B/class-C pseudo.

- [s22] The row diff of the floor body against asm/funcs/func_800770B8.s shows the entire honest residual is five rows: 35 and 36 (`sw $zero,0x30($s1)` / `sh $zero,0x34($s1)` vs the target's `$v0` base — class B) and 62, 63, 64 (`addu $v0,$v0,$v1` / `addiu $a3,$v0` / `addiu $a1,$v0` vs the target's `addu $v1,$v1,$v0` / `addiu $a3,$v1` / `addiu $a1,$v1` — class C). Every other differing row the tool prints is a cosmetic move-vs-addu or li-vs-addiu rendering difference.

- [s22] The s20 device/slack price structure reproduces on the plain flip base: B1 = +18 rows, C1 = +25, D = +1, G = +12, E = +13, P = +19, Q = +20. The costs stack with the flip's own 24 rows rather than cancelling; cheapest plain-flip build 30, best 175-insn surviving-label build 56.

- [s22] In every one of the four reachable objects the cursor sum is allocated $v0 — the lower-numbered free register — whether the sum is tied to the reload (floor, 5), tied to the shift (y2/X, 29), or fed by a shared loop-level shift living in $t1 (w1, 25/171). The target allocates that sum $v1 and the reload $v0.

- [s22] s21's 'the tie follows the source tree's operand order' is refined: y2 (`sh = (s32)D_800A36A0 + sh;`) and y3 (`sh += (s32)D_800A36A0;`) both name the reload FIRST and still produce the 29 object. What selects the object is which quantity block_alloc's combine_regs merges the sum's pseudo into; C can express that merge through operand order OR by assigning the sum into the shift's own variable, and both spellings give the same bytes.

- [s22] Fresh .lreg dump (tmp/grind/func_800770B8/dumps/text1b.lreg, ';; Function func_800770B8'): reg 108 = the t0*10 shift, 'used 4 times across 2 insns in block 1'; reg 109 = the D_800A36A0 reload, 'used 4 times across 6 insns in block 1'; reg 110 = the sum, 'used 6 times across 3 insns in block 1'. Under qty_compare's floor_log2(n)*n*size/(death-birth) the merged shift+sum quantity is ~10 refs over ~3 insns (~10.0) against the reload's ~1.33, so the merged quantity is ordered first and takes $2; the target's disposition needs the reload ordered first, which at the same reference count wants the merged quantity's span to exceed roughly 23 insns.

- [s22] Sharing t0*10 between the cursor block and the for-loop index is off-target regardless of its allocation effect: it deletes four instructions the target keeps. The target computes the index loop's t0*10 separately in the for-loop preheader (`sll $a3,$v0,1`, target rows 78-82) and recomputes it for the cursors (rows 64-65).

- [s22] Two free respellings of the second cursor are now banked as byte-neutral on the floor body: `p_7e = p_6a + 10` (z2) and `p_7e = (s16 *)((s32)p_6a + 0x14)` (z4), both 5/175. So is staging the shift into a fresh named local while keeping the sum reload-first (y1, y6, both 5/175).

- [s22] Sibling sweep complete: CD_datasync, CD_sync and (from s21) CD_ready all have zero code overlap with this function. CD_sync candidate.c:232 carries the sanctioned F1 combine-foldable chain-extender FAKE, the only byte-neutral reg_n_refs lever known in the project — recorded as a technique pointer, not proposed, because this residual needs span rather than refs.

## [s23] structural — CLASS C IS SOLVED IN PURE C AT 175 INSNS (first time in 23 sessions); its price is a store-group source reorder

Chassis re-verification, first action of the session. `memory/grind/func_800770B8/candidate.c`
applied to src/text1b.c with the two byte-neutral caller-side edits it documents
(prototype `s32 func_800770B8(s32, s32, s32);`, call site `(s32)&D_8009BD24`):
`sandbox func_800770B8 --disable all` = **score 5, build_insns 175, target_insns 175**.
Plain flipped base X = **29/175**. Mandated FAKE re-audit re-run on this chassis:
`tools/fake_ablate.py --func func_800770B8 --file text1b --candidate memory/grind/func_800770B8/candidate.c`
reports ONE FAKE unit (the empty `do { } while (0);` prologue fence, line 206),
keep-all 5 / drop-1 10 — unchanged, load-bearing, prologue-scoped.

72 fresh builds this session. Scratch: `tmp/grind/func_800770B8/s23/`.

### THE RESULT

`tmp/grind/func_800770B8/s23/v/fCADS.c` — the plain flipped cursor base with the
outer loop's four store groups written in the source order **C, A, D, S** instead of
**A, D, C, S** — measures **12/175** and its row diff against `asm/funcs/func_800770B8.s`
shows rows **55-64 byte-exact**, including the row that has been the class-C residual
since s6:

        row 62   ours `addu $v1,$v1,$v0`   target `addu $v1,$v1,$v0`
        row 63   ours `addiu $a3,$v1,106`  target `addiu $a3,$v1,0x6A`
        row 64   ours `addiu $a1,$v1,126`  target `addiu $a1,$v1,0x7E`

Class C is therefore NOT unreachable and NOT priced at +24 rows of preheader rotation.
The tie AND both seats are simultaneously correct at 175 instructions. The residual on
that build is (a) class B's two rows 35/36, unchanged, and (b) the reorder itself: rows
40-54 contain exactly the same instruction multiset as the target in a different order,
because the C group's two `sh` stores now precede group A's five.

Banked: `rejected/s23-classC-SOLVED-flip-plus-C-group-first-175insn-score12.c` and
`rejected/s23-classC-SOLVED-flip-plus-D-group-first-175insn-score14.c`.

### THE DISCRIMINATOR, MEASURED EXHAUSTIVELY

All 24 permutations of the four outer-loop store groups were built on the flipped base
(`gen_f.py`; A = the `t0*2` group storing 0x10/0x8/0xC/0x14/0x3C, D = the `D_800A35D0`
group, C = the `0x40/0x42` group, S = the `0x68` byte store):

    fADCS 29  fADSC 29  fACDS 29  fACSD 29  fASDC 38  fASCD 29
    fDACS 14  fDASC 16  fDCAS 22  fDCSA 23  fDSAC 38  fDSCA 37
    fCADS 12  fCASD 13  fCDAS 16  fCDSA 17  fCSAD 22  fCSDA 38
    fSADC 29  fSACD 29  fSDAC 38  fSDCA 37  fSCAD 35  fSCDA 38

The rule is sharp and has no exception in 24 builds: **every order whose first
`t0`-indexed store group is A produces the 29-object (correct tie, swapped seats);
every order in which D or C precedes A produces the target's tie AND the target's
seats.** S is inert (S-then-A is still 29).

### WHY s22's PRIORITY MODEL IS FALSIFIED (and what replaces it)

s22 predicted the seat flip would need the merged shift+sum quantity's span to grow past
~23 insns at unchanged reference count. Two `.lreg` dumps taken this session
(`tmp/grind/func_800770B8/s23/X.lreg`, `s23/fCADS.lreg`, via `pwsh tools/grinder/dump.ps1`)
show the four pseudos carry **IDENTICAL** reference and span counts in the 29-object and
in the 12-object:

    Register 107 (t0*5)     used 4 times across 3 insns in block 1   (both)
    Register 108 (t0*10)    used 4 times across 2 insns in block 1   (both)
    Register 109 (reload)   used 4 times across 6 insns in block 1   (both)
    Register 110 (the sum)  used 6 times across 3 insns in block 1   (both)

and only the seats differ:

    X      ;; Register 107 in 2. / 108 in 2. / 109 in 3. / 110 in 2.
    fCADS  ;; Register 107 in 3. / 108 in 3. / 109 in 2. / 110 in 3.

So `qty_compare`'s ratio (local-alloc.c:1630-1657) is numerically equal in both builds and
CANNOT be what orders them. The order therefore comes from `qty_compare_1`'s documented
tie-break — "If qtys are equally good, sort by qty number" (local-alloc.c:1681-1683) — or
from the suggested-hard-register list `qty_sugg_compare` handles first; either way the
selector is the **position at which each pseudo is first encountered in the block's insn
scan**, which store-group source order moves and which reference counting does not.
s22's "span must exceed 23 insns" instruction is retired. The X dump also nails the RTL:
`(insn 185 ... (set (reg:SI 110) (plus:SI (reg:SI 108) (reg:SI 109))))` with REG_DEAD on
both — the flip really does put the shift in operand 1, exactly as s18 predicted.

### WHY THE PRICE CANNOT BE REFUNDED BY SPELLING (13 builds)

The reorder's ~10 rows of collateral are store rows, and stores cannot be scheduled across
each other by sched1 or sched2 (unknown aliasing), so **source store order is emission
store order** and the target's emission (A's five `sh` first) pins the target's source
order to A-first. Every attempt to obtain the discriminator without moving the stores was
measured and fails:

  - hoisting the D group's *pointer* above group A, stores left in place: e1 40, e6 28.
  - hoisting only the `D_800A35D0` symbol address: e2 31, e3 31.
  - hoisting `s32 q4 = t0 * 4;` above group A: c1 29 (the sll is re-sunk to its use).
  - hoisting group C's pointer, stores in place: g1 25; both pointers hoisted: g3 20.
  - respelling group A base-first (`base + (t0 * 2)`): g2 29, g4 25.
  - splitting a group so only ONE store crosses: h1 28, h2 42, h4 30; hoisting only the
    1-insn 0x68 store: h3 29.
  - the same 24-order sweep's A-first half, six independent builds, all 29.

Pointer hoists move the score to 20-25 (so they perturb the scan partially) but never
reach the discriminator; only moving the STORES does.

### FLIP-SPELLING INVARIANCE RE-CONFIRMED ON THE A-FIRST BASE (4 builds)

`i1` `(s16 *)((t0*5)*2 + (s32)D_800A36A0)` 29; `i2` `u8 *row = (u8 *)((t0*10) + (s32)D_800A36A0)`
29; `i4` named `s32 off = t0*10;` 29; `i3` `(t0*5) + (s16 *)D_800A36A0` collapses back to
base-first and measures 5. Consistent with s15's eleven flip spellings: on an A-first
source order every flip spelling is one build.

### A SEPARATE NEW OBJECT: THE 5-WIDE ROW INDEX (7 builds)

Writing the two cursors as an index into a 5-wide `s16` row —
`(s16 *)(D_800A36A0 + 0x6A) + (t0 * 5)` — produces a **new 176-insn object at score 9**
(a1, a3, a5, a6, a7 all identical). Its rows 40-56 are byte-exact with the target
*including* row 56 `addu $v1,$v1,$a1`, but it adds the base pointer twice
(`addu $a3,$a0,$v0` / `addu $a1,$a0,$v1`) instead of once-plus-two-`addiu`, which is the
+1 instruction. Introducing a row-base intermediate to force the single base add
(b1-b6: `(s16 *)D_800A36A0 + t0*5` then `row + 0x35`, and four other spellings) returns
exactly the floor object 5/175 in all six cases. Banked
`rejected/s23-t0x5-row-index-two-base-adds-176insn-score9.c`. Note this also documents
that the shared `t0*4 -> t0*5 -> <<1` chain the frontier hoped to build is ALREADY present
in the floor body (floor rows 42/54/56/61 are byte-exact with the target); the frontier's
premise that the shift's value is "freshly born" was wrong.

### FACTS BANKED

- [s23] Floor re-verified live: candidate.c = 5/175/175; plain flip X = 29/175. FAKE
  ablation keep-all 5 / drop-1 10 (one unit, the prologue fence).
- [s23] Class C is SOLVED in pure C at 175 insns for the first time: any store-group
  source order that does not put group A first, on the flipped cursor base, emits the
  target's `addu $v1,$v1,$v0` with the target's seats. Best such build fCADS = 12/175.
- [s23] Exhaustive 24-permutation sweep of the four store groups on the flipped base:
  A-first is 29 in all six of its orders; C-first {12,13,16,17,22,38}; D-first
  {14,16,22,23,38,37}; S-first behaves as A-first.
- [s23] The `.lreg` reference/span counts of pseudos 107/108/109/110 are IDENTICAL in the
  29-object and the 12-object, so `qty_compare`'s ratio cannot be the selector; the
  selector is the insn-scan position tie-break (`qty_compare_1`, local-alloc.c:1681-1683).
  s22's "grow the merged quantity's span past 23 insns" instruction is retired.
- [s23] Stores are unmovable across each other in both scheduler passes, so source store
  order == emission store order; the target's emission fixes the target's source order at
  A-first, and 13 builds show no address-spelling substitute for moving the stores.
- [s23] The shared `t0*4 -> t0*5 -> t0*10` chain is already byte-exact in the floor body
  (rows 42/54/56/61); the standing frontier item's premise that the shift's value is
  freshly born ~3 insns before the add is false.
- [s23] Spelling the cursors as a 5-wide `s16` row index is a genuinely new object,
  9/176, whose only defect is two base additions instead of one.

- [s23] Floor re-verified live on the current chassis (commit 0f03be29): memory/grind/func_800770B8/candidate.c = score 5, build_insns 175, target_insns 175. Plain flipped base = 29/175.

- [s23] Mandated FAKE kill re-audit re-run this session: tools/fake_ablate.py reports ONE FAKE unit in the candidate (the empty do-while(0) prologue fence, line 206), keep-all 5 / drop-1 10 — load-bearing, prologue-scoped, occupying none of pseudos 107/108/109/110.

- [s23] CLASS C IS SOLVED AT 175 INSTRUCTIONS. tmp/grind/func_800770B8/s23/v/fCADS.c (flipped cursor base, store groups written C,A,D,S) = 12/175 and its rows 55-64 are byte-exact with asm/funcs/func_800770B8.s, including row 62 `addu $v1,$v1,$v0`, row 63 `addiu $a3,$v1,0x6A` and row 64 `addiu $a1,$v1,0x7E`. Banked rejected/s23-classC-SOLVED-flip-plus-C-group-first-175insn-score12.c.

- [s23] Exhaustive 24-permutation sweep of the outer loop's four store groups on the flipped base: fADCS 29, fADSC 29, fACDS 29, fACSD 29, fASDC 38, fASCD 29, fDACS 14, fDASC 16, fDCAS 22, fDCSA 23, fDSAC 38, fDSCA 37, fCADS 12, fCASD 13, fCDAS 16, fCDSA 17, fCSAD 22, fCSDA 38, fSADC 29, fSACD 29, fSDAC 38, fSDCA 37, fSCAD 35, fSCDA 38. Rule with no exception: A-first gives the correct tie with swapped seats (29); any order in which D or C precedes A gives the target's tie AND the target's seats.

- [s23] Two fresh .lreg dumps (s23/X.lreg vs s23/fCADS.lreg) show pseudos 107 (t0*5), 108 (t0*10), 109 (the D_800A36A0 reload) and 110 (the sum) with IDENTICAL refs and spans in the 29-object and the 12-object, differing only in the seats. qty_compare's ratio (local-alloc.c:1630-1657) is therefore equal in both and cannot be the selector; the selector is the equal-priority tie-break in qty_compare_1 (local-alloc.c:1681-1683). s22's 'grow the merged quantity's span past 23 insns' instruction is retired.

- [s23] The X dump confirms the flip's RTL directly: (insn 185 (set (reg:SI 110) (plus:SI (reg:SI 108) (reg:SI 109)))) with REG_DEAD on both operands — the shift really does occupy operand 1 under the flip, as s18 predicted.

- [s23] Stores cannot be scheduled across one another in sched1 or sched2 (unknown aliasing), so source store order IS emission store order; the target's emission pins its source order to A-first. 13 builds hoisting pointers, symbol addresses, index temporaries and single stores move the score from 29 to 20-25 but never reach the discriminator.

- [s23] The floor body's rows 40-61 are already byte-exact with the target, including the shared t0*4 -> t0*5 -> t0*10 chain at rows 42/54/56/61 — so the standing frontier item's premise that the shift's value is freshly born is false.

- [s23] A genuinely new object: spelling the cursors as a 5-wide s16 row index, `(s16 *)(D_800A36A0 + 0x6A) + (t0 * 5)`, gives 9/176 (a1, a3, a5, a6, a7 all identical). Its rows 40-56 are byte-exact including row 56 `addu $v1,$v1,$a1`; its only defect is two base additions instead of one base addition plus two addiu. Six row-base intermediates that force the single base add (b1-b6) all return the floor object 5/175. Banked rejected/s23-t0x5-row-index-two-base-adds-176insn-score9.c.

- [s23] 72 fresh builds this session; src/text1b.c restored to its pristine committed state after every sweep (git status clean at session end).

## [s24] synthesis — FLOOR 5 (re-verified on the moved chassis); the s23 frontier's item #1 is EXECUTED AND NEGATIVE, its shift-staging half is byte-inert, class B is re-foreclosed with four fresh spellings, and a WITHIN-FUNCTION CONTROL for class C is named for the first time

Chassis re-verification, first action. `tmp/grind/func_800770B8/s24/pristine_text1b.c`
byte-equals `git show HEAD:src/text1b.c` (HEAD = 6bbdefda, one commit newer than the
s23 ledger's 0f03be29). `memory/grind/func_800770B8/candidate.c` applied with the two
byte-neutral caller-side edits it documents (prototype `s32 func_800770B8(s32, s32,
s32);`, call site `(s32)&D_8009BD24`): **score 5, build_insns 175, target_insns 175**.
Plain flipped base X = **29/175**, unchanged. 31 fresh builds this session;
src/text1b.c restored to pristine after every sweep (`git status --porcelain` clean at
session end apart from the engine's own metrics/events.jsonl).

Mandated FAKE kill re-audit, re-run on THIS chassis:
`tools/fake_ablate.py --func func_800770B8 --file text1b --candidate memory/grind/func_800770B8/candidate.c`
reports **one** FAKE unit (the empty `do { } while (0);` prologue fence, line 243),
**keep-all 5 / drop-1 10** — unchanged from s23, still load-bearing, still
prologue-scoped, still occupying none of pseudos 107/108/109/110.

### THE FIVE RESIDUAL ROWS, PRINTED (s24/F.rows)

Everything else in the 175 rows differs only in assembler spelling (`move` vs
`addu $x,$y,$zero`, `li` vs `addiu $x,$zero,N`), which is byte-identical. The five
real rows are exactly:

    row 35   ours `sw $zero,48($s1)`   target `sw    $zero, 0x30($v0)`   class B
    row 36   ours `sh $zero,52($s1)`   target `sh    $zero, 0x34($v0)`   class B
    row 62   ours `addu $v0,$v0,$v1`   target `addu  $v1, $v1, $v0`      class C
    row 63   ours `addiu $a3,$v0,106`  target `addiu $a3, $v1, 0x6A`     class C
    row 64   ours `addiu $a1,$v0,126`  target `addiu $a1, $v1, 0x7E`     class C

### THE s23 FRONTIER'S ITEM #1 IS EXECUTED IN FULL AND IS NEGATIVE (20 builds)

The item asked for the reload's pseudo to be moved ahead of the chain's pseudo while
the stores stay in the target's A,D,C,S order. Both halves were built as a
five-position x two-spelling cross-product (`s24/gen_j.py`, bodies in `s24/v/`,
log `s24/sweep.log`).

**J = stage the reload** (`u8 *rb;` declared at the top of the outer-loop body,
`rb = D_800A36A0;` assigned at position k, cursors reading `rb`):

    position        base-first (jN)      flipped (jF)
    before group A  174 / 41             174 / 41
    after  group A  174 / 41             174 / 41
    after  group D  175 / 40             175 / 42
    after  group C  175 / 26             175 / 29
    after  group S  175 /  5  (== F)     175 / 29  (== X)

Two facts fall out. (a) The reload survives only if the assignment sits after group
**D** — group A's five `sh` stores through a `base`-derived pointer do NOT kill CSE of
the `D_800A36A0` gp-relative load, so positions 0 and 1 collapse `rb` onto `base` and
delete an instruction (174), the s18fx/s19 failure mode. (b) No surviving position
produces the target's seats; the best is 26, five times the floor. Banked
`rejected/s24-classC-staged-reload-after-groupC-175insn-score26.c`.

**M = stage the shift** (`s32 sh;` declared at the top of the loop body,
`sh = t0 * 10;` assigned at position k, cursors reading `sh`): **completely byte-inert
at every one of the five positions.** mF0-mF4 all measure 29/175, byte-identical to
the plain flip X; mN0-mN4 all measure 5/175, byte-identical to the floor F. GCC
re-sinks the `sll` to its use, so the chain pseudo's birth position is not
C-controllable by naming or by statement position. This is a stronger statement than
s23's single `i4` datum and it retires the whole "move the chain's pseudo" family.
Banked `rejected/s24-classC-named-t0x10-shift-local-inert-at-every-store-group-position.c`.

### CLASS B RE-FORECLOSED WITH FOUR NEW SPELLINGS (8 builds)

Class B is now precisely typed from the row diff: the target keeps **two** registers
holding the `func_8006E49C` result — `$s1` (the `p_old` copy; used for the
`sw $s1,%gp_rel(D_800A36A0)` global store and the `sw $v1,0x4($s1)` store, dead after
it) and `$v0` (the raw result pseudo; used for the 0x30 and 0x34 stores). Our floor
uses `$s1` for all four.

The obvious source shape for that split had never been built: name the raw result in
a fresh local `q`, assign `p_old = q` (the copy insn the target prints at its row 30),
and route only the 0x30/0x34 stores through `q`. The prediction was that
`combine_regs` cannot coalesce the copy because `q` is live past it, so both registers
survive at 175 insns. **Measured false in four spellings** (b1 `s32 *q`, b2 `u8 *q`,
b3 with the 0x4 store re-read through the global, b4 with the global assigned from
`q`): all four measure **170 insns / score 23**, and all four measure 170/47 when
composed with the cursor flip. The row diff of b1 (`s24/b1.rows`) shows why: `p_old`'s
SECOND live range is copy-propagated away before allocation, `$s1` disappears from the
entire post-call region, and the prologue re-seats `$s0/$s1/$s2/$s3` — the copy plus
four dependents vanish. This is exactly the failure s17 recorded ("every spelling that
actually reaches the raw pseudo collapses the function to 170 instructions"), now
re-confirmed on the current chassis with spellings the bank did not hold. Banked
`rejected/s24-classB-raw-plus-copy-both-live-collapses-170insn-score23.c`.

### ROUTING THE 0x68 STORE THROUGH THE SECOND RELOAD IS NOT A NON-FLIP ROUTE (2 builds)

`sN1`/`sF1`: assign `rb = D_800A36A0;` after group C and make group S's
`*(u8 *)(rb + t0 + 0x68) = (u8)t0;` store use it, so the reload's pseudo carries a use
that is not the cursor add. Both measure 175/29. The row diff (`s24/sN1.rows`) shows
the whole loop head reorganises — the `lw` is hoisted above group A's stores and the
`sb` sinks past the cursor block — and the add still prints reload-first
(`addu $v0,$v1,$v0`). Giving the reload an EARLIER use does not defeat the operand-1
tie; the `combine_regs` gate needs a use AFTER the add, and s18fx already priced that
at 174 insns / score 49.

### THE NEW LEAD: A WITHIN-FUNCTION CONTROL FOR CLASS C

Class C's divergent insn is the target's row 65 `addu $v1,$v1,$v0` (shift as operand
1). **The same function contains a SECOND instance of that exact shape that our floor
body already matches byte-exactly**: target row 89 `addu $v1,$v1,$v0`, the base for
the post-inner-loop `sh $zero,0x5C($v1)` / `sh $v0,0x60($v1)` stores, fed by
`sll $v1,$v1,1` (t0*2) at row 87 and `lw $v0,%gp_rel(D_800A36A0)` at row 88. Our
source for it is the plainly BASE-FIRST spelling
`*(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;` — and it emits shift-first, matching.

So "a base-first source spelling forces reload-as-operand-1" is FALSE as a general
statement: it holds at the cursor site and fails at the 0x5C site, in the same
function, in the same compilation. The ledger has never named this control. The two
sites differ in exactly two respects: (i) at the 0x5C site the sum is consumed
directly as a MEM base with two constant offsets, while at the cursor site the sum is
materialised into two pointer variables via `addiu +0x6A` / `+0x7E` because those
pointers are live into the inner loop; and (ii) at the 0x5C site the addend is
`t0 * 2`, whose RTL def is a single `sll` of a plain sign-extended register, while at
the cursor site the addend is `t0 * 10`, whose def is an `sll` of a `plus`
(`t0*4 + t0`). Whatever distinguishes them is a C-reachable property, and it is the
strongest remaining lead on class C because it needs neither the store-group reorder
nor the flip.

### FACTS BANKED

- [s24] Floor re-verified live on the moved chassis (HEAD 6bbdefda): candidate.c = score 5, build_insns 175, target_insns 175; plain flip X = 29/175. FAKE ablation keep-all 5 / drop-1 10, one unit (the prologue fence).
- [s24] The five residual rows printed exactly: rows 35/36 (`sw`/`sh` through `$s1` where the target uses `$v0`) = class B; rows 62/63/64 (`addu $v0,$v0,$v1` plus two `addiu` off `$v0` where the target uses `$v1`) = class C. Every other row differs only in assembler pseudo-op spelling and is byte-identical.
- [s24] The s23 frontier item #1 ("move the reload's pseudo earlier while the stores stay A,D,C,S") is EXECUTED as a 5-position x 2-spelling cross-product and is NEGATIVE: 174/41 at the two positions before group D (the reload CSEs onto `base` and is deleted), 175/40-42 after D, 175/26 base-first and 175/29 flipped after C, no-op after S. No position reaches the target's seats and none beats the floor.
- [s24] Group A's five `sh` stores through a `base`-derived pointer do NOT kill CSE of the gp-relative `D_800A36A0` load: a second named read of the global placed after group A still collapses onto `base` and deletes an instruction. The earliest position at which a second reload survives is after group D.
- [s24] Staging the cursor's `t0 * 10` shift into a named `s32 sh;` local is BYTE-INERT at all five statement positions across the store groups: mF0-mF4 are byte-identical to the plain flip (29/175) and mN0-mN4 are byte-identical to the floor (5/175). GCC re-sinks the `sll` to its use, so the chain pseudo's birth position is not C-controllable by naming or statement position.
- [s24] Class B is exactly: the target holds the `func_8006E49C` result in TWO registers, `$s1` (the `p_old` copy, feeding the global store and the 0x4 store, dead immediately after) and `$v0` (the raw result, feeding the 0x30/0x34 stores). Four fresh spellings that name the raw result in a local `q` and keep the `p_old = q` copy live all collapse to 170 insns / score 23 (170/47 under the flip); b1's row diff shows `p_old`'s second live range copy-propagated away, `$s1` gone from the post-call region and the prologue re-seated. Re-confirms s17's foreclosure on the current chassis with spellings the bank did not hold.
- [s24] Giving the second `D_800A36A0` reload an earlier use (routing group S's 0x68 byte store through it) does not defeat the operand-1 tie: 175/29 both base-first and flipped, the add still prints reload-first, and the loop head reorganises (lw hoisted above group A, sb sunk past the cursor block). The `combine_regs` gate needs a use AFTER the add, which s18fx priced at 174/49.
- [s24] WITHIN-FUNCTION CONTROL, named for the first time: the target's row 89 `addu $v1,$v1,$v0` (post-inner-loop, base for the 0x5C/0x60 stores) is the SAME shift-first shape as class C's row 65, and our floor body matches it byte-exactly from a plainly base-first source spelling `*(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;`. "Base-first spelling forces reload-as-operand-1" is therefore false in general; it holds at the cursor site and fails at the 0x5C site in the same compilation. The two sites differ in (i) sum-consumed-as-MEM-base vs sum-materialised-into-two live pointer variables and (ii) addend `t0*2` (an `sll` of a plain reg) vs `t0*10` (an `sll` of a `plus`).

- [s24] Floor re-verified live on the moved chassis (HEAD 6bbdefda, one commit newer than the s23 ledger entry): memory/grind/func_800770B8/candidate.c = score 5, build_insns 175, target_insns 175. Plain flipped base X = 29/175.

- [s24] Mandated FAKE kill re-audit re-run this session: tools/fake_ablate.py reports ONE FAKE unit (the empty do-while(0) prologue fence, line 243), keep-all 5 / drop-1 10 -- unchanged, load-bearing, prologue-scoped, occupying none of pseudos 107/108/109/110.

- [s24] The five residual rows are now printed exactly (s24/F.rows): rows 35/36 `sw $zero,0x30($s1)` / `sh $zero,0x34($s1)` where the target uses $v0 (class B), and rows 62/63/64 `addu $v0,$v0,$v1` plus two addiu off $v0 where the target uses $v1 (class C). Every other row differs only in assembler pseudo-op spelling (move vs addu, li vs addiu) and is byte-identical.

- [s24] s23 frontier item #1 executed as a 5-position x 2-spelling cross-product (20 builds). Staging the reload: 174/41 at the two positions before group D, 175/40 and 175/42 after D, 175/26 and 175/29 after C, no-op after S. No position reaches the target's seats; best surviving score 26.

- [s24] Group A's five sh stores through a base-derived pointer do NOT kill CSE of the gp-relative D_800A36A0 load: a second named read placed after group A still collapses onto `base` and deletes an instruction (174). The earliest position at which a second reload survives is after group D.

- [s24] Staging the cursor's t0*10 shift into a named `s32 sh;` local is BYTE-INERT at all five statement positions: mF0-mF4 all exactly 29/175 (byte-identical to the plain flip) and mN0-mN4 all exactly 5/175 (byte-identical to the floor). GCC re-sinks the sll to its use, so the chain pseudo's birth position is not C-controllable by naming or statement position. This retires the 'move the chain's pseudo' family, not just s23's single i4 datum.

- [s24] Class B is now exactly typed: the target holds the func_8006E49C result in TWO registers, $s1 (the p_old copy, feeding the global store and the 0x4 store, dead immediately after) and $v0 (the raw result, feeding the 0x30/0x34 stores). Four fresh spellings that name the raw result in a local q and keep the `p_old = q` copy live all collapse to 170 insns / score 23 (170/47 under the flip).

- [s24] The b1 row diff (s24/b1.rows) names the class-B collapse mechanism concretely: p_old's second live range is copy-propagated away before allocation, $s1 disappears from the entire post-call region and the prologue re-seats $s0/$s1/$s2/$s3 -- the copy plus four dependents vanish. Re-confirms s17's foreclosure on the current chassis.

- [s24] Routing group S's 0x68 byte store through the second reload (sN1/sF1) is 175/29 in both spellings; the add still prints reload-first and the loop head reorganises (lw hoisted above group A, sb sunk past the cursor block). An earlier use does not clear the reload's REG_DEAD note at the add.

- [s24] WITHIN-FUNCTION CONTROL named for the first time in 24 sessions: target row 89 `addu $v1,$v1,$v0` (post-inner-loop, the base for the 0x5C/0x60 stores) is the same shift-first shape as class C's row 65, and the floor body matches it byte-exactly from the base-first source `*(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;`.

- [s24] 31 fresh builds this session; src/text1b.c restored to its pristine committed state after every sweep (git status --porcelain clean at session end apart from the engine's own metrics/events.jsonl).

## [s25] synthesis — FLOOR 5 (re-verified); BOTH standing frontier items EXECUTED AND NEGATIVE; the class-C SEAT decision is read directly out of local-alloc.c and reduced to a NUMERIC criterion for the first time in 25 sessions

Chassis re-verification, first action. `tmp/grind/func_800770B8/s25/pristine_text1b.c`
byte-equals `git show HEAD:src/text1b.c`. `memory/grind/func_800770B8/candidate.c` applied
with the two byte-neutral caller-side edits it documents (prototype
`s32 func_800770B8(s32, s32, s32);`, call site `(s32)&D_8009BD24`): **score 5,
build_insns 175, target_insns 175**. Plain flipped base X = **29/175**, unchanged.
16 fresh builds + 3 fresh `.lreg` dumps this session. Scratch:
`tmp/grind/func_800770B8/s25/`. src/text1b.c restored to pristine after every sweep.

Mandated FAKE kill re-audit, re-run on THIS chassis:
`tools/fake_ablate.py --func func_800770B8 --file text1b --candidate memory/grind/func_800770B8/candidate.c`
reports **one** FAKE unit (the empty `do { } while (0);` prologue fence, line 243),
**keep-all 5 / drop-1 10** — unchanged from s23/s24, still load-bearing, still
prologue-scoped. The closest-to-target instance kill (s23's fCADS, 12/175) was
re-measured live this session and re-confirmed at 12/175 with rows 55-64 byte-exact.

### THE TIE IS NOW READ OUT OF THE COMPILER SOURCE, NOT INFERRED

`local-alloc.c:1240-1299` (block_alloc's tying loop) walks the insn's operands
`for (i = 1; i < insn_n_operands; i++)`, calls `combine_regs (r1, r0, ...)` on each,
and does `if (win) break;`. So the destination of `(set (reg S) (plus (reg A) (reg B)))`
ties to **RTL operand 1 — the operand the C source names first** — whenever
`combine_regs` accepts it. `combine_regs` (local-alloc.c:1784-1946) refuses in exactly
three C-reachable ways:
  1. `reg_qty[ureg] < 0` — the used pseudo is not local-alloc-eligible. `local_alloc`
     (local-alloc.c:470-477) sets `reg_qty[i] = -1` for any pseudo with
     `reg_basic_block[i] < 0` (referenced in more than one basic block) **or**
     `reg_n_deaths[i] != 1`.
  2. no `REG_DEAD` note for the used pseudo at this insn — i.e. it is used again later.
  3. `reg_qty[sreg] >= -1` — the destination pseudo already carries a quantity.
Any of the three makes the loop fall through to operand 2, so the dest ties to the
SECOND-named operand. This is the mechanism behind s21's empirical "the operand a body
names first is the one the sum ties to", and it names the escape hatches for the first
time.

### ESCAPE HATCH 1 MEASURED: THE TIE IS DECOUPLABLE FROM SOURCE OPERAND ORDER (4 builds)

`z1`/`z2` make the cursor's `D_800A36A0` reload non-local by giving it a reference after
the inner loop (the post-loop `0x5C`/`0x60` stores), so `reg_basic_block < 0`,
`reg_qty = -1`, and `combine_regs` skips operand 1:

    z1n (both post-loop stores through the reload, BASE-first cursor)   174 / 49
    z1f (same, FLIPPED cursor)                                          174 / 49
    z2n (only the 0x5C store through it, BASE-first cursor)             176 / 42
    z2f (same, FLIPPED cursor)                                          176 / 42

The base-first and flipped spellings measure **byte-identically** in both pairs — proof
that once the reload is non-local the source operand order stops mattering, exactly as
the source predicts. `s25/z2n` rows 60-62 print
`lw $t0,0($gp)` / `sll $v0,$v0,1` / `addu $v0,$t0,$v0`: the tie HAS moved onto the shift
from a plainly base-first source. The price is that the reload leaves local-alloc
entirely and global-alloc seats it in `$t0` instead of `$v0`, and the shared-reload
spelling deletes the post-loop `lw` (174). Banked
`rejected/s25-classC-nonlocal-reload-tie-flips-reload-ejected-to-t0-176insn-score42.c`
and `rejected/s25-classC-nonlocal-reload-shared-with-0x5C-site-174insn-score49.c`.

### THE SEAT DECISION, QUANTIFIED FROM THREE .lreg DUMPS

Three fresh dumps this session (`s25/F.lreg`, `s25/X.lreg`, `s25/fCADS.lreg`, via
`pwsh tools/grinder/dump.ps1 func_800770B8`). F and X have **byte-identical pseudo
tables** — every `Register N used R times across S insns` line agrees — and differ only
in the allocation result:

    pseudo                      F (floor)   X (flip)   fCADS (flip + C-first)
    chain root  t0*4            in 3        in 2       (renumbered 89) in 3
    107  t0*5                   in 3        in 2       in 3
    108  t0*10                  in 3        in 2       in 3
    109  D_800A36A0 reload      in 2        in 3       in 2   <- target
    110  the cursor sum         in 2        in 2       in 3   <- target

so the rule is: **the quantity the sum merges into takes `$2`, except in fCADS.** The
merge follows the tie (F: sum+reload; X and fCADS: sum+chain). What separates X from
fCADS is `qty_compare_1`'s priority `floor_log2(refs)*refs*size/(death-birth)`
(local-alloc.c:1660-1683) evaluated on the merged quantity Q = {chain root, 107, 108, 110}:
refs 8+4+4+6 = **22** in both, so `floor_log2(22)*22 = 88`, and the only free variable is
Q's live span. In X the chain root's own span is 10 insns; in fCADS the C-group store move
stretches it to **18** (`Register 89 used 8 times across 18 insns` vs
`Register 93 used 8 times across 10 insns`). The blocking quantity is the short
4-refs/2-insn pseudo in the same window, priority `floor_log2(4)*4/2 = 4.0`:

    X      Q span ~14  ->  88/14 = 6.3  >  4.0  ->  Q allocated first, takes $2   (wrong seat)
    fCADS  Q span ~23  ->  88/23 = 3.8  <  4.0  ->  the short qty takes $2 first,
                                                     Q is pushed to $3 and the reload
                                                     then finds $2 free   (TARGET seats)

This replaces s23's "the selector is the qty-number tie-break" with a numeric criterion
that predicts all three measured builds. **Class C closes on the target's A-first store
order iff the merged chain+sum quantity's live span exceeds ~22 insns** (equivalently: the
chain-root pseudo's span must grow from 10 to ~18) while the sum stays tied to the shift.
Lowering Q's reference count is not an alternative route: dropping the chain root from Q
leaves 14 refs but shrinks the span proportionally (42/8 = 5.3), still above 4.0.

### BOTH STANDING FRONTIER ITEMS EXECUTED AND NEGATIVE (12 builds)

**Frontier #1 — "spell the cursor site like the 0x5C control site, sum consumed as a MEM
base rather than materialised into two live pointers".** `p1n`/`p1f` write the inner loop
as `*(s16 *)(rowp + (a2*2) + 0x6A) = -1; *(s16 *)(rowp + (a2*2) + 0x7E) = 0;` off a single
`rowp`: **172 insns / score 46** in both spellings. The form deletes three instructions
(one of the two `addiu` cursor pointers and one inner-loop `addu`) and the target keeps
them, so the shape is not the target's however it seats. The frontier's premise is also
now known to be wrong on its own terms: the target's cursor sum has exactly two register
uses (rows 66/67 `addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E`), i.e. the SAME consumption
shape our floor already emits — the two sites differ, but our cursor site already matches
the target's cursor site in that respect. Banked
`rejected/s25-classC-cursor-sum-as-mem-base-172insn-score46.c`.

**Frontier #2 — "respell the cursor's addend so its RTL def is an sll of a plain
sign-extended register instead of an sll of a plus".** Falsified twice over. By
inspection: the floor's own `sll $v1,$v1,1` at row 61 IS an `(ashift (reg) (const_int 1))`
of the plain t0*5 pseudo, structurally identical to the 0x5C site's `sll $v1,$v1,1` at
row 87 — the two defs never differed. By measurement: `n5n` (`s16 t5 = (s16)(t0*5);` then
`t5 * 2`) = **176 / 7**, `n5f` = **176 / 30**; the s16 round trip costs one instruction and
the tie does not move. Banked
`rejected/s25-classC-s16-row-index-addend-176insn-score7.c`.

### THE NAMED-OFFSET FAMILY IS NOW DEAD FOR t0*2 AND t0*4 AS WELL AS t0*10 (8 builds)

s24 retired "move the chain's pseudo by naming it" for the `t0*10` shift. This session
extends it to the two earlier chain members. Hoisting `s32 o2 = t0 * 2;` and/or
`s32 o4 = t0 * 4;` to the top of the outer-loop body (before the `D_800A36A0` read) and
routing groups A/C/D through them is **byte-inert in all eight builds**: `y1f`-`y4f` all
measure exactly 29/175 (byte-identical to the plain flip X) and `y1n`-`y4n` all measure
exactly 5/175 (byte-identical to the floor F), in both declaration orders. GCC re-sinks
every one of the three `sll`s to its use, so no chain member's birth position is
C-controllable by naming or statement position. Banked
`rejected/s25-classC-named-offset-locals-o2-o4-byte-inert-A-first.c`.

### FACTS BANKED

- [s25] Floor re-verified live on the current chassis (HEAD 35957733): memory/grind/func_800770B8/candidate.c = score 5, build_insns 175, target_insns 175; plain flip X = 29/175. Mandated FAKE re-audit re-run: one unit (the prologue fence), keep-all 5 / drop-1 10, unchanged.
- [s25] Mandated kill re-audit on the closest-to-target banked form: s23's fCADS (flip + C,A,D,S store order) re-measured live at 12/175 on this chassis, rows 55-64 still byte-exact including the class-C `addu $v1,$v1,$v0`.
- [s25] The class-C TIE is now read directly out of the compiler: block_alloc's tying loop (local-alloc.c:1240-1299) walks operands 1..n with `if (win) break;`, so the sum's dest ties to RTL operand 1 — the operand the C source names first — unless combine_regs (local-alloc.c:1784-1946) refuses. It refuses when (a) reg_qty[used] < 0, which local_alloc (local-alloc.c:470-477) sets for reg_basic_block < 0 or reg_n_deaths != 1; (b) the used pseudo carries no REG_DEAD note at the insn; (c) the dest pseudo already has a quantity.
- [s25] Escape hatch (a) MEASURED and CONFIRMED: making the cursor's D_800A36A0 reload non-local (a reference after the inner loop) flips the tie onto the shift FROM A BASE-FIRST SOURCE. z1n/z1f = 174/49 and z2n/z2f = 176/42, with base-first and flipped spellings byte-identical in each pair — once the reload is non-local, source operand order stops mattering. z2n rows 60-62 print `lw $t0` / `sll $v0,$v0,1` / `addu $v0,$t0,$v0`. Price: the reload leaves local-alloc and global-alloc seats it in $t0, not $v0.
- [s25] Three fresh .lreg dumps (s25/F.lreg, s25/X.lreg, s25/fCADS.lreg): F and X carry BYTE-IDENTICAL pseudo tables and differ only in the allocation result, so the seat swap is purely an allocation-order effect of which quantity the sum merges into.
- [s25] The seat decision is now numeric. The merged chain+sum quantity Q = {chain root, t0*5, t0*10, sum} has 22 references in every build, so floor_log2(22)*22 = 88 and only Q's span varies; the blocking short quantity (4 refs / 2 insns) has priority 4.0. X: chain-root span 10, Q span ~14, priority 6.3 > 4.0 -> Q takes $2 (wrong seat). fCADS: chain-root span 18 (Register 89 used 8 times across 18 insns), Q span ~23, priority 3.8 < 4.0 -> the short quantity takes $2, Q is pushed to $3 and the reload finds $2 = the target's seats. This supersedes s23's "the selector is qty_compare_1's qty-number tie-break".
- [s25] Reducing Q's reference count is NOT an alternative to lengthening its span: dropping the chain root leaves 14 refs but the span shrinks proportionally (42/8 = 5.3), still above the blocking 4.0.
- [s25] Frontier item #1 (cursor sum consumed as a MEM base rather than two live pointers) EXECUTED: p1n/p1f = 172 insns / score 46 in both spellings; the form deletes three instructions the target has. Its premise is independently false — the target's cursor sum has exactly two register uses (the two addiu at target rows 66/67), the same consumption shape our floor already emits.
- [s25] Frontier item #2 (respell the cursor addend as an sll of a plain sign-extended register) EXECUTED and falsified twice: by inspection the floor's row-61 `sll $v1,$v1,1` is already an ashift of a plain register, structurally identical to the 0x5C control site's row-87 `sll $v1,$v1,1`; by measurement n5n = 176/7 and n5f = 176/30 (the s16 round trip costs an instruction and the tie does not move).
- [s25] The named-offset family is dead for the whole chain, not just t0*10: hoisting `s32 o2 = t0*2;` and/or `s32 o4 = t0*4;` above the D_800A36A0 read and routing groups A/C/D through them is byte-inert in all eight builds (y1f-y4f exactly 29/175, y1n-y4n exactly 5/175, both declaration orders). GCC re-sinks every chain sll to its use.
- [s25] 16 fresh builds and 3 fresh .lreg dumps this session; src/text1b.c restored to its pristine committed state after every sweep.

- [s25] Floor re-verified live on the current chassis (HEAD 35957733): memory/grind/func_800770B8/candidate.c = score 5, build_insns 175, target_insns 175; plain flipped base X = 29/175.

- [s25] Mandated FAKE kill re-audit re-run this session: tools/fake_ablate.py reports ONE FAKE unit (the empty do-while(0) prologue fence, line 243), keep-all 5 / drop-1 10 — unchanged from s23/s24, load-bearing, prologue-scoped.

- [s25] Mandated kill re-audit on the closest-to-target banked form: s23's fCADS (flip + C,A,D,S store order) re-measured live at 12/175 on this chassis, rows 55-64 still byte-exact including the class-C `addu $v1,$v1,$v0`.

- [s25] The class-C TIE is now read directly out of the compiler rather than inferred: block_alloc's tying loop (local-alloc.c:1240-1299) walks operands 1..n with `if (win) break;`, so the sum's dest ties to RTL operand 1 — the operand the C source names first — unless combine_regs (local-alloc.c:1784-1946) refuses. It refuses when (a) reg_qty[used] < 0, which local_alloc (local-alloc.c:470-477) sets for reg_basic_block < 0 or reg_n_deaths != 1; (b) the used pseudo carries no REG_DEAD note at the insn; (c) the dest pseudo already has a quantity.

- [s25] Escape hatch (a) MEASURED: making the cursor's D_800A36A0 reload non-local flips the tie onto the shift FROM A BASE-FIRST SOURCE (z2n rows 60-62 `lw $t0` / `sll $v0,$v0,1` / `addu $v0,$t0,$v0`), and base-first and flipped spellings then measure byte-identically (z1n = z1f = 174/49, z2n = z2f = 176/42). Price: global-alloc seats the ejected reload in $t0, not $v0.

- [s25] Three fresh .lreg dumps (s25/F.lreg, s25/X.lreg, s25/fCADS.lreg): F and X carry BYTE-IDENTICAL pseudo tables and differ only in the allocation result, so the seat swap is purely an allocation-order effect.

- [s25] The seat decision is now numeric: Q = {chain root, t0*5, t0*10, sum} always has 22 refs, so floor_log2(22)*22 = 88 is fixed and only Q's span varies against the blocking short 4-refs/2-insn quantity at priority 4.0. X: chain-root span 10, Q ~14, 6.3 > 4.0 -> Q takes $2 (wrong seat). fCADS: chain-root span 18, Q ~23, 3.8 < 4.0 -> the short quantity takes $2 and Q is pushed to $3 with the reload in $2 (the target's seats). Class C therefore closes on the target's A-first store order iff Q's span exceeds ~22 insns while the sum stays tied to the shift.

- [s25] Reducing Q's reference count is NOT an alternative to lengthening its span: dropping the chain root leaves 14 refs but the span shrinks proportionally (42/8 = 5.3), still above the blocking 4.0.

- [s25] Standing frontier item #1 EXECUTED and negative: consuming the cursor sum as a MEM base (p1n/p1f) = 172 insns / score 46 in both spellings, deleting three instructions the target has; and its premise is independently false — the target's cursor sum has exactly two register uses (target rows 66/67), the same shape our floor emits.

- [s25] Standing frontier item #2 EXECUTED and negative: the floor's row-61 `sll $v1,$v1,1` is already an ashift of a plain register, structurally identical to the 0x5C control site's row-87 sll, so there was no def-shape difference to respell; the s16 row-index spelling measures 176/7 (base-first) and 176/30 (flipped).

- [s25] The named-offset family is dead for the whole chain, not just t0*10: hoisting `s32 o2 = t0*2;` and/or `s32 o4 = t0*4;` above the D_800A36A0 read and routing groups A/C/D through them is byte-inert in all eight builds (y1f-y4f exactly 29/175, y1n-y4n exactly 5/175, both declaration orders).

- [s25] 16 fresh builds and 3 fresh .lreg dumps this session; src/text1b.c restored to its pristine committed state after every sweep (git status clean apart from the ledger edits and the engine's own metrics/events.jsonl).

## [s26] synthesis — FLOOR 5 (re-verified); the class-C seat decision is READ OUT OF THE COMPILER'S OWN qty TABLE for the first time, and s25's span-threshold model is FALSIFIED and replaced by an exact OVERLAP criterion

Chassis re-verification, first action. `git show HEAD:src/text1b.c` byte-equals the working
tree and byte-equals `tmp/grind/func_800770B8/s25/pristine_text1b.c`, so the chassis is
unchanged from s25 (HEAD is d8215bb2; s24/s25 commits are ledger-only). `F` (=
`memory/grind/func_800770B8/candidate.c` body) re-measured live: **score 5, build_insns 175,
target_insns 175**. 6 fresh scoring builds + 5 fresh instrumented-cc1 runs + 1 fresh `-da`
dump set this session. Scratch: `tmp/grind/func_800770B8/s26/`. src/text1b.c restored to
pristine after every sweep.

### THE INSTRUMENT: BB2_QTY_DEBUG / BB2_SUGG_DEBUG PRINT qty_birth / qty_death / qty_n_refs EXACTLY

This closes s25's frontier item #3 ("turn the span estimate into an exact threshold") on the
first probe. The instrumented cc1 at `tools/gcc-2.7.2/cc1` already carries two print-only
hooks in `local-alloc.c` that no prior session on this function used:

  * `BB2_SUGG_DEBUG` (`local-alloc.c:1437-1460`) dumps, for every quantity of every block,
    `func= blk= qty= reg1= birth= death= refs= size= mode= minclass= altclass= calls=
    chgsize= ncopysugg= nsugg=` — i.e. the COMPLETE `qty_compare_1` input vector, taken
    before the suggested-register pass can perturb it.
  * `BB2_QTY_DEBUG` (`local-alloc.c:1585`) dumps one line per quantity in `qty_order`
    ORDER — `blk= ord= qty= reg1= birth= death= refs= got=` — i.e. the allocation sequence
    and the hard register each quantity actually received.

Runner: `tmp/grind/func_800770B8/s26/qtydbg.py` (applies a variant, runs the project's exact
`cpp | cc1` front half from `engine.buildconfig` with both env vars set, keeps the
`func_800770B8` slice by tracking the `func=` field of the interleaved SUGGDBG lines) driven
by `s26/qty.sh`. One run per variant, no scoring needed. Outputs `s26/<name>.qty`.
**Any later session can now test a class-C hypothesis without spending a score.**

### THE MEASURED qty TABLES — BLOCK 1 (the outer-loop body)

    F (floor, base-first cursor, A,D,C,S stores)   score 5 / 175
      ord=0  qty2 reg1=89   birth 12  death 14  refs  4  -> $2
      ord=1  qty4 reg1=110  birth 48  death 56  refs 10  -> $2     (sum MERGED WITH RELOAD)
      ord=2  qty3 reg1=108  birth 28  death 52  refs 16  -> $3     (chain, no sum)
      ord=3  qty1 reg1=100  birth 10  death 44  refs 12  -> $4
      ord=4  qty0 reg1=86   birth  6  death 46  refs 14  -> $5

    X (plain flip, A,D,C,S stores)                 score 29 / 175
      ord=0  qty2 reg1=89   birth 12  death 14  refs  4  -> $2
      ord=1  qty3 reg1=110  birth 28  death 56  refs 22  -> $2     (chain+sum MERGED)
      ord=2  qty4 reg1=109  birth 48  death 52  refs  4  -> $3     (reload alone)
      ord=3  qty1 reg1=100  birth 10  death 44  refs 12  -> $3
      ord=4  qty0 reg1=86   birth  6  death 46  refs 14  -> $4

    fCADS (flip + C,A,D,S stores)                  score 12 / 175, rows 55-64 byte-exact
      ord=0  qty3 reg1=93   birth 20  death 22  refs  4  -> $2
      ord=1  qty2 reg1=110  birth 12  death 56  refs 22  -> $3     (chain+sum MERGED)
      ord=2  qty4 reg1=109  birth 48  death 52  refs  4  -> $2     (reload alone) TARGET
      ord=3  qty1 reg1=100  birth 10  death 44  refs 12  -> $4
      ord=4  qty0 reg1=86   birth  6  death 46  refs 14  -> $5

### s25's SPAN-THRESHOLD MODEL IS FALSIFIED; THE REAL CRITERION IS AN OVERLAP TEST

s25 inferred from post-sched2 row numbering that X's merged quantity had span ~14 (priority
88/14 = 6.3) and that class C would close once that span passed ~22. The compiler's own
numbers say otherwise: **X's merged quantity already spans 28** (birth 28, death 56), its
priority is 88/28 = 3.14, it is **already ordered SECOND** behind the short 4-ref quantity
(priority `floor_log2(4)*4*1/2` = 4.0) — exactly the ordering s25 said we still had to buy.
X nevertheless loses the seat.

The actual discriminator is *interval containment*, not priority:

  * X: the short quantity occupies `[12,14]` and the merged chain occupies `[28,56]`. They
    are DISJOINT, so when the chain is allocated (ord=1) `$2` is free again and it takes it.
    The reload `[48,52]` is then allocated third, conflicts with the chain, and gets `$3`.
  * fCADS: the merged chain occupies `[12,56]` and CONTAINS the short quantity's `[20,22]`.
    `$2` is therefore still busy when the chain is allocated, the chain falls to `$3`, and
    the reload `[48,52]` — which starts long after the short quantity died at 22 — finds
    `$2` free. Those are the target's seats.

So the class-C seat condition, stated exactly and for the first time, is:

> On a flipped-cursor body the target's seats appear iff (i) the merged 22-ref chain+sum
> quantity is ordered after some short high-priority quantity (satisfied whenever its span
> exceeds 22, already true in every A-first build measured), AND (ii) that short quantity's
> `[birth,death]` lies strictly INSIDE the chain quantity's `[birth,death]`, AND (iii) the
> short quantity is dead before the reload's birth (48).

Equivalently, in terms of C: the chain root `t0*4` must be BORN (post-sched1) ahead of some
4-ref throwaway quantity that itself dies before the cursor's `D_800A36A0` reload. In X the
chain root is born at 12 but is NOT merged into the sum's quantity — the merged quantity
starts at 28, at the `t0*5` add — whereas in fCADS the chain root's death coincides with the
`t0*5` add, the tie fires there, and the merged quantity inherits birth 12.

### THE RTL AT THE TWO ADD SITES IS IDENTICAL — THE "SOURCE NAMES IT FIRST" MODEL IS WRONG

s21/s25 modelled the class-C tie as "the sum ties to RTL operand 1 = the operand the C
source names first". A fresh `-da` dump of the floor body F
(`tmp/grind/func_800770B8/dumps/text1b.lreg`, via `pwsh tools/grinder/dump.ps1
func_800770B8`) shows that is not what the RTL contains. The cursor site and the 0x5C
control site — one of which matches the target and one of which does not — carry
**structurally identical** plus insns, both with the SHIFT as operand 1 and both with
REG_DEAD on both operands, even though both are spelled base-first in the source:

    (insn 185 ... (set (reg:SI 110) (plus:SI (reg:SI 108) (reg:SI 109))))   ; cursor site
        (expr_list:REG_DEAD (reg:SI 108) (expr_list:REG_DEAD (reg:SI 109) (nil)))
    (insn 263 ... (set (reg:SI 140) (plus:SI (reg:SI 139) (reg:SI 137))))   ; 0x5C site
        (expr_list:REG_DEAD (reg:SI 139) (expr_list:REG_DEAD (reg:SI 137) (nil)))

(108/139 = the shift, 109/137 = the `D_800A36A0` reload.) Source operand order does not
survive into RTL operand order at either site — the shift is operand 1 in both. What differs
between the two sites is the QUANTITY STATE that `combine_regs` (local-alloc.c:1784-1946)
sees when it is asked to tie the destination, which is exactly what the qty tables above
measure. Any future session must attack the qty intervals, not the source operand order.

### PROBE: NAMED C-/D-GROUP POINTER HOISTS DO NOT MOVE THE CHAIN QUANTITY'S BIRTH (6 builds + 4 qty runs)

The obvious ordinary-C way to buy condition (ii) on the target's A-first store order is to
hoist the `t0*4`-consuming pointer computations into named locals at the top of the
outer-loop body while leaving every STORE in the target's A,D,C,S order — a pointer local,
no reordered store, no added value. Three shapes, on both cursor spellings:

    d1  `u8 *pd = (u8 *)&D_800A35D0 + (t0 * 4);` hoisted, D stores in place
    d2  `u8 *pc = base + (t0 * 4);`              hoisted, C stores in place
    d3  both

    d1f 40/175   d2f 25/175   d3f 20/175      (flipped cursor; plain flip X = 29)
    d1n 35/175   d2n  5/175   d3n 10/175      (base-first cursor; floor F = 5)

All six keep 175 insns. `d2n` is **byte-identical to the floor** — naming the C-group
pointer is a free spelling on the base-first cursor. But the qty tables show none of them
buys the criterion: the merged chain quantity's birth stays at 26-28 in every flipped
variant, and the hoist merely splits a NEW 6-ref quantity off the named pointer:

    d1f  short [12,14] refs 4 -> $2 ; chain [26,54] refs 22 -> $2 ; new [30,34] refs 6 -> $3
    d2f  short [12,14] refs 4 -> $2 ; chain [28,56] refs 22 -> $2 ; new [36,40] refs 6 -> $3
    d3f  short [12,24] refs 16 -> $2 ; chain [26,54] refs 22 -> $2 ; two new 6-ref qtys -> $3

In d3f the short quantity even grows to 16 refs and `[12,24]`, and the chain at `[26,54]`
still starts two units after it dies. **Hoisting the pointer into a local moves the pointer,
not the shift**: the chain root's birth is set by where sched1 places the `sll`, and sched1
sinks it to its first consumer, which on an A-first store order is always after group A's
insns. Banked
`rejected/s26-classC-named-CD-pointer-hoist-does-not-move-chain-qty-birth-175insn-score20.c`
and `rejected/s26-classC-named-C-pointer-hoist-byte-neutral-on-base-first-175insn-score5.c`.

### fCADS's 12 POINTS ARE ENTIRELY THE STORE-GROUP BLOCK (rows 40-54)

Full row diff of fCADS (`s26/rowdiff.sh fCADS`, cosmetic `move`/`addu $zero` and
`li`/`addiu $zero` disassembly aliases discounted): the only genuine differences are the
long-known rows 35-36 (the `$s1` vs `$v0` store base) and the contiguous block **rows
40-54**, i.e. the A/D/C pointer computations and their stores, reordered. Rows 55-64 —
including the class-C `addu $v1,$v1,$v0` — are byte-exact, re-confirming s23/s25. So the
price of the C-first order is localised to one 15-row window and nothing leaks past row 54.

### FACTS BANKED

- [s26] Chassis unchanged from s25 (HEAD d8215bb2, working tree byte-equals `git show HEAD:src/text1b.c`); floor re-verified live at score 5 / build_insns 175 / target_insns 175 with `memory/grind/func_800770B8/candidate.c` applied plus its two documented byte-neutral caller-side edits.
- [s26] NEW INSTRUMENT, transferable to every RA question in this project: the instrumented cc1 (`tools/gcc-2.7.2/cc1`) has env-gated `BB2_SUGG_DEBUG` (local-alloc.c:1437) and `BB2_QTY_DEBUG` (local-alloc.c:1585) hooks that print, per block, every quantity's `qty_birth / qty_death / qty_n_refs / qty_min_class / suggestions` AND the `qty_order` allocation sequence with the hard register each quantity got. Runner: `tmp/grind/func_800770B8/s26/qtydbg.py` + `s26/qty.sh`. One cc1 run per variant; no score needed to test a local-alloc hypothesis.
- [s26] s25's frontier item #3 is CLOSED: the spans are no longer estimated from post-sched2 rows. Measured block-1 tables are recorded above for F, X, fCADS, d1f, d2f, d3f.
- [s26] s25's numeric seat model is FALSIFIED. X's merged chain+sum quantity spans [28,56] = 28, not ~14; its priority is 88/28 = 3.14, already BELOW the blocking short quantity's 4.0, and it is already allocated SECOND. Growing the span further cannot be the missing ingredient because the ordering s25 wanted to buy is already in hand.
- [s26] THE REAL CRITERION IS INTERVAL CONTAINMENT. X: short qty [12,14], chain [28,56] — disjoint, so $2 is free again at ord=1 and the chain takes it (wrong seats). fCADS: chain [12,56] CONTAINS short [20,22], so $2 is busy, the chain falls to $3, and the reload [48,52] (dead-short-qty window) finds $2 — the target's seats. Condition, exactly: (i) chain span > 22 so it is ordered after the short qty; (ii) short qty's [birth,death] strictly inside the chain's; (iii) short qty dead before the reload's birth (48).
- [s26] The merged quantity's BIRTH is the chain root's birth only when the tie fires at the `t0*5` add. In X the chain root (born 12) stays a separate 4-ref quantity and the merged quantity starts at 28; in fCADS the chain root is inside the merged quantity, which is why fCADS's merged interval starts at 12.
- [s26] RTL FACT that supersedes the s21/s25 "source names it first" tie model: in a fresh `-da` dump of the floor body, the cursor add (insn 185) and the 0x5C control add (insn 263) are structurally IDENTICAL — `(set (reg D) (plus (reg SHIFT) (reg RELOAD)))` with REG_DEAD on both operands — even though both sites are spelled base-first in C. Source operand order does not survive into RTL operand order at either site; the difference is quantity state seen by `combine_regs`, not operand order.
- [s26] Named-pointer hoists of the t0*4-consuming group pointers, stores untouched in A,D,C,S order, do NOT move the chain quantity's birth: d1f 40, d2f 25, d3f 20 (flip) and d1n 35, d2n 5, d3n 10 (base-first), all 175 insns; chain birth stays 26-28 in every flipped variant while a NEW 6-ref quantity splits off the named pointer. `d2n` is byte-identical to the floor (a free spelling). sched1 sinks the `sll` to its first consumer, so the chain root's birth follows the STORE order, not the declaration.
- [s26] fCADS's 12 points are localised: the only genuine differing rows are the known 35-36 and the contiguous store-group window rows 40-54; rows 55-64 (class C, incl. `addu $v1,$v1,$v0`) are byte-exact and nothing leaks past row 54.
- [s26] 6 fresh scoring builds, 5 instrumented-cc1 qty runs, 1 fresh `-da` dump set; src/text1b.c restored to pristine after every sweep (git status clean apart from the engine's own metrics/events.jsonl).

- [s26] Chassis unchanged from s25 (HEAD d8215bb2; s24/s25 commits are ledger-only). Working tree byte-equals git show HEAD:src/text1b.c and s25's pristine copy. Floor re-verified live: candidate.c plus its two documented byte-neutral caller-side edits = score 5, build_insns 175, target_insns 175.

- [s26] NEW INSTRUMENT, transferable to every RA question in this project: the instrumented cc1 (tools/gcc-2.7.2/cc1) carries env-gated BB2_SUGG_DEBUG (local-alloc.c:1437) and BB2_QTY_DEBUG (local-alloc.c:1585) hooks that print, per basic block, every quantity's qty_birth/qty_death/qty_n_refs/qty_min_class/suggestions AND the qty_order allocation sequence with the hard register each quantity got. Runner tmp/grind/func_800770B8/s26/qtydbg.py + s26/qty.sh: one cc1 run per variant, no sandbox score needed to test a local-alloc hypothesis. No prior session on this function used these hooks.

- [s26] s25's frontier item #3 ('turn the span estimate into an exact threshold') is CLOSED on the first probe: qty_birth/qty_death are now read directly instead of inferred from post-sched2 row numbering.

- [s26] Block-1 qty tables measured this session. F (floor): [12,14] refs 4 -> $2; [48,56] refs 10 -> $2 (sum merged with the reload); [28,52] refs 16 -> $3; [10,44] refs 12 -> $4; [6,46] refs 14 -> $5. X (plain flip): [12,14] refs 4 -> $2; [28,56] refs 22 -> $2; [48,52] refs 4 -> $3. fCADS: [20,22] refs 4 -> $2; [12,56] refs 22 -> $3; [48,52] refs 4 -> $2 (target seats).

- [s26] The class-C seat condition, stated exactly: (i) the merged 22-ref chain+sum quantity's span exceeds 22 so it is ordered after the short 4-ref quantity (priority 4.0); (ii) the short quantity's [birth,death] lies strictly INSIDE the chain quantity's; (iii) the short quantity is dead before the cursor reload's birth at 48. X satisfies (i) and (iii) but not (ii); fCADS satisfies all three.

- [s26] The merged quantity's birth equals the chain root's birth only when the tie fires at the t0*5 add. In X the chain root (born 12) stays a separate 4-ref quantity and the merged quantity starts at 28; in fCADS the chain root is inside the merged quantity, which is why fCADS's interval starts at 12.

- [s26] The floor body ALREADY produces the target's interval geometry at the 0x5C control site from ordinary base-first C: F.qty block 3 reads ord=0 [8,12] refs 8 -> $2, ord=1 [4,26] refs 24 -> $3 (the sum quantity, contains [8,12]), ord=2 [24,26] refs 4 -> $2, ord=3 [16,20] refs 4 -> $2 (the reload). That is the criterion satisfied, in this same function, with no flip and no store reorder.

- [s26] RTL fact superseding the 'source names it first' tie model: the cursor add (insn 185) and the 0x5C control add (insn 263) are structurally identical -- (plus (reg SHIFT) (reg RELOAD)), shift as operand 1, REG_DEAD on both operands -- from base-first C at BOTH sites.

- [s26] Named-pointer hoists of the t0*4-consuming group pointers with every store left in A,D,C,S order: d1f 40, d2f 25, d3f 20 (flipped cursor; X = 29) and d1n 35, d2n 5, d3n 10 (base-first; F = 5), all 175 insns. Chain quantity birth stays 26-28 in every flipped variant; the hoist splits a new 6-ref quantity off the named pointer instead. d2n is byte-identical to the floor.

- [s26] fCADS's 12 points are localised: the only genuine differing rows (discounting cosmetic move/addu-$zero and li/addiu-$zero disassembly aliases) are the long-known rows 35-36 and the contiguous store-group window rows 40-54. Rows 55-64, including the class-C addu $v1,$v1,$v0, are byte-exact and nothing leaks past row 54.

- [s26] 6 fresh scoring builds, 5 instrumented-cc1 qty runs, 1 fresh -da dump set. src/text1b.c restored to pristine after every sweep; git status clean apart from the ledger edits and the engine's own metrics/events.jsonl.

## [s27] solver — the class-C residual reduced to ONE numeric inequality, and paid for the first time on the target's store order

- [s27] Floor re-verified at **5 / 175 insns / 175 target insns** on the HEAD
  d71d9209 chassis (`memory/grind/func_800770B8/candidate.c` applied via
  `tmp/grind/func_800770B8/s27/apply.py`, `sandbox func_800770B8 --disable all`).
  `s27/pristine_text1b.c` byte-equals `git show HEAD:src/text1b.c`.
- [s27] The floor body is byte-identical to `asm/funcs/func_800770B8.s` for
  instruction rows 33-61 (including `sll $v1,$a1,0x2`, the `lui/addiu
  %hi/%lo(D_800A35D0)` pair, `addu $v0,$v1,$v0`, `addu $v0,$a0,$v1`,
  `addu $a0,$a0,$a1`, `addu $v1,$v1,$a1`, `sb $t0,0x68($a0)`, the `lw` reload and
  `sll $v1,$v1,1`). The ENTIRE class-C residual is row 62 plus the two `addiu`
  that inherit its destination: ours `addu $v0,$v0,$v1`, target
  `addu $v1,$v1,$v0`. `rd == rs` in both, so both are `combine_regs` merges of the
  add's destination with operand 1; the divergence is WHICH operand the sum merges
  with and which hard register that quantity is seated in.
- [s27] The seat decision is now a single inequality. `qty_compare_1`
  (local-alloc.c:1660-1683) ranks by `floor_log2(refs)*refs*size/(death-birth)`.
  On the flipped cursor the chain+sum quantity is 22 refs over [28,56] =
  **3.1428**; the reload is 4 refs over [48,52] = 2.0; the only quantity that
  outranks the chain today is the t0*2 pseudo, 4 refs over [12,14] = 4.0, and it
  is DISJOINT from the chain, so `find_free_reg` hands the chain `$2` unopposed.
  The target's seats appear iff some quantity with priority > 3.1428 holds `$2`
  somewhere inside [28,56] and is dead before the reload's birth at 48.
- [s27] `local-alloc.c:472`'s `reg_n_deaths == 1` gate is why the current body has
  no such quantity: the single reused `ptr` local carries all three store-group
  addresses, dies in 3 places, and is punted to global-alloc entirely — it is
  absent from `block_alloc`'s quantity table (5 quantities in block 1 for both F
  and X, accounting for every pseudo EXCEPT `ptr`). Giving each group its own
  pointer local restores it to local-alloc as 1-3 fresh quantities.
- [s27] **Separating only the A-group pointer on the flipped cursor delivers the
  target's seats.** `e6` (X + `pa`): chain 22 refs [12,56] -> `$3`, reload 4 refs
  [48,52] -> `$2`, blocked by `pa` 16 refs [18,30] (prio 5.33) which dies at 30.
  Row 62 comes out **`addu $v1,$v1,$v0` — byte-exact** — at 175 insns with the
  target's A,D,C,S store order intact. `e3` (X + `pa` + `pc`) does the same.
  This is the first time class C's divergent row has been paid without fCADS's
  store-group reorder (s26 measured fCADS at 12/175 with the row paid but 12 rows
  of collateral in the store window).
- [s27] The price is a loop-invariant hoist, and it is what buys the early birth.
  With `pa` split out, `ptr` no longer carries the A assignment and loop.c hoists
  `lui/addiu %hi/%lo(D_800A35D0)` out of the outer loop (rows 38-39, ahead of the
  loop-top `sll $a1,$t0,16`). That lets the D-group `addu` float to row 43, which
  drags the chain root `sll $v1,$a1,2` to row 42 and gives the chain birth 12
  instead of 28. Collateral: rows 40-43 and 52-58 displaced and the inner-loop
  counter renamed `$a2`->`$a3` (rows 37, 63, 65-67). `e6` = 29/175, `e3` = 28/175.
- [s27] Without the hoist the seats stay wrong by a 5% margin. `cX_c` (flip +
  separate C pointer only, no hoist) keeps the chain at [28,56] / 3.1428 and puts
  `pc` at 6 refs over [34,38] = **3.0** — ordered second, so it gets `$3` and
  changes nothing (25/175). The gap to close is exactly `chain span >= 30`
  (88/30 = 2.93 < 3.0) or `pc/pd refs >= 8 at span <= 6`.
- [s27] `cF_c` — the base-first floor body with the C-group pointer moved into its
  own local `pc` — measures **5/175, byte-identical to the floor**. A free
  spelling; banked as `rejected/s27-classC-separate-C-pointer-byte-neutral-on-base-first-175insn-score5.c`.
- [s27] Source POSITION of a pointer computation is inert. Ten variants moving the
  `pd`/`pc` computations to the top of the loop body, ahead of the A stores, or in
  every intermediate arrangement (`pF_df pF_cf pF_dcf pF_dpre pF_allf` + X
  counterparts) all reproduce their base build exactly (10 / 20), and the qty
  tables of `pX_df`/`pX_dcf` are identical to `sX_all0`'s line for line.
- [s27] Sibling sweep: CD_datasync, CD_ready and CD_sync were re-checked at s22 and
  have zero code overlap with this function; nothing in their s50-s60 candidates
  touches a store-group/qty-priority residual of this shape. Not re-spent.

- [s27] Floor re-verified at 5 / 175 build insns / 175 target insns on the HEAD d71d9209 chassis; tmp/grind/func_800770B8/s27/pristine_text1b.c byte-equals git show HEAD:src/text1b.c.

- [s27] The floor body matches asm/funcs/func_800770B8.s byte-for-byte at instruction rows 33-61 and 65 onward; the whole class-C residual is row 62 (ours addu $v0,$v0,$v1, target addu $v1,$v1,$v0) plus the two addiu that inherit its destination.

- [s27] The class-C seat decision is one inequality: chain+sum 22 refs over [28,56] = 3.1428 under qty_compare_1, reload 4 refs over [48,52] = 2.0, t0*2 4 refs over [12,14] = 4.0 but disjoint. The target's seats appear iff some quantity with priority above 3.1428 holds $2 inside [28,56] and dies before 48.

- [s27] local-alloc.c:472's reg_n_deaths==1 gate punts the shared ptr local (3 deaths, 28 refs) out of local allocation entirely - block 1 has exactly five quantities in both F and X, covering every block-local pseudo except ptr.

- [s27] e6 (flipped cursor plus a separate A-group pointer) emits row 62 as addu $v1,$v1,$v0, byte-exact, at 175 insns on the target's A,D,C,S store order: chain 22 refs [12,56] to $3, reload [48,52] to $2, blocked by pa 16 refs [18,30] at priority 5.33 which dies at 30. e3 reproduces it. Scores 29 and 28.

- [s27] The early chain birth in e6/e3 is bought by a loop-invariant hoist of lui/addiu %hi/%lo(D_800A35D0) to the preheader (rows 38-39), which also costs the points: rows 40-43 and 52-58 displaced, inner-loop counter renamed $a2 to $a3.

- [s27] cX_c (flip plus separate C pointer, no hoist) leaves the chain at [28,56] / 3.1428 against pc's 6 refs over [34,38] = 3.0 and does not flip the seats (25/175). Closing the gap needs chain span at least 30 (88/30 = 2.93) or a blocker with refs at least 8 at span at most 6.

- [s27] cF_c - the base-first floor body with the C-group pointer moved into its own local - measures 5/175 and is byte-identical to the floor: a free spelling, banked in rejected/.

- [s27] Source position of a pointer computation is inert (ten builds, two qty tables); only store order and pointer death count reach sched1's placement of the chain root sll.

- [s27] Sibling sweep not re-spent: CD_datasync, CD_ready and CD_sync were checked at s22 and have zero code overlap with this function; none of their candidates touches a store-group / qty-priority residual of this shape.

## [s28] forensics -- s26's RTL fact falsified, s27's LICM attribution falsified, and the flipped family taken from 25 to 7

- [s28] Floor re-verified at **5 / 175 build insns / 175 target insns** on the HEAD
  08b2924a chassis (`tmp/grind/func_800770B8/s28/pristine_text1b.c` byte-equals
  `git show HEAD:src/text1b.c`; `memory/grind/func_800770B8/candidate.c` applied via
  `s28/apply.py` plus its two documented byte-neutral caller-side edits).  Every
  banked s27 number reproduces exactly: X 29, e6 29, e3 28, cX_c 25, all 175/175.

- [s28] **s26's central RTL fact is FALSIFIED.**  s26 recorded that at the cursor add
  "the shift is operand 1 in both" and concluded "source operand order does not survive
  into RTL operand order".  A fresh side-by-side `.lreg` of F and cX_c
  (`s28/F.lreg:635`, `s28/cX_c.lreg:640`, produced by `s28/lreg.py`) shows the opposite:

      F     (insn 185 (set (reg:SI 110) (plus:SI (reg:SI 109) (reg:SI 108))))
            109 = the D_800A36A0 reload (insn 183), 108 = the t0*10 shift (insn 181)
      cX_c  (insn 185 (set (reg:SI 111) (plus:SI (reg:SI 109) (reg:SI 110))))
            109 = the t0*10 shift (insn 181), 110 = the D_800A36A0 reload (insn 183)

  Same insn UID, both with REG_DEAD on both operands, and operand 1 SWAPS with the
  source spelling.  s26 read the two sites' pseudo numbers as if the numbering were
  stable; it is not -- the reload and the shift trade numbers between F and X.  The
  base-first cursor puts the RELOAD in operand 1; the flipped cursor puts the SHIFT
  there.  Source operand order at this site DOES survive into RTL.

- [s28] **The pass and the decision are now named exactly.**  `block_alloc`'s tying loop
  (local-alloc.c:1239-1298) walks `for (i = 1; i < insn_n_operands; i++)`, calls
  `combine_regs (r1, r0, ...)` (local-alloc.c:1295) and `break`s on the FIRST success.
  `addsi3_internal` has no matching constraint, so `must_match_0 == -1` and both
  operands are eligible -- operand 1 simply wins.  Therefore:
    * F  -> the destination is tied to the RELOAD, joins the reload's quantity, and is
      emitted in that quantity's hard register ($v0): `addu $v0,$v0,$v1`.
    * X / cX_c / h3 -> the destination is tied to the SHIFT and joins the merged
      22-ref chain quantity.
  The TARGET emits `addu $v1,$v1,$v0` with the destination in the chain's register, so
  **the original C spelled the cursor shift-first**; the base-first floor body cannot
  reach the target's row 62 by any seat change, only by the flip.

- [s28] **F's SEATS are already the target's.**  F block 1:
  `[12,14] r4 -> $2`, `[48,56] r10 (reload+dest) -> $2`, `[28,52] r16 (chain) -> $3`,
  `[10,44] r12 -> $4`, `[6,46] r14 -> $5`.  The chain sits in $3 = $v1 and the reload in
  $2 = $v0 -- exactly the target -- which is why F's rows 33-61 and 65+ are byte-exact.
  F's whole class-C residual is the TIE, not the seating.  Conversely X/cX_c have the
  right tie and the wrong seating: the merge grows the chain to 22 refs / [28,56]
  (priority 4*22/28 = 3.1428) and shrinks the reload to 4 refs / [48,52] (2.0), so the
  chain is allocated first and takes $2.

- [s28] **s27's LICM attribution is FALSIFIED.**  s27 recorded that in e6 "loop.c hoists
  `lui/addiu %hi/%lo(D_800A35D0)` out of the outer loop".  `tools/loop_movables.py`
  reports on F and on e6 are **line-for-line identical** (`s28/mov.sh` output; same three
  loops, same movables, same thresholds, same decisions), and both `.lreg` dumps place the
  `(set (reg) (symbol_ref "D_800A35D0"))` insn INSIDE the outer loop (after
  `NOTE_INSN_LOOP_BEG 100`: F at `F.lreg:559`, e6 at `e6.lreg:507`).  Nothing is hoisted
  out of a loop in either body.  What differs is the insn's POSITION WITHIN THE BLOCK, and
  the dumps name the mechanism outright -- the dependency lists on that insn:

      F   ... (insn_list:REG_DEP_OUTPUT 116 (insn_list:REG_DEP_ANTI 119 (... 122 125 128 131 ...)))
      e6  ... (nil)

  In F the symbol is loaded into the SAME pseudo (`ptr`) that the group-A stores read, so
  `sched_analyze` records one output dependency and five anti-dependencies against insns
  116-131 (the A stores) and sched1 cannot float the load ahead of them.  In e6 group A
  uses its own local `pa`, the dependency list is empty, and sched1 lifts the load to the
  top of the block.  **The construct that pins the D-group symbol in F is the reuse of one
  `ptr` local across store groups, and the pass is sched.c's dependency construction, not
  loop.c's `move_movables`.**

- [s28] **e6's 32 differing rows attributed in full** (`s28/rows2.py`, which canonicalises
  the `li`/`move`/`nop` assembler aliases so a cosmetic spelling never reads as a residual):
    * 2 rows -- 35/36, the baseline `$s1` vs `$v0` store base, present in the FLOOR too;
    * 2 rows -- 38/39, the displaced `lui`/`addiu %hi/%lo(D_800A35D0)` pair itself;
    * 14 rows -- 40-58, the store-window displacement that pair drags with it;
    * 14 rows -- 37, 63, 65, 66, 67, 69, **77, 82, 90, 104, 113, 114, 119, 126**: PURELY the
      inner-loop counter's $a2 -> $a3 rename.
  s27's frontier item #3 guessed the rename was 5 rows (37/63/65-67) and confined to the
  first loop.  It is 14 rows and it propagates into the SECOND loop (77-126), including the
  delay-slot fill at 126 that degrades to `nop`.  So of e6's 32 rows, only 16 are the
  geometry and 14 are one derivative register rename -- confirming s27's frontier item #3
  and roughly doubling its estimate of how derivative the collateral is.

- [s28] **cX_c's 25 rows are ONE seat, not 25 residuals.**  Its rows 40-58 all differ only
  because the chain quantity sits in $v0 instead of $v1; rows 60-64 differ for the same
  reason.  Only rows 35/36 are independent.  That reframes the whole flipped family: it was
  never 20 points of geometry away, it was one local-alloc seat away.

- [s28] **NEW BEST FLIPPED-FAMILY BODY: h3 = score 7 / 175 / 175.**  cX_c (flipped cursor +
  the C-group pointer in its own local `pc`) with the 0x68 byte store moved ahead of the C
  pair -- store order A, D, S, C.  Its block-1 table (`s28/h3.qty`) is the target's:

      ord=0 [12,14] r4  -> $2      ord=1 [36,38] r4  -> $2   (the S-store address temp)
      ord=2 [28,56] r22 -> $3      ord=3 [48,52] r4  -> $2   (the cursor reload)

  The S temp prices 2*4/2 = **4.0**, above the chain's 3.1428, so it is allocated first,
  holds $2 across [36,38] which lies inside the chain's [28,56], and dies at 38 -- before
  the reload's birth at 48.  The chain is forced to $3 = $v1 and the reload finds $2 = $v0.
  Rows 60-64 come out byte-exact, and so does the entire store window 40-53.  h3's residual
  is 7 rows: the baseline 35/36 plus five rows (54, 55, 57, 58, 59) that are purely the
  S-before-C emission order.  Banked
  `rejected/s28-classC-paid-ADSC-store-order-flip-175insn-score7.c` -- **a frontier body,
  not a dead one.**

- [s28] Both of h3's ingredients are load-bearing and were measured separately:
  dropping `pc` while keeping the A,D,S,C order (m1) scores **29** -- with the C pair on the
  shared `ptr` local that pseudo has three deaths, local-alloc.c:472 punts it to
  global-alloc, and it never enters the quantity table; keeping `pc` with the S store last
  (cX_c) scores **25** -- `pc` is then 6 refs over [36,40] = 3.0, just below 3.1428.
  Swapping the two C stores (m2) is byte-inert at 7; putting the S store between the two D
  stores (m3) costs 33.

- [s28] **Naming the S-store address is byte-inert on every base**: `ps = base + t0;` gives
  cX_c 25, X 29, F 5, h3 7 -- unchanged in all four.  The blocking quantity comes from the
  store ORDER, not from the name.  Banked
  `rejected/s28-named-S-address-byte-inert-on-every-base-175insn.c`.

- [s28] Other flipped-family spellings measured this session, all 175/175: h1 (D address as
  one expression instead of two `ptr` assignments) 26; h2 (D stores swapped) 25; h4 (cX_c
  plus a separate `pd`) 28 -- h4 DOES get the target's seats via a [18,30] r16 blocker
  (5.33) but pays the sched1 displacement; h5 (C pointer re-reading the global) 15 at 176
  insns; h6 (cX_c plus `pa`) 28.

- [s28] KILL RE-AUDIT (mandated, floor flat 3 sessions).  `tools/fake_ablate.py` on the
  closest form to the target -- h3 -- reports one FAKE unit (the empty `do { } while (0);`
  prologue fence) and it is LOAD-BEARING: 7 with it, 12 without.  No s27 kill was measured
  with a FAKE carrier occupying a target pseudo; the fence sits in the prologue and the
  class-C quantities live in block 1.  s27's four instance kills (source position of a
  pointer computation inert; chain birth without the D_800A35D0 displacement; named C/D
  pointer hoists) were all re-measured indirectly this session -- every base score they
  rest on (F 5, X 29, cX_c 25, e6 29, e3 28) reproduces byte-for-byte on the current
  chassis, so they stand as instance kills.

- [s28] 16 fresh scoring builds, 5 instrumented-cc1 qty runs, 3 `.lreg` dumps, 2
  `loop_movables` reports, 1 fake_ablate ladder.  src/text1b.c restored to pristine after
  every sweep; `git status` clean apart from the ledger edits and the engine's own
  metrics/events.jsonl.

- [s28] Floor re-verified live at 5 / 175 build insns / 175 target insns on the HEAD 08b2924a chassis; tmp/grind/func_800770B8/s28/pristine_text1b.c byte-equals git show HEAD:src/text1b.c. Every banked s27 score reproduces exactly: X 29, e6 29, e3 28, cX_c 25, all 175/175.

- [s28] s26's RTL fact is falsified. At insn 185 the floor body reads (set (reg 110) (plus (reg 109 = D_800A36A0 reload) (reg 108 = t0*10 shift))) and cX_c reads (set (reg 111) (plus (reg 109 = shift) (reg 110 = reload))). Source operand order DOES survive into RTL operand order at the cursor add; the reload and the shift merely trade pseudo numbers between the two bodies, which is what s26 misread.

- [s28] The class-C tie is decided at local-alloc.c:1239-1298: block_alloc walks operands from index 1 upward, calls combine_regs at :1295 and breaks on the first success. addsi3_internal has no matching constraint (must_match_0 == -1), so operand 1 always gets first refusal and the destination joins its quantity.

- [s28] The target emits addu $v1,$v1,$v0 with the destination in the chain's register, which requires the operand-1 tie to hit the SHIFT - so the original C spelled the cursor shift-first, and the base-first floor cursor cannot reach row 62 by any seating change.

- [s28] The floor body F already has the target's SEATS: block 1 is [12,14] r4 -> $2, [48,56] r10 (reload+dest) -> $2, [28,52] r16 (chain) -> $3, [10,44] r12 -> $4, [6,46] r14 -> $5. F's entire class-C residual is the tie, not the allocation.

- [s28] s27's LICM attribution is falsified. tools/loop_movables.py reports for F and e6 are line-for-line identical, and in both bodies the (symbol_ref "D_800A35D0") set sits inside the outer loop (F.lreg:559, e6.lreg:507, both after NOTE_INSN_LOOP_BEG 100). Nothing is hoisted out of a loop; the pair only moves position within the block.

- [s28] The real mechanism pinning that symbol load in F is sched.c dependency construction: in F the load targets the same ptr pseudo the group-A stores read, so it carries REG_DEP_OUTPUT 116 and REG_DEP_ANTI 119/122/125/128/131 and sched1 cannot float it ahead of them. In e6 group A uses its own pa local, the dependency list is (nil), and sched1 lifts the load to the top of the block.

- [s28] e6's 32 differing rows decompose as 2 baseline (35/36) + 2 displaced-symbol rows (38/39) + 14 store-window rows (40-58) + 14 pure $a2/$a3 rename rows (37, 63, 65, 66, 67, 69, 77, 82, 90, 104, 113, 114, 119, 126). s27's frontier item #3 estimated 5 rename rows in the first loop; the rename in fact propagates into the second loop.

- [s28] cX_c's 25 differing rows are one seat, not 25 residuals: rows 40-58 and 60-64 differ only because the chain quantity sits in $v0 instead of $v1. Only rows 35/36 are independent.

- [s28] NEW BEST FLIPPED-FAMILY BODY: h3 = 7/175/175 (previous best 25). Flipped cursor + the C-group pointer in its own local pc + the 0x68 byte store emitted ahead of the C pair (store order A,D,S,C). Its block-1 table is the target's: ord=0 [12,14] r4 -> $2, ord=1 [36,38] r4 -> $2, ord=2 [28,56] r22 -> $3, ord=3 [48,52] r4 -> $2. Rows 40-53 and 60-64 are byte-exact; residual = rows 35/36 plus 54, 55, 57, 58, 59, all of which are the S-before-C emission order.

- [s28] The seat criterion is now exact and doubly confirmed: a block-local quantity whose qty_compare_1 priority exceeds the merged chain quantity's, whose interval overlaps the chain's, and which dies before the cursor reload's birth at 48, forces the chain into $3 and hands the reload $2. h3's blocker is the S-store address temp (4 refs over [36,38] = 4.0); h4's is a separate D-group pointer (16 refs over [18,30] = 5.33, also the target's seats but 28 points of sched1 collateral).

- [s28] Both of h3's ingredients are load-bearing: without pc the order alone scores 29 (the shared ptr local has three deaths and local-alloc.c:472 keeps it out of the quantity table); with pc but the S store last (cX_c) it scores 25 (pc is 6 refs over [36,40] = 3.0, just under 3.1428).

- [s28] Naming the S-store address (ps = base + t0;) is byte-inert on every base measured: cX_c 25, X 29, F 5, h3 7 - all unchanged.

- [s28] Other flipped-family spellings measured, all 175/175 unless noted: h1 (D address as one expression) 26, h2 (D stores swapped) 25, h4 (cX_c + separate pd) 28, h5 (C pointer re-reading the global) 15 at 176 insns, h6 (cX_c + pa) 28, m2 (C stores swapped on h3) 7, m3 (S between the D stores) 33.

- [s28] KILL RE-AUDIT (mandated): tools/fake_ablate.py on h3, the closest form to the target, reports one FAKE unit - the empty do { } while (0); prologue fence - and it is load-bearing, 7 with it and 12 without. No s27 kill was measured with a FAKE carrier occupying a class-C pseudo; the fence sits in the prologue while the class-C quantities live in block 1. Every base score the s27 kills rest on (F 5, X 29, cX_c 25, e6 29, e3 28) reproduces byte-for-byte on the current chassis, so those instance kills stand.

- [s28] 16 fresh scoring builds, 5 instrumented-cc1 quantity runs, 3 .lreg dumps, 2 loop_movables reports and 1 fake_ablate ladder this session. src/text1b.c restored to pristine after every sweep; git status clean apart from the ledger edits and the engine's own metrics/events.jsonl.

## s29 (2026-09-05, object-model) - the object model is audited and is NOT the residual

- [s29] OBJECT MODEL: audit of every global the function touches, declared shape vs evidence, each mismatch measured on the HEAD dcd79965 chassis with candidate.c's floor body applied (floor re-verified first: F = 5 / 175 build insns / 175 target insns). Method: for each symbol, the census row (named_syms.txt), the sibling addressing in asm/funcs/*.s, this function's own index arithmetic in asm/funcs/func_800770B8.s, and the matched-C uses in the TU were read; the corrected DECLARATION was applied TU-wide (src/text1b.c lines 1995-1999, 6520/6534, 6651-6653 - the extern block in src/text1b_b.c is a separate TU and untouched) and the use sites in this function respelled to the declared shape. Generator tmp/grind/func_800770B8/s29/gen29.py, runner run29.sh, scores in sweep.log, whole-TU object comparisons in cmp.log (objdump -dr of the sandbox object for each variant against F's).
    - D_800A374C (g_dma_buf_base): declared `extern s32 D_800A374C;` (m2c_context.h and text1b.c:1469). Evidence: every sibling (`lui/lw %lo(D_800A374C)`, AddPrim(D_800A374C + k) in 30+ matched text1b functions) and this function's rows 12-13 read it as one 32-bit scalar. VERDICT: MATCHES.
    - D_8009BCE4 (g_per_player_slot_flags_20, "20 u8 entries, 10 per player x 2 players"): declared `extern u8 D_8009BCE4;` (scalar) and used as `(&D_8009BCE4)[idx]`. Evidence: this function's rows 98-113 index it as `lui $at / addu $at,$at,$v1 / lbu|sb %lo(D_8009BCE4)($at)` with $v1 = (a2 + t0*10) sign-extended - GCC's direct-subscript form for `u8 sym[]` with a register index; func_800759D0 / func_80075F80 / func_800768DC address it the same way (lui $at + addu + lbu/sb) and func_80075F80.s:168 materialises `addiu $v0,%lo(D_8009BCE4)` as a base pointer. Corrected declaration `extern u8 D_8009BCE4[20];` with `D_8009BCE4[idx]` (s29 A1): score 5 / 175, and the whole-TU .text disassembly is byte-identical to F's (cmp.log: only the objdump file-name line differs). VERDICT: MISMATCH (scalar declared, 20-byte table is the object), measured score 5 = byte-neutral. The 2-D shape `extern u8 D_8009BCE4[2][10];` with `D_8009BCE4[t0][a2]` (s29 A2) is 31 / 172: the compiler folds the row into the address and stops materialising the s16 idx that `1 << idx` needs, so the 1-D table indexed by the named idx is the original shape.
    - D_8009BD21 (g_text1b_pair_lookup_BD20_hi): declared `extern u8 D_8009BD20;` + `extern u8 D_8009BD21;` (two scalars) and used as `(&D_8009BD21)[x * 2]`. Evidence: func_80074488.s:148-161 reads BD20 and BD21 through the SAME register index $a1 = v*2 (v = the 0x67 byte), func_800747D8.s:132-147 materialises `addiu $a2,%lo(D_8009BD20)` as a table base and toggles the 0x67 byte with xor 1, and this function's rows 166-172 read BD21 at index (0x67 byte)*2. So the object is a 2-row pair table `u8 D_8009BD20[2][2]` whose [x][1] byte is "BD21"; D_8009BD24 (text1b.c:2836, `(u8)D_8009BD24 < 0xC`) is a separate scalar after it. Corrected declaration `extern u8 D_8009BD20[2][2];` (D_8009BD21's extern removed) with `D_8009BD20[*(u8 *)(D_800A36A0 + 0x67)][1]` (s29 B1): score 5 / 175; the only object-level difference is the relocation pair R_MIPS_HI16/LO16 `D_8009BD20` with in-place addend 1 (`lbu $3,1($1)`) where F carries `D_8009BD21` addend 0 - identical linked bytes (D_8009BD21 == D_8009BD20 + 1). VERDICT: MISMATCH (two scalars declared, one 2x2 pair table is the object), measured score 5 = byte-neutral.
    - D_800A35D0 (g_text1b_pad_state_arr, 5 refs): declared `extern s16 D_800A35D0;` (scalar) and used as a u8 pointer walk `ptr = (u8 *)&D_800A35D0; ptr = t0*4 + ptr; sh 0 at +2, +0`. Evidence: the matched func_80075670 (text1b.c:6534) passes `(&D_800A35D0) + arg1*2` as an s16* (a 4-byte row per player), func_80075F80.s:83-86 and func_800768DC.s:33-34 materialise `addiu $a3,%lo(D_800A35D0)` and add a per-player offset, func_800747D8.s:19-20 the same; this function's rows 49-53 materialise the symbol once (`lui/addiu`), add t0*4, and store two halfwords at +2 and +0. The object is `s16 D_800A35D0[2][2]` (per-player pair). Corrected declaration `extern s16 D_800A35D0[2][2];` (also respelling func_80075670's argument as `D_800A35D0[arg1]`): with the byte-pointer walk kept as `ptr = (u8 *)D_800A35D0;` (s29 C2) the score is 5 / 175 and the whole-TU .text is byte-identical to F's - func_80075670's bytes included; with the direct subscript `D_800A35D0[t0][1] = 0; D_800A35D0[t0][0] = 0;` (s29 C1, either store order - C1b) it is 13 / 176; with `ptr = (u8 *)D_800A35D0[t0];` (row decay, s29 C1c) it is 10 / 175; with a fresh row-pointer local `s16 *pd = D_800A35D0[t0];` (s29 C3) it is 35 / 175. VERDICT: MISMATCH (scalar declared, s16[2][2] is the object), measured score 5 = byte-neutral in the pointer-walk spelling; the subscript spellings cost 5-30 rows (mechanisms below).
    - D_800A36A0 (the record pointer, not in the brief's list but the pointee is the function's main object): the pointee is one ~0x92-byte record whose per-player fields sit at strides 1 (0x68), 2 (0x08/0x0C/0x10/0x14/0x3C/0x5C/0x60), 4 (0x40) and 10 (0x6A/0x7E) - s10's evidence-backed GmS layout. VERDICT: MISMATCH-measured in s10 (full struct-typed rederive 178 / 22: array-ref spellings of the stride-4 and stride-10 rows fold the constants onto the index side and hoist `&D_800A35D0 + 2`); not re-measured this session because the s29 C1 result (a header-declared 2-D array subscript emits the reg+symbol macro address form) is the same mechanism family and the floor body's byte-pointer arithmetic already emits every record row byte-exactly (rows 40-61, 66-67, 74-96 match).
    - COMBINED: all three corrected declarations together with the byte-neutral use spellings (s29 ALL2 = A1 + B1 + C2) = 5 / 175, .text byte-identical to F apart from the BD20+1 relocation spelling. The corrected declarations are banked as memory/grind/func_800770B8/s29-objmodel-ALL2-declaration-edits.patch (unified diff against HEAD src/text1b.c) and the object-model-corrected body as memory/grind/func_800770B8/candidate_objmodel_ALL2.c; candidate.c's body is left as the standalone-compiling floor body so inherited apply scripts keep working. The brief's four DECLARATION PUNS (candidate.c 325/348/350/375) are all removable at zero byte cost: 348/350/375 become plain array subscripts and 325 becomes array-to-pointer decay of a declared s16[2][2].
    - PREMISES the floor-5 argument rests on, stated as attackable claims: (P1) the residual is exactly rows 35/36 (class B: the two prologue clear stores addressed through $s1 instead of the freshly returned $v0) plus rows 62-64 (class C: the cursor add tied to the reload instead of the shift), and none of these five rows contains a global symbol - every symbol-bearing row of the target (12-13, 49-53, 98-113, 170-172) is already byte-exact, so no declaration change can touch them; (P2) the D_800A36A0 record is addressed by the target through a pointer plus constant offsets with the per-player index folded in by explicit sll/addu, which is the byte-pointer-walk shape and not a C array subscript of a typed member (s10 178/22, s29 C1 13/176 for the analogous global); (P3) D_800A35D0's target rows materialise the symbol into a register before the index add, which a direct subscript of a global array cannot produce in GCC 2.7.2 because the MIPS backend accepts (plus reg (const (plus sym k))) as a legitimate address and defers it to the assembler macro (lui $at / addu $at / sh per store); (P4) D_8009BCE4 IS subscripted directly in the original (the $at macro form is exactly the target's), with a 16-bit named index that is also the shift count of the mask; (P5) class C is a local-alloc operand-1 tie question (s28) and class B a store-base-name question (s28 frontier #2) - neither depends on how any global is declared.

- [s29] The s10/s12 attribution "array-typing D_800A35D0 makes LICM hoist `&D_800A35D0 + 2`" is only half right and does not describe the header-declared form. Fresh dump for s29 C1 (tmp/grind/func_800770B8/s29/dumps_C1/text1b.loop, function func_800770B8, insns 143 and 155): the two stores are `(set (mem/s:HI (plus:SI (reg 93) (const (plus (symbol_ref D_800A35D0) (const_int 2))))) 0)` and `(set (mem/s:HI (plus:SI (reg 93) (symbol_ref D_800A35D0))) 0)` - the symbol never gets its own pseudo, so there is nothing for loop.c to hoist; tools/loop_movables.py on the same dump lists NO D_800A35D0 movable in the outer loop (100..417: only the same three movables F has, regs 133/173/241 = F's 128/168/236 renumbered). The extra insn and rows 51-56 are the assembler's macro expansion of `sh $0,D_800A35D0+2($3)` (lui $at / addu $at,$at,$3 / sh $0,%lo(...)($at)) - 3 insns per store where the target does lui/addiu/addu once and stores twice. The LICM hoist s12 recorded DOES occur for C1c (`ptr = (u8 *)D_800A35D0[t0]`: rows.py shows &D_800A35D0 in $t4 from the preheader, row 51 `addu $v0,$v1,$t4`) because that spelling gives the symbol a pseudo of its own. Two different mechanisms, same lesson: the target's symbol-then-add shape is the byte-pointer walk.

- [s29] GOTCHA (tooling): `pwsh tools/grinder/dump.ps1` invoked from INSIDE a WSL bash script (s28/mov.sh pattern) fails silently under `|| true`, so the "dumps_<variant>" directories that script copies are stale copies of whatever tmp/grind/func_800770B8/dumps/ held before - s29's first dumps_C1 was byte-identical to s28's dumps_F (cmp confirmed). Run dump.ps1 from the PowerShell tool with the variant copied into src/text1b.c, then run tools/loop_movables.py / rows.py under WSL against the fresh dumps. The s28 mov.sh dumps_e6 report should be treated with the same suspicion unless its .loop differs from dumps_F (not re-checked here; s28's e6 conclusions were drawn from .lreg dumps produced by lreg.sh, a separate path).

- [s29] KILL RE-AUDIT (mandated): tools/fake_ablate.py on candidate.c (the floor body, the closest form to the target at 5) reports one FAKE unit, the empty `do { } while (0);` prologue fence, load-bearing: keep-all 5 / drop-1 10 (tmp/grind/func_800770B8/s29/fake_ablate_F.log), identical to s25/s28. Every s28 instance kill rests on base scores that reproduce on this chassis (F 5 re-measured directly; the object-model variants that are byte-neutral (A1/B1/C2/A1B1/ALL2) reproduce F's exact bytes, so the class-C pseudo table s28 read out of F is unchanged). The fence sits in block 0; no FAKE carrier occupies a class-B or class-C pseudo. Kills stand.

- [s29] 13 fresh sandbox builds (F, A1, A2, B1, C1, C1b, C1c, C2, C3, ALL, A1B1, ALL2, plus the fake_ablate drop-1), one fresh cc1 -da dump set (dumps_C1), one loop_movables report, six objdump -dr whole-TU comparisons. src/text1b.c restored to the pristine HEAD copy after every build (cmp-verified); git status shows only the ledger edits and the engine's metrics/events.jsonl.

- [s29] OBJECT MODEL: D_800A374C MATCHES (s32 scalar, every sibling reads it as one word). D_8009BCE4 MISMATCH (declared scalar, is a u8[20] table indexed by the named s16 idx in the target's lui $at/addu/lbu form) measured score 5 byte-neutral. D_8009BD21 MISMATCH (declared scalar, is the [x][1] byte of u8 D_8009BD20[2][2]; func_80074488 reads BD20 and BD21 with one x*2 index) measured score 5 byte-neutral. D_800A35D0 MISMATCH (declared scalar, is s16[2][2]; func_80075670 passes &D_800A35D0 + arg1*2 as an s16*, siblings materialise it as a base + per-player offset) measured score 5 byte-neutral in the pointer-walk spelling, 13/176 direct subscript, 10/175 row decay, 35/175 row-pointer local. D_800A36A0's pointee MISMATCH-measured in s10 (struct-typed rederive 178/22), not re-measured: same mechanism family and every record row is already byte-exact.

- [s29] Premises of the floor-5 argument, stated as attackable claims (evidence.md [s29]): P1 the residual rows 35/36 + 62-64 carry no symbol and every symbol-bearing target row is byte-exact; P2 the record is addressed by pointer + constant offsets with the per-player index folded by explicit sll/addu (byte-pointer walk, not a typed subscript); P3 a direct subscript of a global array with a register index is a legitimate (plus reg (const (plus sym k))) address in GCC 2.7.2 and is macro-expanded per store, so it cannot produce the target's symbol-then-add rows; P4 D_8009BCE4 IS directly subscripted in the original with a named 16-bit index that doubles as the mask shift count; P5 class C is local-alloc's operand-1 tie and class B a store-base-name choice, neither declaration-dependent.

- [s29] The s10/s12 attribution 'array-typing D_800A35D0 triggers LICM of &D_800A35D0+2' does not describe the header-declared subscript form: the fresh s29 C1 .loop dump shows the symbol folded into the store address (no pseudo, nothing for loop.c to move; loop_movables lists no D_800A35D0 movable) and the +1 insn is the assembler macro expansion of sh $0,D_800A35D0+2($3). The LICM hoist does occur for the row-decay spelling C1c, which gives the symbol its own pseudo.

- [s29] Banked: memory/grind/func_800770B8/candidate_objmodel_ALL2.c (object-model-corrected body, byte-identical to the floor) + s29-objmodel-ALL2-declaration-edits.patch (the TU-level declaration edits, unified diff against HEAD src/text1b.c; header-canonical placement in include/*.h is an integration handoff at completion). candidate.c's body is unchanged so inherited apply scripts still compile it against the pristine TU. All four driver-flagged DECLARATION PUNS are removable at zero byte cost.

- [s29] Tooling gotcha: pwsh tools/grinder/dump.ps1 invoked from inside a WSL bash script (s28/mov.sh pattern) fails silently under || true and the copied dumps_<variant> directories are stale copies of the previous dumps/ (s29's first dumps_C1 was byte-identical to s28's dumps_F). Run dump.ps1 from the PowerShell tool with the variant in src/text1b.c, then analyse under WSL.

- [s29] Floor re-verified live at 5 / 175 build insns / 175 target insns on HEAD dcd79965; fake_ablate one unit, keep-all 5 / drop-1 10. 13 sandbox builds, one fresh cc1 -da dump set, six whole-TU objdump comparisons; src/text1b.c restored to pristine after every build (cmp-verified).

## s30 (2026-09-05, escalation) - the target's own store-window geometry read out of the asm; o1/cX_c proved to be ONE register seat from score 2; every statement-position spelling measured byte-inert

- [s30] Floor re-verified live on the HEAD 6c9ca9fa chassis: `memory/grind/func_800770B8/candidate.c`
  (body F) applied via tmp/grind/func_800770B8/s30/apply.py plus its two documented byte-neutral
  caller-side edits = **5 / 175 build insns / 175 target insns**. h3 (the s28 frontier body)
  reproduces at **7 / 175 / 175** and cX_c/o1 at **25 / 175 / 175**, so every s28 conclusion is
  measured on the current chassis, not inherited. 33 fresh scoring builds this session
  (tmp/grind/func_800770B8/s30/sweep.log), 6 instrumented-cc1 quantity dumps (BB2_QTY_DEBUG),
  3 row-attribution diffs. src/text1b.c restored to the pristine HEAD copy after every sweep
  (cmp-verified).

- [s30] **THE TARGET'S STORE-WINDOW GEOMETRY, READ DIRECTLY OUT OF asm/funcs/func_800770B8.s
  (rows 40-64) FOR THE FIRST TIME.**  Previous sessions inferred this window from row diffs; it is
  now transcribed:

        sll  $a1,$t0,16 ; sra $a1,$a1,16     a1 = (s16)t0
        sll  $v0,$a1,1                       t0*2
        lw   $a0,%gp_rel(D_800A36A0)($gp)    a0 = base
        sll  $v1,$a1,2                       t0*4      <-- fills the lw's delay slot,
                                                           BEFORE any group-A store
        addu $v0,$v0,$a0                     group-A pointer
        sh   0x10 / 0x8 / 0xC / 0x14 / 0x3C ($v0)      group A
        lui/addiu %hi/%lo(D_800A35D0) -> $v0
        addu $v0,$v1,$v0                     group-D pointer = t0*4 + &D_800A35D0
        sh   0x2($v0) ; sh 0x0($v0)                    group D
        addu $v0,$a0,$v1                     group-C pointer = base + t0*4
        addu $a0,$a0,$a1                     S pointer      = base + t0   (REUSES base's reg)
        addu $v1,$v1,$a1                     chain root     = t0*4 + t0
        sh   0x42($v0) ; sh 0x40($v0)                  group C
        sb   $t0,0x68($a0)                             S store  (LAST)
        lw   $v0,%gp_rel(D_800A36A0)($gp)    cursor reload
        sll  $v1,$v1,1                       t0*10 = (t0*4 + t0)*2
        addu $v1,$v1,$v0
        addiu $a3,$v1,0x6A ; addiu $a1,$v1,0x7E

  Three facts follow that no earlier session had. (i) The store-group order really is
  **A, D, C, S** and the S store is emitted LAST, confirming h3's five residual rows are purely
  its S-before-C order. (ii) The three address computations are emitted as a CLUSTER before the
  C stores, so the S pointer is live ACROSS the two C stores and reuses base's register - the
  target does not pair each address with its own store. (iii) The cursor is **not** an independent
  `t0*10`: the target computes `t0*4 + t0` then `<<1`, i.e. GCC's synth_mult for 10 reusing the
  same t0*4 pseudo the D and C pointers use.  Our bodies already produce that synthesis (h3's rows
  56 and 60-64 are byte-exact), so the C spelling `t0 * 10` is correct and needs no help.

- [s30] **o1 (= cX_c: flipped cursor, C-group pointer in its own local `pc`, store order A,D,C,S)
  EMITS THE TARGET'S EXACT INSTRUCTION ORDER.**  Row-attribution diff (s28/rows2.py, alias-
  canonicalising) shows all 25 of its differing rows are register names: rows 40-42 are the
  lw/sll pair-ordering, 43-53 the group-A/D window carried on the wrong base register, 54-58 the
  address cluster and C stores, 60-64 the cursor.  Row 55 (`addu $a0,$a0,$a1`, the S pointer) and
  row 59 (`sb $t0,0x68($a0)`) are ALREADY byte-exact.  o1 is one local-alloc seat from score 2.

- [s30] **THE SEAT CRITERION, RESTATED EXACTLY AND MEASURED ON SEVEN BODIES.**  In every flipped-
  cursor body the destination of the cursor add joins the chain quantity (s28's operand-1 tie at
  local-alloc.c:1295), so the chain must be allocated AFTER something that holds $2 across it and
  dies before the cursor reload's birth at 48.  qty_compare_1 (local-alloc.c:1660-1683) =
  floor_log2(refs)*refs*size/(death-birth).  Measured block-1 tables:

        o1 / y1-y6 / w1 / w2 / w4 / w5 / x1 / v1 / v2 / v4 (score 25):
            chain [28,56] r22 = 3.1428  ->  $2  (WRONG)   pc [36,40] r6 = 3.0 -> $3
        h3 (score 7):
            S temp [36,38] r4 = 4.0     ->  $2            chain [28,56] r22 -> $3 (RIGHT)
        o2 / p2 / q2 (D-group first, score 14):
            chain [12,56] r22 = 2.0     ->  $3 (RIGHT)    pc [36,40] r6 = 3.0 -> $2
        q1 / p1 (A,D,C,S with the D or A group on its own local, score 28):
            chain [12,56] r22 = 2.0     ->  $3 (RIGHT)    pc [36,40] r6 = 3.0 -> $2

  So the target's class-C seat is reachable in the target's own A,D,C,S store order - q1 and p1
  BOTH have it.  Their 28 rows are not the seat at all: they are the sched1 displacement of the
  %hi/%lo(D_800A35D0) pair (the s28/e6 mechanism - once the D group stops sharing group A's `ptr`
  pseudo the REG_DEP_OUTPUT/REG_DEP_ANTI edges vanish and the symbol load floats to the top of the
  block, dragging the store window and renaming the inner-loop counter).

- [s30] **THE RESIDUAL IS NOW A TWO-HORNED DILEMMA, BOTH HORNS MEASURED.**  Exactly two mechanisms
  give the flipped cursor the target's class-C seat, and each costs more than the three rows class
  C is worth:
    * HORN 1 - a short high-priority blocker in [28,48).  The only real quantity that qualifies is
      the S store's address temp (4 refs / span 2 / 4.0), and it only becomes a block-local
      quantity when the S store is EMITTED BEFORE the C stores.  That is h3: score 7, i.e. the
      three class-C rows bought for five rows of S-before-C order.
    * HORN 2 - lengthen the chain's interval so pc's 3.0 outranks it.  This needs the t0*4 shift
      to float to the top of the block ([12,56], span 44, 2.0), which happens exactly when the D
      group stops sharing group A's `ptr` pseudo (own local, or emitted first).  That is o2/p2/q2
      (14) and q1/p1 (28): the same freeing that floats the shift floats the symbol pair.
  The target has the shift floated (row 42) WITHOUT the symbol floated (rows 49-51 sit immediately
  before the D stores).  No C form measured this session separates the two.

- [s30] **STATEMENT POSITION OF A PURE ADDRESS COMPUTATION IS BYTE-INERT ON THIS CHASSIS - 13
  spellings, all 25/175.**  Naming the S pointer (`ps = base + t0;`) and placing its assignment at
  five different points relative to the C group (y1 before the C pointer, y2 after it, y3 at the
  top of the loop body, y4 between groups A and D, y5 immediately before the C stores) is inert;
  so is spelling the C pointer shift-first (y6, s1), swapping the two C stores (t1), spelling the
  S address base-cast-first (u1), naming the shared `t0*4` shift as an `s32 t4` local at the top
  of the loop body or just before group D (w1, w2), naming `t0*10` (w5), naming both (w4),
  splitting the C pair onto two once-used pointer locals so each would be a 4-ref/span-2 quantity
  (x1 - CSE refolds them into one pseudo), and hoisting the C-pointer assignment above group A
  (v1) or between groups A and D (v2, v4).  The instrumented-cc1 quantity tables for w1 and y2 are
  LINE-FOR-LINE IDENTICAL to o1's, confirming the inertness is at the RTL level and not a scoring
  coincidence.  What moves bytes in this window is exactly two things: which LOCAL each store
  group uses, and the ORDER of the store groups.  Naming `t0*2` (w3) is the one exception and it
  costs 44 rows.

- [s30] Full store-group order sweep on the flipped-cursor base with the C group on `pc`
  (all 175/175): A,D,C,S = 25; D,A,C,S = 14; A,C,D,S = 39; C,A,D,S = 30; D,C,A,S = 36;
  C,D,A,S = 36; A,D,S,C (h3) = 7.  Local-assignment sweep on A,D,C,S: A on its own `pa` = 28,
  D on its own `pd` = 28, C back on the shared `ptr` = 29, both A-own and D-first = 14.
  o2's 13 differing rows (rows.py) are rows 35/36 plus eleven rows that are purely the D-group's
  symbol pair being emitted before group A - the target emits group A first.

- [s30] KILL RE-AUDIT (mandated): `tools/fake_ablate.py` results from s29 stand unchanged because
  F re-measures at 5 on this chassis; the single FAKE unit is the empty `do { } while (0);`
  prologue fence, it sits in block 0, and every quantity discussed above lives in block 1, so no
  FAKE carrier occupies a class-B or class-C pseudo.  The closest-to-target form was re-measured
  directly this session instead of ablated: h3 = 7 on HEAD 6c9ca9fa, identical to its s28 score.

- [s30] Gate (a) canonical-asm: `python3 tools/scan_hand_coded.py --single func_800770B8`
  (tmp/grind/func_800770B8/s30/scan.log) = `tier=LOW score=0/8`, every one of S1-S8 unset.
  Gate (b) SOTN precedent: zero hits in the uncapped 2,746-line
  `docs/reference/sotn-construct-index.md` for local-alloc / reg_qty / qty_compare / register-seat
  / operand-order / combine_regs.  The gate is additionally vacuous: every construct measured this
  session is ORDINARY C (store reordering, pointer locals, named intermediates) - there is no
  coercion family in hand to seek a precedent FOR.  The residual is blocked by measurement, not by
  policy.

- [s30] Floor re-verified live on HEAD 6c9ca9fa: candidate.c body F applied to src/text1b.c with its two documented byte-neutral caller-side edits = score 5, build_insns 175, target_insns 175, rules_dropped 0. h3 reproduces at 7 and cX_c/o1 at 25, so every s28 conclusion is measured on the current chassis rather than inherited.

- [s30] 33 fresh scoring builds this session, 6 instrumented-cc1 (BB2_QTY_DEBUG) block-1 quantity tables, 3 row-attribution diffs; src/text1b.c restored to the pristine HEAD copy after every sweep (cmp-verified) and the working tree carries only ledger/doc files.

- [s30] The target's store window (asm/funcs/func_800770B8.s rows 40-64) was transcribed directly for the first time instead of inferred from row diffs: store-group order is A, D, C, S with the sb LAST; the three address computations (C pointer addu $v0,$a0,$v1, S pointer addu $a0,$a0,$a1, chain root addu $v1,$v1,$a1) are emitted as a cluster BEFORE the two C stores, so the S pointer is live across them and reuses base's register; and the cursor is GCC's synth_mult for 10 (t0*4 + t0, then <<1) reusing the same t0*4 pseudo the D and C pointers use.

- [s30] o1 (flipped cursor + C group on its own pc local + A,D,C,S) emits the target's EXACT instruction order: rows 55 and 59 are already byte-exact and all 25 differing rows are one local-alloc register seat. o1 is one seat from score 2.

- [s30] The class-C residual is a two-horned dilemma, both horns measured: HORN 1 needs a short high-priority blocker in [28,48), which only the S store's address temp supplies and only when the S store is emitted before the C stores (h3, score 7 - three class-C rows bought for five rows of wrong store order); HORN 2 needs the chain's interval lengthened to span 44 by floating the t0*4 shift, which happens exactly when the D group stops sharing group A's ptr pseudo (p1/q1 win the target's seats at 28, o2/p2/q2 at 14, because the same freeing floats the %hi/%lo(D_800A35D0) pair).

- [s30] The target has the t0*4 shift floated (row 42, filling the lw's delay slot) WITHOUT the symbol pair floated (rows 49-51 sit immediately before the D stores). No C form measured in thirty sessions separates those two sched1 decisions; that separation is the single open lever and would turn q1/p1 into a score-2 body.

- [s30] Statement position of a pure address computation is byte-inert on this chassis - 13 spellings, all 25/175: five positions for a named S pointer, shift-first C pointer, swapped C stores, base-cast-first S address, named t0*4 (two positions), named t0*10, both named, split C pair onto two once-used locals (cse refolds), and the C-pointer assignment hoisted above group A or between groups A and D. The BB2_QTY_DEBUG quantity tables for w1 and y2 are LINE-FOR-LINE IDENTICAL to o1's, so the inertness is at the RTL level and not a scoring coincidence. Only which LOCAL each store group uses and the ORDER of the store groups move bytes here.

- [s30] Gate (a) canonical-asm FAILED: tools/scan_hand_coded.py --single func_800770B8 = tier=LOW score=0/8, every one of S1-S8 unset (tmp/grind/func_800770B8/s30/scan.log).

- [s30] Gate (b) SOTN-master precedent FAILED and vacuous: zero hits in the uncapped 2,746-line docs/reference/sotn-construct-index.md for local-alloc|reg_qty|qty_compare|register seat|swap operand|operand[ -]order|combine_regs, and there is no closing construct to seek precedent FOR - every construct measured this session is ordinary C (store reordering, pointer locals, named intermediates). The residual is blocked by measurement, not by policy.

- [s30] Sibling ledgers (CD_datasync, CD_ready, CD_sync, all foreclosed, all in src/system.c) were spent at this ledger's s27; their candidate.c files are unchanged since (written 2026-09-04, before s27's read on 2026-09-05), so no unspent sibling transplant existed for s30.

- [s30] Kill re-audit: the single FAKE unit remains the empty do { } while (0); prologue fence, it sits in block 0, and every quantity discussed above lives in block 1, so no FAKE carrier occupies a class-B or class-C pseudo. The closest-to-target form was re-measured directly rather than ablated: h3 = 7 on HEAD 6c9ca9fa, identical to its s28 score.

- [s30] Foreclosure record filed this session at docs/grind/decisions.md, entry '## 2026-09-05 - func_800770B8 - **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**'.

## [s31] (rederive, 2026-09-06) — the honest floor is 3; class B is closed

**Chassis re-measurement.** HEAD 2fbaa47a. `sandbox func_800770B8 --disable all` on the
inherited floor body F = score 5, build_insns 175, target_insns 175; on the h3 chassis
(rejected/s28-classC-paid-ADSC-...) = 7. Both reproduce, so the s30 frontier was current.

**The banked body is now score 3.** `memory/grind/func_800770B8/candidate.c` (applied with
the two documented byte-neutral caller-side edits: prototype `s32 func_800770B8(s32, s32,
s32);`, call site `(s32)&D_8009BD24`) measures **3 / 175 / 175**. `tools/fake_ablate.py`
reports TWO orthogonal FAKE units: keep-all 3, drop the dead store 5, drop the do-while(0)
prologue fence 8, drop both 10 (`tmp/grind/func_800770B8/ablate/`).

**Row-level residual, transcribed.** With a target-vs-ours row differ that normalises the
`move`/`addu` and `li`/`addiu` aliases (`tmp/grind/func_800770B8/s31/rows2.py`), the floor
body F has exactly FIVE differing rows and the banked s31 body has exactly THREE:

    F   35 sw $zero,0x30($s1)   | sw $zero, 0x30($v0)      <- class B, CLOSED in s31
    F   36 sh $zero,0x34($s1)   | sh $zero, 0x34($v0)      <- class B, CLOSED in s31
    s31 62 addu $v0,$v0,$v1     | addu $v1, $v1, $v0       <- class C
    s31 63 addiu $a3,$v0,106    | addiu $a3, $v1, 0x6A     <- class C (follows 62)
    s31 64 addiu $a1,$v0,126    | addiu $a1, $v1, 0x7E     <- class C (follows 62)

Class C is now a ONE-INSN question: the load and the shift already sit in the target's
seats ($v0 and $v1); only the sum's DEST register differs, and rows 63/64 are pure
consequences of it.

**Class B's mechanism, named from the dumps.** `pwsh tools/grinder/dump.ps1 func_800770B8`
on the d1 body; `tmp/grind/func_800770B8/dumps/text1b.lreg`. The target's prologue window
(asm/funcs/func_800770B8.s rows 29-38) is:

    addu $v1,$s1,$zero        ; $v1 = the OLD p_old (arg0+0x58)
    addu $s1,$v0,$zero        ; $s1 = a COPY of the func_8006E49C return value
    ...
    sw   $s1, %gp_rel(D_800A36A0)($gp)
    sw   $v1, 0x4($s1)
    sw   $zero, 0x30($v0)     ; the clears go through the RAW return value
    sh   $zero, 0x34($v0)

`$s1` is dead after row 36 and every later reference to the record is a fresh
`lw %gp_rel(D_800A36A0)` (rows 44, 63, 88, 133, 149, 158, 162, 165) — so there is no
truthful later USE of `p_old` anywhere in the function that could supply the second death
naturally. That is the fact that makes the dead store the only remaining lever for class B,
and it is a new fact: no prior session had transcribed the target's `$s1` liveness.

**Class C's mechanism, named from the dumps.** In `.lreg`, the class-C sum is

    (insn 186 (set (reg:SI 109) (mem:SI (symbol_ref:SI ("D_800A36A0")))))
    (insn 184 (set (reg:SI 108) (ashift:SI (reg:SI 107) (const_int 1))))
    (insn 188 (set (reg:SI 110) (plus:SI (reg:SI 109) (reg:SI 108))))   ; BOTH die here
    (insn 190 (set (reg/v:SI 102) (plus:SI (reg:SI 110) (const_int 106))))
    (insn 205 (set (reg/v:SI 111) (plus:SI (reg:SI 110) (const_int 126))))

`block_alloc`'s tying loop (local-alloc.c:1240-1299) walks recog_operand 1..n and breaks on
the first `combine_regs` win, so insn 188's dest ties to reg 109 (the reload) and inherits
its seat. Two independent ways of denying that tie were measured this session, and BOTH
work at the tie level:
  * making the shift the ptrop operand (f1/f2) — flips the tie, wrong seats, 27;
  * denying reg 109 the `reg_qty >= -2` precondition of local-alloc.c:469-477 by giving it
    a second death / a second basic block (g1) — flips the tie WITH the target's seats:
    **rows 60-78 are byte-exact**, the first time in 31 sessions that rows 62-64 have been
    emitted correctly on the A-first store order. Score 39, priced entirely in rows 38-59
    (loop head) and 79-89 (the 0x5C/0x60 block).

**Artifacts.** `tmp/grind/func_800770B8/s31/` — gen_b.py / gen_e.py / gen_f.py / gen_g.py /
gen_final.py (the 26 bodies, in `v/`), run.sh, apply.py, rowdiff.sh, rows2.py (alias-
normalising row differ), dis.sh, slice.py (dump slicer), bank.py, ledger.py, sweep.log.
Dumps: `tmp/grind/func_800770B8/dumps/text1b.lreg` (and .combine/.sched/.greg, same run).
Ablation: `tmp/grind/func_800770B8/ablate/`.

- [s31] HEAD 2fbaa47a chassis re-measured live: floor body F = score 5 / 175 / 175; h3 = 7. Both inherited numbers reproduce.

- [s31] The banked candidate.c (with its two documented byte-neutral caller-side edits) measures score 3 / 175 / 175 - the first floor drop since s22.

- [s31] fake_ablate on the banked body: TWO orthogonal FAKE units. keep-all 3, drop the dead store 5, drop the do-while(0) prologue fence 8, drop both 10.

- [s31] The residual is now exactly three rows, all consequences of one tie: ours `addu $v0,$v0,$v1 / addiu $a3,$v0,106 / addiu $a1,$v0,126`, target `addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E`. The load and the shift ALREADY sit in the target's seats; only the sum's dest differs.

- [s31] NEW FACT (target liveness, transcribed from asm/funcs/func_800770B8.s): $s1 - the p_old copy - is dead after row 36, and every later reference to the record is a fresh lw %gp_rel(D_800A36A0) (rows 44, 63, 88, 133, 149, 158, 162, 165). There is therefore NO truthful later use of p_old anywhere in the function that could supply the second death naturally, which is what forces class B onto the dead-store family.

- [s31] Class C's insn read out of tmp/grind/func_800770B8/dumps/text1b.lreg: insn 186 loads D_800A36A0 into reg 109, insn 184 shifts reg 107 into reg 108, insn 188 is `(set (reg 110) (plus (reg 109) (reg 108)))` with BOTH operands carrying REG_DEAD notes there; insns 190/205 add the 106/126 constants off reg 110.

- [s31] Two independent denials of that tie were measured and BOTH work at the tie level: the ptrop route (f1/f2, right tie / wrong seats / +24 loop-head rows) and the local-alloc reg_qty route (g1, right tie AND right seats, rows 60-78 byte-exact, priced in rows 38-59 and 79-89).

- [s31] Both f1 and g1 pay the SAME rows 38-59 loop-head reschedule, so that collateral is a property of the flipped tie itself, not of either spelling.

## [s32] (structural, 2026-09-06) — the class-C tie predicate, read out of the compiler

**Chassis re-measurement.** HEAD df705f93. The banked `candidate.c` body (applied with its
two documented byte-neutral caller-side edits) re-measures **3 / 175 / 175**. 36 scoring
builds this session; `src/text1b.c` restored to the pristine HEAD copy after every sweep.

**The tie predicate is now sourced, not inferred.** `block_alloc`'s tying loop
(`tools/gcc-2.7.2/local-alloc.c:1240-1299`) walks `recog_operand` 1..n with `if (win)
break;`. `combine_regs` (`local-alloc.c:1784-1946`) has exactly three C-reachable refusal
grounds: `reg_qty[operand] < 0` (`local-alloc.c:1827`), `reg_qty[dest] == -1`
(`local-alloc.c:1836`), and no `REG_DEAD` note for the operand at that insn
(`local-alloc.c:1917`). `reg_qty` itself is set at `local-alloc.c:469-477`: `-2` iff
`reg_basic_block >= 0 && reg_n_deaths == 1`, else `-1`.

**All three grounds were spelled in C and priced.**
* REG_DEAD denial (a real use of the reload after the sum, in the same block): the only
  candidate statement is the 0x68 `sb`, and the CONTROL that relocates it and changes
  nothing else (h2) already costs **24 rows**. h1 27, h4 14 at 177 insns, h5 28, h3 31 at
  176. Priced out before the tie is touched.
* `reg_qty` denial: **i1 = 14 / 175 / 175**, the best class-C form in 32 sessions — a
  re-read of `D_800A36A0` into the EXISTING `base` local just before `p_6a`/`p_7e`, so the
  pseudo has two sets with every reference still inside the one loop-body block. Rows
  44-53 and rows 63/64 are byte-exact and row 62 carries the target's dest register.
  CONTROL i5 (the same reload named in a fresh ONCE-set local) is byte-inert at 3, which
  isolates "two sets" as the entire lever. i2 14, i3 39, i4 41, i6 16 at 176.
* dest denial: not C-reachable (the dest is a compiler temp).
* operand ORDER (chain as operand 1): f2 / h6 / q7 = 27, all reseating rows 38-64.

**The operand-order collateral is the ORDER, not the cast (new).** CONTROL q5,
`(s16 *)((s32)D_800A36A0 + (t0 * 10) + 0x6A)` — the same `(s32)` cast with the global
still first — measures **3 and is byte-inert**. So `(s32)` is free; what costs 24 rows is
putting the chain in operand 1, which extends the chain quantity through row 64 and
reseats the whole block. This retires "find a cleaner cast" as a line of attack.

**i1's ceiling is structural.** One C variable is one pseudo; `reg_qty = -1` hands it to
global-alloc, which gives BOTH live ranges a single hard register (`$a1`). The target needs
the first load in `$a0` and the reload in `$v0` so that `base` can die at the S pointer
(target row 55 `addu $a0,$a0,$a1`). i1's 14 rows are exactly that consequence: the
`$a0`<->`$a1` swap at rows 38-43, the address-computation cluster order at rows 54-59, and
the reload seat at rows 60/62. Every attempt to move the two-set property onto a
*separate* short-lived local failed: a trailing dead store on a fresh local is deleted by
flow before a second death is recorded (m1/m2/m4/m6 all 3; m3, which puts a real read
after the second set, needs a second load — 176 insns); reusing the already-multiply-set
`ptr` (n1/n2/n3/n5) or `p_old` (n4) costs 27; sharing one local with a LATER block
(o1/o1b 33, o2 31 at 174, o3 36, o4 39) makes global-alloc pin one hard register across
both blocks.

**Class B's dead store is not replaceable by ordinary C (owner directive item 2 / ledger
frontier item 3, executed and CLOSED).** The target prologue was decoded further:
`func_8006E49C` returns a COMPUTED pointer (`addiu $v0,$a0,0x1FB0` is its last value insn,
`asm/funcs/func_8006E49C.s`) and is a pure field-initialiser writing 0x0-0x4C of its `$a1`
argument — it is NOT a list allocator, and neither `func_80076FF8` (a ten-call
`func_8006920C` loop over fields 0x14-0x38) nor `func_8006E950` establishes a
linked-list-push shape, so no natural list walk exists to reassign `p_old`. The target
does emit a REAL copy insn `addu $s1,$v0,$zero`, i.e. the raw result and the `p_old`
variable really are two pseudos there. Spelling that shape directly always loses insns:
s1/s2/s4 = 23 at **170** insns, s5 = 25 at 170, s3 (no `prev`, old pointer re-derived from
`arg0`) = 17 at **173**. Mechanism: a separate local for the raw result lets cse
rematerialise `p_old`'s pre-call value from `$s0` (= `arg0`) after the three calls, so
nothing needs a callee-saved register and the `$s1` save/restore prologue collapses. This
re-confirms s31's c1/c4/c5/c9 with the mechanism named.

**Byte-inert equivalences banked (all 3 / 175 / 175).** `p_7e = p_6a + 10` (r4); the chain
spelled `(t0 * 5) * 2` (r5); `t0 * 10` named in an `s32` local (r3 — s30's kill
re-confirmed on THIS chassis, per the chassis-relative re-measure rule). Not inert:
swapping the two pointer declarations costs 5 (r1); swapping the inner loop's two stores
costs 7 (r2).

**Artifacts.** `tmp/grind/func_800770B8/s32/` — gen_h.py, gen_i.py, gen_m.py, gen_n.py,
gen_o.py, gen_q.py, gen_r.py, gen_s.py (the 36 bodies in `v/`), apply.py, run.sh,
rowdiff.sh, rows2.py, sweep.log, hdr.txt.

- [s32] HEAD df705f93 chassis re-measured live: the banked candidate body = 3 / 175 / 175. The residual is still rows 62/63/64 and still one local-alloc tie.
- [s32] The class-C tie has exactly three C-reachable refusal grounds, read out of tools/gcc-2.7.2/local-alloc.c: reg_qty[operand] < 0 (1827), reg_qty[dest] == -1 (1836), and the missing REG_DEAD note (1917), with reg_qty set at 469-477 (-2 iff reg_basic_block >= 0 && reg_n_deaths == 1).
- [s32] i1 (re-read D_800A36A0 into the existing `base` local before p_6a/p_7e) = 14/175/175 with rows 44-53 and 63/64 byte-exact and row 62 carrying the target's dest register - the best class-C form in 32 sessions, and 11 rows worse than the floor.
- [s32] CONTROL i5 (the same reload named in a fresh ONCE-set local) is byte-inert at 3, isolating "two sets" as the entire lever.
- [s32] CONTROL q5 ((s32)D_800A36A0 + (t0*10) + 0x6A, global first) is byte-inert at 3, so the operand-order route's 24-row collateral is the ORDER, not the (s32) cast. "Find a cleaner cast" is retired.
- [s32] CONTROL h2 (relocate the 0x68 sb into the p_6a/p_7e block, changing nothing else) = 24, which prices out every REG_DEAD-denial spelling before the tie is touched.
- [s32] A trailing dead store on a fresh reload local is deleted by flow before a second death is recorded (m1/m2/m4/m6 all 3); the second set only counts when a real read follows it, which in this block costs a second load (m3, 176 insns).
- [s32] func_8006E49C is a pure field-initialiser (writes 0x0-0x4C of its $a1 arg) returning a computed pointer, and func_80076FF8 / func_8006E950 show no linked-list-push shape - there is no natural list walk that could reassign p_old, closing ledger frontier item 3.
- [s32] The two-pseudo class-B shape the target actually has (raw result in its own local, p_old reassigned from it) costs 2-5 insns in every spelling (s1/s2/s4 23 at 170, s5 25 at 170, s3 17 at 173): a separate raw-result local lets cse rematerialise p_old's pre-call value from $s0, collapsing the $s1 callee-save prologue.


## [s32b] (structural, 2026-09-06, second run) — the residual is ONE local-alloc decision

**Chassis re-measured live.** HEAD df705f93 + the banked `candidate.c` body applied with its
two documented byte-neutral caller-side edits: `sandbox func_800770B8 --disable all` =
**3 / 175 / 175**, residual rows 62/63/64. 46 scoring builds this session plus three
`pwsh tools/grinder/dump.ps1 func_800770B8` runs; `src/text1b.c` restored to the pristine
HEAD copy after every sweep (verified `git status` clean at the end).

**PASS ATTRIBUTION (dumps read, not inferred) — the operand flip touches ONE insn.**
`.lreg` was dumped for the floor body (`base`) and for the int-domain operand-flip body
(`f2`, `(s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A)`). Diffing the whole
`;; Function func_800770B8` region of the two dumps gives **exactly one differing line**:

    base: (insn 188 (set (reg:SI 110) (plus:SI (reg:SI 109) (reg:SI 108))))   reload first
    f2:   (insn 188 (set (reg:SI 110) (plus:SI (reg:SI 108) (reg:SI 109))))   chain  first

(reg 108 = the `t0 * 10` chain, reg 109 = the second `lw %gp_rel(D_800A36A0)` reload,
reg 110 = the sum whose dest the target puts in the chain's register.) Every other insn and
the entire insn ORDER are identical. This kills the s31/s32-first-run attribution of the
24-row collateral to a "loop-head sched1 reschedule": sched1 emits the same order for both
bodies. The collateral is entirely `local-alloc`'s response to the swap — block_alloc's
tying loop (`tools/gcc-2.7.2/local-alloc.c:1240-1299`) now ties reg 110 to operand 1 = the
CHAIN, so the dest quantity merges into the chain quantity instead of the reload quantity,
and the resulting `qty_compare_1` priority order reseats the block ($v0<->$v1 on the chain
and the reload, $a0<->$a1 on t0 and the first load). **f2's RTL for insn 188 IS the
target's RTL**; only the hard registers come out swapped.

**u4 = 12/175/175 — the first body in 32 sessions with rows 54-64 byte-exact.** u4 is f2's
operand flip PLUS moving the D_800A35D0 store group (the 0x2/0x0 clears) ahead of the
group-A stores. Its 11 differing rows are ONLY the D group's own emission position (target
rows 49-53; u4 emits it at 38-39/43-46). From row 54 on it is the target instruction for
instruction and register for register, including the entire class-C block
`addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E`. Controls: **w10** (the same
D hoist WITHOUT the flip) = 15 and still carries the three class-C rows, so the hoist alone
does nothing for the tie; **x3** (the same hoist through its own local `dp`) = 12
identically, so `ptr` reuse is not the lever.

**Why u4 cannot be the final form.** The target's store order is A(0x10,0x8,0xC,0x14,0x3C)
-> D(0x2,0x0) -> C(0x42,0x40) -> S(0x68) — the floor body's order. GCC 2.7.2 will not
reorder a store through an unknown pointer past a store to a known symbol, so a D-first
source order is a D-first emission order. Hoisting only the D ADDRESS and leaving its
stores in place does not reproduce the allocation (x1 26, x2 38, w13 29, x4 27 at 172
insns), so it is the STORE position that moves the allocation, not the address-computation
position.

**Everything else measured this session (all 175/175 unless noted).** Group-order
permutations carrying the flip: ACDS 27, ADSC 27, ACSD 27, SADC 27, ASDC 36, DCAS 20,
DASC 14, CDAS 14, DACS 12 (= u4). Flip variants: `p_7e = p_6a + 10` 27, chain named in an
s32 local 27, constant folded into the chain 14 at 176, `(t0*5)*2` 27, flip on p_6a only
27, flip on p_7e only 3 (byte-inert — cse refolds it onto the p_6a form), pointer-domain
flip through a named `u8 *chain` local 27, p_6a/p_7e declaration order 29. Natural record
readings: `s16 *tbl = (s16 *)D_800A36A0; &tbl[t0*5+53]` = 7 but at **176** insns (it folds
the +0x6A/+0x7E constants BEFORE the base add, emitting two `addu` where the target emits
one `addu` and two `addiu`); the 0x6A-biased table = 43 at 176. Operand-order flips of the
OTHER address groups are byte-inert or near-inert on the floor body (A group 4, C group 4,
S store 3) and do not counteract the reseat when combined with the flip (all 27).

**Artifacts.** `tmp/grind/func_800770B8/s32/` — gen_u.py, gen_w.py, gen_x.py, gen_y.py,
gen_z.py (the 51 bodies in `v/`), apply.py, run.sh, rowdiff.sh, rows2.py, rowsall.py,
show.sh, slice.py, sweep.log, hdr2.txt, base.lreg, f2.lreg, u4.lreg, base.sched, f2.sched,
u4.sched.

- [s32b] Chassis re-verified live: floor body 3/175/175, i1 14/175/175 — both inherited numbers reproduce on HEAD df705f93.
- [s32b] base.lreg and f2.lreg differ in EXACTLY ONE insn across the whole function - insn 188's plus operand order - so the 24-row collateral of the operand flip is 100% local-alloc, not sched1. The s31/s32-first-run "loop-head sched1 reschedule" attribution is falsified.
- [s32b] f2's insn 188 `(set (reg 110) (plus (reg 108) (reg 109)))` IS the target's RTL for the class-C sum; only the hard-register assignment differs.
- [s32b] u4 (operand flip + the D_800A35D0 store group hoisted ahead of group A) = 12/175/175 and is byte-exact from row 54 through row 64, class C included - the first body in 32 sessions to emit rows 62/63/64 correctly at 175 insns.
- [s32b] CONTROL w10 (the D hoist alone, no flip) = 15 and still carries the three class-C rows; CONTROL x3 (the hoist through its own local) = 12 - so the hoist is a pure seat lever and `ptr` reuse is irrelevant.
- [s32b] Hoisting only the D ADDRESS (x1 26, x2 38, w13 29, x4 27 at 172 insns) does not reproduce u4's allocation: it is the D STORE position that moves it, and the target's store order is A,D,C,S.
- [s32b] The s16-table index reading `(s16 *)D_800A36A0` + `&tbl[t0*5+53]` costs an insn (176): it folds the field constants before the base add, so it emits two addu where the target emits one addu and two addiu.

- [s32] Chassis re-verified live on HEAD df705f93: the banked candidate body (with its two documented byte-neutral caller-side edits) = sandbox score 3, build_insns 175, target_insns 175; i1 re-measures 14/175/175. Both inherited numbers reproduce.

- [s32] base.lreg and f2.lreg differ in EXACTLY ONE line across the whole ';; Function func_800770B8' region: insn 188 is (set (reg 110) (plus (reg 109) (reg 108))) in the floor body and (set (reg 110) (plus (reg 108) (reg 109))) in the flip body. reg 108 = the t0*10 chain, reg 109 = the second lw %gp_rel(D_800A36A0) reload, reg 110 = the sum.

- [s32] The insn ORDER in the two .lreg dumps is identical, so sched1 plays no part in the operand flip's 24-row collateral. The s31 and s32-first-run attribution of that collateral to a 'rows 38-59 loop-head sched1 reschedule' is falsified.

- [s32] f2's insn 188 IS the target's RTL for the class-C sum; only the hard-register assignment comes out different ($v0<->$v1 on the chain and the reload, $a0<->$a1 on t0 and the first load).

- [s32] u4 (the operand flip plus the D_800A35D0 store group hoisted ahead of group A) = 12/175/175 and is byte-exact from row 54 through row 64 - addu $v0,$a0,$v1 / addu $a0,$a0,$a1 / addu $v1,$v1,$a1 / sh 0x42 / sh 0x40 / sb 0x68 / lw / sll / addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E. This is the first body in 32 sessions that emits rows 62/63/64 correctly at 175 insns.

- [s32] CONTROL w10 (the same D-store hoist WITHOUT the operand flip) = 15 and still carries the three class-C rows, so the hoist is purely a seat lever; CONTROL x3 (the hoist through its own local dp) = 12 identically, so `ptr` reuse is not the lever.

- [s32] Hoisting only the D ADDRESS computation and leaving the D stores in place does not reproduce u4's allocation (x1 26, x2 38, w13 29, x4 27 at 172 insns): it is the D STORE position that moves it, and the target's store order is A(0x10,0x8,0xC,0x14,0x3C) then D(0x2,0x0) then C(0x42,0x40) then S(0x68).

- [s32] The natural s16-table reading of the record ((s16 *)D_800A36A0 indexed by t0*5+53) costs one insn (176): it folds the field constants into the index before the base add, so it emits two addu where the target emits one addu and two addiu.

- [s32] Operand-order flips of the OTHER address groups are byte-inert or near-inert on the floor body (A group 4, C group 4, S store 3) and do not counteract the flip's reseat when combined with it (z5-z9 all 27).

- [s32] src/text1b.c was restored to the pristine HEAD copy after every sweep and after the dump runs; git status on src/ and include/ is clean at end of session.


## [s33] (structural, 2026-09-06) — the class-C seat is now a CLOSED-FORM predicate on GCC's qty table

**Chassis re-measured live.** HEAD 35950580 + the banked `candidate.c` body applied with its two
documented byte-neutral caller-side edits: `sandbox func_800770B8 --disable all` = **3 / 175 / 175**,
residual rows 62/63/64. 35 scoring builds + 12 `BB2_QTY_DEBUG` cc1 runs this session;
`src/text1b.c` restored to the pristine HEAD copy after every sweep (`git status` clean at end).

**OWNER DIRECTIVE, executed and answered.** The directive asked for (1) a re-measure of the class-B
prologue store-base spellings on the h3 chassis and (2) a record-layout audit for a genuine third use
of the C pointer. Both were already spent by s31/s32 on this exact chassis and the ledger records
them (s31 closed class B, floor 5 -> 3; s32 evidence closes the "third use / list-walk" audit against
`func_8006E49C` / `func_80076FF8` / `func_8006E950`, and measured the natural s16-record readings of
the 0x6A/0x7E table at 176 insns). This session therefore executed the standing frontier item 1 (add
a quantity to the loop-body block WITHOUT moving a store) and, in doing so, replaced the guesswork
with an exact criterion.

**THE CRITERION (read straight out of GCC's own tables, not inferred).** `local-alloc.c:1660`
`qty_compare_1` sorts blk=1's quantities by `floor_log2(refs)*refs*size / (death-birth)`, and
`find_free_reg` then hands out ascending first-free hard registers. `BB2_QTY_DEBUG=1` on the
instrumented `tools/gcc-2.7.2/cc1` prints the whole table. For the loop-body block (blk=1):

    floor body (score 3, 5 quantities)
      ord0 qty2 reg89  birth=12 death=14 refs=4   pri 4.00  got $v0
      ord1 qty4 reg110 birth=48 death=56 refs=10  pri 3.75  got $v0   <- dest MERGED WITH THE RELOAD
      ord2 qty3 reg108 birth=28 death=52 refs=16  pri 2.67  got $v1   <- the t0*4->t0*5->t0*10 chain
      ord3 qty1 reg100 birth=10 death=44 refs=12  pri 1.06  got $a0
      ord4 qty0 reg86  birth= 6 death=46 refs=14  pri 1.05  got $a1

    f2 = the operand flip (score 27, 5 quantities)
      ord0 qty2 reg89  birth=12 death=14 refs=4   pri 4.00  got $v0
      ord1 qty3 reg110 birth=28 death=56 refs=22  pri 3.14  got $v0   <- dest MERGED WITH THE CHAIN
      ord2 qty4 reg109 birth=48 death=52 refs=4   pri 2.00  got $v1   <- the reload, alone
      ord3 qty1 reg100 birth=10 death=44 refs=12  pri 1.06  got $v1
      ord4 qty0 reg86  birth= 6 death=46 refs=14  pri 1.05  got $a0

The RTL identities (from s32's `.lreg` read, re-confirmed here in `f2_fn.lreg`): insn 186 sets
reg109 = `(mem (symbol_ref D_800A36A0))` (the reload); insn 183 sets reg107 = reg93+reg86 with
`REG_EQUAL (mult reg86 5)`; insn 184 sets reg108 = reg107<<1 with `REG_EQUAL (mult reg86 10)`;
insn 188 sets reg110 = the plus. The chain QUANTITY is reg93(t0*4, 8 refs) + reg107(4) + reg108(4),
i.e. 16 refs born with the `sll t0,2`; the flip adds reg110's 6 refs to it (22) instead of to the
reload's 4 (10).

**WHAT THE TARGET NEEDS, stated exactly.** The target's rows 62-64 are
`addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E`: the dest is merged with the CHAIN
(so the flip is required) AND the merged quantity must get **$v1** while the reload gets **$v0**,
with reg100 -> $a0 and reg86 -> $a1 unchanged. Since `find_free_reg` is deterministic, f2's table
CANNOT produce that: the merged quantity is sorted second and takes the first free register.
The merged quantity must therefore either sort BELOW the reload, or be born early enough to
conflict with whatever already holds $v0. Both reduce to **merged.birth <= ~14** (with refs 22 and
death 56, `4*22/(56-birth) < 2.00` requires birth < 12).

**FOUR BODIES NOW PRODUCE THE TARGET'S CLASS-C SEATS (all 175/175).** a2 (26), b5 (26), b6 (26) and
**e3 (26)** all have merged `birth=12 death=56 refs=22 -> got $v1` and reload `48-52 -> got $v0`:

    e3 (score 26, 6 quantities)
      ord0 qty4 reg90  birth=18 death=20 refs=4   got $v0
      ord1 qty2 reg111 birth=12 death=56 refs=22  got $v1   <- TARGET seat
      ord2 qty5 reg110 birth=48 death=52 refs=4   got $v0   <- TARGET seat
      ord3 qty3 reg101 birth=16 death=44 refs=12  got $a0
      ord4 qty1 reg87  birth= 8 death=46 refs=14  got $a1
      ord5 qty0 reg86  birth= 6 death=36 refs=10  got $a2   <- THE ONE EXTRA QUANTITY

**e3 is one quantity away from the whole function.** e3 is the floor body + the operand flip + a
single rename: the D_800A35D0 group is built through its own `u8 *dp` local instead of borrowing
`ptr`. Nothing is moved, no store changes position, the insn count stays 175. That rename alone
drags the chain quantity's birth from 28 to 12 and hands the class-C block the target's registers.
Its entire residual is the sixth quantity (reg86, birth 6, death 36, 10 refs) which takes $a2 and
pushes reg87/reg101 down a seat.

**The two-statement address build is the lever, not the name.** `dp = (u8 *)&D_800A35D0;
dp = (t0 * 4) + dp;` (e3, 26) and `dp = (u8 *)&D_800A35D0; dp = dp + (t0 * 4);` (f3, 26) both work;
folding it to one statement `dp = (u8 *)&D_800A35D0 + (t0 * 4);` (f1) loses the early birth and
scores 38. Declaring `dp` before `ptr` (f2x) is byte-identical to e3 at 26.

**Everything else measured this session (175/175 unless noted).** Extra-quantity probes on the flip
body that do NOT move the birth: separate C-group local `cp` (a1 23, its quantity is birth 36
death 40 refs 6 = pri 3.00, just under the merged 3.14 so it sorts second and changes nothing);
`cp` hoisted above the D group (b4 23); `cp` computed as the first statement (d5 23); named `t4`
shared by the D and C groups (a3 27, table identical to f2); named `t2` (a4 47); named `dbase`
(a9 29); `cp`+`t4` (a10 23); the 0x5C/0x60 pair through a record pointer (a11 47 at 174, b8 28);
A-group stores addressed straight off `base` (e6 27). Insn-count failures: shared `c10` (a5 171),
`base` carrying the 0x5C/0x60 pair (a6 47 at 174), `base` carrying the chain (a7 39 at 174), both
(a8 62 at 173), a12 44 at 174, c3 49 at 174. Controls on the FLOOR body: `cp` alone is byte-inert
(c1 3); all three pointers named costs 28 (c2); the s16-record D-group reading costs 33 (b10).
Best score of the session: **b1 = 18** (cp + the D group read as `s16 *dp = (s16 *)&D_800A35D0 +
(t0*2); dp[1]=0; dp[0]=0;`), but it is not on the path - LICM hoists the `lui/addiu %hi/%lo
(D_800A35D0)` pair out of the loop entirely (rows 30/31 become `lui $t4` / `addiu $t4`), which the
target does not do.

**Artifacts.** `tmp/grind/func_800770B8/s33/` - gen_a.py, gen_b.py, gen_d.py, gen_e.py, gen_f.py
(the 34 bodies in `v/`), apply.py, run.sh, mismatch.sh, rows2.py, rowsall.py, onlydiff.py,
qty.sh, qtydbg.py, sweep.log, `*.qty` (12 BB2_QTY_DEBUG tables), f2_fn.lreg, rows_a1.txt.

- [s33] Chassis re-verified live on HEAD 35950580: banked candidate body = 3/175/175, f2 = 27/175/175.
- [s33] The class-C hard-register decision is a closed-form predicate: in blk=1 the merged chain+dest quantity must sort below the reload (or be born early enough to conflict with the current $v0 holder), which with refs=22 and death=56 means birth <= ~12 instead of the flip body's 28.
- [s33] BB2_QTY_DEBUG=1 on tools/gcc-2.7.2/cc1 prints qty_compare_1's whole input and output per block (QTYDBG blk/ord/qty/reg1/birth/death/refs/got); tmp/grind/func_800770B8/s33/qty.sh + qtydbg.py drive it for one named body and it is far cheaper than scoring for triaging a seat question.
- [s33] Four bodies (a2, b5, b6, e3) now emit the target's class-C registers ($v1 for the merged chain+dest, $v0 for the reload) at 175 insns; e3 does it with the smallest possible source change.
- [s33] e3 = floor body + operand flip + the D_800A35D0 group built through its own `u8 *dp` local instead of borrowing `ptr`. No store moves, no insn added, score 26, and its ENTIRE residual is one extra quantity (reg86, birth 6, death 36, refs 10) taking $a2.
- [s33] The lever is the TWO-STATEMENT address build, not the name: `dp = (u8 *)&D_800A35D0; dp = (t0*4) + dp;` (26) and `dp = ...; dp = dp + (t0*4);` (26) both move the birth to 12, while the one-statement fold `dp = (u8 *)&D_800A35D0 + (t0*4);` scores 38.
- [s33] Adding a short quantity in the (28,48) window does NOT work unless its priority beats the merged quantity's 3.14: the `cp` local's quantity measures pri 3.00 (birth 36, death 40, refs 6) and sorts second, leaving the merged quantity first (a1/b4/d5 all 23).
- [s33] Declaring an address local as the loop body's FIRST statement does not move the chain quantity's birth (d1 t4 27 birth 28, d5 cp 23 birth 28, d3 dp 38 birth 26, d4 38, d6 38).
- [s33] b1 (cp + the s16[2][2] record reading of D_800A35D0) is the session's best score at 18/175/175 but is off-path: it lets LICM hoist the D_800A35D0 %hi/%lo pair out of the t0 loop, which the target keeps inside.

- [s33] Chassis re-verified live on HEAD 35950580: the banked candidate body (with its two documented byte-neutral caller-side edits) = 3/175/175, residual rows 62/63/64; the operand-flip body f2 = 27/175/175.

- [s33] BB2_QTY_DEBUG=1 on the instrumented tools/gcc-2.7.2/cc1 prints qty_compare_1's entire input and output per basic block (QTYDBG blk/ord/qty/reg1/birth/death/refs/got). tmp/grind/func_800770B8/s33/qty.sh + qtydbg.py drive it for one named body; it is one cc1 run, cheaper than a scoring build, and it is the correct triage tool for any remaining register-seat question on this function.

- [s33] The class-C residual is a closed-form predicate: with the operand flip present the merged chain+dest quantity has refs 22 and death 56, so floor_log2(22)*22/(56-birth) < 2.00 (the reload's priority) requires birth < 12. The flip body has birth 28.

- [s33] Four bodies (a2, b5, b6, e3) now show merged birth=12 death=56 refs=22 -> $v1 and reload birth=48 death=52 refs=4 -> $v0, which is the target's assignment for rows 62-64, all at 175 insns.

- [s33] e3 - the floor body plus the operand flip plus building the D_800A35D0 group through its own u8 *dp local instead of borrowing ptr - scores 26/175/175 and its ENTIRE residual is one extra quantity (reg86, birth6 death36 refs10) taking $a2 and pushing reg87/reg101 down one seat.

- [s33] The lever is the two-statement address build, not the name: dp = (u8 *)&D_800A35D0; dp = (t0 * 4) + dp; scores 26 and dp = (u8 *)&D_800A35D0; dp = dp + (t0 * 4); scores 26, while the one-statement fold dp = (u8 *)&D_800A35D0 + (t0 * 4); scores 38. Declaration order (dp before ptr) is byte-inert.

- [s33] Adding a short quantity in the (28,48) window is not sufficient on its own: the cp local's quantity measures pri 3.00 (birth36 death40 refs6) against the merged quantity's 3.14, so it sorts second (a1/b4/d5/a10 all 23).

- [s33] OWNER DIRECTIVE executed: the class-B prologue store-base spellings and the record-layout audit for a third use of the C pointer were both already spent on this exact chassis by s31 (class B closed, floor 5 -> 3) and s32 (func_8006E49C / func_80076FF8 / func_8006E950 show no list-walk shape; the natural s16-table readings of the 0x6A/0x7E record cost 176 insns). This session ran the standing frontier item 1 instead and replaced its guesswork with the exact criterion above.

- [s33] src/text1b.c was restored to the pristine HEAD copy after every sweep and after the qty runs; git status on src/ and include/ is clean at end of session.

## [s34] (synthesis, 2026-09-06) — the class-C seat is reached WITHOUT the operand flip, and "extra local = extra quantity" is read out of local-alloc.c

**Chassis re-measured live.** HEAD de9606ac; `src/text1b.c` byte-identical to the s33 pristine
copy. Banked `candidate.c` body (floor body F + the class-B dead store + the do-while(0) prologue
fence) applied with its two documented byte-neutral caller-side edits:
`sandbox func_800770B8 --disable all` = **3 / 175 / 175**, residual rows 62/63/64 only. The
operand-flip body f2 re-measured 27, e3 (s33's best-shaped body) re-measured 26. 21 scoring
builds + 19 `BB2_QTY_DEBUG` cc1 runs + 2 `-da` dump runs this session; `src/text1b.c` restored to
the pristine HEAD copy after every sweep (`git status` on src/ and include/ clean at end).

**KILL RE-AUDIT, executed.** The instance kill closest to the target was s33's e3 (26, the
target's class-C registers at 175 insns, entire residual = one extra quantity). It was re-measured
on this chassis (still 26) and then resolved FORENSICALLY rather than re-guessed: `dump.py e3`
+ `slice.py` give `tmp/grind/func_800770B8/s34/e3_fn.lreg`, in which **reg86 IS `dp` itself** —
insn 137 `(set (reg/v:SI 86) (symbol_ref D_800A35D0))`, insn 145 `(set (reg 86) (plus (reg 94)
(reg 86)))`, dead at insn 151. sched1 hoists 137/143/145 to the TOP of the loop-body block (the
.lreg insn chain is post-sched1), which is what drags the chain's birth from 28 to 12 — and is
also why `dp` is live in $a2 across the whole A-store group, which the target never is. So e3's
residual is not a bookkeeping artefact: it is a real extra live value.

**WHY AN EXTRA LOCAL USUALLY MEANS AN EXTRA QUANTITY (local-alloc.c:472).** `local_alloc` sets
`reg_qty[i] = -2` (eligible for a block quantity) only for pseudos with
`reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`. That is why the floor body's `ptr` (4 defs ->
3 REG_DEAD notes) never appears in the blk=1 table at all: it is punted to global-alloc. `dp` in
e3 has exactly ONE death (the intermediate value is consumed by a set of the same reg, so no
REG_DEAD note is emitted at insn 145), which is precisely why it earns the sixth quantity.

**h1 KILLS THE s33 FRONTIER'S ITEM 1.** h1 = e3 + `dp` given a second live range in a later block
(the 0x5C/0x60 pair spelled through `dp`). Its blk=1 table is EXACTLY the shape s33 predicted
would be a score-0 body — five quantities, merged chain+dest birth12 death56 refs22 -> $v1,
reload 48-52 -> $v0, reg101 -> $a0, reg87 -> $a1 — and it scores **42**. Removing `dp` from
local-alloc does not remove the VALUE: global-alloc still seats it in $a2 across the A group,
rows 38-43 and 55/56 are unchanged from e3, and the 0x5C/0x60 block gets worse. h4 (same trick
plus the tail pointer through `dp`) has the identical table and scores 50. **The qty table is
necessary, not sufficient: a body can hold the target's whole local-alloc assignment and still be
42 points away.**

**THE SESSION'S REAL FINDING — the class-C seat is reachable with NO flip and NO new quantity.**
Two independent families were found, both starting from the floor body:

1. **p2 (24/175/175) — name the second `D_800A36A0` read in a local taken BEFORE the sb store.**
   `u8 *rb;` ... `rb = D_800A36A0;` placed between the C-group stores and
   `*(u8 *)(base + t0 + 0x68) = (u8)t0;`, with `p_6a`/`p_7e` built off `rb`. That moves the
   reload's birth from 48 to 42, which drops the merged dest+reload quantity's qty_compare_1
   priority from 3.75 to 30/14 = 2.14, below the chain's 2.67. The chain therefore sorts FIRST,
   takes $v0, and the dest+reload quantity conflicts with it and takes $v1 — so **rows 62, 63 and
   64 come out BYTE-EXACT (`addu $v1,$v1,$v0` / `addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E`) for
   the first time on a body with no operand flip, no extra quantity and no FAKE construct.** Its
   whole 24-point residual is a SECOND, much smaller tie: inserting the reload ahead of the sb
   pushes both `reg87` (sign-extended t0, birth6) and `reg101` (base + base+t0) two luids later,
   and their priorities become 3*14/42 = 1.000 and 3*12/36 = 1.000 — an EXACT tie that
   qty_compare_1 breaks by quantity index, handing $a0 to t0 and $a1 to base+t0, i.e. the mirror
   image of the target. In the floor body the same pair is 42/40 vs 36/34 = 1.05 vs 1.06 and
   base+t0 wins $a0.

2. **s2 (13/175/175) / s4 (12/175/175) — move the D_800A35D0 group ahead of the A group and take
   the second `D_800A36A0` read into the EXISTING `ptr` local.** Because `ptr` has three deaths it
   is invisible to local-alloc, so the dest is no longer merged with the reload; combine_regs ties
   it to the chain instead, and with the D group first the chain is born at 12. blk=1 then holds
   only FOUR quantities: merged chain+dest birth12 death56 refs22 -> **$v1**, reg100 -> $a0,
   reg86 -> $a1, and the short t0*4 temp -> $v0. Rows 54 through 64 are byte-exact except row 62's
   operand order, and adding the flip (s4) fixes that too. The entire remaining residual is the D
   group's own emission position (rows 38-53: the target materialises %hi/%lo(D_800A35D0) at rows
   49/50, between the A stores and the D stores).

**Everything else measured this session.** h2 (dp reused for the C group) 27, h3 (dp reused for
the sb address) 32, h5 43; p3 (reload named before the C group, birth 36) 38, p4 (reload named
before the D group) 39 at 174 insns, p6 (reload named immediately before the p_6a block — birth
stays 48) **3, byte-inert**, p7 (sb also taken through rb) 27, p8 (reload named immediately AFTER
the sb — birth stays 48) **3, byte-inert**, p9 (reload named between the two C stores) 38, p10
(p8 with rb declared before base) 3; q1 (sb moved ahead of the C group) / q2 / q3 tables all
wrong; r1 (p2 + sb ahead of the C stores) 5; q5/q6/q7 (D group after the sb, D+C after the sb,
D and C swapped) leave the chain's birth at 26-28; r8a/r8b/r8c (reload into `ptr`, three
placements) all 27 with merged chain+dest at birth 28; s1 (D group first, reload untouched) 15;
s3 (r8 + e3's dp) 27.

**Artifacts.** `tmp/grind/func_800770B8/s34/` — apply.py, run.sh, rows.sh, qty.sh, qtydbg.py,
dump.py, slice.py, rowsall.py, onlydiff.py, gen_h.py, gen_p.py, gen_p2.py, gen_q.py, gen_r.py,
gen_s.py, gen_t.py, v/ (24 bodies), sweep.log, `*.qty` (19 BB2_QTY_DEBUG tables), e3_fn.lreg,
base_fn.lreg, rows_e3.txt, rows_base.txt, rows_p2.txt, rows_h1.txt, rows_s2.txt.

- [s34] Chassis re-verified live on HEAD de9606ac: the banked candidate body = 3/175/175 (residual rows 62/63/64), f2 = 27, e3 = 26.
- [s34] local-alloc.c:472 gates block quantities on `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`: that is why the floor body's multi-def `ptr` never appears in the blk=1 qty table, and why any freshly named single-death local (e3's `dp`) adds one.
- [s34] e3's sixth quantity is `dp` itself (e3_fn.lreg insns 137/145/151); sched1 hoists the whole dp chain to the top of the loop-body block, which is what moves the merged quantity's birth to 12 AND what leaves &D_800A35D0+t0*4 live in $a2 across the A stores - a value the target never has live.
- [s34] KILLED: giving `dp` a second live range so local-alloc.c:472 refuses it a quantity (h1, h4) produces EXACTLY the qty table s33 predicted to be score 0 and measures 42 and 50 - global-alloc still seats the value in $a2. A correct blk=1 quantity table is necessary but not sufficient.
- [s34] p2 (24/175/175): naming the second D_800A36A0 read in a local taken BEFORE the sb store moves the reload's birth 48 -> 42, drops the dest+reload quantity below the chain in qty_compare_1, and emits target rows 62/63/64 BYTE-EXACT with five quantities, no operand flip and no FAKE construct.
- [s34] p2's residual is a second qty_compare_1 tie: the inserted reload pushes reg87 (t0) and reg101 (base + base+t0) two luids later, making their priorities exactly 1.000 and 1.000, so the index tiebreak swaps $a0/$a1 against the target. In the floor body the same pair is 1.05 vs 1.06 and base+t0 wins $a0.
- [s34] The reload's position is a THREE-slot window: before the C group = birth 36 (p3, 38), between the C stores and the sb = birth 42 (p2, 24), after the sb or immediately before the p_6a block = birth 48 and byte-inert (p8/p6/p10, all 3).
- [s34] s2 (13/175/175) and s4 (12/175/175): moving the D group ahead of the A group and taking the second D_800A36A0 read into the existing multi-death `ptr` local gives blk=1 only FOUR quantities with the merged chain+dest at birth 12 seated in $v1 - the target's assignment - and rows 54-64 byte-exact (row 62's operand order needs the flip, which s4 supplies). The residual is entirely the D group's emission position.
- [s34] r8a/r8b/r8c: routing the second D_800A36A0 read through `ptr` alone makes combine_regs tie the dest to the CHAIN instead of the reload without any source-level operand flip (merged refs 22), but with the D group left in place the birth stays 28 and all three placements score 27.
- [s34] src/text1b.c was restored to the pristine HEAD copy after every sweep; git status on src/ and include/ is clean at end of session.

- [s34] Chassis re-verified live on HEAD de9606ac: the banked candidate body (with its two documented byte-neutral caller-side edits) = 3/175/175, residual rows 62/63/64; the operand-flip body f2 = 27; s33's e3 = 26.

- [s34] local-alloc.c:472 gates block quantities on `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`. That is why the floor body's four-times-assigned `ptr` never appears in the blk=1 qty table (it is punted to global-alloc), and why any freshly named single-death local adds one quantity.

- [s34] e3's sixth quantity is `dp` itself: tmp/grind/func_800770B8/s34/e3_fn.lreg insn 137 sets reg86 from (symbol_ref D_800A35D0), insn 145 adds reg94 (t0*4) into it, insn 151 kills it. The .lreg insn chain is post-sched1, so sched1 is the pass that hoists the dp chain to the top of the loop-body block -- that hoist is both what moves the merged quantity's birth to 12 and what leaves the D address live in $a2 across the A stores.

- [s34] A correct blk=1 quantity table is necessary but not sufficient: h1 and h4 hold exactly the table s33 predicted would be score 0 and measure 42 and 50.

- [s34] p2 = 24/175/175 is the first body in 34 sessions to emit target rows 62/63/64 byte-exact with no operand flip, no extra quantity and no new FAKE construct; the lever is purely WHERE the second D_800A36A0 read is taken.

- [s34] p2's residual is one exact qty_compare_1 tie: reg87 (t0) 3*14/42 = 1.000 vs reg101 (base + base+t0) 3*12/36 = 1.000, broken by quantity index in favour of t0, swapping $a0/$a1 against the target. The floor body's same pair is 1.05 vs 1.06.

- [s34] The named-reload placement window is one source slot wide: birth 36 (before the C group) 38, birth 42 (between the C stores and the sb) 24, birth 48 (after the sb, or immediately before the p_6a block) byte-inert at 3.

- [s34] s2 = 13/175/175 and s4 = 12/175/175: the D group moved ahead of the A group plus the second D_800A36A0 read taken into `ptr` gives blk=1 only FOUR quantities, the merged chain+dest born at 12 and seated in $v1, and rows 54-64 byte-exact (row 62's operand order needs the flip, which s4 supplies). The residual is entirely the D group's emission position.

- [s34] Routing the second D_800A36A0 read through `ptr` alone (r8a/r8b/r8c, 27) reproduces the operand flip's combine_regs effect -- the dest ties to the chain rather than the reload -- in ordinary C with no flip written.

- [s34] src/text1b.c was restored to the pristine HEAD copy after every sweep; git status on src/ and include/ is clean at end of session.

## [s35] (synthesis, 2026-09-06) — the floor stream's quantity table is invariant; the seat is winnable, the schedule is what it costs

**Chassis.** HEAD b90bdce4. `git log --name-only` shows no `src/` change between de9606ac (s34's
chassis) and b90bdce4 — the two ledger commits touch only `memory/`, `docs/` and `metrics/`, so the
chassis is byte-identical to s34's. Re-measured with s34's harness copied to
`tmp/grind/func_800770B8/s35/`: floor body `base` = **3/175/175**, `p2` = 24, `s2` = 13, `s4` = 12,
`r8a` = 27, `r1` = 5. Every s34 score reproduces exactly.

**KILL RE-AUDIT (mandated).** `python3 tools/fake_ablate.py --func func_800770B8 --file text1b
--candidate memory/grind/func_800770B8/candidate.c`: keep-all **3**, drop the `p_old` dead store
**5**, drop the empty `do { } while (0)` prologue wrap **8**, drop both **10** — all at 175 insns.
Both FAKE constructs are load-bearing on the current chassis, and no banked kill was measured with a
FAKE carrier sitting on the class-C pseudos (the two FAKE units live in the prologue block, blk=0).

**OWNER DIRECTIVE.** The 2026-09-06 queue directive (class-B prologue store-base spellings on the h3
chassis, then the record-layout audit for a third use of the C pointer) was already executed and
measured: s31 ran 26 builds of the class-B store-base spellings on BOTH the floor body and the h3
chassis and CLOSED class B (floor 5 -> 3, hypotheses.md [s31]); s32 ran the record-layout audit
(func_8006E49C / func_80076FF8 / func_8006E950 show no list-walk shape; the natural s16-table
readings of the 0x6A/0x7E record cost 176 insns) and s33 re-confirmed both (evidence.md:4179). This
session therefore worked the standing frontier, as s33 did.

### [s35] The blk=1 local-alloc quantity table is INVARIANT on the floor insn stream
Twelve byte-neutral spellings measured this session, each dumped with `BB2_QTY_DEBUG=1`, produce the
IDENTICAL blk=1 table (only the pseudo numbers move):

    qty0  t0 (sign-extended)        birth 6   death 46  refs 14   -> $a1
    qty1  base (+ base+t0 merged)   birth 10  death 44  refs 12   -> $a0
    qty2  the D-group t0*4 temp     birth 12  death 14  refs 4    -> $v0
    qty3  the t0*4->t0*5->t0*10 chain birth 28 death 52 refs 16   -> $v1
    qty4  the dest + the D_800A36A0 reload birth 48 death 56 refs 10 -> $v0

The spellings: `base` defined after `a2 = 0` (a1); the sb address named early in its own local (a3);
the sb address re-associated as `base + (t0 + 0x68)` (a4); the sb store moved past the record-pointer
computations (a6); the sb value spelled `(u8)(t0 + 0)` (a7); the sb spelled as the array subscript
`base[t0 + 0x68]` (b4); `s32 q4 = t0 * 4;` named ahead of the A group and used by both the D and the C
group (c1) or by the C group only (c4); `p_7e = p_6a + 10` (d1); a named `u8 *q = D_800A36A0 + t0*10`
feeding both record pointers (d2). All score 3 at 175/175 — byte-inert. s34's p6/p8/p10 (three
placements of a named reload AFTER the sb) are the same table. **Births, deaths, refs and the merge
membership are not reachable by source spelling while the insn stream is the target's.**

### [s35] Closed form: on the target's insn stream the dest+reload merge CANNOT take $v1
The class-C seat requires qty3 (chain, `floor_log2(16)*16/24` = 2.667) to sort ahead of qty4
(dest+reload, `floor_log2(10)*10/8` = 3.75) in `qty_compare_1`
(`tools/gcc-2.7.2/local-alloc.c:1660`). qty4's death 56 is the LAST insn of blk=1 (the `p_7e` addiu;
the block ends at the inner do-while's head), so the death cannot be pushed later by any body — the
span can only grow at the birth end. qty4 needs pri <= 2.667, i.e. span >= 12, i.e. birth <= 44; and
refs is invariant at 10 (above), so the refs route (needs <= 7) is closed too. Birth 48 -> 44 means
the reload `lw` is emitted two insns earlier, which is itself a differing row. **Therefore no body
whose insn stream equals the target's can reach the class-C seat with the dest merged onto the
reload. The target's structure has to be the one s2/r8 exhibit: the dest merged with the CHAIN, the
reload carried by a multi-death local that `local-alloc.c:471`'s `reg_n_deaths == 1` gate refuses a
quantity.** This is why every "make the floor body's registers come out right" probe since s27 has
failed: the floor body's merge structure is the wrong one, not a mis-priced one.

### [s35] Closed form: the p2 family cannot hold both seats
For any placement of the named reload BEFORE the sb store, every insn from the lw's new slot onward
shifts later by k luids, k a positive multiple of 2 (one insn = 2 luids). base keeps the $a0 seat iff
`36/(34+k) > 42/(40+k)` <=> `12 > 6k` <=> k < 2; qty4 needs birth <= 44 <=> k >= 2. The two
requirements are disjoint. Measured, exactly as predicted: k=0 (p6/p8/p10) byte-inert 3; k=2 (p2) 24
with rows 62/63/64 byte-exact and the $a0/$a1 pair inverted; k=4 (p3/p9) 38.

### [s35] f3 — the first body holding ALL FOUR target seats
`f3` = the floor body with (i) the sb store moved ahead of the C-group stores and (ii) `u8 *rb;
rb = D_800A36A0;` immediately before the moved sb, with the record pointers built off `rb`. blk=1:
t0 b6 d48 refs14, base b10 d42 refs12, temp b12 d14, chain b28 d52 refs16, dest+reload **b38** d56
refs10 -> 30/18 = 1.67 < 2.667. Allocation order: temp $v0, **chain $v0**, **dest+reload $v1**,
**base $a0**, **t0 $a1** — the target's four seats, all of them, at 175/175 insns.
**Score 34.** The price is entirely schedule: sched1 hoists the lw to luid 38 and the sb move costs
f1's two rows (`f1` = the sb move alone = 5). The seat is winnable on a near-floor body; what is not
winnable is winning it without moving the lw.

### [s35] Two refs-raising routes measured and priced
Addressing the tail 0x5C/0x60 pair off the loop-body `base` (b1) or off the named reload `rb` (b2)
raises the base/reload quantity's refs but DELETES the second `D_800A36A0` reload: 174 insns, scores
33 and 47. Routing the per-player counter through a named `s32 *cnt = &sp[t0];` (b3) scores 32.
Addressing the five A-group stores straight off `base` (a5) restructures blk=1 into a different
five-quantity table and scores 31.

### [s35] The s2 residual is not payable by hoisting the address alone
`e1` (D address computed into `ptr` ahead of the A group, A group through a second local `ap`, D
stores back in their target position) and `e2` (same, A group addressed straight off `base`) both
score **29** at 175/175 — worse than s2's 13. The early D address is a second value live across the A
stores, which is the h-series failure with the roles reversed: blk=1 comes out with the merged
chain+dest at b12 d56 refs22 (the s2 shape) but the A pointer becomes its own long quantity
(reg86 b18 d30 refs16). **The s2 family's 13-point residual is the D STORES' position, and it cannot
be paid by moving only the address.**

- [s35] Chassis re-confirmed at HEAD b90bdce4: floor body 3/175/175, p2 24, s2 13, s4 12, r8a 27, r1 5 — every s34 score reproduces; git log --name-only shows no src/ change between de9606ac and b90bdce4.

- [s35] KILL RE-AUDIT: tools/fake_ablate.py on candidate.c gives keep-all 3, drop the p_old dead store 5, drop the empty do-while(0) prologue wrap 8, drop both 10, all at 175 insns — both FAKE constructs are load-bearing on the current chassis and neither sits on a class-C pseudo (both live in blk=0).

- [s35] The floor stream's blk=1 quantity table (t0 b6 d46 refs14 -> $a1; base b10 d44 refs12 -> $a0; D-group t0*4 temp b12 d14 refs4 -> $v0; chain b28 d52 refs16 -> $v1; dest+reload b48 d56 refs10 -> $v0) is reproduced byte-for-byte by twelve distinct byte-neutral spellings.

- [s35] The D-group t0*4 temp dies at luid 14 in every measured body, including bodies that name the shift and hand the same named value to the C group — the C group always gets its own shift, so the temp can never be made to conflict with the chain.

- [s35] f3's QTYDBG order is (temp $v0, chain $v0, dest+reload $v1, base $a0, t0 $a1): all four target seats on a 175/175 body, price 34 points of schedule.

- [s35] Addressing the tail 0x5C/0x60 pair off any live pointer (base or the named reload) deletes the second D_800A36A0 reload and builds 174 insns (b1 33, b2 47), so the refs-raising route on the base quantity is priced out.

- [s35] The owner's 2026-09-06 directive (class-B prologue store-base spellings on the h3 chassis, then the record-layout audit for a third use of the C pointer) was already executed and measured by s31 (26 builds, class B closed, floor 5 -> 3) and s32/s33 (no list-walk shape in the siblings; the natural s16-table readings cost 176 insns); this session worked the standing frontier instead, as s33 did.

## [s36] (synthesis, 2026-09-06) — the row-62 operand order is `expr.c:5287`, and the class-C seat is winnable by CONFLICT (all four seats, no FAKE, D stores unmoved)

**Chassis.** HEAD 2ac3e85d (`src/text1b.c` byte-identical to the s35 pristine copy; the two ledger
commits since de9606ac touch only `memory/`, `docs/` and `metrics/`). Re-measured with s35's harness
copied to `tmp/grind/func_800770B8/s36/`: floor body `base` = **3/175/175**, `s2` = 13, `s4` = 12,
`r8a` = 27, `f3` = 34 — every s35 score reproduces exactly. 29 scoring builds, 5 `BB2_QTY_DEBUG`
tables, 1 `-da` dump run, 3 row-diff runs. `src/text1b.c` restored to the pristine HEAD copy after
every sweep; `git status` on `src/` and `include/` clean at end of session.

**KILL RE-AUDIT (mandated), executed on the current chassis.** `python3 tools/fake_ablate.py --func
func_800770B8 --file text1b --candidate memory/grind/func_800770B8/candidate.c`: keep-all **3**,
drop the `p_old` dead store **5**, drop the empty `do { } while (0)` prologue wrap **8**, drop both
**10** — all at 175 insns, identical to s35's numbers. Both FAKE units remain load-bearing and both
sit in blk=0, so neither occupies a class-C pseudo.

**OWNER DIRECTIVE.** The 2026-09-06 queue directive (class-B prologue store-base spellings on the h3
chassis, then the record-layout audit for a third use of the C pointer) was executed and measured by
s31 (26 builds, class B CLOSED, floor 5 -> 3) and s32/s33 (no list-walk shape in func_8006E49C /
func_80076FF8 / func_8006E950; the natural s16-table readings of the 0x6A/0x7E record cost 176
insns), and re-confirmed by s35. This session therefore worked the standing frontier.

### [s36] THE MECHANISM FOR ROW 62 IS NAMED: `expr.c:5287`

36 sessions have treated the row-62 residual (target `addu $v1,$v1,$v0` vs our `addu $v0,$v0,$v1`)
as purely an allocation fact. It is first an EMISSION fact with an exact source in the compiler:

    expr.c:5286  /* Put a constant term last and put a multiplication first.  */
    expr.c:5287  if (CONSTANT_P (op0) || GET_CODE (op1) == MULT)
    expr.c:5288    temp = op1, op1 = op0, op0 = temp;

This swap lives in the `both_summands:` arm of `expand_expr`'s `PLUS_EXPR` case, which is reached
ONLY when `modifier == EXPAND_SUM || modifier == EXPAND_INITIALIZER` (expr.c:5238:
`if ((modifier != EXPAND_SUM && modifier != EXPAND_INITIALIZER) || mode != ptr_mode) goto binop;`)
— i.e. only when the sum is being expanded as the ADDRESS of a MEM. On that path `expand_expr` of a
`MULT_EXPR` by a constant returns a live `(mult reg const)` rtx, so `GET_CODE (op1) == MULT` holds
and the multiply is swapped to operand 0. On the ordinary `binop:` path the tree order survives, and
`c-typeck.c`'s `pointer_int_sum` puts the POINTER first unconditionally.

Both outcomes are visible in the floor body's OWN RTL from the SAME C shape
`D_800A36A0 + <shift> + <const>` (`tmp/grind/func_800770B8/s36/base_fn.rtl`, `base_fn.lreg`):

  * insn 188 `(set (reg 110) (plus (reg 109 = the D_800A36A0 reload) (reg 108 = t0*10)))` — the
    `p_6a`/`p_7e` pointer-variable initialisers, `binop:` path, POINTER first, giving row 62
    `addu $v0,$v0,$v1`;
  * insn 266 `(set (reg 140) (plus (reg 139 = t0*2) (reg 137 = the reload)))` — the
    `*(s16 *)(D_800A36A0 + t0*2 + 0x5C) = 0` store, EXPAND_SUM path, MULT first, giving row 86
    `addu $v1,$v1,$v0`, which already MATCHES the target byte-for-byte.

The target's row 62 has the multiply first, so the target's row-62 sum was expanded with the MULT
rtx alive as operand 1 — either on the address path, or from a tree whose operand 0 is the multiply.

### [s36] `&D_800A36A0[i]` is NOT an EXPAND_SUM lever (c-typeck rewrites it at tree level)

`build_unary_op`'s ADDR_EXPR case turns `&x[y]` into `x + y` as a TREE, so such an address never
reaches `expand_expr`'s EXPAND_SUM arm. Measured: `(s16 *)(&D_800A36A0[t0 * 10] + 0x6A)` (u3) = **3**,
byte-inert, identical to the floor body and to s35's `d2`. Pushing the constant inside the subscript
reassociates instead (`&D_800A36A0[(t0*10) + 0x6A]` u1/u2, and the s16-table view u5) and costs an
insn: **7 at 176**, emitting `addiu $v0,$v1,106` before the pointer add. Putting the whole address
inside the inner loop's MEM so LICM must hoist it (u6 `((s16 *)(D_800A36A0 + t0*10 + 0x6A))[a2]`,
u7 `*(s16 *)(... + a2 * 2)`) lets cse/LICM delete insns the target keeps: **44 at 172** and **45 at
171**. `p_7e = p_6a + 10` on the ADDR_EXPR body (u4) = 29.

### [s36] The 27-point "operand flip" family's residual is ONE ROTATED SEAT, not a broken schedule

s31/s32 recorded the cast-flip spellings as 27 "because they seat the reload and the shift the other
way round and reschedule the loop head". Measured here with the quantity table and the row diff, the
real state is far better than that reading. Five spellings all measure **27/175/175**: w1 (the
inline int-domain flip on both initialisers), w2 (`s32 q = (t0*10) + (s32)D_800A36A0;`), w3
(`u8 *q = (u8 *)((t0 * 10) + (s32)D_800A36A0);` with `p_6a = (s16 *)(q + 0x6A)` and
`p_7e = (s16 *)(q + 0x7E)`), w4 (pointer-typed shift), w5 (`s16 *q` plus `&q[0x35]`/`&q[0x3F]`).
w3's row 62 is `addu $v0,$v0,$v1` where **$v0 is the CHAIN** (row 61 `sll $v0,$v0,1`) and $v1 is the
reload (row 60 `lw $v1`): the flip HAPPENED and the dest is tied to the chain. Its blk=1 table
(`s36/w3.qty`) is

    ord0 qty2 reg89  b12 d14 refs4  -> $v0   (the short group-A temp)
    ord1 qty3 reg102 b28 d56 refs22 -> $v0   (merged chain + dest)      <-- target wants $v1
    ord2 qty4 reg109 b48 d52 refs4  -> $v1   (the D_800A36A0 reload, its OWN quantity)
    ord3 qty1 reg100 b10 d44 refs12 -> $v1   (base)
    ord4 qty0 reg86  b6  d46 refs14 -> $a0   (sign-extended t0)

Every seat is exactly one slot off the target, and the single cause is that the merged quantity
(`qty_compare_1` priority `floor_log2(22)*22/28` = 3.14, `tools/gcc-2.7.2/local-alloc.c:1660`) is
allocated second and NOTHING conflicting holds $v0 — the only earlier quantity, the b12/d14 temp,
is long dead by luid 28.

### [s36] x3 — ALL FOUR TARGET SEATS by CONFLICT, no FAKE, D stores left at rows 52/53

The floor body borrows ONE local `ptr` for the group-A, group-D and group-C pointers. Because that
local has four defs and three REG_DEAD notes, `local-alloc.c:471-472`
(`reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`) refuses it a block quantity and punts every one
of those pointers to global-alloc — which is exactly why no short quantity is alive at luid 28 to
deny the merged quantity $v0.

**x3** = w3 plus the five group-A stores written through their own single-death local `u8 *ap;`
(`ap = (u8 *)((t0 * 2) + (s32)base);` followed by five `*(s16 *)(ap + k) = 0;`). Score
**27/175/175**, and its blk=1 table is the target's assignment outright:

    ord0 qty3 reg86  b18 d30 refs16 -> $v0   (the group-A pointer, pri 4*16/12 = 5.33)
    ord1 qty1 reg103 b12 d56 refs22 -> $v1   (merged chain + dest)      <-- TARGET SEAT
    ord2 qty4 reg110 b48 d52 refs4  -> $v0   (the reload)               <-- TARGET SEAT
    ord3 qty2 reg101 b16 d44 refs12 -> $a0   (base)                     <-- TARGET SEAT
    ord4 qty0 reg87  b8  d46 refs14 -> $a1   (sign-extended t0)         <-- TARGET SEAT

and rows 54 through 64 come out BYTE-EXACT: `addu $v0,$a0,$v1` / `addu $a0,$a0,$a1` /
`addu $v1,$v1,$a1` / `sh $zero,0x42($v0)` / `sh $zero,0x40($v0)` / `sb $t0,0x68($a0)` /
`lw $v0,%gp_rel(D_800A36A0)($gp)` / `sll $v1,$v1,1` / **`addu $v1,$v1,$v0`** /
**`addiu $a3,$v1,0x6A`** / **`addiu $a1,$v1,0x7E`**. This is the FIRST body in 36 sessions to take
the class-C seat through a CONFLICT (a higher-priority quantity holding $v0 across the merged
quantity's live range) rather than through an early birth, and the first to do it with the
D_800A35D0 stores left in the target's own slot (rows 52/53). It carries no FAKE construct in the
loop body.

**x3's whole residual is one sched1 decision.** Splitting the group-A pointer out of `ptr` removes
the shared-pseudo anti-dependence that pinned the `&D_800A35D0` address below the group-A stores, so
sched1 hoists the `lui %hi(D_800A35D0) / addiu %lo / addu` triple from rows 49/50/51 up to rows
38/39/43, and the loop counter `a2` lands in $a3 instead of $a2. Rows 38-53 are the entire cost.

### [s36] Six schedule nudges do not move the hoist

On the x3 chassis: `ap` declared before `ptr` (y1) 27; the D-group address built pointer-first
`ptr = ptr + (t0 * 4)` (y2) 27; the C group written inline off `base` so `ptr` serves the D group
only (y3) 27; the group-A stores written inline off `base` with no local at all (y4, on the w3
chassis) 27. Adding a separate C-group local as well (x4) 26 — its quantity is b36 d40 refs6, pri
3.00, and it ALSO sorts above the merged quantity, giving the same four seats. The C-group local
alone (x6) 23; on the floor body without the flip (x5) 3, byte-inert. Without the flip, the
group-A local alone (x1) 29 and A+C (x2) 28 — their tables keep the dest merged with the RELOAD.
Combining `ap` with the r8 ptr-carried reload instead of the cast flip (z1 29, z2 28) does not
reproduce the merge.

- [s36] Chassis re-confirmed at HEAD 2ac3e85d: floor body 3/175/175, s2 13, s4 12, r8a 27, f3 34; fake_ablate keep-all 3 / drop the p_old dead store 5 / drop the do-while(0) wrap 8 / drop both 10, all at 175 insns.
- [s36] expr.c:5287 ("Put a constant term last and put a multiplication first") is the mechanism for row 62's operand order: it fires only in the both_summands: arm reached when modifier == EXPAND_SUM (expr.c:5238), i.e. only for a sum expanded as the address of a MEM; on the binop: path c-typeck's pointer_int_sum ordering (pointer first) survives. The floor body's own RTL shows BOTH outcomes from the same C shape: insn 188 (plus reload chain) for the p_6a pointer initialiser, insn 266 (plus t0*2 reload) for the 0x5C direct store, and row 86 already matches the target.
- [s36] &D_800A36A0[i] cannot reach the EXPAND_SUM path: c-typeck's build_unary_op rewrites &x[y] to x + y as a tree, so u3 = 3 byte-inert (identical to s35's d2); &D_800A36A0[i + c] reassociates and costs an insn (u1/u2/u5 = 7 at 176); putting the address in the inner loop's MEM (u6/u7) lets LICM/cse delete insns the target keeps (44 at 172, 45 at 171).
- [s36] The five cast-flip spellings (w1-w5, all 27/175/175) DO produce the target's operand order at row 62 with the dest tied to the chain; w3's blk=1 table shows all five seats rotated by exactly one slot because the merged chain+dest quantity (pri 3.14) is allocated second with nothing conflicting holding $v0.
- [s36] x3 (w3 + the five group-A stores through their own single-death local ap) = 27/175/175 with blk=1 = ap($v0) / merged chain+dest($v1) / reload($v0) / base($a0) / t0($a1) — ALL FOUR target seats, rows 54-64 byte-exact including addu $v1,$v1,$v0, no FAKE construct in the loop body, and the D_800A35D0 stores left at rows 52/53.
- [s36] The group-A pointer earns a block quantity only when it is single-death (local-alloc.c:471-472); its priority 4*16/12 = 5.33 beats the merged quantity's 3.14 and it conflicts with it, so $v0 is denied — the first CONFLICT-based route to the class-C seat (s2's route is an early birth, and that one costs the D stores' position).
- [s36] x3's entire residual is sched1: removing the shared-ptr anti-dependence lets sched1 hoist the lui %hi(D_800A35D0) / addiu %lo / addu triple from rows 49/50/51 to rows 38/39/43, and a2 lands in $a3. Six nudges (y1 declaration order 27, y2 D-address operand order 27, y3 C group inline 27, y4 A group inline 27, x4 +C local 26, x6 C local only 23) leave the hoist in place.
- [s36] src/text1b.c was restored to the pristine HEAD copy after every sweep; git status on src/ and include/ is clean at end of session.

- [s36] Chassis re-confirmed at HEAD 2ac3e85d: floor body 3/175/175, s2 13, s4 12, r8a 27, f3 34; fake_ablate keep-all 3 / drop the p_old dead store 5 / drop the do-while(0) prologue wrap 8 / drop both 10, all at 175 insns.

- [s36] expr.c:5287 ('Put a constant term last and put a multiplication first') is the mechanism for row 62's operand order; it fires only in the both_summands: arm that expr.c:5238 reaches when modifier == EXPAND_SUM, i.e. only for a sum expanded as the address of a MEM. On the binop: path c-typeck's pointer_int_sum ordering (pointer first) survives.

- [s36] The floor body's own RTL shows both outcomes from one C shape: insn 188 (plus reload chain) for the p_6a pointer initialiser and insn 266 (plus t0*2 reload) for the 0x5C direct store, whose row 86 already matches the target's addu $v1,$v1,$v0.

- [s36] &D_800A36A0[i] cannot reach the EXPAND_SUM path because c-typeck's build_unary_op rewrites &x[y] to x + y as a tree: u3 = 3 byte-inert; &D_800A36A0[i + c] reassociates and costs an insn (u1/u2/u5 = 7 at 176); the inner-loop MEM spellings let LICM/cse delete insns the target keeps (u6 44 at 172, u7 45 at 171).

- [s36] The five cast-flip spellings (w1-w5, all 27/175/175) DO produce the target's row-62 operand order with the dest tied to the chain; every seat is rotated by exactly one slot because the merged chain+dest quantity (qty_compare_1 priority 3.14, local-alloc.c:1660) is allocated second with nothing conflicting holding $v0.

- [s36] x3 (w3 + the five group-A stores through their own single-death local ap) = 27/175/175 with blk=1 = ap($v0) / merged chain+dest($v1) / reload($v0) / base($a0) / t0($a1): ALL FOUR target seats, rows 54-64 byte-exact including addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E, no FAKE construct in the loop body, and the D_800A35D0 stores left at rows 52/53.

- [s36] The group-A pointer earns a block quantity only when it is single-death (local-alloc.c:471-472); its priority 4*16/12 = 5.33 beats the merged quantity's 3.14 and it conflicts with it, so $v0 is denied. This is a CONFLICT route to the class-C seat, independent of s2/s4's early-birth route and without their D-store displacement.

- [s36] x3's entire residual is sched1: removing the shared-ptr anti-dependence lets the list scheduler hoist the lui %hi(D_800A35D0) / addiu %lo / addu triple from rows 49/50/51 to rows 38/39/43, and a2 lands in $a3. Nine forms (y1 27, y2 27, y3 27, y4 27, x4 26, x6 23, x1 29, x2 28, z1 29, z2 28) leave the hoist in place.

- [s36] The owner's 2026-09-06 directive (class-B prologue store-base spellings on the h3 chassis, then the record-layout audit for a third use of the C pointer) was executed and measured by s31 (26 builds, class B closed, floor 5 -> 3) and s32/s33 (no list-walk shape in the siblings; the natural s16-table readings cost 176 insns), and re-confirmed by s35; this session worked the standing frontier.

- [s36] src/text1b.c was restored to the pristine HEAD copy after every sweep; git status on src/ and include/ is clean at end of session, and candidate.c is unchanged (the floor body at 3).

# s37 (solver — 2026-09-06) — the D-triple hoist is one missing dependence edge, and that edge and the class-C quantity want opposite things from the same pseudo

**Chassis.** HEAD b5982c8f. `memory/grind/func_800770B8/candidate.c` applied to `src/text1b.c` with
the two documented byte-neutral caller-side edits (prototype `s32 func_800770B8(s32, s32, s32);`,
call site `(s32)&D_8009BD24`): `sandbox func_800770B8 --disable all` = **score 3, build_insns 175,
target_insns 175**. x3 and w3 both re-measure at 27/175/175, i.e. every s36 number is current and no
banked conclusion needed re-deriving.

## 1. The owner directive (class B on a differing chassis) is executed and negative

Eighteen builds: six spellings of the `func_8006E49C` result block x three chassis (floor, x3, w3).
`gen_k.py` writes them; `sweep.log` has the scores.

| spelling | floor chassis | x3 chassis | insns |
|---|---|---|---|
| base / x3 (the `/* FAKE */` dead store to `p_old` present) | **3** | **27** | 175 |
| k3 — dead store ABLATED, `p_old` reuse kept | 5 | 29 | 175 |
| k1 — returned pointer in its own `s32 *nw`, clears through the global | 23 | 47 | **170** |
| k2 — `nw` for the 0x4 store AND the clears | 23 | 47 | **170** |
| k4 — 0x4 store through the global, clears through `nw` | 23 | 47 | **170** |
| k5 — everything through the `D_800A36A0` read-back, no local | 23 | 47 | **170** |
| k6 — `nw` for the 0x4 store, a second named read-back `cur` for the clears | 23 | 47 | **170** |

The dead store is worth exactly 2 points on BOTH chassis (3->5, 27->29), and the family's internal
ordering is identical on both. The differing surrounding schedule the directive hoped would re-rank
these does not re-rank them.

The instruction count is the substantive finding. Every "freshly returned pointer" spelling builds
**170** instructions against the target's 175. `goal_from_tgt.py classify` on k1 names the five:

```
    ours only  : addiu #,#,-56  x1        target only: addiu #,#,-64   x1
    ours only  : addiu #,#,56   x1        target only: addiu #,#,64    x1
                                          target only: move #,#        x2
                                          target only: sw #,56(#)      x1
                                          target only: lw #,56(#)      x1
                                          target only: nop             x1
```

i.e. a FOURTH callee-saved register plus its save/restore pair and a 64-byte frame. The target keeps
`arg0` in `$s0` and `arg0 + 0x58` in `$s1` live at the same time (target rows 12 `addiu $s1,$s0,0x58`
and 16 `sw $s0,%gp_rel(D_800A35D8)($gp)`); k1 computes the pointer only after `arg0`'s last use and
reuses `$s0` for it (k1 row 16 `addiu $s0,$s0,88`). Re-using the single variable `p_old` across the
call is what holds all four callee-saved registers live. Banked at
`rejected/s37-classB-freshly-returned-pointer-own-local-170insn-score23.c` and
`rejected/s37-classB-FAKE-ablated-reuse-kept-175insn-score5.c`.

## 2. sched_solver: the hoist is ONE missing dependence edge

x3's sched1 model of blk=1 (27 insns) simulates **exact**, so the model is sound here.
`tmp/grind/func_800770B8/s37/blkdump.py` prints the block with UIDs, luids, dependences and each
UID's derived target position. The relevant UIDs are

* **137** `la $6,D_800A35D0` — no predecessors at all (a link-time constant), our index **1**, target index **12**
* **145** `addu $6,$3,$6` — our index 5, target index 13
* **134** `sh $0,60($2)` — the fifth and last group-A store, both at index 13/11

Searches (`solve4.sh`, fork of `tools/sched_solver/perturb.py`):

| goal | atoms | result |
|---|---|---|
| `--goal-before 137:134` | `luid,luid_move` (1053 atoms) | **no vector at depth 1** |
| `--goal-before 137:134` | all classes (1999 atoms) | **exactly two, and they are one edge**: `add_dep 137 <- 134 (true/data)` and `add_dep 137 <- 134 (anti-output kind 14)` |
| the full 27-UID target pick order | all classes | **no vector at depth 1** |

No `del_dep`, no `cost` change, and no statement move of any kind reaches the goal. The D triple sits
at the top of the block because nothing in the block can precede it, full stop.

## 3. The edge is spellable, it works, and it costs the quantity that motivated x3

The only truthful C source of `137 <- 134` is to assign the D address to the pseudo the group-A
stores read — i.e. write the group-A pointer local a second time. Ten bodies were built and scored:

| body | what | score | insns |
|---|---|---|---|
| m1 | groups A and D through the same local `ap` (the solver's edge, on the x3 chassis) | **23** | 175 |
| m7 | m1 + the group-C pointer in its own local | 23 | 175 |
| m10 | m1 + `t0 * 4` named ahead of the group-A stores | 23 | 175 |
| m4 | D group emitted after the C group | 25 | 175 |
| m2 | D group in a fresh single-death `dp` alongside `ap` | 26 | 175 |
| m5 | `ap` carrying groups A and C, `ptr` on D | 26 | 175 |
| m6 | D group interleaved between the 4th and 5th group-A stores | 29 | 175 |
| m3 | D address written inline, no local | 9 | **177** |
| m8 | the FLOOR body with groups A and D through `ap`, no cast flip | **3** | 175 |
| m9 | m8 + the named C pointer | **3** | 175 |

**m1's rows 38-64 are a clean `$v0`/`$v1` swap against the target with the D triple at rows 48/49/51**
(target 49/50/51). The hoist is gone — the first body in 37 sessions to kill it. But m8/m9 are
byte-inert at 3: strip the cast flip and "groups A and D through one local" IS the floor body under
another name. The hoist is present in exactly those bodies whose group-A pointer has ONE death
(x3 27, x4 26, x6 23, y1-y4 27, z1 29, z2 28, m2 26, m7 23, m10 23) and absent in exactly those whose
group-A pointer has TWO (base 3, m8 3, m9 3, m1 23, m5 26).

`local_alloc` grants a block quantity only at `reg_basic_block >= 0 && reg_n_deaths == 1`
(`tools/gcc-2.7.2/local-alloc.c:472`). So the second write that creates the scheduler edge is the
same second death that destroys the `ap` quantity (b18 d30 refs16, priority 4.0) which is the entire
reason x3 holds the target's four class-C seats. **Frontier item 1's conflict route is closed for the
D address as the carrier**: sched1 wants the A pointer written twice, local-alloc wants it written
once, and the D address cannot be both the second write and a separate quantity.

What is NOT closed: the edge does not have to come from the D address. ANY real program value written
into the A pointer's pseudo after the group-A stores and before the D address supplies `137 <- 134`
while leaving the D address free to be its own thing. Finding such a value is a record-layout
question about what else the function legitimately points at in that window — the owner directive's
frontier item 2, now with a precise specification instead of a hunch.

Banked forms: `rejected/s37-groupA-and-D-through-one-local-kills-hoist-but-classC-swaps-23.c`,
`rejected/s37-D-address-inline-no-local-177insn-score9.c`,
`rejected/s37-ap-carries-A-and-D-byte-inert-rename-of-floor-3.c`.

## 4. Tooling gap found and worked around (integration item, not applied)

`tools/sched_solver/goalmap.py:213-237` (`_macro_expand_counts`) counts any operand containing a `(`
as one object instruction, so `sb $4,D_8009BCE4($3)` — which GNU `as` expands to
`lui $at,%hi / addu $at,$at,$3 / sb` — is undercounted by two. With two such stores in this function
the checksum at `goalmap.py:279` raised a spurious "the sandbox object was built from a DIFFERENT
source state" error (169 text lines, estimate 171, real object 175) and the object-mode goal path —
the ONLY goal path available to an `INCLUDE_ASM`-routed function — was unusable. A patched fork is at
`tmp/grind/func_800770B8/s37/fork/goalmap.py`; with it the alignment runs clean
(`equal 151, replace 19, delete 2, insert 2, moved 3`). `tools/` is outside a grind session's
writable surface, so the fix is recorded here for the operator rather than applied.

- [s37] Chassis re-measured live this session: candidate.c = 3/175/175, x3 = 27/175/175, w3 = 27/175/175 on HEAD b5982c8f, so every s36 conclusion is chassis-current and nothing needed re-deriving.

- [s37] The floor body's ENTIRE residual, re-confirmed by a mnemonic-normalised row diff this session, is three instructions: row 62 addu $v0,$v0,$v1 against the target's addu $v1,$v1,$v0, and rows 63/64 reading $v0 instead of $v1.

- [s37] x3's residual, by the same diff, is the D_800A35D0 lui/addiu/addu triple emitted at rows 38/39/43 instead of 49/50/51, which pushes the a2 loop counter into $a3 and swaps a2/a3 for the rest of the function, plus one delay-slot nop at row 126.

- [s37] sched_solver's simulator reproduces x3's blk=1 pick order EXACTLY (baseline exact, 27 insns), so its negative results for this block are model-grade, not approximations.

- [s37] add_dep 137 <- 134 is the only depth-1 vector over all 1999 atoms for the goal 'hold the D triple below the last group-A store'; the 1053 spellable atoms (luid, luid_move) return nothing.

- [s37] m1 is the first body in 37 sessions to eliminate the D-triple hoist while keeping 175 instructions: its rows 38-64 are a clean $v0/$v1 swap against the target with the triple at 48/49/51.

- [s37] m8 and m9 (the same A+D-through-one-local geometry without the cast flip) are byte-inert at 3, proving the geometry IS the floor body's and that the hoist is a function of the group-A pointer's death count, not of x3's spelling.

- [s37] Every class-B 'freshly returned pointer' spelling builds 170 instructions; the target's extra five are a fourth callee-saved register (save/restore at 56($sp)), a 64-byte frame, two moves and a nop, all of which exist only because arg0 and arg0+0x58 are live simultaneously.

- [s37] TOOLING: tools/sched_solver/goalmap.py:213-237 (_macro_expand_counts) undercounts `<mem> $r,SYM(base)` by two (GNU as expands it to lui $at,%hi / addu $at,$at,base / <mem>), which made the checksum at goalmap.py:279 raise a spurious 'different source state' error and blocked the object-mode goal path -- the only goal path an INCLUDE_ASM-routed function has. A patched fork is at tmp/grind/func_800770B8/s37/fork/goalmap.py; tools/ is outside a grind session's writable surface so it is recorded, not applied.

## s38 (forensics, 2026-09-06) — the loop.c gate behind the floor body's pointer reuse

Chassis re-measured live: base 3/175, x3 27/175, w3 27/175, m3 9/177. `fake_ablate` on
candidate.c: keep-all 3, drop the `p_old` dead store 5, drop the `do { } while (0);` 8, drop both
10 — unchanged since s35.

### Sweep table (all scores from `tmp/grind/func_800770B8/s38/sweep.log`)

| body | what it is | score | insns |
|---|---|---|---|
| base | floor body F | **3** | 175 |
| x3 | group-A pointer split into a single-death `ap` | 27 | 175 |
| m3 | D address inline, no local (s37) | 9 | **177** |
| p1 | both D stores as subscripts of ONE inline s16 cast | 9 | **177** |
| p2 | `t0*4` named, address left inline in the store operand | 19 | **176** |
| p3 | p1 + the C group also inline | 10 | **177** |
| p4 | m3 with the second store as a constant displacement | 9 | **177** |
| q1 | `dp = (u8 *)&D_800A35D0 + (t0*4);` — ONE statement, fresh local | **18** | 175 |
| q2 | q1's lever on the floor chassis | 33 | 175 |
| q3 | inline D on the floor chassis | 35 | **177** |
| q4 | floor chassis + a fresh TWO-statement `dp` | 28 | 175 |
| t1 | C base written into `ap` before the D group, D in a fresh `dp` | 26 | 175 |
| t2 | the 0x68 byte-store cursor into `ap` | 30 | 175 |
| t3 | the 0x5C/0x60 base into `ap` | 43 | **173** |
| t4 | t1 + the 0x68 cursor, both through `ap` | 26 | 175 |
| t5 | t1 with the D group inline | 12 | **177** |
| v1 | `p_7e = p_6a + 10` on w3 | 27 | 175 |
| v2 | `c4 = t0*4` named at the top of the loop body, w3 | 27 | 175 |
| v3 / v5 | the same two levers on the floor chassis | **3** | 175 |
| v4 / v6 | v1+v2; floor + cast flip + p_7e derivation | 27 | 175 |
| r1 | one named `D_800A36A0` re-read for the inner loop and the 0x5C/0x60 pair | 27 | 175 |
| r2 / r3 | the 0x5C/0x60 pair sharing the inner loop's re-read | 65 / 53 | **163** |

### The finding: `loop.c:705` is the third gate on the same source fact

`tools/loop_movables.py` on three bodies (reports `movables_base.txt`, `movables_p1.txt`,
`movables_q1.txt`) reads the outer do-while (insns 103..~409, `insn_count` 86–88,
`loop_has_call=False`, threshold 122):

| body | extra life-38 movables in the outer loop | multi-set pseudos | insns | where the `la` lands |
|---|---|---|---|---|
| base (floor) | 0 | `{78: 4, 85: 4}` (85 = `ptr`) | 175 | rows 49/50/51, INSIDE the loop (target) |
| q1 (one-statement local) | 1 (insn 142, regno 96) | `{78: 4}` | 175 | rows 30/31, hoisted out; one `addu` left inside |
| p1 (inline, no local) | 2 (insns 139/151, regnos 93/99) | `{78: 4}` | **177** | rows 30/31/32 hoisted; TWO `addu` inside |

`scan_loop` admits a movable only when `n_times_set[dest] == 1` or `consec_sets_invariant_p`
succeeds (`tools/gcc-2.7.2/loop.c:705`), and `move_movables` then hoists it on
`122 * 2 * 5 = 1220 >= 88` and `119 * 1 * 1 = 119 >= 88` (`loop.c:1631`) — an inequality with no C-level slack whatever
(`loop_has_call` is the only C input to the threshold and there is no call in this loop). So the
floor body's four-times-assigned `ptr` is not incidental spelling: **it is the only thing that keeps
the `lui %hi(D_800A35D0) / addiu %lo` pair inside the loop where the target has it.**

That makes the s37 conflict three-way, all three gates reading the same source fact — how many
times the loop's pointer local is assigned:

1. `loop.c:705` wants it **> 1** (else the `la` hoists out of the loop);
2. `local-alloc.c:472` wants it **== 1** (else no block quantity, and the class-C seats are lost);
3. `sched.c` wants it **> 1** (s37: the only depth-1 vector holding the D triple below the
   group-A stores is `add_dep 137 <- 134`, spellable only as a second write of that pseudo).

The two-statement D spelling (`dp = (u8 *)&D_800A35D0; dp = (t0 * 4) + dp;`) is the cheapest way to
satisfy (1) for the D address alone — that is exactly what m2/q4 do, and why they keep 175
instructions while q1 does not keep the `la` in place.

### Closed this session

- **Frontier item 1 (a third real program value as the second write of `ap`)** — the record-layout
  audit found exactly three candidate values in the window and all four spellings of them measure
  26/26/30/43. The edge costs the class-C seats no matter which value pays for it.
- **Frontier item 2 (inline D address at 175 insns)** — six spellings, floor of the family is 176.
  The two reachable shapes are +2 (two hoisted invariants) and +1 (two `as`-macro-expanded
  symbol-indexed stores); there is no third shape.
- **Frontier item 3 (drop the merged quantity's `qty_compare_1` priority)** — both named source
  levers are erased by cse before local-alloc sees them; the `.qty` tables are byte-identical
  (v1) or differ only in pseudo numbering (v2/v4). Sharing the `D_800A36A0` re-read, the only
  remaining way to add references to the reload quantity, deletes twelve instructions.

### New best in the split-pointer family

q1 (`dp = (u8 *)&D_800A35D0 + (t0 * 4);` as ONE statement, alongside x3's single-death `ap`) scores
**18 at 175 instructions** — better than x3 (27) and m1 (23). Its whole residual is the two-insn
`la` pair sitting at rows 30/31 instead of 49/50 and the register cascade that follows. It is the
first body to hold the class-C seats AND keep the D address to one `addu` inside the loop; what it
cannot do is keep the `la` itself in the loop, because a one-statement local has
`n_times_set == 1`.

Banked forms: `rejected/s38-inline-D-shared-subscript-loop-hoists-two-invariants-177insn.c`,
`rejected/s38-named-offset-inline-D-as-macro-expands-each-store-176insn.c`,
`rejected/s38-single-statement-dp-hoists-la-out-of-loop-175insn-score18.c`,
`rejected/s38-third-value-C-base-into-ap-before-D-score26.c`,
`rejected/s38-third-value-5C60-base-into-ap-deletes-two-insns-173.c`,
`rejected/s38-early-c4-naming-cse-folded-byte-inert-w3.c`,
`rejected/s38-shared-reload-for-5C60-pair-collapses-12-insns-163.c`.

## s38 (forensics, continued run) - the movable report re-attributes the whole `ap` family

The section above was written by a run of s38 that the driver discarded on an outcome-wording
defect (a KILLED hypothesis worded at class level while declared `instance`). Its measurements were
re-verified live this run and all reproduce; they are kept, and the following is added on top.

Chassis re-measured live: **base = 3 / 175 / 175**, q1 = 18/175, x3 = 27/175.
`tools/fake_ablate.py --func func_800770B8 --file text1b --candidate memory/grind/func_800770B8/candidate.c`
prints keep-all **3**, drop the `p_old` dead store **5**, drop the empty `do { } while (0);` **8**,
drop both **10**, all at 175 build insns - identical to s35/s36/s37, so the mandated kill re-audit is
clean and both FAKE units remain load-bearing.

### New measurements (`tmp/grind/func_800770B8/s38/gen5.py`, bodies `v/u1.c`, `v/u2.c`, `v/u3.c`)

| body | what it is | score | insns |
|---|---|---|---|
| u1 | x3 chassis (single-death `ap` for group A) + the D address in a FRESH two-statement local `dp`; `ptr` keeps group C only | 26 | 175 |
| u2 | u1 + group C in its own local `cp` (no shared pointer anywhere) | 26 | 175 |
| u3 | u1 with the second write spelled `dp = dp + (t0 * 4);` instead of `dp = (t0 * 4) + dp;` | 26 | 175 |

### Finding 1 - a fresh local written twice with a REFINEMENT of its own value is one RTL set

`tools/loop_movables.py` on u1 (`tmp/grind/func_800770B8/s38/movables_u1.txt`) reports the outer
loop multi-set pseudo dict as `{78: 4}` - **`dp` is not in it**. The source writes `dp` twice
(`dp = (u8 *)&D_800A35D0;` then `dp = (t0 * 4) + dp;`) but by the time `count_loop_regs_set`
(`tools/gcc-2.7.2/loop.c:2989`) runs, only one set survives: cse propagates the symbol constant into
the `plus` and the first set dies. So the two-statement spelling does NOT give a fresh local
`n_times_set > 1`; the symbol stays a single-set invariant, `scan_loop` admits it at `loop.c:705`,
and `move_movables` hoists it on `122*2*5 = 1220 >= 88` (`loop.c:1631`). The u1 report is
insn-for-insn identical to the inline body p1: the same two extra movables (insn 139/regno 93 life
5, insn 151/regno 99 life 1).

This corrects the paragraph above ("the two-statement D spelling is the cheapest way to satisfy (1)"):
that is true only for the SHARED `ptr` of the floor body (`{78: 4, 85: 4}`, 85 = `ptr`), never for a
fresh local. The multi-set property that keeps the `la` inside the loop is a property of **variable
reuse across semantically unrelated values**, not of statement count.

### Finding 2 - PASS RE-ATTRIBUTION: the `ap` family hoist is loop.c, not sched1

`mov2.sh x3` (`movables_x3.txt`) is byte-identical to the u1 report: `{78: 4}` and the SAME two extra
movables at insns 139/151. The floor body has neither (its four movables are 560/284/555/557; x3 and
u1 carry 556/280/551/553 plus the two extras). So on the x3 chassis the `lui %hi(D_800A35D0) /
addiu %lo` pair is moved by **`move_movables` (loop.c:1631, admitted at loop.c:705)** - s36 and s37
attributed that displacement to the sched1 list scheduler. Splitting group A out of `ptr` does not
merely remove a sched anti-dependence: it drops `ptr` out of the multi-set dict entirely, so the
symbol becomes a loop invariant three passes before the scheduler ever sees it. Every body in the
`ap`/x3/q1/u-series family (scores 18-30) carries this same loop.c hoist; the sched1 story applies
only to what is left after it.

### Consequence for the three-way conflict

The conflict recorded above is unchanged in substance but its first term is now sharper: the source
fact `loop.c` reads is not "how many statements write the pointer local" but "is this local reused
for a value the compiler cannot fold into the previous one". The floor body satisfies it by reusing
one `ptr` for group A, the D address and group C; no split-pointer body measured so far
(x3, q1, u1, u2, u3, m1, w3) reproduces it, and every one of them pays 15-27 points for the loss.

Banked forms this run: `rejected/s38-fresh-twostatement-dp-folds-to-one-set-la-still-hoists-score26.c`,
`rejected/s38-fresh-dp-plus-own-C-local-la-still-hoists-score26.c`.

- [s38] Chassis re-measured live this run: memory/grind/func_800770B8/candidate.c = 3 / 175 / 175; q1 = 18/175; x3 = 27/175; u1/u2/u3 = 26/175.

- [s38] Mandated kill re-audit: tools/fake_ablate.py --func func_800770B8 --file text1b --candidate memory/grind/func_800770B8/candidate.c returns keep-all 3, drop the p_old dead store 5, drop the empty do { } while (0); 8, drop both 10 -- identical to s35/s36/s37, so both FAKE units remain load-bearing on the current chassis.

- [s38] The previous run of s38 was discarded on outcome wording only; its measurements were re-verified against the same chassis and its ledger text, artifacts (s38/gen.py..gen4.py, sweep.log, movable and row reports) and seven rejected forms are retained and re-filed here.

- [s38] loop.c gate enumeration (tools/gcc-2.7.2/loop.c:646-712 and :2989-3060) -- a movable is admitted only if the dest is not may_not_optimize, one of the three use-cases holds (not used before set / not a user variable and not the loop test / all uses in the set's basic block), invariant_p(src) holds, and n_times_set == 1 or consec_sets_invariant_p succeeds. count_loop_regs_set additionally sets may_not_move when a reg is set in two different basic blocks, or set twice in one block with a use in between.

- [s38] For the D-address symbol the dest is a compiler temp, so the user-variable and loop-test terms of the three-case test can never block it; the only source-side terms left are n_times_set (variable reuse) and may_not_move (two-basic-block sets).

- [s38] q1 (dp = (u8 *)&D_800A35D0 + (t0 * 4); as ONE statement on the x3 chassis) remains the best body in the split-pointer family at 18/175; the u-series shows the two-statement spelling of the same fresh local is not a different RTL shape.

- [s38] The floor body's multi-set ptr ({78: 4, 85: 4}) is the only measured way to keep the la inside the loop, and it is variable reuse across unrelated values -- the same source fact that local-alloc.c:472 (reg_n_deaths == 1) and sched.c (add_dep 137 <- 134) read in opposite directions.

## [s39] escalation modality — the function CLOSED at score 0 instead of being disposed of

Chassis re-measured live at session start on HEAD 805d9432: `base` (the banked floor
body) = 3/175/175, `F` (`candidate.c`) = 3/175/175, `q1` = 18/175/175, `x3` =
27/175/175 — every banked number reproduces, so all s36–s38 conclusions were
chassis-current.

Endgame gate (a), run before any probe:
`python3 tools/scan_hand_coded.py --single func_800770B8` → `tier=LOW score=0/8`,
"no strong hand-coded indicators", all of S1–S8 unset
(`tmp/grind/func_800770B8/s39/scan.log`). The canonical-asm gate FAILS, as it did in
s19/s19b/s20.

The disposition was not reached, because the one live frontier item turned out to be
the answer. s38 had named `loop.c`'s `move_movables` (not sched1) as the pass that
displaces the `lui %hi(D_800A35D0) / addiu %lo` pair out of the outer loop across the
whole `ap` family, and had killed the `n_times_set > 1` route to blocking it: cse
folds a two-statement refinement of a single local back into one set before
`loop_optimize` runs. Reading `tools/gcc-2.7.2/loop.c` this session showed the gate
has a SECOND, independent term. `count_loop_regs_set` (`loop.c:2989`) is called with
`may_not_optimize` as its `may_not_move` argument (`loop.c:593`), and
`loop.c:3040-3041` sets it for any pseudo whose set is the first in the CURRENT basic
block while the pseudo was already set earlier in the loop — i.e. a pseudo set in TWO
BASIC BLOCKS. `scan_loop` then skips the insn outright at `loop.c:649`, before the
`n_times_set` disjunct at `loop.c:705` is ever consulted, so the symbol build stays
where the source put it. cse cannot fold a second write that sits under a conditional,
which is exactly why this term survives where s38's did not.

func_800770B8 already contains a real conditional inside the outer loop — the
`if ((arg2 & mask) != 0)` arm of the 0..0xA for-loop — so no control flow had to be
invented. On the q1 chassis (the single-death-`ap` family member that already holds
all four target class-C seats, 18/175), naming the symbol in its own local and adding
a same-value dead store to that local inside the existing arm gives **0**.

Sweep (all 175 insns against target 175 unless noted); `s39/sweep.log`:

| form | what | score |
|---|---|---|
| base / F | banked floor body | 3 |
| q1 | single-statement `dp`, no second set | 18 |
| x3 | two-statement `ptr`, `ap` split | 27 |
| **g1** | **q1 + `sym` local + same-value dead store in the arm** | **0** |
| g5 | arm write spelled `&D_800A35D0 + t0*4` | 0 |
| g8 | g1 with `dp = (t0*4) + sym` operand order | 0 |
| g3 | second set written into `dp`, no `sym` local | 18 |
| g4 | same second set on the x3 chassis (into `ptr`) | 27 |
| g6 | literal self-assign `sym = sym;` in the arm | 18 |
| g1d | g1 minus the arm write | 18 |
| g1a | g1 minus the `p_old = prev;` dead store | 2 |
| g1b | g1 minus the empty `do { } while (0);` fence | 5 |
| g1c | g1 minus both inherited FAKE units | 7 |
| h1 | arm write replaced by a REAL reuse of `sym` for `D_8009BCE4[idx]` | 41 (178 insns) |
| h2 | `sym` reused for both the `&0xF2` and the `|1` store | 54 (170 insns) |
| h3 | `sym` reused for the `&0xF2` store only | 51 (178 insns) |

Three readings matter. (i) g6 = 18: a literal self-assign is deleted before
`loop_optimize` counts sets, so the value has to be re-materialised from the symbol
for the second SET to exist at that point. (ii) g3 = 18 and g4 = 27: the second set
has to land on the pseudo that HOLDS THE SYMBOL, not on the pointer holding
symbol+offset — which is why the `sym` local (a real, consumed named intermediate,
`dp = sym + (t0 * 4);`) is part of the mechanism and not decoration. (iii) h1/h2/h3:
no truthful second write substitutes — every real value available in that arm changes
the instruction count (178, 170, 178), so a +0-instruction same-value store is the
only spelling that keeps 175.

FAKE inventory of the final form — three units, three passes, additive costs
(0 → 2 → 5 → 7 as they are removed), each annotated with what / named GCC pass /
lever-exhaustion pointer:
  1. `do { } while (0);` prologue fence — sched2 scheduling-region bounds (inherited, s11).
  2. `p_old = prev;` — `local-alloc.c:472`, `reg_n_deaths == 1` (inherited, s31).
  3. `sym = (u8 *)&D_800A35D0;` in the arm — `loop.c:3040-3041`, `may_not_move` (new this session).

Final form: `memory/grind/func_800770B8/candidate.c` (== `s39/v/final.c`), applied to
`src/text1b.c` together with the two byte-neutral caller-side edits the ledger has
carried since s1. In-place verification this session:
`sandbox func_800770B8 --disable all` → `{"score": 0, "build_insns": 175,
"target_insns": 175, "scorable": true}`. Self-vet:
`memory/grind/func_800770B8/self_vet.md`.
