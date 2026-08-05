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

## Round 4 — the first +8 reduced to an exact RA spec: `refs+1` on pseudo 85

Atom carried over from decBs0, which closed 58 → 0 the same day on it.
`tools/ra_solver/extract.py gnd_land_hit_char_tsuba text1b` gives 16 allocnos and
`simulate.py` reproduces our allocation **16/16 exactly**. The table ends:

| ord | pseudo | hardreg | nrefs | livelen | pri |
|---|---|---|---|---|---|
| 12 | 84 | 22 ($s6) | 5 | 202 | 495 |
| 13 | 86 | 23 ($s7) | 4 | 194 | 412 |
| 14 | 87 | 30 ($fp) | 3 | 95 | 315 |
| 15 | **85** | **none** | 3 | 188 | **159** |

Pseudo 85 is the only unallocated one and costs no stack slot today (it has a
`reg_equiv_constant` — the `lui`/`addiu` rematerialisation earlier rounds observed).
Priority is `floor_log2(refs)*refs*size/livelen*10000`, so **one more reference** takes 85
from 159 to **425**, past 86 (412) and 87 (315). Simulated (`tmp/csz/gn_refs.py`,
`tmp/csz/gn_atoms.py`): `85: none -> $s7`, `86: -> $fp`, `87: -> NONE`. 87 has no constant
equivalence, so it takes a **real spill slot** — the `64(sp)` local, i.e. the first +8.
Rounds 2-3 all bought the spill by adding a live *value* (costing instructions); this atom
adds a *reference to a value that already exists*.

## Round 5 — identities confirmed, spec is UNIQUE, and the +8 has been LANDED (score 65 → 56)

**Identities** (`tmp/csz/findpseudo.py <dump> <func> <pseudo>` against `text1b.i.lreg`):
85 = `p_b388` (`REG_EQUIV symbol_ref D_8009B388` — hence the free rematerialisation);
86 = `p_b2e0`; 87 = `base_offset` (`plus reg121, 12`); 84 = `c1`. All three refs+1
predictions match the target's documented assignment: `p_b2e0`→$s8, `base_offset` evicted
to a real slot, and $s7 taking `p_b388` — "the value we do not keep".

**The ref arithmetic is confirmed on this function** (def at depth 1 + each in-loop use at
depth 2): `c1` 1+2+2=5 ✓, `p_b2e0` 1+1+2=4 ✓, `base_offset` 1+2=3 ✓, `p_b388` 1+2=3 ✓.
**So an in-loop use adds 2 — the needed +1 must land at loop depth 1.** That is why the
round-2 `s.p1 = p_b388 + 2` (in-loop, = refs+2) was wrong: refs+2 rotates `c1` into $s7,
which target keeps. `perturb.py` over 157 single atoms returns **`pseudo 85: refs+1` as the
ONLY solution** — no live/birth/conf/pref alternative exists.

**Measured negative:** moving `p_b388 = &D_8009B388;` INSIDE the `do/while` to buy the
depth-2 def is **inert** — `loop.c` hoists the invariant back to the preheader, and the
extracted model comes back byte-identical (nrefs still 3). Def placement cannot buy refs.

**The landing spelling (UNCOMMITTED, sitting in `src/text1b.c` — see caveat below):**
precompute half 2's pointer outside the loop, which is a depth-1 reference to `p_b388`:
```c
p_b388 = &D_8009B388;
p_b390 = p_b388 + 2;      /* depth-1 ref -> nrefs 3 -> 4, pri 159 -> 425 */
do { ... s.p1 = p_b388; ... s.p1 = p_b390; ... }
```
Result: **score 65 → 56 (best yet), `vars` 48 → 56 (+8, the spill LANDED), frame 104 → 112.**

**Residual: +2 instructions (178 vs 176) and the frame is still 8 short of 120.** The new
`p_b390` pseudo (88) does NOT inherit a constant equivalence cleanly — it and `p_b2e0` both
end unallocated, so something is materialised where target rematerialises. Next round should
either find a depth-1 `p_b388` reference that does NOT introduce a new pseudo, or check
whether `p_b390` can be made to fold to `REG_EQUIV symbol_ref D_8009B390` (target
rematerialises `&D_8009B390` as `lui`/`addiu`, align idx 106-107).

## Next

1. **Kill the +2 insns from the round-5 spelling** (above) — that is now the whole first-half
   residual; the RA spec itself is satisfied.
2. **The second +8 is the phantom class, not a spill** — run the recipe's orphan detector
   (unallocated pseudos in the `-da` greg dump / bare `(use (reg N))` in the combine dump)
   rather than hunting more live values. Producer #1 (folded loop-guard compare) is the
   likely shape: the guard here is `((D_800A326C + 1) * 2)`, recomputed in the `do/while`
   condition — a comparison over a real variable, exactly the documented producer.
