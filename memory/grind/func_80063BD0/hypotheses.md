# Hypothesis ledger — func_80063BD0

## s1 (2026-09-15, recon) — floor 144 -> 0
| id | form (tmp/grind/func_80063BD0/s1/vN.c) | distance | verdict |
|---|---|---|---|
| H1 v1 | for-loop, `bits` local before loop, `(1<<i)` twice, `src` local, inline returns | 75 | KILLED: single-bit transform (srav/andi) + count load hoisted |
| H2 v2 | goto-loop chassis (func_800644FC style) | 88 | KILLED: nothing hoisted (li/la inside loop) |
| H3 v3 | for-loop + `mask` local, `bits` before loop | 76 | KILLED: same as v1 minus the transform |
| H4 v4 | v3 with `D_800A3454[idx] |= mask` direct (no `bits`) | 66 | CONFIRMED lever: shared address pseudo (la+addu, `lw/sw 0(a2)`), count reload in-loop |
| H5 v5 | element pointer `bp = &D_800A3454[idx]` | 76 | KILLED |
| H6 v6 | inline `(1 << i)` in test and `|=` | 58 | KILLED: cse does not unify the shifts; srav/andi |
| H7 v7 | v4 + `((s32 *)D_800A347C)[k]` inline (no `src` local) | 43 | CONFIRMED: source pointer read where used |
| H8 v8 | v7 + `goto found` arm after loop + single trailing `return 1` | 10 | CONFIRMED: shared final `li v0,1` block; cross-jump gone |
| H9 v9 | v8 with inline shifts | 34 | KILLED: srav/andi again |
| H10 v10 | v8 with `(mask = 1 << i)` inside the condition | 4 | CONFIRMED: la before li; residual = base copy coalesced |
| H11 v11 | v10 with `continue` + `goto found` | 4 | KILLED: continue label does not survive jump1 |
| H12 v12 | v10 with the arm INSIDE the loop + `break` | 0 | CONFIRMED |
| H13 v13 | v12 respelled: `bits = D_800A3454[idx]; mask = 1 << i; if (!(bits & mask)) { D_800A3454[idx] |= mask; ... break; }` | 0 | CONFIRMED -- adopted |
| H14 v14 | v13 with `D_800A3454[idx] = bits | mask;` | 0 | CONFIRMED (alternate) |
All kills are instance kills on this chassis (GCC 2.7.2 -O2 -mel -msoft-float, no FAKE constructs anywhere).

## 2026-09-15 re-dispatch (driver session 1, recon) -- floor 0 -> 0 (declaration spelling only)
| id | form | distance | verdict |
|---|---|---|---|
| H15 | s1 candidate re-applied (flat `[][10][3]`, BANNED spelling) | 0/144 | CONFIRMED chassis unchanged (measurement only, not submitted) |
| H16 | header-canonical `Unk800F0EC8Record D_800F0EC8[][10]` in include/game.h + members `.unk0/.unk4/.unk8`, scalar externs removed from text1b.c and text1b_b.c, alias-suffixed undefined_syms_auto.txt rows | 0/144, oracle ok=true | CONFIRMED -- adopted (candidate_merge.patch); needs scope grant -> ruling-request / integration handoff |
Frontier: none on the C -- the body is solved; the only open item is the scope grant + Judge ruling on the aggregate-merge family (all five prongs documented in evidence.md).

## 2026-09-15 driver session 1 (recon, post-handoff) -- floor 0 -> 0, LANDED FOR REVIEW
| id | form | distance | verdict |
|---|---|---|---|
| H17 | candidate_merge.patch applied to clean HEAD aeeb5610d under the executed scope grant (tools/grinder/scope_allow.txt:70) | canonical C 0/144; sandbox 0/144 rules_dropped 0; func_80060C60 0/22; verify-oracle --rebuild ok=true SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa | CONFIRMED -- submitted candidate-ready |
| H18 | Sibling inheritance (func_800644FC s1 `extern s32 D_800A3454[]`, func_8004473C/f27569e73 header-canonical record pattern, func_80061250/func_8006288C source-pointer idiom): all already on main and consumed by the candidate body | 0/144 (same measurement) | CONFIRMED -- no unspent sibling spelling remains (every shared block already matches) |
Frontier: none -- the body is solved and now sits in src/ awaiting layer-1 + Judge final call.
