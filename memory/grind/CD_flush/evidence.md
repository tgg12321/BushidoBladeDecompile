# CD_flush — evidence

Identity: PsyQ libcd `bios.c` v1.86 `CD_flush(void)` (memory/closer/libcd-identity.md; named_syms.txt
0x800817A0). C reference: SOTN `src/main/psxsdk/libcd/bios.c:353` @db41b28 (matched PS1 code).

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit CONCERN (tmp/audit-2026-09-29/SUMMARY.md): the Intr reset was spelled through
`extern volatile u8 g_cd_status_a/b/c` plus a `volatile u8 *p94 = &g_cd_status_a` local and a staged
read-back (`c = 0; v0 = c; b = v0; *p94 = 2`).

Change: the reset is SOTN's two statements (bios.c:361-362; CdlNoIntr / CdlComplete spelled as their values 0 / 2):

    Intr.ready = Intr.c = 0;
    Intr.sync = 2;

No handle, no staging. With Intr defined in the TU and volatile, GCC 2.7.2 emits the target's store of
c, volatile read-back of c, store of ready, and `&Intr` materialised in $v1 for the sync store
(asm/funcs/func_800817A0.s, the pre-C split of this function, :37-48).

`sandbox CD_flush --disable all --candidate`: 8 before the rebuild; `--diff` shows 3 operand-only hunks,
each the same address expressed two ways (`sb zero,0(at)` against main's `g_cd_status_c` relocation vs
`sb zero,2(at)` against `.data`, Intr+2 = 0x800A1496; `lbu v0,0/2(v0)` likewise; `sb v0,0/1(at)` =
Intr+1). The scorer masks section-relative immediates, so these count before the rebuild and vanish once
main's object also addresses Intr (memory/grind/CD_sync/evidence.md, "How it was scored").

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox CD_flush --disable all` = 0 (56/56 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash CD_flush` = 2000e3e407478f2e (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.
