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
