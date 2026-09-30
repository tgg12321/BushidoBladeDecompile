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
