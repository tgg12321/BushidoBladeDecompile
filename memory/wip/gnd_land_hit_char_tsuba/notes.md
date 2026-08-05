# gnd_land_hit_char_tsuba — WIP (current state 2026-08-05)

`src/text1b.c:12751`. HEAD baseline: honest pure-C distance **65**, 88 rules, canonical
ASM-SUSPECT. Measure: `wsl bash tmp/csz/gn_score.sh` (driver `tmp/csz/gn_var.py`),
`wsl bash tmp/csz/frame.sh gnd_land_hit_char_tsuba text1b`.

## Candidates (banked as diffs; tree left clean)

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
- `64` — a live 4-byte local holding a pointer, spilled and reloaded. **LANDED** (below).
- `68..79` — 12 bytes **no instruction on either side touches**: phantom slots
  ([[phantom-slot-frame-lever]]). **This is the whole remaining delta.**

## What landed, and why (65 → 40)

**The first +8 came from an exact RA spec: `refs+1` on pseudo 85.** `tools/ra_solver`
reproduces our allocation 16/16 exactly. Pseudo 85 (`p_b388`, `REG_EQUIV symbol_ref
D_8009B388`) was the only unallocated allocno, priority 159
(`floor_log2(refs)*refs*size/livelen*10000`), so one more reference takes it to 425, past 86
(412) and 87 (315) — evicting 87 (`base_offset`, no constant equivalence) to a **real** spill
slot, i.e. the `64(sp)` local. `perturb.py` over 157 single atoms returns `pseudo 85: refs+1`
as the **ONLY** solution. Ref arithmetic (def at depth 1 + each in-loop use at depth 2) checks
out on all four allocnos, so **an in-loop use adds 2 and the needed +1 must land at loop depth
1** — which is why the round-2 in-loop spelling was wrong. The landing spelling precomputes
half 2's pointer outside the loop:
```c
p_b388 = &D_8009B388;
p_b390 = p_b388 + 2;      /* depth-1 ref -> nrefs 3 -> 4, pri 159 -> 425 */
```

**The +2 instruction gap then closed via split-init accumulation.** Target folds each `rN - K`
into an accumulator held across the call (`addu a2,a2,v0`); we emitted
`addiu v0,v0,-K; addu v0,sX,v0`. Per [[split-init-accumulation-sanctioned]]:
```c
a2_offset = (s32)r4 - 0x19;
a2_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
```
Sweep (`tmp/csz/gn_split.py <variant>`, driver `tmp/csz/gn_sweep.sh`) is monotone in the number
of statements split: none 56/178, 2nd-half a2 49/178, 2nd-half a0 51/177, 2nd-half both 44/177,
+1st-half a2 43/177, +1st-half a0 41/176, **all four 40/176**. This **supersedes the round-4
negative** ("all four → 68/178"), which was measured on the pre-round-5 source.

**If this lands it needs the family's `/* FAKE */` annotation** — not yet added, since nothing
is being committed.

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
- **Pointer reassociation is inert** (`p_b2e0 + stride + 0xC` → `p_b2e0 + 0xC + stride`): 65 →
  65. GCC canonicalizes the PLUS chain.
- **Def placement cannot buy refs.** Moving `p_b388 = &D_8009B388;` inside the `do/while` is
  inert — `loop.c` hoists it back to the preheader; the extracted model is byte-identical.
- **The exec_game identity/holder levers do NOT transfer** (frame stayed 48 in every variant):
  inlining the `c100`/`c1` holders as literals scores 67 (they are load-bearing here), and
  splitting `a0_offset`/`a2_offset` per call-half is inert — GCC coalesces the split because
  the two ranges do not overlap.
- **Phantom producer #1 (folded guard compare) is unreachable here.** Three spellings naming
  the entry guard `((D_800A326C + 1) * 2) > 0` on top of candidate_40 (`tmp/csz/gn_phantom.py`,
  driver `tmp/csz/gn_ph_run.sh`) are **all inert**: 40 / 176 / vars 56. Target's entry guard is
  a **bare `blez $v1`** (`asm/funcs/gnd_land_hit_char_tsuba.s:53`) and the loop condition
  `slt $v1,$s2,$v1; bnez` (161-162) — a `blez` needs no compare pseudo, so a named local has
  nothing to strand. (Target's `D_800A326C` traffic: prologue read, store at 22, guard re-read
  at 49, loop-condition re-read at 157, `+= 1` read/store at 165/168 — our C already matches.)
- **Phantom producer #2 (combine orphan-USE) has no site.** Orphan census
  (`tmp/csz/gn_orphan.sh`): `.greg` dispositions run 72-152 with **86 and 88 absent**, i.e. we
  already carry exactly two unallocated pseudos (`p_b2e0`, `p_b390`); combine emits **zero**
  bare `(use (reg N))` anywhere in the function, consistent with the arithmetic being all
  s32/u32 with no HImode intermediate to strand.

## Next

1. **Producers #1 and #2 are ruled out; #3 is what remains** — live named locals on multi-read
   values, at zero instruction cost. `D_800A3418` is NOT the candidate it looks like: each
   `^=` update is followed by a single use, so it is not multi-read per segment. Look for a
   value the function reads more than once between updates.
2. Also test whether giving pseudo 88 (`p_b390`) a clean `REG_EQUIV symbol_ref D_8009B390`
   changes the slot accounting — target rematerialises `&D_8009B390` as `lui`/`addiu` (align
   idx 106-107), so ours materialising it differently may be why our two orphans pay fewer
   bytes than target's phantom block.
3. **Do NOT commit.** The owner runs the gate.
