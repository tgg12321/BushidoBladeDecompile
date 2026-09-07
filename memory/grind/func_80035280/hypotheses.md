# Hypothesis ledger — func_80035280

## s1 (2026-09-07, recon) — chassis: HEAD (INCLUDE_ASM). Floor reached: 56.

### H1 — DATA MODEL, D_80106A58 split aggregate. KILLED (instance).
Statement: with `extern s32 D_80106A58;` unchanged and the use-site
`base = (u8 *)&D_80106A58;`, loop 2 compiles to the target's single-induction
addressing (`lw 0x4($a2)` / `lbu 0x0($a2)` / `addiu $a2,$a2,8`), so no
aggregate-merge declaration change is needed for the loop-2 residual.
Probe: measured the loop spelled `s = base + i * 8` (build 109/108, score 63)
against the self-incrementing walker (build 111/108, score 56, two givs).
Result: the single-base addressing is reachable from the CURRENT declaration;
the extra instructions in the walker form come from giv formation, not from the
declaration. Corroborated by sibling func_8003C714's Judge PASS on 2026-09-05
with this declaration untouched, where `extern u8 D_80106A58[24]` is a banned
construct. Kill scope: instance (this chassis, this body, no FAKE constructs).

### H2 — DATA MODEL, D_80106A73 scalar declaration. CONFIRMED.
Statement: the scalar `extern u8 D_80106A73;` read through a plain pointer
deref produces a non-MEM_IN_STRUCT_P load that cse.c does not invalidate across
the `p[8]` stores, collapsing the target's 3 `lbu` + 3 `sw` flag block into
1 + 1 and leaving build_insns at 107 vs target 108.
Mechanism: expr.c:4567-4577 (MEM_IN_STRUCT_P only for PLUS_EXPR addresses /
&aggregate / aggregate type), cse.c:7565-7576 (the `p[8]` store can only set
`nonscalar`, never `all`, since its address is a PLUS in SImode), cse.c:1701
invalidate_memory (a `nonscalar` write drops only in_struct or varying-address
entries, and cse_rtx_addr_varies_p returns 0 for a MEM addressed by a REG with
a constant qty).
Probe: replaced `*f` with `src[3]` (`src = (u8 *)&D_80106A73 - 3`, a PLUS_EXPR
address). Measured build_insns 107 -> 111, and the flag block became the
target's 24-instruction shape. See evidence.md OBJECT MODEL.

### H3 — LICM of the /1800 magic. CONFIRMED as the sole remaining structural residual.
Statement: the build hoists the 0x91A2B3C5 constant into the loop-2 preheader
while the target materialises it in the loop, and that single decision accounts
for the whole remaining instruction-count delta.
Mechanism: loop.c:1631 with threshold 122 (loop.c:532, no call in the loop),
savings hard-coded 1 (loop.c:897) and lifetime 1 (loop.c:791) against
insn_count 62, measured in tmp/grind/func_80035280/dumps/code6cac_b.loop.
Probe: rejected/s1-diagnostic-real-call-in-loop2-adds-jal.c — a real call inside
loop 2 sets loop_has_call, drops threshold to 61, and 61 < 62 leaves the
constant in the loop. The emitted preheader and loop head then match the target
instruction-for-instruction; build 110 = 108 + the jal and its delay slot.

### H4 — route (A), relocate a real call into loop 2. KILLED (instance).
Statement: func_80035280 has no callee in or after loop 2 that could honestly be
placed inside the loop, so the fix that closed sibling func_8003C714 is not
available to this body.
Probe: read asm/funcs/func_80035280.s end-to-end — the only call is the leading
`jal func_80077D00` at 25A88, and the epilogue at 25C20 is
`lw ra / addiu sp / jr ra / nop` with nothing between it and the loop's back
edge. Adding a call therefore adds two instructions that the target does not
have (measured: 110 vs 108).
Result: relocating an existing call is impossible on this body; inventing a call
would be a dead call with no semantic purpose (checklist T1/T2) and is not
proposed. Kill scope: instance (this body/chassis; a future rederive that finds
a genuine callee for loop 2 re-opens it).
Measured on: HEAD chassis, candidate.c body, zero FAKE constructs present.

## Live frontier for session 2+

### F1 — loop.c:1609 `moved_once[regno]` doubling (CHEAPEST untried route).
`if (moved_once[regno]) insn_count *= 2;` runs BEFORE the loop.c:1631 test. With
insn_count 62 -> 124 and threshold*savings*lifetime pinned at 122, the test
`122 >= 124` FAILS and the constant stays in the loop — the exact target
behaviour, with no call and no padding. The margin is 2, so nothing else needs
to move. What it requires: pseudo 112 (the 0x91A2B3C5 holder) must already have
been moved once in this same loop, i.e. that pseudo must be the destination of
an earlier movable that move_movables processed and moved. Next probe: read
loop.c:1210-1340 (the `m->match` / `m0->lifetime += m->lifetime` merging paths)
and loop.c:780-900 (where movables are built) to establish under what C-visible
condition two movables share one regno; then look for a natural C spelling of
the three divisions that makes GCC reuse one pseudo for the /1800 magic — e.g.
a second `/1800` whose cse-merge leaves two sets of one pseudo. Confirm any
candidate by re-reading the .loop dump for a "halved since already moved" line.

### F2 — route (B) raw: push loop-2 insn_count from 62 to >= 123.
Needs +61 RTL insns inside the loop that emit no bytes (they must survive cse
and jump, which run BEFORE loop, and die in combine or later). This is the
combine-foldable chain-extender family, which owner ruling 2026-07-01 moved into
the sanctioned F1 family as a FAKE-annotated last resort — un-annotated or
byte-materialising instances remain FAIL. Sibling func_8003C714 reached
insn_count 123 by this route and measured sandbox 0, then rejected the forms as
inadmissible padding and closed on route (A) instead; both of its forms are in
memory/grind/func_8003C714/rejected/. Read those two files BEFORE re-deriving
anything here — a 61-insn chain is very likely to fail the human-programmer test
for the same reason it did there, so treat F2 as the fallback to F1, and expect
to need a ruling-request rather than a direct submission.

### F3 — register-naming residual (do NOT start here).
Score 63 at 109/108 is a register permutation, not a shape difference: the
diagnostic form with correct LICM scores 57 while carrying two extra call
instructions. Grind this only after F1 or F2 settles the preheader, then use
tools/ra_solver (`inverse_compose.py classify`) on the residual seats rather
than guessing spellings. Deltas to aim at, from this session's disassembly:
flag block build a1(acc)/v0(byte)/v1(mask)/a0(addr) vs target
v0(acc)/v1(byte)/a0(mask)/a1(addr); loop 2 build t0(base)/t2(magic30)/t3(magic1800)
vs target a2(base)/t1(magic30)/v0(magic1800); and the target does not bias the
dst base (it uses `0x21($a1)` with a1 = p, where a walker-spelled build biases
to `p + 0x24` and uses -3/-2/-1/0).

## Open declaration question for whoever spends H2
The honest spelling of the H2 fix is an aggregate covering
0x80106A70..0x80106A73, not the `src[3]` measurement pointer. But
include/code6cac.h:472 already declares `extern u8 D_80106A70[3];`, and the
matched sibling func_80034F88 (src/code6cac_b.c:3573+) both reads
`u8 *q = &D_80106A73;` as a scalar and carries an INTEGRATION HANDOFF note
asking for D_80106A70 to stay `[3]` with D_80106A73 left as its own scalar.
Any widening must be measured against func_80034F88's score before it is
proposed, and it is a header change, so it needs the integration-handoff path
(tools/grinder/scope_allow.txt), not a TU-local declaration — no-new-park-
categories.md prong (d) forbids the TU-local spelling.

## [s1] With the existing `extern s32 D_80106A58;` declaration and the use-site `base = (u8 *)&D_80106A58;`, loop 2 compiles to the target's single-induction addressing (lw 0x4($a2) / lbu 0x0($a2) / addiu $a2,$a2,8), so the flagged SPLIT-AGGREGATE declaration change buys nothing for this function's residual.
- mechanism: GCC 2.7.2 strength reduction (loop.c giv formation): the self-incrementing `base += 8` walker with a separate `*(s32 *)(base + 4)` read makes two givs and two addiu increments, while `s = base + i * 8` keeps one base register with a +4 displacement, which is exactly the target's shape.
- probe: Measured both spellings on the HEAD chassis: walker form build_insns 111 / target 108, sandbox score 56 (banked rejected/s1-walker-two-IV-floor56.c); `s = base + i*8` form build_insns 109 / target 108, sandbox score 63 (memory/grind/func_80035280/candidate.c). Disassembled the sandbox object both ways to confirm the giv count.
- result: Single-base addressing is reachable from the declaration already on main. Independently corroborated: sibling func_8003C714, whose loop 2 is line-for-line this loop, took a Judge PASS on 2026-09-05 with `extern s32 D_80106A58;` untouched, and the `extern u8 D_80106A58[24]` retype sits on that function's banned_constructs list.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (src/code6cac_b.c:3741 INCLUDE_ASM baseline, target_insns 108), candidate.c body, zero FAKE constructs present

## [s1] The scalar declaration `extern u8 D_80106A73;` read through a plain pointer deref makes the flag byte a non-MEM_IN_STRUCT_P load that survives the p[8] stores in cse, collapsing the target's three lbu plus three sw into one of each and leaving the build four instructions short.
- mechanism: expr.c:4567-4577 sets MEM_IN_STRUCT_P on an INDIRECT_REF only when the address tree is a PLUS_EXPR, an ADDR_EXPR of an aggregate, or the type is aggregate; cse.c:7565-7576 can therefore only mark the `p[8] = v` store `nonscalar` (never `all`, because the store address IS a PLUS and its mode is not QImode); and cse.c:1701 invalidate_memory drops only entries with in_struct set or a varying address, while cse_rtx_addr_varies_p returns 0 for a MEM addressed by a REG holding a known constant.
- probe: Compiled the plain-deref body (build_insns 107 vs target 108, flag block 20 insns vs the target's 24 at asm/funcs/func_80035280.s:25A90-25AEC), then respelled the same three reads as `src[3]` with `src = (u8 *)&D_80106A73 - 3` so the address tree is a PLUS_EXPR. Read the pass attribution out of the .cse dump slice (three lbu present in .jump, one in .cse) rather than guessing.
- result: build_insns went 107 -> 111 and the emitted flag block became the target's shape (lw / lbu / and / andi / or / sw, three times) modulo register naming and two scheduler slots. The declaration is under-specified; the honest form is an aggregate covering 0x80106A70..0x80106A73, which makes the read an ARRAY_REF and gets MEM_IN_STRUCT_P from expr.c:4888. That is a header change colliding with include/code6cac.h:472 and with matched sibling func_80034F88, so it needs measurement against that sibling first.
- verdict: CONFIRMED

## [s1] The build hoists the 0x91A2B3C5 (/1800) magic constant into the loop-2 preheader while the target materialises it inside the loop at 25B30/25B38, and that single LICM decision accounts for the entire remaining instruction-count delta.
- mechanism: loop.c:1631 moves a movable iff `already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count`; loop.c:532 sets threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122 with no call in the loop; loop.c:897 hard-codes savings = 1 for a move_insn constant load; loop.c:791 gives lifetime 1 for a set with an immediately following use. 122 * 1 * 1 = 122 >= insn_count 62, so it hoists.
- probe: Read the numbers out of tmp/grind/func_80035280/dumps/code6cac_b.loop (function slice): 'Loop from 109 to 255: 62 real insns.' / 'Insn 135: regno 112 (life 1), move-insn savings 1 moved to 273' (0x91A2B3C5) / 'Insn 153: regno 118 (life 35), move-insn savings 1 moved to 275' (0x88888889). Then flipped loop_has_call with a real call inside loop 2 and re-measured.
- result: With loop_has_call = 1 the threshold drops to 61 and 61 < 62, so 0x91A2B3C5 stays in the loop while 0x88888889 (lifetime 35) still hoists - exactly the target's split. The emitted preheader and loop head then match asm/funcs/func_80035280.s:25B18-25B54 instruction-for-instruction; build_insns 110 = 108 plus the jal and its delay slot, score 57. Subtract the call and the count is 108 == 108.
- verdict: CONFIRMED

## [s1] Relocating an existing call into loop 2 - the fix that closed sibling func_8003C714 on 2026-09-05 - has no honest spelling on this body, because func_80035280's only call is the leading jal func_80077D00 at 25A88 and nothing sits between the loop's back edge and the epilogue.
- mechanism: loop.c prescan_loop sets loop_has_call only from a real CALL_INSN inside the NOTE_INSN_LOOP_BEG..NOTE_INSN_LOOP_END range, and a real call emits a jal plus a delay slot, so a call that is not already in the function's byte stream adds two instructions to the count.
- probe: Read asm/funcs/func_80035280.s end to end: single call at 25A88, epilogue at 25C20 is lw ra / addiu sp / jr ra / nop with no intervening statement. Measured the diagnostic form with a synthesised func_800344B4() call in the loop's terminal arm: build_insns 110 vs target 108, score 57 (rejected/s1-diagnostic-real-call-in-loop2-adds-jal.c).
- result: The diagnostic proves the mechanism and disproves the transfer: the sibling's route needs a real callee this function does not have, and inventing one would be a dead call with no observable effect (cheat checklist T1 and T2), so it is not proposed as a candidate. The remaining routes are loop.c:1609 moved_once doubling (insn_count 62 -> 124 > 122) and raw insn_count >= 123.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), candidate.c body plus one synthesised call, zero FAKE constructs present

## s2 (2026-09-07, structural) — chassis: HEAD (INCLUDE_ASM, target 108). Floor reached: 39.

### H5 — inline-index loop-2 spelling. CONFIRMED (floor 63 -> 39).
Statement: spelling every loop-2 memory reference with its index inline rather
than through `u8 *s` / `u8 *dst` walker locals reproduces the target's register
assignment for the whole of loop 2 and for `p`, dropping the score from 63 to
39 with no change in shape.
Mechanism: with the indices inline, cse1 shares the two base computations
itself and strength reduction forms exactly the target's two givs; the s1
spelling introduced two extra user pseudos that changed the allocno ordering and
pushed `p` from $t0 to $t1 and the counter from $a3 to $t0.
Probe: sandbox on the HEAD chassis. s1 body 63 (build 109), this body 39
(build 109), byte-field-first reorder 67 (build 110), values-into-locals 93
(build 92). Disassembled the sandbox object and aligned it against
asm/funcs/func_80035280.s position by position.
Result: loop 2 is now register-identical to the target apart from the mfhi temp
($t3 vs $t2), which is displaced only because $t2 holds the wrongly-hoisted
0x91A2B3C5. New candidate.

### H6 — the loop.c:1631 desirability boundary. CONFIRMED by measurement.
Statement: the product threshold * savings * m->lifetime for the 0x91A2B3C5
movable is exactly 122, so the constant is left in the loop only when the loop's
RTL insn_count is at least 123.
Probe: padded the loop-2 body with K extra stores (gen_pad.py) and read the
.loop dump at insn_count 117, 120, 123, 135. Hoisted at 117 and 120; "not
desirable" at 123 and 135; 0x88888889 (life 35) hoisted at every count.
Result: s1's derivation from loop.c:532/791/897 is confirmed empirically. This
supersedes any future attempt to re-derive the number.

### H7 — loop.c:1609 moved_once doubling (the s1 frontier's F1). KILLED (class).
Statement: the loop.c:1609 doubling cannot make the 0x91A2B3C5 movable
undesirable at any insn_count at or below 122, because the only event that sets
moved_once for that pseudo — an earlier move of that same pseudo's set out of a
loop nested inside loop 2 — relocates the set to the inner preheader while its
use stays put and therefore raises m->lifetime to at least 2, so the loop.c:1631
product 122 * savings * lifetime is at least 2 * insn_count.
Mechanism: loop.c:791 computes m->lifetime as a function-wide luid span
(regno_last_uid minus regno_first_uid); loop.c:430-433 scans loops last-first,
so the only loop that can run before loop 2 and set moved_once for a pseudo
still set inside loop 2 is a loop nested within it; loop.c:1912 sets
moved_once[regno] only after that pseudo's set was moved.
Probe: rejected/s2-nested-loop-movedonce-inflates-lifetime.c puts the /1800
division inside an inner loop. The .loop dump shows the doubling firing and the
hoist happening anyway: "Insn 304: regno 114 (life 18), move-insn savings 1
halved since already moved  moved to 306" against "Loop from 106 to 281: 67 real
insns" (122 * 18 = 2196 >= 134).
Result: F1 collapses into F2; it buys nothing over the raw insn_count route and
should not be spent again.
kill_scope: class
predicate_cite: tools/gcc-2.7.2/loop.c:1631
measured_on: HEAD chassis (target_insns 108), nested-loop probe body, zero FAKE
constructs present.

### H8 — reaching insn_count >= 123 from a natural loop-2 spelling. KILLED (instance).
Statement: the four natural spellings of loop 2 measured this session and in s1
produce loop-2 insn_counts of 62, 62, 55 and 44 against the 123 that the
loop.c:1631 gate requires, so none of them comes within a factor of two of the
boundary.
Mechanism: cse1 runs before loop and already shares the two base computations
and the four store addresses, and MIPS reg+offset addressing means each byte
store is a single RTL insn, so the pre-LICM RTL count for this body tracks the
final instruction count almost one to one (62 pre-LICM against 61 emitted).
Probe: read "Loop from N to M: K real insns" out of the .loop dump for each
body (loopslice.py).
Result: raising insn_count by spelling is not a live route on this body; the
remaining +61 would have to come from RTL that survives cse1 and dies later,
which is the F2 padding family, not a spelling.
kill_scope: instance
measured_on: HEAD chassis (target_insns 108), four natural bodies, zero FAKE
constructs present.

### H9 — the residual is a compiler-configuration difference. CONFIRMED.
Statement: passing -msoft-float to cc1 (the PS1 R3000 has no FPU, and GCC's
CONDITIONAL_REGISTER_USAGE fixes the FP hard registers under that flag, halving
n_non_fixed_regs and therefore the loop.c:532 threshold) makes func_80035280
compile to 108 == 108 with the target's exact loop-2 shape, with no call and no
padding, and is codegen-neutral for 31 of the project's 32 C files.
Mechanism: threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs); with FP
registers fixed the threshold falls below the loop-2 insn_count of 62, so
0x91A2B3C5 (lifetime 1) is left in the loop while 0x88888889 (lifetime 35) still
hoists.
Probe: regenerated the .loop dump with the flag ("not desirable" for regno 112),
built the object through the real pipeline with the flag (build_insns 108,
engine score 55), and swept cc1 output for all 32 stems with and without the
flag (31 identical ignoring the flag-echo comment; only code6cac_b differs, in
func_800324D0 alone).
Result: CONFIRMED as the mechanism, and BLOCKED as an action. func_800324D0
scores 0 without the flag and 3 with it, and its natural un-FAKEd spelling
scores 27 under both, so adopting the flag would require re-grinding that
function. The flag lives in Makefile:35 and engine/buildconfig.py:43, both
surfaces a grind session may not touch, and the decision is governed by
.claude/rules/no-compiler-divergence.md. Recorded as evidence only.

## Live frontier for session 3+

### F1' — the toolchain-configuration question (highest value, NOT a grind axis).
Everything about func_80035280's structural residual is explained and measured:
with -msoft-float it is 108 == 108 with the target's exact loop-2 shape, and the
flag is codegen-neutral for 31/32 TUs. The single blocker is func_800324D0
(same TU), 0 -> 3. This is an operator/owner decision, not something a grind
session can act on. Exact steps if it is ever taken: add -msoft-float to
CC_FLAGS and CC_FLAGS_GP in Makefile:35-36 and engine/buildconfig.py:43-44,
re-grind func_800324D0 from its residual of 3 (its /* FAKE */ duplicated-tail
construct was tuned under the hard-float register set and will need
re-derivation), then verify-oracle. Do NOT spend grind sessions re-deriving this
finding; it is measured in evidence.md FACT 11.

### F2' — the register-naming residual under the CURRENT chassis.
The candidate is at 39 with build 109 / target 108. The +1 and the $t2/$t3 shift
are the LICM decision and are not reachable by spelling (H7, H8). What IS still
open is the flag block: the candidate emits (addr $a0, acc $a1, mask $v1, byte
$v0) where the target has (addr $a1, acc $v0, mask $a0, byte $v1), and four
positions in the second flag arm are ordered `lbu / and / andi / or / li` where
the target has `and / lbu / li / andi / or`. That is 4-6 points of the 39.
Next probe: tools/ra_solver inverse_compose.py classify on the flag-block seats,
using the s2 candidate as the base rather than the s1 body — do not guess
spellings. Note the target zeroes the loop-1 counter (`addu $a3,$zero,$zero`) as
the FIRST insn after the jal, before the D_80106A73 address is formed; the
candidate emits it fourth.

### F3' — the D_80106A73 declaration, still unresolved from s1.
The `src[3]` PLUS_EXPR spelling is a measurement device, not a declaration. The
honest form is an aggregate covering 0x80106A70..0x80106A73, which collides with
include/code6cac.h:472 (`extern u8 D_80106A70[3];`) and with matched sibling
func_80034F88's integration-handoff note. Unchanged from s1; must be measured
against func_80034F88's score before it is proposed, and it is a header change
needing the integration-handoff path.

## [s2] Spelling every loop-2 memory reference with its index inline (((u8 *)p)[i * 4 + 0x21], *(s32 *)(base + i * 8 + 4)) instead of introducing u8 *s / u8 *dst walker locals reproduces the target's register assignment for the whole of loop 2 and for p, dropping the sandbox score from 63 to 39 with no change in shape.
- mechanism: With the indices inline, cse1 shares the two base computations itself and loop.c strength reduction forms exactly the target's two givs; the s1 spelling introduced two extra user pseudos that changed the allocno ordering and pushed p from $t0 to $t1 and the loop counter from $a3 to $t0, permuting the whole loop-2 temp set.
- probe: sandbox func_80035280 --disable all on the HEAD chassis for four bodies, then position-by-position alignment of the disassembled sandbox object against asm/funcs/func_80035280.s: s1 body 63 (build 109), inline-index body 39 (build 109), byte-field-first reorder 67 (build 110), values-into-locals 93 (build 92).
- result: New floor 39, superseding the s1 floor of 56. Loop 2 is now register-identical to the target (dst $a1, record $a2, counter $a3, 0x88888889 $t1, same $v0/$v1/$a0 temp rotation) apart from the mfhi temp $t3 vs $t2, which is displaced only because $t2 holds the wrongly-hoisted 0x91A2B3C5. Banked as memory/grind/func_80035280/candidate.c.
- verdict: CONFIRMED

## [s2] The product threshold * savings * m->lifetime for the 0x91A2B3C5 movable is exactly 122, so the constant is left in the loop only when the loop-2 RTL insn_count is at least 123.
- mechanism: loop.c:1631 moves a movable iff already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count; loop.c:532 sets threshold from n_non_fixed_regs, savings is n_times_used[regno] which loop.c:597 bcopies from the count of SETS in the loop, and loop.c:791 gives lifetime 1 for a set with an immediately following use.
- probe: Padded the loop-2 body with K extra stores (tmp/grind/func_80035280/s2/gen_pad.py) and read the .loop dump at loop-2 insn_count 117, 120, 123 and 135. Hoisted (moved to ...) at 117 and 120; not desirable at 123 and 135; 0x88888889 (life 35) hoisted at every count.
- result: s1's derivation of 122 from the compiler source is confirmed empirically, so no future session needs to re-derive it. The gate flips between insn_count 120 and 123.
- verdict: CONFIRMED

## [s2] The loop.c:1609 insn_count doubling cannot make the 0x91A2B3C5 movable undesirable at any insn_count at or below 122, because the only event that sets moved_once for that pseudo relocates that pseudo's set to an enclosing preheader while its use stays put and therefore raises m->lifetime to at least 2, leaving the loop.c:1631 product at or above 2 * insn_count.
- mechanism: loop.c:791 computes m->lifetime as a function-wide luid span (regno_last_uid minus regno_first_uid); loop.c:430-433 scans loops last-first, so the only loop that can run before loop 2 and set moved_once for a pseudo still set inside loop 2 is a loop nested within it; loop.c:1912 sets moved_once[regno] only after that pseudo's set has been moved.
- probe: rejected/s2-nested-loop-movedonce-inflates-lifetime.c places the /1800 division inside an inner loop within loop 2. The .loop dump shows the doubling firing and the hoist happening anyway: 'Insn 304: regno 114 (life 18), move-insn savings 1 halved since already moved  moved to 306' against 'Loop from 106 to 281: 67 real insns' (122 * 1 * 18 = 2196 >= 134).
- result: The s1 frontier's cheapest untried route defeats itself: the very act that sets moved_once inflates the lifetime term far more than the doubling helps. F1 collapses into F2 and buys nothing over the raw insn_count route.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis (src/code6cac_b.c INCLUDE_ASM baseline, target_insns 108), nested-loop probe body, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/loop.c:1631

## [s2] The four natural spellings of loop 2 measured in s1 and s2 produce loop-2 RTL insn_counts of 62, 62, 55 and 44 against the 123 the loop.c:1631 gate requires, so none of them comes within a factor of two of the boundary.
- mechanism: cse1 runs before loop and already shares the two base computations and the four store addresses, and MIPS reg+offset addressing makes each byte store a single RTL insn, so the pre-LICM RTL count for this body tracks the final instruction count almost one to one (62 pre-LICM against 61 emitted).
- probe: Read 'Loop from N to M: K real insns' out of the .loop dump for each of the four bodies with tmp/grind/func_80035280/s2/loopslice.py: s1 walker-locals body 62, byte-field-first 62, inline-index candidate 55, values-into-locals 44.
- result: Raising insn_count by spelling is not a live route on this body; the missing 61 insns would have to come from RTL that survives cse1 and dies later, which is the F2 padding family rather than a spelling. Recorded so session 3 does not repeat the search.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), four natural bodies (s1 candidate, byte-first, inline-index, values-into-locals), zero FAKE constructs present

## [s2] Passing -msoft-float to cc1 makes func_80035280 compile to 108 == 108 with the target's exact loop-2 shape, with no call and no padding, and is codegen-neutral for 31 of the project's 32 C files; the single blocker is func_800324D0, which scores 0 without the flag and 3 with it.
- mechanism: The PS1 R3000 has no FPU. GCC 2.7.2's CONDITIONAL_REGISTER_USAGE fixes every FP hard register under -msoft-float, dropping n_non_fixed_regs by about 32 and therefore halving the loop.c:532 threshold (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs). The halved threshold falls below the loop-2 insn_count of 62, so 0x91A2B3C5 (lifetime 1) is left in the loop while 0x88888889 (lifetime 35) still hoists - exactly the target's split.
- probe: Three measurements: (1) regenerated the .loop dump with the flag, which prints 'Insn 135: regno 112 (life 1), move-insn savings 1 not desirable' and still hoists regno 118; (2) built code6cac_b.o through the real pipeline with the flag (tmp/grind/func_80035280/s2/build_o.sh), giving build_insns 108 == target 108 with the loop head lui v0,0x91a2 / lw v1,4(base) / ori v0,0xb3c5 / mult v1,v0 ... matching asm/funcs/func_80035280.s:25B18+, engine score 55; (3) swept cc1 output for all 32 src stems with and without the flag (tmp/grind/func_80035280/s2/sf3.py), ignoring only the cc1 flag-echo comment line: IDENTICAL 31 of 32, DIFFERS code6cac_b with 8 diff lines, all inside func_800324D0 (one li 0xff moving from $8 to $2 plus its bne).
- result: CONFIRMED as the mechanism behind the entire structural residual, and recorded as evidence only - not acted on and not proposed as a candidate. func_800324D0 (src/code6cac_b.c:2554) currently matches only through a /* FAKE */ duplicated-statement-into-arms construct tuned under the hard-float register set; its natural un-duplicated spelling was measured this session at score 27 under BOTH configurations, so the flag does not make that FAKE removable and adopting it would require re-grinding that function from a residual of 3. CC_FLAGS lives in Makefile:35 and engine/buildconfig.py:43, both surfaces a grind session may not touch, and the question is governed by .claude/rules/no-compiler-divergence.md.
- verdict: CONFIRMED
