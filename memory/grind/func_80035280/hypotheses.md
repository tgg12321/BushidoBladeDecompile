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

## s3 (2026-09-07, structural) — chassis: HEAD (INCLUDE_ASM, target 108). Floor reached: 16.

### H9 — the flag-block accumulator must have reg_n_deaths == 1. CONFIRMED (floor 39 -> 18).
Statement: the four-seat register permutation in the flag block is caused by the
accumulator pseudo failing local-alloc's eligibility test, and splitting the
accumulator into one local per merged bit makes every flag-block seat match the
target exactly.
Mechanism: local-alloc.c:470-477 sets `reg_qty[i] = -2` (eligible for local
allocation) only when `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`. A
single reused `v` is set four times, carries four REG_DEAD notes (.lreg:
"Register 77 used 10 times across 16 insns in block 0; dies in 4 places"), gets
reg_qty = -1, and is allocated by global-alloc AFTER local-alloc has already
given $v0/$v1 to the six short per-arm temps and $a0 to the address pointer.
Probe: BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 run of the instrumented cc1
(tools/gcc-2.7.2/cc1, NOT build/cc1) — block 0 has exactly seven quantities and
the accumulator is not one of them; greg prints `;; 77 conflicts: 72 74 76 77
2 3 4 29`. Then measured the split body.
Result: 39 -> 18, build 109 both ways, and build insns 10..28 become the
target's flag block instruction-for-instruction. Confirmed downstream: the
global list falls from 13 to 12 allocnos and no block-0 pseudo "dies in 4
places" any more.

### H10 — declaration order / statement order as a seat lever. KILLED (instance).
Statement: on the single-accumulator body, declaring `v` first among the locals
and hoisting `v = p[8];` above the `f`/`src` pointer setup each measured 39,
exactly the score of the unmodified s2 body, so neither moved any flag-block
register seat.
Mechanism: the seat assignment is decided by local-alloc's eligibility test
(local-alloc.c:472) on reg_n_deaths, which is a property of how many times the
variable is written, not of where it is declared or where its first assignment
appears; cse1 normalises the RTL order before local-alloc runs.
Probe: tmp/grind/func_80035280/s3/v/v1.c (v declared first) and v2.c (v = p[8]
hoisted), sandbox 39 / build 109 each, against the s2 body's 39 / 109 measured
first this session. Also measured the neighbouring form that drops the `f`
pointer (v3.c): 44 / build 111.
Result: order levers are inert here; the death count is the lever. Banked at
rejected/s3-single-accumulator-global-allocno-score39.c and
rejected/s3-no-f-pointer-adds-two-insns-score44.c.
kill_scope: instance
measured_on: HEAD chassis (target_insns 108), single-accumulator s2 body plus
two order variants, zero FAKE constructs present.

### H11 — a two-way alternating accumulator split. KILLED (instance).
Statement: splitting the flag accumulator into two alternating locals (fA/fB)
measured 28 against the three-way split's 18, because each of the two pseudos is
still written twice and so still fails local-alloc.c:472's `reg_n_deaths == 1`.
Probe: tmp/grind/func_80035280/s3/v/v5.c, sandbox 28 / build 109, banked at
rejected/s3-two-alternating-accumulators-score28.c.
Result: the split has to be one local per merged bit; a partial split only
partially lands on the target's seats.
kill_scope: instance
measured_on: HEAD chassis (target_insns 108), two-accumulator body, zero FAKE
constructs present.

### H12 — loop-1 counter-zeroing placement. CONFIRMED (18 -> 16).
Statement: writing the loop-1 counter initialisation as a statement immediately
after the `func_80077D00()` call, with loop 1 spelled `for (; i < 3; i++)`, puts
`addu $a3,$zero,$zero` where the target has it — the first instruction after the
jal's delay slot — and takes the last two points off the prologue.
Probe: four placements measured on the split body: after the call 16, before the
flag block 18, immediately before loop 1 18, and as a `s32 i = 0;` declaration
initialiser 29 at build 110 (that form is emitted before the call, so the value
must survive it and costs a real instruction — banked at
rejected/s3-i-declaration-initialiser-adds-insn-score29.c).
Result: build insns 1..38 are now identical to the target. ADMISSIBILITY is open
— see the note in candidate.c: this is a real initialisation of a real loop
counter, but it is moved eight statements earlier purely for a scheduling slot.
The split-only body without the hoist is 18 and is kept at
tmp/grind/func_80035280/s3/v/v4.c.

### H13 — -msoft-float closes THIS body. KILLED (instance).
Statement: the s3 candidate built through the real pipeline with -msoft-float
scores 16 at build 109, identical to the hard-float build, because the flag
lowers the loop.c:532 threshold only to about 58 while this body's loop-2 RTL
insn_count is 55.
Mechanism: -msoft-float fixes the 32 FP hard registers, so
`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` falls from 122 to about 58;
loop.c:1631 then still finds `58 * 1 * 1 >= 55` and hoists the 0x91A2B3C5
movable. The s1 walker body's insn_count of 62 clears 58, which is why s2
measured that body at 108 == 108 under the flag.
Probe: tmp/grind/func_80035280/s3/soft_v6.o, scored against build/src/code6cac_b.o
with engine score_func: {"score": 16, "target_insns": 108, "build_insns": 109};
the .loop slice for the hard-float build of the same body reads "Loop from 110
to 274: 55 real insns" with both constants "moved to".
Result: s2's FACT 11 has to be restated. The configuration question, if the
operator ever takes it, is "add the flag AND find a loop-2 spelling whose
insn_count lands in [59, 122] while keeping the inline-index giv shape", not
"add the flag". The s1 walker spelling clears 59 but costs two giv instructions
(build 111).
kill_scope: instance
measured_on: HEAD chassis (target_insns 108), s3 candidate body built with
-msoft-float through the real pipeline, zero FAKE constructs present.

## Live frontier for session 4+

### F4 — the loop.c:1631 LICM residual is the ENTIRE remaining 16 (unchanged, config-bound).
Build insns 1..38 match the target exactly; everything left is the wrongly
hoisted 0x91A2B3C5 (+2 preheader insns, the `nop` in the `lw 0x4($a2)` load-delay
slot, and the mfhi temp displaced from $t2 to $t3). The gate needs loop-2
insn_count >= 123 under the shipped configuration and the five spellings measured
across s1/s2/s3 give 44, 55, 55, 62, 62. Do NOT re-derive the gate (s2 FACT 8
measured the boundary at 120/123) and do NOT re-derive the -msoft-float story
(s3 FACT 19 supersedes s2 FACT 11). The only untried honest direction is a loop-2
spelling that is byte-equivalent to the current one but carries more pre-combine
RTL; s2's H8 measured four spellings in the 44-62 band and none is within a
factor of two of 123, so treat this as config-bound unless a rederive session
finds a different decomposition of the three clock fields.

### F5 — admissibility of the two s3 constructs (ask BEFORE submitting, do not respell).
The 16-point body carries (1) the split accumulators flags/flags0/flags1/flags2
and (2) the `i = 0;` hoist. Neither is dead and neither is a coercion: every
split local is written once and holds a value that IS stored to p[8] and read by
the next arm, and `i = 0;` is the loop's real initialisation. But flags0/flags1
are read TWICE, so this is not the once-written/once-read named-intermediate
shape, and the only reason not to reuse one `v` is codegen. Since the body is not
at distance 0 there is nothing to submit yet; when it is, the right move is a
`ruling-request` naming both constructs rather than a speculative
candidate-ready. The un-hoisted fallback is 18 (tmp/grind/func_80035280/s3/v/v4.c)
and the un-split fallback is 39
(rejected/s3-single-accumulator-global-allocno-score39.c).

### F6 — the reg_n_deaths lever is likely to transfer to other queue functions.
local-alloc.c:472's `reg_n_deaths[i] == 1` test is generic: ANY function whose
target holds an accumulator in a low register ($v0/$v1) across a straight-line
block, where our build puts it in an argument register, is a candidate for the
same one-local-per-value split. The diagnostic is one line of the .lreg dump
("dies in N places" for the pseudo in question) plus the ";; N regs to allocate"
list in .greg. Worth a cross-function sweep by a later session or by the
solver-modality driver; the mechanism is not specific to func_80035280.

## [s3] The flag-block four-seat register permutation is caused by local-alloc.c:472's eligibility test reg_n_deaths[i] == 1, and splitting the single reused accumulator into one local per merged bit puts every flag-block seat on the target's register.
- mechanism: local-alloc.c:470-477 sets reg_qty[i] = -2 (eligible for local allocation) only when reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1; a single reused `v` is set four times, carries four REG_DEAD notes, gets reg_qty = -1, and is left to global-alloc, which runs only after local-alloc has handed $v0/$v1 to the six short per-arm temps and $a0 to the address pointer.
- probe: BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 run of the instrumented cc1 (tools/gcc-2.7.2/cc1) shows exactly seven block-0 quantities with the accumulator absent, and the greg slice prints ";; 77 conflicts: 72 74 76 77 2 3 4 29"; then measured the three-way split body with sandbox.
- result: sandbox 39 -> 18 at build 109, and build instructions 10..28 become the target's 24-instruction flag block instruction-for-instruction. The global allocno list falls from 13 to 12 and no block-0 pseudo "dies in 4 places" any more.
- verdict: CONFIRMED

## [s3] On the single-accumulator body, declaring `v` first among the locals and hoisting `v = p[8];` above the pointer setup each measured 39, the same score as the unmodified s2 body, so neither order lever moved any flag-block register seat.
- mechanism: the seat assignment is decided by local-alloc.c:472's reg_n_deaths test, which counts how many times the variable is written, not where it is declared or where its first assignment appears; cse1 normalises RTL order before local-alloc runs.
- probe: tmp/grind/func_80035280/s3/v/v1.c and v2.c, sandbox 39 / build 109 each; the neighbouring form that drops the `f` pointer (v3.c) measured 44 / build 111.
- result: order levers are inert on this function; the death count is the lever. Banked at rejected/s3-single-accumulator-global-allocno-score39.c and rejected/s3-no-f-pointer-adds-two-insns-score44.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), single-accumulator s2 body plus two order variants, zero FAKE constructs present

## [s3] Splitting the flag accumulator into two alternating locals measured 28 against the three-way split's 18, because each of the two pseudos is still written twice and still fails local-alloc.c:472's reg_n_deaths == 1 test.
- mechanism: local-alloc.c:472 requires exactly one REG_DEAD note per pseudo; a two-way alternation gives each pseudo two, so both stay ineligible for local allocation and only part of the flag block lands on the target's seats.
- probe: tmp/grind/func_80035280/s3/v/v5.c, sandbox 28 / build 109.
- result: the split has to be one local per merged bit. Banked at rejected/s3-two-alternating-accumulators-score28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), two-accumulator body, zero FAKE constructs present

## [s3] The s3 candidate built with -msoft-float scores 16 at build 109, identical to the hard-float build, so the flag does not close this body.
- mechanism: -msoft-float fixes the 32 FP hard registers, so loop.c:532's (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) falls from 122 to about 58; loop.c:1631 then still finds 58 * 1 * 1 >= 55 for this body's loop-2 insn_count of 55 and hoists the 0x91A2B3C5 movable anyway. The s1 walker body's insn_count of 62 clears 58, which is why s2 measured that body at 108 == 108 under the flag.
- probe: built code6cac_b through the real pipeline with -msoft-float (tmp/grind/func_80035280/s2/build_o.sh, artifact tmp/grind/func_80035280/s3/soft_v6.o) and scored it against build/src/code6cac_b.o with engine score_func: {"score": 16, "target_insns": 108, "build_insns": 109}. The .loop slice for the same body reads "Loop from 110 to 274: 55 real insns" with both constants "moved to".
- result: s2's FACT 11 is restated - the configuration question is "add the flag AND find a loop-2 spelling with insn_count in [59, 122] keeping the inline-index giv shape", not "add the flag". The s1 walker spelling clears 59 but costs two giv instructions (build 111).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), s3 candidate body built with -msoft-float through the real pipeline, zero FAKE constructs present

## [s3] The flag-block four-seat register permutation is caused by local-alloc.c:472's eligibility test reg_n_deaths[i] == 1, and splitting the single reused accumulator into one local per merged bit puts every flag-block seat on the target's register.
- mechanism: local-alloc.c:470-477 sets reg_qty[i] = -2 (eligible for LOCAL allocation) only when reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1. A single reused `v` is set four times, so it carries four REG_DEAD notes (.lreg: 'Register 77 used 10 times across 16 insns in block 0; dies in 4 places'), gets reg_qty = -1, and is left to global-alloc - which runs only after local-alloc has already handed $v0 and $v1 to the six short per-arm temps and $a0 to the address pointer. Giving each accumulator value its own local makes reg_n_deaths == 1 per pseudo, so they become block-0 quantities, and local-alloc's qty_compare priority floor_log2(n_refs)*n_refs*size/(death-birth) ranks the accumulator chain ahead of the two-reference temps, so find_free_reg hands it $v0 first.
- probe: Ran the instrumented cc1 (tools/gcc-2.7.2/cc1, NOT the engine's tools/gcc-2.7.2/build/cc1, which emits no QTYDBG lines) with BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 and read the block-0 trace: exactly seven quantities (reg1 = 73, 78, 82, 83, 87, 88, 92) with the accumulator absent, got = 2,2,2,3,3,3,4. The greg slice prints ';; 77 conflicts: 72 74 76 77 2 3 4 29'. Then measured the split body with sandbox --disable all.
- result: 39 -> 18 at build 109 from the split alone, and build instructions 10..28 become the target's 24-instruction flag block instruction-for-instruction and register-for-register against asm/funcs/func_80035280.s:25AA4-25AEC. Confirmed downstream in the s3 dumps: the global allocno list falls from ';; 13 regs to allocate' to ';; 12 regs to allocate' and no block-0 pseudo says 'dies in 4 places' any more.
- verdict: CONFIRMED

## [s3] Writing the loop-1 counter initialisation as a statement immediately after the func_80077D00() call, with loop 1 spelled `for (; i < 3; i++)`, puts the counter zeroing where the target has it and takes two more points off the score.
- mechanism: The target's first instruction after the jal's delay slot is `addu $a3,$zero,$zero`. The RTL position of the initialisation decides where the first-pass scheduler can place it inside block 0; the earlier the statement, the earlier the insn's LUID and the earlier the ready-list lets it issue.
- probe: Measured four placements on the split body: statement immediately after the call = 16 (build 109); statement immediately before the flag block = 18 (build 109); statement immediately before loop 1, i.e. equivalent to `for (i = 0; ...)` = 18 (build 109); `s32 i = 0;` declaration initialiser = 29 (build 110).
- result: 16 is the session floor. Build instructions 1..38 - prologue, the whole flag block and the whole of loop 1 - are now identical to the target. The declaration-initialiser form is emitted BEFORE the call, so the value must survive it and GCC spends a real extra instruction (build 110), which is why it is 29 rather than 16.
- verdict: CONFIRMED

## [s3] On the single-accumulator body, declaring `v` first among the locals and hoisting `v = p[8];` above the pointer setup each measured 39, the same score as the unmodified s2 body, so neither order lever moved any flag-block register seat.
- mechanism: The seat assignment is decided by local-alloc.c:472's reg_n_deaths test, which counts how many times the variable is written, not where it is declared or where its first assignment appears; cse1 normalises the RTL order before local-alloc runs.
- probe: tmp/grind/func_80035280/s3/v/v1.c (v declared first) and v2.c (v = p[8] hoisted above the f/src setup): sandbox 39 / build 109 each, against the s2 body re-measured at 39 / 109 first this session. The neighbouring form that drops the `f` pointer and spells src = (u8 *)&D_80106A73 - 3 directly (v3.c) measured 44 / build 111.
- result: Both order levers were inert here; the death count is what moves the seats. The two-pointer spelling is also confirmed right - the target genuinely keeps both $a1 (= &D_80106A73, for the lbu) and $a2 (= $a1-3, the loop-1 walker), and dropping `f` costs two instructions. Banked at rejected/s3-single-accumulator-global-allocno-score39.c and rejected/s3-no-f-pointer-adds-two-insns-score44.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (src/code6cac_b.c:3741 INCLUDE_ASM baseline, target_insns 108), single-accumulator s2 body plus two order variants, zero FAKE constructs present

## [s3] Splitting the flag accumulator into two alternating locals measured 28 against the three-way split's 18, because each of the two pseudos is still written twice and so still fails local-alloc.c:472's reg_n_deaths == 1 test.
- mechanism: local-alloc.c:472 requires exactly one REG_DEAD note per pseudo. A two-way alternation (fA/fB) leaves each pseudo with two, so both stay ineligible for local allocation and fall through to global-alloc, and only part of the flag block lands on the target's seats.
- probe: tmp/grind/func_80035280/s3/v/v5.c, sandbox 28 / build 109, against the three-way split's 18 / 109 measured in the same batch.
- result: The split has to be one local per merged bit; a partial split only partially helps. Banked at rejected/s3-two-alternating-accumulators-score28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), two-accumulator body, zero FAKE constructs present

## [s3] The s3 candidate built through the real pipeline with -msoft-float scores 16 at build 109, identical to the hard-float build, so the flag does not close this body.
- mechanism: -msoft-float fixes the 32 FP hard registers, so loop.c:532's (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) falls from 122 to about 58. loop.c:1631 then still finds 58 * 1 * 1 >= 55 for this body's loop-2 insn_count of 55 and hoists the 0x91A2B3C5 movable anyway. The s1 walker body's insn_count of 62 clears 58, which is why s2 measured that body at 108 == 108 under the flag.
- probe: Built code6cac_b with -msoft-float through cpp | cc1 | prologue_fix | maspsx | multu_pad | as (tmp/grind/func_80035280/s2/build_o.sh, artifact tmp/grind/func_80035280/s3/soft_v6.o) and scored it against build/src/code6cac_b.o with engine score_func: {"score": 16, "target_insns": 108, "build_insns": 109}. The .loop slice for the same body reads 'Loop from 110 to 274: 55 real insns' with both constants 'moved to'.
- result: s2's FACT 11 is restated: the configuration question, if the operator ever takes it, is 'add the flag AND find a loop-2 spelling whose insn_count lands in [59, 122] while keeping the inline-index giv shape', not 'add the flag'. The s1 walker spelling clears 59 but costs two giv instructions (build 111). Bracketed both ways this session.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), s3 candidate body built with -msoft-float through the real pipeline, zero FAKE constructs present

## [s4] Spelling loop 1's byte stores as `((u8 *)p + i)[0x17]` instead of `((u8 *)p)[0x17 + i]` flips the loop-1 address add to the target's operand order and takes the floor from 16 to 15.
- mechanism: The subscript `0x17 + i` builds a PLUS whose first operand is the index expression, and combine hands that to the addu pattern as `addu $v1, $a3, $t0`; adding the index to the POINTER and leaving a constant subscript puts the pointer in the first operand slot, giving the target's `addu $v1, $t0, $a3`.
- probe: tmp/grind/func_80035280/s4/v/cand_ptrplus.c installed at src/code6cac_b.c:3741 and measured with `sandbox func_80035280 --disable all`: {"score": 15, "target_insns": 108, "build_insns": 109} against the s3 body re-measured at 16/109 first this session. Cross-checked on the s4 mini-TU harness at 29 normalized diffs versus 31. Two neighbouring spellings measured no change (see the KILLED entry below).
- result: New session floor 15, saved as memory/grind/func_80035280/candidate.c. Independently rediscovered by permuter campaign 1 as `((u8 *)p)[(unsigned long long)(0x17 + i)]` - the only improvement that campaign found in 29,979 iterations.
- verdict: CONFIRMED

## [s4] The two neighbouring spellings of the loop-1 index add - `((u8 *)p)[i + 0x17]` and `(((u8 *)p) + 0x17)[i]` - are both inert, measuring 31 normalized diffs each against the accepted form's 29.
- mechanism: Commuting the operands inside the subscript expression does not commute the emitted addu, because combine canonicalises the PLUS before the operand order is fixed; folding the constant into the pointer and subscripting by `i` produces the same canonical form. Only moving the INDEX onto the pointer and leaving a constant subscript changes which operand reaches the first slot.
- probe: tmp/grind/func_80035280/s4/v/l1_iplus.c and l1_castdiff.c through tmp/grind/func_80035280/s4/score.sh: 31 diffs each, against the accepted `((u8 *)p + i)[0x17]` form at 29 and the s3 body at 31. Banked at rejected/s4-loop1-index-commute-nochange.c.
- result: The lever is specifically "pointer + index, then a constant subscript"; do not re-try the commuted spellings.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 mini-TU harness (tmp/perm_80035280/compile.sh, validated byte-identical to the full-TU sandbox object this session; target_insns 108), s3/s4 body with the three-way flag split, zero FAKE constructs present

## [s4] A 30k-iteration decomp-permuter campaign on the split-accumulator chassis finds nothing beyond the loop-1 address-add fix.
- mechanism: The permuter mutates the C AST locally (variable reuse, declaration order, casts, expression re-association, statement swaps). Every remaining point on this body is downstream of one loop.c:1631 LICM decision that needs loop-2 RTL insn_count to more than double, which is not a local AST mutation.
- probe: tools/permuter_campaign.py launch --dir tmp/perm_80035280 --label s4-split-accum-chassis -j 8 on the minimal-TU harness (base_score 600). 29,979 iterations, harvested with --stop. finds_new = 1, best_new_score = 590, and that find is the loop-1 transform spelled with an `(unsigned long long)` index cast plus inert noise. Log at tmp/grind/func_80035280/s4/perm/campaign_split_accum.log.
- result: The split-accumulator basin is exhausted for local mutation. A future permuter session on this function must not re-seed this chassis (chassis rule 2026-09-01).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), s3/s4 split-accumulator body in the validated minimal-TU harness, zero FAKE constructs present

## [s4] The structurally different walker loop-2 chassis - the highest loop-2 RTL insn_count ever measured here (62) - is a strictly worse permuter basin and never approaches the split-accumulator body.
- mechanism: The walker spelling keeps `dst` and `base` as explicit induction variables and three per-field locals, so loop 2 carries more pre-combine RTL (62 vs 55) but also emits two extra giv instructions and loses the target's `addiu $a1, $a1, 4` shape. 62 is still nowhere near the loop.c:1631 requirement of 123, so the extra RTL is paid for and buys nothing.
- probe: The s1 walker loop 2 grafted onto the s3/s4 flag-split prologue (rejected/s4-walker-loop2-on-split-prologue-score55.c) measures 55 normalized diffs against the accepted body's 29. Seeded as permuter campaign 2 (tools/permuter_campaign.py launch --dir tmp/perm_80035280_w --label s4-walker-loop2-chassis -j 8, base_score 960): 25,284 iterations, 14 finds, best 695 - well short of the other chassis's 590, and no find keeps the /1800 magic inside loop 2. Harvested with --stop; log at tmp/grind/func_80035280/s4/perm/campaign_walker.log.
- result: "More loop-2 RTL" is not on its own a productive direction to search from; the insn_count lever has to arrive without giving up the inline-index giv shape, and the permuter cannot construct that.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), walker loop-2 body on the s3/s4 flag-split prologue, zero FAKE constructs present

## [s4] `savings` and `m->lifetime` in the loop.c:1631 move test are pinned at their minima and can only move in the direction that makes the hoist MORE eager, so insn_count is the only C-movable term in the whole product.
- mechanism: `savings = m->savings = n_times_used[regno]` (loop.c:793) and `n_times_used` is a bcopy of `n_times_set` (loop.c:597), i.e. the count of SETS of the constant pseudo inside the loop - exactly 1 for a single constant load, and any C change that adds a second set raises it. `m->lifetime` (loop.c:791) is the luid span between the pseudo's first and last uid; the const load sits immediately before its single `mult`, so it is already the minimum 1, and separating set from use only raises it (that is why the /30 magic with lifetime 31-35 hoists so freely). `threshold` was already pinned at 122 by s1/s2.
- probe: Read of tools/gcc-2.7.2/loop.c lines 520-540, 690-800, 1520-1640, 2158-2215 and 2989-3060 this session, cross-checked against the s3 .loop dump line `Insn 137: regno 115 (life 1), move-insn savings 1  moved to 292`.
- result: The frontier's claim that insn_count is the only lever is now proven from source for all three factors, not just for threshold. Two further escape routes were closed in the same read: `loop_has_call` is set only by a real CALL_INSN (prescan_loop, loop.c:2202), and the movable is always recorded for a compiler-generated constant temp because the skip test at loop.c:695-701 is satisfied by `! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)` unconditionally. The one remaining source-level blocker, `may_not_move` (loop.c:3038-3044), requires the constant pseudo to be set in two basic blocks of the loop, and the target's loop 2 is a single straight-line basic block.
- verdict: CONFIRMED

## [s4] Spelling loop 1's byte stores as ((u8 *)p + i)[0x17] instead of ((u8 *)p)[0x17 + i] flips the loop-1 address add to the target's operand order and takes the floor from 16 to 15.
- mechanism: The subscript 0x17 + i builds a PLUS whose first operand is the index expression, and combine hands that to the addu pattern as `addu $v1, $a3, $t0`; adding the index to the POINTER and leaving a constant subscript puts the pointer in the first operand slot, giving the target's `addu $v1, $t0, $a3` in loop 1 of asm/funcs/func_80035280.s.
- probe: tmp/grind/func_80035280/s4/v/cand_ptrplus.c installed at src/code6cac_b.c:3741 via tmp/grind/func_80035280/s4/install.py and measured with `sandbox func_80035280 --disable all`: score 15, target_insns 108, build_insns 109 - against the s3 body re-measured at 16/109 first this session on the same chassis. Cross-checked on the s4 minimal-TU harness at 29 normalized instruction diffs versus the s3 body's 31.
- result: New session floor 15, saved as memory/grind/func_80035280/candidate.c and re-verified at 15 after being written back from that file. Permuter campaign 1 independently rediscovered the same transform as `((u8 *)p)[(unsigned long long)(0x17 + i)]` - the only improvement it found in 29,979 iterations.
- verdict: CONFIRMED

## [s4] The two neighbouring spellings of the loop-1 index add, `((u8 *)p)[i + 0x17]` and `(((u8 *)p) + 0x17)[i]`, are inert on this body: both measure 31 normalized instruction diffs against the accepted form's 29.
- mechanism: Commuting the operands inside the subscript expression does not commute the emitted addu, because combine canonicalises the PLUS before the operand order is fixed; folding the constant into the pointer and subscripting by i produces the same canonical form.
- probe: tmp/grind/func_80035280/s4/v/l1_iplus.c and l1_castdiff.c scored with tmp/grind/func_80035280/s4/score.sh against tmp/perm_80035280/target.o: 31 diffs each, versus 29 for `((u8 *)p + i)[0x17]` and 31 for the s3 body. Banked at memory/grind/func_80035280/rejected/s4-loop1-index-commute-nochange.c.
- result: The lever is specifically 'pointer + index, then a constant subscript'; the commuted spellings buy nothing and should not be re-tried on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 minimal-TU harness (tmp/perm_80035280/compile.sh, validated byte-identical to the full-TU sandbox object for this function this session; target_insns 108), s3/s4 three-way-flag-split body, zero FAKE constructs present

## [s4] A 29,979-iteration decomp-permuter campaign on the split-accumulator chassis produced exactly one improvement, and it was the loop-1 address-add fix already found by hand.
- mechanism: The permuter mutates the C AST locally (variable reuse, declaration order, casts, expression re-association, statement swaps). Every remaining point on this body is downstream of one loop.c:1631 LICM decision that needs the loop-2 RTL insn_count to more than double, which is not a local AST mutation.
- probe: tools/permuter_campaign.py launch --func func_80035280 --dir tmp/perm_80035280 --label s4-split-accum-chassis -j 8, base_score 600, on the validated minimal-TU harness. Waited in-turn, harvested with --stop: iterations 29,979, finds_new 1, best_new_score 590; the find is the loop-1 transform spelled with an (unsigned long long) index cast plus inert noise (a &D_80106A58 hoist, `int new_var2 = ~4;`, `long flags0`). Log tmp/grind/func_80035280/s4/perm/campaign_split_accum.log, find tmp/grind/func_80035280/s4/perm/find_590_source.c.
- result: This chassis is exhausted for local mutation; under the 2026-09-01 chassis rule a future permuter session may not re-seed it. The campaign is a positive data point for the loop-1 fix and a negative one for everything else in the basin.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (src/code6cac_b.c:3741 INCLUDE_ASM baseline, target_insns 108), s3/s4 split-accumulator body in the validated minimal-TU harness, zero FAKE constructs present

## [s4] The structurally different walker loop-2 chassis - the highest loop-2 RTL insn_count measured on this function (62) - is a worse permuter basin: 25,284 iterations reached a best score of 695 against the split-accumulator chassis's 590.
- mechanism: The walker spelling keeps dst and base as explicit induction variables plus three per-field locals, so loop 2 carries more pre-combine RTL (62 vs 55) but emits two extra giv instructions and loses the target's `addiu $a1, $a1, 4` shape. 62 is still far below the loop.c:1631 requirement of 123, so the extra RTL is paid for and buys nothing.
- probe: The s1 walker loop 2 grafted onto the s3/s4 flag-split prologue (rejected/s4-walker-loop2-on-split-prologue-score55.c) scores 55 normalized diffs against the accepted body's 29. Seeded as campaign 2: tools/permuter_campaign.py launch --dir tmp/perm_80035280_w --label s4-walker-loop2-chassis -j 8, base_score 960; waited in-turn across eight windows; harvested with --stop at iterations 25,284, finds_new 14, best_new_score 695. Log tmp/grind/func_80035280/s4/perm/campaign_walker.log.
- result: Seeding a search from 'more loop-2 RTL' is not productive on its own - the insn_count lever has to arrive without giving up the inline-index giv shape, and no find in this campaign kept the /1800 magic inside loop 2. Both campaigns are harvested, stopped and confirmed dead by permuter_campaign.py status.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (target_insns 108), walker loop-2 body on the s3/s4 flag-split prologue, zero FAKE constructs present

## [s4] In the loop.c:1631 move test, `savings` and `m->lifetime` are pinned at their minima for this constant and can only move in the direction that makes the hoist more eager, so insn_count is the only C-movable term in the product.
- mechanism: savings = m->savings = n_times_used[regno] (loop.c:793) and n_times_used is a bcopy of n_times_set (loop.c:597), i.e. the count of SETS of the constant pseudo inside the loop - exactly 1 for a single constant load, and any C change adding a second set raises it. m->lifetime (loop.c:791) is the luid span between the pseudo's first and last uid; the const load sits immediately before its single mult, so it is already the minimum 1, and separating set from use only raises it - which is exactly why the /30 magic with lifetime 31-35 hoists freely. threshold was already pinned at 122 by s1/s2.
- probe: Read of tools/gcc-2.7.2/loop.c lines 520-540, 690-800, 1520-1640, 2158-2215 and 2989-3060 this session, cross-checked against the s3 .loop dump line 'Insn 137: regno 115 (life 1), move-insn savings 1  moved to 292'. Two further escape routes closed in the same read: loop_has_call is set only by a real CALL_INSN (prescan_loop, loop.c:2202), and the movable is always recorded for a compiler-generated constant temp because the skip test at loop.c:695-701 is satisfied unconditionally by `! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)`. The one remaining source-level blocker, may_not_move (loop.c:3038-3044), needs the constant pseudo set in two basic blocks of the loop, and the target's loop 2 is one straight-line basic block.
- result: The frontier's insn_count claim is now proven from compiler source for all three factors rather than just for threshold, so a later session does not need to re-open savings, lifetime, loop_has_call or movable-creation.
- verdict: CONFIRMED

### H14
## [s5] The loop.c:1631 desirability threshold for loop 2 is 58 on the shipped `-msoft-float` chassis, not the 122 banked by s1/s2/s4, so the 0x91A2B3C5 hoist is refused at loop-2 insn_count 59 rather than 123.
- mechanism: loop.c:532 sets `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`. `-msoft-float` (canonical CC_FLAGS since 2026-09-07) marks the 32 FP registers fixed, roughly halving n_non_fixed_regs; the s1/s2 measurement of 122 was taken before that flag was adopted, and s4 re-derived it from loop.c source without re-measuring n_non_fixed_regs. With savings and lifetime both pinned at 1 (s4 H13, still CONFIRMED), the move test reduces to `threshold >= insn_count`.
- probe: Built the s4 body with N extra real statements inside loop 2 and read the `-dL` dump for the 0x91A2B3C5 movable, N chosen to sweep insn_count. Measured: 55 "moved", 57 "moved", 59 "not desirable", 61/63/71/79/87/103/123 "not desirable". The flip is between 57 and 59, and threshold is even by construction, so threshold = 58 and n_non_fixed_regs = 28. Artifacts tmp/grind/func_80035280/s5/pad{0,1,2,3,4,8,12,16,24,34}.c, probe.sh, last.loop.
- result: The s4 frontier's "insn_count must reach 123 against a natural ceiling of 62" is void. The required lift was 55 -> 59, four insns, and it is paid for by four ordinary named intermediates that combine folds away. This closed the function.
- verdict: CONFIRMED

### H15
## [s5] Staging loop 2's three clock fields and its raw record byte through fresh named intermediates, with the stores interleaved between the assignments, reaches loop-2 insn_count 59 at zero byte cost and takes func_80035280 to distance 0.
- mechanism: each `u8` intermediate forces a QImode truncation insn at RTL-expansion time that survives cse (it is a distinct pseudo) and is therefore counted by count_loop_regs_set (loop.c:2989) into insn_count, but is folded back into the `sb` by combine, so build_insns is unchanged at 108. `t` (s32) contributes the SImode load temp for the `base[i*8]` read. Total lift 55 -> 59, one insn past threshold 58, so move_movables prints "not desirable" for the 0x91A2B3C5 movable and the lui/ori stays in the loop as the target has it.
- probe: sandbox func_80035280 --disable all = score 0, target_insns 108 == build_insns 108; verify-oracle build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true. Body saved to memory/grind/func_80035280/candidate.c and in place at src/code6cac_b.c.
- result: MATCHED. Self-vet at memory/grind/func_80035280/self_vet.md claims the named-intermediate family (no-new-park-categories.md:214, six prongs checked) for the seven intermediates and the pointer-alias family (pointer-alias-fake-exception.md:5) for `f` and `base`; all four /* FAKE */ annotations are in src.
- verdict: CONFIRMED

### H16
## [s5] At loop-2 insn_count 58 -- one insn short of the gate -- fourteen structurally different loop-2 spellings all stay at 29 diffs, so the intermediate COUNT, not the shape, is what closes this function.
- mechanism: every spelling that stages three (not four) values through fresh intermediates lands on exactly 58 real insns, and move_movables still prints "moved" at 58 because the test is `>=`.
- probe: measured with tmp/grind/func_80035280/s5/probe.sh -- nested-block u8 intermediates (vK), u8 with an extra block for the record byte (vL), s8 variants (vM, vN), u16 in each of the three positions and in all three (wA-wD), a dest pointer local (wF, yB), an s32+u8 double local on the first field (wE), an index local `j` (yC), a source pointer local `b` (yD), a u8 local for the record byte (yE, zD), and `((s32 *)(base + i*8))[1]` reads (yF). All: insn_count 58, diffs 29, "moved".
- result: KILLED as a route to 0 -- three intermediates are not enough on this chassis regardless of how they are spelled or typed. A fourth intermediate on the record byte, typed s32 rather than u8 (`s32 t = base[i*8];`), is what supplies the last insn: zD with `u8 id` measures 58/29, zC with `s32 t` measures 59/0.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (src/code6cac_b.c, -msoft-float CC_FLAGS, target_insns 108), s4 split-accumulator prologue plus each listed loop-2 spelling, /* FAKE */ constructs present = the inherited f alias, base view, and flags0/1/2 split

### H17
## [s5] Spelling `&D_80106A58` inline at loop 2's use sites reaches the insn_count gate but creates a second address movable, and recomputing the record pointer inside the loop destroys the $a2 giv.
- mechanism: each inline `(u8 *)&D_80106A58` is its own address-materialisation movable. When `base` is also kept it hoists separately (loop.c move_movables), so the preheader carries two %hi/%lo pairs instead of the target's one; when `base` is dropped and the pointer is rebuilt from `i * 8` inside the loop, loop.c can no longer form the stride-8 giv and emits `sll`/`addu` per iteration instead of the target's `addiu $a2, $a2, 8`.
- probe: xA (inline everywhere, no base local) insn_count 61, "not desirable", 113 insns, 39 diffs; xB / xD (base kept, symbol inline for the s32 reads) insn_count 60, "not desirable", 113 insns, 39 diffs; xC (pointer recomputed inside the loop) insn_count 59, "not desirable", 111 insns, 35 diffs, and the instruction diff shows `sll a1,a3,3; addu a1,a1,v0` replacing the giv.
- result: KILLED as a route to 0. Useful negative: reaching the gate is necessary but not sufficient -- the padding must not disturb the two givs ($a1 stride 4, $a2 stride 8) or the single preheader address materialisation. The named intermediates satisfy that; extra address handles do not.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (-msoft-float CC_FLAGS, target_insns 108), s4 prologue plus the xA/xB/xC/xD loop-2 spellings, /* FAKE */ constructs present = the inherited f alias and flags0/1/2 split (xA and xC additionally drop the base alias)

### H18
## [s5] The three constructs inherited from s1/s3 -- the `i = 0;` hoist, the `f = &D_80106A73;` alias, and the flags0/flags1/flags2 split -- are each still load-bearing on the matched body.
- mechanism: not re-derived this session; each was deleted from the matched body in isolation and the resulting diff count recorded.
- probe: m1.c folds `i = 0;` back into the for-header -> 2 diffs. m2.c drops `f` and writes `src = &D_80106A73 - 3;` -> 50 diffs. m4.c collapses flags0/1/2 into the single `flags` accumulator -> 44 diffs. cur.c (the shipped body) -> 0 diffs. All via tmp/grind/func_80035280/s5/probe.sh.
- result: CONFIRMED still load-bearing. The s3/s4 frontier item asking whether the split accumulators and the `i = 0;` hoist could be dropped before submission is answered: no. They are carried into the submission with /* FAKE */ annotations and family claims in self_vet.md.
- verdict: CONFIRMED
