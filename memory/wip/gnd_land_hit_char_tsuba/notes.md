# gnd_land_hit_char_tsuba — WIP seed (2026-08-05)

**State: honest pure-C distance 65 (unchanged). Frame delta decomposed; one measured kill.**
Tree at HEAD `71253c2d`. 88 rules, canonical ASM-SUSPECT, insn count already exact (176/176).

## The frame delta, decomposed

Target `addiu sp,sp,-120`, ours `-104`: **+16 bytes of locals**. The callee-save SET is
identical (s0-s8 + ra, 10 regs) and every slot shifts by exactly 16, so 22 of the 49
non-rename diff lines collapse to this one number. `vars` = 80 (target) vs 64 (ours).

Layout, read off the stores (both sides agree on every field offset):
- `0..15`  — a0-a3 home area
- `16..59` — the `S46C s` local struct: `s.p1` @20, `s.zero18` @40, `s.zero1C` @44,
  `s.c20` @48, `s.c24` @52. Ours rounds to 64. **Field offsets match target exactly**, so
  the struct's layout and placement are already right — growing `S46C` is not the answer
  unless the growth is entirely past `byte28`.
- `64` — **target only**: a live 4-byte local holding a pointer. Target spills it:
  ```
  lui   s8,%hi(D_8009B2E0) ; addiu s8,s8,%lo ; addiu v0,s8,12 ; addu v0,s0,v0
  sw    v0,64(sp)                       <- stored to the local
  ...  lw a3,64(sp)                     <- reloaded at the second use
  ```
  Ours keeps the same value in `$s8` (`addu v0,s0,s7 ; addiu s8,v0,12`) and never spills.
  This is the `base_offset` variable.
- `68..79` — 12 bytes **no instruction on either side touches**: the phantom slots
  ([[phantom-slot-frame-lever]]).

## Measured kill

- Reassociating `base_offset = (s32 *)(p_b2e0 + stride + 0xC)` to `(p_b2e0 + 0xC + stride)`
  to mirror target's `(SYM+12)+stride` operand order — **inert, 65 → 65**. GCC canonicalizes
  the pointer PLUS chain, so source-level association is not the lever for that pair.

## Next (the documented recipe, not yet run here)

1. **Frame gradient first, score second** — `tmp/frame_probe.sh` pattern: cpp | project cc1
   | read `# vars=` for the function. Separates "wrong frame" from "wrong codegen"; the
   score is a poor instrument for a 16-byte delta.
2. **Orphan detector** on the `-da` greg dump: which pseudos are unallocated in TARGET's
   shape but allocated in ours. 12 bytes = 3 phantom words; producer #1 (folded loop-guard
   compare) is the likeliest — the loop guard here is `((D_800A326C + 1) * 2)`, recomputed
   in the `do/while` condition, which is exactly the "guard comparison involving a real
   variable" shape.
3. The 4th word is the `base_offset` spill at 64(sp). Register pressure is already maximal
   (all of s0-s8 live); the question is what makes TARGET need one more simultaneously-live
   value. Note ours holds `p_b2e0` in `$s7` across the loop where target rematerialises
   `lui/addiu` per use (align idx 66-67, 106-107) — dropping the `p_b2e0` local so the
   symbol is re-materialised may free the register AND force the spill. **Untried.**

Instruments: `wsl bash tmp/csz/d.sh gnd_land_hit_char_tsuba text1b` then
`python tmp/csz/align.py gnd_land_hit_char_tsuba` (28 RENAME + 49 MOVED/FRAME).
