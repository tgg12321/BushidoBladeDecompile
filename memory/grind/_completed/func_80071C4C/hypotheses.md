# func_80071C4C matching ledger

## Reconstruction evidence

- Canonical gate: `C`, hand-coded tier `LOW`; pure C is mandatory.
- Target: 270 instructions; initial state is whole-body `INCLUDE_ASM`.
- `func_8006F100` is a matched semantic sibling using the same
  `Obj_8006F100` / `Spr_8006F100` object model and the same sprite-selection,
  extent, position-table, and `func_80073C78` call sequence.
- The tail advances `D_800A3550` by 8, clamps at 255, updates mode state, copies
  the state and motion bytes into 10-byte records rooted at `D_800A3568`, and
  finally calls `func_8006F038(arg0)`.

## Measurements

| Session | Candidate | Honest distance | Notes |
|---|---|---:|---|
| s0 | whole-body stub | 270/270 | No C body. |
| s1 | plain semantic reconstruction | 80/270 | 253 insns. Missing target's skip for state 5/16; sprite-field initialization was too late, letting constants propagate; tail call/store order and strength-reduced byte indices differed. |
| s2 | semantic fixes + target field lifetime | 10/270 | 270 insns. Only the mode-mask/load schedule and register assignment of the two real loop offsets remain. |
| s3 | named destination offsets; split mode update | 9/270 | Loop register assignment closed. Pointer-typed state local changed the desired expression's register seats; retain the real address as the project's existing `s32` address type instead. |
| s4 | mask at real bitfield use | 0/270 | 270 insns. Keeping `mode` as the call result and applying `& 0x3F` only in the real write yields the target load/mask schedule. |

## Full-cluster ablation

- Plain form: removed all six real named offsets (`ctx`, `idx`, and the two
  loop-local `dst`/`ctx` pairs) together and inlined their expressions.
- Measured with `sandbox --disable all --candidate ... --diff`: **52/270**,
  273 instructions, 12 source-level and 13 operand-only hunks.  The compiler
  promotes extra address bases into `$s` registers and grows the frame from
  0x78 to 0x80.  The named values are therefore load-bearing as a cluster.
- Each local is written once, holds a real consumed offset/index, survives
  neutral naming, and adds no dead code.  They are ordinary named
  intermediates under the 2026-08-31 ruling, not Tier-2 constructs.

## Rejected object-model integration experiment

- Tested a declaration-level `ReplayMotionSelections` merge for the
  `D_800A3560` byte cluster.  Existing consumers remained byte-identical under
  a full build, but this target regressed 0 -> 2 (271 instructions): the
  original uses the separately classified small-data symbol `D_800A3561` for
  the scalar current-player read while using absolute indexed addressing for
  `D_800A3560`/`D_800A3562`.  A single aggregate base emits `lui; lbu +1`
  instead of the target's one `lbu %gp_rel(D_800A3561)`.
- Rejected and fully reverted.  No toolchain/gate-list change was made; the
  body retains the already-reviewed declarations and spelling used by matched
  sibling `func_8006F100` (decision PASS 2026-09-22).

## Construct audit

- Initial candidate uses only ordinary C and the sibling's established types.
- `idx` is a once-written, real, consumed intermediate also present in the
  matched sibling; it expresses the selected final sprite entry.
- No fake/no-semantic-purpose construct has been introduced.

## Final verification

- Live `sandbox func_80071C4C --disable all --diff`: **0/270**, 270 target
  and 270 build instructions, zero source-level and zero operand-only hunks.
- `verify-oracle --rebuild --allow-dirty`: `ok: true`, build and artifact
  SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
- `tools/check_completion_integrity.py`: clean (silent success).
- `tools/audit_asm_cheats.py --check-new`: clean (silent success).
- `git diff --check`: clean.

## Cleanup 2026-09-29 — per-local measurement of the tail copy loops' `dst`/`ctx` (retro-audit finding)

The 2026-09-29 retro-audit found only a six-local cluster ablation (52/270) for the four copy-loop locals.
Each was re-measured on its own (cleanup-locals/gen.py; `sandbox --disable all`, landed body = 0/270).
Name AxxBxx = loop 1 (D_800A3554 bound) / loop 2 (D_800A3558 bound); digits = dst, ctx; 1 kept, 0 inlined.

| variant | score | insns | | variant | score | insns |
|---|---:|---:|---|---|---:|---:|
| A11_B11 (landed) | 0 | 270 | | A00_B00 (all four inlined) | 12 | 268 |
| A01_B11 (loop 1 dst inlined) | 4 | 270 | | A11_B01 (loop 2 dst inlined) | 4 | 270 |
| A10_B11 (loop 1 ctx inlined) | 5 | 269 | | A11_B10 (loop 2 ctx inlined) | 5 | 269 |
| A01_B01 | 8 | 270 | | A10_B10 | 10 | 268 |

Every one of the four is individually load-bearing; none can be removed. Respellings measured
(no locals unless stated): `((u8 *)D_800A3568)[i * 10]` 12, `+ 1` base variant 12, `10 * i`/`3 * i` 12,
`((u8 (*)[10])D_800A3568)[i][k]` with either source form 12, `*(D_800A3560 + i * 3)` 12; with `ctx` only:
`((u8 (*)[10])D_800A3568)[i][k]` 8, embedded `D_800A3560[ctx = i * 3]` 2; ctx declared before dst 8;
`u8 *rec = (u8 *)(D_800A3568 + i * 10); ... *rec / rec[1]` 0 (same two locals per loop, no reduction).

Mechanisms (instrumented cc1 `tools/gcc-2.7.2/cc1`, dumps in cleanup-locals/):
- `ctx` (both loops): with the index in a named local the read's address is `(plus (reg/v ctx)
  (symbol_ref D_800A3560))`, accepted as is by explow.c memory_address (explow.c:419
  GO_IF_LEGITIMATE_ADDRESS); inlined, the address is `(plus (mult i 3) sym)`, which is not legitimate, so
  memory_address forces the whole sum into a pseudo (explow.c:447 force_operand; rtl insn 598). loop.c
  strength_reduce (loop.c:5527/3992, "giv at 600 combined with giv at 598 ... reduced to reg 335") then turns
  the entire address into one pointer giv (`lui/addiu` init, `lbu 0(a1)`), where the target keeps the
  `D_800A3560(a1)` sym+index form and the `i*3` giv. Loop 2 identical (insns 662/664).
- `dst` (both loops): loop.c strength_reduce reduces both the i*10 and i*3 givs to new pseudos; the giv that
  comes first in the loop body gets live length 14, the other 15 (BB2_ALLOC_DEBUG, alloc_*.txt). global.c
  allocno_compare (global.c:643, floor_log2(refs)*refs/live_length) allocates the live-length-14 one first,
  into $4. With `dst` computed before `ctx`, i*10 is first -> $a0, i*3 -> $a1 (target). Inlined, i*10 is
  computed inside the store after the `ctx` statement, so i*3 takes $a0 (loop 1: pseudo 335 = i*3 in $4,
  333 = i*10 in $5; loop 2: 331 = i*3 in $4, 329 = i*10 in $5): the 4-point operand-only swap.
