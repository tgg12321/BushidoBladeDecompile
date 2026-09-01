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
