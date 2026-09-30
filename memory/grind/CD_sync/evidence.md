# CD_sync — evidence

Identity: PsyQ libcd `bios.c` v1.86 `CD_sync(int mode, u_char *result)` (memory/closer/libcd-identity.md).
C reference: SOTN `src/main/psxsdk/libcd/bios.c:232` @db41b28 (matched PS1 code, splat.us.main.yaml:169).

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit FAIL (tmp/audit-2026-09-29/SUMMARY.md, "Shared-declaration follow-up"): the body reached the
Intr bytes through `extern volatile u8 g_cd_status_a` while the same TU defines Sony's
`static volatile CD_intr Intr`, and `idx_1495 = 1 + idx_1494` was the cross-symbol idiom refused for this
function on 2026-07-20 (decisions.md:950).

Change: the TU declares Intr once (`static volatile CD_intr Intr = {0};`, SOTN bios.c:80 @db41b28) and no
other C handle to those bytes. CD_sync reaches them through `volatile CD_intr *intr = &Intr` (the
`Type* t = &g_Thing;` shape of pointer-alias-fake-exception) and `idx_1495 = &Intr.ready` (a member
address, no arithmetic). Every other statement is unchanged.

### How it was scored

`python3 -m engine.cli sandbox CD_sync --disable all --candidate <file>` (scores a copy). Before the
rebuild the reference object is main's system.o, whose Intr accesses carry `g_cd_status_*` relocations,
while a candidate's carry `.data`-section relocations to Intr. The scorer resolves named relocations but
masks section ones (engine/score.py:78-96), so a byte-identical candidate still scores 2 here: the one
`lui/addiu` pair that forms &Intr. The `--diff` view (operands unmasked) is the check: "no differing
instructions" means every instruction and immediate is identical.

| candidate | score | --diff |
|---|---|---|
| main body, `idx_1494 = &Intr.sync` (1 + idx_1494 kept) | 2 | no differing instructions |
| `idx_1494 = &Intr.sync; idx_1495 = &Intr.ready` | 2 | no differing instructions |
| **landed: `intr = &Intr` + `idx_1495 = &Intr.ready`** | 2 | no differing instructions |
| same, `idx_1495 = &intr->ready` | 2 | no differing instructions |
| plain `Intr.sync`/`Intr.ready` everywhere, no handles (rejected/ff-intr-plain-member-21.c) | 21 | 7 source-level hunks |

Single-site ablations on the u8-handle form (each replaces one handle access with plain member access):
t0 read `Intr.sync` = 5; ready shift `Intr.ready` = 10, `*idx_1495` = 9; ready callback `Intr.ready` = 15;
sync callback `Intr.sync` = 26; tail read `Intr.sync & 0xFF` = 5; tail store `Intr.sync = 2` = 5.
On the landed form, the ready callback via `intr->ready` (dropping idx_1495) = 14. So every handle is
needed, and no site can be plain member access.

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox CD_sync --disable all` = 0 (160/160 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash CD_sync` = 36b70614f987fd0f (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.

## 2026-09-30 — Alarm merge (laneB, landed with CD_cw)
The timeout alarm's per-word externs (D_800F19B8 / Alarm_plus_0x4 / Alarm_plus_0x8) are
replaced by Sony's `Alarm_t Alarm` {time, count, name} (include/system.h), completing
the merge CD_cw's retro-audit FAILed on. This body is respelled member-for-word, and the
`pp = &Alarm_plus_0x8` pointer-alias FAKE is dropped: with the struct, the direct
`Alarm.name` argument is byte-exact (sandbox 0, measured before and after the landing;
the alias's 2026-09 ablation was measured on the per-word model only).
Evidence: memory/grind/CD_cw/evidence.md 2026-09-30 (variants B0/B1).

## 2026-09-30 — SOTN's verbatim body (laneB, landed with CD_ready)
With Sony's Alarm_t merged and the bios.c static-inline helpers above CD_sync, this
function is SOTN's own body (src/main/psxsdk/libcd/bios.c @aa53500) and scores 0; every
FAKE construct it carried (pointer-alias handles, do-while(0) wraps, named staging
intermediates, staged reuse) is gone. Evidence: memory/grind/CD_ready/evidence.md
2026-09-30.
