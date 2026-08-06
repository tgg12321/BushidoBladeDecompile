# SetPacketData (src/main.c) — WIP checkpoint 2026-08-06

**Honest floor: 27 (was 34 at session start). Uncommitted src edits in place.**
Wiring: `asmfix.txt` `SetPacketData: replace_with_asmfile "asm/funcs/SetPacketData.s"`.

## What moved, and why (all of it was cheat removal, not codegen tricks)

| step | construct removed | score |
|---|---|---|
| start | — | 34 |
| redundant `*(volatile s32*)&D_800F1AF4` casts x2 + `volatile s32 *p_af4` aliases x2 | volatile coercion | 30 |
| `__asm__ ("la %0, D_800F1AE2" : "=r"(p_ae2))` -> `volatile u16 *p_ae2 = &D_800F1AE2;` | general-purpose inline asm | 27 |
| `register s32 r_arg1 asm("s4")` pin -> plain local; `volatile s32 *flag` -> `s32 *flag` | register pin + volatile coercion | 27 (inert) |
| param-alias local `r_arg1` dropped, `arg1` used directly ([[drop-param-alias-local]]) | — | 27 (inert) |

The `la` replacement is the notable one: honest C (`&D_800F1AE2`, already declared
`extern volatile u16`) reproduces target's `lui/addiu/lw 0(reg)` address-materialisation
form exactly. The inline asm was never needed.

## HAZARD — do not de-volatilize D_800F1AF4

`extern volatile s32 D_800F1AF4;` (main.c:3445) is shared with **func_8008C184, which is
already COMPLETED-C and therefore NOT in the queue**. Removing the volatile silently takes
that function 0 -> 4. Measured and reverted this session. The global is an ISR-decremented
packet countdown polled in a loop, which is the [[legitimate-volatile-interrupt-touched]]
two-prong shape, so the volatile stays on the extern. Only the *redundant* re-castings
inside SetPacketData were debt.

## The remaining 27 — two clusters

**1. Prologue arg-homing (the dominant one).** Target:
```
addiu sp,sp,-48
sw s4,32(sp)          <- s4 saved FIRST, out of the normal save order
move s4,a1            <- arg1 homed immediately at entry
lui v1,... (etc)
```
Ours saves s4 in normal order at index 5 and homes `a1` late, into `s3`, in the `beqz`
delay slot at index 13. This cascades into an s-register rotation (s3<->s4, s2<->s0) across
the whole body and an extra `j`/`move s2,zero` pair.
This is the prologue save-order class that [[no-new-park-categories]] names explicitly as a
known hard wall (the func_8007C2A0/C4B8 twins). Dropping the param alias did NOT move it —
GCC still sinks the copy past the early-return test.

**2. Address-materialisation form.** A few sites still differ `lui;lw %lo(sym)(reg)` (ours,
combine folded the %lo into the load) vs `lui;addiu;lw 0(reg)` (target, full `la`).
Candidate lever: [[defeat-combine-symbol-fold]] — pre-compute a displaced pointer so
combine cannot fold the displacement into the addressing mode. NOT yet tried.

## BLOCKER — one volatile is LOAD-BEARING, so COMPLETED-C needs a ruling

`volatile s32 *loop_flag = flag;` (main.c:3498, the polling loop over the D_800F1AEC
packet-state block) is **not** removable debt: de-volatilizing it takes the honest score
**27 -> 39**. The loop reads `loop_flag[1]` / `loop_flag[2]` as bare statements to
materialise loads that the target also emits.

So SetPacketData cannot reach COMPLETED-C on cheat-removal alone. It needs either:
 (a) a ruling that D_800F1AEC qualifies for the [[legitimate-volatile-interrupt-touched]]
     two-prong carve-out, or
 (b) pure-C structure that reproduces those loads without volatile.

(a) looks well-supported and should be checked FIRST: D_800F1AEC is the same packet-state
block as D_800F1AF4, whose `extern volatile` is already load-bearing for the COMPLETED-C
func_8008C184; and the identical `volatile s32 *loop_flag` pattern already ships in the
sibling function at main.c:3399. Precedent exists in-tree; what is missing is the explicit
two-prong finding (identify the ISR that mutates the block).

## Measured negatives (do not re-run)

- Hoisting `s0 = 0;` above the `if (*flag != 0) return -1;` test, to make it available to
  the `beqz` delay slot from the entry block and free the arg-home to the prologue:
  **27 -> 33, worse.** Reverted.

## Un-run resume levers (in priority order)

1. `defeat-combine-symbol-fold` on the remaining folded `%lo` load sites.
2. Make the `arg1` copy non-sinkable so it homes in the prologue: try using `arg1`
   unconditionally *before* the `*flag` early-return test, or restructure the early return
   so the test is not the first basic block.
3. One `volatile_cheat_count = 1` still reported for the body — locate and classify it
   (likely `*(volatile u16 *)(D_800A3044 + 4)`, which would be legitimate MMIO).

## Queue-state discrepancy

`engine/queue.json` records SetPacketData with **3** rules; `asmfix.txt` has only the one
`replace_with_asmfile` and `regfix.txt`'s "compound regfix" block for it is comment-only.
The queue entry is stale (the 2 dead rules were deleted in dcbfa6d6). `queue regen` after
the build lock frees would correct it.
