# CD_init — evidence

Identity: PsyQ libcd `bios.c` v1.86 `CD_init(void)` (memory/closer/libcd-identity.md; named_syms.txt
0x800819C4). C reference: SOTN `src/main/psxsdk/libcd/bios.c:414` @db41b28 (matched PS1 code).

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit CONCERN (tmp/audit-2026-09-29/SUMMARY.md): the inlined CD_flush reset was spelled through
`extern volatile u8 g_cd_status_a/b/c` plus a `volatile u8 *p94` local and a staged read-back.

Change: the reset is SOTN's two statements (bios.c:433-434; CdlNoIntr / CdlComplete spelled as their values 0 / 2):

    Intr.ready = Intr.c = 0;
    Intr.sync = 2;

`sandbox CD_init --disable all --candidate`: 8 before the rebuild; `--diff` shows 3 operand-only hunks,
the same three address-expression differences as CD_flush (Intr+2 / Intr+2 / Intr+1 against main's
`g_cd_status_c` / `g_cd_status_c` / `g_cd_status_b` relocations; see memory/grind/CD_flush/evidence.md).
The target's lines are asm/funcs/func_800819C4.s:70-81.

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox CD_init --disable all` = 0 (123/123 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash CD_init` = 342b2622e1869eab (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.
