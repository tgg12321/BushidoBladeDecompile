# damage_DebugDisp WIP

## TL;DR (2026-06-13, session 1)

Floor lowered 9 → 8 via the user-sanctioned **loop-exit-work-inside-loop-sched-fence**
respelling on the 4-block `CopyBlock` copy loop in the if-skip-able copy block. The
`for(;;)/continue/break` with the tail-word copy INSIDE the loop and `k = 0;` OUTSIDE
puts the `NOTE_INSN_LOOP_END` mid-block, pinning the `k=0` after the `sw` and
materialising the load-delay nop on the trailing-word `lw`. Retires the two trailing-
region rules (`insert "nop" @ 53`, `reorder 55,54 @ 54-55`). Cheat-reviewer PASS.

The remaining 8 diffs are TWO independent register-allocation / scheduling problems
in OTHER regions of the function — both untouched this session, both already-known
shape, neither closed by any clean lever found here.

## Resume (apply candidate, then iterate)

```pwsh
cp memory/wip/damage_DebugDisp/candidate.c /tmp/cand.c
# splice into src/code6cac_c_mid.c lines 249-322 (or just diff in the for-loop fence)
& tools/eng.ps1 sandbox damage_DebugDisp --disable all   # expect score 8
```

## Remaining gap (8 diffs)

### Inner sum-loop $a0/$a1 swap (3 diffs)

Build: `$a0 = j` (loop counter), `$a1 = sum` (accumulator), `$v1 = bp`.
Target: `$a0 = sum`, `$a1 = j`, `$v1 = bp` — same regs used, $a0/$a1 swapped.

The 3 diffs are at `addiu`/`addu`/`sltiu` in the inner loop body plus the `beq` post-
loop comparison.

GCC's local-alloc tiebreaker assigns the lower-pseudo-number first. `j = 0;` is in
source order BEFORE `sum = 0;`, so j gets the lower pseudo, allocated to $a0 first.
Reordering source to put `sum = 0;` first (V11) rotated multiple regs (sum→$a1,
bp→$a0, j→$v1) instead of just flipping $a0/$a1 — score went 8→9.

### Constants-vs-moves order in second-loop preheader (~5 diffs)

Build:
```
move a0,t1      (ap = base)
move a2,a0      (a2p = ap)
lui  t0,0x8000  (const 0x80000000)
lui  a3,0x1f
ori  a3,a3,0xffff (const 0x1FFFFF)
```

Target:
```
lui  $t0,0x8000
lui  $a3,0x1f
ori  $a3,$a3,0xffff
move $a0,$t1
move $a2,$a0
```

The constants are LICM-hoisted from inside the do-while body; the moves are the
pre-loop init. The basic-block scheduler picks moves first because their INSN_PRIORITY
(longest dependency chain to function exit through `lw v1,120(a0)`) exceeds the
constants' priority. To flip, the constants would need higher priority OR lower LUID.

## Ruled-out this session

- swap inner-loop init order `sum=0` first (V11): score 9 (regression — rotation)
- `for (j=0; j<0x24; j++)` form (V7): score 9
- `j` as `s32` (V8): score 9 — slti vs sltiu changes the cmp opcode
- swap inner-loop body order `j++; sum += *bp; bp++` (V10, V12): score 5-10
- sum at function-level scope (V9): stays 8 — sum's outer scope doesn't change alloc priority
- inline init declarations with combined decl+init (V2, V3): score 8-13

## Next hypotheses (not yet tried)

1. **BB2_ALLOC_DEBUG dump on cc1** — see which pseudos receive which regs and why,
   for the inner sum loop. Lever evidence may emerge from the conflict graph.
2. **Block-local sum split**: `sum_inner` used only in the inner loop + `sum_outer =
   sum_inner;` for the compare. Shortens inner pseudo's live range, may flip
   priority. Not tried.
3. **Sibling cross-reference**: find a function in `code6cac_c_mid.c` (or its
   sister files) with the same `(byte sum-loop, post-loop compare)` shape that
   MATCHED in pure C, and diff its declaration / RTL shape against this one.
4. **Constants-vs-moves: structural lever for the second loop** — instead of
   biasing LUID by renaming constants (rejected as cheat-by-spelling), look for
   a control-flow / type change that makes GCC emit constants first naturally.
   None found yet.

## Pointers

- Rule used (sanctioned): `.claude/rules/loop-exit-work-inside-loop-sched-fence.md`
- Rejected technique: `magic_base/magic_max` constant-rename — score 4 but
  no-semantic-purpose. See `meta.json.rejected_forms[0]`.
