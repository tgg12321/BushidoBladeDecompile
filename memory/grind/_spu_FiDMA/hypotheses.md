# Hypothesis ledger — _spu_FiDMA

## Session 1 (2026-09-07, recon) — frontier CLOSED, function matched

### H1 — CONFIRMED (floor 48 -> 0)
**Statement:** The banked psyz transplant seed
(`memory/grind/_spu_FiDMA/psyz-seed.c`, from Xeeynamo/psyz
`decomp/src/libspu/spu.c:161-181`), spliced in place of
`INCLUDE_ASM("asm/funcs", _spu_FiDMA);` at src/main.c and with `D_800A2D14`
renamed to its current symbol name `_spu_transferCallback`, reproduces the
48-instruction target exactly against the HEAD declarations, with no
declaration change, no coercion construct and no rules.
**Mechanism:** the seed is the original Sony source shape, so no codegen lever is
involved — the guarded `_spu_Fw1ts()` call, the SPUCNT read-modify-write, the
bounded 0xF00 spin, the `volatile` double-load of the callback slot before the
`jalr`, and the `DeliverEvent(0xF0000009, 0x20)` fallthrough each map 1:1 onto the
target's instruction groups.
**Probe:** splice + `sandbox _spu_FiDMA --disable all` + `verify-oracle`.
**Result:** `score 0` (target_insns 48, build_insns 48); full build
`ok: true, build_matches: true`. Body banked at
`memory/grind/_spu_FiDMA/candidate.c`; self-vet at
`memory/grind/_spu_FiDMA/self_vet.md` (no FAKE construct, no family claimed).

### H2 — KILLED (instance): the fork-segfault premise
**Statement:** On the current chassis (HEAD src/main.c, cc1 rebuilt 2026-08-07),
compiling Sony's literal `while (*(volatile u16 *)(D_800A2CDC + 0x1AA) & 0x30)
{ timeout++; if (timeout > 0xF00) break; }` inside src/main.c crashes cc1, as the
2026-07 campaign recorded at `memory/closer/phase3-progress.md:353`.
**Mechanism claimed by the original finding:** a decompals-fork bug on the
"volatile read in the while condition + early exit in the body" loop class.
**Probe:** compile that exact spelling as part of the H1 body.
**Result:** no crash; the TU compiles and the loop emits the exact target bytes
including the trailing `addiu $v1,$v1,-1` that the 2026-07 analysis attributed to
reorg.c's increment-compensation clause and believed unreachable from our fork.
The premise that routed this function to the fork-divergence-inline-asm carve-out
is therefore not true of the current compiler.
**kill_scope:** instance. **measured_on:** HEAD src/main.c chassis 2026-09-07,
cc1 = tools/gcc-2.7.2/cc1 (post-2026-08-07 rebuild), no FAKE constructs present,
full 48/48 function.

## Frontier for any future session
Empty — the function matches at distance 0 with ordinary C and the full build
re-verified against the oracle. If a later chassis change reopens it, start from
`memory/grind/_spu_FiDMA/candidate.c`, not from a fresh derivation.
