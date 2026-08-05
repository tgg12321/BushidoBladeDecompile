# gnd_land_hit_char_tsuba — WIP (current state 2026-08-05, round 6)

`src/text1b.c:12751`. HEAD baseline: honest pure-C distance **65**, 88 rules, canonical
ASM-SUSPECT. Measure: `wsl bash tmp/csz/gn_score.sh` (driver `tmp/csz/gn_var.py`),
`wsl bash tmp/csz/frame.sh gnd_land_hit_char_tsuba text1b`.

## Candidates (banked as diffs, tree left clean)

| file | score | insns | frame |
|---|---|---|---|
| `candidate_56.diff` | 56 | 178 vs 176 | vars 56 (target 64) |
| `candidate_40.diff` | **40** | **176 vs 176** | vars 56 (target 64) |

`candidate_40` = `candidate_56` + split-init accumulation on **all four** offset statements.

## The frame delta, decomposed

Target `addiu sp,sp,-120`, ours `-112`; `vars` 64 vs 56. Callee-save SET is identical
(s0-s8 + ra). Layout read off the stores (both sides agree on every field offset):

- `0..15` a0-a3 home area · `16..59` the `S46C s` local struct (`s.p1` @20, `s.zero18` @40,
  `s.zero1C` @44, `s.c20` @48, `s.c24` @52) — field offsets match target exactly.
- `64` — a live 4-byte local holding a pointer, spilled and reloaded. **LANDED in round 5.**
- `68..79` — 12 bytes **no instruction on either side touches**: phantom slots
  ([[phantom-slot-frame-lever]]). **This is the whole remaining delta.**

## Round 5 — the first +8, landed via an exact RA spec (`refs+1` on pseudo 85)

`tools/ra_solver` reproduces our allocation 16/16 exactly. Pseudo 85 (`p_b388`,
`REG_EQUIV symbol_ref D_8009B388`) was the only unallocated allocno, priority 159. Priority is
`floor_log2(refs)*refs*size/livelen*10000`, so **one more reference** takes it to 425, past 86
(412) and 87 (315) — evicting 87 (`base_offset`, no constant equivalence) to a **real** spill
slot, i.e. the `64(sp)` local. `perturb.py` over 157 single atoms returns `pseudo 85: refs+1`
as the **ONLY** solution.

Ref arithmetic confirmed on this function (def at depth 1 + each in-loop use at depth 2):
`c1` 1+2+2=5 ✓, `p_b2e0` 1+1+2=4 ✓, `base_offset` 1+2=3 ✓, `p_b388` 1+2=3 ✓ — **so an
in-loop use adds 2; the needed +1 must land at loop depth 1.** The landing spelling
precomputes half 2's pointer outside the loop:
```c
p_b388 = &D_8009B388;
p_b390 = p_b388 + 2;      /* depth-1 ref -> nrefs 3 -> 4, pri 159 -> 425 */
```

## Round 6 — the +2 instructions are GONE (56 → 40, count now exact)

Target folds each `rN - K` into an accumulator held across the call
(`addu a2,a2,v0`); we were emitting `addiu v0,v0,-K; addu v0,sX,v0`. Split-init accumulation
([[split-init-accumulation-sanctioned]]) on the offset statements fixes it:
```c
a2_offset = (s32)r4 - 0x19;
a2_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
```
Measured sweep (`tmp/csz/gn_split.py <variant>`, driver `tmp/csz/gn_sweep.sh`) — monotone in
the number of statements split: none 56/178, 2nd-half a2 only 49/178, 2nd-half a0 only 51/177,
2nd-half both 44/177, +1st-half a2 43/177, +1st-half a0 41/176, **all four 40/176**.

**This supersedes the round-4 negative** ("splitting all four → score 68, 178 insns"), which
was measured on the PRE-round-5 source. On the round-5 spelling it is the best form found.

**If this lands it needs the family's `/* FAKE */` annotation** per the sanctioned-family
rule — not yet added, since nothing is being committed.

## Residual at score 40 — the phantom +8 and nothing else

```
target: addiu a2,s4,-25   |  ours: move a1,zero      (one moved pair, counts equal)
target: slt v1,s2,v1      |  ours: slt v1,s3,v1      ($s2/$s3 rename)
target: lw ra,116(sp) ... addiu sp,sp,120
ours:   lw ra,108(sp) ... addiu sp,sp,112            (every slot -8)
```
The epilogue shift IS the +8; the rename and the moved pair follow from it.

## Settled negatives — do NOT re-run

- **`S46C` is not under-sized.** Growing it 44→60 gives +16 with field offsets unchanged, but
  its two other users in text1b.c (`func_8005D46C`, `func_8005FA98`) match the oracle today at
  `vars= 48` with ZERO rules. Growing it breaks them.
- **Pointer reassociation is inert** (`p_b2e0 + stride + 0xC` → `p_b2e0 + 0xC + stride`): 65 → 65.
  GCC canonicalizes the PLUS chain.
- **Def placement cannot buy refs.** Moving `p_b388 = &D_8009B388;` inside the `do/while`
  is inert — `loop.c` hoists it back to the preheader; the extracted model is byte-identical.
- **The exec_game identity/holder levers do NOT transfer** (frame stayed 48 in every variant):
  inlining the `c100`/`c1` holders as literals scores 67 (they are load-bearing here), and
  splitting `a0_offset`/`a2_offset` per call-half is inert — GCC coalesces the split because
  the two ranges do not overlap.

## Round 7 — phantom producer #1 via the entry guard is a MEASURED NEGATIVE

Hypothesis was [[phantom-slot-frame-lever]] producer #1 (a folded guard-compare pseudo left
ref'd-but-dead, paid a free slot by reload's `alter_reg`) applied to the entry guard
`((D_800A326C + 1) * 2) > 0`. Three spellings on top of `candidate_40`
(`tmp/csz/gn_phantom.py`, driver `tmp/csz/gn_ph_run.sh`) — name the whole guard value; name
only the `+1`; name it as a `count` — are **all completely inert**: score 40, 176 insns,
`vars` 56 in every case.

**Why, from the target asm** (`asm/funcs/gnd_land_hit_char_tsuba.s`): the entry guard compiles
to a **bare `blez $v1, .L8005D7D0`** (line 53) and the loop condition to
`slt $v1,$s2,$v1; bnez` (161-162). A `blez` needs no compare pseudo at all, so there is
nothing for a named local to strand — the compare folds identically whether or not the value
has a C name. Producer #1 is not reachable through this guard.

Target's `D_800A326C` traffic for reference: read in the prologue (line 2), stored at 22,
re-read for the entry guard at 49, re-read for the loop condition at 157, re-read + stored for
the trailing `+= 1` at 165/168. Our C already matches that shape — both the guard and the loop
condition recompute `(D_800A326C + 1) * 2` from a fresh read.

## Next

1. **The remaining +8 is still the phantom class**, but producer #1 is ruled out (above). Try
   the other two documented producers: **#3 live named locals on multi-read values** (the
   obvious candidate is `D_800A3418`, read repeatedly as `(u32)(D_800A3418 * K) >> 0xF` and
   `(D_800A3418 & 1) * 0xC` between its `^=` updates), and **#2 combine orphan-USE** (needs an
   HImode intermediate with a second genuinely-narrow use — less likely here, the arithmetic
   is all s32/u32).
2. Run the recipe's orphan detector on our own build first (`-da` greg dump unallocated set /
   bare `(use (reg N))` in the combine dump) to see how many orphans we already carry — that
   bounds how many more are needed and what shape they take here.
3. **Do NOT commit.** The owner runs the gate.
