# Hypothesis ledger - func_80069F80

## Session 1 (recon, 2026-09-15) - floor 136 -> 5

### H1 - CONFIRMED. The function is a sibling of func_8006D3DC / func_80069AE4.
Mechanism: text1b.c contains a family of "draw descriptor" call sites that fill
a stack struct and hand `&s` to func_80073728 / func_8007352C, then
`SetDrawMode(arg0[7], 1, 0, func_8006E480(header, 0), 0)`,
`AddPrim(D_800A374C + K, arg0[7])`, `arg0[7] += 0xC`.
Probe: transcribed asm/funcs/func_80069F80.s against src/text1b.c:6090-6130
(func_8006D3DC) and src/text1b.c:5611-5700 (func_80069AE4), then measured.
Result: first reconstruction scored 24/136 with 136/136 instructions and
identical control flow. The family shape is correct.

### H2 - CONFIRMED. The shared descriptor type is 0x3C bytes, not 0x2C.
Mechanism: the target frame is 0x70 with callee saves at 0x58; locals run
sp+0x18..sp+0x57. A 0x2C struct leaves 16 bytes unaccounted, giving frame 0x60
and shifting every `sw/lw $sN` offset.
Probe: appended `s32 sp44, sp48, sp4C, sp50;` to the struct.
Result: 24 -> 12. Every save/restore offset and both `addiu $sp` insns became
exact. The extra fields are ordinary unwritten struct members of the shared
type (this call site does not use them); no pad, no FAKE, no `(void)&`.

### H3 - KILLED (instance). Reordering the `s.sp28 = 0;` / `s.sp2C = 3;` /
### `s.sp1C = s.sp18 + 0xC;` statements can fix BOTH the idx 70/71 store order
### and the idx 68-73 v0/v1 allocation at the same time.
Mechanism: GCC 2.7.2 local-alloc sorts quantities in a basic block by
`qty_compare` priority = `floor_log2(n_refs)*n_refs*size/lifetime`
(tools/gcc-2.7.2/local-alloc.c:1641), so statement order changes lifetimes and
therefore which quantity lands in $v0; two stores to the same base are
memory-dependent in sched.c, so their emitted order follows source order.
Probe: all six permutations measured on the G chassis
(sp28,sp2C,sp1C)=8, (sp2C,sp28,sp1C)=6, (sp28,sp1C,sp2C)=9,
(sp2C,sp1C,sp28)=6, (sp1C,sp2C,sp28)=6, (sp1C,sp28,sp2C)=9; plus the five
sp28-first cross-products with every site-82 spelling ZE/ZG/ZF/ZH/ZK=8/8/9/8/8.
Result: the two goals are anti-correlated on this chassis. sp2C-first buys the
allocation (worth 3) and loses the store order (worth 2); sp28-first does the
reverse. Best reachable by permutation alone is 5.
kill_scope: instance. measured_on: HEAD 2026-09-15 chassis (-mel -msoft-float),
candidate G, NO FAKE constructs present, floor 5.

### H4 - KILLED (instance). Source-level spelling of
### `s.sp18 = p1; s.sp1C = p1 + 0xC;` can force the target's two-live-register
### form (`addiu $v1,$v0,0xC ; sw $v0,0x18 ; sw $v1,0x1C`).
Mechanism hypothesised: RTL emission order feeds sched1's `rank_for_schedule`
tie-break, so putting the `+ 0xC` computation first in the source would keep it
first in the schedule, leaving both values live and defeating
local-alloc's `combine_regs` tie.
Probe: five spellings measured - G (`s.sp18=p1;` then `s.sp1C=p1+0xC;`)=5,
E (separate `q1` temp, sp18 then sp1C)=5, F (separate `q1`, sp1C then sp18)=6,
H (memory re-read `s.sp18=ptr[1]; s.sp1C=s.sp18+0xC;`)=5,
K (`s.sp18=p1; p1+=0xC; s.sp1C=p1;`)=5. Then read the .sched dump.
Result: DISPROVEN AT THE PASS LEVEL, not just empirically. In the E chassis the
pre-schedule RTL order already WAS `insn 212 (reg78 = reg77+12)` before
`insn 215 (mem[fp+24] = reg77)` - the source order was right - and sched pass 1
emitted 215 before 212 anyway. Both insns carry equal INSN_PRIORITY because
GCC 2.7.2 makes the two stores memory-dependent on one another, so the
tie-break is decided inside sched.c, downstream of any statement ordering.
Moving the C statements cannot reach this.
kill_scope: instance. measured_on: HEAD 2026-09-15 chassis (-mel -msoft-float),
candidates E and G, NO FAKE constructs present, floor 5;
dump tmp/grind/func_80069F80/dumps/text1b.sched.

## Live frontier for session 2
1. **Raise the addiu's INSN_PRIORITY at idx 84-86 by changing WHAT is stored,
   not the order.** sched1's tie is between `sw 0x18` and the `addiu`. If the
   value written to +0x04 fed a longer dependency chain - e.g. the table
   pointer is derived through a typed pointer (`(s8 *)hdr + 0xC` on a
   `s8 *` member, or a two-step `hdr + 8 + 4`) - the addiu side of the tie
   gains a link and wins. Next probe: give the +0x04 member a pointer type in
   the struct and spell the derivation through the pointer, then re-read
   .sched to confirm the priority actually moved before trusting the score.
2. **Attack idx 70/71 from the other side: make the `+0x14` (ot index) store
   not need a fresh pseudo at all.** The target's `$v0` there holds the
   constant 3. If the ot index in the original is written from an expression
   already live in a register (for example computed once and reused by the
   second block, which stores 2 to the same member), the quantity's n_refs and
   lifetime both change and the sp28-first source order may keep the good
   allocation. Next probe: hoist a single `s32 ot` local that carries 3 then 2
   and measure both statement orders - note this must stay an ordinary
   consumed value, NOT a constant-holder (a dead `s32 three = 3;` would be the
   named-local FAKE family and is not wanted at floor 5).
3. **Permuter on the score-5 body.** The residual is 5 instructions in one
   block and is a pure sched1/local-alloc tie; this is exactly the shape
   decomp-permuter's randomizer closes. Next probe: import the candidate,
   run a fresh-seed campaign via tools/permuter_campaign.py, harvest in-turn.

## [s1] func_80069F80 is a member of the text1b.c draw-descriptor family (func_8006D3DC / func_80069AE4): fill a stack struct, call func_80073728 then func_8007352C, then SetDrawMode(arg0[7],1,0,func_8006E480(header,0),0) / AddPrim(D_800A374C+0xC, arg0[7]) / arg0[7] += 0xC.
- mechanism: Structural transcription of asm/funcs/func_80069F80.s against the two sibling C bodies already in the TU; identical call sequence and identical struct field offsets relative to sp+0x18.
- probe: Wrote the full body from the sibling shape and measured `sandbox func_80069F80 --disable all`.
- result: Score 24/136 on the first try, with 136/136 instructions emitted and byte-identical control flow. The family shape is right.
- verdict: CONFIRMED

## [s1] The shared draw-descriptor type is 0x3C bytes, not the 0x2C the sibling reconstructions declare; this call site simply does not write the last four words.
- mechanism: Target frame is 0x70 with callee saves at sp+0x58, so locals span sp+0x18..sp+0x57. A 0x2C struct gives frame 0x60 and shifts every callee-save sw/lw offset plus both addiu $sp insns.
- probe: Appended `s32 sp44, sp48, sp4C, sp50;` to the struct and re-measured.
- result: Score 24 -> 12. All ten frame-offset instructions became exact. These are ordinary unwritten struct members of the shared type, not a pad, and carry no FAKE construct.
- verdict: CONFIRMED

## [s1] On the current chassis, permuting the three statements `s.sp28 = 0;`, `s.sp2C = 3;` and `s.sp1C = s.sp18 + 0xC;` fixes the idx 70/71 store order and the idx 68-73 v0/v1 seat at the same time.
- mechanism: local-alloc sorts a block's quantities by qty_compare priority = floor_log2(n_refs)*n_refs*size/lifetime (tools/gcc-2.7.2/local-alloc.c:1641), so statement order sets lifetimes and hence the $v0 seat; two stores to the same base are memory-dependent in sched.c so their emitted order tracks source order.
- probe: Measured all six permutations on the best chassis (8, 6, 9, 6, 6, 9) plus the five sp28-first cross-products with every site-82 spelling (ZE/ZG/ZF/ZH/ZK = 8/8/9/8/8).
- result: The two goals are anti-correlated here: sp2C-first buys the allocation (worth 3) and loses the store order (worth 2), sp28-first does the reverse. 5 is the best value reachable by permuting these three statements; both residual sites must be attacked by something other than their relative order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-15 chassis (-mel -msoft-float), candidate G body, NO FAKE constructs present, floor 5

## [s1] Spelling `s.sp18 = p1; s.sp1C = p1 + 0xC;` differently at idx 84-86 reproduces the target's two-live-register form (addiu $v1,$v0,0xC ; sw $v0,0x18 ; sw $v1,0x1C) instead of the tied single-register form (sw ; addiu ; sw).
- mechanism: Hypothesised that RTL emission order feeds sched pass 1's rank_for_schedule tie-break, so computing the +0xC first in the source would keep it first in the schedule and leave both values live, defeating local-alloc's combine_regs tie.
- probe: Measured five spellings (plain, separate temp before/after, memory re-read, in-place `p1 += 0xC` mutation): 5, 5, 6, 5, 5. Then ran `pwsh tools/grinder/dump.ps1 func_80069F80` and read tmp/grind/func_80069F80/dumps/text1b.sched.
- result: Disproven at the pass level, not merely empirically. In the E chassis the pre-schedule RTL order already was insn 212 (reg78 = reg77 + 12) ahead of insn 215 (mem[fp+24] = reg77) - the source order was already correct - and sched pass 1 emitted 215 before 212 anyway. Both insns carry equal INSN_PRIORITY because GCC 2.7.2 makes the two same-base stores memory-dependent on each other, so the choice happens inside sched.c downstream of any statement ordering. Statement reordering is the wrong lever for this residual; the next attack must change the dependency chain length, not the order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-15 chassis (-mel -msoft-float), candidates E and G, NO FAKE constructs present, floor 5; dump tmp/grind/func_80069F80/dumps/text1b.sched

## Session 2 (structural, 2026-09-15) - floor 5 -> 0

### H5 - CONFIRMED. The idx 84-86 residual (sw ; addiu ; sw instead of
### addiu ; sw ; sw) is decided by sched pass 1's adjust_priority(), and it
### flips when the pseudo holding p1 + 0xC is set MORE THAN ONCE.
Mechanism (read from tools/gcc-2.7.2/sched.c this session, not inferred):
sched.c schedules each block in reverse. When an insn is scheduled, every
predecessor whose ref_count drops to zero goes through adjust_priority();
with reload not yet done n_deaths is always 0 (REG_DEAD notes were stripped),
so the insn has INSN_PRIORITY raised to max_priority (= LAUNCH_PRIORITY
0x7f000001 of the insn just scheduled) IF birthing_insn_p() holds, and
birthing_insn_p() returns `reg_n_sets[REGNO] == 1`. In the session-1 body the
addiu `reg119 = reg77 + 12` sets a once-written pseudo, so the moment
`sw reg119` is scheduled the addiu is boosted above the pending `sw reg77`
(priority 4) and is placed immediately before its store: sw ; addiu ; sw. If
the destination pseudo has two sets the boost is skipped, the addiu stays at
priority 4, rank_for_schedule falls through to the INSN_LUID tie-break, and
the RTL order (addiu first, as the source already had it) is preserved:
addiu ; sw ; sw = target. The session-1 dump shows the boost directly:
ready list at T-9 is `212 (4) 215 (7f000001)` (tmp/grind/func_80069F80/s2/f.sched).
Probe: `s32 tbl;` assigned at both fills inside the join block (`tbl =
s.sp18 + 0xC; s.sp1C = tbl;` and `tbl = p1 + 0xC; s.sp18 = p1; s.sp1C = tbl;`),
everything else as the session-1 body (V1).
Result: 5 -> 2. The idx 84-86 diff is gone; the remaining 2 is the idx 70/71
store order.

### H6 - CONFIRMED. With the two-set `tbl` in place, the sp28-first statement
### order (`s.sp28 = 0; s.sp2C = 3;`) fixes idx 70/71 WITHOUT losing the
### v0/v1 seat that H3 measured it losing on the session-1 body.
Mechanism: H3's coupling ran through local-alloc's combine_regs tying the
once-set +0xC pseudo into the loaded pointer's quantity, which changed the
block's quantity ordering when the constant-3 pseudo's lifetime moved. A
twice-set pseudo is its own quantity and is not tied, so the constant-3
quantity keeps its seat under either statement order.
Probe: V4 = V1 + `s.sp28 = 0;` moved ahead of `s.sp2C = 3;`.
Result: **sandbox distance 0/136** (136/136 insns), confirmed twice this
session with V4 in src/text1b.c. Diff: tmp/grind/func_80069F80/s2/final.diff.

### H7 - KILLED (instance). Carrying `tbl` across the join block AND the
### `arg1 & 1` block (second and/or third fill) also closes the residual.
Mechanism hypothesised: the same reg_n_sets > 1 effect with the arg1&1 fill
as the second writer instead of the first fill.
Probe: V2 (`tbl` at fills 2 and 3, first fill inline) = 7; V3 (`tbl` at all
three fills) = 9.
Result: worse than the floor-5 base. A pseudo referenced in two basic blocks
leaves local-alloc for global.c, and the third fill needs the in-place
`p2 += 0x14` mutation (anti-dependence sw ; addiu ; sw) that session 1 found,
not the shared local. Both writers must sit in the join block.
kill_scope: instance. measured_on: HEAD 2026-09-15 chassis (-mel
-msoft-float), V2/V3 bodies, NO FAKE constructs present, base floor 5.

## [s2] The +0xC table pointer must be a local assigned at both fills of the join block; a once-assigned temp (or the inline expression) always schedules after its store.
- mechanism: tools/gcc-2.7.2/sched.c adjust_priority() boosts a newly-ready insn to LAUNCH_PRIORITY only when birthing_insn_p() holds, i.e. reg_n_sets == 1 for the pseudo it sets; a twice-set pseudo falls through to the INSN_LUID tie-break in rank_for_schedule and keeps the RTL order (addiu ; sw ; sw).
- probe: V1 = session-1 body + `s32 tbl` assigned at both fills in the join block (`tbl = s.sp18 + 0xC; s.sp1C = tbl;` / `tbl = p1 + 0xC; s.sp18 = p1; s.sp1C = tbl;`).
- result: 5 -> 2; the idx 84-86 diff disappeared, leaving only the idx 70/71 store order. Same spelling as the matched in-TU function func_80069E18 (src/text1b.c:5793).
- verdict: CONFIRMED

## [s2] With the two-set `tbl`, writing `s.sp28 = 0;` before `s.sp2C = 3;` fixes the idx 70/71 store order without losing the v0/v1 seat.
- mechanism: the session-1 coupling came from combine_regs tying the once-set +0xC pseudo to the loaded pointer's quantity; a twice-set pseudo is its own local quantity, so the constant-3 quantity keeps its seat under the sp28-first order.
- probe: V4 = V1 with `s.sp28 = 0;` moved ahead of `s.sp2C = 3;`; measured with `sandbox func_80069F80 --disable all`.
- result: distance 0/136, 136/136 instructions, confirmed twice with V4 in src/text1b.c. No FAKE constructs, no sanctioned-family claim.
- verdict: CONFIRMED

## [s2] Carrying `tbl` into the `arg1 & 1` fill (fills 2+3, or all three) closes the residual as well.
- mechanism: hypothesised the same reg_n_sets > 1 effect with a different second writer.
- probe: V2 (fills 2 and 3) = 7; V3 (all three fills) = 9.
- result: both worse than the floor-5 base; a pseudo live in two basic blocks leaves local-alloc, and the third fill needs session 1's in-place `p2 += 0x14` mutation. Both writers must sit in the join block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-15 chassis (-mel -msoft-float), V2/V3 bodies, NO FAKE constructs present, base floor 5
