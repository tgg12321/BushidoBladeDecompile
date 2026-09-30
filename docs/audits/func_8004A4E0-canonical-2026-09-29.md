# func_8004A4E0 canonical-asm authorization audit — 2026-09-29

**Recommendation (A): the whole-body authorization (`inline_asm_canonical.txt:198`) is CORRECT.**
Decisive signal: a custom callee-saved-register ABI across `jal` (S7-class ABI property — reaudit
trap 3; `rules/inline-asm-allowed.md` custom-calling-convention ground). No pure-C candidate built:
the ABI evidence decides it. Supersedes the 2026-09-16 memo's "likely overshoots, re-derive as C
with ~6 islands".

Auditor: subagent audit-8004a4e0. Key claims ($s5, $s4, add/sub/syscall/addi counts) spot-verified
by the orchestrator against `asm/funcs/func_8004A4E0.s` and `asm/funcs/func_8004A940.s`.

## Trap 0
No owner ruling on record. Only `decisions.md:28114` (the unruled 2026-09-16 collateral finding).

## Tools (both blind to this property)
- `scan_hand_coded.py --single`: tier=LOW, 0/8 — S7 checks callee-side unsaved callee-saves; this
  function is the CALLER and saves everything.
- `engine.cli canonical`: ASM-PARTIAL 6/163 — 2 trapping `addi` (0x8004A5A0, 0x8004A67C), 4 `ctc2`
  (0x8004A6B4/6B8/6CC/6D0). Per-instruction only.

## Decisive evidence (main callee func_8004A940, canonical :197, frameless)

| Reg | func_8004A4E0 | Callee | What C would do |
|---|---|---|---|
| `$s4` in | set before each `jal func_8004A940` (0x8004A668/66C, 67C, 6D4/6D8, 710/714, 750/754); reloaded from scratchpad after (688, 6E4, 724, 760); never read locally | 0x8004A95C `addu $t0,$t0,$s4` → `jalr` (handler-table base) | dead stores, deleted |
| `$s5` back | written once `$s5=0` @0x8004A524; read as `3-$s5` @0x8004A618/648/698/6F4/734 → 0x1F800008 | written @0x8004A9F4, AA04, AA08, AB6C, AB7C | constant 0, `3-s5` folds to `li 3` |
| `$s0` both | set @0x8004A584; read after callee @0x8004A5C0 `lhu 0($s0)` | advanced by func_8004A76C (0x8004A798), func_8004A808 (0x8004A870/924/92C), func_8004A940 | GCC assumes unchanged |
| `$s6`/`$fp` in | 0x8004A538 `$s6=$fp`; never read locally | read @0x8004AAA4/AAB4 | dead, deleted |
| `$s3` ghost | saved/restored @0x8004A4FC/5FC, unused in body | func_8004A940 clobbers unsaved (0x8004AA40) | not saved → prologue mismatch |

**Why islands can't work:** they'd have to cover every `$s4`/`$s5`/`$s6`/`$fp`/`$s0` handoff and the
`jal` sites — GPR asm steering codegen (forbidden, `inline-asm-allowed.md`). Register pins are
forbidden (`completion-standard.md`) and GCC would still fold `3-s5`, delete the dead pre-call
stores, and omit the `$s3` save. Remainder = full call protocol + prologue = whole body.

**Supporting (not decisive):** neither jump-table `jr` (0x8004A558, index `lbu` @0x8004A53C;
0x8004A5B8) has a range check; the function has no `sltiu`/`sltu`. Corpus: 58/58 non-`$ra` `jr` in
pure-C functions are guarded (`jrguard.py 40` → guarded=58 unguarded=0). At a 12-insn window, 3
misses (func_80027AD8, func_80038C70) — two spot-checked, both guarded by earlier `beqz $v0`.

## Citation corrections (entry text only — needs an owner ruling committed first)
1. `:198` "see known_blocked.txt" — pruned but recoverable: `git show 713493325^:known_blocked.txt`
   line 65 (accurate, 0x8004A5A0).
2. `:198` gives only per-insn reasons (`handwritten_overflow_op`, `cop2_function`); omits the
   decisive cross-call register ABI above.
3. `known_psyq_stdlib.txt:28` "add/addi/sub/syscall" — function has 0 add, 0 sub, 0 syscall; 2 addi.
4. `:182` (func_8004A808) "caller does NOT set $a0 or $s0 before jal" — it does, earlier on the
   path: `$a0` @0x8004A594 (`andi $a0,$a1,0xFFFF`), `$s0` @0x8004A584/5A0/5C4, live through the
   dispatch. `$a0` is ordinary argument passing; only the `$s0` handoff is custom. Scratchpad
   0x1F800008 claim is accurate (0x8004A650, 0x8004A738).
5. `tmp/asmaudit/FINDINGS.md:66,209` "standard ABI" — false.
6. `docs/grind/decisions.md:28115` and memory `project/func-8004a4e0-canonical-underevidenced.md`
   cite `:201`; entry is now `:198`. Memo's re-derive recommendation superseded; "does not exist"
   imprecise (see 1).
7. Minor: `:185` (math_RotMatrixZYX) "genuinely USES $s5..$s7/$fp" — true, but `$s3` is a ghost
   save of the same kind as :185's own evidence (2).

## Files
`jrguard.py` (run under WSL: `python3 tmp/audit-2026-09-29/8004a4e0/jrguard.py 40`).

## Provenance
Tracked copy (2026-09-29) of `tmp/audit-2026-09-29/8004a4e0/FINDINGS.md` (gitignored). The `jrguard.py` helper named above stays in that gitignored directory. Owner ruling applying the entry-text corrections: Q40, `docs/grind/owner-rulings-2026-09-26.md` (803d0fea1).
