# Evidence bank — func_8007526C

## s1 (recon, 2026-09-07) — chassis HEAD f43e0467

### OBJECT MODEL: D_800A36A0 — MATCHES (measured, score 13 with offsets byte-exact)
`func_8007526C` touches exactly ONE global: `D_800A36A0`, loaded once per call as
`lw $a0, %gp_rel(D_800A36A0)($gp)` (asm/funcs/func_8007526C.s:4). It is already
declared in src/text1b.c as `extern u8 *D_800A36A0;` (three redundant decls at
src/text1b.c:6624, :6659, :6791) and lives in sdata_syms.txt:226, so it is a
GP-relative *pointer variable*, not an array. Verdict: **MATCHES**. The declaration
is correct and needs no fix — proven by measurement, not inspection: with the
correct addressing form (see H1 below) every load/store in the built function
carries the target's exact byte offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C
off `$a0`) and the exact `lw $a0, 0($gp)` preamble. No other global, no other
symbol, no struct/aggregate question arises. There is no second flagged symbol.

The `u8 *` + explicit-byte-offset idiom is the established, already-matched idiom
for this pointer elsewhere in the same file (e.g. `func_80075670`, src/text1b.c:6670-6700,
which uses `u8 *base = D_800A36A0;` and `(s16 *)(work + i * 2)`), so the object
model here is consistent with matched neighbours — this is not an unresolved
aggregate-merge question.

### Baseline
- HEAD ships `INCLUDE_ASM("asm/funcs", func_8007526C);` (asm-until-matched); sandbox on
  bare HEAD reports score 91 / `no_c_body: true`, i.e. the full 91-insn function.
- The pre-migration ledger pinned floor 48 (migration_pin.json). Re-measured on the
  current chassis: the retired body minus its `register ... asm("a0"/"a2")` pins
  measures **48** — the pin was chassis-accurate.
- Target shape: `i = 0; p = D_800A36A0; do { switch (*(u8*)(p+0x10)) { 1,2,3,4 } i++; p += 2; }
  while (i < 2);` — a 2-iteration loop over two interleaved `s16` player slots
  (stride 2 bytes), fields at 0x8/0xC/0x10/0x14/0x18/0x38/0x3C.

### H1 CONFIRMED — index-based addressing kills the +0x10 induction-variable bias (48 -> 13)
Writing the loop cursor as an explicit pointer bump (`p = D_800A36A0; ... p += 2;`) makes
GCC 2.7.2 `loop.c` strength-reduction create a NEW basic induction variable biased by the
first address used in the loop:
    tmp/grind/func_8007526C/dumps/text1b.loop:
      "Insn 219: dest address src reg 72 benefit 2 used 1 lifetime 1 replaceable mult 1 add 16"
      (insn 281) (set (reg:SI 130) (plus:SI (reg/v:SI 72) (const_int 16)))
The emitted preamble becomes `lw v0,0(gp); addiu a0,v0,16` and EVERY memory offset shifts by
-0x10 (`lhu v1,-8(a0)` where the target has `lhu $v1, 0x8($a0)`). That is the bulk of the
48-point residual.

Rewriting the cursor as an index-derived pointer INSIDE the loop —
    base = D_800A36A0;  i = 0;
    do { p = base + i * 2;  switch (*(u8 *)(p + 0x10)) { ... }  i++; } while (i < 2);
makes `p` a giv over the biv `i` with add_val 0, so the pre-header initialiser is a bare
`lw a0,0(gp)` and all field offsets stay absolute. **Measured 48 -> 13**, build 93 insns
vs target 91. Every offset, block order, register seat and delay slot now matches.
This is ordinary C (no FAKE construct, no sanctioned-family claim needed).

### H2 KILLED (instance) — re-loading the global per access is worse, not better
Spelling every access as `*(u16 *)(D_800A36A0 + i * 2 + OFFS)` (no local cursor at all)
measures **25**, build 98 insns. `cse` collapses the repeated global loads before `loop.c`
runs, so the source-level repetition does not survive; it only costs extra address insns.
Banked as rejected/global-reload-per-access-score25.c. Loop insn_count only rose 91 -> 95.

### The remaining 13: loop.c move_movables hoists the switch comparison constants
The ONLY residual is that the four switch decision-tree comparison constants are hoisted
into the loop pre-header:
    build:  move a2,zero / li t3,2 / li t2,1 / li t1,3 / li t0,4 / li a3,200 / lw a0,0(gp)
    target: addu $a2,$zero,$zero / addiu $a3,$zero,0xC8 / lw $a0,%gp_rel(D_800A36A0)($gp)
and the target rematerialises `addiu $v0,$zero,N` inside the loop, in the branch delay
slots that our build fills with `nop`. Net +2 insns (4 hoisted `li`, 2 delay slots reclaimed).
The 0xC8 constant is hoisted in BOTH — it is not part of the residual.

Pass attribution is DUMP-PROVEN, not guessed (tmp/grind/func_8007526C/dumps/text1b.loop:6-11):
    Loop from 14 to 259: 91 real insns.
    Insn 222: regno 123 (life 1), move-insn savings 1  moved to 267
    Insn 228: regno 125 (life 1), move-insn savings 1  moved to 269
    Insn 234: regno 126 (life 1), move-insn savings 1  moved to 271
    Insn 237: regno 127 (life 1), move-insn savings 1  moved to 273
    Insn 69:  regno 89  (life 3), move-insn savings 2  moved to 275     <-- the 0xC8, also hoisted in target

### The exact gate predicate (read from the compiler source, arithmetic worked out)
`loop.c:1631` decides whether a movable is worth moving:
      if (already_moved[regno]
          || (threshold * savings * m->lifetime) >= insn_count
          || ...)
with `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532).
- `loop_has_call` is set ONLY by a real `CALL_INSN` in the loop (`prescan_loop`, loop.c) —
  this function has no call, so the multiplier is 2.
- `n_non_fixed_regs` (regclass.c:380-387) counts non-fixed hard regs. MIPS
  `FIRST_PSEUDO_REGISTER` is 68 (config/mips/mips.h:1181) and `FIXED_REGISTERS`
  (mips.h:1188) marks 8 as fixed ($0, $1, $26-$29, $31, $67). We do NOT build with
  `-msoft-float` (Makefile:35 `CC_FLAGS`), so `CONDITIONAL_REGISTER_USAGE` (mips.h:524)
  does not fix the 32 FP regs. => n_non_fixed_regs = 60, **threshold = 2 * 61 = 122**
  (decremented by 3 per movable actually moved, loop.c:1718/1904).
- For each case constant: `savings = n_times_used = 1`, `lifetime = 1` (dump says "life 1").
=> 122 * 1 * 1 = 122 >= insn_count(91) => HOISTED.
For the target NOT to hoist them with the same compiler and flags, the loop's
`insn_count` at `scan_loop` time must exceed the threshold, i.e. the original source's
loop body must expand to **>= 123 RTL insns at loop.c time** (ours: 91; the
global-reload spelling reached only 95). Neither `savings` (>= 1 by construction) nor
`lifetime` (>= 1 for any separate set+use) can be driven lower, and the three structural
gates at loop.c:695-700 all pass unconditionally for compiler-generated constant pseudos
(`! REG_USERVAR_P && ! REG_LOOP_TEST_P` is true for them). So insn_count is the only
free variable in the predicate.

- [s2] OBJECT MODEL: D_800A36A0 - MATCHES (measured). It is the only global func_8007526C touches, is already declared `extern u8 *D_800A36A0;` in src/text1b.c (lines 6624, 6659, 6791) and listed in sdata_syms.txt:226, and is loaded once as `lw $a0, %gp_rel(D_800A36A0)($gp)`. With the correct addressing form the built function reproduces the target's exact gp-relative load and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C), so the declaration needs no fix - proven by measurement (score 13, offsets byte-exact), not by inspection. No second global, no aggregate/struct question, no MISMATCH and no MISMATCH-unmeasured symbol for this function.

- [s2] Bare HEAD ships INCLUDE_ASM for this function, so sandbox reports 91 / no_c_body; the pre-migration pin of 48 re-measured accurately once the retired body was stripped of its `register ... asm("a0"/"a2")` pins.

- [s2] New honest floor is 13 (build 93 insns vs target 91), reached with ordinary C only - no FAKE construct, no sanctioned-family claim required.

- [s2] The entire remaining residual is four `li` insns in the pre-header: GCC 2.7.2 loop.c move_movables hoists the switch decision-tree comparison constants 1, 2, 3 and 4, where the target rematerialises `addiu $v0,$zero,N` inside the loop in the branch delay slots our build fills with nop. The 0xC8 constant is hoisted in BOTH builds and is not part of the residual.

- [s2] Pass attribution is dump-proven, not inferred: tmp/grind/func_8007526C/dumps/text1b.loop lines 4-11 read `Loop from 14 to 259: 91 real insns` then `Insn 222: regno 123 (life 1), move-insn savings 1  moved to 267` and three siblings for constants 1, 3 and 4, plus `Insn 69: regno 89 (life 3), move-insn savings 2  moved to 275` for the 0xC8.

- [s2] The exact gate is loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count`. threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) at loop.c:532; loop_has_call is set only by a real CALL_INSN in prescan_loop and this loop has none, so the multiplier is 2. n_non_fixed_regs = 60 (FIRST_PSEUDO_REGISTER 68 at config/mips/mips.h:1181, minus the 8 entries flagged in FIXED_REGISTERS at mips.h:1188; CONDITIONAL_REGISTER_USAGE at mips.h:524 does not fire because Makefile:35 CC_FLAGS has no -msoft-float). So threshold = 122, and with savings = lifetime = 1 the predicate is 122 >= 91 -> hoist. Keeping the constants in the loop requires the loop to carry >= 123 RTL insns at scan_loop time.

- [s2] Neither savings nor lifetime can be driven below 1 for a separate constant set+use, and the three structural gates at loop.c:695-700 pass unconditionally for compiler-generated constant pseudos (`! REG_USERVAR_P && ! REG_LOOP_TEST_P` is true), so insn_count is the only free variable in the predicate for this residual.

- [s2] src/text1b.c was reverted to HEAD (INCLUDE_ASM) at the end of the session; the score-13 body lives in memory/grind/func_8007526C/candidate.c and applies cleanly under the existing `extern u8 *D_800A36A0;` declaration at src/text1b.c:6659.

## s3 (recon, 2026-09-07) — chassis HEAD 4eedc052, floor RE-MEASURED at 13

### Chassis re-measurement
`memory/grind/func_8007526C/candidate.c` applied verbatim over the `INCLUDE_ASM` line at
src/text1b.c:6660 still measures **score 13, build_insns 93, target_insns 91** on HEAD
4eedc052 (`wteng main sandbox func_8007526C --disable all`). The banked floor is
chassis-accurate; nothing in the s1/s2 conclusions needed restating.

### THE PREDICATE BOUNDARY IS NOW PINNED EXACTLY BY MEASUREMENT, not arithmetic
s1/s2 derived `threshold = 122` from the compiler sources. s3 measured the boundary
directly by dialling the loop's RTL `insn_count` one insn at a time with 1-insn pad
stores (`*(u16 *)(p + 0x40 + 2k) = 0;`, each exactly one `sh` RTL insn) appended to the
candidate's loop body, then reading the `Loop from N to M: K real insns` line and the
per-movable verdicts out of tmp/grind/func_8007526C/dumps/text1b.loop
(script: tmp/grind/func_8007526C/s3/bisect.py):

| loop insn_count | verdict on the four switch-comparison constants (regnos 126/128/129/130) |
|---|---|
| 92 (the candidate, unpadded) | all four `moved to` — HOISTED |
| 122 (30 pads) | FIRST one `moved to`, other three `not desirable` |
| 123 (31 pads) | ALL FOUR `not desirable` — matches the target |
| 133 (14 three-insn pads, pointer-bump chassis) | all four `not desirable` |

The 122 row is the direct fingerprint of `threshold -= 3` at loop.c:1719: the first
movable passes `122 * 1 * 1 >= 122`, which drops threshold to 119 and makes the
remaining three fail. This confirms the `(threshold * savings * m->lifetime) >= insn_count`
predicate at loop.c:1631 with threshold = 122, savings = 1, lifetime = 1, and it confirms
the derivation of `n_non_fixed_regs = 60` (FIRST_PSEUDO_REGISTER 68 at
tools/gcc-2.7.2/config/mips/mips.h:1181; FIXED_REGISTERS at mips.h:1188 flags exactly 8
entries — $0, $1, $26, $27, $28, $29, $31 and reg 67).

**The 0xC8 movable (regno 92, life 3, savings 2) is `moved to` the pre-header at EVERY
insn_count tested, including 123 and 133** — `122 * 2 * 3 = 732`. That is precisely what
the target does (`addiu $a3,$zero,0xC8` in the pre-header, asm/funcs/func_8007526C.s:3),
so the target's loop insn_count lies in the interval **[123, 732]** and our reconstruction
is at 92. THE ENTIRE REMAINING RESIDUAL IS A SINGLE INTEGER: the loop needs **>= 123 RTL
insns at loop.c scan_loop time, i.e. +31 over the current form**, while still collapsing
to the target's 91 final insns.

### The target's own shape is the POINTER-BUMP form, not the index-derived form
Read directly off asm/funcs/func_8007526C.s: the loop-closing branch's delay slot is
`addiu $a0, $a0, 0x2` (line 100) and every field access is an absolute offset off `$a0`
(0x8/0xC/0x10/0x14/0x18/0x38/0x3C), with a bare `lw $a0, %gp_rel(D_800A36A0)($gp)`
pre-header (line 4). So the ORIGINAL source almost certainly bumped a pointer; our
index-derived cursor is a *different* spelling that happens to produce the same
addressing. This matters for the next session: the +0x10 giv bias that s1 saw in the
pointer-bump spelling is NOT caused by the pointer bump per se.

### The giv bias is INDEPENDENT of insn_count (measured) — it is combine_givs base choice
In the padded pointer-bump run at insn_count 133 the dump still says `biv 72 can be
eliminated` and combines every address giv onto the LAST giv in the list
(`giv at ... combined with giv at 344`, which is `add 90` there; in the unpadded run it
was `add 16`). So raising insn_count does NOT restore the unbiased base — combine_givs
always picks the last-registered address giv as the combination base, and the bias equals
that giv's `add` constant. The index-derived cursor sidesteps this entirely (its address
givs have `add 0` relative to the recomputed `p`). Consequence: the two residual
phenomena are governed by DIFFERENT mechanisms and the next session must not assume that
fixing insn_count also permits reverting to the pointer-bump spelling.

### `loop_has_call` is not reachable here (class kill)
`threshold` would halve to `1 * (1 + 60) = 61` if `loop_has_call` were set, and `61 >= 92`
is false — i.e. the CURRENT 92-insn loop would already leave all four constants in place.
But `prescan_loop` (tools/gcc-2.7.2/loop.c:2202) sets `loop_has_call` on exactly one
condition, `GET_CODE (insn) == CALL_INSN`, and a CALL_INSN in this loop emits a `jal` in
the final asm. asm/funcs/func_8007526C.s contains no `jal` anywhere in the loop body
(lines 5-100), so no C form that sets `loop_has_call` can produce the target bytes.

### The movable-registration gates are confirmed unconditional (F2 is closed)
Read at tools/gcc-2.7.2/loop.c:695-716: the three-way disjunct is
`(! maybe_never && ! loop_reg_used_before_p (...)) || (! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)) || reg_in_basic_block_p (...)`.
For a compiler-generated switch-comparison constant pseudo the SECOND disjunct is
unconditionally true (it is not a user variable and it is not the loop test register), so
no ordinary switch/if-chain spelling can stop the constant from being *registered* as a
movable. Likewise `savings` (= n_times_used, >= 1) and `m->lifetime` (>= 1 for any
separate set + use) cannot be driven below 1, and raising either only makes hoisting MORE
likely. insn_count therefore remains the only free variable in the predicate — this is
now source-confirmed, not inferred.

### Sibling func_80074B18 carries nothing transferable (F3 closed)
memory/grind/func_80074B18/ holds only migration_pin.json (floor 79) and
retired-chassis-2026-08/body.c — no evidence.md, no hypotheses.md, and no entry naming it
in docs/grind/decisions.md. Its body uses D_800A36A0 in a completely different way: as a
large struct base with a pointer member at +4 and a byte at +0x65, driving nested
`SetTile`/`SetSemiTrans` loops over 0x10- and 0xC-byte strides. It has neither the 2-slot
stride-2 interleaving nor a switch-in-loop, so there is no addressing or constant-hoist
finding to propagate. The [[sibling-ledger-propagation]] check is done and negative.

- [s3] OBJECT MODEL: D_800A36A0 - MATCHES (re-confirmed by measurement this session). It remains the only global func_8007526C touches, is declared 'extern u8 *D_800A36A0;' at src/text1b.c:6659 immediately above the INCLUDE_ASM line, and is loaded once as a gp-relative load at asm/funcs/func_8007526C.s:4. With candidate.c applied the build reproduces that exact gp-relative load and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C), measured score 13. No second global, no MISMATCH, no MISMATCH-unmeasured symbol, no aggregate/struct declaration question outstanding for this function.

- [s3] Chassis re-measurement: memory/grind/func_8007526C/candidate.c applied verbatim over src/text1b.c:6660 measures score 13, build_insns 93, target_insns 91 on HEAD 4eedc052. The banked floor of 13 is chassis-accurate and needed no correction.

- [s3] THE PREDICATE BOUNDARY IS NOW MEASURED, NOT DERIVED. Padding the candidate loop one RTL insn at a time gives: insn_count 92 -> all four switch constants 'moved to' the pre-header; 122 -> the FIRST is 'moved to' and the other three are 'not desirable'; 123 -> all four 'not desirable'; 133 -> all four 'not desirable'. The 122 row is the direct fingerprint of the threshold -= 3 decrement at loop.c:1719, and it independently confirms threshold = 122 and hence n_non_fixed_regs = 60.

- [s3] The 0xC8 movable (regno 92, life 3, savings 2, product 732) is hoisted at EVERY tested insn_count including 123 and 133 - exactly what the target does at asm/funcs/func_8007526C.s:3. So the target's loop insn_count lies in the interval [123, 732] and the current form sits at 92: the entire remaining 13-point residual reduces to needing +31 RTL insns inside the loop at loop.c scan_loop time that collapse again before the final 91-insn output.

- [s3] The target's own shape is the POINTER-BUMP form, read directly off the asm: the loop-closing branch delay slot is 'addiu $a0, $a0, 0x2' (asm/funcs/func_8007526C.s:100) with absolute field offsets off $a0 and a bare gp-relative load in the pre-header. The banked score-13 candidate uses an index-derived cursor instead, which is a DIFFERENT spelling that coincidentally lands on the same addressing - the next session must not assume the original bumped nothing.

- [s3] The +0x10 giv bias is INDEPENDENT of insn_count (measured): in the padded pointer-bump run at insn_count 133 the dump still reads 'biv 72 can be eliminated' and combines every address giv onto the LAST-registered one (add 90 there, add 16 unpadded). combine_givs always picks the last-registered address giv as the combination base and the bias equals that giv's add constant, so raising insn_count does not restore the unbiased base and the two residual phenomena have different mechanisms.

- [s3] Movable registration cannot be defeated (source-confirmed at tools/gcc-2.7.2/loop.c:695-716): the second disjunct is unconditionally true for expand_end_case's constant pseudos, and savings and lifetime both have a hard floor of 1. Combined with the loop_has_call class kill, insn_count is now the only remaining free variable in the loop.c:1631 predicate.

- [s3] Sibling check done and negative: memory/grind/func_80074B18/ holds only migration_pin.json (floor 79) and retired-chassis-2026-08/body.c, with no evidence, hypotheses or decisions.md entry. Its body uses D_800A36A0 as a large struct base (pointer at +4, byte at +0x65) with nested 0x10/0xC-stride loops - no 2-slot interleaving, no switch in a loop, nothing transferable.

- [s3] src/text1b.c was restored byte-for-byte to HEAD at the end of the session (git diff --stat shows only the pre-existing metrics/events.jsonl churn); the score-13 body lives only in memory/grind/func_8007526C/candidate.c.
