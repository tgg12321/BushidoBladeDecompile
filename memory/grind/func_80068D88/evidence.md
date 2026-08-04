# Evidence bank — func_80068D88

## Session 1 (2026-08-03, recon modality) — FLOOR 5 -> 0

- **The honest floor is 0.** With session 0's `candidate.c` applied plus ONE
  change (transposing the `cur_init` / `prev_init` declarations),
  `sandbox func_80068D88 --disable all` reports `"score": 0`, 81 target insns /
  81 build insns, 0 rules dropped. Unmasked instruction diff against the
  oracle-matching reference object `build/src/text1b.o`: every opcode and every
  register identical; the only entries that differ are four branch/jump target
  ADDRESSES, which differ only because the sandbox object places the function at
  a different offset (exactly the entries `engine.score.normalized_insns`
  masks). No regfix rule, no asmfix rule, no `register asm()` pin, no `__asm__`,
  no volatile, no alias rename, no dead store, no unused local.

- **The score-5 residual was 5 register renames and nothing else.** Measured
  before any edit this session (session 0's form as-banked): idx 1 / 7 / 9 / 22 /
  24 differ only in which of $a1 / $a3 holds `prev_init` vs `cur_init`. Session
  0's `p_a2` live-range split had already made the instruction count, opcode
  sequence, scheduling and delay slots identical.

- **Mechanism, measured not guessed — GCC pseudo numbers follow DECLARATION
  order, and `global.c` breaks a symmetric tie on pseudo number.** From the
  `cc1 -da` `.greg` dump (`tmp/grind/func_80068D88/s1/dump/text1b.i.greg`, line
  41917): `;; 10 regs to allocate: 74 77 116 75 79 80 113 76 82 78`, with
  pseudos 79 and 80 carrying BYTE-IDENTICAL conflict sets
  (`74 75 76 77 78 79 80 2 3 29`) and receiving hard regs 5 ($a1) and 7 ($a3).
  The pseudo->local map (74=outer, 75=p_idx, 76=p_prev, 77=p_cur, 78=p_matrix,
  79=prev_init, 80=cur_init) is confirmed independently for each by the `addiu`
  displacement its register holds (+0x6E, +0x7C, +0x80, +0x8C), and it is
  exactly the source declaration order. `tools/gcc-2.7.2/global.c`
  `allocno_compare` scores priority as
  `floor_log2(n_refs)*n_refs/live_length * 10000 * size` and, on a tie, falls
  through to `return *v1 - *v2;` — lower allocno first — after which `find_reg`
  gives the first-allocated of two symmetric allocnos the lower free hard
  register. So the C source's declaration order, and nothing else, decides which
  of the two values lands in $a1.

- **Therefore: the session-0 ledger's stated mechanism was WRONG in its
  direction, and the correction is what unlocked the function.** Session 0
  recorded "global.c's allocator processes pseudos in conflict-degree order
  (most-constrained first); prev_init's live range is longer ... giving it a
  higher conflict degree, so it's processed first". The dump shows the two
  pseudos have *identical* conflict sets, so conflict degree cannot be the
  discriminator at all; the discriminator is the numeric tie-break on allocno
  index. This is why every session-0 lever aimed at shrinking or lengthening a
  LIVE RANGE (Lever A's block-local `prev_reload`, the `prev_tmp`/`cur_tmp`
  copies, the dead self-assign) measured as no-ops in the `.greg` dump — they
  were aimed at a mechanism that was not the operative one.

- **Cheap-probe lesson for the pipeline.** The closing lever cost one Edit and
  one sandbox run once the `.greg` dump had been read. Seven structural
  reorderings were tried blind across session 0 (several made the score WORSE by
  perturbing scheduling) before the dump was consulted for THIS specific
  question. Read the allocation dump and identify the actual discriminator
  BEFORE sweeping structural variants.

- **`(void)arg0; (void)arg1;` are load-bearing-free and inherited.** The
  function genuinely ignores both parameters (they are consumed by the eight
  thin wrappers at text1b.c:14986-15046 that call it with literal argument
  pairs); the casts are ordinary unused-parameter suppression, present since
  HEAD, and have no codegen effect.

- **The `p_a2` / `D_800A34E4` reload is IN THE TARGET BYTES, not a live-range
  device.** Target loop body: idx 42 `sw a0,0(gp)` publishes `D_800A34E4`,
  idx 45 `sw a1,0(gp)` publishes `D_800A34E8`, and then idx 52 `lw a0,0(gp)` /
  idx 53 `lw v1,0(gp)` RE-READ both globals for the second read-modify-write.
  The original C therefore genuinely re-read the globals rather than reusing the
  live locals; reusing `p_a`/`p_b` instead would delete two instructions the
  shipped binary contains. (Recorded because the in-session layer-1 reviewer
  challenged this construct as a redundant round-trip; the target bytes settle
  it. The reviewer reproduced the diff itself and formally RETRACTED that
  finding to PASS.)

- **DISPOSITION: the distance-0 form is CONTESTED and was NOT applied.** The
  layer-1 `cheat-reviewer` FAILed the declaration transposition, citing the
  narrowing clause of the 2026-07-17 owner ruling in
  `.claude/rules/param-local-alias-prologue-pair-flip.md` ("The declaration
  order is a freely tunable prologue-ordering knob; no exhaustion dossier can
  sanction it"). Reading that rule directly: the construct it archives is an
  ADDED literal-rename alias local (`Rect *_r = r; s32 *_out = out;`), and its
  own legitimate/forbidden line (L101-105) turns on whether the local carries
  NEW semantic information — which both `cur_init` and `prev_init` do. The scope
  question is genuinely ambiguous, so this session did NOT self-approve: it
  reverted `src/text1b.c` to the reviewer-cleared score-5 body (re-verified at
  score 5), banked the distance-0 form unapplied behind a CONTESTED banner in
  `memory/grind/func_80068D88/candidate.c`, and filed a ruling request as the
  2026-08-03 func_80068D88 entry in `docs/grind/decisions.md`. If the owner
  rules it acceptable the function is one Edit from COMPLETED-C; if forbidden,
  the next lever is `allocno_compare`'s PRIORITY term (`n_refs` /
  `live_length`) rather than its index tie-break — see hypotheses.md and the
  outcome frontier.

- WIP rejected_form: {'form': 'register asm("$7")/asm("$5") pins on prev_init/cur_init (inherited from HEAD)', 'score': 18, 'reason': 'cheat -- stripped by --disable all sandbox; matches .claude/rules register-asm-pins forbidden category. Confirmed: swapping which pin number goes to which variable produced IDENTICAL score (12/17 depending on other changes), proving the pins have zero effect on the honest score.'}

- WIP rejected_form: {'form': '__asm__ volatile("" ::: "memory") barriers between swap store and reload', 'score': 18, 'reason': 'cheat -- dead-branch-scheduling/CSE-defeat pattern, stripped by sandbox; confirmed via source-level diff of the sandboxed copy that these lines are deleted before compilation.'}

- WIP rejected_form: {'form': 'prev_tmp/cur_tmp fresh copy locals right before the swap store', 'score': 17, 'reason': 'no improvement over baseline; register choice unaffected'}

- WIP rejected_form: {'form': 'declare cur_init and compute it before prev_init (near top, right after outer/p_cur)', 'score': 8, 'reason': 'worse -- disturbs instruction scheduling order, not just register choice'}

- WIP rejected_form: {'form': 'move D_800A3724 store to after prev_init/cur_init computed', 'score': 7, 'reason': 'worse'}

- WIP rejected_form: {'form': 'declare p_cur first among the pointer locals (before p_idx/p_prev/p_matrix)', 'score': 7, 'reason': "worse -- shifts p_cur's own register choice away from target's a2"}

- WIP rejected_form: {'form': 'swap C statement order of *p_cur=prev_init / *p_prev=cur_init store pair', 'score': 12, 'reason': 'fixes the swap-section register choice but BREAKS the loop-section register choice (p_a/p_b) as a side effect -- net worse than the p_a/p_a2 split alone at 5'}

- WIP rejected_form: {'form': 'Lever A (register-alloc-pure-c): block-local `prev_reload = D_800A37D4;` at the swap-store site instead of reusing the shared prev_init pseudo', 'score': 5, 'reason': "no improvement -- confirmed via -da greg dump that the allocation order/registers were byte-identical to the un-shrunk baseline (75->a1, 80->a3 either way), i.e. this did not actually shrink prev_init's live range/conflict-degree the way the analogous saTan0Main switch-case lever did (that case had genuinely separate control-flow arms; this function is straight-line so the fresh read didn't decouple the conflict graph)"}

- WIP rejected_form: {'form': "Lever D-style dead self-assign `cur_init = cur_init;` immediately after its read, to try to boost cur_init's ref-count/priority above prev_init's", 'score': 5, 'reason': 'no improvement -- likely eliminated before reaching register allocation; NOT committed as a FAKE-annotated candidate since it produced zero measured effect (no lever-exhaustion justification to annotate a no-op)'}

- == imported from memory/wip notes.md ==
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

