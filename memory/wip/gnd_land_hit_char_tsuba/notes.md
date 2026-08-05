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

## Round 2 — struct-size hypothesis KILLED; pressure mechanism CONFIRMED

Frame gradient (`wsl bash tmp/csz/frame.sh gnd_land_hit_char_tsuba text1b`):
`ours[.frame $sp,104 # vars= 48, regs= 10/0, args= 16]` vs target 120 ⇒ **vars 48 → 64**.

**The attractive hypothesis — that `S46C` is under-sized (44→60 would give +16 with every
field offset unchanged) — is decisively FALSE.** `S46C` has three users in text1b.c, and
the other two match the oracle exactly today with `vars= 48` and **zero rules**:

| function | ours | target | rules |
|---|---|---|---|
| func_8005D46C | `.frame $sp,88 # vars= 48, regs= 5/0, args= 16` | `addiu sp,sp,-88` | 0 |
| func_8005FA98 | `.frame $sp,80 # vars= 48, regs= 4/0, args= 16` | `addiu sp,sp,-80` | 0 |
| gnd_land_hit_char_tsuba | `.frame $sp,104 # vars= 48, regs= 10/0, args= 16` | `addiu sp,sp,-120` | 88 |

Growing the struct would break two matched functions. The +16 is **register pressure**,
local to this function: target holds one more simultaneously-live value than it has
callee-saves (both sides already use all of s0-s8 + ra), so one pseudo spills to `64(sp)`.

### Two independent producers of the first +8, both measured, NOT additive

| change | vars | score | insns |
|---|---|---|---|
| baseline | 48 | 65 | 176 |
| `s.p1 = p_b388 + 2;` instead of `&D_8009B390` (forces `p_b388` live across the loop instead of being rematerialised as `lui`/`addiu`) | **56** | 65 | 177 |
| all four `(s32)rN - K + (rand term)` split into `x = rN - K; <call>; x += …` ([[split-init-accumulation-sanctioned]]) — this is target's shape: it hoists `addiu $a0,$s5,-25` / `addiu $a2,$s4,-12` / `-50` / `-25` ABOVE their calls, we compute them after | **56** | 68 | 178 |
| both together | **56** | 68 | 178 |

Both create the SAME single spill slot. The mechanism is confirmed (pressure → spill →
`vars` grows in 8-byte steps) but neither spelling is free: each costs 1-2 instructions
where target has none, so neither is the original's wording.

## Next

1. **The second +8 is the phantom class, not a spill** — run the recipe's orphan detector
   (unallocated pseudos in the `-da` greg dump / bare `(use (reg N))` in the combine dump)
   rather than hunting more live values. Producer #1 (folded loop-guard compare) is the
   likely shape: the guard here is `((D_800A326C + 1) * 2)`, recomputed in the `do/while`
   condition — a comparison over a real variable, exactly the documented producer.
2. **The first +8 needs a zero-instruction spelling.** Target rematerialises
   `&D_8009B390` (`lui`/`addiu`, align idx 106-107) so it is NOT derived from `p_b388`;
   the extra live value target carries is something else. Diff the two sides' callee-save
   assignments (ours: `&D_8009B2E0`→$s7 and `p_b388` rematerialised; target: `&D_8009B2E0`
   →$s8 and $s7 holding a value we do not keep) to identify it.
