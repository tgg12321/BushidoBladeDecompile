# func_80068D88 (text1b.c) — WIP checkpoint, 2026-07-06

## TL;DR
Floor moved 18 -> 5 (honest, cheat-free `sandbox --disable all`). HEAD's committed
form (from a much earlier session) relied on `register T x asm("$N")` pins plus
`__asm__ volatile("":::"memory")` scheduling barriers between a swap-store and its
immediate reload. Both are recognized cheat patterns: the sandbox's `--disable all`
mode literally deletes `asm("$N")` annotations and neutralizes the barrier
statements from the SOURCE before compiling for the honest score — that's why HEAD
scored 18 despite looking like a byte-identical "match" with the cheat intact. This
session removed both cheat forms entirely (candidate.c has zero `register`/`asm`
anywhere) and closed most of the resulting gap via one structural, non-cheat lever.

## The one lever that worked
The loop body reused a single pointer variable `p_a` for two unrelated purposes
(the matrix-entry pointer, then reloaded from D_800A34E4 for a second RMW). That
reuse gave `p_a`'s pseudo-register a long live range spanning both uses (and the
loop back-edge), which conflicted with far more of the function's other pseudos
than target's real register choice would suggest. Splitting the SECOND use into
its own nested-scope fresh local (`p_a2`) shrank that live range enough that GCC's
*unpinned* register allocator naturally landed on target's exact register choice
for the entire loop body (idx 38-58 in the objdump — all now byte-for-byte
identical, confirmed via `engine.score.normalized_insns` unmasked diff). This
closed 12 of the 13 non-branch-target instruction diffs.

## What's left: a genuine register-allocation tie (not a bug, not a cheat gap)
Only the swap-section register choice for `prev_init` (loaded from `D_800A37D4`)
vs `cur_init` (loaded from `*p_cur`) remains wrong: GCC's unpinned allocator puts
prev_init in $a1 and cur_init in $a3; target has them swapped. Traced via `-da`
RTL dumps at EVERY pass (rtl/cse/cse2/loop/greg/lreg/flow/jump/jump2/dbr/sched/
sched2) — when register pins ARE used, the RTL is correct (hard-bound to the
right registers) all the way through cc1's pipeline, matching target exactly.
Without pins, `global.c`'s allocator processes pseudos in conflict-degree order
(most-constrained first); prev_init's live range is longer (defined early, used
in the strength-reduction calc AND the swap store), giving it a higher conflict
degree than cur_init, so it's processed first and claims the lower-numbered free
register ($a1) — the reverse of target.

This is a **register-rename-only floor (0 structural diffs)** — the exact pattern
`.claude/rules/no-new-park-categories.md` calls out as NOT a valid park category.
The standing answer is more search (permuter / a different C shape), not pins.
See `meta.json.rejected_forms` for ~7 structural reorderings tried this session
(swap statement order, swap declaration order, defer/hoist the loads, reorder the
four pointer locals) — none closed it; several made things worse by disturbing
either instruction scheduling or the (already-correct) p_cur/p_idx/p_prev/p_matrix
register assignments.

## Resume instructions
1. Apply `candidate.c` to `src/text1b.c` (replaces the current func_80068D88 body).
2. Confirm floor: `sandbox func_80068D88 --disable all` should read `score: 5`.
3. Try `next_hypotheses` in `meta.json`, starting with the directed permuter —
   this is exactly the register-rename-plateau case it's meant for.
4. Do NOT reintroduce `register ... asm(...)` pins or `__asm__` barriers — both
   are stripped by the sandbox and were the root cause of the score-18 illusion.
