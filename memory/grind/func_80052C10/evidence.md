# Evidence bank — func_80052C10

## s2 (structural) — MIPS has no REG_ALLOC_ORDER; default order is ascending, puts $s0-$s7 BEFORE $t8/$t9

`grep -r REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/` → no hits. Per
`tools/gcc-2.7.2/global.c:1079` (`#ifndef REG_ALLOC_ORDER`), the allocator's
default preference is ascending hard-register number: $v0(2),$v1(3),
$a0-$a3(4-7),$t0-$t7(8-15),$s0-$s7(16-23),$t8(24),$t9(25). This means the s1
frontier's "~13 other live pseudos" estimate for reaching $t9 was wrong — it
did not count $s0-$s7 (8 registers) sitting ahead of $t8/$t9 in the default
order. The correct threshold, swept empirically this session, is 22.

## s2 — $t9 reachability swept empirically (gen2.py pointer-pressure chassis)

Diagnostic-only (never landed) chassis: `target` pointer declared first (live
range spans whole function), N other `vu32 *pK` pointers declared after,
all N dereferenced+stored BEFORE `*target = 0;` to force overlap. Swept via
`tools/grinder/dump.ps1 func_80052C10 -Candidate <file>`, reading `.greg`
register dispositions for pseudo 72 (= `target`):

| N (other pointers) | target's hard reg | reg name |
|---|---|---|
| 5  | 2  | $v0 |
| 13 | 16 | $s0 |
| 15 | 18 | $s2 |
| 20 | 23 | $s7 |
| 22 | 25 | $t9 |
| 23 | 30 | (past t9; a competitor claims t9 first) |

Confirmed at N=22 via `sandbox --disable all --candidate
tmp/grind/func_80052C10/s2/q22.c --diff`: score 106, 110 build insns (target
has 6), with an `addiu $sp,$sp,-32` prologue — the save overhead $s0-$s7 usage
imposes. Saved to tmp/grind/func_80052C10/s2/ (gen2.py, q13/q15/q20/q22/q23.c
+ .greg dumps). This confirms the s1 frontier's hypothesis 1 (mechanism is
real, $t9 is reachable) but also shows the practical corollary is false on
this chassis: no N in the swept range reaches $t9 without also forcing a
110-insn body with register-save overhead, categorically unlike the target's
6-insn, no-prologue shape. Narrows (does not close) the register-seat
anomaly: the mechanism exists, but this function's actual asm shows no trace
of the register pressure it would require.


## s1 (recon) — baseline + grant re-audit

- Target: `asm/funcs/func_80052C10.s` — 6 insns: `lui $t9,0x1f80; ori $t9,$t9,0x400;
  sw $zero,0($t9); jr $ra; nop; nop`. Writes zero to absolute address 0x1F800400
  (scratchpad RAM region, per AGENTS.md 0x1F800000-0x1F8003FF — this address is one
  word past the documented 1KB scratchpad window, so it's the tail of a larger
  scratchpad-resident struct many sibling functions poke via `lui $at + sw $vN,
  off($at)` — e.g. func_80017F28/5C/A0/80018094 all touch 0x1F8000xx the same way).
- `canonical func_80052C10` -> verdict ASM-WHOLE, reason: "whole-function
  canonical-asm grant in inline_asm_canonical.txt". Grant text
  (`inline_asm_canonical.txt:331`, user-authorized 2026-06-07): claims
  "combine.c always folds compile-time-known absolute offsets into the sw
  displacement (natural cc1 form is lui;sw 0x400(r))" making the 3-insn
  lui+ori+sw form "unreachable from any C", plus a non-natural register ($t9).
- Queue entry (`engine/queue.json`) carries a 2026-10-08 `reopen_reason`:
  "re-audit of whole-body INCLUDE_ASM canonical entries. Decide afresh whether
  the original was hand-written asm (scan_hand_coded STRONG evidence + Judge)
  or compiled C that must be decompiled; many game entries were auto-added
  2026-05-17 as handwritten_overflow_op." This function is exactly that
  re-audit case.

## OBJECT MODEL: func_80052C10

No flagged global/census signal applies — the target body touches only a raw
absolute scratchpad address (0x1F800400), not a declared extern/global symbol;
there is no header declaration to reconcile. N/A for the declaration-fix
hypothesis.

## s1 — the grant's premise is measurably WRONG

Tested `*(vu32 *)0x1F800400 = 0;` (natural C, using the existing `vu32` typedef,
`include/common.h:18`) as the whole function body, via
`sandbox func_80052C10 --disable all --diff`:

```
score 4; target 6 insns, build 5 insns
hunk 1/2 operand-only: target lui/ori/sw on $t9 vs ours on $v0
hunk 2/2 source-level: target has one more trailing `nop`
```

This DIRECTLY contradicts the grant's claim that combine.c "always" folds the
offset into the sw displacement — with a `volatile` dereference, our cc1
generates the SAME lui+ori+sw 3-insn shape as the target (no fold to a `sw
0x400($reg)` displacement form), matching insn-for-insn except for the
register. The grant's "unreachable from any C" premise does not hold for this
specific address/volatile combination. Tried the pointer-local form (`vu32 *p
= ...; *p = 0;`) too — identical codegen, same score 4.

## s1 — scan_hand_coded: no STRONG signal

`python3 tools/scan_hand_coded.py --single func_80052C10 --json`:
`"gate_signals": [], "score": 0, "tier": "LOW", "tier_reason": "no strong
hand-coded indicators"`. Every S-check (S1-S12) reads false or "too short".
Per the reopen_reason's own criterion (STRONG scan_hand_coded evidence needed
to keep canonical), this function does NOT clear that bar as hand-written.

## s1 — remaining gap, register-seat anomaly

Per [[register-alloc-pure-c]] step 0: MIPS gcc-2.7.2 prefers LOWEST-numbered
scratch regs first ($v0=2 before $t9=25). The TARGET uses $t9 (the highest
scratch reg), our natural build picks $v0 (the lowest) — by that rule's own
diagnostic, the TARGET is the anomaly, i.e. something not present in our
single-statement body pins the original to a late register. Candidates for
next session: (a) genuinely hand-written register choice (consistent with a
human picking an "out of the way" scratch reg) — but weighed against scan_hand_coded's
LOW/0 tier; (b) the real original C had additional register pressure/liveness
(e.g. more statements, inlined context) not reproduced by this minimal
single-store body, pushing the natural allocator past $v0..$t8. Not yet
distinguished. No register pin attempted (forbidden; [[register-asm-pins]]).

## s1 — remaining gap, trailing nop

Target ends `jr $ra; nop; nop` (two nops after the jr); our build emits only
one (`jr $ra; nop`). Next function in the TU (func_80052C28) starts
immediately after the second nop with no gap, so it is not inter-object
padding ([[rodata-object-alignment]] "Inter-object padding") — it is inside
this function's own 6-insn target span. Not yet explained; no C form tried
reproduces it. Flagged `source-level` by the sandbox diff (register-seat
levers cannot reach it).

## s1 — register-allocation dump: single-pseudo functions always land $v0

`.greg` dump (`tools/grinder/dump.ps1 func_80052C10`, `tmp/grind/func_80052C10/s1/3AB48.greg.txt`)
for the `*(vu32 *)0x1F800400 = 0;` body: the entire function is ONE pseudo
(reg 72, no conflicts listed, "Hard regs used: 2") and global.c allocates it
to `$v0` — the lowest-numbered scratch register, as expected per
[[register-alloc-pure-c]]'s ordering rule. There is no competing live value in
this function to push the allocation past `$v0`. To reach `$t9` by genuine
register pressure the function would need ~13 other simultaneously-live
scratch pseudos (`$v0,$v1,$a0-$a3,$t0-$t8` all occupied before `$t9` is
touched) — implausible for a body whose entire semantic content is "write
zero to one address." This sharpens (not yet fully kills) hypothesis 1: no
*minimal* natural-C body reaches `$t9`; only a body with substantial other
live state could, and no such state is suggested by any caller.

## s1 — non-volatile form: combine.c DOES fold here (grant partially right)

Tested `*(s32 *)0x1F800400 = 0;` (no `vu32`) via `--candidate`:
score 4, but build_insns=4, hunks reclassed as BOTH source-level (`lui
$at,0x1f80` + `sw $zero,1024($at)` — the fold the grant describes — vs
target's unfolded 3-insn `lui;ori;sw`). So the grant's "combine.c always
folds" claim is correct for the **non-volatile** read of this address, and
only fails to describe the **volatile** form (which matches the target's
unfolded 3-insn shape insn-for-insn except for register, as already banked).
The grant's absolute "always"/"unreachable from any C" wording is still
measurably wrong (volatile is exactly the counter-example), but its premise
materializes for the more natural non-volatile spelling — the grant's error
is narrower than first read: it omitted the `volatile` case rather than being
wrong about combine.c's default behavior.

## s1 — caller-site evidence: function genuinely takes no parameter

Every call site (`grep -rn func_80052C10 src/main/*.c`) calls it as `void
func_80052C10(void)` except two in `src/main/2B344.c` (lines 2289, 2693)
which already carry a landed `/* FAKE: the argument is unproven -- the halt
stub ignores its arguments */` annotation explaining the caller passes an
argument the callee ignores, to match a specific call-site's register
pressure — i.e. prior landed work already established func_80052C10 is a
true `void(void)` sink with no parameter-carried state. This rules out "a
missing parameter explains the register pressure" as a hypothesis — no
caller evidence supports one, and the two argument-carrying call sites are
about THEIR OWN register pressure, not this callee's body.

## s1 — sibling function immediately after has a confirmed hand-written insn

`asm/funcs/func_80052C28.s` (the very next function in this TU, starting
right where our target's second trailing `nop` ends, per the earlier
no-inter-object-padding finding) carries an explicit `/* handwritten
instruction */` comment on its second insn (`sub $a1,$t0,$a1` — a non-canonical
operand order no natural C `a-b` expression would select, per
[[compare-operand-order-register]]'s family of findings). This is an
affirmative hand-written-asm signal in the immediate neighborhood, which
argues for keeping/strengthening the canonical-asm reading of this cluster
rather than reopening it — `scan_hand_coded`'s LOW/0 tier on `func_80052C10`
specifically does not generalize to its neighbors having no hand-written
content either.

- [s1] sandbox --diff on `*(vu32 *)0x1F800400 = 0;`: score 4, 5 build insns vs 6 target insns; hunk1 operand-only (lui/ori/sw on $v0 vs target's $t9, otherwise identical 3-insn unfolded shape); hunk2 source-level (target has an extra trailing nop after jr). Reproduced this session via `sandbox --candidate memory/grind/func_80052C10/candidate.c --diff`, saved to tmp/grind/func_80052C10/s1/sandbox_vu32_score4.txt.

- [s1] sandbox --diff on `*(s32 *)0x1F800400 = 0;` (no volatile): score 4 but only 4 build insns; combine.c folds the offset into the sw displacement (`lui $at,0x1f80; sw $zero,1024($at)`), confirming the canonical-asm grant's 'combine.c always folds' claim IS correct for the non-volatile reading of this address; it fails only to describe the volatile case.

- [s1] .greg dump (tmp/grind/func_80052C10/s1/3AB48.greg.txt) shows the vu32 single-statement body has exactly one pseudo (reg 72, 'Hard regs used: 2', zero conflicts), allocated to $v0 by global.c's lowest-free-register rule (register-alloc-pure-c step 0) -- reaching $t9 needs ~13 other live scratch values in-function, none suggested by any evidence gathered.

- [s1] Every caller of func_80052C10 (grep across src/main/*.c) calls it as a true void(void) sink; the two call sites that pass an argument (src/main/2B344.c:2289,2693) already carry a landed `/* FAKE: the argument is unproven -- the halt stub ignores its arguments */` annotation explaining THEIR OWN register pressure, not the callee's -- rules out a missing-parameter explanation for func_80052C10's own register seat.

- [s1] asm/funcs/func_80052C28.s (the function immediately following func_80052C10 in this TU, confirmed via rodata-object-alignment's no-inter-object-padding check to abut it directly with no gap) carries an independent, already-landed `/* handwritten instruction */` comment on its second insn -- an affirmative hand-written-asm signal in the immediate neighborhood that argues against reopening the canonical-asm grant on func_80052C10's LOW/0 scan_hand_coded tier alone.

- [s1] scan_hand_coded --single func_80052C10 --json: score 0, tier LOW, no strong indicators (re-confirmed from prior session, not re-run this session).

- [s2] grep REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/* -- no hits; MIPS port uses global.c's #ifndef REG_ALLOC_ORDER default (ascending hard-reg-number preference), contradicting the s1 frontier's informal ~13-register pressure estimate (which omitted $s0-$s7).

- [s2] Empirical sweep (tmp/grind/func_80052C10/s2/{p5,p9,p13,q13,q15,q20,q22,q23}.greg): target pseudo's hard-reg assignment climbs monotonically with N -- N=5->$v0(2), N=13->$s0(16), N=15->$s2(18), N=20->$s7(23), N=22->$t9(25), N=23->reg30 (a competitor claims t9 first once pressure exceeds the threshold).

- [s2] sandbox --candidate q22.c --diff: score 106, 110 build insns vs target's 6, with an addiu $sp,$sp,-32 prologue -- confirms reaching $t9 via this chassis categorically changes the function's shape (prologue/epilogue overhead the real target asm does not have).

- [s2] Baseline reconfirmed this session: candidate.c (*(vu32*)0x1F800400 = 0;) measures sandbox score 4, matches s1's recorded floor exactly (saved tmp/grind/func_80052C10/s2/baseline_diff.txt).
