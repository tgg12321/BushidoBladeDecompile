# Hypothesis ledger — func_80019310

## s1 (recon, 2026-09-06)

- H1 CONFIRMED — Pure-C body + four PsyQ GTE macro islands in the func_800203B4 `move $12,%0` spelling reproduces the target. Probe: v1 (islands without "memory" clobber, `s32 out[6]`) → sandbox 4 stripped / build 79 insns; v4 (`s32 out[3]` + "memory" on island 1) → 0; v5 (loop condition re-read inline, no `count` local) → 0; v6/final ("memory" on all four islands) → 0, 81/81, kept and stripped. Candidate = memory/grind/func_80019310/candidate.c, in place at src/code6cac.c:339.

- H2 CONFIRMED — The 0x8001934C reload of the MATRIX pointer between islands 1 and 2 comes from a memory clobber on island 1. Mechanism: cse.c does not invalidate memory across an `asm volatile` that lacks a mem clobber; `"memory"` → `(clobber (mem:BLK (scratch)))` (stmt.c expand_asm_operands) → cse_insn invalidate_memory. Probe: v1 vs v4 differ only by the clobber (plus the out[] size) and the reload appears exactly there. Both loads take $t1 with no coercion.

- H3 KILLED (instance) — `VECTOR out;` struct local as the gte_stlvnl target via `"r"(&out)`. Measured 38 (v3, no clobber) / 37 (v2, with island-1 clobber) on the s1 chassis (islands present, zero FAKE constructs): GCC dropped the stlvnl island, its swc2 stores and the three `out.v? << 7` stores; frame 8. `s32 out[3]` is the closing form; mechanism not dumped (nothing depends on it). measured_on: s1 chassis, v2/v3 bodies, no FAKE.

- H4 KILLED (instance) — `s32 out[6]` local. Frame 32 vs target 24 (v1). Frame = locals(8-aligned) + 8 on this leaf; 12-16 bytes of locals is the target. measured_on: s1 chassis, v1 body, no FAKE.

- H5 CONFIRMED — Selective vs uniform "memory" clobber is byte-neutral: v4 (island 1 only) == v6 (all four) == 0. Uniform is the banked form (macro body with its full clobber list, no per-island tuning).

## Frontier (post-s1)
1. Judge/layer-1 classification of the island-1 "memory" clobber (byte-load-bearing; the SDK macro's own clobber list vs an inserted barrier). If FAILed: `ruling-request` asking exactly "is the named SDK macro's clobber list part of the condition-3 template" — do NOT respell (volatile param / cast) — those are the banned spellings of the same reload.
2. Integration handoff surfaces: inline_asm_canonical.txt line + owner_cluster_grants.txt row (func_80019310 is named in the 2026-09-01 grant, decisions.md:18119, but has no row).
3. None on the C side — the body is closed at 0.

## s2 (recon, 2026-09-06 — chassis re-baseline after the Judge FAIL / driver reject)

- H6 CONFIRMED — The banked candidate.c still reaches sandbox 0 on HEAD 52900508 (81/81, rules_dropped 0, stripped and kept). Probe: apply.py + sandbox --disable all, artifacts in tmp/grind/func_80019310/s2/. Floor 0 is chassis-current, not a stale ledger number.

- H7 KILLED (instance) — Transplant of a sibling spelling from func_800187F4 (its s1 ledger) onto this chassis. Measured nothing: the sibling has no candidate.c and its floor is 642, so there is no shared-block spelling to inherit; this candidate is already at 0. measured_on: s2 chassis (HEAD 52900508), candidate.c body df4741401a310eb2, no FAKE constructs.

- H8 CONFIRMED — A Judge PASS ruling is the only door that clears body df4741401a310eb2: review_disposition() (grindlib.py:1338) keys on the Judge's last verdict by timestamp, and Invoke-JudgeRuling (grind.ps1:571-584) writes the clearance from candidate.c's hash on PASS. No construct change is warranted (the 05:55 final call ruled every construct legitimate; the defect was citation-only and is already fixed in the body's comments). Respelling the island-1 reload (volatile param / cast) remains OFF the table — those are banned spellings of the same reload.

## Frontier (post-s2)
1. ruling-request (this session's outcome): ask the Judge to PASS body df4741401a310eb2 as re-cited, clearing it for a bytes + FINAL CALL submission with layer-1 skipped.
2. After a PASS: the driver's grant door still returns None (LOW tier, no registry row). If the merge is refused on that ground, the next session files the integration-handoff record (owner-gated) naming the single operator step: add `func_80019310 cop2-addressing-preamble-cluster.md widened anchor (owner grant 2026-09-01, decisions.md:18082)` to tools/grinder/owner_cluster_grants.txt.
3. None on the C side — the body is closed at 0 and re-measured this session.
