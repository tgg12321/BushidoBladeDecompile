# s8 probes (manual, Claude, 2026-09-28)

All measured against src/text1b.c at main 2ee9ceb99..840a5f5af (project cc1
`tools/gcc-2.7.2/build/cc1`, build flags). "codediff 0" = the function's cc1 `.s`
is identical to the banked candidate's apart from the frame size and sp offsets.

| file | what | result |
|---|---|---|
| setxywh-bar1-neutral-41.c | bar 1's eight vertex stores written as PsyQ `setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w, -4 * row + 2)` (else arm `-2 * row + 1`); macro text defined locally | codediff 0 vs candidate (= 41). `2 - row * 4` costs +4 insns, `(row << 2)` +1 |
| zero-s16-descriptor-locals-vars120.c | `s16 xpos, ypos, semi = 0` read ONLY at the descriptor's out-of-loop x/y/semi stores (phase 1, phase-2 head, phase-4 head) | codediff 0, vars 96 -> 120 (+3 untouched slots, between recs and the bevel givs). HOLDER CLASS: refused (frame reservation); banked as mechanism evidence only |
| inloop-recs-index-190.c | bars index `recs[i + 1]` / `(&recs[i + 1])[row]` inside the row loop | 190/639 (sandbox). CSE folds i == 5 into the if-arm addresses |
| inloop-rec-assign-unhoisted.c | `rec = &recs[i + 1];` at the top of the row-loop body | not hoisted: the chain's first insn (i<<1, life 2, savings 2) is "not desirable" (116 < 269), so the whole chain stays in-loop |

`tools/` are copies of the scratch scripts (they expect to live in `tmp/c21c/`):
- `orph.py <cands...>` - splice, cpp, cc1 -dc; prints vars=, orphan `(use (reg))` count, codediff/posdiff vs `base`.
- `dump.sh <name> -dL|-ds|-df` - extra RTL dumps for a name orph.py compiled; `loops.sh` prints loop sizes + movables.
- `mkdbg.sh` + `patch_comb.py` - build a PRIVATE diagnostic cc1 in WSL `/tmp/gccdbg` that logs every combine and every orphan placement (BB2_COMB_DEBUG=1). Diagnostic only, never a build path; `comb.sh` checks its output is byte-identical to the build cc1 (it is).
- `sweep1..4.py` - the mutation sets described in evidence.md s8.
