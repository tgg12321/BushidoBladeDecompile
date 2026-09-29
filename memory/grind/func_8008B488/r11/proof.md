# func_8008B488 — Ruling 11 submission for `temp` (manual s4, 2026-09-28)

Ruling: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner, 2026-09-26). Its "What
stays banned" paragraph names this function: "func_8008B488 `rate` ... can return only as a fresh
submission that meets every prong, and nothing here pre-decides one." This is that fresh
submission. It is **not** the 2026-09-25 body: that one shared one function-scope `u16 rate`
across all five ADSR blocks (AR, DR, SR, RR, SL) and one function-scope `adsr` across five blocks.
This body keeps `adsr` block-scoped (one per block, as in the old landable candidate) and keeps
AR, DR and RR one variable per value. Only two values share, SR and SL, because the measurement
below shows that is the smallest sharing that reaches the target, and that no one-variable-per-value
spelling does. SOTN's `var_a2` counts for nothing under Ruling 11 and is not relied on.

Why no earlier ruling admits `temp`: Ruling 5 fails 1(a) (the SR value feeds the ADSR2 register
halfword at `(pos + 5) * 2`, the SL value the ADSR1 halfword at `(pos + 4) * 2`, two consumers) and
1(b) (the statements differ in the field read and the clamp constants, not in one selector
subscript). It is not a record pointer (Ruling 6). Ruling 8 is vmNoiseOn's. The two values are
two different quantities, not one meaning at constant offsets (Ruling 9). There is no verified
original source (Ruling 10, whose scope text names this function's LIBSPU body as unverified).
So Ruling 11 governs.

Files (memory/grind/func_8008B488/r11/):
- `final.c` — the submitted body, byte-identical to what is spliced into src/main.c.
- `one-var-per-value-form.c` — the (C)(1) spelling.
- `dumps.txt` — (D)(1) excerpts for both spellings and for the escape variants in (D)(3).
- `all.log` — every sandbox-equivalent score quoted below.
- `../rejected/`: `one-var-per-value-12.c` (= the (C)(1) form), `sr-fake-chain-extender-sr-only-4.c`,
  `sl-chain-extender-loopwide-s5-18.c`, `sl-fake-write-chain-read-in-sr-12.c`,
  `sl-fake-write-cross-block-5.c`, `arsl-share-plus-sr-fake-chain-0.c` (scores with the chassis).
- Scripts (banked copies of tmp/f8b488s4/ and of the standalone scorer tmp/f8b488s2/{score.py,
  pp.sh, compile.sh, head.h, mkperm.sh}; they run from the repo root under WSL and read/write
  tmp/f8b488s4/ and tmp/f8b488s2/): `all.sh` regenerates (partitions from `base-s3-candidate.c`, the s3 landable body at 9b95aa8aa)
  everything (`gen_part.py` partitions, `mkfinal.py`, `mkfam.py`, `mkfam2.py`, `mkalt.py`, then
  `dumps.sh`, `dumps2.sh`, `dumpesc.sh`/`dumpvar.sh`, `collect.sh`); `mk.py` splices a body into a
  scratch copy of src/main.c (landing chassis applied: the two hand-transcribed jtbl consts
  deleted) and dumps every RTL pass; `pmap.py` derives which pseudo is which from each
  spelling's own f.lreg. The standalone scorer is tmp/f8b488s2/score.py (head.h + body, the
  build's cc1 | prologue_fix | maspsx | multu_pad | as pipeline, objdump diff vs the target).
  Its counts include **+1** for the jtbl `%lo` addend that the landing chassis removes; the engine
  sandbox agrees (final.c: engine sandbox 4 on the clean tree = the two jtbl addend hunks, 0
  source-level; 0 with the chassis, see "Landing" in evidence.md).

## The variable and its values

```c
for (voice = 0; voice < 24; voice++) {
    u16 temp;                      /* declared in the loop body */
    ...
    if (bSetAll || (mask & 0x2000)) {           /* SR block */
        ...
        temp = attr->sr;           /* W1 */
        if (temp >= 0x80) {        /* R1 */
            temp = 0x7F;           /* W2 */
        }
        smode = 0x100; ... switch (attr->s_mode) ...
        adsr = RXX[pos + 5]; adsr &= 0x3F;
        RXX[pos + 5] = adsr | ((temp | smode) << 6);    /* R2 */
    }
    ... RR block (its own rr_rate) ...
    if (bSetAll || (mask & 0x8000)) {           /* SL block */
        ...
        temp = attr->sl;           /* W3 */
        if (temp >= 0x10) {        /* R3 */
            temp = 0xF;            /* W4 */
        }
        adsr = RXX[pos + 4];
        RXX[pos + 4] = (adsr & 0xFFF0) | temp;          /* R4 */
    }
}
```

- Value 1, the clamped **sustain rate**: writes {W1, W2}, read by R1 (W1 only) and R2 (W1 or W2).
- Value 2, the clamped **sustain level**: writes {W3, W4}, read by R3 (W3 only) and R4 (W3 or W4).
- No read can reach both values: W3 dominates R3 and R4 inside the SL block, and W1 dominates R1
  and R2 inside the SR block. The variable's scope is the loop body, so nothing carries across
  iterations. Two values.

Target bytes (asm/funcs/func_8008B488.s): W1 `lhu a1,0x34(s0)` 0x8008B898, W2 `addiu a1,zero,0x7F`
0x8008B8AC, R2 `or v0,a1,a2` 0x8008B930 (with `andi a0,v0,0x3F` at 0x8008B92C: a0 is busy while
the value is live); W3 `lhu a1,0x38(s0)` 0x8008B9D0, W4 `addiu a1,zero,0xF` 0x8008B9E4, R4
`or v0,a1,v0` 0x8008BA04. Both values sit in a1; smode sits in a2 (0x8008B8B4, 0x8008B904 ff.).

## (A) A fresh local, not a borrow
`u16 temp` is a local of this function, declared once, at the top of the `for` body. W1-W4 are in
two sibling `if` blocks of that body, so the loop body is the innermost scope enclosing all of its
writes. No other declaration moved or was re-scoped relative to the (C)(1) form (the diff is `temp`'s
declaration, the two per-value declarations it replaces, and the identifiers). No `&temp`. Not a
parameter, global, `static` or `register` variable.

## (B) Every write is live
(1) W1 is read by R1 on every path and by R2 when W2 does not run. W2 is read by R2. W3 is read by
R3, and by R4 when W4 does not run. W4 is read by R4. No write is dead.
(2) Re-store check (Ruling 5 2(c) with its 2026-09-26 clarification):
- W1 (`= attr->sr`) is the first write to `temp` in its block's lifetime on every path: `temp` is
  declared in the loop body, so each iteration starts with it indeterminate, and nothing writes it
  before the SR block. It never re-stores a held value. It is a load of `attr->sr`, which no other
  statement in the iteration loads.
- W2 (`= 0x7F`) runs only when `temp >= 0x80`, so `temp` holds a different value on every
  incoming path.
- W3 (`= attr->sl`): incoming paths are (i) SR block not taken, where `temp` is indeterminate, and
  (ii) SR block taken, where `temp` holds min(attr->sr, 0x7F). Record of a feasible path where it
  holds a different value: `attr->mask == 0` (bSetAll), `attr->sr == 5`, `attr->sl == 3`: `temp`
  holds 5 and W3 stores 3. It is the only load of `attr->sl` in the iteration.
- W4 (`= 0xF`) runs only when `temp >= 0x10`.

## (C) Same statements; real computations
(1) `one-var-per-value-form.c`: value 1 in `u16 sr_rate`, declared at the top of the SR block
(`if (bSetAll || (mask & 0x2000)) {`), which encloses W1 and W2; value 2 in `u16 sl_rate`, declared
at the top of the SL block (`if (bSetAll || (mask & 0x8000)) {`), which encloses W3 and W4.
(2) `diff one-var-per-value-form.c final.c`: `temp`'s declaration with its comment added to the
loop body, the two block declarations removed, and `sr_rate`/`sl_rate` renamed at W1-W4 and
R1-R4. No statement added or removed.
(3) Each value has a load in the target's bytes: `lhu a1,0x34(s0)` (value 1) and
`lhu a1,0x38(s0)` (value 2), plus the clamp compares `sltiu v0,a1,0x80` / `sltiu v0,a1,0x10`.
Neither value is constant-only or a bare copy.

## (D) Allocator-dump proof of necessity

### (D)(1) Dumps — `dumps.txt`
Both spellings, spliced into a scratch copy of src/main.c with the landing chassis, compiled with
the build compiler `tools/gcc-2.7.2/build/cc1` and the build flags (engine.buildconfig CC_FLAGS:
`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
-msoft-float`, plus `-da`), and with the instrumented `tools/gcc-2.7.2/cc1` (`BB2_ALLOC_DEBUG=1`,
`BB2_FINDREG_DEBUG=<pseudo>`). For every dumped body the instrumented cc1's func_8008B488 asm is
identical to the build cc1's (`identity:` / `stock==dbg asm` lines). Pseudo numbers come from
`pmap.py` (writes classified from each spelling's own f.lreg), not assumed.

| value | final (reuse) | split (one var per value) |
|---|---|---|
| SR rate + SL level | pseudo 80: `used 16 times across 45 insns; dies in 2 places`; ALLOCDBG `ord=13 hardreg=5 nrefs=16 livelen=41 pri=15609`; FINDREG conflicts `2 3 4 29` → **$a1** | — |
| SR rate | (pseudo 80) | pseudo 250: `used 8 times across 32 insns`; `ord=19 hardreg=6 nrefs=8 livelen=31 pri=7741`; conflicts `2 3 4 5 16 29` → $a2 |
| SL level | (pseudo 80) | pseudo 306: `used 8 times across 13 insns`; `ord=10 hardreg=4 nrefs=8 livelen=10 pri=24000`; conflicts `2 3 29` → $a0 |
| smode | pseudo 251: `used 10 times across 27 insns`; `ord=16 hardreg=6 pri=11111`; conflicts `2 3 4 5 29` → $a2 | pseudo 251: `ord=16 hardreg=5 pri=11111`; conflicts `2 3 4 29` → $a1 |

In every trace `someone_prefers`, `own_copy_prefs` and `own_full_prefs` are empty. None of these
pseudos is a local-alloc quantity (each value's clamp puts it in more than one basic block; no
`;; Register N in` line in f.lreg), and none crosses a call. Final asm: `lhu $5,52($16)`,
`li $5,0x7f`, `li $6,0x100`, `or $2,$5,$6`, `lhu $5,56($16)`, `li $5,0xf`, `or $2,$5,$2` — the
target's seats. Split asm: sr_rate in $6 / smode in $5 (the 8 SR hunks), sl_rate in $4 (the 4 SL
hunks).

### (D)(2) The mechanism — two decisions of global.c, both named
1. **Allocation order** (global.c:575 `qsort (allocno_order, ..., allocno_compare)`;
   allocno_compare, global.c:635-655: priority = floor_log2(n_refs) × n_refs / live_length × 10000
   × size). The SR value and smode conflict with each other and with nothing but v0/v1/a0/sp at
   the time they are allocated (conflicts above), so whichever is allocated first takes $a1 and the
   other $a2. Final: pseudo 80 has n_refs 16 (the SR refs plus the SL refs; flow counts each
   reference loop-depth-weighted, flow.c:2081/2329/2515/2725 `reg_n_refs[regno] += loop_depth`)
   over live length 41 → 4 × 16 / 41 → 15609 > smode's 3 × 10 / 27 → 11111: ord 13 before ord 16,
   so the SR value gets $a1 and smode $a2 (target). Split: sr_rate has n_refs 8 over 31 → 7741 <
   11111: smode first takes $a1, sr_rate $a2.
2. **find_reg's register choice** (global.c:952-1098). With no preferences, pass 0 takes the
   lowest-numbered register (MIPS defines no REG_ALLOC_ORDER) that is not in `used1` (fixed
   registers, the class complement, and `hard_reg_conflicts[allocno]`: the hard registers of
   allocnos already allocated that are live at some point where this allocno is live) and is in
   `regs_used_so_far`. Final: pseudo 80 is live in the SR block, where $a0 holds the masked adsr
   (`andi $4,$2,0x3f`, allocated earlier) while the value is live, so $a0 is in its conflicts
   (`2 3 4 29`) and it gets $a1 — for both values, since one pseudo gets one hard register
   (2.7.2's global-alloc does not split live ranges). Split: pseudo 306 (SL) is live only inside
   the SL block, where only the v0/v1 temporaries are live (conflicts `2 3 29`); $a0 is free, so
   it gets $a0 — the target's $a1 is never reached.

### (D)(3) Necessity, not effect
**The property of the reuse spelling:** the SL value's pseudo is also live in a block where $a0 is
held (the SR block) without being live across any call, so find_reg excludes $a0 and seats it in
$a1. (That the same pseudo also lifts the SR value's priority above smode's is the second effect;
see "The SR half" below for why it is not claimed as necessary on its own.)

**Every one-variable-per-value spelling lacks it, because the SL value has its own variable.**
Its writes and reads are fixed statements of the SL block ((C)(2): no extra reads or writes), and
the target's bytes put them there (W3 at 0x8008B9D0 through R4 at 0x8008BA04, with the clamp's
branch `bnez v0` at 0x8008B9DC making it multi-block). So its pseudo is live from W3 to R4 inside
the SL block and nowhere else. The target's SL block (0x8008B9C0-0x8008BA08) writes only v0, v1
and a1 (it reads s0-s3), and no value is live in $a0 across it. Every $a0 reference in the
target is a short set-then-use window: 0x8008B4EC-B4F4, B540-B54C (call argument), B554-B560,
B57C-B640 and B65C-B720 (the two volume blocks), B740 and B760 (call arguments), B784-B78C,
B7B0-B7B8, B824-B830 (AR), B878-B880 (DR), B92C-B938 (SR) and **B9B0-B9B8 (RR: `andi
a0,v0,0xFFC0` then `or v0,a0,v0`), the last one**. Nothing references $a0 from 0x8008B9BC on —
not the SL block, the loop step (`addiu s4,s4,1` / `slti` / `bnez`) or the wait loop — and every
window starts with a write, so $a0 is dead at the loop top and throughout the SL block.
(Correction after layer-2 round 2: the first text said $a0 does not appear after 0x8008B92C,
which is false — the SR store reads it at B938 and the RR block sets and reads it at B9B0/B9B8.)
So nothing holding $a0 is ever live at the same time as a per-value SL
pseudo, $a0 is never in its conflicts, no conflicting allocno prefers $a0 (the SL block has no
call and no hard-register copy, so no preference exists to propagate — `someone_prefers` empty),
and find_reg's pass 0 always reaches $a0 before $a1. Declaration scope (block, loop body,
function), declaration order and type change none of this: measured scope_func 13, scope_loop 13,
scope_loop_rev 13, type_s32 13, type_u32 13 (type_u8 19 / 389 insns and type_s16 29 add
extensions the target does not have).

**The sanctioned families cannot give a per-value SL pseudo that property either** (Ruling 11 (D)(3)
says "whatever ... other respelling"; each family was measured on the split body and dumped where
it could matter — `fam/`, `fam2/`, dumps.txt "escape variants"):
- *Self-assignment* (`sl_rate = sl_rate;` after the clamp, and in the SR block with a
  function-scope `sl_rate`) and *dead store* (`sl_rate = 0;` before W3, and in the SR block): all
  13, and the dumps show the SL pseudo unchanged (slself: 306 `used 8 times across 13 insns`,
  conflicts `2 3 29`; sldeadsr: 77, same): neither the no-op move nor the dead set reaches
  flow's count or global_conflicts.
- *Chain-extender with a constant base* (`0x3F + sl_rate - sl_rate` in the SR/DR/RR blocks,
  `1 + sl_rate - sl_rate` after the loop): all 13; folded before flow (slchsr: pseudo 77 `used 8
  times across 13 insns`, conflicts `2 3 29`, $a0).
- *Chain-extender with a non-constant base*, which survives to flow and is folded by combine
  (`(x) + sl_rate - sl_rate` in the AR/DR/SR/RR stores, the adsr2 store, the pitch store, `pos`,
  the loop step, the tail; `fam2/`): 19-73. These are the escapes that do extend the pseudo — and
  the only way a READ alone can: a read of the SL variable outside the SL block that no write in
  the same iteration reaches is reached, on the path that skips the (conditional) SL block, from
  the previous iteration, so the pseudo becomes live around the loop's back edge through the whole
  body (sl2sr: pseudo 77 `used 12 times across 319 insns; dies in 0 places; crosses 3 calls`,
  conflicts `2 3 4 5 6 7 16 17 18 19 20 29`). A call-crossing allocno gets `used1 =
  call_used_reg_set` (global.c:970-975), which excludes $a1, and it lands in $s5 (+1 saved
  register, frame 0x30 → 0x38). Every block other than SL precedes SL in the iteration, and
  everything after SL is the loop step / exit, which the skip path reaches too.
- *A same-iteration FAKE write plus a chain-extender read in another block* (layer-2 round 1
  objection; `fam3/`): `sl_rate = <x>; /* FAKE */` in the SR block after the `adsr &= 0x3F` mask
  (x = `attr->sl`, `sr_rate`, `smode`, `adsr`, `pos`, a volatile RXX read), or before the mask,
  or in the DR / RR block, followed by `... + sl_rate - sl_rate` in that block's store, with
  `sl_rate` declared in the loop body: 13-14 alone; with the SR chain-extender 5-6, except the
  two copy writes `sl_rate = smode` / `sl_rate = sr_rate` (13: cse propagates the copy and the SR
  chain-extender folds with it) — the 4 SL hunks remain in every one, 387 insns. (Correction
  after layer-2 round 2: the range was first given as "5-6" for all of them.) This does make the per-value SL pseudo live in the SR
  block **at flow** (f3load: `Register 80 used 14 times across 17 insns; dies in 2 places`; the
  pseudo appears in f.flow insns 843, 860, 864 in the SR block and 996-1038 in the SL block).
  But combine then substitutes the FAKE write into the chain-extender and folds the pair away —
  that zero-byte fold is what makes it a chain-extender at all. In f.combine the setting insn 843
  is gone; what remains of the pseudo in the SR block is insn 1139, a bare
  `(use (reg/v:HI 80))` carrying a REG_DEAD note that combine left in place of the folded
  read; every other reference is SL-block insns 996, 1000, 1005, 1038. global_conflicts
  (global.c:662 ff.), which runs inside global_alloc (toplev.c:3080) after combine
  (toplev.c:3004), marks an allocno live only from `basic_block_live_at_start` (global.c:693)
  or at a store (`note_stores (..., mark_reg_store)`, global.c:777); a REG_DEAD note only
  clears (global.c:770), and a USE is not a store. Pseudo 80 is in the live-at-start set of
  exactly one block, 107 (the SL block's clamp join: "Registers live at start: 29 30 72 73 74
  75 76 80"), so it is never live anywhere in the SR block and the SR-block segment
  contributes no conflict:
  FINDREG pseudo 80 `conflicts: 2 3 29` → $a0 in all five dumped variants (f3load, f3srrate,
  f3rxx, f3early, f3dr; dumps.txt "fam3"). The flow-time refs survive (nrefs 14, pri 38181) but
  only move its allocation order, and the SL seat is decided by the conflict set.
- *The FAKE write and the chain-extender read in DIFFERENT basic blocks* (layer-2 round 2
  objection: combine cannot fold a write into a read in another block; `fam4/`, r11/mkfam4.py,
  dumps.txt "fam4"). Writes: `attr->sl`, `pos`, `adsr` (computations) and copies of another
  block's value (`sr_rate`, `rr_rate`, `ar_rate`); placements: after the SR clamp with the read in
  the SR store (the smode switch's blocks between), after the SR mask with the read in the RR
  store, after the AR clamp with the read in the DR store, after the RR clamp with the read in
  the RR store — each alone and with the SR chain-extender, 20 variants: 6-29, none below the SL
  residual. What happens, from the dumps:
  - The read still folds (it is `(x + sl) - sl` inside one insn chain of its own block), and
    combine leaves the pseudo's death as a bare `(use (reg/v:HI 80))` with REG_DEAD at the START
    of the read's block: f4pos f.combine insn 1147 sits directly after code_label 828, before the
    `adsr & 0x3F` insn 843 that births the masked adsr the target keeps in $a0. So the pseudo is
    dead before $a0 is set in that block.
  - Between the write and that block it is live (f4pos: live at start of blocks 82-91, the smode
    switch, and of the store block 92, whose first insn is that death USE), but the target holds nothing in $a0 there (0x8008B8C4-B914 references v0, v1, a2
    only; the $a0 windows are listed above), so no $a0 allocno is live with it: f4pos, f4load,
    f4srcopy, f4rrcopy `conflicts: 2 3 29` → $a0.
  - The write itself is not folded away, so it is emitted: `lhu a0,56(s0)` (f4load, 388 insns),
    `move a0,a1` (f4srcopy / f4rrcopy, 388), `move a0,s3` in place of the target's delay-slot
    `nop` at index 264 (f4pos, 387 insns but a differing instruction). None of these is in the
    target.
  - A write before the RR block with the read in the RR store (f4tork) leaves the pseudo live
    across the RR guard, where the skip path joins: `used 14 times across 289 insns; dies in 0
    places; crosses 3 calls`, conflicts `2 3 4 5 6 7 16 17 18 19 20 29` → $s5, frame 0x30 → 0x38.
  **In general**, for a per-value SL pseudo to be seated in $a1 its conflicts must contain $a0,
  so it must be live where the target holds $a0 (one of the windows above, all before
  0x8008B9BC) without crossing a call and without an allocno already in $a1 being live with it.
  Any extra write outside the SL block that is still present at global_alloc is emitted as an
  instruction unless it is a register copy whose source already sits in the same hard register
  and dies there (final.c:1796-1806 skips same-REGNO sets; jump2 deletes no-op moves,
  jump.c:451-456). For the SL pseudo in $a1 that source is a value the target holds in $a1 at
  that point — in the target's $a0 windows above, $a1 is either unreferenced or holds a value
  of that same block (the note-pitch operand at B540, the left/right volume at B640/B720, the
  AR/DR/SR/RR rate at B830/B880/B938/B9B8). The SL variable would then
  hold a copy of that value. **Under Ruling 11's own definition of a value (a group of writes
  reaching a common read), every such FAKE write — copy or not — is a second value of the SL
  variable**: the chain-extender read it reaches is one the SL-block writes can never reach
  (they come later in the iteration, and the variable's scope is the iteration). So every
  write+read escape is a two-value SL variable, i.e. a reuse spelling carrying FAKE constructs
  on top, not a one-variable-per-value spelling; when the write copies another block's value it
  is exactly the permuter's re-share (`sl_rate = center`) plus a detour. Those are measured here
  only to show that the extra FAKE machinery buys nothing: SL moves off $a0 only when its
  variable genuinely shares a value the target holds in $a1 (the permuter's re-shares move it;
  the reuse this submission makes, with the SR value, closes it).
- *Duplicated into arms* (if/else clamp, `sl_ifelse_arms`, and `sr_ifelse_arms`): 14, 388 insns —
  the arms' writes are not re-merged (cross-jump leaves the extra insn), and the value is still
  written and read only inside the SL block, so the argument above applies unchanged.
- *do { } while (0)* around the SR load/clamp: 13. *Pointer alias* (`sl_p = &sl_rate;` read
  through the pointer; likewise for SR): 34 / 36, 388 insns — taking the address sends the
  variable to the stack.
- *Hoisting/sinking*: moving W3 or R4 out of the SL block changes where the target's `lhu
  a1,0x38(s0)` / `or v0,a1,v0` sit, so it cannot reproduce the bytes (and is not a (C)(2)
  spelling).

**The SR half — disclosed, and why the SR value is the SL value's partner.** The SR seat alone is
NOT unreachable per value: a sanctioned combine-foldable chain-extender on the SR value
(`(((sr_rate | smode) + sr_rate - sr_rate) << 6)`, fam/sr_chain_ior.c) lifts sr_rate to n_refs 12,
priority 11612 > 11111, and gives it $a1 (dumps.txt `[srch]`: 250 `used 12 times across 34 insns`,
`ord=15 hardreg=5`), leaving only the 4 SL hunks (score 5 = 4 + addend). So the necessity proven
above is the SL value's: **the SL value must share a pseudo with a value of another block**. The
partitions sweep (all 52 groupings of {AR, DR, SR, RR, SL}, all.log) shows which partners work:
- SL alone → every grouping that leaves SL alone scores 5 (SR shares with something: the 4 SL
  hunks remain) or 13, never 1.
- {DR, SL} → 19.
- {AR, SL} → 9, {RR, SL} → 9: SL fixed, the 8 SR hunks remain; they close only with the FAKE
  chain-extender added on sr_rate (alt/arsl_srchain.c, alt/rrsl_srchain.c: 1 = the addend).
- **{SR, SL} → 1 (the addend only), with no other construct.**
So every byte-exact form known shares the SL value with another block's value. Byte-exact forms
with no FAKE construct exist with two shared variables (all.log: e.g. {AR,SL}+{SR,RR},
{AR,SL}+{DR,SR}, {AR,SR}+{RR,SL}, {DR,SR}+{RR,SL} all score 1); **{SR, SL} is the only
byte-exact form that uses a single two-value variable and no FAKE construct** — one reused
variable instead of two, each of which would need its own Ruling 11 admission.
(Correction after layer-2 round 1: the first text said "the only one that needs no FAKE
construct", which all.log contradicts.) Ruling 1 (4) (simplest-known-form: "when multiple
byte-exact forms are known, the one with the fewest no-semantic-purpose constructs lands") and the
dead-store-fake-exception's last-resort prerequisite both select it. The partner's second effect
(its SL references lift its priority over smode) is what closes SR in that form.

### (D)(4) Measured alternatives
Scores are the standalone scorer's (+1 addend) unless marked engine; all.log has every line.
- Full one-variable-per-value spelling (`one-var-per-value-form.c`): **13** (12 + addend). Engine
  sandbox `--disable all` on the clean tree: 16 (0 source-level, 11 operand-only hunks: the 12
  seat instructions plus the jtbl addends), 387 insns; final.c on the same tree: 4 (0
  source-level, 2 operand-only: the two jtbl addends).
- Ablation: the variable holds two values, so (D)(4)'s per-value ablation is the full split above.
  The wider partition sweep (52 groupings) is in all.log; its minimum is 1, reached by 24
  groupings, every one of which shares SL with another value and has SR sharing too (with SL or
  with others).
- Structural respellings of the split body: SR/SL if/else clamps 14; do-while(0) 13; function-scope
  and loop-body declarations in either order 13; s32/u32 13, u8 19, s16 29. From earlier sessions
  (same body, evidence.md s1-s3): ternary 12+, `a < b ? a : 0x7F` 13, inline clamp helper 16/21,
  block-local load + if/else copy 14, every SR/SL statement-order permutation 16-29 (s3 sweep).
- Permuter from the one-var body (tools/permuter_campaign.py, `--stack-diffs`, standalone
  workspace built by r11/mkperm.sh; permuter base score 65 = standalone 13). The workspaces were
  built from codegen-identical earlier revisions of the split body (with the no-op `(s16)` casts
  and/or the `sp10`/`sp14` names; asm identical, checked by the scorer):
  - Campaign 1 (tmp/perm_b488_split, block-scoped per-value locals, -j4): 27,395 iterations,
    1,318 s, **0 finds**.
  - Campaign 2 (tmp/perm_b488_split2, fresh seed, same body, -j6): 34,808 iterations, 1,275 s,
    **0 finds**.
  - Block scope stops the permuter from moving a per-value local into another block, so
    Campaign 3 starts from the function-scope per-value spelling (alt/scope_func.c, standalone 13):
    tmp/perm_b488_fscope, fresh seed, -j6, 36,712 iterations, 1,266 s, **6 finds, best 20**.
    What they do (read in full): every find writes `sr_rate` or `sl_rate` with another block's
    value — `sr_rate = note & 0xFF` (20), `sr_rate = attr->volume.left & 0x7FFF` (30),
    `sr_rate = vol_left` (30), `sl_rate = center` (45, the same find s2's campaign made),
    `sl_rate = attr->note` (65), `sr_rate = note` (65). Each is a reuse (the per-value variable
    holds a note- or volume-block value too), which is the (D)(3) property; none keeps one value
    per variable, and none reaches 0.
  - Across the three campaigns (~98,900 iterations) no one-value-per-variable find scores below
    the base.

## (E) An honest generic name
`temp` — form (i) of Ruling 11 (E), a generic scratch word claiming nothing about either value.
(Form (ii) was considered — both values are sustain-phase envelope parameters — and not used, so
that nothing hangs on a judgment about "same kind".) Codegen is name-independent.

## (F) Annotation
The declaration carries: `/* two values: the clamped sustain rate (SR block), then the clamped
sustain level (SL block); Ruling 11, proof in memory/grind/func_8008B488/r11/proof.md */`.

## (G) Layer-2
Required: a fresh cheat-reviewer, default-FAIL, walking (A)-(H) with this file and dumps.txt.

## (H) Everything else
The rest of the body is the landable candidate banked 2026-09-25 (memory/grind/func_8008B488/
candidate.c history), unchanged except:
- AR, DR and RR keep one clamp local each (`ar_rate`, `dr_rate`, `rr_rate`), now declared at the
  top of their own blocks (innermost scope) instead of at function scope — codegen-identical
  (scope_func 13 = split 13 with them at function scope in the old candidate).
- The two `(s16)` casts on the volume-mode switch operands were dropped: SpuVolume's fields are
  already `s16`, and the bytes are identical without them (measured: 1 = 1).
- The wait-loop locals `sp10`/`sp14` were renamed `i`/`v`, the names the same SPU settle-wait
  idiom already carries in src/main.c (`volatile s32 i; volatile s32 v = 0xD; ... v = v * 13;`,
  ~line 2513). Names only; bytes identical.
Other constructs, each ordinary C with its semantic reading:
- `*(volatile u16 *)(_spu_RXX + (reg) * 2)` for every SPU register access (hardware registers;
  without volatile the read-modify-write narrows to `lbu`, 34).
- `adsr = RXX[...]; adsr &= MASK; RXX[...] = adsr | ...;` in AR/DR/SR/RR (s32 `adsr`, one per
  block): the Ruling 4 compound-assignment split, sanctioned as ordinary C; SL keeps the single
  expression `(adsr & 0xFFF0) | temp`.
- `bSetAll = mask == 0;` (the SDK's "mask 0 means set everything"), `pos = voice * 8` (the voice's
  register base), the volume blocks' `vol_* = ... & 0x7FFF; volmode_* = 0; switch ...; clamp`
  (each a single value: all writes reach the one store), `amode`/`smode`/`rmode` (a default then a
  switch: one value each; RR's `case 3: break;` keeps the default — the target compares with 3
  explicitly, 0x8008B984).
- The trailing `v = 1; for (i = 0; i < 2; i++) v *= 13;` over two `volatile s32` stack locals:
  the SPU write-settle wait (the target's `sw/lw 0x10/0x14($sp)` pairs).
- SpuVoiceAttr is PsyQ libspu.h's layout (0x40 bytes; the callers build it in an `s32 buf[16]`).
  The landing chassis (Makefile/buildconfig RODATA_ALIGN2 and the jtbl consts) is described in
  evidence.md "Landing chassis".
