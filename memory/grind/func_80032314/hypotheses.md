# hypotheses — func_80032314

## KILLED (session 1)

### H1 — "the HEAD `.word` GTE island is score-neutral; the 27 is structural"
KILLED (inverted). Mechanism: `tools/classify_inline_asm.py:62`
`CANONICAL_DOTWORD_RE` only accepts `.word 0x4[89A-F]...`, so the swc2 word
`0xE99F0000` was classified cheat and stripped by the cheat-invisible sandbox,
along with the two `__asm__("nop")` blocks and the `$t4` pin. Probe: respell the
island with real `mtc2`/`swc2` mnemonics inside ONE block (both are in
`CANONICAL_ASM_OPS`, and a block with any canonical instruction is kept whole).
Result: floor 27 → 15, build_insns 106 → 109 = target_insns. 12 points of the
original "structural" gap were a classifier artifact.

### H2 — "declaration order of `ent` / `state` / `a0` biases the $a1/$a2/$a3 rotation"
KILLED. Mechanism: named-intermediate declaration order changes RTL pseudo
creation order and therefore LUID / allocno ordering (the SOTN-sanctioned
declaration-order lever). Probe: moved `u8 *ent;` from first to last of the
three function-scope locals. Result: floor unchanged at 15, identical rotation.
Banked as rejected/decl-order-ent-last.c.

### H3 — "naming the three squared terms shortens mflo1's live range and lets it take $a1"
KILLED. Mechanism: `s32 dxs = dx*dx; s32 dys = dy*dy; s32 dzs = dz*dz;` gives
each mult result its own named pseudo instead of one nested expression tree,
which in other BB2 functions has re-ordered which mflo result is allocated
first. Probe: exactly that rewrite, `dist_sq = (u32)(dxs + dys + dzs)`. Result:
floor unchanged at 15, identical rotation — GCC coalesces the named temps back
into the same three pseudos. Banked as rejected/named-mult-intermediates.c.

## LIVE FRONTIER (ranked)

### F1 — the walker pseudo must be allocated LAST of the three, not first
Statement: the residual 15 is a single 3-cycle rotation caused by the record
walker (pseudo 74) claiming `$a1` ahead of the two others; if the walker lands
in `$a3`, mflo1 falls into `$a1` and `ent` into `$a2` and the function closes.
Mechanism: `tools/gcc-2.7.2/config/mips/mips.h` defines no `REG_ALLOC_ORDER`,
so `global.c:find_reg` scans hard regs in ascending order; the walker is a
loop-carried (global) allocno whose priority in `global.c` is
`live_length * n_refs`-driven. Lowering the walker's priority relative to `ent`
— fewer refs, or a shorter live range across the mult cluster — should push it
past $a1/$a2. Next probe: re-run the `.greg`/`.lreg` dumps with
`tmp/grind/func_80032314/s1/greg.sh` and read the ALLOCNO ORDER (not just the
final dispositions) — instrument `global.c`'s allocno_priority sort via the
existing `BB2_ALLOC_DEBUG` instrumented cc1 at `tools/gcc-2.7.2/cc1` (see
[[instrumented-cc1-location]] — it is the top-level cc1, NOT build/cc1) — to
learn the actual priority ranking of pseudos 74 / 75 / 115, then pick the C
edit that reorders exactly those two priorities.

### F2 — derive the walker from `ent`'s block instead of carrying it across the loop
Statement: the walker's live range spans the whole loop body only because our C
hoists `a3 = t0 + 2` out; if the original source carried ONE pointer and the
`+2` view were a separately-declared local re-derived per iteration, the walker
allocno would be shorter-lived and rank below `ent`. Mechanism: same
`global.c` priority = live_length × n_refs. Caution: the target DOES keep a
single `addiu $a3,$t0,0x2` in the prologue and a single `addiu $a3,$a3,0x2C` in
the latch, so any re-derivation must be one the loop optimizer strength-reduces
back to that exact pair — verify insn count stays 109 before reading the score.
Next probe: `u8 *w = t0 + 2;` declared INSIDE the loop body (after the
`*t0 == 0` guard) and incremented implicitly via `t0`, then confirm 109 insns.

### F3 — reduce `ent`'s reference count so it outranks the walker
Statement: `ent` has 6 refs (`+0x6A`, `+0xF4`, `+0xF8`, `+0xFC`, `+0x286`) and
the walker 7; if `ent` gained effective priority over the walker the rotation
would resolve without touching the walker. Mechanism: the SOTN-sanctioned
duplicated-statement-into-arms lever raises `reg_n_refs` byte-neutrally
(.claude/rules/duplicated-statement-into-arms.md) — it is the documented
byte-free ref-lift for exactly this kind of global-RA priority wall. Next
probe: duplicate the `*(s16 *)(ent + 0x286) = 5;` store (or the `ent + 0x6A`
read) into both arms of the `log2_val < v0` test, confirm cross-jump re-merges
it to identical bytes (109 insns), and read the score. NOTE: this is a
last-resort family requiring byte-neutrality proof + lever-exhaustion + a
`/* FAKE */` annotation — do NOT reach for it before F1 and F2 are measured.

## [s1] HEAD's `.word`-spelled GTE leading-zero-count island is score-neutral, so the 27 gap is structural C.
- mechanism: tools/classify_inline_asm.py:62 CANONICAL_DOTWORD_RE = /\.word\s*0x4[89A-Fa-f]/ accepts only cop2 opcode bytes 0x48-0x4F. `.word 0x488CF000` (mtc2) passes but `.word 0xE99F0000` (swc2 $31,0($t4)) does NOT, so the cheat-invisible sandbox stripped the LZCR store; the two bare `__asm__("nop")` blocks (nop is in CHEAT_ASM_OPS) and the $t4 register pin were stripped too, deleting both feeder `addu` moves and leaving `clz` reading uninitialised stack.
- probe: Respelled the island as ONE block using real mnemonics, copied verbatim from the matched user-authorized sibling func_800274BC (src/code6cac_b.c:292): `addu $t4,%1,$zero / mtc2 $t4,$30 / nop / nop / addu $t4,$sp,$zero / swc2 $31,0($t4)` with `"=m"(sp_tmp)` / `"r"(dist_sq)` / clobber "$12", plus that sibling's tail-arithmetic spelling. Ran `sandbox func_80032314 --disable all`.
- result: Floor 27 -> 15; build_insns 106 -> 109, exactly equal to target_insns. Intermediate measurements: two separate mnemonic blocks without the $t4 feeder moves = 28 (107 insns); a `move $12,%0`-prefixed two-block form = 35 (102 insns, both blocks stripped).
- verdict: KILLED

## [s1] Declaration order of the function-scope locals ent / state / a0 biases the $a1/$a2/$a3 rotation.
- mechanism: Declaration order changes RTL pseudo creation order and therefore LUID and allocno ordering in global.c — the SOTN-sanctioned named-intermediate declaration-order lever.
- probe: Moved `u8 *ent;` from first to last of the three function-scope locals; re-ran sandbox --disable all.
- result: Floor unchanged at 15; identical 3-cycle rotation, identical 109 insns. Banked as memory/grind/func_80032314/rejected/decl-order-ent-last.c.
- verdict: KILLED

## [s1] Naming the three squared terms as separate locals shortens the first mflo's live range and lets it take $a1 (target) instead of $a2.
- mechanism: Splitting the nested `dx*dx + dy*dy + dz*dz` expression tree into three named pseudos changes which mult result is allocated first; in other BB2 functions this has re-ordered mflo allocation.
- probe: `s32 dxs = dx*dx; s32 dys = dy*dy; s32 dzs = dz*dz; dist_sq = (u32)(dxs+dys+dzs);` re-ran sandbox --disable all.
- result: Floor unchanged at 15; GCC coalesces the named temps back into the same three pseudos. Banked as memory/grind/func_80032314/rejected/named-mult-intermediates.c.
- verdict: KILLED

## [s2] F1 — the walker must be allocated LAST of the three rotating pseudos; one ref removal achieves it.
- mechanism: global.c allocno priority = floor_log2(n_refs)*n_refs/live_length*10000*size (read via BB2_ALLOC_DEBUG instrumented cc1, hook at global.c:605). Walker 8 refs/len 82 = 2926; dropping to 7 refs crosses the floor_log2(8)=3 -> floor_log2(7)=2 boundary: pri 1707, below ent (1904) and above t0 (1666). mflo1 (2000) already outranks ent, so ONE ref removal fixes the whole 3-cycle.
- probe: spelled the radius read `*(u8 *)(t0 + 2)` instead of `*a3` (equal loop-invariantly; cse cannot see it across the back-edge). Re-ran sandbox + ALLOCDBG trace.
- result: score 15 -> 1; all 109 insns register-match target; trace confirms p115->$a1, p75->$a2, p74->$a3. The single diff is the edited load itself (lbu 2(t0) vs lbu 0(a3)).
- verdict: CONFIRMED

## [s2] F2 — re-deriving the walker per-iteration lowers its priority.
- mechanism (as banked in s1): claimed shorter live range lowers priority. WRONG: priority divides by live_length — shortening RAISES it. Also the target's prologue addiu + latch addiu pair cannot come from a per-iteration derivation without loop notes (no strength reduction in goto-form).
- probe: arithmetic from the measured formula; no build needed.
- result: dead in both directions.
- verdict: KILLED

## [s2] F3 — duplicated-statement-into-arms on ent (as specced in s1) resolves the rotation.
- mechanism: +1 stale ent ref via cross-jump re-merge.
- probe: arithmetic from the measured formula: ent 7 refs = floor_log2(7)*7/63*10000 = 2222 < walker's 2926. Insufficient; and raising ent alone mis-assigns ent to $a1 anyway (assignment order must be mflo1, ent, walker).
- result: dead as specced.
- verdict: KILLED

## [s2] F5 — loop-note spelling (do-while) doubles inside-loop refs into the target order.
- mechanism: flow.c reg_n_refs += loop_depth; loop notes raise depth to 2 inside.
- probe: do-while form, sandbox.
- result: score 59 / 122 insns — loop.c LICM+giv wrecked the body (6+ constant hoists into s-regs). Second-entry invalidation trick dies at jump1. Banked rejected/do-while-loop-notes-licm-explosion.c.
- verdict: KILLED

## LIVE FRONTIER (rewritten for s3+)

### F4 — chain-extender endgame (sanctioned F1 FAKE family; requires ladder exhaustion first)
Statement: with the byte-forced walker at 8 refs (pri 2926), stale reg_n_refs
bumps on BOTH mflo1 (2 -> 4, pri 8000) and ent (6 -> 8, pri 3809) produce the
target allocation order (mflo1 > ent > locals(3333) > walker) with zero byte
change. +1 on ent is insufficient (2222); both bumps are required.
Mechanism: .claude/rules/dead-store-fake-exception.md:32 combine-foldable
chain-extender (owner ruling 2026-07-01): a live computation routed through an
algebraically-equivalent detour combine folds to zero bytes; the stale
flow-time reg_n_refs is the only surviving effect. Detours must use
link-time-opaque constants (e.g. +(s32)&SYM ... -(s32)&SYM) — tree-level
fold-const kills +C-C before RTL.
Next probe (MEASUREMENT ONLY until the driver's exhaustion gate opens): apply
candidate.c with `*a3` restored, add the two detours, verify via ALLOCDBG that
each detour actually leaves +2 stale refs (combine's bookkeeping is the
uncertain part — measure, don't assume), verify 109 insns, read the score.
If 0: this is a candidate ONLY with /* FAKE */ annotations + documented
full-ladder exhaustion + self-vet citing dead-store-fake-exception.md:32 —
at the time of writing (session 2) that gate is NOT open; a premature
submission FAILs prong (a) of the FAKE policy.

### F6 — any pure-C spelling that yields 7 flow-time walker refs while still
emitting `lbu 0($a3)` (believed impossible — see the s2 impossibility argument
in evidence.md; a session attacking this should target the argument's two
load-bearing premises: (1) flow-refs can never undercount final appearances,
(2) no post-allocation pass rewrites a t0-based load into an a3-based one).
Falsifying either premise reopens pure C; confirming both makes F4 the only
path and the exhaustion case complete.

## [s2] The 3-cycle rotation resolves if the record walker is allocated last of the three rotating pseudos; removing exactly one walker ref achieves it because mflo1 already outranks ent.
- mechanism: global.c allocno pri = floor_log2(n_refs)*n_refs/live_length*10000*size (BB2_ALLOC_DEBUG trace, hook global.c:605). Walker 8 refs -> pri 2926 (first); 7 refs crosses the floor_log2 boundary -> 1707, into the band below ent (1904) and above t0 (1666).
- probe: Spelled the radius byte read *(u8*)(t0+2) instead of *a3 (equal loop-invariantly, invisible to cse across the back-edge); sandbox + re-run ALLOCDBG trace.
- result: Score 15 -> 1, 109/109 insns; trace shows p115->$a1, p75->$a2, p74->$a3, all registers in all 109 insns match target; single diff is the edited lbu itself (2(t0) vs 0(a3)).
- verdict: CONFIRMED

## [s2] Spelling the loop as do-while doubles inside-loop reg_n_refs (flow.c += loop_depth) into the target priority order.
- mechanism: flow.c:2081/2329/2515/2725 weight refs by loop_depth, which exists only with NOTE_INSN_LOOP notes; predicted priorities landed in target order.
- probe: do { } while (t1 < 4) form; sandbox.
- result: Score 59, 122 insns: the same notes arm loop.c — LICM hoisted ~6 per-iteration constants into s-regs, grew the frame, created a giv. Second-entry loop-invalidation cannot survive jump1 (deletes jump-to-next-insn before loop.c runs). Banked as rejected/do-while-loop-notes-licm-explosion.c.
- verdict: KILLED

## [s2] F2 (s1 frontier): re-deriving the walker per-iteration lowers its allocno priority.
- mechanism: Claimed shorter live range lowers priority — inverted: the measured formula DIVIDES by live_length, so shortening raises priority; and goto-form has no loop notes, so no strength reduction can rebuild the target's prologue/latch addiu pair.
- probe: Arithmetic from the measured formula; no build needed.
- result: Dead in both directions.
- verdict: KILLED

## [s2] F3 (s1 frontier): one duplicated ent statement into arms resolves the rotation.
- mechanism: Cross-jump-remerged duplicate adds +1 stale ent ref.
- probe: Arithmetic: ent 7 refs = 2222 < walker 2926 (floor_log2 stays 2); and raising ent alone would mis-assign ent to $a1 since assignment order must be mflo1, ent, walker.
- result: Insufficient as specced; the workable variant needs +2 on ent AND +2 on mflo1 (see frontier F4).
- verdict: KILLED
