# getintr — evidence

Identity: PsyQ libcd `bios.c` v1.86 `getintr(void)` (memory/closer/libcd-identity.md).
C reference: SOTN `src/main/psxsdk/libcd/bios.c:115` @db41b28 (matched PS1 code, splat.us.main.yaml:169).

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit FAIL (tmp/audit-2026-09-29/SUMMARY.md, "Shared-declaration follow-up"): getintr already wrote
`Intr.sync` / `Intr.ready` / `Intr.c`, but the TU also declared `extern volatile u8 g_cd_status_a/b/c`
over the same bytes (two handles, prong (c)).

Change: the body is unchanged except the two tags below. The TU now declares Intr once, at the position
Sony's bios.c has it (above getintr; SOTN bios.c:80 @db41b28), and no `g_cd_status_*` declaration remains.

### Volatile locals (Q50 route A, Q53)

`volatile char nReg;` and `volatile Result_t buf;` are SOTN bios.c:116 and :117 @db41b28, the same locals
used the same way (the IRQ-number latch re-read against the MMIO register, and the 8-byte result buffer
filled from the parameter FIFO and copied out by `_memcpy`). The file is a splat.us.main member
(`config/splat.us.main.yaml:169`, `c, psxsdk/libcd/bios`), and bios.c carries no INCLUDE_ASM, NON_MATCHING
or version guard. The citation answers whether the construct is admissible; it does not make the
qualifier non-match-motivated. No asynchronous writer touches these automatics, so the qualifier is a match
lever and Q53 (with the route A text of legitimate-volatile-interrupt-touched.md) requires a `/* FAKE: ... */`
annotation and proof that simpler spellings were tried. Q52 only adds SOTN-marked constructs on top of
that. (Corrected after layer-2 rev-intr, 2026-09-30: the first landing cited Q52 alone and called the
qualifier "Sony's own"; SOTN is itself a decompilation, so that overstated the provenance.) src carries
bare `/* SOTN: ... */` tags on each line plus one FAKE comment naming the mechanism (every access becomes
a $sp-slot memory round-trip instead of a register) and the exhaustion below.

Simpler spellings tried (`sandbox getintr --disable all --candidate`):

| candidate | score |
|---|---|
| main body | 0 |
| nReg non-volatile (rejected/ff-intr-nonvolatile-nReg-67.c) | 67/354 |
| buf non-volatile (rejected/ff-intr-nonvolatile-buf-1.c) | 1/354 |
| both non-volatile (rejected/ff-intr-nonvolatile-locals-68.c) | 68/354 |

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox getintr --disable all` = 0 (354/354 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash getintr` = 6869eaab8b9fe4ea (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.

Round 2 (rev-intr, 2026-09-30): the tags are now bare `/* SOTN: src/main/psxsdk/libcd/bios.c:116|117 @db41b28 */`,
with one `/* FAKE: ... */` above the two declarations. Rebuild SHA1 == oracle, sandbox 0 (354/354).
`layer2 hash getintr` = 6869eaab8b9fe4ea, unchanged: the body hash does not cover comments.
