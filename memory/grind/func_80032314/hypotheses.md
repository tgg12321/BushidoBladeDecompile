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
