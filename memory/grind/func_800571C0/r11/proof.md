# func_800571C0 — Ruling 11 submission for `temp` (was `nr`) (laneG, 2026-09-30)

Answers the retro-audit CONCERN on ecfdc52a6 (tmp/audit-2026-09-29/review/batch_01.md, owner Q49 fix-forward):
"`nr` holds the right-side count, then gets reused as the side flag (`nr = 0` / `nr = 1`, then `if (nr != 0)`).
A fresh `side` flag scores 47/295 ... It's undisclosed, and no family is claimed." Ruling:
`.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11, with (C)(3)'s per-branch-constants clause (owner Q20),
the Q30 set-aside and the Q31 mechanism + search standard.

What changed against the landed body (variants/landed_body.c): `nr` -> `temp` (Ruling 11 (E)(i)) with the (F)
declaration comment. No statement changed. sandbox 0 (287/287).

Files: `final.c` (== ../candidate.c); `one-var-per-value-form.c` (== variants/split_block.c); `variants/`;
`dumps.txt`; `tools/` (dump.sh / runall.sh / fr.sh dumps, r11table.py pseudo naming, collect.py -> dumps.txt,
mkperm.sh permuter workspace, permfinds.py).

## The variable `temp` and its values (both in $s6 in the target)
- V1 the count of clear probe steps on the right-hand side: `temp = 0;` (function entry), `temp++;` (each clear
  right step), and the tie-break `temp = 0;` (`rand() & 1` picks the left side); read by `nl != 0 || temp != 0`,
  `nl == temp`, `nl < temp`, `nl = temp`. Target: `addu $s6,$zero,$zero` 0x800571D0, `addiu $s6,$s6,1`
  0x80057478, `addu $s6,$zero,$zero` 0x800574CC, `sll $v0,$s6,24` 0x80057498 / 0x800574D8, `addu $s2,$s6,$zero`
  0x800574EC.
- V2 which side was chosen: `temp = 0;` (right: `nl < temp`, after `nl = temp`) or `temp = 1;` (left), read by
  `if (temp != 0)` in the waypoint loop (left: facing + angle). Target: `addu $s6,$zero,$zero` 0x800574F4 (delay
  slot), `addiu $s6,$zero,1` 0x800574F8, `sll $v0,$s6,24` 0x80057520.
V1's writes cannot reach V2's read (both arms of `if (nl < temp)` write V2 first), and V2's writes reach no V1
read. Two values in Ruling 11's sense.

## (A) Fresh local, not a borrow
A local, declared once at function scope (V1's first write is at function level), not a parameter/global/
static/register, never `&temp`. No other declaration moves against the one-var form.

## (B) Every write is live; no re-store
(1) V1's writes reach the `nl != 0 || temp != 0` test; V2's both reach `if (temp != 0)` (the loop runs at least
once: `ret = nl--` with nl >= 1 there). (2) V1 `temp = 0` at entry: first write. Tie-break `temp = 0`: runs only
when `nl == temp` and not both zero, so temp != 0 there on every path (not a re-store). V2 `temp = 0`: runs when
`nl < temp`, nl >= 0, so temp >= 1 there. V2 `temp = 1`: path with V1 = 0 (nl > 0, no clear right step) holds 0.
`temp++` always changes the value.

## (C) Same statements; real computations
(1) one-var-per-value-form.c: V1 keeps the name `nr` at function scope; V2 is `s8 toL;` declared at the top of
the `if (nl != 0 || nr != 0)` block, the innermost scope enclosing both of its writes and its read.
(2) `diff final.c one-var-per-value-form.c`: declarations and identifiers (and the (F) comment) only.
(3) V1: `temp++` (`addiu $s6,$s6,1`). V2: two different constants on different feasible paths, chosen by the
runtime test `nl < temp` — the per-branch-constants clause (owner Q20).

## (D)(1) Dumps — dumps.txt
`tools/dump.sh`: the body substituted into a copy of src/text1b.c (engine.inlineasm.substitute_body), cpp with
engine/buildconfig.py CPP_FLAGS + CPP_DEFS, the INSTRUMENTED `tools/gcc-2.7.2/cc1` with the build's CC_FLAGS
(text1b is a -G0 file, not in NO_SR_FILES) plus `-dr -ds -dt -df -dc -dl -dg`, BB2_ALLOC_DEBUG=1 and
BB2_FINDREG_DEBUG=75; the build cc1's .s on the same .i is identical (instcheck). The copied text1b.c is the tree
at 2026-09-30 with the staged func_8005763C cheat-cleanup in it (a later function; no effect on this one's
dumps beyond file context). final: temp = 75, nl = 74. one-var form: nr = 75, toL = 189.

## (D)(2) Mechanism — global.c allocno priority order (allocno_compare, global.c:635-655; allocation loop
global.c:575-598) with find_reg's call-saved restriction (global.c:970-975)
`temp` crosses calls (func_80053614 / rand), so only call-saved registers ($s0-$s7, $fp) serve it. The pseudos
competing for the last of them are three call-argument values of the probe loop (125/126/127, 9 refs over
118-121 insns: priority 2231 / 2250 / 2288). final: temp 13 refs / 161 insns -> 2422, ord 16, above all three ->
$s6; pseudo 125 is the one left without a register (spilled to sp+120, as in the target, `sw $9,120($sp)` /
`lw $6,120($sp)` in the build). One-var form: nr 10 refs / 149 insns -> 2013, ord 20, below all three: FINDREG 75
pass1_used = all 32 registers -> no register, nr is spilled (`sb $0,120($sp)` at entry), the call-argument
pseudos take $s6/$s7/$fp, and toL (3 refs / 12 insns, no call) takes $a0. 47 (295 insns).

## (D)(3) Necessity (Q31 mechanism + search)
(a) The mechanism above from banked dumps of both spellings. The property: the variable's priority exceeds 2288
(the highest competing call-argument pseudo). With its own variable, V1's references are fixed by its
statements ((C)(2)): 10 refs over a live range that ends at `nl = nr`, so floor_log2(10) * 10 / 149 * 10000 =
2013; V2's 3 references and 12-insn range are what lift the shared allocno to 2422. Declaration order or scope
changes only the pseudo number, which breaks priority ties only (allocno_compare's second key), and the type
(s8/u8/s32) leaves allocno_size at 1. (b) Every per-value spelling proposed is banked and measured (table).
(c) None reaches the target.

## (D)(4) Measured alternatives (sandbox --disable all; target 287)
| spelling (variants/) | score | insns |
|---|---|---|
| final.c | **0** | 287 |
| landed body (`nr`) | 0 | 287 |
| one-var-per-value, `s8 toL` in the if block (split_block = one-var-per-value-form.c) | 47 | 295 |
| same, `u8 toL` (split_block_u8) | 68 | 295 |
| same, `s32 toL` (split_block_s32) | 72 | 294 |
| function-scope flag: `s8` before `nl` / `u8` after `nr` / `s32` after `ang` (s_funcscope_*) | 47 / 68 / 72 | 295 / 295 / 294 |
| structural: `toL = nl >= nr; if (toL == 0) nl = nr;` (r_R1_cmpflag) | 49 | 294 |
| structural: `a = toL != 0 ? base + ang : base - ang;` (r_R3_ternary) | 47 | 295 |
| permuter from the one-var form | see § Permuter | |
(Only two values, so no single-value ablation beyond the full split.) No FAKE-construct spelling measured.

### Permuter
Workspace tools/mkperm.sh (reduced TU: the body's own declarations + the body; compile.sh = the build's cpp |
build cc1 (CC_FLAGS) | prologue_fix | maspsx (MASPSX_FLAGS) | multu_pad | as; the landed body built there differs
from target.o in 0 instructions). Campaign from one-var-per-value-form.c (tools/permuter_campaign.py, 2 jobs,
--stack-diffs, 2026-09-30 19:58-20:22): 19,957 iterations in 1,400 s, permuter score 3550 -> best 425 (found in
the first minutes; nothing better afterwards; stopped). Nothing reached the target (score 0). What the finds
change (permuter.txt, the 6 best): do-while(0) wraps around `nr++` / `right = probe; nr++;` (425, 435, 605 — a
FAKE family, set aside under Q30), an extra `nl = nr;` (525), a `nr++; nr--;` cancellation pair plus two
staging locals (584), and `nr = 0xB8;` staging a constant offset through the count variable (689) — the last
puts a second value into `nr`, the reuse this submission declares. None is a one-variable-per-value spelling
that matches.

## (E) Name
`temp`: Ruling 11 (E)(i). (A count and a side flag are not one kind of quantity; `nr` stated only V1's role.)

## (F) Annotation
final.c's declaration comment names both values, cites Ruling 11 / Q20 and this file.

## Disclosed, not claimed under Ruling 11: `nl`
`nl` counts the clear left steps, takes the chosen side's count (`nl = temp` when the right side wins, or 0 on a
tie-break), and is then counted down in place as the waypoint index: `ret = nl--;` and
`for (...; nl >= 0; nl--, ...)` (target $s2: `addiu $s2,$s2,1` 0x8005736C, `addu $s2,$s6,$zero` 0x800574EC,
`addiu $v1,$v0,-1` / `addu $s2,$v1,$zero` 0x80057500/04, `addiu $a1,$s2,-1` 0x800575B8). Its later writes are
decrements of its own value, the in-place update reading of Ruling 4, not a second unrelated value; the
statement `ret = nl--` has no same-statement one-variable spelling. For the record: a separate block-scoped
index (`ret = nl; for (k = nl - 1; k >= 0; k--)`, variants/nlsplit_kblock.c) scores 103 (293);
the same priority mechanism applies (nl alone 11 refs / 154 insns -> 2142, spilled).
