# Evidence bank — func_8008C1E8

- == imported from memory/wip notes.md ==
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


## == s1 (2026-08-18, recon) — floor 28 -> 23, map rebuilt ==

**The 2026-08-06 WIP src edits were LOST** (session started with the s4 pin +
la-asm still in src; floor read 28). Re-applied from this evidence file; the
documented replacements now measure 25, not 27 (context drifted). Everything
below is measured against the s1 body (memory/grind/func_8008C1E8/candidate.c,
in place in src/main.c at session end, sandbox 23, build 160 vs target 159 insns).

### Full diff map at floor 23 (artifacts: tmp/grind/func_8008C1E8/s1/)
- build.txt / target.txt: numbered side-by-side listings (159/160 insns).
- Remaining diff = ONE coupled cluster + its 2-insn satellite:
  1. **s-register rotation over 4 pseudos** (most of the 23): arg1 s3<->s4,
     outer counter (C `s0`) s2<->s0, loop_flag copy s0<->s3, wait_val s4<->s2.
     s1 (inner counter, then p_af8 pointer reuse — target reuses s1 for BOTH,
     ours does too) and s5 (packet-size) already match.
  2. **Entry shape**: target `sw s4 @0x20` FIRST in prologue, `move s4,a1` at
     insn 3, beqz delay = `addu s0,zero,zero`, inline `j .L43C; li v0,-1`.
     Ours: normal save order, arg-home `move s3,a1` IN the beqz delay slot,
     plus an extra `j main_work; move s2,zero` trampoline pair (the +1 insn
     count together with the H3 addiu).
- Confirmed identical modulo rotation: both DeliverEvent cleanups, the whole
  send_byte body (loop_flag[1]/[2] RMW + bare re-reads), both inner_wait call
  loops, epilogue.

### New structural finds (semantics corrections, both CONFIRMED)
- **[s1-H2] 24:** `if ((...&0x80) != loop_flag[3]) { s1 = 0; goto loop_continue; }`
  — target executes s1=0 in that bne's delay slot and clobbers it immediately
  on fall-through (lui s1), i.e. the zero is taken-path-only. Old C was
  functionally divergent from target here (kept s1==s5 on the goto path).
  reorg fills the slot from the target thread. Unconditional placement before
  the if = 26 (sched1 steals it into the lhu load-delay slot) — rejected/.
- **[s1-H3] 23:** loop_continue test through `volatile s32 *p_af4c = &D_800F1AF4;`
  gives target's full lui/addiu/lw form (direct global read combine-folds %lo).

### Twin function (shared wall — solve once, apply twice)
func_8008BF04 (main.c:3337, same `(u8*, s32)` signature, receive-side twin,
still queued) carries regfix `$19 <-> $20 @ 0-134` (= s3<->s4, the SAME arg1
rotation) + `$16 <-> $17 @ 68-121` + prologue-offset substs + fill_delay.
Whatever C lever flips the allocation here almost certainly retires the twin's
rotation rules too. tmp/duplicates_leads.txt has no entry for either.

### Measured negatives this session (see rejected/)
- s1=0 unconditional before the loop_flag[3] if: 26.
- s0=0 at main_work head (trampoline collapses 160->159 but): 31. Entry
  cluster is coupled to the arg-home; do not attack s0-placement standalone.

### Standing cautions carried forward
- Do NOT de-volatilize D_800F1AF4 (breaks COMPLETED-C func_8008C184) nor the
  D_800F1AEC flag/loop_flag pointers (27->39, 2026-08-06). COMPLETED-C will
  additionally need the two-prong ISR finding for the D_800F1AEC block and a
  classification pass on the p_* pointer blocks (natural spelling vs
  pointer-alias family) — frontier item 3.
- Queue-state note: queue.json's "3 rules" for this func is stale; asmfix has
  only the one replace_with_asmfile (line 52).

- [s1] Floor at session end: 23 (sandbox func_8008C1E8 --disable all; build 160 vs target 159 insns), edits in place in src/main.c and saved to memory/grind/func_8008C1E8/candidate.c

- [s1] Remaining diff is ONE coupled cluster: s-reg rotation arg1 s3<->s4, counter s2<->s0, loop_flag-copy s0<->s3, wait_val s4<->s2 (s1/s5 match), plus the entry arg-home (target: sw s4 first, move s4,a1 at insn 3, s0=0 in beqz delay) and our extra j/move trampoline

- [s1] Twin func_8008BF04 (main.c:3337, same signature/structure, queued) carries regfix $19<->$20 @ 0-134 and $16<->$17 @ 68-121 — the identical rotation wall; one mechanism closes both

- [s1] Target's allocation gives arg1 the HIGHEST callee-save (s4) i.e. lowest allocation priority of the four rotated pseudos; ours gives it s3 — RA-priority forensics is the next axis

- [s1] volatile on the D_800F1AEC flag/loop_flag pointers remains load-bearing (2026-08-06: de-volatilize = 27->39); COMPLETED-C additionally needs the two-prong ISR finding for that block and a classification pass on the p_* pointer blocks

- [s1] cheat state: zero pins, zero inline asm in the function body now; only remaining rule is the asmfix replace_with_asmfile wiring (line 52)
