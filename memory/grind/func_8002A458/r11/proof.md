# func_8002A458 — Ruling 11 submission for `dx`/`dy`/`dz`, `temp` and `temp2` (manual s2, 2026-09-28)

Rulings: `.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11 (owner, 2026-09-26) and its
(C)(3) **GTE-macro input copies** clause (owner, 2026-09-28, sixteenth batch, Q28 — whose owner
text names func_8002A458). This is a FRESH submission. The 2026-09-25 layer-2 FAIL of `lzc_in` /
`len` / `dx`,`dy`,`dz` (Rulings 5/6/9, staged-value bound 2) is not contested; Ruling 11 is a
different test, and Q28 says `lzc_in` "may return only as a fresh submission meeting (C)(3)'s
2026-09-28 GTE-macro input copy clause and every other prong".

Files here:
- `final.c` — the submitted body (byte-identical to the splice into src/code6cac_b.c).
- `one-var-per-value-form.c` — the one-variable-per-value spelling (C)(1).
- `variants/*.c` — every measured spelling below; `variants/scores*.txt` their sandbox lines.
- `dumps.txt` — (D)(1) excerpts; `dumps.sh`, `fr.sh`, `mk.py`, `collect.py` produced them.
- `genr11.py` / `mkM.py` generate the variants from `variants/final.c`; `mkperm.sh` / `permx.py`
  the permuter workspace and the find extraction.

Scores are `sandbox func_8002A458 --disable all` (416 target insns), current build compiler
(stock cc1, PLUS->IOR patch removed). The variants carry the submitted body's header-exact
gte_Lzc islands; the (D)(4) numbers were also measured on the earlier joined-island spelling
and are identical.

## The variables and their values

```c
dx = tip.x - base.x;                   /* W1: (*(s32 **)(scr+0x64))[0] - (*(s32 **)(scr+0x60))[0] */
  ... ratan2(dx, dz); dx*dx (temp); ratan2(dx, dz); dx*dx (len_sq); (dx << 12) / temp   R1..R5
dx = hit.x - p[0];                     /* W2: *(s32 *)(scr+0x100) - p[0], p = *(s32 **)(scr+0x60) */
  hit_sq = dx*dx + ...                                                                  R6
dx = obj.x - p[0];                     /* W3: *(s32 *)(obj+0xF4) - p[0] */
  if (hit_sq >= dx*dx + ...)                                                            R7
```
`dy` and `dz` are the same with the y/z components (+4/+8, `0xF8`/`0xFC`, `p[1]`/`p[2]`); `dy`'s
R-set is the two sums, `ratan2(dy, hlen)` and `qy`; `dz`'s is both `ratan2(dx, dz)`, the sums and
`qz`. Each variable has exactly three values: W1 reaches only R1..R5 (W2 kills it before R6), W2
only R6, W3 only R7.

```c
temp = dx*dx + dz*dz - dy*dy;          /* W1 */
  printf(.., temp); temp < 0; temp2 = temp; (u32)temp < 0x400; LUT[temp]; (u32)temp >> shift
temp = LUT[len_sq] >> 3;               /* W2 (small arm)  */
temp = (tbl << 16) >> (0x13 - ..);     /* W3 (LZC arm)    */
  qx/qy/qz = (d << 12) / temp          /* read by both W2 and W3 -> one value */
```
Two values: the squared horizontal length {W1}, and the segment length {W2, W3} (both reach the
three divisions). The horizontal length is NOT in `temp`: it is its own local `hlen` (measured:
sharing it is unnecessary, `temp_r1_own` = the final body scores 0; see (D)(4)).

```c
temp2 = temp;                          /* W1: the copy, read ONLY as gte_ldlzc's "r" input */
  __asm__ volatile ("move  $12,%0": :"r"(temp2): ...);
temp2 = LUT[(u32)temp >> shift];       /* W2: the table byte */
  hlen = (u32)(temp2 << 16) >> (0x13 - ..);
```
Two values: the island input copy {W1}, the table byte {W2}.

## (A) Fresh local, not a borrow
All five are locals of func_8002A458, declared once, not parameters/globals/static/register, no
`&` anywhere. Scope: `dx`/`dy`/`dz` have writes at function level (W1) and inside
`if (*hit != 0)` (W2/W3), so function scope is the innermost enclosing scope; `temp` has writes
at function level (W1) and inside both arms of the site-2 if/else, so function scope; `temp2`
has W1 at function level (before the site-1 `if`) and W2 inside the LZC arm, so function scope.
No other declaration moved or re-scoped relative to the one-variable-per-value form (diff: only
the added per-value declarations and identifiers).

## (B) Every write is live
(1) dx/dy/dz W1 is read by the first ratan2 / the squared sum on every path; W2 by `hit_sq` and
W3 by the comparison, both on the path `*hit != 0` (feasible: *hit is set by the limb loop).
`temp` W1 is read by `temp < 0` on every path; W2/W3 by the divisions on every path that gets
past the site-1 `return`. `temp2` W1 is read by the island on the path `(u32)temp >= 0x400`
(feasible: any segment longer than 32 units); W2 by `temp2 << 16` on the same arm.
(2) Re-store check (Ruling 5 2(c), 2026-09-26 clarification) — no write stores a value the
variable holds on every feasible incoming path:
- dx W2 stores hit.x - base'.x where base' = the base after `**(scr+0x60) = **(scr+0x64)` (the
  old tip). Differing path: base.x = 0, tip.x = 100 (dx holds 100), hit.x = 150: W2 stores 50.
  W3 stores obj+0xF4 x minus base'.x; differing path: hit.x = 150 (holds 50), obj.x = 100: W3
  stores 0. Same records for dy/dz with their components.
- `temp` W2/W3 store the segment length |d| (rounded LUT sqrt); held value is dx²+dz²-dy².
  Differing path: dx = 100, dy = dz = 0: holds 10000, stores about 100.
- `temp2` W2 stores a D_8008D118 byte (0..255); held value is the squared length, >= 0x400 on
  that arm (the arm is entered only when (u32)temp >= 0x400), so it never already holds W2's value.

## (C) Same statements; real computations
(1) `one-var-per-value-form.c`: dx/dy/dz W1 stay in `dx`/`dy`/`dz`; W2 in `hx`/`hy`/`hz`, W3 in
`ox`/`oy`/`oz`, declared at the top of `if (*hit != 0) {` (the innermost block enclosing their
writes); `temp` W1 in `h_sq` and W2/W3 in `len`, both function scope; `temp2` W1 in `n` (function
scope: its write precedes the site-1 `if`) and W2 in `tbl`, declared in the LZC arm's inner block.
(2) `diff variants/final.c one-var-per-value-form.c` = added declarations and changed identifiers
only; no statement added, removed or moved.
(3) dx/dy/dz: every write is a load-subtract (`lw` + `subu` into s2/s3/s1 in the target at all
three sites). `temp` W1 is the mult/mflo/addu/subu chain ending `subu $a1,$v0,$v1`; W2 `srl
$a1,$v0,3`, W3 `srlv $a1,$a0,$v0`. `temp2` W2 is `lbu $a0,%lo(D_8008D118)($at)`. `temp2` W1 is a
bare copy, admitted only under the Q28 clause, next.

### (C)(3) GTE-macro input copy clause (Q28) for `temp2` W1
- (a) One write, `temp2 = temp;`, whose whole right-hand side is the named local `temp`, no
  cast; `temp` is read after the copy on every path (the `< 0x400` test, then the LUT index or
  the shift index).
- (b) Its only reader is the bare variable as the whole "r" input operand of the
  `move  $12,%0` statement of the gte_ldlzc unit of the site-1 gte_Lzc island, a qualifying
  unit (§ Scorer ruling (A)-(B): the inline_o.h expansion statement for statement and character
  for character against engine/gtemacro.py PINNED gte_ldlzc :207-210 / gte_nop :1095-1097 /
  gte_stlzc :1074-1077, with an `mtc2`), itself admitted in this body under § Owner ruling
  2026-09-26 (inline_o.h class; `auth:` row lands before the body). No C statement reads W1.
- (c) After it, on the LZC arm, `temp2` holds W2, the table byte: a load whose `lbu` is in the
  target (0x8002A5E8), not a copy or constant.
- (d) The copy is in the target's bytes: `addu $a0,$a1,$zero` at 0x8002A590, in the delay slot
  of the `beqz` into the LZC arm; the build emits it at the same position (sandbox 0, no scored
  hunk).
- (e) Every other prong: this file.

## (D) Allocator-dump proof of necessity

### (D)(1) Dumps — `dumps.txt`
Commands: `dumps.sh` and `fr.sh` (run as tmp/func_8002A458/*.sh; they call mk.py: the variant is
spliced into src/code6cac_b.c, preprocessed with engine.buildconfig's CPP flags and compiled with
`cc1 <CC_FLAGS> -da`). stock = tools/gcc-2.7.2/build/cc1 (the build compiler), dbg =
tools/gcc-2.7.2/cc1 (BB2_ALLOC_DEBUG / BB2_FINDREG_DEBUG=89); every dbg `.s` is identical to its
stock `.s` (dumps.sh prints `stock==dbg asm` for all five spellings).

### (D)(2) Mechanisms, one per variable

**dx/dy/dz — allocation class by call crossing (global.c find_reg; local-alloc.c).**
Final: pseudos 83/84/85 (`dx`/`dy`/`dz`) are `used 14/13/14 times across 95-98 insns; crosses 4
calls` (f.lreg) and global-alloc seats them `hardreg=18/19/17` = s2/s3/s1 (ALLOCDBG ord 17/18/16),
the target's seats at all three sites. A pseudo that crosses calls cannot take a call-clobbered
register on find_reg's first pass (global.c:970-983: `used1` starts from `call_used_reg_set`
when calls are crossed), so it takes a free call-saved one.
One-var form (`dxyz_all_own`): the six end-block pseudos 272-277 are `used 3 times across 2
insns in block 30`: every reference in one basic block, crossing no call, so local-alloc
allocates them (block_alloc), `;; Register 27x in 2.` = $v0: find_free_reg takes the lowest free
hard register in numeric order (config/mips defines no REG_ALLOC_ORDER), and a quantity that
crosses no call may take a call-clobbered one. The target has `subu $s2/$s3/$s1` there.

**temp — two decisions (global.c set_preference/find_reg copy preference; cse.c make_regs_eqv).**
(i) *The segment length's register (primary).* Final: `temp` (87) is the operand of printf's
argument move `(set (reg:SI 5 a1) (reg/v:SI 87))` (f.lreg insn 121), so set_preference
(global.c:1671, copy=1) gives it a hard-reg COPY preference for $a1: FINDREG `own_copy_prefs: 5`,
`own_full_prefs: 5`, conflicts `2 3 4 12 13 14 15 29`, ALLOCDBG ord 0 hardreg 5. Its later value,
the segment length, therefore sits in $a1 (`srl $a1,$v0,3`, `srlv $a1,$a0,$v0`, the three
`div $zero,$v0,$a1` / `bnez $a1` / `bne $a1,$at`), as in the target. One-var forms: the segment
length in its own `len` (91 in temp_all_own and in the reviewer's temp_temp2_split): `own_copy_prefs`
EMPTY, `own_full_prefs: 3 4`, conflicts `2 29`, ALLOCDBG ord 0 hardreg 3 = $v1 (dumps.txt, fr2.sh):
the seat hunks 9-18 of both spellings' diffs.
(ii) *The cse canonical register (only while `temp2` is shared).* When cse1 records
`temp2 = temp`, the copy becomes the quantity's canonical register only if it lives beyond the
cse block AND its last use is later than the current canonical register's
(`uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`, cse.c:844-857). Final:
`temp` (87) is last used at the site-2 divisions, so it stays canonical; the `< 0x400` test reads
87 before and after cse (f.rtl / f.cse), i.e. $a1, as in the target. temp_all_own: the squared
length `h_sq` (90) is last read at the shift index, before `temp2 << 16`, so the copy (89) becomes
canonical and cse1 rewrites the test operand from 90 to 89 (f.rtl `(ltu (reg 90))`, f.cse
`(ltu (reg 89))`): hunks 2/4/5 of its diff. With `temp2` also split (temp_temp2_split, the
reviewer's C1) the copy's last read is the island, before the squared length's last read, so the
squared length stays canonical and (ii) does not arise; (i) still fails the spelling.
temp_all_own = 15 = (i) + (ii); temp_temp2_split = 13 = (i) + temp2's own copy seat.

**temp2 — conflict + preference in find_reg (global.c set_preference :1681, find_reg).**
Final: FINDREG for the copy/table-byte pseudo 89: `conflicts: 2 3 5 12 13 14 15 29`,
`own_full_prefs: 4`, hardreg 4 = $a0 (ALLOCDBG ord 1), the target's copy register. The $v1
conflict comes from 133-135 (the lz/shift code inside 89's range, local-allocated to $v1); the
$a0 preference from `(set (reg 139) (ashift (reg 89) 16))` with 139 local-allocated to $a0
(set_preference's first-operand rule: a non-copy SET_SRC is reduced to its first operand, and a
hard or local-allocated destination gives the global source a full preference). One-var form
(`temp2_split`): the copy n (89) is read only by the island; `conflicts: 2 5 12 13 14 15 21 29`
(no 3), `own_copy_prefs` and `own_full_prefs` EMPTY, so the first free register in numeric
order, 3 = $v1 (ALLOCDBG ord 13). The target has `addu $a0,$a1,$zero`.

### (D)(3) Necessity, not effect
Standard: Ruling 11 (D)(3) as amended by Q31 (owner 2026-09-28, eighteenth batch, "Mechanism +
search", ee84164e3): (a) the mechanism for each variable is named by pass and source location
from banked dumps of the reuse and one-variable-per-value spellings ((D)(2) above, dumps.txt);
(b) every one-variable-per-value spelling proposed by the author or a reviewer is banked in
variants/ with its sandbox score (scores*.txt), including the round-1 reviewer's C1 (13), c5 (2),
c6 (4), c7 (37) and c8 (2); (c) no banked counting spelling reaches the target (all > 0; the
FAKE-carrying ones M1-M3/M5 are measured too, and miss as well). The per-variable arguments
below go further than Q31 requires and argue the property for every per-value spelling; where a
sentence there is stronger than the banked measurements, the measurements are what this
submission rests on.


**dx/dy/dz — property of the reuse spelling:** the end-block delta is written into a pseudo that
crosses calls (it also holds the segment delta, live across ratan2 / func_80032854 /
func_8002E838 / func_80053614), so find_reg may not give it a call-clobbered register.
**Every one-variable-per-value spelling lacks it, because W2 and W3 have their own variables:**
by (C)(2) the per-value variable is written and read only by the fixed end-block statements
(the three subtractions and the `hit_sq` sum / the comparison), which lie in one basic block
after the last call of the path (func_80054434 in the enclosing condition) and before the next
(func_80032854, reached only after the comparison). Its live range therefore crosses no call and
touches one block in any declaration order, scope (block or function), type, or statement order
that (C)(2) allows, so local-alloc allocates it; and if it were ever left to global-alloc, the
first pass of find_reg (no call crossed, so call-clobbered registers are allowed, numeric order,
and the fixed statements copy it into no hard register, so no preference) would still start at
$v0..$t7. It can never be seated in s2/s3/s1.

**temp — property of the reuse spelling:** the segment length is written into the pseudo that is
also printf's argument, i.e. into the one pseudo that the argument move `(set $a1 temp)` gives an
$a1 copy preference, so find_reg seats it in $a1.
**Every one-variable-per-value spelling lacks it, because the segment length has its own
variable:** by (C)(2) that variable is written only by the two site-2 arms (`LUT >> 3` and the
LZC-arm shift) and read only by the three divisions `(d << 12) / len`. None of those insns moves
it to or from a hard register or a local-allocated pseudo (the divisions read it as the divisor
of `div`, the writes set it from shifts of temporaries), and the only insn in the function that
relates any of these values to $a1 is printf's argument move, which reads the squared length, a
different variable in every per-value spelling. So its copy-preference set never contains $a1,
whatever the declaration scope, order or type. expand_preferences (:829-871) cannot import one: no
statement copies the squared length into it or it into the squared length. Measured in both
per-value shapes: `own_copy_prefs` empty, `own_full_prefs` {v1, a0}, conflicts {v0, sp}, and it is
the highest-priority allocno (ord 0, pri 26666 against 17142 next), so it is allocated before any
other pseudo can occupy a register and takes $v1. Were it allocated later instead, the only way to
$a1 would still be a preference or $v1/$a0 being taken for its whole range, which the fixed
statements do not produce (its range, the site-2 LUT sqrt and the divisions, holds only
short-lived temporaries that local-alloc places around it). It is never seated in $a1.
This holds with `temp2` shared or split, so it covers every per-value spelling of `temp` on its
own; mechanism (ii) is an additional miss only while `temp2` is shared (hunks 2/4/5).

**temp2 — property of the reuse spelling:** the island-input pseudo is also the table-byte
pseudo, so its live range runs through the lz/shift code (a $v1 conflict) and it is the first
operand of `<< 16` into an $a0 local (an $a0 preference).
**Every one-variable-per-value spelling lacks it, because the copy has its own variable:** by
(C)(2) and Q28 (b) that variable's only reader is the island, so its range is copy to island:
the pseudos live there are the squared length (a1), the `sltu` result (v0), $t4-$t7 (island
clobbers), $s5 (scr) and sp, never $v1, and no insn sets it from, or uses it as the first
operand into, a hard or local-allocated register (the copy's source `temp` is a global pseudo,
unallocated when set_preference runs, and does not die at the copy, so expand_preferences
:829-871 merges nothing). With no preference find_reg returns the lowest free register, $v1,
whatever the allocation order. Placement: the copy must precede the `if` (inside the LZC arm
combine folds it into the island: measured 23 on the earlier chassis, hypotheses.md S2-4);
declaring it `u32` changes nothing (M4 = 2).

### Sanctioned FAKE families (each excluded by mechanism, one measured per variable)
- **Dead store** (dead-store-fake-exception): a dead write of a per-value local is deleted by
  flow before allocation (insn_dead_p), so it adds no range.
- **Self-assign** (`hx = hx;`): measured with all six end-block locals at function scope,
  self-assigned at entry (M2): 37, identical to the per-value spelling without them.
- **Combine-foldable chain-extender**: its sanctioned effect is reg_n_refs only. None of the
  three mechanisms reads reg_n_refs: the dx/dy/dz class depends on calls crossed / blocks
  touched, `temp` on the cse last-use position, `temp2` on conflicts and preferences. Measured
  on `h_sq` (M3, `(u32)(h_sq + ((h_sq << 4) & 0xF)) < 0x400`): 15, unchanged. A detour that
  instead EXTENDS a live range is outside the family's scope; measured anyway on the copy (M5,
  a zero-valued `((s32)n << 4) & 0xF` read at the `<< 16` use): 2, unchanged.
- **Cancelling / annihilating use, pointer alias**: an alias takes the variable's address
  (addressable, so a stack slot; excluded by (A) for the per-value locals as well); a cancelling
  pair adds references inside the existing range (reg_n_refs only, as above).
- **do{...}while(0)**: measured around the per-value end block (M1): 89. It raises the loop-depth
  weight of references (reg_n_refs), not the block count or the calls crossed.
- **Duplicated-into-arms + jump2 cross-jump**: the end-block deltas have no branch between their
  write and their read, and the copy none between its write and the island; duplicating a write
  needs an `if` that is not in the (C)(2) statement list, and a cross-jump re-merge runs after
  allocation, so it cannot change any decision above.
- **Hoisting/sinking writes across branches**: moving a per-value write out of its block changes
  the statement list ((C)(2)); for the copy, sinking it into the arm is the measured 23 above.
- **Post-allocation passes** (jump2 no-op-move deletion toplev.c ~3142, sched2, reorg
  redundant_insn): they act after the seats are chosen and never renumber registers.

### (D)(4) Measured alternatives (sandbox --disable all)
- Full one-variable-per-value spelling (`one-var-per-value-form.c`): **50** (417 insns).
- Ablations, each value split out with the rest still shared:
  - dx/dy/dz: W2 own (hx..hz), W3 still shared: **6**; W3 own (ox..oz), W2 shared: **6**; W2 and
    W3 own: **37**; one trio shared by W2 and W3 but not W1: **37**. Per variable (that
    variable's W2 and W3 both split, the other two reused): dx **26**, dy **34**, dz **40**.
  - temp: squared length own (`h_sq`): **15**; segment length own (`len`): **15**; both: **15**
    (hunks 9-18 = the segment-length $a1 seat, hunks 2/4/5 = the cse swap). Both split AND
    `temp2` split (the 2026-09-28 reviewer's C1, variants/temp_temp2_split.c): **13** (the
    segment-length seat + the copy seat; the cse swap does not arise).
    (The horizontal length also sharing `temp` scores 0 too: not needed, so it has its own
    `hlen`.)
  - temp2: copy own (`n`) and table byte own (`tbl`): **2** (the $v1 seat alone). No copy at
    all (the island reads `temp`; Q28 (e)), header-exact islands (variants/nocopy.c, the
    reviewer's c8): **2** (the `move $a0,$a1` missing). Reviewer's c5 (`u8 tbl`) 2, c6 (copy
    before the `< 0` test) 4.
- Structural respellings: 320 single-role site-1 dataflow spellings (sum/copy direction, copy
  placement, every consumer on either variable, root into the sum's variable or a fresh one;
  s2/gen1.py, s2/gen1_scores.txt, measured on the dx/dy/dz-reuse chassis): floor **2**, none
  below. Inline `lut_sqrt()` helper: **15** (one LZCR slot for both calls: integrate.c:2092
  assign_stack_temp inside expand_assignment's temp level, expr.c:2660-2664). Site-1 copy staged
  through `len_sq`: copy coalesced into $a1. End block as fresh single-use locals with the direct
  comparison (the 2026-09-25 honest candidate): **30**. Copy inside the LZC arm: **23**. `u32`
  copy: **2**.
- Island alternatives (for the `auth:` row): islands removed 70 (402 insns); inline_c.h form
  (`mtc2 %0` / `swc2 0(%0)`, no `move $12`) 8 (412 insns); the old joined single-statement
  islands 0 (not a qualifying unit, so not used).
- Permuter from the one-variable-per-value body (tools/permuter_campaign.py, workspace by
  mkperm.sh, build cc1, --stack-diffs, -j4; base permuter score 1085 = sandbox 50):
  - Campaign 1 (tmp/perm_a458_onevar, joined-island one-var body): about 107,200 iterations,
    4,694 s, 196 finds, best permuter score 395. The 24 lowest, re-scored with the sandbox after
    restoring `__asm__` (the permuter prints `asm`, which the sandbox strips as unrecognised
    asm): 33-62, none at 0. The three lowest (33/34/36) move the gte_stlzc statements ahead of
    the gte_ldlzc/nop statements (the LZC result stored before its input is loaded: a changed
    island, invalid); the lowest valid one (38) splits a segment delta `dy = base.y; dy = tip.y -
    dy;` (one value each, no reuse). perm_scores.txt.
  - Campaign 2 (tmp/perm_a458_onevar_he, header-exact one-var body, fresh seed): about 35,300
    iterations, 1,518 s, 99 finds, best permuter score 275. The 16 lowest, sandbox-scored:
    37-59, none at 0. The ten with fewer than 416 insns move an island statement (e.g. the
    lowest, 37, moves gte_ldlzc's `mtc2` statement away from its `move`: no longer a unit, so
    stripped; invalid); the lowest intact find (45) splits a segment delta `dy = tip.y; dy = dy
    - base.y;` (one value, no reuse). perm2_scores.txt.

## (E) Honest names
- `temp`, `temp2`: form (i) generic scratch words.
- Layer-2 round 1 (2026-09-28): FAIL on the `temp` (D)(2)/(D)(3) argument only (the cse
  argument was presented as universal; it does not hold with `temp2` split, and the
  segment-length seat was unproven). Fixed above; body unchanged. dx/dy/dz, temp2, the
  islands, the do-while(0) and the rest were checked sound.
- Layer-2 round 2 (2026-09-28): PASS. Its 18 counter-spellings (temp split x6 = 15, temp +
  temp2 split x6 = 13, temp2 per-value x4 = 2, dx/dy/dz per-value x2 = 37) are banked in
  variants/review2/ with variants/scores_review2.txt; none reaches 0. Header comment wording
  fixed per its non-blocking notes (comment-only).
- `dx`, `dy`, `dz`: form (ii), a name for the one kind all values share: every write is
  `point.c - base.c`, the offset of a point from the segment base on that axis (W1 the tip, W2
  the stage hit point, W3 obj+0xF4; W2/W3 through `p`, the base pointer `*(s32 **)(scr+0x60)`
  that W1 also reads). "dx" = the x offset, true of every write; no value is anything else (not
  a length, not a square).

## (F) Annotation
Each declaration carries a comment naming its values and citing Ruling 11 (and Q28 for `temp2`)
and this file.

## (G) Layer-2
Required: fresh cheat-reviewer, default-FAIL, walking (A)-(H) for each variable with this file.

## (H) Everything else
- GTE islands: two header-exact gte_Lzc units (6 statements each), inline_o.h class route
  (inline-asm-policy.md § Owner ruling 2026-09-26): `auth:` row before the body, region hashes.
  (An owner-instructed registry row for this function also exists, f38e6053f, given for the
  joined form; the header-exact islands do not rely on it.)
- `do { rec = &D_800F5F68[id * 0x1B8]; } while (0);`: do-while-zero-exception (owner
  2026-07-06), FAKE-annotated; unwrapped, the rec/scr and id/obj seats swap: 16.
- `lz = ~1; lz &= sp_tmp;`: Ruling 4 compound split (the target's `li $v0,-2; and`).
- `hlen`, `hit_sq`, `len_sq`, `qx..qz`, `p`, `rec` (a cursor), `i`, `diff` (one value: the yaw
  difference folded into 0..0x800): single-meaning locals.
- `Vec3i` struct copies (`*(Vec3i *)(scr + 0xC8) = ...`, base := tip): the file's existing
  12-byte type; its typedef moves above this function (it was defined further down).
