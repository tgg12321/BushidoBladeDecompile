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

## s4 (recon, 2026-09-07) — chassis HEAD 45290724 — ROOT CAUSE FOUND: the build is missing `-msoft-float`; func_8007526C is BYTE-EXACT with it

### Chassis re-measurement
`memory/grind/func_8007526C/candidate.c` applied verbatim over the `INCLUDE_ASM` line in
src/text1b.c still measures **score 13, build_insns 93, target_insns 91** (`wteng main
sandbox func_8007526C --disable all`). The banked floor is chassis-accurate.

### The s1-s3 premise was right about the predicate and wrong about the free variable
s3 pinned the residual to loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count`
and concluded the only free variable is `insn_count`, requiring the loop to carry >= 123
RTL insns (we carry 92) — an implausible +31 that must then vanish again before the final
91-insn output. That reading treated `threshold` as a constant of the toolchain. It is not.
`threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (tools/gcc-2.7.2/loop.c:532),
and `n_non_fixed_regs` (regclass.c) counts the hard regs NOT marked in `fixed_regs`.
`CONDITIONAL_REGISTER_USAGE` (tools/gcc-2.7.2/config/mips/mips.h:524-536) marks **all 32 FP
registers fixed when `!TARGET_HARD_FLOAT`**. So:
  - hard float (what we build with today): n_non_fixed_regs = 60, threshold = 2 * 61 = **122**
    -> 122 * 1 * 1 = 122 >= 92 -> the four switch-comparison constants are HOISTED.
  - soft float: n_non_fixed_regs = 28, threshold = 2 * 29 = **58**
    -> 58 * 1 * 1 = 58 >= 92 is FALSE -> the four constants STAY IN THE LOOP.
    The 0xC8 movable (savings 2, lifetime 3) still gives 58 * 2 * 3 = 348 >= 92 -> still
    hoisted, which is exactly what the target does (`addiu $a3,$zero,0xC8`,
    asm/funcs/func_8007526C.s:3).
The PlayStation has no FPU; every float operation in PS1 code is a libgcc call. A period
PsyQ cc1psx is a soft-float configuration by construction, so the FP registers were fixed in
the original compile. Our Makefile:35 `CC_FLAGS` has no `-msoft-float`, so this port's
`mips-mips-gnu` default (hard float, like the big-endian default that `-mel` already had to
correct — see [[mel-endianness-adoption]]) leaves the 32 FP regs allocatable and doubles
every loop.c movable threshold project-wide.

### MEASURED: with `-msoft-float` the function is byte-identical to the target
Manual reproduction of the exact Makefile:150 pipeline
(`cpp | cc1 | prologue_fix | maspsx | multu_pad | as`) on src/text1b.c carrying
candidate.c, driven by tmp/grind/func_8007526C/s1/pipe.sh and pipe2.sh:

| build | insns | vs target (asm/funcs/func_8007526C.s, 91 insns) |
|---|---|---|
| current flags (hard float) | 93 | sandbox score 13 — the four `li` in the pre-header |
| `-msoft-float` | 90 | identical except ONE missing load-delay `nop` at the loop top |
| `-msoft-float` + func_8007526C in the maspsx label-nop gate list | **91** | **byte-identical, 91/91** |

The only surviving word-level difference in the final comparison is
`lw $a0, %gp_rel(D_800A36A0)($gp)` reading `8f840000` in the un-linked object versus
`8f8405d4` in the target — that is the unrelocated R_MIPS_GPREL16 addend, filled by `ld`.
Comparison script tmp/grind/func_8007526C/s1/shift2.py (branch/jump targets masked);
objdump artifacts tmp/grind/func_8007526C/s1/{hardo,softo,softnop}.dis.
The cc1-level diff (tmp/grind/func_8007526C/s1/{hard,soft}.func.s) shows precisely the four
hoisted `li $8..$11` disappearing from the pre-header and reappearing as `li $2,N` inside the
loop, in the branch delay slots — the target's `addiu $v0,$zero,N` shape.

### The missing `nop` is the known maspsx dot-label gate, not a second unknown
The target's `nop` at asm/funcs/func_8007526C.s:6 is the load-delay slot for
`lw $a0,%gp_rel(D_800A36A0)($gp)` consumed by `lbu $v1,0x10($a0)` across the loop-top label.
maspsx drops it because a label intervenes ([[maspsx-is-label-dot-prefix]]); the documented
remedy is the per-function gate list `maspsx_label_nop_funcs.txt`
(.claude/rules/maspsx-label-nop-gate.md), which currently has 21 entries and does not list
func_8007526C. Adding it (tested via a tmp copy passed to `--label-nop-funcs`, so no repo
gate file was modified this session) produces the 91st insn in the right place.

### BLAST RADIUS OF `-msoft-float`, MEASURED PROJECT-WIDE (not estimated)
tmp/grind/func_8007526C/s1/blast.sh compiled EVERY src/*.c twice (identical flags, with and
without `-msoft-float`) and diffed the cc1 assembly. Of 32 translation units, 30 are
byte-identical apart from the `# -w -mel -msoft-float -o` options-comment line. Exactly two
TUs change code:
  1. **text1b** — 29 diff lines, and a `.ent`-range attribution pass
     (tmp/grind/func_8007526C/s1/blast/) shows every one of them lies inside
     **func_8007526C**. No other function in the TU moves.
  2. **code6cac_b** — 6 diff lines, all inside **func_800324D0** (src/code6cac_b.c:2554):
     the same phenomenon, one loop-invariant `li $8,0xFF` de-hoisted from the pre-header to
     `li $2,0xFF` inside the loop. func_800324D0 is already COMPLETED-C (not in
     engine/queue.json) and carries a `/* FAKE */` duplicated-tail construct from a 21-session
     grind, so a GLOBAL flag flip would change its bytes and break the oracle for it.
The surgical form is therefore a per-file flag, for which the Makefile already has the exact
mechanism: `cc_flags_for` at Makefile:133 appends `-fno-strength-reduce` for members of
`NO_SR_FILES`. A `SOFT_FLOAT_FILES := text1b` list appended the same way touches only
text1b.c, whose sole affected function is the one we are matching.

## s5 (recon, 2026-09-07) — chassis HEAD 793d8b08 — s4's byte-claim INDEPENDENTLY RE-VERIFIED; the disposition entry is now filed in a validator-conforming shape

### Why this session exists
s4 was DISCARDED by the driver validator, and the discard reason was purely clerical:
"owner-gated claim rejected: no OWNER-ESCALATION / CANONICAL-ASM GRANT PATH entry in
docs/grind/decisions.md names func_8007526C". s4 HAD appended an entry, but its heading
read `## 2026-09-07 — func_8007526C — **INTEGRATION HANDOFF (...)**` and the driver's
pre-check (tools/grinder/grind.ps1:1302) requires a single LINE containing the literal
token `OWNER-ESCALATION` or `CANONICAL-ASM GRANT PATH` **and** the function name. None of
s4's technical content was ever disputed; the driver-stamped discarded-session marker
voids the span, so s5 re-measured the entire claim from scratch rather than inheriting it,
and re-filed a self-contained entry titled
`## 2026-09-07 — func_8007526C — OWNER-ESCALATION: **INTEGRATION HANDOFF (bytes proven;
remedy is a build-flag change, the severe-blocker class)**`.

### Chassis re-measurement
`memory/grind/func_8007526C/candidate.c` applied verbatim over the `INCLUDE_ASM` line at
src/text1b.c:6660 measures **score 13, build_insns 93, target_insns 91** on HEAD 793d8b08
(`wteng main sandbox func_8007526C --disable all`). The banked floor of 13 is
chassis-accurate; nothing needed correction.

### INDEPENDENT REPRODUCTION of the `-msoft-float` byte-proof (fresh preprocess, s5 scripts)
s5 did NOT reuse s4's `text1b.i` or object files. `tmp/grind/func_8007526C/s5/repro.sh`
regenerates the preprocessed TU with the exact `Makefile:38-39` CPP flags and drives the
exact `Makefile:150` pipeline (`cpp | cc1 | prologue_fix | maspsx | multu_pad | as`),
using `tools/gcc-2.7.2/build/cc1` — the same binary `Makefile:12 CC1` names, verified, not
assumed. Comparison is `tmp/grind/func_8007526C/s5/cmp.py`, which extracts the function
from each objdump and compares word-for-word against the little-endian words embedded in
asm/funcs/func_8007526C.s, masking only branch/jump targets.

| build | build_insns | masked word diffs vs the 91-insn target |
|---|---|---|
| current flags (hard float) | 93 | 86 |
| `+ -msoft-float` | 90 | 89 (shift artefacts of ONE missing `nop`) |
| `+ -msoft-float` + func_8007526C in the maspsx label-nop list | **91** | **1** |

The one surviving word is index 2, `lw $a0, %gp_rel(D_800A36A0)($gp)`: `8f840000` in our
un-linked object versus `8f8405d4` in the target. `mipsel-linux-gnu-objdump -r
tmp/grind/func_8007526C/s5/softnop.o` reports `R_MIPS_GPREL16 D_800A36A0` at that word, so
the addend is supplied by `ld` and the function is byte-identical after linking. The
byte-proof is therefore CONFIRMED by an independent second measurement, not inherited.

### BLAST RADIUS re-measured inside text1b.c by .ent attribution
`tmp/grind/func_8007526C/s5/attrib.py` diffs the hard-float and soft-float cc1 outputs for
src/text1b.c with difflib and attributes every changed line to its enclosing `.ent` range:
**29 changed lines total — 27 inside func_8007526C, 2 in the file-header options comment.
No other function in the translation unit moves.** This independently reproduces s4's
finding for the TU that matters and is what makes the per-file remedy safe.

### The blocking surfaces are BOTH on the severe-blocker denylist (read this session)
.claude/rules/integration-handoff-self-serve.md:56-73 denylists, for the self-serve
scope-widening path, `Makefile` and `*.ld` by path-class regex AND names
`maspsx_label_nop_funcs.txt` explicitly among "the maspsx fidelity-gate lists —
assembler-behavior gates are substrate-adjacent"; the same file's "what STILL pends the
owner" list names "any change to the oracle, the substrate (cc1/maspsx/linker/Makefile),
or build flags". So neither of the two remaining steps is reachable by a Judge
ESCALATE(integration-handoff) scope grant, and neither is reachable by a grind session.
This is an INTEGRATION HANDOFF in the strict sense of the brief — bytes proven, blocked
only by a surface the session may not touch — and explicitly NOT an endgame lock and NOT
an exhausted ladder.

- [s5] OBJECT MODEL: D_800A36A0 - MATCHES (re-confirmed by measurement this session). It remains the only global func_8007526C touches, is already declared `extern u8 *D_800A36A0;` at src/text1b.c:6659 immediately above the INCLUDE_ASM line and listed in sdata_syms.txt:226, and is loaded once as `lw $a0, %gp_rel(D_800A36A0)($gp)` at asm/funcs/func_8007526C.s:4. With candidate.c applied the build reproduces that exact gp-relative load (R_MIPS_GPREL16 relocation confirmed by `objdump -r` on tmp/grind/func_8007526C/s5/softnop.o) and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C), measured score 13 under current flags and a 91/91 byte-identical function under the handoff's build configuration. No second global, no MISMATCH, no MISMATCH-unmeasured symbol, no aggregate or struct declaration question outstanding for this function.

- [s5] Chassis re-measurement on HEAD 793d8b08: candidate.c applied over src/text1b.c:6660 measures score 13, build_insns 93, target_insns 91. Floor unchanged and chassis-accurate.

- [s5] The `-msoft-float` byte-proof is now supported by TWO independent measurements taken by different sessions with separately written scripts. s5's chain regenerates text1b.i from source with the Makefile's own CPP flags and uses `tools/gcc-2.7.2/build/cc1` (the binary Makefile:12 names): hard float 93 insns / 86 masked diffs, `-msoft-float` 90 insns / 89 masked diffs (one missing nop shifting everything), `-msoft-float` + the maspsx label-nop opt-in 91 insns / 1 masked diff, and that one diff is the unrelocated R_MIPS_GPREL16 addend on the D_800A36A0 load.

- [s5] Soft-float blast radius inside src/text1b.c re-measured by .ent attribution: 29 changed cc1 output lines, 27 of them inside func_8007526C and 2 in the options comment. No other function in the TU changes, so a per-file `SOFT_FLOAT_FILES := text1b` opt-in (mirroring the existing NO_SR_FILES mechanism at Makefile:129-133) is surgical. A GLOBAL flag flip is NOT safe — a prior sweep found code6cac_b's already-COMPLETED-C func_800324D0 also moves.

- [s5] Both remaining steps are severe-blocker class and cannot be self-served: .claude/rules/integration-handoff-self-serve.md:56-73 denylists `Makefile` (path-class regex) and names `maspsx_label_nop_funcs.txt` among the maspsx fidelity-gate lists, and its "still pends the owner" list covers build-flag and substrate changes. A Judge ESCALATE(integration-handoff) scope grant cannot reach either path.

- [s5] src/text1b.c was restored byte-for-byte to HEAD at the end of the session (7097 lines, `git status` clean for src/); the score-13 body lives only in memory/grind/func_8007526C/candidate.c.

- [s4] OBJECT MODEL: D_800A36A0 - MATCHES (re-confirmed by measurement this session). It is the only global func_8007526C touches, is already declared extern u8 *D_800A36A0; at src/text1b.c:6659 immediately above the INCLUDE_ASM line and listed in sdata_syms.txt:226, and is loaded once as lw $a0, %gp_rel(D_800A36A0)($gp) at asm/funcs/func_8007526C.s:4. With candidate.c applied the build reproduces that exact gp-relative load (R_MIPS_GPREL16 relocation confirmed by objdump -r on tmp/grind/func_8007526C/s5/softnop.o) and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C): score 13 under current flags, and a 91/91 byte-identical function under the handoff's build configuration. No second global, no MISMATCH, no MISMATCH-unmeasured symbol, and no aggregate or struct declaration question outstanding for this function.

- [s4] Chassis re-measurement on HEAD 793d8b08: memory/grind/func_8007526C/candidate.c applied verbatim over the INCLUDE_ASM line at src/text1b.c:6660 measures score 13, build_insns 93, target_insns 91. The banked floor is chassis-accurate and needed no correction.

- [s4] The -msoft-float byte-proof now rests on TWO independent measurements by different sessions with separately written scripts. s5's chain regenerates text1b.i from source with the Makefile's own CPP flags and uses tools/gcc-2.7.2/build/cc1: hard float 93 insns / 86 masked diffs, -msoft-float 90 / 89, -msoft-float plus the maspsx label-nop opt-in 91 / 1, and the single residual word is the unrelocated R_MIPS_GPREL16 addend on the D_800A36A0 load.

- [s4] Soft-float blast radius inside src/text1b.c, measured by .ent attribution: 29 changed cc1 output lines, 27 inside func_8007526C and 2 in the options comment. No other function in the TU changes. The per-file remedy (SOFT_FLOAT_FILES := text1b, mirroring NO_SR_FILES at Makefile:129-133) is therefore surgical; a global flag flip is not, because code6cac_b's already-COMPLETED-C func_800324D0 also moves.

- [s4] Both remaining steps are severe-blocker class and cannot be self-served: .claude/rules/integration-handoff-self-serve.md:56-73 denylists Makefile by path-class regex and names maspsx_label_nop_funcs.txt among the maspsx fidelity-gate lists, and the same file's 'still pends the owner' list covers build-flag and substrate changes.

- [s4] s4's discard was purely clerical: it had filed an entry, but its heading lacked the literal OWNER-ESCALATION / CANONICAL-ASM GRANT PATH token that the driver pre-check at tools/grinder/grind.ps1:1302 requires on the same LINE as the function name. s5 re-filed a self-contained entry with a conforming title, and did not rely on the voided s4 span - every claim in it was re-measured this session.

- [s4] src/text1b.c was restored byte-for-byte to HEAD at the end of the session (7097 lines); the score-13 body lives only in memory/grind/func_8007526C/candidate.c.

## s6 (recon, 2026-09-07) - chassis HEAD 9066e9ad - FLOOR 13 -> 1

### OBJECT MODEL: D_800A36A0 - MATCHES (re-confirmed, measured)
func_8007526C still touches exactly one global, D_800A36A0, already declared
`extern u8 *D_800A36A0;` in src/text1b.c and listed in sdata_syms.txt:226. This session's
build reproduces the target's `lw $a0, %gp_rel(D_800A36A0)($gp)` and every absolute field
offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) with the declaration unchanged, at score 1.
No MISMATCH and no MISMATCH-unmeasured symbol; no aggregate/struct question arises.

### H6-1 CONFIRMED - a goto-spelled loop removes loop.c from the picture entirely: 13 -> 1
Every session s1-s5 treated the four hoisted switch-comparison `li` as a loop.c
move_movables threshold problem and searched for +31 RTL insns of loop body
(loop.c:1631, threshold 122 vs insn_count 92). That framing had the wrong lever.

GCC 2.7.2 emits NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END only from expand_start_loop,
i.e. only for `while`, `for` and `do ... while` STATEMENTS. loop_optimize locates loops
exclusively by scanning for those notes (tools/gcc-2.7.2/loop.c), so a loop spelled as a
label plus a backward `goto` is never entered by loop.c at all. Two consequences, both
wanted here:
  (a) move_movables never runs, so the four switch-comparison constants are NOT hoisted -
      they are materialised inside the loop, in the branch delay slots, exactly as the
      target does at asm/funcs/func_8007526C.s:8/12/18/20;
  (b) strength_reduce never runs, so there is no address giv and therefore no +0x10
      induction-variable bias - the very bias that made the natural pointer-bump spelling
      measure 48 in s1/s2 (rejected/pointer-increment-biases-iv-by-0x10.c). `p += 2` at the
      loop bottom compiles literally to the target's `addiu $a0,$a0,0x2` in the closing
      branch delay slot (asm/funcs/func_8007526C.s:100).
The 0xC8 pre-header constant that loop.c used to hoist for us is supplied instead by an
ordinary named local `lim`, initialised before the loop and read at the three `sh $a3,...`
store sites in cases 1 and 3; the `>= 0xC8` tests keep the literal because they assemble to
`slti` immediates (asm/funcs/func_8007526C.s:33/57). Pre-header order
`move a2,zero / li a3,200 / lw a0,0(gp)` matches asm/funcs/func_8007526C.s:2-4 exactly.

MEASURED: `sandbox func_8007526C --disable all` -> score 1, build_insns 90,
target_insns 91, on the current unmodified build configuration (NO -msoft-float, no
compiler-flag change of any kind). Body banked as memory/grind/func_8007526C/candidate.c.
Disassemblies: tmp/grind/func_8007526C/s6/base.dis and gated.dis.

This supersedes the s5 -msoft-float theory as the route to the bytes: the four constants
stay in the loop under the project's existing hard-float configuration. The build-flag
half of the s5 blocker is GONE.

### The whole remaining residual is one maspsx-gate nop (measured, 91/91)
The single missing word is the load-delay nop at asm/funcs/func_8007526C.s:6. It sits
between `lw $a0, %gp_rel(D_800A36A0)($gp)` (the last pre-header insn) and the loop-top
label .L80075278, whose first insn is `lbu $v1, 0x10($a0)` - a load consuming the base
register produced by the immediately preceding load, ACROSS a label. maspsx's is_label()
matches only $L-prefixed locals while this GCC fork emits .L, so maspsx does not see the
hazard and drops the nop. This is verbatim the LOAD-CONSUMER symptom that
.claude/rules/maspsx-label-nop-gate.md documents as a pure-C RETIREMENT path ("canonical
-> C, pure-C distance 1, the lone diff a missing nop across a .L merge label"), with three
precedents: spu_DmaTransfer (main.c, commit 25f8f56), cdrom_DmaToRam (system.c,
2026-05-26) and gnd_get_fog - the last of which is in THIS SAME TU, src/text1b.c.

PROVEN: tmp/grind/func_8007526C/s6/repro.sh rebuilds src/text1b.c through the exact
Makefile:150 pipeline with the CURRENT flags, once with the repo's
maspsx_label_nop_funcs.txt and once with a TEMPORARY COPY that appends func_8007526C
(no repo gate file was modified; the copy was passed as --label-nop-funcs=<temp>).
tmp/grind/func_8007526C/s6/cmp.py does a branch/jump-target-masked word comparison against
asm/funcs/func_8007526C.s:

| build | insns | masked word diffs vs the 91-insn target |
|---|---|---|
| current flags, repo gate list | 90 | 1 (missing nop + index shift) |
| current flags + func_8007526C in the label-nop list | 91 | 1 |

The one remaining word in the 91/91 row is `lw $a0,0($gp)` = 8f840000 un-relocated versus
8f8405d4 in the target: an R_MIPS_GPREL16 D_800A36A0 addend that ld fills at link time. So
the function is byte-identical after linking, with ordinary C and the project's existing
compiler flags.

- [s6] src/text1b.c was restored byte-for-byte to HEAD at the end of the session; the
  score-1 body lives only in memory/grind/func_8007526C/candidate.c.

## s7 (recon, 2026-09-07) — chassis HEAD a04d3e60

### OBJECT MODEL: D_800A36A0 — MATCHES (re-measured this session, score 1)
`func_8007526C` touches exactly ONE global, `D_800A36A0`, already declared in
src/text1b.c as `extern u8 *D_800A36A0;` (src/text1b.c:6624, :6659, :6791) and listed in
sdata_syms.txt:226, i.e. a GP-relative pointer variable, not an array and not an
aggregate. With the banked s6 body applied the built function reproduces the target's
`lw $a0, %gp_rel(D_800A36A0)($gp)` preamble and every absolute field offset (0x8, 0xC,
0x10, 0x14, 0x18, 0x38, 0x3C) word-for-word — verdict **MATCHES**, proven by measurement
(90 of 91 target words identical, sandbox score 1), not by inspection. There is no second
global, no MISMATCH symbol and no MISMATCH-unmeasured symbol for this function, so no
declaration fix is available or needed. This re-confirms the s1 and s2 OBJECT MODEL
entries on the current chassis.

### Chassis re-measurement (the brief's chassis check reported "measurement unavailable")
- Bare HEAD ships `INCLUDE_ASM("asm/funcs", func_8007526C);` at src/text1b.c:6660.
- With `memory/grind/func_8007526C/candidate.c` (the s6 goto-spelled body) applied:
  `sandbox func_8007526C --disable all` → **score 1**, target_insns 91, build_insns 90,
  scorable true, rules_dropped 0. The honest floor on this chassis is **1**, NOT the 13
  recorded in state.json's floor_history and NOT the 48 the SessionStart queue banner
  still quotes; the queue's 48 is the stale pre-s1 pin.
- `canonical func_8007526C` → verdict **C**, asm_insns 0, total 91, distance 1
  ("pure-C distance 1 <= 50 — pure-C target"). The function remains routed pure-C; there
  is no canonical-asm grant path here and `scan_hand_coded` has nothing to work with.

### The single residual word is an assembler-layer nop, and its predicate is in maspsx
The one missing word is the load-delay `nop` at asm/funcs/func_8007526C.s:6, sitting
between `lw $a0, %gp_rel(D_800A36A0)($gp)` (:4) and the loop-top merge label
`.L80075278:` (:5) whose first instruction is `lbu $v1, 0x10($a0)` (:7). I read the
mechanism directly out of the tool source this session rather than inheriting it:
`is_label()` at tools/maspsx/maspsx/__init__.py:257 is
`re.match(r"\$L(b|e)?\d+:$", line)` — it recognises only `$L`-prefixed locals, while this
GCC fork emits `.L`-prefixed ones, so a load whose destination is consumed across a `.L`
merge label is not seen as a load-delay hazard and loses its nop. The narrow workaround
already in the tool (`_handle_nop_before_next_instruction`,
tools/maspsx/maspsx/__init__.py:810-820) is explicitly scoped to the **jalr-consumer**
case only (`lw $rN; .L<n>:; jal $31,$rN`), with an in-source comment saying broadening the
label-skip cascades through index-anchored rules elsewhere. Our consumer is an ordinary
`lbu`, so it is outside that carve-out; the per-function opt-in list
`maspsx_label_nop_funcs.txt` is the sanctioned way to cover a LOAD-CONSUMER case
(.claude/rules/maspsx-label-nop-gate.md), and func_80022F34 is the precedent already in
that list (maspsx_label_nop_funcs.txt:22), applied by the OWNER in the 2026-09-06
foreclosed-bucket review (commit d4338774).

### That surface is mechanically owner-only — re-confirmed by reading the rule
`.claude/rules/integration-handoff-self-serve.md:60-63` names
`maspsx_label_nop_funcs.txt` verbatim on the "Path denylist for add-scope-allow
(severe-blocker class, always refused)" list, alongside the other maspsx fidelity-gate
lists, and :64-65 puts everything under `tools/` behind a path-class regex. So neither a
grind session nor the driver can add the line; only the owner can. This is consistent
with the two standing judge rulings (docs/grind/decisions.md 2026-09-07 11:56 and 12:10)
and is why an INTEGRATION HANDOFF disposition is NOT available: that rule requires bytes
proven at sandbox == 0, and this function measures 1.

- [s5] OBJECT MODEL: D_800A36A0 - MATCHES (re-measured this session, score 1). It is the only global func_8007526C touches, is already declared `extern u8 *D_800A36A0;` in src/text1b.c (lines 6624, 6659, 6791) and listed in sdata_syms.txt:226, i.e. a GP-relative pointer variable, not an array and not an aggregate. With the banked s6 body applied the build reproduces the target's `lw $a0, %gp_rel(D_800A36A0)($gp)` preamble and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) word-for-word - proven by measurement (90 of 91 target words identical), not inspection. No second global, no MISMATCH symbol, no MISMATCH-unmeasured symbol, so no declaration fix is available or needed.

- [s5] Chassis re-measurement (the brief reported 'measurement unavailable'): with candidate.c applied over src/text1b.c:6660, `sandbox func_8007526C --disable all` returns score 1 / target_insns 91 / build_insns 90 / rules_dropped 0, and `canonical func_8007526C` returns verdict C, asm_insns 0, distance 1. The honest floor is 1. The 13 in state.json floor_history and the 48 in the SessionStart queue banner are both stale and were superseded by the s6 body.

- [s5] The single missing word is the load-delay nop at asm/funcs/func_8007526C.s:6, between the gp-relative load at :4 and the `lbu $v1, 0x10($a0)` at :7 that follows the loop-top merge label `.L80075278:` at :5.

- [s5] Mechanism read from tool source this session: maspsx's is_label() at tools/maspsx/maspsx/__init__.py:257 matches only $L-prefixed locals while this GCC fork emits .L, so the load-delay hazard across that merge label is invisible and the nop is dropped. The existing .L carve-out at tools/maspsx/maspsx/__init__.py:810-820 is deliberately scoped to the indirect-call consumer case, with an in-source comment that broadening the label-skip cascades through index-anchored rules elsewhere; our consumer is an ordinary lbu, so it is outside that carve-out.

- [s5] The sanctioned remedy is the per-function opt-in list maspsx_label_nop_funcs.txt (LOAD-CONSUMER case, .claude/rules/maspsx-label-nop-gate.md), which is on the always-refused add-scope-allow denylist at .claude/rules/integration-handoff-self-serve.md:60-63 - owner-only by design. The controlling precedent is func_80022F34, which sat in this exact shape, was foreclosed, and completed when the owner applied the one line in the 2026-09-06 foreclosed-bucket review (commit d4338774); it is now maspsx_label_nop_funcs.txt:22.

- [s5] An INTEGRATION HANDOFF disposition is NOT available and must not be re-filed: .claude/rules/integration-handoff-self-serve.md:12-15 requires bytes proven at sandbox == 0, and this function measures 1 on every buildable configuration. Both previously filed handoffs were FAILed by the Judge on exactly that ground (docs/grind/decisions.md 2026-09-07 11:56 and 12:10).

- [s5] The loop.c insn_count >= 123 axis (s3's frontier F1) is MOOT, not open: the goto-spelled loop bypasses loop.c entirely so move_movables never runs on it. The 2026-09-07 12:10 judge ruling states this explicitly. Recorded here so no later session re-derives it.

- [s5] src/text1b.c was reverted to its HEAD INCLUDE_ASM form at the end of this session; the tree carries only ledger edits.

## s7 (recon, 2026-09-07) — chassis HEAD 19c9eda0 — floor RE-CONFIRMED at 1

### OBJECT MODEL: D_800A36A0 — MATCHES (measured this session, score 1)
`func_8007526C` touches exactly ONE global, `D_800A36A0`. It is already declared
`extern u8 *D_800A36A0;` in src/text1b.c (lines 6624, 6659, 6791) and listed in
sdata_syms.txt:226, i.e. a GP-relative pointer *variable*, not an array. Re-measured on
today's chassis (HEAD 19c9eda0) with `memory/grind/func_8007526C/candidate.c` applied at
src/text1b.c:6660: `sandbox func_8007526C --disable all` -> **score 1, build_insns 90,
target_insns 91**. Every one of the 90 emitted words is word-identical to
asm/funcs/func_8007526C.s, including the `lw $a0, %gp_rel(D_800A36A0)($gp)` preamble
(asm/funcs/func_8007526C.s:4) and all absolute field offsets (0x8, 0xC, 0x10, 0x14, 0x18,
0x38, 0x3C). The declaration therefore needs no fix — proven by measurement, not
inspection. There is no second global, no aggregate/struct question, no MISMATCH and no
MISMATCH-unmeasured symbol for this function. Verdict: **MATCHES**.

### Chassis re-measurement (the brief's CHASSIS CHECK said "measurement unavailable")
The dispatch brief arrived with an EMPTY ledger digest ("floor history: (none yet)",
"live frontier: (empty)", "session 1 of a cumulative grind") and the SessionStart queue
banner still quotes the pre-migration pin of 48. Both are stale. The real ledger
(state.json, 5 floor_history entries) and the measurements below are authoritative:
- HEAD 19c9eda0, `candidate.c` applied: **score 1** (90/91 words). Re-verified, not quoted.
- `canonical func_8007526C` -> verdict **C**, asm_insns 0, distance 1, reason
  "pure-C distance 1 <= 50 — pure-C target". The function is correctly routed to pure C;
  it is NOT a canonical-asm candidate, so the canonical-asm endgame gate FAILS here and
  no CANONICAL-ASM GRANT PATH disposition is available.

### The precedent function is now IN the gate file — direct confirmation of the route
`maspsx_label_nop_funcs.txt` currently holds 21 entries and line 22 is **func_80022F34**,
the controlling precedent the s6 Judge ruling named (decisions.md 2026-09-07 12:10). That
line was applied by the OWNER in the 2026-09-06 foreclosed-bucket review (commit d4338774,
`[infra-rule: maspsx-label-nop]`), after which func_80022F34 landed. `func_8007526C` is
absent from that file. So the sanctioned route for this exact residual is demonstrated,
executed once already, and owner-only by construction: the file is named verbatim on the
add-scope-allow denylist at .claude/rules/integration-handoff-self-serve.md:56-58.

### No new C axis exists for the residual word (re-derived from the target, not quoted)
The target's first four words are fixed in order (asm/funcs/func_8007526C.s:2-6):
`addu $a2,$zero,$zero` / `addiu $a3,$zero,0xC8` / `lw $a0,%gp_rel(D_800A36A0)($gp)` /
`.L80075278:` / `nop` / `lbu $v1,0x10($a0)`. Any byte-matching build must therefore place
the gp-relative load as the LAST insn before the loop-top merge label, with the label's
first insn consuming `$a0`. That is precisely the load-consumer-across-a-`.L`-label shape
whose delay `nop` maspsx suppresses, because `is_label()` (tools/maspsx/maspsx/__init__.py:257)
only recognises `$L`-prefixed labels while this cc1 fork emits `.L`
([[maspsx-is-label-dot-prefix]], .claude/rules/maspsx-label-nop-gate.md). The hazard is a
property of the required instruction ORDER, not of any C spelling, so re-ordering the C
cannot both keep the target order and remove the hazard. This re-confirms the s5/s6 class
kill (predicate tools/maspsx/maspsx/__init__.py:257) from the target bytes independently.

- [s6] OBJECT MODEL: D_800A36A0 -- MATCHES (measured, score 1). It is the only global func_8007526C touches, is already declared `extern u8 *D_800A36A0;` in src/text1b.c (lines 6624, 6659, 6791) and listed in sdata_syms.txt:226, i.e. a GP-relative pointer variable and not an array. With candidate.c applied the build reproduces the target's exact `lw $a0, %gp_rel(D_800A36A0)($gp)` preamble (asm/funcs/func_8007526C.s:4) and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) -- 90 of 91 words word-identical. The declaration needs no fix, proven by measurement rather than inspection. No second global, no aggregate/struct question, no MISMATCH and no MISMATCH-unmeasured symbol for this function.

- [s6] Chassis re-measurement on HEAD 19c9eda0: sandbox func_8007526C --disable all with candidate.c applied gives score 1 / build_insns 90 / target_insns 91. The dispatch brief's CHASSIS CHECK said 'measurement unavailable' and shipped an EMPTY ledger digest ('session 1', 'floor history: (none yet)', 'live frontier: (empty)') while the SessionStart queue banner still quotes the pre-migration pin of 48. All three are stale; the real ledger is memory/grind/func_8007526C/state.json with five floor_history entries (48 -> 13 -> 13 -> 13 -> 1).

- [s6] canonical func_8007526C returns verdict C, asm_insns 0, distance 1 -- the function is correctly routed to pure C and is not a canonical-asm candidate, closing the CANONICAL-ASM GRANT PATH branch.

- [s6] maspsx_label_nop_funcs.txt holds 21 entries and line 22 is func_80022F34 -- the controlling precedent named by the s6 Judge ruling (docs/grind/decisions.md:25229), applied by the OWNER in the 2026-09-06 foreclosed-bucket review (commit d4338774, [infra-rule: maspsx-label-nop]), after which that function landed. func_8007526C is absent. The sanctioned route for this exact residual is therefore demonstrated, already executed once, and owner-only by construction: the file is named verbatim on the add-scope-allow denylist at .claude/rules/integration-handoff-self-serve.md:56-58.

- [s6] DISPOSITION IS BLOCKED ON MODALITY, NOT ON EVIDENCE. This session's mandated modality is `recon`. tools/grinder/grindlib.py:812 mechanically refuses an owner-gated outcome whose escalation_ref carries a foreclosure title (RESOLVED BY STANDING RULING / LADDER EXHAUSTED) unless modality == 'escalation'. The only other line in docs/grind/decisions.md that names this function alongside OWNER-ESCALATION is the 2026-09-07 INTEGRATION HANDOFF at docs/grind/decisions.md:25107, and citing it would route straight back to the Judge (tools/grinder/grind.ps1:1428) in violation of the standing judge constraint in state.json ('Do not re-file an integration handoff for func_8007526C') -- that body was already FAILed twice, at docs/grind/decisions.md:25223 and :25227. So this session returns `progress` with the kills banked, exactly as the ladder design requires, and the correct next step is for the DRIVER to assign `escalation` modality; the honest floor of 1 is <= ENDGAME_LOCK_MAX_FLOOR, so that session files the RESOLVED BY STANDING RULING (2026-07-27) foreclosure record.

- [s6] No new C axis exists for the residual word. The target fixes the pre-loop insn order (asm/funcs/func_8007526C.s:2-6), so every byte-matching build necessarily places the gp-relative load immediately before the .L80075278 merge label whose first insn consumes $a0. The suppressed delay nop is a consequence of that required order, not of any C-level choice.

## s7 (recon, 2026-09-07) — chassis re-measured on HEAD 52ce9db3

### OBJECT MODEL: D_800A36A0 — MATCHES (measured; unchanged since s1/s2)
`func_8007526C` touches exactly one global, `D_800A36A0`, declared `extern u8 *D_800A36A0;`
in src/text1b.c (lines 6624/6659/6791) and listed in sdata_syms.txt:226 as a GP-relative
pointer variable. With the banked body applied, the built function reproduces the target's
`lw $a0, %gp_rel(D_800A36A0)($gp)` preamble and every absolute field offset
(0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) — 90 of 91 words word-identical. The declaration
needs no fix; this is a MEASURED verdict (score 1), not an inspection. No second global,
no aggregate/struct question, no MISMATCH and no MISMATCH-unmeasured symbol.

### Chassis check (the recon deliverable)
The dispatch brief carried no floor ("measurement unavailable", ledger floor "(none)") and
the queue banner still says 48, so this session re-measured from scratch rather than
quoting the ledger. `memory/grind/func_8007526C/candidate.c` applied at src/text1b.c:6660,
build configuration completely unmodified:

    "score": 1, "target_insns": 91, "build_insns": 90, "scorable": true
    (artifact: tmp/grind/func_8007526C/s7/sandbox_floor1.json)

The tree was restored from tmp/grind/func_8007526C/s7/text1b.c.bak immediately afterwards;
`git status --porcelain` shows only metrics/events.jsonl. So the honest floor on THIS
chassis is **1**, confirming s5 and s6 and retiring the stale 48 (queue banner,
migration_pin.json) and 13 (floor_history s1-s4), which belong to the pointer-bump and
index-cursor spellings respectively, not to the banked goto-spelled body.

### Frontier status: no C-side axis remains open
The single residual word is the load-delay `nop` at asm/funcs/func_8007526C.s:6, between
`lw $a0, %gp_rel(D_800A36A0)($gp)` and the loop-top merge label `.L80075278` whose first
insn is `lbu $v1, 0x10($a0)`. It is emitted by the assembler layer (maspsx `is_label()`
only matches `$L`, this cc1 fork emits `.L` — tools/maspsx/maspsx/__init__.py:257,
.claude/rules/maspsx-label-nop-gate.md), so no C spelling can produce it; the Judge
affirmed exactly this on 2026-09-07 12:10 after reading the asm and the s6 artifacts
itself. The s3 `loop.c scan_loop insn_count >= 123` axis is moot under the goto spelling
(GCC 2.7.2 emits NOTE_INSN_LOOP_BEG only for while/for/do, so loop.c never runs on this
loop). Disposition re-filed this session as an OWNER-ESCALATION record in
docs/grind/decisions.md (2026-09-07, "OWNER-ONLY GATE-LINE REMEDY"), superseding the
earlier `-msoft-float` handoff, which is moot: the current body needs no flag change at
all, only the one owner-only `maspsx_label_nop_funcs.txt` line (precedent func_80022F34,
commit d4338774, list line maspsx_label_nop_funcs.txt:22).

- [s7] OBJECT MODEL: D_800A36A0 — MATCHES (measured, score 1 with 90/91 words byte-exact). func_8007526C touches exactly one global, D_800A36A0, already declared `extern u8 *D_800A36A0;` in src/text1b.c (lines 6624, 6659, 6791) and listed in sdata_syms.txt:226 as a GP-relative pointer variable. With the banked body the build reproduces the target's `lw $a0, %gp_rel(D_800A36A0)($gp)` preamble and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C). The declaration needs no fix. There is no second global, no aggregate/struct question, and no MISMATCH or MISMATCH-unmeasured symbol for this function.

- [s7] Chassis re-measured this session because the dispatch brief carried no floor ("measurement unavailable", ledger floor "(none)") and the queue banner still says 48: with candidate.c applied the sandbox reports score 1 / build_insns 90 / target_insns 91 (tmp/grind/func_8007526C/s7/sandbox_floor1.json). The 48 (queue banner, migration_pin.json) belongs to the pointer-bump spelling and the 13 (floor_history s1-s4) to the index-cursor do/while spelling; neither describes the banked body.

- [s7] src/text1b.c was restored from tmp/grind/func_8007526C/s7/text1b.c.bak immediately after the measurement; `git status --porcelain` shows only metrics/events.jsonl, so the tree is clean for the driver's scope check.

- [s7] The s3 axis (drive loop.c scan_loop insn_count to >= 123 so move_movables stops hoisting the four switch constants) is moot under the banked spelling: a goto-formed loop gets no NOTE_INSN_LOOP_BEG, so loop.c never scans it. This is why the floor fell from 13 to 1 without ever reaching that boundary.

- [s7] Disposition re-filed this session at docs/grind/decisions.md:25231 as `## 2026-09-07 — func_8007526C — OWNER-ESCALATION: OWNER-ONLY GATE-LINE REMEDY (honest floor 1; one line in maspsx_label_nop_funcs.txt)`. It supersedes the earlier 2026-09-07 -msoft-float handoff, which is moot: the current body needs no Makefile or compiler-flag change of any kind. The record requests nothing be granted to the pipeline — maspsx_label_nop_funcs.txt is denylisted at .claude/rules/integration-handoff-self-serve.md:56-58 and the Judge affirmed that denylist on 2026-09-07 12:10.

- [s7] Precedent for the remedy: func_80022F34 sat in the identical shape, was foreclosed, and landed only after the owner applied the one gate line in the 2026-09-06 foreclosed-bucket review (commit d4338774, `[infra-rule: maspsx-label-nop]`); the line lives at maspsx_label_nop_funcs.txt:22.

---

## [s1 2026-09-07, modality recon] — chassis re-measure + the moved_once/insn_count-doubling lever

CHASSIS RE-MEASURE (HEAD 73e786dd, after the 121e34d7 Match was reverted by 73e786dd).
`src/text1b.c` carries `INCLUDE_ASM("asm/funcs", func_8007526C);` again. With the legal
(non-banned) index-cursor do-while body applied — now `memory/grind/func_8007526C/candidate.c` —
`sandbox func_8007526C --disable all` prints **score 13, build_insns 93, target_insns 91**.
That is the honest floor a session may actually spend: the floor-1 number in floor_history
belongs to the goto-spelled body, which layer-2 FAILED on 2026-09-07 and which is now
`state.json` banned_constructs[0] (moved to `rejected/goto-loop-banned-layer2-fail-score1.c`).

OBJECT MODEL: `D_800A36A0` — **MATCHES**. It is the only global func_8007526C touches. It is a
gp-relative POINTER (`lw $a0, %gp_rel(D_800A36A0)($gp)`, asm/funcs/func_8007526C.s:4), declared
`extern u8 *D_800A36A0;` at src/text1b.c:6624/6659/6662/6791 and used with byte-offset casts by
every one of the 10 sibling consumers in the same TU (e.g. src/text1b.c:6737
`tbl = (s16 *)(arg2 * 2 + (s32)D_800A36A0)`, :6887 `*(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C)`).
The census row (docs/naming/residual_named.csv:12) describes exactly the shape the target
compiles to: "2 instances, +2 stride; switch on state byte field 0x10 (0-4)". Every field offset
the function touches (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) is a multiple of 4 while the
per-iteration stride is 2, i.e. the underlying storage is a 16-bit table indexed
`[row][instance]` — which the file-wide `u8 *` + byte-offset idiom already expresses. No
declaration change is available or indicated; no signal is flagged. Not re-measured as a
separate build because there is no alternative declaration to measure: the existing decl already
produces the target's addressing (all 90 emitted words of the banned body are word-identical).

THE RESIDUAL IS ONE loop.c DECISION, AND THE ARITHMETIC IS NOW FULLY PINNED.
`pwsh tools/grinder/dump.ps1 func_8007526C` → tmp/grind/func_8007526C/dumps/text1b.loop, at
`;; Function func_8007526C`, with the candidate body applied:

    Loop from 14 to 263: 92 real insns.
    Insn 229: regno 126 (life 1), move-insn savings 1  moved to 271    <- switch constant
    Insn 235: regno 128 (life 1), move-insn savings 1  moved to 273    <- switch constant
    Insn 241: regno 129 (life 1), move-insn savings 1  moved to 275    <- switch constant
    Insn 244: regno 130 (life 1), move-insn savings 1  moved to 277    <- switch constant
    Insn  76: regno  92 (life 3), move-insn savings 2  moved to 279    <- the 0xC8
    Insn 113: regno 101 (life 2), done move-insn matches 76

move_movables moves a movable iff `(threshold * savings * m->lifetime) >= insn_count`
(tools/gcc-2.7.2/loop.c:1631), with `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`
(loop.c:532). This hard-float cc1 has n_non_fixed_regs == 60 → threshold == 122, and
122 * 1 * 1 = 122 >= 92 moves each switch constant. The target keeps all four inside the loop
(asm/funcs/func_8007526C.s:8/12/18/20) and hoists only the 0xC8 (line 3, `addiu $a3,$zero,0xC8`),
which is what savings 2 / life 3 gets you at any insn_count below 732. Everything in the 13-point
residual is that single predicate; s3's independently measured boundary (insn_count >= 123) is
exactly `insn_count > threshold` and is hereby re-derived from the source, not just observed.

NEW LEVER, MEASURED: the `moved_once` insn_count DOUBLING (loop.c:1609-1611).

    if (moved_once[regno]) { insn_count *= 2; ... "halved since already moved" }

`moved_once` is per-FUNCTION (alloca'd once in loop_optimize, loop.c:344) and is set at
loop.c:1912 whenever a movable is actually moved. The doubling mutates move_movables' LOCAL
insn_count for every movable considered AFTER it in the same loop's movable list. So one
already-moved movable, scanned FIRST, permanently doubles the desirability denominator for the
rest of the loop — which is precisely the lever that leaves savings-1/lifetime-1 constants behind.

Probe (rejected/inner-arming-loop-moved-once-doubling-score8.c): give the 0xC8 a named local
`lim`, put `lim = 0xC8;` as the first statement of the loop body, and wrap that statement in a
REAL inner loop `for (k = 0; k < 2; k++) { lim = 0xC8; }`. Measured, same chassis:

    Loop from 20 to 43: 4 real insns.                            <- arming loop, scanned FIRST
    Insn 31: regno 76 (life 69), global move-insn savings 1  moved to 298
    Loop from 14 to 285: 95 real insns.
    Insn 298: regno 76 (life 72), move-insn savings 1 halved since already moved  moved to 308
    Insn 251: regno 126 (life 1), move-insn savings 1 NOT DESIRABLE
    Insn 257: regno 128 (life 1), move-insn savings 1 NOT DESIRABLE
    Insn 263: regno 129 (life 1), move-insn savings 1 NOT DESIRABLE
    Insn 266: regno 130 (life 1), move-insn savings 1 NOT DESIRABLE

    sandbox score 13 -> 8, build_insns 93 -> 96.

That is the target's exact movable shape (0xC8 hoisted to the pre-header, all four switch
constants left in the dispatch) reached with NO build-flag change, on the current chassis. The
loop is processed innermost/last-first (loop_optimize iterates loop numbers downward), so the
arming loop runs before the real loop and arms moved_once[lim] in time. The only remaining
problem is COST: the arming loop itself emits 5 final insns (init k, the empty body's counter
add, the compare and the branch, plus a scheduling nop) that the target does not have, and it is
a dead loop with no semantic purpose — a coercion construct in no sanctioned family. It is banked
as a mechanism proof, not as a submission.

WHY do-while(0) CANNOT ARM IT (class result). The obvious zero-cost arming wrapper —
`do { lim = 0xC8; } while (0);` — cannot work, and not for a heuristic reason: scan_loop rejects
a zero-trip loop outright before it ever collects movables. Measured: `Loop from 18 to 32 is
phony.` The predicate is tools/gcc-2.7.2/loop.c:570 (`if (INSN_UID (scan_start) >= max_uid_for_loop
|| GET_CODE (scan_start) != CODE_LABEL) { ... "is phony" ... return; }`), and the source comment
20 lines above it names the construct verbatim: "This case can happen for things like
do {..} while (0)." No movable is ever moved out of such a loop, so moved_once is never set by
one. Measured score with the do-while(0) wrapper: 13 (unchanged), loop insn_count 91.

CONFIGURATION CONTEXT (unchanged from s5, restated because it is what the arithmetic means).
threshold == 122 is a property of the hard-float register file: 2 * (1 + 60). The period-correct
PsyQ compile had the 32 FP registers fixed (no FPU on the PS1), giving 2 * (1 + 28) == 58 < 92,
under which the plain candidate body needs no lever at all and is byte-exact. That is a
toolchain-configuration divergence of the same class as -mel, not a C-spelling question, and the
Judge has denylisted re-filing it (state.json judge_constraints[0]). The C-side work is therefore
to reproduce a threshold-58 outcome on a threshold-122 chassis, and the moved_once doubling is
the first mechanism found that does so.

- [s8] OBJECT MODEL: D_800A36A0 -- MATCHES. It is the only global the target touches: a gp-relative pointer (lw $a0, %gp_rel(D_800A36A0)($gp), asm/funcs/func_8007526C.s:4) declared `extern u8 *D_800A36A0;` at src/text1b.c:6624/6659/6662/6791 and consumed with byte-offset casts by all 10 sibling functions in the same TU (src/text1b.c:6737, :6887). The census row docs/naming/residual_named.csv:12 matches the compiled shape exactly (2 instances, +2 stride, switch on the state byte at field 0x10). Every touched field offset (0x8/0xC/0x10/0x14/0x18/0x38/0x3C) is a multiple of 4 against a 2-byte per-iteration stride, i.e. a 16-bit [row][instance] table, which the existing u8*-plus-byte-offset idiom already expresses; no alternative declaration exists to measure and no signal is flagged. Not measured as a separate build for that reason.

- [s8] CHASSIS: on HEAD 73e786dd (src/text1b.c back to INCLUDE_ASM after the 121e34d7 Match was reverted by 73e786dd) the legal index-cursor do-while body measures score 13, build_insns 93, target_insns 91. The floor-1 number in floor_history belongs to the goto-spelled body, which layer-2 FAILED and which is now state.json banned_constructs[0]; it has been moved out of candidate.c to rejected/goto-loop-banned-layer2-fail-score1.c, and candidate.c now holds the best LEGAL form (13).

- [s8] The entire 13-point residual is one predicate: move_movables moves a movable iff (threshold * savings * lifetime) >= insn_count (tools/gcc-2.7.2/loop.c:1631), threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) (loop.c:532). This hard-float cc1 has n_non_fixed_regs == 60 so threshold == 122, and 122*1*1 >= 92 hoists each of the four switch-comparison constants. The .loop dump names them: 'Loop from 14 to 263: 92 real insns', insns 229/235/241/244 (regno 126/128/129/130, life 1, savings 1) all moved, and insn 76 (regno 92, life 3, savings 2, the 0xC8) also moved. s3's independently measured boundary of insn_count >= 123 is exactly insn_count > threshold, now re-derived from the compiler source rather than only observed.

- [s8] moved_once is per-FUNCTION, not per-loop: it is allocated once in loop_optimize (tools/gcc-2.7.2/loop.c:344) and set at loop.c:1912 whenever a movable is actually moved, so a movable hoisted out of an earlier-processed loop permanently doubles move_movables' local insn_count for every movable scanned after it in a later loop (loop.c:1609-1611, dump text 'halved since already moved').

- [s8] Requirement (c) for the moved_once lever, derived from the inner-scan arithmetic and worth not re-deriving: the arming loop must NOT contain the switch dispatch. If it does, the inner scan (insn_count 95 < threshold 122) hoists the four constants into the INNER pre-header first, which places them clustered at the top of the outer loop body instead of interleaved through the dispatch as the target has them at asm/funcs/func_8007526C.s:8/12/18/20.

- [s8] COST measured, and it is the only thing standing between the moved_once lever and the target: a real arming loop emits 5 final insns (build_insns 96 vs target 91). loop.c empties its body by hoisting the invariant out, but nothing later deletes the emptied loop -- flow.c and jump2 both leave the counter init, the increment, the compare and the branch standing.

- [s8] Configuration context (restated, NOT a re-filing): threshold == 122 is 2*(1+60) for a hard-float register file. The period-correct PsyQ compile had the 32 FP registers fixed, giving 2*(1+28) == 58 < 92, under which the plain candidate body needs no lever at all and is byte-exact. Re-filing that as an integration handoff is denylisted by state.json judge_constraints[0]; it is recorded only to explain what the C-side lever is emulating.

## s1 (recon, 2026-09-07) -- chassis HEAD 3368f17b, floor RE-MEASURED at 13

### OBJECT MODEL: D_800A36A0 -- MATCHES (re-confirmed by measurement this session)
`func_8007526C` touches exactly ONE global, `D_800A36A0`, declared `extern u8 *D_800A36A0;`
at src/text1b.c:6624/:6659/:6791 and listed in sdata_syms.txt:226, i.e. a GP-relative
pointer variable, not an array and not an aggregate.  With
memory/grind/func_8007526C/candidate.c applied over the INCLUDE_ASM line the build
reproduces the target's `lw $a0, %gp_rel(D_800A36A0)($gp)` (asm/funcs/func_8007526C.s:4)
and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) -- measured
score 13, build_insns 93, target_insns 91.  This session additionally MEASURED the
alternative declaration the census row implies (docs/naming/residual_named.csv:12: two
instances, +2 stride, 16-bit fields): spelling the whole body through
`u16 (*t)[2] = (u16 (*)[2])D_800A36A0;` with `t[2][i] .. t[15][i]` measures score 13,
build_insns 93 -- byte-for-byte the same compile.  So there is no MISMATCH and no
MISMATCH-unmeasured symbol: the two candidate object models are indistinguishable in the
bytes, and the declaration needs no fix.

### Chassis re-measurement
On HEAD 3368f17b (src/text1b.c ships `INCLUDE_ASM("asm/funcs", func_8007526C);` after the
121e34d7 Match was reverted by 73e786dd), candidate.c applied verbatim measures
**score 13, build_insns 93, target_insns 91** (`wteng main sandbox func_8007526C
--disable all`).  The banked floor of 13 is chassis-accurate.  (The floor-1 number in
state.json's floor_history belongs to the goto-spelled body, which is
state.json banned_constructs[0].)

### H1 KILLED -- array-indexed addressing does NOT inflate the loop's scan_loop insn_count
The inherited frontier F2 needs the main loop to carry >= 123 RTL insns at loop.c time
while still collapsing to 91 final insns; the banked forms sit at 92 (candidate) and 95
(s2's global-reload spelling).  The obvious untried source of pre-combine RTL was the
data model itself: `t[row][i]` computes `base + row*4 + i*2`, which is not a valid MIPS
address, so at expand time each of the ~25 accesses should force a separate `addu` temp.
MEASURED: it does not survive to loop.c.  The .loop dump for the array-spelled body reads
`Loop from 14 to 543: 92 real insns.` -- IDENTICAL to the byte-offset spelling -- and the
sandbox score and build_insns are identical (13 / 93).  Mechanism: cse1 runs before
loop_optimize and collapses every `base + i*2` into one pseudo, leaving the constant row
offset inside the MEM as a `plus(reg, const)` address that costs no insn.  Banked as
rejected/array-model-insn-count-unchanged-score13.c.  Consequence for F2: the addressing
spelling is not a free variable for insn_count; the three measured points remain 92 / 95 /
95 against a requirement of >= 123.

### H2 CONFIRMED -- a SECOND route to the target's movable shape: `threshold -= 3` decay
Every previous session treated `insn_count` (F2) and `moved_once` doubling (s8's F1) as the
only two ways through the loop.c:1631 desirability test.  There is a third: `threshold` is
not a constant during move_movables -- it is decremented by 3 for every movable actually
moved (tools/gcc-2.7.2/loop.c:1904, and the same decrement on the move_insn path at
loop.c:1718).  Movables are considered in physical loop order, and statements written
BEFORE the `switch` in the source are scanned before the dispatch tree (expand_end_case
reorders the tree to the front of the switch, not the front of the loop body), so movables
placed at the top of the loop body decay threshold before the four switch-comparison
constants are ever tested.

MEASURED with rejected/threshold-decay-8-movables-score17.c (eight distinct non-zero
constants each stored to memory, so each use is register-required and cse cannot fold it):
    tmp/grind/func_8007526C/s1/decay8.loop.txt
      Loop from 14 to 303: 108 real insns.
      Insn 25/30/35/40/45  (regno 75-79,  life 1, savings 1)  moved to 311/313/315/317/319
      Insn 50/55/60        (regno 80-82)                      not desirable
      Insn 269/275/281/284 (regno 134/136/137/138)            NOT DESIRABLE
      Insn 116             (regno 100,    life 3, savings 2)  moved to 321
threshold decays 122 -> 119 -> 116 -> 113 -> 110 -> 107 across the first five moves, and
107 * 1 * 1 = 107 < insn_count 108 makes every later savings-1 / lifetime-1 movable fail.
All four switch-comparison constants stay inside the dispatch and the 0xC8 movable still
hoists to the pre-header (107 * 2 * 3 = 642 >= 108) -- the target's exact shape
(asm/funcs/func_8007526C.s:2-3 vs `addiu $v0,$zero,N` at lines 8/12/18/20), on the current
hard-float chassis, with no build-flag change and no arming loop.  sandbox score 17,
build_insns 106.

### The cost bound on H2, worked out (this is what the next session needs)
Let k be the number of extra movables placed ahead of the dispatch and c the number of RTL
insns each adds to the loop.  The first switch constant fails iff `122 - 3k < 92 + c*k`.
  c = 1 (bare `li`, use is an already-existing register-required slot) => k >= 8
  c = 2 (the measured probe: `li` + a store)                          => k >= 7 (the probe
        hit the boundary at the 6th movable because the 8 carriers had already raised
        insn_count to 108 before scanning began)
So the lever needs 7-8 extra MOVED movables.  A movable is only moved if `m->lifetime >= 1`
(loop.c:791), i.e. the constant must have at least one use that survives cse1 in a context
that REQUIRES a register.  On MIPS/GCC-2.7.2 those contexts are exactly: a store of a
non-zero constant to memory (`sh $reg`), and an eq/ne branch against a non-zero constant
(`beq`/`bne`, since the MIPS branch predicate is reg_or_0_operand).  Everything else --
`+ 0xA`, `slti`, shifts, array indices -- takes a 16-bit immediate, so cse1 folds the
constant into the insn, the holder's lifetime drops to 0, and the movable is never moved
(product 0 < insn_count).  This is why the target hoists exactly one movable: 0xC8 is the
function's ONLY non-zero constant stored to memory.
Consequence: func_8007526C's own semantics supply exactly ONE qualifying carrier, and that
carrier is the movable that must REMAIN hoisted.  Every additional carrier must invent a
store or a branch, and each of those emits at least one surviving instruction; a local
array carrier is worse still because the target has no stack frame at all (no `addiu $sp`),
so allocating one costs prologue/epilogue insns on top.  H2 is therefore a confirmed
mechanism with no zero-cost legitimate spelling identified in this function yet.

### Two source-read kills that close adjacent variants of the same lever
- `threshold -= 3` is per-MOVABLE, not per-consecutive-set.  Read at
  tools/gcc-2.7.2/loop.c:1904: the decrement sits AFTER the closing brace of
  `for (count = m->consec; count >= 0; count--)`, so a movable with N consecutive invariant
  sets still decays threshold by 3 once.  A single reused variable written 8 times at the
  top of the loop body cannot substitute for 8 separate carriers.
- Matched movables arm `already_moved`, NOT `moved_once`.  Read at loop.c:1968: when a
  movable m is moved, every m1 with `m1->match == m` gets `already_moved[m1->regno] = 1`
  and `m1->done = 1`, but `moved_once` is set only for m's own regno at loop.c:1912.  A
  `done` movable is skipped by the `if (!m->done ...)` guard at loop.c:1584 and never
  reaches the doubling site at loop.c:1609, so s8's moved_once doubling cannot be armed by
  giving two registers the same invariant value inside a single loop -- it still requires a
  separately-processed loop.  (This session's dump also shows the phenomenon live in the
  unmodified candidate: `Insn 232: regno 170 (life 2), done move-insn matches 139`.)

- [s9] OBJECT MODEL: D_800A36A0 - MATCHES (re-confirmed by measurement this session). It is the only global func_8007526C touches, declared `extern u8 *D_800A36A0;` at src/text1b.c:6624/:6659/:6791 and listed in sdata_syms.txt:226, loaded once as `lw $a0, %gp_rel(D_800A36A0)($gp)` at asm/funcs/func_8007526C.s:4. With candidate.c applied the build reproduces that gp-relative load and every absolute field offset (0x8, 0xC, 0x10, 0x14, 0x18, 0x38, 0x3C) at score 13. This session ALSO measured the alternative object model the census row implies (docs/naming/residual_named.csv:12 - two instances, +2 stride, 16-bit fields): the whole body spelled through `u16 (*t)[2] = (u16 (*)[2])D_800A36A0;` with t[2][i]..t[15][i] measures score 13, build_insns 93 and the identical loop insn_count 92, i.e. byte-for-byte the same compile. No MISMATCH and no MISMATCH-unmeasured symbol; no declaration fix is available or needed.

- [s9] CHASSIS: on HEAD 3368f17b (src/text1b.c back to INCLUDE_ASM after the 121e34d7 Match was reverted by 73e786dd) memory/grind/func_8007526C/candidate.c applied verbatim measures score 13, build_insns 93, target_insns 91. The banked floor is chassis-accurate; the floor-1 entry in state.json's floor_history belongs to the banned goto-spelled body.

- [s9] The loop.c:1631 desirability predicate has THREE free variables, not the two the inherited ledger records. Beyond insn_count (F2) and moved_once doubling (s8's F1), `threshold` itself decays by 3 for every movable actually moved (tools/gcc-2.7.2/loop.c:1904 on the consec path, :1718 on the move_insn path), and that decay alone reproduces the target's movable shape.

- [s9] MEASURED with 8 invented carriers (rejected/threshold-decay-8-movables-score17.c, dump excerpt tmp/grind/func_8007526C/s1/decay8.loop.txt): loop insn_count 108, threshold decays 122->119->116->113->110->107 over five moves, the sixth carrier and all four switch-comparison constants (insns 269/275/281/284, regno 134/136/137/138) print 'not desirable', and the 0xC8 movable (insn 116, life 3, savings 2) still prints 'moved to 321'. Sandbox score 17, build_insns 106 - the shape is the target's, the cost is 13 surviving instructions.

- [s9] COST BOUND on the decay lever: the first switch constant fails iff 122 - 3k < 92 + c*k, giving k >= 8 carriers when each adds c = 1 RTL insn and k >= 7 at c = 2. A carrier is only MOVED if m->lifetime >= 1 (loop.c:791), i.e. it needs a use that survives cse1 in a register-REQUIRED context. On MIPS/GCC-2.7.2 those contexts are exactly a store of a non-zero constant to memory (`sh $reg`) and an eq/ne branch against a non-zero constant (the MIPS branch predicate is reg_or_0_operand). `+ 0xA`, slti, shifts and array indices all take 16-bit immediates, so cse1 folds the holder and its lifetime drops to 0 and it is never moved. func_8007526C's semantics supply exactly ONE qualifying constant - 0xC8 - and that is the carrier which must REMAIN hoisted, which is also why the target's pre-header holds exactly one moved movable.

- [s9] A local-array carrier is strictly worse than a memory-store carrier here: asm/funcs/func_8007526C.s has no stack frame at all (no `addiu $sp` in the prologue or epilogue), so introducing any addressable local costs frame setup instructions on top of the carrier's own.

- [s9] Array-indexed addressing is NOT an insn_count lever (measured, not inferred): the `u16 (*t)[2]` / t[row][i] spelling produces 'Loop from 14 to 543: 92 real insns.' - the same count as the hand-written `p = base + i*2` byte-offset spelling - because cse1 runs before loop_optimize and collapses the common `base + i*2` subexpression, leaving the constant row offset inside the MEM as a free plus(reg, const) address. F2's measured points are now 92 (candidate), 92 (array model), 95 (s2 global-reload) against a requirement of >= 123.

- [s9] `threshold -= 3` is per-MOVABLE, not per-consecutive-set: at tools/gcc-2.7.2/loop.c:1904 the decrement sits after the closing brace of `for (count = m->consec; count >= 0; count--)`. One reused variable written N times invariantly at the top of the loop is ONE movable and ONE decrement, so k decrements genuinely require k distinct carrier registers.

- [s9] Matched movables arm already_moved but NOT moved_once: loop.c:1968 sets already_moved[m1->regno] and m1->done, while moved_once is assigned only for the moved movable's own regno at loop.c:1912, and a `done` movable is skipped by the guard at loop.c:1584 before ever reaching the doubling site at loop.c:1609. This is visible live in the unmodified candidate's dump ('Insn 232: regno 170 (life 2), done move-insn matches 139'), and it closes the hoped-for cheap arming route for s8's F1 - the doubling still needs a separately-processed loop.

- [s9] src/text1b.c was restored byte-for-byte to HEAD at the end of the session (`git status --porcelain` shows only the two ledger files, the two new rejected forms, and the pre-existing metrics/events.jsonl churn). candidate.c is unchanged and remains the best legal form at score 13.

- [s10] CHASSIS: the s1 candidate measures score 13 / build_insns 93 / target_insns 91 on HEAD 7a577773 with the current unmodified build configuration; loop insn_count 92. The ledger's floor of 13 is current.
- [s10] MOVABLE SCAN ORDER IS A C-LEVEL LEVER AND IT IS FREE. expand_end_case reorders the switch decision tree to the FRONT of the case statement, so in the s1 body the four switch-comparison constants are movable-list entries 1-4 (dump: insns 229/235/241/244, then insn 76 = the 0xC8, then 113) and NOTHING written inside the switch can decay `threshold` (tools/gcc-2.7.2/loop.c:1904) ahead of them. A statement written BEFORE the switch in the loop body is physically before the dispatch and is scanned first. Naming the 0xC8 as a local assigned there (`lim = 0xC8;`) costs zero: score 13, build_insns 93, loop insn_count 92 -> 91, and the dump becomes `Insn 19: regno 75 (life 63), move-insn savings 1  moved to 268` FIRST, then the four constants at threshold 119. Banked as memory/grind/func_8007526C/candidate.c; dump at tmp/grind/func_8007526C/s10/vB_limtop.loop.
- [s10] THE moved_once DOUBLING IS ARMABLE AND REPRODUCES THE TARGET SHAPE ON HARD FLOAT: score 13 -> 5. With the `lim` chassis, a real loop placed TEXTUALLY AFTER the main loop that also moves `lim` sets moved_once[75] before the main loop is scanned (loop_optimize walks loop numbers downward, so the later loop is processed first - dump prints `Loop from 265 to 288: 4 real insns.` before `Loop from 14 to 260: 91 real insns.`). The main loop then reads `Insn 19: regno 75 ... halved since already moved  moved` (insn_count 91 -> 182, loop.c:1609-1611) and insns 226/232/238/241 all print `not desirable` while the 0xC8 still hoists - exactly asm/funcs/func_8007526C.s:3 plus the in-dispatch `addiu $v0,$zero,N`. Measured cost ladder: `for (k=0;k<2;k++){lim=0xC8;}` score 6 / 94 insns; `while (i>0){lim=0xC8;i--;}` score 7 / 96; `do { lim = 0xC8; } while (--i);` score 5 / 93. 5 is the lowest score any non-goto body has reached. Artifacts tmp/grind/func_8007526C/s10/vC_arm_after.loop, vE_arm_dowhile_i.c.
- [s10] A ZERO-COST THRESHOLD-DECAY CARRIER DOES NOT SURVIVE TO loop.c (pass-attributed by dump, not inferred). The cheapest conceivable carrier is an invariant value whose only use is a dead register copy: `m0 = (s32)base + 0x101; d0 = m0;` with d0 never read. MEASURED: loop insn_count unchanged at 92, no movable for it in the dump, score unchanged at 13 - and the constant 257 appears in tmp/grind/func_8007526C/dumps/text1b.rtl and text1b.jump but is ABSENT from text1b.cse and text1b.loop, i.e. cse1 deletes the chain before loop_optimize runs. Any decay carrier therefore needs a live use and costs >= 2 surviving instructions; with k such carriers the requirement 122 - 3k < 92 + 2k gives k >= 7, i.e. >= 14 surviving instructions, strictly worse than the do-nothing floor of 13. s9's 8-carrier score of 17 is the empirical point on that line.
- [s10] Three inherited frontier probes KILLED by measurement: (a) a named holder for the 0xA ramp step used at all six sites - cse1 folds it into the addiu immediates, no movable, insn_count 92, score 13; (b) the eq/ne exit test `while (i != 2)` - no new movable because cse merges the exit-test 2 with the switch's case-2 constant (that pseudo becomes life 2 / savings 2 and is still moved), and the score gets worse, 15; (c) the `while (i < 2) { do { ...; i++; } while (i < 2); }` same-back-edge nest - cc1 collects ONE loop from it, not two (single `Loop from 24 to 273: 92 real insns.` in the dump), so it cannot arm moved_once, and the extra test costs 3 instructions for score 29.
- [s10] The loop KEYWORD is not a lever on this chassis: `for (i = 0; i < 2; i++)` with the same body is a byte-for-byte identical compile to the do-while (score 13, build_insns 93, loop insn_count 92, same five movables, all moved).

- [s10] Chassis re-measured: the s1 candidate is score 13 / build_insns 93 / target_insns 91 on HEAD 7a577773 with the unmodified build configuration; loop insn_count 92. The ledger floor of 13 is current.

- [s10] SCAN ORDER, the new fact: expand_end_case reorders the switch decision tree to the FRONT of the case statement, so in the s1 body the four switch-comparison constants are movable-list entries 1-4 and nothing written inside the switch can decay threshold ahead of them. A statement written BEFORE the switch is scanned first, and naming the 0xC8 there is free (score 13, build_insns 93, insn_count 92 -> 91, dump `Insn 19: regno 75 (life 63) ... moved to 268` first).

- [s10] moved_once doubling is armable on hard float and reproduces the target's movable shape: an arming loop placed textually AFTER the main loop is processed first (dump prints `Loop from 265 to 288: 4 real insns.` before `Loop from 14 to 260: 91 real insns.`), the main loop's first movable prints `halved since already moved`, insn_count 91 -> 182, and insns 226/232/238/241 print `not desirable` while the 0xC8 still hoists.

- [s10] Measured arming cost ladder on the new chassis: `for (k=0;k<2;k++){lim=0xC8;}` score 6 / build_insns 94; `while (i>0){lim=0xC8;i--;}` score 7 / 96; `do { lim = 0xC8; } while (--i);` score 5 / 93. Score 5 is the lowest any non-goto body has reached; the 2-instruction residual is the arming loop's decrement and back branch. The construct is a semantically dead loop and is banked ONLY as a mechanism proof (rejected/arming-loop-after-main-score5.c).

- [s10] A decay carrier with a dead use never reaches loop.c: the carrier constant 257 is in text1b.rtl and text1b.jump and absent from text1b.cse and text1b.loop, so cse1 deletes it; insn_count stays 92 and no movable appears. Live-use carriers cost at least 2 instructions each and the requirement 122 - 3k < 92 + 2k needs k >= 7, i.e. 14 or more surviving instructions -- worse than the floor of 13.

- [s10] Three inherited s9 frontier probes are now measured dead: the 0xA step holder (cse1 folds it, no movable, score 13), the eq/ne exit test (cse merges the constant with the case-2 comparison, score 15), and the same-back-edge nest (cc1 sees one loop, not two, score 29). The for-loop chassis is a byte-identical compile to the do-while (score 13).

## s11 (2026-09-07, rederive) — the loop.c predicate arithmetic, closed out in full

Chassis re-measured at dispatch: the s10 candidate body applied at src/text1b.c:6660 gives
`sandbox func_8007526C --disable all` -> **score 13, build_insns 93, target_insns 91**, and the
banked closest-form `rejected/arming-loop-after-main-score5.c` still measures **score 5,
build_insns 93** (mandated kill re-audit; both numbers unchanged from s10, so every s10 kill is
still chassis-valid).

### The 13-point residual, read off the cc1 asm rather than inferred
`tmp/grind/func_8007526C/dumps/text1b.s` (this session's dump) shows the entire divergence is the
prologue of the loop:

    ours:                          target (asm/funcs/func_8007526C.s):
      move  $6,$0                    addu   $a2,$zero,$zero
      li    $7,0xc8                  addiu  $a3,$zero,0xC8
      li    $11,2                    lw     $a0,%gp_rel(D_800A36A0)($gp)
      li    $10,1                  .L80075278:
      li    $9,3                      nop
      li    $8,4                      lbu    $v1,0x10($a0)
      lw    $4,D_800A36A0             addiu  $v0,$zero,2      <- in-loop, $v0 reused
    .L902:                            beq    $v1,$v0,...
      lbu   $3,16($4)                 slti   $v0,$v1,0x3
      beq   $3,$11,.L912              ...
      slt   $2,$3,3

Every other insn of the body is already word-identical in shape.  The target keeps all four
switch-comparison constants **inside** the loop, each in a dead-on-arrival `$v0`, two of them
filling branch delay slots.  We hoist all four into $8/$9/$10/$11, which is +4 live registers
across a 91-insn loop and is the whole 13-point score.

### `.loop` dump, this session (tmp/grind/func_8007526C/dumps/text1b.loop)

    Loop from 14 to 260: 91 real insns.
    Insn 19:  regno 75  (life 63), move-insn savings 1  moved to 268   <- lim (0xC8)
    Insn 226: regno 124 (life 1),  move-insn savings 1  moved to 270   <- the 2
    Insn 232: regno 126 (life 1),  move-insn savings 1  moved to 272   <- the 1
    Insn 238: regno 127 (life 1),  move-insn savings 1  moved to 274   <- the 3
    Insn 241: regno 128 (life 1),  move-insn savings 1  moved to 276   <- the 4

### The desirability arithmetic is now fully pinned (tools/gcc-2.7.2/loop.c)
`move_movables` moves a movable iff `already_moved[regno] || (threshold * savings * m->lifetime)
>= insn_count` (loop.c:1630-1632).  For each of the four constants `savings = n_times_used = 1`
(loop.c:790) and `m->lifetime = 1` (loop.c:791 — the `li` and its `beq` are adjacent, luid
distance 1); **both are already at their floor, so the left-hand side cannot be lowered from the
C side at all.**  That leaves exactly three numeric levers, and s11 measured or derived the bound
on each:

1. **`threshold`** = `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532) = 122 today.
   `loop_has_call` is set **only** by a `CALL_INSN` in the loop (loop.c:2199-2202).  A call inside
   this loop would halve threshold to 61, and 61*1*1 = 61 < 91 rejects all four constants while
   61*1*63 still hoists the 0xC8 — i.e. **a call in the loop is byte-exactly equivalent to the
   `-msoft-float` finding of s5**, reached through a different predicate.  The target has no call,
   so this is a dead end, but it is worth recording that the target's shape is reproduced by
   *either* `n_non_fixed_regs == 28` *or* `loop_has_call == 1`.
2. **`threshold -= 3` decay** (loop.c:1719 for move-insn movables, loop.c:1904 for the general
   case).  `lim` decays it to 119 before the constants are scanned, so the constants need
   `insn_count >= 120`, or 11 further free movables scanned before insn 226 to bring threshold
   below 91.  s10 reached 8 (score 17).  s11 re-confirmed why no more exist: every remaining
   in-loop constant (0xA addend, +1 increment, the 0 stores, the 0xC8 compare bound) folds into
   an immediate field (`addiu`/`slti`/`$zero`), so cse1 deletes any holder for it —
   `v5_u8state_local.c` (a named `lim2 = 0xA` holder used at all six `+ 0xA` sites) measures
   score 13 / build_insns 93, i.e. exactly the baseline: no movable created, no decay.
3. **`insn_count *= 2` doubling** (loop.c:1609-1611) when `moved_once[regno]` is already set.
   `moved_once` is allocated and zeroed once per **function** (loop.c:344-345) and is set only at
   loop.c:1912, on the main move path; the `m1->match` merge path sets `already_moved` but not
   `moved_once` (loop.c:1968).  Loops are scanned **last-first** (loop.c:435:
   `for (i = max_loop_num-1; i >= 0; i--)`, "so inner ones are done before outer ones"), and
   `loop_number_loop_starts` is in order of loop *beginnings*.  Consequences, both now proven
   from the source rather than guessed: a loop placed textually **before** the main loop can never
   arm it, and an **inner** loop arms its **outer** loop.  Those are the only two shapes, and both
   are already banked (arming-loop-after-main score 5, inner-arming-loop score 8).

### Why the `insn_count >= 120` axis has no headroom
`insn_count` is the count of `'i'`-class insns between the loop notes at loop time
(`count_loop_regs_set`, loop.c:2989-3007), i.e. **after cse1/jump1 and before cse2, combine, flow
and jump2**.  To exceed 119 we need +29 insns present at loop time that are all gone by the final
output.  The only post-loop pass that deletes whole insn groups here is jump2 cross-jumping, and
it needs identical duplicated tails; the target's arms already share the maximum
(`.L800753B8` merges the case-2/case-4 zeroing tails, `.L800753C0` merges the arm exits) and its
case-1 and case-3 tails are deliberately **not** merged.  Every s11 variant confirmed the 1:1
coupling instead: `s16 i` adds 6 loop-time insns and 6 emitted insns (99 vs 93); the u16-element
array model adds 1 and 1.  No ordinary-C statement measured this session raised loop-time
insn_count without raising `build_insns` by the same amount.

### Sibling transplant (mandated) — func_80075670, src/text1b.c:6666, matched on main
Its spellings were transplanted and **both fail**: the `((s16 *)(work + i * 2))[K/2]` element-array
addressing (score 47/60) and the `s16 i` counter declaration (score 28).  What the sibling *does*
supply is a live worked example of the very mechanism this function needs — its own `.loop` dump
section reads `Loop from 103 to 405: 87 real insns. / Insn 552: regno 120 ... halved since
already moved  moved to 554 / Insn 273: regno 144 (life 1), move-insn savings 1 not desirable`.
That is a life-1 movable rejected at threshold 119 against a doubled insn_count of 174, produced
by its genuine inner `for (i = 0; i < 2; i++)` loops.  func_8007526C has no semantic work after
its main loop and no semantic inner loop, which is precisely why the same mechanism can only be
reached here by a dead loop.

- [s11] Chassis re-measured at dispatch: the s10 candidate body applied at src/text1b.c:6660 gives score 13, build_insns 93, target_insns 91 -- matching the ledger's recorded floor of 13, so the chassis has not moved since s10.

- [s11] MANDATED KILL RE-AUDIT: the closest banked form, rejected/arming-loop-after-main-score5.c, still measures score 5 / build_insns 93 on HEAD 7ab27738. Neither it nor the candidate contains a FAKE construct, so tools/fake_ablate.py has no carrier to strip; every s9/s10 instance kill remains chassis-valid.

- [s11] The whole 13-point residual is read off this session's cc1 asm dump rather than inferred: our prologue emits li $7,0xc8 / li $11,2 / li $10,1 / li $9,3 / li $8,4 before the loop, where the target emits only addiu $a3,$zero,0xC8 and keeps the four comparison constants inside the loop in a reused $v0 (two of them filling branch delay slots). Every other insn of the body is already word-identical in shape.

- [s11] This session's .loop dump confirms the movable list exactly: Loop from 14 to 260: 91 real insns. / Insn 19: regno 75 (life 63) move-insn savings 1 moved to 268 / Insn 226,232,238,241: regno 124,126,127,128 (life 1) move-insn savings 1 moved to 270,272,274,276.

- [s11] Both factors on the left of move_movables' desirability test are already at their floor for the four constants: savings = n_times_used = 1 (loop.c:790) and m->lifetime = 1 (loop.c:791 -- the li and its beq are adjacent, so the luid distance is 1). The left-hand side of (threshold * savings * m->lifetime) >= insn_count at loop.c:1631 cannot be lowered from the C side at all, which leaves exactly three numeric levers: threshold's initial value, the threshold -= 3 decay, and the insn_count *= 2 doubling. s11 bounded all three.

- [s11] NEW: loop_has_call == 1 is byte-exactly equivalent to s5's -msoft-float finding, reached through a different predicate. threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) at loop.c:532; a call in the loop gives threshold 61, and 61*1*1 = 61 < insn_count 91 rejects all four switch constants while 61*1*63 still hoists the 0xC8 -- the target's exact movable shape. loop_has_call is set only by a CALL_INSN in the loop (loop.c:2202) and the target contains no call, so it is not a usable lever; it is recorded because the target's shape now has two independent toolchain-level explanations.

- [s11] moved_once is allocated and zeroed once per FUNCTION (loop.c:344-345), not per loop, and is assigned only at loop.c:1912 on the main move path; the m1->match merge path sets already_moved but not moved_once (loop.c:1968). Combined with the last-first scan order at loop.c:435, the doubling has exactly two reachable geometries -- a sibling loop after the main loop, or a loop nested inside it -- and both are already banked at scores 5 and 8.

- [s11] SIBLING TRANSPLANT (mandated; func_80075670, src/text1b.c:6666, COMPLETED-C on main): both of its spellings fail here -- the ((s16 *)(work + i * 2))[K/2] element-array addressing (score 47 switch form / 60 if-chain form) and the s16 i counter (score 28). What it does supply is a live worked example of the exact mechanism func_8007526C needs: its own .loop section reads Loop from 103 to 405: 87 real insns / Insn 552: regno 120 ... halved since already moved / Insn 273: regno 144 (life 1), move-insn savings 1 not desirable -- a life-1 movable rejected at threshold 119 against a doubled insn_count of 174, produced by its genuine inner for (i = 0; i < 2; i++) loops. func_8007526C has no semantic work after or inside its main loop, which is precisely why the same mechanism is only reachable here through a dead loop.

- [s11] src/text1b.c was restored to its committed INCLUDE_ASM state at the end of the session; git diff is clean for src/. The candidate saved to memory/grind/func_8007526C/candidate.c is the unchanged s10 body (best non-banned form, score 13) with an s11 re-measurement header.

## s12 (2026-09-07, structural) — evidence

Chassis re-measured on HEAD f2842664: candidate.c = **score 13, build_insns 93, target_insns 91**;
`rejected/arming-loop-after-main-score5.c` = **score 5, build_insns 93**. Unchanged from s10/s11.
Full score table for the session: `tmp/grind/func_8007526C/s12/scores.txt`.

### The three s11 frontier items are all now measured

1. **Two-pass state split (frontier #1) — measured, 63/99.** The split is genuinely admissible C
   (order-equivalent to the interleaved pass), and it *does* arm the `moved_once` doubling: the
   .loop dump prints `Insn 19: regno 75 ... halved since already moved  moved` for the textually
   earlier pass. It loses anyway because splitting the loop halves `insn_count`: pass A counts 37
   real insns, doubled 74, against threshold 119, so `119 * 1 * 1 >= 74` at loop.c:1631 keeps every
   comparison constant desirable. **The doubling only beats the threshold while `insn_count` stays
   above about 60** (2*insn_count > 119), and any split of the main loop that supplies the second
   loop drops it below that. This is the durable arithmetic: an arming loop must be a second loop of
   *additional* work sitting alongside an *undivided* 91-insn main loop.

2. **Arming without a second loop (frontier #2) — closed by source read.** `moved_once` is written
   at exactly one site (loop.c:1912) inside `move_movables`, which has exactly one call site
   (loop.c:966) inside `scan_loop`. Both the `m->partial && m->match` branch (loop.c:1641) and the
   STRICT_LOW_PART partial-movable construction (loop.c:838-862) sit inside that same move block,
   downstream of that same call. No path reaches line 1912 without a `scan_loop` invocation, i.e.
   without a second `NOTE_INSN_LOOP_BEG`.

3. **Case-label order (frontier #3) — measured, no headroom.** 1/3/2/4 (the candidate) 13; 1/2/3/4
   31; 2/4/1/3 55; 4/3/2/1 60. The candidate's existing order is the optimum of the permutation
   space. (4/3/2/1 hits the target's exact 91-word count but reverses block layout — coincidence.)

### The threshold-decay route is priced out by the target's pre-header

Decay alone needs `122 - 3k < 91 + k`, i.e. `k >= 8` moved movables — which s1/s9 measured working
(all four constants `not desirable` at 8 movables). But each moved movable emits its hoisted insn
into the pre-header, and the **target's entire pre-header is three instructions**
(asm/funcs/func_8007526C.s:2-4: `addu $a2,$zero,$zero`, `addiu $a3,$zero,0xC8`, `lw $a0,%gp_rel(D_800A36A0)($gp)`),
of which only the 0xC8 is a movable. Zero spare slots. The one apparently-free extra movable — the
gp-relative base load written inside the loop, which the target already has in its pre-header — is
**not loop-invariant at all** (the loop stores through `p`, `invariant_p` rejects the MEM); it
measured 16/95, the load simply re-emitted in the loop.

### NEW: the loop-time / emitted coupling is NOT 1:1 (s11's kill relaxed)

s11 banked an instance kill stating that every ordinary-C statement raises loop-time `insn_count`
and emitted `build_insns` by the same amount, which would close the Judge-named
"`insn_count >= 120` while still collapsing to 91" axis. **That coupling breaks under
duplicated-statement-into-arms.** GCC 2.7.2 runs the first jump pass with cross-jumping OFF and only
`jump2` with it ON, so duplicated arm tails survive to loop time and are counted by
`count_loop_regs_set`, then get re-merged afterwards. Measured on the real loop-advance statement
`i++` duplicated into all five switch arms (four cases + default):

    loop-time insn_count   91 -> 96   (+5; ".loop: Loop from 14 to 275: 96 real insns.")
    emitted   build_insns  93 -> 95   (+2; jump2 re-merged 3 of the 5 copies to identical bytes)

Exchange rate on this shape: **5 loop-time insns per 2 surviving emitted words**. Reaching
`insn_count` 120 needs +29 loop-time insns, which at this rate costs ~+12 emitted words against a
91-word target — so this particular duplication is not the answer, but the axis is demonstrably
open. A winning form needs a duplication whose copies merge **completely** (0 surviving words), or a
duplication with a much better rate; `i++` is only 1 insn per copy, so it merges poorly (the merge
has to also absorb the arms' differing branch structure).

- [s12] Mandated kill re-audit on HEAD f2842664: candidate.c still score 13 / build_insns 93 / target_insns 91; rejected/arming-loop-after-main-score5.c still score 5 / build_insns 93. Neither body contains a FAKE construct, so tools/fake_ablate.py has no carrier to remove and every banked instance kill remains chassis-valid.

- [s12] The moved_once doubling has a floor on loop size: it only beats the decayed threshold while the loop's insn_count exceeds about 60 (2 * insn_count > 119). Measured on the two-pass split, where pass A's 37 insns doubled to 74 and all four comparison constants hoisted regardless.

- [s12] moved_once is written at exactly one site (tools/gcc-2.7.2/loop.c:1912), reached only through the single move_movables call at loop.c:966 inside scan_loop, so arming it always costs a second NOTE_INSN_LOOP_BEG. The m->partial / STRICT_LOW_PART construction at loop.c:838-862 is downstream of the same call and offers no shortcut.

- [s12] The threshold-decay route is priced out by the target's pre-header budget: decay alone needs k >= 8 moved movables (122 - 3k < 91 + k), each of which emits its hoisted insn into the pre-header, but the target's whole pre-header is three instructions (asm/funcs/func_8007526C.s:2-4) of which only the 0xC8 slot is a movable -- zero spare slots.

- [s12] The one apparently-free extra movable, the gp-relative D_800A36A0 load that the target already keeps in its pre-header, is not loop-invariant when written inside the loop: the loop stores through p, invariant_p rejects the MEM, and the load is simply re-emitted (16/95).

- [s12] Switch case-label source order is a real codegen lever with no headroom left: 1/3/2/4 = 13, 1/2/3/4 = 31, 2/4/1/3 = 55, 4/3/2/1 = 60. The candidate's order is the optimum.

- [s12] n_non_fixed_regs (the other half of threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs), loop.c:532) is a compilation-wide constant set once in regclass.c:380-387; the only code path that lowers it is globalize_reg at regclass.c:530, reached only from a file-scope global register variable -- a register-asm pin, i.e. a banned construct. It is not a C-side lever.

## s13 (2026-09-07, structural) — evidence

Chassis re-measured on HEAD 34eb8142: `memory/grind/func_8007526C/candidate.c` applied at
src/text1b.c:6660 measures **score 13, build_insns 93, target_insns 91**, `.loop`
`Loop from 14 to 260: 91 real insns`, lim = regno 75 moved, the four switch comparison
constants = regnos 124/126/127/128 (life 1, savings 1) all moved to the pre-header.
Unchanged from s10/s11/s12. Full 18-row measured table: `tmp/grind/func_8007526C/s13/scores.txt`.

### 1. The threshold DECAY STAIRCASE is now measured exactly (new)

`move_movables` decays `threshold` by 3 per moved movable (tools/gcc-2.7.2/loop.c:1904) and
tests `(threshold * savings * m->lifetime) >= insn_count` (loop.c:1631) with savings = 1 and
lifetime = 1 for every one of the four constants. Starting threshold is 122
(= 2 * (1 + n_non_fixed_regs), loop.c:532, hard float). `lim` is always scanned first (s10) and
always moves, taking threshold to 119. Each comparison constant that then moves decays it
further. Measured staircase, confirmed by four independent variants this session:

    insn_count <= 113   -> all four constants hoist            (x1 = 106, base = 91)
    114 .. 116          -> the LAST TWO are "not desirable"    (w8 = 114, x2 = 114)
    117 .. 119          -> the last three are "not desirable"  (interpolated, not spent)
    insn_count >= 120   -> ALL FOUR are "not desirable"        (w9 = 124, w7 = 130, w6 = 141)

The Judge-named axis is exactly right and its number is **120**, not "about 120".

### 2. FIRST ORDINARY-C FORM THAT REJECTS ALL FOUR CONSTANTS (the session headline)

Until now the only construct that stopped the hoist was a *dead* arming loop after the main
loop (s10, `rejected/arming-loop-after-main-score5.c`, the moved_once doubling). s13 reached
the same end state with **no dead code at all**, purely by duplicating a real statement into
the switch-arm exit paths — the `duplicated-statement-into-arms` shape:

  `rejected/dup-tails-base-3insn-insncount124-score27.c` (variant w9): the real loop tail
  `i++; p = base + i * 2;` duplicated into all ten switch-arm exit paths (four cases, their
  if-taken paths, case 3's nested-if path, and an explicit `default:`), with nothing left after
  the switch. `.loop` reads `Loop from 17 to 362: 124 real insns` and **all four constants
  print `not desirable`**, with only `lim` moved. **score 27, build_insns 102.**

It is dead as a candidate only because the duplication does not fully cross-jump back: it buys
34 loop-time insns for 11 extra emitted words against a 91-word target.

### 3. The measured exchange rate of duplication (loop-time insns bought per emitted word)

| variant | copies x tail insns | loop insn_count | build_insns | rate |
|---|---|---|---|---|
| w2 ptr chassis, `i++; p += 2;` at the 5 OUTER arm exits only | 5 x 2 | 90 -> 99 | 94 -> 94 | **+9 / +0 (perfect merge)** |
| w3 ptr chassis, `i++; p = base + i*2;` at the 5 outer exits | 5 x 3 | 90 -> 104 | 94 -> 96 | +14 / +2 |
| x1 base chassis, `i++` at all 10 exits | 10 x 1 | 91 -> 106 | 93 -> 98 | +15 / +5 |
| w8 ptr chassis, `i++; p += 2;` at all 10 exits | 10 x 2 | 90 -> 114 | 94 -> 99 | +24 / +5 |
| w9 base chassis, `i++; p = base + i*2;` at all 10 exits | 10 x 3 | 91 -> 124 | 93 -> 102 | +33 / +9 |
| w7 ptr, 11 exits + the 4-store blocks duplicated into nested arms | 11 x 2 + 8 | 90 -> 130 | 94 -> 117 | +40 / +23 |

The structural rule the table shows: **duplication placed at the five OUTER switch-arm exits
cross-jumps back completely (w2: +9 loop-time insns at zero emitted cost); duplication placed
inside the arms' if-bodies does not**, because jump2's `cross_jump` cannot merge tails whose
predecessors have differing branch structure. The free budget is therefore about **+9**
loop-time insns, against the **+29** needed to clear 120 from the base chassis's 91.
Duplicating the 4-store blocks themselves is the worst shape measured and should not be retried.

### 4. Structural levers killed outright this session

- **Local declaration order is inert.** All five permutations of `base / p / i / lim`
  (lim,i,p,base | i,base,p,lim | p,base,lim,i | lim,base,p,i | base,lim,p,i) measure
  score 13 / build_insns 93 / insn_count 91. Declaration order renumbers the pseudos (regno 75
  becomes 72 / 73 / 74) but changes neither the movable scan order nor local-alloc's
  $8/$9/$10/$11 assignment. s12's frontier item 3 is closed.
- **Per-access address arithmetic is not free loop-time insn_count.** Writing every access as
  `*(u16 *)(base + i * 2 + K)` with no `p` local
  (`rejected/no-p-local-address-per-access-cse1-collapses-score13.c`) still measures
  `Loop from 14 to 541: 91 real insns`, score 13, build 93 — cse1 runs before loop in the
  toplev pass order (jump1, cse1, loop) and collapses the repeated address computations before
  `count_loop_regs_set` ever sees them.
- **Loop spelling is inert.** `for (i = 0; i < 2; i++)`, `while (i < 2)` and the candidate's
  `do { } while (i < 2)` all measure 91 / 93 / score 13, with identical movable lists.
- **The explicit pointer-IV chassis costs.** Writing `p += 2` instead of letting strength
  reduction derive the pointer from `p = base + i * 2` measures score 47 / build 94 /
  insn_count 90 (w1), consistent with the banked
  `rejected/pointer-increment-biases-iv-by-0x10.c`.

### 5. The arithmetic that frames every future session on this function

For the four constants NOT to hoist out of a loop whose loop-time insn_count equals the
target's own emitted 91 words, `threshold` must be <= 90, i.e.
`2 * (1 + n_non_fixed_regs) <= 90`, i.e. `n_non_fixed_regs <= 44`. Under the project's current
flags it is 60 (hard float). So any C form whose loop-time insn_count is the target's 91 will
hoist, and the only C-side escape is to inflate loop-time insn_count above the emitted count
by at least 29 words — priced this session at roughly one extra emitted word per three
loop-time insns beyond the first nine free ones. This is the honest statement of the residual
and it is consistent with the banked `-msoft-float` finding (threshold 58) being the original
build's actual mechanism.

### 6. Tooling note for future sessions (cost a full sweep this session)

PowerShell's `python3` is not on PATH on this host, so a sweep script that shells
`python3 apply.py` from PowerShell silently no-ops and every variant reports the BASELINE
score. Apply through `bash <script>` invoked from PowerShell — on this box PowerShell's `bash`
is WSL bash, while the Bash tool is Git Bash and has no `/mnt/c`. Always confirm a variant
actually landed in src/text1b.c before trusting a flat sweep.
The working harness is `tmp/grind/func_8007526C/s13/sweep2.ps1` (apply -> sandbox -> cc1 -da
dump -> extract the func's `.loop` lines), which reports score, build_insns and loop-time
insn_count in one pass; reuse it rather than rebuilding one.

- [s13] Chassis re-measured on HEAD 34eb8142: candidate.c applied at src/text1b.c:6660 gives score 13, build_insns 93, target_insns 91, and .loop 'Loop from 14 to 260: 91 real insns' with lim (regno 75) plus all four switch comparison constants (regnos 124/126/127/128, life 1, savings 1) moved to the pre-header. Unchanged from s10/s11/s12.

- [s13] For the four constants NOT to hoist out of a loop whose loop-time insn_count equals the target's own emitted 91 words, threshold must be <= 90, i.e. 2 * (1 + n_non_fixed_regs) <= 90, i.e. n_non_fixed_regs <= 44; under the project's current flags it is 60 (hard float). So the only C-side escape is to inflate loop-time insn_count above the emitted count by at least 29 words.

- [s13] The free (fully cross-jumped) duplication budget on this function is about +9 loop-time insns: 5 copies of a 2-insn real tail placed at the OUTER switch-arm exits measured 90 -> 99 loop insns with build_insns unchanged at 94.

- [s13] Every measured geometry that actually reaches insn_count >= 120 costs at least 11 emitted words above the 91-word target; the cheapest is w9 at score 27 / build_insns 102, and the most expensive (duplicating the 4-store blocks into the nested-if arms) is score 67 / build_insns 117.

- [s13] The decay staircase measured on two independent chassis: insn_count <= 113 hoists all four constants, 114..116 rejects the last two, >= 120 rejects all four.

- [s13] Local declaration order is codegen-inert here: five permutations of base/p/i/lim all measure score 13 / build 93 / insn_count 91 and differ only in pseudo numbering.

- [s13] cse1 runs before the loop pass, so per-access address arithmetic (base + i * 2 + K spelled at every access, no p local) does not raise loop-time insn_count at all -- it still measures 91.

- [s13] TOOLING: PowerShell's `python3` is not on PATH on this host, so a sweep that shells `python3 apply.py` from PowerShell silently no-ops and every variant reports the BASELINE score. Apply through `bash <script>` from PowerShell (which is WSL bash; the Bash tool is Git Bash and has no /mnt/c), and verify the variant landed in src/text1b.c before trusting a flat sweep. The working harness is tmp/grind/func_8007526C/s13/sweep2.ps1.

- [s13] src/text1b.c was restored to its committed INCLUDE_ASM state at the end of the session; `git status --porcelain src/` is clean.

## s14 (2026-09-07, synthesis) — evidence

Chassis re-measured at dispatch: `memory/grind/func_8007526C/candidate.c` applied over the
`INCLUDE_ASM("asm/funcs", func_8007526C);` line at src/text1b.c:6660 measures
**score 13, build_insns 93, target_insns 91**, `.loop` `Loop from 14 to 260: 91 real insns`,
`lim` = regno 75 moved to the pre-header and the four switch comparison constants
(regnos 124/126/127/128, life 1, savings 1) all `moved to`. Unchanged from s10–s13.

MANDATED KILL RE-AUDIT: the closest-to-target banked form,
`rejected/arming-loop-after-main-score5.c`, still measures **score 5, build_insns 93** on the
current chassis, with its dump reading `Loop from 263 to 281: 3 real insns / Insn 268: regno 75
(life 116), global move-insn savings 1 moved to 289 / Loop from 14 to 260: 91 real insns /
Insn 19: regno 75 (life 120), global move-insn savings 1 halved since already moved moved to 291 /
Insn 226,232,238,241 ... not desirable`. Neither that body nor the candidate carries a FAKE
construct, so `tools/fake_ablate.py` has no carrier to strip; every banked instance kill from
s9–s13 remains chassis-valid.

Full measured table this session: `tmp/grind/func_8007526C/s14/scores.txt`; harness
`tmp/grind/func_8007526C/s14/sweep.ps1` (git-checkout -> apply -> sandbox -> cc1 `-da` dump ->
extract the function's `.loop` lines), a hardened rewrite of s13's `sweep2.ps1` that also works
from the committed `INCLUDE_ASM` state.

### 1. NEW CLASS KILL — `do { ... } while (0);` cannot arm the moved_once doubling (loop.c:570)

The most attractive unexplored shape on the arming axis was a `do { ... } while (0);` wrapper
placed INSIDE the main loop body. It is an owner-sanctioned family (2026-07-06 ruling, ANY
codegen effect including register allocation), and unlike s10's dead trailing loop it emits
nothing — so if it produced a scannable inner loop it would have been the zero-cost version of
the score-5 mechanism proof.

It does emit a `NOTE_INSN_LOOP_BEG`, and loop.c does find the loop — but `scan_loop` returns
before doing any work:

    tmp/grind/func_8007526C/dumps/text1b.loop (v1 variant):
      Loop from 18 to 32 is phony.
      Loop from 14 to 274: 91 real insns.
      Insn 23:  regno 75  (life 65), move-insn savings 1  moved to 282
      Insn 240/246/252/255: regno 124/126/127/128 (life 1), savings 1  moved to 284/286/288/290

The predicate is `tools/gcc-2.7.2/loop.c:568-575`: scan_loop prints "is phony" and `return`s
unless `scan_start` (the first non-note insn after `NOTE_INSN_LOOP_BEG`, or the label targeted
by a `while`-style entry jump) is a `CODE_LABEL`. A `do { } while (0)` has no back edge, so its
top label is unreferenced and jump1 — which runs before loop — deletes it. `move_movables` is
therefore never called for that region, `moved_once` (written at exactly one site,
loop.c:1912) is never set, and the main loop's movable list is bit-identical to the baseline.

Measured at three placements, all **score 13 / build_insns 93 / insn_count 91**: wrapping
`lim = 0xC8;` alone (v1), wrapping `lim = 0xC8; p = base + i * 2;` (v2), and wrapping
`p = base + i * 2;` alone (v3). Banked as
`rejected/dowhile0-inner-arming-phony-loopc570-score13.c`. This independently re-derives and
extends s8's finding (which measured the do-while(0) shape only as a trailing sibling loop) to
the nested placement, and it closes the sanctioned-family route to a zero-cost arming.

### 2. FRONTIER ITEM 1 KILLED — the perfect cross-jump merge belongs to the POINTER chassis, not to the tail's content

s13 measured `w2` (pointer chassis, `i++; p += 2;` duplicated into the five OUTER switch-arm
exits) at +9 loop-time insns for +0 emitted words, and hypothesised that the 2-word leak in `w3`
came from `p = base + i * 2` re-associating differently per arm — i.e. that a tail built only
from same-register increments, with no address recomputation, would keep merging perfectly and
could be lengthened to buy the +29 loop-time insns needed to clear insn_count 120 for free.

Measured on the chassis that actually scores 13 (`p = base + i * 2` recomputed at the loop top):

| variant | tail duplicated into the 5 outer exits | loop insn_count | build_insns | score |
|---|---|---|---|---|
| base (candidate) | — | 91 | 93 | 13 |
| t1 | tail moved to the loop bottom, single copy | 91 | 93 | 13 |
| a1 | `i++;` (1 insn, NO address recomputation) | 96 (+5) | 95 (+2) | 16 |
| t2 | `i++; p = base + i * 2;` (3 insns) | 108 (+17) | 97 (+4) | 32 |

**Even a bare one-insn `i++` tail leaks 2 emitted words at 5 outer-exit copies on this chassis.**
The premise is false: `w2`'s zero-cost merge is a property of the pointer-bump chassis (whose own
combine_givs bias already costs score 47, s3/s13), not of the duplicated tail's content. Banked as
`rejected/5outer-iplus-base-chassis-leaks-2-score16.c` and
`rejected/5outer-3insn-tail-base-chassis-score32.c`.

### 3. FRONTIER ITEM 2 KILLED — the duplication axis's emitted cost is QUANTIZED

s13 banked `w9` (10 copies of the 3-insn tail) at loop insn_count 124 / build_insns 102 / score 27
and hypothesised that trimming to the minimum copy count that still clears 120 would return
roughly one emitted word per removed non-merging copy.

Removing exactly one copy (the case-3 inner-if copy) gives
`rejected/w9-minus1-copy-insncount120-build102-score27.c`:

    Loop from 17 to 350: 120 real insns.
    Insn 22: regno 75 (life 70), move-insn savings 1  moved to 358
    Insn 319/325/331/334: regno 140/142/143/144 (life 1), savings 1  not desirable
    score 27, build_insns 102

`insn_count` fell 124 -> 120 (landing exactly on the measured threshold, with all four constants
still rejected) while **build_insns did not move at all**. The emitted cost of this duplication
geometry does not respond to copy count; removing a second copy would drop insn_count below 120
and re-hoist the constants. **102 — eleven words above the 91-word target — is the floor of the
insn_count axis as currently spelled**, and it is reached at 9 copies just as at 10.

### 4. The two surviving axes and their measured floors (the synthesis)

Everything measured across s1–s14 collapses to two mechanisms that both reach the target's exact
movable shape (`lim`/0xC8 hoisted, all four switch comparison constants left in the loop), and to
one number each:

| axis | mechanism | how it is reached | measured floor |
|---|---|---|---|
| **ARMING** | `insn_count *= 2` at loop.c:1609-1611 when `moved_once[regno]` is set | a second NON-PHONY loop that moves regno 75 and is scanned before the main loop (i.e. textually after it, or nested inside it) | **build_insns 93, score 5** — the residual is the arming loop's own 2 emitted instructions (its decrement and back branch) |
| **INSN_COUNT** | `(threshold * savings * lifetime) >= insn_count` at loop.c:1631 with threshold 119 after `lim`'s -3 decay | duplicate a real statement into the switch-arm exits so jump2 re-merges the copies after loop.c has counted them | **build_insns 102, score 27** at insn_count 120 |

The arming axis is 5 points from a byte match and the insn_count axis is 27, so the arming axis
is where the remaining value is — but its floor is structural: a loop cannot emit zero
instructions (s10), and the one construct that would have emitted zero (`do { } while (0)`) is
rejected as phony before scan_loop runs (§1 above). Any future zero-cost arming must come from a
second loop whose emitted instructions are ABSORBED — either merged with instructions the target
already contains, or filled into delay slots reorg would otherwise fill with nop. Note also that
a dead arming loop fails cheat-checklist T1/T2 on its own terms (s10 said so explicitly), so the
arming axis only becomes submittable if the second loop carries real semantics — and the target's
asm has nothing after the main loop but `jr $ra / nop`.

- [s14] Chassis re-measured at dispatch: candidate.c applied at src/text1b.c:6660 gives score 13, build_insns 93, target_insns 91, .loop 'Loop from 14 to 260: 91 real insns' with lim (regno 75) and the four switch comparison constants (regnos 124/126/127/128, life 1, savings 1) all moved to the pre-header. Unchanged from s10-s13.

- [s14] MANDATED KILL RE-AUDIT: rejected/arming-loop-after-main-score5.c still measures score 5 / build_insns 93 with all four constants 'not desirable'; neither it nor the candidate carries a FAKE construct, so tools/fake_ablate.py has no carrier to strip and every s9-s13 instance kill remains chassis-valid.

- [s14] CLASS KILL: `do { ... } while (0);` cannot arm move_movables' moved_once doubling. It does emit a NOTE_INSN_LOOP_BEG, but scan_loop prints 'Loop from N to M is phony.' and returns at tools/gcc-2.7.2/loop.c:568-575 because scan_start is not a CODE_LABEL -- a do-while(0) has no back edge, so jump1 (which runs before loop) deletes its unreferenced top label. move_movables is never called for the region and moved_once (written only at loop.c:1912) is never set. Measured at three placements inside the main loop, all score 13 / build 93 / insn_count 91.

- [s14] FRONTIER 1 KILLED: the zero-cost cross-jump merge s13 measured for 5 outer-exit copies (w2, +9 loop insns / +0 emitted) is a property of the POINTER-BUMP chassis, not of the duplicated tail's content. On the candidate chassis a bare one-insn `i++` tail at the five outer exits measures insn_count 96 / build 95 (+5 / +2) and the 3-insn tail measures 108 / 97 (+17 / +4).

- [s14] FRONTIER 2 KILLED: the duplication axis's emitted cost is quantized and does not respond to copy count. w9 minus one copy lands loop insn_count on exactly 120 with all four constants 'not desirable' and build_insns UNCHANGED at 102. Removing a second copy would drop below 120 and re-hoist, so build_insns 102 (11 words over the 91-word target) is the floor of the insn_count axis as currently spelled.

- [s14] SYNTHESIS: every measurement s1-s14 collapses to two mechanisms that both reproduce the target's exact movable shape, with one measured floor each -- the moved_once ARMING axis at build_insns 93 / score 5 (residual = the arming loop's own 2 emitted instructions) and the insn_count >= 120 axis at build_insns 102 / score 27. The arming axis carries the remaining value; the open question is whether a second non-phony loop exists whose emitted instructions are absorbed rather than added.

- [s14] src/text1b.c was restored to its committed INCLUDE_ASM state at the end of the session; `git status --porcelain src/` is clean.

- [s14] Chassis re-measured at dispatch: candidate.c applied at src/text1b.c:6660 gives score 13, build_insns 93, target_insns 91, .loop 'Loop from 14 to 260: 91 real insns' with lim (regno 75) and the four switch comparison constants (regnos 124/126/127/128, life 1, savings 1) all moved to the pre-header -- unchanged from s10-s13.

- [s14] Kill re-audit: rejected/arming-loop-after-main-score5.c still measures score 5 / build_insns 93 with all four constants 'not desirable'; neither it nor candidate.c carries a FAKE construct, so fake_ablate has no carrier and all s9-s13 instance kills remain chassis-valid.

- [s14] A do-while(0) placed inside the main loop DOES emit a NOTE_INSN_LOOP_BEG and loop.c does find the region, but scan_loop prints 'Loop from 18 to 32 is phony.' and returns at tools/gcc-2.7.2/loop.c:568-575 because scan_start is not a CODE_LABEL: the wrapper has no back edge, so jump1 deletes its unreferenced top label before loop runs. move_movables is never called for it and moved_once (written only at loop.c:1912) is never set.

- [s14] Measured on the candidate chassis, five outer-arm-exit copies of a one-insn 'i++' tail give loop insn_count 96 and build_insns 95 (+5 loop-time for +2 emitted); the 3-insn tail gives 108 and 97 (+17 for +4). s13's zero-cost 5-copy merge (w2, +9/+0) is a property of the pointer-bump chassis, not of the tail's content.

- [s14] w9 minus one copy lands loop insn_count on exactly 120 -- the measured rejection threshold, all four constants 'not desirable' -- with build_insns UNCHANGED at 102. The duplication axis's emitted cost is quantized and does not respond to copy count, so 102 (11 words over the 91-word target) is this geometry's floor.

- [s14] SYNTHESIS: fourteen sessions of measurement reduce to two mechanisms that each reproduce the target's exact movable shape, with one number each -- the moved_once arming axis at build_insns 93 / score 5 (residual: the arming loop's own two emitted instructions) and the insn_count>=120 duplication axis at build_insns 102 / score 27. The arming axis is 22 points closer and is where the remaining value sits.

- [s14] Nobody has ever printed the word-by-word diff of the score-5 arming build against asm/funcs/func_8007526C.s; every session since s10 has assumed its two extra words are the arming loop's decrement and back branch. That assumption is now the ledger's single largest unverified premise.

- [s14] TOOLING: tmp/grind/func_8007526C/s14/sweep.ps1 is a hardened rewrite of s13's harness -- it git-checkouts src/text1b.c first and its apply.py handles BOTH the committed INCLUDE_ASM state and an already-applied body, so a sweep can no longer silently report baseline scores.

- [s14] src/text1b.c was restored to its committed INCLUDE_ASM state; git status --porcelain shows no src/ modification.

## s15 (2026-09-07, synthesis) -- the insn_count axis is ZERO-COST after all; floor 13 -> 1 with a real loop

### 1. Frontier item F1 answered: the score-5 arming body's residual is 1 nop + 3 arming words
Built `rejected/arming-loop-after-main-score5.c` with the exact production pipeline
(`tmp/grind/func_8007526C/s15/build.sh arm`, artifacts `arm.s`/`arm.o`/`arm.dis`) and ran a
difflib alignment of the masked word streams against `asm/funcs/func_8007526C.s`
(`tmp/grind/func_8007526C/s15/align.py`).  Result, exactly:
  * target[3] `nop` -- MISSING from the build.  This is the load-delay nop at
    `asm/funcs/func_8007526C.s:6`, between `lw $a0, %gp_rel(D_800A36A0)($gp)` and the
    `.L80075278:` label: the maspsx .L-label load-consumer blind spot.
  * build[88..90] `addiu $a2,$a2,-1 / bnez $a2,... / addiu $a2,$a2,-1` -- THREE extra words,
    not the two every session since s10 assumed (reorg duplicates the decrement into the
    back branch's delay slot).
  * target[90] `nop` (the `jr $ra` delay slot) vs build[92] `addiu $a2,$a2,1` -- the arming
    loop also changes what reorg puts in the epilogue delay slot.
  * Every other difference in the alignment is branch/jump label TEXT only (masked equal).
So the arming axis is 3 emitted words over target, not 2, and the ONLY non-arming residual
in that body was the maspsx nop.  The assumption in the s14 frontier is dead.

### 2. Mechanism census of loop.c:1631, completed by reading the compiler
`(threshold * savings * m->lifetime) >= insn_count` at tools/gcc-2.7.2/loop.c:1631 has exactly
four inputs, and s15 closed out the three that are not insn_count:
  * `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532).  A CALL in
    the loop HALVES it to 61 < 91, which alone would make all four constants not desirable --
    but `prescan_loop` (loop.c:2158-2211) sets `loop_has_call` only on a real `CALL_INSN`, and
    `asm/funcs/func_8007526C.s` contains zero `jal`/`jalr`, so any call payload adds words the
    target does not have.  `n_non_fixed_regs` is set once in regclass.c:380-387 from
    `fixed_regs[]` (only `-msoft-float`, which is denylisted, moves it) and decremented at
    regclass.c:530 only inside `globalize_reg` -- i.e. by global register variables, a
    forbidden register-pin family, and 16 of them would be needed.
  * `savings = m->savings = n_times_used[regno]` (loop.c:793) = 1; a constant with zero uses
    would not be emitted at all.
  * `m->lifetime = uid_luid[regno_last_uid] - uid_luid[regno_first_uid]` (loop.c:791) = 1;
    a set insn and its consumer are distinct 'i'-class insns so their luids always differ by
    at least 1 (loop.c:406-409).
  * The four constants cannot be kept out of the movable list either: the guard at
    loop.c:686-700 admits them through clause (2) (`! REG_USERVAR_P && ! REG_LOOP_TEST_P`),
    which is always true for compiler-generated comparison constants.
Therefore insn_count is the only C-side input, and the bar is insn_count >= 120 (not 123):
after `lim` is moved, `threshold -= 3` (loop.c:1904) leaves 119, so 119 < insn_count suffices.
Confirmed by measurement: insn_count 115 rejects only the last two constants (119 >= 115 and
116 >= 115 still pass), insn_count 121 and 123 reject all four.

### 3. THE FINDING: combine-folded compound-assignment splits buy insn_count for FREE
`count_loop_regs_set` (loop.c:2989-3007) counts every 'i'-class insn between the loop notes,
and it runs BEFORE cse2, combine, flow and jump2.  Every previous payload (s12-s14) relied on
jump2 cross-jumping duplicated tails and measured 3 loop insns per extra emitted word.  A
same-variable compound-assignment split chain is different: `combine` folds
`vN = vN + a; vN = vN + b;` back into a single `addiu`, so the chain is counted at loop time
and costs NOTHING at emission.  Measured ladder, all on the s10..s14 candidate chassis, pure C,
no FAKE construct, `sandbox --disable all` score / build_insns / loop insn_count:
    base (no split)                                      13 / 93 /  91
    one shared local `v`, all 6 sites, 2 addends         23 / 93 /  93
    one shared local `v`, all 6 sites, 5 addends         20 / 87 / 121   (all 4 not desirable)
    distinct local per site, all 6, 5 addends            11 / 90 / 121   (all 4 not desirable)
    distinct local, case 1 UN-split, 4 sites, 6 addends  20 / 91 / 115   (2 of 4 hoisted)
    distinct local, case 1 UN-split, 4 sites, 8 addends   1 / 90 / 123   (all 4 not desirable)
    distinct local, case 1 UN-split, 4 sites, 10 addends  1 / 90 / 131   (all 4 not desirable)
Each extra addend on a split site is worth +1 loop insn; introducing the local itself is worth
another +1 per site.  build_insns FALLS as the split deepens because the locals also remove
reloads, which is why the case-1 sites must stay un-split.

### 4. Floor 1 reached with a REAL do-while loop (no goto, no arming loop, no FAKE)
`tmp/grind/func_8007526C/s15/s2_k8.c` (now `memory/grind/func_8007526C/candidate.c`) measures
`"score": 1, build_insns 90, target_insns 91`.  The alignment
(`tmp/grind/func_8007526C/s15/align.py s2k8`, artifact `s2k8.dis`) shows the 90 emitted words
are word-identical to the target's 91 minus exactly one word: the load-delay `nop` at
`asm/funcs/func_8007526C.s:6` -- the same owner-authorized maspsx label-nop gate line that the
s5/s6/s7 packets identified.  This is the state the Judge asked for in the s14 constraints
block ("reach floor 1 with a real while/for/do loop"), reached without the banned goto loop.
The open question is admissibility of the split DEPTH, which is why s15 returns a
ruling-request rather than a submission.

### 5. Two secondary spellings measured and dead
  * `s16` locals instead of `s32` add only +2 loop insns across all six sites (93): the
    sign-extensions are folded into the lhu/sh before loop.c counts.  The insn_count gain
    comes from the split, not from the local's width.
  * Staging the four memory-to-memory `*(u16 *)(p+0x14) = *(u16 *)(p+0x18);` copies and the
    two `+ 1` increments through named locals -- ordinary C needing no split justification --
    buys only +6 loop insns for eight staged statements AND costs 2 emitted words, so it
    cannot replace split depth on the way to insn_count >= 120.
