# Evidence bank — func_80052B7C

## s1 (recon) — baseline + object model

- `canonical func_80052B7C` → verdict **ASM-WHOLE**, reason "whole-function canonical-asm
  grant in inline_asm_canonical.txt". This function ALREADY carries an owner authorization
  (`inline_asm_canonical.txt:340`): "LIBGTE matrix-vector leaf (4-arg form): 8x ctc2 $0-$7 ...
  from *a0 words + *a1 halfwords, lwc2 $0/$1, two unfilled GTE latency nops, mvmva 1,0,0,0,0,
  swc2 $9/$10 and swc2 $11 IN the jr-ra delay slot (0x80052BE0). Verbatim PsyQ hand-written
  asm, unreachable from C under GCC 2.7.2. Sibling of authorized func_80052B44.
  Owner-authorized 2026-08-06 (canonical-asm grant, docs/grind/decisions.md commit
  f010b4b6; evidence packet docs/grind/auth-packets-2026-08-06.md)."
- `sandbox func_80052B7C --disable all` → score 24, `no_c_body: true` (src/main/3AB48.c:115
  is still `INCLUDE_ASM("asm/funcs", func_80052B7C);` — literally zero C has ever been
  written for this function). `target_insns` 24 matches `asm_insns`+rest from canonical (24
  total asm insns per canonical output).
- No DATA MODEL section present in the task brief for this function (it touches no globals —
  pure register-in/register-out leaf) — the recon DATA-MODEL-FIRST requirement does not apply
  (no flagged symbol signals). No `OBJECT MODEL:` entry needed.

## s1 — instruction-level decode of asm/funcs/func_80052B7C.s

24 insns, 5 logical chunks, all cop2/GTE, no branches except the trailing `jr $ra`:
1. `lw $t0-$t4, 0x0/0x4/0x8/0xC/0x10(a0)` + `ctc2 $t0..$t4 -> CR0..CR4` — loads a0's first 20
   bytes (5 words) into GTE control regs 0-4 (the 3x3 rotation matrix slot).
2. `lh $t5-$t7, 0x0/0x2/0x4(a1)` + `ctc2 $t5..$t7 -> CR5..CR7` — loads a1's first 3
   **halfwords** (sign-extended) into GTE control regs 5-7 (TRX/TRY/TRZ, normally 32-bit).
3. `lwc2 $0,0(a2)` / `lwc2 $1,4(a2)` — loads a2's first 8 bytes (one VECTOR/SVECTOR-shaped
   value) into GTE data regs 0/1 (VXY0).
4. two `nop` (GTE latency) + `mvmva 1,0,0,0,0` (sf=1,mx=0,v=0,cv=0,lm=0).
5. `swc2 $9,0(a3)` / `swc2 $10,4(a3)` / `swc2 $11,8(a3)` (last one in the `jr $ra` delay
   slot) — stores GTE data regs 9/10/11 (IR1/IR2/IR3) to a3.

## s1 — vendored-macro mapping (`include/psxsdk/inline_o.h`, `gtemac.h`)

Checked every vendored macro whose body touches CR0-4, CR5-7, DR0/1, or does an mvmva/stores
DR9-11 (`grep -n "define gte_" inline_o.h` + manual read of each hit):

- Chunk 1 **matches exactly** `gte_SetRotMatrix(r1)` (`inline_o.h:272-283`): same 5 lw + 5
  ctc2 into CR0-CR4, same byte offsets 0/4/8/C/10. (Byte-identical to `MATRIX.m[3][3]+pad`,
  20 bytes — `include/psxsdk/libgte.h:29-33`.)
- Chunk 2 has **no vendored-macro match**. Every `ctc2 ... $5`/`$6`/`$7` site in
  `inline_o.h` was enumerated (12 total: `gte_SetTransMatrix` l.325-333, `gte_ldtr`
  l.334-341, `gte_SetTransVector` l.342-350, `gte_mvlvtr` l.1087-1094, `gte_FlipTRX`
  l.1189-1194) — every one of them loads/derives CR5-7 from **32-bit words** (`lw`) or
  register-to-register ops (`mfc2`/`cfc2`), never from `lh` (sign-extended halfwords). No
  Sony macro in the vendored headers loads TRX/TRY/TRZ from a 3x s16 source. This chunk is
  the one piece of func_80052B7C that is genuinely hand-written with no PsyQ-macro
  equivalent — consistent with the existing `inline_asm_canonical.txt:340` authorization
  text ("verbatim PsyQ hand-written asm, unreachable from C").
- Chunk 3 **matches exactly** `gte_ldv0(r1)` (`inline_o.h:16-20`): `lwc2 $0,($12)` /
  `lwc2 $1,4($12)`.
- Chunk 4's `mvmva` word (`.word 0x4A480012` after byte-swap of the dump's `1200484A`)
  **matches exactly** `gte_rt()` (`inline_o.h:421-425`, `.word 0x4A480012`, preceded by the
  same two `nop`s for GTE latency).
- Chunk 5 **matches exactly** `gte_stlvl(r1)` (`inline_o.h` — `swc2 $9,($12)` /
  `swc2 $10,4($12)` / `swc2 $11,8($12)`, the IR1/IR2/IR3 leaf store; distinct from
  `gte_stlvnl` which stores CR25-27 instead).
- No single `gtemac.h` composite macro covers the whole function: `gte_RotTrans(r1,r2,r3)`
  (closest composite: `ldv0`+`rt`+`stlvnl`+`stflg`) stores via `stlvnl`+`stflg`, not
  `stlvl` alone, and has no TR-load step at all — it is not an exact match.

## s1 — in-file precedent (src/main/3AB48.c, same TU)

Lines 102-106 and 111-113 carry pre-written descriptive comments for two sibling
not-yet-ported GTE leaves in this same file:
- `gte_MulMatrix0ClearTrans` (102-107): rotation-matrix-only load, zero translation, 3
  MVMVA cycles, 9 sh stores — a bigger relative, not a byte match for us.
- `gte_SetRotMatrixClearTrans` (111-114): "Loads a packed 3x3 rotation matrix from *a0 into
  CR0-CR4 and **zeroes** the translation CR5-CR7" — this is func_80052B7C's near-twin
  MINUS the real TR data: where that one `ctc2`s zero into CR5-7, func_80052B7C loads real
  halfwords from a1 into CR5-7 instead. Both leaves still sit as `INCLUDE_ASM` (unconverted),
  so no landed C template exists yet for either in this file.

## s1 — current policy for landing this (2026-10-15 ruling, `.claude/rules/inline-asm-policy.md`
§ "PsyQ GTE macro calls")

GTE code in a body must be **calls** to the vendored macros, not written-out `__asm__` copies
of their text, wherever a macro exists for the operation; item 3 of that section forbids
"written-out bodies" and "standalone call of a macro without a cop2 instruction." It does not
explicitly rule on a chunk with **no** vendored-macro equivalent (our chunk 2) — this is a
genuine gap between the policy text and this function's shape, flagged as the frontier's open
question rather than resolved here.

- [s1] canonical func_80052B7C -> ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt' (row at inline_asm_canonical.txt:340, owner-authorized 2026-08-06, docs/grind/decisions.md commit f010b4b6).

- [s1] sandbox func_80052B7C --disable all -> score 24, no_c_body: true; src/main/3AB48.c:115 is still INCLUDE_ASM("asm/funcs", func_80052B7C); -- no C has ever landed for this function.

- [s1] No DATA MODEL / flagged-global signals for this function (pure register-in/out GTE leaf, no globals touched) -- recon's OBJECT MODEL requirement does not apply.

- [s1] asm/funcs/func_80052B7C.s decodes as 5 chunks: (1) 5 lw + 5 ctc2 CR0-CR4 from a0 (MATRIX rotation, offsets 0/4/8/C/10); (2) 3 lh + 3 ctc2 CR5-CR7 from a1 (TR regs loaded from sign-extended halfwords, NOT the usual 32-bit words); (3) 2 lwc2 DR0/DR1 from a2; (4) 2 nop + mvmva 1,0,0,0,0 (word 0x4A480012); (5) 3 swc2 DR9/DR10/DR11 to a3, last one in the jr $ra delay slot.

- [s1] Chunk 1 is byte-identical to include/psxsdk/inline_o.h's gte_SetRotMatrix(r1) (lines 272-283): same 5 lw + 5 ctc2 sequence into CR0-4, same offsets, which match MATRIX.m[3][3]+pad (libgte.h:29-33, 20 bytes).

- [s1] Chunk 3 is byte-identical to gte_ldv0(r1) (inline_o.h:16-20): lwc2 $0,($12) / lwc2 $1,4($12).

- [s1] Chunk 4's mvmva word 0x4A480012 is byte-identical to gte_rt()'s .word 0x4A480012 (inline_o.h:421-425), including the preceding two latency nops.

- [s1] Chunk 5 is byte-identical to gte_stlvl(r1) (inline_o.h, swc2 $9/$10/$11 store of DR9-11, the IR1/IR2/IR3 leaf store, distinct from gte_stlvnl which stores CR25-27).

- [s1] Chunk 2 has NO vendored-macro match: every ctc2-into-CR5/CR6/CR7 site in inline_o.h was enumerated (gte_SetTransMatrix l.325-333, gte_ldtr l.334-341, gte_SetTransVector l.342-350, gte_mvlvtr l.1087-1094, gte_FlipTRX l.1189-1194) and every one of them sources CR5-7 from lw (32-bit) or register-to-register ops, never from lh (sign-extended halfwords). No single gtemac.h composite (closest: gte_RotTrans) covers the whole function either, since gte_RotTrans stores via stlvnl+stflg, not stlvl alone, and has no TR-load step.

- [s1] Same TU (src/main/3AB48.c) carries pre-written descriptive comments for two sibling not-yet-ported GTE leaves: gte_SetRotMatrixClearTrans (lines 111-114) is func_80052B7C's near-twin MINUS real TR data (it ctc2's zero into CR5-7 instead of loading a1's halfwords); both leaves are still INCLUDE_ASM, so no landed C template exists yet in-file for either.

- [s1] Current policy (.claude/rules/inline-asm-policy.md section 'PsyQ GTE macro calls', owner rulings 2026-10-15) mandates GTE code be macro CALLS, not written-out __asm__ copies, wherever a macro exists for the operation -- but does not explicitly address a chunk with no vendored-macro equivalent at all (our chunk 2); this is an open policy question, not resolved this session.
