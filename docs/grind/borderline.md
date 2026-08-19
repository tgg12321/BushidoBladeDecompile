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
