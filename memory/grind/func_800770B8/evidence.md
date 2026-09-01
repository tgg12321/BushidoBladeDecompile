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
