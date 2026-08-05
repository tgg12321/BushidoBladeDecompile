# gnd_land_hit_char_tsuba — WIP (current state 2026-08-05, round 4)

**Honest pure-C distance 65.** 88 rules, canonical ASM-SUSPECT, insn count already exact
(176/176). The whole residual is a **+16-byte frame delta** plus the register renames that
follow from it. Round 4 reduced the first half of that delta to an exact, simulated RA spec.

Measure: `wsl bash tmp/csz/gn_score.sh` (driver `tmp/csz/gn_var.py`),
`wsl bash tmp/csz/frame.sh gnd_land_hit_char_tsuba text1b`.

## The frame delta, decomposed

Target `addiu sp,sp,-120`, ours `-104`; `vars` 64 vs 48. Callee-save SET is identical
(s0-s8 + ra) and every slot shifts by exactly 16, so 22 of the 49 non-rename diff lines
collapse to this one number. Layout read off the stores (both sides agree on every field
offset):

- `0..15` — a0-a3 home area.
- `16..59` — the `S46C s` local struct (`s.p1` @20, `s.zero18` @40, `s.zero1C` @44,
  `s.c20` @48, `s.c24` @52). Field offsets match target exactly.
- `64` — **target only**: a live 4-byte local holding a pointer, spilled and reloaded:
  ```
  lui s8,%hi(D_8009B2E0) ; addiu s8,s8,%lo ; addiu v0,s8,12 ; addu v0,s0,v0
  sw  v0,64(sp)                    <- stored to the local
  ... lw a3,64(sp)                 <- reloaded at the second use
  ```
  Ours keeps the same value in `$s8` (`addu v0,s0,s7 ; addiu s8,v0,12`) and never spills.
- `68..79` — 12 bytes **no instruction on either side touches**: phantom slots
  ([[phantom-slot-frame-lever]]).

## Settled negatives — do NOT re-run these

- **`S46C` is not under-sized.** Growing it 44→60 would give +16 with every field offset
  unchanged, but the struct's two other users in text1b.c (`func_8005D46C`,
  `func_8005FA98`) match the oracle today at `vars= 48` with ZERO rules. Growing it breaks
  them. The +16 is register pressure local to this function.
- **Pointer reassociation is inert.** `base_offset = (s32*)(p_b2e0 + stride + 0xC)` →
  `(p_b2e0 + 0xC + stride)` to mirror target's operand order: 65 → 65. GCC canonicalizes
  the PLUS chain.
- **Two spellings DO produce the first spill, but neither is free** (each costs 1-2 insns
  where target has none, so neither is the original's wording): `s.p1 = p_b388 + 2` instead
  of `&D_8009B390` (vars 48→56, score 65, 177 insns); and splitting all four
  `(s32)rN - K + (rand term)` per [[split-init-accumulation-sanctioned]] (vars 56, score 68,
  178 insns). Not additive — both create the SAME single slot.
- **The exec_game identity/holder levers do NOT transfer** (measured, frame stayed 48 in
  every variant): inlining the `c100`/`c1` holders as literals scores **67** (they are
  load-bearing here, unlike exec_game's), and splitting `a0_offset`/`a2_offset` per
  call-half is inert — GCC coalesces the split because the two ranges do not overlap.

## Round 4 — the first +8 is now an exact RA spec: `refs+1` on pseudo 85

Carried over from decBs0, which closed 58 → 0 the same day on this same atom.
`tools/ra_solver/extract.py gnd_land_hit_char_tsuba text1b` gives 16 allocnos and
`simulate.py` reproduces our allocation **16/16 exactly**, so the model is trustworthy
here. The table ends:

| ord | pseudo | hardreg | nrefs | livelen | pri |
|---|---|---|---|---|---|
| 12 | 84 | 22 ($s6) | 5 | 202 | 495 |
| 13 | 86 | 23 ($s7) | 4 | 194 | 412 |
| 14 | 87 | 30 ($fp) | 3 | 95 | 315 |
| 15 | **85** | **none** | 3 | 188 | **159** |

Pseudo 85 is the only unallocated one, and it costs no stack slot today because it has a
`reg_equiv_constant` — this is the `lui`/`addiu` rematerialisation the earlier rounds
observed (almost certainly `p_b388` / `&D_8009B390`; **CONFIRM against the `.lreg`/`.greg`
dump before spelling anything**).

**Give pseudo 85 exactly ONE more real reference and the frame becomes target's.**
Priority is `floor_log2(refs)*refs*size/livelen*10000`, so 3 → 4 refs takes 85 from 159 to
**425**, past 86 (412) and 87 (315). Simulated (`tmp/csz/gn_refs.py`):

```
85: none -> 23 ($s7)      86: 23 -> 30 ($fp)      87: 30 -> NONE
```

87 is then the evicted pseudo. Unlike 85 it is expected to be a computed value with no
constant equivalence, so it takes a **real spill slot** — the `64(sp)` local above, i.e.
the first +8. `refs+2` works too but rotates $s6 as well, so **refs+1 is the minimal one
to aim for**.

Why this beats rounds 2-3: every spelling tried there bought the spill by adding a live
*value*, which costs instructions. This atom adds a *reference to a value that already
exists* — which is exactly what "zero-instruction spelling" means.

## Next

1. **Realize `refs+1` on pseudo 85 at zero instruction cost.** First identify 85 exactly
   (`.lreg`/`.greg` dump → which C variable), then find a reading of the body where that
   value is genuinely referenced once more. NOTE the decBs0 realizer: a `goto` loop carries
   no `NOTE_INSN_LOOP_BEG`, and `flow.c` weights `reg_n_refs += loop_depth` (flow.c:2081/
   2329/2515/2725), so a use that sits *inside* a real loop is worth 2 refs, not 1. This
   function's main loop is ALREADY a `do/while`, so the free +1 must come from moving a use
   into that loop (or a second real loop) — not from a loop-form change.
2. **The second +8 is the phantom class, not a spill** — run the recipe's orphan detector
   (unallocated pseudos in the `-da` greg dump / bare `(use (reg N))` in the combine dump)
   rather than hunting more live values. Producer #1 (folded loop-guard compare) is the
   likely shape: the guard here is `((D_800A326C + 1) * 2)`, recomputed in the `do/while`
   condition — a comparison over a real variable, exactly the documented producer.
