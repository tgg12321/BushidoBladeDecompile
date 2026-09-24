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
