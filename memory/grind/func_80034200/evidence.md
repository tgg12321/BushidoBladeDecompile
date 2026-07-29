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
