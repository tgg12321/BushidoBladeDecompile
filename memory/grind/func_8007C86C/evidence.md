# Evidence bank — func_8007C86C

- WIP rejected_form: {'form': "Round 14 W1 (2026-06-06): dispatch test swap — `if (D_8009BE74 != 1)` replacing `if ((u32)(D_8009BE74 - 1) >= 2U)` (semantically equivalent for u8 D_8009BE74 with values 0/1/2/3 since only ==1 maps to wide-mode in target's branching).", 'score': 23, 'score_note': 'build_insns 48 (3 short). Same magnitude as round-10 T2 mode_m1 hoist.', 'reason': "Regression (+11). The `!= 1` form encodes the dispatch as a direct equality test, generating different RTL than the unsigned `>= 2U` form which uses a subtract+compare; the equality form's RTL shifts the dispatch branch's INSN_PRIORITY downward and the surrounding allocation cascade follows. Same regression family as the mode_m1 hoist. (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 W2 (2026-06-06): dispatch-outer Y-clamp-inner restructure — move the entire Y-clamp INSIDE each dispatch arm (duplicate Y-clamp code in both narrow and wide branches; dispatch becomes the outer if/else).', 'score': 37, 'score_note': 'build_insns 64 (+13 vs target 51). Worst regression among W1-W5.', 'reason': 'Severe regression (+25). Duplicating Y-clamp into both dispatch arms compounds the cascade: each arm now has 6-7 insns of Y-clamp logic AND the dispatch boundary creates extra cross-block register lifetime extensions. The build_insns jump from 50→64 (+14) is the duplicated clamp blocks. Confirmed structurally different from any code-shrinking lever — code expansion compounds the cascade rather than shifting allocation. (Round 14, 2026-06-06.)'}

- WIP rejected_form: {'form': 'Round 14 W3 (2026-06-06): fused mask-shift in narrow arm — `var_v1 = (var_a1 & 0x3FF) << 0xA;` instead of two-step `var_v1 = var_a1 & 0x3FF; var_v1 = var_v1 << 0xA;`.', 'score': 14, 'score_note': 'build_insns 50 (matches candidate, +2 vs score). Minor regression.', 'reason': "Regression (+2). The two-step form is load-bearing for floor 12. Combine canonicalizes the fused form to a single RTL insn `(set var_v1 (ashift (and var_a1 0x3FF) 0xA))`, but cc1's combine then re-splits into `andi`+`sll` at insn-emission time with different pseudo lifetimes than the manual two-step form. The intermediate `(and var_a1 0x3FF)` pseudo has a SHORTER livelen in the fused form, shifting allocation by 2 cascading diffs. (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 W4 (2026-06-06): Y-clamp branch-sense flip — `if (var_a1 < 0) var_a1 = 0; else if ((D_8009BE7A - 1) < var_a1) var_a1 = D_8009BE7A - 1;` instead of bgez-first form.', 'score': 17, 'score_note': 'build_insns 50 (matches candidate, +5 vs score).', 'reason': "Regression (+5). bltz-first vs bgez-first Y-clamp variant. Same regression family as round-3 v29 X-clamp inverted-if (score 22). The bltz-first form emits the comparison via a different RTL chain (sign-check feeds different downstream consumers); cc1's combine eliminates redundant tests but the resulting allocation order shifts var_a1's lifetime earlier into the dispatch boundary, regressing 5 points. (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 W5 (2026-06-06): X-clamp branch-sense flip mirror — `if (arg0 < 0) var_v0_2 = 0; else if ((D_8009BE78 - 1) < arg0) var_v0_2 = D_8009BE78 - 1; else var_v0_2 = arg0;`', 'score': 16, 'score_note': 'build_insns 50 (matches candidate, +4 vs score).', 'reason': "Regression (+4). Mirror of W4 applied to X-clamp. The bltz-first X-clamp form changes arg0's sign-check RTL chain in a way that affects the X-preserve pseudo's livelen and priority; surrounding cascade regresses 4 points. (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 directed permuter output-95-1 (~3000 iter, 8min wallclock at HEAD d8c647d8): UB-conditional-init — reads `var_v0_2` and `new_var` BEFORE they are assigned on any reachable path through their first read.', 'score': None, 'reason': "FORBIDDEN UB-conditional-init cheat-by-spelling. The C source places `var_v0 = var_v0_2 & 0xFFF;` at function-top BEFORE var_v0_2 is initialized (overwritten in both dispatch arms but the function-top read is dead UB on every path). Additionally `int new_var; ... new_var = 0; var_v0_2 = new_var; ... var_a1 = new_var;` — the `var_a1 = new_var;` line on (arg0 >= 0, var_a1 < 0) path READS new_var WITHOUT IT BEING WRITTEN (new_var only set in the arg0<0 branch). Same family as session-3's permuter findings, rejected_forms 'Initial var_v0_2 = arg0 before the if/else clamp' (score 20) and 'UB conditional-init patterns from session 3's permuter'. Vetted FAIL on cheat-reviewer tests #1 (no semantic purpose), #2 (human would not read uninitialized locals), #5 (matches known cheat family). (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 directed permuter output-100-1: `var_a1 = 0;` inside `arg0 < 0` else arm — SEMANTIC CHANGE (zeros Y on any negative-X call regardless of arg1).', 'score': None, 'reason': "FORBIDDEN semantic-change cheat-by-spelling. The function's specification clamps X and Y INDEPENDENTLY. This form makes Y depend on X's sign (when X < 0, Y is zeroed instead of clamping arg1). Same family as C7A0 round-13 S1 finding (forbidden 2026-06-06 per round-13 commit d8c647d8). Vetted FAIL on cheat-reviewer tests #1 (changes program behavior), #2 (no human would zero Y based on X's sign), #5 (matches forbidden semantic-change family). (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 directed permuter output-160-1: `var_a1 = 0;` inside `arg0 >= 0` THEN branch — SEMANTIC CHANGE (zeros Y on any in-range positive-X call regardless of arg1).', 'score': None, 'reason': "FORBIDDEN semantic-change cheat-by-spelling. Different arm than output-100-1 but identical intent: makes Y dependent on X's value when X is in [0, D-1]. Vetted FAIL on cheat-reviewer same tests. (Round 14, 2026-06-06.)"}

- WIP rejected_form: {'form': 'Round 14 directed permuter output-170-1: `var_a1 = 0;` inside X-clamp positive-in-range else arm (same as output-160-1) + `var_v0 ^ 0` identity in return.', 'score': None, 'reason': 'FORBIDDEN: combined semantic-change cheat (var_a1 = 0 in arg0 in-range arm, same intent as output-160-1) + identity-operation cheat (`var_v0 ^ 0` adds zero RTL value, no semantic purpose, exists only to perturb codegen). Two cheats compounded. Vetted FAIL. (Round 14, 2026-06-06.)'}

- WIP rejected_form: {'form': 'Round 14 directed permuter output-175-1: UB-conditional-init read of `new_var2` in wide-mode (else) branch — `new_var = new_var2 & 0xFFF;` reads new_var2 WITHOUT IT BEING ASSIGNED on the wide path (only narrow branch sets `new_var2 = var_v0_2;`).', 'score': None, 'reason': 'FORBIDDEN UB-conditional-init cheat-by-spelling. The wide-mode arm reads `new_var2 & 0xFFF` where `new_var2` is declared as `s16` with no initialization and only assigned inside the narrow branch (taken when D_8009BE74 != 1). On the wide-mode path (D_8009BE74 == 1), new_var2 is uninitialized at the time it is read. Classic permuter UB attractor. Vetted FAIL same tests. (Round 14, 2026-06-06.)'}

- WIP rejected_form: {'form': 'Round 10 T1 (v2 Lever 2 block-local precompute): move `var_v1 = var_a1 & 0xFFF;` UNCONDITIONAL precompute from before dispatch INTO the else (wide-mode) arm only — single-set pseudo confined to wide arm; narrow arm unchanged.', 'score': 18, 'score_note': "build_insns 51 matches target's 51 (vs candidate's 50). EXACT magnitude match with C7A0 round-9 T1 (+6).", 'reason': "Regression (+6 vs candidate). The shortened var_v1 livelen widens the masked-Lev cascade rather than narrowing it; the precompute hoisted ABOVE dispatch was load-bearing for the score-12 floor's specific X-preserve assignment pattern. Disproves the SOTN-duplicate-read-can-be-narrower hypothesis from v2 research. Cross-port: C7A0 round-9 T1 = 18 (+6), C86C round-10 T1 = 18 (+6) — IDENTICAL. (Workflow round 10, 2026-06-05, git_head 07635825.)"}

- WIP rejected_form: {'form': 'Round 10 T2 (v2 Lever 1 mode hoist): declare `s32 mode_m1 = D_8009BE74 - 1;` at function entry, use `mode_m1` in dispatch `if ((u32)mode_m1 >= 2U)` instead of inline `D_8009BE74 - 1`.', 'score': 23, 'score_note': 'build_insns 48 (3 short vs target 51). EXACT magnitude match with C7A0 round-9 T2 (+11).', 'reason': "Regression (+11 vs candidate). GCC CSEs mode_m1 back to the inline form AND the byte-global early read shifts allocation in the wrong direction (build_insns drops to 48). The pseudo-priority rebalance the v2 mechanism predicted doesn't materialize. Cross-port: C7A0 round-9 T2 = 23 (+11), C86C round-10 T2 = 23 (+11) — IDENTICAL. (Workflow round 10, 2026-06-05, git_head 07635825.)"}

- WIP rejected_form: {'form': 'Round 10 T3 (v2 Lever 5 arg1_neg sign hoist): `s32 arg1_neg = arg1 < 0;` precomputed at entry, then Y-clamp predicate uses `if (!arg1_neg)` instead of `if (var_a1 >= 0)`.', 'score': 28, 'score_note': 'build_insns 51 matches target. EXACT magnitude match with C7A0 round-9 T3 (+16).', 'reason': "Severe regression (+16 vs candidate). The sign-extend pseudo target.s emits at idx 14-15 is NOT synthesizable via a comparison-result hoist — the comparison RTL is materially different from the sign-extend insn pair target emits, and target's pseudo dies at the bltz check while the hoisted `arg1_neg` survives across the X-clamp. Cross-port: C7A0 round-9 T3 = 28 (+16), C86C round-10 T3 = 28 (+16) — IDENTICAL. (Workflow round 10, 2026-06-05, git_head 07635825.)"}

- WIP rejected_form: {'form': 'Round 10 T4 (v2 Lever 4 pointer share): `s16 *limits = &D_8009BE78;` at entry, then `limits[0] - 1` (X-limit) and `limits[1] - 1` (Y-limit) instead of direct globals.', 'score': 18, 'score_note': 'build_insns 50 matches candidate. EXACT magnitude match with C7A0 round-9 T4 (+6).', 'reason': "Regression (+6 vs candidate). CSE collapses both `limits[0]` and `limits[1]` LO16 references back to the same shape as direct global access, but the introduced pointer pseudo's livelen costs the masked-Lev cascade. Same outcome as round-7 T1's lim_x save — the pointer-arithmetic vs scalar-arithmetic distinction the v2 lever predicted is invisible to cc1's RTL after combine. Cross-port: C7A0 round-9 T4 = 18 (+6), C86C round-10 T4 = 18 (+6) — IDENTICAL. (Workflow round 10, 2026-06-05, git_head 07635825.)"}

- WIP rejected_form: {'form': 'Round 10 T5 (v2 Lever 3 combined): apply Lever 1 (mode_m1 hoist) + Lever 2 (var_v1 precompute in else only) + drop r_e4 named local (`return var_v1 | var_v0 | 0xE4000000;`) simultaneously.', 'score': 29, 'score_note': 'build_insns 48 (3 short). EXACT magnitude match with C7A0 round-9 T5 (+17).', 'reason': "Severe regression (+17 vs candidate). The individually-negative levers (T2 +11, T1 +6) compound their regressions rather than cancelling. Per session-5/8 ALLOCDBG findings cited in the v2 research, fewer pseudos CAN flip the sort, but only if the removed pseudos were the ones biasing AGAINST target's choice — in this case they were biasing TOWARD target's choice. Cross-port: C7A0 round-9 T5 = 29 (+17), C86C round-10 T5 = 29 (+17) — IDENTICAL. (Workflow round 10, 2026-06-05, git_head 07635825.)"}

- WIP rejected_form: {'form': 'N1 (session 7, 2026-06-04): `s32 mode_minus_one = D_8009BE74 - 1;` block-local before dispatch', 'score': 12, 'reason': 'Flat. Mode-flag block-local has zero priority effect — combine eliminates the named intermediate before allocno priority calc; equivalent to direct inline read.'}

- WIP rejected_form: {'form': 'N2 (session 7, 2026-06-04): drop `r_e4` named intermediate, return `var_v1 | var_v0 | 0xE4000000` directly', 'score': 12, 'reason': 'Flat. OR-chain associativity collapse via combine canonicalization is allocation-neutral; r_e4 not load-bearing for floor 12.'}

- WIP rejected_form: {'form': 'N3 (session 7, 2026-06-04): unsigned-comparison combined sign+limit on both X and Y clamps (`if ((u32)arg0 < (u32)D_8009BE78) ... else if (arg0 < 0) ... else ...`)', 'score': 37, 'reason': 'WORST regression (12 -> 37). Branchless-equivalent encoding via single unsigned compare moves codegen to entirely different shape — emits sltu + nested branch tree, scheduling cascade through dispatch region.'}

- WIP rejected_form: {'form': 'N4 (session 7, 2026-06-04): `s32 mode = D_8009BE74;` UNCONDITIONALLY at function TOP (extended-lifetime variant of N1)', 'score': 20, 'reason': "Regression (12 -> 20). Top-level read extends mode's pseudo lifetime across entire clamp region; bumps its allocno priority enough to shift another pseudo's allocation, regressing 8 points."}

- WIP rejected_form: {'form': 'Permuter output-130-1 (session 7, 2026-06-04): `var_a1 = arg1;` unconditionally at function top + `var_a1 = 0;` overwrite in arg0<0 else branch (permuter weighted score 130 vs base 225)', 'score': None, 'reason': "FORBIDDEN cheat-by-spelling (dead conditional store on arg0<0 path). Same family as func_8007B844's directed-permuter find (forbidden 2026-06-01 per [[no-new-park-categories]]). Vetted per 6-test checklist: dead-store-on-one-path = YES, human-natural = NO, GCC-internals-justification = YES, permuter-found-only = YES."}

- WIP rejected_form: {'form': '(not tested directly here; same s16 raw_arg0 = arg0 literal-rename chain documented for C7A0)', 'score': 10, 'reason': 'Same FORBIDDEN form as memory/wip/func_8007C7A0/meta.json rejected_forms[0] — not re-tested this session because the verdict is the same.'}

- WIP rejected_form: {'form': "UB conditional-init patterns from session 3's permuter (~5400 iters)", 'score': None, 'reason': 'Every closing form the directed permuter found used UB-conditional-init (a local read before any path writes it). Forbidden cheat-by-spelling per .claude/rules/no-new-park-categories.md. See memory/project/func-8007c86c-permuter-ub-cheat-findings.md.'}

- WIP rejected_form: {'form': 'Initial var_v0_2 = arg0 before the if/else clamp (early-init + conditional overwrite)', 'score': 20, 'reason': 'Regression AND is borderline cheat-by-spelling — the initial store is dead on two of three paths (arg0<0 overwrites with 0, arg0>limit-1 overwrites with limit-1) and only used on the fall-through path. Same shape family as the UB-conditional-init permuter rejected in session 3. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Inverted goto-merge for X clamp (`if arg0<0 {v=0; goto post_x;} if limit<arg0 {v=limit; goto post_x;} v=arg0; post_x:`)', 'score': 16, 'reason': "Regression. Explicit goto-merge doesn't induce cc1 to emit a real park-merge insn; the merge label gets coalesced by jump-thread/CSE and the cascade gets WORSE (16 vs 12). (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'X-mask precompute (`var_v0 = var_v0_2 & 0xFFF;` before dispatch, narrow-mode overwrites)', 'score': 14, 'reason': "Mirror of the Y wide-mask SOTN duplicate-read lever doesn't translate to X. The X-precompute happens between Y-precompute and the mode-dispatch and disturbs the bnez delay slot scheduling — narrow-arm needs the overwrite, wide-arm uses the precompute, but the combined emission is 2 insns worse than the bare candidate. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 's16 xlim block-local in arg0 clamp (`s16 xlim = D_8009BE78 - 1; if (xlim < arg0) ...`)', 'score': 20, 'reason': "Same regression family as session 2's documented xlim test (which scored 25 from clean v8 base). Block-local adds an extra sign-extension insn for xlim that target doesn't have. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 's16 ylim block-local in arg1 clamp', 'score': 21, 'reason': 'Same regression family — extra sign-extension insn. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'OR-chain reordering `return var_v0 | 0xE4000000 | var_v1`', 'score': 13, 'reason': "Slight regression (12 -> 13). cc1's combine canonicalises associativity differently from this source layout; the `(var_v1 | (var_v0 | 0xE4000000))` form (= r_e4 named-intermediate form) is the minimum. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v1 (round 2): operand-flip on X clamp comparison (`if (arg0 > (D_8009BE78 - 1))` per compare-operand-order-register.md)', 'score': 12, 'reason': "Flat — score 12 unchanged. The compare-operand-order lever works for sibling-call wrappers where LHS is a local and target's allocation flows from RTL-emission order, but here the operand swap is folded by combine before allocno priority kicks in. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v2 (round 2): operand-flip on BOTH X and Y comparisons', 'score': 12, 'reason': 'Flat — same combine fold as v1. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v3 (round 2): var_a1 = arg1 preload moved to AFTER X-clamp (instead of at function top)', 'score': 12, 'reason': "Flat — arg1's pseudo lifetime is bounded by its uses regardless of preload timing; cc1's pseudo creation order is RTL-emission order, not declaration order. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v4 (round 2): param mutation — write clamp result to arg0 itself (instead of var_v0_2)', 'score': 16, 'reason': "REGRESSION — does NOT flip X-preserve register to $a3 (ALLOCDBG confirms pseudo 72 still gets $a2). Borderline cheat-by-spelling (parameter mutation as result-holder) but irrelevant to the lever question. Differs from rejected_forms[2]'s var_v0_2 = arg0 initial-overwrite cheat. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v5 (round 2): s16 lim_x = D_8009BE78 block-local in X-clamp (read full halfword once, use twice as lim_x - 1)', 'score': 12, 'reason': 'Flat — block-local lim_x folded back to inline form by combine because s16 forces sign-extension that combine eliminates. Distinct from prior xlim test (which scored 20 — held the -1 form). (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v6 (round 2): s32 lim_x = D_8009BE78 block-local (full halfword as s32 to avoid sign-extension) — THE FIRST KNOWN LEVER that flips X-preserve pseudo to $a3', 'score': 18, 'reason': "REGRESSION but WITH MECHANISM EVIDENCE — IS the first known lever that flips X-preserve pseudo to target's $a3 (ALLOCDBG confirms pseudo 76 → reg 7 at ord=8 instead of candidate's pseudo 72 → reg 6 at ord=10). Also produces target's missing idx-16 `move a3, v0` park-merge. COST: lim_x's lifetime introduces extra early `move $3, $4` preserve and shifts pseudo 77 to $a1 (target $a2), netting +6 cascade diffs. THIS IS THE NEXT-SESSION BASELINE FOR DIRECTED PERMUTER. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v7 (round 2): s32 lim_x AND s32 lim_y block-local (both clamps)', 'score': 21, 'reason': "REGRESSION. lim_y mirrors v6's effect on Y-clamp slot but compounds the cascade. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v8 (round 2): var_v0_2 s32 retype (vs s16) — eliminates one sign-extension', 'score': 17, 'reason': 'REGRESSION. Changes upper-clamp result width which propagates through mask operations differently from target. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v9 (round 2): Y-clamp BEFORE X-clamp (change pseudo creation order)', 'score': 24, 'reason': "REGRESSION. Early Y-clamp lengthens arg0's lifetime, exacerbating the preserve cascade. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'v10 (round 2): declare var_v0_2 LAST (after var_v0, var_v1) — declaration order swap', 'score': 12, 'reason': 'Flat — confirms ALLOCDBG finding that pseudo numbers for autos follow RTL-emission order, not C declaration order. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v11 (round 2): no var_a1 preload — Y-clamp writes directly to arg1 parameter (in-place clamp)', 'score': 12, 'reason': 'Flat — arg1 is read after writes (mask), so this is NOT the forbidden Lever D pattern. Same allocation as candidate. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v12 (round 2): OR-chain swap `return (var_v0 | 0xE4000000) | var_v1`', 'score': 13, 'reason': 'Minor regression. cc1 canonicalises OR associativity differently from this source layout. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v13 (round 2): drop precomputed `var_v1 = var_a1 & 0xFFF` and inline mask+shift in each dispatch arm', 'score': 16, 'reason': 'REGRESSION. Removes the SOTN duplicate-read precompute lever proven necessary (session 4 evidence). (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v14 (round 2): no widemask precompute + var_a1 & 0xFFF inside narrow else only', 'score': 18, 'reason': 'REGRESSION. Same as v13 effectively — drops the duplicate-read benefit. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v15 (round 2): decl swap (var_a1 before var_v0_2)', 'score': 12, 'reason': 'Flat — same as v10, decl order does not affect cc1 pseudo numbering. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': "v16 (round 2): s32 limit_minus_one = D_8009BE78 - 1 named intermediate (vs v6's lim_x which holds full value)", 'score': 18, 'reason': 'REGRESSION — same effect family as v6 (the -1 form vs the full form behaves identically wrt allocno priority). (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v17 (round 2): v6 (lim_x) + no widemask precompute', 'score': 23, 'reason': 'WORST REGRESSION. Two simultaneous lever interventions cancel/compound badly. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'v18 (round 3): lim_x compare-only + var_v0_2 = D_8009BE78 - 1 (re-read global) in assignment', 'score': 14, 'reason': "REGRESSION 12 -> 14 with build_insns dropping to 49. Combine eliminates one of the duplicate reads but the resulting allocation is worse than candidate (lim_x's pseudo gets a brief lifetime, X-preserve picks wrong reg). (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v19 (round 3): s32 lim_x = (s32)D_8009BE78 (explicit cast variant of v6)', 'score': 18, 'reason': "Flat 18 — same as v6. Explicit (s32) cast doesn't change combine behavior on the lim_x load. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v20 (round 3): s32 lim_x = D_8009BE78 - 1 used twice (pre-subtract)', 'score': 18, 'reason': "Flat 18 — same as v6 family. Pre-subtracting the -1 into lim_x doesn't change priority calculation; cc1 emits the same allocation as v6. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v21 (round 3): s32 lim_x at function scope (outer decl), assigned inside if-branch only', 'score': 18, 'reason': "Flat 18 — same as v6. cc1's pseudo creation order is RTL-emission order, not decl scope; outer decl with conditional assignment behaves identically to inner-scope decl. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v22 (round 3): v6 + var_a1 = arg1 deferred to inside both branches of the X-clamp', 'score': 18, 'reason': "Flat 18 — same as v6. Deferring var_a1's preload position doesn't shift arg1's pseudo lifetime in a way that helps. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v23 (round 3): s16 lim_x = D_8009BE78 (narrow type variant of v6)', 'score': 12, 'reason': "Flat 12 — folds back to candidate's score. s16 forces sign-extension that combine eliminates, collapsing lim_x to inline form. (Workflow round 3, 2026-06-03, confirms rejected_forms v5 entry from round 2.)"}

- WIP rejected_form: {'form': 'v24 (round 3): s32 lim_x AND s32 lim_y symmetric in both clamps', 'score': 21, 'reason': "REGRESSION 18 -> 21. lim_y mirrors v6's effect on Y-clamp slot and compounds the cascade. Matches rejected_forms v7 from round 2. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v25 (round 3): s32 lim_x at function scope, UNCONDITIONALLY loaded at top (vs v21 conditional assignment)', 'score': 22, 'reason': "REGRESSION 18 -> 22. Unconditional load of D_8009BE78 outside the arg0>=0 branch forces lim_x's lifetime to extend across the entire function, severely disturbing allocation of arg1/var_a1 and the Y-clamp slot. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v26 (round 3): s32 lim_x = D_8009BE78 - 1, compared as `lim_x < arg0`, used as var_v0_2 = lim_x (no -1 needed since pre-subtracted)', 'score': 18, 'reason': "Flat 18 — same as v6 family. Equivalent to v20 but with single-use lim_x; folds to v6's allocation pattern. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v27 (round 3): flip branch direction — primary case `arg0 < lim_x` -> var_v0_2 = arg0, else lim_x - 1', 'score': 20, 'reason': "REGRESSION 18 -> 20, build_insns drops to 48. The flipped comparison induces combine to drop a sign-compare, but the resulting branch layout doesn't match target. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'v28 (round 3): s32 lim_x AND s32 lim_x_m1 = lim_x - 1 as separate locals (forced split)', 'score': 18, 'reason': 'Flat 18 — same as v6. The two-local form just emits the same combine-folded code as v6 (lim_x_m1 folds into use sites). (Workflow round 3, 2026-06-03.)'}

- WIP rejected_form: {'form': 'v29 (round 3): inverted if structure — `if (arg0 < 0) { var_v0_2 = 0; } else { lim_x ... }`', 'score': 22, 'reason': "REGRESSION 18 -> 22. Inverting the outer if doesn't change the resulting decision-tree codegen (cc1 applies branch-sense canonicalization), but the source-order change disturbs scheduling. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'Round 15 V_A (2026-06-08): per-arm explicit return, inline OR-tree, no var_v0/var_v1 fall-through merge — `if ((u32)(D_8009BE74 - 1) >= 2U) return (((s32)var_a1 & 0x3FF) << 0xA) | ((s32)var_v0_2 & 0x3FF) | 0xE4000000; return (((s32)var_a1 & 0xFFF) << 0xC) | ((s32)var_v0_2 & 0xFFF) | 0xE4000000;`', 'score': 22, 'score_note': 'build_insns 50 (matches candidate). +10 vs candidate.', 'reason': 'Regression (+10). Each-arm-returns form materializes `lui $at, 0xE400` twice and forces separate epilogue paths. The shared post-dispatch r_e4 + var_v0/var_v1 merge block from candidate is load-bearing for floor 12. (Round 15, 2026-06-08.)'}

- WIP rejected_form: {'form': "Round 15 V_B (2026-06-08): switch (D_8009BE74) { case 1: { var_v1 <<= 0xC; var_v0 = var_v0_2 & 0xFFF; break; } default: { var_v1 = var_a1 & 0x3FF; var_v1 <<= 0xA; var_v0 = var_v0_2 & 0x3FF; break; } } — case-1-first switch form (NOT same as round 14 W1's != 1 if-form).", 'score': 24, 'score_note': 'build_insns 48 (3 short vs target 51). +12 vs candidate.', 'reason': 'Regression (+12). Switch on u8 D_8009BE74 emits direct branch tree, dropping 3 insns BUT cascading allocation in wrong direction. Same regression family as W1 (== 1 / != 1 direct-equality forms). (Round 15, 2026-06-08.)'}

- WIP rejected_form: {'form': 'Round 15 V_C (2026-06-08): per-arm explicit return with r_e4 = var_v0 | 0xE4000000 computed INSIDE each arm (no shared post-dispatch merge block).', 'score': 18, 'score_note': 'build_insns 50 (matches candidate). +6 vs candidate.', 'reason': "Regression (+6). Closer than V_A but per-arm r_e4 still costs +6. The shared r_e4 computation in candidate's post-dispatch block is load-bearing — duplicating the OR-tree compound in each arm shifts allocation. (Round 15, 2026-06-08.)"}

- WIP rejected_form: {'form': 'Round 15 V_D (2026-06-08): 4-arm Y-clamp on arg1 directly (no `var_a1 = arg1` preload — var_a1 lifetime starts inside Y-clamp arms): `if (arg1 >= 0) { if ((D_8009BE7A - 1) < arg1) var_a1 = D_8009BE7A - 1; else var_a1 = arg1; } else var_a1 = 0;`', 'score': 12, 'score_note': 'build_insns 50 (matches candidate). FLAT — no progress, no regression.', 'reason': "Flat. Combine eliminates the absence-of-preload as equivalent to candidate's preload form; matches round 2 v11 (in-place arg1 clamp = 12 flat). var_a1's pseudo lifetime ends up identical to candidate's despite the source-form difference. (Round 15, 2026-06-08.)"}

- WIP rejected_form: {'form': "Round 15 V_E (2026-06-08): branch-sense flip with arm swap — `if (D_8009BE74 == 1) { wide-mode body } else { narrow-mode body }`. Distinct from W1 (`!= 1` test, default-body in then) because both the test direction AND the arm placement are flipped relative to candidate's `>= 2U` form.", 'score': 24, 'score_note': 'build_insns 48 (3 short vs target 51). +12 vs candidate.', 'reason': "Regression (+12). Same magnitude family as V_B (switch) — direct equality test against constant 1 produces the same RTL as the case-1-first switch. The reduction in branch-test insn count (3 fewer) doesn't compensate for allocation cascade. (Round 15, 2026-06-08.)"}

- WIP rejected_form: {'form': "Round 15 V_F (2026-06-08): combo of round 2 v6's `s32 lim_x = D_8009BE78` block-local + V_D's no-preload arg1 Y-clamp.", 'score': 18, 'score_note': "build_insns 49. +6 vs candidate (= v6's cost).", 'reason': "Regression (+6). V_D's no-preload form is allocation-neutral; combo matches v6's score-18 floor. Disproves the hypothesis that V_D might offset v6's `move $3, $4` cascade — they're independent allocation effects. (Round 15, 2026-06-08.)"}

- WIP rejected_form: {'form': 'Round 15 directed permuter output-90-1 (best score, ~44k iters, HEAD 49e869c0): `var_a1 = 0;` inside `arg0 < 0` else arm of X-clamp — SEMANTIC CHANGE (zeros Y when X<0 regardless of arg1).', 'score': None, 'reason': "FORBIDDEN semantic-change cheat-by-spelling. EXACT same shape as round 14 output-100-1 and C7A0 round-13 S1. Convergent attractor across distinct directed-permuter macro surfaces. Vetted FAIL on cheat-reviewer tests #1 (changes program behavior), #2 (no human would zero Y based on X's sign), #5 (matches forbidden semantic-change family documented across rounds 14 / C7A0 13). (Round 15, 2026-06-08.)"}

- WIP rejected_form: {'form': "Round 15 directed permuter output-125-1: UB-CONDITIONAL-INIT — `var_a1 = arg1;` assigned ONLY in X-clamp's else (arg0<0) arm; `var_a1` is READ on the arg0>=0 path (Y-clamp `if (var_a1 >= 0)`) where it has never been assigned. Plus wraps the dispatch+return in `if (1) { ... }` — no semantic purpose, only shifts cc1 RTL.", 'score': None, 'reason': 'FORBIDDEN UB-conditional-init cheat-by-spelling (compounded with `if (1)` wrapper cheat). Vetted FAIL on cheat-reviewer tests #1 (reads uninitialized local on a real execution path), #2 (no human would conditionally initialize, then unconditionally read), #5 (matches forbidden UB-conditional-init family — round 14 output-95-1, session 3 permuter findings, round 14 output-175-1). The `if (1)` wrapper is independently a no-semantic-purpose cheat. (Round 15, 2026-06-08.)'}

- WIP rejected_form: {'form': "Round 15 directed permuter output-165-1: `var_v0_2 = arg0;` re-assignment INSIDE narrow-mode (then) arm of the dispatch block — SEMANTIC CHANGE (when arg0<0, var_v0_2 was clamped to 0; this re-assigns the unclamped negative arg0, then `& 0x3FF` masks negative bits to non-zero). Plus a `dummy_label_303033:` dead label inside the Y-clamp's inner if (dead-goto label-pad).", 'score': None, 'reason': 'FORBIDDEN: two cheats compounded — (a) semantic-change re-assignment of var_v0_2 in dispatch arm (same family as round 14 output-100-1/160-1 semantic-changes of var_a1, just on a different intermediate), (b) dead-goto label-pad (`[[global-label-drift-sibling-cheat]]` archived dead-goto pad family). Vetted FAIL on tests #1, #2, #5. (Round 15, 2026-06-08.)'}

- WIP rejected_form: {'form': 'Round 15 directed permuter output-165-2: TWO cheats compounded — (a) `var_a1 = (D_8009BE7A ^ 0) - 1;` identity-op cheat (`^ 0` adds zero RTL semantic value, only perturbs codegen — same family as round 14 output-170-1 `var_v0 ^ 0`); (b) `var_v0_2 = arg0;` re-assignment INSIDE wide-mode (else) arm of dispatch (different arm than 165-1, identical cheat intent — semantic change of var_v0_2).', 'score': None, 'reason': 'FORBIDDEN double cheat (identity-op + semantic-change). Same convergent attractor family as round 14 output-170-1 and round 15 output-165-1. Vetted FAIL on tests #1, #2, #5. (Round 15, 2026-06-08.)'}

- WIP rejected_form: {'form': 'Round 15 directed permuter output-175-1: UB-CONDITIONAL-INIT — `s16 new_var;` declared in dispatch block, assigned ONLY in narrow-mode (then) arm (`new_var = var_v0_2;`), READ in wide-mode (else) arm (`var_v0 = new_var & 0xFFF;`) where it has never been assigned. Identical shape to round 14 output-175-1 (s16 new_var2 read uninitialized in wide arm).', 'score': None, 'reason': 'FORBIDDEN UB-conditional-init cheat-by-spelling. EXACT same shape as round 14 output-175-1 — convergent attractor across distinct directed-permuter macro surfaces. Vetted FAIL on tests #1, #2, #5. (Round 15, 2026-06-08.)'}

- == imported from memory/wip notes.md ==
# func_8007C86C — WIP (current state 2026-08-04, round 15)

GP0 drawing-area packet builder (`0xE4000000`): clamps x to `[0, D_8009BE78-1]`
and y to `[0, D_8009BE7A-1]`, then packs them into a GP0 word at 10/10 or 12/12
bits depending on `D_8009BE74`. Twin of `func_8007C7A0` (`0xE3000000`, constant
differs only) — solve one, apply to both. 21 regfix substs at
`regfix.txt:3050-3071`; 0 asmfix; no prologue_config / frame_fix entries.

## Where it stands

| body | score | build_insns | stream vs target (51) |
|---|---|---|---|
| committed HEAD | 20 | 50 | frame wrong (vars=24 vs 16) |
| `candidate.c` (rounds 1-14) | **12** | 50 | park insn MISSING |
| `candidate_frame51.c` (round 15) | 13 | **51** | **1:1 in kind and order** |

`candidate_frame51.c` is the structurally closest form in 15 rounds: all 51
instructions match target in kind and order, and **every remaining difference
is a register NAME** — ours→target: carrier `a0`→`a3`, limit-save `v1`→`a2`,
arm temp `a0`→`v0`, tail constant `v0`→`a0`. It costs +1 on the masked-
Levenshtein metric (13 vs 12) only because that metric counts renames.

The lever that closed the long-standing "MISSING park insn" gap is a staged
carrier plus a join copy:

```c
s32 x = arg0;            /* carrier, live across the clamp */
s16 tx;
if (arg0 >= 0) {
    if ((D_8009BE78 - 1) < arg0) tx = D_8009BE78 - 1; else tx = x;
} else tx = 0;
x = tx;                  /* the join copy = target's `move a3,v0` */
```

## Frame is NOT an open problem (correction, 2026-08-04)

The frame-mismatch census flagged this function as reserving 8 bytes too many
(delta −8). That measured the **committed HEAD body**. `candidate.c` already
emits `frame=16 / vars=16` — correct — via its `s16 var_v0_2 / s16 var_a1`
locals. There was never an inverse-phantom problem here; HEAD is just an older,
worse shape. Correction recorded in `memory/wip/_frame_census_2026-08-04.md`.

## The remaining gap — mechanism (round 11, instrumented cc1)

`$a2`-vs-`$a3` X-preserve tiebreaker. ALLOCDBG on the candidate body: pseudo 72
(the X-preserve backup) has priority **1818**, the LOWEST in the 11-pseudo
allocno sort (nrefs=2, livelen=11). Allocation walks ascending and stops at the
first free register, so it lands in `$a2`; target has it in `$a3`. Flipping it
requires some other pseudo to own `$a2` across pseudo 72's live range with
higher priority — the **chain-extension** mechanism, FORBIDDEN per
[[register-alloc-pure-c]] §6. Corroborated empirically by every round since.

## Do NOT re-run (measured negative / inert)

- **Permuter** — ~125k cumulative iters across 4 independent runs (rounds 1, 2,
  12 random; round 13 directed PERM_GENERAL with 5 hand-authored spots). Every
  sub-baseline candidate is a forbidden family: dead-conditional-store,
  semantic change (zeroing Y on the arg0<0 path), UB (uninitialized `var_a1` /
  `var_v1` reads), synthetic shared-zero routing, named-intermediate-for-bool.
- **Clamp shape** — if/else, ternary, assign-first, nested, goto-form, switch
  dispatch, unsigned single-compare, `arg0 >= D` compare: 14-27.
- **Arm ordering** — `<0`-first regresses to 16-28 (rounds 4, 6, 15).
- **Carrier / param forms** — SOTN param-reassignment (16-28), local copy of y
  (+4 over param-in-place), staged-temp sourced from `arg0` instead of the
  carrier (20), s32 signature widening (24).
- **Types** — u16/u32 on the clamp locals and casts at mask sites: inert
  (combine erases the distinction under the 0x3FF/0xFFF masks). s32 widening
  of `var_v0_2`: 17.
- **Named intermediates** — un-decremented limit local (inert), `mode_m1`
  hoist (23), `arg1_neg` sign hoist (28), limits-pointer share (18).
- **Declaration order** — all permutations inert (pseudo numbers for autos do
  not track decl order).
- **Tail OR association** — `hi|(lo|C)`, staged `lo|C`, `(lo|C)|hi`: inert or +1.
- **Precomputes** — narrow-mask X precompute (18), identity precompute (22),
  single-expression return (22-24).
- **TU re-attribution / rodata reorder** — independently FAILED by
  cheat-reviewer 2026-06-05 (commit `2e5098e9`): all four
  [[no-new-park-categories]] evidence criteria unmet; the globals are already
  display.c-exclusive and the 21 rules are a codegen signature, not a layout one.

## Build gate

Applying either candidate breaks the oracle (score-13 form → SHA1 `b4122d17`)
because the 21 regfix substs are calibrated to HEAD's emission shape. Landing
one requires rewriting or retiring those rules — a completion-gate activity,
not a worker one. src was reverted after every experiment and the full build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Round 16 (2026-08-04) — stream-exact form, verified on this twin

`candidate_stream51.c` (banked here, twin-adapted from C7A0) is the first body
whose 51 instructions match target one-for-one in opcode, operand shape and
order. Measured on THIS function: 51/51 insns, `sandbox --disable all` score 15
— identical to C7A0, confirming the twins move in lock-step. The score is higher
than the old 12/13 only because masked Levenshtein counts renames; those forms
carried real structural errors and this one does not.

Two levers: an **`s16` carrier** (target's `move a3,a0` copies the raw halfword
before sign-extension, and the join copy carries no extension) and a **named
tail temp** (`pkt = lo | 0xE4000000; return hi | pkt;`) to pin the OR
association GCC otherwise reassociates.

## Resume here

Start from **`candidate_stream51.c`**. The stream is done; only the register
assignment is open, and round 17 CLOSED the pure-C route to it. An exhaustive
273-atom scan of the RA model leaves exactly three perturbations that reach the
gating `$a2` placement, and all three need a copy relationship with hard `$v1`
or hard `$a2` — registers a two-parameter leaf function never has in scope. The
one route not needing a new hard reg (`hi` conflicting with the limit-save) is
disproven from target's own bytes: target writes `$v1` for the first time at
insn 39 while the limit-save dies at insn 13, so they cannot conflict in the
original's compilation either. **Do not spend more rounds on C spellings for the
allocation.** Full mechanism, the measured negative list (including the round-17
variable-reuse family, 5/5 stream-breaking), and the probe tooling are in
`memory/wip/func_8007C7A0/notes.md`; per-round narrative for rounds 1-14 is in
git history for this file.

Sibling: `memory/wip/func_8007C7A0/` — identical pattern and floor.


## s1 (2026-08-10, RECON, git HEAD 2d6270da) — floor 12 -> 5 via the sibling's chassis; residual proven identical to the sibling's banned join-temp family; disposition is coupled to C7A0's pending owner ruling

- **Provenance (from the task brief, new since the old WIP ledger):**
  func_8007C86C = `get_ce` (static), verbatim-linked Sony PsyQ 4.0 LIBGPU SYS
  module code (census 2026-07-09, 100% of non-reloc-masked bits). Twin
  func_8007C7A0 = `get_cs`. The old WIP rounds 1-16 never had this fact.
- **Baseline at HEAD 2d6270da:** canonical verdict C (distance 20, target 51);
  sandbox --disable all on the committed HEAD body = 20 @ 51 build insns,
  21 rules dropped, cheat-asm stripped 154.
- **Sibling chassis measured (H1):** func_8007C7A0's floor-5 candidate.c
  (clamp both params in place, per-arm returns with block-scoped hi/lo,
  wide-first `< 2U` dispatch) with 0xE4000000: **sandbox 5 @ 50** — exact
  lock-step with the sibling. New floor 5 (old ledger floor 12; the old
  12-form is superseded, preserved in rejected/superseded-12form-staged-precompute.c).
- **Residual verified from bytes (H2):** side-by-side in
  tmp/grind/func_8007C86C/s1/ (ours_5form_50.txt vs target_51.s). 47/51
  target insns reproduced 1:1 (prologue, both limit loads, full Y clamp,
  dispatch with the wide-mask andi in the branch delay slot, cross-jumped
  OR tail). The ONLY divergence: target's X-clamp three-arm join
  (`addiu v0,a2,-1` / `move v0,a3` / `move v0,zero` then `move a3,v0`
  writeback at .L8007C8AC) vs ours writing $a3 directly per arm. This is
  BYTE-FOR-BYTE the sibling's residual.
- **Family-ban inheritance:** the sibling's ledger proves (s7/s8/s12 P2
  mechanism theorem, 19 structural kills s7+s12+s13, ~118k permuter iters
  s14) that every C dataflow materializing this join routes a temp back into
  a live-initialized variable — layer-1-banned under any spelling (x/tx
  alias FAIL 14:36, ternary self-read FAIL 16:10, docs/grind/decisions.md).
  The Judge ESCALATED the sibling's ruling request 17:31 and it now sits as
  a pending OWNER-ESCALATION (decisions.md 2026-08-10). By twin-equivalence
  (same module, same bytes modulo the constant, same residual) the owner's
  ruling there governs this function identically.
- **What this session deliberately did NOT do:** re-derive the sibling's
  exhaustion packet on this function (the C7A0 frontier explicitly directed
  "apply candidate.c with the constant swapped; do not spend this function's
  sessions on it"), re-measure the published SOTN get_cs/get_ce reference
  (measured on the sibling: different library build, constant clamps — loses),
  or re-run permuter/structural sweeps already dead on the shared chassis.
- **Disposition:** floor 5 banked; src/display.c REVERTED to HEAD after
  measurement (the 21 regfix rules at regfix.txt:3050-3071 stay calibrated
  to HEAD's emission; applying the candidate without retiring them breaks
  the oracle — same build gate as the old WIP notes documented).

- [s1] Baseline at HEAD 2d6270da: canonical verdict C (distance 20, target 51); HEAD body sandbox 20 @ 51, 21 rules dropped, cheat-asm stripped 154

- [s1] Sibling 5-chassis with 0xE4000000: sandbox 5 @ 50, 21 rules dropped, cheat-asm stripped 153 — new floor 5 (was 12); old 12-form preserved in rejected/superseded-12form-staged-precompute.c

- [s1] Residual byte-verified identical to func_8007C7A0's: X-clamp three-arm $v0 join + move a3,v0 writeback; every C spelling reaching it is layer-1-banned on the sibling (x/tx alias, temp->param writeback, published-SOTN ternary self-read) under the any-spelling family rule

- [s1] func_8007C7A0 sits as a pending OWNER-ESCALATION (docs/grind/decisions.md 2026-08-10, Judge ESCALATE 17:31) on exactly this residual; by twin-equivalence the owner's ruling governs func_8007C86C identically

- [s1] src/display.c reverted to HEAD after measurement — the 21 regfix rules stay calibrated to HEAD's emission shape; applying the candidate without retiring them breaks the oracle

- [s1] Published SOTN get_cs/get_ce reference NOT re-measured here: measured on the sibling, it is a different library build (constant clamps vs BB2's halfword globals) and loses

## 2026-08-10 owner ruling — RESOLVED (no new grind session)

- [ruling] docs/grind/decisions.md, "2026-08-10 — OWNER RULING (in person) —
  func_8007C7A0 + twin func_8007C86C: ternary clamp ban LIFTED": the layer-1
  ternary-family ban is lifted for func_8007C7A0 and this twin ONLY, as a
  single-instance grant on provenance-plus-measurement grounds. It creates no
  family; the join-temp-writeback ban stays in force everywhere else.
- [ruling condition 4] This function's body was measured INDEPENDENTLY rather
  than assumed symmetric with the sibling. Constants and arm shapes were read
  off asm/funcs/func_8007C86C.s directly:
    * packet constant 0xE4000000 — line 53, `lui $a0, (0xE4000000 >> 16)` @ 8007C920
    * x limit D_8009BE78 (signed halfword) — lines 7-8, `lh %lo(D_8009BE78)` @ 8007C884
    * y limit D_8009BE7A (signed halfword) — lines 25-26, `lh %lo(D_8009BE7A)` @ 8007C8C4
    * dispatch D_8009BE74 (unsigned byte), `addiu -1` + `sltiu $v0,$v0,0x2`
      — lines 38-42 @ 8007C8F0..8007C8FC; `bnez` taken for the <2U arm
    * <2U arm: `andi $v1,$a1,0xFFF` (delay slot, line 44 @ 8007C904) +
      `sll $v1,$v1,12` (line 50) + `andi $v0,$a3,0xFFF` (line 51)
      => (y & 0xFFF) << 12 | (x & 0xFFF)
    * else arm: `andi $v1,$a1,0x3FF` (line 45) + `sll $v1,$v1,10` (line 46) +
      `andi $v0,$a3,0x3FF` (line 48)
      => (y & 0x3FF) << 10 | (x & 0x3FF)
  The masks and shifts do coincide with func_8007C7A0's; that is now a
  verified fact about this function's bytes, not an inherited assumption. Only
  the packet constant differs (0xE4000000 vs 0xE3000000).
- [merge] `sandbox func_8007C86C --disable all` = **0**, target_insns 51,
  build_insns 51, rules_dropped 21, cheat_asm_stripped 152 (this differs from
  the s1 recon number 5 @ 50 because s1 measured the sibling's floor-5 chassis,
  not the ternary body merged here).
- [merge] `retire func_8007C86C` dropped all 21 regfix.txt rules; full build
  SHA1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa (== oracle).
- [status] Merged UNCOMMITTED in the working tree pending ruling condition 3
  (fresh independent adversarial layer-2 review of the final diff, author's
  verdict not credited) and `queue done`, both run by the coordinator.
