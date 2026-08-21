# Hypothesis ledger — func_800858D0

## s1 (2026-08-20, recon, post-migration chassis)

### H1 — CONFIRMED: folding the banked A+B statement moves lowers the floor 22 → 15
Statement: applying the sched_solver-derived A (buf[1]=0x60093 before var_s0=0) and
B (0x3C store moved to end of init block) moves to the floor-22 candidate improves the
honest floor on the new chassis.
Probe: applied candidate + A + B to src/main.c, `sandbox func_800858D0 --disable all`.
Result: **score 15**, target_insns 72 == build_insns 72. Banked as the new candidate.c.

### H2 — CONFIRMED: the current multi-set-t form produces ZERO loop.c movables,
matching target's nothing-hoisted emission profile
Statement: the shared scratch `t` (24 then 1) leaves loop.c with no movable to hoist.
Probe: `dump.ps1 func_800858D0`, read main.loop at the function ("Loop from 52 to 146:
30 real insns"). Result: no "moved to" lines; t = reg 75 appears only as "possible biv,
reg 75, const = 24 / = 1 ... biv discarded, never incremented". The ledger's old open
question ("why does move_movables not hoist target's constants?") is thereby RESOLVED at
the decision-profile level: our form already reproduces the no-hoist outcome. What does
NOT match is the register split (target: 24 in $v1, 1/shift in $v0; ours: both in $a1).

### H3 — KILLED (analytically, from loop.c + mips.md + measured loop size): any
single-set loop-invariant constant pseudo in this loop is unconditionally hoisted
Mechanism: move_movables' test is `threshold*savings*lifetime >= insn_count`
(loop.c:1631) with threshold = (loop_has_call?1:2)*(1+n_non_fixed_regs) (loop.c:532).
MIPS FIXED_REGISTERS (mips.h:1188) fixes 8 of 68 regs → n_non_fixed_regs = 60 →
threshold = 61 (loop has calls). Our loop is 30 real insns (main.loop), so even the
minimum product 61*1*1 >= 30 always moves. The reg_single_usage substitution escape
(loop.c:735-768) cannot fire for either constant: movhi's insn condition
(mips.md:3352-3358) rejects (set (mem:HI) (const_int 24)), and sllv rejects a const
shifted operand, so validate_replace_rtx fails and the movable is created anyway.
Consequence: target's original C CANNOT have spelled 24 and 1 as plain single-set
constants; each must have been multi-set (or otherwise not a movable). Two-variable
single-set spellings are dead a priori — do not sweep them.

### H4 — CONFIRMED (mechanism named): the preheader/delay-slot identity is decided by
which insn sched1 leaves at loop-body block top; reorg then steals that insn
Probe: main.sched block 2 (insns 53-140, backward scheduler). Our t=0x18 set (insn 71,
priority 1) is ready from T-14 but loses every cycle to the priority-0x7f000001
address-arith chain, so it is placed last in backward time = FIRST in the block; reorg
then duplicates it into the preheader + loop-back delay slot (the two `li a1,24`
positions). Target's block-top insn was `addiu a0,sp,16` instead, with its li v1,24
adjacent to the store. So cluster (a) of the residual is a sched1-priority question,
not a loop.c or placement-sweep question.

### H5 — KILLED (re-measure of old-chassis kill): block-scope `s32 *p = buf;` at
loop-body top is byte-inert on the new chassis too
Probe: applied p at loop top, p[0] store, func_8008B488(p); sandbox = 15 (unchanged).
canon_reg folds the single-set alias before sched1 ever sees it. Banked in
rejected/p-loop-top-block-scope-inert-15.c.

## [s1] Folding the banked A+B statement moves into the floor-22 candidate lowers the honest floor on the post-migration chassis
- mechanism: sched_solver-derived LUID swaps (buf[1]=0x60093 before var_s0=0; 0x3C store last in init block) align block-0 emission order with target
- probe: applied candidate + A + B over the INCLUDE_ASM at src/main.c:857; sandbox func_800858D0 --disable all
- result: score 15, build_insns 72 == target_insns 72, frame 96, saves 3
- verdict: CONFIRMED

## [s1] The multi-set scratch t produces ZERO loop.c movables, matching target's nothing-hoisted emission profile
- mechanism: two in-loop sets make t neither a movable (n_times_set != 1, not consec-invariant) nor a biv ('possible biv, discarded, never incremented')
- probe: dump.ps1; read main.loop at 'Loop from 52 to 146: 30 real insns'
- result: no 'moved to' lines for this loop; t = reg 75 only appears as discarded biv; ledger's old move_movables question resolved at decision-profile level
- verdict: CONFIRMED

## [s1] Any single-set loop-invariant constant pseudo in this loop is unconditionally hoisted, so two-variable single-set spellings of the 24/1 are dead a priori
- mechanism: move_movables test threshold*savings*lifetime >= insn_count (loop.c:1631); threshold = 1*(1+n_non_fixed_regs) = 61 with calls (loop.c:532, mips.h FIXED_REGISTERS 8/68 fixed); insn_count = 30; 61*1*1 >= 30 always. reg_single_usage substitution (loop.c:735) cannot fire: movhi condition (mips.md:3352) rejects const-source HI stores, sllv rejects const shift operands
- probe: read loop.c:1595-1640, loop.c:700-768, mips.md movhi patterns, mips.h FIXED_REGISTERS; loop size from main.loop dump
- result: spelling class killed analytically; consistent with old-chassis measurements (t-for-24-only / t-for-1-only both re-hoisted to 27)
- verdict: KILLED

## [s1] Block-scope s32 *p = buf; at loop-body top changes the addiu a0,sp,16 placement on the new chassis
- mechanism: name the call arg early so its set anchors at loop top (hoist-call-arg-local family)
- probe: applied p at loop top with p[0] store and func_8008B488(p); sandbox
- result: score 15 unchanged - canon_reg folds the single-set alias before sched1; banked in rejected/p-loop-top-block-scope-inert-15.c
- verdict: KILLED

## s2 (2026-08-20, structural)

### H6 — CONFIRMED: an honest two-carrier spelling exists (t={1,increment}, u={0x18,limit-reload})
Statement: pairing each in-loop constant with a naturally loop-variant second set
(matching target's own register pairings v0={1,shift,incr-temp}, v1={24,lbu limit})
yields two multi-set, non-hoisted carriers.
Probe: do-while forms measured 25 -> 17 -> 13 (see evidence.md s2).
Result: carriers landed (24+lbu shared v1-class pseudo; li v0,1/sllv adjacent to jal
= cluster b CLOSED at 17; offset-reuse for the 1 pinned it late via store anti-deps = 13).
Verdict: CONFIRMED — but superseded: the goto spelling makes carriers unnecessary.

### H7 — KILLED: increment through an int temp (t = var_s0 + 1; var_s0 = t)
Mechanism: reading var_s0 in int context sign-extends BEFORE the add; CSE reuses the
loop-top sign-extend pseudo across both calls -> 4th callee-save, 73 insns (score 25).
Only direct s16 arithmetic (var_s0 = var_s0 + 1) gives target's addiu-raw + move + extend-temp.
Banked: rejected/two-carrier-incr-temp-sext-save-25.c.

### H8 — CONFIRMED (THE MATCH): goto-spelled loop (no loop notes) reproduces the entire
target emission profile; sandbox = 0
Statement: spelling the loop as `loop: ... if (var_s0 < D_80101BCC) goto loop;` removes
NOTE_INSN_LOOP_BEG/END so (a) loop.c/move_movables never hoists the single-set 24/1
pseudos, (b) birthing_insn_p (sched.c:2504-2526, reg_n_sets==1 gate) lets adjust_priority
(sched.c:2584-2590) launch each li adjacent to its consumer, (c) the unlaunchable multi-set
a0=sp+16 hard-reg set is the lone pri-1 straggler at block top per the rank_for_schedule
LUID tiebreak (sched.c:2461-2464), which reorg steals into preheader + loop-back delay
slot, holding $a0 live across the sign-extend span so local-alloc assigns the extend $a1
and the 24 $v1 (both Phase 5 RA parts land together).
Probe: applied the spelling with all-direct constants, sandbox --disable all.
Result: score 0, 72/72 insns, per-word disassembly identical to target modulo relocs;
(s16) casts additionally proven inert and removed.
Verdict: CONFIRMED. The s1 open question ("why does move_movables not hoist target's
single-set constants?") is answered: there was no loop for loop.c to see.
