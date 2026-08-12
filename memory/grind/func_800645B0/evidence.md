# Evidence — func_800645B0 (src/text1b.c)

## Session 1 (2026-08-12, modality: recon) — reached distance 0, layer-1 FAILed

### Baseline
- `canonical func_800645B0` -> verdict **C**, asm_insns 0, total 78, distance 21.
  Pure-C target; no canonical-asm routing question exists for this function.
- `sandbox func_800645B0 --disable all` at session start: **score 21**,
  target_insns 78, build_insns 80, rules_dropped 1.
- regfix.txt:2521 carries `func_800645B0: reorder 3,1,2 @ 1-3` (a prologue
  save-order reorder). It is the function's only rule.
- HEAD source carried two cheat-asm register pins
  (`register s32 s3 asm("$19")`, `register s32 one asm("$3") = 1`), a
  goto-based inner loop (`loop_inner:` label), and a bare `do { } while (0);`.

### What the target actually is
`asm/funcs/func_800645B0.s` (78 insns) is a plain nested loop, not a goto
chain: outer counter `$s3` stepping by 4 while `< 15`; inner counter `$a0`
0..3; `$s0 = $s3 + $a0` is the slot index; `$s2 = 1 << $s0` the bit mask;
`D_800A3444` is the occupancy bitmask. On a free slot it seeds three s32 words
(`D_800F0D78` / `D_800F0D7C` / `videoDec`, a 12-byte-strided triple indexed by
the slot) each as `D_800A347C[k] + (rand() & 0xFF) - 0x7F`, then an s16
(`D_800F0BCC`, 2-byte stride) as `rand() & 7`, sets the bit, and breaks out of
the inner loop. Returns 1. `D_800F10EC = 1` in the prologue.

### Measured ladder (each step a separate `sandbox --disable all` run)
| step | change | score | build_insns |
|---|---|---|---|
| 0 | HEAD (pins stripped by the sandbox) | 21 | 80 |
| 1 | natural nested do/while + `break`, pins deleted | 17 | 81 |
| 2 | `val` reused for const-1 and the `D_800A3444` read; split RMW; `last = rand()` named | 10 | 78 |
| 3 | `val = val \| mask; D_800A3444 = val;` (OR in place) | 3 | 78 |
| 4 | `j += 1;` moved between `idx = i + j;` and `val = 1;` | **0** | 78 |

Step 4 was FAILed by the layer-1 cheat-reviewer and is a BANNED construct.
Steps 1-3 were ruled legitimate and are retained.

### Mechanism facts established (reusable)
- **loop.c hoists the const-1.** `mask = 1 << idx` inside the inner loop makes
  the const-1 pseudo a single-set loop invariant; `scan_loop`/`move_movables`
  hoist it into a fresh callee-save (`$s4`), which adds a save/restore pair
  (81 insns vs 78) and cascades the whole allocation. Confirmed by reading
  `tools/gcc-2.7.2/loop.c` (`threshold = (loop_has_call ? 1 : 2) * (1 +
  n_non_fixed_regs)` at loop.c:532; the movable test needs `n_times_set == 1`).
  The target keeps the constant INSIDE the loop in caller-save `$v1`.
- **The fix is multi-set reuse.** Routing the constant through the same C
  variable that later holds the `D_800A3444` read makes `n_times_set > 1` AND
  lands the pseudo in `$v1`, the register the target uses for both roles.
- **`val = val | mask; D_800A3444 = val;` beats `D_800A3444 = val | mask;`**
  by 7 points: it makes `val` the OR destination (matching `or $v1,$v1,$s2`)
  and raises `val`'s reference count over `j`'s, flipping the $v1/$a0
  assignment so val=$v1 / j=$a0 as in the target.
- **The read-modify-write spelling controls a load-delay nop.** The fused
  `D_800A3444 |= mask;` emitted `lw; nop; or; sw` after the fourth `rand()`;
  naming the read earlier (`last = rand(); val = D_800A3444; *sh = last & 7;`)
  reproduces the target's `lw $v1,%gp_rel(D_800A3444)` immediately after the
  `jal`, with the `andi`/`lui`/`addu`/`sh` cluster filling the load delay.

---

## Session 2 (2026-08-12, modality: recon) — banned lever replaced; residual re-characterised. Floor 3.

Starting point: session 1's constructs 1-3 with `j += 1;` restored to its
natural position at the end of the inner-loop statement group ("A_base").
Re-measured: **score 3, build_insns 78** — confirms the ledger's step-3 figure
and that reverting the banned construct costs exactly the 3 points it bought.

### The distance-3 residual at A_base (unmasked `diff_pairs`)
Target steals `addu $s0,$s3,$a0` into the inner back-edge delay slot
(`bnez $v0,.L800645E0` / `addu $s0,$s3,$a0`, asm lines 69-70) and places the
loop label after it; our build steals `li $v1,1` instead. Everything else is
identical; the remaining pairs are branch/jump target ADDRESSES only (the
sandbox object places the function at a different section offset).

### ROOT CAUSE FOUND — sched.c `birthing_insn_p`, not a LUID tie
`cc1 -da` dumps of the real build (`tmp/grind/func_800645B0/s1/dump/`,
extracted per-function with `extract.py`) show the inner-loop block verbatim:

```
;;      -- basic block number 2 from 24 to 42 --
;; insn[  28]: priority = 1   (addu idx = i + j)
;; insn[  31]: priority = 1   (li   val = 1)
;; ready list at T-6: 31 (1) 28 (7f000001), now 28 31      <- A_base
;; ready list at T-6: 31 (1) 28 (1),        now 31 28      <- with idx multi-set
```

`sched.c:birthing_insn_p` returns `reg_n_sets[dest] == 1` for a destination
that is live at that point, and `adjust_priority` then raises that insn's
INSN_PRIORITY to `max_priority` (0x7f000001 in the dump). `idx` is assigned
exactly once in A_base, so `addu` gets the bonus; `val` is assigned three
times, so `li` gets none. The scheduler runs BACKWARD (T-1 is the last insn of
the block), so the bonused insn is picked first and therefore emitted LAST;
`li` ends up first in the block and reorg.c steals the block's first insn into
the back-edge delay slot. That is the whole of the 3-point gap at A_base —
and it is a REGISTER-PRESSURE HEURISTIC, not a source-order tie, which is why
session 1's statement-reorder appeared to be the only lever.

### The legitimate lever: make `idx` multi-set (variant K, now candidate.c)
`idx2 = idx << 1; idx = idx2 + idx;` (the index variable reused to hold the
12-byte-stride word index) gives `reg_n_sets[idx] == 2`, kills the bonus, and
the .sched dump above flips to `now 31 28` — `addu` first in the block, `addu`
in the back-edge delay slot, exactly as target. The target's own register
allocation does the same thing: `$s0` holds `i+j`, then `$s1+$s0`, then
`$s0<<2`. Score stays 3 but the residual MOVES to a different, narrower axis.

### The distance-3 residual at K (side-by-side, `listing.py`)
```
idx | target                  | build (K)
 18 | jal  rand               | sll  s1,s0,0x1
 19 |  sll  s1,s0,0x1 (delay) | jal  rand
 20 | addu s0,s1,s0           | addu s0,s0,s1
```
Two independent sub-diffs:
1. **Delay-slot fill.** In target the `*3` sum is emitted AFTER the first
   `rand()` call, so the only fillable insn before the call is the `sll` and
   reorg.c puts it in the jal delay slot. In K both the `sll` and the sum
   precede the call (they are separate statements ahead of the first store),
   so reorg fills the slot with the sum instead. NOTE: A_base, which leaves
   the sum INLINE in the store's address expression, gets this region exactly
   right — GCC expands the call-bearing RHS before the address arithmetic.
   So the two residuals are currently mutually exclusive: A_base has the
   address region right and the loop top wrong; K has the loop top right and
   the address region wrong.
2. **Commutative operand order.** Target `addu $s0,$s1,$s0` (shift result
   first); K emits `addu $s0,$s0,$s1`. cse.c's `fold_rtx` canonicalises the
   PLUS operands via its hash-table `must_swap` path (cse.c:5278ff); the
   source-level operand order (`idx2 + idx`) does not survive.

### Measured NEGATIVES this session (all `sandbox --disable all`, 78-insn target)
| variant | form | score | insns |
|---|---|---|---|
| A_base | session-1 step 3, `j += 1;` natural | 3 | 78 |
| I_idx_reuse | `idx = (idx2 + idx) << 2;`, stores use `idx` | 12 | 79 |
| J_idx_reuse_split | `idx = idx2 + idx; idx = idx << 2;` | 11 | 78 |
| K_idx_reuse_sum | `idx = idx2 + idx;`, stores use `(idx << 2)` | **3** | 78 |
| L_mul3_mul2 | `idx2 = idx * 2; idx = idx * 3;` | 3 | 78 |
| M_mul12_mul2 | `idx2 = idx * 2; idx = idx * 12;` | 12 | 79 |
| N_mul3_shift2 | `idx2 = idx << 1; idx = idx * 3;` | 3 | 78 |
| O_mul12_shift2 | `idx2 = idx << 1; idx = idx * 12;` | 12 | 79 |
| P_mul12_only | `idx2 = idx; idx = idx * 12;`, s16 uses `idx2 * 2` | 21 | 81 |
| Q_declswap | K with `idx2` declared before `idx` | 3 | 78 |
| R_declswap_mul3 | N with `idx2` declared before `idx` | 3 | 78 |
| S_idx2_first | K with `idx2` declared first of all locals | 3 | 78 |

Two axes are therefore MEASURED DEAD:
- **Declaration order does not control the commutative PLUS operand order.**
  Pseudo numbers do follow declaration order (confirmed in the .combine dump:
  i=72, j=73, idx=74, idx2=75, mask=76, val=77), but Q/R/S all stay at 3 with
  the same `addu $s0,$s0,$s1`. The SOTN "named-intermediate declaration order"
  family is spent on this residual.
- **Writing the stride as a multiply (`* 3` / `* 12`) does not help.** `* 3`
  is byte-identical to the explicit `idx2 + idx` (L/N == K); `* 12` costs an
  extra insn (79) because the `idx * 2` value is no longer shared with the
  s16 store.

### Integration state (unchanged, NOT done by this session)
`regfix.txt:2521` (`func_800645B0: reorder 3,1,2 @ 1-3`) is the function's only
rule and the sandbox drops it. Retirement + `queue done` is the operator's job.

- [s1] Reverting the BANNED construct (j += 1 relocated between `idx = i + j;` and `val = 1;`) costs exactly 3 points: the legitimate session-1 form measures score 3, build_insns 78, rules_dropped 1 — confirming the ledger's step-3 figure.

- [s1] sched.c:birthing_insn_p returns `reg_n_sets[dest] == 1` for a live destination and adjust_priority raises that insn to max_priority (printed as 7f000001 in the -da sched dump). This is a whole-function set count, so ANY second assignment to the variable anywhere in the function removes the bonus.

- [s1] The GCC 2.7.2 list scheduler is BACKWARD (T-1 is the last insn of the block), so a priority bonus makes an insn emit LATER, not earlier — the opposite of the naive reading, and the reason session 1 mis-diagnosed the residual as a plain LUID tie.

- [s1] Local pseudo numbers follow DECLARATION order in this compiler (t.i.combine: i=72, j=73, idx=74, idx2=75, mask=76, val=77) — a reusable fact for LUID/regno-based levers elsewhere, even though it did not move this residual.

- [s1] The target's own allocation reuses one register for three roles ($s0 = i+j, then $s1+$s0, then $s0<<2), which is direct evidence that the original C reused the slot-index variable for the derived word index.

---

## Session 3 (2026-08-12, modality: structural) — FLOOR 3 -> 1. Both s2 residuals closed at once; one new, fully-mechanised residual left.

(This is the third grind session; the ledger's earlier "Session 2" block was
written by the second session, whose digest labelled itself s1/s2 inconsistently.
Everything below is new work.)

### The winning form (variant AA, tmp/grind/func_800645B0/s2/sweep6.py)
```c
idx2 = idx << 1;
last = rand();
idx  = idx2 + idx;
*((s32 *)((s32)&D_800F0D78 + (idx << 2))) = base[0] + (last & 0xFF) - 0x7F;
```
**score 1, build_insns 78** (was 3). Two independent effects, both required:
1. `idx = idx2 + idx;` is a REAL set of `idx`, so `reg_n_sets[idx] == 2` and
   sched.c's `birthing_insn_p` bonus does not fire on `addu idx,i,j`; the
   inner-loop block emits `addu` first and reorg.c steals it into the back-edge
   delay slot, as target.
2. Naming the FIRST `rand()` into the existing `last` temp puts the CALL ahead
   of the sum in source order, so the sum is emitted AFTER the `jal` and reorg
   fills the jal delay slot with the `sll` — the target's shape. (With the sum
   ahead of the call the sum fills the slot: that was the previous session's
   variant K.) Byte-identical alternatives: AB (every `rand()` named), AC (sum
   written `idx * 3`). Control AD (call named but the sum left inline) = 3.

### The remaining 1 point, and its exact mechanism
```
idx | target              | build
 20 | addu s0,s1,s0       | addu s0,s0,s1
```
`optabs.c:399-417` (`expand_binop`): for a commutative optab GCC swaps op0/op1
when `target == op1` (or when op1 is a REG and op0 is not). So **any**
`idx = <x> + idx;` expands as `(plus idx x)` — confirmed in the .rtl dump
itself (`dump_AA/f_rtl.txt` insn 53), i.e. BEFORE cse runs. The previous
session's cse.c `fold_rtx`/`must_swap` hypothesis is therefore WRONG: the
operand order is fixed at RTL expansion by the assignment target, not by cse.
Corollary: emitting the target's `(plus idx2 idx)` requires the sum's
DESTINATION pseudo to differ from `idx` — which is mutually exclusive with
effect 1 above unless some other real set of `idx` exists.

### The allocation constraint that blocks the obvious fix
| variant | shape | score | insns |
|---|---|---|---|
| CA | `wid = idx2 + idx;` (third local), stores use `(wid << 2)` | 3 | 78 |
| DA | CA + `idx = wid << 2;`, stores use `idx` | 12 | 78 |
CA gets the operand order and every register right and fails only on the loop
top (idx single-set). DA closes the loop top too — but flips `idx`/`idx2`
between `$s0` and `$s1` and costs 12. Mechanism, from `dump_DA/f_lreg.txt` +
`f_greg.txt` and `global.c:allocno_compare`:
- `local_alloc` runs BEFORE `global_alloc`. The 12-byte-stride BYTE OFFSET is
  normally a BLOCK-LOCAL CSE temp with high allocno priority
  (`floor_log2(n_refs)*n_refs/live_length`), so it claims `$s0` first; the
  block-local `idx2`, whose range overlaps it, is pushed to `$s1`; the
  multi-block `idx` then re-uses `$s0` because its range does not overlap.
- Routing the byte offset through `idx` (a multi-block pseudo) removes that
  early claimant: `idx2` takes `$s0`, `idx` conflicts and takes `$s1`.
- Declaration order does not move it (sweep 10 EA/EB/EC/ED/EE all 12/78) —
  a second, independent confirmation of the earlier H7 kill.

### Measured NEGATIVES this session (all `sandbox --disable all`, 78-insn target)
| variant | form | score | insns |
|---|---|---|---|
| A_base | previous baseline (sum inline, idx single-set) | 3 | 78 |
| T_split | `idx = i; idx += j;` | 3 | 78 |
| U_half | `idx = idx << 1;` after the stores, s16 uses `idx` | 3 | 78 |
| V_both | T_split + U_half | 3 | 78 |
| W_late | `idx = idx2 + idx;` after stores that compute the sum inline | 3 | 78 |
| AA/AB/AC | **first rand() named + `idx = idx2 + idx`** | **1** | 78 |
| AD | first rand() named, sum left inline | 3 | 78 |
| BA..BG | AA with the sum respelled (`idx + idx2`, `+=`, decl swap, `idx+idx` for idx2, `idx*2`, call-before-shift) | 1 | 78 |
| CA/CB/CC | third local `wid` for the sum, call named | 3 | 78 |
| CD | third local `wid`, sum before the call | 10 | 78 |
| DA/DC/DD | byte offset assigned into `idx` | 12 | 78 |
| DB | byte offset as `idx = wid * 4` | 3 | 78 |
| DE | `idx = wid;` copy | 22 | 75 |
| EA..EE | DA with five declaration orders | 12 | 78 |
| EF | byte offset in `wid`, `idx = idx2;` copy | 3 | 78 |
| FA/FB/FC/FD | halfword offset re-assigned into `idx` (`<<1`, `+idx`, `*2`) | 3 | 78 |
| GA/GB/GC | `idx = idx2 + (i + j);` (recompute so op1 != target) | 19 | **79** |
| GD | `idx2 = (i+j) << 1; idx = idx2 + (i+j);` | 13 | **79** |

### Axes MEASURED DEAD this session
- **Foldable second sets of `idx` do nothing.** A copy, a split-init, or a
  recomputation of a value already live is folded before flow recomputes
  `reg_n_sets`; `dump_A_base/f_sched.txt` and `dump_U_half/f_sched.txt` carry a
  byte-identical ready list (`T-6: 31 (1) 28 (7f000001), now 28 31`). A
  reg_n_sets lever needs a set whose VALUE is not already available.
- **Source-level respelling of the sum cannot change its operand order.** Eight
  spellings (sweep 7) are byte-identical; the order is decided in
  `expand_binop`, keyed on the assignment target.
- **Recomputing `i + j` to dodge that swap costs a real instruction** (79 vs 78).
- **Declaration order is dead on this function** (now confirmed in two shapes).

### Tooling added (tmp/grind/func_800645B0/s2/)
`sweep5..sweep12.py` (variant sweeps), `diffvar*.py` (apply a variant, sandbox
it, print the UNMASKED target-vs-build differing instruction pairs),
`dumpvar.sh` + `dumpAA.py`/`dumpV9.py`/`dumpV10.py` (cc1 `-da` all-pass dump for
an arbitrary variant, per-function slices), `schedvar.py` (ready-list extract).

---

- [s1] Reproducible tooling now exists: dump.sh (cpp+cc1 -da on the real build flags), extract.py (per-function slice of any -da dump), listing.py (side-by-side unmasked target-vs-build instruction listing), and four sandbox sweep harnesses.

- [s2] The honest pure-C floor for func_800645B0 is now 1 (sandbox --disable all: score 1, target_insns 78, build_insns 78, rules_dropped 1), down from 3. The form is memory/grind/func_800645B0/candidate.c (sweep6 variant AA).

- [s2] The single remaining unmasked diff is instruction index 20: target `addu s0,s1,s0` vs build `addu s0,s0,s1`. Everything else in the 78-instruction function is identical; the other diff pairs are branch/jump target ADDRESSES only (the sandbox object places the function at a different section offset).

- [s2] optabs.c:399-417 (expand_binop) swaps a commutative operand pair when `target == op1`, or when op1 is a REG and op0 is not. This decides the emitted PLUS operand order at RTL expansion — the swapped form is already present in the cc1 -da .rtl dump — so no source-level respelling of the expression can change it. Only the assignment DESTINATION can.

- [s2] reg_n_sets (the input to sched.c's birthing_insn_p bonus) is taken after the fold passes, so a second assignment that is a copy, a split-init pair combine re-merges, or a recomputation of a value already live in another pseudo is NOT counted. Nine such spellings measured inert; the A_base and U_half .sched dumps are byte-identical.

- [s2] local_alloc runs before global_alloc, so a high-priority BLOCK-LOCAL pseudo claims $s0 ahead of any multi-block pseudo. In this function the 12-byte-stride byte offset must therefore stay a block-local CSE temp: routing it through the multi-block `idx` hands $s0 to `idx2` and costs 12 points, in every declaration order tried.

- [s2] The three constraints on the last point are now fully enumerated and pairwise-measured: (a) the sum's dest must NOT be `idx` (operand order), (b) `idx` needs a fold-surviving second set (loop-top order), (c) the byte offset must stay block-local ($s0/$s1 assignment). CA satisfies (a)+(c) = 3, AA satisfies (b)+(c) = 1, DA satisfies (a)+(b) = 12.

- [s2] When TWO ready insns both carry the birthing bonus, the .sched dump shows the LATER insn picked first (e.g. `ready list at T-25: 52 (7f000001) 63 (7f000001), now 63 52`), i.e. emitted last — so making the const-1 pseudo single-set would also restore the target's loop-top order, if loop.c could be kept from hoisting it. That is the untried side of the residual.

- [s2] Session tooling is reusable for any function: diffvar*.py prints the UNMASKED target-vs-build instruction pairs for an arbitrary source variant, and dumpvar.sh + dumpAA.py produce a full cc1 -da pass set with per-function slices.
