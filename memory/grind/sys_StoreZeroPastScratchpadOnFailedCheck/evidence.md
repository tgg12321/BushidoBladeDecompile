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

- [s3] decomp-permuter campaign (chassis A, candidate.c): 38,674 iterations / 833.3s, 0 novel finds. Workspace tools/decomp-permuter/nonmatchings/func_80052C10/. Summary: tmp/grind/func_80052C10/s3/campaign_summary.txt.

- [s3] decomp-permuter campaign (chassis B, pointer-local form, confirmed byte-identical sandbox score 4 to chassis A before permuting): 46,787 iterations / 1096.8s, 0 novel finds. Workspace tools/decomp-permuter/nonmatchings/func_80052C10_ptr/. Same summary file.

- [s3] Both campaigns were seeded fresh this session (no prior permuter campaign existed in the ledger for this function) and both harvested+stopped within-session per fresh-seed discipline (>=20k iterations, 0 novel, before stopping). No campaign left running at session end.

- [s3] Sibling puts.c (src/main/psxsdk/libc2/puts.c) read in full: shares no construct with func_80052C10's chassis (NULL-check + putchar loop vs single MMIO store) — nothing to transplant, not a missed lead.

- [s3] sandbox --diff on candidate.c reconfirmed this session: score 4, 5 build insns vs 6 target insns, hunk1 operand-only ($v0 vs target $t9), hunk2 source-level (missing trailing nop) -- unchanged from s1/s2.

- [s3] Built two hand-crafted decomp-permuter workspaces (no pre-existing nonmatchings entry for this function): tools/decomp-permuter/nonmatchings/func_80052C10/ (chassis A) and .../func_80052C10_ptr/ (chassis B), each with base.c/compile.sh/target.o/settings.toml, target.o assembled from asm/funcs/func_80052C10.s with the gp=64 prelude directive stripped (32-bit r3000 assembler rejects it, same fix as tools/mar_perm_workspace.sh uses) and compile.sh modelled on tools/decomp-permuter/nonmatchings/func_80020E74/compile.sh with the sdata-* maspsx flags dropped (sdata_syms.txt/sdata_funcs.txt/sdata_exclude.txt do not exist in this repo).

- [s3] Chassis B (named pointer local) independently verified byte-identical (sandbox score 4, same hunk structure) to the banked candidate before permuting -- not a regression, a lateral chassis swap for more mutator surface.

- [s3] Combined 85,461 permuter iterations across both chassis, 0 novel finds total -- both campaigns harvested+stopped within this session per fresh-seed discipline; no campaign left running (verified via pgrep after harvest).

- [s3] No pre-existing permuter campaign was banked on this chassis before this session, so the CHASSIS RULE's 'do not re-seed' restriction did not apply at launch time; it now does for any future session targeting these same two chassis shapes.

- [s3] Sibling puts.c read in full: no shared construct with func_80052C10, confirmed not a missed lead.

- [s4] ENUMERATION: 16 spellings, best 4, 16 at the floor (all flat). Ran `tools/spelling_enum.py`
  on a two-decl region (`vu32 *p = ...; u32 val = 0; *p = val;`) -- its decl-regex only recognized
  `val` (the `vu32 *p` line fails the DECL_RE because there is no whitespace between the `*` and
  `p`; it falls through to ANCHOR), so the tool itself only produced the named/inlined-`val` pair
  (v0/v1, both score 4). Widened by hand to 14 more spellings covering every axis the single-store
  statement admits: pointer named vs inlined (v14 vs v6/v7/v11/v12), decl order p-before-val vs
  val-before-p (v2 vs v3), address as a named `u32 addr` local vs inline literal (v4), split
  decl+assign for the pointer (v5), explicit `(u32)0` / `0u` value casts (v6/v9), extra parens
  around the cast target (v7/v12), `volatile u32 *` vs `vu32 *` spelling (v8), lowercase hex
  literal (v11), `vu32 *const p` (v10), and a trailing explicit `return;` (v13, tests whether an
  explicit return statement changes trailing-nop emission). Swept with
  `tools/sweep_variants.py --func func_80052C10 --file main/3AB48 --variants
  tmp/grind/func_80052C10/s4/enum --json` (via WSL, `python3 -m venv` toolchain) -- every one of
  the 16 variants (v0-v14 plus the tool's v0/v1) scored identically: score 4, build_insns 5, same
  two hunks (operand-only $t9/$v0 seat + source-level missing trailing nop) as the banked
  candidate. Full output: tmp/grind/func_80052C10/s4/sweep_output.json.

- [s4] sandbox --diff confirms the same two hunks as prior sessions: hunk1 operand-only (target lui/ori/sw use $t9, ours use $v0), hunk2 source-level (target has one extra trailing nop after jr $ra that no C body here emits).

- [s4] spelling_enum.py's DECL_RE does not match a pointer declaration written as 'TYPE *name = expr;' when there is no whitespace between '*' and the name (e.g. 'vu32 *p'), so it falls through to an ANCHOR statement instead of a DECL -- a tool parsing limitation worth noting for future sessions using the tool on pointer-typed regions.

- [s4] ENUMERATION: 16 spellings, best 4, 16 at the floor -- full histogram and per-variant scores in tmp/grind/func_80052C10/s4/sweep_output.json.

- [s4] The zero-hit sweep covers declaration axis (named/inlined pointer and value), ordering axis (p-before-val / val-before-p), address representation (literal vs named local), value representation (0 / (u32)0 / 0u), cast parenthesization, volatile-vs-vu32 typedef spelling, hex literal case, const-qualified pointer, and an explicit trailing return -- the full syntactic spelling space reachable from this one-statement chassis without changing its semantics.

## s5 (synthesis) — trailing double-nop census: the tail occurs only in Sony asmpsx modules

`tmp/grind/func_80052C10/s5/dnop.py` scans all 1437 `asm/funcs/*.s` for a body whose last three insns
are `jr $ra; nop; nop`. Output (`tmp/grind/func_80052C10/s5/double_nop_funcs.txt`): exactly 11
functions — LoadAverageCol, Lzc, MulMatrix, MulMatrix0, MulMatrix2, ReadGeomScreen, RotMatrix,
RotMatrixZYX, SetGeomScreen, _remove_ChgclrPAD, and func_80052C10. Every one of the other 10 has a
canonical-asm row in `inline_asm_canonical.txt` tagged "[re-audit 2026-10-15: PsyQ 4.0 ... built by
asmpsx]" (LIBGTE MSC06/SMP_00/MTX_03/REG09/REG13/FGO_01, LIBAPI CHCLRPAD). No compiled-C function
in the binary ends with that tail. GCC 2.7.2 emits nothing after a function's final `jr $ra` delay
slot and aligns functions with `.align 2`, and maspsx inserts nops only for load/mult hazards inside
a body. So the second nop is not something GCC can emit inside this function's span. The rule's
representation for a nop trailing a canonical asm function with no module evidence is that
function's own `asm/funcs/*.s` (`.claude/rules/rodata-object-alignment.md:63`).

## s5 — neighbourhood: func_80052C10 sits inside a contiguous hand-asm run

In `src/main/3AB48.c:114-122` the run func_80052B7C (LIBGTE matrix-vector leaf, ctc2/mvmva),
gte_ReadFarColor (cfc2), **func_80052C10**, func_80052C28 (trapping `sub` + `addi`, which GCC 2.7.2
never emits for C int arithmetic), func_80052C4C (trapping `sub`) is all canonical asm. func_80052C10
starts on a 16-byte boundary (0x80052C10) and abuts func_80052C28 with no gap.

## s5 — block-local pressure chassis does not reach $t9 either (s4 frontier item 2)

The s4 frontier asked whether pressure from short-lived temporaries in a single block (local-alloc
rather than global.c) could reach $t9 without $s0-$s7 saves. Chassis: N `u32 aK = *(vu32
*)0x1F800{4K}` loads, then `*(vu32 *)0x1F800400 = 0;`, then N stores of the aK. Measured with `sandbox
--disable all --diff --candidate` (files `tmp/grind/func_80052C10/s5/blk{14,22,24}.c` +
`_diff.txt`). N=14: score 116, 119 insns, the 0x400 address goes in $s0 with a stack frame. N=22:
score 212, 214 insns, the address goes in $a1. N=24: score 238, 240 insns, the address goes in $v1.
GCC spills the excess values to the stack instead of climbing to $t8/$t9. On this chassis the
address never takes $t9, and every N that adds pressure also adds a frame.

## s5 — kill re-audit

`tools/fake_ablate.py` on candidate.c: "No FAKE-annotated constructs found ... nothing to ablate"
(`tmp/grind/func_80052C10/s5/fake_ablate.txt`). None of the six banked instance kills was measured
with a FAKE carrier, and all were measured on the same single-store chassis or on diagnostic
pressure chassis. Nothing is void. The baseline re-measured at score 4 (operand-only $t9/$v0 seat +
source-level trailing nop): `tmp/grind/func_80052C10/s5/baseline_diff.txt`.

## s5b (synthesis, after the 2026-10-16 00:36 Judge ruling) — the Judge's TU-boundary shape, measured

The Judge (docs/grind/decisions.md:1380) FAILed the canonical grant and named one C shape to test: the
trailing nop as a TU boundary, with func_80052C10 ending its own object at 0x80052C24 and an
"8-aligned next object" starting at 0x80052C28. It also argued the pad "does not fit the asmpsx 16-byte
module pad" because 0x80052C28 is not 16-aligned. Both grounds were measured this session.

- **No link mechanism 8-aligns an object start (original binary).** `tmp/grind/func_80052C10/s5/tu_boundary_census.py`
  over build/bb2.map (link order mirrors the original): of 225 non-empty .text objects, 102 start at
  4 mod 8 with no pad before them. 60+ of those are LIBSCAN-verbatim PsyQ modules whose boundaries are
  the original's own (libc2 rand/strcpy/prnt, libgte msc*/mtx_*/reg*/smp_*/fgo_*, libsnd ssend/scsmvol/...,
  libspu s_i/s_ini/..., libapi a07/a10/a13/c57). PSYLINK concatenated objects at 4-byte granularity
  (docs/grind/rodata-align-2026-09-30.md, "PSYLINK concatenated modules at 4-byte granularity").
  An object boundary at 0x80052C24 would therefore have put func_80052C28 at 0x80052C24, not 0x80052C28.
  Output: `tmp/grind/func_80052C10/s5/tu_boundary_census.txt`.
- **Scratch link (tmp/grind/func_80052C10/s5/link.sh, link_out.txt).** a.o = func_80052C10 compiled from
  `*(vu32 *)0x1F800400 = 0;` (cc1 -O2 -G0 -mel -msoft-float, maspsx, as), b.o = a stub func_80052C28.
  With the repo's link model (bb2.ld `.main ... SUBALIGN(2)`, bb2.ld:36): b lands at 0x80052C24, no pad.
  With GNU ld's default (as emits .text 2**4 for every object; 250/250 build objects carry 2**4): b lands
  at 0x80052C30, a three-word pad, not one. With an explicit `. = ALIGN(8)`: still 0x80052C30 (b's own
  2**4 dominates). No link model the repo has, or the original evidently had, yields one word at 0x80052C24.
  Reaching it needs a hand-chosen per-object 8-byte alignment that no other object in the binary shows,
  which is the speculative-boundary case .claude/rules/rodata-object-alignment.md:48-49 bans.
- **The asmpsx 16-byte pad is module-relative, so 0x80052C28 not being 16-aligned does not exclude it.**
  The other ten jr;nop;nop functions (all Sony asmpsx modules) end at ABSOLUTE addresses that are not
  16-aligned: LoadAverageCol 0x8007E43C, Lzc 0x8007F21C, MulMatrix 0x8007EC5C, MulMatrix0 0x8007E5EC,
  MulMatrix2 0x8007ED6C, ReadGeomScreen 0x8007EF9C, RotMatrix 0x8007F5EC, RotMatrixZYX 0x8007F87C,
  SetGeomScreen 0x8007F00C (all 12 mod 16); _remove_ChgclrPAD 0x80079120 (0 mod 16). E.g. mtx_03.o
  (MulMatrix) runs 0x8007EB4C..0x8007EC5C = 0x110 = 17x16 bytes: padded relative to its own start. For
  func_80052C10's word the module start must be 8 mod 16. Two function starts in the 3AB48 run qualify:
  gte_SetMatrixRotTransIRVec 0x80052A88 (module of 0x1A0 = 26x16 bytes through 0x80052C28) and
  math_LerpSVector 0x80052788 (0x4A0 = 74x16). Every function from 0x80052A88 to 0x80052C4C is already
  INCLUDE_ASM/hand-asm in src/main/3AB48.c:107-121. So the asmpsx-module reading fits the bytes exactly,
  contrary to the ruling's (a) objection.
- Baseline unchanged: score 4, hunk1 operand-only ($t9 vs $v0), hunk2 source-level (missing nop):
  `tmp/grind/func_80052C10/s5/baseline_diff_postruling.txt`. No FAKE construct in the candidate (fake_ablate: nothing to ablate).
- **scan_hand_coded over the run (tmp/grind/func_80052C10/s5/run_scan.txt).** math_LerpSVector STRONG
  (S9), math_LerpMatrix3x3 STRONG (S9), func_80052C28 STRONG (S9); gte_MulMatrix0ClearTrans,
  gte_SetMatrixRotTransIR, gte_SetMatrixRotTransIRVec, gte_SetRotTransMatrix,
  gte_SetRotMatrixClearTrans, func_80052B7C all LOW. math_LerpSVector (0x80052788) is one of the two
  module starts the module-relative pad allows, and it carries a gate-class signal of its own: a
  hand-asm module 0x80052788..0x80052C28 (74x16 bytes, pad word = func_80052C10's last word) fits
  the bytes, with func_80052C28 (STRONG) starting the next object. Per the ruling, neighbours'
  signals do not transfer to func_80052C10's own tier, so this is evidence, not a grant.

- [s5] sandbox --disable all --diff on candidate.c: score 4, 6 vs 5 insns, hunk1 operand-only ($t9/$v0), hunk2 source-level (trailing nop): tmp/grind/func_80052C10/s5/baseline_diff_postruling.txt

- [s5] 102/225 linked .text objects start at 4 mod 8, including 60+ verbatim PsyQ modules: the original link concatenated at 4-byte granularity (tmp/grind/func_80052C10/s5/tu_boundary_census.txt)

- [s5] The repo link model (bb2.ld:36 SUBALIGN(2)) inserts no pad at an object boundary; GNU default gives 3 words, ALIGN(8) gives 3 words; none gives 1 (tmp/grind/func_80052C10/s5/link_out.txt)

- [s5] Sony asmpsx 16-byte pads are module-relative: 9 of 10 jr;nop;nop Sony modules end at 12 mod 16

- [s5] A hand-asm module 0x80052788 (math_LerpSVector, STRONG S9) .. 0x80052C28 is 74x16 bytes, so func_80052C10's last word is that module's pad; func_80052C28 (STRONG S9) starts the next object (tmp/grind/func_80052C10/s5/run_scan.txt)

- [s5] func_80052C10's own scan_hand_coded tier is still LOW; per the ruling, neighbours' signals do not transfer, so the canonical grant stays unavailable without a new signal class

## [s6] solver
- classify (object path, run in WSL) says SCHED with a one-nop multiset difference. That is a mis-route: the function has no load, and the nop is after jr's delay slot (tmp/grind/func_80052C10/s6/classify.txt).
- Allocation order is ascending in all three allocators: no REG_ALLOC_ORDER in config/mips/mips.h; local-alloc.c:2248, global.c:1058, reload1.c:3769.
- $25 appears in the MIPS port only behind TARGET_ABICALLS/TARGET_LONG_CALLS (mips.md:6165 call_internal2 and related, mips.c:4874 .cpload). The build uses -mno-abicalls (Makefile:42).
- Pressure ladder (s6/ladder.txt): the one-block seat is $(N+2). It reaches $16 at N=14, already with a frame, and $25 at N=23, framed.
- No pad word sits between any functions from 0x80052754 to 0x80052C10.

- [s6] sandbox --disable all --diff on candidate.c gives score 4: hunk 1 operand-only (lui/ori/sw in $t9 vs $v0), hunk 2 source-level (target has an extra nop after jr's delay slot).

- [s6] inverse_compose classify needs WSL (Windows python cannot find objdump). Its SCHED verdict is a mis-route: there is no load-delay nop to schedule.

- [s6] No REG_ALLOC_ORDER in config/mips/mips.h. local-alloc.c:2248, global.c:1058 and reload1.c:3769 all scan registers ascending.

- [s6] Hard reg 25 is referenced in the MIPS port only by abicalls/long-calls call templates and .cpload (mips.md:6165 etc., mips.c:4874), and the build uses -mno-abicalls.

- [s6] The pressure ladder (tmp/grind/func_80052C10/s6/ladder.txt) puts the address in $(N+2) and crosses into $s0 with a frame at N=14, before it can reach $t8/$t9.

- [s6] No pad word sits between any functions from 0x80052754 to 0x80052C10.

## s7 (forensics) facts
- A 4-line reorder-mode hand-asm source (`li $25,0x1f800400; sw $0,0($25); j $31; nop`) assembled with maspsx --aspsx-version=2.34 and the Makefile AS_FLAGS gives exactly the 6 target words (tmp/grind/func_80052C10/s7/handasm_reorder.txt). The trailing nop is the author's explicit nop after the assembler-filled delay-slot nop. No module padding is needed.
- scan_hand_coded --single: LOW 0/12 (s7/scan_single.txt). S12 exempts the trailing nop as "tail padding" (tools/scan_hand_coded.py:735-737).
- The binary has no abicalls fingerprints: no `jalr $t9`, and only __SN_ENTRY_POINT writes $gp (s7/t9_census.txt).
- The only compiled-C `lui/ori $t9` seat, func_80060E38 (src/main/51268.c:216), is framed (-0x70) with $fp in use.
- 0x800526A0..0x80052C4C is a run of 13 canonical frame-free hand-asm routines; func_80052C10 sits in the middle of it (s7/run_census.txt).

- [s7] Hand-asm reorder-mode body reproduces the target 6 words byte-exact (s7/handasm_reorder.txt)

- [s7] scan_hand_coded --single func_80052C10: LOW 0/12. S12 exempts the trailing nop as 'tail padding' (tools/scan_hand_coded.py:735-737).

- [s7] No abicalls fingerprint anywhere in the binary

- [s7] func_80060E38, the only compiled-C lui/ori $t9 seat, is framed (-0x70, $fp)

- [s7] func_80052C10 is surrounded by 13 canonical hand-asm routines

- [s8] sandbox func_80052C10 --disable all --diff on the banked candidate.c reconfirms score 4/6: 1 operand-only hunk ($t9 vs our $v0 scratchpad-address register) and 1 source-level hunk (missing trailing nop after jr $ra).

- [s8] m2c, decomp.me corpus search, and two additional sibling-asm transplant checks (func_800520B8, func_80060E38) beyond the already-read func_80052B7C all independently reproduce or corroborate findings already banked in s1-s7 (recon/structural/solver/forensics) -- no novel C-level mutation axis surfaced this session.

- [s8] docs/grind/decisions.md 2026-10-16 00:36 ruling (cross-knowledge hit) already FAILed the canonical-asm re-audit for this exact function: scan_hand_coded has no S1-S12 gate-class hit; this session did not re-attempt that gate.

- [s9] OBJECT MODEL: func_80052C10's full body touches exactly one memory operand
  (`*(vu32 *)0x1F800400 = 0;`) and references ZERO named globals -- no `D_*`/`g_*`
  symbol, no struct field, no array index. Per-symbol verdict table (the only row
  possible, since the body has no other operand):
    - literal address 0x1F800400: MATCHES (no mismatch). Checked whether this
      address falls inside any declared scratchpad model: include/game.h:1017-1077
      defines `Unk1F800000Rec` (51268's view of the scratchpad from 0x1F800000),
      whose last member `unkB8` is `Unk1F8000B8Union` (game.h:1017-1025), a union
      with `u8 raw[0x400 - 0xB8]`. sizeof(Unk1F800000Rec) = 0xB8 + (0x400-0xB8) =
      0x400 bytes exactly, spanning 0x1F800000..0x1F8003FF. 0x1F800400 is the first
      byte PAST that struct's end, not a field inside it -- confirmed by address
      arithmetic (0x1F800000 + 0x400 = 0x1F800400), not inference. 17AFC's
      `ScrPad`/sibling view (game.h:1169-1170, 0x1F8002B8..0x1F8003FF) ends at the
      same boundary. grep across named_syms.txt / symbol_addrs.txt /
      undefined_syms_auto.txt for "1F800400": zero hits -- no census entry claims
      this address as a struct member or array element anywhere in the project.
      grep across src/main/*.c for "0x1F8004": zero other hits -- func_80052C10 is
      the only code in the whole project that touches this address, so there is no
      sibling-use evidence of a different intended shape either.
  CONCLUSION: there is no aggregate to declare here and no split-scalar/offset-cast
  to correct (the split-scalars-hide-aggregate / header-type-correction levers do
  not apply -- there is nothing to merge or retype). The existing bare-literal
  `*(vu32 *)0x1F800400 = 0;` form is already the correct, simplest declaration;
  object-model audit surfaces no new lever.
  PREMISES the flat floor=4 argument rests on, stated explicitly for future
  attack: (1) the residual's only two hunks are the $t9 register seat for this
  store's address computation (operand-only, s1/s6/s8) and the missing trailing
  `nop` after `jr $ra` (source-level/not-scored boundary question, s5); (2) no
  global/struct is involved in either hunk -- (1)'s register seat is a pure
  codegen question over a literal constant, and (2)'s trailing nop is a
  TU/linker-boundary question (s5), so object-model correction cannot touch
  either hunk; (3) per s6's solver class-kill, the $t9 seat requires >=23 live
  scratch pseudos under ascending register allocation with a save/restore frame,
  which a 1-statement function cannot organically generate without a FAKE
  pressure-inflation construct (not attempted: no sanctioned family covers
  inventing >=18 new live locals in a function that uses none).
  measured_on: include/game.h:1017-1077 struct definitions (static read); grep
  census over named_syms.txt/symbol_addrs.txt/undefined_syms_auto.txt/src/main/*.c
  for "1F800400"/"0x1F8004" (zero hits beyond func_80052C10 itself); no chassis
  change, no FAKE construct, sandbox re-confirms floor 4 unchanged
  (tmp/grind/func_80052C10/s9/sandbox_candidate.json).

- [s9] OBJECT MODEL audit (mandated this session): func_80052C10's body has exactly one memory operand and ZERO named-global references -- no D_*/g_* symbol, struct field, or array index anywhere in the function.

- [s9] 0x1F800400 = 0x1F800000 + sizeof(Unk1F800000Rec) exactly (0xB8 + (0x400-0xB8) = 0x400); the write targets the word immediately past the declared scratchpad struct's end, not any field within it.

- [s9] grep for '1F800400' across named_syms.txt, symbol_addrs.txt, undefined_syms_auto.txt: zero hits. grep for '0x1F8004' across src/main/*.c: zero hits outside this function's own asm/permuter artifacts. No other code in the project touches this address, so there is no transplantable sibling shape.

- [s9] Per the OBJECT-MODEL AUDIT directive's own precedent case (func_80033550, three per-word scalars that were really one 12-byte record): that pattern requires >=2 adjacent/related scalar accesses to merge into an aggregate. func_80052C10 has only ONE access to ONE address with no sibling access anywhere -- the precedent's structural precondition (multiple splat accesses implying a hidden record) is absent here, so the aggregate-merge family does not apply by its own terms, not merely by an unexamined default.

- [s9] Sandbox re-measurement of the banked candidate.c this session: score 4/6 (target_insns=6, build_insns=5), identical to s1-s8 -- confirms no regression and no improvement from the audit.

- [s9] The flat floor's residual (register seat $t9 for the store address + the missing trailing nop) involves no global/struct at all: the $t9 seat is a codegen question over a literal constant (not a global), and the trailing nop is a TU/linker-boundary question (s5) -- object-model correction has no surface to act on for either hunk.

- [s9] CC1PSX SELF-DISPROOF (driver, ruling 2026-09-08): candidate b819a672de95 scores 4 under our cc1 and 4 under the original cc1psx — SOURCE-SIDE: the original compiler is no closer from this source, so the residual is a spelling not yet found (a pure-C preimage exists by construction).

- [s10] candidate.c (*(vu32 *)0x1F800400 = 0;) re-measured floor 4: hunk 1 operand-only ($t9 vs $v0 seat), hunk 2 source-level (extra trailing nop)

- [s10] scan_hand_coded --single func_80052C10 = LOW 0/12

- [s10] cc1psx_check 2026-10-16T06:03Z: ours 4, psx 4, closer false

- [s10] Rotation record appended to docs/grind/decisions.md: 2026-10-16 — func_80052C10 — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): ROTATED
