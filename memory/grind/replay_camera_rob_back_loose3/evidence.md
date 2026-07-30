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
