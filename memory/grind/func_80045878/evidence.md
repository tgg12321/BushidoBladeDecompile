# Evidence bank — func_80045878

## s1 (recon, 2026-07-23)
- Baseline reconfirmed: canonical verdict C; sandbox --disable all score=10,
  build_insns=107, target_insns=108, 10 rules dropped.
- m2c target shape captured: tmp/grind/func_80045878/s1/m2c_target.c. Confirms
  a record-init/dispatch fn; struct pointer var_s1; s3=a0+3 written TWICE
  (pre-if + else). HEAD build dump: .../s1/build_head_score10.txt.
- Gap decomposition CONFIRMED via objdump: (A) 1 missing insn = folded else
  recompute `addiu s3,s2,3`; (B) tail store block base-copy/register mismatch
  (~9 diffs) — target `addu v0,s1,zero` + stores via v0 + `sh s2,..` direct +
  scratch in v1; ours stores via s1 with an extra `move v0,s2`.
- PROBE (arm-split, set s3 in both arms, drop pre-if init): build 107->108
  (Gap A recompute MATERIALIZES) BUT score 10->11 — sched1 hoists the fresh
  recompute 3 slots early (feeds next call arg) vs target's last position.
  => Gap A insn is REACHABLE in pure C; residual is scheduling-anchor only.
  Saved rejected/armsplit-s3-materializes-but-sched-early.c (building block).
- Duplicates: no useful analog — text1a_c near-clone report lists func_80045878
  under no lead; nearest COMPLETED-C siblings are unrelated shapes.

- WIP rejected_form: {'form': 's16 *p = s1; tail stores via p', 'score': 10, 'reason': 'Not a cheat, no effect: GCC copy-propagates the single-set alias (s1 dead after the block), so no `addu v0,s1,zero` materializes. build stayed 107. Need a form that keeps the copy live.'}

- == imported from memory/wip notes.md ==
# func_80045878 (text1a_c.c) — WIP, BLOCKED, register/codegen cluster (10 rules)

## TL;DR (2026-06-14)
HEAD byte-matches only via 10 regfix rules = 1 lost-codegen `insert_after
"addiu $19,$18,3" @30` + a 9-rule tail-store `subst` cluster (idx 86-94).
Two coupled gaps toward COMPLETED-C; no pure-C form found this session.

## Gap A — folded recompute (build is 1 insn short: 107 vs 108)
Target recomputes `addiu $s3,$s2,3` (= a0+3) TWICE. Source has `s3=a0+3` (top,
line 1877) and `s3 = a0 - -3` (else, line 1891) — GCC folds them to one. The
`insert_after @30` injects the missing one. Need a structural change so the
else recompute survives (shared-end-label family).

## Gap B — tail store base copy
Target's final store block (`.L800459DC`):
```
addu  v0, s1, zero          ; copy s1 base into caller-save $v0
addiu v1, s2, 3             ; v1 = a0+3
sh    v1, 0x16(v0)          ; s1[11] = a0+3
ori   v1, zero, 0x8000      ; v1 = 0x8000
sh    s2, 0x4(v0)           ; s1[2]  = a0
sh    s5, 0x8(v0)           ; s1[4]  = a1   (s5 = a1)
sh    s2, 0x14(v0)          ; s1[10] = a0
sh    s2, 0x10(v0)          ; s1[8]  = a0
sw    v1, 0x18(v0)          ; *(s1+0x18) = 0x8000
```
My build stores directly through `$s1` (no copy). A single-set `s16 *p = s1;`
alias is copy-propagated away (s1 dead after the block) → no `addu v0,s1,zero`
materializes, build stayed 107. Need the copy to survive: keep s1 live past
the copy, reuse the param as a cursor, or name the copy as a kept value (see
param-reuse-base-copy-cse-canon).

## Resume steps
1. Keep HEAD body. Work Gap A (recompute survival) and Gap B (materialized
   base copy) in pure C.
2. VERIFY via full build: the masked sandbox CANNOT see the $v0-vs-$s1 base
   choice; need build_insns 108 AND retire SHA1 == oracle.

## Ruled out (do not re-derive)
- `s16 *p = s1;` tail alias: copy-propagated, no effect (still 107 / floor 10).

## Pointers
- `.claude/rules/param-reuse-base-copy-cse-canon.md` (materialize a base copy)
- `.claude/rules/lost-codegen-insert-cheat.md` + `shared-end-label.md` (Gap A)
- Same hard cluster class as this batch's motion_SetMotion / func_8006156C.


- [s1] Baseline: canonical verdict C; sandbox --disable all score=10, build_insns=107, target_insns=108, 10 rules dropped.

- [s1] m2c target shape captured (tmp/grind/func_80045878/s1/m2c_target.c): record-init/dispatch fn, struct ptr var_s1, s3=a0+3 written twice (pre-if + else).

- [s1] Gap A (1 insn) = folded else recompute `addiu s3,s2,3`; Gap B (~9 insns) = tail store base-copy/register mismatch.

- [s1] PROBE arm-split: build 107->108 materializes Gap A recompute (Gap A is pure-C-reachable) but score 10->11 due to sched1 hoisting the recompute 3 slots early; saved rejected/armsplit-s3-materializes-but-sched-early.c as a building block.

- [s1] Duplicates scan: no useful analog for func_80045878 in the near-clone report.

- [s1] src/text1a_c.c reverted to the score-10 HEAD form; sandbox re-verified score=10. No rules/pipeline/engine files touched.

## s2 (structural, 2026-07-23)
- Full target asm captured (asm/funcs/func_80045878.s). Confirms HEAD source is
  STRUCTURALLY the target up to Gap A: pre-if `s3=a0+3` lands in the beqz delay
  slot (0x800458AC == HEAD 2c2c), else recompute at 0x800458F8 (else last).
- Gap A mechanism NAILED via cc1 -da dumps (tmp/grind/func_80045878/s2/): the
  else recompute is RTL insn 74 `(set reg75 (plus reg72 3))` — CSE DELETES it
  (gone in dump.i.cse: insn 71->code_label 76 directly) because reg75 already
  == reg72+3 (reg72=a0 in callee-save s2, unchanged across the else calls;
  reg75 untouched). Pre-if init (needed to anti-dep-anchor the redefinition for
  sched) is EXACTLY what feeds cse the available expression => catch-22. No
  value-neutral spelling escapes: cse constant-folds `a0-(-3)`, `(a0+1)+2`, etc.
  to (plus a0 3) BEFORE the availability check.
- Gap B root cause NAILED via RTL: our fork expands `s1[11]=a0+3` by truncating
  a0 to HImode FIRST (insn 213 `reg100=(HI)a0`; insn 215 `reg101=(SI)reg100+3`),
  then cse REUSES reg100 for the three `s1[N]=a0` HI stores (insns 220/226/229
  substitute reg100 for (subreg:HI reg72)). Net: base kept in s1, a0-HI CSE'd
  into v0, `move v0,s2`. TARGET instead adds 3 to full s2 (`addiu v1,s2,3`),
  stores s2 DIRECTLY 3x, and COPIES base `addu v0,s1,zero`.
- KILLED H-C (param types s16/s16/u32*): score 10->43, build 107->112. s16
  params inject sign-extends on every a0/a1 use; target has ZERO. Original params
  are register-width s32. m2c's narrow inference is wrong here.
- KILLED frontier item 3 (A+B coupling): applied arm-split (Gap A materialized,
  build 108) and disassembled the tail — BYTE-IDENTICAL to HEAD's tail
  (`sh v0,4(s1)...; li v0,0x8000; sh s5,8(s1); sw v0,24(s1)`, base=s1). Gap A's
  presence does NOT change Gap B. The two gaps are FULLY INDEPENDENT.
- KILLED Gap B lever H-B (mixed p/s1 base copy): `s16 *p=s1; p[..]=..;` for the
  HI stores + s1 referenced in the trailing word store -> score 10, build 107.
  cse copy-propagates `p=s1` regardless (both are the same available value).
  Target's base copy is a local_alloc live-range-split of the join block
  (.L800459DC, 2 preds), NOT a C-level pointer copy — unreproducible by any
  clean `p=s1`. Probed SI-temp for a0+3 (`s32 t=a0+3; s1[11]=t;`) -> build 106
  (drops the (HI)a0 truncation) but score stays 10; wrong direction.
- Artifacts: tmp/grind/func_80045878/s2/{dump.i.rtl,dump.i.cse,dump.i.greg,
  f_rtl.txt,f_cse.txt,f_greg.txt,dump.sh,dis.sh}.

- [s2] Full target asm (asm/funcs/func_80045878.s): HEAD source is structurally the target up to Gap A; pre-if s3=a0+3 lands in the beqz delay slot (0x800458AC), else recompute at 0x800458F8 (else last).

- [s2] Gap A = cse deletion of RTL insn 74 (else recompute); confirmed via dump.i.rtl (present) vs dump.i.cse (gone). reg72=a0 stays in callee-save s2 across the else calls so a0+3 is an available expression.

- [s2] Gap B = fork truncates a0->HImode for the s1[11]=a0+3 store (RTL insn 213 reg100=(HI)a0; insn 215 (SI)reg100+3), then cse reuses reg100 for the three s1[N]=a0 HI stores (base kept in s1, move v0,s2). Target adds 3 to full s2 and stores s2 directly, with base copied `addu v0,s1,zero`.

- [s2] Gap A and Gap B are INDEPENDENT: arm-split tail is byte-identical to HEAD tail.

- [s2] Param narrowing (s16/s16/u32*) is dead: original params are register-width s32 (target has zero sign-extends).

- [s2] Target's tail base copy is a local_alloc live-range split of the 2-predecessor join block (.L800459DC), not reproducible by a C-level p=s1 copy (cse always copy-propagates it).
