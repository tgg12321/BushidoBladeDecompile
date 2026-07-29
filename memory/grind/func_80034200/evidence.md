# Evidence bank — func_80034200

## Shape
- File: `src/code6cac_b.c` (line ~3850).
- Void void function. Writes `g_disp_enable = DISP_LOADING` (i.e. `D_800A3768 = 0xA`) up-front, then a nested loop that ORs shifted bytes into an accumulator and finally stores `D_800A3784 = acc`.
- Target 40 insns, frame -8 bytes with **no stores** (phantom slot).
- Callee: none. Uses only caller-saves ($a0-$a3, $t0-$t3, $v0-$v1). No callee-saves, no `sw $ra`.

## Sandbox measurements (session s1)
- Canonical verdict: C (pure-C target, distance 16, well under 50 threshold).
- **Cheat-scaffold source (as-inherited)**: sandbox `--disable all` → **score 16** (build_insns 40).  Uses `register asm("$N")` pins on all locals + `volatile char _pad[8]`. Pin-based allocator coercion is a forbidden cheat family (`inline-asm-policy` / `no-new-park-categories`), so 16 is a cheatful floor, not real progress.
- **Clean pure-C candidate.c**: sandbox `--disable all` → **score 21** (build_insns 37 vs target 40). This is the HONEST starting floor.
- Diagnose reports `LARGE` (d21). Metric divergence between sandbox masked score and diagnose is expected; both point at the same 21-insn honest gap.

## What the 21-diff decomposes to (clean-build vs target objdump — tmp/grind/func_80034200/s1/{clean,build}.dis)
Three components, cleanly separable:

1. **Missing prologue/epilogue** (2 insns): target has `addiu $sp,-8` / `addiu $sp,8` with no stores in between. Clean C emits no frame at all. Classic **phantom frame slot** (see [[phantom-frame-slots-gcc272]]) — some live-but-untouched local, or GCC's frame-size accounting for spill space it never used, holds -8 open in target.
2. **Missing outer-loop-tail global reload** (1 insn net): target re-reads `D_800A389B` inside the outer-loop tail (`lui/lbu $v0, D_800A389B; slt $v0, $a3, $v0`). Our C `while (i < D_800A389B)` gets CSE'd because there is no intervening write, so GCC hoists to a register (`move $t3, $v1` — the initial load) and reuses it. Net -1 (we save 2 reload insns, spend 1 move insn).
3. **Register-rename cluster** (~18 diffs — the residue after the 2 above). Same COMPUTATION, different register choices. Notable pairing:
   - target uses `$a0` for the OR-accumulator, `$a3` for outer counter i, `$a2` for shift amount, `$a1` for base pointer, `$t3`/`$t2` for innerBound/const-3.
   - clean-C build uses `$t0` for accumulator, `$a2` for i, `$a0` for shift, `$a5→a1` for base, `$t4`/`$t2` for innerBound/const-3.
   - Function has no formal parameters, so all these are freely-allocated pseudos. GCC's `global.c` priority pass picked a mirror-image assignment.

## What the cheat scaffold was doing (why it "worked")
- `register T x asm("$aN")` on every local pinned all live values into their target arg-register slots directly, sidestepping the register-rename cluster (component 3 above).
- `volatile char _pad[8]; (void)_pad;` forced the phantom -8 frame (component 1).
- `a0 = (count & 0) + 0;` was a hand-crafted expression to force a specific initialization pattern (component 3-related — steering the `move a0, zero` into a particular delay slot).
- Result: 16-instruction diff with all cheats in place; 21 without. These cheats are literal instances of `inline-asm-policy`'s expanded cheat catalog (register pins, `dead-vars-local-array` unwritten array) — NOT sanctioned techniques.

## Toolchain state
- Sandbox `--disable all` scoring works, `strip_cheat_asm=true`; but note register-asm PIN declarations (`register T x asm("$N")`) are compile-time allocator hints and are NOT syntactically `__asm__` blocks — they are not stripped by cheat_asm_stripped, so the 16 score for the pinned form reflects GCC actually honoring the pins.
- `cheat_asm_stripped=347` in the clean build reflects OTHER functions in `code6cac_b.c` (this func has none).

## Artifacts
- `tmp/grind/func_80034200/s1/orig.c` — the as-inherited src snapshot (cheat scaffold).
- `tmp/grind/func_80034200/s1/build.dis` — objdump of the cheat-scaffold sandbox .o (floor 16).
- `tmp/grind/func_80034200/s1/clean.dis` — objdump of the clean pure-C sandbox .o (floor 21).
- `memory/grind/func_80034200/rejected/register-asm-pins-plus-volatile-pad.c` — the cheat form (do not re-propose).
- `memory/grind/func_80034200/candidate.c` — clean pure-C starting point applied to src/ this session.

- [s1] canonical verdict C, distance 16, target 40 insns (pure-C target)

- [s1] sandbox --disable all with as-inherited src = 16 (cheatful); with clean pure-C candidate = 21 (honest floor)

- [s1] target has frame -8 with ZERO stores between prologue and epilogue -- phantom frame slot

- [s1] target re-reads D_800A389B at outer-loop tail via lui+lbu; clean-C hoists to a register (CSE)

- [s1] residual is a register-rename cluster: target uses $a0 accumulator, $a3 outer-i, $a2 shift; clean-C build uses $t0/$a2/$a0 respectively

- [s1] function is void-void, no calls, no callee-saves used, no $ra save -- allocator has $a0-$a3 free from entry

- [s1] cheat_asm_stripped byte counts (366 vs 347) reflect other functions in code6cac_b.c; this func contains no __asm__ blocks

## Session s2 (structural) — floor 21 -> 11, all pure C

Three levers landed, each measured with `sandbox --disable all` AND with a cc1
`.s` dump (`tmp/grind/func_80034200/s2/probe.sh`, plus a standalone-TU harness
`sweep2.py` that reproduces the in-file codegen exactly — same registers, same
`vars=8` — so variants cost ~1s each instead of a full sandbox run).

- [s2] **L1 goto-form outer loop => the D_800A389B tail reload appears.** A `goto`
  loop carries no NOTE_INSN_LOOP_BEG, so loop.c never runs scan_loop on it and
  the tail `lbu D_800A389B` is not hoisted. Every real-loop spelling hoists it
  (see rejected/real-outer-loop-always-hoists-tail-read.c for the full gate
  walk-through). Floor 21 -> 20 -> 16 as the rest of the shape came in.
- [s2] **L2 `base += 2;` BEFORE `i++;` in the tail** => cc1 schedules `addiu i,i,1`
  into the tail lbu's load-delay slot; the maspsx `#nop` disappears and
  build_insns goes 41 -> **40 == target_insns**. Floor 16 -> 14. (Instance of
  [[loop-counter-fills-load-delay]].) The source-level reorder `s32 n =
  D_800A389B; i++;` does NOT do it — the lever is specifically which of the two
  increments is written first.
- [s2] **L3 OR straight into `acc` in both arms (drop the `v0` temp)** => acc's
  allocno priority rises to the top of the list and acc lands in `$a0` exactly
  like target. Floor 14 -> **11**. `.greg` allocation order changed from
  `86 77 82 72(shift) 74(acc) 76(base) ...` to `77 74(acc) 82 72(shift) 76(base) ...`.
- [s2] **The -8 phantom frame is free with this shape**: cc1 reports
  `.frame $sp,8 # vars= 8` for every goto-form / real-loop variant, and `vars= 0`
  only for the s1 hand-written do-while-with-explicit-guard form. H1 closed
  without any frame-coercion construct.
- [s2] Final residual (11) decomposes into exactly two items: (a) `addiu $t2,$zero,3`
  sits in the outer loop head instead of the preheader (~6 positional diffs);
  (b) one register swap, ours shift=$a1/base=$a2 vs target base=$a1/shift=$a2
  (~5 diffs). Everything else — all 40 instructions, the frame, both global
  reads, p/end_p/useReal/innerBound/const3/i/acc — matches.
- [s2] **Declaration order and statement order are INERT for this function's RA.**
  14 declaration-order / init-order / tail-order permutations and 15 further
  statement-level mutations (endp-from-p, `p != end_p`, shift-before-or,
  `base = base + 2`, s16 narrowing of useReal and innerBound, inlined bound,
  negated compare, advance-early) all produced byte-identical allocation. The
  allocno priorities are not tied — `allocno_compare` falls back to allocno
  number only on an exact tie, and changing the pseudo numbering (via
  declaration order) changed nothing.
- [s2] **global.c priority formula** (read from the frozen source):
  `floor_log2(n_refs) * n_refs / live_length * 10000 * size`. `floor_log2` is a
  step function, so a single ref crossing a power of two is a large lever — this
  is why dropping one `shift` reference (the single-`sllv` form w18) flipped the
  whole rotation to target, and why statement shuffling that preserves ref counts
  does nothing.
- [s2] **Constant-holder in the preheader does NOT lower the floor.** Binding
  `emptyVal = 3;` between the `innerBound` and `base` assignments puts
  `li $11,3` in the preheader at target's slot AND fixes base to `$a1`, but it
  swaps innerBound/const3 ($10<->$11) and swaps shift/i ($a3<->$a2). Measured
  sandbox = **11, identical to the clean form**. So the FAKE-family construct
  buys nothing here and no owner ruling is needed on it.
- [s2] Making the INNER loop a goto loop too lowers shift's loop_depth weight and
  does flip base above shift — but with `3` unhoisted both arms end in an
  identical `sll $2,$2,$sh`, jump.c's find_cross_jump merges them, and
  build_insns drops to 31 (target 40). The `li 3` hoist is what keeps the two
  `sllv` distinct.

- [s2] [s2] Honest pure-C floor moved 21 -> 20 -> 16 -> 14 -> 11 across four measured structural levers; every step verified with `sandbox func_80034200 --disable all`.

- [s2] [s2] build_insns is now 40, exactly equal to target_insns 40 (was 37 at the s1 clean baseline and 41 mid-session).

- [s2] [s2] The committed candidate is 100% pure C: no register-asm pins, no __asm__, no volatile, no dead stores, no unused locals, no padding array. The s1 cheat scaffold has been fully removed from src/code6cac_b.c.

- [s2] [s2] Everything in the function now matches target except two items: (a) `addiu $t2,$zero,3` sits in the outer loop head instead of the preheader (~6 positional diffs), (b) one register swap, ours shift=$a1/base=$a2 vs target base=$a1/shift=$a2 (~5 diffs). The frame, both global reads, the delay-slot fills, and p/end_p/useReal/innerBound/const3/i/acc all match.

- [s2] [s2] A standalone-TU harness (tmp/grind/func_80034200/s2/sweep2.py) reproduces the in-file codegen EXACTLY - same registers, same vars=8, same instruction sequence - so structural variants cost ~1s each instead of a full sandbox run. Reusable for any future session on this function.

- [s2] [s2] global.c:allocno_compare = floor_log2(n_refs)*n_refs/live_length*10000*size, with an allocno-number tiebreak only on exact ties. floor_log2's step behaviour is why a single reference change reorders everything and why order-preserving mutations are inert.

- [s2] [s2] loop.c/jump.c mechanism reading is banked in the rejected-form headers: scan_loop's three-way movable gate, invariant_p's MEM case, rtx_addr_can_trap_p(SYMBOL_REF)==0, move_movables' threshold test, and duplicate_loop_exit_test's REG_LOOP_TEST_P marking.

- [s2] [s2] Structural constraint proven: outer-LICM and the tail reload are mutually exclusive under this compiler, so the preheader `li 3` cannot come from loop.c in any form that also has the reload.
