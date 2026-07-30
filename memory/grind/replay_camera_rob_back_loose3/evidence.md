# Evidence bank — replay_camera_rob_back_loose3

## Session 1 (recon, 2026-07-30)

### Baseline
- `canonical` → verdict **C** (asm_insns 0, total 114, distance 17). Pure-C target, no
  canonical-asm route.
- `sandbox --disable all` on the HEAD form → **score 17**, target_insns 114,
  build_insns 115, rules_dropped 8.
- **The HEAD form carries two cheats that the sandbox does NOT strip** (it strips
  regfix/asmfix + cheat-asm only):
  1. `register s32 angC asm("$3");` — register pin. `$3` = `$v1`, which is exactly the
     register target uses for `angC`. The pin IS hypothesis B spelled as a cheat.
  2. `cosA = (s16)*(volatile u16 *)(&Judge[((s16)angA + 0x400) & 0xFFF]);` — volatile
     coercion on a game-state global (forbidden, `inline-asm-policy` expanded catalog;
     `Judge` is a static sin/cos table, not IRQ-touched, so
     `legitimate-volatile-interrupt-touched` does not apply).
- **Removing both cheats → honest cheat-free distance 26** (build_insns 113 vs target
  114). *This 26, not 17, is the real floor of the function.* Isolation measurements:
  pin removed / volatile kept → 17 (pin is score-inert, as designed);
  volatile removed → 26. So the entire 17→26 delta is the volatile.

### Diff decomposition (cheat-free baseline, 26 diffs, two INDEPENDENT mechanisms)
Mechanism **A — the `cosA` load (region tgt[62:66])**, ~4 diffs + register cascade:
```
TGT   lui at,%hi(Judge) ; addu at,at,a3 ; lhu v0,%lo(Judge)(at) ; sll v0,v0,16 ; sra v0,v0,16 ; mult v0,...
BLD   lui at,%hi(Judge) ; addu at,at,a3 ; lh  v1,%lo(Judge)(at) ;                              mult v1,...
```
Target loads cosA **unsigned** and sign-extends with an explicit `sll 16 / sra 16` pair
(hence target 114 insns vs our 113). Every other `Judge` read in the function is a plain
`lh` in BOTH target and build — so only this ONE read has the split shape.
The `volatile` cheat reproduced the split shape but ALSO forced the address into a real
register (`la $v0,Judge ; addu $a3,$a3,$v0 ; lhu $v0,0($a3)` = 4 insns, 115 total), which
is why the HEAD form still needs the three `@60-62` regfix substs to get back to the
`$at`-based `lui/addu/lhu %lo` form. Dropping the volatile fixes the addressing for free.

Mechanism **B — hard-register assignment**, 13 diffs, fully mirror-image:
| value | target | our build |
|---|---|---|
| `angC` (`lhu 4(a0)`, live through its `(s16)angC + 0x400` cos-index chain) | `$v1` | `$v0` |
| the `& 0xFFF`/`<<1` index temp for angC | `$v0` | `$v1` |
| `sinAxsinB_12` (`sra rD,t0,12`, then 2 `mult`s) | `$v0` | `$a0` |

`angA`→`$a3` and `angB`→`$a2` agree in both. So B is plausibly ONE root cause: if `angC`
lands in `$v1`, `$v0` is free for the index temp and (after the temp dies) for
`sinAxsinB_12`, which is exactly target's cadence. 10 of the 13 are the angC/temp swap
(lhu, andi, sll, `addu at,at,rX`, plus the 5-insn `sll/sra/addiu/andi/sll` cos-index chain
and its `addu at,at,rX`); 3 are `sra` + 2×`mult` on sinAxsinB_12.

A and B are **independent**: the with-volatile build (correct A shape) still shows all 13
B diffs, and the no-volatile build (wrong A shape) shows the same 13.

### RTL evidence for B (artifact: tmp/grind/.../s1/rtl/base.i.{lreg,greg})
- `angC`'s load is greg insn 44:
  `(set (reg/v:SI 2 v0) (zero_extend:SI (mem/s:HI (plus (reg a0) (const_int 4)))))`
  — already hard-assigned by **local-alloc** (lreg), not global-alloc. `angB` is insn 18
  → `a2`, `angA` → `a3`, both matching target.
- `sinAxsinB_12` is the pseudo lreg reports as "used 3 times across 5 insns"; greg
  dispositions give it hard reg 4 (`$a0`).
- The whole function is ONE basic block (block 0, no branches) — so no cross-block levers,
  and combine sees every def/use pair adjacently.

### Family / sibling map
- The 3x3 rotation-matrix family (6 `Judge` reads, `(u16 *a0, s16 *a1)` signature) has
  exactly **3 members, all in src/text1a_c.c, ALL still rule-carrying**:
  `_SelectSection` (10 rules), `replay_camera_rob_back_loose3` (8),
  `hirahira_w_ctrl_2` (63). **There is no matched template in this family** — solving
  this function is expected to unlock the two siblings.
- Wider `((s16)ang + 0x400) & 0xFFF` family: 16 functions; the 8 zero-rule ones
  (`disp_CalcFov`, `camera_CalcAngles`, `func_800475A4`, `func_80042F10`, …) have at most
  2 `Judge` reads and none exhibits mechanism A.
- **35 functions' target asm contains the `lhu` + (≤3 insn gap) + `sll 16 / sra 16`
  signature**; 29 of them are zero-rule/matched. The instructive matched precedent is
  `func_80065344` (src/text1b.c:14641):
  ```c
  u16 *p = &D_800F0BAA;  u16 v1 = *p;  v1 += 0x1C6;  *p = v1;  if ((s16)v1 < 0x11C8) ...
  ```
  The raw u16 there has a SECOND use (it is stored back), which is what stops combine from
  folding `zero_extend(mem)+sll+sra` into `lh`.

### Fold mechanism for A (measured — three spellings killed)
GCC 2.7.2 combine (`simplify_shift_const`) rewrites `ashiftrt(ashift(zero_extend(mem)))`
→ `sign_extend(mem)` = `lh` whenever the zero-extended value has a single use and is dead
after the chain. All three width-coercion spellings therefore collapse to `lh` (26/113):
`u16` staging local; `(s16)*(u16 *)&Judge[i]`; `u16`-typed `cosA` cast at all 4 use sites —
the last measured BOTH with `extern s16 Judge[]` and with all four `Judge` decls in
text1a_c.c retyped to `extern u16 Judge[]`. Only `volatile` (an un-combinable MEM) escaped
the fold, and that is the cheat.

Incidental finding: this GCC accepts conflicting-type `extern` redeclarations with only a
warning (text1a_c.c already contains such pairs, e.g. `D_800A9D04` at lines 1152/1191) —
so a "retype only this function's own decl" trick would *compile*, but it is a re-typed
alias view of the same object = the alias-rename cheat family. Do not.

### Tooling built this session (reusable)
- `tmp/grind/replay_camera_rob_back_loose3/s1/apply.py` — swap the function block in
  src/text1a_c.c for a variant file (type-agnostic on the `extern … Judge[];` boundary).
- `…/probe.ps1 <variant …>` — apply + `sandbox --disable all`, one line of score per
  variant. NOTE: the FIRST variant's row prints blank (output-capture quirk) — pass a
  throwaway variant first.
- `…/diffasm.py [obj]` — normalized target-vs-`.o` opcode diff via difflib (nop/immediate
  formatting noise is NOT normalized; read it as a shape diff, trust `sandbox` for scores).
- `…/dumpbld.sh <obj>` / `…/greg.sh` — objdump the function out of a whole-file `.o`;
  produce the cc1 `-da` RTL dump set into `s1/rtl/`.
- **`diagnose --detail` is NOT safe as an iteration tool here** — it reported the same
  d17 diff for a `src/` state whose sandbox score was 26 (it reads a stale object).
  Use `sandbox` for scores and `diffasm.py` on the fresh sandbox `.o` for shapes.

- [s1] canonical: verdict C, asm_insns 0, total 114, distance 17 — pure-C target, no canonical-asm route.

- [s1] sandbox --disable all on HEAD: score 17, target_insns 114, build_insns 115, rules_dropped 8 (8 regfix rules at regfix.txt:1000-1010).

- [s1] HEAD's C carries TWO cheats the sandbox does not strip: a register pin 'register s32 angC asm("$3")' ($3 = $v1 = exactly the register target wants for angC) and a volatile coercion 'cosA = (s16)*(volatile u16 *)(&Judge[...])' on a non-IRQ static sin/cos table.

- [s1] Removing both cheats raises the honest distance to 26 (build_insns 113 vs target 114). Isolated: pin removed alone -> 17 (score-inert, as designed); volatile removed alone -> 26. The real floor of this function is 26, not 17.

- [s1] Mechanism A (cosA load): target = 'lui at,%hi(Judge); addu at,at,a3; lhu v0,%lo(Judge)(at); sll v0,v0,16; sra v0,v0,16', our cheat-free build = same addressing but a single 'lh v1'. Target's extra sll/sra pair is why target has 114 insns and our build 113. Every OTHER Judge read in the function is a plain 'lh' in both target and build — only this one read has the split shape.

- [s1] The volatile cheat produced the correct split shape but forced address materialization into a real register ('la $v0,Judge; addu $a3,$a3,$v0; lhu $v0,0($a3)' = 4 insns, 115 total) — which is exactly what regfix rules @60-62 exist to rewrite back to the $at form. Dropping the volatile fixes the addressing for free.

- [s1] Mechanism B (13 diffs, mirror-image register swap): angC target $v1 / build $v0; its '& 0xFFF'+'<<1' index temp target $v0 / build $v1; sinAxsinB_12 target $v0 / build $a0. angA->$a3 and angB->$a2 agree in both.

- [s1] RTL: angC's load is greg insn 44 '(set (reg/v:SI 2 v0) (zero_extend:SI (mem/s:HI (plus (reg a0) (const_int 4)))))' — assigned by LOCAL-alloc, not global-alloc; sinAxsinB_12's pseudo (lreg: 'used 3 times across 5 insns') gets hard reg 4 ($a0). The whole function is a single basic block (no branches), so there are no cross-block levers and combine sees every def/use pair adjacently.

- [s1] Family map: the 6-Judge-read 3x3 rotation-matrix family has exactly 3 members, all in src/text1a_c.c, ALL still rule-carrying — _SelectSection (10 rules), replay_camera_rob_back_loose3 (8), hirahira_w_ctrl_2 (63). There is no matched template in this family; the existing regfix comments on the siblings name the SAME two mechanisms, so a closing form likely unlocks all three.

- [s1] 35 functions' target asm contains the 'lhu' + (<=3 insn gap) + 'sll 16 / sra 16' signature and 29 of those are matched/zero-rule. The instructive precedent is func_80065344 (src/text1b.c:14641): its raw u16 is ALSO stored back to memory, i.e. it has a second use of the zero-extended value — which is precisely what blocks combine's fold there.

- [s1] Incidental: this GCC accepts conflicting-type extern redeclarations with only a warning (src/text1a_c.c already contains such pairs, e.g. D_800A9D04 at lines 1152/1191), so a 'retype only this function's Judge decl' trick would compile — but it is a re-typed alias view of one object, i.e. the forbidden alias-rename family. Recorded so a future session does not mistake compilability for legitimacy.

- [s1] Tooling caveat for future sessions: 'diagnose --detail' reads a STALE object — it reported the same d17 diff for a src/ state whose sandbox score was 26. Use 'sandbox' for scores and objdump/diffasm.py on the fresh sandbox .o for shapes.

- [s1] src/text1a_c.c was restored to HEAD at end of session (git checkout); no source changes were left in the tree.

## Session 2 (structural, 2026-07-30) — FLOOR 26 -> 13, mechanism A CLOSED in pure C

### The closing form (banked as candidate.c, sandbox --disable all = 13, build_insns 114 == target 114)
Two changes on top of the s1 cheat-free baseline:
1. `u16 rawA = Judge[((s16)angA + 0x400) & 0xFFF];` staging local + `cosA = (s16)rawA;`
   (the sanctioned narrow-view spelling s1 had already measured at a flat 26 ON ITS OWN).
2. `a1[5] = -sinA;` MOVED from the tail of the function to sit BETWEEN the `rawA`
   load and the `(s16)rawA` cast.
Change 2 is the load-bearing one. Mechanism: GCC 2.7.2 combine's `can_combine_p`
refuses to combine a MEM load into a later user across an insn that may WRITE
memory, so the intervening store prevents `simplify_shift_const` from ever seeing
the `ashiftrt(ashift(zero_extend(mem)))` chain — target's three-instruction shape
(`lhu`; `sll 16`; `sra 16`) survives verbatim, including the `$at`-based
`lui/addu/lhu %lo` addressing. It costs ZERO extra instructions because sched1
runs AFTER combine and hoists the store back out; target's own schedule fills that
load-delay slot with `mflo t9`, and our build now does the same.

### The fold is NOT broken by giving the u16 a second use (three instruments, all KILLED)
s1's frontier hypothesis A ("the raw halfword needs a second, flow-LIVE use") is
WRONG, measured three ways on top of the u16 staging spelling:
- `a1[9] = rawA;`        (low-half-only store)  -> still `lh`, score 35 / 114 insns.
- `a1[9] = rawA + 1;`    (SI add, truncated)    -> still `lh`, score 38 / 115 insns.
- `a1[9] = rawA >> 8;`   (needs the zero-extended HIGH bits) -> still `lh`, score 42 /
  116 insns; GCC kept the `lh` and re-derived the unsigned value with `andi`/`srl`.
So combine rewrites the LOAD itself and patches the other users from the
sign-extended register; no width-of-use argument can stop it. Only an insn that
blocks combination outright (a memory write between load and cast, or `volatile`'s
un-combinable MEM) preserves the split shape — and the store is pure C.

### Position, not presence, of the store is the mechanism
- store between the load and the cast (`e5`)            -> 13 / 114.
- store AFTER the cast (`e6`)                           -> 26 / 113 (inert).
- a DIFFERENT store in the same slot, `a1[0] = ...` (`f4`) -> 13 / 114. Any store works.
- the same interleaving applied to the s16 baseline (no `u16` staging local, `e1`/`e2`/`e3`)
  -> 26 / 113, because that load is already a `sign_extend` MEM: there is no
  `zero_extend` for combine to fold, so there is nothing for the store to protect.
  Both halves of the form are required.

### Mechanism B is untouched and is now the WHOLE remaining gap
The 13 residual diffs are register naming ONLY — there is no instruction-shape
difference left anywhere in the function (`diffasm` on the score-13 object shows the
`tgt[62:67]` cosA region matching exactly). They are the same mirror-image swap s1
recorded: angC `$v1`/ours `$v0`, angC's index temp `$v0`/ours `$v1`, sinAxsinB_12
`$v0`/ours `$a0`.

### Structural levers measured DEAD for mechanism B (15 forms, before AND after the A fix)
On the s1 baseline, all of these scored EXACTLY 26 / 113 — bit-identical, i.e. the
RTL is unchanged, GCC canonicalises the spelling away: named local for angC's cos
index; named local for its sin index; both; both computed immediately after the
angC load; `angC = a0[2]` hoisted next to angA/angB; the whole angC/sinC pair moved
ahead of sinA/sinB; named locals for ALL SIX Judge indices; angC reused as its own
index holder; a two-variable `sinAxsinB` spelling; multiply-operand-order swaps on
`prod_*` and on all four `cosA_*` products; a walking `s16 *J = Judge` pointer;
`cosA` narrowed to `s16`. Two forms were WORSE: hoisting the cos index to
immediately after the angC load (43 / 112) and `sinAxsinB_12 = sinA*sinB;
sinAxsinB_12 >>= 12;` (36 / 113). Re-run on the score-13 baseline, the survivors
(named cos index, angC reuse, two-variable sinAxsinB, angC hoisted, a different
intervening store) are ALL still exactly 13 / 114; only the cosA operand-order swap
moved, and it moved the wrong way (17).

### RTL / local-alloc numbers for B (artifact: s2/rtl/base/f.i.{lreg,greg})
From the `.lreg` flow dump of the cheat-free baseline, the qty_compare_1 priority
inputs (`floor_log2(n_refs) * n_refs * size / live_length`):
- angC  = pseudo 97, "used 3 times across 12 insns" -> priority 0.25 (allocated LATE,
  it takes whatever register is left over).
- its `& 0xFFF` index temp = pseudo 111, "used 2 times across 2 insns" -> 1.00, and
  pseudo 114 (the `<< 1`), "used 2 times across 4 insns" -> 0.50.
- sinAxsinB_12 = pseudo 81, "used 3 times across 17 insns" -> 0.176 (also late).
`greg`'s "20 regs to allocate" list contains none of 97/111/114/81, confirming all
four are assigned by LOCAL-alloc. Since every structural spelling above leaves these
ref-counts and live-lengths untouched, none of them can move the allocation — which
is exactly what the 13 identical scores show. Closing B needs either a change to
these two numbers for pseudo 97/111 or an understanding of local-alloc's tie-break
among the many priority-1.00 quantities (a forensics/instrumented-cc1 job, not a
structural one).

### Tooling (session 2, reusable)
- `tmp/grind/.../s2/gen.py` — declarative variant generator (edit-list per variant,
  stacked on the baseline or on the e5 form); `variants/*.c` are the emitted forms.
- `…/s2/probe.ps1 <variant …>` — apply (via s1/apply.py) + sandbox, one row each.
  NOTE the same s1 quirk: the FIRST row is stale, always pass a throwaway first.
  ALSO: `diffasm.py` reads the sandbox object of the LAST probe run — re-apply the
  variant you want to diff before diffing it.
- `…/s2/rtl.sh <tag>` — cc1 `-da` dump set for whatever is currently in src/.
- `…/s2/qty.py <tag>` — pulls "used N times across M insns" + greg dispositions.
- `…/s2/head.py <obj> <n>` — first n instructions of the function out of a .o.
- src/text1a_c.c was restored to HEAD at end of session; the score-13 form lives in
  memory/grind/replay_camera_rob_back_loose3/candidate.c.

- [s2] [s2] NEW CHEAT-FREE FLOOR = 13 (was 26). The form is banked verbatim in memory/grind/replay_camera_rob_back_loose3/candidate.c: a u16 staging local for the cosA read plus the a1[5] = -sinA store MOVED between that load and the (s16) cast. Zero rules, zero pins, zero volatile, zero inline asm.

- [s2] [s2] build_insns is now 114 == target_insns 114. There is no instruction-shape difference anywhere in the function; all 13 residual diffs are register names.

- [s2] [s2] The store's POSITION is the mechanism, not its presence: between load and cast -> 13; after the cast -> 26; a different store (a1[0]) in the same slot -> 13.

- [s2] [s2] BOTH halves of the form are required: the same store interleaving applied to the plain s16 baseline (no u16 staging local) is inert at 26/113, because that load is already a sign_extend MEM with no zero_extend for combine to fold.

- [s2] [s2] The move is semantics-preserving: a1 (the output matrix) never aliases the static sin/cos table Judge, and the relative order of the a1[] stores among themselves is unchanged.

- [s2] [s2] s1's frontier-A hypothesis is dead: a real flow-live second use of the u16 does NOT block the fold, including one that needs the zero-extended high bits (a1[9] = rawA >> 8 kept the lh and re-derived the unsigned value with andi/srl).

- [s2] [s2] s1's frontier-C prediction is strengthened: the closing idiom is a source-spelling property (a store interleaved into the middle of the rotation-matrix computation), exactly the kind of thing that should port to the two siblings _SelectSection and hirahira_w_ctrl_2.

- [s2] [s2] Residual mechanism-B diffs (13): angC target $v1 / build $v0; angC's & 0xFFF + << 1 index temp target $v0 / build $v1; sinAxsinB_12 target $v0 / build $a0.

- [s2] [s2] local-alloc priority inputs from the .lreg flow dump: angC = pseudo 97 'used 3 times across 12 insns' (floor_log2(3)*3/12 = 0.25); index temp = pseudo 111 'used 2 times across 2 insns' (1.00) and pseudo 114 '2 times across 4 insns' (0.50); sinAxsinB_12 = pseudo 81 '3 times across 17 insns' (0.176). greg's '20 regs to allocate' list contains none of them.

- [s2] [s2] Tooling caveat: diffasm.py reads the sandbox object left by the LAST probe run — re-apply the variant you want to diff before diffing it (a stale read cost one turn this session).

- [s2] [s2] src/text1a_c.c was restored to HEAD at end of session; the tree carries only memory/grind ledger files.

## Session 3 (structural, 2026-07-30) — FLOOR 13 -> 12, mechanism B HALF-CLOSED

### The one change that moved the floor (banked in candidate.c, sandbox --disable all = 12)
On top of s2's score-13 form, `angC = a0[2]; sinC = Judge[angC & 0xFFF];` is moved
from BEFORE the `sinAxsinB_12 = (sinA * sinB) >> 12;` statement to AFTER it.
build_insns stays 114 == target 114.

Mechanism: `a0[2]` is the LAST use of the parameter pointer `a0`, so where that
read sits decides where the hard register $a0 dies AND puts the angC load after
the `mult` in the pre-allocation stream. Local-alloc then gives angC target's
**$v1** and its `& 0xFFF`/`<< 1` cos-index temp target's **$v0** — the
mirror-image swap s1 recorded and s2 declared unreachable. The build now
reproduces target's `lhu v1,0x4(a0)` at insn 18 verbatim, and the angC/temp half
of mechanism B is closed (score 13 -> 12; the drop is only 1 because the fix also
perturbs the schedule around the cosB/cosC index chains — see below).

s2 had only ever moved this read EARLIER (hoisted next to angA/angB: 26 flat;
hoisted with its index chain: 43/112). **Moving it LATER was the untried
direction.** Controls: after `cosB` too (m2) -> also 12; after the whole cosA
block (m3) -> 90; the same delay applied to `a0[1]` instead of `a0[2]` (m4) ->
13 (inert). So it is specifically a0[2]-as-last-use that matters.

### The residual 12 diffs are ONE value, and its cause is measured
`sinAxsinB_12`: target $v0, our build $a0. It is a scheduling-COUPLED allocation:
- target: `mult t2,t3` @17 ... `mflo t0` @22 ... `sra v0,t0,12` @26. mflo/sra are
  delayed past the cosC index chain, which holds $v0 at insns 20-24 and DIES at
  24, so $v0 is free for sinAxsinB_12 from 26 onward.
- ours: `mflo v0` @19, `sra a0,v0,12` @20 — BEFORE the index chain, so
  sinAxsinB_12's live range overlaps the temp's; $v0 is unavailable and
  local-alloc hands it the leftover $a0 (free because the pointer a0 is dead).
- Splitting the multiply from the shift (`sinAxsinB = sinA * sinB;` ... then
  `sinAxsinB_12 = sinAxsinB >> 12;` after the sinC read) reproduces target's
  schedule EXACTLY (`mflo t0` @22, `sra` @26, insn-for-insn in that window) —
  but flips angC back to $v0 and idxC to $v1, scoring 13. **The candidate has
  target's REGISTERS with the wrong schedule; the split form has target's
  SCHEDULE with the wrong registers. No spelling measured in s2+s3 has both.**
  Banked as rejected/split-sasb-target-schedule-loses-angC-reg.c.
- A second, smaller residual: target computes BOTH cos index chains before
  EITHER cos load; our build loads cosB in between. Named `idxB`/`idxC` locals
  fix that ordering but the score stays 12.

### Structural axes measured DEAD this session (23 forms, on the 13/12 baselines)
- **Eager a1[] stores** (s2's frontier-3 probe): every element stored as soon as
  it is computable -> 73/109 (combine/CSE eats 5 insns). Eager sub-groups: the
  four cosA_* singles -> 13 (inert); the two sinAxsinB sums -> 79/110; the
  sinAxcosB pair -> 53/112. Only the ONE combine-blocking store belongs out of
  the tail. `a1[0]` instead of `a1[5]` in the blocking slot -> 12 (still fine).
- **Tail store ORDER**: index order and sums-first -> 13 (inert); reversed ->
  65/107; a1[8] first -> 42/112.
- **Whole declaration-block permutations** (reversed / one group / first-use
  order) -> 13, bit-identical. Declaration order renumbers every pseudo (the
  .lreg dumps show pseudo 188 vs 192 for the same quantity) and STILL produces
  the identical assignment, which kills "qty creation order breaks the
  priority-1.00 tie" as a source-reachable lever.
- **Type narrowing / sign-extend splitting**: angC as u16, as s16, all three
  ang* as u16, `(s16)angC` split into its own s32 local (for A, for C, for all
  three) -> all 13/114, CSE canonicalises the width away. Only hoisting that
  sign-extend local ahead of the sinC read moved it, to 30/113.
- **Fewer locals**: folding single-use intermediates into their consumers ->
  87/116; folding just the four cosA_* products -> 71/111.
- **Shortening sinAxsinB_12's live range** (making prod_sinC and prod_cosC
  adjacent, to raise its qty priority from 0.176) -> 25/115: it costs an insn.
  Writing the shift inline at both uses without making them adjacent -> 13.
- **No angC local at all** (two `a0[2]` reads) -> 13, CSE merges them.

### Toolchain fact confirmed from source (tools/gcc-2.7.2/toplev.c)
Pass order is combine (l.3004) -> sched1 (l.3028, `-O2` enables
`flag_schedule_insns`) -> **local_alloc (l.3052)** -> global_alloc (l.3077) ->
sched2 (l.3105). So local-alloc sees a POST-scheduled stream: source statement
order only reaches allocation through whatever sched1 does not normalise away.
That is the mechanism behind the ~30 bit-identical "inert" forms of s2+s3, and
it is also why the one lever that DID work (moving the last use of a parameter
register) worked — it changes a liveness fact sched1 cannot undo.

### Tooling (session 3, reusable)
- `tmp/grind/.../s3/gen.py` — statement-key model of the function (each statement
  is a key; a variant is an ordered key list), emits `variants/*.c`.
  `gen2..gen7.py` are the per-batch generators built on it.
- `.../s3/probe.ps1 <variant ...>` — apply (via s1/apply.py) + sandbox, one row
  each. Same quirk as s1/s2: the FIRST row is stale, always pass a throwaway
  first. Run it from the repo root (`Set-Location` first — the PowerShell tool's
  cwd is not guaranteed).
- `.../s3/sbs.py <lo> <hi> [obj]` — SIDE-BY-SIDE target-vs-build listing by insn
  index, no normalisation and no diffing. This is the tool that made the residual
  legible; prefer it over diffasm.py, whose nop/immediate noise hides the real
  register story.
- `.../s3/rtl.sh <tag>` — cc1 `-da` dump set into `s3/rtl/<tag>` (run through
  `bash tools/wsl.sh '...'`; the Bash tool here is Git Bash, not WSL).
- `.../s3/cmp.py` / `norm.py` — dump-vs-dump comparison, raw and with insn/pseudo
  numbers erased. Verdict on these two: NOT worth much — the dep-list numbering
  noise makes "DIFFERENT" the answer for every pass even when the final assembly
  is byte-identical. Read scores + sbs.py instead.

- [s3] NEW CHEAT-FREE FLOOR = 12 (was 13). The single change vs s2's candidate: 'angC = a0[2]; sinC = Judge[angC & 0xFFF];' moved from before to AFTER the 'sinAxsinB_12 = (sinA * sinB) >> 12;' statement. build_insns stays 114 == target 114. Zero rules, zero pins, zero volatile, zero inline asm.

- [s3] Mechanism: a0[2] is the LAST use of the parameter pointer a0, so its position decides where hard reg $a0 dies and puts the angC load after the mult in the pre-alloc stream. Local-alloc then gives angC target's $v1 and its cos-index temp target's $v0 — the mirror-image swap s1 found and s2 declared unreachable. The build reproduces target's 'lhu v1,0x4(a0)' at insn 18 verbatim.

- [s3] Direction matters: s2 only ever moved that read EARLIER (26 flat, or 43/112 with its index chain). Later is the winning direction. Controls: delayed past cosB too -> also 12; delayed past the whole cosA block -> 90; the same delay applied to a0[1] instead of a0[2] -> 13 (inert).

- [s3] The residual 12 diffs are ONE value: sinAxsinB_12, target $v0 vs our $a0. Target delays mflo/sra past the cosC index chain (mult @17, mflo t0 @22, sra v0,t0,12 @26) so $v0 is free after the temp dies at 24; ours emits mflo v0 @19 / sra a0,v0 @20 before the chain, so the ranges overlap and local-alloc takes the leftover $a0.

- [s3] NEAR-MISS worth inheriting: splitting the multiply from the shift (sinAxsinB = sinA*sinB; ... sinAxsinB_12 = sinAxsinB >> 12 after the sinC read) reproduces target's SCHEDULE exactly (mflo t0 @22, sra @26) but flips angC back to $v0 / idxC to $v1, scoring 13. The candidate has target's REGISTERS with the wrong schedule; the split form has the schedule with the wrong registers. No spelling in s2+s3 has both.

- [s3] Secondary shape residual: target computes BOTH cos index chains before EITHER cos load; our build loads cosB in between. Named idxB/idxC locals fix that ordering but the score stays 12.

- [s3] Dead structural axes (23 forms): eager a1[] stores (73/109; cosA-singles subgroup inert at 13; sums 79/110; scb pair 53/112); tail store order (index/sums-first inert 13, reversed 65/107, a1[8]-first 42/112); whole decl-block permutations (reversed/one-group/first-use-order all bit-identical 13, despite renumbering every pseudo — .lreg shows 188 vs 192 for the same quantity); type narrowing of angC (u16/s16/all-ang-u16/sign-extend-split — all 13; sign-extend local hoisted ahead of sinC 30/113); folding single-use locals into consumers (87/116, or 71/111 for the cosA_* four); making sinAxsinB_12's two consumers adjacent to shorten its live range (25/115 — costs an insn); no angC local at all, two a0[2] reads (13, CSE merges).

- [s3] Toolchain fact from tools/gcc-2.7.2/toplev.c: pass order is combine (3004) -> sched1 (3028, enabled by -O2) -> local_alloc (3052) -> global_alloc (3077) -> sched2 (3105). local-alloc sees a POST-scheduled stream, which is why ~30 statement-order spellings across s2+s3 are bit-identical, and why the one lever that worked (moving the last use of a parameter register) worked: it changes a liveness fact sched1 cannot normalise away.

- [s3] src/text1a_c.c was restored to HEAD at end of session; the tree carries only memory/grind ledger files.

- [s3] NEW CHEAT-FREE FLOOR = 12 (s2 banked 13, s1's honest baseline was 26). Zero regfix/asmfix rules, zero register pins, zero volatile coercion, zero inline asm. build_insns 114 == target_insns 114. Verified by applying memory/grind/replay_camera_rob_back_loose3/candidate.c itself to src/text1a_c.c and running sandbox --disable all.

- [s3] The single change vs s2's candidate is moving 'angC = a0[2]; sinC = Judge[angC & 0xFFF];' from before to AFTER the 'sinAxsinB_12 = (sinA * sinB) >> 12;' statement. a0[2] is the LAST use of the parameter pointer a0, so its position decides where hard reg $a0 dies and puts the angC load after the mult in the pre-allocation stream.

- [s3] That change gives angC target's $v1 and its '& 0xFFF'/'<< 1' cos-index temp target's $v0 -- the mirror-image swap s1 documented and s2 declared unreachable by structural means. The build now reproduces target's 'lhu v1,0x4(a0)' at insn 18 verbatim.

- [s3] DIRECTION was the whole trick: s2 had only ever moved that read EARLIER (hoisted next to angA/angB -> 26 flat; hoisted together with its index chain -> 43/112). Later is the winning direction. Controls: delayed past cosB as well -> also 12; delayed past the whole cosA block -> 90; the identical delay applied to a0[1] instead of a0[2] -> 13 (inert). So it is specifically a0[2]-as-last-use.

- [s3] The residual 12 diffs are ONE value: sinAxsinB_12, target $v0 vs our $a0. Target: mult t2,t3 @17, mflo t0 @22, sra v0,t0,12 @26 -- mflo/sra delayed past the cosC index chain, which holds $v0 at insns 20-24 and dies at 24, leaving $v0 free from 26. Ours: mflo v0 @19, sra a0,v0,12 @20 -- before the chain, so the live ranges overlap, $v0 is unavailable, and local-alloc hands sinAxsinB_12 the leftover $a0 (free because the pointer a0 is dead by then).

- [s3] NEAR-MISS for the next session: splitting the multiply from the shift reproduces target's SCHEDULE exactly (mflo t0 @22, sra @26, insn-for-insn in that window) but flips angC back to $v0 / the temp to $v1, scoring 13. The candidate has target's REGISTERS with the wrong schedule; the split form has target's SCHEDULE with the wrong registers. No spelling measured across s2+s3 has both -- and that separability was invisible from scores alone, it took the side-by-side listing to see.

- [s3] Secondary shape residual: target computes BOTH cos index chains before EITHER cos load, our build loads cosB in between. Named idxB/idxC locals fix that ordering; the score stays 12 either way.

- [s3] TOOLCHAIN FACT read from tools/gcc-2.7.2/toplev.c: pass order is combine (l.3004) -> sched1 (l.3028, enabled by -O2 via flag_schedule_insns) -> local_alloc (l.3052) -> global_alloc (l.3077) -> sched2 (l.3105). local-alloc therefore sees a POST-scheduled stream. This is the mechanism behind the ~30 bit-identical 'inert' statement-order forms of s2+s3, and behind why the one lever that worked worked: moving a parameter's last use changes a liveness fact sched1 cannot normalise away. Corollary for future sessions: on this function, prefer levers that change LIVENESS of hard-register-tied values (parameter last uses, call-clobber boundaries) over levers that merely permute arithmetic order.

- [s3] Declaration order is dead at the whole-block level, not just for angC: reversed / one-group / first-use-order all score 13 bit-identically, and the .lreg dumps show the pseudo NUMBERS did change (188 vs 192 for the same quantity) with the assignment unchanged. That kills 'qty creation order breaks the priority-1.00 tie' as a source-reachable lever.

- [s3] 23 structural forms measured this session, 18 of them at build_insns 114. Empirical regularity worth inheriting: on the s2 baseline every 114-insn form scored EXACTLY 13 and every form that moved the allocation also changed the insn count (and always for the worse) -- until the delayed-a0[2] form broke that invariant at 114/12. So 'score is pinned at N for all N-insn forms' is NOT evidence of exhaustion here; it was evidence that the axis being varied did not touch liveness.

- [s3] src/text1a_c.c was restored to HEAD at end of session (git checkout). The working tree carries only memory/grind ledger files plus the untouched metrics/events.jsonl churn.

## Session 4 (permuter, 2026-07-30) - FLOOR 12 -> 0.  MATCH, CHEAT-FREE.

### The closing change (banked in candidate.c; `sandbox --disable all` = 0, 114 == 114, rules_dropped 8)
ONE line on top of s3's score-12 form: the cosB index ADD is hoisted into a named
`s32 idxB` computed immediately after `sinB` (i.e. BEFORE the `sinA * sinB`
multiply), while the `& 0xFFF` mask stays at the use site:
    idxB = (s16)angB + 0x400;      ...      cosB = Judge[idxB & 0xFFF];

Mechanism. The whole s3 residual was `sinAxsinB_12` (target $v0, ours the leftover
$a0) because our mflo/sra were scheduled ahead of the cosC index chain and so
overlapped it on $v0. Materialising the cosB index early puts an independent,
ready quantity into that scheduling window; sched1 places it ahead of the
multiply's result, mflo/sra slide into target's positions (mflo @22, sra @26),
the cos-index temp dies before sinAxsinB_12 is defined, and local-alloc hands
sinAxsinB_12 $v0. Instruction count is unchanged (the addiu exists in target too,
it just moves).

### Why this is not a previously-rejected idxB form
rejected/s32-masked-idxB-temp-keeps-sext-loses-registers.c put the WHOLE index
(`((s16)angB + 0x400) & 0xFFF`) into the local AND placed it LATE, after the sinC
read -> 13. The closer differs in both respects: ADD only (mask stays at the use)
and EARLY (before the multiply). Narrow (u16/s16) carriers remain dead - they fold
the sign-extend away and lose four instructions.

### Isolation of the two halves the permuter emitted together
The permuter's find also interleaved the angA/angB reads with their sin lookups.
Measured separately on the real build:
  - idxB hoist + interleave   -> 0 / 114
  - idxB hoist, NO interleave -> 0 / 114   (the banked candidate - minimal diff vs s3)
  - interleave, NO idxB hoist -> 12 / 114  (inert)
So the idxB hoist alone is the closer; the interleave is noise.

### Campaign record
Chassis: minimal-TU workspace (typedefs + `extern s16 Judge[]` + the function),
full BB2 stage pipeline in compile.sh (cc1 -> prologue_fix -> maspsx -> multu_pad),
function extracted at offset 0 and assembled against a prelude-prefixed target.o
built from asm/funcs/replay_camera_rob_back_loose3.s. Validated before launch: the
workspace base reproduced exactly the known 12-diff residual.
base.c carried `PERM_LINESWAP(...)` over the 9-statement middle region.
**The permuter did NOT see the macro** - it logged "No perm macros found.
Defaulting to randomization" and ran its ordinary randomizer instead. The win came
from the randomizer's `new_var` hoist transform, not from the directed lineswap.
Campaign `lineswap-mid9`: base_score 1520 (permuter metric; harvest later reported
100), 3438 iterations / 140 s, 398 output dirs, several at permuter score 0; the
first score-0 landed inside the first minute. Harvested and STOPPED in-session
(9 worker processes killed).

### Tooling caveats for future sessions
- `permuter_campaign.py harvest` reported `finds_new: 0` and NEGATIVE
  `seconds_since_launch` for every find in a workspace created fresh THIS session.
  The launch-vs-mtime comparison is skewed (Windows-side file mtimes vs the WSL
  launch timestamp), so the new/old classification is unreliable here - read the
  output-*/ directories directly, not the `finds_new` count.
- `--stop-on-zero` did not stop the campaign on the score-0 finds; stop it yourself.
- A minimal-TU permuter chassis is faithful for this function: the workspace's
  base diff matched the full-file sandbox residual exactly, and the workspace's
  score-0 find reproduced as `sandbox --disable all` = 0 on the real build.

- [s4] MATCH. `sandbox --disable all` = 0 on replay_camera_rob_back_loose3 with the s4 candidate applied to src/text1a_c.c; build_insns 114 == target_insns 114; rules_dropped 8. The form is pure C: zero regfix/asmfix rules, zero register pins, zero volatile, zero inline asm, zero dead code. The 8 regfix rules at regfix.txt:1000-1010 are now provably unnecessary and their retirement is the operator's step.

- [s4] The closing change vs s3's score-12 candidate is ONE hoisted line: `idxB = (s16)angB + 0x400;` placed immediately after the sinB lookup (before the sinA*sinB multiply), with `cosB = Judge[idxB & 0xFFF];` at the original site. Only the ADD moves into the local; the mask stays at the use.

- [s4] Mechanism: the s3 residual was entirely sinAxsinB_12 ($v0 vs our leftover $a0), caused by mflo/sra being scheduled ahead of the cosC index chain. An early, independent, ready quantity in that window lets sched1 delay mflo/sra past the chain into target's slots (mflo @22, sra @26); the chain's temp then dies before sinAxsinB_12 is defined and local-alloc gives it $v0. Insn count unchanged at 114.

- [s4] Distinguished from the banked rejects: the earlier idxB attempts folded the `& 0xFFF` mask into the local AND placed it late (after the sinC read) -> 13; narrow u16/s16 carriers fold the sign-extend away and lose instructions. Position AND which half of the expression is named are both load-bearing.

- [s4] The permuter emitted the closer together with an angA/angB read-interleave; measured separately, idxB-hoist-only = 0 and interleave-only = 12, so the banked candidate keeps s3's read order and adds only the hoist.

- [s4] Campaign telemetry: label lineswap-mid9, minimal-TU chassis, 3438 iterations in 140 s, 398 output dirs, first permuter-score-0 inside the first minute; harvested and stopped in-session. NOTE the base.c PERM_LINESWAP macro was NOT recognised ("No perm macros found. Defaulting to randomization") - the find came from the plain randomizer's named-intermediate hoist. Directed-macro syntax in a hand-built workspace needs verifying against the launch log before trusting a campaign to be directed.

- [s4] Tooling: `harvest` misclassified every find as pre-existing (finds_new 0, negative seconds_since_launch) in a workspace created this session - Windows mtime vs WSL launch-timestamp skew. Read output-*/ directly. `--stop-on-zero` did not stop the run.

- [s4] The closing idiom (a named local for a table index, hoisted ahead of the multiply) is a property of the shared 3x3 rotation-matrix source idiom and should be tried on the two rule-carrying siblings in the same file: _SelectSection (10 rules) and hirahira_w_ctrl_2 (63 rules).

- [s4] src/text1a_c.c is left carrying the matching form (NOT reverted, unlike s1-s3) so the driver can verify bytes.
