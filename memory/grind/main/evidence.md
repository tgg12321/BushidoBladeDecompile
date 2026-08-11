# Evidence bank — main

## Session 1 (recon, 2026-08-11)

### Baseline
- canonical: verdict C, total 189 insns, distance 20 (pure-C target).
- sandbox --disable all at session start: **score 20**, build 185/189 insns,
  25 regfix rules dropped (regfix.txt:1566-1590), 68 cheat-asm insns stripped
  (those pins/asm are in OTHER ings.c functions — main's body itself is pure C).
- Register map is ALREADY correct: s0=ot, s1=idx, s2=env, s3=voice, s4=tbl
  (matches target exactly). Frame 0x28, saves ra+s0..s4 — matches.

### CONFIRMED win 1 — call arities (floor 20 → 9)
Target 0x7A4C-0x7A5C: `func_80016A8C` is called with THREE args:
a0=0x80118800, a1=$s2(env), a2=$s1(idx) — env/idx are UNINITIALIZED at that
point (original passed garbage; the callee ignores them). Target 0x7CBC-0x7CC4:
`func_80016E60` is called with TWO args: a0=env, a1=idx.
Fix applied in src/ings.c: widened both definitions
(`func_80016A8C(u8 *arg0, u8 *arg1, s32 arg2)` at line ~311,
`func_80016E60(u8 *arg0, s32 arg1)` at line ~441 — the only callers are in
main, verified by repo-wide grep; unused params emit zero code in their own
bodies) and updated main's two call sites to
`func_80016A8C((u8 *)0x80118800, env, idx);` / `func_80016E60(env, idx);`.
Measured: 20 → 9.

### CONFIRMED win 2 — `cnt = tbl[idx]` named-first (floor 9 → 2)
The 0xFFFECC00 cluster (target 0x7BB4-0x7BF0): target materializes the
0xFFFECC00 constant in $a0 (lui hoisted into the bnez-voice delay slot,
re-materialized after PutDrawEnv, ori after the .L800173D0 join label — all
reorg.c delay-slot/dup effects that come free), and interleaves the tbl[idx]
load with the adj computation so the constant stays live longer.
The C shape that produces it exactly:
```c
{
    s32 cnt = (s32)tbl[idx];
    s32 adj = D_800A38B4 + 0xFFFECC00u;
    s32 remaining = cnt - adj;
    ...
}
```
(named load first, then adj, then subtraction). Measured: 9 → 2.
- REJECTED variant: single-expression
  `remaining = (s32)tbl[idx] - (s32)(D_800A38B4 + 0xFFFECC00u);` → score 13.
- Prior form (adj first, tbl[idx] inline in the subtraction) → 9.

### The LAST cluster (score 2) — combine fold of ((x-1)<<8)+0x80
Target 0x7B58-0x7B6C (inside the GetRCnt poll loop):
```
lbu  $3, D_800A36F1(gp)
addiu $3, $3, -1        <- WE ARE MISSING THIS
sll  $3, $3, 8
addiu $3, $3, 0x80      <- ours: addiu $3, $3, -128
```
Our build folds to `sll; addiu -128` (188 vs 189 insns; score 2 = the missing
insn + the differing addiu constant).
- Mechanism CONFIRMED by micro-experiment (tmp/grind/main/s1/fold.c + cc1 -da):
  the -128 first appears in the **.combine** dump (cse dump still has -1).
  The transform is combine.c:8196 `(ashift (plus foo C) N) -> (plus (ashift
  foo N) C<<N)` — UNCONDITIONAL when C is CONST_INT — followed by 2-insn
  const-merge (-256 + 0x80 = -128). A 2->2 split via find_split_point is
  accepted, so the distribution always goes through on a single-use chain.
- Statement-splitting does NOT block it (RTL-level, not tree-level fold):
  `s32 lim = D_800A36F1 - 1; if (cnt >= (lim << 8) + 0x80)` still folds
  (measured: still score 2, still 188 insns).
- `lim` computed BEFORE the GetRCnt call → lbu moves above the jal (target has
  it after) and score REGRESSES to 7. The lbu must stay after the call, i.e.
  the value must be computed after GetRCnt returns.
- Blocking requires (per combine.c try_combine semantics): the (x-1) pseudo
  NOT dying at the sll (a second real use → added_sets_2 forces a PARALLEL
  that fails recog), OR the addiu/sll being in different basic blocks (no
  LOG_LINK). Target has the whole chain in one BB, so the original's shape
  had some other property — unknown yet.
- Ruled out analytically (produce wrong bytes): opaque `one` variable (emits
  subu reg-reg), u8/u16 intermediate (emits andi), s16 intermediate
  (promoted-mode, identical RTL), `| 0x80` (disjoint-bits ior->plus merge
  likely folds identically, and target uses addiu anyway).

### Current src state (applied, floor 2)
main loop-head / poll-loop region as of session end:
```c
    do {
        s32 cnt = GetRCnt(0xF2000001u);
        s32 lim = D_800A36F1 - 1;
        if (cnt >= (lim << 8) + 0x80) break;
        rand();
    } while (1);
```
(the lim split is byte-identical to the original inline form — kept because it
is the natural reading; either spelling scores 2.)

### Cosmetic notes for future diff reads
- All `li`/`move` vs `addiu`/`addu` lines in sidediff output are assembler
  aliases, NOT diffs. Use tmp/grind/main/s1/sidediff.py.
- The two-label loop head (.L80017278/.L8001727C with `addiu $a1,$zero,0x1008`
  between them) and all the delay-slot duplications of that li are reorg.c
  steal-from-target effects — they come free; the loop-head li a1 ordering
  (li BEFORE the D_800A36AC lw) already matches after win 1.

- [s1] canonical: verdict C, 189 insns; session-start honest floor 20 (25 rules regfix.txt:1566-1590; 68 stripped cheat-asm insns are in OTHER ings.c functions - main's body is pure C)

- [s1] register map s0=ot s1=idx s2=env s3=voice s4=tbl and frame 0x28 already match target

- [s1] floor now 2 with edits in place in src/ings.c: build 188/189 insns; sole residual = missing addiu $3,$3,-1 after lbu D_800A36F1 (+ addiu 0x80 vs -128) inside the GetRCnt poll loop

- [s1] combine block preconditions (from combine.c try_combine): second real use of the (x-1) value (added_sets_2 -> PARALLEL fails recog) or cross-BB LOG_LINK break; target chain is single-BB so original had some other single-use-breaking shape - unfound

- [s1] ruled out analytically for the fold cluster: opaque-one variable (subu reg-reg, wrong bytes), u8/u16 intermediate (andi, extra insn), s16 intermediate (promoted mode, identical RTL)

- [s1] rules map: @20-22+@167-171 arities (solved), @27-29+labels @146-169 loop-head li a1 (solved), @104-114 0xFFFECC00 (solved), @83-84 fold cluster (open) - all 25 rules retirable once fold closes

- [s1] regression guard: func_80016A8C/func_80016E60 signature widenings are load-bearing; sole callers are in main (repo grep), unused params emit no code
