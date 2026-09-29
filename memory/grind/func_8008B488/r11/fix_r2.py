#!/usr/bin/env python3
"""Apply the layer-2 round-2 corrections to tmp/f8b488s4/proof.md (and bank it)."""
import shutil
P = 'tmp/f8b488s4/proof.md'
s = open(P, encoding='utf-8').read()


def one(o, n):
    global s
    assert s.count(o) == 1, o[:80]
    s = s.replace(o, n)


one("""the SL block and nowhere else. The target's SL block (0x8008B9C0-0x8008BA08) writes only v0, v1
and a1 (it reads s0-s3), and no value is live in $a0 across it: $a0 does not appear anywhere after
0x8008B92C (the SR block's `andi a0`) — not in the RR or SL blocks, the loop step
(`addiu s4,s4,1` / `slti` / `bnez`) or the wait loop — and every use of $a0 in the loop body
starts with a write (first: `lhu a0,0x14(s0)` at 0x8008B4EC), so $a0 is dead at the loop top. So nothing holding $a0 is ever live at the same time as a per-value SL
pseudo,""",
"""the SL block and nowhere else. The target's SL block (0x8008B9C0-0x8008BA08) writes only v0, v1
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
pseudo,""")

one("""  `sl_rate` declared in the loop body: 13-14 alone, 5-6 with the SR chain-extender — the 4 SL
  hunks remain in every one, 387 insns.""",
"""  `sl_rate` declared in the loop body: 13-14 alone; with the SR chain-extender 5-6, except the
  two copy writes `sl_rate = smode` / `sl_rate = sr_rate` (13: cse propagates the copy and the SR
  chain-extender folds with it) — the 4 SL hunks remain in every one, 387 insns. (Correction
  after layer-2 round 2: the range was first given as "5-6" for all of them.)""")

one("""  In general: an extra write+read of the SL variable outside the SL block either (i) is removed
  at or before combine (self-assign, dead store, chain-extender — measured above: no conflict
  added), or (ii) survives to global_alloc, in which case it is a real instruction in the
  emitted code; the target's SR/DR/RR blocks contain no instruction beyond the ones their own
  values need, so a surviving write+read must be one of those instructions — the SL variable
  then holds that block's value, a second value, which is the reuse spelling, not a
  one-variable-per-value one.""",
"""- *The FAKE write and the chain-extender read in DIFFERENT basic blocks* (layer-2 round 2
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
  - Between the write and that block it is live (f4pos: live at start of blocks 82-92, the smode
    switch), but the target holds nothing in $a0 there (0x8008B8C4-B914 references v0, v1, a2
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
  the reuse this submission makes, with the SR value, closes it).""")

one("""  `sl-chain-extender-loopwide-s5-18.c`, `sl-fake-write-chain-read-in-sr-12.c`,""",
    """  `sl-chain-extender-loopwide-s5-18.c`, `sl-fake-write-chain-read-in-sr-12.c`,
  `sl-fake-write-cross-block-5.c`,""")

open(P, 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
