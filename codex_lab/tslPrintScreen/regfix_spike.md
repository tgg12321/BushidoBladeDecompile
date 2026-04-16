# `tslPrintScreen` Regfix Spike

This spike is deliberately separate from the main build:
- no edits to `regfix.txt`
- no edits to `tools/regfix.py`
- no edits to `src/config.c`

## Goal

Test whether the current `660` lab baseline is close enough that a *small*,
targeted `regfix` can carry it further.

This is not meant to justify a giant opaque patch. The spike only targets:
- early `v1`/`a0` token register mismatches
- the `func_80052C10` live-range neighborhood
- the old-`s0` / new-`s0` carry through the setup block

The ugly tail block is intentionally left mostly untouched in the first pass.

## Files

- `regfix_spike.txt`: candidate lab-only directives
- `regfix_spike_stage2.txt`: second-stage label-aware cleanup applied on top of
  the first spike
- `test_regfix_spike.py`: compile + apply lab regfix + compare against
  `target.o`

## First-Pass Hypothesis

The current `660` source already made one strong source-shape improvement.
What remains looks like:
- a small early register selection mismatch
- one live-range shaping mismatch around `func_80052C10`
- one carry mismatch where the target briefly keeps old `s0` in `a2`

If those are real late-stage mismatches, a small lab regfix should improve the
object diff materially. If it does not, that is evidence that the function is
still too source-shape-wrong for regfix to be the next step.

## Current Result

Stage 1 result:
- raw object diff reduced from `20` differing words to `6`

Stage 2 result:
- exact object match against `target.o`

What Stage 2 added:
- a tiny label-aware cleanup pass over the marker-scan tail
- direct `.L27` targeting once the tail entry label was pinned correctly
- `.L9+4` / `.L26+8` style targeting where the existing labels were attached to
  the instruction group just before the desired branch destination

Interpretation:
- the function is close enough that `regfix` can finish it cleanly
- the final obstacle really was label attachment in the tail, not missing logic
- this is now a viable candidate for promotion into the real build once you want
  to fold the lab result back into the live workflow

Promotion note:
- promotion into the real build succeeded
- the only extra step needed in the live repo was remapping a few label
  references from isolated-lab names like `.L7` / `.L9` / `.L26` to the
  corresponding full-TU labels used in `src/config.c`'s compiled output
