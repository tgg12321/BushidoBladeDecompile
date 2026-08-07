# saEft00Add — WIP (grind2 lane, 2026-08-07)

## State: candidate = HEAD's alias form + mandatory FAKE annotations. Awaiting SHA1.

Candidate: `tmp/cand/system_final.c` (measures 133/133, honest score 0).

## History this session

1. Tried to REPLACE the two pointer aliases with an alias-free spelling: an
   in-body `extern volatile CdlREAD D_800A14D0;` one-object view with the two
   la-form sites as `D_800A14D0.tslmode` (+0x30) / `D_800A14D0.mode` (+0x0C).
   It reached 133/133 with only relocation-addend differences.
2. **Layer-2 FAILED it.** Grounds, which I accept: the struct member spelling is
   the SAME address-materialization coercion in different syntax; the cited
   precedent `func_800827D0` does not license it (there the one-object view is
   genuinely load-bearing because target caches ONE base register and reads
   SEVERAL members off it — here the two sites are independent single-use
   materializations); and every variant swept lived inside the alias family.
3. Ran the non-alias sweep the FAIL demanded (below). All plateau at 131/133.
4. Disposition is therefore the sanctioned carve-out
   ([[pointer-alias-fake-exception]]), which is exactly what the 2026-08-07
   adjudication said applies "only on documented exhaustion".

## The gap

Target materializes each address into its own register — 0x80082440 and
0x8008252C: `lui / addiu / lw 0(reg)`. A direct global read leaves a bare
`(mem (symbol_ref))`, which aspsx expands to the 2-insn `%hi/%lo` macro form and
drops both `addiu`. That is the entire 4-instruction gap. The third read of the
SAME word (src/system.c:1139) stays direct in both target and candidate — target
uses macro form there, so the mix is target's, not ours.

## Lever exhaustion

NON-ALIAS family (`tmp/sweep_nonalias_saeft.py`) — none reaches the insn count:

| variant | insns | score |
|---|---|---|
| x0 direct scalars (control) | 131/133 | 4 |
| x1 value locals (not address aliases) | 131/133 | 4 |
| x2 callbacks after the tslmode test | 135/133 | 16 |
| x3 split byte-read of mode | 133/133 | 6 |
| x4 captured callback return values | 131/133 | 4 |
| x5 hoisted predicate flag | 131/133 | 4 |

ALIAS / struct family (`tmp/sweep_saeft.py`): struct at A+B 133/133; A only 132;
B only 132; A+B+C 134; tail `.sectors` 134; `buf`/`p` via struct 134.
The A+B form is the one layer-2 FAILed.

## Prerequisites for the carve-out (all four met)

1. **Exhaustion documented** — the two tables above; the direct form was measured
   FIRST, not reached for after the alias.
2. **GCC-pass interaction named** — address materialization: the direct read
   leaves `(mem (symbol_ref))` for the assembler macro; the alias gives the
   symbol address its own pseudo that survives to the emitted `la` form.
3. **`/* FAKE: ... */` annotation** at BOTH alias declarations — present in
   `tmp/cand/system_final.c`.
4. **Layer-1 + layer-2 review** — layer-2 pending on this revised form.

## In-file precedent (verified against engine/queue.json — none of these is queued)

- `saEft00Add_sub` (src/system.c:1155) is COMPLETED and ships
  `volatile s32 *tsl = &D_800A1500;` — the identical shape on the identical word.
- `func_800826CC` (src/system.c:1172) is COMPLETED and ships
  `volatile s32 *ps; ps = &D_800A14D0; *ps = sectors;` with a comment recording
  that target materializes the address.

Both alias the same Sony cdread block; the shape is already the accepted,
committed form in this file.

## Scorer artifact (settled, NOT at issue)

The recorded distance-1 is the LO16 `.text`-addend artifact: `D_80082050` is
static, so its reference relocates section-relative, and the sandbox's
cheat-stripping of SIBLING functions shifts the addend by 0x9B0. Confirmed this
session via `tmp/reloc_check.py`. Sandbox scores also drift run to run because
`build/src/system.o` is being rebuilt by another lane; the masked instruction
diff and the relocation table are stable, the scalar score is not.

## Scope

`tmp/scope_check_saeft.py`: across all 45 functions in system.o only saEft00Add's
region is touched. (That check compares instruction TEXT, not relocations.)

## Next step

Build window -> apply -> `retire` -> `queue done` (SHA1 == oracle
`62efab4f73f992798c43e8c730aa43baa10bb4fa`) -> layer-2 PASS -> `Match:` commit
citing the artifact evidence, the exhaustion tables, the carve-out, and the
c73a9c8b / 7b46c476 revert history.
