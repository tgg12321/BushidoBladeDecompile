# func_8002C61C — WIP (current state 2026-08-05, sched_solver goal-mapper round)

`src/code6cac_b.c:1013`. Per-frame dispatcher: mode switch over `D_80101F32`,
then three copy/average loops that move `Vec3i` triples out of scratchpad RAM
(`0x1F800000`) into the `D_801020xx` block, then a pad-poll tail.

Residual is NOT one class. `tmp/triage12/func_8002C61C.shape.txt` splits it into
a STRUCT pair at line 9, a RENAME cluster, a large OFFSET family at 173–255
(frame/offset class), and **two SCHED clusters at 137–147 and 164–171** — the
latter are what this round closed.

## The scheduling clusters — SOLVED, spelling verified

`code6cac_b` was not in the sched model before this round; extracting it gives
1684/1686 blocks exact, and **func_8002C61C itself is 74/74 blocks exact in both
passes**, so the model is authoritative here.

`perturb.py --goal-from-target code6cac_b` finds one differing, dependence-valid
block:

```
block 28 (4 insns): ours [290,287,284,281]  goal [287,284,281,290]
  2 vectors, both pure source order:
    luid swap 281 <-> 290
    luid_move 290 -> immediately before 281
```

In emission terms target puts `move t1,zero` **first**, before the pointer
setup; we put it last:

| | target | ours |
|---|---|---|
| uid 290 | `move $9,$0` — 1st | 4th |
| uid 281 | `la $7,D_801020D8` | 1st |
| uid 284 | `addu $8,$7,1100` | 2nd |
| uid 287 | `li $6,0x1f800000` | 3rd |

That is exactly `shape.txt`'s `MISSING @137 T: move t1,zero` plus the
`EXTRA @141-143` / `MISSING @144-147` pair.

**The C claim: `t1 = 0;` is the FIRST statement of the block, before the
`Vec3i *` initializers.** HEAD writes the three pointer initializers as
declarations-with-initializers and `t1 = 0;` after them. Split the declarations
from their assignments and hoist `t1 = 0;`:

```c
Vec3i *dst_a;  Vec3i *dst_b;  Vec3i *sp_src;
t1 = 0;
dst_a   = (Vec3i *)&D_801020D8;
dst_b   = (Vec3i *)((u8 *)&D_801020D8 + 0x44C);
sp_src  = (Vec3i *)0x1F800000;
```

**Verified by compiling the TU** (snapshot, `tools/sched_solver/mkasm.sh`):
`move $9,$0` moves ahead of the `la $7,D_801020D8` cluster, target's order.
Scored against target on the honest stream: **34 → 33 differing (-1)**, body
254 → 255 insns (target has one more instruction in that cluster than we did,
so the direction is right).

The **`D_801020FC` loop immediately after is structurally identical** and shows
the same `shape.txt` pattern at 164–171, and the same edit moves its `move
$9,$0` too — **but it is metric-NEUTRAL (+0), alone or combined.** Do not treat
the second hoist as justified: block 31 (that loop's block) is reported by the
mapper only as **dependence-invalid** — its `la SYM`/`addu r,1100` text is
identical to block 28's, so the move-pairer cross-paired them and no reliable
target order was ever derived for it. The D8 loop is the measured win; the FC
loop rests on structural analogy alone.

## Not a completion candidate

The other residual families are untouched and much larger:
- **OFFSET family, lines 173–255** — every `lw` displacement differs by a
  constant (target `lw v0,72(a2)` vs ours `lw v0,-24(a2)`): a base-pointer
  origin difference in the third loop, frame/offset class.
- **STRUCT @9** — target `andi a0,v1,0xffff` + `beq a0,v0` where we emit a bare
  `beq v1,v0`: the `mode` compare wants a u16 truncation we fold away.
- **RA component inside the same cluster** — target materialises the address
  into `$v0` and copies (`move a3,v0`); we compute straight into `$a3`.

## Caveats

- `code6cac_b` extraction reports **parity=False**: the instrumented cc1 differs
  from `build/cc1` on this TU by exactly **one instruction** (`or $4,$4,$2` vs
  `addu $4,$4,$2`, line 9204) inside `func_80030900` — which is also the only
  function whose blocks miss (2/1686). func_8002C61C is unaffected.
- `.tgt.s` is only valid at HEAD: regfix rules are calibrated to HEAD's
  instruction indices, so regenerate it before, never after, a source edit.

## Resume here

Apply the two `t1 = 0;` hoists, re-measure the sandbox score, then work the
OFFSET family — it is the largest remaining block and is not a scheduling
question. Instruments: `tools/sched_solver/{mkasm.sh,goalmap.py,perturb.py}`,
`tmp/triage12/func_8002C61C.{shape,diff}.txt`.
