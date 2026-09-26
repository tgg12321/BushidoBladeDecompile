# Hypothesis ledger — single_game_SetStatusUpData

## [s2] manual lane slotB3 2026-09-26

- KILLED (s1 frontier 1): "drafts directly, floor well under 100" -- per-role draft is 102 (wf); the
  residual is register allocation, not transcription.
- CONFIRMED (s1 frontier 2): the byte-assembled mask word `(e[8]<<24)|(e[7]<<16)|(e[6]<<8)|e[5]`
  reproduces the or-tree as written.
- CONFIRMED (s1 frontier 3): the interleaved opp/self zero-init tail matches in statement order with
  `other` read once; the duplicated `p[0x440] = 0` is in the original.
- CONFIRMED: the target was compiled from ONE variable for five roles (see evidence.md s2 ablation);
  with it the body is 0/516 (wf). POLICY-BLOCKED: .claude/rules/ordinary-c-judge-decidable.md Ruling 1
  ("Multi-WRITE carriers remain banned ... a fresh local written more than once is admitted ONLY if it
  meets every prong of Ruling 5 ... 6 ... 8 ... 9 ... or 10"); the five writes feed different consumers
  (Ruling 5 1(a)), are different templates (1(b)), and any name is generic (1(f)); Ruling 6 needs a
  record pointer (C/D); Ruling 9 needs one meaning (a)/(b); Ruling 10 needs public original source.
  Same class as func_8002DE20's 2026-09-26 policy-question (a pair shared by 12 tests).
- KILLED (ordinary levers against the allocation gap, all measured): declaration order; per-role
  locals declared at function vs block scope; stat values assigned before vs inside the tests; u8
  stat/level types (a5: 146); m merged with the stat pair only (d1/d3); bonus as its own local vs
  staged through c; init orders of lo/hi1/hi2 (n2-n4); separate zero-loop counter (x1).
- OPEN: permuter campaign from candidate.c (honest per-role body) -- launched s2, see below.
- OPEN (landing prerequisites if the owner admits the scratch): (a) D_80099D88 aggregate merge at
  code6cac.h with func_80055948 respelled (measured 0/127) and the D_80099D8B row kept with the
  `alias of D_80099D88+3; retire with func_80058580` suffix; (b) cpu_practice_honmokuroku_data_tbl
  retype to `u8 [][4]` + code6cac_b.c:5026 consumer respelled -- byte-neutrality unmeasured;
  (c) the zero-loop / player-loop shared counter `i` (x1: 37 without it) needs the frozen
  "Variable reuse for codegen control" family + FAKE annotation, as owner-ruled for func_8003800C
  (decisions.md 2026-08-25, same `< 8U` / `< 2` signedness split); (d) the `c` staging is
  staged-value-reused-variable (c's real job is the 0x404 value) with FAKE annotation.
- DONE: permuter from the per-job body -- campaign 1 found `low_cat = cat < 2` (102 -> 35, kept);
  campaign 2 (from 35) found only constant carriers / offset-in-variable forms (29-32, banned
  shapes), stopped. Honest per-job floor 35/516.
- KILLED (mechanism, local-alloc.c combine_regs): a per-job variable used in one block is tied to
  the dying input of the insn that sets it, so the target's untied `andi a2,v0,0xff` (case-2
  level) and `or a2,v0,v1` (flag word) prove a variable live in other blocks. No per-job spelling
  reaches 0.
- OPEN (owner): docs/grind/borderline.md 2026-09-26 "func_80055138 — one scratch variable reused for
  five different values". The 0/516 body is full-build SHA1 == oracle with the header model.
- Siblings: func_80022F34 / func_80062020 / func_8005509C share no code block with this function
  (func_8005509C is the callee; its record-pointer spelling `(u8 *)&D_80101EC8 + arg0 * 0x44C` is
  used here). Nothing to transplant.
