# hirahira_w_ctrl — WIP (opened 2026-08-04)

`src/text1a.c:753` — **not `text1a_c`**, and the honest distance is **27, not
16**; the brief's census line was stale on both counts. Two loops that walk a
coordinate table, negate two of each triple into a 3-entry `s16 buf`, and call
`func_8004A348` / `func_800523E0` / `func_80044DE4` per entry.

**16 regfix rules + 1 `tools/prologue_config.json` entry + a register pin in
source** (`register s32 *s7_a4 asm("s7") = a4;` at line 755 — cheat-asm).
No asmfix / frame_fix entries.

## Where it stands

| | score | frame | insns |
|---|---|---|---|
| committed HEAD | 27 | 72 | 132 |
| **`candidate.c`** | **23** | 72 | 132 |
| target | 0 | 72 (`0x48`) | 132 |

**Frame and instruction count are already correct at HEAD** — the whole gap is
register naming plus commutative-add operand order.

## Two findings (27 → 23)

1. **The register pin is NOT load-bearing.** Replacing
   `register s32 *s7_a4 asm("s7") = a4;` with a plain `s32 *s7_a4 = a4;` scores
   the **same 27**. That cheat-asm can be retired for free, independently of any
   further progress — worth doing on its own.
2. **Reuse `offset` as the pointer carrier for the second address add.** Target
   emits `addu s0,s0,s2` (destination reuses the offset register) where we
   emitted `addu v1,s2,s0`. Writing

   ```c
   offset = offset + (s32) a2;
   p = (u16 *) offset;
   ```

   instead of `p = (u16 *) (a2 + offset);` matches it. **27 → 23.**

Both together: 23. The candidate carries both.

## Measured negative

- The same reuse applied to the **first** add (`a1 + offset`) regresses to 30 —
  only the second one matches target.
- Plain operand-order swaps (`offset + a1`, `offset + a2`, both) are **inert**
  at 27 — GCC canonicalises commutative operands, so the source order alone does
  not reach it; the destination reuse is what matters.
- Dropping the pin combined with any of the above changes nothing versus the
  same form with the pin — consistently confirming it is inert.

## Residual (23) — diff shape

`tmp/sbs.sh` on HEAD showed 28 differing lines of 133, all register naming in
three swapped pairs plus the two adds now addressed:

- `s4`/`s5` swapped across the prologue saves (lines 8-12),
- `s6`/`s7` swapped throughout (lines 22, 24, 54, 58-59, 84, 110),
- `s0`/`s3` swapped in the second loop (lines 72-73, 78, 114-119).

That is three independent 2-cycles, not one rotation — a different shape from
`saTan4FireDisp`'s 3-cycle, and likely three separate allocno ties.

## The operand-order lever does NOT apply here (checked, round 2)

[[compare-operand-order-register]] is the legitimate surface — flipping a
**comparison's** operand order (`local > GLOBAL` instead of `GLOBAL < local`) is
an RTL-emission-order lever with real precedent. **Its precondition is absent in
this function:** all three comparisons in the body are against **constants** —
`if (i < 0x12)`, `if (i < 0x14)`, `while (t1 < 4)`. The rule's own "does not
apply" list excludes constant-RHS comparisons, and `func_8007C7A0` round-13
re-confirmed that empirically. There is no global-vs-local compare to flip.

The other axis — permuting operands *within* a commutative `|` / `&` / `+`
expression — is [[or-tree-shape-shift]], **FORBIDDEN**. It was also measured
inert here anyway (`offset + a1` / `offset + a2` / both all score 27).

Note the kept lever is neither of those: `offset = offset + (s32) a2;
p = (u16 *) offset;` is a **destination reuse** of an existing live variable
(the sanctioned variable-reuse family), not an operand permutation. `offset`
genuinely holds the computed byte address afterwards.

## Resume here

Start from `candidate.c` (23), which already carries the pin retirement. The
residual is purely the three 2-cycle allocno ties, so the next step is RA
Step-0: `tmp/allocdbg.sh text1a` + `tmp/allocpick.py` (adjust its `WANT` set to
this function's pseudos) gives the allocno priorities directly, and the
`saTan4FireDisp` round-10 write-up shows how to turn them into a sized lever
rather than guessing. Do not re-sweep operand orders — both axes are settled
above.

**CORRECTION (operator, 2026-08-04 ~08:20): the pin retirement is NOT a free
commit.** It is inert under the HONEST sandbox metric, but with the 16 regfix
rules ENABLED the emission shifts and the full build MISMATCHES (measured:
SHA1 8e1160d1, reverted, oracle re-verified green). Same gate as the display
twins: rules are calibrated to the pinned emission. The pin retires only as
part of the COMPLETION (when all rules go with it).

## Sibling

`hirahira_w_ctrl_2` (`text1a_c`, dist 58, 63 rules, verdict **ASM-SUSPECT**) was
not examined — the ASM-SUSPECT verdict means it needs the `canonical` gate run
before any pure-C work, not a diff reading.

## Instruments

`tmp/hw.py`, `tmp/hw2.py` (sweeps), `tmp/hw_bank.py`, `tmp/sbs.sh`,
`tmp/frame_probe.sh`, `tmp/allocdbg.sh`, `tmp/allocpick.py`.
