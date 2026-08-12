# Evidence — func_800645B0 (src/text1b.c)

## Session 1 (2026-08-12, modality: recon) — SOLVED, honest distance 21 -> 0

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

### Byte-exactness proof (not a masked zero)
`engine.diagnose.diff_pairs(mask=False)` on the cheat-stripped sandbox object
vs `build/src/text1b.o` leaves exactly four text differences, all
branch/jump target *addresses*, and all four carry identical relative
displacements to the target: `bnez +0xBC`, `j +0x1C`, `bnez -0xD0`,
`bnez -0xE8`. They differ only because the sandbox object places the function
at a different section offset. Instruction sequence, opcodes, registers,
immediates and frame layout are identical (78/78, frame 0x28, `ra` at 0x20).

### Mechanism facts established (reusable)
- **loop.c hoists the const-1.** `mask = 1 << idx` inside the inner loop makes
  the const-1 pseudo a single-set loop invariant; `scan_loop`/`move_movables`
  hoist it into a fresh callee-save (`$s4`), which adds a save/restore pair
  (81 insns vs 78) and cascades the whole allocation. Confirmed by reading
  `tools/gcc-2.7.2/loop.c` (`threshold = (loop_has_call ? 1 : 2) * (1 +
  n_non_fixed_regs)` at loop.c:532; the movable test needs `n_times_set == 1`).
  The target keeps the constant INSIDE the loop in caller-save `$v1` — proof
  that GCC did not hoist it for the original source.
- **The fix is multi-set reuse, and the target names the register for you.**
  Routing the constant through the same C variable that later holds the
  `D_800A3444` read makes `n_times_set > 1` (not a movable) AND lands the
  pseudo in `$v1`, which is exactly the register the target uses for both
  roles. This is [[defeat-licm-hoist-var-reuse]] applied verbatim.
- **`val = val | mask; D_800A3444 = val;` beats `D_800A3444 = val | mask;`**
  by 7 points: it makes `val` the OR destination (matching `or $v1,$v1,$s2`)
  and raises `val`'s reference count over `j`'s, flipping the $v1/$a0
  assignment so val=$v1 / j=$a0 as in the target.
- **The last 3 points were a first-pass-scheduler tie.** At the inner-loop top
  `addu $s0,$s3,$a0` and `li $v1,1` are both ready at cycle 0 with equal
  priority; reorg.c then steals whichever lands FIRST into the inner
  back-edge delay slot and moves the loop label past it. Target steals the
  `addu`; our build stole the `li`. Simply swapping the two source statements
  did NOT change the outcome (measured: still 3). Interposing the
  already-required `j += 1;` statement between them DID (0). So the tie is
  broken by the intervening statement, not by the relative order of the two
  statements alone.
- **The read-modify-write spelling controls a load-delay nop.** The fused
  `D_800A3444 |= mask;` emitted `lw; nop; or; sw` after the fourth `rand()`;
  naming the read earlier (`last = rand(); val = D_800A3444; *sh = last & 7;`)
  reproduces the target's `lw $v1,%gp_rel(D_800A3444)` immediately after the
  `jal`, with the `andi`/`lui`/`addu`/`sh` cluster filling the load delay.

### Integration state (NOT done by this session — out of a grind session's surface)
- `regfix.txt:2521` (`func_800645B0: reorder 3,1,2 @ 1-3`) is now REDUNDANT:
  the sandbox drops it and the function matches without it. The operator/driver
  must run `engine retire func_800645B0` (full-build SHA1 verify, auto-rollback)
  and then `queue done func_800645B0`. A grind session may not edit regfix.txt,
  run retire, or commit.
- No other function was touched. `src/text1b.c` still contains cheat-asm in
  OTHER functions (the sandbox reports 328 cheat-asm instructions stripped
  file-wide); that is pre-existing and out of scope here.
