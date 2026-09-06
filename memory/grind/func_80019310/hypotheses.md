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
