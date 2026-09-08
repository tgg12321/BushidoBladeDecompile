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
