# Hypothesis ledger — func_800204C0

## s1 (2026-09-08, recon)
- H1 CONFIRMED: the func_800203B4-granted island spelling (move $12,%0 macro bodies) reproduces
  all 25 island insns byte-exact here (same three $t4 preambles, same 0x4A486012 command).
- H2 CONFIRMED: head `*p += 1; if ((*p & 7) == 2)` (memory read-modify-write + re-read) yields the
  `move $v1,$v0` delay-slot copy AND the 8-byte phantom frame slot (vars=32). Sandbox 0.
- H3 KILLED (instance): head `cnt = *p + 1; *p = cnt; if ((cnt & 7) == 2)` with an s32 cnt local —
  sandbox 10 (vars=24, no copy). rejected/head-local-cnt-frame24.c.
- Open: none on codegen — residual is 0. The remaining step is the operator registry row
  (tools/grinder/owner_cluster_grants.txt) per the func_80019310 precedent; not a session surface.

## s1b (2026-09-08, recon — post-refusal re-dispatch, HEAD 387fa8f8)
- H4 CONFIRMED: the Judge-PASSed body (hash 8655cc28f3aa5cc7) still measures sandbox 0 (122/122)
  on HEAD 387fa8f8 — chassis unchanged since the refusal; no re-spelling needed.
- H5 KILLED (instance): the refusal's "islands are C-expressible" branch — gte_SetRotMatrix
  respelled as C word loads + ctc2-only islands scores 0/12 in the island region (loads seated
  $t0/$a0/$v0/$a0/$v1 block-ahead, no $t4 copy). Measured on HEAD 387fa8f8, no FAKE constructs,
  first island only (the other three islands kept in the granted spelling).
  rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c.
- H6 CONFIRMED: STRONG scan tier is not available (LOW 1/8, S4 only) — the only grant door is the
  operator registry row; filed as an integration handoff (decisions.md 2026-09-08 entry).
- Open on codegen: none. Next session after the registry row lands: submit candidate.c EXACTLY
  (Judge clearance 8655cc28f3aa5cc7 skips layer-1), candidate-ready.

## [s1] The Judge-PASSed body (hash 8655cc28f3aa5cc7, candidate.c) still measures sandbox 0 on the current chassis HEAD 387fa8f8.
- mechanism: Chassis unchanged since the 2026-09-08 refusal; the refusal was a grant-door failure, not a bytes failure.
- probe: cp candidate.c src/code6cac.c; sandbox func_800204C0 --disable all; canonical func_800204C0
- result: score 0, 122/122, rules_dropped 0, cheat_asm_stripped 20 (tmp/grind/func_800204C0/s1/sandbox_s1b.json); canonical ASM-PARTIAL 11/122 cop2.
- verdict: CONFIRMED

## [s1] Respelling the gte_SetRotMatrix island as five C word loads plus ctc2-only islands (first island only, other three in the granted spelling, no FAKE constructs) reproduces the 12-insn island region of the target on HEAD 387fa8f8.
- mechanism: The target region is the SDK macro text: redundant move $t4,$v1 preamble then lw $t5/$t6 + ctc2 interleaved; GCC 2.7.2 schedules the C loads block-ahead and never emits a redundant register copy.
- probe: tmp/grind/func_800204C0/s1/mk_thin1.py -> thin1.c; try.py cc1 gradient vs asm/funcs/func_800204C0.s
- result: 0 of 12 region insns reproduced: loads seated $t0/$a0/$v0/$a0/$v1 as a block ahead of each ctc2 pair, no $t4 copy, 11 insns vs 12 (tmp/grind/func_800204C0/s1/thin1_try.txt). Same class as func_80019310 s3 H10 and func_800203B4 s6. rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 387fa8f8 chassis, candidate.c body with island 1 respelled in C, zero FAKE constructs, cc1 direct gradient

## [s1] The STRONG scan-tier door of grant_canonical_asm is available for func_800204C0.
- mechanism: tools/scan_hand_coded.py signals S1/S2/S6 are required for STRONG; GTE wrapper bodies score LOW (known misroute artifact named in the 2026-09-01 grant record).
- probe: python3 tools/scan_hand_coded.py --single func_800204C0
- result: tier LOW 1/8, only S4 front loads (tmp/grind/func_800204C0/s1/scan_hand_coded.txt). Only the owner_cluster_grants.txt row door remains, and that file is operator-maintained.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 387fa8f8, asm/funcs/func_800204C0.s target bytes (scanner reads the target, independent of C)

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated ("FORECLOSED" legacy wording); INCLUDE_ASM at `src/code6cac_tu2.c:3010`. "ASM-PARTIAL" is only the canonical gate's label for "contains cop2" (11/122 insns); nothing in src is inline asm today. `candidate.c` is a 113 KB copy of the whole pre-split src/code6cac.c - the body is its lines 2110-2200. That body scored 0 (122/122) on 2026-09-08 and the Judge PASSED it (hash 8655cc28f3aa5cc7); the merge was refused only because the Grinder had no grant route (scan tier LOW, no owner_cluster_grants.txt row). Ledger paths citing src/code6cac.c are stale.
- CONSTRAINTS: the 2026-09-01 owner grant names func_800204C0 as a confirmed carrier (`pre-slim-2026-10-01:docs/grind/decisions.md` ~line 17959) - its islands are covered once the C reaches 0. Since the Q38 audit, registry rows need a recorded owner instruction (`tools/grinder/owner_cluster_grants.txt` comments; func_8003E6D8 at line 93 is the exact precedent). The 2026-09-26 inline_o.h class route forbids any edit to macro text incl. the DMPSX command word (`.claude/rules/inline-asm-policy.md:93-104`); swapping the word needs a per-function owner grant like Q61 (:118). The 2026-09-25 scorer keeps only GPR statements of header-exact macro units (:76-90).
- BLOCKER: admission, not bytes. [I] the candidate's joined-statement islands (copied from func_800203B4) aren't header-exact units, so today's sandbox likely strips them and scores >0. [I] it reads the record via raw `u8 *arg0` offsets; arg0 is a PracticeMenuRec* (func_80023F08 passes `&D_80101EC8[idx]`, asm 80026D60); fields +4, +0x350, +0x352, +0x354 - raw-offset reads are banned by the func_80021424 HANDOFF cleanup bar.
- PLAN:
  1. Paste only candidate.c lines 2125-2200 over the INCLUDE_ASM; cast func_80032854's 3rd arg to `(s32 *)` per the prototype at tu2:2751; `sandbox --disable all --diff` to see what the scorer does with these islands.
  2. Retype to `PracticeMenuRec *rec`; in `include/code6cac.h:60` replace `unk_352[]` with `s16 unk_352; s16 unk_354[3]` (or SVec4i16). Keep the `+= 1`-then-reread counter head (load-bearing, evidence.md item 6).
  3. Rewrite islands as inline_o.h units from `engine/gtemacro.py` PINNED (gte_SetRotMatrix, gte_ldlv0, gte_rtv0, gte_stlvnl) in the func_8002F770 form (`src/code6cac_b_tu2.c:3297-3325`), command word 0x4A486012 for gte_rtv0. Risk: "memory" clobbers may move the `lh 0x350` / magic-constant sequence.
  4. OWNER RULING: (a) per-function DMPSX-word grant like Q61 if step 3 hits 0, or (b) a registry row under the 2026-09-01 grant keeping the func_800203B4 spelling. Batch with func_800207C8.
  5. `auth:` commit, fresh layer-2, `verify-oracle --rebuild`, `queue done`.
- DEPENDS: PracticeMenuRec header shared with the L1/L2 cleanup series (`memory/grind/func_80021424/HANDOFF.md`).
- ODDS/LANE: 1 manual session + 1 owner question; bytes ~85% [I].
