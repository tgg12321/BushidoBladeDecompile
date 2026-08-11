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

## Session 2 (structural, 2026-08-11)

### Session-start state correction
- The s1 SRC edits were NOT in the tree at session start (sandbox scored 20,
  build 185/189, arities back to 1-arg). Only the ledger was committed
  (623155a0); the driver evidently discarded the uncommitted src edits.
  Reapplied all four s1 edits from candidate.c → sandbox 2 (188/189)
  confirmed. NEXT SESSIONS: always verify src against candidate.c before
  trusting the digest floor.

### CONFIRMED win 3 — chained same-variable accumulation closes the fold
cluster (floor 2 → 0)
The form (applied in src/ings.c poll loop):
```c
s32 cnt = GetRCnt(0xF2000001u);
s32 lim = D_800A36F1;
lim = lim - 1;
lim = lim << 8;
lim = lim + 0x80;
if (cnt >= lim) break;
```
Measured: micro-harness (tmp/grind/main/s2/foldM4.c → foldM4.s) emits
`lbu; addiu -1; sll 8; addiu 0x80; slt` exactly; whole-file sandbox
--disable all = **0** (189/189 insns, 25 rules dropped, 68 cheat-asm insns
in other ings.c functions still stripped — main's own body is pure C).

### Mechanism (pinned in tools/gcc-2.7.2/combine.c — read, not guessed)
- The fold everyone fights is the ashift/plus distribution at combine.c:8196
  (unconditional in simplify_shift_const) — but the distribution only reaches
  the insn stream via try_combine, and its unrecognizable 2-operand result
  (set r (plus (ashift r 8) -256)) must pass the 2->2 SPLIT gate at
  combine.c:1821-1836. The last guard there is
  `&& ! reg_referenced_p (i2dest, newpat)` ("We can't overwrite I2DEST if
  its value is still used by NEWPAT").
- When the WHOLE chain accumulates through ONE C variable (one pseudo,
  GCC 2.7.2 has no SSA), every combine attempt's newpat still contains that
  pseudo as the chain root, so the split is refused at every level:
  - 2-insn (addiu-1 → sll): newpat = (set rL (plus (ashift rL 8) -256)),
    i2dest = rL, rL referenced → refused.
  - 3-insn (addiu-1, sll, addiu-0x80): newpat = (set rL (plus (ashift rL 8)
    -128)), i2dest = rL (the self-referencing sll) → refused. added_sets_1/2
    both 0 (each value dies at/is set by the next insn), so no PARALLEL path.
  - (sll → addiu 0x80) 2-insn alone re-splits into the same two insns —
    byte-neutral either way.
- Corollary measured constraints (analysis, from the same code read):
  - The lbu MUST share the pseudo. `s32 lim = D_800A36F1 - 1;` (fresh
    subexpr temp for the load) leaves newpat referencing only the temp,
    i2dest (lim) is substituted away → split proceeds → folds. This is
    why every s1 statement-split (fresh named intermediates) failed (H5).
  - Minimal blocking form is 3 statements (`lim = D_800A36F1; lim = lim - 1;
    lim = lim << 8;` + `cnt >= lim + 0x80` in the compare) — analyzed but
    NOT measured; the measured-0 form is the 4-statement chain.
- Secondary finding from the same read: can_combine_p refuses combining
  across a CALL_INSN (combine.c:929, `INSN_CUID (insn) < last_call_cuid &&
  ! CONSTANT_P (src)`) — unusable here (no call sits between addiu and sll
  in target bytes) but bankable for other functions.

### Policy status — WHY THIS IS A RULING-REQUEST, NOT candidate-ready
- Every statement in the form is live (each value read by the next); no dead
  stores, no volatile, no pins, no asm. Detectors have nothing to flag.
- But the operative family is same-variable reuse steering COMBINE with a
  byte-MATERIALIZING effect. Nearest sanctions:
  - split-init-accumulation (user 2026-06-13, commit ad11a8c8): scope
    sentence "This sanctions the specific shape above (split a real `a+b`
    into init + `+=` on the same var, combine folds it back)" — byte-NEUTRAL
    refs-lift, single step. Ours: 3-step chain, combine does NOT fold back.
    The memory adds: "New *adjacent* spellings still need their own user
    ruling."
  - "Variable reuse for codegen control" (frozen SOTN list): scope "reusing
    one C variable for two unrelated values to influence loop-invariant
    detection or RA" — ours is related staged values steering combine.
- No frozen family covers the construct verbatim → per the standing prime
  directive (first reach of an unsanctioned family = ruling request), s2
  returns ruling-request with bytes proven. If the owner sanctions it, the
  next session submits candidate-ready with the self-vet citing the new
  ruling; if refused, the frontier reverts to finding a different
  fold-blocking shape (multi-use is byte-impossible per the s2 nop-slot
  audit: every free slot in the target loop is a literal nop, so any extra
  live insn mismatches).
