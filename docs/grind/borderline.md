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
- **resolution** — an earlier entry SPENT or superseded (e.g. by a later owner
  ruling or measurement); recorded for closure.
- **integration-handoff** — a driver-executed scope grant per
  `.claude/rules/integration-handoff-self-serve.md` (emitted mechanically by
  `tools/grinder/grind.ps1`).

> Entries below are HISTORICAL and dated — each describes state AS OF ITS OWN
> DATE. For current park state read `engine/queue.json` (`park_reason` /
> `unparked_from`), never a count quoted in an entry (e.g. the 2026-08-19
> re-audit's "24 parks STAND" predates the 2026-08-24 migration sweep).

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

## 2026-08-21 — func_800858D0 — policy-question
category: policy-question
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-21 — func_800858D0 — JUDGE ESCALATE on ruling request (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy (endgame-lock standing ruling 2026-07-27, extended by judge-sole-gate 2026-08-18); terminal OWNER-ACCEPTED INCOMPLETE park; candidate preserved at memory/grind/func_800858D0/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-08-24 — parked-set migration hygiene sweep (9 functions) — policy-question
category: policy-question
evidence: owner-directed parked-set review 2026-08-24. The 2026-08-19 migration's rodata-refusal heuristic counted quotes INSIDE cheat constructs (`asm("s0")` pin specifiers, `__asm__("...")` templates) as .rodata evidence — every pin-carrying parked body false-failed its own eligibility check. tools/migrate_include_asm.py now strips asm constructs before the literal scan (strip_asm_constructs). Bodies re-checked: zero real string/const/FP literals.
disposition taken: 9 parked functions migrated to INCLUDE_ASM under asm-until-matched (special_camera_get_rot_dir, func_80062020, func_80037A20, func_80037B00, func_800871D4, func_80048AD0, func_800645B0, func_80078654, func_80061658) — 7 rules retired (func_800645B0 ×1, func_80078654 ×6), all in-source pins/barriers/injected-asm removed, batch build SHA1 == oracle, integrity audit OK. func_800324D0 stays deferred on its REAL ground (asm references C-generated jtbl). Metadata repaired: func_80045294 inert prologue_config entry removed (build-verified); func_80057CC8 migration-pin floor corrected 9->30 (decisions.md:8107); func_80061658 floor_history corrected past the banned-form 0 (sanctioned floor 7); 15 stale park_reason strings annotated. No park dispositions changed; no standards touched.

## 2026-08-24 — docs/escalations shelf retirement (_spu_FiDMA fork-crash question) — policy-question
category: policy-question
evidence: docs/escalations/closer-cc1-fork-divergence.md + spu3-fork-crash-evidence/ (8-probe grid; preserved in git history at this commit's parent). The decompals cc1 fork SIGSEGVs on the faithful Sony volatile-MMIO wait-loop spelling that original cc1psx compiles to exact target bytes; the closest crash-avoiding C spelling lands 2 words short (loop notes lost, reorg increment-compensation never fires). This is a fork CRASH bug, distinct from ordinary codegen divergence.
disposition taken: shelf retired under judge-sole-gate (2026-08-18 — no owner-wait surfaces). Standing policy applies: no-compiler-divergence forbids fork patches and cc1psx adoption, so DispUpdateStatusMessage/_spu_FiDMA remains ordinary INCOMPLETE queue work — the frontier is a crash-avoiding pure-C spelling that recovers the 2-word loop-note residual. A later owner ruling could spend this entry by classifying a minimal cc1 SIGSEGV bug-fix as in-scope (analogous to maspsx bug-fix scope); until then it is refused by default.

## 2026-08-24 — cc1 fork-crash: measure-first results — the packet is now CONCRETE — policy-question
category: policy-question
evidence: Packet-4 measure-first steps executed same day (decisions.md escalation-packet rulings entry). (1) Gate-1 RECONFIRMED on the current 2026-08-07 -mel cc1: identical crash pattern to the 2026-07-13 grid (SIGSEGV exit 139 on the faithful spelling + probes 1/4/6/8; clean on 2/3/5/7). (2) The psyz PsyQ-4.0 seed's loop shape (`timeout++; if (timeout > 0xF00) break;` inside a volatile-cond while) ALSO CRASHES — the workaround hypothesis is dead; the trigger class is broader than the July grid recorded. (3) gdb backtrace (out-of-tree diagnostic, instrumented-cc1 precedent) LOCALIZES the bug: SIGSEGV in arith_operand (config/mips/mips.c:383) on op=0x55797380 — exactly the LOW 32 BITS of a heap rtx pointer (0x5555_5579_7380) — reached from reorg.c:3562 fill_slots_from_thread -> insn-recog. A 64-BIT HOST POINTER TRUNCATION in the fork's build (an rtx stored through an int-typed path in the reorg eager-fill thread handling). cc1psx never crashed because it ran 32-bit; its output for the faithful source is the byte oracle (tmp/closer/spu3/t_psx.s).
disposition taken: refused by default under no-compiler-divergence pending an owner ruling on the NOW-CONCRETE question: is a minimal 64-bit-host-portability crash fix (int->pointer type correction in the reorg fill path, or equivalently a -m32/build-flags change to how the 1996 source is compiled for the HOST) in bug-fix scope, analogous to maspsx bug-fix scope — given the fix changes what cc1 ACCEPTS, with codegen externally pinned by the cc1psx exhibit and the full-build SHA1 oracle? Governance prerequisites either way: track the cc1 binaries in git; record host build flags in oracle/manifest.json.

## 2026-08-24 — func_80083794 — canonical-asm-grant
category: canonical-asm-grant
evidence: owner packet-2 ruling, docs/grind/decisions.md "2026-08-24 — escalation-packet rulings" (commit 608c8a4c, landed before code): libgcc __main / crt0 ctor-walker routed COMPLETED-INLINE-ASM-CANONICAL as provably-prebuilt object code — REG_PARM_STACK_SPACE arithmetic-floor-9 non-existence proof (decisions.md 2026-08-13), 7 provenance results, _start adjacency (same crt0 grant family, 2026-08-06). scan_hand_coded LOW 0/8 disclosed — the grant rests on the non-existence proof, not scanner tier.
disposition taken: inline_asm_canonical.txt entry written; fused .s split (motion_Close gets its own file + INCLUDE_ASM, returned to active with a provenance-first directive); 18 legacy rules retired; queue done accepted COMPLETED-INLINE-ASM-CANONICAL at SHA1 62efab4f...; layer-2 cheat-reviewer PASS (resubmission after queue reconciliation — the first review correctly FAILed on bookkeeping gaps).

## 2026-08-24 — func_80038C70 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:10376 — '2026-08-24 — func_80038C70 (motion_SetMotion, src/code6cac_c_mid.c) — OWNER-ESCALATION — ESCALATED WITH DECISION PACKET'
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-24 — func_80041188 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md -- '## 2026-08-24 - func_80041188 - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**' (appended this session; the single decidable question is whether the owner grants a canonical-asm authorization for func_80041188 as an explicit override of the LOW scan_hand_coded tier, which would retire all 16 regfix/asmfix rules plus the project's last prologue_config.json entry and take the rules-to-zero campaign to 0 carriers, versus declining the override and accepting 17 permanent carriers).
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — func_800460E4 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:10742 - '## 2026-08-25 - func_800460E4 - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**'
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — func_80060A68 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md - '## 2026-08-25 - func_80060A68 - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**' (grind s10): both endgame gates re-measured and failed (scan_hand_coded --single = tier LOW 1/8, S4 only; no construct in play so no SOTN-master precedent is applicable), floor flat at 2 across ten sessions and six modalities, and the decidable question posed is the representation/routing choice for the project's last two asmfix rules (asmfix.txt:82-83, body-coupled whole-body splice): keep the legacy splice while the function is INCOMPLETE, or authorize an operator-side lane to repair the 2026-08-24 INCLUDE_ASM migration failure (sha1 699695d890f0570a3039af734ed84a6a7c44302c != oracle) so those rules retire and the function continues as an ordinary INCLUDE_ASM queue item at honest floor 2. No grant, family sanction or evidence-bar override is requested.
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — SioSyncroRead — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-25 — SioSyncroRead — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: SioSyncroRead volatile_extern_allowlist.txt; function stays ACTIVE.

## 2026-08-25 — func_80019568 — family-candidate
category: family-candidate
filed by: grind session s3b (permuter modality), at the explicit instruction of the
2026-08-25 12:52 layer-1 review (docs/grind/decisions.md:11110).
construct: a per-slot enable flag local inside the loop-1 body —
`s32 enable = 0;` declared at the top of the loop body, `enable = 1;` in the
packet-valid arm, and the write-out `sp.output[i + 2] = enable;` (spelled `o[2] = enable;`
through the per-iteration record pointer) appearing in BOTH arms. The else arm READS the
declaration's initialiser; there is no in-arm `enable = 0;` assignment, which is what
distinguishes it from the shape the driver has banned for this function
(`s32 enable; ... enable = 1; o[2] = enable; ... enable = 0; o[2] = enable;` — the
12:52 layer-1 FAIL, banked as rejected/layer1-fail-0825-1252.c).
semantics: `sp.output[i + 2]` is the slot's enable word; target stores 1 in the valid arm
(`addiu $v0,$zero,1; sh $v0,0x4($a2)`, target line 34) and 0 in the invalid arm
(`sh $zero,0x4($a2)`, target line 55). Both stores are real program output; neither the
flag nor either store is dead.
mechanism (read out of tools/gcc-2.7.2/loop.c, and CORRECTED this session by ablation):
scan_loop hoists the `1` unless the value pseudo fails
`n_times_set == 1 || consec_sets_invariant_p` (loop.c:706-709). The loop-top default and
the in-arm override are two non-consecutive sets, so no movable is created and the
`addiu $v0,$zero,1` stays in the loop, filling target's lhu load-delay slot. Because the
flag is a pseudo DISTINCT from the shifted voice id, local-alloc also seats the voice temp
in $v0 and the lhu reload in $v1 (target's assignment) instead of the mirror that every
carrier-reuse spelling produces.
why it is a family candidate rather than an obvious pass: the gate it trips is the SAME
loop.c gate the FAKE-gated variable-reuse family trips (defeat-licm-hoist-var-reuse.md /
staged-value-reused-variable.md). The difference is that the two sets belong to a fresh
variable whose two values are the program's own two values, not to a borrowed carrier.
It is not named-local-fake-exception (that rule's scope is a constant HOLDER, one value),
not dead-store-fake-exception (nothing here is dead), and only arguably
duplicated-statement-into-arms (the two copies are the same statement text but that rule's
prerequisite 2 demands byte-neutrality vs the shared-label form, and here the joined
spelling measures 21/136 — four target instructions short — so the duplication is NOT
byte-neutral; it reproduces a duplication the target itself has).
measured stakes: with the construct, `sandbox func_80019568 --disable all` = 0,
build_insns 141 == target_insns 141, re-verified this session. Without any named local at
this store the honest floor is 8. The full ablation table (11 spellings) is in
memory/grind/func_80019568/evidence.md [s3b]; three permuter campaigns from three distinct
seeds (~28k iterations) found no other spelling that reaches 0.
consequence of a YES: func_80019568 completes as COMPLETED-C and its 5 regfix rules retire
(owner's rules-to-zero campaign). Consequence of a NO: the function's honest floor is 8 and
it returns to the ladder with every spelling at this divergence measured and banked.
disposition taken: session emitted `ruling-request`; no code committed.

## 2026-08-25 — func_800307D0 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md — '## 2026-08-25 — func_800307D0 (cpu_check_tubazeri_2, src/code6cac_b.c) — **OWNER-ESCALATION — ESCALATED WITH DECISION PACKET**' (final entry in the file, filed by this session); prior related entries: decisions.md:1311 (2026-07-22 owner ruling, option b, integer-cast form), :11147/:11151/:11155 (2026-08-25 FAIL/PASS/FAIL adjudications of the current body).
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — func_800645B0 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md — '## 2026-08-25 — func_800645B0 — **OWNER-ESCALATION — ESCALATED WITH DECISION PACKET**' (appended this session; decidable question: repair the solver toolkits' target-stream derivation for INCLUDE_ASM-routed functions so the owner's own 2026-08-24 solver directive becomes executable — a fidelity/routing question that lowers no standard and requests no family grant)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — func_8002EA24 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:11688 -- 2026-08-25 func_8002EA24 OWNER-ESCALATION -- ESCALATED WITH DECISION PACKET (tooling-repair authorization; solver-first directive executed)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — func_80062020 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md - '## 2026-08-25 - func_80062020 (src/text1b.c) - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**' (filed by this session under the binding judge_constraint of the 2026-08-25 21:17 ruling; replaces the partly-false 2026-07-24 packet)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-25 — func_80072CD4 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md - '## 2026-08-25 - func_80072CD4 (src/text1b.c) - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**' (fidelity/routing question: Option A toolchain-fidelity calibration probe vs Option B return to ACTIVE with modality solver/ra_solver; no family grant, no permanent-rule sanction and no canonical evidence-bar override requested)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-26 — special_camera_get_rot_dir — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md — '2026-08-26 — special_camera_get_rot_dir (src/code6cac_b2_post.c) — OWNER-ESCALATION — ESCALATED WITH DECISION PACKET': may include/code6cac.h be added to tools/grinder/scope_allow.txt for this function so the stale 1-arg CdRead prototype at include/code6cac.h:510 can be corrected at its canonical location? Bytes are proven at floor 0 (verify-oracle SHA1 match); all four placements of the corrected prototype are enumerated and only the header one is honest.
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-26 — motion_Close — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:13770 — ## 2026-08-26 — motion_Close (0x80083804, src/ings2.c) — **OWNER-ESCALATION — ESCALATED WITH DECISION PACKET (RE-FILED)**
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-26 — func_80078654 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md - '## 2026-08-26 - func_80078654 - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**' (routing/investment question: instrument the [flow_analysis, global_alloc) deletion window and keep the function active, or leave it active under standing policy with no further body-level grinding; both endgame gates re-measured FAIL, no standard is asked to be lowered)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-26 — func_800324D0 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:14092 - 2026-08-26 func_800324D0 OWNER-ESCALATION - ESCALATED WITH DECISION PACKET (representation/routing: may asm/rodata/jtbl_800105A0.s be wired back into the link so func_800324D0 can migrate to INCLUDE_ASM and its four register-asm pins leave main?)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-26 — func_8002FC80 — policy-question
category: policy-question
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-26 — func_8002FC80 — JUDGE ESCALATE on final call (policy-question) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy (endgame-lock standing ruling 2026-07-27, extended by judge-sole-gate 2026-08-18); terminal OWNER-ACCEPTED INCOMPLETE park; candidate preserved at memory/grind/func_8002FC80/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-08-26 — func_80061250 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-08-26 — func_80061250 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80061250 src/text1b.c src/text1b_b.c undefined_syms_auto.txt; cleared 1 superseded ban(s); function stays ACTIVE.

## 2026-08-30 — escalation batch — owner rulings landed (all packets dispositioned)
category: policy-question
evidence: docs/grind/decisions.md '2026-08-30 — OWNER RULINGS — escalation batch resolved' (10 rulings covering all 23 escalated items; recommendations reviewed and approved verbatim in-session)
disposition taken: rulings recorded before code (ruling-record-lands-before-code); all items return to ACTIVE (motion_Close routes COMPLETED-INLINE-ASM-CANONICAL per ruling 3); owner-cluster canonical-grant door added registry-bound (ruling 4); scope grant include/code6cac.h for special_camera_get_rot_dir (ruling 2); instrumentation grants diagnostic-only (ruling 1); no standard lowered.

## 2026-08-30 — motion_Close — canonical-asm-grant
category: canonical-asm-grant
evidence: owner ruling 2026-08-30 (decisions.md, ruling 3) extending the 2026-08-24 func_80083794 prebuilt-object routing; s14 provenance census — 2/1,435 asm functions carry the below-$sp+16 callee-save fingerprint (motion_Close + granted twin, byte-contiguous crt0/libgcc ctor/dtor pair)
disposition taken: inline_asm_canonical.txt entry written (INCLUDE_ASM body is the accepted form, mirroring func_80083794); queue done after layer-2 review; COMPLETED-INLINE-ASM-CANONICAL.

## 2026-08-30 — main — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:14884 - 2026-08-30 - main (src/ings.c) - OWNER-ESCALATION - ESCALATED WITH DECISION PACKET (queue-routing question only; explicitly does NOT re-open the 2026-08-24 maspsx branch-fill DECLINE and lowers no standard)
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-30 — func_80062020 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:15732 — ## 2026-08-30 (s10) — func_80062020 (src/text1b.c) — **OWNER-ESCALATION — ESCALATED WITH DECISION PACKET**
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-31 — func_80045878 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md - 2026-08-31 (s13b) - func_80045878 (src/text1a_c.c) - OWNER-ESCALATION - ESCALATED WITH DECISION PACKET
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-31 — func_800324D0 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:16729
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-31 — func_8002FC80 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002FC80 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-08-31)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-08-31 — func_8002FC80 — policy-question
category: policy-question
evidence: session-filed escalation: docs/grind/decisions.md:16851 — '2026-08-31 — func_8002FC80 — OWNER-ESCALATION — ESCALATED WITH DECISION PACKET'
disposition taken: terminal OWNER-ACCEPTED INCOMPLETE park (ruling 2026-08-18 — no pending states); re-attemptable if a later owner ruling spends this entry.

## 2026-08-31 — func_8002D320 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002D320 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-08-31)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-01 — cop2 materialize-then-copy widened anchor — family-candidate
category: family-candidate
evidence: FOR — memory/grind/func_800203B4 ledger: islands are literal PsyQ SDK macro bodies, residual measured as exactly 25 un-C-expressible insns (11 cop2 + 12 in-island integer + 2 delay nops), island partition measured minimal (s6), GCC 2.7.2 MIPS backend has zero cop2 mnemonics (s8); the 2026-08-17 func_8002FDB0 cluster ruling covers the same idiom with an `$aN` copy source. AGAINST — scan_hand_coded LOW 1/8; zero-hit SOTN census (2026-09-01 s9 record); Judge FAIL(CONSTRUCT) 2026-09-01 09:01 (decisions.md:17546) holding the non-$aN widening is a family extension; AUTO-REJECT clause "a LOW scan tier is an answer, not an obstacle to be waived". Would-be beneficiaries per the Judge's hypothetical enumeration: func_80017FA0, func_80019310, func_800203B4, func_800204C0, func_8002FF20, func_80031890, func_8003E6D8.
disposition taken: NOT granted (first-draft grant FAILed by layer-2 review and struck). Question logged for the owner's own hand per [[judge-sole-gate]] rule 4 and surfaced in the 2026-09-01 session report; all seven functions keep their current dispositions until an explicit owner ruling lands.

## 2026-09-01 — tools/ra_solver inverse_compose --target-object — policy-question (resolved)
category: policy-question
evidence: owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry, Ruling C); mirrors the sched_solver --target-object escape granted 2026-08-25; closes the ledger-documented pipeline-wide gap (get_alarm 2026-08-30 record: every post-migration RA-seat residual un-analysable).
disposition taken: operator lane implements; read-only diagnostic, no gate or byte surface touched; engine suite kept green.

## 2026-09-01 — libcd CD_intr aggregate merge — policy-question (resolved)
category: policy-question
evidence: owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry, Ruling D); applies the already-sanctioned per-word-splat->aggregate-merge family (2026-08-17) to 0x800A1494/95/96; prong-(a) evidence banked (target base+offset addressing, 2026-07-09 naming census, closer Ruling 1 naming the merge, CD_datasync F14); volatility grounded by matched in-TU consumers.
disposition taken: one sibling-scoped structural session authorized; mandatory prong-(c) asm-consumer check first; byte-neutrality via verify-oracle --rebuild; candidates face the normal gates.

## 2026-09-01 — cop2 materialize-then-copy widened anchor — canonical-asm-grant (RESOLUTION of today's family-candidate entry)
category: canonical-asm-grant
evidence: owner GRANT 2026-09-01 (decisions.md "cop2 materialize-then-copy WIDENED ANCHOR — OWNER GRANT"), issued as an informed ruling with the objections presented (LOW scan tier, zero-hit SOTN census, Judge 09:01 routing FAIL). Same evidence set + same 4-point mechanical check as the 2026-08-17 cluster ruling; mechanical membership enumerated (68 in-band carriers, 66 with non-$aN-source sites; scan method in the cluster rule doc).
disposition taken: today's earlier family-candidate entry is RESOLVED-GRANTED. func_800203B4 integrated per its foreclosure record's trigger-1 recipe under the full gates (sandbox 0 re-verify, fresh layer-2 cheat-reviewer, verify-oracle --rebuild, queue done). Remaining confirmed carriers (func_80019310, func_800204C0, func_8002FF20, func_80031890, func_8003E6D8) keep their queue states; islands covered only when their C bodies independently reach zero.

## 2026-09-01 — func_8002FF20 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-01 — func_8002FF20 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_8002FF20/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-09-02 — func_80027640 — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/decisions.md:20175
disposition taken: FORECLOSED silently (owner ruling 2026-08-31 — no pending states, no packets); re-attemptable if a later owner ruling spends this entry.

## 2026-09-02 — func_800300B4 — policy-question
category: policy-question
evidence: layer-1 cheat-reviewer FAIL x2 (docs/grind/decisions.md:20470, :20478) against the gte_ldv0 SDK-macro-body pack (lhu/lhu/sll/or through the macro's $12 copy) inside cluster island 2; the Judge's 07:30 PASS ruling (decisions.md:20474) was struck by the second FAIL as a rule-4 scope reinterpretation. Question for the owner: does cop2-addressing-preamble-cluster.md mechanical condition 3 ("in-island GPR instructions limited to the cop2 addressing preamble") admit an SDK GTE-macro body whose GPR instructions read through the macro's own redundant $t4 copy? FOR: the owner's own 2026-09-01 grant text (inline_asm_canonical.txt:367, func_800203B4) names "lhu VX0/VY0 pack" as an authorized island unit under the same unchanged 4-point check; the identical island is on main in cluster siblings func_8002E838 (49d6927e) and func_80031890 (1a2e49e4); the pack is measured unreachable from C (memory/grind/func_800300B4 H4 class kill, sandbox 19 both spellings, local-alloc.c:2249). AGAINST: the reviewer reads the lhu/sll/or as C-expressible data processing and holds that a same-pipeline ruling cannot widen the cluster's scope. Bytes: sandbox 0 (83/83) and verify-oracle ok with the candidate (memory/grind/func_800300B4/candidate.c). Session record: docs/grind/decisions.md 2026-09-02 func_800300B4 FORECLOSED entry.
disposition taken: FORECLOSED silently (owner ruling 2026-08-31 — no pending states); candidate preserved at memory/grind/func_800300B4/candidate.c; re-attemptable unchanged if a later owner ruling spends this entry.

### 2026-09-02 addendum (session s1h) — provenance correction for the entry above
The island is Sony's `gte_ldlv0` ("load 32-bit VECTOR into V0"), not `gte_ldv0`: its body in PsyQ Run-time Library 4.5
inline_c.h:101-110 is verbatim `lhu $13,4(%0); lhu $12,0(%0); sll $13,$13,16; or $12,$12,$13; mtc2 $12,$0; lwc2 $1,8(%0)`
(clobbers $12,$13); the target differs only by the older-SDK `move $12,%0` prefix and a one-register temp shift
(memory/grind/func_800300B4/hypotheses.md H9/H10, excerpt at tmp/grind/func_800300B4/s1/psyq45_inline_c_excerpt.txt). The
lhu/lhu/sll/or is the SDK header text, i.e. the template itself. Subsequent pipeline events: Judge ruling PASS 2026-09-02 07:59
(decisions.md:20575) lifting the island-2 ban, then layer-1 FAIL 08:12 (decisions.md:20579) holding the question owner-only
and re-imposing the ban. Ban-compliant floor measured 19 on HEAD db16e520 (seat-swap recovery by loop-depth weighting
measured closed, H14); the question above is unchanged and still open for the owner.

### 2026-09-02 addendum (session s3) — the ban-compliant residual is now the island alone
Structural session s3 closed every remaining ban-compliant divergence outside island 2: the honest
floor of the pack-in-C chassis went 19 (s1) -> 11 (s2) -> **7** (s3, HEAD 9c1533fc). The 7 is a
single diff hunk containing only the `gte_ldlv0` SVECTOR pack; every other instruction of
func_800300B4 — all four call-crossing register seats, all scheduling order, the whole tail —
byte-matches in ban-compliant pure C:

    TGT: addiu v0,s3,44 ; move t4,v0 ; lhu t6,4(t4) ; lhu t5,0(t4) ; sll t6,t6,0x10 ;
         or t5,t5,t6 ; mtc2 t5,$0 ; lwc2 $1,8(t4)
    BLD: lhu v0,48(s3) ; lhu v1,44(s3) ; sll v0,v0,0x10 ; or v1,v1,v0 ; addiu v0,s3,44 ;
         move t4,v0 ; mtc2 v1,$0 ; lwc2 $1,8(t4)

(`addiu`, `move t4,v0` and `lwc2` are matching context.) Six distinct C spellings of the pack all
measure exactly 7 (memory/grind/func_800300B4/hypotheses.md H23): the target's pack is based on
`$t4` — a register that exists only inside the asm block's own `move $12,%0` preamble — with temps
`$t5`/`$t6`, and `local-alloc.c:2249 find_free_reg` allocates hard regs in numeric order with
`$v0`/`$v1` free, so no C temp can reach `$t5`/`$t6`. The measured distance between this function
and COMPLETED-C is therefore now exactly the policy question in the entry above, with no remaining
codegen residual attached to it. Best ban-compliant form (two do-while(0) FAKE wraps, not a
submission): memory/grind/func_800300B4/best_ban_compliant.c. Evidence: evidence.md s3 section.

### 2026-09-02 addendum (session s6) — the only non-policy escape route is now measured closed
The entry above listed three re-activation triggers, two of them owner rulings and one a toolchain
finding: "a toolchain finding that lets GCC 2.7.2 base a C-side halfword pair on the asm-internal
$t4 copy and seat its temps in $13/$14 with $2/$3 free". Session s6 measured that trigger dead, so
the question in this entry is now the *whole* remaining distance between func_800300B4 and
COMPLETED-C, with no alternative route for a future session to take instead.

Evidence (memory/grind/func_800300B4/hypotheses.md H29/H30; dumps
tmp/grind/func_800300B4/s6/suggdbg_all.txt and suggdbg_packhigh.txt, produced by the instrumented
cc1 with BB2_SUGG_DEBUG=1): `find_free_reg` has exactly one bypass of ascending numeric hard-reg
order — the `just_try_suggested` restriction to `qty_phys_copy_sugg`/`qty_phys_sugg`
(tools/gcc-2.7.2/local-alloc.c:2207-2213); the ascending scan at :2249 takes the `int regno = i;`
arm because MIPS defines no REG_ALLOC_ORDER (regclass.c:112). The three island-2 pack quantities
carry `ncopysugg=0 nsugg=0`, so the bypass never runs for them, and every suggestion recorded
anywhere in this function is one of {4,5,6,7,30} — suggestions come only from copies between a
pseudo and a hard register, and nothing but hardcoded-$N asm ever copies to $t5/$t6. The FFR lines
show $13/$14 free and simply passed over (`qty=4 ... used=0,1,4,26..67` -> got $2). A contention
probe that makes $v0/$v1 busy across the pack (the explicit conditional in the original predicate,
never previously tested) moves the pack quantity exactly two registers, $3 -> $5, and scores 9
against the baseline 7.

Status of the two remaining triggers is unchanged: (a) an owner ruling that condition 3 admits an
SDK GTE-macro body; (b) an owner ruling making the func_800203B4 / func_8002E838 / func_80031890
islands citable precedent for cluster siblings. Bytes unchanged: `candidate.c` = sandbox 0 (83/83)
and verify-oracle ok; ban-compliant floor = 7 (best_ban_compliant.c, one sanctioned do-while(0)
wrap whose necessity is itself a class kill at local-alloc.c:1666, hypotheses.md H27).

### 2026-09-02 — func_800300B4 — s7 addendum (solver modality): the residual is DOUBLY locked, and the first lock is cse.c, not the register allocator

The policy question in this entry is unchanged, but the evidence behind it was incomplete: every
session from s1 to s6 attributed the 7-insn ban-compliant residual to the register allocator. The
solver triage that the `solver` modality mandates was run for the first time in s7 and re-types it.

`python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_800300B4 --target-object
build/src/code6cac_b.o --ours-object tmp/sandbox/func_800300B4/code6cac_b.o` reports
**FIRST DIVERGENCE: PRE-RA** — the register-blanked instruction multisets differ (ours
`lhu #,44(#)` + `lhu #,48(#)`, target `lhu #,0(#)` + `lhu #,4(#)`), which no RA or scheduler
perturbation can express. The cc1 `-da` dumps name the owning pass in one read: `.rtl` carries
`(mem:HI (reg 76))` and `(mem/s:HI (plus (reg 76) 4))` — i.e. the FRONT END already emits exactly the
target's addressing shape through the `lv` pointer — and `.cse` carries
`(mem:HI (plus (reg 72) (const_int 44)))`.

The predicate is `find_best_addr` (tools/gcc-2.7.2/cse.c:2622, reached from `fold_rtx`'s MEM case at
cse.c:5034): it walks the address's equivalence class, takes the lowest `ADDRESS_COST` and breaks
ties by the HIGHEST `rtx_cost` (cse.c:2717-2726). On MIPS `ADDRESS_COST` is 1 for a bare REG
(config/mips/mips.h:2897) and `mips_address_cost` is also 1 for `(plus reg SMALL_INT)`
(config/mips/mips.c:1653-1654), so the two tie and the rtx_cost tiebreak at cse.c:2720
unconditionally hands the win to the PLUS form — for any pointer whose equivalence class contains a
`(plus reg CONST_INT)` entry, i.e. for every semantically-correct spelling of `arg0 + 0x2C`.

Six spellings were measured and re-classified (pointer named, pointer indexed, `&lv[i]` sub-word
reads, a named `u16 *` off arg0, and a definition-order control), plus the FAKE-free control: all
score 7 (FAKE-free 19) and all classify PRE-RA with the identical multiset delta. A deliberately
semantics-changed positive control that bases the reads on `mat` — a pointer LOADED FROM MEMORY, so
cse holds no `(plus reg const)` entry for it — flips the classification to `RA` with a MATCHING
multiset, isolating the predicate exactly.

That positive control also states the whole residual in one block:

    ours  : addiu a1,s3,44 ; lhu v0,4(a0) ; lhu v1,0(a0) ; move t4,a1 ; mtc2 v1,$0 ; or v1,v1,v0 ; sll v0,v0,0x10
    target: addiu v0,s3,44 ; lhu t5,0(t4) ; lhu t6,4(t4) ; move t4,v0 ; mtc2 t5,$0 ; or t5,t5,t6 ; sll t6,t6,0x10

So the residual carries TWO independent, individually sufficient locks:

1. **PRE-RA, cse.c:2720** — every semantically-correct C spelling renders the two halfword loads as
   `lhu 44($s3)` / `lhu 48($s3)`, never `lhu 0(reg)` / `lhu 4(reg)`.
2. **RA, local-alloc.c:2207 / :2249** (s6 H29) — even given the correct displacements, the loads
   would be based on a C pseudo seated in `$a0`/`$v0`, never on `$t4`; and `$t4` is written only by
   the island's own `move $12, %0`.

The bearing on the question this entry asks is direct. The target's two halfword loads are based on
a register that only the asm template defines, and the C the compiler is given already produces the
right RTL before cse rewrites it. The only instruction stream that can produce the target's bytes is
one in which the `lhu/lhu/sll/or` are emitted inside the `gte_ldlv0` template — which is what the
PsyQ 4.5 `inline_c.h:101-110` macro body does verbatim. Whether cluster condition 3
(`.claude/rules/cop2-addressing-preamble-cluster.md:106`) admits that template text remains the
owner's call; s7 adds only that no C-side route to those bytes exists, now measured at both layers
rather than one.

Evidence: `memory/grind/func_800300B4/hypotheses.md` H31/H32/H33; classifier reports
`tmp/grind/func_800300B4/s7/classify_base.txt` and `cls_v_*.txt`; cc1 `-da` dumps
`tmp/grind/func_800300B4/dumps/code6cac_b.{rtl,cse,greg}` with the per-pass scan
`tmp/grind/func_800300B4/s7/scanpass.py`; sandbox scores `tmp/grind/func_800300B4/s7/sb_v_*.txt`.

### 2026-09-02 — func_800300B4 — s8 addendum (forensics modality): the cse lock's source-shape enumeration is complete, and breaking that lock is measured to buy nothing

Not a re-request. The policy question in this entry is unchanged; this addendum only tightens the
evidence behind "the C route is closed", so the owner sees a finished proof rather than a partial one.

1. **The cse layer is now enumerated to exhaustion, both branches of find_best_addr.** s7 pinned the
   pack's 44/48 displacements to branch one — the ADDRESS_COST tie plus the rtx_cost tiebreak at
   tools/gcc-2.7.2/cse.c:2717-2726. That tiebreak uses a strict `>`, so it only defeats a BARE-REG
   incumbent; two `(plus REG CONST_INT)` addresses tie and the incumbent would survive. That escape
   is reachable in ordinary C with byte-identical semantics, and s8 built and measured it:
   `hb = (u16 *)(arg0 + 0x28); packed = hb[2] | ((u32)hb[4] << 16);` (and a second base at 0x20).
   Both score 7, and both emit an island-2 window byte-identical to the banked best form. The `-da`
   dumps (tmp/grind/func_800300B4/s8/dumps_v_d28/) show the pointer really does materialise in
   `.rtl` — `(mem/s:HI (plus (reg/v:SI 78) …))` — and is then rebased onto arg0 in `.cse` by
   find_best_addr's SECOND branch, the `flag_expensive_optimizations`-gated REG+const associative
   merge at cse.c:2750 (selection loop cse.c:2793-2807), which folds `(plus reg72 0x28)` and the
   index constant together into `(plus reg72 0x2C)`. Zero displacement loses branch one; nonzero
   displacement loses branch two; every semantically-identical spelling of this pack has the address
   `arg0 + literal`, so one branch or the other always fires. (hypotheses.md H35, class kill,
   predicate cse.c:2750.)

2. **Even a body with that lock fully broken still measures 7.** s7's positive control
   `v_probe_matbase` changes semantics so the pack's base is loaded from memory; its cse class then
   holds no `(plus reg const)` entry, `inverse_compose.py classify` flips PRE-RA → RA, and the
   register-blanked instruction multiset MATCHES the target. Its honest score is nonetheless **7**
   (tmp/grind/func_800300B4/s7/sb_v_probe_matbase.txt) — identical to every ban-compliant form ever
   measured on this chassis. Its remaining diff is seats only, `lhu t5,0(t4) ; lhu t6,4(t4)` against
   `lhu v1,0(a0) ; lhu v0,4(a0)`, and those seats are $t4/$t5/$t6 — which no C pseudo can occupy,
   because $t4 is written only by the island's own `move $12, %0`. So the two locks are not merely
   independent; the RA lock alone holds the number at 7. (hypotheses.md H36.)

Net: the question the owner is being asked is unchanged and is the whole remaining distance —
does cop2-addressing-preamble-cluster.md condition 3 admit the verbatim PsyQ 4.5 `gte_ldlv0` SDK
macro body, whose lhu/lhu/sll/or is template text rather than an addressing preamble? What s8 adds
is that the alternative (respell the pack in ordinary C) is now closed at the cse layer by a complete
enumeration of both find_best_addr branches, on top of the local-alloc closures of s5/s6, and that
succeeding at the cse layer would not have moved the floor anyway.

### 2026-09-02 — func_800300B4 — s9 addendum (forensics modality): the RA lock's register-pressure axis is closed by a reference-budget argument, and the seat model is now predictive

The s7/s8 addenda closed the pre-RA (cse.c) lock and showed breaking it buys nothing. s9 closes
the last open axis on the RA side of the same residual, so the island-2 pack is now understood
end to end rather than merely observed.

1. `REG_ALLOC_ORDER` is not defined for the MIPS back end (`tools/gcc-2.7.2/config/mips/mips.h`),
   so `find_free_reg` takes the `#else` arm at `tools/gcc-2.7.2/local-alloc.c:2251` and scans hard
   registers in plain numeric order. Verified against every quantity of two whole compilations:
   the seat is exactly the lowest-numbered register not in `first_used`.
2. In the ban-compliant 7-form the island-2 pack quantities are allocated $2/$3/$2 while $5..$11
   and $13..$15 are demonstrably free at those calls (`tmp/grind/func_800300B4/s9/suggdbg_region.txt`).
   The target's pack sits in $t5/$t6 based on $t4 — registers the C side can only reach if regnos
   2..12 all conflict.
3. Making them conflict is measured inert and then closed as a class (hypotheses.md H38/H39): a
   maximal ordinary-C pressure form scores 52 with the pack seats unchanged at $2/$3, because
   `qty_compare_1` (local-alloc.c:1666) ranks the 3-instruction pack temp at priority 1.333 and
   allocates it 4th of 19, ahead of anything long-lived enough to span it. Reaching regno 13 would
   require nine competitors each live across the pack's 3-instruction range with >= 5 references
   inside it — >= 45 register references in a window that can carry at most ~9.

Net effect on this entry's question: unchanged in substance, stronger in evidence. Both halves of
the doubly-locked residual — cse.c find_best_addr on both branches (s7 H32, s8 H35), and
local-alloc on all three of its decision points (s5 H27 priority, s6 H29 suggestion bypass, s9 H39
ordering + reference budget) — are now enumerated to exhaustion with named predicates. The whole
remaining distance between func_800300B4 and COMPLETED-C is the owner's cluster-condition-3
reading recorded at the head of this entry, and no source-side input to any pass remains untried.

### 2026-09-02 owner ruling — the func_800300B4 and func_8002FF20 entries above are SPENT
Owner ruling 2026-09-02 (decisions.md "2026-09-02 — OWNER RULING", Ruling A) answers the
condition-3 question YES: the verbatim PsyQ macro body (incl. gte_ldlv0's lhu/lhu/sll/or pack)
is the template. Executed: allowlist lines for func_8002E838 / func_80031890 (mislabeled
COMPLETED-C on 2026-09-02 01:27/01:34 — reclassified COMPLETED-INLINE-ASM-CANONICAL); registry
rows for func_80031890 + func_8002FF20 (named in the 2026-09-01 widened-anchor grant);
`grant_rescan --apply` on func_800300B4 + func_8002FF20 (bans superseded, returned to active).
Their candidates still pass layer-1, the Judge and full-build SHA1 before landing.

## 2026-09-02 — func_8002FF20 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002FF20 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:18082; row per owner ruling 2026-09-02)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-02 — func_800300B4 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_800300B4 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-02 — func_800325E0 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_800325E0 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-02 — func_800861BC — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-02 — func_800861BC — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_800861BC include/sound.h; function stays ACTIVE.

## 2026-09-03 — func_80033550 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-03 — func_80033550 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80033550 include/code6cac.h undefined_syms_auto.txt named_syms.txt; function stays ACTIVE.

## 2026-09-03 — func_80062020 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-03 — func_80062020 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80062020 include/game.h src/text1b_b.c; function stays ACTIVE.

## 2026-09-03 — func_80062020 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-03 — func_80062020 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80062020 include/game.h src/text1b_b.c undefined_syms_auto.txt  (merged with prior line); function stays ACTIVE.

## 2026-09-03 — func_80062020 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-03 — func_80062020 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): cleared 2 superseded ban(s); function stays ACTIVE.

## 2026-09-04 — get_alarm — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-04 — get_alarm — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: get_alarm volatile_extern_allowlist.txt; function stays ACTIVE.

## 2026-09-04 — main (src/ings.c) — policy-question
category: policy-question
evidence: docs/grind/decisions.md 2026-09-04 OWNER RULING (foreclosed-bucket re-evaluation), Ruling D. Residual = two branch-displacement words at 0x80017494 / 0x800174B4 (memory/grind/main/evidence.md:1467-1516, whole-EXE verified; sandbox --disable all = 0, 189/189, zero rules, zero cheat constructs). Mechanism pinned three ways to cc1 reorg's redundancy thread-skip retargeting UNFILLED branches (reorg.c:3433 -> redundant_insn -> reorg_redirect_jump; instrumented-cc1 DBRDBG trace tmp/grind/main/s2/dbr_trace.txt; cc1psx on the identical ings.i emits one label with all seven branches on it, tmp/grind/main/s2/main_psx.s). The 2026-08-24 DECLINE (decisions.md:10303) rejected a from-scratch ASPSX fill reimplementation + -fno-delayed-branch; the narrower remedy — a PER-FUNCTION-gated maspsx post-pass that re-emits the pre-fill label for reorder-mode (unfilled) branches whose target label is immediately preceded by insn P where P is verbatim the delay fill of another branch to that label ("retarget iff filled" ASPSX parity; opt-in list, same mechanism as maspsx_label_nop_funcs.txt) — was never built or scoped. Read-only census over TARGET asm (scan script retained by the operator, 2026-09-04): 761 fill-dup sites; 44 main-like; 36 sites in 33 matched functions where target legitimately sits on the POST-P label, so a GLOBAL rule would break ~33 matched functions and only the per-function form is a candidate. Against: selectivity comes almost entirely from the opt-in list (the pattern precondition holds at 761 sites), which resembles the retired label regfix rules more than the label-nop gate; the owner declined the class once (2026-08-24) and affirmed twice (2026-09-01, 2026-09-02); exactly one proven beneficiary (main; the 11 census siblings are INCLUDE_ASM with RA/scheduler residuals far above 2). For: the cc1psx exhibit is evidence about ASPSX/maspsx fidelity on one label-placement behaviour (NOT a "toolchain is the variable" claim — no-compiler-divergence.md forbids that framing and this entry does not adopt it); the transform is narrow, semantically neutral and oracle-enforced; the label-nop precedent (2026-09-02, func_80027640) is the same "model the original assembler's output" shape. Rule status: .claude/rules/no-compiler-divergence.md §2 makes a NEW per-function maspsx gate category an owner ruling; .claude/rules/maspsx-gate-lists.md classifies gates as fidelity only when no C spelling exists (probe-proven: 7 escape hatches enumerated s2/s33, each byte-visible or a forbidden family; permuter blind because engine/score.py masks branch targets), the transform is narrow, semantically neutral and oracle-enforced, and growth carries the [infra-rule] tag. canonical-asm gate: scan_hand_coded LOW 0/8 (refused, unchanged).
disposition taken: main STAYS FORECLOSED under its own records (2026-08-24 DECLINE; affirmed 2026-09-01 and 2026-09-02). Nothing granted. The decidable question is presented to the owner in the session report for their own hand: "Is a per-function-gated maspsx 'retarget iff filled' parity post-pass (opt-in list, oracle-enforced, main as first entry) a FIDELITY gate under maspsx-gate-lists, or cheat-by-config?" Only a subsequent explicit owner ruling landed per ruling-record-lands-before-code spends this entry. If the owner rules cheat-by-config, main's re-activation trigger A ("census candidates regrow") must be reworded — with the rule files retired nothing can regrow it, so as written it can never fire.
SPENT by owner ruling 2026-09-04 (decisions.md "OWNER RULING — `main`: the per-function maspsx prefill-label gate is a FIDELITY gate; build it"): the owner answered the plain-terms question "legitimate tool fix: build it"; the gate is implemented as maspsx_prefill_label_funcs.txt (fidelity class) with main as first entry; main proceeds through the manual completion path.

## 2026-09-05 — func_80034F88 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-05 — func_80034F88 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80034F88 include/code6cac.h src/code6cac.c undefined_syms_auto.txt; function stays ACTIVE.

## 2026-09-05 — func_80022F34 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-05 — func_80022F34 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80022F34 include/code6cac.h src/code6cac.c; function stays ACTIVE.

## 2026-09-05 — func_80022F34 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-05 — func_80022F34 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_80022F34/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-09-05 — func_800480C0 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-05 — func_800480C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_800480C0/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-09-06 — func_8004473C — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-06 — func_8004473C — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_8004473C include/game.h undefined_syms_auto.txt named_syms.txt; cleared 1 superseded ban(s); function stays ACTIVE.

## 2026-09-06 — _addque2 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-06 — _addque2 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: _addque2 volatile_extern_allowlist.txt include/gpu.h undefined_syms_auto.txt named_syms.txt; function stays ACTIVE.

## 2026-09-06 — func_80019310 — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/decisions.md 2026-09-06 entry 'func_80019310 (src/code6cac.c) - blocked at the operator registry row (bytes PROVEN, Judge PASS on record, LOW scan tier)': operator step = add the func_80019310 row to tools/grinder/owner_cluster_grants.txt (2026-09-01 widened-anchor grant, decisions.md:18119) then unpark; see the entry's Routing note.
disposition taken: FORECLOSED silently (owner ruling 2026-08-31 — no pending states, no packets); re-attemptable if a later owner ruling spends this entry.

## 2026-09-06 — func_8002C61C — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-06 — func_8002C61C — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_8002C61C/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-09-06 — func_80019310 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_80019310 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:18082; row per owner ruling 2026-09-06 foreclosed-bucket review)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-06)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-07 — func_8007526C — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/decisions.md:25231 — 2026-09-07 — func_8007526C — OWNER-ESCALATION: OWNER-ONLY GATE-LINE REMEDY (honest floor 1; one line in maspsx_label_nop_funcs.txt)
disposition taken: FORECLOSED silently (owner ruling 2026-08-31 — no pending states, no packets); re-attemptable if a later owner ruling spends this entry.

## 2026-09-08 — func_800204C0 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-08 — func_800204C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_800204C0/candidate.c; re-attemptable if a later owner ruling spends this entry.
