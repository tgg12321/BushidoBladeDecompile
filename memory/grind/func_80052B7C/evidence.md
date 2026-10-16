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

## s2 (structural) — built the draft, measured it, found s1's "exact match" claim was wrong for chunk 1

- [s2] Applied s1's draft to src/main/3AB48.c (added `#include <psxsdk/inline_o.h>`, replaced the INCLUDE_ASM line with `gte_SetRotMatrix(rot); <chunk-2 written island>; gte_ldv0(v0); gte_rt(); gte_stlvl(out);`) and ran `sandbox func_80052B7C --disable all --diff`: score 22, build_insns 27 vs target 24. All 5 hunks source-level. The extra 3 insns are a `move $t4,aN` staging register before each of chunks 1/3/4, which inline_o.h's macros insert explicitly (`move $12,%0` then use `$12`/`$13`/`$14`/`$15`) that the target's asm does not have (it addresses straight off a0/a2/a3).

- [s2] Switched the `#include` to `<psxsdk/inline_c.h>` instead (same macro names, different PsyQ header: inline_c.h's `gte_ldv0`/`gte_rt`/`gte_stlvl`/`gte_SetRotMatrix` bind the pointer argument directly as `%0` with no staging move) and re-measured: score 16, build_insns 24 == target_insns 24. Still 5 hunks, still all source-level, but now purely about internal chunk ordering, not extra instructions. Chunks 3 (`gte_ldv0`) and 4 (`gte_rt`'s mvmva word) matched the target exactly (no hunk touches them). This is the floor-dropping measurement for this session (24 -> 16); NOT candidate-ready (distance != 0). Saved output is reproducible by re-running the same edit; not separately filed to tmp/ this session since the sandbox JSON above is the full record and re-running costs one engine call.

- [s2] **s1's evidence.md claim that chunk 1 "matches exactly" gte_SetRotMatrix is WRONG at the instruction-order level** (it was a manual text comparison of the macro body against the decoded target, never actually built). BOTH vendored headers' `gte_SetRotMatrix` interleave 2-lw+2-ctc2 then 3-lw+3-ctc2; the target's actual order is batched (all 5 `lw`, then all 5 `ctc2` -- asm/funcs/func_80052B7C.s lines 1-5 vs 9-13). No finer-grained vendored macro exists in inline_c.h to compose that batched order (grepped for a standalone CR0-4 ctc2-only macro: none found). So chunk 1, like chunk 2, has no vendored-macro-CALL spelling that reproduces the target's exact instruction order.

- [s2] **New finding on chunk 5** (`gte_stlvl`): target's last `swc2 $11,8(a3)` sits in the `jr $ra` delay slot. Our build (either header) emits the `swc2` before `jr $ra` with an explicit `nop` in the delay slot instead. `gte_stlvl`'s asm template bundles all 3 `swc2`s into one volatile asm statement, which GCC's delay-slot filler (reorg.c) treats as one opaque multi-instruction unit -- it does not reach inside a compound inline-asm block to steal its last instruction for a jr's delay slot. No vendored macro stores only the last DR word alone. Splitting gte_stlvl into 3 separate single-instruction asm statements would very likely fix this but is a written-out copy of an existing macro's body (disfavored by inline-asm-policy.md's "Calls, not copies" item 3 when a macro already exists for the op) -- this sharpens, not replaces, the existing frontier item 3 ruling-request.

- [s2] `tools/grinder/dump.ps1 func_80052B7C` produced no `.rtl`/`.sched` output this session (`tmp/grind/func_80052B7C/dumps/main/3AB48.rtl: No such file or directory`) -- the mechanism for finding 2 above (reorg.c not reaching into a compound asm block) is a reasoned inference from the known GCC delay-slot-filling model and the measured instruction sequence, NOT yet dump-confirmed. Flagged for the next session rather than guessed past that caveat.

- [s2] sandbox func_80052B7C --disable all --diff with the s1 draft + #include <psxsdk/inline_o.h>: score 22, build_insns 27 (3 extra insns per chunk from inline_o.h's explicit 'move $12,%0' staging register, absent from the target).

- [s2] Same draft with #include <psxsdk/inline_c.h> instead: score 16, build_insns 24 == target_insns 24; all 5 remaining diff hunks classed source-level (no reg-alloc/scheduling lever applies). Output saved to tmp/grind/func_80052B7C/s2/sandbox_inline_c_score16.txt.

- [s2] Chunks 3 (gte_ldv0) and 4 (gte_rt) match target byte-for-byte with inline_c.h; chunks 1 and 5 do not, for two distinct reasons (order vs. delay-slot), both now evidenced in memory/grind/func_80052B7C/evidence.md s2.

- [s2] tools/grinder/dump.ps1 func_80052B7C produced no .rtl/.sched dump this session ('No such file or directory') -- the chunk-5 delay-slot mechanism is reasoned, not dump-confirmed; flagged rather than guessed past.

- [s2] src/main/3AB48.c reverted to INCLUDE_ASM("asm/funcs", func_80052B7C); before finishing -- git status --short on src/ is clean; only the three ledger files under memory/grind/func_80052B7C/ changed.

## s3 (structural) — chassis re-verify, gtemac.h re-check, .dbr dump confirmation

- [s3] Dispatch's "chassis discontinuity" (measured 24 vs ledger floor 16) is NOT a real
  regression: src/main/3AB48.c is INCLUDE_ASM between sessions per asm-until-matched rule 1,
  so the top-of-session measurement is always the asm-only floor (24). Re-applying
  candidate.c verbatim (gte_SetRotMatrix via inline_c.h + written chunk-2 island + gte_ldv0 +
  gte_rt + gte_stlvl) and re-running `sandbox func_80052B7C --disable all --diff` reproduces
  score 16, build_insns 24 == target_insns 24, same 5 source-level hunks s2 recorded
  (tmp/grind/func_80052B7C/s3/sandbox_recheck.txt). Floor 16 stands; nothing to re-derive.

- [s3] Frontier item 3 (re-check gtemac.h for a finer CR0-4-only or DR9-11-only primitive)
  resolved: `grep -n "define gte_" include/psxsdk/gtemac.h` lists only composite multi-step
  macros (`gte_RotTransPers`, `gte_RotTrans`, `gte_LocalLight`, `gte_MulMatrix0`,
  `gte_ApplyMatrix`, etc. -- 48 entries, none touching only CR0-4 or only DR9-11 in isolation);
  `grep -n "define gte_" include/psxsdk/inline_c.h` lists every leaf primitive (incl.
  `gte_SetRotMatrix`, `gte_stlvl`, `gte_SetTransMatrix`, `gte_ldtr`, `gte_SetTransVector`,
  `gte_mvlvtr` -- the same set s1/s2 already enumerated for chunk 2). No macro in either
  vendored header stores/loads a strict subset of CR0-4 or DR9-11 alone. KILLED (instance):
  no finer-grained vendored primitive exists for chunk 1's batched order or chunk 5's
  single-word store, in EITHER header, confirmed by a full `define gte_` enumeration of both
  files this session (not just the CR5-7-touching subset s1 checked).

- [s3] Got `tools/grinder/dump.ps1 func_80052B7C` to produce real dumps this session: the
  prior "No such file or directory" failure (s2) was `dump.ps1` not pre-creating the
  `dumps/<stem-dir>/` subdirectory (only `dumps/` itself) before invoking cc1 with
  `-dumpbase dumps/main/3AB48` -- creating `tmp/grind/func_80052B7C/dumps/main/` by hand
  before the call let it succeed. Full dump set now at
  `tmp/grind/func_80052B7C/dumps/main/3AB48.{rtl,combine,cse,cse2,dbr,flow,greg,jump,jump2,
  loop,lreg,s,sched,sched2}`.

- [s3] **Finding 2's mechanism (chunk 5 delay-slot) is now DUMP-CONFIRMED, not just reasoned**
  (`tmp/grind/func_80052B7C/dumps/main/3AB48.dbr` lines 112-148, read with the candidate.c
  body spliced in via dump.ps1's default TU dump -- the candidate body was live in src/ this
  session so no `-Candidate` flag was needed). The RTL shows: insn 19 = `gte_ldv0` (asm_operands,
  2 lwc2), insn 21 = `gte_rt` (2 nop + mvmva word), **insn 23 = `gte_stlvl`'s entire 3-swc2 body
  as ONE `asm_operands` RTL node** (`"swc2 $9,0(%0);swc2 $10,4(%0);swc2 $11,8(%0)"`, a single
  insn, not three), then **insn 32 = the `jump_insn` `(return)`** immediately after a chain of
  `NOTE_INSN_DELETED` notes (30, 28, 29) between insn 23 and 32 -- no ordinary single-MIPS-op
  insn sits between the asm block and the return for reorg.c's `fill_simple_delay_slots` to
  select as the delay-slot filler; the only candidate is insn 23 itself, and reorg.c's delay-slot
  filler does not split or partially duplicate a multi-instruction `asm_operands` node into a
  delay slot (it treats inline asm as an opaque, non-relocatable unit by construction -- GCC's
  `asm_noperands`/`volatile_insn_p` path marks `asm_operands` ineligible for the dbr scheduler's
  insn-splitting). The aggregate pass-2 stats for this file region show the function's one
  delay-slot-needing jump got 0 delays filled, consistent with observed output (`nop` in the
  slot, `swc2 $11,8(a3)` placed before `jr $ra` instead of in it). This upgrades finding 2 from
  "reasoned inference" to "confirmed by the actual .dbr RTL dump" -- the mechanism is GCC's
  reorg.c / `dbr_schedule` treating `asm_operands` as opaque to delay-slot filling, not merely
  "a multi-instruction block it doesn't reach inside of" (same conclusion, now with the exact
  RTL insns and pass-stage cited).

- [s3] docs/grind/decisions.md 2026-10-15 20:38 func_80052B7C ruling FAILs the written-island landing route for all three affected chunks (1, 2, 5), citing inline-asm-policy.md:55-57 and :70-72; this session verified the citation matches the live rule text.

- [s3] scan_hand_coded.py --single func_80052B7C re-run fresh this session: still tier=LOW score=0/12, no gate-class signal (tmp/grind/func_80052B7C/s3/scan.txt) -- the whole-body grant-path precedent (func_8004A76C, 2026-10-15 decisions.md entry) does not currently apply here.

- [s3] Chunks 3 (gte_ldv0) and 4 (gte_rt) remain the only two chunks with a fully confirmed byte-identical macro-call landing (s2); chunks 1, 2 and 5 have no legal pure-C spelling under current rules.

- [s3] The s2/s3 written-island candidate is moved to memory/grind/func_80052B7C/rejected/written-island-chunks-1-2-5.c; do not resubmit it or a respelling of the same construct -- the ruling is final per review-discipline-before-commit's body-keyed verdict rule.

- [s4] import.py's logged 'Assembler:' line is 'mips-linux-gnu-as -march=vr4300 -mabi=32 "$INPUT" -o "$OUTPUT"', distinct from the project's own maspsx/aspsx-2.34 pipeline logged on the 'Compiler:' line of the same run.

- [s4] nonmatchings/func_80052B7C/ after the --keep run contains base.c, compile.sh, settings.toml, target.s but no target.o; IMPORT_EXIT=1.

- [s4] The ledger's s3 class kill (H4, predicate_cite .claude/rules/inline-asm-policy.md:55) already forecloses the written-island construct that would otherwise be the only way to express chunk 1/5's internal reordering in C -- so even a working permuter workspace (e.g. a hand-built one routing target.s through maspsx like tools/mar_perm_workspace.sh does for marionation_Exec) would have no legal closing form to search for beyond what s2/s3 already measured.

- [s4] The macro-call chassis itself (4 pointer params used once each, 4 sequential asm-statement calls, no loops/scalars/branches) has no free C-level variable for PERM_VAR/PERM_LINESWAP/PERM_GENERAL to act on without crossing into the already-killed written-island or a semantically invalid statement reorder (gte_rt depends on both prior loads).

- [s4] src/main/3AB48.c was left exactly as found (INCLUDE_ASM("asm/funcs", func_80052B7C); at line 115) at the end of this session; git status is clean.

## s5 (enumerate) -- enumerator precondition check

- [s5] `sandbox func_80052B7C --disable all --diff` on unmodified src/ (session dispatch
  re-check): score 24, `no_c_body: true` -- confirms src/main/3AB48.c is still
  `INCLUDE_ASM("asm/funcs", func_80052B7C);`, unchanged since s1.

- [s5] `sandbox func_80052B7C --disable all --diff --candidate memory/grind/func_80052B7C/candidate.c`:
  `error: score unavailable: 'func_80052B7C not found in tmp/sandbox/func_80052B7C/main/3AB48.o'`.
  Read `memory/grind/func_80052B7C/candidate.c` in full: it is a pure C comment block (72 lines,
  no function definition at all) -- this is BY DESIGN, per its own header text ("No src/ edit is
  banked here... src/main/3AB48.c keeps INCLUDE_ASM"), not a measurement bug. Confirms s3's
  conclusion holds: no legal C body for this function exists to even attempt to measure.

- [s5] Read `tools/spelling_enum.py` lines 1-40 (full module docstring): the tool's documented
  input is a complete function body containing a marked `/* ENUM-BEGIN */ ... /* ENUM-END */`
  region of `TYPE name = expr;` locals, `name = expr;` assignments, and/or commutative
  `A*B`/`A+B` sub-expressions. func_80052B7C's only possible legal chassis (4 bare macro-call
  statements, chunks 3/4 only -- chunks 1/2/5 have zero legal C spelling per s3/H4) contains
  none of these: no scalar locals, no assignments, no arithmetic operators anywhere. The tool
  has no applicable input for this function.

- [s5] sandbox func_80052B7C --disable all --diff on unmodified src/ this session reproduces score 24, no_c_body: true -- chassis unchanged since s1, no C ever landed.

- [s5] sandbox func_80052B7C --disable all --diff --candidate memory/grind/func_80052B7C/candidate.c errors 'func_80052B7C not found in tmp/sandbox/func_80052B7C/main/3AB48.o' because candidate.c is pure comment (72 lines, no function body) -- confirmed by direct read, matches the file's own stated reason (no legal C landing form exists per s3).

- [s5] tools/spelling_enum.py's full module docstring (lines 1-40) documents its input precondition as a function body containing named-local declarations, assignments, or commutative arithmetic sub-expressions marked between ENUM-BEGIN/END; func_80052B7C's only legal chassis (gte_ldv0(v0); gte_rt(); as bare macro-call statements) has none of these.

- [s5] This extends s4's permuter-modality finding (no scorable target.o buildable) to the enumerate modality via a different mechanism: s4 found a tooling/assembler incompatibility, this session finds a tooling/object-model incompatibility (the enumerator needs mutable C statement content, this chassis has none).

- [s5] All prior structural kills (s2/s3 H2-H5) and the ruling (H4, docs/grind/decisions.md 2026-10-15 20:38) are unchanged and re-confirmed consistent this session; no new src/ edit was made, none was possible.

## s6 (synthesis) -- first measured LEGAL body; s1's chunk-2 dismissal was wrong

- [s6] s1's chunk-2 enumeration dismissed gte_ldtr (inline_c.h:371-376) because "every ctc2-into-CR5-7
  site sources from lw or register-to-register ops". That reasoning confused the macro with its
  operands: gte_ldtr takes three REGISTER operands ("ctc2 %0,$5; ctc2 %1,$6; ctc2 %2,$7"), so the
  halfword loads are ordinary C (`tr->vx` etc.) that GCC emits itself. It was never measured until s6.
- [s6] Chassis `gte_SetRotMatrix(rot); gte_ldtr(tr->vx, tr->vy, tr->vz); gte_ldv0(v0); gte_rt();
  gte_stlvl(out);` (inline_c.h, SVECTOR *tr, SVECTOR *v0): sandbox --disable all = **19**, 24/24 insns,
  4 hunks all source-level (tmp/grind/func_80052B7C/s6/sandbox_ldtr.txt). Chunk 2 emits
  `lhu v0/v1/t0` + `ctc2 $5-$7`.
- [s6] Same with `(s32)` casts on the three operands: score 19, chunk 2 now emits `lh a0/v1/v0` (sign
  extension, matching target opcode) (tmp/grind/func_80052B7C/s6/sandbox_ldtr_s32.txt). The cast is
  semantic, not F2 padding: TRX/TRY/TRZ are 32-bit signed, lhu zero-extends negative translations.
  Registers differ (a0/v1/v0 vs target t5/t6/t7) but are inside the same hunk-1 source-level reorder.
- [s6] This is the first body ever measured that satisfies the Judge constraint's first branch (pure
  vendored-macro calls, no __asm__ in the body). Legal floor = 19; the old 16 was the rejected island.
- [s6] Kill re-audit: s2 instance kills (SetRotMatrix order, stlvl delay slot) re-measured on the s6
  chassis -- hunks 1 (interleaved lw/ctc2) and 4 (swc2 before jr, nop in slot) persist unchanged;
  fake_ablate on candidate.c: "No FAKE-annotated constructs found" (tmp/grind/func_80052B7C/s6/fake_ablate.txt),
  so no kill was measured under a FAKE carrier. Remaining residual = chunks 1 and 5 only.
- [s6] src/main/3AB48.c reverted to INCLUDE_ASM after measurement (asm-until-matched); body banked in candidate.c.

- [s6] s1's chunk-2 'no vendored macro' claim was wrong: gte_ldtr takes register operands; measured legal body scores 19 (tmp/grind/func_80052B7C/s6/sandbox_ldtr_s32.txt)

- [s6] Uncast tr->vx operands produce lhu (zero-extend), cast (s32) produces target's lh; both score 19

- [s6] All 4 remaining hunks are source-level and sit inside gte_SetRotMatrix (order) and gte_stlvl (delay slot) asm bodies

- [s6] Frontier item 'new DMPSX word for chunk 2' is void: chunk 2 is reachable today

- [s6] src/main/3AB48.c reverted to INCLUDE_ASM; best legal body banked in memory/grind/func_80052B7C/candidate.c

## s7 (synthesis) -- calibration + delay-slot predicate; floor 19 unchanged

- [s7] cc1psx calibration: the s6 legal chassis (tmp/grind/func_80052B7C/s7/t.c) compiled with the build cc1
  (tools/gcc-2.7.2/build/cc1) and with the original tools/cc1psx.exe (GNU C 2.7.2.SN.1, via cc1psx_wrapper.sh)
  gives a byte-identical function body (tmp/grind/func_80052B7C/s7/{ours.s,psx.s,cc1_vs_cc1psx.txt}; only
  file-header directives differ). The residual is NOT a cc1 port divergence: SN's own compiler emits the
  same interleaved SetRotMatrix order and the same `swc2 $11; j $31` (unfilled slot) from this C.
- [s7] Both compilers emit `j $31` with NO delay-slot insn; the build's assembler stage then pads a nop. The
  target's swc2 $11 in the jr slot would require reorg.c to pick the asm insn as a slot candidate, and
  reorg.c:740-744 (stop_search_p, case INSN) returns 1 for ANY ASM_INPUT or asm_operands insn
  (tmp/grind/func_80052B7C/s7/reorg_stop_search_p.txt). config/mips/mips.md has 0 cop2/swc2 patterns, so a
  swc2 to GTE data reg $11 can only come from an asm insn -> GCC 2.7.2 (either build) never puts it in a
  delay slot. Chunk 5 (hunks 3+4 of the diff, 2 of the 19 points' worth of hunks) is closed for any C body.
- [s7] scan_hand_coded --single on the whole 3AB48 GTE cluster (func_80052B7C, gte_SetMatrixRotTransIR,
  gte_SetMatrixRotTransIRVec, gte_SetRotTransMatrix, gte_SetRotMatrixClearTrans): all LOW 0/12, no gate-class
  signal (tmp/grind/func_80052B7C/s7/cluster_scan.txt). The scanner's 12 signals have no GTE/cop2
  delay-slot signal at all, which is exactly what all five bodies share (cop2 op in the jr slot,
  t0..t7 sequential batched loads). No sibling supplies STRONG-tier cluster evidence today.
- [s7] SOTN construct index (docs/reference/sotn-construct-index.md) has no ctc2 / SetRotMatrix / stlvl
  hack entry (only one gte_stdp duplicated-statement hit at line 2042); no SOTN precedent for a GTE-
  delay-slot construct.
- [s7] --diff on candidate.c re-measured: 4 hunks, all source-level, unchanged from s6 (score 19).

- [s7] cc1 and cc1psx produce byte-identical func_80052B7C bodies from the s6 legal chassis (tmp/grind/func_80052B7C/s7/cc1_vs_cc1psx.txt)

- [s7] reorg.c:740-744 stop_search_p rejects all asm insns as delay-slot fillers; mips.md has no cop2 store pattern

- [s7] All five 3AB48 GTE leaves scan LOW 0/12 (tmp/grind/func_80052B7C/s7/cluster_scan.txt)

- [s7] SOTN construct index has no ctc2/SetRotMatrix/stlvl precedent (only gte_stdp at sotn-construct-index.md:2042)

- [s7] Legal chassis --diff unchanged: 4 source-level hunks, score 19

- [s8] sandbox func_80052B7C --disable all --diff this session: score 19, target_insns=24, build_insns=24, 4 hunks all source-level, 0 operand-only, 0 not-scored (tmp/grind/func_80052B7C/s8/sandbox_diff.txt) — reconfirms s6/s7's floor with a fresh re-application rather than trusting the chassis-check number alone.

- [s8] gte_SetRotMatrix (include/psxsdk/inline_c.h:297-309) and gte_stlvl (include/psxsdk/inline_c.h:1086-1092) read in full this session: both are single __asm__ volatile statements with a literal fixed instruction-text string, confirming s3's finding independently with the exact macro text cited.

- [s8] The 2026-10-15 20:38 owner ruling (docs/grind/decisions.md) already forecloses the written-island landing route for chunks 1/2/5 under inline-asm-policy.md items 2-3 (the wall); this session did not re-litigate that ruling, only re-confirmed the legal (macro-calls-only) chassis's residual is structurally unreachable.

- [s8] src/main/3AB48.c was restored to INCLUDE_ASM("asm/funcs", func_80052B7C); before this session ended; git status shows only memory/grind/func_80052B7C/{candidate.c,hypotheses.md} changed.

- [s9] sandbox func_80052B7C --disable all --diff --candidate memory/grind/func_80052B7C/candidate.c re-measured this session: score 19, target/build both 24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical classification to s6/s7/s8.

- [s9] Sibling func_8004A76C (same TU, src/main/3AB48.c) reviewed per the inheritance rule: its 2026-10-15 CANONICAL-ASM GRANT PATH entry rests on a custom $s0 callee-saved-register ABI cluster (func_8004A4E0/func_8004A808/func_8004A940) unrelated in shape to func_80052B7C's GTE macro-ordering residual -- no spelling to transplant; co-located in the same source file only, not a content sibling.

- [s9] No src/ edit made this session; src/main/3AB48.c is unchanged (still INCLUDE_ASM stub per the standing ruling).

## s10 (solver)
- classify (object mode, WSL): FIRST DIVERGENCE RA; register-blanked multisets match (tmp/grind/func_80052B7C/s10/classify.txt). Note inverse_compose.py must run inside WSL (objdump path); Windows python fails with FileNotFoundError.
- .lreg (legal chassis): TR pseudos 77,80,83 -> hard regs 4,3,2. mips.h: no REG_ALLOC_ORDER -> local-alloc.c:2249 lowest-free scan.
- Scratch 8-"r"-operand probe (tmp/grind/func_80052B7C/s10/ra_probe.s): allocator gives v0,v1,t0..t5 with lh hoisted first; target uses t0..t7 with lw first. Target registers are fixed-string, not allocated.
- Census: no vendored GTE header contains `lw $8..$11` (0 hits) — only $12-$15 are hardcoded in vendored macros.
- Sandbox re-measure: 19 (tmp/grind/func_80052B7C/s10/sandbox_diff.txt). fake_ablate: no FAKE constructs (tmp/grind/func_80052B7C/s10/fake_ablate.txt).
- Interpretation (not a kill): target = t0..t7 data regs + nop after mvmva + swc2 in jr slot is the shape of a hand-written leaf; only the whole-body grant branch of the Judge constraint fits, which needs a scan_hand_coded gate-class signal (engine-side).

- [s10] classify object mode must run under WSL; Windows python fails on objdump path

- [s10] sandbox legal chassis = 19, 4/4 hunks source-level (tmp/grind/func_80052B7C/s10/sandbox_diff.txt)

- [s10] .lreg: TR pseudos -> a0/v1/v0; scratch 8-operand probe -> v0,v1,t0..t5; target t0..t7

- [s10] no vendored GTE header contains lw $8..$11

- [s10] Combined with s7 (reorg.c:740 jr-slot) and s8 (fixed-string macro order), all three backend layers (pre-RA order, RA, delay slot) are now closed on vendored-macro chassis; target shape (t0..t7, nop after mvmva, swc2 in jr slot) fits a hand-written leaf

## s11 (forensics)
- [s11] Legal chassis re-measured 19, 4/4 source-level (tmp/grind/func_80052B7C/s11/sandbox_diff.txt). No FAKE constructs (re-audit: nothing to ablate).
- [s11] cc1 emits the tail as one #APP block + bare `j $31` in reorder mode (s11/3AB48_cc1.s); the slot is filled by maspsx with `nop  # DEBUG: branch/jump` (tools/maspsx/maspsx/__init__.py:1484-1486), never by swapping. s7's reorg.c:740 attribution is correct but incomplete: the assembler stage is the actual deciding layer, and it has no fill path either. Hand re-run of maspsx+as (s11/asm_stage.sh -> maspsx_out.s.o) reproduces our body word-for-word.
- [s11] gte_rt's mvmva is a `.word` (inline_c.h), so maspsx inserts no post-cop2 nop; target's nop after mvmva is not a cc1 output.
- [s11] jr-slot census of the oracle EXE (s11/jr_census.py/.txt): cop2-class insn in the jr $ra slot at exactly 5 sites, all in the 3AB48 GTE cluster (gte_SetMatrixRotTransIR, gte_SetMatrixRotTransIRVec, gte_SetRotTransMatrix, gte_SetRotMatrixClearTrans, func_80052B7C); 0 compiled-C sites; Sony libgte uses jr;nop. The 4 siblings hold grants at inline_asm_canonical.txt:335/:338/:339/:341.
- [s11] label_census / loop_movables / nrefs_census: no labels, no loops, no ALLOCDBG rows — no C-level lever surface.

- [s11] fact: legal chassis re-measured 19, 4/4 hunks source-level, 0 operand-only (tmp/grind/func_80052B7C/s11/sandbox_diff.txt); no FAKE constructs to ablate

- [s11] fact: cc1 output for the chassis ends with an #APP block followed by a bare `j $31` in reorder mode (tmp/grind/func_80052B7C/s11/3AB48_cc1.s)

- [s11] fact: maspsx __init__.py:1484-1486 always appends a nop after reorder-mode jumps; a hand re-run of maspsx+as reproduces our body exactly (s11/maspsx_out.s.o)

- [s11] fact: gte_rt's mvmva is a raw .word in inline_c.h, so no assembler hazard nop follows it; target's nop after mvmva is not a cc1 output

- [s11] fact: EXE census finds a cop2 insn in the jr $ra slot at only 5 sites, all in the 3AB48 GTE cluster, with 0 compiled-C sites (s11/jr_census.txt)

- [s12] m2c (tools/m2c/) has zero cop2/GTE mnemonics (ctc2/lwc2/swc2/mvmva) anywhere in arch_mips.py's instruction table; grep confirms 0 matches, so every GTE instruction in this function always becomes an M2C_ERROR stub regardless of input ordering.

- [s12] decomp.me corpus search (gcc2.7.2-psx bucket) returns a max similarity of 0.026 for this target, with no hit touching cop2/GTE code at all.

- [s12] gtemac.h's 5 composite macros (lines 260-358) are all calls-to-calls over existing leaves (gte_SetRotMatrix, gte_ldv0, gte_rtv0/gte_rtir, gte_stclmv/gte_stsv/gte_stlvnl) for whole-pipeline ops; none batches the rotation-matrix and TR-vector loads together, and none re-expresses a leaf's own fixed asm string.

- [s12] Sibling candidates func_8004A4E0 and func_8004A76C are both still INCLUDE_ASM stubs -- no transplantable C body exists in the cluster.

- [s12] The legal macro-call chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl) re-measures 19, 4/4 hunks source-level, unchanged from s6-s11.

- [s13] sandbox func_80052B7C --disable all --diff on the s6/s8 chassis: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored (tmp/grind/func_80052B7C/s13/sandbox_diff.txt).

- [s13] gte_SetRotMatrix (include/psxsdk/inline_c.h:297) and gte_stlvl (:1086) are each one fixed-string __asm__ volatile macro, re-read this session byte for byte -- unchanged since s8's citation.

- [s13] No new structural lever surfaced: block-local var splits, declaration order, type narrowing and statement re-association all require an operand-only hunk to act on, and none exists on this chassis.

- [s13] src/main/3AB48.c left as INCLUDE_ASM("asm/funcs", func_80052B7C); -- no landing form banked this session.

- [s14] sandbox func_80052B7C --disable all --diff: score 19, target 24 insns, ours 24 insns, 4 hunks, 4 source-level, 0 operand-only, 0 not-scored (tmp/grind/func_80052B7C/s14/sandbox_diff.txt).

- [s14] Hunk 1: target batches 5 lw then 5 ctc2 into t0-t7 (rotation matrix) then CR5-7 via halfword loads; our body (gte_SetRotMatrix + gte_ldtr) interleaves 2-lw+2-ctc2 then 3-lw+3-ctc2 then the halfword loads into a0/v1/v0 -- a fixed instruction-sequence divergence inside the vendored macros, not a register-seat or scheduling difference.

- [s14] Hunk 2-4: target's final swc2 $11,8(a3) sits in the jr $ra delay slot; our build emits it before jr with an explicit nop -- gte_stlvl's swc2 body is a single opaque __asm__ volatile asm_operands node reorg.c's delay-slot filler cannot split or select into (confirmed s3/H3 via .dbr dump, unchanged this session).

- [s14] tools/fake_ablate.py confirms the candidate.c s6 body carries zero /* FAKE */ constructs -- the kill-re-audit's own precondition (a FAKE carrier occupying the target pseudo) does not apply here, so s8/s9/s13's structural-modality class kills were never at risk of the func_8002EA24-s8-style invalidation and stand as measured.

- [s14] docs/grind/decisions.md 2026-10-15 20:38 ruling stands: no written __asm__ island inside the C body is admitted for this function (inline-asm-policy.md items 3/item-1), so the only remaining non-C path is the whole-body inline_asm_canonical.txt:340 grant, gated on scan_hand_coded reaching STRONG tier -- unchanged, out of this session's reach (no engine/tools edits permitted).

- [s15] sandbox func_80052B7C --disable all --diff this session: score 19, target 24 insns, build 24 insns, 4 hunks all classed source-level (0 operand-only, 0 not-scored) -- identical classification to s8/s9/s13/s14, confirming no reg-alloc/scheduling lever exists on this chassis and no regression since last measured.

- [s15] The legal macro-call chassis body is exactly 5 statements (4 gte_SetRotMatrix/gte_stlvl-style macro calls or their expansions plus an if/return pair) with zero local declarations and zero assignments -- there is no C-level sub-expression anywhere in the function for spelling_enum.py's decl/assign/swap axes to enumerate over.

- [s15] The ENUM region in this session's probe file spans the function's ENTIRE body (there is no code outside it to widen into), so the sweep SOP's step-3 widen-to-neighbouring-block fallback is not available -- this is a terminal class kill for the spelling-sweep modality on this chassis, not a partial result awaiting a wider region.

- [s16] sandbox func_80052B7C --disable all --diff this session: score 19, 4/4 hunks source-level, 0/4 operand-only -- identical classification to s8/s12/s13/s14's re-measurements.

- [s16] Hunk 1 (chunk 1): target batches 5 lw then 5 ctc2 (t0..t7); ours interleaves 2+3 lw/ctc2 pairs via gte_SetRotMatrix -- confirms s6/s8's finding that no vendored leaf or gtemac.h composite expresses the batched order.

- [s16] Hunks 2-4 (chunk 5): target's final swc2 $11,8(a3) sits in the jr $ra delay slot; ours emits it before an explicit nop via gte_stlvl -- confirms s7/s11's finding that this is decided at the maspsx/reorg.c stage, not reachable from C.

- [s16] spelling_enum.py (plain and swap axis) both report zero named locals, zero assignments, 5 anchors -> 1 distinct spelling on the full-body region -- third confirmation of s9's and s15's class kill, now also re-run against this exact session's fresh chassis application rather than relying on the prior banked logs.

- [s17] sandbox func_80052B7C --disable all --diff on the legal macro-call chassis (with #include <psxsdk/inline_c.h> present) measures score 19, target 24 insns == build 24 insns, 4/4 hunks source-level, 0/4 operand-only, 0/4 not-scored (tmp/grind/func_80052B7C/s17/sandbox_diff.txt) -- exact match to the ledger's s8/s13/s14/s15/s16 recorded state.

- [s17] Without that #include, the same body measures worse (27 vs 24 insns, jal calls in place of inline cop2 asm) -- a repro trap for any future session that copies only candidate.c's function body and not its include line.

- [s17] spelling_enum.py's region scanner (tools/spelling_enum.py:89/:217) finds 0 named locals, 0 assignments, 5 anchors -> 1 distinct spelling on both the plain and operand-swap axis -- fourth consecutive enumerate pass (s9, s15, s16, s17) landing on the identical empty mutation space.

- [s17] The residual (chunk 1: gte_SetRotMatrix's batched lw/ctc2 order; chunk 5: gte_stlvl's delay-slot swc2 placement) remains proven to sit entirely inside each macro's own fixed __asm__ volatile string (include/psxsdk/inline_c.h:297, :1086), which spelling_enum.py's C-level syntax scanner cannot see or vary -- no new lever surfaced this session.

## s18 (synthesis) evidence
- KILL RE-AUDIT (dispatcher-mandated): s6 legal macro-call chassis (candidate.c body + `#include <psxsdk/inline_c.h>`) re-applied fresh at src/main/3AB48.c:115 and re-measured: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only (tmp/grind/func_80052B7C/s18/sandbox_diff.txt). fake_ablate on candidate.c: "no FAKE-annotated constructs found ... nothing to ablate" (tmp/grind/func_80052B7C/s18/ablate.txt). All instance kills were measured on this same chassis with no FAKE carrier, so none is voided by chassis drift.
- NEW: batched-load idiom census over all 1437 asm/funcs/*.s (which includes every COMPLETED-C function's .s, e.g. puts.s, func_800203B4.s, func_8002EA24.s), script tmp/grind/func_80052B7C/s18/batch_census.py, output .../batch_census.txt. Pattern = a run of >=5 consecutive `lw/lh $tN` immediately followed by >=5 `ctc2` of the same t-registers in the same order (target chunk 1+2 shape). Hits: 16 functions, ALL independently hand-written:
  * 8 verbatim Sony PsyQ 4.0 LIBGTE asmpsx modules: SetRotMatrix (MTX_09, 0x8007EEEC -- its 10-insn body is opcode- AND register-identical to the target's chunk 1: lw $t0..$t4 0..16($a0); ctc2 $t0..$t4,$0..$4), ApplyMatrix, ApplyMatrixLV, CompMatrix, MulMatrix, MulMatrix0, MulMatrix2, SetColorMatrix.
  * 4 game functions with independent hand-written proof: func_8004A940 (scan_hand_coded STRONG S7/S9/S11, inline_asm_canonical.txt:198), func_8004C404 (STRONG S7/S9, :200), func_8004DDB4 (STRONG S9, :210), func_80054440 (trapping addi, :162).
  * the 3AB48 GTE cluster: gte_SetMatrixRotTransIR, gte_SetRotMatrixClearTrans, gte_SetRotTransMatrix (all granted, inline_asm_canonical.txt:335-341) + func_80052B7C.
  * 0 compiled-C sites. Negative control: func_800203B4 (COMPLETED-C, uses gte_SetRotMatrix by name) does not hit -- the vendored inline_c.h/inline_o.h SetRotMatrix interleave (2 lw/2 ctc2/3 lw/3 ctc2, $12-$15), so no C body in this EXE produces the batched t-register form.
  Reading: target chunk 1 is the asmpsx LIBGTE house idiom (Sony's own SetRotMatrix body, inlined), which co-occurs in this EXE only with code proven hand-written by other gate-class signals. This is a candidate gate-class scanner signal (zero false positives on compiled C across the whole EXE), complementary to s11's jr-slot census (5/5 cluster, 0 compiled C). It is evidence for the whole-body grant branch, not a C lever; adding it to tools/scan_hand_coded.py is an engine/tools change outside grind scope.

- [s18] s18 sandbox --disable all --diff on the legal chassis: score 19, 24/24 insns, 4/4 hunks source-level (tmp/grind/func_80052B7C/s18/sandbox_diff.txt).

- [s18] fake_ablate on candidate.c: nothing to ablate (tmp/grind/func_80052B7C/s18/ablate.txt).

- [s18] Sony LIBGTE SetRotMatrix (asm/funcs/SetRotMatrix.s, PsyQ 4.0 MTX_09, asmpsx-built per inline_asm_canonical.txt:328) is opcode- and register-identical to the target's chunk 1 (lw $t0..$t4, 0..16($a0); ctc2 $t0..$t4, $0..$4).

- [s18] Batched-load idiom census: 16/16 hits are hand-written (Sony asmpsx modules, STRONG-tier game funcs, the granted 3AB48 cluster); 0 compiled-C false positives across all 1437 functions (tmp/grind/func_80052B7C/s18/batch_census.txt). Complements s11's jr-slot cop2 census (5/5 cluster, 0 compiled C).

- [s18] src/main/3AB48.c reverted to INCLUDE_ASM; git status clean outside the ledger.

## s19 (solver)
- Legal chassis re-measured 19 (4/4 source-level). fake_ablate: no FAKE constructs. Artifacts: tmp/grind/func_80052B7C/s19/sandbox_diff.txt, ablate.txt.
- classify (object mode) prints RA. 5 of the 8 differing data registers are literal in the gte_SetRotMatrix template (include/psxsdk/inline_c.h:298-307), so the residual is template-level and pre-RA. Only gte_ldtr's three operands are allocated. s19/classify.txt.
- Measured allocation of the ldtr operands: baseline {a0,v1,v0}; locals-before-SetRotMatrix and ldtr-first both give {t0,v1,v0}, which is the lowest free set (local-alloc.c:1512). Both score 19. s19/v1_locals_pre.txt, s19/v2_ldtr_first.txt.
- [s19] src/main/3AB48.c reverted to INCLUDE_ASM.

- [s19] Legal chassis = 19, 4/4 hunks source-level, 0 operand-only (s19/sandbox_diff.txt)

- [s19] classify object mode prints RA, but 5/8 differing data regs are literal in the inline_c.h:298-307 template

- [s19] ldtr operand allocation: baseline {a0,v1,v0}; locals-first and ldtr-first both {t0,v1,v0}, score 19
- [s20] Dump attribution (s20/func.dbr:33-34, func.lreg:26-28): hunk 1's rotation regs are literal in insn 15's asm template (RTL expand, inline_c.h:298); the TR operands are local-alloc lowest-free (a0,v1,v0), with no global allocnos; hunks 2-4 come from insn 35 (single asm_operands, three swc2) directly before an unfilled return_internal. No loops, no labels.
- [s20] inline_o.h variant of the legal body measures 31 (s20/inline_o_variant.txt): every pointer-taking inline_o.h macro starts with `move $12,%0` (inline_o.h:273), and the target has no move instructions.

- [s20] sandbox --disable all on legal chassis = 19 (s20/sandbox_diff.txt)

- [s20] func.dbr:33-34 insn 15 asm template holds literal rotation regs

- [s20] func.lreg:26-28 local-alloc gives ldtr operands a0,v1,v0

- [s20] inline_o.h variant = 31 (s20/inline_o_variant.txt)

## s21 (forensics)
- [s21] Legal chassis re-measured 19, 4/4 source-level; fake_ablate nothing to ablate (s21/v_base.txt, s21/ablate.txt).
- [s21] gte_nop() between gte_rt and gte_stlvl is stripped by the sandbox (cheat_asm_stripped 70 -> 71; s21/v_nop.txt, s21/obj_nop.txt) per engine/gtemacro.py:30/:1095 (standalone gte_nop = scheduling barrier). Score 19.
- [s21] gte_rt_b() removes the two pre-RT nops (s21/obj_rtb.txt) yet scores 19 identically: the score does not see those nops; the variant moves bytes away from target.
- [s21] Vendored headers have value-operand ctc2 only for CR5-7 (gte_ldtr inline_c.h:372), CR13-15, CR21-23, CR26; none for CR0-4 (re-confirms s6).

- [s21] Legal chassis = 19, 4/4 source-level, 0 operand-only (tmp/grind/func_80052B7C/s21/v_base.txt); fake_ablate nothing to ablate (s21/ablate.txt).

- [s21] Standalone gte_nop() is stripped by the sandbox as a scheduling barrier (cheat_asm_stripped 70->71; s21/v_nop.txt, s21/obj_nop.txt; engine/gtemacro.py:1095).

- [s21] gte_rt_b() removes the two pre-RT nops (s21/obj_rtb.txt) and the score stays 19: the scorer does not count those nop slots.

- [s21] Vendored value-operand ctc2 macros exist only for CR5-7 (inline_c.h:372), CR13-15, CR21-23 and CR26, none for CR0-4 (re-confirms s6).

- [s22] Chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl) re-measures 19 fresh this session, 4/4 source-level hunks, 0 operand-only -- 9th consecutive session (s6/s8/s9/s11-s21) confirming the residual is locked inside the vendored GTE macro templates and reorg's asm-insn handling, not a C-level choice.

- [s22] Both named GTE-cluster siblings (func_8004A4E0, func_8004A76C) received fresh OWNER-ESCALATION/CANONICAL-ASM-GRANT-PATH dispositions on 2026-10-15 (docs/grind/decisions.md:1324, :1345) but neither holds a C body; the sibling-transplant inheritance mechanism is vacuous for this cluster.

- [s22] docs/reference/sotn-construct-index.md (1,365 PSX entries) has zero hits for the rotation-matrix/ctc2-batching construct, so the Q55 precedent-citation route to admit a respelling is not available.

- [s22] m2c (s12) is categorically blind to this function's body (14/14 instructions are cop2/GTE opcodes absent from arch_mips.py's instruction table) and decomp.me's gcc2.7.2-psx corpus (s12) has no structural match (similarity 0.022-0.026, none touching cop2) -- both rederive avenues already exhausted prior to this session and reconfirmed structurally unchanged this session (no corpus/tool update since s12).

- [s23] Chassis re-audit (s23): applying the s6 legal 5-statement macro-call body to src/main/3AB48.c and running sandbox func_80052B7C --disable all --diff again measures score 19, 4 hunks all source-level, 0 operand-only, 0 not-scored -- bit-identical hunk classification to s8/s9/s13-s22 (hunk 1: gte_SetRotMatrix's batched 5-lw/3-lh-then-8-ctc2 target order vs our paired/tripled order plus register seats; hunks 2-4: gte_stlvl's trailing nop/swc2 delay-slot placement).

- [s23] psyz (Xeeynamo/psyz) libgte.c implements SetRotMatrix, ReadRotMatrix, RotMatrix/RotMatrixX/Y/Z/YXZ/ZYX and ApplyRotMatrix as pure C software (global MATRIX M struct, field copies, trig/matrix multiply) with zero cop2 instructions anywhere in the 3280-line file -- it is a cross-platform port, not a byte-matching PS1 hardware decomp of libgte, so it cannot inform the cop2-level residual regardless of which PsyQ SDK version it targets.

- [s23] No external corpus checked across this function's grind history (decomp.me corpus search s12, sotn-decomp construct-index grep s22, psyz libgte.c s23) contains a real hardware cop2 precedent for the batched-lw-then-ctc2 rotation-matrix order or the cop2-store-in-jr-delay-slot shape this residual needs.

- [s23] The residual is unchanged from s8 onward: both hunks sit entirely inside gte_SetRotMatrix's and gte_stlvl's own fixed __asm__ volatile template strings (include/psxsdk/inline_c.h:297, :1086), which are literal text at RTL expand, not computed from any C-level expression -- confirmed again this session with no new finding against that conclusion.

- [s24] src/main/3AB48.c is unchanged from session start (git diff --stat empty) -- both chassis variants were measured live and reverted to INCLUDE_ASM before this outcome was written.

- [s24] sandbox --diff on the reverted stub confirms score 24/no_c_body (tmp/grind/func_80052B7C/s24/sandbox_reaudit.txt), i.e. the committed tree is back to its pre-session state.

- [s24] Full per-variant diff output banked at tmp/grind/func_80052B7C/s24/chassis_sandbox_diff.txt.

- [s24] The structural-modality lever set (block-local var split, declaration order, type narrowing, statement reassociation) was already class-killed on this chassis at s8/s13/s14 (predicate include/psxsdk/inline_c.h:297) and re-confirmed by statement-level reordering at s19; this session's finer-grained sub-word-load-staging variant is a new instance of the same null result, not a new class kill (only the 'r'-constraint, no-modifier spelling of gte_ldtr was tried).

- [s24] Hunks 2-4 (final swc2 in the jr $ra delay slot) remain covered by the standing class kills at s7 (predicate tools/gcc-2.7.2/reorg.c:740) and s11 (maspsx cannot fill a cop2 store into an unfilled jump's delay slot); this session did not re-touch that axis.

- [s25] candidate.c's ledger-header comment was corrupted in two spots (stray mid-word */, and a bare #include splitting a comment block); this is the root cause of the s25 dispatch's 'chassis check: measurement unavailable' and was likely affecting every prior session's dispatch-time auto-measurement since whenever the corruption was introduced.

- [s25] After the fix, sandbox --candidate on candidate.c measures cleanly at 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored, matching the ledger's recorded floor exactly -- no regression, no improvement, just restored measurability.

- [s25] Two additional structural sub-axes (redundant-cast removal on gte_ldtr's args; s32-typed block-local staging vs s24's s16-typed pre-SetRotMatrix staging) both measure 19, extending s8/s9/s13-s16/s19/s24's finding that this chassis has zero C-level mutation axes in the 5-statement macro-call body.

- [s25] This is now the 5th structural-or-equivalent session (s8, s13, s14, s19, s24, s25) landing on the identical null result; the residual stays locked inside gte_SetRotMatrix's and gte_stlvl's own fixed __asm__ volatile template strings.

- [s26] sandbox func_80052B7C --disable all --candidate memory/grind/func_80052B7C/candidate.c --diff = score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored (tmp/grind/func_80052B7C/s26/sandbox_diff.txt), matching the s6-s25 floor and hunk classification exactly.

- [s26] spelling_enum.py's region scanner reports the identical zero-mutation-axis result on the full function body for the 5th time across independent enumerate-modality sessions (s9, s15, s16, s17, s26), with and without the commutative-operand-swap axis.

- [s26] The function already carries a filed decisions.md ruling (2026-10-15 20:38) rejecting a written-island mix as a workaround for chunks 1/2/5; that ruling is unchanged and still binding.

- [s26] src/main/3AB48.c was never touched this session (git status clean on that file); candidate.c and hypotheses.md were updated to append the s26 re-confirmation.

- [s27] Chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h) re-measures 19/24 insns, 4/4 hunks source-level, 0 operand-only this session -- unchanged since s6.

- [s27] fake_ablate.py confirms zero FAKE-annotated constructs exist on this chassis to ablate, for the 7th consecutive session this has been checked (s14/s18-s22/s27).

- [s27] spelling_enum.py's exhaustive sweep of the 5-statement region (both swap and no-swap axes) reports exactly 1 distinct spelling for the 6th consecutive enumerate-modality session (s9, s15, s16, s17, s26, s27) -- the region contains zero decl/assign nodes for the tool's mutation axes to act on, because every statement is a bare macro call.

- [s27] Prior sessions (s18-s21, s24-s26) already established the residual (hunk 1: gte_SetRotMatrix/gte_ldtr batching order; hunks 2-4: gte_stlvl's delay-slot swc2 placement) is fixed entirely inside each vendored macro's own literal __asm__ volatile template string at RTL expand (include/psxsdk/inline_c.h:297, :1086), outside any C-level spelling space by construction -- this session's sweep is consistent with and re-confirms that finding, not a new discovery.

- [s27] A docs/grind/decisions.md ruling (2026-10-15 20:38) already rejects written-island forms for this function's macro-less chunks; the only live non-grind-session paths are the scan_hand_coded gate-class signal addition and the inline_asm_canonical.txt:340 whole-body grant audit, both named in the ledger's live frontier and both outside grind-session scope.

- [s27] CC1PSX SELF-DISPROOF (driver, ruling 2026-09-08): candidate 2d0d81a51e56 scores 19 under our cc1 and 19 under the original cc1psx — SOURCE-SIDE: the original compiler is no closer from this source, so the residual is a spelling not yet found (a pure-C preimage exists by construction).

- [s28] sandbox --disable all on candidate.c = 19 this session (tmp/grind/func_80052B7C/s28/sandbox_diff.txt), all 4 hunks source-level

- [s28] state.json cc1psx_check: ours 19, psx 19, closer=false -- the original compiler reproduces the residual from this C

- [s28] scan_hand_coded LOW 0/12, no gate-class signal (s28/scan.txt)

- [s28] SOTN construct index census for the batched GTE load construct = 0/1365 (s22); psyz has no cop2 precedent (s23)

- [s28] 28 sessions across structural, enumerate, synthesis, forensics, rederive, solver, recon, permuter, escalation modalities
