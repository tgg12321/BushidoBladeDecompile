# Evidence bank — func_80075670

## Session 1 (2026-08-11, recon) — CLOSED at sandbox 0

- Baseline: canonical verdict C (112 insns, pure-C target). Honest floor at
  session start: **20** (112/112 insns — pure register-rename diffs, zero
  structural diffs). 9 regfix rename rules carried
  (regfix.txt:2594-2608, `$7<->$3 @0-12`, substs @27/29/70/74,
  `$7<->$5`+`$7<->$6 @36-61` and `@75-101`).
- No duplicate/sibling lead (tmp/duplicates_leads.txt: no hit for 80075670).
- Diff decomposition (ours.txt vs asm/funcs/func_80075670.s):
  - Top half (idx 0,5,12,27,29): base pointer target `$v1`, ours `$a3`.
  - Bottom half (idx 37-101): 3-register rotation. Target
    (base, const2, const1) = ($a1, $a2, $a3); ours = ($a3, $a1, $a2).
    Both builds allocate const2-before-const1; the ONLY order difference is
    the base pointer: target allocates it FIRST (gets $a1), ours LAST.
  - Post-loop block (idx 68-79): target shares ONE `lw gp` reload between
    the read (`addu $v0,$a3,$s0`) and the write (`addu $a3,$a3,$v1`
    self-update); old C re-derefed the global for the read → 2 reloads.
- **Root cause (measured, closed the function):** the old C used one `base`
  local across the whole function. Its unioned live range (idx 0→101)
  gives it the lowest global.c allocno priority (refs/live-length), so it
  was allocated last → $a3, rotating everything else. Splitting into three
  pointer locals — `base` (top, ranges [0-13]∪[27-33], dead across both
  jals → caller-save OK → $v1), `work` (bottom, [37-101], short range +
  loop-weighted refs → allocated first → $a1), `q` (post-loop block only →
  fresh $a3 reload) — reproduced target allocation exactly.
  - Probe 1 (split base/work/q, q updated via `q = q + k` with separate
    global re-read): 20 → 4 (113 insns, +1 = extra `lw gp`).
  - Probe 2 (read AND write both through `q` in one statement:
    `*(u8 *)(q + ((arg1+1)&1) + 0x68) = (*(u8 *)(q + arg1 + 0x68) + 1) & 1;`):
    **0** (112/112). The write-address sum coalesces onto q's register
    (q dies at the sum) → `addu $a3,$a3,$v1` emitted naturally.
- Also removed pre-existing dead `p = p;` and empty `;` statements —
  match holds without them (they were debt, not load-bearing).
- Final body: src/text1b.c func_80075670 == memory/grind/func_80075670/candidate.c.
  Sandbox `--disable all`: score 0, rules_dropped 9.

## Artifacts
- tmp/grind/func_80075670/s1/ours.txt — baseline objdump (distance-20 build)
- tmp/grind/func_80075670/s1/ours2.txt — probe-1 objdump (distance-4 build)
