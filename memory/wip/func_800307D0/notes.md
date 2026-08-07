# cpu_check_tubazeri_2 WIP — floor 9→4

## TL;DR (2026-06-13)
Pure-C candidate at sandbox floor **4** (HEAD: 9). Reviewer PASS. Resume by
applying `candidate.c` to `src/code6cac_b.c` and confirming sandbox=4.

## Resume steps

1. `& tools/eng.ps1 verify-oracle --rebuild` (clean baseline).
2. Replace `src/code6cac_b.c:2465-2515` (function `cpu_check_tubazeri_2`) with
   `memory/wip/cpu_check_tubazeri_2/candidate.c` body.
3. `& tools/eng.ps1 sandbox cpu_check_tubazeri_2 --disable all` → expect score 4.
4. Continue iterating from there — DO NOT re-derive the rejected forms in
   `meta.json`.

## What survives (the 4 real diffs)

After the 6 reg-rename rules and the (0,...) comma cheat go away cleanly,
4 instruction-level diffs persist:

| idx | ours | target | category |
|---|---|---|---|
| 25 | `addu s2, s0, v0` | `addu s2, v0, s0` | commutative-addu operand swap |
| 37 | `addiu v1, s2, 818` | `move v1, s2` | strength-reduce ptr precompute |
| 38 | `lhu v0, 2(v1)` | `lhu v0, 820(v1)` | strength-reduce displacement encode |
| 40 | `sh v0, 0(v1)` | `sh v0, 818(v1)` | strength-reduce displacement encode |

The 3 idx-37/38/40 diffs are ONE strength-reduction decision: our GCC
strength-reduces `ptr + 0x332` (and `ptr + 0x334`) into a new BIV `q = ptr +
0x332` at loop entry, then accesses with small displacement (2 / 0) per iter.
Target's GCC keeps `v1 = ptr = s2` and uses large displacement (820 / 818)
per iter. Same iter count, same operation count, different register layout.

## Levers attempted (ALL ruled out, see `meta.json.rejected_forms`)

- Block-local v0 split (Lever A) — broke equal-branch dataflow, raised floor.
- u16 ptr stride-1 instead of s32 stride-2 — same strength-reduction outcome.
- For-loop with comma-init — shifted prologue scheduling, cascaded diffs.
- Index-based loop without ptr — strength-reduces same way + adds prologue.
- v0-first add via uintptr_t cast — inserts conversion insns, doesn't swap rs/rt.
- `((u8*)0 + v0 + (uintptr_t)a0)` byteptr-first — same problem.
- Explicit `s2_int = v0 + (s32)a0` intermediate — same.
- Reorder declarations to put v0 before/after others — no allocation change.
- Move `ptr` decl inside the `if` block — no codegen change.

## Why the 4 persist (best understanding)

**operand swap (idx 25)**: GCC canonicalizes commutative-`plus` operands by
pseudo-register number. Function-param `a0`'s pseudo < local `v0`'s pseudo
(params come first), so `(plus a0_pseudo v0_pseudo)` emits `addu rs=a0_hw,
rt=v0_hw`. Target's compiler had `v0_pseudo < a0_pseudo` for some unknown
RTL-gen reason. C-level reshapings tried so far don't flip this ordering.

**strength-reduction (idx 37/38/40)**: GCC's `loop.c:strength_reduce()` sees
`ptr + 0x332` and `ptr + 0x334` as GIVs of BIV `ptr` and creates a new BIV
`q = ptr_init + 0x332`. The `benefit` calc decides this is profitable, so
old BIV `ptr` gets eliminated and `q` becomes the loop's BIV. Target's GCC
made the OPPOSITE benefit decision — kept `ptr`, didn't strength-reduce.
We don't know what RTL-shape flips that decision; pure-C reshapings tried
don't move it.

## Concrete next levers (un-attempted; see `meta.json.next_hypotheses`)

- Instrumented `cc1` dump with loop.c tracing — examine the benefit calc on
  our build and identify the specific RTL state difference vs target.
- Decomp-permuter starting from candidate.c base (sandbox 4) — randomization
  may find a structural lever not in the manual playbook.
- m2c-reconstructed structure — examine what control-flow / loop shape m2c
  produces from target's asm; may reveal a different C form to try.

## Constraints

- `-fno-strength-reduce` is FORBIDDEN as a flag-hunt per
  `[[compiler-flags-canonical]]` / `[[no-compiler-divergence]]`. The
  `NO_SR_FILES` Makefile knob requires user policy sign-off.
- All current candidate constructs are SOTN-allowed families per the
  cheat-reviewer PASS. No coercion constructs are present.
