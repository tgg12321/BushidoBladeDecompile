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

## == s2 (2026-08-18, structural) — floor 23 -> 1 ==

### FIRST: the s1 src edits were LOST AGAIN
src/main.c at s2 start still carried the pre-2026-08-06 body (the
`register s32 r_arg1 asm("s4")` pin + the `__asm__("la %0, D_800F1AE2")`
block), i.e. NOT the s1 candidate. Re-applying memory/grind/func_8008C1E8/
candidate.c restored floor 23 before any new work. This is the SECOND
consecutive session that had to re-apply the previous session's body — the
grind driver does not persist src edits, so ALWAYS diff src/main.c's function
body against candidate.c at session start (tmp/probe.py in this session's
scratch does the swap; it locates the function by name and brace-matches the
end, so it survives line drift).

### THE FINDING — goto-loops have no NOTE_INSN_LOOP_BEG, so loop.c and the
### loop-depth weighting of reg_n_refs never ran on this function

GCC 2.7.2 emits NOTE_INSN_LOOP_BEG/END only for front-end loop constructs
(`while`, `do/while`, `for`). A loop built from labels + `goto` is invisible to
loop.c (invariant motion, exit-test duplication) AND to flow.c's loop-depth
weighting of reg_n_refs. Every body this function has ever carried spelled all
three of its loops with goto. That single fact explains the whole residual:

1. **The 4-pseudo callee-save rotation** (s1's "dominant ~20-point cluster").
   global.c allocno_compare sorts allocnos by
   `floor_log2(n_refs) * n_refs / live_length` and assigns hard regs in
   REG_ALLOC_ORDER (callee-saves ascending s0,s1,...), so priority order ==
   s-number order. flow.c adds `loop_depth` to reg_n_refs per reference. With
   no loop notes every reference counted once and the order came out
   `st->s0, i->s1, retries->s2, arg1->s3, wait_val->s4`. With real C loops the
   depth-2 references of `retries` (both callback loops) and of loop.c's
   hoisted constant 5 outrank `st` (depth 1) and `arg1` (depth 0), and the
   order is EXACTLY target's `retries->s0, i->s1, 5->s2, st->s3, arg1->s4`.
   The rotation was never an allocator wall — it was a missing loop note.
2. **`wait_val` was never a source variable.** Target's `addiu $s2,$zero,5` in
   the wait-loop preheader is loop.c hoisting the literal 5 out of
   `while ((x & 5) != 5)`; the separate `addiu $v1,$zero,5` at the top is the
   duplicated exit test's own copy. Deleting the hand-written `wait_val` local
   and writing the real `while` reproduces both.
3. **`st = flag` (target `addu $s3,$v1,$zero`)** is likewise loop.c hoisting a
   loop-invariant copy out of the outer loop body — hence `volatile s32 *st =
   flag;` must be declared INSIDE the loop body, not before it.
4. **The `if (D_800F1AF4 == 0)` top guard** is expand_end_loop's duplicated
   exit test, free with a real `while`.

Measured ladder (each step is a single edit on top of the previous):
  23  s1 candidate (all-goto) re-applied
   8  all three loops rewritten as real C loops (`while` / `while` / `do-while`),
      `wait_val` deleted. Registers now match target exactly.
   3  `retries = 0;` moved ABOVE the `if (*flag != 0) return -1;` guard —
      reorg then fills the entry `beqz` delay with `addu s0,zero,zero` like
      target, which leaves the arg-home `addu s4,a1,zero` at prologue position
      3 and hoists `sw s4,0x20(sp)` to the front of the save block. NOTE: this
      is the same edit the 2026-08-06 WIP session measured as 27->33 on the
      all-goto body — it is worth +30 points only once the loop notes exist.
      Coupling confirmed, in the direction s1 predicted.
   1  `volatile s32 *st = flag;` moved from before the `while` to inside its
      body, so loop.c's hoist lands in the preheader AFTER the duplicated top
      test and `i = 0` gets the top test's beqz delay slot.

### The single remaining instruction
Outer-loop exit test: ours `lui v0,%hi(D_800F1AF4); lw v0,%lo(D_800F1AF4)(v0)`
(2 insns, combine folded the lo_sum into the load) vs target
`lui v0,%hi; addiu v0,v0,%lo; lw v0,0(v0)` (3 insns). build 158 / target 159;
every other instruction, including all six callee-save assignments and every
delay slot, is identical (verified with tmp/grind/func_8008C1E8/s2/norm2.py,
which normalises the objdump `move`/`li` aliases against the target's
`addu`/`addiu` spellings — those hunks are NOT real differences).
Confirmed mechanism for the un-folded form: a NAMED (REG_USERVAR_P) pointer
local holding `&SYM` blocks combine from folding the lo_sum into the MEM. Our
three other 3-insn sites (entry `*flag`, cleanup_B, the final return) all get
it that way. Four spellings of that idiom at the exit test were measured and
all are worse — see hypotheses.md [s2-H6].

### Standing cautions carried forward (unchanged)
- Do NOT de-volatilize D_800F1AF4 (breaks COMPLETED-C func_8008C184) nor the
  D_800F1AEC flag/st pointers. COMPLETED-C still needs the two-prong ISR
  finding for the D_800F1AEC block and a classification pass on the
  p_af8/p_af4b/p_af4/p_ae2 block-local pointers.
- Twin func_8008BF04 (main.c:3337) is also written as a goto-loop and carries
  the identical rotation as regfix ($19<->$20, $16<->$17). The loop-note
  finding should retire those rules outright — highest-value follow-on in the
  whole queue.

- [s2] [s2] The s1 src edits had been LOST AGAIN: src/main.c still carried the pre-2026-08-06 body (register-asm s4 pin plus la inline asm). This is the second consecutive session that had to re-apply candidate.c before doing any new work. Always diff the src body against candidate.c at session start.

- [s2] [s2] Floor at session end: 1 (sandbox func_8008C1E8 --disable all; build_insns 158 vs target_insns 159), edits in place in src/main.c and saved to memory/grind/func_8008C1E8/candidate.c

- [s2] [s2] GCC 2.7.2 emits NOTE_INSN_LOOP_BEG/END only for front-end loop constructs; label+goto loops are invisible both to loop.c and to flow.c's loop_depth weighting of reg_n_refs. That single fact accounted for ~22 of the 23 points: the 4-pseudo callee-save rotation, the entry arg-home/trampoline cluster, and both of the previously hand-spelled 'hoists'.

- [s2] [s2] Target's s2 (the constant 5) and s3 (the state-pointer copy) are loop.c preheader hoists, NOT source variables. The s1 ledger's wait_val local was a mis-derivation and is deleted in the s2 body.

- [s2] [s2] Register allocation now matches target EXACTLY: retries->s0, i->s1, const5->s2, st->s3, arg1->s4, pkt_len->s5. The prologue, every delay slot, both DeliverEvent cleanups, both callback loops and the epilogue are instruction-identical.

- [s2] [s2] Sole residual: the outer-loop exit test compiles to lui v0,%hi(D_800F1AF4); lw v0,%lo(D_800F1AF4)(v0) (combine folded the lo_sum) where target has lui; addiu v0,v0,%lo; lw v0,0(v0). Confirmed that a named REG_USERVAR_P pointer local blocks the fold (that is how the other three 3-insn sites work); four placements of such a pointer at the exit test all measured worse.

- [s2] [s2] Because our top and bottom outer-loop tests are byte-identical copies of one expression while target's differ (top folded 2-insn, bottom un-folded 3-insn), target's original C cannot have both tests coming from a single while (D_800F1AF4 != 0); it has a hand-written top guard and a differently-spelled bottom test.

- [s2] [s2] Twin func_8008BF04 (main.c:3337) is written in the same goto-loop style and carries regfix $19<->$20 @ 0-134 and $16<->$17 @ 68-121, the identical rotation. The loop-note rewrite should retire those rules outright.

- [s2] [s2] Unchanged cautions: do NOT de-volatilize D_800F1AF4 (breaks COMPLETED-C func_8008C184) or the D_800F1AEC pointers. COMPLETED-C still needs the two-prong ISR finding for the D_800F1AEC block and a classification pass on the four block-local volatile-pointer reads. No self_vet.md was written this session because the session is not candidate-ready.

- [s2] [s2] Verification note: the aligned diff must be read through tmp/grind/func_8008C1E8/s2/norm2.py, which normalises objdump's move/li aliases against the target listing's addu/addiu spellings; those hunks are display artifacts, not real differences.

## == s3 (2026-08-18, structural) — floor 1 -> 1 (held); last-insn pointer family CLOSED, volatile gate OPENED ==

### FIRST: the s2 src edits were LOST AGAIN (third consecutive session)
src/main.c at s3 start still carried the pre-2026-08-06 body — the
`register s32 r_arg1 asm("s4")` pin and the `__asm__("la %0, D_800F1AE2")`
block. Note the function has been RENAMED since s2: it is `SioSyncroWrite`
(src/main.c:3451), not `func_8008C1E8`, and the twin is `SioSyncroRead`
(src/main.c:3337). The ledger directory is `memory/grind/SioSyncroWrite/`.
Re-applying candidate.c restored floor 1. Note also that a stale `build/`
makes `sandbox` fail outright with "not found in any build/src/*.o" — run
`engine build` once at session start.
`tmp/grind/SioSyncroWrite/s3/probe.py <bodyfile>` does the body swap (locates
the function by name, brace-matches the end, survives line drift);
`tmp/grind/SioSyncroWrite/s3/batch.sh vX vY ...` applies+scores a list of
variant bodies in ONE WSL round trip and now flags compile errors explicitly.

### GOTCHA that cost three measurements — silent stale objects
Three s3 variants (vA/vB/vD2) put a declaration after a statement, or referenced
a do-block-scoped name in the `while` condition. cc1 errors, the sandbox does
NOT fail loudly, and a stale object is scored instead: those runs reported
30/151, 30/151 and 43/142 insns — plausible-looking numbers that mean nothing.
ALWAYS check for a compile error before believing a score whose build_insns
moved by more than a couple. batch.sh now greps the captured stderr.

### THE s3 FINDING — the last instruction is unreachable by any C pointer
Residual (unchanged): the outer-loop exit test compiles to
`lui v0,%hi(D_800F1AF4); lw v0,%lo(D_800F1AF4)(v0)` where target has
`lui v0,%hi; addiu v0,v0,%lo; lw v0,0(v0)` (asm/funcs/SioSyncroWrite.s:151-153).
The un-folded form appears exactly when the ADDRESS is materialised into a
pseudo, which at C level means a named pointer local. s3 read the actual pass
source, tools/gcc-2.7.2/loop.c:688-701, instead of guessing. scan_loop treats a
loop-invariant set as movable if ANY ONE of these holds:
  (1) `reg_in_basic_block_p (p, SET_DEST (set))` — the set and ALL uses are in
      the same basic block;
  (2) `! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)`;
  (3) `! maybe_never && ! loop_reg_used_before_p (...)`.
A named C pointer is a uservar, so (2) is false by construction. Therefore the
only way to stop the hoist is to make (1) false as well — put the pointer's
assignment in a DIFFERENT basic block from the load.
  * s3 did exactly that (assignment before `if (i == pkt_len)`, and again
    directly after the inner wait loop; declaration placed in the function's
    declaration block so the do-while condition can see it). The `la` DID stay
    in the loop — mechanism confirmed by disassembly,
    tmp/grind/SioSyncroWrite/s3/vA2.fn.txt offsets 0x5bc4/0x5bc8:
    `lui s2,%hi(D_800F1AF4); addiu s2,s2,%lo(D_800F1AF4)`.
  * But it landed in **s2, a callee-save**, displacing loop.c's const-5 hoist,
    because the pointer is then live across the `if (i == pkt_len)` block, which
    contains the DeliverEvent/callback calls. Score 6 (build_insns still 158).
Target's address register is `$v0`, dead one instruction later — that only
happens when the set is ADJACENT to the load, which is precisely loop.c case
(1), which always hoists. **The two requirements are mutually exclusive for any
C pointer variable in this loop.** The pointer-idiom family is now closed from
both ends: s2 killed the same-BB placements (hoisted; 9 / 11 / 22), s3 killed
the different-BB placements (kept in the loop, but buys a callee-save; 6 / 6).

The remaining live theory for target's form is reload REMATERIALISATION: one
`&D_800F1AF4` pseudo, hoisted by loop.c, denied a hard register by global.c
(a 7th callee-save is not worth its save/restore cost), and rematerialised from
its REG_EQUIV symbol_ref at each of its three uses — which would explain all
three of target's un-folded sites (cleanup_B 0x8008C244, exit test 0x8008C410,
final return 0x8008C428) coming from ONE source pointer, while the top guard
stays folded because it is a hand-written direct global read. s3 measured four
ref-count / live-range configurations of that shared pointer, trying to make
global.c decline the callee-save; it always won one:
  * exit test + final return (2 refs):            16, build_insns 159
  * exit test only (1 ref):                       11, build_insns 161
  * all three sites, decl before the loop:        19, build_insns 157
  * all three sites, decl at function top:        22, build_insns 157
    (reproduces s2's banked 22 exactly — chassis consistency check passed)
Whether that pseudo wins or loses a hard register is a global.c cost decision
on which the C source exposes no lever. The next modality for this instruction
is forensics on .greg/.lreg (is target's pointer allocno spilled with a
REG_EQUIV?), NOT more spellings.

Neutral chassis control re-measured: hand-written top guard +
`if (D_800F1AF4 != 0) do { ... } while (D_800F1AF4 != 0);` scores 1 with 158
insns, identical to the plain `while` — confirming s2's note that the do-while
chassis is free, so it stays available as a carrier for any future exit-test
idea.

### STYLE EVIDENCE from the original Sony object
Every single reference to D_800F1AEC inside the verbatim-linked LIBCOMB COMB
blob (asm/funcs/_comb_control.s lines 84, 258, 356, 549, 670, 723) is the
UN-FOLDED `lui;addiu %lo` form. Sony's LIBCOMB C holds the block base in a
pointer variable everywhere. That is independent support for the shared-pointer
reading of the target codegen, and against any "the loop test reads the global
directly" reading.

### FRONTIER ITEM 2 ADVANCED — the D_800F1AEC two-prong evidence is COMPLETE
Prong (a), IRQ writer, cited exactly: HandleSio (the static COMB.OBJ IRQ8/SIO
handler @0x8008C9F4..0x8008CD8C, installed via bios_SysEnqIntRP(3,&D_800A304C)
from r_sioinit) materialises the block base at asm/funcs/_comb_control.s:549-550
(`lui/addiu %lo(D_800F1AEC)` into $a0, address 0x8008CC78) and stores
**`sw $zero, 0x0($a0)` at 0x8008CCD4** — i.e. it clears D_800F1AEC ITSELF, not
merely the +4/+8/+0xC words. It also does the already-granted
`sw $v0,0x8($a0)` @0x8008CD48 (D_800F1AF4 countdown) and `sw $v0,0xC($a0)`
@0x8008CCFC (D_800F1AF8 latch).
Prong (b), use-site construct: SioSyncroWrite reads the block through
`volatile s32 *st` with double-read-across-sequence-point (`st[1] += 1; st[1];`
and `st[2] -= 1; st[2];`) and an IRQ-mutated-loop-bound (the D_800F1AF4
countdown drives the outer loop). Both are named constructs in
.claude/rules/legitimate-volatile-interrupt-touched.md, and the four sibling
words of the SAME struct (D_800F1AF0/AF4/AF8 and D_800F1B00/04) already carry
the operator-audited 2026-07-10 grant at volatile_extern_allowlist.txt:41-45.

MEASURED, and this is the point: changing the file-scope declaration from
`extern s32 D_800F1AEC;` (src/main.c:3431) to `extern volatile s32 D_800F1AEC;`
is **score-neutral for BOTH functions that share it** — SioSyncroWrite 1 -> 1
(158/159) and the already-matched sibling SioAnsyncWrite 0 -> 0 (25/25). So the
body's `volatile s32 *flag = &D_800F1AEC;`, which today reads as a
volatile-coercion-by-pointer-type on a non-volatile extern, can be replaced by
the natural declaration at ZERO cost. NOT applied in s3: the allowlist file's
own header requires a layer-2 cheat-reviewer confirmation plus a commit audit
block, neither of which a grind session may produce. Draft entry for whoever
files it:

    D_800F1AEC    # SioSyncroWrite (double-read-across-sequence-point via
    # `volatile s32 *st` + IRQ-mutated-loop-bound on the +8 countdown) and
    # SioAnsyncWrite busy flag — IRQ writer: HandleSio `sw $zero,0x0($a0)`
    # @0x8008CCD4 with $a0 = &D_800F1AEC materialised @0x8008CC78
    # (asm/funcs/_comb_control.s:549-550). Same Ruling-4 class grant and same
    # struct as D_800F1AF0/AF4/AF8 (allowlist lines 43-45).

### Standing cautions carried forward (unchanged)
- Do NOT de-volatilize D_800F1AF4 (breaks COMPLETED-C func_8008C184) nor the
  D_800F1AEC flag/st pointers (27 -> 39, 2026-08-06).
- Still needed before candidate-ready: the classification pass on the
  p_ae2 / p_af8 / p_af4b / p_af4 block-local `volatile T *p = &GLOBAL;` reads
  (natural read spelling vs a coercion family) and self_vet.md. The D_800F1AEC
  finding above materially helps: p_af8 / p_af4b / p_af4 all point at globals
  that are ALREADY `extern volatile` at file scope, so those three add no
  qualifier and are pure address-materialisation spellings; p_ae2 likewise
  (D_800F1AE2 is `extern volatile u16`). Only `flag` was adding a qualifier,
  and that is exactly what the allowlist entry above fixes.
- Twin SioSyncroRead (src/main.c:3337, formerly func_8008BF04) is still written
  in the goto-loop style and still carries the identical rotation as regfix
  ($19<->$20 @ 0-134, $16<->$17 @ 68-121). The s2 loop-note rewrite should
  retire those rules outright — highest-value transfer in the whole queue.

- [s3] Floor at session end: 1 (sandbox SioSyncroWrite --disable all; build_insns 158 vs target_insns 159), s2 body in place in src/main.c and saved to memory/grind/SioSyncroWrite/candidate.c.

- [s3] The s2 src edits were LOST AGAIN — third consecutive session that had to re-apply candidate.c before any new work. The function has also been RENAMED: it is SioSyncroWrite (src/main.c:3451), the twin is SioSyncroRead (src/main.c:3337). A stale build/ additionally makes sandbox fail outright with 'not found in any build/src/*.o' — run engine build once at session start.

- [s3] PASS ATTRIBUTION, read from source not guessed: tools/gcc-2.7.2/loop.c:688-701 lists the three alternative conditions under which scan_loop treats a loop-invariant set as movable. A named C pointer is a uservar so condition (2) never applies to it, which is the whole reason the exit-test address behaves differently from the three sites that already match.

- [s3] Disassembly proof that the different-basic-block placement works mechanically: tmp/grind/SioSyncroWrite/s3/vA2.fn.txt offsets 0x5bc4/0x5bc8 carry lui s2,%hi(D_800F1AF4); addiu s2,s2,%lo INSIDE the loop. It fails only on register class — s2 is a callee-save and it evicts loop.c's const-5 hoist.

- [s3] Six distinct pointer spellings for the exit test are now banked as rejected forms (s2: 9 / 11 / 22 / 1; s3: 6 / 6 / 16 / 11 / 19). Do not re-propose a pointer-idiom spelling for this instruction.

- [s3] Every reference to D_800F1AEC inside the verbatim-linked Sony LIBCOMB COMB object (asm/funcs/_comb_control.s lines 84, 258, 356, 549, 670, 723) uses the un-folded lui;addiu %lo form — Sony's C holds block bases in pointer variables, which supports the shared-pointer reading of target's codegen.

- [s3] MEASUREMENT HYGIENE: three s3 variants silently scored a STALE object because cc1 rejected them (C89 declaration-after-statement, and a do-block-scoped name referenced in the while condition). They reported 30/151, 30/151 and 43/142 — plausible numbers that mean nothing. tmp/grind/SioSyncroWrite/s3/batch.sh now captures cc1 stderr and flags COMPILE-ERROR; distrust any score whose build_insns jumps by more than a couple.

- [s3] D_800F1AEC two-prong volatile evidence is COMPLETE and cited to the instruction: IRQ writer HandleSio (static COMB.OBJ handler @0x8008C9F4..0x8008CD8C, installed via bios_SysEnqIntRP(3,&D_800A304C) from r_sioinit) stores sw $zero,0x0($a0) @0x8008CCD4 with $a0 = &D_800F1AEC materialised @0x8008CC78; use-site constructs are double-read-across-sequence-point and IRQ-mutated-loop-bound. The natural extern volatile declaration is score-neutral for both consumers. Draft allowlist entry is in evidence.md; NOT filed in-session (the file's header mandates a layer-2 cheat-reviewer confirmation and a commit audit block).

- [s3] Classification of the remaining block-local volatile pointer reads is now mostly resolved: p_ae2, p_af8, p_af4b and p_af4 all point at globals that are ALREADY extern volatile at file scope, so they add no qualifier and are pure address-materialisation spellings. Only flag was adding a qualifier, and the allowlist entry above is the sanctioned fix for it.

- [s3] Twin SioSyncroRead (src/main.c:3337) is unchanged and still carries regfix $19<->$20 @ 0-134 and $16<->$17 @ 68-121 — the identical rotation the s2 loop-note rewrite dissolves here. Highest-value transfer of this ledger, out of scope for a SioSyncroWrite session.
