# marionation_Exec (system.c) — WIP, lift model VALIDATED, execution in progress

## TL;DR (2026-07-02)
The 42-rule wall's register-rotation core is a COMPUTED AND NOW VALIDATED
plan: probe t1 (tmp/probe_mar.py — chain-extender on the `D_800F19C0 =
&D_80016248;` store through tbl_125c, sanctioned per
[[dead-store-fake-exception]] §chain-extender) moved tbl s6→s5 (its
target reg) at IDENTICAL insn count. First of six slots confirmed
byte-free. The tie-break arithmetic is verified against global.c:604-625
(double-math truncation → exact tie at equal refs/len; fallback `*v1-*v2`
= ascending allocno ✓). reg_n_refs increments are `+= loop_depth`
(flow.c:2067 — goto-loops do NOT increase depth; only real do/while
loops do; the poll loop is a GOTO loop → +1 per site).
ASM-SUSPECT verdict is the distance>50 heuristic only (0 asm insns).

## Target ground truth (read from asm/funcs/marionation_Exec.s, 2026-07-02)
- Register map CONFIRMED from bytes: status→s0 (call result), saved→s1
  (`andi $s1,$v0,3`), idx_1494→s2, idx_1496→s3, arg1→s4, tbl→s5,
  idx_1495→s6, arg0→s7.
- Poll shape: current C is CORRECT (call-at-loop-top; the delay-slot
  `andi v0,s0,4` / duplicated `andi v0,s0,2` at .L1D8+delay are reorg
  steal-from-target artifacts of the plain two-if shape, not source).
- The masked-56 diffs live in the TAIL (check block + two copy loops):
  a SECOND local-alloc cascade — target: check→a2 (the reason for the
  body's `register s32 check asm("$6")` pin), src→a0, i→v1, b→v0;
  ours-natural: src→a2, i→a0, b→v1. Same class as
  cpu_side_move_dir_4's block (see its WIP — density/coloring analysis).
- check2 micro-shape: target `lbu v0,-1(s3); andi a2,v0,0xff` — an
  emitted byte-mask AFTER lbu that also serves as the v0→a2 move; plain
  `check = *(idx_1496-1);` folds it. Candidate spellings to probe: u8
  intermediate + s32 check, or `& 0xFF` on a cross-pseudo copy (watch
  the F2-not-sanctioned boundary — find the spelling GCC emits naturally).

## Baseline (verified via cc1 -da .greg/.lreg, 2026-07-01, HEAD c6d96b06)
pri = floor_log2(nrefs)*nrefs/livelen*1e4 — reproduces dispositions exactly:
| pseudo | ident | refs/len | pri | now | target |
|---|---|---|---|---|---|
| 81 | (short local) | 4/10 | 8000 | s0 | s0 ✓ |
| 73 | arg1 | 4/82 | 975 | s1 | s4 |
| 80 | saved | 2/21 | 952 | s2 | s1 |
| 76 | idx_1494 | 6/146 | 821 | s3 | s2 |
| 78 | idx_1496 | 5/146 | 684 | s4 | s3 |
| 72 | arg0 | 2/76 | 263 | s5 | s7 |
| 79 | tbl_125c | 3/148 | 203 | s5→s6 | s5 |
| 77 | idx_1495 | 2/144 | 139 | s7 | s6 |

## (superseded lift budget removed — see session-2 conclusion; the tie
## arithmetic + site accounting live in git history of this file)

## Execution state (2026-07-02, session 2 — MAJOR: t3 structural discovery)
**t3 (NO coercion, pure shape fix) lands 5/8 registers**: reshape BOTH
copy arms to target's byte order — `dst = a1; src = (u8*)&...; if
(dst != 0) { copy }` (arm 1 currently tests `a1` first — inconsistent
with arm 2; target bytes at 71A48/71A8C prove move-then-la-then-beqz in
BOTH). Result: saved=s1 ✓ i1494=s2 ✓ i1496=s3 ✓ arg1=s4 ✓ p81=s0 ✓
(insns 140; arg1 refs 4→3 = pri 365, matching the byte evidence).
**Remaining trio {arg0=s5→s7, tbl=s6→s5, i1495=s7→s6}: NOT a priority
problem** — with pris {arg0 263 > tbl 203 > i1495 139}, target's order
{tbl > i1495 > arg0} is UNREACHABLE by the formula (i1495 has the same
2 refs in target's bytes). ⇒ find_reg's CONFLICT/PREFERENCE machinery
decides this trio. NEXT LEVER (named, unrun): rebuild the instrumented
cc1 (hooks already in tools/gcc-2.7.2 sources — build into tmp/gccdbg,
canonical binary untouched) and read the find_reg walk / preferences
for these three at the t3 base.
**Refuted this session:** t5/t6 late-site chain-extenders MATERIALIZE
(+6 insns — combine only folds chains ADJACENT to the base's def; the
csmd4-style lever works at init sites only). t4 (=t3+t1 tbl-chain):
tbl overshoots to 540 > arg1's 365 → s4 ✗ — the t1 chain is
INCOMPATIBLE with t3; do not combine them.

**SESSION-2 CONCLUSION (instrumented ALLOCDBG on t3, pin-on and
pin-off identical):** the trio allocates in pure priority order (arg0
263 → tbl 202 → i1495 138 taking s5/s6/s7); no preference or conflict
effects exist to exploit. Target's order {tbl s5, i1495 s6, arg0 s7}
implies ORIGINAL livelens incompatible with the current tail structure
(i1495 < 76, arg0 > 144 — ours: 144/76, near-exactly swapped!).
⇒ **the tail region's true C shape (check block + copy loops — where
the 56 masked diffs live) reshapes late-function LIVENESS, and the trio
is its CONSEQUENCE.** Stop treating the rotation as a separate wall:
derive the tail byte-by-byte (target: check2 `lbu v0,-1(s3); andi
a2,v0,0xff` + `move v0,a2` returns; copy loops with src→a0, i→v1
counting DOWN via `addiu v1,v1,-1; bne v1,a3`; ours: src→a2, i→a0)
exactly like the t3/csmd4-block derivations, verifying the trio's
dispositions after each shape probe (tmp/probe_mar_allocdbg.py).
The arg0/i1495 livelen SWAP hints the original tail keeps arg0 (a1's
sibling) alive longer (e.g. a0 tested LATER/differently) while i1495
dies earlier — look for shapes where the copy loops' dst/src derivation
changes which pointers stay live across the tail.

## Superseded session-1 lift budget (kept one line for history)
The 07-01 arithmetic budget (saved+2/76+2/78+3/tbl+2/1495+2) is
SUPERSEDED by t3: the arm reshape alone lands saved/76/78/arg1
naturally (no lifts); only the {arg0,tbl,i1495} trio remains and it is
conflict/preference-driven, not priority-driven. t1 (tbl chain at the
D_800F19C0 init) remains valid IN ISOLATION but incompatible with t3.

## Known gotchas
- **`new_var` is UNDECLARED in this function** (src/system.c:555) — the
  build only passes because the Makefile PIPE swallows cc1's
  error-recovery exit. cc1 recovers as-if-declared; RTL/bytes are fine.
  DO NOT "fix" the decl mid-derivation (it may shift pseudo numbering);
  fix it in the final committed form and re-verify SHA1.
- The 42 rules are index-anchored: mid-derivation full builds are
  meaningless (rules misapply on shifted indices). Iterate at cc1-dump
  level (dispositions) + sandbox insn-count; the ONLY end gate is
  retire-all-42 + full SHA1.
- Masked sandbox is rotation-blind here (like motion_SetMotion).
- After the RA rotation lands, the residual = the 7 prologue + 7
  epilogue rename substs should vanish together (they encode the same
  rotation); the 9 debug_printf scheduling substs + reorder + label-dup
  tricks are a SECOND wall (cf. csmd4's block — the v15-style arg5-first
  restructure may apply; see memory/wip/cpu_side_move_dir_4/notes.md).

## Pointers
- [[duplicated-statement-into-arms]] — the lever + prerequisites.
- memory/wip/motion_SetMotion/notes.md — the proven recipe + merge-
  direction steering.
- memory/project/register-alloc-deep-dive.md § marionation_Exec — the
  prior sessions' rotation map (matches this baseline).
- tmp/rtl_dump.sh + tmp/probe_fn.py + tmp/probe_msm.py — the probe kit.
