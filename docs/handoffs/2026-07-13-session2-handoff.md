# Session handoff — 2026-07-13 (session 2, comprehensive)

Continuation of the 2026-07-13 handoff (`62e86923`). This session ran roughly
14 hours: overnight Grinder + a long parallelized non-Grinder wrap-up phase.

**72 commits ahead of origin** (15 Grinder Match: + 57 docs/rules/audits/paperwork).
Oracle green throughout the session; verified green at handoff time.

## TL;DR — pending owner-decision items

Six items surfaced this session need your call. Ranked by impact:

1. **func_80037540 — Judge held, family scope for review.** (§1)
2. **Project-wide instrument gap: permuter blind on frame-diff functions.** (§2)
3. **Ruling-2 scope for `_spu_FiDMA` outer scheduling.** (§3)
4. **`D_8010277C u8→s8` header flip candidate (MEDIUM).** (§4)
5. **Kengo rename map: 14 SAFE-tier applies + 9 known-wrong entries to delete.** (§5)
6. **Engine subsystem docs: 7 semantic-conflict prose vs named_syms.** (§6)

Detail on each below. All read-only or paperwork-only; the tree is clean.

---

## §1 — func_80037540: Judge held, corrected evidence, family scope

The Grinder ran 4 sessions on `func_80037540` (src/code6cac_b2_post.c) and
stopped clean at handoff. Full ledger at `memory/grind/func_80037540/`.

**Current state (score 0, held pending owner ruling):** the committed
`src/code6cac_b2_post.c` body carries `s32 sp[8]` — a live, 75%-written
argv buffer passed to `bios_Exec`. Score 0, target_insns 43 == build_insns 43,
oracle green.

**The story that unfolded across the 4 sessions:**
- **s2** — refuted a "search space is closed" argument by finding
  `tslLineG5Init` (COMPLETED-C, sandbox 0, cheat-free) as a phantom-slot
  precedent.
- **s3** — killed the phantom-slot escape via ~80 measurements; concluded
  the frame band [25,32] requires a narrow load (lh/lb) that the target
  doesn't have.
- **s4** — **PERMUTER REFUTED s3's proof.** Over 18,769 samples, 7,798
  reached the frame band with ZERO narrow loads. But the CONCLUSION
  survives on stronger ground: the corrected condition is
  **register-ineligibility**, not narrow-load necessity. Of 800 mechanically
  classified frame-band candidates, **0 clean** — every one uses (i) an
  aggregate with ≥2 elements that must be written (+2 stores), (ii)
  volatile qualifier (cheat, free), (iii) address-escape (cheat, forbidden
  2026-07-01), or (v) an unwritten tail (`sp[7]/sp[8]`, the held form).

**Family systemically present in BB2's remaining work:**
- `func_80037540` — `sp[8]` (75%-written argv buffer, held)
- `func_8003DBE4` — `s32 buf[2]; /* dead local */` (active, distance 21)
- `file_LoadSectors` (= `func_800165F8`, src/ings.c:163) — `s32 _pad[2];`
  (active)
- `func_800644FC` — has `s32 dummy_pad; __asm__ volatile("":"=m"(dummy_pad));`
  (active)

**s4 found (and BANKED, did NOT land) a volatile-dead-pad byte-match** —
`s32 sp[6]; volatile char new_var;` where `new_var` is never written and
never read. Volatile prevents register promotion and DCE, so it takes a
stack home; nothing touches it so no load/store emitted. Byte-match against
target. **NOT LANDED** because it's the volatile-coercion cheat family
(same intent as banned `__asm__ volatile("":"=m"(...))`; also same shape as
SOTN's shipped `volatile u32 pad[4]; // FAKE`). Banked at
`memory/grind/func_80037540/rejected/volatile-dead-pad-byte-match-is-cheat.c`.

**The engine cheat-invisible sandbox correctly prices ALL family members
at 0** — measured. `sandbox --disable all` returns score 15 on the volatile
form (cheat_asm_stripped=19, target 43 == build 43). "Cheating can't help"
holds against this family.

**What the owner is being asked:** the held `sp[8]` argv buffer is the
mildest family member — strictly milder than SOTN's `volatile u32 pad[4]`
and the 356 dead-pad precedents s4 censused. The decision isn't
"cheat vs no cheat"; it's **which forbidden construct** to accept for the
0/800 clean case. The Judge held it correctly; the evidence packet is now
materially stronger + corrected (s3's proof was wrong, s4's is right).

**Recommended reading order:** `memory/grind/func_80037540/state.json` for
the 4-session floor history; then `evidence.md` [s4] entries for the
independent-instrument classification. The frontier notes are the
ruling-request packaging.

---

## §2 — Project-wide instrument gap: permuter blindness on frame-diff functions

**Discovered by s4 while working func_80037540. Affects the permuter
modality for every frame-shortfall function in the queue.**

`tools/permuter_campaign.py` invokes decomp-permuter but **never passes
`--stack-diffs`**. decomp-permuter's default Scorer sets
`stack_differences=False`, which normalizes sp-relative offsets away. Any
function whose remaining gap is a frame-size or stack-offset shift scores
0 under this metric even when the bytes are completely wrong.

**Measured** on func_80037540:
- Chassis `sp[8]` (true match) → blind score 0, honest score 0.
- Chassis `sp[6]` (15 insns wrong) → blind score 0, honest score 114.

The 15-off form is **indistinguishable from the true match** under the
default metric. The first launch this session literally printed
`"base score = 0 ... Found zero score! Exiting."` on a non-matching form.

**Blast radius** — any queue item whose remaining gap is a frame/stack
delta will show false matches under the permuter modality. Named examples
from the s4 evidence:
- `func_80037540` (current, held)
- `func_8003DBE4`
- `file_LoadSectors` / `func_800165F8`
- `func_800644FC`

**Fix** — patch `tools/permuter_campaign.py` to pass `--stack-diffs` (or
add `stack_differences=True` at the Scorer construction site). Simple
one-line-ish change but requires care re: how the campaign harness routes
the flag.

**Not fixed this session.** Reported here so:
- Owner can decide whether to patch immediately
- Grinder's permuter-modality output on frame-diff functions should be
  treated as suspect until fixed

---

## §3 — Ruling-2 scope for `_spu_FiDMA` outer scheduling

**Paperwork landed; src close deferred pending owner ruling.**

Ruling-2 (`fork-divergence-inline-asm`, sanctioned 2026-07-13) is codified
at `.claude/rules/fork-divergence-inline-asm.md`. Four-gate evidence bar:
crash + cc1psx counter-exhibit + target-byte match + probe grid.

**Attempted close of `_spu_FiDMA`** (in DispUpdateStatusMessage splice in
`src/main.c`):
- Wait-loop asm island landed byte-exact against target's
  `0x80088BD8..0x80088C10` (volatile RMW + condition check + loop +
  delay-slot `+1` fill + reorg-compensation `-1`).
- **4 residual insns in outer structure** (`0x80088C14..0x80088C5C`):
  bios_DeliverEvent's jal delay slot + speculative `lui $a0, 0xF000` preload
  for the else branch.
- These are **ordinary scheduler-level differences**, not crashes.
  Ruling-2 explicitly does NOT cover fork-divergences without a crash.

**Reverted to paperwork-only.** DispUpdateStatusMessage stays `active`.
When Grinder reaches it, the wait-loop lever is validated; the outer
scheduling is ordinary pure-C grind work.

**Owner options:**
- **(a)** Widen Ruling-2 to include scheduling-class divergence (needs
  its own evidence gate — the current four-gate bar requires a crash).
- **(b)** Let Grinder search work. May yield to a novel C form; may not.
- **(c)** Accept as canonical-asm authorization for the whole `_spu_FiDMA`
  function (departure from the "region-scoped only" constraint in the
  current rule).

Full session detail appended to `docs/escalations/closer-cc1-fork-divergence.md`.

---

## §4 — `D_8010277C` u8→s8 header flip candidate (MEDIUM)

**Audit report: `docs/naming/header-type-audit-2026-07-13.md`.**

Under the new `header-type-correction-from-use-sites` rule (four-prong
test, first confirmed case: `g_file_vram_timer u16→s16` in `4598a98e`).

**Candidate: `include/code6cac.h:401` `extern u8 D_8010277C;` → `extern s8`.**

Evidence:
- **Prong (a):** codebase-wide grep shows ~11 compensating `(s8)` scalar
  casts across `src/code6cac.c`, `src/code6cac_b.c`, `src/code6cac_c_ab.c`
  — signed array-index sites (`(&D_8008D538)[(s8)D_8010277C]`).
- **Prong (b):** casts predate the session (verified via git blame). Prong
  hinges on whether target emits `lb` (signed) vs `lbu` (unsigned) at the
  reads — needs a build/sandbox verification not run in the audit pass.
- **Prong (c):** would be single-line canonical extern edit. Clean.
- **Prong (d):** 3 `(u8*)` / `(s8*)&D_8010277C` address-view casts (byte-array
  access over adjacent bytes) would SURVIVE the flip. Prong (d) is only
  partially met — the flip doesn't fully eliminate all compensating
  addressing casts. **This is what keeps it at MEDIUM, not HIGH.**

**Rejected/LOW candidates in the audit** (see report):
- `D_80102784/85/86` (u8→s8) — LOW; casts predate but are masked to
  never-negative, so behavior-neutral, prong (b) fails.
- `D_80102787` (u8→s8) — REJECT; the compensating cast was added TODAY
  (commit `e83bce24`, 2026-07-13). Prong (b) timing collision. **Owner
  may want to consider whether that just-added cast itself should be
  reverted** as a smell.

**Notable out-of-strict-scope finding:** the strongest signed semantics
in the tree live on **file-local externs declared inconsistently u16-vs-s16
across TUs** (not in any `include/*.h`) — especially `D_800A3528` with
`if ((s16)x < 0)` + `(s16)x % 3`. Would need hoisting into a shared
header to enter the rule's scope; that's an owner decision (prong (c)
scope constraint).

**Layer-2 verification protocol per the rule:** if you attempt the flip,
the reviewer must independently run the grep + cast-necessity table
against the tree; the audit summary is a starting point, not a verdict.

---

## §5 — Kengo rename map: apply 14, delete 9

**Audit report: `docs/naming/kengo-rename-audit-2026-07-13.md`.**

`tools/rename_funcs.py` carries a 381-entry rename map. Cross-checked
against current-main `named_syms.txt` + `symbol_addrs.txt`:
- 228 APPLIED (in `named_syms` as `<name>_<ADDR>`)
- 114 APPLIED via glabel/decompiled-C (not in named_syms)
- **25 SUPERSEDED** (different name applied at that address)
- **14 PENDING** (only `func_XXXXXXXX` placeholder)
- 0 STALE
- **0 RISKY** — every size-only Kengo match (the historical #1
  false-positive source) is already applied.

**14 PENDING (all SAFE-tier, all system.c CD-ROM + config.c stage/game):**
- `0x800800DC` `cdrom_CheckReady` · `0x80080168` `cdrom_SetDebugLevel`
- `0x800806A4` `cdrom_FramesToBcd` · `0x800807A8` `cdrom_BcdToFrames`
- `0x80081718` `cdrom_SendCmd` · `0x800817A0` `cdrom_ClearIrq`
- `0x80081880` `cdrom_ConfigSPU` · `0x80081974` `cdrom_Shutdown`
- `0x80081D1C` `cdrom_DmaToRam` · `0x80081E1C` `cdrom_DmaChain`
- `0x8003F168` `stage_ExecInitFunc` · `0x8003F218` `game_SetPlayerCount`
- `0x8003F274` `stage_InitCollision` · `0x8003F5CC` `stage_ApplyLighting`

Recommended action: spot-check bodies (readability names, not
functional guarantees) then apply as a single named batch to
`named_syms.txt` + `src/*.c` grep-replace. Keep the CD-ROM cluster
internally consistent since the 10 pending interleave with group-B
addresses currently carrying auto-names.

**25 SUPERSEDED — proposal engine feedback material:**
- **Group A (synonyms — no-op)**: minor spelling differences.
- **Group B (map name is more specific than auto-name; 11 entries)**:
  `cdrom_GetMode` etc. — replacement decisions, best routed to the
  proposal engine.
- **Group C (map name is WRONG; 9 entries)**: `game_SndInit` →
  `motion_CheckSituation_wrapper`, `gpu_LoadClut256` → `suDraw2DLib`,
  `cdrom_Initialize` → `tsltm2loadimage_helper`, etc. **Recommended:
  DELETE these 9 entries from `tools/rename_funcs.py`** so a future
  blind `--apply` can't reintroduce known-wrong names.

The 342 already-applied need no work.

---

## §6 — Engine subsystem docs: 7 semantic conflicts flagged

**Audit report: `docs/engine/audit-2026-07-13.md`.**

engine-docs-auditor swept all 14 `docs/engine/*.md` files. 4 got surgical
fixes committed (`72de981c`): README/ai/cross_reference/motion — dead
`SUBSYSTEM_MAP`/`WORK_QUEUE`/`dc.sh` refs, stale decomp-status counts, 2
mis-filed function entries (notably `func_80082A14` mis-labeled as
"VSync wait loop"; it's actually `damage_CalcHitDamage2`, combat).

**7 semantic conflicts flagged NOT auto-fixed** — cases where doc prose
contradicts a `named_syms` MISNAMED entry, and rewriting from a possibly-
wrong auto-name would be riskier than leaving it. Owner should resolve
each by picking authoritative name + reconciling the prose:

- `D_800A36A4` — doc "stance" vs `g_stage_id_current`
- `D_8008D538` — doc "per-stance mask" vs `g_char_class_id_table`
- `action_check_defense` at `0x80086130`
- `calc_loc_mat_fw*` family (MISNAMED)
- `Vu0SetLightColMatrix*` family (MISNAMED)
- `D_8008EC38`
- `ang_hosei`

Details in the report's "Needs owner attention" section.

---

## What landed this session (72 commits)

### Grinder autonomous (15 Match: + ledger)
Overnight completions on `claude-opus-4-8` both roles:
`func_8001FAE4, func_8001FB34, func_800233AC, se_data_set,
DispSamnailWindow, func_8003047C, func_80030BA8, func_80032064,
func_80033498, func_800342A0, func_800343F0, mottest_disp`,
plus prior overnight `cpu_get_move_pattern_table_number,
camera_SetMatrix_8001DBE4, func_8001B138 (owner-close)`.

Notable Judge behavior: rejected a proposed u16 retype on `func_80033498`
using the new `header-type-correction-from-use-sites` rule — the four
prongs are being enforced correctly.

### Rules & policy (3 new)
- `.claude/rules/header-type-correction-from-use-sites.md` (`f1de3289`) —
  4-prong rule, first case `func_8001B138`
- `.claude/rules/fork-divergence-inline-asm.md` (`c5f8fde3`) — Ruling-2,
  4-gate evidence bar
- `.claude/rules/decomp-loop.md` (`421124a1`) — manual-path per-function
  loop extracted from CLAUDE.md

### Ruling paperwork & evidence
- `docs/escalations/closer-cc1-fork-divergence.md` — Ruling-2 grant +
  attempted-close note
- `docs/escalations/spu3-fork-crash-evidence/` — 8-probe grid, cc1psx
  counter-exhibit, results table

### Doc corpus refresh
- CLAUDE.md 259→165 (`421124a1`)
- CONTRIBUTING.md 281→200 (`0465af8b`)
- BUILD.md dc.sh → engine CLI (`15129315`)
- README.md status counts + retired refs (`9816e5d5`)
- `docs/naming/README.md` refreshed for post-audit state (`0e2d278a`)
- 8 more docs swept for retired refs (`dc521bbb`)
- 6 rules swept for retired dc.sh command examples (`33a20114`)
- 7 tools' docstrings + allowlist cleaned (`9e796d6c`)
- 3 fleet-era superpowers specs RETIRED head notes (`407b1424`)
- `IMMUTABLE_PLATEAUS.md` refreshed to engine syntax + historical note
- Format docs cross-checked; STAGE_BIN.md field-table bug fixed (`b8e83a76`)
- 4 engine subsystem docs surgical fixes (`72de981c`)

### Doc archival
- 6 dc.sh-era + dated snapshots → `docs/history/` with head notes
- New `docs/history/README.md` index (`aed821fe`)
- 1 memory/project note archived (`16b64d79`)

### Audits (6 reports)
- `docs/naming/proposals_audit_2026-07-13.md` (naming DB, 976 rows resolved)
- `docs/naming/header-type-audit-2026-07-13.md` (type candidates)
- `docs/naming/kengo-rename-audit-2026-07-13.md` (rename map)
- `memory/project/audit-2026-07-13.md` (per-function notes)
- `docs/engine/audit-2026-07-13.md` (subsystem docs)
- (format-docs findings in commit body of `b8e83a76`)

## Session invariants that held

- Oracle green throughout the session (verified fresh at handoff time)
- Grinder productivity maintained during the parallel cleanup phase
  (SCOPE VIOLATION safety mechanism validated 3-4 times; discarded
  sessions reverted stray edits + respawned cleanly; some parallel
  agent work was rolled back and had to be reapplied — this is the
  Grinder doing its job, not a bug)
- No CLAUDE.md/AGENTS.md drift (both fresh, CLAUDE.md under hygiene target)
- No src/*.c modifications by non-Grinder agents at commit time
- Every completion-class Grinder commit passed the default-FAIL Judge

## Key file pointers

| Purpose | File |
|---|---|
| Live queue | `& tools/wteng.ps1 main queue next` (also `queue status`) |
| Project status | `docs/STATUS.md` |
| Grinder ledgers | `memory/grind/<func>/` |
| Judge decisions | `docs/grind/decisions.md` |
| Grinder journal | `docs/grind/journal.md` |
| Escalations | `docs/escalations/` |
| Sanctioned lever catalog | `.claude/rules/codegen-technique-index.md` |
| Handoffs directory | `docs/handoffs/` |
| Archived docs | `docs/history/` |
| Audit reports | `docs/naming/*audit-2026-07-13.md`, `docs/engine/audit-2026-07-13.md` |

## Grinder state at handoff

**STOPPED cleanly.** pid 9408 exited via STOP sentinel at 14:39:44.
Last activity: session 4 on `func_80037540` produced the s4 correction
(volatile dead-pad byte-match banked as cheat + permuter-blindness project-
wide finding + 0/800 clean-form classification). Ledger persistence
intact; a future Grinder restart resumes cleanly.

To resume: `Start-Process pwsh -WindowStyle Hidden -ArgumentList
'-NoProfile','-File',"`"$PWD\tools\grinder\grind.ps1`"",'-Model',
'claude-opus-4-8','-JudgeModel','claude-opus-4-8'` (fable-5 quota was
exhausted this thread; both roles on opus-4.8).

## Where to look next

**If picking up conversationally, prioritize by owner-decision impact:**

1. **§2 permuter-blindness fix** — cheapest, highest-leverage. One-line
   patch to `tools/permuter_campaign.py`. Unblocks the permuter modality
   for every frame-diff function in the queue.

2. **§1 func_80037540 ruling** — decision on which family member to
   accept. `sp[8]` argv is the mildest; SOTN precedent exists.

3. **§3 Ruling-2 scope decision for `_spu_FiDMA`** — widen, wait, or
   whole-function.

4. **§5 Kengo apply-batch (14) + delete-batch (9)** — mechanical wins.

5. **§4 `D_8010277C` u8→s8** — needs build verification then follow the
   rule's protocol.

6. **§6 engine docs semantic conflicts** — case-by-case owner review.

**If picking up autonomously** — restart Grinder. The queue advances
naturally; owner decisions above can wait.
