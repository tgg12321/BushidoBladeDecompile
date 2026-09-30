# cdrom_IrqHandler — evidence

Identity: PsyQ libcd `bios.c` v1.86 `callback()` (the static CD IRQ callback CD_init / CD_initintr install
with `InterruptCallback(2, ...)`; memory/closer/libcd-identity.md). C reference: SOTN
`src/main/psxsdk/libcd/bios.c:210` @db41b28 (matched PS1 code; there `static inline`, here out of line).

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit CONCERN (tmp/audit-2026-09-29/SUMMARY.md): the callback arguments were read through
`volatile u8 *s1 = &g_cd_status_b; volatile u8 *s3 = s1 - 1;`, a handle on a separate extern object
plus arithmetic, landed 2026-03-26 before the aggregate-merge rules.

Change: both locals are gone. The callbacks take `Intr.ready` and `Intr.sync`, as SOTN's callback() does
(bios.c:223, :226). With Intr defined in the TU, loop.c hoists both member addresses and cse relates
them as offsets of one object, which gives the target's `lui/addiu $s1, &Intr+1` and
`addiu $s3, $s1, -1` (asm/funcs/func_80081E1C.s:73-74, :80) without any handle.

`sandbox cdrom_IrqHandler --disable all --candidate`: 2 before the rebuild; `--diff` shows one
operand-only hunk, `addiu s1,s1,0` (main's `g_cd_status_b` relocation) vs `addiu s1,s1,1` (`.data`,
Intr+1 = 0x800A1495), the same address. Every other hunk is a not-scored branch-target cascade.

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox cdrom_IrqHandler --disable all` = 0 (57/57 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash cdrom_IrqHandler` = 1f88b801b0297d08 (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.
