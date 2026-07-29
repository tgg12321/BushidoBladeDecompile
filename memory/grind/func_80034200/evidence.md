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

## Session s3 (structural) -- floor 11 -> 10 -> 2 -> 0.  MATCHED, pure C.

`sandbox func_80034200 --disable all` = **0**, build_insns 40 == target_insns 40,
rules_dropped 0; `verify-oracle` = ok, build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked original. The form is
100% pure C: zero regfix/asmfix rules, zero `__asm__`, zero register-asm pins,
zero volatile, zero dead stores, zero unused locals, zero FAKE-family constructs.

### [s3] CORRECTION to the s2 corollary -- outer-LICM and the tail reload are NOT mutually exclusive
s2 banked "outer-LICM (which would place the preheader `li 3`) and the tail
reload are mutually exclusive under this compiler -- they cannot both come from
loop.c". That is FALSE. The discriminator is the DECLARED WIDTH of the loop-bound
variable, which s2's sweep never varied:

- `s32 n; ... n = D_800A389B;` -- the u8 global needs a widening conversion, so
  RTL expansion puts the load in a FRESH COMPILER TEMP and copies the temp into
  `n` as a separate insn. loop.c:scan_loop's movable gate (frozen source, lines
  688-701) accepts the temp via condition (2) `!REG_USERVAR_P && !REG_LOOP_TEST_P`,
  hoists it, and cse2 folds it against the entry-guard load. Reload gone. This is
  exactly the shape s2 measured, in every real-loop spelling it tried.
- `u8 n; ... n = D_800A389B;` -- no conversion, so the load's SET_DEST *is* the
  user variable. All three movable conditions now fail:
    (1) `!maybe_never` fails: loop.c:921-930 sets maybe_never past ANY CODE_LABEL
        or JUMP_INSN, and the inner loop supplies both well before the tail.
    (2) fails: REG_USERVAR_P(n) is true.
    (3) `reg_in_basic_block_p` fails: regno_first_uid[n] is the PRE-LOOP
        `n = D_800A389B;` insn, not the tail load, so it returns 0 at once.
  => the tail `lbu D_800A389B` is not a movable and survives inside a REAL
  do-while outer loop. Measured: variant b04 (`u8 n`) rel=Y; variants b01/b02/
  b03/b05 (`s32 n`, four statement placements) all rel=n.

### [s3] Restoring the real outer loop is what places the preheader `li 3`
With a real outer loop, the inner loop's LICM hoist of the constant 3 becomes an
invariant of the OUTER loop too and is hoisted a second time, landing in the
outer preheader at target's slot. No constant-holder local, no FAKE construct --
which retroactively confirms s2's decision not to seek a ruling on that form.
Floor 11 -> 10 with the guard written `n = D_800A389B; if (i < n)`. The `i < n`
spelling is load-bearing: it keeps the signed `blez`, whereas `if (n > 0)` on a
`u8` folds to `beq`.

### [s3] `base` as a strength-reduced induction variable fixes BOTH remaining residuals
Writing `base = &D_800F65F8 + i * 2;` inside the loop (dropping the pre-loop
initialisation and the tail `base += 2;`) makes base a giv:
- **Preheader ordering.** loop_optimize runs move_movables BEFORE strength_reduce,
  so the giv's preheader init is emitted AFTER the hoisted movables. Preheader
  becomes [lbu D_800A3874, li 3, la D_800F65F8] == target. A plain
  `base = &D_800F65F8;` source statement can never reach that position: source
  insns precede the movables, so the `la` always came out first.
- **The base<->shift register swap.** The giv is a fresh, high-priority allocno
  that takes $a1 directly; shift falls to $a2. Target triple (acc=$a0, base=$a1,
  shift=$a2) reached with NO ref-count mutation at all -- so the s2 frontier's
  "change a reference COUNT" programme was not needed and the instrumented-cc1
  probe it called for was never built.
The `&GLOBAL + i * 2` idiom is corroborated as original: the immediately preceding
already-matched function in the same file writes
`*(&D_800F65F8 + (D_800A3874 * 2)) = D_800A3898;` (src/code6cac_b.c:3846).
Floor 10 -> 2.

### [s3] The last 2 points were loop-head emission order
`p = base;` must be written BEFORE `end_p = base + 2;` (target: `addu $v1,$a1,$zero`
then `addiu $t0,$a1,2`). A 12-form sweep over every legal ordering of
{base-giv, useReal, p, end_p} plus the `end_p = p + 2` alternatives showed the two
effects are separable but coupled: 5 of 12 orderings keep the target register
triple, and of those the `...,P,E` orderings also give target's emission order.
Floor 2 -> 0.

### [s3] Method note -- allocno inputs are readable from `-da` with NO instrumented cc1
The s2 frontier's next-probe called for building an instrumented cc1 under
tmp/gccdbg to print allocno_n_refs / allocno_live_length. That is unnecessary:
cc1's `.lreg` dump already prints `Register N used R times across L insns` for
every pseudo, which are exactly global.c's `allocno_n_refs` and
`allocno_live_length`, and the `.greg` dump prints the resulting
`;; N regs to allocate:` priority order plus `;; Register dispositions:`.
`allocno_compare`'s priority is
`floor_log2(n_refs) * n_refs / live_length * 10000 * size` (read from the frozen
global.c). Reference counts are weighted by loop depth: a ref outside any loop
counts 1, inside one loop 2, inside two loops 3. Measured on the s2 score-11 form:
shift refs=9 live=29 pri=9310 (got $a1); base refs=5 live=21 pri=4761 (got $a2) --
i.e. base would have needed >=8 refs, or shift <=6, to flip by ref count alone.
`tmp/grind/func_80034200/s3/probe.py` prints all of this per variant in ~1s and is
reusable for any function on this project.
