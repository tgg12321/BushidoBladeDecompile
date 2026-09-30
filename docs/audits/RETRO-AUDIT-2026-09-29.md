# Retro-audit of landings 2026-09-19..09-29 — outcome and handoff (closed 2026-09-30)

Scope: 101 Match/cheat-cleanup commits since the 2026-09-19 integrity audit (98 functions), plus
shared-declaration follow-ups. Working notes (gitignored): `tmp/audit-2026-09-29/` (SUMMARY.md,
review/batch_0*.md, mech/, q2-*/, g8-screen/). Owner rulings: Q37–Q55 in
`docs/grind/owner-rulings-2026-09-26.md` (batches 21–27), encoded in e3daf03a8.

## Findings
- Mechanically clean (integrity checker, asm-cheat audit, sandbox 0, rulings precede landings), but
  20 semantic FAILs on main, ~30 CONCERNs; 23 landings had no recorded layer-2 PASS.
- Follow-ups found more: libcd CD_* (4 FAIL), func_8002F2D0 (Q41), func_80049718,
  func_800747D8 / func_80075670 / func_800768DC / func_800770B8 (SelWork cluster).

## Process fixes now on main
- **Q39 gate** (42bb8c9b9): `queue done` requires a layer-2 PASS recorded on the exact body
  (`engine/layer2.py`; `engine.cli layer2 hash|record|check|show`; `--expect-hash` required).
- **Departures audit** (1ea88ec25 + a9381e69b): `engine/departures.py`, run by
  `tools/check_completion_integrity.py`; queue items and records carry `addr`; flags anything that left
  the queue after the gate without a PASS on its body at departure. Known limits in its docstring.
- Rules Q46–Q55 (SOTN precedent suffices for matched PS1 code; `/* SOTN: <file>:<line> @<commit> */`;
  FAKE where match-motivated; Q55 precedence) — e3daf03a8.

## Disposition of every audited function

Fixed forward (layer-2 PASS recorded):
- func_80036940 + CdState (Q43) — ff8d33612
- func_8006E534 + func_80070188/8006F100/80071C4C + func_80070F78 (Q44/Q54 -G8 split) — 344674809, 6fd3fb9d3, 9cf311a54, 8302faf93
- getintr, CD_sync, CD_datasync, CD_flush, CD_init, cdrom_IrqHandler (Q42 Intr) — 5dec24b3b
- _SsSndDecrescendo — 9de7d3e8c; _SsSndCrescendo (bonus: FAKE pair unneeded) — 17975ba65
- func_80022580 (Q47) — bcfae589c
- func_800167EC / func_80033D38 (D_80106A50 handle, Q50) — 672e5abf4
- func_80048BA4 (one MATRIX) + Judge[] respell in func_80056CB8/800571C0/80057CC8 — 17e01239b
- func_8004A4E0 entry text (Q40) — 67459eda2

Re-landed honestly by the orchestrator session after reopen: CD_cw (181820b49), CD_ready (598b2e3a8),
PutDispEnv (b6b45920b), func_8006CFBC (9813176b2), func_80052D00 (0eb256779).

Reopened (INCLUDE_ASM + queue; bodies and frontier banked in `memory/grind/<f>/`), still in the queue:
- func_8001C8DC, func_80075F80, func_800759D0, func_8002F770, func_8002D780, func_8002EBDC,
  func_8002F2D0 (Q37/Q38/Q41)
- func_8001F2E4 — reuse needs a Ruling 11 package; jitter reuse is a cse (not allocator) mechanism → owner Q below
- func_8002C22C — honest fix = 2×0x44C table declaration over 89 per-word names in 9 files
- func_8002CD58 — `dist` cross-role reuse; SOTN citation failed; angle split banked (0); owner's 2026-09-25 GTE-island approval still stands
- func_80026DA4 — selected-record role change at a join; implicit-int calls documented
- func_80074E08 — reused `table`, 12 in every honest spelling
- func_80049718 — p_anim/tbl/new_var3 reuse; honest partial (b1) banked
- SelWork cluster: func_800747D8, func_80075670, func_800768DC, func_800770B8 — see
  `memory/grind/func_800768DC/selwork-cluster-2026-09-30.md` (one landing for all SelWork unions)

## Owner decisions at close (2026-09-30, Q56–Q60 in docs/grind/owner-rulings-2026-09-26.md, 29th batch)
- **Q56 sdata_exclude.txt — audit in two steps (NEXT SESSION).** 105 rows / 103 functions (99 completed C),
  160 symbol entries, liveness unmeasured. (1) Mechanically test each row (drop it, rebuild/sandbox); delete
  dead rows. (2) Live rows must meet .claude/rules/maspsx-gate-lists.md (a)-(d) — Sony's ASPSX reproduces the
  shipped bytes; unprovable rows removed and their functions reopened.
- **Q57 — allowed:** a Q33/Q46 union whose s32 member rounds sizeof up for alignment (e.g. 0x92→0x94), when
  every member offset is unchanged, no filler is added and the build is byte-identical. Unblocks the SelWork
  cluster. **Rule text encoded** in .claude/rules/no-new-park-categories.md (Q33 condition (5) and
  § "Trailing alignment padding (… Q57)" after the Q46 extension); decisions.md 2026-09-30 OWNER RULING — Q33
  trailing alignment padding.
- **Q58 — widened:** Ruling 11 (D)(2) accepts any named compiler pass (e.g. cse) as the necessity mechanism,
  with the same dump-proof bar and all other conditions. Helps func_8001F2E4. **Rule text encoded** in
  .claude/rules/ordinary-c-judge-decidable.md Ruling 11 (D), § "Any named compiler pass (… Q58)"; decisions.md
  2026-09-30 OWNER RULING — Ruling 11 (D)(2) accepts any named compiler pass.
- **Q59 — no change:** keep `/* FAKE */` even where the layout suggests the original wrote it that way.
- **Q60 — deferred:** no project-wide sweep; audit functions as other work touches them.

## Follow-ups (not started)
- func_80065800 (orchestrator laneA): sandbox 0 with a ready landing package
  (`memory/grind/func_80065800/tools/land.py`, 713f59461) that owes a layer-2 review.
- `_ss_score`: adopt Sony's `SeqStruct *_ss_score[32]` (SOTN libsnd_i.h:176) across ~15 main.c functions.
- 36 scalar `&Judge` uses in code6cac_b / code6cac_b_tu2 / code6cac_tu2.
- `g_camera_view_state` duplicate linker name for D_800FF558; GaugeWork 0x6A u8 vs s16 views.
- Triaged CONCERNs (Q49) not yet worked: func_8005763C false layer-2 claim, func_80021DB0 /
  func_800571C0 undisclosed reuse, func_80031B24 OOB j[25], func_80021424 overlapping decls,
  func_8001DCB0 cite base+offset evidence.
