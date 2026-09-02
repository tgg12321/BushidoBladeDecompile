# Evidence bank — func_800480C0

## s1 — recon (2026-09-02, chassis HEAD dd2808d5, -mel, 0 regfix/asmfix rules)

- **Shape.** 74-insn loop: `p = arg0 + (s16)arg1*4; p = arg0 + (*p & ~3);
  count = *p++; while (count--) { word, lhu*4 -> func_800482C8(arg0 + (word & ~3),
  a1v+arg2, a2v+arg3, a3v+arg4, v0v+arg5) }`. It is the 6-arg twin of COMPLETED-C
  `func_80047FBC` (4-arg, src/text1b.c:84) — identical prologue idiom (`addu $s0,$a0`;
  `addu $s2,$s0`), identical loop body, plus two extra stack args (`lw $v1,0x68($sp)`,
  `lw $a0,0x6C($sp)`) sign-extended into $s4/$s3 inside the `if`.
- **Frame forensics.** `.frame $sp,0x58`: outgoing args 0x00-0x17 (5-arg call; `sw $v0,0x10($sp)`
  is the only sp-relative store outside the register saves), vars 0x18-0x37 = 32 bytes with
  ZERO `sw`/`lw` anywhere in `asm/funcs/func_800480C0.s`, regs $s0-$s6+$ra at 0x38-0x57.
  Exactly the same allocated-but-untouched 32-byte region the three text1b siblings carry
  (`engine/volatile_cheats.py:753-767`: func_80047EE8, func_80047FBC, func_800481E8 all
  `("pre_pad", 8)`).
- **Attribution ladder** (`tmp/grind/func_800480C0/s1/ladder.txt`), all `sandbox --disable all`:
  | body | score |
  |---|---|
  | clean (no `arg0 = 0`; pad stripped by detector) | 32 (74/74) |
  | + `arg0 = 0; /* FAKE */` (pad still stripped) | **20** (74/74) — honest floor as the engine reads it today |
  | + `volatile u32 pre_pad[8]` honoured (real build) | **0** — full build SHA1 == oracle (`s1/build.log`) |
  All 20 residual insns at score 20 are sp-offset deltas only (`addiu $sp,-0x38` vs `-0x58`,
  8 saves, 8 restores, 2 stack-arg loads, final `addiu $sp`): the instruction STREAM is
  byte-identical (`s1/cand1_build.txt` vs target, diffed in-session).
- **Why the sandbox cannot read 0.** The volatile-cheat stripper removes every unused local
  array unless `_is_sanctioned_pad` finds an exact `(func, name, count)` row in
  `engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` (line 746). `engine/` is on the
  scope-grant DENYLIST (`.claude/rules/integration-handoff-self-serve.md:63`,
  `tools/grinder/grindlib.py:402`) — owner-only. The siblings got their rows by owner ruling
  2026-08-20 (func_80047FBC) and the 2026-08-22 parked-but-proven audit (func_800481E8).
- **Canonical gate:** verdict C, distance 20. `scan_hand_coded --single`: LOW 1/8 (S4 only) —
  no canonical-asm signal; not wanted anyway, the C form is proven.
- **Duplicate leads:** `tmp/duplicates_leads.txt` has no entry for func_800480C0; scan S5
  reports no >0.5-jaccard sibling (the twin differs by the two stack args).
- **Retired chassis (2026-08-24 pin):** old pinned floor 36 came from a 33-rule regfix body
  with five register-asm pins (`retired-chassis-2026-08/body.c`); superseded entirely.

- [s1] target .frame 0x58: args 0x00-0x17, vars 0x18-0x37 with ZERO sw/lw in asm/funcs/func_800480C0.s, regs 0x38-0x57 - same layout as siblings func_80047EE8/func_80047FBC/func_800481E8 (engine/volatile_cheats.py:757-767)

- [s1] sandbox strips volatile pre_pad unless _SANCTIONED_UNWRITTEN_PADS (engine/volatile_cheats.py:746) has an exact row; engine/ is scope-grant denylisted (.claude/rules/integration-handoff-self-serve.md:63) - owner-only

- [s1] scan_hand_coded --single func_800480C0: LOW 1/8 (S4 only) - no canonical-asm signal

- [s1] retired 2026-08 chassis floor 36 (33 regfix rules, 5 register pins) is superseded; ledger floor now 20 (honest as the engine reads it) / 0 (bytes proven)
