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
evidence: scan_hand_coded --single func_8002FF20 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, pre-slim-2026-10-01:docs/grind/decisions.md:17921; row per owner ruling 2026-09-02)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-02)
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
evidence: scan_hand_coded --single func_80019310 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, pre-slim-2026-10-01:docs/grind/decisions.md:17921; row per owner ruling 2026-09-06 foreclosed-bucket review)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-06)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-08 — func_800204C0 — integration-handoff (REFUSED: no executable remedy in verdict)
category: integration-handoff (REFUSED: no executable remedy in verdict)
evidence: judge ESCALATE packet in docs/grind/decisions.md (2026-09-08 — func_800204C0 — JUDGE ESCALATE on ruling request (integration-handoff) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait))
disposition taken: REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/func_800204C0/candidate.c; re-attemptable if a later owner ruling spends this entry.

## 2026-09-21 — func_8002D780 — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002D780 tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; census row :75; row per owner Ruling 3 terms, 2026-09-15, pre-slim-2026-10-01:docs/grind/decisions.md:26863; operator-added 2026-09-21)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-21)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-21 — func_8002EBDC — canonical-asm-grant
category: canonical-asm-grant
evidence: scan_hand_coded --single func_8002EBDC tier=OWNER-CLUSTER (cop2-addressing-preamble-cluster.md (owner ruling 2026-08-17; census row :80; row per owner Ruling 3 terms, 2026-09-15, pre-slim-2026-10-01:docs/grind/decisions.md:26863; operator-added 2026-09-21)) (driver-verified); judge ESCALATE packet in docs/grind/decisions.md (2026-09-21)
disposition taken: inline_asm_canonical.txt entry written by the driver per owner ruling 2026-08-18; function stays ACTIVE for canonical-asm integration.

## 2026-09-22 — func_8003FA24 — family-candidate
evidence: pre-slim-2026-10-01:memory/grind/func_8003FA24/rejected/half-multiwrite-carrier.c (sandbox 0, full-build SHA1 == oracle) + .md; manual-lane layer-2 cheat-reviewer FAIL. The only closing form found stages the in-loop packet tag constants (4/3/2) through a fresh `s16` local written 11x (`half = 4; *packet++ = half; ...`), so loop.c (threshold 58, inner loops 50/46 real insns) never admits them as movables. In the target the tag carrier (v0) is a different pseudo from every existing variable (flags = a2), and s32 carriers are CSE-folded. Ruled out under the y1/`c` multi-write-carrier lineage (pre-slim-2026-10-01:docs/grind/decisions.md:1731, :16313).
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
evidence: pre-slim-2026-10-01:memory/grind/func_800187F4/evidence.md [s2] and [s2 cont.] + template.c/candidate.c (real Makefile recipe on the spliced TU, tmp-free harness pre-slim-2026-10-01:memory/grind/func_800187F4/tools/fast.sh: 662/662 byte match; the sandbox cannot score it yet because the ldlvl/stlvl/lddp/sqr0/gpf0/gpl12/rtv0tr header statements are not in engine/gtemacro.py PINNED). The rope integrator takes two vector lengths with the LUT square root (D_8008D118 + GTE leading-zero count). In the original, the first root's code copies the squared length into a second register just before the branch (`beqz v0; move a0,a1`, 0x80018E24) and hands that copy to the GTE `mtc2 $30`; the compare, the small-value table lookup and the shift all use the original (a1); later the same a0 receives the table byte (`lbu a0`). GCC 2.7.2 keeps two registers only for two C variables. A fresh single-use copy variable does survive (cse does not merge it because the GTE statement is on the other branch) but global.c seats it in $v1 (conflicts only $v0 and the asm clobbers $t4-$t7; find_reg takes the lowest free register) — 2 instructions off. Only when the copy's variable also holds the table byte (so it is live while the shift amount occupies $v1) does it land in $a0, which is the func_800288C8 `tbl` / func_8002A458 `lzc_in` shape that layer-2 FAILED on 2026-09-25 and that Ruling 11 (C)(3) names as a bare copy. func_80018094 (COMPLETED, 2026-09-09) carries the same copy (`lut = sum_sq;`, staged-value family, Judge PASS under the older rules). Without the copy the whole 644-instruction body is otherwise exact (every register, the frame, both roots).
disposition taken: not landed; func_800187F4 stays INCLUDE_ASM/active; the byte-exact template and the copy-free fallback (2 instructions off) are banked. Question for the owner (plain language): "Three unfinished functions (and one finished one) use the same square-root routine: the chip's leading-zero instruction gets a COPY of the squared length, in its own register, and that register later holds the lookup-table byte. The only C that reproduces this is one local variable that first receives a plain copy of the squared length (used only as the chip's input) and later the table byte. Compiler dumps show a separate copy variable always lands in the wrong register, because only the later table-byte use keeps the other register busy. Our rules refuse a value that is just a copy of another variable. Allow this one kind of copy — a copy of a still-live local whose only reader is a GTE macro input, in a variable that later holds a real computed value — with the same dump proof, an honest name and layer-2 review?"
options: (A, recommended) Allow it narrowly under Ruling 11: the copy counts as a value only when its sole reader is a verbatim GTE macro's input operand, the copied variable is still live afterwards (so the copy is what makes the second register), the same local's other values are real computations, and the R11 (D) dump proof covers every copy-free spelling. (B) Do not allow — these functions stay INCLUDE_ASM and keep grinding in ordinary C.

## 2026-09-29 — naming sweep — name-drift alias table — policy-question
category: policy-question
evidence: naming sweep 2026-09-29 (docs/naming/sweep-2026-09-29/) renamed functions whose rulings / journal entries were filed under the old names; engine/dossier.py aliases() resolves them from this table.
disposition taken: alias table recorded here; no policy change.
  math_TransposeMatrixInPlace = func_80042ED8 · _SsSeqGetEof = func_80084A7C

## 2026-09-29 — func_80070188 (with func_8006E534) — a word view of the 0x800A3560 slot records — policy-question
category: policy-question
evidence: pre-slim-2026-10-01:memory/grind/func_80070188/evidence.md [s2] (layer-2 FAIL section) + rejected/record-merge-e534-word-pun-0.c + tools/merge_reps.py (`union_reps`). The bytes at 0x800A3560 are two 3-byte records (base+stride addressing in the original binary: func_8006F100.s:73-75/100-102/267, func_80070C70.s:114-116/166, func_80070188.s:58-76/386-419). Declaring them as records closes func_80070188 (sandbox 0, full-build SHA1 == oracle, 2026-09-29) and deletes two FAKE named intermediates elsewhere (func_8006F97C `rec`, func_80070C70 `ctx`). Layer-2 PASSED every construct but FAILED the landing on one line: func_8006E534 (COMPLETED, on main) initializes the first four bytes with one `sw -1` (func_8006E534.s:85), spelled `*(s32 *)D_800A3560 = -1;`, which is a per-use pointer pun over the merged object (aggregate-merge prong (d)). Four member stores compile to four `sb`. A declaration with a genuine word member, `union { Unk800A3560Record rec[2]; s32 word; } D_800A3560;` with `D_800A3560.word = -1;`, matches every consumer (func_80070188 and all eight others; only GPREL-name artifacts remain before the landing rebuild; measured 2026-09-29 with tools/sc.py MERGE=union).
disposition taken: not landed; func_80070188 stays INCLUDE_ASM/active; the manual worker keeps searching for a closing form without the merge. Question for the owner (plain language): "The 6 bytes at 0x800A3560 are two small 3-byte records, and declaring them that way makes func_80070188 match exactly (and removes two workaround variables elsewhere). One already-finished function clears the first 4 of those bytes with a single 32-bit store, which today is written as a pointer cast; the reviewer rejected keeping that cast once the bytes are declared as records. The only cast-free spelling is to declare the storage as a union of the two records and one 32-bit word, and write the word. May the declaration be a union with a word member for this purpose?"
options: (A) Allow the union: the record array plus one s32 member at the same address, used only by func_8006E534's single word store; layer-2 reviews the landing. (B) Do not allow — func_80070188 keeps grinding without the record declaration.
resolution (owner, 2026-09-29): answered, twentieth batch Q33, option A in substance ("Allow, with evidence (Recommended)"; verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 20). Rule text: .claude/rules/no-new-park-categories.md, aggregate-merge "Amendment to prong (d): a union word view over small fields"; record docs/grind/decisions.md 2026-09-29 OWNER RULING — a union word view over small fields. func_80070188 had already landed COMPLETED-C (9e69a87c7) without the record declaration; a record-union respelling of D_800A3560 and its consumers (func_8006E534's word store included) is a fresh submission under that amendment, with its own layer-2. Nothing is pre-decided.

## 2026-09-29 — func_8005C8A8 — a constant kept in a stack slot, spelled as cancellation through a second name — policy-question
category: policy-question
evidence: memory/grind/func_8005C8A8/evidence.md s3b + s3c; probes/s3b/size_dumps.txt (landed and literal dumps with command lines); probes/s3b/engine_sandbox_scores.txt; rejected/size-tile-cancel-0.c; fix1-merges.patch. The function returns 0x4F0, the bytes of the prim buffer it fills. The original sets that value once at entry into a stack slot and loads it at the return (`li $t0,0x4F0; sw $t0,0x70($sp)` ... `lw $v0,0x70($sp)`). Written as `size = 0x4F0;`, cse gives the set a REG_EQUAL constant. local-alloc.c update_equiv_regs (1024-1032, 1078-1110) makes it REG_EQUIV and, since it is read once, rewrites the return to `li $v0,0x4F0` and deletes the set: the slot disappears and every later frame offset shifts, sandbox 33. Written as `size = (s32)tile + 0x4F0 - arg2;` (tile was just set to arg2), the constant only appears after combine, with no REG_EQUAL, and the whole function byte-matches (sandbox 0, full-build SHA1 == oracle). Layer-2 round 2 FAILED that spelling as a hidden constant (a second name for arg2 cancelled against it: the named-intermediate entry's "no extra handles to one object"). Layer-2 PASSED everything else in the landing: the xpos Q27 (A) local, and the five game.h merges. `register`, `u32` and a moved set all keep the literal behaviour; the target computes no end-of-buffer value to derive it from.
disposition taken: not landed; func_8005C8A8 stays INCLUDE_ASM/active (no rotation); candidate.c is the literal form (33 with fix1-merges.patch applied).
Question for the owner (plain language): "This function returns a fixed number (0x4F0). The original program stored that number in a stack slot when the function starts and read it back at the end. If we write the number directly, the compiler notices it is a constant and just re-creates it at the end, so the slot vanishes and the function stops matching. The only C we have found that keeps the slot writes the number as 'buffer start + 0x4F0 - buffer start' (the same address under two names, cancelling out), which the reviewer rejected as a disguised constant. May a constant-holder local be spelled this way, as a FAKE-annotated cancellation, when the dumps show the plain literal loses its stack slot to update_equiv_regs?"
options: (A) Allow narrowly: a FAKE-annotated constant-holder whose initialisation cancels one live value against itself, only when dumps show the literal is rematerialized by update_equiv_regs and loses a slot the target has; layer-2 reviews. (B) Do not allow — func_8005C8A8 keeps grinding for another source form.
resolution (owner, 2026-09-30): answered, twenty-third batch Q45, option B ("Don't allow it [...] It exists only to hide the constant from the compiler [...] The function stays unfinished, 33 instructions off, and keeps being worked for another spelling."; verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 23, d023686ea). The cancellation spelling (`size = (s32)tile + 0x4F0 - arg2` and any variant that cancels a live value against a second name for it) is REFUSED, not narrowly admissible. func_8005C8A8 stays INCLUDE_ASM/active (no rotation) at admissible floor 33 (candidate.c literal form + fix1-merges.patch); rejected/size-tile-cancel-0.c stays rejected. Ledger note: memory/grind/func_8005C8A8/evidence.md s3d.

## 2026-09-30 — ORCHESTRATOR READING (Ruling 14): Ruling 13 (B) renewed for this run — policy-question
category: policy-question
evidence: .claude/rules/ordinary-c-judge-decidable.md § Ruling 14 (A); owner's words verbatim in docs/grind/owner-rulings-2026-09-26.md, twenty-eighth batch ("I give you authority to grant borderline rule questions if they are within SOTN's standards and are by no means a cheat or workaround or regression."). Ruling 13 was scoped "until the owner next speaks"; the owner has spoken since, in batches 20-28, and the owner's 2026-09-30 words do not name Ruling 13 (B). The author's reading, not the owner's words: clearly-fine ordinary C with a truthful semantic reading (Ruling 13 (B)'s test) is "within SOTN's standards" and by no means a cheat or workaround, so Ruling 13 (B) is renewed for the 2026-09-30 overnight run, still yielding to the entry that governs a construct.
disposition taken: renewed for the 2026-09-30 run only, pending owner ratification; unratified, it is not precedent after the run; functions landed under it stay landed.
resolution (owner, 2026-09-30): ratified, thirtieth batch Q64 ("for item 4, i agree with that precedent. Honest C is the explicit goal"; verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 30). The author's reading, flagged to the owner: STANDING, not run-scoped. Ruling 13 (B) is standing together with Ruling 13 (D)'s wall as its limit; Ruling 13 (C)/(E) and Ruling 14 (B)-(D) stay run-scoped. Rule text: .claude/rules/ordinary-c-judge-decidable.md § Ratification (owner, 2026-09-30, thirtieth batch, Q64).

## 2026-09-30 — cdrom_SetMix + func_80035F78 (likely func_80036140) — C tentative definitions with ASPSX's COMMON rule — policy-question
category: policy-question
evidence: pre-slim-2026-10-01:memory/grind/cdrom_SetMix/evidence.md 2026-09-30 (commit 200b10c10) and probes-0930/; pre-slim-2026-10-01:memory/grind/func_80036140/research-common-gp.md § 0 (the measured ASPSX 2.34 table: only a `.comm` tentative definition gives gp at the base and lui/%lo at sym+k, which is the target's shape for g_cd_atv / D_800A36B8; extern, static and initialized all give gp at every offset). Scratch closing form, nothing in-tree touched: `CdlATV g_cd_atv;` / `CdlATV D_800A36B8;` as tentative definitions in code6cac_b4.c (probes-0930/b4_tentative.diff); a 1-line maspsx parser fix for the 3-field `.comm x,4,1` our cc1 writes (probes-0930/maspsx_comm3_parse.diff; today maspsx crashes on it, probes-0930/p_tentative.c); maspsx's existing upstream flag `--use-comm-section`, so the storage stays a COMMON symbol that GNU ld resolves to the symbol-file address without allocating it (probes-0930/ldtest/). Result: cdrom_SetMix 0/18, func_80035F78 0/12, with maspsx_comm_syms.txt empty (the gp choice is upstream maspsx's own `.comm`-driven rule, not a per-function list). Byte-neutrality of parser fix + global flag on today's tree: every src/*.c compiled both ways with its exact Makefile recipe, 53/53 objects identical, 0 failed (probes-0930/bothways.sh). decisions.md 2026-09-30 (COMMON-gate ruling, point 3) says such a global, declaration-driven model needs its own owner ruling.
disposition taken: not decided under Ruling 14 (B)(3) — build-flag/substrate change; stays open for the owner; items stay active, no rotation. cdrom_SetMix / func_80035F78 remain INCLUDE_ASM at floor 6.
Question for the owner (plain language): "Two small CD functions (and probably a third) need their variables declared the way the original programmer did: 'declared, no starting value'. Sony's assembler treats that kind of variable in a special way, and that is exactly what the shipped bytes show. Our C can already say it, but two things in our assembler helper tool stop it from working: a one-line parsing bug (our compiler writes the declaration with one extra field the helper doesn't expect), and an existing, off-by-default option of the helper's upstream project that keeps such variables as 'common' symbols so the linker puts them at their known addresses. Both changes leave every current file byte-for-byte identical. This is not the per-function list you ruled a cheat: nothing names a function, and the behaviour follows from how each file declares its variables. Adopt the parser fix and turn the option on for all files?"
options: (A, recommended) Adopt both as their own reviewed commit (full oracle check, engine buildconfig mirrored, layer-2), then land cdrom_SetMix and func_80035F78 with tentative definitions, each with its own layer-2; func_80036140 judged fresh. (B) Do not adopt — the three functions stay INCLUDE_ASM and keep being worked in ordinary C (no ordinary-C route found so far).
resolution (owner, 2026-09-30): answered, thirtieth batch Q62, option A ("Go ahead with your recommendations."; verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 30). The maspsx three-field .comm parser fix and the upstream --use-comm-section flag for every file land as their own byte-neutral substrate: commit with a layer-2 PASS; then cdrom_SetMix and func_80035F78 land with tentative definitions, each with its own layer-2 (a tentative definition only for an object whose original bytes are all zero); func_80036140 judged fresh. maspsx_comm_syms.txt stays retired and empty. Rule text: .claude/rules/maspsx-gate-lists.md § The global COMMON model. Nothing is pre-decided.

## 2026-09-30 — func_8002D780, func_8002EBDC, func_8002F2D0, func_8002F770 — gte_rtv0's DMPSX command word — policy-question
category: policy-question
evidence: memory/grind/{func_8002D780,func_8002EBDC,func_8002F2D0,func_8002F770}/evidence.md 2026-09-30 and candidate.c (commits de1a4ca43, bceeedb8a). All four, reopened under Q38/Q41, now match (func_8002D780 0/202 on the sandbox; func_8002EBDC 0/182, func_8002F2D0 0/270, func_8002F770 0/298 measured with probes-0930/score_nostrip.py because gte_SetRotMatrix / gte_ldlv0 are not yet in engine/gtemacro.py PINNED) with EVERY GTE island written as the pinned inline_o.h / gtemac.h statements (gte_Lzc, gte_ApplyRotMatrix, gte_SetRotMatrix, gte_ldlv0, gte_ldv0, gte_rtv0, gte_stlvnl; "$12"-"$15","memory" on each), except one character run: gte_rtv0's DMPSX placeholder `.word 0x0000013f` is carried as the post-DMPSX word `.word 0x4A486012` (once in D780/F2D0/F770, twice in EBDC), which the 2026-09-26 class grant's prong (C) sends to the per-function owner-row route. Precedent: the identical word, granted per function for func_8002DE20 (Q11) and func_800187F4 (Q29); independent mapping already recorded (nugget inline_n.h :516-520, PSn00bSDK inline_c.h :1183-1186). Other open items, not part of this question: the PINNED entries for gte_SetRotMatrix / gte_ldlv0 (an engine: commit with layer-2), func_8002D780's two remaining FAKEs (`flag` parameter staging under staged-value-reused-variable, 9/202 without it; the `m = dist` re-store, 4/202 without it), and the det/sum shared-value paperwork for F2D0/F770 (Ruling 11). Checked alternative: spelling gte_rtv0 from inline_c.h (its one-statement form, where the 2026-09-24 Extension admits the swap without an owner row) also scores 0 in all four (candidate_alt_rtv0_inline_c_h.c), but it would mix islands from two Sony headers inside one function, and inline_c.h and inline_o.h define the same macro names, so one original source file could not have included both; not proposed.
disposition taken: not decided under Ruling 14 (B)(3); stays open; items active, no rotation.
Question for the owner (plain language): "Four 3D-math functions now match the original exactly using only Sony's own chip-command snippets, copied character for character, except for one detail: one 'rotate vector' command. Sony's header writes it as a placeholder number, and a separate Sony tool replaced it with the real command after compiling. We don't have that tool, so the snippet carries the real command, which is what the game contains. You approved exactly this swap, one function at a time, for func_8002DE20 and func_800187F4. Approve the same swap for these four functions?"
options: (A, recommended) Grant a per-function row for each of the four, covering only gte_rtv0's post-DMPSX word 0x4A486012 (like Q11/Q29); each landing still needs its other open items and a fresh layer-2. (B) Do not grant — the four stay INCLUDE_ASM and active.
resolution (owner, 2026-09-30): answered, thirtieth batch Q61, option A ("Go ahead with your recommendations."; verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 30, with a correction: the question said func_800187F4 had this exact word approved; Q29 covered four other DMPSX words). Four named per-function grants for the gte_rtv0 post-DMPSX word 0x4A486012 only; the 2026-09-26 class prong (C) stays strict (the standing-rule idea was offered outside the recommendation and is not adopted). Rows land with each function. Rule text: .claude/rules/inline-asm-policy.md § Per-function grants: func_8002D780, func_8002EBDC, func_8002F2D0, func_8002F770. Each landing still owes its other open items and a fresh layer-2.

## 2026-09-30 — func_8001C8DC (+ func_8003CF84) — two scalar bytes indexed from the first one's address (F4) — policy-question
category: policy-question
evidence: pre-slim-2026-10-01:memory/grind/func_8001C8DC/evidence.md s1 (commit 6c7d46d1a) and s1/ (scratch full builds of HEAD, scripts, variants, disassembly diffs). The reopened body (rejected/retro-audit-2026-09-29.c: two scalars D_800A37D2 / D_800A37D3 plus `p = &D_800A37D2; p[t != 0]++`) matched the oracle when it landed (025f88d91; sandbox 2 = the jtbl %lo addend, operand-only). Measured replacements: (1) one object `u8 D_800A37D2[2]`, every C consumer converted (func_8001C8DC, func_800343F0, func_8003CF84, func_8001CE60, func_80055138), D_800A37D3 row dropped: full build 687de142b791d59599e4d5340878736005c9ef6e, 42 differing words, only in func_8001C8DC's two `>= 100` clamps and func_80055138's four uses; the flag- and player-indexed accesses do match. Mechanism: the target reaches each byte by its own symbol at multi-use sites (`lui $v0,%hi(D_800A37D2); lbu $v0,%lo(D_800A37D2)($v0)` ... `sb $v0,%lo(D_800A37D2)($at)`, the same for D_800A37D3), which is a scalar's DECL_RTL; an array/struct element at a constant offset goes through change_address -> memory_address (emit-rtl.c:1293-1315), which forces constant addresses into a register (explow.c:396-399) that cse shares, so the direct form cannot be produced; respelled clamps (`*D`, `*(D + 1)`) give the same SHA1. (2) plain branches 29 / 36. (3) `(&D_800A37D2)[t != 0]` without the pointer local 19. The accesses are lui/%hi + %lo, not gp-relative ($gp), and neither symbol is in sdata_syms.txt, so the -G0/-G8 choice does not bear on them and the per-file -G8 route (Q10/Q44/Q54) cannot help. A per-file declaration (Q21) cannot either: func_8001C8DC needs both properties in one function. SOTN matched PS1 code has no F4 precedent (2026-08-18 survey; the !FAKE `&entity_ranges[0]` at src/dra/7879C.c:1949-1952 @aa53500 stays inside one array), so Q55 does not apply.
Side note, same bytes: func_8003CF84 (src/code6cac_c2.c:954), a completed function, carries the refused `(&D_800A37D2)[D_800A3748]` idiom on main — a retro-audit-class defect. Not touched; with the array declaration it matches, so it is decided together with this question.
disposition taken: not decided under Ruling 14 (B)(3): F4 refusal (2026-07-20; 2026-08-18 "Surveyed and NOT extended"); stays open for the owner; func_8001C8DC stays INCLUDE_ASM/active, no rotation.
Question for the owner (plain language): "The shipped code treats the two bytes at 0x800A37D2 and 0x800A37D3 as two separate one-byte variables (each read and written by its own name), and also reaches the second one by indexing from the first one's address. Our compiler cannot produce both behaviours from a single array or struct: element accesses come out through a shared address register, which the game does not have. The only C that matches is the two separate byte variables plus `p = &D_800A37D2; p[t != 0]++`, the 'index past one variable into the next' pattern you refused on 2026-07-20. Allow it for this byte pair, annotated FAKE, with this compiler evidence as the proof?"
options: (A) Admit this pair: two scalar declarations plus the pointer index from &D_800A37D2, FAKE-annotated at each use and citing this compiler proof; func_8001C8DC lands with a fresh layer-2, and func_8003CF84's `(&D_800A37D2)[D_800A3748]` becomes admitted with the same annotation. (B) No — the measurements stand as recorded; func_8001C8DC stays INCLUDE_ASM/active and keeps being worked; func_8003CF84's line stays recorded debt for a later fix.
resolution (owner, 2026-09-30): answered, thirtieth batch Q63, option A CONDITIONAL ("Go ahead with your recommendations."; verbatim record docs/grind/owner-rulings-2026-09-26.md, batch 30): first measure a two-member struct at 0x800A37D2 (every consumer converted, full build); if it and the array both fail, the two scalars plus the FAKE-annotated index from &D_800A37D2 are admitted for this pair only, the pointer local p carrying its own FAKE annotation under pointer-alias-fake-exception's prerequisites (func_8001C8DC; func_8003CF84 via a cheat-cleanup: commit). If a single object matches, it is used instead. Rule text: .claude/rules/no-new-park-categories.md § Owner ruling 2026-09-30 — the D_800A37D2 / D_800A37D3 byte pair.

## 2026-10-01 — func_8005C8A8 — the 0x4F0 slot after Q45: sibling end-pointer evidence, every other mechanism measured — policy-question
category: policy-question
evidence: memory/grind/func_8005C8A8/evidence.md s4 + s5; probes/s5/scores.txt (variant files and harness alongside); probes/s3b/size_dumps.txt. Floor 33 re-measured 2026-10-01 (candidate.c + fix1-merges.patch's game.h hunk); everything else in the function already passed layer-2 (xpos Q27 (A), the five game.h merges). The target stores 0x4F0 once at entry into a stack slot and loads it at the return. A literal is rematerialized (cse REG_EQUAL -> update_equiv_regs REG_EQUIV, local-alloc.c 1019-1032). Escaping that needs either (1) a set whose constant cse cannot see, which only cancellations give (Q45 refused `(s32)tile + 0x4F0 - arg2`), or (2) two counted sets with one store. New since Q45, measured: a running buffer-offset accumulator (`size = 0x4D8; mode_off = arg2 + size; size += 0x18;` and a 3-step form) stays 33, because cse folds the first value into its user and flow deletes the dead set before counting; a one-member union constructor keeps the slot through its (clobber), 4 off, but has no semantic purpose. New evidence: eight finished functions in the same file (func_8005D814, func_8005E098, func_8005E54C, func_8005F1C8, func_8005FC9C, func_800600C8, func_80060414, func_80060768) set `end_off = <buffer start> + K` at entry and `return end_off - <start>;` (their targets subtract at the return). Spelled that way here, the function scores 110 (probes/s5/e1.c): here the original subtracted at entry. `end_off = arg2 + 0x4F0; size = end_off - arg2;` at entry matches exactly (probes/s5/e2.c, sandbox 0); layer-2 failed it on 2026-09-29 as a round trip (end_off's value is not in the target's bytes) before Q45, and Q45's reasoning covers it.
disposition taken: not landed; func_8005C8A8 stays INCLUDE_ASM/active at floor 33 (no rotation); probes banked.
Question for the owner (plain language): "func_8005C8A8 returns a fixed size, 0x4F0, that the original kept in a stack slot from entry to exit. You refused the 'start + 0x4F0 - start under another name' spelling (Q45). I measured every other route: a running offset through the buffer, structs, wider types, and the author's own pattern. Eight finished sibling functions in the same file write `end = start + size` at entry and `return end - start;` at the end. Copied as-is it misses badly here, because this original subtracts at entry. The only exact form is that same end-pointer variable with the subtraction moved to entry: `end_off = arg2 + 0x4F0; size = end_off - arg2;`. The only other thing that keeps the slot is a do-nothing one-member union, which I won't propose. Allow the sibling end-pointer form, FAKE-annotated and citing the siblings, or keep the function open?"
options: (A) Allow narrowly for func_8005C8A8: `end_off = arg2 + 0x4F0; size = end_off - arg2;` at entry, with a FAKE annotation citing the eight siblings' end_off idiom and the dumps; landing with a fresh layer-2. (B) Do not allow: Q45 stands for this form too; func_8005C8A8 stays INCLUDE_ASM/active at 33 and is worked only if a new mechanism turns up. Recommendation: (B) unless the owner reads the sibling idiom as the author's real source. The value is still a constant computed by cancellation, which is what Q45 refused; the siblings show the end pointer but not this placement.
update (2026-10-01, deeper round, laneA): dumps name the deciding pass (evidence.md s5b): with the literal, cse writes REG_EQUAL 1264 and local-alloc update_equiv_regs promotes it and folds it into the return (the slot disappears); with the matching forms, cse sees no constant, combine folds the subtraction to 1264 with no note, and the pseudo is spilled to the original's slot. One more exact form, without a second variable name: describe the chunk as a type of its real layout (15 tiles, the 0x3E8 sprite area, 0x18 of draw-mode space = 0x4F0) and write `size = (u8 *)((Buf5C8A8 *)arg2 + 1) - (u8 *)arg2;` (end of the chunk minus its start; probes/s5/t2.c, sandbox 0). It matches only while arg2 stays an integer: with arg2 typed as a pointer to that struct, the identical subtraction folds early and scores 33 (p1). So it is still a constant computed by cancellation, with the compiler's view blocked by the integer-to-pointer cast instead of by a second name. Added option: (C) Allow `(u8 *)((T *)arg2 + 1) - (u8 *)arg2` with T declared from the function's own accesses, FAKE-annotated, fresh layer-2. Recommendation unchanged: (B).

## 2026-10-01 — func_80058580 (a) — one work variable whose stale value the original reads on one path — policy-question
category: policy-question
evidence: memory/grind/func_80058580/evidence.md [s6] (layer-2 FAIL rev-58580-r11, body e5799063f1d7abd7, rejected/l2-fail-2026-10-01-e5799063.c) and r11/README.md. The body links byte-identical (full build SHA1 == oracle). In the target, the 0x394 slot switch reads $s3 with no write on one path (p[0x39C] == 1 and opponent state neither 0x19 nor 0x1A, 0x80059D6C -> 0x80059DB0): it switches on whatever the CPU's scratch register held from earlier code (the side bit, angle, guard product, force flag, path flag, bearing or coin bit, depending on the path). Only one shared C variable (`work3`) reproduces that; split locals leave the slot uninitialized there (it is spilled, 7652 differing words). Under Ruling 11's value definition (writes that can reach a common read) this makes nine of work3's roles one merged value, so the per-value twins and ablations are not true one-variable-per-value spellings, and Ruling 11 cannot certify it.
Question for the owner (plain language): "In this AI function the original code, on one rare path, makes a decision based on a scratch variable it never set on that path, so it uses whatever an earlier, unrelated calculation left there. Our C reproduces that only with one shared scratch variable (`work3`) across those calculations, which is what the shipped bytes show. The reused-variable rule (Ruling 11) cannot certify it, because that stale read joins nine of the variable's jobs into one 'value'. Allow the shared scratch variable for this function, documented as reproducing an original read-before-write, or keep the function open?"
options: (A, recommended) Allow narrowly for func_80058580: work3 lands as one Ruling 11 variable whose merged stale-read value is disclosed in its comment (the path, the target addresses), with the rest of the Ruling 11 package and a fresh layer-2. (B) Do not allow: the function stays INCLUDE_ASM/active. There is no other C that reproduces the read-before-write.
recommendation: (A). The behaviour is in the shipped bytes and the read is disclosed, not hidden; no other spelling exists.

## 2026-10-01 — func_80058580 (b) — Q34 copy clause with a constant start value (running maximum) — policy-question
category: policy-question
evidence: memory/grind/func_80058580/evidence.md [s6], r11/README.md (value `best` of work2). The random pick loop keeps the best score so far in work2: `work2 = -1;` before the loop, `work2 = score;` when a pick beats it (target `li $s2,-1` 0x8005A108/0x8005A118, `addu $s2,$s3,$zero` 0x8005A350). Splitting it out as its own local costs 23 words (r11/scores_v1.txt). The reviewer: Q34 admits exactly one plain copy write and nothing else in that value, and Q20 needs two different constants, so no clause admits `-1` plus a copy.
Question for the owner (plain language): "A shared scratch variable holds a running maximum for a few lines: start at -1, then copy in each better score. The copy rule (Q34) allows one copy and nothing else, and the constants rule (Q20) needs two different constants, so this ordinary 'start at -1, keep the best' pattern falls between them. Extend Q34 to allow one constant start value plus one copy when the target shows both, or keep it refused?"
options: (A, recommended) Extend Q34: a value may be one constant initial write plus one plain copy, both shown in the target, with the usual Q34 receipts. (B) Keep refused: work2's `best` must be split or respelled (costs 23 words so far; search continues).
recommendation: (A). It is the plainest loop-maximum code; nothing is added or hidden.

## 2026-10-01 — func_80058580 (c) — `0x200 - ((x * 0x100) >> 12)` instead of `0x200 - (x >> 4)` vs Q45 — policy-question
category: policy-question
evidence: memory/grind/func_80058580/evidence.md [s5] (13 -> 7) and [s6] (rev-58580-dm). Two sites compare 0x43C against 0x200 minus 0x438 scaled down 16x. The target loads 0x438 with `lh` and shifts right 4 (`lh; sra 4` at 0x8005AB34 and 0x8005ACCC); these are the only such unfolded pairs in the whole binary. With `x >> 4` GCC's cse merges the shift into the halfword sign-extension (cse.c fold_rtx associative-shift fold) and emits `lhu; sll 16; sra 20` (6 words off). Every equivalent fixed-point rescale (`(x * 0x100) >> 12`, `(x << 8) >> 12`, `(x * 2) >> 5`, `(x << 4) >> 8`, ...) gives the target's `lh; sra 4`; nothing else tried does (s16/s32 temps, `/ 16`, inline functions, bitfields, statement splits; also the original PsyQ cc1psx folds `>> 4`). The function itself scales 0x438 by `* k >> 12` elsewhere (`(CPU_S16(0x438) * work3) >> 12`), so `* 0x100 >> 12` reads as the same 4.12 fixed-point idiom with k = 0x100 (1/16). The reviewer: the form was picked among equivalent rescalings only to dodge the fold, close to Q45 (constant cancellation refused).
Question for the owner (plain language): "Twice the code divides a value by 16 the 'fixed-point' way. Written as `x >> 4`, our compiler merges the shift into the load and produces different bytes; written as `(x * 0x100) >> 12` (multiply by the 4.12 constant for 1/16, then drop 12 bits, the same pattern the function already uses for this value) it produces exactly the shipped bytes. Unlike Q45 nothing cancels out: it is one ordinary fixed-point multiply. Is that acceptable, annotated, or does Q45 cover it?"
options: (A, recommended) Allow as ordinary fixed-point C at these two sites, with a comment naming the 4.12 factor and the measured cse fold. (B) Treat as Q45-class: refuse; the function stays open on these two sites (6 words).
recommendation: (A). It is a real fixed-point scaling the same function uses for this field, not a cancellation; disclosed at the site.

## 2026-10-01 — func_80058580 (a)/(b)/(c), func_8005C8A8 — owner rulings Q74-Q77 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 35; docs/grind/decisions.md 2026-10-01 OWNER RULING Q74-Q77.
disposition taken: the three 2026-10-01 func_80058580 policy-questions are SPENT (Q74 (a) allowed narrowly, Q75 (b) Q34 extended, Q76 (c) allowed at the two sites); the 2026-10-01 func_8005C8A8 policy-question is closed REFUSED (Q77: Q45 stands for options A and C). Each landing still needs its own fresh layer-2.

## 2026-10-01 — func_800770B8 — p_old restore store (Q66 re-ask after Q65) — policy-question
category: policy-question
evidence: memory/grind/func_800770B8/evidence.md (2026-10-01 laneA entries; probes/s40/). Q66 refused the dead restore `p_old = prev;` "for now", to be re-measured after Q65. Measured on the regenerated Q65 series with the f1C/f20 unions: candidate.c 0/175, k0 (no restore) still 2/175 (the 0x30/0x34 clears on $s1, target $v0) — Q65 does not touch it. cc1 dumps name the decision: cse puts p_old and the call's $v0 in one class with p_old canonical (make_regs_eqv), so the D_800A36A0 reloads for the clears become p_old; only a write to p_old between the f04 store and the clears takes it out of the class. For no extra instruction that write's value must never be materialized: dead (the refused restore) or folded by combine into its user. The only non-dead form, a pointer pre-increment for the f04 store (`*++p_old = (s32)prev;`), gives the target's register split but its plain-register store makes cse forget D_800A36A0 (cse.c 7564-7574): one extra lw (3/176). In-struct variants need Q45-refused constant cancellation. Permuter from k0: 36,505 iterations, no find. Prior record: 39 sessions + laneD (class B), no other route.
Question for the owner (plain language): "func_800770B8 (SelWork setup) still matches only with the variable-reuse form you refused in Q66: one variable holds the list pointer, then the new work-area pointer, then is set back to the list pointer with a do-nothing store. You asked to re-check after the gp change (Q65); I did: it changes nothing (still 2 instructions off without the store). The compiler dumps show why: the shipped code requires that variable to be overwritten right there with a value that never reaches an instruction, which in C means a dead store (or a pointer increment, which costs one extra load here). So the original source almost certainly contained such a store. Allow it now, narrowly, or keep the function open?"
options: (A) Allow narrowly for func_800770B8 (Q66's "Allow narrowly"): p_old as a Ruling 11 reused local (list pointer, work pointer) plus the `p_old = prev;` restore as a FAKE dead store, with the cse dump proof (this entry's evidence), the Ruling 11 package and a fresh layer-2. (B) Keep refused: the function stays INCLUDE_ASM/active; no admissible spelling is known.
recommendation: (A). The target bytes require a write whose value is unused at exactly that point; the dead store reproduces it with nothing hidden, and every alternative route (39 sessions, Q65 re-measure, permuter, the pre-increment) is measured.

## 2026-10-01 — func_800770B8 — owner ruling Q78 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 36; docs/grind/decisions.md 2026-10-01 OWNER RULING Q78.
disposition taken: the 2026-10-01 func_800770B8 policy-question (Q66 re-ask after Q65) is SPENT: option (A) allowed narrowly. The landing still needs its own fresh layer-2.

## 2026-10-01 — q65-adoption step 12/14/15 — how maspsx aligns `static` storage (Sony measured) — policy-question
category: policy-question
evidence: 55fee8b52:memory/grind/q65-adoption/q56/adopt/lcomm_align_probe.sh + .out (Sony ASPSX 2.34 -G8 + PSYLINK 2.37 under dosemu2: twenty `.lcomm` statics of 1..16 bytes all land 4-aligned, e.g. an 8-byte static at +4, a 1-byte static followed by the next at +4); chain.sh (round-2 generators, s08..s16, oracle at every step with only the 8-byte case changed); subword.sh (46 sub-word statics in the generated blocks). Layer-2 round 1 (step 15, finding 1) requires g_anim_hit_data, D_800A344C and D_800A3454 to be 2-element s32 arrays (their code indexes element 1). Each would be an 8-byte static at a 4-mod-8 address. maspsx (upstream rule, kept by step 12) aligns an 8-byte static to 8, so the build cannot place them; Sony's tools can. The same probe shows Sony never places a static at a 1- or 2-mod-4 address, which our current blocks do 46 times (A6 filler pieces such as code6cac_c_mid's u8+s16 runs, and halfword pairs such as text1b D_800A33C8/D_800A33CA).
disposition taken: generators fixed for every other round-1 finding; step 12 unchanged; series not regenerated (step 15 cannot both satisfy finding 1 and build under the current maspsx model).
Question for the owner (plain language): "Sony's own assembler/linker put every static variable on a 4-byte boundary, whatever its size. Our assembler model lines them up by size instead (8-byte ones on 8, 2-byte ones on 2). That difference now matters: the reviewer asked for three two-element arrays that only fit Sony's way, and Sony's way also means 46 small statics we currently define on their own are really parts of bigger objects (or just padding). Should maspsx follow Sony's measured rule?"
options: (A, recommended) Model Sony fully: every static 4-aligned (step 12, with the probe as its calibration and an engine test). The three arrays land as reviewed; the A6 static filler pieces become alignment padding (no object); the gp-reached halfword names inside a 4-byte slot (D_800A33CA, D_800A33EA, D_800A345E, D_800A350E, D_800A3512 and tu1d's) become elements of their 4-aligned object, in one more byte-identical reconciliation step with layer-2 on each changed body. Most faithful; adds a step. (B) Narrow: only 8-byte statics align 4 (measured byte-identical end to end); smaller statics keep the size rule as recorded debt. Unblocks the series now but models half of a measured behaviour. (C) Keep the current model: the three per-word pairs stay two names each, which layer-2 refused (cross-symbol storage); step 15 stays blocked.
recommendation: (A). It is the measured behaviour of the original tools, and it removes the A6 static pieces instead of inventing objects for padding.

## 2026-10-01 — q65-adoption step 15 — D_800A3264 inside text1b's .sdata block, named by a pointer in asm data — policy-question
category: policy-question
evidence: tmp/orch/q65_fixes.md (step 15 finding 3); asm/data/7D920.data.s:22040 (`.word D_800A3264` at 0x8009B0D8 inside the pointer table D_8009B0C0); the table's only code user is text1b's func_80060768 (`lui/addiu %hi/%lo(D_8009B0C0)`, asm/funcs/func_80060768.s:13-14); D_800A3264 lies between text1b's gp-reached objects D_800A3250 and D_800A326C, so text1b's one .sdata section must contain it (it cannot stay in the blob without splitting a single input section).
disposition taken: generator defines it in text1b (global K3 `T D_800A3264 = <original bytes>;`, truthfully commented) and logs BORDERLINE-A2; not landed.
Question for the owner (plain language): "One 8-byte variable sits in the middle of text1b's small initialized data, so by layout it belongs to text1b. The rule also asks that no other file refers to it, and the only other reference is a pointer inside a data table whose owning source file we have not identified (only text1b's code uses that table). Accept it as text1b's (a normal global definition, so a pointer from any file still links), or hold the adoption?"
options: (A, recommended) Accept: define it in text1b as a global initialized object with the evidence in its comment; the table's ownership is recorded as open. (B) Hold: the series cannot land until the table's owner is established (there is no layout in which text1b's block excludes it).
recommendation: (A). The layout fixes the owner; a global definition is correct whichever file owns the table.

## 2026-10-01 — q65-adoption step 15 — D_800A3530 / D_800A3534 left in the data blob — policy-question (non-blocking)
category: policy-question
evidence: s15 log ORPHAN-LEFT (generator 55fee8b52:memory/grind/q65-adoption/q56/adopt/s15_apply.py § 1b); text1b_tu1d's func_80070188 / func_80070F78 reach them only through lui/%lo (indexed stores and an address-of), no gp access; D_800A3534's label spans 12 bytes, more than a small static can hold (a static over 8 bytes is .bss, not in this block); D_800A3540/D_800A3544 joined text1b_tu1d's block under (A1).
disposition taken: left in asm/data (a blob piece between text1b_tu1c's and text1b_tu1d's blocks); the series can land with them there.
Question for the owner (plain language): "Two pieces of text1b_tu1d data sit just in front of its block but cannot join it under the rule: one is 12 bytes, too big for a small static, so it was probably several smaller variables we cannot tell apart yet. Leave them as raw data for now?"
options: (A, recommended) Leave them in asm/data, logged; revisit when text1b_tu1d's accesses show the real object sizes. (B) Split the 12 bytes into small objects by guess (refused: not evidence).
recommendation: (A).

## 2026-10-01 — q65-adoption step 12/14/15 questions — owner rulings Q79-Q81 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 37; docs/grind/decisions.md 2026-10-01 OWNER RULING Q79-Q81.
disposition taken: the three 2026-10-01 q65-adoption policy-questions are SPENT: static alignment option (A) Sony model (Q79); D_800A3264 option (A) text1b K3 global, table owner open (Q80); D_800A3530/D_800A3534 option (A) left in asm/data, logged (Q81). Each series step still needs its own layer-2.

## 2026-10-01 — q65-adoption step 14 (A8) — camera_CalcAngles: the D_800A33C8 / D_800A33CA join is not byte-identical under our cc1 -G0 — policy-question
category: policy-question
evidence: 55fee8b52:memory/grind/q65-adoption/q56/adopt/cam_probe.sh + cam_probe.out, measureA2.sh, try_cam*.sh, g8diff.sh; s14_apply.py A8_PAIRS. A8 (Q79, 8c57bc4ab) joins each gp-reached s16 name at a 2-mod-4 static address into its slot's static. Four of the five pairs join byte-identically (D_800A33E8/EA, D_800A345C/5E, D_800A350C/0E, D_800A3510/12; the series builds to the oracle through step 15 with them). The fifth, text1b's D_800A33C8 / D_800A33CA, is written by camera_CalcAngles (today `D_800A33C8 = -ratan2(..); D_800A33CA = s0; return &D_800A33C8;`). As `s16 D_800A33C8[2]`, in every spelling measured (`[0]`, `*X`, `&X[0]`, a two-member struct, the stores swapped, a temporary for the first value, a do-while(0) around the stores), before and after the switch, our cc1 at -G0 puts the element's constant address in a register (memory_address: a symbol address is not a legitimate -G0 MIPS address) that cse shares with the returned address: one extra instruction, build not the oracle (+4 bytes). cc1psx -G8 on the same C (a static s16[2] written [0], [1], its address returned) emits the target's `la $2,D_pair` / `sh $3,D_pair` / `sh $16,D_pair+2` exactly, and so does our cc1 at -G8 (cam_probe.out): the original was compiled -G8. Compiling text1b with cc1 -G8 (GP_FILES) after the switch is not neutral for the rest of the file (exe 16 bytes short, first differences in .rodata at 0x80010068). Keeping the two names as two statics is not placeable once statics are 4-aligned (A8): D_800A33CA lands at 0x800A33CC.
disposition taken: series not regenerated; every other round-2 fix and the other four A8 joins are in the generators (55fee8b52:memory/grind/q65-adoption/q56/adopt/).
Question for the owner (plain language): "Sony's tools put each static on a 4-byte boundary, so D_800A33CA has to be the second half of D_800A33C8, a two-element array. One small function writes both halves and returns the array's address. Written as an array, our compiler adds one instruction, because we compile that file without the small-data option the original compiler used; the original compiler, and ours with that option, produce the exact original code, but switching the whole file to that option changes other functions. I found no other C spelling. How should this one function be handled?"
options: (A) A narrow exception for camera_CalcAngles' slot: D_800A33C8 and D_800A33CA stay two s16 names, defined as one 4-byte static block written in C as `static s16 D_800A33C8; static s16 D_800A33CA;` with maspsx's A8 alignment applied per object size for statics under 4 bytes - i.e. A8 for objects of 4+ bytes only (the measured Sony rule says otherwise, so this is a model exception, recorded). (B) Per-function cc1 small-data option: not available (cc1 flags are per file). (C) Hold the adoption until a C spelling or a file split puts camera_CalcAngles in a -G8 file (it sits in text1b's merged M3 group, which came from sound.c; a split would need split evidence). (D) Land Q65 with camera_CalcAngles back to INCLUDE_ASM (it is a completed function; asm-until-matched would re-queue it as an open item with this residual).
recommendation: (D) if the owner wants the adoption now - the function goes back to the queue with a precise one-instruction residual and the model stays exact; otherwise (A).

## 2026-10-01 — func_80058580 (d) — the best score's s16 compare inside a shared work variable — policy-question
category: policy-question
evidence: memory/grind/func_80058580/evidence.md [s10]; layer-2 rev-58580-dm-b FAIL (body 9dfd17f47e702d1f), rev-58580-r11-b PASS. Everything else in the function is now cast-free and links byte-identical (candidate.c). The random pick loop keeps its best score in work2 (Q75 value: `li $s2,-1`, `addu $s2,$s3,$zero`). The original compares that value as a 16-bit signed number: `sll v0,s2,16; sra v0,v0,16; slt v0,v0,s3` at 0x8005A338 (score in $s3 is compared full width). In C that is `(s16)work2 < score`. The reviewer refuses it as a redundant width cast (values -1..0x27E). Measured, all off: no cast 8 words; best as its own `s16` local 23 (it then gets $s4: 3 references cannot outrank score/pick, it reaches $s2 only as part of work2); also with a do-while loop 23; `s16 score` 12; both s16 26; work2 as s16 222; reusing the function's other s16 locals pbest / sc / et 34 / 27 / 133. The only other exact spelling is the shift pair `(work2 << 16 >> 16) < score`, the same redundancy in arithmetic form.
Question for the owner (plain language): "In the AI routine, the shared scratch variable that holds the 'best score so far' (you allowed that in Q75) is compared as a 16-bit number: the shipped code sign-extends it from 16 bits right before the compare. Writing that needs a `(s16)` cast on the shared variable at that one compare, which the rules refuse as a redundant width cast (the value always fits). Every other spelling is measured and misses. Allow this one cast at this one compare, or keep the function open?"
options: (A, recommended) Allow narrowly for func_80058580: `(s16)work2 < score` at 0x8005A338 only, with a comment citing the sll/sra pair, the Q75 value and the measurements. (B) Allow the shift-pair form instead. (C) Refuse: func_80058580 stays INCLUDE_ASM/active at 8 words.
recommendation: (A). The narrowing is in the shipped bytes at that compare (the s16 view of the value, matching the function's other s16 scores pbest/sc); it is one disclosed cast, not a hidden pin or coercion of unrelated code.

## 2026-10-01 — func_80058580 (d), q65-adoption step 14 (A8) camera_CalcAngles — owner rulings Q82-Q84 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 38; docs/grind/decisions.md 2026-10-01 OWNER RULING Q82-Q84.
disposition taken: func_80058580 (d) SPENT, option (A) allowed narrowly (Q82). camera_CalcAngles: Q83 (text1b -G8 if proven) failed its proof (EXE 16 bytes short); Q84 chose option (D): land Q65 with camera_CalcAngles reverted to INCLUDE_ASM and re-queued, A8 unchanged. Each landing still needs its own layer-2.

## 2026-10-01 — func_8002AB08 — reused locals whose original code re-stores a value they already hold — policy-question
category: policy-question
evidence: memory/grind/func_8002AB08/r11/README.md (value table, "Round 2"), r11/round2/rs_v*.c, writeset_8c.txt; layer-2 round 1: rev-2AB08-dm PASS, rev-2AB08-r11 FAIL. The body links byte-identical (sandbox 0, full-build SHA1 == oracle). Each pass of the blade loop picks its two points: pass 0 sets `alt = 0; temp1 = 0; temp2 = 1; c = 1;`, a pass with the second blade (other->unk_8C != 0) sets `alt = 1; temp1 = 0; temp2 = 1; c = 1;`, a 4/5-phase pass sets `alt = 0; temp1 = 1; temp2 = 2; c = 0;`. Nothing in the loop changes unk_8C (its callees never write it), so every later pass takes the same arm: the second arm's `temp1 = 0; temp2 = 1; c = 1;` and the third arm's `alt = 0;` always store the value the variable already holds. The original does exactly that: the second arm runs the stores it shares with pass 0 (0x8002AEE4 -> 0x8002AEF0..0x8002AEFC) and the third arm has `move $fp,$zero` (0x8002AF18). Dropping them costs 1 to 13 instructions (rs_v1..v4). Ruling 11 (B)(2) (via Ruling 5 2(c)) refuses a write that re-stores a held value on every feasible path; temp1, temp2 and alt are Ruling 11 locals (each also holds a later value; splitting them costs 22 / 16 / 118).
disposition taken: not landed; func_8002AB08 stays INCLUDE_ASM/active; candidate.c is the byte-exact body (everything else layer-2 PASSed or answered in r11/README.md round 2). Tree reverted to the oracle.
Question for the owner (plain language): "In this function each pass of a loop sets up its two points in one of three branches. The original writes the same 'point 0 / point 1' values again in one branch and 'alt = 0' again in another, values the variables always already hold there (the compiler kept those stores; the bytes show them). The reused-variable rule refuses a write that only re-stores a value. Allow these re-stores for func_8002AB08 because the shipped code contains them (addresses cited), or keep the function open?"
options: (A, recommended) Allow narrowly for func_8002AB08: the re-store writes (`temp1 = 0; temp2 = 1; c = 1;` in the unk_8C arm, `alt = 0;` in the 4/5 arm; `c` is the arm's flag for func_8002CA8C, one role), each with its target address in the variable's comment, under the rest of the Ruling 11 package and a fresh layer-2. (B) Do not allow: the function stays INCLUDE_ASM/active; no spelling without the stores matches.
recommendation: (A). Each arm sets the full pass state, the stores are in the target, and nothing is hidden.

## 2026-10-01 — func_8002AB08 — owner ruling Q85 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 39; docs/grind/decisions.md 2026-10-01 OWNER RULING Q85.
disposition taken: the 2026-10-01 func_8002AB08 policy-question (per-arm re-stores) is SPENT: option (A) allowed narrowly. The landing still needs its own fresh layer-2.

## 2026-10-01 — q65-adoption step 15 — eight small read-only data items in files that become -G8 — policy-question (record)
category: policy-question
evidence: layer-2 round 2, step-15 reviewer (rev-q65r2-s15) fix 8; definitions: src/code6cac_b_rodata_pre.c:3 `_bb2_101C_pre_lead` (4 B), src/code6cac_b_tu2.c `D_800107BC[1]`, src/code6cac_tu2.c `D_800100E0[1]` and `D_80010428[1]` (zero words), src/text1a_b_post_rodata.c `D_80015F4C[4]`, src/text1a_b_tail_rodata.c `D_80016240[8]`, `D_80016254[8]`, `D_800162CC[8]` (format strings read by Sony library code: display.c GPU_printf, system.c set_alarm / printf). cc1psx -G8 places a small const (8 bytes or less) and a short literal in .sdata (layer-2 round 1 calibration by rev-q65-sub; literal case in gp-model doc A.3b); our cc1 keeps it in .rodata, and maspsx's A3 move covers `.data` only. All eight are C definitions (`const`), which our cc1 emits in `.rdata`; maspsx -G8 moves only `.data` objects (A3) and never treats a `.rdata` symbol as small data, so no access to them becomes gp-relative and they stay at their .rodata addresses. The original EXE has all eight inside .rodata (0x800100E0 .. 0x800162CC): the build is byte-identical (the series reaches the oracle at step 15). The files get maspsx -G8 under Q69 because the A4 library test reads only `.text`: the three rodata-only files hold no code, and the zero words are rodata-alignment pads transcribed by the rodata-align project.
disposition taken: nothing modelled or moved (A3 stays `.data` only); logged here and in step 15's body.
Question for the owner (plain language): "Eight tiny read-only items sit in files that the adoption compiles with the small-data option. The original compiler would have put such tiny constants in small data, but in the shipped game they are in normal read-only data, so either their files were not compiled with that option (the three files of strings belong to Sony library code, which was not) or they are pieces of bigger objects. Our build already puts them where the game has them, so nothing changes in the bytes. Is recording this enough, or should the library test also classify read-only-only files by who uses their data?"
options: (A, recommended) Record only: byte-neutral, the objects stay where the shipped bytes have them; revisit when the rodata-only files are folded into their owners. (B) Extend the A4 library test to rodata-only files read by library code (these three would get -G0); a rule change with its own proof.
recommendation: (A).

## 2026-10-01 — q65-adoption step 15 — D_800A3224 / D_800A3290: parts of bigger objects defined as their own — policy-question (record)
category: policy-question
evidence: layer-2 round 2, step-15 reviewer fix 4; generator log lines DATA-MODEL (55fee8b52:memory/grind/q65-adoption/q56/adopt/s15_apply.py DATA_MODEL_NOTES). D_800A3220..3227 is an 8-byte RECT: code6cac_c2's func_8003D2C4 passes `&D_800A3220` to LoadImage, whose callee reads x/y/w/h (bytes 0x3F0, 0x1DC, 0x10, 0x24), but include/code6cac.h declares D_800A3220 `u32`, so step 15 defines the RECT's w/h half as its own object `s32 D_800A3224 = 0x240010;`. D_800A328C..3293 is an 8-byte record: text1b stores `&D_800A328C` as a descriptor's p_static for func_8007352C and its neighbours D_800A327C / 3284 / 3294 are 8-byte records, but text1b declares D_800A328C `s32`, so step 15 defines its second word as `s32 D_800A3290 = 0xe140000;`. Both are byte-identical; each carries a comment stating this evidence.
disposition taken: defined as declared (the rule's types follow existing declarations), each tail its own object, logged here; no retype in the adoption.
Question for the owner (plain language): "Two small pieces of data are really the second halves of 8-byte records (a screen rectangle and a drawing descriptor) whose first halves the code declares as plain 4-byte numbers. The adoption keeps the existing declarations and defines each second half separately, with a comment. Fixing the types properly means changing those declarations and the code that uses them. Leave this for a later data-model cleanup?"
options: (A, recommended) Record only: land the adoption as is; retype D_800A3220 as a RECT and D_800A328C as its 8-byte record in a later aggregate-merge cleanup with its own review. (B) Retype both inside the adoption (a header change and an extra reconciliation step before step 15).
recommendation: (A).

## 2026-10-01 — q65-adoption step 08 — func_80044100 left unprototyped in sound and text1a_b — policy-question
category: policy-question
evidence: 55fee8b52:memory/grind/q65-adoption/q56/adopt/s08_apply.py (docstring, func_80044100). func_80044100 is defined in text1a_c.c `void (s32 a0, s32 a1)`. text1a_b calls it as `func_80044100(8)` and the target bytes leave $a1 unset at that call; sound calls it with two arguments. Today text1a_b declares it `void (s32, ...)` and sound `void (s32, s32)`; the Q67 merge puts both in one file, so they need one declaration. A prototype with two parameters would force the one-argument call to pass a second value (different bytes); the varargs form is a spelling chosen to make the call compile. Step 08 declares it unprototyped, `extern void func_80044100();`, in both, which compiles both calls as written, byte-identically. Per-file-gp-model A5 lists only three owner-decided declarations (D_80102C00, D_800153F0, func_8004153C); func_8004153C got the same unprototyped treatment by owner ruling.
disposition taken: unprototyped in step 08 (the round-2 step-08 reviewer flagged it as a possible owner question).
Question for the owner (plain language): "One function is called once with two arguments and once with only one (the shipped code really passes just one there). To keep both calls exactly as the game has them in the merged file, its declaration has no argument list, the old-C way, like func_8004153C, which you already approved. Allow the same for this function?"
options: (A, recommended) Allow: `extern void func_80044100();` in the merged file, recorded next to A5's func_8004153C. (B) Refuse: keep the varargs declaration `void (s32, ...)` (also byte-identical, but a varargs prototype asserts a variable argument list the definition does not have). (C) Hold step 08 until a different evidence-based declaration is found.
recommendation: (A).

## 2026-10-01 — q65-adoption step 15 small rodata items, D_800A3224 / D_800A3290, step 08 func_80044100 — owner rulings Q86-Q88 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 40; docs/grind/decisions.md 2026-10-01 OWNER RULING Q86-Q88.
disposition taken: all three 2026-10-01 entries SPENT with option (A): record only (Q86); later aggregate-merge cleanup for the two half-records (Q87, follow-up owed); func_80044100 unprototyped, listed in A5 (Q88). Each series step still needs its own layer-2.

## 2026-10-01 — camera_CalcAngles — no -G0 spelling exists for the A8 array; the evidence points to a -G8 file that ends before text1b's hand-written asm — policy-question
category: policy-question
evidence: memory/grind/camera_CalcAngles/evidence.md (2026-10-01 laneA section), g0proof/run_all.sh -> g0proof/g0proof.out (RTL dumps, cc1psx calibration, respellings, -G0/-G8 per-function diff of text1b). (1) Under -G0, an element store's constant address is forced into a register at expand (explow.c:398), cse1 rewrites the returned address to that register (a -G0 symbol costs more than a register), and neither cse2 nor combine can put the symbol back while the register is live to the return: no array/struct/union spelling reaches `sh v1,%gp_rel(D_800A33C8)`. The original cc1psx at -G0 emits the same extra instruction, so this is the file's real compile mode, not our compiler. (2) Under -G8 (cc1psx and ours alike) every function body is emitted after all file-scope asm; text1b has whole-body hand-written asm (math_RotMatrixZYX..func_800525D8, inline_asm_canonical.txt) between C functions, so one -G8 file cannot give the shipped order - this is where Q83's "16 bytes short" comes from. (3) The four C bodies that change under -G8 change because of tiny declarations of larger objects: func_800475A4 is identical once `extern u8 g_cam_bone_data2;` (a structure used at +0x10/+0x18) is declared unsized; func_800477E8's is `extern s8 D_800EF070;` (one field of a record). (4) No text1b static is used on both sides of the hand-written block (g0proof/span.py).
disposition taken: none; camera_CalcAngles stays INCLUDE_ASM and active (Q84). Best honest-C floor 6 (59 vs 58 insns); a temporary for the first value reaches 58 insns but still stores through a register (score 2) and is a named-intermediate device.
Question for the owner (plain language): "camera_CalcAngles can't be written in C the way the files are set up now - I proved no spelling works, and Sony's own compiler agrees when run the same way. Its code only comes out right with the small-data option (-G8). Compiling all of text1b that way failed earlier, but I found why: with that option the compiler moves hand-written assembly to the top of the file, and text1b has a block of hand-written assembly in the middle, so the original can't have been one -G8 file either. The likely truth is that the C before that assembly block (which includes camera_CalcAngles) was its own file compiled with -G8, and the assembly and later C were separate. Our split rule only accepts gp-access evidence for file boundaries, so this needs your call."
options: (A) Allow this kind of boundary evidence: split text1b immediately before its first hand-written asm function and compile the first part with cc1 -G8, landing only on a both-ways proof (every other function byte-identical, oracle SHA1). Prerequisites in that part: func_80048FFC (rotated) lands in C first (an INCLUDE_ASM would also float), and the small-declared objects get their real types (g_cam_bone_data2 measured; D_800EF070's record to measure). The exact cut is tested like any split (statics, .rodata/.sdata contiguity). (B) Model exception for this slot: D_800A33C8/D_800A33CA stay two s16 statics (A8 for 4+ byte statics only) - departs from Sony's measured alignment. (C) Leave camera_CalcAngles in assembly (active) until (A)'s prerequisites exist.
recommendation: (A) as the evidence-consistent model, staged: real types for the small-declared objects and func_80048FFC first (each worth doing anyway), then the split + -G8 proof; until then (C).

## 2026-10-01 — camera_CalcAngles — owner ruling Q89 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 41; docs/grind/decisions.md 2026-10-01 OWNER RULING Q89.
disposition taken: the 2026-10-01 camera_CalcAngles policy-question is SPENT with option (A), staged: prerequisites first (func_80048FFC in C; real types for the small-declared objects), then the split + -G8 proof; camera_CalcAngles stays INCLUDE_ASM/active until it lands.

## 2026-10-01 — func_80023F08 — u32 move-class bit-set — owner ruling Q90 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 42; docs/grind/decisions.md 2026-10-01 OWNER RULING Q90; layer-2 rev-23F08-B-r11 FAIL item 1.
disposition taken: asked directly by the orchestrator after the layer-2 FAIL (no separate policy-question entry); SPENT, allowed narrowly (Q90). The landing still needs its own fresh layer-2.

## 2026-10-02 — camera_CalcAngles / func_80048FFC — Q89's first prerequisite cannot be met in C; may the -G8 head part end before func_80048FFC? — policy-question
category: policy-question
evidence:
- memory/grind/camera_CalcAngles/evidence.md (2026-10-02 oct2-a1 section); memory/grind/func_80048FFC/evidence.md.
- func_80048FFC is 4/232 at -G0 AND at -G8. Its one hunk is the copy `old = phase` and the shift `phase >>= 1`, which sit above call two's a1/a2/a3 setup instead of below it.
- Mechanism, from sched.c and the dumps:
  - The copy, the shift and the arg moves always tie on priority, in both sched passes (anti/output deps are cost-free on MIPS; nothing before call one is a load at sched1).
  - Ties go by original insn order.
  - calls.c emits the arg moves after every argument expression.
  - The copy only survives cse/combine when the shift sits between it and the rect.h store; with the shift after call two it collapses to 231 insns.
  - So every admissible spelling puts the copy and the shift first.
- Measured and killed:
  - 2240 statement orders and a 60k-iteration permuter (earlier lanes);
  - 465 do-while(0) wrappers;
  - comma/argument placements of the shift; a RECT pointer; recomputed destination args; halvings/i++ after call two; pre-increment p in the call; an s16 copy; copy+shift right before call two; `phase = old >> 1` (tmp/camera_CalcAngles/g8 x_a..x_l: 4, 53, 93, 78, 4, 56, 4, 4, 4, 53).
- The gp-span test passes for a cut before func_80048FFC as well (no gp symbol on both sides). The head statics split cleanly: D_800A33B0..E4 are used only before func_80048FFC.
- There is no independent positive evidence of an original file boundary at func_80048FFC.

disposition taken:
- The orchestrator refused the alternate cut under its delegation (it changes an owner ruling's scope; there is no boundary evidence).
- The real-types prerequisite was done separately (cheat-cleanup landing: g_cam_bone_data2 and D_800EF070 typed as transform nodes).
- camera_CalcAngles and func_80048FFC stay INCLUDE_ASM and active, not rotated.

Question for the owner (plain language): "camera_CalcAngles is waiting on func_80048FFC, as your Q89 requires, because an assembly function inside a -G8 file gets moved to the top of the file. func_80048FFC is one instruction pair from matching, and I can show why our compiler can never put that pair in the original order from any C we're allowed to write. The original programmers' code clearly did something we can't see; it is not a flag. Two ways forward: let the -G8 part end just before func_80048FFC for now, a boundary with no evidence of its own that is undone once func_80048FFC lands; or leave both functions in assembly."

options:
- (A) Stage Q89 with the cut before func_80048FFC:
  - head part func_800460E4..func_80048F58 at -G8 with camera_CalcAngles in C;
  - func_80048FFC..func_8004A09C as their own -G0 TU with today's flags;
  - the TUs merge back under Q89 as written once func_80048FFC is in C;
  - same split tests and both-ways proof.
- (B) Keep camera_CalcAngles and func_80048FFC in assembly (active) until someone finds a func_80048FFC spelling.

recommendation: (B) unless you are comfortable with a boundary placed only by our toolchain's limits; (A) is byte-neutral and reversible, but it is scaffolding, not evidence.

update 2026-10-02 (oct2-a2): the entry above is moot. func_80048FFC landed in C (e43b1f03b), with every halving
at the loop end and an s16 strip-two height. Q89's first prerequisite is met; the next entry is the new blocker.

## 2026-10-02 — camera_CalcAngles — Q89's single cut cannot reach the oracle at -G8; may the pre_rodata block be its own rodata-only TU? — policy-question
category: policy-question

Evidence: memory/grind/camera_CalcAngles/evidence.md (2026-10-02 oct2-a2 sections) and q89split/ (private full links,
tree untouched).
- The cut before INCLUDE_ASM math_RotMatrixZYX at -G0 gives the oracle. No gp symbol is reached from both
  parts, and each part defines every gp symbol it reaches. Head-only K3/K1 objects and tail-only statics
  move to their part.
- With the head part at cc1 -G8 plus camera_CalcAngles in C, the build MISMATCHES: the head's .rodata grows
  by 4 bytes.
- Cause: under -G8, cc1 emits every data object as it parses and every function body at the end of the file.
  So the merged text1a_b_pre_rodata block (D_800153F0..D_80015840, src/text1b.c "merged from
  text1a_b_pre_rodata.c") is emitted before the head's three compiler jump tables. The original has the
  tables first.
- Function-scope forms give the same result: D_800153F0 as a `static const` local of its only C user
  func_80049F4C, or as an initialized local record. Both are emitted at parse time, ahead of the tables
  (q89split/mkfs.py). The other block items are used by tail code, mostly hand-written asm, so they
  cannot be function-local.
- The block at the top of the tail fails even at -G0: func_80058580's table at 0x8001585C needs phase 4,
  and the tail object would start at 0x800153F0 (phase 0).
- Passing placements (oracle at -G8 head + camera C; q89split/mks23.py):
  - S1: the whole block as its own rodata-only -G0 object between the parts;
  - the tail starting at jtbl_8001541C, with only D_800153F0 in its own object;
  - the tail starting at jtbl_8001545C, with D_800153F0 and jtbl_8001541C in their own object.

  Tail starts at D_80015470 or D_80015840 fail. Every passing placement needs a rodata-only object beyond
  Q89's "only this cut".
- Tension:
  - Q67/A7 merged this rodata-only file into the group.
  - The orchestrator notes the bytes do not locate the boundary (three placements pass).
  - The orchestrator refused it under its delegation as item-2 build-structure territory.

disposition taken: camera_CalcAngles stays INCLUDE_ASM and active (not rotated). Nothing landed beyond the
ledger.

Question for the owner (plain language): "Your Q89 lets text1b split once so camera_CalcAngles can be compiled
the -G8 way. That split is clean, but the -G8 part then puts one block of read-only tables in front of three
jump tables. The original has them after. No C spelling inside that part changes this: the compiler always
writes tables like that first. The block reaches the original bytes only if it is its own small data-only
file between the two parts, as it was before the Q67 merge. Three slightly different cut points inside it
all work. May it be its own file?"

options:
- (A) Grant S1: restore the text1a_b_pre_rodata block verbatim as its own -G0 rodata-only TU between
  text1b (head, -G8) and text1b_tu1b (tail).
  - Record the two other passing placements.
  - Land the split, then the -G8 opt-in with the camera_CalcAngles Match, each oracle-green and with its
    own layer-2.
- (B) Keep camera_CalcAngles in assembly; text1b stays one -G0 file.

recommendation: (A). It is a move of existing text with no new code, and it is reversible.

## 2026-10-02 — _exeque — does closer Ruling 4 still admit a NEW volatile row (field-level GpuCtx.unk08) after Q91? — policy-question
category: policy-question

Evidence: memory/grind/_exeque/evidence.md [s13], [s13b]; rejected/volatile-unk08.c (body_hash e2745fa88ffbd7c1);
layer-2 rv2-exeque-1 FAIL (memory/grind/_exeque/layer2.jsonl, item 3).
- Score is 0/187 (oracle green) with `volatile s32 unk08` on GpuCtx +0x08 (the old D_8009BE7C). It is 2/187
  with any non-volatile spelling.
- Cause, named from a reorg dump: fill_simple_delay_slots moves `g_gpu_ctx.unk08 = 0` into the drawsync_cb
  `jalr` delay slot. The target keeps `sw $zero,0($v1); jalr $v0; nop` (0x8007D988).
- This is formally the only route. A non-volatile store never conflicts with the call's resources. Only
  volatile, a label/jump, or asm keeps it out of the slot (evidence.md [s13b], with reorg.c and jump.c line
  cites).
- unk08's only consumers are _addque2 (sets 1) and _exeque (test-and-clear; _exeque is also the DMA-2 IRQ
  callback). Both are verbatim PsyQ 4.0 LIBGPU/SYS.
- The reviewer's ground: the use site is none of legitimate-volatile-interrupt-touched's three shapes.
  Closer Ruling 4 (c80d976e, 2026-07-10, ground-truth-codegen volatile for census-proven Sony module state,
  no IRQ prong; text at cd19d7a2^:docs/closer/rulings.md:68-83) is in no current catalog file. Its existing
  rows (_que, _qin, _qlog in this module) are grandfathered, not a route for new rows.
- Reviewer's question, verbatim: "Does closer Ruling 4 (2026-07-10; ground-truth-codegen volatile for
  census-proven Sony library module state, no IRQ prong) survive owner ruling Q91 as a route of the IRQ-touched
  extern allowlist for NEW rows, including a field-level qualifier on a struct member (GpuCtx.unk08), given that
  legitimate-volatile-interrupt-touched.md now states the two prongs as BOTH required and does not mention
  Ruling 4?"
- The orchestrator declined to grant it under its delegation (Q91 item 3 refuses volatile outside the catalog
  even when annotated).

disposition taken: _exeque stays INCLUDE_ASM and is not unparked. The passing body and measurements are banked
in its ledger. A grant needs a rules: commit that writes the route into a catalog file, then a fresh layer-2.
After that, the banked body (1 FAKE do-while(0)) is the landing candidate.

Question for the owner (plain language): "Sony's library code for _exeque reaches the original bytes only if one
flag in the graphics library's state is declared volatile. The compiler then cannot tuck the flag's clear
into the next call's delay slot. No other C spelling can produce those bytes, and that is proven from the
compiler source. The flag is shared between the DMA interrupt handler and the main code. The volatile rule's
three listed use shapes do not include 'test the flag, clear it, call the callback'. In July you allowed
volatile on Sony library state when the code is unreachable without it (closer Ruling 4); three rows of this
same module use that. May that route still add a new row, here for one struct field?"

options:
- (A) Write closer Ruling 4 into legitimate-volatile-interrupt-touched.md as a third route: census-proven Sony
  library module state, codegen measured unreachable without volatile, per-symbol or per-field allowlist row.
  Land _exeque with the field-level row.
- (B) Add "test-and-clear of an IRQ-shared flag" as a fourth use-site shape. The handler-side clear in
  _exeque, running from DMA-2 IRQ context, versus _addque2's set qualifies.
- (C) Refuse. _exeque stays INCLUDE_ASM permanently at 2/187.

recommendation: (A). It restores a July owner ruling that the module's existing rows already rely on. It is
narrow (Sony census state only), and the evidence is a named compiler decision, not a score chase.

addendum 2026-10-02 (fresh lane oct2-b9, memory/grind/_exeque/evidence.md [s14]): a fresh re-derivation
reached the same result. Every reorg.c / jump.c route was re-checked from source, and 13 new tail
spellings were measured; none goes below 2 without volatile. New evidence: the original PsyQ cc1psx, given
the same non-volatile source, also fills the slot. So the original source differed at this site; this is not
a compiler-fidelity gap. Also noted, not used: SOTN sys.c @aa53500 (matched) declares this module's queue
state volatile (sys.c:58, 96-97), and its _exeque tests and later clears a member of it (sys.c:817, 821).
That is a possible Q55 citation, but it concerns a different object of the same module.
Q55 checked (evidence.md [s14b]): the citation fails condition (2). SOTN's volatile object is the
packet queue, which is BB2's `_que` and is already volatile. SOTN declares the counterpart of GpuCtx
non-volatile (sys.c:82-86, including the drawsync callback) and has no pending flag.

## 2026-10-02 — func_80020E74 — may D_800A38C6 be reached by indexing past D_800A38C4 (a third Q63-style pair)? — policy-question
category: policy-question

Evidence: memory/grind/func_80020E74/evidence.md (measurements, mechanism), candidate.c (byte-exact body),
dm/ (data-model script, private full-link harness), d38/ (failing single-object spellings).
- With an honest data model (D_800A38C0[2], D_8008DB1C[27][8], menuDat[18], typed Tbl800A3860Entry /
  PracticeMenuRec.unk_48), candidate.c relinks to the oracle SHA1 (private link with build/ objects).
- Its one blocked construct: `(&D_800A38C4)[i] = loads[i]; /* FAKE */` in the per-slot loop. The target
  stores to D_800A38C4 + 2*i (`addu $at,$at,$s4; sh %lo(D_800A38C4)($at)`), so the site is a real loop index.
- Every other function reads and writes 0x800A38C6 by its own name. Declaring the pair as `u16 D_800A38C4[2]`
  or as a two-member struct, with every consumer converted, fails twice: func_80020CDC and func_80020D38
  each gain 3 instructions (scratch SHA1 a067d75a..., EXE +24 bytes). func_80020E74 itself is exact under the
  array.
- Cause, from RTL dumps: `D_800A38C4[1]` is a constant offset from a symbol. It is forced into a pseudo at
  expand, and CSE reuses that pseudo for the store after the seq_Reset call, so it lives in $s0. The first
  CSE pass stops at a loop-end note, but the rerun after loop.c ignores loop notes. 17 spellings of
  func_80020D38 (do-while(0) at every position, goto, switch, value local, pointer after the call, store in
  both arms) all score 9. The original compiler gives the same 9 (engine cc1psx-check), so the original
  source accessed 0x800A38C6 by name there.
- func_80021280 (COMPLETED) already walks `(u16 *)&D_800A38C4` past into D_800A38C6. SOTN @aa53500 has no
  `(&sym)[i]` indexing to cite (Q55).
- The orchestrator declined under its delegation: item 3 refuses cross-symbol derivation, and
  aggregate-merge-family.md keeps the Q63/Q73 exceptions to their own symbols.

disposition taken: func_80020E74 stays INCLUDE_ASM and is not unparked. The byte-exact body (3 FAKEs: frame
layout `loads[130]`, the shared `j` local, this index) and every measurement are banked in its ledger.

Question for the owner (plain language): "func_80020E74 records which character model is loaded in each of two
slots. It writes slot i with an index from the first slot's variable. Every other function uses the second
slot's variable by its own name. Making the two one array is the SOTN fix, but then two small functions that
clear slot 2 around a call get 3 extra instructions each, in our compiler and in Sony's original one. So the
original code most likely had two variables and indexed past the first, as with the D_800A37D2/D3 pair you
allowed in Q63. May this pair get the same narrow admission?"

options:
- (A) Admit it for D_800A38C4 / D_800A38C6 in func_80020E74 only, Q63-style: two scalars, one indexed store
  FAKE-annotated with the evidence, rule text added to aggregate-merge-family.md.
- (B) Refuse. func_80020E74 stays INCLUDE_ASM until a byte-exact array spelling of func_80020CDC /
  func_80020D38 is found.

recommendation: (A). It is the same compiler evidence Q63 rested on, measured on both compilers, and one site.

resolution (2026-10-02, lane oct2-a6): RESOLVED without the cross-symbol admission, so the question is moot.
Option (B)'s frontier closed: func_80020CDC / func_80020D38 are byte-exact under `u16 D_800A38C4[2]`. Each
reaches the pair through one FAKE-labelled pointer local, which keeps CSE from sharing the constant address
across seq_Reset. Commits: cheat-cleanup 860dd811e (rv2-20E74-A3 PASS) and Match 40c63a962 (rv2-20E74-B PASS);
func_80020E74 is COMPLETED-C (queue 1f4459b0e).

## 2026-10-02 — camera_CalcAngles — owner ruling Q94 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 44; docs/grind/decisions.md 2026-10-02 OWNER RULING Q94.
disposition taken: the 2026-10-02 camera_CalcAngles S1 policy-question is SPENT with option (A): the pre_rodata block
is its own rodata-only -G0 file between the Q89 parts.

## 2026-10-02 — _exeque — owner ruling Q95 — resolution
category: resolution
evidence: docs/grind/owner-rulings-2026-09-26.md batch 44; docs/grind/decisions.md 2026-10-02 OWNER RULING Q95.
disposition taken: the 2026-10-02 _exeque policy-question is SPENT: the field-level volatile on GpuCtx.unk08 is
allowed (this field only). The landing still needs its own fresh layer-2.

## 2026-10-06 — _SpuSetAnyVoice — volatile on the RAM shadow D_800F7298 — policy-question
category: policy-question
evidence: src/main/psxsdk/libspu/ (\_SpuSetAnyVoice, `extern SpuUnion D_800F7298;`, landed 3a5e03773): a KSEG0 RAM shadow
of the SPU register block read and written through PsyQ's `union SpuUnion`, whose `raw` member is volatile (the type is
right for _spu_RXX at 0x1F801C00, MMIO). On RAM that volatile is outside the catalog (mmio-volatile-type-level,
legitimate-volatile-interrupt-touched Routes A/B); it is load-bearing (a non-volatile view scores 45: the `& 0xFF`
read narrows to lbu). Found by rev-w2b1 (Phase 2 worker-2 batch 1, F06).
disposition taken: the body is left textually unmoved (its redundant `(SpuUnion *)` cast kept on the unedited line), so
no review re-certifies it; recorded as debt. Question for the owner: may D_800F7298 keep PsyQ's SpuUnion type (volatile
raw) as an interim per-function label, or must it get a non-volatile type (and the function be re-opened)?

## 2026-10-06 — Phase 2 asm-operand bodies — may typing edit a canonical GTE island's operand expressions? — policy-question
category: policy-question
evidence: memory/grind/phase2-2026-10-03/lt/plan.txt (owner-blocked (1)); lt/f02/plan.txt "RULING NEEDED". 17AFC
func_8002CD58 / func_8002DAD0 / func_8002D780 (146 sites), 87A0 (35), 51268 func_80067D14 (3) and 9F9C func_800203B4
(4, scratch tmp/p2/i203/) pass raw offsets (`obj + 0xA8`, `(s32 *)(obj + 0xF8)`) as "r" operands of canonical GTE
islands. Typing `obj` changes that operand text (hashed in tools/canonical_asm_regions.json); the alternatives are a
labelled u8 * byte view kept only for the asm (an alias local), or locals computed outside the asm (also edits the text).
disposition taken: bodies left as at HEAD; debt rows in the phase-2 commits. Question for the owner: may an operand
EXPRESSION (not the asm template) be retyped to the member form (`&rec->unkA8`) with an `auth:` re-hash of the region?

## 2026-10-06 — func_80031B24 — extend Q96 to its &D_800A37E8 vector passes? — policy-question
category: policy-question
evidence: 17AFC func_80031B24 hands &D_800A37E8 (the D_800A37E8 / EA / EC s16 vector) to func_800274BC / func_80032854,
which read [0..2]: cross-symbol address derivation, refused by completion-bar item 3 except under Q96, which names only
func_80027AD8 / func_8002AB08. One-object `s16 D_800A37E8[3]` scores 0 in func_80031B24 / 8002A458 / 8002AB08 but 2 in
func_80027AD8 (Q96's basis: memory/grind/judge-decl-cleanup/followups/func_80027AD8.vec-investigation.md).
disposition taken: unchanged; debt row (lt/plan.txt owner-blocked (3)). Question for the owner: extend Q96 to
func_80031B24's two call sites (FAKE-labelled in Q96's form), or leave the debt row?
