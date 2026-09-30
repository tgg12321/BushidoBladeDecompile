# Evidence bank — cdrom_SetMix

## 2026-09-30 — REVERTED: the match depended on the per-function maspsx COMMON gate (a cheat)
Owner ruling 2026-09-30 (docs/grind/decisions.md): `maspsx_comm_syms.txt` is a cheat; this function's
row (`cdrom_SetMix: g_cd_atv`) was its dependency. The function is INCLUDE_ASM again and back in the queue. The landed
body is banked as `cheated-comm-gate-body.c` (a lead, not landable). Its honest floor with the list
emptied: 6/18 (`sandbox --disable all`, 2026-09-30; `migration_pin.json`). Frontier: reach the target's
`sym+N` addressing without any assembler gate — the declaration and access spelling are the only lever.

## 2026-09-30 — laneB: the only closing form is a faithful tentative definition (needs a substrate ruling)

Residual on the banked body (6/18, one source-level hunk): val1..val3 stores come out
`sb aN,k(gp)`; the target has `lui at; sb aN,%lo(g_cd_atv+k)(at)` while val0 stays gp. That is
ASPSX 2.34's COMMON rule (a tentative definition gets gp only at its base;
memory/grind/func_80036140/research-common-gp.md, measured on the real assembler). No C
spelling of an `extern` reaches it: the storage class is the only lever, and per that table
`extern` (via sdata_syms) = gp at every offset, `static` (.lcomm) = gp at every offset, initialized
= gp at every offset; only `.comm` gives base-gp + offset-absolute. Pointer/cast respellings
fold back to `sym+k` (cse) or change the instruction shape (`sb k(reg)`).

Scratch proof (nothing in-tree touched; probes-0930/):
- C: `CdlATV g_cd_atv;` / `CdlATV D_800A36B8;` tentative definitions in code6cac_b4.c + the
  banked bodies (b4_tentative.diff).
- Our cc1 writes `.comm g_cd_atv,4,1` (3 fields); maspsx's parser expects 2 and crashes
  ("too many values to unpack", p_tentative.c). 1-line parser fix: maspsx_comm3_parse.diff.
- maspsx's upstream `--use-comm-section` keeps the storage COMMON (else maspsx invents a local
  .sbss label); GNU ld then resolves it to the linker-script address (`g_cd_atv = 0x800A3718`
  in undefined_syms_auto.txt) without allocating it: ldtest/ (nm: `800a3718 A foo`, reloc
  `lui 0x800a / sb 0x3719`).
- Result (build_scratch.sh, scratch maspsx + --use-comm-section): cdrom_SetMix 0/18,
  func_80035F78 0/12, no gate list involved (maspsx_comm_syms.txt empty); the gp decision is
  upstream maspsx's own `.comm`-driven `comm_symbols` rule.
- Byte-neutrality of parser fix + global --use-comm-section on today's tree (bothways.sh, every
  src/*.c with its exact Makefile recipe, stock vs scratch): 53/53 objects identical, 0 failed
  (no current file emits `.comm`).
Status: this is the "faithful, GLOBAL, declaration-driven model of ASPSX's COMMON rule" that
decisions.md 2026-09-30 (comm-gate ruling, point 3) says needs its own owner ruling. Filed as
STATUS: QUESTION to the orchestrator. Not landed.

## 2026-09-30 (operator session f1) -- landing under owner Q62 (global COMMON model, 60643418a)

The owner answered the question above: option A (thirtieth batch Q62). The substrate lands first as its own
`substrate:` commit: the maspsx three-field `.comm` parse for PUBLIC tentative definitions (a `.local` +
`.comm` uninitialized static fails closed, per the substrate's layer-2 round 1) and `--use-comm-section` for
every file, byte-neutral (tmp/q62/bothways2.sh: 53/53 identical, 0 failed; full rebuild == oracle).

Landing body: probes-0930/b4_tentative.diff applied verbatim to src/code6cac_b4.c (`CdlATV g_cd_atv;` and
`CdlATV D_800A36B8;` as file-scope tentative definitions, cdrom_SetMix and func_80035F78 in plain C), with the
file's header comment updated.

Q62 point 4 (no starting value), checked:
- original bytes over each object's whole extent (CdlATV = 4 bytes), disc/SLUS_006.63:
  g_cd_atv 0x800A3718 = file offset 0x93F18: 00 00 00 00; D_800A36B8 0x800A36B8 = file offset 0x93EB8:
  00 00 00 00.
- no translation unit gives either an initializer (grep src/ include/: only these two tentative definitions
  and the `extern` declarations in include/code6cac.h:318/339).
- both have symbol-file rows (undefined_syms_auto.txt:923/936, named_syms.txt:1019/1020), so GNU ld resolves
  the COMMON symbol to its address without allocating it; maspsx_comm_syms.txt stays empty.

Mechanical: sandbox --disable all cdrom_SetMix 0/18, func_80035F78 0/12, 0 cheat-asm stripped; full rebuild
(with the substrate and the func_8001C8DC landing also in the tree) build/bb2.exe SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. Layer-2 body hashes: cdrom_SetMix 2a9b3fb257626e2c,
func_80035F78 b854a0ed86e24081.
