# func_80043454 — evidence (manual session 2026-09-25)

Fresh start: no prior C (pre-include-asm-body.c was a void placeholder).
Final: candidate.c = honest floor 0 (479/479), full build SHA1 == oracle
62efab4f73f992798c43e8c730aa43baa10bb4fa with it spliced into src/text1a_c.c.

## Measured progression (sandbox --disable all, scratch in tmp/f43454/)
| step | change | score |
|---|---|---|
| v1 | first structural C (arg0 s32, `while (count--)`) | 157 |
| v2 | arg0 `s16` (prototype at the forward decl changed s32->s16) + `while (--count != -1)` | 133 |
| v3 | skip branch `if (!packed) ... else switch (type) {0, 8, default}` | 118 |
| v4 | `packed = 1` before `mode = 2`; `SCRATCH_PTR + (count & 0x7FFF) + 1` | 115 |
| v7 | while-count loop spelled `while ((count = *SCRATCH_PTR++) != 0)` (was `for(;;incr)`) — removes the case-1/2 loop-bottom branch inversion | 115 (0 source-level hunks) |
| v13c | mode-1/2 counted loops as backward-goto loops + `s32 c3 = arg3; u8 d1 = arg1;` invariants | 7 |
| v14h5 | align-up spelled `(u8 *)p + (4 - ((u32)p & 3))` | **0** |

## Mechanisms (named, with cc1 source)
- **Loop-bottom `bne loop; j AEC` vs `beq AEC; j loop`**: reorg.c relax_delay_slots
  swaps a condjump+jump pair when mostly_true_jump(other) > 0, on each of the 2 reorg
  passes. With the outer loop a `for(;;incr)`, the continue label is followed by the
  increment, not NOTE_INSN_LOOP_VTOP, so the pass-2 swap-back does not fire. With
  `while ((count = *p++) != 0)` the continue label is the rotated test (VTOP right
  after it) -> mostly_true = 1 -> swapped back = target.
- **Register seats (count/case-0 local s2/s3, base/kind s4/s5)**: global.c
  allocno_compare priority = floor_log2(refs)*refs/live_length, refs weighted by
  flow.c loop_depth from LOOP_BEG/END notes (tmp/f43454/prio.py reproduces the
  observed allocation order exactly). With the mode-1/2 loops as while/do loops
  (depth 4) count=23077 > case-0 local 22727 and kind=17679 > base 16563 -> swapped
  seats. Backward-goto loops carry no LOOP notes (depth 3) -> count/kind drop below
  -> target seats. Because a goto loop gets no loop.c invariant motion, the
  per-iteration invariants the target keeps in s1/s0 ((s16)arg3, (u8)arg1) are
  written as locals before the loop (c3/d1).

## Kept constructs and their measured necessity
- mode-1/2 goto loops: while 115, do-while-in-if 113 (v15a), goto 0.
- c3 (`s32`): `s16 c3` = 21 (re-extends per call); no local = no hoist (v7b 102).
- forward decl `extern void func_80043454(s16, s16, s16, s16)` (was s32 first
  param, never checked — pre-include-asm-body.c says "do not trust it"); target
  sign-extends a0 at entry = an s16 parameter. Callers unaffected (oracle green).

## Layer-2 FAIL (2026-09-25) — goto chassis rejected
The 0-score body (rejected/goto-loop-chassis-s1-0.c) FAILED manual-lane layer-2:
backward-goto respelling of the mode-1/2 counted loops is the unsanctioned goto-loop
family (decisions.md 2026-09-07 func_8007526C owner ruling; special_camera_get_rot_dir
2026-08-26). The hand-hoisted c3/d1 locals fall with it. BANNED for this function:
goto-loop respelling of any counted loop, and invariant locals whose only role is to
replace loop.c motion lost to such a respelling. Cleared (reuse freely): s16 first
param in the forward decl, SCRATCH_PTR walk, `while ((count = *SCRATCH_PTR++) != 0)`,
align-up `(u8 *)p + (4 - ((u32)p & 3))`, packed/mode/kind structure, skip-branch switch.
Policy question logged: docs/grind/borderline.md 2026-09-25 func_80043454.

## Frontier (ordinary real loops)
Matching-build allocno priorities (tools/prio.py on the goto body; tools/dump.sh + tools/refs.py for per-insn weighted refs): case-0 local r162 22727 ->
s2, count 21758 -> s3, base 14986 -> s4, kind 12500 -> s5, type 12039 -> s6.
All-while body (rejected/while-loops-seat-swap-115.c; 109 with the align fix):
count 23077 (refs 105, live 273) > r162 22727 and kind 17679 (refs 66, live 224) >
base 16563 (refs 168, live 710). A closing ordinary spelling must drop count below
22727 (refs <=103 or live >=278) AND put kind below base but above type 12039 (kind
refs <=63, or live >=240, or base refs >=180). Final-code use counts pin refs, so the
lever is pre-RA pseudo structure / live ranges, not statement count.
Best ordinary so far: candidate.c = per-case counter `i = count; while (--i != -1)`
(57): kind 17679 -> s3, base -> s4, i 12500 -> s5, header/skip count -> a0.
Ruled out this session: explicit case-0 arg locals (234 / 146).

## 2026-09-26 manual session (slotA) — ordinary real-loop form reaches 0
candidate.c = all three counted loops as plain `while (--count != -1)` (no goto, no
c3/d1, no per-case counter) + mode-2's `switch (kind)` written one body per kind
(cases 0/1/2/3, 2 and 3 repeating 0 and 1) instead of `case 0: case 2:` / `case 1:
case 3:` label sharing. Sandbox 0/479 (tmp/f43454/score.py with the s16 forward-decl
substitution). Mechanism (prio.py on the dump): the duplicate bodies exist at flow
time and are re-merged by jump2 cross-jump after reload, so final bytes equal the
label-shared form, but they add live length inside the mode-2 loop and base refs:
count 105/302 = 20861 (< r160 22727), base 184/768 = 16771 (> kind), kind 66/253 =
15652 (> type 12039) -> count/base/kind = s3/s4/s5 = target. Same body with
label-shared mode-2 cases (tmp/func_80043454/w0.c) = 109.
- cc1psx calibration on the all-while w0 body: count in $18, kind in $20 — same
  seats as our cc1, so the residual was source shape, not compiler fidelity.
- Separate header variable (`nverts` for the stream header, count only for group
  counts) on the closing body: 13 (header value leaves s3) -> the target used one
  `count` variable for the header and the per-group counts.
- func_80052C10 declared noreturn (probe only): 122, wrong direction.
