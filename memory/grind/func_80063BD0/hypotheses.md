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
