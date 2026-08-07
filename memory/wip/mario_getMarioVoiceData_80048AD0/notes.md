# mario_getMarioVoiceData_80048AD0 — WIP (grind2 lane, 2026-08-07)

## State: honest distance 0 (was 16). BLOCKED on an owner policy ruling — not applied.

Candidate: `tmp/cand/text1b_final.c` (47/47 insns, score 0). HEAD carries FOUR
`register asm("$N")` pins; the candidate removes all of them.

NB the NAME is census-SUSPECT (it loads BGM and initialises a sound table; it
does not fetch voice data). Do NOT rename — the naming campaign owns that.

## The closing form — two changes, mechanically inseparable

**(A) Declaration correction.** `src/text1b.c:596` declares
`extern s32 snd_LoadBgm(u8);`, but `src/sound.c:133` DEFINES
`s32 *snd_LoadBgm(s32 a0)`. The bogus `u8` parameter forces an
`andi $a0,$a0,0xff` truncation into the jal delay slot where target has `nop`.
(`src/text1b_b.c:65` carries the same wrong declaration.)

**(B) Index-form loop + reuse of `sound` as the record counter**, `/* FAKE */`
annotated — `for (sound = 0; sound < 0x11; sound++)` with the five per-record
stores written as `*(s16 *)(q - 8 + sound * 0x68) = sound;` etc. (full text in
the candidate), `s32 sound;` (was `u8`), `delta = (s32)(p - base);` before the loop.

Why the index form (16 -> 6): pointer-walking spellings make loop.c
strength-reduce spawn an EXTRA induction variable. The index form leaves exactly
the TWO walking pointers target has ($v1 = p at offset 0, $a1 = q = p+0xA with
negative displacements, p's advance filling the branch delay slot) — 47/47 insns
with every opcode, operand and offset equal; the whole residual was register
choice.

## Why the reuse (6 -> 1) — named mechanism, RTL-verified

From `tools/gcc-2.7.2/global.c` plus `-da` dumps (`tmp/rtl/g5`, `tmp/rtl/m1`):

- `set_preference` (:1671) — a PLUS's first operand has RTX format 'e', so
  `(set (reg 4 a0) (plus (reg delta) (const_int 1768)))` (the snd_PlayBgm
  argument) records preference {$a0} on **delta**.
- `expand_preferences` (:829) — merges preferences between the allocno SET by an
  insn and any allocno that DIES in it. `sound` is copied into $a0 for
  snd_LoadBgm, so whatever pseudo dies in `sound = D_80099BCC[...]` inherits {$a0}.
- `prune_preferences` (:882) — puts $a0 into `regs_someone_prefers[counter]`
  (contributed by lower-priority conflicting delta), so the higher-priority
  counter AVOIDS $a0 — unless it also prefers $a0, which equal allocno sizes
  subtract back out.

Measured, bodies otherwise identical:
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
| **non-reuse spellings WITH the prototype corrected (p1..p4)** | **6** |
| **reuse + corrected prototype (p5, p6)** | **0** |

p1..p4 is the decisive control: with (A) already applied, every non-reuse
spelling still plateaus at 6. The reuse is required, not decorative.

Scope: `tmp/scope_check_48AD0.py` — across all 259 functions in text1b.o only
the target function's instruction text changes.

## Layer-2 verdict: (B) CLEARED, (A) FAILED

The reviewer independently reproduced the exhaustion table and the p1..p6
control, and verified the global.c mechanism against the source and both dumps.

- **(B) CLEARS** the SOTN variable-reuse carve-out — exhaustion documented,
  mechanism named and verified, `/* FAKE */` annotated, layer-1 + layer-2 done.
- **(A) FAILS.** `.claude/rules/header-type-correction-from-use-sites.md`
  explicitly does NOT sanction width flips (u8 -> s32) or scalar -> pointer
  return changes. This edit is both, and the width half is load-bearing
  (reproduced: uncorrected + reuse = 1; corrected + reuse = 0). So it reads as a
  byte-motivated ABI change citing a rule whose text excludes it — even though
  the correction is independently TRUE. Lesser point: `src/text1b_b.c:65` is
  left uncorrected.

Both changes are needed for distance 0, so nothing can land until (A) resolves.

## BLOCKED — owner policy question (escalated; do NOT self-resolve)

> May a verified-true cross-TU prototype correction — where one TU's local
> `extern` contradicts the actual definition in another TU — close a byte gap,
> when the correction is a width flip plus a scalar->pointer return change that
> `header-type-correction-from-use-sites` textually excludes?

What makes it more than a coercion: the two declarations genuinely contradict
each other, which is a defect in OUR tree (hand-written during decompilation,
not something the original source could have had), and the target bytes agree
with sound.c's signature, not text1b.c's.

Reviewer's alternatives: (i) SOTN-evidence pack + a new sanctioned family,
(ii) a lever reaching andi-free codegen without a width/pointer-changing
declaration edit, (iii) surface to the user.

Shape of the (ii) wall, for whoever attempts it: keeping `u8 sound` for the call
is what avoids the `andi`, but the $a0 preference that fixes the counter/delta
allocation is only reachable by making the counter the pseudo that DIES in
`sound = D_80099BCC[idx]` — which forces it to double as the index, and target
keeps the index in $v0 (that combination is variant m1, score 2).

## Next step

Owner ruling on (A). If granted: build window -> apply both -> `retire` ->
`queue done` (SHA1 == oracle) -> `Match:` commit keeping the two claims
narratively separate per the reviewer, and fix `src/text1b_b.c:65` in the same
change.
