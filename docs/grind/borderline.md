# Borderline ledger

Established by the 2026-08-18 owner ruling (`.claude/rules/judge-sole-gate.md`):
user-escalation/approval is removed from the workflow; the default-FAIL Judge
applies the frozen standing policy, and anything borderline is APPENDED HERE for
the owner to evaluate later — nothing in this file is pending, and no entry
authorizes anything by itself. Only a subsequent owner ruling (landed standalone
per ruling-record-lands-before-code) can spend an entry.

Entry schema:

```
## YYYY-MM-DD — <function or scope> — <category>
category: canonical-asm-grant | family-candidate | needs-user-downgrade | policy-question
evidence: <scanner output / SOTN citation / reviewer question — pointers, not prose dumps>
disposition taken: <what the pipeline actually did under current policy>
```

Categories:
- **canonical-asm-grant** — a pipeline-executed inline_asm_canonical.txt grant
  (STRONG S1/S2/S6 + Judge PASS); logged for owner audit.
- **family-candidate** — a proposed frozen-list extension with exhibited
  precedent; REFUSED under the current list, logged for batch review.
- **needs-user-downgrade** — a cheat-reviewer NEEDS_USER verdict, mapped to
  FAIL; the reviewer's question recorded here.
- **policy-question** — a genuine project-architecture question (e.g. global
  rodata reorder); work proceeds elsewhere, nothing blocks on it.

---

## 2026-08-18 — refused-family back-fill (parked-set audit) — family-candidate
category: family-candidate
evidence: tmp-workspace parked-set audit + SOTN family surveys 2026-08-18 (sweeps @ sotn-decomp 8bd7c777); per-family verdicts below; full ruling record .claude/rules/no-new-park-categories.md § 2026-08-18 additions (commit 9984979f)
disposition taken: F3/F6/F7/phantom-slot SANCTIONED by owner ruling 2026-08-18 (b) and their carriers unparked (5329893e). F1 constant-staging chain (survey WEAK: genus shipped, species not — carriers func_80061658/func_80061710/func_800611A4 stay parked), F2 signedness-split dual read (WEAK — func_8001F938 stays parked), F4 cross-symbol idiom (ABSENT — struct merge is the sanctioned route), F5 union CLOBBER (ABSENT — func_80038C70 stays parked) REMAIN REFUSED with the survey as their standing evidence record.

## 2026-08-18 — parked-set name-drift alias table — policy-question
category: policy-question
evidence: parked-set audit 2026-08-18; phase-2 naming reset 2651e2e5 + PsyQ-library adoption renamed nine parked functions after their rulings were filed, so decisions.md rulings are unfindable from current queue names
disposition taken: alias table recorded here; no policy change.
  func_80038C70 = motion_SetMotion · func_80047FBC = InitHiraRmd_80047FBC ·
  CD_sync (0x80080DB0) = cpu_side_move_dir_4 · CD_datasync (0x80081BB0) = saEft01Init ·
  func_80041688 = gnd_init_80041688 · func_800307D0 = cpu_check_tubazeri_2 ·
  func_8003800C = damage_DebugDisp · func_80056FE8 = ang_hosei_80056FE8 ·
  func_80047EE8 = AddTbpOfst_80047EE8

## 2026-08-19 — SioSyncroWrite — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:6202 - 2026-08-19 - SioSyncroWrite - OWNER-ESCALATION - INTEGRATION HANDOFF (bytes proven; blocked by a commit surface a grind session may not stage). Requested action: add `SioSyncroWrite volatile_extern_allowlist.txt` to tools/grinder/scope_allow.txt (precedent scope_allow.txt:22-26), or land the D_800F1AEC allowlist grant directly per the six operator steps in the entry.
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-19 — func_8001B748 — policy-question
category: policy-question
evidence: session-filed escalation: 2026-08-19 - func_8001B748 - OWNER-ESCALATION - INTEGRATION HANDOFF (bytes proven at 0; blocked only by grind-session file scope) - docs/grind/decisions.md:6426
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-19 — func_8002D518 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md 2026-08-19 - func_8002D518 - OWNER-ESCALATION - INTEGRATION HANDOFF (bytes proven at distance 0; blocked by a stale banned_constructs tripwire in memory/grind/func_8002D518/state.json that no session may edit)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-19 — SioSyncroWrite + func_8001B748 + func_8002D518 — entries SPENT (operator integration)
category: resolution
evidence: all three 2026-08-19 INTEGRATION HANDOFF entries above were executed by the operator on owner instruction 2026-08-19; each reached COMPLETED-C under full two-layer adversarial review (verdicts in docs/grind/decisions.md; Match commits bdbe39fe / 6b797866 / 1d33c03b).
disposition taken: parks lifted via queue done; owner ruling integration-handoff-self-serve (2026-08-19) makes future bytes-proven handoffs pipeline-executable so this class never parks again.

## 2026-08-19 — stale-park re-audit — six parks unparked (rulings 2026-08-17/18 spent)
category: resolution
evidence: owner-directed read-only re-audit of all 33 parks against post-park family additions (2026-08-17 named-intermediate clarification + aggregate merge; 2026-08-18 F6/F7/phantom-pad; Ruling 4 + 2026-08-18 Sony census). Six refusal grounds are superseded: func_800481E8 (phantom-pad family), CD_sync=cpu_side_move_dir_4 (aggregate merge; also fixes the live dual-handle prong-(c) violation at src/system.c:374/648), func_80038170 (phantom-pad + judge-sole-gate), get_alarm=func_8007DC9C (Ruling 4 via confirmed LIBGPU/SYS census identity), func_80033550 (F6+F7 seam — session must verify or ruling-request), func_800611A4 (named-intermediate clarification). 24 parks STAND (register pins, or-tree swaps, frame-direction mismatches, RA walls — unchanged grounds); 3 UNCLEAR (func_80017848, CD_datasync=saEft01Init, plus the 07-20/07-22 LUID-tie trio where the F6 cancellation pair is a new measurable lever, not a re-sanction).
disposition taken: six items returned to active via `queue unpark` (park_reason preserved in unparked_from); they re-enter the distance-ordered lane and face every normal gate — nothing is pre-approved by the unpark. Queue-name/ledger-name alias table for 8 renamed parks is in the 2026-08-18 alias entry above.

## 2026-08-19 — asm-until-matched migration executed (owner ruling, rules commit 398b988d)
category: resolution
evidence: docs/superpowers/plans/2026-08-19-asm-until-matched-migration.md; commits 38464a02..HEAD (prototype + 4 sweep batches + unglue). 191 of 259 INCOMPLETE functions converted to INCLUDE_ASM("asm/funcs", <func>); retired chassis + rule stacks banked per-function in memory/grind/<func>/retired-chassis-2026-08/; honest floors pinned in migration_pin.json (queue regen reads pin or ledger floor_history minimum). Rules 1573 -> 708. Oracle SHA1 verified on every commit; completion census byte-exact at 1036/179/259 after each batch.
disposition taken: 68 functions deferred with their old representation intact (tmp/migration-deferred.txt; classes: 26 asm-references-C-generated-jtbl, ~38 rodata-emitting bodies, 2 renamed-asm-file, 2 measured byte-position-coupled: func_80040B44, func_80041688). Deferred functions migrate individually when solved, or via a future guarded pass; their rules remain the only rule debt outside jtbl-infra/canonical-extraction wirings.

## 2026-08-19 — wave-2 rule retirement (deferred set) — measured dead end
category: resolution
evidence: --with-jtbl prototype on both jtbl-only candidates (func_80038C70, func_800460E4): INCLUDE_RODATA-supplied tables land at the wrong TU rodata position (GCC pools compiled rodata in its own order) — SHA1 mismatch, auto-rolled-back. The strings class (~38 functions) faces the same positional coupling, worse. Global rodata reorders to force placement are the forbidden speculative class (no-new-park-categories).
disposition taken: the 68 deferred functions keep the legacy representation; their 708 rules retire per function at COMPLETED-C (each solve replaces rule-supplied bytes with C-emitted rodata natively). No further mechanical wave planned.

## 2026-08-19 — func_80038170 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-19 — func_80038170 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80038170 include/code6cac.h; cleared 1 superseded ban(s); function stays ACTIVE.

## 2026-08-19 — motion_Close — rename-migration non-actionable (recorded)
category: resolution
evidence: motion_Close @0x80083804 has no standalone asm file — splat split it fused inside asm/funcs/func_80083794.s (one file spans both census functions; func_80083794 = the paired motion_Open). Splitting the .s buys nothing now: func_80083794 is itself rodata-deferred, so both keep the legacy representation regardless.
disposition taken: stays deferred with its sibling; the file split happens naturally whenever either function is solved. file_LoadSectors (the other renamed-asm deferral) was migrated this date: asm/funcs/func_800165F8.s renamed to file_LoadSectors.s with glabel/endlabel updated (no pipeline references to the old name), INCLUDE_ASM landed, SHA1 MATCH, floor-pin 14.

## 2026-08-19 — owner decision: no repo-wide rule reset; grind through the 41 rule-carriers
category: resolution
evidence: owner Q&A 2026-08-19 (post asm-until-matched). Full TU-resplit reset (spec docs/superpowers/specs/2026-08-06-tu-resplit-campaign.md) evaluated and declined: it costs hand-maintained bb2.ld surgery, invalidates the deepest chassis-relative ledgers, buys nothing toward solving, and its original target population (237 asmfix lines) is down to 2.
disposition taken: the 33 active rule-carriers (672 rules) retire through normal grinding; the 8 parked (36 rules) stay on the policy track. TU-resplit is held in reserve as a PER-FUNCTION scalpel, only if a specific deferred function livelocks because of its representation, with its ledger re-measured on the new chassis in the same session.

## 2026-08-20 — func_80038170 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-20 — func_80038170 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80038170 include/code6cac.h undefined_syms_auto.txt  (merged with prior line); cleared 1 superseded ban(s); function stays ACTIVE.

## 2026-08-20 — func_80038170 — policy-question
category: policy-question
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-20 — func_80038170 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy (endgame-lock standing ruling 2026-07-27, extended by judge-sole-gate 2026-08-18); terminal OWNER-ACCEPTED INCOMPLETE park; candidate preserved at memory/grind/func_80038170/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-08-20 — func_80047FBC — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:7530 — ## 2026-08-20 — func_80047FBC (src/text1b.c) — **OWNER-ESCALATION** — bytes proven at 0; AND-gate (b) PASSES with a cited SOTN-master precedent; blocked ONLY on an owner-class engine allowlist row
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-20 — func_8002EA24 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:7843
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-20 — func_80047EE8 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:8765 — ## 2026-08-20 — func_80047EE8 / AddTbpOfst_80047EE8 (src/text1b.c) — **OWNER-ESCALATION** — bytes proven (full-build SHA1 == oracle); AND-gate (b) PASSES with cited SOTN-master precedent; blocked ONLY on an owner-class engine allowlist row
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-20 — func_800481E8 — policy-question
category: policy-question
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-20 — func_800481E8 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy (endgame-lock standing ruling 2026-07-27, extended by judge-sole-gate 2026-08-18); terminal OWNER-ACCEPTED INCOMPLETE park; candidate preserved at memory/grind/func_800481E8/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-08-20 — func_80041688 — policy-question
category: policy-question
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-20 — func_80041688 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy (endgame-lock standing ruling 2026-07-27, extended by judge-sole-gate 2026-08-18); terminal OWNER-ACCEPTED INCOMPLETE park; candidate preserved at memory/grind/func_80041688/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-08-20 — func_80049A2C — policy-question
category: policy-question
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-20 — func_80049A2C — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy (endgame-lock standing ruling 2026-07-27, extended by judge-sole-gate 2026-08-18); terminal OWNER-ACCEPTED INCOMPLETE park; candidate preserved at memory/grind/func_80049A2C/candidate.c; re-attemptable if a later owner ruling spends this entry.
