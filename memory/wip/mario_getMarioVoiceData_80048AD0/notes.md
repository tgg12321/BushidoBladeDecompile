# mario_getMarioVoiceData_80048AD0 — WIP (grind2 lane, 2026-08-07)

## State: honest distance 0 is REACHABLE but BLOCKED. Two layer-2 FAILs, two reverts.

Function is back at queue top, unchanged, oracle green. Candidate body:
`tmp/cand/text1b_final.c`. HEAD carries FOUR `register asm("$N")` pins that the
candidate removes.

NB the NAME is census-SUSPECT (it loads BGM and initialises a sound table; it
does not fetch voice data). Do NOT rename — the naming campaign owns that.

## The closing form — two changes, measured INSEPARABLE

**(A) Declaration change.** `src/text1b.c:596` + `src/text1b_b.c:65` declare
`extern s32 snd_LoadBgm(u8);`; `src/sound.c:133` DEFINES `s32 *snd_LoadBgm(s32)`.
The `u8` parameter forces `andi $a0,$a0,0xff` into the jal delay slot where
target has `nop`.

**(B) Index-form loop + reuse of `sound` as the record counter**, `/* FAKE */`
annotated — `for (sound = 0; sound < 0x11; sound++)` with the five per-record
stores as `*(s16 *)(q - 8 + sound * 0x68) = sound;` etc., `s32 sound;` (was
`u8`), `delta = (s32)(p - base);` before the loop.

Why the index form (16 -> 6): pointer-walking spellings make loop.c
strength-reduce spawn an EXTRA induction variable. The index form leaves exactly
the TWO walking pointers target has ($v1 = p at offset 0, $a1 = q = p+0xA with
negative displacements, p's advance filling the branch delay slot) — 47/47 insns
with every opcode, operand and offset equal; the residual was register choice.

## Why the reuse (6 -> 1) — mechanism, RTL-verified, CONFIRMED by two reviewers

From `tools/gcc-2.7.2/global.c` plus `-da` dumps (`tmp/rtl/g5`, `tmp/rtl/m1`):

- `set_preference` (:1671) — a PLUS's first operand has RTX format 'e', so
  `(set (reg 4 a0) (plus (reg delta) (const_int 1768)))` (the snd_PlayBgm
  argument) records preference {$a0} on **delta**.
- `expand_preferences` (:829) — merges preferences between the allocno SET by an
  insn and any allocno that DIES in it. `sound` is copied into $a0 for
  snd_LoadBgm, so the pseudo dying in `sound = D_80099BCC[...]` inherits {$a0}.
- `prune_preferences` (:882) — puts $a0 into `regs_someone_prefers[counter]`
  (from lower-priority conflicting delta), so the higher-priority counter AVOIDS
  $a0 — unless it also prefers $a0, which equal allocno sizes subtract back out.

```
separate counter:            ;; 79 conflicts: ... (no preferences line) -> i=$a2, delta=$a0
counter carries preference:  ;; 79 preferences: 4                       -> i=$a0, delta=$a2  (target)
```

## Lever exhaustion (~60 variants, `tmp/sweep2_48AD0.py` .. `sweep9_48AD0.py`)

| family | best |
|---|---|
| pointer-walk loop shapes (n0..n5, g1..g4) | 15 |
| struct-array record loop (s1..s4) | 14 (45 insns) |
| all 48 declaration/statement orderings | 6 (insensitive) |
| call-arg respellings to break delta's preference (k2..k7) | 6 |
| allocno-structure: drop `q`, reuse `temp_v0` (m2..m4) | 13 |
| anonymous / flat index temps (n1..n5) | 2 |
| **non-reuse spellings WITH (A) applied (p1..p4)** | **6** |
| **reuse + (A) (p5, p6)** | **0** |
| reuse WITHOUT (A) | 1 |

Scope: `tmp/scope_check_48AD0.py` — across all 259 functions in text1b.o only
the target function's instruction text changes.

## Layer-2: (B) CLEARS on its merits. (A) FAILED TWICE. Both attempts reverted.

Attempt 1 — (A)+(B) bundled, (A) justified via
`.claude/rules/header-type-correction-from-use-sites.md`. FAIL: that rule's text
explicitly excludes width flips (u8 -> s32) and scalar -> pointer return changes.

Attempt 2 — (A) split into its OWN commit (f035516b), re-justified purely as a
declaration-correctness bug plus the measured fact that it is BYTE-NEUTRAL alone
(oracle unchanged with (A) applied and the old pinned body in place). Applied,
oracle verified green, `queue done` accepted COMPLETED-C. FAIL again, revert
ordered and done: a5b0b6a4 reverts f035516b; body change and queue mutation
rolled back; oracle re-verified green. Grounds:

- The re-split is **cosmetic re-adjudication**, not new substance — no new
  use-site semantics or cross-consumer evidence between submissions.
- ZERO use sites in the tree positively REQUIRE the new type. The one live call
  passes a `u8` local needing no conversion under either declaration — which is
  precisely WHY (A) is byte-neutral alone. Byte-neutrality is evidence of
  *inertness*, not of independent motivation.
- (A) is load-bearing ONLY as the counterpart removing the truncation that (B)'s
  own widening of `sound` to s32 introduces — no standalone motivation.
- (B)'s mechanism was RE-VERIFIED as genuine and would likely clear the SOTN
  carve-out alone, but is inseparable from (A).

**Do NOT re-split, re-word or re-file (A). Refused twice on substance.**

## Next lever — path (ii), UNEXPLORED (reviewer's words), not exhausted

Reach the andi-free codegen WITHOUT any width/pointer-changing declaration edit.
Shape of the wall: keeping `u8 sound` for the call is what avoids the `andi`,
but the $a0 preference that fixes the counter/delta allocation is only reachable
by making the counter the pseudo that DIES in `sound = D_80099BCC[idx]` — which
forces it to double as the index, and target keeps the index in $v0 (that
combination is variant m1, score 2). Un-tried: routing a $a0 copy-preference to
the counter through some OTHER dying-pseudo/SET pair, or removing delta's own
$a0 preference by changing what feeds snd_PlayBgm's argument without adding an
instruction (k2..k7 covered only same-shape respellings).

Otherwise the remaining paths are the reviewer's (i) a SOTN-evidence pack for a
new sanctioned family covering width/pointer declaration corrections backed by
an in-tree definition, or (iii) an owner ruling. (iii) is escalated and open.
