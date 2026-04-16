# `tslPrintScreen` Codex Lab

This lab is isolated from the live `permuter/tslPrintScreen` workspace.

Purpose:
- try a more deliberate late-stage strategy
- keep notes separate from Claude's current workflow
- only promote changes back into `src/config.c` or the main permuter folder when
  they are clearly better

## Current Baseline

Starting point:
- copied from the best semantically sane Codex candidate, not the live source file
- recent live debug score before isolation: `785`

Current methodology:
1. preserve the cleanest known semantics
2. isolate only meaningful codegen nudges
3. rerun focused checks from this lab
4. only adopt results that remain readable and defensible

## Current Result

Best confirmed lab score:
- `660`

Best confirmed semantic improvement:
- replace the outer-loop `-2` path with a direct `goto past_marker`
- this removed one artificial tail live range and improved the score from `720`
  to `660`

Important result:
- the lower score did not come from permuter junk
- it came from a semantically defensible control-flow simplification

Promotion result:
- the live `src/config.c` version has now been promoted and linked successfully
- the repo matches after a real two-stage `regfix` promotion
- the only extra live-build work beyond the lab was remapping a few branch
  label targets from isolated-lab names to full-TU names

Rejected experiments:
- making the old `s0` explicit before loading the new pointer regressed badly
- removing the carried `new_var` alias and reloading `*(s16 *)s3` directly
  regressed badly
- replacing `flag == new_var5` with a direct `*((s16 *)s1) == new_var5`
  compare regressed badly
- changing the copy-block `t1` temp to `int` was neutral and not worth keeping

Current remaining diff profile at `660`:
- register differences: `20`
- reorderings: `1`
- insertions: `2`
- deletions: `3`

Interpretation:
- the function is now solidly in compiler-behavior territory
- the next likely wins are declaration/live-range shaping or a small eventual
  `regfix`, not broad semantic rewrites

## Regfix Spike

Separate lab-only spike files:
- [regfix_spike.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/codex_lab/tslPrintScreen/regfix_spike.md>)
- [regfix_spike.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/codex_lab/tslPrintScreen/regfix_spike.txt>)
- [regfix_spike_stage2.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/codex_lab/tslPrintScreen/regfix_spike_stage2.txt>)
- [test_regfix_spike.py](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/codex_lab/tslPrintScreen/test_regfix_spike.py>)

Best current spike result:
- lab-only two-stage regfix now matches `target.o` exactly

Takeaway:
- yes, `regfix` was worth pursuing here
- the marker-scan tail was the last blocker
- a small second-stage label-aware pass finished the lab
- promotion needed one extra label-remap pass because the full TU used different
  local labels than the isolated lab object

## Hot Regions

1. Early sentinel/token setup
- the `lh` / `lhu` pairing and the carried token are still slightly off in the diff

2. `func_80052C10` neighborhood
- target carries extra live values around the call
- the source may need a shape that induces those live ranges without fake code

3. `s2` setup
- `s2`, previous `s0`, next `s0`, and the `0x18` / `0x14` stores are allocator
  sensitive

4. Post-`func_8003FA24` block
- grouping this block already produced a large score improvement
- this is a strong candidate for manual perm search

5. Marker scan tail
- branch-delay and load timing are still not quite aligned

## Promotion Rule

Promotion outcome:
- the final result is now live in `src/config.c`
- `regfix.txt` carries the first-stage cleanup
- `regfix_stage2.txt` carries the label-aware second-stage cleanup
