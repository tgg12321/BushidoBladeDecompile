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
