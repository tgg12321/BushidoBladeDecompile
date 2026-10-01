# func_800770B8 — evidence

## 2026-09-30 — ff-c, retro-audit-class fix-forward (laneC reviewer l2-759D0-r1 frontier, memory/grind/func_800759D0/evidence.md)

func_800768DC and func_800770B8 are fixed together: they share the D_800A35D0 declaration (TU-wide),
and so do func_800747D8 and func_80075670 (both completed, respelled byte-neutrally).

Object model:
- D_800A35D0 = `s16 D_800A35D0[2][2]`, one {s16, s16} pair per player. Evidence: data
  asm/data/91C98.data.s (two words 0x800A35D0..D7; D_800A35D8 is its own object); every consumer
  indexes it by player*4 (func_800768DC 0x80076948 `sll $v1,$s0,2` added to the base at 0x8007695C,
  the same index that forms SelWork f40[player] at +0x40, the parallel s16[2][2] passed as the other
  pointer to func_800692C0); func_800770B8 clears halves +2 and +0 of pair t0 (0x80077184-0x8007718C);
  func_800747D8 / func_80075670 pass pair 0 / pair arg1 to func_800692C0 (s16 *arg3). Replaces the
  `extern s16 D_800A35D0` scalar and its `(&D_800A35D0) + (player * 2)` index-past-scalar.
- D_8009BCE4 = `u8 D_8009BCE4[20]` (laneC ledger: 20 per-character flag bytes, no other symbol
  inside); `(&D_8009BCE4)[x]` -> `D_8009BCE4[x]`.
- D_8009BD21 is byte 1 of record 0 of `u8 D_8009BD20[][2]` (already declared in this TU and in
  text1b_tu1e.c; data 0x01 0x02 0x03 0x04 = two 2-byte records, asm/data/7D920.data.s:23805-23817).
  `(&D_8009BD21)[f67 * 2]` -> `D_8009BD20[f67][1]`; the C extern of D_8009BD21 is deleted (the
  dlabel / linker row stay for the INCLUDE_ASM users; no C handle remains).
- SelWork_800768DC 0x1C..0x23: two per-player s16 pairs (func_80075F80 lh/sh 0x1C/0x20 with a
  player-scaled base, asm/funcs/func_80075F80.s:107-156) that func_800770B8 clears with ONE word store
  each: 0x800772BC `sw $zero,0x20($a0)`, 0x800772C0 `sw $zero,0x1C($a0)`. Declared as Q33/Q46 union
  word views `union { s16 half[2]; s32 word; } f1C, f20;` in place of pad1C[0x18]'s first 8 bytes
  (pad24[0x10] keeps every later offset); the word members are named only at those two sites.

func_800770B8's `sym`: a typed row pointer to D_800A35D0 (pointer-alias-fake-exception typed re-view,
`s16 (*p)[2]`), needed by the existing FAKE same-value re-set (loop.c may_not_move; see that comment).
Exhaustion: no row pointer at all (direct subscripts, re-set dropped) 19/175; row pointer kept, re-set
dropped 18/175.

Scores (tmp/ffc/score_full.py = sandbox_score --disable all on full-file variants; diffs in
ff-c-2026-09-30/):

| variant | 800747D8 | 80075670 | 800768DC | 800770B8 |
|---|---|---|---|---|
| f1 declarations + consumers, 770B8 byte pointer kept | 0 | 0 | 0 | 0 |
| f2 + typed row pointer, D_8009BD20[][1], unions declared (word stores still cast) | 0 | 0 | 0 | 0 |
| **f2c + word stores through SELWORK_800768DC->f20.word / f1C.word (proposed)** | **0** | **0** | **0** | **0** |
| f3 f2c without the row pointer | — | — | — | 19 |
| f3b f2c, row pointer kept, re-set dropped | — | — | — | 18 |

### Applied (uncommitted, for layer-2), 2026-09-30

f4 = f2c + a FAKE pointer-alias annotation at `sym`'s declaration (text in src), regenerated on HEAD
1c47528d6 (after func_80074E08's reopen). Under the lock: rebuild build_sha1
62efab4f73f992798c43e8c730aa43baa10bb4fa; integrity OK; sandbox 0 for all four; layer-2 hashes
func_800747D8 d8db9fc860829384, func_80075670 ae836e096f26313c, func_800768DC 0d456e1392bb3646,
func_800770B8 120523d39b6ddfad. Exact bytes: tmp/ffc/backup_45/text1b_tu2.c.

## 2026-09-30 — REOPENED under owner Q37 (layer-2 rev-45, SelWork cluster)

Reopened together with the other SelWork consumers; the landed body is in
rejected/selwork-cluster-2026-09-30.c. Frontier (one landing for the whole cluster, rev-45's
findings, the Q33(5) size question, p_old, D_8009BD20, GaugeWork): 
pre-slim-2026-10-01:memory/grind/func_800768DC/selwork-cluster-2026-09-30.md (+ the reviewed patch beside it).

## 2026-09-30 — laneD (SelWork cluster, Q57)

Measurements, chosen variants and the one-type SelWork: pre-slim-2026-10-01:memory/grind/func_800768DC/laneD-2026-09-30.md;
Q57 (d) layout search: pre-slim-2026-10-01:memory/grind/func_800768DC/q57-layout-search-2026-09-30.md.

func_800770B8 status (laneD): all-member body byte-exact (candidate.c, 0/175) but it keeps the p_old
reuse + FAKE dead restore that rev-45 item 3 refused; owner question filed via the orchestrator
2026-09-30; not in this landing (INCLUDE_ASM).

## 2026-09-30 — owner Q66: "Refuse for now (Recommended)" — p_old reuse stays banned; INCLUDE_ASM

Frontier, measured by laneD on the all-member SelWork chassis. Details are in
pre-slim-2026-10-01:memory/grind/func_800768DC/laneD-2026-09-30.md; the bodies are in
pre-slim-2026-10-01:memory/grind/func_800768DC/laneD-2026-09-30/.

| form | score | notes |
|---|---|---|
| candidate.c: all-member body, p_old reuse + FAKE restore | 0/175 | refused (Ruling 11 (B)(1), Q66) |
| k0: no restore | 2/175 | see below |
| v7, m1-m3: separate list / work locals (rev-45's suggestion) | 20/175 (170 insns) | see below |
| r1/r2: Ruling 4 split of the list pointer | 20 | |
| r3: function-scope work pointer reused as the loop base | 34 | |

- **k0.** The only diff is the base register of the 0x30/0x34 clears: $s1 (p_old) where the target has
  $v0 (the func_8006E49C return). The cause is cse. It keeps the D_800A36A0 reload in
  SELWORK->f30/f34 equivalent to the pseudo stored into D_800A36A0, unless that pseudo is overwritten
  after the store. The restore was what broke that equivalence.
- **v7, m1-m3.** The list pointer arg0 + 0x58 is set once, so it is tied to arg0's register ($s0). The
  function then uses one fewer callee-saved register.

Re-measure after owner-adopted Q65 (the per-file gp model) lands. The residual involves D_800A36A0's
%gp_rel addressing, which Q65 changes.

## 2026-10-01 — laneA: re-measured on the regenerated Q65 series (scratch clone, tag step16 = b55913da8, base 6c73a2796)

SelWork f1C/f20 given the Q33/Q46 union word views (uncommitted, scratch only; text1b_b's other f1C/f20
element reads respelled `.half[...]`); f3C stays `s16 f3C[2]` as landed with func_80075F80. The function now
lives in text1b_b (M4 merge). `engine sandbox --disable all`:
- candidate.c (f3C.half -> f3C): 0/175 (only the masked D_8009BD21 reloc-symbol hunk at insn 163).
- k0 (candidate without the refused `p_old = prev;` restore): **2/175, unchanged by Q65** — the same
  operand-only hunk at insn 35: `sw zero,48 / sh zero,52` based on $s1 where the target uses $v0.
So Q66's "may shift after Q65" did not happen: the gp model does not touch this residual (it is the cse
equivalence of the D_800A36A0 reload, not its addressing). Frontier unchanged: an ordinary-C spelling that
breaks that equivalence without a dead write. Scripts: tmp/func_800770B8/{mk.py,scratch_sbx.sh} (not banked).

## 2026-10-01 — laneA s40: the cse decision named; the no-dead-write route measured (+1 lw); permuter 36.5k

Chassis: private clone of the Q65 series (step16 + SelWork f1C/f20 unions, /tmp/l770/tree; probes and
scripts in probes/s40/). Dumps: cc1 -da on k0 and on candidate.c (c0).
- **Decision.** k0 `.cse`: insn 77 `(set 75 v0)` puts p_old (pseudo 75) and $v0 in one quantity with 75
  first (cse.c make_regs_eqv: a pseudo beats a non-fixed hard reg); the D_800A36A0 store records the memory
  in that class, so both reloads for the 0x30/0x34 clears become `(reg 75)` (insns 88/93) -> $s1. In c0 the
  restore `(set 75 80)` (insn 86) takes 75 out of the class first; the reload becomes insn 89 `(set 82 (reg
  v0))` and the clears use pseudo 82 -> $v0. So the target's split (D_800A36A0 and f04 stores on $s1, clears
  on $v0, no extra insn) needs p_old to be written between the f04 store and the clears **with a value that
  is never materialized**. The only ways a write leaves no instruction: it is dead (refused, Q66), or combine
  folds it into its one user.
- **The non-dead route exists and costs exactly one insn.** p1 `*++p_old = (s32)prev;` (and p2 `p_old++;
  *p_old = ...`, p5 `p_old = (s32 *)&((SelWork *)p_old)->f04; *p_old = ...`): combine folds `75 = 75 + 4` into
  the store (`sw $v1,4($s1)`) and the clears move to $v0 as in the target — but the store's address is a
  plain register, neither MEM_IN_STRUCT nor a PLUS, so cse.c note_mem_written (7564-7574) sets `all` and
  invalidates the D_800A36A0 memory entry: one extra `lw $v0,%gp_rel(D_800A36A0)($gp)` (3/176). Making that
  store in-struct needs an INDIRECT_REF of a PLUS_EXPR on the incremented pointer (`(p_old += 2)[-1]`,
  constant cancellation, Q45-refused) or a fake aggregate type; not pursued.
- p3/p4 `p_old = (s32 *)SELWORK;` before the clears: cse makes it `75 = 75` and deletes it; 2/175 (= k0).
- Permuter from k0 on this chassis (2 workers, 36,505 iterations, 2036 s): no find at or below base.
Conclusion: on the post-Q65 chassis every byte-exact form needs a write to p_old whose value never reaches an
instruction; the only non-dead one (pointer pre-increment) costs one load. Filed as a policy-question
(docs/grind/borderline.md 2026-10-01 func_800770B8), re-asking Q66 with this evidence.

## 2026-10-01 — laneA s41: landing package under owner ruling Q78 (rules: 6c8c276c3), on main

Chassis: a clone of main (HEAD 3717bfd7a) with SelWork's f1C/f20 as Q33/Q46 union word views
(probes/s41/unions.py, mkmain.sh; text1b_tu2.c's other 14 f1C/f20 element accesses respelled `.half[...]`)
and `func_8006E950`'s declaration `(s32 a0, s32 *a1)` (its callers pass integers 6 / 0x32 / 0x5F; the Q65
series step 10 makes the definition agree); full build SHA1 == oracle. Bodies: probes/s41/ (F.c =
candidate.c; ablations by mkF.py, splits by mkF.py / mkS.py). Scores: `engine sandbox func_800770B8
--disable all` (msbx.sh); re-confirmed on the spliced main tree (sandbox 0, full build oracle).

| body | score | insns | what differs from F |
|---|---|---|---|
| F (candidate.c) | 0 | 175 | — |
| F_norestore | 2 | 175 | no `work = list;` (Q78 store): clears on $s1 (0x80077144/48) |
| F_nodowhile | 5 | 175 | no empty do-while(0): prologue frame-save order (3 source-level hunks) |
| F_noreset | 18 | 175 | no `row = D_800A35D0;` re-set: D_800A35D0's lui/addiu hoisted out of the outer loop |
| F_norow | 19 | 176 | no row pointer at all (direct `D_800A35D0[t0][k]`) |
| F_split0 | 20 | 170 | one variable per value, F's exact statement list: `s32 *list` (entry list pointer), `void *work` (result) |
| F_split / F_split_b / F_split_w | 20 / 20 / 20 | 170 | split with `SelWork *work` (f04 through work) / declared then assigned / clears through work |
| S_top / S_first / S_after | 20 / 20 / 20 | 170 | split, list assigned at the top / before the sp clears / after the D_800A35D8 store |
| S_init | 15 | 173 | split, list as the declaration's initializer |
| S_r1 / S_r2 | 20 / 20 | 170 | split, Ruling 4 two-statement list pointer (laneD r1 / r2) |
| S_glob / S_darg | 20 / 20 | 170 | split, f04 stored through SELWORK / list from D_800A35D8 + 0x58 |

Earlier split forms on the previous chassis (laneD 2026-09-30, pre-slim-2026-10-01:memory/grind/func_800768DC/
laneD-2026-09-30.md:86-93): v7 / m1-m3 20, r1/r2 20, r3 (function-scope work reused as the loop base) 34.
Permuter from the split body (S_base, 2 workers, 18,206 iterations, campaign label "split-R11-search", workspace probes/s41/mkpermm.sh): no zero;
best find 240 (base 883) re-writes `list` inside the loop (`list = D_800A36A0`), i.e. a reuse, not a split.

**Q78 store (`work = list;`). Dumps: dumps/s41/F and dumps/s41/F_norestore (`.cse`; command lines in cmd.txt).**
F_norestore: insn 77 `(set (reg/v 75) (reg 2 v0))` (work = the call result); insn 80 `(set (mem D_800A36A0) (reg
75))`; the f04 store (insn 85) and both clears (insns 90/95) are based on `(reg 75)`, because cse.c make_regs_eqv
(tools/gcc-2.7.2/cse.c:826) made pseudo 75 the first register of the quantity it shares with the non-fixed hard
reg $v0, and the D_800A36A0 memory entry is in that class; .greg seats 75 in $s1, so the clears are on $s1.
F: the f04 store's SELWORK reload is kept as insn 83 `(set (reg 82) (reg 75))`, insn 85 stores f04 on 75, then
insn 88 `(set (reg 75) (reg 80))` (the restore) removes 75 from the class (cse.c delete_reg_equiv, cse.c:887);
the clears' reloads now resolve to reg 82 (insns 93/98), which .greg seats in $v0: `sw $zero,0x30($v0)` /
`sh $zero,0x34($v0)` as in the target. The restore is dead (work is not read after; flow deletes it). Lever
exhaustion: bf8588dd8^:memory/grind/func_800770B8/hypotheses.md class B, sessions s1-s39 (s7 named the cse
pseudo identity; s31 measured the restore), and s40 above (the non-dead pointer pre-increment route costs one
lw; `p_old = SELWORK` is a cse no-op; permuter 36,505 iterations from k0, no find).

**Ruling 11 package for `work` (two real values).**
- (A) a local, declared once at function scope (the innermost scope enclosing its writes: the entry
  assignment and the block's call-result write), no other declaration moved, address never taken.
- (B)(1) value 1 (`(void *)(arg0 + 0x58)`) is read by func_8006E950, func_80076FF8 and `list = work`; value 2
  (func_8006E49C's result) by the D_800A36A0 store. The third write is the Q78 dead store, admitted by Q78
  alone. (B)(2) Ruling 5 2(c): no write re-stores a value work already holds on every path (the call result is
  not known equal to the list pointer; the restore stores the list pointer while work holds the call result).
- (C)(1)/(2) the one-variable-per-value spelling with the same statement list is F_split0 (only declarations /
  identifiers differ, and the restore has nothing to restore). (C)(3) both values are real computations in the
  target: `addiu $s1,$s0,0x58` (0x800770E8) and the call result moved into $s1 (`addu $s1,$v0,$zero`,
  0x80077128).
- (D)(1) allocation dumps: dumps/s41/F/f.lreg, f.greg and dumps/s41/F_split0/f.lreg, f.greg (cmd.txt). (D)(2)
  the deciding decision: local-alloc.c:472 takes a pseudo as a local-alloc quantity only if it dies exactly
  once. In F_split0 the list pseudo (75) dies once, local-alloc takes it and ties it to arg0's quantity
  (`;; Register 72 in 16.` / `;; Register 75 in 16.` in F_split0/f.lreg; asm `addu $16,$16,88`): one
  callee-saved register fewer, 170 insns. In F, work (75) dies in 2 places (`Register 75 ... dies in 2 places`,
  F/f.lreg), stays out of local-alloc, and global-alloc seats it in $s1 (`75 in 17`, F/f.greg) beside arg0 in
  $s0: the target's two registers (`addu $s0,$a0,$zero` 0x800770C0, `addiu $s1,$s0,0x58` 0x800770E8). (D)(3)
  every split spelling proposed (laneD v7/m1-m3/r1-r3 and the twelve above) measured; none byte-identical.
  (D)(4) full split 20; per-value ablations n/a (two values); structural respellings S_init / S_r1 / S_r2 /
  S_glob / S_darg / F_split_w; permuter from the split body (above).
- (E) name `work` (a generic name Ruling 11 (E) lists); type `void *`, true of both values (an s32 list and the
  SelWork area). (F) the declaration comment names both values and cites Ruling 11, Q78 (6c8c276c3) and this
  entry. (G) fresh layer-2 pending.

**`list` (`s32 *list = work;`).** A fresh local written once with the live list pointer just before work is
overwritten, read by the f04 store and by the Q78 restore: ordinary C saving a value before its variable is
reused (Ruling 1, named intermediate relaxed to once-written: a real, consumed value). The copy is in the
target: `addu $v1,$s1,$zero` at 0x80077124, stored by `sw $v1,0x4($s1)` at 0x80077140.

**Row pointer `row` (pointer-alias-fake-exception) and its re-set `row = D_800A35D0;` (dead-store-fake-
exception).** Exhaustion: direct subscripts 19/176 (F_norow), row pointer without the re-set 18 (F_noreset);
prior record bf8588dd8^:memory/grind/func_800770B8/hypotheses.md s38-s39 (s38: a second straight-line write is
folded by cse; s39: a set in a second basic block trips loop.c count_loop_regs_set's may_not_move,
loop.c:3040-3041, so scan_loop skips the hoist at loop.c:649) and 188 banked rejected forms there. The re-set's
stored value is never read (store-level deadness, Ruling 2). Mechanism and annotation at the statements.

**do-while(0)** (do-while-zero-exception, sanctioned; single level): 5 without it (F_nodowhile: the prologue's
frame-save stores interleave with the first body insns); prior record s4-s11 (sched2 region bound; s9
sched_solver sweep).

**Data model and casts.** SelWork f1C/f20: Q33/Q46 union word views, byte evidence `sw $zero,0x20($a0)` /
`sw $zero,0x1C($a0)` at 0x800772BC/C0 (one word store over each s16 pair; func_80075F80 reads and writes the
halves, asm/funcs/func_80075F80.s:107); the other consumers (func_80075830, func_800759D0, func_80075F80)
respelled `.half[...]`, each sandbox 0, full build oracle. `D_8009BD20[f67][1]` replaces the `D_8009BD21`
alias; its undefined_syms_auto.txt row (`retire with func_800770B8`) goes (the asm data's own dlabel stays).
Casts in the body: `(void *)(arg0 + 0x58)` and `(void *)func_8006E49C(...)` (integer addresses: arg0 is the
s32 buffer base, func_8006E49C returns s32); `(s32 *)D_800A35D8` (the s32 buffer base passed as the callee's
`s32 *`, as Q65 step 10 spells text1b_b's two calls); `(u8)` / `(s16)` value narrowings; SELWORK is game.h's
typed view of D_800A36A0.

## 2026-10-01 — laneA s41b: layer-2 round 1 (rev-770B8-r11 PASS, rev-770B8-dm FAIL on one defect)

rev-770B8-r11 PASS (work / Q78 restore / list copy). rev-770B8-dm FAIL: `u16 sp[2]` read through `(s16)` casts at
all four reads, while the target reads it signed (`lh` + `slt`, 0x800772C4-D0; the `lhu` is only the
increment's load): Ruling 1(4) simplest form, refused signedness-split shape. Fix: `s16 sp[2]`, the four casts
gone (the reviewer's measurement tmp/rev770b8/s16sp.c: 0/175). Everything else in dm scope PASSED; the three
`.half` bodies PASSED (func_80075830 82bec16a269994a7, func_800759D0 e57b77f57c869a3a, func_80075F80
61aca6158823c712). Re-measured on the fixed body (s41 table unchanged: F 0, F_norestore 2, F_nodowhile 5,
F_noreset 18, F_norow 19/176, splits 15-20); dumps/s41 and probes/s41 regenerated from it (same insn numbers and
allocations as quoted in s41). Spliced main: full build oracle, sandbox 0, layer2 hash dfd71d95a3fa9ed0.

## 2026-10-01 — LANDED (COMPLETED-C)

Layer-2 round 2 rev-770B8-r2 PASS, full scope (re-generated the s41 dumps from the staged src, byte-equal):
func_800770B8 dfd71d95a3fa9ed0 (match), func_80075830 82bec16a269994a7, func_800759D0 e57b77f57c869a3a,
func_80075F80 61aca6158823c712 (cheat-cleanup). Landed in aa8064128 (Match), queue done 0b9653409; full build
SHA1 == oracle; check_completion_integrity OK. Non-blocking reviewer note: `q->f00 = arg1` stores an s32 into the
`void *` member without a cast (later typing cleanup of SelWork f00 / the arg1 parameter).
