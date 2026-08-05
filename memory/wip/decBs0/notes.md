# decBs0 — WIP (current state 2026-08-05)

`src/text1a.c:1252`. HEAD baseline: honest pure-C distance **58**, 39 regfix rules, plus a
`register s16 *fp_ptr asm("fp")` pin (cheat-asm — the sandbox strips it, hence honest 58 vs
`--keep-cheat-asm` 52). Measure: `wsl bash tmp/csz/d.sh decBs0 text1a` /
`wsl bash tmp/csz/frame.sh decBs0 text1a`.

**Status: owner-blocked on a per-file `-G8` ruling.** The residual mechanism is proven and the
C form that closes it is known; only the build-flag question remains.

## Standing constraint from the layer-2 FAIL (commit a6a83d99)

The stride-3 pointer walk over `tbl[29]/[32]/[35]` was REJECTED — mechanism-motivated
`MEM_IN_STRUCT_P` coercion outside [[walking-pointer-serializes-parallel-loads]]'s scope.
**The whole store side stays closed**; reopening it needs fresh SOTN evidence + an owner
ruling. Cleared and now in use: the real `do {} while (outer < 2)` rewrite and the pin removal.
Cleanup on any landing: stray comment at `regfix.txt:927`.

## Candidates (banked as diffs; tree left clean)

| file | score | insns | notes |
|---|---|---|---|
| `candidate_11.diff` | 11 | 132 vs 134 | pin removal + real `do`-loop + plain indexed stores. Frame/RA EXACT (72, `vars=16`, `$fp`=fp_ptr, `$s3`=tbl). |
| `candidate_g8_array.diff` | **2** at -G8 / 16 at -G0 | **134 vs 134** | candidate_11 + colour triple as `extern s16 g_anim_select[3]`. Frame exact at -G8. |

## The residual is a memory-dependence problem — PROVEN

`sched_solver` (model exact for text1a, 528/528 blocks) localises the entire residual to
**block 19**; every other block is goal == identity in both passes.

**Do NOT trust `--goal-from-target` here.** It reports `GOAL INVALID (2 dependence violations)`
and blames duplicate text; the real cause is that target's three `lhu $2,SYM` share an operand
skeleton and mis-pair. Both streams hold the same 14 insns — only the `lhu`s move. Hand-derived
pass-2 goal (pick order):
`257,255,253,250,248,245,243,240,235,238,230,228,223,221,215,306,219,213,303,211,206,200,204,198,196,193,191,189,187`

Searches against that goal (`tmp/csz/bs0_perturb*.sh`): depth 1 `--atoms luid,luid_move` → **no
vector**; depth 1 ALL atoms (2163 pass-2 / 1906 pass-1) → **no vector**. Applying the
hypothesised edges by hand (`tmp/csz/bs0_edges.py`): **each colour load depending on EVERY
preceding `tbl[]` store — 12 edges — reproduces target's pass-2 order EXACTLY**; the minimal
one-edge-per-pair version does not. A 12-edge change is why no depth-1/2 atom search could
ever find it.

## Why the aggregate is the fix (mechanism, measured)

`sched.c:true_dependence` (821-838) drops the edge via exclusion #1, which requires
`!MEM_IN_STRUCT_P(load) && !rtx_addr_varies_p(load)`. **An in-struct load at a FIXED address
defeats exclusion #1 without triggering exclusion #2** (which needs a *varying* in-struct
load). Per `expr.c:4589-4700`, a **constant-index** `ARRAY_REF` falls through to the shared
handler at `expr.c:4888`, which sets `MEM_IN_STRUCT_P = 1` on a `plus_constant(symbol, off)` —
in-struct, fixed. Exactly the shape needed.

Use-site evidence for the aggregate: `func_80041E10` writes all three as R/G/B from one packed
colour, `func_800420D0` sets `[0] = -1` as a sentinel, decBs0 copies them into a
3-halfword-stride matrix column.

With `extern s16 g_anim_select[3]` the block order becomes **target's, exactly**
(`negu/sh 8/li 1/sh 10/sh 12/lhu/nop/sh 58/lhu/nop/sh 64/lhu/j/sh 70`). The scheduling class
is CLOSED. At `-G0` the cost is a base register (`la $23,g_anim_select` in the *prologue*,
above loop head `.L285`, addressed `0/2/4($23)`) which burns `$s7` + save/restore and displaces
fp_ptr → 16, 138 insns. At `-G8` the hoist disappears, loads become direct
`lhu $2,g_anim_select / +2 / +4` → **2, 134 vs 134, frame exact**. The leftover 2 is a
*relocation-form* artifact only (ours `lhu v0,2(gp)` + `GPREL16 g_anim_select`; target
`lhu v0,0(gp)` + `GPREL16 D_800A323A`) — same address, same linked bytes. Full-build SHA1 is
the only way to settle it.

## Path 2 (defeat the -G0 hoist in pure C) — EXHAUSTED; it is a cost-model decision

1. **Target really is gp-relative** (`asm/funcs/decBs0.s:116-122`):
   `lhu $v0, %gp_rel(D_800A3238/A/C)($gp)`, three direct loads, no base register. Target's
   `$s7` holds `D_800A9B28` and `$fp` holds `D_800F62E0` — **no spare callee-save** exists.
2. **sdata_syms.txt + maspsx cannot dissolve it.** They run strictly downstream of cc1 (the
   Makefile passes `--sdata-syms`/`--sdata-funcs`/`--sdata-exclude` to **maspsx only**; cc1
   always gets `-G0`). Measured: maspsx rewrites every direct reference
   (`sh $2,g_anim_select+2` → `%gp_rel(g_anim_select+2)($gp)`) but leaves **`la $23,...`
   untouched** — by then cc1 has already spent the register and the RA cascade.
3. **Spelling is not the lever.** A struct (`{ s16 r, g, b; }`, `.r/.g/.b`) measures
   **identically: 16, 138 insns** — `ARRAY_REF` and `COMPONENT_REF` share the `expr.c:4888`
   path. [[defeat-licm-hoist-var-reuse]] does not apply: its lever is a *C variable's* pseudo
   going multi-set, and no C variable holds this address — cc1 invents the pseudo itself.
4. **Target's loads are fixed-address in RTL, so they must be `MEM_IN_STRUCT_P` for target's
   dependence to exist** — the original source DID use an aggregate. `-G8` is the only measured
   config where cc1 both treats the load as in-struct and declines to CSE a base: positive
   evidence the original TU was built with small data enabled.
5. So at `-G0` the base register is chosen by the **address-cost model**, not the source shape
   (3 refs × 2 insns beats `la` + 3 × 1 insn). Every remaining `-G0` lever is a coercion —
   volatile, barriers, or `extern s16 X[1]` one-element arrays.

## The ruling needed

`Makefile:104` has `GP_FILES :=` empty, with `cc_flags_for`/`maspsx_flags_for` already wired to
switch a listed file to `CC_FLAGS_GP` + `MASPSX_FLAGS_GP`. Mechanism exists and is documented,
but unused, and [[compiler-flags-canonical]] settled flags project-wide. Measured blast radius:

- Full build, HEAD source + `GP_FILES := text1a`: **MISMATCH, −24 bytes**.
- cc1-level `-G0` vs `-G8` across the TU's 37 functions: only **2 change** —
  `gnd_land_hit_char_die_main` (−2) and `func_80040D48` (−1). decBs0 itself unchanged at HEAD
  source. Total cc1 delta −3.
- The other −3 is likely `MASPSX_FLAGS_GP` (`Makefile:22`) being a *reduced* flag set: it omits
  `--expand-lb`, `--expand-lb-funcs`, `--multu-funcs`, `--expand-dest-funcs`,
  `--label-nop-funcs`, `--sdata-funcs`, `--sdata-exclude`. Looks like an infrastructure gap.

## Next

1. **Ruling YES**: bring `MASPSX_FLAGS_GP` to parity, apply `candidate_g8_array.diff`, retire
   the 39 rules + pin, re-match the 2 perturbed siblings, full-build SHA1.
2. **Ruling NO**: path 2 is exhausted; anything further at `-G0` is a coercion, and the store
   side stays closed.
3. **Do NOT commit a completion.** The owner runs the gate.
