# mario_getMarioVoiceData_8005BE84 — WIP (floor 4)

## TL;DR
Pure-C floor **4** (down from clean-C 21; HEAD cheat form scores 0 via asm("$16")/asm("$3") pins).
Resume by applying `candidate.c` to `src/text1b.c` (replace the function) AND keeping the data decl
`extern s16 D_8009AD1C[][2];` (already in candidate context). Confirm floor 4 with
`sandbox mario_getMarioVoiceData_8005BE84 --disable all`.

## The data
`D_8009AD1C` is an array of 4-byte entries `{s16, s16}` (use `s16 D_8009AD1C[][2]`). The function
reads `[arg0][0]` (the first s16) as a voice-id, returns it (or -1 if negative), after a sequence of
title/sound calls. `temp_a0 = (arg0<<1)+1` is passed to func_80085E4C twice.

## What's solved (the 4->0 is ONLY scheduling)
- Cached pointer `p` reused for all 3 `lh` reads (was recomputed: -5).
- `base = &D_8009AD1C[0][0]` computed BEFORE the index multiply -> GCC emits `addu s1,v0,v1` with
  v0=index, v1=base, matching target operand order (the score 8->4 win).

## The remaining 4 (the wall)
Target schedule:
```
lh v0,0(s1); nop; bltz v0,...; sll s0,s0,1   (shift arg0<<1 in the BLTZ DELAY SLOT)
...later... addiu a0,s0,1; sll a0,0x10; sra   (+1 and sign-extend LATE)
```
My candidate computes `temp_a0 = (arg0<<1)+1` and GCC schedules the whole shift+add EARLY (before
the lh), filling the bltz delay slot with the `addiu +1` instead. The original source used
`register s32 arg_save asm("$16")` to pin arg0 in s0 unshifted so the shift defers to the delay
slot. Every attempt to defer the shift in pure C (shift-in-place inside/outside the if) ALSO
disturbs the addu operand order or RA and scores WORSE (15-17). The shift-late and base-first
requirements appear mutually exclusive under GCC 2.7.2's scheduler without the pin.

## Next ideas
- decomp-permuter (scheduling/RA search) — this is exactly its niche; not yet run.
- Look for a sibling function in text1b.c with the same (arg<<1)+1-passed-late pattern that matched
  in pure C, and copy its shape.
