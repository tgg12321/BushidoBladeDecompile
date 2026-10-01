# Grinder judge decisions — the owner's audit trail

> APPEND-ONLY, driver-written. Three structural conventions: (1) the heading
> tokens `OWNER-ESCALATION` and `CANONICAL-ASM GRANT PATH` are machine-read by
> the driver (`tools/grinder/grind.ps1` session validation) — never reword
> them; (2) a `DISCARDED-SESSION MARKER (driver-stamped)` heading VOIDS the
> ruling-shaped text immediately ABOVE it (the discarded session's append) —
> read downward past a marker before crediting any entry; (3) entries are
> historical, dated records — for current state read `engine/queue.json`. Exception to append-only (owner ruling 2026-09-28, entry at the end of this file): outdated OWNER RULING entries are deleted by owner-directed cleanup; a cited entry that is no longer here resolves against the pin commit named in that entry (`git show <pin>:docs/grind/decisions.md`).

> Entries before 2026-10-01 rotated out; full history + any `decisions.md:N` citation resolves at git tag pre-slim-2026-10-01.

## Standing owner rulings — index (rotated 2026-10-01)

One line per OWNER RULING/DECISION entry still present at the rotation (the 2026-09-28 cleanup had already deleted the outdated ones). The operative text is the cited `.claude/rules/` file; the full entry resolves at the tag line given. Later rulings (ruling Q-numbers) are in docs/grind/owner-rulings-2026-09-26.md.

- 2026-07-13 18:05 — func_80037540 — OWNER RULING: oversized-locals family carve-out — GRANTED — `pre-slim-2026-10-01:docs/grind/decisions.md:443`
- 2026-07-14 13:45 — func_8003B3A4 — OWNER RULING (Trenton, recorded by operator) — SANCTIONED — `pre-slim-2026-10-01:docs/grind/decisions.md:501`
- 2026-07-17 10:35 — hirahira_w_frie — OWNER RULING (escalation option a) — TOMBSTONE NARROWED — `pre-slim-2026-10-01:docs/grind/decisions.md:653`
- 2026-07-19 — motion_SetMotion — OWNER RULING (escalation option b) — FAMILY REFUSED — `pre-slim-2026-10-01:docs/grind/decisions.md:789`
- 2026-07-20 — func_80045294 (saTan0Init, src/text1a_c.c) — OWNER RULING (escalation option b) — REFUSED / OWNER-ACCEPTED INCOMPLETE — `pre-slim-2026-10-01:docs/grind/decisions.md:942`
- 2026-07-20 — cpu_side_move_dir_4 (src/system.c) — OWNER RULING (escalation option b) — REFUSED / OWNER-ACCEPTED INCOMPLETE — `pre-slim-2026-10-01:docs/grind/decisions.md:948`
- 2026-07-20 — func_80057CC8 (src/text1b.c) — OWNER RULING (escalation option b) — REFUSED / OWNER-ACCEPTED INCOMPLETE — `pre-slim-2026-10-01:docs/grind/decisions.md:954`
- 2026-07-27 — func_800611A4 (src/text1b.c) — OWNER RULING (escalation option b) — REFUSED / OWNER-ACCEPTED INCOMPLETE — `pre-slim-2026-10-01:docs/grind/decisions.md:1658`
- 2026-07-27 — func_80061658 (src/text1b.c) — OWNER RULING (escalation option b) — REFUSED / OWNER-ACCEPTED INCOMPLETE — `pre-slim-2026-10-01:docs/grind/decisions.md:1664`
- 2026-07-27 — func_80061710 (src/text1b.c) — OWNER RULING (escalation option b) — REFUSED / OWNER-ACCEPTED INCOMPLETE — `pre-slim-2026-10-01:docs/grind/decisions.md:1670`
- 2026-07-27 — func_8001F938 (src/code6cac.c) — OWNER RULING (escalation options a+b refused; disposition = OWNER-ACCEPTED INCOMPLETE) — REFUSED / OW... — `pre-slim-2026-10-01:docs/grind/decisions.md:1676`
- 2026-08-04 — -mel toolchain-configuration adoption (OWNER-DIRECTED) — func_80060E38 → COMPLETED-C — `pre-slim-2026-10-01:docs/grind/decisions.md:3394`
- 2026-08-06 — owner rulings on the pending shelf (four items) — `pre-slim-2026-10-01:docs/grind/decisions.md:3488`
- 2026-08-06 — SOTN evidence verdicts + owner rulings (resolves the two evidence passes) — `pre-slim-2026-10-01:docs/grind/decisions.md:3507`
- 2026-08-06 — owner ruling: ALL asmfix entries are debt; end state is ZERO regfix + ZERO asmfix — `pre-slim-2026-10-01:docs/grind/decisions.md:3544`
- 2026-08-06 — owner grants 9 canonical-asm authorizations (evidence packets batch 1+2) — `pre-slim-2026-10-01:docs/grind/decisions.md:3593`
- 2026-08-07 — OWNER ELECTION: migrate the oracle-compiler baseline to reproducible stock GCC 2.7.2 — `pre-slim-2026-10-01:docs/grind/decisions.md:3734`
- 2026-08-07 — OWNER APPROVES the full naming reset wave (307 ops) — `pre-slim-2026-10-01:docs/grind/decisions.md:3756`
- 2026-08-07 — ELECTION REVISED: legacy-oracle adopted as the committed reproducible baseline (SOTN pattern) — `pre-slim-2026-10-01:docs/grind/decisions.md:3773`
- 2026-08-07 — owner rules on the two open auth-shelf questions (batch 1+2 close-out) — `pre-slim-2026-10-01:docs/grind/decisions.md:3805`
- 2026-08-07 — owner RATIFIES the D_800F1AEC carve-out, converted to the rule's own spelling — `pre-slim-2026-10-01:docs/grind/decisions.md:3838`
- 2026-08-07 — owner CONFIRMS the option-(b) ruling as standing; RTL pathway deferred — `pre-slim-2026-10-01:docs/grind/decisions.md:3868`
- 2026-08-08 — func_80048AD0 — delegated owner ruling attempted; GRANT WITHHELD after layer-2 FAIL — escalation remains OPEN — `pre-slim-2026-10-01:docs/grind/decisions.md:3986`
- 2026-08-10 — func_80036FD4 — CLEAN RECORD FORM ALSO BYTE-PROVEN (second full-build green); layer-2 FAIL stands on annotation/span/ruling grounds —... — `pre-slim-2026-10-01:docs/grind/decisions.md:4118`
- 2026-08-10 — OWNER RULING (in person) — func_80036FD4 record merge: GRANTED, form (ii) — `pre-slim-2026-10-01:docs/grind/decisions.md:4183`
- 2026-08-10 — OWNER RULING (in person) — func_8007C7A0 + twin func_8007C86C: ternary clamp ban LIFTED, single-instance, conditions binding — `pre-slim-2026-10-01:docs/grind/decisions.md:4245`
- 2026-08-11 — OWNER RULING (in person) — func_80052930: canonical-asm AUTHORIZED (gte-3x3 cluster completion) — `pre-slim-2026-10-01:docs/grind/decisions.md:4293`
- 2026-08-11 — OWNER RULING (in person) — raw `.word` cop2 spellings: canonical iff cop2-INTERNAL; GPR-carrying or non-cop2 = injection — `pre-slim-2026-10-01:docs/grind/decisions.md:4316`
- 2026-08-11 — OWNER RULING (in person) — main (ings.c): chained same-variable accumulation GRANTED as a sanctioned family extension — `pre-slim-2026-10-01:docs/grind/decisions.md:4343`
- 2026-08-17 — OWNER RULINGS — all five pending escalations resolved (batch evaluation, five independent adversarial packets) — `pre-slim-2026-10-01:docs/grind/decisions.md:5509`
- 2026-08-17 — func_8002FDB0 — OWNER RULING — GRANTED (canonical-asm authorization, existing GTE family) — `pre-slim-2026-10-01:docs/grind/decisions.md:5515`
- 2026-08-17 — func_8001979C — OWNER RULING — GRANTED (named-intermediate family clarified: scoped by shape, six mandatory bounds) — `pre-slim-2026-10-01:docs/grind/decisions.md:5532`
- 2026-08-17 — func_8003B9D0 — OWNER RULING — flat-array retype REFUSED; per-word-symbol -> aggregate-merge family GRANTED (struct-table form) — `pre-slim-2026-10-01:docs/grind/decisions.md:5548`
- 2026-08-17 — func_8001E404 — OWNER RULING — "reconstructed compiled-out call site" REFUSED; unwritten-pad carve-out RE-SCOPED (three functions) — `pre-slim-2026-10-01:docs/grind/decisions.md:5568`
- 2026-08-17 — MoveImage — OWNER RULING — const/RTX_UNCHANGING type-level family REFUSED (on evidence); function reopened under the Phase-3 psxsdk-ad... — `pre-slim-2026-10-01:docs/grind/decisions.md:5588`
- 2026-08-18 — func_8003CF84 — OWNER RULING — pad carve-out EXTENDED: trailing 8-byte pad, this function only — `pre-slim-2026-10-01:docs/grind/decisions.md:5613`
- 2026-08-18 — WORKFLOW — OWNER RULING: user-escalation/approval REMOVED; Judge is the sole gate; borderline ledger established — `pre-slim-2026-10-01:docs/grind/decisions.md:5631`
- 2026-08-18 — WORKFLOW — OWNER-DIRECTED CHEAT AUDIT: three findings, zero-tolerance review — no false completions found; documentation/detector/reco... — `pre-slim-2026-10-01:docs/grind/decisions.md:5851`
- 2026-08-19 — func_80038170 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:7083`
- 2026-08-20 — func_80038170 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:7130`
- 2026-08-20 — func_80038170 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:7266`
- 2026-08-20 — func_80017FA0 (src/code6cac.c) — MATCHED IN PURE C — SUPERSEDES THE 2026-07-27 OWNER RULING (REFUSED / OWNER-ACCEPTED INCOMPLETE) — `pre-slim-2026-10-01:docs/grind/decisions.md:7573`
- 2026-08-20 - func_80017FA0 (src/code6cac.c) - MATCHED IN PURE C (goto-formed inner loop) - SUPERSEDES THE 2026-07-27 OWNER RULING AND THE 2026-08-2... — `pre-slim-2026-10-01:docs/grind/decisions.md:7655`
- 2026-08-20 — OWNER RULING — `_SANCTIONED_UNWRITTEN_PADS` extended: func_80047EE8 + func_80047FBC, `("pre_pad", 8)` each — `pre-slim-2026-10-01:docs/grind/decisions.md:8872`
- 2026-08-20 — func_800481E8 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:8993`
- 2026-08-20 — OWNER RULING — commutative-operand-order carve-out: single justified target-matching order sanctioned — `pre-slim-2026-10-01:docs/grind/decisions.md:9018`
- 2026-08-20 — func_80041688 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:9202`
- 2026-08-20 — func_80049A2C — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:9346`
- 2026-08-21 — func_800858D0 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:9641`
- 2026-08-21 — OWNER RULING — `find_empty_if_dead_reads` gains a strictly-keyed F6 allowlist: func_800858D0, condition `D_80101BCC` — `pre-slim-2026-10-01:docs/grind/decisions.md:9756`
- 2026-08-22 — OWNER RULING — parked-but-proven audit: three `_SANCTIONED_UNWRITTEN_PADS` rows, the func_80038170 integration, two driver-gate fixes,... — `pre-slim-2026-10-01:docs/grind/decisions.md:9851`
- 2026-08-24 — cc1 fork-crash fix — OWNER RULING: APPROVED as bug-fix scope — `pre-slim-2026-10-01:docs/grind/decisions.md:10220`
- 2026-08-24 — cc1 fork-crash fix ADOPTED (per the same-date owner ruling) — `pre-slim-2026-10-01:docs/grind/decisions.md:10238`
- 2026-08-24 — OWNER CAMPAIGN: rules-to-zero (operator-filed, owner in conversation) — `pre-slim-2026-10-01:docs/grind/decisions.md:10256`
- 2026-08-24 — packet resolutions: func_80038C70 + func_80041188 — OWNER RULING (auto-reject class established) — `pre-slim-2026-10-01:docs/grind/decisions.md:10547`
- 2026-08-25 — func_800460E4 — OWNER RULING: YES (decision packet of 2026-08-25 09:49) — `pre-slim-2026-10-01:docs/grind/decisions.md:10904`
- 2026-08-25 — SioSyncroRead — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:10977`
- 2026-08-25 — func_800307D0 (cpu_check_tubazeri_2) — OWNER RULING: (a) (decision packet of 2026-08-25 16:01) — `pre-slim-2026-10-01:docs/grind/decisions.md:11345`
- 2026-08-25 — func_800645B0 — OWNER RULING: YES (decision packet of 2026-08-25 16:19) — `pre-slim-2026-10-01:docs/grind/decisions.md:11378`
- 2026-08-25 — func_8003800C (damage_DebugDisp) — OWNER RULING: YES (proactive, on the 17:06 layer-1 FAIL) — `pre-slim-2026-10-01:docs/grind/decisions.md:11393`
- 2026-08-26 - get_alarm / func_8007DC9C (src/display.c) - STATUS CORRECTION (not an escalation, no owner decision requested) — `pre-slim-2026-10-01:docs/grind/decisions.md:12750`
- 2026-08-26 — func_8002FC80 — JUDGE ESCALATE on final call (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:14150`
- 2026-08-26 — func_80061250 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:14382`
- 2026-08-27 — func_80041188 — OWNER RULING — split-increment construct ALLOWED; escalation spent; continue-directive to s45 — `pre-slim-2026-10-01:docs/grind/decisions.md:14578`
- 2026-08-30 — OWNER RULINGS — escalation batch resolved (all 23 escalated items dispositioned) — `pre-slim-2026-10-01:docs/grind/decisions.md:14615`
- 2026-08-31 — func_8002FC80 — JUDGE ESCALATE on final call (canonical-asm-grant) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:16649`
- 2026-08-31 — func_8002D320 — JUDGE ESCALATE on final call (canonical-asm-grant) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:16787`
- 2026-08-31 — OWNER RULING (ordinary-c-judge-decidable, 73bee8f8) — escalation retirement + migration — `pre-slim-2026-10-01:docs/grind/decisions.md:16814`
- 2026-09-01 — FORECLOSED-BUCKET REVIEW — OWNER RULING (batch): 12 unparks, 2 grants, 1 conditional, 4 foreclosures affirmed, 1 question logged — `pre-slim-2026-10-01:docs/grind/decisions.md:17582`
- 2026-09-01 — cop2 materialize-then-copy WIDENED ANCHOR — OWNER GRANT (informed ruling) — `pre-slim-2026-10-01:docs/grind/decisions.md:17921`
- 2026-09-01 — CD_sync (src/system.c) — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED (owner Ruling D executed in full; the CD_intr aggregate... — `pre-slim-2026-10-01:docs/grind/decisions.md:18118`
- 2026-09-01 - func_80045294 (saTan0Init, src/text1a_c.c) - RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED (owner Ruling A named probe executed... — `pre-slim-2026-10-01:docs/grind/decisions.md:18244`
- 2026-09-01 — CD_ready (src/system.c) — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED (owner Ruling A discharged in full; both live frontier... — `pre-slim-2026-10-01:docs/grind/decisions.md:18662`
- 2026-09-01 — func_8002FF20 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:19814`
- 2026-09-02 — OWNER RULING (two clarifications, informed) — SDK GTE-macro bodies under cluster condition 3; compound-assignment splits are ordinary C — `pre-slim-2026-10-01:docs/grind/decisions.md:20422`
- 2026-09-02 — func_800861BC — INTEGRATION HANDOFF: bytes proven (oracle SHA1), closure runs through the aggregate-merge family (owner ruling 2026-08... — `pre-slim-2026-10-01:docs/grind/decisions.md:20503`
- 2026-09-02 — func_800861BC — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:20637`
- 2026-09-02 — OWNER RULING — foreclosure mechanics: unpark resets the exhaustion window; standing-ruling foreclosure is scoped to the endgame-lock f... — `pre-slim-2026-10-01:docs/grind/decisions.md:20941`
- 2026-09-02 — OWNER RULING — foreclosed-bucket disposition: `_SANCTIONED_UNWRITTEN_PADS` row for func_80030580, maspsx label-nop opt-in for func_800... — `pre-slim-2026-10-01:docs/grind/decisions.md:21023`
- 2026-09-03 — func_80033550 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:21527`
- 2026-09-03 — func_80062020 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:21801`
- 2026-09-03 — func_80062020 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:21854`
- 2026-09-03 — func_80062020 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:22016`
- 2026-09-04 — get_alarm — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:22385`
- 2026-09-04 — OWNER RULING — foreclosed-bucket re-evaluation: 3 unparks with named probes, 3 foreclosures affirmed, 4 driver/engine defects ordered... — `pre-slim-2026-10-01:docs/grind/decisions.md:22421`
- 2026-09-04 — OWNER RULING — `main`: the per-function maspsx prefill-label gate is a FIDELITY gate; build it (spends the 2026-09-04 borderline.md po... — `pre-slim-2026-10-01:docs/grind/decisions.md:22884`
- 2026-09-05 — func_80034F88 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:23408`
- 2026-09-05 — func_80022F34 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:23775`
- 2026-09-05 — func_80022F34 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:24012`
- 2026-09-05 — func_800480C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:24204`
- 2026-09-06 — func_8004473C — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:24411`
- 2026-09-06 — _addque2 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:24463`
- 2026-09-06 — func_8002C61C — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:24663`
- 2026-09-07 — foreclosed-bucket review (owner-directed, all 4 items) — 3 UNPARKED, 1 CLOSED — `pre-slim-2026-10-01:docs/grind/decisions.md:25148`
- 2026-09-07 — OWNER RULING (delegated: "research those pending items and follow through with your best judgment") — `-msoft-float` ADOPTED as canoni... — `pre-slim-2026-10-01:docs/grind/decisions.md:25287`
- 2026-09-08 — func_800204C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:25470`
- 2026-09-08 — OWNER RULING — rotation replaces foreclosure (`.claude/rules/rotation-not-foreclosure.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:25710`
- 2026-09-08 — func_800335D8 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26042`
- 2026-09-10 — func_80018094 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26246`
- 2026-09-10 — func_80074B18 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26345`
- 2026-09-11 — _spu_gcSPU — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26641`
- 2026-09-14 — func_80018094 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26678`
- 2026-09-14 — maspsx `.L`-label load-delay arm — OWNER RULING (substrate / global maspsx behavior change) — `pre-slim-2026-10-01:docs/grind/decisions.md:26713`
- 2026-09-14 — func_80018094 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26789`
- 2026-09-15 — OWNER RULING — the candidate-path no-progress tripwire + a registry row for func_80018094 — `pre-slim-2026-10-01:docs/grind/decisions.md:26826`
- 2026-09-15 — func_80063BD0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:26945`
- 2026-09-15 — _clr — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:27013`
- 2026-09-15 — func_80054604 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:27075`
- 2026-09-15 — func_80076D74 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:27162`
- 2026-09-15 — func_8002D780 — JUDGE ESCALATE on final call (canonical-asm-grant) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait) — `pre-slim-2026-10-01:docs/grind/decisions.md:27395`
- 2026-09-23 — OWNER RULING — one role repeated per block: a reused local (`.claude/rules/ordinary-c-judge-decidable.md` Ruling 5) — `pre-slim-2026-10-01:docs/grind/decisions.md:28551`
- 2026-09-23 — OWNER RULING — Ruling 5 extension: identical writes, record picked beforehand (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28609`
- 2026-09-23 — OWNER RULING — verbatim PsyQ GTE macro islands (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28675`
- 2026-09-23 — OWNER RULING — Ruling 6: one record pointer, one write per exclusive path (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28714`
- 2026-09-23 — OWNER RULING — Ruling 7: sprintf's SOTN buffer-end line, sprintf only (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28760`
- 2026-09-24 — OWNER RULING — DMPSX placeholder command words (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28796`
- 2026-09-24 — OWNER RULING — Ruling 8: vmNoiseOn's SOTN pan-stage `temp`, vmNoiseOn only (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28850`
- 2026-09-25 — OWNER RULING — Ruling 9: one meaning, several constant offsets (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28888`
- 2026-09-25 — OWNER RULING — Ruling 10: verified original source, verbatim reuse (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28943`
- 2026-09-25 — OWNER RULING — scorer: header-exact GTE macro statements are scored as written (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:28986`
- 2026-09-25 — OWNER RULING — Ruling 9 amendment (b′): the layout the code assumes (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29039`
- 2026-09-25 — OWNER RULING — scorer amendment `0(reg)` ≡ `(reg)` (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29099`
- 2026-09-25 — OWNER RULING — func_800288C8 owner-cluster row: none now; granted when a body passes review (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29140`
- 2026-09-25 — OWNER RULING — Ruling 9 (b′)(3) clarification: past-the-end into unreferenced bytes (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29175`
- 2026-09-25 — oracle compiler: narrow PLUS->IOR condition ADOPTED (per owner ruling bcdc1648e) — `pre-slim-2026-10-01:docs/grind/decisions.md:29237`
- 2026-09-26 — OWNER RULING — rotate only when truly stuck across multiple sessions (`.claude/rules/rotation-not-foreclosure.md` Ruling 4) — `pre-slim-2026-10-01:docs/grind/decisions.md:29276`
- 2026-09-26 — OWNER RULING — Ruling 11: a reused local proven necessary by the allocator (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29304`
- 2026-09-26 — OWNER RULING — aggregate-merge prong (a): compiler-necessity evidence, minimal span (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29336`
- 2026-09-26 — OWNER RULING — Ruling 9 (c) clarification: a `break` confined to a loop in between (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29359`
- 2026-09-26 — OWNER RULING — inline_o.h GTE macro blocks as a class (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29373`
- 2026-09-26 — OWNER RULING — per-file -G8 by proof (`.claude/rules/compiler-flags-canonical.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29396`
- 2026-09-26 — OWNER RULING — aggregate-merge (a4′): mixed-field struct (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29433`
- 2026-09-26 — OWNER RULING — -G8 screening scope (`.claude/rules/compiler-flags-canonical.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29458`
- 2026-09-26 — OWNER RULING — maspsx COMMON gate (`maspsx_comm_syms.txt`) (`.claude/rules/maspsx-gate-lists.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29482`
- 2026-09-26 — OWNER RULING — func_8002DE20 per-function GTE grant (`.claude/rules/inline-asm-policy.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29511`
- 2026-09-26 — OWNER RULING — path-wise re-store (`.claude/rules/ordinary-c-judge-decidable.md` Ruling 5 2(c), Ruling 11 (B)(2)) — `pre-slim-2026-10-01:docs/grind/decisions.md:29539`
- 2026-09-26 — OWNER RULING — aggregate-merge (a4′): forced-in bytes typed by their real users (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29572`
- 2026-09-26 — OWNER RULING — aggregate-merge (a4′): signedness when the binary cannot tell (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29610`
- 2026-09-26 — OWNER RULING — COMMON-list row proof when the archived toolchain is not byte-faithful (`.claude/rules/maspsx-gate-lists.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29646`
- 2026-09-26 — OWNER RULING — -G8 split / respelling order (`.claude/rules/compiler-flags-canonical.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29679`
- 2026-09-26 — OWNER RULING — a compiler patch is a cheat (Q17) (`.claude/rules/no-compiler-divergence.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29699`
- 2026-09-26 — OWNER RULING — scheduling fallout (nops and shifted branch offsets) (`.claude/rules/maspsx-gate-lists.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29737`
- 2026-09-26 — OWNER RULING — Ruling 12: local copies of a stack-passed parameter (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29764`
- 2026-09-26 — OWNER RULING — per-branch constants as a Ruling 11 value (`.claude/rules/ordinary-c-judge-decidable.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29796`
- 2026-09-27 — OWNER RULING — per-file declarations of the same bytes (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29819`
- 2026-09-27 — OWNER RULING — dummy constant locals as array subscripts (`.claude/rules/named-local-fake-exception.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29868`
- 2026-09-27 — OWNER RULING — per-file declarations win over FAKE-construct spellings (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29930`
- 2026-09-27 — OWNER RULING — cc1psx corroboration only where it reproduces the matching form (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29953`
- 2026-09-27 — OWNER RULING — proof standard for Q21 (1): mechanism + search (`.claude/rules/no-new-park-categories.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:29990`
- 2026-09-28 — OWNER RULING — outdated ledger entries are deleted (`.claude/rules/judge-sole-gate.md` § The borderline ledger) — `pre-slim-2026-10-01:docs/grind/decisions.md:30020`
- 2026-09-28 — OWNER DECISION — per-branch constants read only inside their branch: approved, then WITHDRAWN (no rule change) — `pre-slim-2026-10-01:docs/grind/decisions.md:30053`
- 2026-09-28 — OWNER RULING — always-zero narrow frame locals and per-branch constant holders, as FAKE locals (`.claude/rules/named-local-fake-except... — `pre-slim-2026-10-01:docs/grind/decisions.md:30068`
- 2026-09-28 — OWNER RULING — a GTE-macro-input copy as a Ruling 11 value (`.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11 (C)(3)) — `pre-slim-2026-10-01:docs/grind/decisions.md:30083`
- 2026-09-28 — OWNER RULING — func_800187F4 per-function GTE grant: four DMPSX command words (`.claude/rules/inline-asm-policy.md` § Per-function gra... — `pre-slim-2026-10-01:docs/grind/decisions.md:30096`
- 2026-09-28 — OWNER RULING — FAKE-construct spellings do not count against Ruling 11 necessity (`.claude/rules/ordinary-c-judge-decidable.md` § Ruli... — `pre-slim-2026-10-01:docs/grind/decisions.md:30107`
- 2026-09-28 — OWNER RULING — Ruling 11 (D)(3) proof standard: mechanism + search; a resized FAKE construct is the same construct (`.claude/rules/ord... — `pre-slim-2026-10-01:docs/grind/decisions.md:30116`
- 2026-09-29 — OWNER RULING — Ruling 13: unattended-run delegation, clearly-fine ordinary C without precedent (`.claude/rules/ordinary-c-judge-decida... — `pre-slim-2026-10-01:docs/grind/decisions.md:30134`
- 2026-09-29 — OWNER RULING — a union word view over small fields (`.claude/rules/no-new-park-categories.md` § aggregate merge, amendment to prong (d)) — `pre-slim-2026-10-01:docs/grind/decisions.md:30147`
- 2026-09-29 — OWNER RULING — a plain copy as one value of a Ruling 11 variable (`.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11 (C)(3)) — `pre-slim-2026-10-01:docs/grind/decisions.md:30163`
- 2026-09-29 — OWNER RULING — a trailing unused local array with sibling evidence (`.claude/rules/no-new-park-categories.md` § Phantom-frame-slot vol... — `pre-slim-2026-10-01:docs/grind/decisions.md:30177`
- 2026-09-29 — OWNER RULING — one cast store on a local array (`.claude/rules/no-new-park-categories.md` § aggregate merge, "Amendment: one cast stor... — `pre-slim-2026-10-01:docs/grind/decisions.md:30202`
- 2026-09-30 — OWNER RULING — the per-function maspsx COMMON gate is a cheat (Q9/Q15/Q18 withdrawn) (`.claude/rules/maspsx-gate-lists.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:30231`
- 2026-09-30 — OWNER RULING — object-relative rodata alignment; per-file align sed retired (`.claude/rules/rodata-object-alignment.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:30270`
- 2026-09-30 — OWNER DECISION — maspsx `.L`-label mflo-hazard fix declined (func_80058580) — `pre-slim-2026-10-01:docs/grind/decisions.md:30305`
- 2026-09-30 — OWNER RULING — maspsx `.L`-label mflo-hazard fix adopted (supersedes the same-day decline) — `pre-slim-2026-10-01:docs/grind/decisions.md:30323`
- 2026-09-30 — OWNER RULING — Q33 union word views on struct members (`.claude/rules/no-new-park-categories.md` § aggregate merge, amendment to prong... — `pre-slim-2026-10-01:docs/grind/decisions.md:30350`
- 2026-09-30 — OWNER RULING — duplicated calls into arms, byte-identical only (`.claude/rules/duplicated-statement-into-arms.md` § Duplicated calls) — `pre-slim-2026-10-01:docs/grind/decisions.md:30362`
- 2026-09-30 — OWNER RULING — volatile locals: SOTN precedent or target-byte proof (`.claude/rules/legitimate-volatile-interrupt-touched.md` § Owner... — `pre-slim-2026-10-01:docs/grind/decisions.md:30371`
- 2026-09-30 — OWNER RULING — SOTN precedent suffices (`.claude/rules/no-new-park-categories.md` § Owner ruling 2026-09-30 — SOTN precedent suffices) — `pre-slim-2026-10-01:docs/grind/decisions.md:30383`
- 2026-09-30 — OWNER RULING — Q50 overrides the 2026-09-24 SOTN-reuse decline (`.claude/rules/ordinary-c-judge-decidable.md` Rulings 9, 10, 11; `.cla... — `pre-slim-2026-10-01:docs/grind/decisions.md:30401`
- 2026-09-30 — OWNER RULING — SOTN's self-marked fakes count, with the same FAKE marking (`.claude/rules/no-new-park-categories.md` § SOTN precedent... — `pre-slim-2026-10-01:docs/grind/decisions.md:30412`
- 2026-09-30 — OWNER RULING — family prerequisites still owed on SOTN precedent (`.claude/rules/no-new-park-categories.md` § SOTN precedent suffices,... — `pre-slim-2026-10-01:docs/grind/decisions.md:30421`
- 2026-09-30 — OWNER RULING — Q50 precedence over older refusals, matched SOTN code only (`.claude/rules/no-new-park-categories.md` § Owner ruling 20... — `pre-slim-2026-10-01:docs/grind/decisions.md:30430`
- 2026-09-30 — OWNER RULING — Ruling 14: overnight-run delegation, borderline rule questions within SOTN's standards (`.claude/rules/ordinary-c-judge... — `pre-slim-2026-10-01:docs/grind/decisions.md:30443`
- 2026-09-30 — OWNER RULING — per-function GTE grants for func_8002D780 / func_8002EBDC / func_8002F2D0 / func_8002F770 (`.claude/rules/inline-asm-po... — `pre-slim-2026-10-01:docs/grind/decisions.md:30465`
- 2026-09-30 — OWNER RULING — the global COMMON model (tentative definitions, every file) (`.claude/rules/maspsx-gate-lists.md` § The global COMMON m... — `pre-slim-2026-10-01:docs/grind/decisions.md:30477`
- 2026-09-30 — OWNER RULING — the D_800A37D2 / D_800A37D3 byte pair (`.claude/rules/no-new-park-categories.md` § Owner ruling 2026-09-30 — the D_800A... — `pre-slim-2026-10-01:docs/grind/decisions.md:30490`
- 2026-09-30 — OWNER RULING — Ruling 13 (B) ratified as standing (`.claude/rules/ordinary-c-judge-decidable.md` § Ratification (owner, 2026-09-30, th... — `pre-slim-2026-10-01:docs/grind/decisions.md:30501`
- 2026-09-30 — OWNER RULING — Q33 trailing alignment padding (`.claude/rules/no-new-park-categories.md` § aggregate merge, amendment to prong (d), Tr... — `pre-slim-2026-10-01:docs/grind/decisions.md:30511`
- 2026-09-30 — OWNER RULING — Ruling 11 (D)(2) accepts any named compiler pass (`.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11 (D), Any na... — `pre-slim-2026-10-01:docs/grind/decisions.md:30548`
- 2026-09-30 — OWNER RULING — the per-file gp model (`.claude/rules/per-file-gp-model.md`) — `pre-slim-2026-10-01:docs/grind/decisions.md:30558`
- 2026-09-30 — OWNER RULING — func_800770B8 reused local + restore store REFUSED (no rule change) — `pre-slim-2026-10-01:docs/grind/decisions.md:30615`
- 2026-09-30 — OWNER RULING — Q67: per-file static blocks, 4 merges, exact-size fillers (Q65 scope, part 1) — `pre-slim-2026-10-01:docs/grind/decisions.md:30626`
- 2026-09-30 — OWNER RULING — Q68: maspsx models cc1psx's .sdata placement (Q65 scope, part 2) — `pre-slim-2026-10-01:docs/grind/decisions.md:30635`
- 2026-09-30 — Q65 per-file gp model: amendments A1-A3 (owner rulings Q67 and Q68) — `pre-slim-2026-10-01:docs/grind/decisions.md:30646`
- 2026-09-30 — OWNER RULING — Q69: maspsx -G8 for every C file, Sony library code -G0 (per-file gp model A4) — `pre-slim-2026-10-01:docs/grind/decisions.md:30681`
- 2026-09-30 — OWNER RULING — Q70: three typing defaults (per-file gp model A5) — `pre-slim-2026-10-01:docs/grind/decisions.md:30702`
- 2026-09-30 — OWNER RULING — Q71: fewest aligned pieces for runs no single object can occupy (A6) — `pre-slim-2026-10-01:docs/grind/decisions.md:30710`
- 2026-09-30 — OWNER RULING — Q72: a data-only file between members of an approved merge group joins it (A7) — `pre-slim-2026-10-01:docs/grind/decisions.md:30722`
- 2026-09-30 — OWNER RULING — the practice-menu per-word reads (Q73) — `pre-slim-2026-10-01:docs/grind/decisions.md:30735`
- 2026-10-01 — OWNER RULING — inline-asm audit: three canonical-asm entries de-authorized and re-queued — `pre-slim-2026-10-01:docs/grind/decisions.md:30767`

## 2026-07-23 19:27 — func_80049718 — ruling: Cluster A (`p_anim = &D_800EF980[arg0]`) closes to distance 0 ONLY via a single- — **PASS**

RULING REQUEST resolved in favor of disposition (a): the base-pointer local `s16 *tbl = D_800EF980; p_anim = tbl + arg0;` is ORDINARY POINTER ARITHMETIC, not a pointer-alias-fake-exception. It is committable as clean COMPLETED-C with NO /* FAKE */ annotation and NO fake-exception gating. Reasoning, tied to the ledger and rule files I verified myself: (1) The construct is byte-for-byte the same lever that register-alloc-pure-c.md's 'Confirmed CLOSURES' section documents for tslPolyF4Init (system.c) as 'Ordinary pointer arithmetic' -> COMPLETED-C, no annotation. I did not take the rule doc on faith: I confirmed on main that tslPolyF4Init is a genuine zero-cheat closure -- src/system.c:233,239-240 carries the identical `s32 *base; base = g_cd_sector_buf; elem = base + idx;` split, it has 0 rules in regfix.txt AND 0 in asmfix.txt, and it is not present in engine/queue.json (i.e. it byte-matches the oracle with zero cheats and no FAKE tag). This is direct precedent on the IDENTICAL construct, not the 'cluster precedent / circumstantial evidence' the owner policy forbids crediting. (2) The distinguishing line between the two candidate rules is redundancy/semantic-role, and it lands cleanly on the ordinary side. pointer-alias-fake-exception.md governs a REDUNDANT second handle to a global -- `Type* t = &g_Thing; t->field` where `g_Thing.field` would do with zero extra arithmetic, unused aliases (`tilemap = &g_Tilemap`), pass-through aliases (`e2 = self`) -- where the local adds no computation and its only role is codegen steering. Here `tbl` is the LIVE BASE OPERAND of a genuine pointer-arithmetic expression `tbl + arg0`, consumed exactly once; it REPLACES the `&D_800EF980[arg0]` subscript rather than being held redundantly alongside a direct use. There is one computation spelled two equivalent ways, not a redundant handle. (3) Against the 6-test cheat checklist: semantic purpose YES (tbl is the table base; tbl+arg0 is the element pointer); human-writable-from-spec YES (base+offset pointer arithmetic is bread-and-butter C, and the name `tbl` carries real meaning, no pad/dummy/spill/fake coercion-announcing name); GCC-steering-as-SOLE-function NO -- and this is decisive: every instruction the construct emits is LIVE in the output (it flips the `addu $s0,$v1,$v0` operand order and changes base/index register assignment; hypotheses.md/evidence.md s2 confirm the base is materialized into its own pseudo and used), it is NOT dead / DCE'd. The hallmark that separates the forbidden coercion families (unused arrays, dead stores, dead self-assigns, alias renames) from ordinary code is precisely that those are dead-in-output while this computation is present-in-output. (4) The s1 layer-1 cheat-reviewer FAIL is an INPUT, never proof (owner policy); it conflated the array base-pointer split with the redundant-second-handle pointer-alias family. The correct classification test -- redundant handle vs live base operand of genuine arithmetic -- resolves to the latter, which register-alloc-pure-c already settled. The full clean-respelling exhaustion ledger the grind built (hypotheses.md/rejected: bare pointer-add=11, cast=4, index-precompute=4, reuse-var=9; only a base-holding pseudo reaches 0) is good diligence and belongs in the commit body as the mechanism explanation, but it is NOT a prerequisite for legitimacy here, because this is ordinary C, not a last-resort FAKE exception (tslPolyF4Init needed no such ledger). Cluster B (the block-2 store reorder, 6->4) is an independent pure statement reorder already PASSed at layer-1 and is unaffected by this ruling.

## 2026-07-23 19:34 — func_80049718 — final call — **PASS**

FINAL CALL for func_80049718 (text1b.c). Bytes were already settled (sandbox 0, rules retired, full-build SHA1 == oracle per the task premise); I ruled only on whether the committed C is legitimate pure-C a human could have written from spec, free of cheats by any spelling. The diff makes three changes, all of which I verified independently against the tree rather than trusting the ledger:

(1) CLUSTER A -- the prologue address computation. It replaces `p_anim = &D_800EF980[arg0];` wrapped in a semantically-empty `do{...}while(0)` (plus an empty `if(0){}`) with a base-pointer local: `{ s16 *tbl = D_800EF980; p_anim = tbl + arg0; }`, and moves the real statements (var_s3=arg1; the InitFadePanel check) out to straight-line code. This is ORDINARY POINTER ARITHMETIC, not a cheat. `tbl` holds a real, used value (a pointer to the anim table) and p_anim is derived from it; a human writing 'get a pointer to the table and index it by arg0' writes exactly this. The name 'tbl' describes the data (D_800EF980 is a table), not a codegen intent. It contains no __asm__, no hardcoded $N, no register-asm pin, no constraints, no asm("Sym") alias-rename -- GCC fully understands the operation; it is categorically different from the forbidden inline-asm-injection / alias-rename families (.claude/rules/inline-asm-injection.md). I confirmed this exact lever is already documented as a clean COMPLETED-C closure: register-alloc-pure-c.md lines 176-181, tslPolyF4Init (system.c, 2026-06-14), retired the identical `s32 *base = g_cd_sector_buf; elem = base + idx;` -- base-pointer local that materializes the base before the shift to flip the addu operand order -- explicitly labeled 'Ordinary pointer arithmetic', NO /* FAKE */. This is a genuine same-construct precedent, not circumstantial. The prior s1 Judge ruling (git 3bd87bf9, recorded in state.json judge_constraints) directed exactly this: commit the POINTER spelling as clean COMPLETED-C, NO /* FAKE */, ORDINARY path, preferred over the integer-cast spelling; src/text1b.c:959-960 uses precisely `s16 *tbl = D_800EF980; p_anim = tbl + arg0;` with no annotation and no cast contrivance -- the ruling was honored.

(2) CLUSTER B -- reordering four independent halfword init stores in the `if(var_s3!=1)` block so `*(s16*)(obj+4)=6` follows the two zero stores (obj+8, obj+0xA). Every store writes a real value to a distinct struct offset; the order is behaviorally free; source-statement reordering to match schedule is a standard pure-C technique, not a scheduling barrier or dead construct.

(3) Removal of the `do{}while(0)` wrapper and the empty `if(0){}`. These only DELETE no-op perturbers (previously inflating the floor 11->6) -- strictly improving legitimacy; nothing coercive is added.

Independent checks: src/text1b.c:958-965 confirms the perturbers are gone from the actual committed source; grep of regfix.txt and asmfix.txt for func_80049718 returns NO matches, confirming HEAD's 2 regfix reorder rules are genuinely retired (not moved into a rule file or respelled as cheat-asm). The full function body reads as ordinary game logic (struct-field inits, an animation-setup branch, calls to sibling funcs) with zero __asm__, zero register pins, zero volatile coercion, zero dead stores, zero unused arrays. All six cheat-checklist tests pass for every construct in the diff. This is legitimate COMPLETED-C.

## 2026-09-01 07:23 — func_800770B8 — ruling: May a source-level arithmetic IDENTITY be used purely to create an RTL dependenc — **FAIL**

The construct is an algebraically-equivalent detour on a LIVE computation whose only effect is the reference/dependence flow it creates. That is not the 'opaque arithmetic variables' family (whose mechanism, per .claude/rules/loop-rotation-two-shift.md:81, is hiding a value from constant analysis -- `s32 one = 1;`; nothing here is opaque, the value is fully transparent). Its correct family is the combine-foldable chain-extender (.claude/rules/dead-store-fake-exception.md:51-58, owner ruling 2026-07-01), and that entry's defining prerequisite is failed by construction: it sanctions a detour 'that combine folds back to the direct form with ZERO emitted bytes.' The session's own s11/s14 datum is that `(t0*4)/2` DOES fold and is inert (score 29), and `>>1` was chosen precisely because fold() will not collapse it -- so it materializes, as candidate.c's own class-D read-out shows: our row 42 emits `sra $2,$3,0x1` where the target emits `sll $v1,$a1,2`. A byte-materializing chain-extender is an explicit FAIL (tools/grinder/roles/grind-session.md:78-81). Non-membership in the frozen list is therefore a clean FAIL, not a packet (.claude/rules/ordinary-c-judge-decidable.md Ruling 1). Note the claimed rule tension is not real: the role prompt's forbidden-catalog line naming `s32 one = 1;` is a stale entry contradicted by the frozen list in the same file and by no-new-park-categories.md:190 -- but that family does not cover this shape either way. Verified myself: no-new-park-categories.md:179-214 (SOTN-accepted list, no entry covers an arithmetic-identity dependence edge), dead-store-fake-exception.md:51-58, loop-rotation-two-shift.md:78-83, and the class-D residual text in memory/grind/func_800770B8/candidate.c. Answering the session's explicit ask: yes, record floor 5, close this family, and pursue the type-forced derivation instead -- a cursor really derived from group C's t0*4 pointer has a semantic reading and would not emit a `sra` the target does not have. This ruling does NOT clear the separate unresolved do-while(0) prologue wrap; full evidence in hypotheses.md [s14], evidence.md [s14], rejected/s14-classD-*.c.

## 2026-09-01 — func_800770B8 — **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**

Disposition record for `func_800770B8` (src/text1b.c), filed under the owner's standing
auto-ruling of 2026-07-27 (`.claude/rules/endgame-lock-disposition.md`) and recorded, not
asked, per the 2026-08-31 ruling (`.claude/rules/ordinary-c-judge-decidable.md`). This is a
proof-of-foreclosure record. Nothing is addressed to the owner and nothing waits on a reply.

**Chassis, measured this session (s19).** `memory/grind/func_800770B8/candidate.c` applied to
src/text1b.c: `sandbox func_800770B8 --disable all` = **score 5, build_insns 175,
target_insns 175, rules_dropped 0**. The floor has been flat at 5 since s11 (nine consecutive
sessions) across the modalities escalation, structural, synthesis, solver and forensics.

**What the residual is.** The floor-5 body is the target modulo FIVE in-place register names,
verified again this session by a normalised opcode+register row diff
(`tmp/grind/func_800770B8/s19/rows.py`): the only genuinely differing rows are 35, 36
(class B — two stores go through the `p_old` copy `$s1` instead of the raw
`func_8006E49C` result `$v0`) and 62, 63, 64 (class C — `addu $v0,$v0,$v1` /
`addiu $a3,$v0,0x6A` / `addiu $a1,$v0,0x7E` against the target's `addu $v1,$v1,$v0` /
`addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E`). Every other diff line is an objdump alias
(`move` vs `addu …,$zero`, `li` vs `addiu …,$zero`). There is no instruction slack anywhere:
175 == 175.

**Gate (a) — canonical-asm evidence: FAILED.** `python3 tools/scan_hand_coded.py --single
func_800770B8` returns `tier=LOW score=0/8`, with every one of S1–S8 unset ("no strong
hand-coded indicators"): 0 multu/mflo pairs, no empty-body branches, 5 spills over 175 insns
and 14 distinct registers, max load burst 3 in any 8-insn window, no high-similarity sibling
(jaccard < 0.5), no BIOS jumptable pattern, all callee-save uses paired with an `$sp` save, no
redundant mask-before-shift. The function is ordinary compiler output; the canonical-asm grant
path is not available.

**Gate (b) — in-hand SOTN-master precedent: FAILED, and vacuous.** There is no closing
construct in hand to seek precedent for. Both residual classes are attributed to named GCC
passes and both attributions are measured dead:

- class C is decided by `local-alloc.c` `block_alloc`'s operand-tying loop
  (tools/gcc-2.7.2/local-alloc.c:1240-1298), which ties the destination of RTL insn 185
  `(set (reg:SI 110) (plus:SI (reg:SI 109) (reg:SI 108)))` to the FIRST operand for which
  `combine_regs` succeeds. All ten `combine_regs` gates (local-alloc.c:1784-1946) were
  enumerated and typed in s18: six are structurally impossible for two SImode pseudos in a
  plain `addsi3`, one is unreachable on MIPS (all candidates GR_REGS), one detaches the dest
  from BOTH operands, and the only two C-reachable gates both require the `D_800A36A0`
  reload to stay live past the add — which in a zero-slack single basic block must add or
  delete an instruction. Gate 1 was actually built and priced: 174 insns, score 49
  (`rejected/s18fx-classC-blocklocal-reload-defeats-op1-tie-174insn-score49.c`).
- class B is foreclosed by price four times over (s7/s8/s9/s14): every spelling that reaches
  the raw result pseudo lets flow.c delete the copy and its four dependents, collapsing the
  body to 170 insns, five FEWER than the target's 175.
- the ONLY construct ever measured to repair the alternative (flipped) basin was the
  arithmetic identity detour `(t0 * 4) >> 1`, which the Judge **FAILED** on 2026-09-01
  (docs/grind/decisions.md, 07:23 entry) as a byte-materialising chain-extender outside the
  frozen family list. Under the 2026-08-24 / 2026-08-31 auto-reject class that is a clean
  FAIL, not an open question.
- Precedent census run this session: `docs/reference/sotn-construct-index.md` (1,056 lines)
  has **zero** PSX/GCC-2.7.2 entries matching `operand order|addend|tie|combine_regs|
  local-alloc|reload live` or `identity|detour|dependence|chain-extend|redundant read`.
  A NEGATIVE census is a failed gate, not an open question.

**The last live frontier item was killed this session.** s18 carried one well-posed untried
question: run `sched_solver`'s `perturb.py` on the PLAIN operand-order flip (29/175), whose
loop-head collateral is multiset-identical to the target's and therefore, unlike s17's Q00
basin, a topologically valid perturb input. s19 re-measured the flip on today's chassis
(score 29 / 175 / 175, confirmed) and row-diffed it: under the flip, rows 60–64 carry **five**
differing rows (`lw $v1` vs `lw $v0`, `sll $v0,$v0,1` vs `sll $v1,$v1,1`, plus 62/63/64 still
wrong) where the floor-5 body carries **three** and matches rows 60 and 61 exactly. A
scheduling repair reorders emissions; it does not undo the flip's seat swap. So the flip
basin's arithmetic ceiling, with its entire 24-row loop-head collateral perfectly repaired, is
class B (2) + class C region (5) = **7** — strictly WORSE than the standing floor of 5. The
solver run is therefore incapable of dropping the floor and is not worth spending; the item is
closed, not deferred.

**Exhaustion evidence (pointers, not assertions).** 19 sessions; ≥5 distinct modalities
(escalation, structural, synthesis, solver, forensics, permuter); **88** banked rejected forms
in `memory/grind/func_800770B8/rejected/`; two telemetered permuter campaigns totalling
**33,926 iterations** / 9 finds, best permuter score 35, none matching (evidence.md lines 502,
504); and ~1,900 enumerated sandbox builds across the swept axes — flip spellings (11 collapse
onto one build), store-group order (24/24), loop shape (315), inner-block position (240), wrap
position/count (63 + 18 + 79×3), loop-head demand order (33), dependence direction (13),
reference count (8), pointer-cursor derivation (8), partial array typing (20), procedural
factoring (8), local declaration order (120), qty3 birth delay (5), and a 1,435-function corpus
census proving the class-B 2+2 split unique to this function. All three solver backends have
been run with FULL-disposition goals: `inverse.py global` NEGATIVE at depth 2 and depth 3 for
both `{"75":2,"110":3}` and `{"110":3}`; `inverse.py local` REACHABLE in exactly one family
(`live_shrink qty3 born later`) which five C spellings kill on bytes; `perturb.py` with the
object-level goal reports **no differing block in sched1 OR sched2**, align
{equal 170, replace 5, delete 0, insert 0, moved 0}.

**Both gates FAIL — the common case.** Applying the standing ruling: FORECLOSED silently. The
function is not completable in pure C with any construct currently available to this pipeline,
and the only construct that would close it is already Judge-FAILED.

**Re-activation triggers.** (i) an owner grant that extends the frozen family list to cover an
arithmetic-identity / dependence-edge construct on a live computation (the 2026-09-01 07:23
FAIL would then be revisitable); (ii) a toolchain finding that changes `local-alloc.c`
`block_alloc` operand-tying behaviour, or any chassis change that gives the body instruction
slack (today 175 == 175); (iii) a canonical-asm class grant covering ordinary-compiler-output
functions, which the LOW scanner tier currently denies; (iv) an owner unpark.

**Evidence pointers.** `memory/grind/func_800770B8/evidence.md` and `hypotheses.md` (s1–s19),
`candidate.c` (the floor-5 body, cheat-free, one annotated single-level do-while(0) wrap),
`rejected/` (88 forms), `tmp/grind/func_800770B8/s19/` (this session's row-diff tool and logs),
`tmp/grind/func_800770B8/s18/` (lreg/greg dumps, solver logs).

## 2026-09-01 08:41 — func_800770B8 — DISCARDED-SESSION MARKER (driver-stamped)

Text appended above by session s19 of func_800770B8, which the driver DISCARDED as invalid (owner-gated claim rejected: no OWNER-ESCALATION / CANONICAL-ASM GRANT PATH entry in docs/grind/decisions.md names func_800770B8). It is not a ruling and carries no standing; terminal-sounding language in that span is void.

## 2026-09-01 — func_800770B8 — OWNER-ESCALATION record — **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**

Proof-of-foreclosure record for `func_800770B8` (src/text1b.c), filed under the owner's standing
auto-ruling of 2026-07-27 (`.claude/rules/endgame-lock-disposition.md`) and RECORDED, not asked,
per the 2026-08-31 ruling (`.claude/rules/ordinary-c-judge-decidable.md`). Nothing here is
addressed to the owner and nothing waits on a reply. This entry supersedes the identically-titled
span written by the session the driver discarded at 2026-09-01 08:41 (that span carries no
standing; its header line omitted the token the driver's validator matches). Every number below
was re-measured or re-run by THIS session.

**Chassis, measured this session.** `memory/grind/func_800770B8/candidate.c` applied to
src/text1b.c together with the two byte-neutral caller-side edits it documents (prototype
`s32 func_800770B8(s32, s32, s32);`, call site `(s32)&D_8009BD24`), scored against a reference
`build/src/text1b.o` regenerated from PRISTINE main source: `sandbox func_800770B8 --disable all`
= **score 5, build_insns 175, target_insns 175, rules_dropped 0**. The floor has been flat at 5
since s11 — nine consecutive sessions across the modalities escalation, structural, synthesis,
solver and forensics.

**What the residual is.** The floor-5 body is the target modulo FIVE in-place register names, with
zero instruction slack (175 == 175): rows 35, 36 (class B — two stores go through the `p_old` copy
`$s1` instead of the raw `func_8006E49C` result `$v0`) and rows 62, 63, 64 (class C —
`addu $v0,$v0,$v1` / `addiu $a3,$v0,0x6A` / `addiu $a1,$v0,0x7E` against the target's
`addu $v1,$v1,$v0` / `addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E`). Every other line a raw diff
prints is an objdump alias (`move` vs `addu ...,$zero`, `li` vs `addiu ...,$zero`).

**Gate (a) — canonical-asm evidence: FAILED.** `python3 tools/scan_hand_coded.py --single
func_800770B8` (re-run this session, log at `tmp/grind/func_800770B8/s19b/scan.log`) returns
`tier=LOW score=0/8`, "no strong hand-coded indicators", with every one of S1–S8 unset: 0
multu/mflo pairs, no empty-body branches, 5 spills over 175 insns and 14 distinct registers, max
load burst 3 in any 8-insn window, no high-similarity sibling (jaccard < 0.5), no BIOS jumptable
call pattern, all callee-save uses paired with an `$sp` save, no redundant mask-before-shift. This
is ordinary GCC 2.7.2 output; the canonical-asm grant path is not available.

**Gate (b) — in-hand SOTN-master precedent: FAILED, and vacuous.** Census re-run this session over
`docs/reference/sotn-construct-index.md` (1,056 lines): ZERO matches for
`combine_regs|local-alloc|reg_qty|operand[ -]order|register seat|swap operand` and ZERO for
`identity|arithmetic detour|redundant read`. A negative census is a FAILED gate, not an open
question. It is additionally vacuous because there is no closing construct in hand to seek
precedent for:

- class C is decided by `local-alloc.c` `block_alloc`'s operand-tying loop
  (tools/gcc-2.7.2/local-alloc.c:1240-1298), which ties the add's destination to the first operand
  for which `combine_regs` succeeds. s18 enumerated and typed all ten `combine_regs` gates
  (local-alloc.c:1784-1946): six are structurally impossible for two SImode pseudos in a plain
  `addsi3`, one is unreachable on MIPS, one detaches the dest from both operands, and the only two
  C-reachable gates require the `D_800A36A0` reload to stay live past the add. Gate 1 was built
  and priced at 174 insns / score 49
  (`rejected/s18fx-classC-blocklocal-reload-defeats-op1-tie-174insn-score49.c`).
- THIS session closed the one narrow question s18 left open — whether an ALREADY-PRESENT
  instruction (rather than an added one) can be the extra consumer that keeps the reload live.
  Three fresh builds say no: deriving both inner cursors from the loop-top `base` local lets
  cse/flow DELETE the reload outright (score 41, 174 insns —
  `rejected/s19-classC-base-reuse-deletes-reload-174insn-score41.c`), while deriving `p_7e` from
  `p_6a` and routing both cursors through a shared `pb = D_800A36A0 + t0*10` each collapse onto the
  byte-identical floor-5 body (score 5, 175 insns). A consumer that folds into the same `addiu` is
  not a separate live use, so gate 2's REG_DEAD requirement (local-alloc.c:1917) is untouched.
- class B is foreclosed by price four times over (s7/s8/s9/s14): every spelling that reaches the
  raw result pseudo lets flow.c delete the copy and its four dependents, collapsing the body to
  170 insns — five FEWER than the target's 175.
- the alternative (flipped operand order) basin is arithmetically capped ABOVE the standing floor:
  s19 re-measured the plain flip at score 29 / 175 / 175 and row-diffed it — its class-C region
  carries five differing rows where the floor-5 body carries three, and a scheduling perturbation
  reorders emissions without undoing a seat swap, so the flip's ceiling with all 24 collateral rows
  perfectly repaired is 2 + 5 = 7 > 5. The sched_solver `perturb.py` run on that basin therefore
  cannot drop the floor under any outcome and is closed, not deferred.
- the ONLY construct ever measured to repair the flipped basin was the arithmetic identity detour
  `(t0 * 4) >> 1`, which the Judge **FAILED** on 2026-09-01 (docs/grind/decisions.md, 07:23 entry)
  as a byte-materialising chain-extender outside the frozen family list. Under the 2026-08-24 /
  2026-08-31 auto-reject class that is a clean FAIL, not an open question.

**Exhaustion evidence (pointers, not assertions).** 19 sessions; >=5 distinct modalities
(escalation, structural, synthesis, solver, forensics, permuter); **89** banked rejected forms in
`memory/grind/func_800770B8/rejected/`; two telemetered permuter campaigns totalling **33,926
iterations** / 9 finds, best permuter score 35, none matching (evidence.md lines 502, 504); and
~1,900 enumerated sandbox builds across the swept axes — flip spellings (11 collapse onto one
build), store-group order (24/24), loop shape (315), inner-block position (240), wrap
position/count (63 + 18 + 79x3), loop-head demand order (33), dependence direction (13), reference
count (8), pointer-cursor derivation (8 + 3 this session), partial array typing (20), procedural
factoring (8), local declaration order (120), qty3 birth delay (5), and a 1,435-function corpus
census proving the class-B 2+2 split unique to this function. All three solver backends were run
with FULL-disposition goals: `inverse.py global` NEGATIVE at depth 2 and depth 3 for both
`{"75":2,"110":3}` and `{"110":3}`; `inverse.py local` REACHABLE in exactly one family
(`live_shrink qty3 born later`) which five C spellings kill on bytes; `perturb.py` with the
object-level goal reports no differing block in sched1 OR sched2, align
{equal 170, replace 5, delete 0, insert 0, moved 0}.

**Both gates FAIL — the common case. Applying the standing ruling: FORECLOSED silently.** The
function is not completable in pure C with any construct currently available to this pipeline, and
the only construct that would close it is already Judge-FAILED.

**Re-activation triggers.** (i) an owner grant extending the frozen family list to cover an
arithmetic-identity / dependence-edge construct on a live computation (the 2026-09-01 07:23 FAIL
would then be revisitable); (ii) a toolchain finding that changes `local-alloc.c` `block_alloc`
operand-tying behaviour, or any chassis change giving the body instruction slack (today 175 ==
175); (iii) a canonical-asm class grant covering ordinary-compiler-output functions, which the LOW
scanner tier currently denies; (iv) an owner unpark.

**Evidence pointers.** `memory/grind/func_800770B8/evidence.md` and `hypotheses.md` (s1-s19b),
`candidate.c` (the floor-5 body, cheat-free, one annotated single-level do-while(0) wrap),
`rejected/` (89 forms), `tmp/grind/func_800770B8/s19b/` (this session's apply/gen scripts, scan
log, pristine-reference recovery), `tmp/grind/func_800770B8/s19/` (row-diff tool and logs),
`tmp/grind/func_800770B8/s18/` (lreg/greg dumps, solver logs).

## 2026-09-01 — func_800770B8 — **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**

Proof-of-foreclosure record for `func_800770B8` (src/text1b.c), re-filed after the owner's
2026-09-01 FORECLOSED-BUCKET REVIEW returned the function to the active lane under Ruling A
with a NAMED PROBE. This session (s20, escalation modality) EXECUTED that probe in full — 36
fresh builds — and it did not drop the floor. Filed under the standing auto-ruling of
2026-07-27 (`.claude/rules/endgame-lock-disposition.md`) and RECORDED, not asked, per the
2026-08-31 ruling (`.claude/rules/ordinary-c-judge-decidable.md`): nothing here is addressed
to the owner and nothing waits on a reply.

**Chassis, re-measured this session on a pristine reference.** `engine build-c text1b` from
untouched main source regenerated `build/src/text1b.o` (sha1
55a39c6dbcf00ec4feda82df16f9325319e1e5a8) BEFORE any probe, so no candidate object could
contaminate the scorer. `memory/grind/func_800770B8/candidate.c` applied to src/text1b.c with
the two byte-neutral caller-side edits it documents (prototype `s32 func_800770B8(s32, s32,
s32);`, call site `(s32)&D_8009BD24`): `sandbox func_800770B8 --disable all` = **score 5,
build_insns 175, target_insns 175, rules_dropped 0**. Flat at 5 since s11 — ten consecutive
sessions across the modalities escalation, structural, synthesis, solver and forensics.

**The residual.** Five in-place register names, no skeleton difference (s18: goal_from_tgt.py
classify = 5 renamed pairs, 0 skeleton-differing; sched_solver align honobj->tgtobj = {equal
170, replace 5, delete 0, insert 0, moved 0}). Rows 35-36 (class B: two stores go through the
`p_old` copy `$s1` instead of the raw `func_8006E49C` result `$v0`) and rows 62-64 (class C:
`addu $v0,$v0,$v1` / `addiu $a3,$v0,0x6A` / `addiu $a1,$v0,0x7E` against the target's
`addu $v1,$v1,$v0` / ...).

**Ruling A's reopen ground, answered by measurement.** The review reopened this item because
both class closures were PRICE arguments resting on "zero insn slack (175 == 175)" while the
ledger held measured 173/174-insn spellings, and the fence-device x slack-spelling
cross-product had never been built. It is built now
(`tmp/grind/func_800770B8/s20/gen.py`, runner `s20/run.sh`, log `s20/sweep.log`): bases {F =
floor candidate, C = s12 flipped-CABD `perm/Q12.c`} x devices {none, s17 C1 duplicated-arm
join label, s17 B1 post-copy branch} x slack {none, D = s13 do-while third loop (-1 insn),
G = s18fx gate-1 block-local reload (-1), E = D+G (-2), P = s13 pointer walk (-2), Q = P+D
(-3)} = 36 builds.

1. **The "zero slack" sentence is FALSE as a constructibility claim, and the closure survives
   anyway.** Insn counts compose exactly additively, so eight builds this session are 175-insn
   bodies carrying a surviving CODE_LABEL — the object s17 said could not exist: FPb 175/43,
   FPc 175/51, FEb 175/56, FEc 175/65, CPb 175/52, CPc 175/60, CEb 175/47, CEc 175/56.
2. **SCORE is additive on the same axes, and that is the real lock.** Measured deltas on the
   floor base: device B1 = +18 rows, device C1 = +25; slack D = +1, P = +21, G = +36, E = +37,
   Q = +22. Combinations land within 2 of the sum of their parts (FDc = 31 = 5+25+1; FPc = 51
   = 5+25+21; FPb = 43 = 5+18+21). The cheapest -1-insn slack costs +1 row but the cheapest
   -2-insn slack costs +21, and the DEVICE's own collateral (+18 minimum) already exceeds by
   nine times the TWO rows class B is worth. Best 175-insn label build = 43 vs a floor of 5;
   even a hypothetically free -2 slack leaves it at 23. Class B is foreclosed by the device's
   price, independently of the instruction budget — the correction the review asked for
   strengthens the closure instead of reopening it.
3. **The flip ceiling recomputed over the CABD basin is 10, worse than the 7 the review
   suspected was mis-basined.** All twelve C-base builds are >= 12 (C00 12, CD0 13, CG0 32,
   CE0 33, CP0 35, CQ0 36, device builds 30..61); with s12's exhaustive 79-position
   second-wrap sweep on that same base (minimum 10 at W039/W040, at the price of a SECOND
   FAKE-annotated wrap) the basin bottoms out at 10 — five worse than the floor. The only
   sub-5 number ever recorded on any flipped basin is s14's P8 = 4/175, which IS the
   `(t0 * 4) >> 1` arithmetic-identity detour the Judge FAILED on 2026-09-01 (decisions.md
   07:23 entry) and which remains banned; no build in this sweep uses it or any respelling of
   it.

**Gate (a) — canonical-asm evidence: FAILED (re-run this session).**
`python3 tools/scan_hand_coded.py --single func_800770B8`
(`tmp/grind/func_800770B8/s20/scan.log`) returns `tier=LOW score=0/8`, "no strong hand-coded
indicators", every one of S1-S8 unset: 0 multu/mflo pairs, no empty-body branches, 175 insns
with 5 spills over 14 distinct registers, max load burst 3 in any 8-insn window, no
high-similarity sibling, no BIOS jumptable call pattern, all callee-save uses paired with an
`$sp` save, no redundant mask-before-shift. Ordinary GCC 2.7.2 output; the canonical-asm grant
path is not available.

**Gate (b) — in-hand SOTN-master precedent: FAILED, against the UNCAPPED index.** The review's
Correction 2 (precedent censuses ran against a cap-truncated derived artifact) is answered
directly: `docs/reference/sotn-construct-index.md` has since been rebuilt uncapped — 2,746
lines, commit `aa53500226ee84be763f3e8702b27de06456b3a7`, generated 2026-09-01, against the
1,056-line artifact s19 used. Re-censused on the new index: ZERO hits for
`combine_regs|local-alloc|reg_qty|operand[ -]order|register seat|swap operand`. The gate is
additionally vacuous, because after this session's sweep there is still no closing construct
in hand to seek precedent FOR: the only constructs ever measured to emit either target row
group are the two label devices (>= +18 rows of collateral, measured above) and the banned
identity detour. The 958-hit `dup_if_else_arm` class in the rebuilt index does not rescue this
residual — the C1 device IS a duplicated-arm form and it is dead on PRICE (+25 rows), not on
precedent.

**Exhaustion.** Twenty sessions; modalities escalation, structural, synthesis, solver,
forensics, permuter (two telemetered campaigns totalling 33,926 iterations, both fresh-seed
exhausted); 93 banked rejected forms in `memory/grind/func_800770B8/rejected/`; the mechanism
of each residual class named at compiler-source level (class B: cse.c:8038-8063 EBB
termination + jump.c label demotion; class C: local-alloc.c:1240-1298 block_alloc operand
tying, all ten combine_regs gates at local-alloc.c:1784-1946 typed in s18); global allocation
NEGATIVE at depth 3 on the full goal; sched1 and sched2 report no differing block against the
target object.

**Evidence pointers.** `memory/grind/func_800770B8/evidence.md` and `hypotheses.md` (s1-s20,
[s20] sections appended this session), `candidate.c` (the floor-5 form), `rejected/` (93
forms, four added this session including the first 175-insn surviving-label builds),
`tmp/grind/func_800770B8/s20/` (gen.py, run.sh, apply.py, v/ = the 36 bodies, sweep.log,
scan.log), `tmp/grind/func_800770B8/s19b/`, `s19/`, `s18/`.

**Re-activation triggers.** (i) an owner class grant covering an arithmetic-identity /
dependence-edge construct on a live computation (the banned P8 shape measures 4/175 today);
(ii) an owner grant covering a label device whose collateral is not paid down — i.e. accepting
a 43-row-worse body is not the question; the question would be a grant for a construct that
splits a cse extended basic block WITHOUT a CODE_LABEL, which GCC 2.7.2 does not offer today;
(iii) a toolchain finding that alters block_alloc's operand tying or cse2's EBB termination;
(iv) an owner unpark.

## 2026-09-05 - func_800770B8 - **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**

Proof-of-foreclosure record for `func_800770B8` (src/text1b.c), filed by session s30 (escalation
modality) under the owner's standing auto-ruling of 2026-07-27
(`.claude/rules/endgame-lock-disposition.md`) and RECORDED, not asked, per the 2026-08-31 ruling
(`.claude/rules/ordinary-c-judge-decidable.md`). Nothing here is addressed to the owner and
nothing waits on a reply. This is the second filing after the owner's 2026-09-02 re-activation
with the exhaustion window RESET; the nine sessions since (s21-s29, modalities structural,
synthesis, solver, forensics, object-model) plus this one worked the ladder from the 09-01 named
probe forward and did not move the floor.

**Chassis, measured this session.** `memory/grind/func_800770B8/candidate.c` applied to
src/text1b.c with the two byte-neutral caller-side edits it documents (prototype
`s32 func_800770B8(s32, s32, s32);`, call site `(s32)&D_8009BD24`):
`sandbox func_800770B8 --disable all` = **score 5, build_insns 175, target_insns 175,
rules_dropped 0** on HEAD 6c9ca9fa. The floor has been flat at 5 since s11 - twenty consecutive
sessions - across the modalities escalation, structural, synthesis, solver, forensics,
object-model and permuter. src/text1b.c was restored to its pristine HEAD copy after every build
this session (cmp-verified) and the working tree carries only ledger files.

**The residual, and what s30 newly established about it.** The floor body is the target modulo
FIVE in-place register names: rows 35/36 (class B - the two prologue clear stores addressed
through the `p_old` copy `$s1` instead of the freshly returned `func_8006E49C` result `$v0`) and
rows 62/63/64 (class C - `addu $v0,$v0,$v1` / two inheriting `addiu` against the target's
`addu $v1,$v1,$v0`). 175 == 175, no instruction slack.

This session transcribed the target's own store window (asm/funcs/func_800770B8.s rows 40-64)
directly rather than inferring it from row diffs, which had never been done, and it changes the
shape of the class-C argument in three ways. (i) The target's store-group order is A, D, C, S with
the `sb` emitted LAST, so h3's five extra rows really are only its S-before-C order. (ii) The
target emits the three address computations as a CLUSTER (`addu $v0,$a0,$v1` C pointer,
`addu $a0,$a0,$a1` S pointer, `addu $v1,$v1,$a1` chain root) and only then the two `sh` and the
`sb` - the S pointer is live across the C stores and reuses base's register. (iii) The cursor is
GCC's synth_mult for 10 (`t0*4 + t0`, then `<<1`) reusing the same t0*4 pseudo the D and C
pointers use, which our C spelling `t0 * 10` already reproduces byte-exactly.

Against that transcription, the flipped-cursor body with the C group on its own pointer local and
the target's A,D,C,S store order (`o1`, score 25) turns out to emit **the target's exact
instruction order**: rows 55 and 59 are already byte-exact and all 25 differing rows are one
local-alloc register seat. The seat is decided by qty_compare_1 (local-alloc.c:1660-1683):
chain [28,56] 22 refs = 3.1428 beats the C pointer [36,40] 6 refs = 3.0, so the chain takes $2.

**The residual is a two-horned dilemma and both horns were measured this session.** Exactly two
mechanisms give the flipped cursor the target's class-C seat, and each costs more than the three
rows class C is worth:
* HORN 1 - a short high-priority blocker in [28,48). The only real quantity that qualifies is the
  S store's address temp (4 refs / span 2 / priority 4.0), and it only becomes a block-local
  quantity when the S store is EMITTED BEFORE the C stores. That is `h3`: **score 7** - the three
  class-C rows bought for five rows of wrong store order.
* HORN 2 - lengthen the chain's interval so the C pointer's 3.0 outranks it. This needs the t0*4
  shift to float to the top of the block ([12,56], span 44, priority 2.0), which happens exactly
  when the D group stops sharing group A's `ptr` pseudo. `p1` (A on its own local) and `q1` (D on
  its own local), both in the target's A,D,C,S order, DO win the target's seats - and score 28,
  because the same freeing that floats the shift floats the `%hi/%lo(D_800A35D0)` pair (the s28/e6
  sched.c dependency mechanism), dragging the store window and renaming the inner-loop counter.
  The D-first orders (`o2`/`p2`/`q2`) win the seat for 14, of which 11 rows are purely the D group
  being emitted before group A.
The target has the shift floated (row 42, filling the `lw`'s delay slot) WITHOUT the symbol pair
floated (rows 49-51 sit immediately before the D stores). No C form measured in thirty sessions
separates the two.

**Statement position of a pure address computation is byte-inert on this chassis - 13 spellings,
all 25/175, two of them dump-verified.** Naming the S pointer and placing its assignment at five
different points relative to the C group (y1-y5); spelling the C pointer shift-first (y6, s1);
swapping the two C stores (t1); spelling the S address base-cast-first (u1); naming the shared
`t0*4` shift as an `s32` local at the top of the loop body or just before group D (w1, w2); naming
`t0*10` (w5); naming both (w4); splitting the C pair onto two once-used pointer locals so each
would be a 4-ref/span-2 quantity (x1 - cse.c refolds them onto one pseudo); and hoisting the
C-pointer assignment above group A (v1) or between groups A and D (v2, v4) - every one is 25/175,
and the instrumented-cc1 (BB2_QTY_DEBUG) block-1 quantity tables for w1 and y2 are LINE-FOR-LINE
IDENTICAL to o1's, so the inertness is at the RTL level and not a scoring coincidence. What moves
bytes in this window is exactly two things: which LOCAL each store group uses, and the ORDER of the
store groups. Both were swept exhaustively this session (six store orders x four local
assignments, 33 builds total, all 175/175).

**Gate (a) - canonical-asm evidence: FAILED (re-run this session).**
`python3 tools/scan_hand_coded.py --single func_800770B8`
(`tmp/grind/func_800770B8/s30/scan.log`) returns `tier=LOW score=0/8`, "no strong hand-coded
indicators", every one of S1-S8 unset: 0 multu/mflo pairs, no empty-body branches, 175 insns with
5 spills over 14 distinct registers, max load burst 3 in any 8-insn window, no high-similarity
sibling, no BIOS jumptable call pattern, all callee-save uses paired with an `$sp` save, no
redundant mask-before-shift. Ordinary GCC 2.7.2 output; the canonical-asm grant path is not
available.

**Gate (b) - in-hand SOTN-master precedent: FAILED, and vacuous.** Re-censused against the uncapped
2,746-line `docs/reference/sotn-construct-index.md`: ZERO hits for
`local-alloc|reg_qty|qty_compare|register seat|swap operand|operand[ -]order|combine_regs`. The
gate is additionally vacuous because there is no closing construct to seek precedent FOR: every
construct measured this session and in s21-s29 is ORDINARY C (store reordering, pointer locals,
named intermediates, corrected global declarations). No coercion, no FAKE beyond the single
pre-existing empty `do { } while (0);` prologue fence, and nothing that would need a family grant.
The residual is blocked by measurement, not by policy. The one construct that ever measured below
the floor on a flipped basin (s14's P8 = 4/175) is the `(t0 * 4) >> 1` arithmetic-identity detour
the Judge FAILED on 2026-09-01 (decisions.md 07:23 entry); it remains banned and no form filed here
uses it or any respelling of it.

**Exhaustion.** Thirty sessions; modalities escalation, structural, synthesis, solver, forensics,
object-model, rederive and permuter (two telemetered campaigns totalling 33,926 iterations, both
fresh-seed exhausted); 135 banked rejected forms in `memory/grind/func_800770B8/rejected/` (seven
added this session); the mechanism of each residual class named at compiler-source level (class B:
cse.c:8038-8063 EBB termination + jump.c label demotion, and the s20 measurement that the cheapest
label device costs +18 rows against a 2-row prize; class C: the operand-1 tie at
local-alloc.c:1295 and the seat decision at local-alloc.c:1660-1683, both read out of the
instrumented compiler's own quantity table rather than modelled); the object model of every global
the function touches audited and corrected byte-neutrally in s29; global allocation NEGATIVE at
depth 3 on the full goal; sched1 and sched2 reporting no differing block against the target object.

**Evidence pointers.** `memory/grind/func_800770B8/evidence.md` and `hypotheses.md` ([s30] sections
appended this session, H-s30-1 .. H-s30-7), `candidate.c` (the floor-5 body) and
`candidate_objmodel_ALL2.c` + `s29-objmodel-ALL2-declaration-edits.patch`, `rejected/` (135 forms),
`tmp/grind/func_800770B8/s30/` (gen30.py / gen30b.py / gen30c.py / gen30d.py, apply.py, run.sh,
qty.sh, qtydbg.py, rd.sh, v/ = the 33 bodies, sweep.log, scan.log, o1.qty / o2.qty / p1.qty /
q1.qty / w1.qty / y2.qty), and `tmp/grind/func_800770B8/s29/`, `s28/`, `s20/`, `s19b/`, `s19/`,
`s18/`.

**Re-activation triggers.** (i) A toolchain or compiler-source finding that separates sched1's
float of the t0*4 shift from its float of the `%hi/%lo(D_800A35D0)` pair - that single separation
turns q1/p1 (28) into a score-2 body and, with class B, into the match; (ii) an owner class grant
covering an arithmetic-identity / dependence-edge construct on a live computation (the banned P8
shape measures 4/175 today); (iii) a class grant covering a construct that splits a cse extended
basic block WITHOUT a CODE_LABEL, which GCC 2.7.2 does not offer today (the class-B horn);
(iv) an owner unpark.

### Not an integration handoff

Nothing is bytes-proven and nothing is blocked by an untouchable surface. `src/text1b.c` carries
`INCLUDE_ASM("asm/funcs", func_800770B8);` with 0 regfix / 0 asmfix rules; the best honest form is
5 masked points away and lives in `memory/grind/func_800770B8/candidate.c`. The working tree was
restored to HEAD before this record was filed.

## 2026-09-06 20:18 — func_800770B8 — final call — **PASS**

Three FAKE units, all inside frozen-list families with prerequisites met. (1) empty do{}while(0) prologue fence -> do-while-zero-exception (owner 2026-07-06: any body incl. empty, ANY codegen effect, single level, inline annotation present; nesting clause N/A). (2) p_old=prev and (3) sym=(u8*)&D_800A35D0 -> dead-store-fake-exception (owner 2026-07-01): both are same-value stores to LOCALS whose stored value is never read, each annotated with what / named GCC pass (local-alloc.c:472 reg_n_deaths==1; loop.c:3040-3041 may_not_move + loop.c:649) / lever-exhaustion pointer. No dead store to a global, no pin, no __asm__, no volatile or alias coercion, no semantic lie: sym holds the true &D_800A35D0 and is really consumed by dp = sym + t0*4. Decisive fact: exhaustion is real, not asserted - 38 prior sessions in state.json floor_history, 191 banked rejected/ forms, and s39's own controls show the construct is the mechanism rather than a lucky spelling (g1d 18, g6 18, h1/h2/h3 41/54/51 all with insn-count changes, g1a 2 / g1b 5 / g1c 7 ablations). Independently verified: git status/diff shows the only build-file change is src/text1b.c (no pipeline .txt, Makefile, rule/config or prebuilt-.o surface - no build-time output rewriting of any kind); the applied src body is byte-identical to candidate.c; state.json banned_constructs is empty and the sole judge_constraint (no t0*4->t0*2 identity detour) is not touched by this body; both cited precedent lines verified in docs/reference/sotn-construct-index.md:65 (src/dra/66590.c:390 dest = val1; // fake) and :74 (src/main/psxsdk/libc/sprintf.c:129 } while (0); // FAKE), both untagged PSX entries. Caller-side prototype correction (s32 return, scalar arg1) is an ordinary declaration fix the bytes demand. Full evidence: memory/grind/func_800770B8/evidence.md [s39] sweep table, hypotheses.md classes A/B/C, self_vet.md, rejected/.

## 2026-09-08 — func_800204C0 (src/code6cac.c) — **CANONICAL-ASM GRANT PATH: blocked at the operator registry row (bytes PROVEN, Judge PASS on record, LOW scan tier)**

Filed by grind session s1b (recon modality, re-dispatch after the 387fa8f8 merge refusal). **This is
NOT an exhaustion claim, NOT an endgame lock, and NOT a question to the owner.** It is a
proof-of-foreclosure record of the integration-handoff kind, identical in shape to the func_80019310
entry above (2026-09-06, decisions.md:24517): the function is SOLVED (pure-C body + four PsyQ SDK GTE
macro islands, sandbox 0), the Judge has already PASSed the exact body, and the ONLY missing piece is
one line in a file no grind session may write.

### State of proof (all re-measured this session on HEAD 387fa8f8)

- Body: `memory/grind/func_800204C0/candidate.c`, Judge hash `8655cc28f3aa5cc7`, Judge PASS
  2026-09-08 06:15 (`state.json` review_ledger; docs/grind/decisions.md 2026-09-08 ruling).
- `sandbox func_800204C0 --disable all` = score 0, 122/122, rules_dropped 0
  (`tmp/grind/func_800204C0/s1/sandbox_s1b.json`). `canonical` = ASM-PARTIAL, 11/122 cop2 insns.
  Zero pins, zero aliasing blocks, zero scheduling barriers, zero `/* FAKE */`; the C body is
  ordinary (field reads through `u8 *arg0`, a `+= 1` counter with re-read, signed `/ 150` and
  `/ 0x1000`, a `(s16)` compare, a double tail store).
- `tools/scan_hand_coded.py --single func_800204C0` = tier LOW 1/8 (S4 front loads only; none of
  the STRONG signals S1/S2/S6), the GTE-wrapper-misroute artifact the 2026-09-01 grant record itself
  names (`tmp/grind/func_800204C0/s1/scan_hand_coded.txt`).
- The refusal's other branch, "the islands are C-expressible (respell them in C)", is measured DEAD on
  this chassis: gte_SetRotMatrix respelled as five C word loads plus ctc2-only islands reproduces 0 of
  the 12 island-region insns — GCC 2.7.2 seats the loads in $t0/$a0/$v0/$a0/$v1 as a block ahead of
  each transfer pair and never emits the redundant `move $t4,$v1` preamble
  (`tmp/grind/func_800204C0/s1/thin1_try.txt`, hypotheses.md H5,
  `rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c`). Same class result as func_80019310 s3
  H10 and func_800203B4 s6.

### Why the driver refused the merge, and why no session can cure it

`grant_canonical_asm` (tools/grinder/grindlib.py:1493) has two evidence doors: STRONG scan tier, or a
row naming the function in `tools/grinder/owner_cluster_grants.txt`. The tier is LOW and the registry
has no `func_800204C0` row (rows exist for func_80031890, func_8002FF20 and func_80019310 under the
same grant). The registry header states it is OPERATOR-MAINTAINED ONLY and `tools/` is outside session
scope. The owner grant that admits this function already exists: the 2026-09-01 widened-anchor GRANT
names `func_800204C0` as a confirmed handwritten-tagged carrier (decisions.md:17959;
.claude/rules/cop2-addressing-preamble-cluster.md:155). The island spelling is character-identical to
the integrated func_800203B4 body (inline_asm_canonical.txt:367). Nothing is being asked of the owner;
the ruling is landed. The missing surface is purely clerical.

### The exact operator step (the whole remedy)

1. Append ONE row to `tools/grinder/owner_cluster_grants.txt`, same shape as the func_80019310 row:
   `func_800204C0 cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:17921; row per owner ruling 2026-09-02)`
2. Re-activate the function (`queue unpark func_800204C0 --reason "registry row added"`) if the driver
   foreclosed it on this entry.
3. The next session submits `candidate.c` EXACTLY (the Judge clearance skips layer-1); the driver
   re-proves bytes, runs FINAL CALL, and its grant door writes the `inline_asm_canonical.txt` line
   itself (tier "OWNER-CLUSTER"). Honest bucket: COMPLETED-INLINE-ASM-CANONICAL. The operator's fresh
   layer-2 cheat-reviewer still runs on the C before acceptance.

### Routing note

As with func_80019310, the session cites this entry in `escalation_ref` WITHOUT the grant-path branch
keyword so the driver takes its borderline-log + silent-foreclosure branch (grind.ps1:1474) rather than
re-dispatching sessions that cannot act until the registry row exists. Re-activation triggers: the
registry row above, OR a future rule/toolchain change that lets a session write the row itself.
Evidence pointers: `memory/grind/func_800204C0/evidence.md` (s1 items 1-10, s1b items 11-16),
`hypotheses.md` (H1-H6), `rejected/` (two disproven forms), `tmp/grind/func_800204C0/s1/`.

## 2026-09-08 01:23 — func_800204C0 — ruling: INTEGRATION HANDOFF filed for func_800204C0 : docs/grind/decisions.md:25395 — 20 — **ESCALATE**

RULING REQUEST on the integration handoff filed at docs/grind/decisions.md:25395. The claim holds; the work is sound and complete; the only blocker is a registry file no grind session may write, and it is also outside the driver's own scope-grant classes.

WHAT WAS BUILT. A 97-instruction pure-C body (field reads through a byte pointer, a counter incremented in memory and re-read, signed divisions by 150 and 4096, a 16-bit compare, a double tail store) plus four GTE inline-asm islands that are the PsyQ SDK macro bodies character-for-character (gte_SetRotMatrix, gte_ldlv0 with its lhu/lhu/sll/or pack, the MVMVA command word 0x4A486012, gte_stlvnl), spelled identically to the already-integrated func_800203B4 (inline_asm_canonical.txt:367). No pins, no scheduling barriers, no free-standing GPR asm outside an island, no /* FAKE */ constructs, no build-time output rewriting (only src/code6cac.c would change).

WHAT I VERIFIED MYSELF (not taken from the ledger). (1) grindlib body_hash of candidate.c = 8655cc28f3aa5cc7, the body already PASSed at FINAL CALL (state.json review_ledger 2026-09-08 06:15); nothing in the C changed. (2) Applied candidate.c to src/code6cac.c transiently: sandbox func_800204C0 --disable all = score 0, 122/122, rules_dropped 0; canonical = ASM-PARTIAL 11/122 cop2; src restored to INCLUDE_ASM afterwards, tree clean. (3) candidate.c differs from the committed TU only in the function region (diff of the file minus that region is empty). (4) Target asm carries the grant's defining evidence: three materialize-then-copy preambles (addu $t4,$v1/$v0/$v0 at asm/funcs/func_800204C0.s:25,37,59) and 8 splat handwritten-instruction tags on the cop2 transfers. (5) scan_hand_coded tier LOW 1/8 (S4 only), which the 2026-09-01 grant record itself names as the GTE-wrapper-misroute artifact. (6) The 'islands are C-expressible' branch of the 387fa8f8 refusal is measured dead: the C-loads respelling reproduces 0 of 12 island-region instructions (GCC 2.7.2 never emits the redundant $t4 copy) — rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c, same class as func_80019310 s3 and func_800203B4 s6.

FAMILY. The cop2 addressing-preamble cluster, WIDENED-ANCHOR owner GRANT of 2026-09-01: func_800204C0 is named by name as a confirmed handwritten-tagged carrier (decisions.md:17959; .claude/rules/cop2-addressing-preamble-cluster.md:155). Its load-bearing prerequisite — the pure-C body independently at sandbox 0 — holds (verified above). Condition 3 (template = SDK macro body) holds per owner ruling 2026-09-02.

THE BLOCKER AND THE REMEDY. grant_canonical_asm (tools/grinder/grindlib.py:1493) opens on a STRONG scan tier or a row naming the function in tools/grinder/owner_cluster_grants.txt. The tier is LOW and the registry has rows for func_80031890, func_8002FF20 and func_80019310 under this same grant, but none for func_800204C0. The registry header says OPERATOR-MAINTAINED ONLY. The whole remedy is one line, same shape as the func_80019310 row: `func_800204C0 cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:17921; row per owner ruling 2026-09-02)`. I am setting scope_paths to that file so the record names the exact surface. I expect the driver to REFUSE it mechanically (tools/ is outside _SCOPE_GRANT_ALLOWED_RE), borderline-log this packet and foreclose silently — which is precisely the func_80019310 precedent (refused 12427b10 -> foreclosed 2aed243e -> operator row 2cef233c in the 2026-09-06 foreclosed-bucket review -> unpark -> merged 3869ca31 as COMPLETED-INLINE-ASM-CANONICAL). After the row lands: queue unpark func_800204C0, the next session submits candidate.c EXACTLY (clearance skips layer-1), the driver re-proves bytes, runs FINAL CALL, and its owner-cluster door writes the inline_asm_canonical.txt line (bucket COMPLETED-INLINE-ASM-CANONICAL).

WHY NOT FAIL / PASS. FAIL is wrong: no construct is objectionable and no evidence is missing — a FAIL would re-grind a solved function against a wall no session can move. A bare PASS is wrong: the body is already cleared, and resubmission would hit the same grant-door refusal as 387fa8f8 (a livelock). One record defect, not a fail ground: the handoff entry cites a 'docs/grind/decisions.md 2026-09-08 ruling' for the 06:15 PASS, but that PASS is recorded only in state.json review_ledger and journal.md:1860 (commit 387fa8f8 did not touch decisions.md). Evidence: memory/grind/func_800204C0/evidence.md items 1-16, hypotheses.md H1-H6, rejected/ (2 forms), tmp/grind/func_800204C0/s1/.

## 2026-09-08 — func_800204C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait)

**Filed by the grinder Judge (2026-09-08)** — verdict ESCALATE (integration-handoff): the work is
sound but the grant is above the Judge's standing authority. Per the owner's
2026-08-18 ruling (judge-sole-gate, b9d91163) the driver disposes it immediately;
nothing waits on the owner.

**The Judge's packet:**

RULING REQUEST on the integration handoff filed at docs/grind/decisions.md:25395. The claim holds; the work is sound and complete; the only blocker is a registry file no grind session may write, and it is also outside the driver's own scope-grant classes.

WHAT WAS BUILT. A 97-instruction pure-C body (field reads through a byte pointer, a counter incremented in memory and re-read, signed divisions by 150 and 4096, a 16-bit compare, a double tail store) plus four GTE inline-asm islands that are the PsyQ SDK macro bodies character-for-character (gte_SetRotMatrix, gte_ldlv0 with its lhu/lhu/sll/or pack, the MVMVA command word 0x4A486012, gte_stlvnl), spelled identically to the already-integrated func_800203B4 (inline_asm_canonical.txt:367). No pins, no scheduling barriers, no free-standing GPR asm outside an island, no /* FAKE */ constructs, no build-time output rewriting (only src/code6cac.c would change).

WHAT I VERIFIED MYSELF (not taken from the ledger). (1) grindlib body_hash of candidate.c = 8655cc28f3aa5cc7, the body already PASSed at FINAL CALL (state.json review_ledger 2026-09-08 06:15); nothing in the C changed. (2) Applied candidate.c to src/code6cac.c transiently: sandbox func_800204C0 --disable all = score 0, 122/122, rules_dropped 0; canonical = ASM-PARTIAL 11/122 cop2; src restored to INCLUDE_ASM afterwards, tree clean. (3) candidate.c differs from the committed TU only in the function region (diff of the file minus that region is empty). (4) Target asm carries the grant's defining evidence: three materialize-then-copy preambles (addu $t4,$v1/$v0/$v0 at asm/funcs/func_800204C0.s:25,37,59) and 8 splat handwritten-instruction tags on the cop2 transfers. (5) scan_hand_coded tier LOW 1/8 (S4 only), which the 2026-09-01 grant record itself names as the GTE-wrapper-misroute artifact. (6) The 'islands are C-expressible' branch of the 387fa8f8 refusal is measured dead: the C-loads respelling reproduces 0 of 12 island-region instructions (GCC 2.7.2 never emits the redundant $t4 copy) — rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c, same class as func_80019310 s3 and func_800203B4 s6.

FAMILY. The cop2 addressing-preamble cluster, WIDENED-ANCHOR owner GRANT of 2026-09-01: func_800204C0 is named by name as a confirmed handwritten-tagged carrier (decisions.md:17959; .claude/rules/cop2-addressing-preamble-cluster.md:155). Its load-bearing prerequisite — the pure-C body independently at sandbox 0 — holds (verified above). Condition 3 (template = SDK macro body) holds per owner ruling 2026-09-02.

THE BLOCKER AND THE REMEDY. grant_canonical_asm (tools/grinder/grindlib.py:1493) opens on a STRONG scan tier or a row naming the function in tools/grinder/owner_cluster_grants.txt. The tier is LOW and the registry has rows for func_80031890, func_8002FF20 and func_80019310 under this same grant, but none for func_800204C0. The registry header says OPERATOR-MAINTAINED ONLY. The whole remedy is one line, same shape as the func_80019310 row: `func_800204C0 cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:17921; row per owner ruling 2026-09-02)`. I am setting scope_paths to that file so the record names the exact surface. I expect the driver to REFUSE it mechanically (tools/ is outside _SCOPE_GRANT_ALLOWED_RE), borderline-log this packet and foreclose silently — which is precisely the func_80019310 precedent (refused 12427b10 -> foreclosed 2aed243e -> operator row 2cef233c in the 2026-09-06 foreclosed-bucket review -> unpark -> merged 3869ca31 as COMPLETED-INLINE-ASM-CANONICAL). After the row lands: queue unpark func_800204C0, the next session submits candidate.c EXACTLY (clearance skips layer-1), the driver re-proves bytes, runs FINAL CALL, and its owner-cluster door writes the inline_asm_canonical.txt line (bucket COMPLETED-INLINE-ASM-CANONICAL).

WHY NOT FAIL / PASS. FAIL is wrong: no construct is objectionable and no evidence is missing — a FAIL would re-grind a solved function against a wall no session can move. A bare PASS is wrong: the body is already cleared, and resubmission would hit the same grant-door refusal as 387fa8f8 (a livelock). One record defect, not a fail ground: the handoff entry cites a 'docs/grind/decisions.md 2026-09-08 ruling' for the 06:15 PASS, but that PASS is recorded only in state.json review_ledger and journal.md:1860 (commit 387fa8f8 did not touch decisions.md). Evidence: memory/grind/func_800204C0/evidence.md items 1-16, hypotheses.md H1-H6, rejected/ (2 forms), tmp/grind/func_800204C0/s1/.

**Constraint recorded for any future session:** Submit memory/grind/func_800204C0/candidate.c EXACTLY (body 8655cc28f3aa5cc7, Judge-cleared) once tools/grinder/owner_cluster_grants.txt carries a func_800204C0 row; do not respell the islands (measured dead, rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c) and do not add FAKE/pins/barriers to chase a STRONG scan tier.

## 2026-09-08 22:31 — func_8005D554 — ruling: func_8005D554 reaches honest distance 0 (176/176 instructions, register allocati — **FAIL**

The closing carriers nv/nw (candidate.c: `nv = (s32)r4 - 0xC; ... a2_offset = nv; ... nv = ret; s.ret = nv;`, mirrored as nw) are FRESH locals written twice so reg_n_sets > 1 defeats sched.c birthing_insn_p. That shape is a settled non-member of every frozen family: staged-value-reused-variable bound 2 excludes an invented carrier; the named-intermediate entry (no-new-park-categories.md:225-226) says verbatim multi-WRITE carriers are NOT this entry; ordinary-c-judge-decidable Ruling 1 (owner, 2026-08-31) reaffirms 'Multi-WRITE carriers remain banned' and that the y1 FAIL (decisions.md:1736, fresh local written twice for set_preference) and the func_80045878 `c` FAIL (decisions.md:16311) stand. defeat-licm-hoist-var-reuse does not reach it: that family is loop.c movable admission (an invariant the target recomputes inline that GCC would otherwise hoist); the ledger itself concedes the mechanism here is sched.c:2505 priority, and loop.c does not hoist on this chassis (evidence.md s1: 'not desirable', threshold 29 vs insn_count 98). Real, consumed values do not lift the ban (y1's staging jobs were also real). Independently verified: rule texts, both precedent entries, the candidate text; the sandbox-0 claim was NOT re-scored (src/text1b.c holds INCLUDE_ASM and I am read-only). Exhaustion evidence (evidence.md s3 ablation table, rejected/) is genuine but is a prerequisite inside a family, not a substitute for membership. Non-membership is FAIL(CONSTRUCT), not a packet.

## 2026-09-09 — func_8005D554 (src/text1b.c) — OWNER-ESCALATION — **LADDER EXHAUSTED (non-endgame residual, floor 6): ROTATED**

PROOF-OF-FORECLOSURE RECORD (owner ruling 2026-08-31,
`.claude/rules/ordinary-c-judge-decidable.md` — a recorded disposition, never a
question to the owner; rotation per the 2026-09-08 ruling
`.claude/rules/rotation-not-foreclosure.md`, which retires the foreclosed
state). Filed by grind session s23 in the driver-assigned `escalation` modality.
Nothing here claims that policy blocks the function: a pure-C preimage exists by
construction, and this record exists so the next attempt starts from the measured
frontier instead of re-deriving it.

The honest floor is **6**, above `ENDGAME_LOCK_MAX_FLOOR = 5`, so the 2026-07-27
standing endgame-lock ruling is NOT this function's subject (owner ruling
2026-09-02) and no endgame-lock status is claimed here.

**Ladder accounting.** s1–s23: twenty-three sessions with the honest floor flat
at 6 from s1 onward, across EIGHT distinct modalities — recon, structural,
permuter, enumerate, synthesis, solver, forensics, rederive — plus this
escalation session. Roughly 2,500 complete spellings have been measured and
histogrammed (s4 alone swept 1,224; s5b 55; s15 61; s16 44; s17 66; s22's
group/chassis sweeps). 102 disproven forms were banked in
`memory/grind/func_8005D554/rejected/` before this session; this session adds two
more (104). The kill ledger carries 45 instance kills and 15 predicate-cited
class kills.

**State on main.** `src/text1b.c:2692` carries
`INCLUDE_ASM("asm/funcs", func_8005D554);` — the function has never had a cheat
committed on main, and no byte-match is being held up by a cheat construct: there
is no admissible candidate at distance 0. The sandbox's `cheat_asm_stripped: 155`
is the INCLUDE_ASM body itself, not a coercion.

**Floor re-measured this session.** `memory/grind/func_8005D554/candidate.c`
applied to `src/text1b.c` and measured with
`sandbox func_8005D554 --disable all`: **score 6, target_insns 176,
build_insns 176, rules_dropped 0**. `tools/fake_ablate.py --func func_8005D554
--file text1b --candidate memory/grind/func_8005D554/candidate.c` reports "no
FAKE-annotated constructs found; nothing to ablate" — the floor is a clean,
FAKE-free floor, so no banked kill is contaminated by a carrier occupying the
contested pseudo.

**The residual, stated exactly.** 176/176 instructions, frame 120 == target,
register allocation byte-identical to the target. The whole distance is two
identical 3-instruction rotations, one per loop half. We emit
`[addiu a2,s4,-K] [addiu a0,sp,16] [lw v1,gp] [move a1,zero]`; the target emits
`[addiu a0,sp,16] [addu a1,zero,zero] [lw v1,gp] [addiu a2,s4,-K]`
(`asm/funcs/func_8005D554.s`, 0x4DEB4–0x4DEC0). The target births the a2 base
LAST in the pre-call group; every admissible C spelling births it FIRST.

**Why the only known route to 0 is closed.** The single body ever measured at
distance 0 (`rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c`)
wins by giving the a2-site base a FRESH local that is WRITTEN TWICE in the
source: `reg_scan` counts two sets, so the pseudo is not a `loop.c` movable and
`sched.c:2505 birthing_insn_p` does not fire the LAUNCH boost, and `combine.c`
then erases the redundant set so no instruction is materialised. The Judge FAILed
that body at FINAL CALL on 2026-09-08 (decisions.md:26063) and made the ban
binding: "No fresh (invented) local may be written more than once to act as a
staging carrier for the a2-site base `(s32)r4 - K` or for any other value in this
function, under any name." Both legal ways to reach `reg_n_sets >= 2` without a
fresh multi-write local were then swept flat: borrowing an existing local (s17,
66 spellings, best 19 — every existing carrier is born too early and costs
+2/+3) and borrowing a parameter (s5b, retired together with the
zero-constant-holder lever). At `reg_n_sets == 1` the pseudo is either hoisted by
LICM (+2 insns) or, when the def-to-use LUID gap is <= 3, escapes the hoist and
takes the birthing_insn_p boost, which measures 8
(`rejected/fresh-single-set-base-gap1-birth-boost-fires-scores-8.c`, re-measured
at 8 this session on the current chassis).

**Gate (a) — canonical-asm scan: FAIL.**
`python3 tools/scan_hand_coded.py --single func_8005D554` →
`HAND_CODED: tier=LOW score=0/8 (176 insns)`, "no strong hand-coded indicators";
all of S1–S8 unchecked (0 multu/mflo pairs, no empty-body branches, 29 spills
over 14 distinct registers, max load burst 2, no high-similarity siblings, no
BIOS jumptable, no unsaved $sN, no redundant mask-before-shift). This is
compiler-shaped code; the canonical-asm grant path does not apply.

**Gate (b) — SOTN-master precedent census: FAIL.** The closing construct is a
fresh local written more than once purely as a staging carrier to control
call-argument emission order. `docs/reference/sotn-construct-index.md`
(sotn-decomp master `aa535002`) has no entry for it: greps for multi-write /
staging-carrier / reassignment shapes return exactly one hit
(`src/st/rcat/e_frozen_half.c:451`), and that is a comment marking a *program
bug*, not a codegen carrier. The nearest class, `new_var_temp` ("RA / scheduling
temporaries", 20 PSX hits at index lines 1423–1442), is a DECLARATION-shape
index: it records the declaration line only, so it cannot be shown that any of
those temporaries is written more than once, and no sotn-decomp checkout is
available here to exhibit one. Per the owner's standing bar, "genre-adjacent" and
"same spirit" do not qualify — a negative census is a FAILED gate, not an open
question.

**cc1psx self-disproof (banked by the driver in `state.json.cc1psx_check`,
2026-09-10T00:24Z).** Our candidate scores 6; the period-correct PsyQ cc1psx
scores 18 on the same body; `closer: false`. The residual is not a
compiler-provenance artifact.

**New measurements this session (s23).**

1. Kill re-audit on the current chassis (HEAD main @ 465f9fd0), FAKE-free:
   `fresh-single-set-base-gap1-birth-boost-fires-scores-8.c` → 8/176,
   `zero10-dep-folded-by-combine-byte-identical-to-boost-ctl-scores-8.c` → 8/176,
   `a2-statements-at-maximal-pre-call-birth-point-scores-6.c` → 6/176. All three
   reproduce their banked scores exactly; no banked kill was measured under a
   FAKE carrier.

2. Frontier item 1 (the `may_not_optimize` / standalone-CLOBBER LICM escape) was
   settled both in the compiler source and by measurement. Source: `reg_n_sets`
   is incremented ONLY under `case SET:` in `reg_scan_mark_refs`
   (`tools/gcc-2.7.2/regclass.c:1736` ff.), so a CLOBBER leaves
   `reg_n_sets == 1` — but `count_loop_regs_set` (`tools/gcc-2.7.2/loop.c:3018`)
   both sets `may_not_move[regno] = 1` for an explicit CLOBBER AND increments
   `n_times_set[regno]` for a CLOBBER pattern exactly as for a SET
   (`loop.c:3024`–`3049`). So the construct would suppress LICM while KEEPING the
   birthing_insn_p boost that is what costs — it lands on the score-8 chassis,
   not the score-0 one. Measured anyway with the only ordinary-ish emitter of a
   standalone CLOBBER for an SImode pseudo (`expr.c:2996` `store_constructor`
   into a register, spelled as a one-member union with a non-constant
   initialiser): both halves → **54/178**, half-1 only → **35/178**. The union
   carrier materialises +2 instructions instead of folding away. Banked as
   `rejected/union-constructor-clobber-carrier-costs-2-insns-scores-54.c` and
   `rejected/union-constructor-clobber-half1-only-scores-35.c`. Independently,
   that construct is outside the frozen family list and would be an AUTO-REJECT
   under the owner's 2026-08-24 ruling, so it is not argued for here beyond the
   negative measurement.

3. Frontier item 2 (the `reg_in_basic_block_p` first-uid escape) is closed on the
   source predicate: `loop.c:1062` returns 0 only when
   `regno_first_uid[regno] != INSN_UID (insn)`, i.e. only when the base register
   is MENTIONED at a lower uid than its set. `reg_scan_mark_refs` fills
   `regno_first_uid` from any REG occurrence, so the mention must be a real read
   or a real second set of the same local. A second set is the Judge-banned fresh
   multi-write carrier (or the s17-swept existing-local borrow); a read before the
   only set reads an uninitialised value on the first iteration, which is a
   semantic change, not a spelling. Both disjunct-3 routes therefore reduce to
   constructs already disposed of.

**Evidence pointers.** `memory/grind/func_8005D554/evidence.md` (2,370 lines),
`hypotheses.md` (2,687 lines), `state.json` floor_history s1–s23 (flat at 6),
`rejected/` (104 forms), this session's artifacts in
`tmp/grind/func_8005D554/s23/`, and the Judge's 2026-09-08 22:31 FAIL at
`docs/grind/decisions.md:26063`.

**Re-activation triggers.** (i) An owner class grant that covers a fresh local
written more than once purely as a staging carrier for call-argument emission
order — the one construct measured to reach 0 here; (ii) an exhibited
sotn-decomp master body (file+line, with the write count visible) showing that
construct shipping on PSX/GCC 2.7.2, which would convert gate (b); (iii) any
toolchain-fingerprint change (the driver re-measures candidates on one);
(iv) movement on the coupled sibling `src/ings.c` family or on any other
`text1b.c` function whose residual is the same pre-call argument-group rotation.
Rotation is not terminal: the item returns automatically on queue drain,
toolchain change, or sibling movement.

## 2026-09-16 — _exeque — OWNER-ESCALATION — **RESOLVED BY STANDING RULING (2026-07-27): ROTATED**

**Driver-assigned modality:** escalation (s12). Honest floor flat at 2/187 across
s4-s11 (8 consecutive sessions) spanning >=6 distinct modalities (permuter x2,
structural x2, enumerate, synthesis, solver, forensics, rederive) — the R1
ladder-exhaustion trigger. Re-confirmed this session on the current chassis:
`sandbox _exeque --disable all` = 2/187 (`build_insns` 186, `target_insns` 187,
`rules_dropped` 0, `cheat_asm_stripped` 143) with `memory/grind/_exeque/candidate.c`
applied verbatim to `src/display.c` (s4-s11's do-while(0) chassis, both FAKE-wrap
constructs unchanged, no volatile/register-pin/cheat-asm present).

**Endgame-lock gate (a) — canonical-asm scan tier:** `python3 tools/scan_hand_coded.py
--single _exeque` this session → `HAND_CODED: tier=LOW score=0/8` ("no strong
hand-coded indicators"; all 8 signals S1-S8 unset — no multu pacing, no empty
branches, no register spills beyond ordinary allocation, no front-loaded load
bursts, no jaccard-similar siblings, no BIOS jumptable, all callee-saves have
`$sp` saves, no redundant pre-shift mask). **FAILS.**

**Endgame-lock gate (b) — SOTN-master precedent for the closing construct:**
the residual (`hypotheses.md` H6, hypotheses/frontier "jalr-delay-slot residual")
is a single `sw $zero,0($v1)` (the `D_8009BE7C = 0;` clear) that our build's
reorg.c `fill_simple_delay_slots` moves into the following `jalr`'s delay slot,
where target keeps it as a separate instruction ahead of an explicit unfilled
nop. The only known closing spelling is `volatile s32 *p = &D_8009BE7C;`
(rejected form, `memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c`,
KILLED instance s2/re-confirmed H6) under the
`legitimate-volatile-interrupt-touched` two-prong carve-out's "guard-clear-and-
invoke" use-site shape — which is NOT one of that rule's three catalogued
shapes. This session searched `docs/reference/sotn-construct-index.md` (2,746
lines, PSX-tagged entries) for a PSX-provenance precedent of a `guard-clear-
and-invoke` volatile shape or any `extern volatile` IRQ-touched-global grant
matching this residual: zero hits for `extern volatile`, `IRQ`, or
`interrupt` anywhere in the index, and zero hits for a "clear-and-invoke"
shape near the `volatile`/pad entries that do exist (those are all unrelated
pad/dummy-local exhibits, lines 29-919). **A negative census after a real
search is evidence of no precedent, not an open question. FAILS.**

Both gates FAIL — the common case. Per the owner's 2026-07-27 standing ruling
and the 2026-08-24 `ENDGAME_LOCK_MAX_FLOOR = 5` amendment, floor 2 <= 5 puts
this in the endgame-lock branch.

**cc1psx self-disproof (banked in state.json `cc1psx_check`, 2026-09-16T07:16:53Z):**
attempted and inconclusive by tooling gap, not by result — `ok: false`,
`error: "cc1psx produced no scorable object: '_exeque not found in
tmp/cc1psx/_exeque/psx.o'"`. No `ours`/`psx` comparison was produced. This
does not affect the gate outcome above (neither gate depends on the cc1psx
comparison), but is recorded per the disposition template's requirement to
state the banked result.

**Exhaustion (ledger-sourced):** 11 sessions total (s1-s11), floor history
15(s1)->12(s2)->12(s3)->2(s4)->2(s5)->2(s6)->2(s7)->2(s8)->2(s9)->2(s10)->2(s11);
flat at 2/187 for the last 8 consecutive sessions across permuter (s4,s5),
structural (s6), enumerate (s7), synthesis (s8), solver (s9), forensics (s10),
rederive (s11) — 7 distinct modalities in the flat window, exceeding R1's >=4
threshold. Axis 2 (register-steering the store address into a call-clobbered
register to create a reorg.c resource conflict) is FORMALLY class-killed
(s10, predicate-cited: `tools/gcc-2.7.2/reorg.c:663-671` MEM case sets
`in_dest=0` unconditionally for a store's address operand, so it never
contributes a register bit to the computed "set" resources at
`reorg.c:690-693` — true for every possible hard-register assignment). Axis 3
(RTL-shape restructuring) is narrowed: s11's fresh m2c re-derivation produced
three structurally different spellings (single-exit top-level accumulator,
merged final-block `&&` condition, pointer-local elision), all three measured
WORSE (7/187, 5/187, 5/187) than the banked chassis. `sched_solver` (s9)
formally proved the search space for a scheduling-order lever is EMPTY (zero
sched1/sched2 order vectors differ from target across the whole function) —
not merely exhausted, structurally absent. The one remaining un-closed avenue
(axis 3's "insert a genuinely separate real statement between the store and
the jalr" sub-probe) was not run this session; it is carried into the
rotation record below as the re-activation-relevant lever, not as unfinished
exhaustion — R1 forbids a second full ladder cycle regardless.

**Re-activation triggers:** (1) an owner class grant extending
`legitimate-volatile-interrupt-touched`'s catalogued use-site shapes to cover
"guard-clear-and-invoke" (would directly close gate (b)); (2) a toolchain
fidelity finding affecting `reorg.c` delay-slot fill order; (3) a future
sibling function closing the same jalr-delay-slot residual shape by a novel
pure-C lever (sibling-ledger propagation would transplant it here); (4) an
un-run structural probe closing axis 3 (inserting a real intervening
statement between the `D_8009BE7C=0` store and the `D_8009BE80()` call,
per hypotheses.md's live frontier item) — this is grindable, not gate-blocked,
but R1 defers it to the next active window rather than a second ladder cycle
now.

**Candidate on disk:** `memory/grind/_exeque/candidate.c` (floor 2/187, unchanged
this session). `src/display.c` currently carries this body applied (re-verify
`sandbox _exeque --disable all` before any future session trusts it — the
ledger's own convention is that src drifts back to `INCLUDE_ASM` between
grinder sessions).

A pure-C preimage of `_exeque` exists by construction; nothing here claims
otherwise. This is a rotation record, not a question to the owner.

## 2026-09-25 — OWNER RULING — func_800288C8 owner-cluster row: none now; granted when a body passes review (`.claude/rules/inline-asm-policy.md`)

Question (second batch, 2026-09-25; the borderline.md entry "func_800288C8 — scorer strips verbatim
header GTE statements"). This paraphrase follows the recommendation. The first-batch scorer ruling
kept every admission requirement for func_800288C8, including an owner-instructed
tools/grinder/owner_cluster_grants.txt row for the inline_o.h class, and created none. Should that
per-function row be granted now?

The operator recommended no per-function entry until a body passes review. The body separately
failed on `tbl`, a pointless copy, so an entry now would authorise a function that can't land. "When
func_800288C8 later has a passing body, grant its entry then." The recommendation is quoted verbatim
in the rule file.

Owner (Trenton), verbatim: "Go ahead with your recommendations".

**Ruling (inline-asm-policy.md § "Scorer amendment (owner, 2026-09-25, second batch)", paragraph
"func_800288C8's row"; the author's narrowing).**
- No owner_cluster_grants.txt row for func_800288C8 is granted now.
- The row is granted, with no new owner question, when a func_800288C8 body passes review. All three
  of these must hold:
  - `sandbox --disable all` is 0;
  - the full-build SHA1 == oracle;
  - a fresh layer-2 PASSes every construct and states that the row is the only outstanding item.
- The operator adds the row, citing this ruling, in its own commit before the body lands. The body
  that lands is byte-identical to the body the layer-2 PASSed.
- A body still carrying the `tbl` copy, or anything else a reviewer FAILs, gets no row.

**Author's interpretations (flagged for the rule-text layer-2):**
1. "Passes review" is read as the three conditions above. A reviewer cannot PASS a body whose island
   admission is missing, so the review states that the row is the only outstanding item.
2. The approval is read as a standing instruction for this one function. It is not a class grant.

**Rule-text layer-2:** round 1 FAILed on four wording defects and round 2 on the scan-result
wording; the reviewer's replacement wordings were applied verbatim; round 3 PASS.

## 2026-09-30 — OWNER DECISION — maspsx `.L`-label mflo-hazard fix declined (func_80058580)

> **SUPERSEDED the same day** by "OWNER RULING — maspsx `.L`-label mflo-hazard fix adopted" below. Kept as history.

Context: func_80058580's floor includes two nops the original assembler (ASPSX) inserted after a
cross-jump join label (`mflo; subu; .L: nop; mult`). maspsx's `is_label()` only matches `$L`
labels, so its mflo/mult hazard arm never fires at this GCC fork's `.L` labels. A scratch-tree
probe (is_label also matching `.L`, plus an unconditional jump ending the hazard) kept the whole
build byte-identical and moved the function 57 -> 55 (memory/grind/func_80058580/probes/).
Asked whether to adopt it, the owner (Trenton) answered, verbatim: **"No let's find an avenue
without any kind of compiler or maspx fix or patch"**.

**Record.** No maspsx or compiler change is made. The probe stays in the ledger as evidence only.
Consequence (ledger, same day): the two nops are unreachable from C in the current pipeline
(the label position is fixed by the target's jump bytes; only maspsx inserts nops there), so
func_80058580 cannot reach 0 under the current substrate; work continues on the rest of its
residual.

## 2026-09-30 — OWNER RULING — func_800770B8 reused local + restore store REFUSED (no rule change)

Thirty-first batch (verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 31, Q66). Owner chose "Refuse for
now (Recommended)": "Keep the current bans. 770B8 stays asm and the ordinary-C search continues, re-measured after
Q65 lands, since its residual depends on D_800A36A0's addressing. The other SelWork functions land now." The
candidate (one local `p_old` for the list pointer then the new work-area pointer, plus a FAKE-annotated dead
restore store `p_old = prev;`; 2/175 without the store, 20/175 with two separate variables) stays refused under
the standing bans: Ruling 11 (B)(1) (a Ruling 11 variable may carry no dead write) and the Ruling 4 / Ruling 1
multi-WRITE carrier ban (a carrier whose extra write is dead). No rule text changes. func_800770B8 stays
INCLUDE_ASM and active; its ordinary-C search continues and is re-measured after the Q65 adoption lands.
