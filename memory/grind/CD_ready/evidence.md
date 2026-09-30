# CD_ready — evidence

Identity: PsyQ libcd `bios.c` v1.86 `CD_ready(int mode, u_char *result)` (memory/closer/libcd-identity.md).
C reference: SOTN `src/main/psxsdk/libcd/bios.c:260` @db41b28 (matched PS1 code). Older notes:
memory/wip/CD_ready/notes.md.

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit FAIL (tmp/audit-2026-09-29/SUMMARY.md, "Shared-declaration follow-up"): the body reached the
Intr bytes through `extern volatile u8 g_cd_status_a` plus `1 + idx_1494` / `idx_1494 + 2` while the TU
defines Sony's `static volatile CD_intr Intr` (the idiom refused for CD_sync on 2026-07-20,
decisions.md:950).

Change: Intr is declared once in the TU (SOTN bios.c:80 @db41b28), with no other C handle. CD_ready uses
`volatile CD_intr *intr = &Intr`, `idx_1495 = &Intr.ready`, `idx_1496 = &Intr.c` (member addresses).
`*(idx_1496 - 1)` in check2 is kept: it is the ready byte reached through the completion-byte handle,
arithmetic inside the one object. Every other statement is unchanged.

### How it was scored

Same method and caveat as memory/grind/CD_sync/evidence.md: before the rebuild a byte-identical candidate
scores 2 (the `.data`-relative &Intr pair against main's `g_cd_status_a` relocation). `--diff` "no
differing instructions" is the check.

| candidate | score | --diff |
|---|---|---|
| main body, `idx_1494 = &Intr.sync` (+1 / +2 kept) | 2 | no differing instructions |
| `&Intr.sync`, `&Intr.ready`, `&Intr.c` | 2 | no differing instructions |
| **landed: `intr = &Intr`, `&Intr.ready`, `&Intr.c`** | 2 | no differing instructions |
| same with `&intr->ready`, `&intr->c` | 2 | no differing instructions |
| plain member access everywhere, no handles (rejected/ff-intr-plain-member-31.c) | 31 | 11 source-level hunks |
| check2 through `*idx_1495` instead of `*(idx_1496 - 1)` | 19 | 12 operand-only hunks |

Single-site ablations on the u8-handle form (one handle access replaced by plain member access):
t0 `Intr.sync` = 5; tb `Intr.ready` = 24, `*idx_1495` = 22; ready callback `Intr.ready` = 24; sync
callback `Intr.sync` = 20; completion check `Intr.c` = 15; completion clear `Intr.c = 0` = 14; check2 read
`Intr.ready` = 15, `*idx_1495` = 19; check2 clear `Intr.ready = 0` = 14, `*idx_1495 = 0` = 19. On the
landed form: ready callback `intr->ready` = 13; check2 read and clear via `intr->ready` = 13; completion
check and clear via `intr->c` = 19. Every handle is needed.

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox CD_ready --disable all` = 0 (179/179 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash CD_ready` = 752cc81b0c982a88 (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.

## Layer-2 rev-intr FAIL (2026-09-30) and Q37 fallback — CD_ready REOPENED

rev-intr FAILed check2's `*(idx_1496 - 1)`: it reaches Intr.ready by stepping -1 from `&Intr.c`,
member-to-member address arithmetic, the same derived-address class as the refused `1 + idx_1494`
(prong (b)/(d)), outside pointer-alias-fake-exception, with no SOTN citation (SOTN bios.c:279-281 uses
plain `Intr.ready`), and undefined behaviour. The target really addresses the ready byte as
`lbu/sb -0x1($s3)` with `$s3 = &Intr+2` (asm/funcs/CD_ready.s:158, :163).

Honest check2 respellings, scored against the rebuilt reference (Intr in both objects, so score 0 would be a
real match). Banked bodies: memory/grind/CD_ready/rejected/.

| check2 read / clear | score (build insns) |
|---|---|
| block-local `volatile u8 *rdy; rdy = &intr->ready;` at the label (ff-intr-check2-block-handle-intr-ready-11.c) | 11 (179) |
| block-local `rdy = &Intr.ready` at the label | 13 (181) |
| block-local `volatile u8 *rdy = &Intr.ready;` at block start | 13 (180) |
| function-scope `rdy = &Intr.ready` beside the other handles | 25 (182) |
| function-scope `rdy = &intr->ready` | 25 (182) |
| function-scope `rdy`, assigned after `new_var3` | 20 (183) |
| read `*rdy`, clear `Intr.ready = 0` | 23 (183) |
| read `Intr.ready`, clear `*rdy = 0` | 24 (184) |
| plain `Intr.ready` (both), from the first round | 15 / 14 (single-site) |
| `*idx_1495` (both), from the first round | 19 |
| `intr->ready` (both), from the first round | 13 |
| whole function in SOTN's shape (while(1), plain Intr members, inlined callback; ff-intr-sotn-shape-20.c) | 20 (176) |

None reaches 0, so under Q37 CD_ready is reverted to `INCLUDE_ASM("asm/funcs", CD_ready);`. The landed
body (score 0 through the refused arithmetic) is banked as
rejected/rev-intr-fail-member-to-member-arith-0.c. The asm references D_800A1494, whose
undefined_syms_auto.txt row now reads "retire with CD_cw, CD_ready". After the src commit:
`queue reopen CD_ready --file system`.

Leads for the next attempt: the target's `-1($s3)` is cse relating the ready byte to the completion
byte's register. The block-local handle derived from `intr` (11) is the closest honest form, and its
residual is operand-only (8 hunks, 0 source-level).

Applied 2026-09-30 (`lock.ps1 rebuild ff-intr`): build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
oracle. `layer2 hash CD_ready` = 8dc0c4b626cbfc4a (body_kind asm). This supersedes the C-body hash
752cc81b0c982a88 in "Post-rebuild" above. check_completion_integrity flags CD_ready
("NOT in queue ... 1 cheat construct") until `queue reopen CD_ready --file system` runs after the src commit.

## 2026-09-30 — laneB: SOTN's own shape closes it once the Alarm merge exists

The rev-intr FAIL's frontier was check2's `-1($s3)` ready-byte access. With Sony's
`Alarm_t Alarm` merged (CD_cw landing 181820b49) and the bios.c static-inline helpers
(set_alarm / get_alarm / callback, bios.c:95 / :102 / :210 @aa53500) available in the TU,
SOTN's matched CD_ready (src/main/psxsdk/libcd/bios.c:260 @aa53500) compiles to the target
VERBATIM: plain `Intr.c` / `Intr.ready`, no handles, no wraps, no staging. cse/loop hoist
&Intr, &Intr+1, &Intr+2 into $s2/$s6/$s3 and express check2's ready byte as -1($s3) by
themselves. The earlier SOTN-shape probe (rejected/ff-intr-sotn-shape-20.c, 20) differed
only in using per-word Alarm externs and hand-inlined helpers.

Measured (probes-0930/gen.py builds a full system.c copy with the helper block moved
above CD_sync, Sony's order; scored with memory/grind/CD_cw/probes-0930/score_full.py):
| variant | CD_ready | CD_sync | CD_datasync | CD_cw |
|---|---|---|---|---|
| SOTN CD_ready (candidate.c), CD_sync/CD_datasync as on main | 2 (0 scored hunks; the &Intr lui/addiu against main's D_800A1494 relocation, same artifact as CD_cw's) | 0 | 0 | 0 |
| + SOTN CD_sync (probes-0930/CD_sync_sotn.c, bios.c:232) | 2 | 0/160 | 0 | 0 |
| + SOTN CD_datasync (probes-0930/CD_datasync_sotn.c, bios.c:459) | 2 | 0 | 0/91 | 0 |
| CD_datasync with a direct return on each exit (rejected/datasync-direct-returns-4.c) | – | – | 4/91 (94 insns) | – |
Every other scorable system.c function stays 0 in the combined file.

So CD_sync and CD_datasync can shed every FAKE they carry (pointer-alias handles,
do-while(0) wraps, named staging intermediates, the staged `src` reuse): each becomes
SOTN's verbatim body. The only construct left needing paperwork is CD_datasync's `ret`,
written on each of its three exits exactly as SOTN's (bios.c:460): Q51 citation + Q53
FAKE note, direct returns = 4.
