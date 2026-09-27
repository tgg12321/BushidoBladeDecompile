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

## 2026-09-08 — func_8002D780 — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/decisions.md:26057
disposition taken: ROTATED (owner ruling 2026-09-08 rotation-not-foreclosure — no pending states, no packets; returns automatically).

## 2026-09-08 — func_800335D8 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-08 — func_800335D8 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_800335D8 include/code6cac.h undefined_syms_auto.txt named_syms.txt; function stays ACTIVE.

## 2026-09-10 — func_80018094 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-10 — func_80018094 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; ROTATED to the back of the active worklist (owner ruling 2026-09-08, rotation-not-foreclosure; returns automatically); candidate preserved at memory/grind/func_80018094/candidate.c.

## 2026-09-10 — func_80074B18 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-10 — func_80074B18 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; ROTATED to the back of the active worklist (owner ruling 2026-09-08, rotation-not-foreclosure; returns automatically); candidate preserved at memory/grind/func_80074B18/candidate.c.

## 2026-09-10 — func_8002EBDC — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/decisions.md 2026-09-10 — func_8002EBDC (src/code6cac_b.c) — blocked at the operator registry row (bytes PROVEN 0/182, Judge PASS 1b44e6afef0b58e6 on record, scan tier LOW; the whole remedy is one row in tools/grinder/owner_cluster_grants.txt, then queue unpark + resubmit candidate.c exactly)
disposition taken: ROTATED (owner ruling 2026-09-08 rotation-not-foreclosure — no pending states, no packets; returns automatically).

## 2026-09-11 — _spu_gcSPU — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-11 — _spu_gcSPU — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; ROTATED to the back of the active worklist (owner ruling 2026-09-08, rotation-not-foreclosure; returns automatically); candidate preserved at memory/grind/_spu_gcSPU/candidate.c.

## 2026-09-14 — func_80018094 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-14 — func_80018094 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; ROTATED to the back of the active worklist (owner ruling 2026-09-08, rotation-not-foreclosure; returns automatically); candidate preserved at memory/grind/func_80018094/candidate.c.

## 2026-09-14 — func_80018094 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-14 — func_80018094 — JUDGE ESCALATE on final call (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; ROTATED to the back of the active worklist (owner ruling 2026-09-08, rotation-not-foreclosure; returns automatically); candidate preserved at memory/grind/func_80018094/candidate.c.

## 2026-09-15 — func_80063BD0 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-15 — func_80063BD0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80063BD0 include/game.h src/text1b_b.c undefined_syms_auto.txt; cleared 1 superseded ban(s); function stays ACTIVE.

## 2026-09-15 — _clr — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-15 — _clr — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: _clr include/gpu.h undefined_syms_auto.txt; function stays ACTIVE.

## 2026-09-15 — func_80054604 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-15 — func_80054604 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80054604 include/game.h src/text1b_b.c undefined_syms_auto.txt; function stays ACTIVE.

## 2026-09-15 — func_80076D74 — integration-handoff
category: integration-handoff
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-15 — func_80076D74 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): scope grant: func_80076D74 include/game.h src/text1b_b.c undefined_syms_auto.txt; function stays ACTIVE.

## 2026-09-15 — func_8002D780 — canonical-asm-grant (REFUSED: tier LOW, not STRONG-class and not owner-cluster-enumerated)
category: canonical-asm-grant (REFUSED: tier LOW, not STRONG-class and not owner-cluster-enumerated)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-15 — func_8002D780 — JUDGE ESCALATE on final call (canonical-asm-grant) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; ROTATED to the back of the active worklist (owner ruling 2026-09-08, rotation-not-foreclosure; returns automatically); candidate preserved at memory/grind/func_8002D780/candidate.c.

## 2026-09-20 — func_80018094 — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/owner_actions.md:10-11 (## 2026-09-20 — func_80018094 — merge-refused-islands)
disposition taken: ROTATED (owner ruling 2026-09-08 rotation-not-foreclosure — no pending states, no packets; returns automatically).

## 2026-09-20 — func_800747D8 — policy-question
category: policy-question
evidence: session-filed disposition: docs/grind/decisions.md:28568
disposition taken: ROTATED (owner ruling 2026-09-08 rotation-not-foreclosure — no pending states, no packets; returns automatically).

## 2026-09-21 — func_8002D780 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002D780 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; census row :75; row per owner Ruling 3 terms, 2026-09-15, decisions.md:27024; operator-added 2026-09-21)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-21)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-21 — func_8002EBDC — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002EBDC tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; census row :80; row per owner Ruling 3 terms, 2026-09-15, decisions.md:27024; operator-added 2026-09-21)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-21)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-22 — func_8003FA24 — family-candidate
evidence: memory/grind/func_8003FA24/rejected/half-multiwrite-carrier.c (sandbox 0, full-build SHA1 == oracle) + .md; manual-lane layer-2 cheat-reviewer FAIL. The only closing form found stages the in-loop packet tag constants (4/3/2) through a fresh `s16` local written 11x (`half = 4; *packet++ = half; ...`), so loop.c (threshold 58, inner loops 50/46 real insns) never admits them as movables. In the target the tag carrier (v0) is a different pseudo from every existing variable (flags = a2), and s32 carriers are CSE-folded. Ruled out under the y1/`c` multi-write-carrier lineage (decisions.md:1833, :16474).
disposition taken: not committed; func_8003FA24 stays INCLUDE_ASM/active at floor 17 (plain stores) with every other construct cleared by layer-2. Question for the owner: is a fresh HImode staging local whose every write is a real, consumed store value (no dead writes) inside or outside the multi-write-carrier ban?
resolution (owner, 2026-09-22): AUTO-REJECT per escalation-not-parked (no SOTN precedent + backsliding). SOTN master scan (tmp/sotn-decomp @ aa53500, tmp/fa24/sotn_scan.py): zero instances of one local assigned >=2 constants each immediately stored; the literal-then-store shape occurs only twice tree-wide, never repeated. The nearest hit (src/dra/8D3E8.c var_v1) is a per-switch-arm shared temp written once per path, not straight-line staging. The fresh multi-set staging carrier stays banned for func_8003FA24 by any spelling; the function keeps grinding for an authentic source shape.

## 2026-09-23 — func_8006B120 — family-candidate
category: family-candidate / needs-user-downgrade
evidence: memory/grind/func_8006B120/rejected/ruling5-loop-selector-carrier.c (sandbox 0/278, full-build SHA1 == oracle) + memory/grind/func_8006B120/hypotheses.md; manual-lane layer-2 cheat-reviewer FAIL (2026-09-23). The only closing form reuses one function-scope `p1` at five sites, each `p1 = s.p0 + 0xC; s.p1 = p1;` (the prelude, then one in each of four do-while loops). Receipts: one local per site 11/278 (per-write-11.c), static-inline helper 11/278, allocation dump (the shared pseudo goes to global.c, conflicts v0/a0 -> v1 as in the target; per-site pseudos are local-alloc'd to v0), permuter carrier-free-11 (53,192 iterations, best 65 vs 245, every find a rec0 multi-write borrow). Layer-2 FAILed it under Ruling 5 prong 1(b): the record is chosen through a loop-index subscript (`s.p0 = tbl[i + k];`) and handed over via the struct member, not a constant subscript in the write or in a per-block binding. Also 1(d): the prelude write is at function scope, not in a sibling block. Every other construct cleared.
disposition taken: not committed; func_8006B120 stays INCLUDE_ASM/active at floor 11 (per-write form). Question for the owner: does Ruling 5 extend to a reused local whose writes are textually identical (`p1 = s.p0 + 0xC;`), where the sibling record is picked just before the write by a struct-member store with a loop-index subscript (`s.p0 = tbl[i + k];`), and one write sits at function scope rather than in a sibling block?
resolution (owner, 2026-09-23): EXTENDED — the owner selected "Yes, extend Ruling 5" (option text quoted verbatim in decisions.md 2026-09-23 OWNER RULING — Ruling 5 extension). The author's narrowed rule text (not the owner's words): identical-text writes and consumers; record picked beforehand by a direct member store from one and the same array at every site, whose subscript is a constant or the innermost enclosing loop's counter plus an optional constant; at most one function-scope site. Rule text: .claude/rules/ordinary-c-judge-decidable.md "Ruling 5 extension". func_8006B120's reuse form is re-submitted to a fresh layer-2 under the extended text.

## 2026-09-23 — func_8001FBE8 — family-candidate
category: family-candidate / policy-question
evidence: memory/grind/func_8001FBE8/rejected/shared-rec-ruling5-0.c (= candidate.c, sandbox 0/289; the same body before two byte-neutral simplifications reached full-build SHA1 == oracle) + evidence.md/hypotheses.md; manual-lane layer-2 cheat-reviewer FAIL x2 (2026-09-23). Every other construct is cleared. The only closing form uses one function-scope `u8 *rec` in two mutually exclusive paths: `rec = &D_80101EC8 + D_800A3758 * 0x44C;` in the call-free D_800A3758 != 0xFF block (which returns), and `rec = &D_80101EC8 + i * 0x44C;` in the loop. The target keeps the call-free block's pointer in callee-saved $s1, which is only explained by a shared pseudo with the loop's call-crossing pointer. Receipts: split locals score 14/289 (function-scope, block-local, and else-if-restructured; every scored hunk is operand-only s1 vs a1). Layer-2 FAILed it under Ruling 5 prongs 1(a)-(d), since the two writes differ by more than a constant subscript and each is read many times. Precedent (read, not just located): SOTN src/dra/8D3E8.c (PSX DRA) :1311/:1362-1373 and :1422/:1474-1483 write one local `var_v1` once per exclusive switch arm, with a different base per arm (`D_800B0A5C[..]`, `D_800B0A6C[..]`, `D_800B0A7C[..]`). The 2026-09-22 func_8003FA24 resolution already distinguished this "written once per path" shape from banned straight-line staging.
disposition taken: not committed; func_8001FBE8 stays INCLUDE_ASM/active at split-local floor 14. Question for the owner: is one pointer variable reused across mutually exclusive code paths (each path writes it once, then reads it throughout that path) ordinary C, or a Ruling 5 multi-write carrier?
resolution (owner, 2026-09-23): ALLOWED WITH LIMITS. The owner answered "Agree, go ahead" to the author's four limits (quoted verbatim in the rule file). Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 6"; record in decisions.md 2026-09-23 OWNER RULING — Ruling 6. func_8001FBE8 is re-submitted to a fresh layer-2 under that text.

## 2026-09-23 — sprintf — family-candidate
category: family-candidate
evidence: memory/grind/sprintf/rejected/sotn-args-frame-walk-0.c (sandbox 0/535, full-build SHA1 == oracle with the rodata re-attribution in memory/grind/sprintf/evidence.md) + evidence.md 2026-09-23; manual-lane layer-2 cheat-reviewer FAIL (2026-09-23). The only closing form is SOTN's verbatim `bufPtr = (char*)&args - sizeof(printf_info) - 4;` (SOTN src/main/psxsdk/libc/sprintf.c:88-90, PSX psxsdk, comment "Need to use args to force args on the stack"): it takes the va_list's address and walks back over `info` to land on `&buf[0x200]`. Layer-2 FAILed it as cross-object address derivation between locals (#5). Receipts: the truthful `bufPtr = &buf[sizeof(buf)]` = 96/535 because args then lives in a register, while the target keeps it at sp+0x220 with $s7 free (so not a spill); five stdarg spellings 82-96. Note docs/grind/sotn-family-surveys-2026-08-18.md:287 classified this same SOTN line as "varargs frame walking in imported PsyQ library code, not a named-symbol crossing". Every other construct is cleared.
disposition taken: not committed; sprintf stays INCLUDE_ASM/active. Question for the owner: is SOTN's `&args`-relative buffer pointer in PsyQ's sprintf (a pointer to the end of the local buffer, computed from the address of the adjacent va_list local) acceptable, or banned as cross-object derivation?
resolution (owner, 2026-09-23): ALLOWED FOR sprintf ONLY. The owner selected "Allow it, sprintf only". Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 7"; record in decisions.md 2026-09-23 OWNER RULING — Ruling 7. sprintf is re-submitted to a fresh layer-2 under that text.

## 2026-09-24 — naming sweep — name-drift alias table — policy-question
category: policy-question
evidence: naming sweep 2026-09-24 (docs/naming/sweep-2026-09-24/) renamed functions whose rulings / journal entries were filed under the old names; engine/dossier.py aliases() resolves them from this table.
disposition taken: alias table recorded here; no policy change.
  cdrom_IsIdle = func_80036D88 · cdrom_StartRead = replay_camera_Init · cdrom_Pause = game_FrameInit ·
  cdrom_StartAudio = func_80036FD4 · cdrom_ReadWait = func_800372F4 · bitstream_ReadBits = func_8003D888 ·
  gpu_AddDrawMove = func_800401CC · math_RotMatrixZXY = func_80042874 · math_RotMatrixXYZ = func_80042C80 ·
  gpu_OffsetTexPolyFT3 = func_80043BD0 · gpu_OffsetTexPolyFT4 = func_80043C7C · gpu_OffsetTexPolyGT3 = func_80043D34 ·
  gpu_OffsetTexPolyGT4 = func_80043DE0 · gte_SetMatrixRotTransIR = func_80052A20 · PCclose = func_800836B8 ·
  __main = func_80083794 · func_80083804 = motion_Close

## 2026-09-25 — naming sweep (INFERRED-name audit) — name-drift alias table — policy-question
category: policy-question
evidence: naming sweep 2026-09-25 (docs/naming/sweep-2026-09-25/) renamed or RESET functions whose rulings / journal entries were filed under the old names; engine/dossier.py aliases() resolves them from this table.
disposition taken: alias table recorded here; no policy change.
  pcdrv_LoadFile = file_LoadAll · pcdrv_LoadSectors = file_LoadSectors · func_80016768 = disp_SetFramebufferMode ·
  func_8003032C = cpu_get_dist · func_800467B8 = snd_LoadBgm · func_800468B0 = snd_PlayBgm ·
  func_80047E5C = snd_GetFadeCurve · math_Length3D = func_80052720

## 2026-09-25 — func_80073C78 — compiler PLUS->IOR patch vs target `ori` — policy-question
category: policy-question
evidence: memory/grind/func_80073C78/evidence.md ("The last 2 insns" + "Stock-cc1 reproduction recipe") + candidate.c (honest `+` body, floor 2/362) + rejected/ior-spelling-patch-workaround.c (sandbox 0, full-build SHA1 == oracle; manual-lane layer-2 cheat-reviewer FAIL 2026-09-25, tests 2 and 3: `u | du0` beside `u + du1` is a source-level workaround for tools/cc1-no-plus-to-ior.patch). The target has `ori a1,v1,0x0` / `ori a0,v0,0x0` at the UV stores. Stock GCC 2.7.2 (pinned upstream 43d1cdb6, documented recipe, research-only build in the WSL home dir, never installed) turns the natural sibling-identical `u + du0` (du0 an always-0 local) into exactly those bytes: combine rewrites PLUS->IOR, then reload substitutes du0's constant 0. Across text1b.c up to and including the function, the whole diff between that build and the oracle compiler is those two lines. The oracle compiler cannot produce `ori` from any `+` spelling. This is shipped-binary evidence for docs/ORACLE-COMPILER.md's OPEN QUESTION: the original compiler performed this rewrite.
disposition taken: not committed; func_80073C78 stays INCLUDE_ASM and is ROTATED (floor 2 with the honest `+` body). Question for the owner: "Our compiler has one deliberate change from standard GCC: it never turns `a + b` into `a | b`. This function's original machine code contains exactly that rewrite, and standard GCC reproduces it from the natural `+` code. Should we revisit that compiler change, or keep it? If we keep it, this function stays unfinished rather than landing with a deliberately odd `|` in the C."
also noted (unrelated, 2026-09-25): the live tools/gcc-2.7.2/reorg.c no longer matches its SHA1 in the docs/ORACLE-COMPILER.md tracked-state manifest (the `sha1sum -c` check reports `reorg.c: FAILED`). tools/build_oracle_cc1.sh refuses to run until the tree is restored or the manifest is re-recorded with a rationale. This session did not modify the live compiler tree.
resolution (owner, 2026-09-25): PATCH KEPT, `|` REFUSED, STUDY AUTHORIZED. The owner approved the operator's recommendation ("Go ahead and do your recommendations then"). tools/cc1-no-plus-to-ior.patch stays, and the `|`-for-`+` spelling stays refused. A scratch-only study of a narrower patch is authorized; adopting one is NOT, and returns to the owner with evidence. func_80073C78 stays rotated. Rule text (the author's narrowing): docs/ORACLE-COMPILER.md § "Owner ruling 2026-09-25"; record in decisions.md 2026-09-25 OWNER RULING — oracle compiler.
superseded (owner, 2026-09-26, Q17): a compiler patch is a cheat. The PLUS->IOR patch and its adoption (9bc64b751) are superseded; the census (tmp/patchcensus/) found func_80073C78 matches under the unpatched compiler, and func_800174F4, the one function depending on the patch, is reverted and re-queued. Rule text: .claude/rules/no-compiler-divergence.md § "Owner ruling 2026-09-26 (ninth batch, Q17)"; record in decisions.md 2026-09-26 OWNER RULING — a compiler patch is a cheat (Q17). The two resolution lines below are history.
resolution (owner, 2026-09-25, second batch): ADOPTION AUTHORIZED, CONDITIONAL ON A SCAN. The study landed as ac5d2b0bb. The owner approved the operator's recommendation ("Go ahead with your recommendations"). First, the target binary is scanned for register-plus-register `or`/`addu` sites with provably disjoint operands. A compile-proven discriminating site selects between the narrow `(plus REG CONST_INT)` skip and `exprop`. If no site discriminates, narrow is adopted and `exprop` is recorded as a live alternative, and a later discriminating site reopens the choice for the owner. Required adoption steps: re-record the manifest (the 2026-08-24 reorg.c crash fix and the build/cc1 hash); fix the `--stock` self-check; commit the patch under tools/, replacing tools/cc1-no-plus-to-ior.patch; rebuild build/cc1 from the recipe; verify the oracle SHA1, engine test and fixtures-verify; let the toolchain-fingerprint auto-return run. Afterwards func_80073C78 may land from its honest `+` body, and `main`'s 2026-08-11 FAKE chain may be replaced by the natural spelling, each with its own layer-2. func_80073C78 stays rotated until then. Rule text (the author's narrowing): docs/ORACLE-COMPILER.md § "Owner ruling 2026-09-25 (second batch)"; record in decisions.md 2026-09-25 OWNER RULING — oracle compiler adoption (second batch).

## 2026-09-25 — func_800759D0 (and func_8007636C) — one role, differing constant offsets — policy-question
category: policy-question
evidence: memory/grind/func_800759D0/rejected/function-scope-q-multiwrite-0.c (sandbox 0/364, full-build SHA1 == oracle) + evidence.md ablation table; manual-lane layer-2 cheat-reviewer FAIL (2026-09-25). The only closing form so far uses one function-scope `s32 q`, the sprite image pointer stored to the render descriptor's `s.sp1C`: `q = s.sp18 + 0xC;` once at the head and `q = s.sp18 + 0x24;` in each of three loops, every write read by `s.sp1C = q;` in the same block. A q referenced in several blocks goes to global.c, which seats it in $a1 at every site as the target does. Receipts: one local per site 25/364 (rejected/per-site-q-split-25.c), head without q 3, loop 1 without q 22, loop 2 block-local q 30. Layer-2 FAILed it: Ruling 5 1(b) (the writes differ by a constant offset, not a selector subscript), 1(f) (single-letter name), extension (A) (not textually identical), Ruling 6 (A)/(C)/(D); "allocator effect alone is never sufficient". Every other construct cleared (`zero` constant-holder, `(D_8009BCF8 + i)->unk0`, `((u8 *)D_8009BCF8)[index]`).
also noted: func_8007636C (same TU, same S_80074488 descriptor) landed COMPLETED-C on main in d844de59a (2026-09-24) with the SAME construct — a function-scope `q` written at five sites with `+ 0xC` and `+ 0x24` — with no decisions.md entry. It should be flagged for re-audit under the current rule text. Its source was not touched by this session.
disposition taken: not committed; func_800759D0 stays INCLUDE_ASM/active at the per-site floor while other mechanisms are searched. Question for the owner: is one local reused for the same role (the pointer stored to the same struct member at every site) acceptable when the per-site writes differ only by a constant offset (`+ 0xC` vs `+ 0x24`), or does it stay a banned multi-write carrier?
resolution (owner, 2026-09-25): ALLOWED UNDER TIGHT CONDITIONS. The owner approved the operator's recommendation ("Go ahead and do your recommendations then"). The conditions: one meaning shown by layout evidence, one consumer, a descriptive name, and never a variable that changes meaning. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 9"; record in decisions.md 2026-09-25 OWNER RULING — Ruling 9. func_800759D0 is re-submitted to a fresh layer-2 under that text with `q` renamed. func_8007636C is re-audited under it with `q` renamed, and is not reverted ahead of that re-audit.

## 2026-09-25 — func_8008B488 — SOTN-verbatim shared ADSR `rate` local (Ruling 8 shape) — SETTLED (inadmissible; not an owner question)
category: policy-question (SETTLED 2026-09-25 by existing rulings; see resolution below)
evidence: memory/grind/func_8008B488/rejected/sotn-shared-rate-ruling5-0.c (sandbox 0/387; full-build SHA1 == oracle 2026-09-25 with the chassis in evidence.md: the compiler-emitted jtbl_80016460/80016480 replace the hand-transcribed const arrays, and `main` leaves RODATA_ALIGN2_FILES + its engine/buildconfig.py mirror because the target's zero word at 0x8001647C is final.c's `.align 3`) + evidence.md/hypotheses.md + rejected/. Not submitted to layer-2: the author judges it fails Ruling 5 as written. The function is PsyQ LIBSPU's SpuSetVoiceAttr body (older build than SOTN/psyz). The only closing form declares one `u16 rate` at function scope and uses it in the five ADSR blocks: `rate = attr->ar` / `->dr` / `->sr` / `->rr` / `->sl`, each followed by its clamp (`if (rate >= 0x80) rate = 0x7F;`, etc.) and its register write. This is SOTN's own `u16 var_a2`, shared the same way in its matched `_SpuSetVoiceAttr` (sotn-decomp src/main/psxsdk/libspu/s_sva.c; psyz decomp/src/libspu/sr_sv.c does the same). Receipts: one local per block 12/387 (either declaration order), all block-scoped 12, ternary clamp 12, s32 locals 15, SOTN if/else clamp 10 (392 insns); every scored hunk in the 12 forms is operand-only (SR: rate/smode seats swapped; SL: rate in a0, target a1). Author's reading (no allocator dump): SL's clamped value gets a1 only when its pseudo also spans a block where a0 is busy. Ruling 5 fails at 1(a) (the writes feed two different registers, ADSR1 and ADSR2), 1(b) (different field names), and 1(e) (the clamp constants). Ruling 8 is vmNoiseOn only.
disposition taken: not committed; func_8008B488 stays INCLUDE_ASM/active; the landable candidate.c scores 12/387 with the chassis (16 without it), the shared-`rate` form 0. Question for the owner (plain language): "This function is Sony's sound-library code for setting a voice's settings. It matches the original exactly, but only when one variable, `rate`, is shared by the five envelope-setting blocks. That is how SOTN's matched copy of the same Sony function is written. Every version with a separate variable per block is off by a few register choices. It's the same kind of question as vmNoiseOn's `temp`, which you allowed for that one function. Should this be allowed for func_8008B488 too, or as a general rule for SOTN-verbatim reuse in the same Sony library function?"
resolution (2026-09-25, manual s2): WITHDRAWN as an owner question. A layer-2 construct ruling (2026-09-25) found the shared `rate` INADMISSIBLE under existing rulings: it fails Ruling 5 prongs 1(a), 1(b), 1(c) and 1(e) and Ruling 6. Ruling 8 is explicitly vmNoiseOn-only and non-citable. The owner declined "allow as a class" on 2026-09-24 (decisions.md, Ruling 8 entry). The function-scope `adsr` written in five blocks fails independently. So this is settled, not pending the owner. func_8008B488 stays INCLUDE_ASM/active at the landable per-block floor (12/387 with the chassis, 16 without). Manual s2 allocator dump (memory/grind/func_8008B488/evidence.md, "Manual s2"): under global.c no one-pseudo-per-block spelling can close either residual. SL's value conflicts only with v0/v1, so it takes a0. SR's smode outranks the rate. The first permuter campaign's only find re-shared `sl_rate` with another block, which is evidence only.

## 2026-09-25 — prnt — original-library-source shared scratch `n` (BSD _doprnt) — policy-question
category: policy-question
evidence: memory/grind/prnt/rejected/single-n-reno-verbatim-0.c (sandbox 0/418; full-build SHA1 == oracle 2026-09-25 with the chassis in memory/grind/prnt/evidence.md: keep D_80015A68/7C/84 as named arrays, delete jtbl_80015A98) + evidence.md ("Why the honest per-role form sits at 21", Reno excerpts with URL/sccsid). Manual-lane layer-2 cheat-reviewer FAIL 2026-09-25 on the single `n` only (Ruling 5 1(a)/(b)/(f); Ruling 6 n/a; Ruling 8 vmNoiseOn-only; owner declined original-source reuse as a class 2026-09-24); every other construct cleared. prnt is PsyQ LIBC2 PRNT, a putchar port of 4.3BSD-Reno _doprnt (doprnt.c 5.39, where `n` is declared "random handy integer" and used for the width/precision digits and all padding loops). Split per role = 21/418 (419 insns): the accumulators fall to $a1/$a2 because they never cross a call; the target has them in $s0 with the loop counters.
related: tonight's other multi-write rotations (func_800759D0 `q`, func_8008B488 `rate`, 2026-09-25 entries above) — same Ruling 5 wall; this one differs in that the reused variable is the original library source's own declared variable, not a SOTN or decomp-author spelling.
disposition taken: not committed; prnt stays INCLUDE_ASM/active at the honest per-role floor 21/418. Question for the owner (plain language): "prnt is Sony's copy of BSD printf. The original BSD code uses one scratch integer, `n`, for several unrelated jobs (reading the width and precision digits, and counting padding). Splitting it into one variable per job is 21 instructions off. Should a reused variable that copies the original library source's own variable be allowed? You turned this down as a general rule for vmNoiseOn."
resolution (owner, 2026-09-25): ALLOWED AS A NARROW VERIFIED-ORIGINAL-SOURCE EXCEPTION. The owner approved the operator's recommendation ("Go ahead and do your recommendations then"). The conditions: a public, pinned original source (never a decompilation), a transcription with every difference listed, and the original's own variable reused verbatim. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 10"; record in decisions.md 2026-09-25 OWNER RULING — Ruling 10. prnt is re-submitted to a fresh layer-2 under that text once its ledger carries the SHA-256 and second-copy provenance that prong (A) requires.

## 2026-09-25 — func_800288C8 — scorer strips verbatim header GTE statements — policy-question
category: policy-question
evidence: memory/grind/func_800288C8/rejected/tbl-copy-six-stmt-lzc-90.c (full-build SHA1 == oracle 2026-09-25; sandbox --disable all 90 because engine/inlineasm.py strips every __asm__ statement without a cop2 op, i.e. inline_o.h gte_ldlzc/gte_stlzc's `move $12,%0` and gte_nop's `nop`); memory/grind/func_800288C8/evidence.md "Why header-exact" (the four statements are 4 loop RTL insns that decide a global.c priority tie; joined islands floor at 8). Same wall: func_8002A458 (memory/grind/func_8002A458/evidence.md, header-exact islands fixed its rec/scr seats but were unscorable).
related: func_8002A458; func_80018300 (reached sandbox 0 with inline_o.h islands — lead not yet studied)
disposition taken: layer-2 FAIL (also on a `tbl` copy construct); not committed; function rotated. Question for the owner (plain language): "Sony's header writes this chip snippet as six lines. Our scorer deletes two `move` lines and two `nop` lines before comparing. Should those header lines count when scoring?"
resolution (owner, 2026-09-25): SCORER BUG, FIX AUTHORIZED. The owner approved the operator's recommendation ("Go ahead and do your recommendations then"). The sandbox is to keep and score the statements of a whole, verbatim, cop2-bearing PsyQ GTE macro expansion, and strip everything else as today. This is an engine fix pinned by `engine test`, with func_80018300 checked first. Scoring is not admission: the region grant, the auth row and an owner-instructed owner_cluster_grants.txt row are still required, and none is created for func_800288C8, which also failed on `tbl`. Rule text (the author's narrowing): .claude/rules/inline-asm-policy.md § "Scorer ruling (owner, 2026-09-25)"; record in decisions.md 2026-09-25 OWNER RULING — scorer.
resolution (owner, 2026-09-25, second batch): NO ROW NOW; `0(reg)` EQUALS `(reg)` WHEN SCORING. The owner approved the operator's recommendation ("Go ahead with your recommendations"). No per-function tools/grinder/owner_cluster_grants.txt row is granted for func_800288C8 now. It is granted, citing the ruling, when a body passes review: sandbox 0, full-build SHA1 == oracle, and a fresh layer-2 PASS that leaves the row as the only outstanding item. A body carrying the `tbl` copy gets none. Separately, the engine's GTE-macro unit recognizer treats a memory operand `0(reg)` as equal to `(reg)`, because maspsx cannot parse the header's `($12)` and the two assemble to identical bytes. Nothing else is normalized: any other offset, register or edit is still an edit. Rule text (the author's narrowing): .claude/rules/inline-asm-policy.md § "Scorer amendment (owner, 2026-09-25, second batch)"; records in decisions.md 2026-09-25 OWNER RULING — scorer amendment `0(reg)` ≡ `(reg)`, and 2026-09-25 OWNER RULING — func_800288C8 owner-cluster row.

## 2026-09-25 — func_800759D0 — Ruling 9 (b) on a latent-bug path — policy-question
category: policy-question
evidence: memory/grind/func_800759D0/rejected/ruling9-cells-placeholder-overrun-0.c (sandbox 0/364, full-build SHA1 == oracle; landed as 23045f51f under Ruling 9 after a layer-2 PASS, then reverted to INCLUDE_ASM after a combined layer-2 re-review FAILED it) + evidence.md "Ruling 9 FAIL". When the cursor is on an unavailable cell, func_80075F80 writes the placeholder 0x14 into arg2[f3C]. In the same frame, func_80077374 case 2 calls func_800759D0, whose loop 2 draws that slot: `cells = table[0x14 + 1] + 0x24`. D_SEL.BIN sheet [21] (0xC84) has 1 header and 2 cells and ends at 0xCA0, so +0x24 = 0xCA8 is past the record. Every other reachable write reaches the cell array of its sheet (the +0xC/+0x24 census in evidence.md "Ruling 9 (b)"). Ruling 9 (b) fails (BASE + K is not a sub-object at every write), and so does (f) (the "three headers" comment is false on that path). Per-site floor 25/364; a block-local loop-2 site 30.
related: func_8007636C (71b14499d, Ruling 9 re-audit PASS): its pick list holds only confirmed entries in cases 3-5, so it is not affected.
disposition taken: 23045f51f reverted (src back to INCLUDE_ASM; queue entry restored as rotated); the ledger was corrected. Question for the owner: "func_800759D0: the original code itself reads `table[21] + 0x24` past the end of a 1-header sheet on the unavailable-cell path, which looks like a latent bug in the original game. Ruling 9 (b) as written is judged on the actual data at every reachable write, so the body fails. Should (b) be judged by the layout the code assumes (loop 2 treats every sheet as 3-header) rather than by what the data happens to hold on a buggy path?"
resolution (owner, 2026-09-25, second batch): JUDGE (b) BY THE LAYOUT THE CODE ASSUMES, UNDER FOUR CONDITIONS. The owner approved the operator's recommendation ("Go ahead with your recommendations"). Ruling 9 prong (b), with the matching prong (f), is amended as (b′). A write is judged by the layout the code consistently imposes only when four conditions hold. (1) The code's own accesses (target instructions, other readers of the same object) establish the layout; argument does not. (2) Every write reaches a same-kind sub-object on every path shown to be normal. (3) Every reachable path where one does not is documented in the ledger as an anomaly in the original (bad data, placeholder, bug), with its trigger and the measured bytes. (4) The source comment describes what the code assumes, not a claim about all the data. (b′) does not admit a variable whose writes differ in meaning on normal paths, and it reopens no banned item. func_800759D0's reverted body may be re-submitted to a fresh layer-2 once its ledger and comment meet (b′); the outcome is not pre-decided. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Amendment (b′)"; record in decisions.md 2026-09-25 OWNER RULING — Ruling 9 amendment (b′).

## 2026-09-25 — func_800759D0 — (b′)(3) past-the-end into unreferenced data — policy-question
category: policy-question
evidence: memory/grind/func_800759D0/pending-bprime-0.c (sandbox 0/364; full-build SHA1 == oracle, spliced over INCLUDE_ASM at bcdc1648e) + evidence.md "Ruling 9 (b′)" (full (1)-(3) record; census tmp/f759d0/bprime_census.py). Re-submitted under Ruling 9 amendment (b′) (bcdc1648e). Layer-2 (2026-09-25) found (b′)(1), (2), prongs (5)/(6) and the `zero` holder hold, and FAILED it as UNDECIDED on (b′)(3): on the placeholder path, loop 2's `cells = table[21] + 0x24` = 0xCA8 is 8 bytes past sheet [21] (0xC84..0xCA0), which (3) calls an anomaly, but 0xCA8 is byte 8 (the ubase/vbase word `d000b400`) of a header-shaped 12-byte record at 0xCA0 that no table in D_SEL.BIN points to. The text does not settle whether "lands inside a record on a sub-object of a different kind" means BASE's own record or any record. Every other reachable write reaches its sheet's first cell.
disposition taken: not committed; src stays INCLUDE_ASM; the body is banked as pending-bprime-0.c, candidate.c stays the per-site 25/364. Question for the owner (plain language): "In one draw path, the game reads 8 bytes past the end of a sprite sheet. Those bytes happen to sit in the middle of a neighbouring block of data that nothing points to. Does that still count as a plain 'read past the end' bug, which the new rule allows? Or does landing inside other data count as 'meaning something different', which the rule forbids?"
resolution (owner, 2026-09-25): PAST THE END. The owner selected "Past the end (Recommended)". A BASE + K past the end of BASE's record counts as outside it under (b′)(3), even among other bytes of the image, provided it is not the start of any object the program actually references. The ledger must record the search showing the address is unreferenced. Hitting the start of a referenced object, or of a used sub-object of one, still fails. func_800759D0 (pending-bprime-0.c) re-lands only after a fresh layer-2 under that text. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md (b′)(3) "Clarification (owner, 2026-09-25)"; record in decisions.md 2026-09-25 OWNER RULING — Ruling 9 (b′)(3) clarification: past-the-end into unreferenced bytes.

## 2026-09-25 — func_80043454 — backward-goto counted loops (loop-depth RA weighting) — policy-question
category: policy-question
evidence: memory/grind/func_80043454/rejected/goto-loop-chassis-s1-0.c (sandbox 0/479; full-build SHA1 == oracle 2026-09-25 with it spliced into src/text1a_c.c) + evidence.md "Mechanisms" (tmp-free: flow.c weights reg_n_refs by LOOP-note depth; global.c allocno_compare order reproduced exactly). The mode-1/2 counted loops are written `if (--count != -1) { c3 = arg3; d1 = arg1; loopN: ...; if (--count != -1) goto loopN; }`; the mode-0 loop stays `while (--count != -1)`. As real while/do loops the same code lands count/kind in swapped registers: while 109, do-while-in-if 113, goto 0. Manual-lane layer-2 cheat-reviewer FAIL 2026-09-25: the goto-loop family is not sanctioned (decisions.md 2026-09-07 func_8007526C owner ruling; special_camera_get_rot_dir 2026-08-26 precedent); the hand-hoisted c3/d1 fall with it. Every other construct (s16 first param, SCRATCH_PTR walk, align-up, packed/mode/kind structure) cleared.
related: func_8007526C (2026-09-07, goto-loop family declared moot/not sanctioned when -msoft-float closed it ordinarily); special_camera_get_rot_dir (2026-08-26 FAIL, loop-spelling asymmetry for loop-depth weighting).
disposition taken: not committed; func_80043454 stays INCLUDE_ASM and is rotated; candidate.c is the best ordinary body (57/479, per-case counter `i`; plain all-while body 109). Question for the owner (plain language): "This function matches exactly only when two of its three counting loops are written with `goto` instead of `while`. The loops do the same thing either way, but GCC counts how deeply code is nested inside loops when it decides which variables get registers, and a goto loop doesn't count as a loop. You declined to approve goto loops as a general technique on 2026-09-07. Should they be allowed here, or should this function wait for a different fix?"

## 2026-09-26 — func_800620B8 — merging four adjacent sprite tables on compiler evidence alone — policy-question
category: policy-question
evidence: memory/grind/func_800620B8/rejected/table-merge-BA00x12-0.c (sandbox 0/501; full-build SHA1 == oracle 2026-09-26 with it spliced and `extern u16 D_8009BA00[12][4];` in include/game.h) + evidence.md "Layer-2 FAIL" and "Why split symbols cannot reach the target". Manual-lane layer-2 cheat-reviewer FAIL 2026-09-26 on the aggregate merge alone (prong (a), no-new-park-categories.md:250-274; precedent decisions.md:27263). The gte_stsz island, the goto-shared case tails, SetTransMatrix(tv - 0x14), the `rot` local and the scratch-word tag link were accepted. The four splat labels D_8009BA00/BA30/BA50/BA58 are contiguous 8-byte sprite records {clut x, clut y, u, v} used only by this function. The original keeps 0x8009BA00 in $fp across the loop and rebuilds the other three in $t0. GCC only does that when the four addresses are offsets of one symbol: cse relates them to BA00's register, so loop.c hoists it. With four separate symbols the body floors at 47/501 (39 with four invented table-pointer locals, which are not landable). The same-shape records continue before BA00 (through D_8009B8E8 at least), so no data or sibling evidence fixes the object's bounds.
related: func_80076D74 (2026-09-15 PASS, sibling stride reads); func_80054604 (2026-09-15 PASS, one base register reaching the span)
disposition taken: not committed; func_800620B8 stays INCLUDE_ASM/active; candidate.c is the landable split-symbol body at 47/501. Question for the owner (plain language): "func_800620B8 matches exactly only if four small sprite tables that sit next to each other in memory are declared as one table. The splitting tool gave them four names. Nothing else in the game proves they were one table. The only evidence is how the original compiler handled them in this function: it kept the first table's address in a register and built the other three from it. It only does that for parts of one object. Our merge rule requires evidence that doesn't come from matching the function itself. Should compiler behavior like this count as evidence for merging, or should the function wait for other evidence?"

## 2026-09-26 — func_800620B8 — addendum to the table-merge policy question (session 2) — policy-question
category: policy-question
evidence: memory/grind/func_800620B8/evidence.md "Session 2" (proof items 1-5 + evidence search) + scan_base.py. Two new facts since the question above. (1) The split-table floor (47/501) is now PROVEN structural, not a search gap: in the original bytes the register holding table 0x8009BA00 ($fp) is used exactly once inside the loop, and GCC 2.7.2 only gives that register the last free callee-saved slot (over the two stack-spilled locals) if it is referenced at least four weighted times; the missing references can only come from the other three table addresses being computed from it, which the compiler does only when they are offsets into one object (dump-verified with the instrumented cc1; a model-check variant moves the allocation exactly as predicted). (2) The original PsyQ compiler (cc1psx, calibration only) agrees: given four separate tables it produces the 47-shape code; given one table it produces the original's `$fp` = 0x8009BA00 and the other three rebuilt in `$t0` exactly. Independent evidence search came back empty: a scan of every address formed into 0x8009B7F0..0x8009BA5F in the original main EXE and the movie overlay finds 40 formations, all to separate labels, no offset crossing a label, no data pointers; every indexed table nearby is indexed over exactly its own label; no census/config row.
related: the 2026-09-26 func_800620B8 question directly above; func_8005E098 / D_8009B398 (include/game.h, accepted base+offset merge whose derived addresses stayed in registers).
disposition taken: not committed; func_800620B8 stays INCLUDE_ASM/active; no rotation requested by this worker (the orchestrator decides). Refined question for the owner (plain language): "We can now show that the original game code was compiled with these four sprite tables as ONE table: the original compiler, fed four separate tables, produces different machine code, and fed one table, produces exactly the shipped code. This proof comes from how the compiler assigned registers in this one function; no other function in the game touches these tables, so there is no second source. Is a proof like this — from the original compiler's register choices in the function itself — enough to declare the tables as one object?"
resolution (owner, 2026-09-26): COMPILER-NECESSITY EVIDENCE ACCEPTED, MINIMAL SPAN. The owner chose "Accept, minimal span (Recommended)": "Compiler-necessity proof + cc1psx confirmation + merged object limited to the labels actually used. Unblocks 800620B8, likely 8005D814." Aggregate-merge prong (a) may be met by (a1) dump-proven necessity covering every separate-object spelling, (a2) cc1psx confirmation (split -> non-target shape, merged -> target shape; calibration use only), (a3) the minimal span from the lowest- to the highest-addressed label the function references, and (a4) one record layout. Prongs (b)-(e) unchanged. func_800620B8's merge (D_8009BA00..D_8009BA5F) may be re-submitted to a fresh layer-2 once its ledger meets (a1)-(a4); the outcome is not pre-decided. Rule text (the author's narrowing): .claude/rules/no-new-park-categories.md aggregate-merge entry, "Amendment (owner ruling 2026-09-26)"; record in decisions.md 2026-09-26 OWNER RULING — aggregate-merge prong (a).

## 2026-09-26 — func_8002DE20 — one pair of cross-product locals shared by all twelve same-side tests — policy-question
category: policy-question
evidence: memory/grind/func_8002DE20/candidate.c (sandbox 0/506; full-build SHA1 == oracle 2026-09-26 with it spliced over INCLUDE_ASM in src/code6cac_b.c, then reverted) + evidence.md (floor table 506 -> 0, receipts) + rejected/. Not submitted to layer-2: the author judges the pair fails Ruling 5 as written. The function cuts a rotated triangle with the plane z = 0 and runs twelve same-side tests (both cut endpoints inside the triangle (0,0)/A/B, then the cut segment against each edge). Each test computes two cross products and compares their signs, `if ((side_a ^ side_b) >= 0)`. The closing form declares one `s32 side_a, side_b;` at function scope and writes the pair once per test (12 writes each, every write read only by the sign test right after it). This is the idiom func_8002E6B0 uses in the same file (`cross_center`/`cross_point`, on main since 2026-08-19). Mechanism: a local used in several blocks is a global pseudo, so local-alloc.c combine_regs cannot tie the xor result to it; the target's `xor v0,a0,v1` is untied at every test. Receipts on the final body: one fresh pair per test 90/506, cross products inline in the condition 90, per-group pairs 54, fresh pairs with only the last group sharing 72. Ruling 5 fails at 1(a) (the consumer is a sign test, not a struct member or call argument) and 1(b) (the writes are different cross products). Rulings 6, 9 and 10 do not apply. The GTE islands (inline_o.h gte_ldv0/gte_rtv0/gte_stlvnl with the header's own "$12"-"$15","memory" clobbers, which the target's reload registers require) are a separate admission step: the function is a 2026-08-17 cluster census member with no owner_cluster_grants.txt row yet.
disposition taken: not committed; func_8002DE20 stays INCLUDE_ASM/active; the banked candidate.c is the 0/506 body. The ordinary-C search continues (permuter from the one-pair-per-test body). Question for the owner (plain language): "func_8002DE20 checks whether a line segment touches a triangle, using twelve 'which side of this edge is the point on' tests. Each test computes two numbers and compares their signs. The function matches the original exactly only when the same two variables are reused for every test. That is also how func_8002E6B0, the function right after it, is written. With a new pair of variables per test it is 90 instructions off. Should reusing one pair of variables for the same kind of test, many times over, be allowed?"
resolution (owner, 2026-09-26): ALLOWED WITH PROOF. The owner chose "Allow with proof (Recommended)": "Admit only when allocator-dump proof shows necessity, honest generic name, and layer-2 still reviews. Unblocks 80055138, 8003993C, 8002DE20 (C part)." Each of side_a/side_b is judged on its own under Ruling 11: allocation dumps proving no one-variable-per-value spelling can match (necessity, not effect), an honest generic or true-kind name, a declaration comment, a fresh layer-2. The outcome is not pre-decided. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 11"; record in decisions.md 2026-09-26 OWNER RULING — Ruling 11.

## 2026-09-26 — func_80036140 — CD-mix record copy needs the original's "common variable" assembler rule — policy-question
category: policy-question
evidence: memory/grind/func_80036140/evidence.md (floor table + proven facts 1-5) + integration/ (patches, full-model-body.c, scratch whole-file builder xb.py, gp_off_census.py). Instruction-complete in a scratch model: with (1) ReplayCamRec extended over 0x80101E78..0x80101EA7, (2) g_cd_atv / D_800A36B8 merged into `CdlATV` (libcd's 4-byte mix record; CdMix takes CdlATV*), (3) the file compiled with -G8 and (4) maspsx told those two objects were COMMON in the original file, every function in code6cac_b2_post.c scores 0 and func_80036140 scores 2 (only the jump-table operand, a rodata-placement artifact the oracle settles). Without (3)+(4) the CdlATV merge breaks cdrom_SetMix / func_80035F78; without the merge the target's 4-byte `lwl/lwr/swl/swr` record copy has no C form except a per-use pointer pun (auto-FAIL). Landable split-symbol floor today: 50/512 (candidate.c).
related: func_800620B8 (2026-09-26, merge-evidence question: the record extension past 0x80101E99 rests on this function's member-access codegen alone, same class); CD_cw (3-arg `.comm` maspsx parse failure).
disposition taken: not committed; func_80036140 stays INCLUDE_ASM/active; ledger banked. Question for the owner (plain language): "func_80036140 copies one 4-byte CD volume record over another. The original assembler treated a variable differently depending on whether the original C file *defined* it or only *referred to* it: for variables the file defined, it used the fast global-pointer shortcut for the variable's first byte but not for 'variable + 1/2/3'. Our assembler shim assumes every such variable is only referred to, so it uses the shortcut for all four bytes, and the three functions that touch these two records stop matching. Matching them also needs this file compiled with the small-data setting (-G8) the original evidently used here (the same setting text1a already uses), which in turn means the one not-yet-decompiled neighbour, func_80036940, has to be linked as a separate assembly object like save_vc_ctrl. Should we (a) add a per-variable list telling the assembler shim 'these were defined in the original file' (two entries now: the current and target volume records), and (b) compile this CD file with -G8? Both change only how the build models the original tools; every other function in the file was measured unchanged."
owner (2026-09-26): NOT DECIDED in the 2026-09-26 batch; the owner left func_80036140's build-model changes (per-file -G8, the maspsx COMMON no-gp model) for separate investigation (docs/grind/owner-rulings-2026-09-26.md (batch 1)). The aggregate-merge amendment of the same day does not decide it.
resolution (owner, 2026-09-26, second batch): PER-FILE -G8 ALLOWED WITH PROOF; COMMON MODEL STILL UNDECIDED. The owner chose "Allow with that proof (Recommended)": "Requires gp-relative accesses in the original bytes + cc1psx confirmation + neighbours moved unchanged; layer-2 still reviews. Unblocks func_80034708 (and part of func_80036140)." Item (b) of the question is answered only in that form: a new -G8 TU holding functions that each show gp-relative accesses in their original bytes which the adjacent functions lack, confirmed by cc1psx at -G8 and not at -G0, with every other function moved unchanged and compiled with its source file's flags. Whole-file -G8 of code6cac_b2_post.c is admitted only if every function in it meets that proof, and no LINKED_ASM_FUNCS entry (e.g. for func_80036940) is admitted. Item (a), the maspsx COMMON-no-gp model, remains NOT DECIDED (the owner left it for separate investigation), so a landing that depends on it is still blocked. Rule text (the author's narrowing): .claude/rules/compiler-flags-canonical.md § "Per-file -G8 by proof"; record in decisions.md 2026-09-26 OWNER RULING — per-file -G8 by proof.
resolution (owner, 2026-09-26, fourth batch): COMMON GATE ALLOWED AS A GATED LIST (Q9); func_80036940 FIRST (Q10). Q9: the owner chose "Allow, gated list (Recommended)": "maspsx_comm_syms.txt naming only functions whose listed variables are proven by Sony's assembler; full oracle + layer-2 before use." Item (a) is answered in that form: a per-function, fidelity-class gate whose rows each need the shipped base-gp/sym+N signature and Sony's ASPSX 2.34 reproducing the shipped words from a tentative definition (extern/static/initialized/-G0 variants differing), landing with a full oracle rebuild, engine test, every object byte-identical both ways, and a fresh layer-2. Q10: the owner chose "Do 36940 first, then both (Recommended)": "Bring func_80036940 back from rotation now and decompile it, then move both into one -G8 file together. No rule change." func_80036940 was unparked (c4e2f96c3); func_80036140 lands after it, both in one -G8 file under the Per-file -G8 ruling. Nothing is pre-decided. Rule text (the author's narrowing): .claude/rules/maspsx-gate-lists.md § "maspsx_comm_syms.txt"; owner record docs/grind/owner-rulings-2026-09-26.md (batch 4); records in decisions.md 2026-09-26 OWNER RULING — maspsx COMMON gate, and — func_80036140's -G8 neighbour func_80036940.

## 2026-09-26 � func_8002DE20 � addendum: layer-2 FAIL; TWO owner items � policy-question
category: policy-question
evidence: memory/grind/func_8002DE20/rejected/layer2-fail-cross-pair-inline-o-islands-0.c (sandbox 0/506; full-build SHA1 == oracle) + evidence.md "Layer-2 FAIL". The manual-lane layer-2 FAILed the landing on two grounds and cleared everything else (struct view, mid_i, return form, z nudge).
disposition taken: reverted; src stays INCLUDE_ASM; function stays active (not rotated). Owner items: (1) the shared cross_a/cross_b pair question in the entry above (unchanged). (2) The GTE snippets are Sony's inline_o.h form, which needs a per-function owner-instructed tools/grinder/owner_cluster_grants.txt row (inline-asm-policy.md:360-364), and the header's provenance still needs a second independent copy. Question for the owner (plain language): "func_8002DE20 uses the same kind of Sony math-chip snippets as func_80018300, func_8002CD58 and func_8002DAD0, which you approved one at a time. Should it get its approval entry too? (Separately from the variable question above.)"
resolution (owner, 2026-09-26): item (1) ALLOWED WITH PROOF under Ruling 11 (resolution line under the entry above). Item (2) GRANTED AS A CLASS: the owner chose "Grant as a class (Recommended)": "Character-identical to a pinned inline_o.h copy; no per-function owner row needed." Islands that are, statement for statement and character for character, a macro in the engine/gtemacro.py PINNED PsyQ 4.3 inline_o.h text (two independent copies, silent-hill-decomp + xenogears-decomp; not the formatter-rewritten croc copy) need no owner_cluster_grants.txt row. NOT covered: joined-statement islands, DMPSX placeholder substitutions (not character-identical; the 2026-09-24 Extension is scoped to the inline_c.h rule), and `0($12)` for the header's `($12)` (the scoring-only equivalence is not an admission ruling). func_8002DE20's banked islands are joined, `0($12)`-spelled (gte_ldv0 / gte_stlvnl) and carry the substituted gte_rtv0 word, so as banked they are outside the class and still need a per-function row or a further owner ruling. Manual path only until the driver's grant door learns the class. Rule text (the author's narrowing): .claude/rules/inline-asm-policy.md § "Owner ruling 2026-09-26"; record in decisions.md 2026-09-26 OWNER RULING — inline_o.h GTE macro blocks as a class.
resolution (owner, 2026-09-26, second batch): `0($12)` KEEP STRICT. The owner chose "Keep strict wording": "Only exact character copies; func_8002DE20's blocks need a per-function grant." No rule change; the inline_o.h class stays character-identical only. func_8002DE20's islands need a per-function owner-instructed row. Record in decisions.md 2026-09-26 OWNER RULING — `0($12)` in the inline_o.h class: keep strict.
resolution (owner, 2026-09-26, fourth batch, Q11): PARSER FIX + PER-FUNCTION ROW. The owner chose "Fix parser + grant row (Recommended)": "Blocks become header-exact except the DMPSX-patched word; per-function owner_cluster_grants row for that only. Plus the engine recognizer update. Layer-2 reviews everything." A byte-neutral maspsx parser fix lets the header's `($12)` build, so no `0($12)` is admitted; func_8002DE20's row covers only its three gte_rtv0 units' `.word 0x4A486012` for `.word 0x0000013f` (2026-09-24 Extension prongs); every other island must be class-exact. The class's prong (C) is unchanged for every other function. The row, the parser fix and the recognizer update land with the function after a fresh layer-2; nothing is pre-decided. Rule text (the author's narrowing): .claude/rules/inline-asm-policy.md § "Per-function grant: func_8002DE20"; owner record docs/grind/owner-rulings-2026-09-26.md (batch 4); record in decisions.md 2026-09-26 OWNER RULING — func_8002DE20 per-function GTE grant.

## 2026-09-26 — func_80055138 — one scratch variable reused for five different values — policy-question
category: policy-question
evidence: memory/grind/func_80055138/integration/probe-shared-scratch-v-hdr0.c (0/516; full-build SHA1 == oracle 2026-09-26 with it spliced over INCLUDE_ASM and the header model applied — code6cac.h gains the D_80099D88 record table and the `u8 [][4]` cpu_practice_honmokuroku_data_tbl type, func_80055948 and func_80033DF4 respell their reads — then reverted; integration/apply_model.py) + evidence.md s2 (floor table, ablation, mechanism). Not submitted to layer-2: the author judges the variable fails Ruling 1/5 as written. The function sets up a player's status record for the current game mode, then scans both players' move lists for three stats per section. The closing form declares one `s32 v;` and uses it for (1) the case-2 level `D_800A37D2 / 5`, (2) the case-2 else level `D_800A37D2 / 3` with its reset to 0, (3) the case-3 table index `(D_800A38E2 / 10) * 2` (minus one), (4) the move entry's byte-assembled flag word, (5) the entry's stat bytes e[1] and e[2]. One variable per job scores 102/516; with a permuter-found named intermediate (`low_cat = cat < 2`) the per-job body is 35/516 (candidate.c; two permuter campaigns, 4500 iterations, found nothing lower except banned constant-carrier shapes). Moving any single job to its own variable costs 2-45 (ablation in evidence.md). Why no per-job spelling can match: in the target, the values of jobs 1 and 4 land in `$a2` straight out of `andi`/`or` whose other input dies there; a variable used inside one block is tied to that dying input by local-alloc (our per-job build puts them in `$v1`/`$v0`), so the target's variable must be live in other blocks too — the other jobs. Ruling 5 fails 1(a) (different consumers), 1(b) (different templates) and 1(f) (no role name fits five jobs); Rulings 6, 9 and 10 do not apply. Same class as func_8002DE20's 2026-09-26 shared cross-product pair, but broader (five meanings, not one test repeated). The body also relies on two constructs that do have a home: one loop counter shared by the two loops (`i < 8U` / `i < 2`, the owner-ruled func_8003800C shape, decisions.md 2026-08-25) and staging the section base through `c` (staged-value-reused-variable).
disposition taken: not committed; func_80055138 stays INCLUDE_ASM/active; candidate.c is the per-job 35/516 body; no rotation requested by this worker (the orchestrator decides). Question for the owner (plain language): "func_80055138 sets up a player's stats at the start of a match. The original programmer evidently used one throwaway variable for five unrelated short-lived values (two different level numbers, a table position, a bit mask, and two stat bytes read from a move list). We can prove this from the machine code: with one variable per value the compiler places them differently and the function is off by 35 of 516 instructions at best; with one shared variable it matches exactly. The current rules only allow reusing a variable when every use has the same meaning. Should a shared scratch variable be allowed when the machine code proves the original had one, or should this stay banned?"
resolution (owner, 2026-09-26): ALLOWED WITH PROOF. The owner chose "Allow with proof (Recommended)" (text in the rule). `v` may be re-submitted to a fresh layer-2 under Ruling 11 once the ledger carries the (D) necessity proof and the variable takes an honest generic name (`v` is single-letter and fails (E)). The outcome is not pre-decided. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 11"; record in decisions.md 2026-09-26 OWNER RULING — Ruling 11.

## 2026-09-26 — func_8006F97C — Ruling 9 prong (c): a `break` confined to a nested loop between the write and its consumer — policy-question
category: policy-question
evidence: memory/grind/func_8006F97C/rejected/ruling9-block1-break-span-0.c (sandbox 0/515; full-build SHA1 == oracle 2026-09-26 with it spliced over INCLUDE_ASM in src/text1b.c, then reverted) + evidence.md "Layer-2 FAIL" and "Ruling 9 prong walk" + hypotheses.md s2. Manual-lane layer-2 FAIL 2026-09-26 on one ground: the shared `cells` (four writes `cells = s.header + K`, K 0x24/0xC, each read once by `s.table = cells;`) meets Ruling 9 except prong (c) at its first site, where loop 1 (a search that ends in `break;`) sits between `cells = s.header + 0x24;` and `s.table = cells;`. The reviewer called it borderline owner policy, not a hard ban, and accepted every other construct. The original computes the value before the loop (0x8006FA40) and stores it after (0x8006FBB4). The loop advances s.header and calls rsin/rcos, so the write cannot move after it without an extra load. Every break-free spelling of the loop is longer: condition-in-header 55/520, `i = n` exit 6/519 (the original's found path jumps straight out). Per-site variables are 96-98/510. A carrier-free permuter reached best 500 from base 1800, and every find reused a variable for an unrelated value. A second run from the break-free body stayed at base.
related: func_8007636C (Ruling 9 re-audit PASS 71b14499d: the same `cells` idiom in the same module, with an `if` between write and consumer); func_800759D0.
disposition taken: not committed; func_8006F97C stays INCLUDE_ASM/active (not rotated by this worker). candidate.c is the break-free 6/519 body. The 0/515 body is banked in rejected/. Question for the owner (plain language): "func_8006F97C matches exactly using one variable, `cells`, that Ruling 9 otherwise allows. At one of its four uses, the value is computed just before a small search loop and stored just after it, and the loop stops early with `break` when it finds a match. The `break` only leaves that inner loop and always lands before the store. Nothing can skip the store. Ruling 9 says there may be no `break` between the write and its use, and it doesn't say whether a `break` belonging to a loop in between counts. Every way of writing the loop without `break` adds instructions. Should a `break` that only exits a loop sitting between the write and the use be allowed?"
resolution (owner, 2026-09-26): INNER-LOOP BREAK ALLOWED. The owner chose "Yes, inner-loop break OK (Recommended)": "The break never skips the use. Unblocks func_8006F97C (full match, everything else already accepted)." Ruling 9 (c) is clarified: a `break` exiting a for/while/do loop whose whole statement lies between the write and its consumer does not violate (c); `return`, `goto`, a `switch` break and `continue` still do. rejected/ruling9-block1-break-span-0.c may be re-submitted to a fresh layer-2; the outcome is not pre-decided. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md Ruling 9 (c) "Clarification (owner, 2026-09-26)"; record in decisions.md 2026-09-26 OWNER RULING — Ruling 9 (c) clarification.

## 2026-09-26 — func_8003993C — one scratch variable for two values, plus one pointer set in both branches — policy-question
category: policy-question
evidence: memory/grind/func_8003993C/rejected/pending-shared-sel-win-and-key-0.c (sandbox 0/526; full-build SHA1 == oracle 2026-09-26 with it spliced over INCLUDE_ASM in src/code6cac_c_mid.c, then reverted) + evidence.md [s2]/[s2b]. Not submitted to layer-2: the author judges both variables fail the rulings as written. The function is the replay-playback frame step. (1) One local `win` holds the replay window used by the event loop at the end AND, earlier, the 0/1 weapon-set selector `(flags >> 1) & 1` inside the per-player loop. In the target both live in `$s2`; a separate selector is allocated to `$v0` (local-alloc.c:472) and the whole global allocation order shifts: 82/526 with the two split (candidate.c). Ruling 5 fails 1(a)/(b)/(f); Rulings 6, 9, 10 do not apply. (2) `key`, the keyframe-entry pointer, is written once in each arm of the weapon if/else (`key = table + frame * 4; field = base + *(u16 *)(key + 2);`), from the practice table in one arm and the character's table in the other. The target's untied `addu a0,...` shows one pseudo spanning both arms: 4/526 with `key` removed, 13 with a separate `key` per arm. Ruling 6 fails only (C): the two writes read two different tables. Everything else in the body is ordinary (s16 packed-angle reads, a named `next` intermediate, direct per-case stores).
related: func_80055138 (2026-09-26, one scratch variable for five values), func_8002DE20 (2026-09-26, one shared cross-product pair) — same class.
disposition taken: not committed; func_8003993C stays INCLUDE_ASM/active; candidate.c is the ordinary 82/526 body; no rotation requested by this worker (the orchestrator decides). Question for the owner (plain language): "func_8003993C (the replay player) matches the original exactly only with two variable reuses the original programmer evidently made. First, one variable holds two unrelated short-lived values: which of two weapon tables to use, and later how many replay frames are left. Split into two variables, the compiler places everything differently and the function is 82 of 526 instructions off. Second, one pointer ('the current keyframe entry') is set in each branch of an if/else, from a different table in each branch; with a separate pointer per branch it is 13 off, with none 4 off. Should (a) a scratch variable reused for unrelated values, and (b) one pointer with the same meaning set from different tables in exclusive branches, be allowed when the machine code proves the original had them?"
resolution (owner, 2026-09-26): ALLOWED WITH PROOF. The owner chose "Allow with proof (Recommended)" (text in the rule). Both (a) `win` and (b) `key` are fresh locals no earlier ruling admits, so each may be re-submitted to a fresh layer-2 under Ruling 11 once the ledger carries the (D) necessity proof for it and it meets (E) (`win` names only one of its values and needs a new name). The outcome is not pre-decided. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 11"; record in decisions.md 2026-09-26 OWNER RULING — Ruling 11.

## 2026-09-26 — func_80034708 — the practice-parameter menu needs a -G8 translation unit — policy-question (owner-answered)
category: policy-question
evidence: memory/grind/func_80034708/evidence.md F3, F10-F12 (cc1psx) + integration/landing-s2-g8.patch (the full staged landing: sandbox 0/544, full-build SHA1 == oracle) + integration/cc1psx/. The target reads the two-entry cursor array D_800A3174 gp-relative 16 times while the loop walks &cursor[i]; its neighbours have no such reads. Under -G0 every honest array/struct spelling measures 90 (the array address stays in a register); under -G8 the body is exact. The original PsyQ cc1psx agrees: 16 gp-direct cursor reads at -G8, none at -G0. The function's <=8-byte strings sit in .sdata and its 9-byte ones in .rodata, the -G8 section rule. The only -G0 form reaching the floor is two scalars plus a cross-object `(&D_800A3174)[i]` pun (rejected).
disposition taken: layer-2 FAILed the landing (new -G8 file unauthorized at the time; PracticeParams 0x78 merge fails prong (a)); reverted, oracle rebuilt, lock released; func_80034708 stays INCLUDE_ASM/active. Question for the owner (plain language): "func_80034708 only matches when compiled with the small-data setting (-G8) the original compiler evidently used for it. Its four neighbours in the file do not need it and would be moved unchanged into their own file. May a function get its own -G8 file when the original bytes and the original compiler both show it?"
resolution (owner, 2026-09-26, second batch): PER-FILE -G8 ALLOWED WITH PROOF (Q5) and MIXED-FIELD STRUCT ALLOWED WITH THE SAME PROOF (Q7). Q5: the owner chose "Allow with that proof (Recommended)": "Requires gp-relative accesses in the original bytes + cc1psx confirmation + neighbours moved unchanged; layer-2 still reviews. Unblocks func_80034708 (and part of func_80036140)." Rule text (the author's narrowing): .claude/rules/compiler-flags-canonical.md § "Per-file -G8 by proof", prongs (i)-(vi) including (iv-a). Q7: the owner chose "Allow with same proof (Recommended)": "Same dump proof + cc1psx confirmation + minimal span of bytes actually used; layer-2 still reviews. Unblocks func_80034708 (with -G8)." Rule text (the author's narrowing): .claude/rules/no-new-park-categories.md aggregate-merge amendment, (a4′). The PracticeParams 0x78 merge is judged fresh against (a1), (a2) and (a4′)(1)-(5), and the -G8 split against (i)-(vi); nothing is pre-decided. Owner record: docs/grind/owner-rulings-2026-09-26.md (batch 2); records in decisions.md 2026-09-26 OWNER RULING — per-file -G8 by proof, and 2026-09-26 OWNER RULING — aggregate-merge (a4′): mixed-field struct.

## 2026-09-26 — func_80034708 — -G8 screening rule vs plain small externs — policy-question
category: policy-question
evidence: memory/grind/func_80034708/evidence.md [s5] F28 + integration/landing-s4-rejected.patch (layer-2 FAIL objection 1; the other two objections are fixed in the generator, integration/). The per-file -G8 rule (compiler-flags-canonical.md (iii)) keeps the older screening sentence: every extern of 8 bytes or less that the new TU references must be in sdata_syms.txt or honestly typed larger than 8 bytes. func_80034708 increments D_800A37B8, a 4-byte counter owned by another file. It is not in sdata_syms.txt (no original function reads it gp-relative, and adding it would make maspsx gp-relative it in func_80035480 / func_80035618 / func_80035828, which read it with lui/%lo), and it is truly 4 bytes. Its one use here (`D_800A37B8++`) compiles to the same instructions at -G0 and -G8. The screening rule came from a different hazard: a small extern whose ADDRESS the compiler would otherwise keep in a register. The already-approved -G8 files reference 29 small externs outside sdata_syms.txt (text1a_pre 7, text1a_post 22, some outside gp range).
disposition taken: reverted (layer-2 FAIL), oracle rebuilt, lock released; func_80034708 stays INCLUDE_ASM/active; the fixed landing is ready to re-apply. Question for the owner (plain language): "The small-data (-G8) rule says every small shared variable a -G8 file touches must be one the assembler shim treats as small data, or be declared bigger than 8 bytes. func_80034708 adds 1 to a 4-byte frame counter that lives in another file; the original never used the fast small-data access for it, and the compiled code is identical either way. The two -G8 files you already approved touch 29 such variables. Should the rule only apply to small variables whose code actually changes under -G8 (proven by the same before/after build), rather than to every small variable?"
resolution (owner, 2026-09-26, third batch): SCREEN ONLY EXTERNS WHOSE CODE CHANGES. The owner chose "Only if code changes (Recommended)": "A small variable must be listed only when -G8 changes its compiled instructions; proven by building it both ways (bytes identical). Unblocks func_80034708; matches existing text1a practice." A small extern outside sdata_syms.txt (here D_800A37B8) is exempt from listing only when the -G8 and -G0 builds of the TU have every access to it identical in bytes and relocation, with the listing banked in the ledger; otherwise it must be listed or typed larger. The landing is judged fresh against that text; nothing is pre-decided. Rule text (the author's narrowing): .claude/rules/compiler-flags-canonical.md § "Screening scope (owner ruling 2026-09-26, third batch)"; owner record docs/grind/owner-rulings-2026-09-26.md (batch 3); record in decisions.md 2026-09-26 OWNER RULING — -G8 screening scope.

## 2026-09-26 — func_8005D814 — a combine-foldable detour whose effect is loop.c hoisting — family-candidate
category: family-candidate
evidence: memory/grind/func_8005D814/rejected/ (the split body is banked in the ledger: candidate.c at commit of this entry; dumps/final_split/) + evidence.md "LAYER-2 FAIL #3" / "#4". The split body (sandbox 0/545, full-build SHA1 == oracle 2026-09-26, then reverted) wrote the tile loop's two first-pair addresses as `hdr2_end = &D_8009B398[2] + 1; s.header = hdr2_end - 1;` and `cell2_end = &D_8009B3F0 + 1; s.table = cell2_end - 1;`. Combine folds each detour back to the plain address with zero bytes, but before that loop.c sees a second use of the constant and a `forces` link (loop.c:1200-1224, which relies on the stale regno_last_uid its own comment questions at :1205), so it hoists the direct-form address into the loop preheader. Layer-2 FAILed it: dead-store-fake-exception.md admits a combine-foldable chain-extender only when its sole surviving effect is the extra reg_n_refs count; steering loop-invariant motion is a different effect (same class as the func_800620B8 extender FAILed the same day).
disposition taken: not committed; func_8005D814 stays INCLUDE_ASM/active and returns to the D_8009B3F0[2] aggregate-merge route (the inadmissible extender does not count against (a1)). Question for the owner, recorded only, nothing waits on it (plain language): "A no-op detour like `p = &x + 1; use(p - 1);` compiles to exactly the same bytes as `use(&x)`, but it makes the compiler's loop optimizer move the address load out of the loop. The current sanction for such detours covers only their effect on register priority. Should a byte-neutral detour that changes which values the loop optimizer hoists be admitted under the same FAKE-annotation rules?"

## 2026-09-26 — func_80036140 — the CD state record must run to 0x80101EA7; two of its new fields are used only by other functions — policy-question
category: policy-question
evidence: memory/grind/func_80036140/evidence.md § "Record extension 0x80101E9C..0x80101EA7" (ledger commit 6dd123325; dumps under memory/grind/func_80036140/landing/dumps/, cc1psx runs under landing/psx/). The joint -G8 landing (func_80036140 + func_80036940, plus cdrom_SetMix + func_80035F78 in a second -G8 file) reaches the oracle SHA1 in a scratch full build only when the CD state record `CdState D_80101E58` (today 0x80101E58..0x80101E9B) is extended through 0x80101EA7. Three measured facts force that object model:
(1) at -G8 the target's `la reg,sym; lX 0(reg) .. sX 0(reg)` read-modify-writes of 0x80101E9C and 0x80101EA4 appear only when the variable is larger than 8 bytes. cc1 marks every variable of 8 bytes or less as small data (mips.h ENCODE_SECTION_INFO, SYMBOL_REF_FLAG), mips_address_cost prices such an address 1 (the price of a register), and cse.c find_best_addr then replaces the register holding the address by the symbol itself. Measured at -G8: plain separate variables 18, pointer read-modify-write locals 18, a function-scope pointer alias 42; the record member 0 (the jump-table operand aside). cc1psx agrees (separate: direct `lhu/lw sym`; record: `la $2,D_80101E58+68; lhu $3,0($2)`).
(2) an object over 8 bytes that holds 0x80101E9C and cannot overlap the existing record must start at 0x80101E9C and so covers 0x80101E9E..0x80101EA7 too;
(3) 0x80101EA0 (g_cdread_expected_pos) must then be an offset of the SAME symbol as the existing record: as a member of a separate 12-byte record at 0x80101E9C, cdrom_ReadyCallback scores 12 and func_80036140 8 (cse can no longer relate its address to the record register it already holds, cse.c use_related_value, so the address sits in $s0 across the calls).
Of the four fields in 0x80101E9C..0x80101EA7, func_80036140 uses two (0x80101E9C s16, 0x80101EA4 s32). The other two are used by other functions with their own widths: 0x80101E9E (u16: cdrom_StartRead stores 0, game_FrameLoop adds 2 through a u16 pointer) and 0x80101EA0 (s32: cdrom_ReadyCallback reads/compares/increments, func_80036940 stores). Aggregate-merge (a4′)(2)/(3) (no-new-park-categories.md) says a byte the function under judgment never touches must be an offset-named filler (`u8 unk3E[6]`), and (a4′)(5) fails any struct whose members a necessary other consumer cannot use without a pointer pun — for this span the two clauses contradict each other: a filler forces `*(u16 *)` / `*(s32 *)` puns on the four other consumers.
related: the 2026-09-26 func_80036140 question above (build model, answered: Q9 gate, Q10 -G8 file); func_80034708 (its (a4′) struct had one filler no function touches).
disposition taken: not committed; func_80036140 stays INCLUDE_ASM/active; everything else (maspsx gate + registration, the two -G8 splits, the CdlATV merge, the body, all prong records) is built and verified in scratch and waits only on this. Question for the owner (plain language): "func_80036140 only compiles to the shipped code if the CD state block is declared 12 bytes longer (0x80101E9C..0x80101EA7) — the compiler and Sony's own compiler both confirm that separately declared variables, pointer copies, or a separate small block there give different code. Two of the four new fields are the ones func_80036140 uses. The other two already have their own users elsewhere (a 16-bit counter used by cdrom_StartRead / game_FrameLoop, and the 32-bit expected disc position used by cdrom_ReadyCallback / func_80036940). The struct-merge rule says fields the function doesn't use must be anonymous padding, but then those other functions could only reach them through pointer casts, which the same rule bans. May those two fields be real named fields, typed the way their existing users read and write them?"
options: (A, recommended) Allow — inside a span proven by the merge rule, a byte the function doesn't touch but another existing function does is a named member whose width/type follow that function's accesses (still no role name beyond the existing symbol's, still no pun anywhere). (B) Do not allow — func_80036140 stays INCLUDE_ASM (rotated).
resolution (owner, 2026-09-26, sixth batch, Q13): YES — forced-in bytes may be named members typed by their real users' original accesses (rule text: no-new-park-categories.md aggregate-merge (a4′) amendment, 61f37ea5b). Applied to this span: 0x80101E9E qualifies (u16: game_FrameLoop 80036F9C `lhu`, 80036FAC `sh`; cdrom_StartRead 80036E0C `sh`). 0x80101EA0 does not under that text: its only original accesses (cdrom_ReadyCallback 800360A4 `lw` + `bne`, 800360E4 `lw` + `addiu`, 800360FC `sw`; func_80036940 80036AAC `sw`) carry no admissible signedness evidence — follow-up raised with the orchestrator.
resolution (owner, 2026-09-26, seventh batch, Q14): KEEP EXISTING TYPE when the binary cannot tell. The owner chose "Keep existing type (Recommended)": "When no signedness-revealing instruction exists anywhere and both choices are byte-identical, the member keeps its current declared type on main (or the SDK type it's assigned from); layer-2 checks byte-identity." For 0x80101EA0 that means the ledger lists every access and every use of the loaded value in cdrom_ReadyCallback and func_80036940 as not signedness-revealing, banks signed/unsigned builds of every accessing function showing byte-identity, and names main's s32 declaration (g_cdread_expected_pos, file and line) as the kept type. The landing is judged fresh by layer-2; nothing is pre-decided. Rule text (the author's narrowing): .claude/rules/no-new-park-categories.md (a4′) Q13 amendment, "When the binary cannot tell"; owner record docs/grind/owner-rulings-2026-09-26.md (batch 7); record in decisions.md 2026-09-26 OWNER RULING — aggregate-merge (a4′): signedness when the binary cannot tell.

## 2026-09-26 — func_8001A820 — our compiler folds five scratchpad halfword reads the original kept unfolded — policy-question
category: policy-question
evidence: memory/grind/func_8001A820/evidence.md [s2] FIDELITY FINDING + fidelity/ (cc1psx_N1_sites.txt, ourcc1_N1_sites.txt, micro_all.c + micro_results.txt, scan_luibase_halfword.py, psx.sh/micro.sh to re-run). The natural candidate (memory/grind/func_8001A820/candidate.c) matches every instruction of the 576 except five sites: the shipped code reads a signed 16-bit scratchpad value with `lhu` and then sign-extends it with `sll 16; sra 16`; our cc1 folds each into one `lh` (sandbox 33 = those five sites (20) + a 13-instruction %lo relocation display artifact). Sony's own compiler (tools/cc1psx.exe, "GNU C 2.7.2.SN.1"), run on the same preprocessed file, produces the shipped shape at all five sites from that plain C. Micro-tests narrow it: cc1psx keeps the unfolded form when the pointer register holds a round constant loaded by `lui` alone (0x1F800000, 0x1F810000, 0x80100000) and folds to `lh` when the constant needs `lui`+`ori` (0x1F8001B0) or the pointer is a parameter; our cc1 folds every case, under every respelling tried (u16/s16 temporaries, `+800 < 0`, bitfield, packed word `>> 16`, separate statements), with or without -mel / -msoft-float. No completed function in the binary reads a halfword through such a register, so no matched function contradicts Sony's behaviour. Two of the five sites (yaw, roll) can be forced in our cc1 by reading into a u16 temporary before an unrelated store (Judge precedent: replay_camera_rob_back_loose3, 2026-07-30); the other three (the three `< -0x320` tests of the surface normal after func_80053614) have no store, call or second use in the shipped code, so no ordinary C reaches them in our cc1; `volatile` on scratchpad is banned (decisions.md 2026-08-20, func_80017FA0) and does not match the stores anyway.
related: func_80073C78 / the PLUS->IOR study and narrow patch (owner rulings 2026-09-25, docs/ORACLE-COMPILER.md) — the same class: a shipped-binary behaviour of the original compiler that our fork does not reproduce; -mel (2026-08-04) and -msoft-float (2026-09-07) fidelity adoptions.
disposition taken: not committed; func_8001A820 stays INCLUDE_ASM/active; ledger banked (candidate.c = natural form). The C also needs two Ruling 11 packages (a reused angle variable and a shared bisection bound) before it could land, independent of this question.
Question for the owner (plain language): "func_8001A820 now matches the original everywhere except five places where it reads a signed 16-bit number from the scratchpad. The original game (and Sony's own compiler, which we have) reads it as unsigned and then fixes the sign with two extra instructions; our compiler merges that into a single signed read. It only happens when the pointer holds a 'round' address like 0x1F800000, and no other finished function in the game has this situation. Three of the five places cannot be reproduced with any ordinary C on our compiler. Should we run a scratch-only study (like the PLUS->IOR one) to find exactly why Sony's compiler keeps these reads unmerged and what the narrowest matching fix to our compiler would be, with adoption coming back to you with evidence?"
options: (A, recommended) Authorize a scratch-only study, same terms as the 2026-09-25 PLUS->IOR study: variants built outside the repository, nothing installed, adoption returns to the owner with a full-corpus byte-neutrality check. (B) Do not study it — func_8001A820 stays INCLUDE_ASM (rotated).
withdrawn: owner 2026-09-26 Q17, compiler patches are cheats. No compiler study or patch will be made; func_8001A820 continues in ordinary C only (permuter and non-lui-only-base spellings pending, see memory/grind/func_8001A820/hypotheses.md).
correction (2026-09-27): the five-site divergence was caused by the PLUS->IOR patch itself, not by the original compiler. The stock cc1 after its retirement (d94fef9a0) rewrites `(plus s7 const)` into IOR because s7 holds 0x1F800000, so the halfword load is not folded and plain C matches all five sites (memory/grind/func_8001A820/evidence.md [s3]). The question is moot.

## 2026-09-26 — func_80027AD8 — two local copies of a stack-passed pointer parameter — policy-question
category: policy-question
evidence: memory/grind/func_80027AD8/evidence.md [s2] (bank 3 and later) + probes/final1-two-copies.c (sandbox 2/574: the only residual is the jump-table relocation, which only the full build can certify; the full build was NOT yet run because the landing lock was busy for 90+ minutes). The original keeps the record pointer (6th argument, 0x64(sp)) in TWO callee-saved registers: `lw s5,0x64(sp)` for the field reads and `addu fp,s5,zero` for the value passed on to func_800278C0 at its three call sites. GCC 2.7.2 makes two pseudos only for two C variables. The parameter itself cannot be either one. assign_parms puts a REG_EQUIV note on a stack-passed parameter, and local-alloc.c update_equiv_regs (~1058-1064) then doubles its live length, which halves its global-alloc priority (BB2_ALLOC_DEBUG: 7 refs / 266 -> 526, below thresh's 650). So the parameter never gets s5 or fp. Measured on the final body: dropping the field-read copy (fields through `rec`) scores 35, dropping the argument copy (tbl passed) scores 52, both copies 2. No rule admits a single-value local that is a bare copy of a parameter (Ruling 11 (C)(3) and the brief's "bare invented copies" line). The alternatives measured (reusing the parameter for the R6a flag, probe m10; a multi-value fresh local) are each banned by Ruling 11 (A) or (C)(3), and they also score worse.
disposition taken: not landed; func_80027AD8 stays INCLUDE_ASM/active; honest per-variable candidate (candidate.c) is the fallback. Question for the owner (plain language): "The original game keeps one pointer argument in two registers at once: one copy is used to read the record's fields, the other is only passed along to another function. The only C that reproduces this copies the argument into two local variables at the top of the function (`tbl = rec; tbl_arg = (s32)rec;`). The compiler dumps show why the argument itself can't be used: GCC deliberately gives stack-passed arguments half priority for registers. Allow two plain local copies of an argument when compiler dumps prove this mechanism, with a comment and layer-2 review?"
resolution (owner, 2026-09-26, eleventh batch, Q19): ALLOWED WITH R11-STYLE PROOF, PER COPY. The owner chose "Allow with R11-style proof (Recommended)": "Dumps must prove necessity by mechanism for each copy (no spelling without it matches, incl. all FAKE families), honest role names, each copy genuinely used; layer-2 reviews." Ruling 12 admits a once-written copy of a never-written stack-passed parameter only with, for each copy, allocation dumps and a mechanism argument covering every copy-free spelling (including sanctioned FAKE families) checked against the real allocator (local-alloc suggestions, find_reg preferences, expand_preferences, reload/reorg), an honest role name, genuine use, an annotation and a fresh layer-2. Both copies here (tbl, tbl_arg) are judged separately and fresh; nothing is pre-decided. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md "Ruling 12"; owner record docs/grind/owner-rulings-2026-09-26.md (batch 11); record in decisions.md 2026-09-26 OWNER RULING — Ruling 12: local copies of a stack-passed parameter.

## 2026-09-26 — func_8001CE60 — one frame-count variable for the announcement length and the clock's frames left — policy-question
category: policy-question
evidence: memory/grind/func_8001CE60/evidence.md [s2] + probes/shared-limit-left-n-0.c (sandbox 0/588) + dumps/timer-alloc.txt. The VS-mode round director (func_8001CE60, a from-scratch 588-instruction body) is 21/588 with one variable per value (candidate.c): every remaining difference is a register choice in the on-screen clock block (`frames left = limit*30 - elapsed; secs = left/30; centisecs = left%30*100/30`). It is 0/588 only when ONE local holds the announcement length (set to 80 after a draw, 100 otherwise, in the two branches of an if/else, then compared with the announcement counter) and, later, the clock's frames left. Mechanism (dumps): shared, the variable dies in two blocks, so local-alloc gives it no quantity (local-alloc.c combine_regs, `reg_qty[sreg] == -1`) and global.c seats it in v1 as the target does; split, the clock value is block-local and combine_regs ties it to the dying `limit*30` chain, which then loses v0 (35/588 insns off). The original PsyQ compiler (cc1psx, calibration) behaves the same: split gives the wrong clock registers, shared gives the target's. Separate variables measured (all 21): block-local, function-scope, no variable, split total/secs forms, operand orders; a dead store / self-assign / do-while(0) on the clock variable in another block (21, flow deletes them); sharing with the id or the elapsed+1 value instead (8, 10). Ruling 11 admits a reused local with dump proof, but its (C)(3) refuses a value whose writes are all literal constants — the announcement length is exactly that (80 or 100, chosen by the branch).
disposition taken: not landed; func_8001CE60 stays INCLUDE_ASM/active; the per-value candidate (21) is banked and the search for an ordinary form continues (focused permuter on the clock block). Question for the owner (plain language): "One function only matches if a single local variable (call it `frames`) is used for two frame counts at different times: first how long the round announcement lasts (80 frames after a draw, 100 otherwise, picked in an if/else), later how many frames are left on the match clock. Compiler dumps (and Sony's original compiler) show that separate variables put the clock arithmetic in the wrong registers. Our reused-variable ruling allows this with dump proof, except that it refuses a value that is only ever a plain number, and the announcement length is a plain number picked by the if/else. Allow a per-branch constant like this as one of the values, with the same dump proof, an honest name and layer-2 review?"
options: (A, recommended) Allow it narrowly: a value whose writes are constants chosen by the arms of one if/else (each arm assigns once, and the value is read after the arms join) counts under Ruling 11 (C)(3); everything else in Ruling 11 unchanged. (B) Do not allow — func_8001CE60 stays INCLUDE_ASM and keeps grinding in ordinary C.
correction (same author, same day): in the evidence line above, "(35/588 insns off)" is wrong; the split spelling scores 21/588 (588 == 588 insns), every scored hunk operand-only.
update (same author, 2026-09-26, later): a form that does not need this ruling was found — the shared local holds the announcement COUNTER value (`(temp = D_800A36E8) != 0`, a load) and the clock's frames left, both real computations under Ruling 11 (C)(3), sandbox 0/588 (memory/grind/func_8001CE60/evidence.md s2 cont.). The announcement length stays in its own per-branch local `end`. The question above is kept for the record; func_8001CE60 no longer depends on it.
resolution (owner, 2026-09-26, eleventh batch, Q20): PER-BRANCH CONSTANTS ALLOWED. The owner chose "Allow per-branch constants (Recommended)": "Only when the constant differs by path (a real choice made at runtime); a single unconditional constant still doesn't count; same R11 necessity proof and layer-2." Ruling 11 (C)(3) now counts a value whose writes are all constants when at least two write different constants on different feasible paths chosen by a runtime condition; a single unconditional constant, or one repeated constant, still fails. Per the update above, func_8001CE60's current form does not rely on it. Rule text (the author's narrowing): .claude/rules/ordinary-c-judge-decidable.md Ruling 11 (C)(3) "Per-branch constants"; owner record docs/grind/owner-rulings-2026-09-26.md (batch 11); record in decisions.md 2026-09-26 OWNER RULING — per-branch constants as a Ruling 11 value.

## 2026-09-26 — func_8001CE60 — the score bytes need an array in one file and single bytes in another — policy-question
category: policy-question
evidence: memory/grind/func_8001CE60/evidence.md [s2 cont., landing attempt 1] + probes/func_800340A0-array-respelling-5.c + tmp/func_8001CE60/mini/t.c. func_8001CE60 (0/588 in the sandbox, byte-exact in the full build) reads and increments the P1/P2 score and tiebreak bytes (0x800A3898/99, 0x800A38AA/AB) by a player index, so it needs them declared as 2-element arrays; the aggregate-merge declaration in include/code6cac.h built everything except func_800340A0 (COMPLETED-C, code6cac_b.c), which then differs (5/88): GCC 2.7.2 (and cc1psx, same result) addresses an array element through a register when the same element is used twice nearby, while the shipped func_800340A0 uses the direct single-byte form both times. Respellings of func_800340A0 with the arrays all stay at 5. So no one declaration compiles both functions; the original must have declared the bytes differently in the two files. SOTN keeps such mismatched declarations, annotated, where the original bytes demand them (memory reference sotn-prototype-struct-precedent-2026-08-10; Silent Hill keeps two annotated views).
disposition taken: not landed; func_8001CE60 stays INCLUDE_ASM/active with candidate.c banked. Question for the owner (plain language): "The P1/P2 round scores are four bytes that one function (func_8001CE60) reads by player number, so it needs them declared as small arrays. Another function that is already finished (func_800340A0, in a different file) only still matches if the same bytes are declared as separate single bytes: with the array declaration both our compiler and Sony's original compiler generate slightly different code for it. So no single declaration works for both files; the original game's source must have declared them differently in the two files. Allow the two files to declare the same bytes differently (arrays in code6cac.c, single bytes in code6cac_b.c), each marked with a comment explaining why, with the compiler evidence recorded?"
options: (A, recommended) Allow it narrowly: a data object may be declared with different C types in two files only when compiler evidence (ours and cc1psx) shows no single declaration compiles every user, each declaration is file-local with an annotation citing the evidence, and layer-2 reviews. (B) Do not allow — func_8001CE60 stays INCLUDE_ASM.
update (same author, 2026-09-27): the per-file form was built with the landing lock and matches the oracle (full rebuild 62efab4f..., sandbox 0/588); it was reverted pending this ruling. Exact edits: memory/grind/func_8001CE60/probes/per-tu-landing.diff.
