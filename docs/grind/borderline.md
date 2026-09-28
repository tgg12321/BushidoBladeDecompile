# Borderline ledger

Established by the 2026-08-18 owner ruling (`.claude/rules/judge-sole-gate.md`):
user-escalation/approval is removed from the workflow; the default-FAIL Judge
applies the frozen standing policy, and anything borderline is APPENDED HERE for
the owner to evaluate later — nothing in this file is pending, and no entry
authorizes anything by itself. Only a subsequent owner ruling (landed standalone
per ruling-record-lands-before-code) can spend an entry. Outdated entries are deleted by owner-directed cleanup (owner ruling 2026-09-28, decisions.md); an entry cited by date and title that is no longer here is in git history at pin commit `f2bf53757` (`git show f2bf53757:docs/grind/borderline.md`).

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

## 2026-08-18 — parked-set name-drift alias table — policy-question
category: policy-question
evidence: parked-set audit 2026-08-18; phase-2 naming reset 2651e2e5 + PsyQ-library adoption renamed nine parked functions after their rulings were filed, so decisions.md rulings are unfindable from current queue names
disposition taken: alias table recorded here; no policy change.
  func_80038C70 = motion_SetMotion · func_80047FBC = InitHiraRmd_80047FBC ·
  CD_sync (0x80080DB0) = cpu_side_move_dir_4 · CD_datasync (0x80081BB0) = saEft01Init ·
  func_80041688 = gnd_init_80041688 · func_800307D0 = cpu_check_tubazeri_2 ·
  func_8003800C = damage_DebugDisp · func_80056FE8 = ang_hosei_80056FE8 ·
  func_80047EE8 = AddTbpOfst_80047EE8 · get_alarm = func_8007DC9C (carried over 2026-09-28 from the deleted 2026-08-19 stale-park re-audit entry)

## 2026-08-24 — func_80083794 — canonical-asm-grant
category: canonical-asm-grant
evidence: owner packet-2 ruling, docs/grind/decisions.md "2026-08-24 — escalation-packet rulings" (commit 608c8a4c, landed before code): libgcc __main / crt0 ctor-walker routed COMPLETED-INLINE-ASM-CANONICAL as provably-prebuilt object code — REG_PARM_STACK_SPACE arithmetic-floor-9 non-existence proof (decisions.md 2026-08-13), 7 provenance results, _start adjacency (same crt0 grant family, 2026-08-06). scan_hand_coded LOW 0/8 disclosed — the grant rests on the non-existence proof, not scanner tier.
disposition taken: inline_asm_canonical.txt entry written; fused .s split (motion_Close gets its own file + INCLUDE_ASM, returned to active with a provenance-first directive); 18 legacy rules retired; queue done accepted COMPLETED-INLINE-ASM-CANONICAL at SHA1 62efab4f...; layer-2 cheat-reviewer PASS (resubmission after queue reconciliation — the first review correctly FAILed on bookkeeping gaps).

## 2026-08-30 — motion_Close — canonical-asm-grant
category: canonical-asm-grant
evidence: owner ruling 2026-08-30 (decisions.md, ruling 3) extending the 2026-08-24 func_80083794 prebuilt-object routing; s14 provenance census — 2/1,435 asm functions carry the below-$sp+16 callee-save fingerprint (motion_Close + granted twin, byte-contiguous crt0/libgcc ctor/dtor pair)
disposition taken: inline_asm_canonical.txt entry written (INCLUDE_ASM body is the accepted form, mirroring func_80083794); queue done after layer-2 review; COMPLETED-INLINE-ASM-CANONICAL.

## 2026-08-31 — func_8002FC80 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002FC80 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-08-31)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-08-31 — func_8002D320 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002D320 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-08-31)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-01 — cop2 materialize-then-copy widened anchor — canonical-asm-grant (RESOLUTION of today's family-candidate entry)
category: canonical-asm-grant
evidence: owner GRANT 2026-09-01 (decisions.md "cop2 materialize-then-copy WIDENED ANCHOR — OWNER GRANT"), issued as an informed ruling with the objections presented (LOW scan tier, zero-hit SOTN census, Judge 09:01 routing FAIL). Same evidence set + same 4-point mechanical check as the 2026-08-17 cluster ruling; mechanical membership enumerated (68 in-band carriers, 66 with non-$aN-source sites; scan method in the cluster rule doc).
disposition taken: today's earlier family-candidate entry is RESOLVED-GRANTED. func_800203B4 integrated per its foreclosure record's trigger-1 recipe under the full gates (sandbox 0 re-verify, fresh layer-2 cheat-reviewer, verify-oracle --rebuild, queue done). Remaining confirmed carriers (func_80019310, func_800204C0, func_8002FF20, func_80031890, func_8003E6D8) keep their queue states; islands covered only when their C bodies independently reach zero.

## 2026-09-02 — func_8002FF20 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002FF20 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:17921; row per owner ruling 2026-09-02)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-02 — func_800300B4 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_800300B4 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-02 — func_800325E0 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_800325E0 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; registry per ruling 2026-08-30)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-06 — func_80019310 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_80019310 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:17921; row per owner ruling 2026-09-06 foreclosed-bucket review)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-06)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-08 — func_800204C0 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-08 — func_800204C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_800204C0/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-09-21 — func_8002D780 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002D780 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; census row :75; row per owner Ruling 3 terms, 2026-09-15, decisions.md:26863; operator-added 2026-09-21)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-21)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-21 — func_8002EBDC — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002EBDC tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; census row :80; row per owner Ruling 3 terms, 2026-09-15, decisions.md:26863; operator-added 2026-09-21)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-21)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-22 — func_8003FA24 — family-candidate
evidence: memory/grind/func_8003FA24/rejected/half-multiwrite-carrier.c (sandbox 0, full-build SHA1 == oracle) + .md; manual-lane layer-2 cheat-reviewer FAIL. The only closing form found stages the in-loop packet tag constants (4/3/2) through a fresh `s16` local written 11x (`half = 4; *packet++ = half; ...`), so loop.c (threshold 58, inner loops 50/46 real insns) never admits them as movables. In the target the tag carrier (v0) is a different pseudo from every existing variable (flags = a2), and s32 carriers are CSE-folded. Ruled out under the y1/`c` multi-write-carrier lineage (decisions.md:1731, :16313).
disposition taken: not committed; func_8003FA24 stays INCLUDE_ASM/active at floor 17 (plain stores) with every other construct cleared by layer-2. Question for the owner: is a fresh HImode staging local whose every write is a real, consumed store value (no dead writes) inside or outside the multi-write-carrier ban?
resolution (owner, 2026-09-22): AUTO-REJECT per escalation-not-parked (no SOTN precedent + backsliding). SOTN master scan (tmp/sotn-decomp @ aa53500, tmp/fa24/sotn_scan.py): zero instances of one local assigned >=2 constants each immediately stored; the literal-then-store shape occurs only twice tree-wide, never repeated. The nearest hit (src/dra/8D3E8.c var_v1) is a per-switch-arm shared temp written once per path, not straight-line staging. The fresh multi-set staging carrier stays banned for func_8003FA24 by any spelling; the function keeps grinding for an authentic source shape.

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

## 2026-09-27 — func_800187F4 (also func_800288C8, func_8002A458) — the square-root routine's copy of the squared length for the GTE instruction — policy-question
category: policy-question
evidence: memory/grind/func_800187F4/evidence.md [s2] and [s2 cont.] + template.c/candidate.c (real Makefile recipe on the spliced TU, tmp-free harness memory/grind/func_800187F4/tools/fast.sh: 662/662 byte match; the sandbox cannot score it yet because the ldlvl/stlvl/lddp/sqr0/gpf0/gpl12/rtv0tr header statements are not in engine/gtemacro.py PINNED). The rope integrator takes two vector lengths with the LUT square root (D_8008D118 + GTE leading-zero count). In the original, the first root's code copies the squared length into a second register just before the branch (`beqz v0; move a0,a1`, 0x80018E24) and hands that copy to the GTE `mtc2 $30`; the compare, the small-value table lookup and the shift all use the original (a1); later the same a0 receives the table byte (`lbu a0`). GCC 2.7.2 keeps two registers only for two C variables. A fresh single-use copy variable does survive (cse does not merge it because the GTE statement is on the other branch) but global.c seats it in $v1 (conflicts only $v0 and the asm clobbers $t4-$t7; find_reg takes the lowest free register) — 2 instructions off. Only when the copy's variable also holds the table byte (so it is live while the shift amount occupies $v1) does it land in $a0, which is the func_800288C8 `tbl` / func_8002A458 `lzc_in` shape that layer-2 FAILED on 2026-09-25 and that Ruling 11 (C)(3) names as a bare copy. func_80018094 (COMPLETED, 2026-09-09) carries the same copy (`lut = sum_sq;`, staged-value family, Judge PASS under the older rules). Without the copy the whole 644-instruction body is otherwise exact (every register, the frame, both roots).
disposition taken: not landed; func_800187F4 stays INCLUDE_ASM/active; the byte-exact template and the copy-free fallback (2 instructions off) are banked. Question for the owner (plain language): "Three unfinished functions (and one finished one) use the same square-root routine: the chip's leading-zero instruction gets a COPY of the squared length, in its own register, and that register later holds the lookup-table byte. The only C that reproduces this is one local variable that first receives a plain copy of the squared length (used only as the chip's input) and later the table byte. Compiler dumps show a separate copy variable always lands in the wrong register, because only the later table-byte use keeps the other register busy. Our rules refuse a value that is just a copy of another variable. Allow this one kind of copy — a copy of a still-live local whose only reader is a GTE macro input, in a variable that later holds a real computed value — with the same dump proof, an honest name and layer-2 review?"
options: (A, recommended) Allow it narrowly under Ruling 11: the copy counts as a value only when its sole reader is a verbatim GTE macro's input operand, the copied variable is still live afterwards (so the copy is what makes the second register), the same local's other values are real computations, and the R11 (D) dump proof covers every copy-free spelling. (B) Do not allow — these functions stay INCLUDE_ASM and keep grinding in ordinary C.
