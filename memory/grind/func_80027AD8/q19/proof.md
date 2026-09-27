# func_80027AD8 — Ruling 12 (D) proof for `tbl` and `tbl_arg` (2026-09-27, manual slotK)

Ruling: .claude/rules/ordinary-c-judge-decidable.md § Ruling 12 (owner Q19, 2026-09-26, recorded in
6b4b62b99). Every number below was measured on the STOCK cc1 (PLUS->IOR patch retired by d94fef9a0;
tools/gcc-2.7.2/build/cc1 and the instrumented tools/gcc-2.7.2/cc1 both rebuilt 2026-09-26 23:14/23:17,
before every dump here was taken).

## Bodies (all generated from ONE base; q19/v/, generator q19/scripts/mk.py)
- **cand** = q19/v/cand.c: the landing body. It holds two copies of the stack-passed parameter `rec`
  (6th argument, 0x64(sp) in the target): `Tbl8008E194 *tbl; tbl = rec;` (every field read and the R2
  NULL test) and `Tbl8008E194 *tbl_arg; tbl_arg = rec;` (the three func_800278C0 calls, passed as
  `(s32)tbl_arg` because func_800278C0's 4th parameter is `s32`; the cast sits on the use, not the copy).
- **no_tbl**: `tbl` removed; the field reads and the NULL test read `rec`. **no_arg**: `tbl_arg` removed;
  the calls pass `(s32)tbl`. **no_arg_rec**: `tbl_arg` removed; the calls pass `(s32)rec`.
  **neither** = memory/grind/func_80027AD8/candidate.c (byte-identical; both copies removed).
  The twins differ from cand ONLY by the removed declaration/assignment and the renamed reads (mk.py).
- FAKE-family bodies (see (D)(3)): `*_selfassign` (`rec = rec;`, dead param self-assign), `*_deadstore`
  (`rec = NULL;` on the R6a path, where rec is dead: Ruling 2 store-level dead), `no_tbl_chain`
  (combine-foldable detour on a read), `no_tbl_dowhile` (`do { } while (0);` around the entry copy),
  `arg_sink` (the argument copy sunk to each call site: hoist/sink + duplicated-into-arms),
  `no_tbl_cancel{,3,16,32,40,48}` and `no_arg_rec_cancel{1,2,3,1one,2one}` (cancelling live uses
  `(s32)rec + (s32)rec - (s32)rec`), `c3_stg_opp` (no_tbl_cancel3 + opp staged through itself first).

## Prongs (A)-(C), (E), (F)
- (A) `rec` is the 6th argument, passed on the stack: the target loads it with `lw $s5, 0x64($sp)`
  (0x80027B3C) and cand's asm with `lw $21,100($sp)` (frame 0x50). cand never writes `rec` (no
  assignment, no compound assignment, no ++/--, no `&rec`): `grep -n "rec\b" q19/v/cand.c` shows it
  only in the parameter list and the two copies.
- (B) Both copies are fresh locals of the parameter's own type `Tbl8008E194 *`, declared once at
  function scope, written exactly once with the bare parameter and NO cast, never written again, and
  their addresses are never taken.
- (C) `tbl` carries every field read of the record (`tbl->unkC` x1, `tbl->unk0` x1, `tbl->unkD` x4) and
  the R2 NULL test; `tbl_arg` carries every pass of the record to func_800278C0 (3 calls). `rec` itself
  is read only by the two copies. Neither copy feeds the other in the source.
- (E) `tbl` (the record, read field by field) and `tbl_arg` (the record, passed on) are the ruling's
  own examples and true of every read.
- (F) The declaration comment in cand cites Ruling 12 and this file.

## (D)(1) Dumps and command lines
`bash tmp/func_80027AD8/r11/dump.sh <tag> <body.c> [pseudo...]` (banked: q19/scripts/dump.sh):
splices the body into a scratch copy of src/code6cac_b.c exactly as the landing does (replacing the
`INCLUDE_RODATA jtbl_80010548` + `INCLUDE_ASM func_80027AD8` lines); preprocesses with
`mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc <engine.buildconfig CPP_DEFS>`;
runs the BUILD cc1 `tools/gcc-2.7.2/build/cc1 <CC_FLAGS> -df -dl -dg -dJ -dS` (CC_FLAGS =
`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float`)
and the instrumented `tools/gcc-2.7.2/cc1 <CC_FLAGS>` with `BB2_ALLOC_DEBUG=1 BB2_SUGG_DEBUG=1`, plus
one `BB2_FINDREG_DEBUG=<pseudo>` run per pseudo (q19/scripts/findreg.sh). Identity check: every body's
func_80027AD8 asm from the instrumented cc1 equals the build cc1's ("IDENTITY OK" for all 26 dumped tags,
tmp/func_80027AD8/r11/all.log and the individual runs). Combine / sched2 dumps: `-dc` / `-dR` on the same .i. Verbatim
excerpts per body: q19/rtl/<tag>.txt (Register lines from .flow and .lreg, sched1 life updates,
the entry insns, .greg dispositions, ALLOCDBG, FINDREG, local-alloc quantities, asm or asm diff).

Pseudo map (cand, from the RTL sets in q19/rtl/cand.txt): 72 pass, 73 ch, 74 limb, 75 thresh,
76 flag, 77 rec (parameter), 78 arg6, 79 out, 80 tbl, 81 tbl_arg, 82 vec, 83 scr, 84 player, 85 opp,
90 cat, 91 sign, 92 same. In the twins that lose a declaration the later numbers shift down by one
(no_tbl: 80 tbl_arg, 81 vec, 84 opp; no_arg / no_arg_rec: 80 tbl, 81 vec, 84 opp).

cand, verbatim:
```
flow: Register 77 used 2 times across 9 insns in block 0; pointer.
flow: Register 80 used 9 times across 163 insns; dies in 5 places; crosses 2 calls; pointer.
flow: Register 81 used 4 times across 124 insns; dies in 3 places; crosses 3 calls; pointer.
;; register 80 life shortened from 163 to 134
;; register 81 life shortened from 124 to 94
lreg: Register 80 used 9 times across 134 insns; dies in 5 places; crosses 2 calls; GR_REGS or none; pointer.
lreg: Register 81 used 4 times across 94 insns; dies in 3 places; crosses 3 calls; GR_REGS or none; pointer.
ALLOCDBG func=func_80027AD8 ord=30 pseudo=72 hardreg=20 nrefs=14 livelen=188 pri=2234
ALLOCDBG func=func_80027AD8 ord=31 pseudo=80 hardreg=21 nrefs=9 livelen=134 pri=2014
ALLOCDBG func=func_80027AD8 ord=32 pseudo=90 hardreg=20 nrefs=2 livelen=12 pri=1666
ALLOCDBG func=func_80027AD8 ord=33 pseudo=91 hardreg=16 nrefs=2 livelen=13 pri=1538
ALLOCDBG func=func_80027AD8 ord=34 pseudo=85 hardreg=22 nrefs=7 livelen=93 pri=1505
ALLOCDBG func=func_80027AD8 ord=35 pseudo=92 hardreg=21 nrefs=2 livelen=14 pri=1428
ALLOCDBG func=func_80027AD8 ord=36 pseudo=82 hardreg=23 nrefs=16 livelen=578 pri=1107
ALLOCDBG func=func_80027AD8 ord=37 pseudo=81 hardreg=30 nrefs=4 livelen=94 pri=851
ALLOCDBG func=func_80027AD8 ord=38 pseudo=75 hardreg=-1 nrefs=4 livelen=123 pri=650
```
The entry, .flow then .combine (q19/rtl/cand.combine.head.txt), verbatim:
```
flow:    (insn 14 12 16 (set (reg/v:SI 77) (mem:SI (plus:SI (reg:SI 0 $0) (const_int 20)))) ... (expr_list:REG_EQUIV (mem:SI (plus:SI (reg:SI 0 $0) (const_int 20)))
flow:    (insn 36 33 39 (set (reg/v:SI 80) (reg/v:SI 77)) ... (expr_list:REG_DEAD (reg/v:SI 77)
flow:    (insn 39 36 43 (set (reg/v:SI 81) (reg/v:SI 80)) ...
combine: (note 14 12 16 "" NOTE_INSN_DELETED)
combine: (insn 36 33 39 (set (reg/v:SI 80) (mem:SI (plus:SI (reg:SI 0 $0) (const_int 20)))) 159 {movsi_internal2} (nil) (nil))
combine: (insn 39 36 43 (set (reg/v:SI 81) (reg/v:SI 80)) ...
greg:    (insn 39 25 43 (set (reg/v:SI 30 $fp) (reg/v:SI 21 s5)) 159 {movsi_internal2} (insn_list 36 (nil)) (nil))
```
FINDREG (cand), verbatim (q19/rtl/findreg.out.txt):
```
== cand:80   conflicts: 2 3 4 5 6 7 16 17 18 19 20 29  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
== cand:81   conflicts: 2 3 4 5 6 16 17 18 19 20 21 22 23 29  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
== cand:85   conflicts: 2 3 4 5 16 17 18 19 20 21 29  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
== cand:92   conflicts: 2 3 4 5 6 7 16 17 18 19 20 29  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
== cand:75   conflicts: 2 3 4 5 6 7 16 17 18 19 20 21 22 23 29 30  someone_prefers: (empty)  own_* (empty)
```
(Full per-line blocks with used_so_far / pass0_used / pass1_used / used2_noconflict are in
findreg.out.txt; the lines above are copied from it with the empty sets marked.)

no_tbl (no field-read copy), verbatim:
```
flow: Register 77 used 9 times across 170 insns; dies in 5 places; crosses 2 calls; pointer.
;; register 77 life shortened from 170 to 138
lreg: Register 77 used 9 times across 276 insns; dies in 5 places; crosses 2 calls; GR_REGS or none; pointer.
lreg: (insn 14 12 16 (set (reg/v:SI 77) (mem:SI (plus:SI (reg:SI 0 $0) (const_int 20)))) 159 {movsi_internal2} (nil) (expr_list:REG_EQUIV (mem:SI (plus:SI (reg:SI 0 $0) (const_int 20))) (nil)))
ALLOCDBG func=func_80027AD8 ord=30 pseudo=72 hardreg=20 nrefs=14 livelen=188 pri=2234
ALLOCDBG func=func_80027AD8 ord=33 pseudo=84 hardreg=21 nrefs=7 livelen=92 pri=1521
ALLOCDBG func=func_80027AD8 ord=35 pseudo=81 hardreg=22 nrefs=16 livelen=578 pri=1107
ALLOCDBG func=func_80027AD8 ord=36 pseudo=77 hardreg=23 nrefs=9 livelen=276 pri=978
ALLOCDBG func=func_80027AD8 ord=37 pseudo=80 hardreg=30 nrefs=4 livelen=94 pri=851
== no_tbl:77  conflicts: 2 3 4 5 6 7 16 17 18 19 20 21 22 29  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
== no_tbl:84  conflicts: 2 3 4 5 16 17 18 19 20 29  someone_prefers: (empty)  own_* (empty)
```
no_arg (tbl passed to the calls), verbatim:
```
lreg: Register 80 used 11 times across 149 insns; dies in 7 places; crosses 4 calls; GR_REGS or none; pointer.
ALLOCDBG func=func_80027AD8 ord=31 pseudo=80 hardreg=21 nrefs=11 livelen=149 pri=2214
ALLOCDBG func=func_80027AD8 ord=34 pseudo=84 hardreg=22 nrefs=7 livelen=92 pri=1521
ALLOCDBG func=func_80027AD8 ord=36 pseudo=81 hardreg=23 nrefs=16 livelen=576 pri=1111
ALLOCDBG func=func_80027AD8 ord=37 pseudo=75 hardreg=30 nrefs=4 livelen=122 pri=655
```
no_arg_rec (the parameter passed to the calls), verbatim:
```
flow: Register 77 used 5 times across 132 insns; dies in 3 places; crosses 3 calls; pointer.
;; register 77 life shortened from 132 to 102
lreg: Register 77 used 5 times across 204 insns; dies in 3 places; crosses 3 calls; GR_REGS or none; pointer.
ALLOCDBG func=func_80027AD8 ord=31 pseudo=80 hardreg=21 nrefs=8 livelen=130 pri=1846
ALLOCDBG func=func_80027AD8 ord=37 pseudo=75 hardreg=30 nrefs=4 livelen=123 pri=650
ALLOCDBG func=func_80027AD8 ord=38 pseudo=77 hardreg=-1 nrefs=5 livelen=204 pri=490
```
Local-alloc: no pseudo 72-99 of any body is a local-alloc quantity ("(none: every pseudo 72-99 is
global)" in every q19/rtl/<tag>.txt; all their Register lines lack "in block"), so local-alloc's
suggestions (local-alloc.c :1512, :2207-2212, :2283-2298) never touch them.

## (D)(2) Mechanism
1. **assign_parms puts a REG_EQUIV on a stack parameter's load.** function.c:3817-3855: a parameter
   passed in memory with no conversion is copied to its pseudo, and the copy insn gets
   `REG_EQUIV (mem <incoming slot>)` (no_tbl insn 14 above; the same insn in every body).
2. **update_equiv_regs halves its priority.** local-alloc.c:1058-1064: for a pseudo set once
   (`reg_n_sets[regno] != 1` -> skip, :1018-1021) that carries a REG_EQUIV note, `reg_live_length[regno] *= 2`.
   no_tbl's rec: sched1 leaves 138 (";; register 77 life shortened from 170 to 138"), .lreg / ALLOCDBG
   show 276. global.c:615/643 orders allocnos by floor_log2(refs)*refs/live_length, so rec ranks at
   978 instead of 1956. (vec, 82, carries a REG_EQUIV constant and is doubled the same way in every body:
   289 -> 578.)
3. **A copy in which the parameter dies escapes the halving.** combine (flow -> combine) merges the
   parameter load into the first copy because rec dies there (flow insn 36 `REG_DEAD 77`): combine's
   insn 36 is `(set (reg/v:SI 80) (mem ... 20))` with no note and insn 14 is deleted. update_equiv_regs
   adds no note of its own either (its MEM case, :1051-1055, needs `reg_basic_block >= 0`, but tbl is
   global), so tbl keeps 134 and ranks 2014. cse.c make_regs_eqv turned `tbl_arg = rec` into
   `tbl_arg = tbl` (flow insn 39: `(set (reg/v:SI 81) (reg/v:SI 80))`).
4. **global.c find_reg (:952) seats the copies.** No allocno here has own preferences or someone_prefers
   (FINDREG above), so each takes the lowest register not in `used` (pass 0 = registers already used,
   pass 1 = any non-conflicting): tbl (2014, after pass 2234) -> pass 1 -> $s5 (21). cat -> $s4, sign ->
   $s0 by pass 0. opp (1505) conflicts with tbl (both live at the entry) -> $s6. same (1428) does not
   conflict with tbl (tbl is dead in R6a, whose paths all return) -> pass 0 -> $s5, the target's
   `addu $s5,$v1,$zero` at 0x80027D8C. vec -> $s7. tbl_arg (851) conflicts with $s0-$s7 -> $fp (30).
   thresh (650) conflicts with $s0-$s7 and $fp -> no register; reload gives it the stack slot 0x18
   (`sw $7,24($sp)`, the target's `sw $a3, 0x18($sp)`). Frame 0x50.
5. **After allocation** (toplev.c order: reload, sched2, jump2, dbr_schedule). reload rewrites pseudos
   to their hard registers and adds only thresh's stack home. greg insn 39 `(set (reg/v:SI 30 $fp)
   (reg/v:SI 21 s5))` survives jump2 (q19/rtl/cand.jump2.fp_copy.txt, from cand.jump2 line 166: `(insn 39 1633 43 (set (reg/v:SI 30 $fp)` ... `(reg/v:SI 21 s5))`). dbr_schedule moves it into the delay
   slot of the first branch, as in the target (`bne $v1,$v0,.L80027C34` / `addu $fp,$s5,$zero`). None of
   these passes changes which register an operand uses, and none deletes or adds a copy between $s5 and
   $fp.

## (D)(3) Necessity — each copy, every copy-free spelling
**Target facts** (asm/funcs/func_80027AD8.s): (T1) the record is loaded ONCE from its stack slot, into
$s5, AFTER `lh $s2,4($s1)` and `lw $s6,0($s1)` (0x80027B34 `lh $s2,0x4($s1)`, 0x80027B38 `lw $s6,0x0($s1)`, 0x80027B3C `lw $s5,0x64($sp)`); (T2) $fp is written exactly
once, by `addu $fp,$s5,$zero` (0x80027B64, in a branch delay slot), and is read only as `addu $a3,$fp,$zero` at the three
func_800278C0 calls; (T3) $s5 holds the record for every field read and the NULL test and holds `same`
in R6a; (T4) thresh has no register (0x18(sp)); (T5) opp is in $s6.

**Copy A (`tbl`).** Without `tbl`, the field reads go through one of: the parameter pseudo, or
`tbl_arg` (then there is only one copy, which is the no_arg spelling, covered under Copy B). Through the
parameter, two independent mechanisms keep the bytes from the target:
- *Priority (T3/T5).* The parameter is set once (prong (A): never written) and keeps its REG_EQUIV, so
  its live length is doubled (Mechanism 2). A spelling may add a write to it only through a FAKE family:
  a self-assign `rec = rec;` emits no insn (expr.c store_expr :2845 `temp != target`), and a dead
  `rec = ...` store is deleted by flow before sets are counted (flow.c :1479 insn_dead_p; Ruling 2).
  Both are measured byte-identical to no_tbl with rec still 9 refs / 276 (no_tbl_selfassign,
  no_tbl_deadstore). Any live second write is a new value in the parameter, which Ruling 11 (A) and
  Ruling 12 (A) forbid. So the only lever left is extra references. A combine-foldable detour folded
  already at tree level adds none (no_tbl_chain: 9 refs, identical bytes). A cancelling live use does
  add some: `(s32)rec + (s32)rec - (s32)rec` adds 2 refs per pair (no_tbl_cancel: 11). The exact count
  that ranks rec between opp and pass is 15 refs = 3 pairs (no_tbl_cancel3: `pseudo=77 hardreg=21
  nrefs=15 livelen=278 pri=1618`, opp -> 22, vec -> 23, tbl_arg -> 30, thresh -1). That reaches the
  target's register assignment. So priority ALONE does not prove necessity, and the second mechanism
  carries it:
- *Load position (T1).* The parameter's load is insn 14, emitted by assign_parms before any statement,
  so its LUID is lower than every body insn's. In sched1 the parameter loads and the player/opp loads
  are all birthing-boosted to the same maximum priority (sched.c adjust_priority; BB2_SCHED_DEBUG on
  no_tbl_cancel3, q19/rtl/no_tbl_cancel3.sched1_entry.txt: `ADJPRI insn=12/14/16/30/33/18 ... birth=1`, then `PICK ... 33, 30, 18, 16, ...`).
  Equal-priority ties are broken by LUID, so the load stays ahead of `lh $s2` / `lw $s6`
  (post-sched1 order `12 14 16 18 30 33`). sched2, run after reload with no birthing boost, keeps the
  load ahead of player/opp as well (q19/rtl/no_tbl_cancel3.sched2.order.txt: `... 1709 14 53 ... 10 30
  33 ...` against cand's `... 10 30 33 36(tbl load) ...`). The final asm then has `lw $21,100($sp)`
  right after `sw $21,60($sp)` in the prologue: the only residual of no_tbl_cancel3, sandbox 6/574
  (hunks: the moved `sw s5`/`lw s5` pair). A load placed after opp must be emitted after `opp = ...`,
  i.e. by a statement, and the only statement that loads the parameter's value unchanged is a copy.
  Staging opp through itself first (c3_stg_opp: `opp = ch; player = *(s16 *)(opp + 4); opp = *(u8
  **)opp;`) does not move the load either (sandbox 6, same hunks). A copy whose read is the parameter's
  last use is merged with the load by combine (Mechanism 3), which is what puts the load at the
  statement's position in cand.
- *Preferences / local-alloc / post-allocation.* rec has no own or someone preferences (FINDREG
  no_tbl:77) and is not a local quantity. The post-allocation passes do not move a register assignment
  (Mechanism 5).
- *Other families.* do-while around the entry: no_tbl_dowhile 48. Pointer alias: an alias of the
  parameter IS a copy, which is this ruling's subject. Duplicated-into-arms / cross-jump: see arg_sink
  below (cse merges per-site copies into one value).

**Copy B (`tbl_arg`).** (T2) requires, at allocation time, an insn `(set P_fp P_s5)` between two distinct
long-lived pseudos that hold the record: fp's value is copied from $s5's, not loaded. The candidates for
P_fp:
- *None (no_arg).* With no 10th long-lived allocno, $fp goes to thresh (`pseudo=75 hardreg=30`) and the
  calls read $s5. T2 and T4 fail; sandbox 52.
- *The parameter (no_arg_rec).* Its load has REG_EQUIV and is halved (132 -> 102 -> 204; pri 490 <
  thresh 650). It gets no register (`hardreg=-1`), so each call reloads it from 0x64(sp); sandbox 57.
  Raising its refs with cancelling uses cannot give T2 either. Now the parameter is live after the field
  copy `tbl = rec`, so combine cannot merge the load into tbl (rec does not die there). The stack load
  therefore targets the PARAMETER and tbl becomes a register copy FROM it, the reverse of T1/T2.
  Measured: no_arg_rec_cancel2one: `lw $23,100($sp)` ... `move $21,$23`, with $fp not set from the
  record at all; no_arg_rec_cancel1/2/3 seat the parameter in $s7/$s5/$s0, and none has a `move $fp,...`
  from the record.
- *A compiler temporary.* expand frees expression temporaries at the end of each statement, cse makes no
  new pseudos, loop.c has no loop here, and reload creates no long-lived copies. None of them can hold
  the record across the three calls.
- *Per-call-site copies (arg_sink, hoist/sink + duplicated-into-arms).* cse merges each
  `tbl_arg = tbl;` into tbl, which gives the same code as no_arg (sandbox 52).
- *FAKE families on the parameter* (no_arg_rec_selfassign, no_arg_rec_deadstore): byte-identical to
  no_arg_rec (57).
So the fp pseudo must be a C variable whose value is the record, written from the record: a copy. The
permuter campaign from the copy-free body rediscovered exactly this form. Its finds output-1703-1 /
1719-1 add `new_var = (s32)rec` passed to func_800278C0 (see (D)(4)).

## (D)(4) Measured alternatives (sandbox `--disable all`, stock cc1)
| body | score/574 |
|---|---|
| cand (both copies) | 2 (only the jump-table relocation addend, `lw v0,%lo(jtbl)(at)`; full build certifies it) |
| no_tbl | 35 |
| no_arg | 52 |
| no_arg_rec | 57 |
| neither (= candidate.c) | 80 |
| arg_sink | 52 |
| no_tbl_dowhile | 48 |
| no_tbl_selfassign / _deadstore / _chain | 35 / 35 / 35 |
| no_arg_rec_selfassign / _deadstore | 57 / 57 |
| neither_selfassign / _deadstore | 80 / 80 |
| no_tbl_cancel2 / cancel3 / cancel4 | 19 / 6 / 25 |
| c3_stg_opp | 6 |
Structural respellings measured earlier (evidence.md s2): the two-copy family was reached only after
the count-block, dispatch and entry-order respellings; the copy-free twins of the final body are above.
Permuter: campaign `r11-neither80` from q19/v/neither.c (2 workers, 32413 iterations, stopped): the best
find is 1679 (base 3178). Every find adds a bare copy or alias of another value (`new_var = player`,
`= scr`, `= ch + 0x286`, `= &vec[2]`). output-1703-1 and 1719-1 add `new_var = (s32)rec` passed to
func_800278C0, which is copy B. The earlier campaign base97 (3500 iterations) found nothing valid.

## The missing final `return` (owner question, settled from the target bytes)
The body ends with the switch and no `return` statement. The target's switch-default path is:
`.L80028198: beqz $v0, .L8002839C / nop`. It is reached ONLY from three branches, each of which sets
$v0 in its delay slot: `bne $s4,$v0,.L80028198 / sltiu $v0,$s0,0x16` (0x80028114),
`bne $a1,$v0,.L80028198 / sltiu $v0,$s0,0x16` (0x80028124) and
`beqz $v0,.L80028198 / sltiu $v0,$s0,0x16` (0x8002814C). The instruction before `.L80028198` is an
unconditional `j`, so nothing falls through into it. `beqz $v0` is taken only when $v0 == 0, and
`.L8002839C` is the epilogue: ten `lw` restores, `addiu $sp,$sp,0x50`, `jr $ra`, with no write to $v0.
So on that path the original returns $v0 == 0, the value of `limb < 22`, and never materializes it.
The caller func_80031B24 reads the result (`r == 2`, `r != 0`) and sees 0; func_8002AB08 ignores it.
GCC emits exactly these bytes only when the function has no return statement on that path. Measured:
a final `return 0;` scores 56 (it adds `move $v0,$zero` before the epilogue and re-routes the
cross-jumps of the 5-state tails), and `if ((u32)limb >= 0x16) return 0;` before the switch scores 64.
The behavior is deterministic (v0 is provably 0), and the C matches it by leaving that path without a
return statement, as the original source did.
