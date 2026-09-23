# func_80021424 — evidence (manual session 2026-09-23)

COMPLETED-C in one manual session (first-ever work on this function; no prior ledger).
Oracle GREEN 2026-09-23 with the body in `candidate.c` spliced into src/code6cac.c.

## Floor trajectory (sandbox --disable all, prototype patched via tmp wrapper)
- 297 (INCLUDE_ASM) → 63 first draft (u16 id param) → 54 (`s32 id`: target keeps
  id un-truncated in a3, no andi) → 31 (per-character record typed: u16 [][3]
  table at +0x4E, s16 modulus at +0x14) → 7 (no `ch` temp; rec+0x4A re-read per
  use) → 2 (0x7FF3 tail spelled `f4E[f86][t + 3]`; `f4E[f86 + 1][t]` swaps the
  addu operand order, score 7) → residual 2 = jtbl lo16 addend (rodata order).

## Rodata findings (needed for the oracle, invisible to the sandbox)
- The compiler's switch table must follow D_800100FC in .rodata → D_800100FC's
  definition moved from end-of-file to just before func_80021424.
- The removed `jtbl_80010414[6]` placeholder carried a 6th zero word that the
  5-entry compiler table doesn't emit → `const u32 D_80010428[1] = {0}` at EOF,
  same pattern as D_800100E0 after func_8001C8DC's table.

## Object-model changes (code6cac.h)
- `D_800A3860`: `s32` → `Tbl800A3860Entry *[]` (per-character record pointers,
  indexed by rec+0x4A); sibling uses in func_80020D70 / func_800213A0 re-spelled.
- `D_801027B0`: `s32` → `s32 [][5]` (20-byte per-character stride, same as the
  existing `D_801027BC[][5]` declaration).
- Caller prototype `func_80021424(u8 *, u16, u8 *)` → `(u8 *, s32, u8 *)`.
- Also: in the first branch, a `ch` temp with `D_801027B0[ch][0]` schedules the
  ch*20 before id*6 (2 insn-run mismatch); inlining rec+0x4A fixes it.

## Layer-2 review 1 (2026-09-23): FAIL → remedied
- FAIL on the 0x7FF3 tail `f4E[f86][t + 3]` (out-of-row subscript on a declared
  `u16 [15][3]`); banked as rejected/t-plus-3-out-of-row.c.
- Remedy (reviewer's suggestion): model the +0x4E region as in-bounds fields
  `u16 f4E[3]; u16 f54[3][3]; u16 f66[11][3];` → accesses f4E[f84]/f4E[f86],
  f54[f86][t], f66[id - 0x7FF5][f86]. Oracle GREEN with this layout (same bytes;
  the constant 0x54 folds like the `[t + 3]` form did). `f4E[f86 + 1][t]` on the
  old [15][3] model remains measured-negative (swapped addu operands, score 7).
- Everything else in review 1 passed without objection (ch re-reads, fallthrough,
  sibling re-spellings, s32 prototype, D_80010428 tail word, D_800100FC move).
