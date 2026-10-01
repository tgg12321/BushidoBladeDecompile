# func_80057E84 — Ruling 11 package for vtx / node / route (laneB, 2026-10-01)

**Re-run (round 2)** on the body after the data-model fix (../dm/README.md: CpuRoute / NavPoly /
NavPolySet in include/code6cac.h, no casts in the body), on main after the Q65 adoption with that
data-model cleanup applied: every score, pseudo, find_reg trace and allocation line below is unchanged
(scores_b.txt, scores_s.txt, pseudos.txt, findreg.txt regenerated); permuter re-run: permuter.txt.

Ruling: `.claude/rules/reused-local-necessity.md` § Ruling 11 (owner 2026-09-26; Q31/Q58 proof
standard). Each of `vtx`, `node`, `route` is judged on its own. The fourth multi-write local, the
counter `i`, is a Q51 SOTN reuse (below), not part of this package.

**Why Ruling 11 and not 5/6/9/10.** Recorded layer-2 verdict 2026-09-25 (hypotheses.md): `vtx` fails
Ruling 5 1(a)-(c) and Ruling 6 (A); `node` fails Ruling 5 ext (A)/(B), 1(c) and Ruling 6 (A) (both
corner blocks run in one iteration); both are on Ruling 9's banned list. `route` (then `buf`) passed
as a staged-value borrow, but a multi-write local may only claim Rulings 5-12 / Q51 now
(`ordinary-c-judge-decidable.md` Ruling 1), and its corner-block writes are not exclusive regions
(Ruling 6 (A)). No public original source (10). Ruling 11 admits what 5-10 fail.

## Bodies (scripts in this directory; run from anywhere under WSL)
- **reuse spelling** = `memory/grind/func_80057E84/candidate.c` (the body spliced at landing; round 1
  measured it with `prep_hdr.py`'s header edit, round 2 on main with the data-model cleanup).
- `gen.sh` -> `gen.py` writes into tmp/func_80057E84/r11/b/: `pv_<var>.c` (every value of one variable in
  its own fresh local, declared where the variable is), `pv_all.c` (all nine values), `pvbs_*.c` (each
  value declared at the innermost block enclosing it), `r_<value>.c` (one value split). Comments are
  stripped first.
- `structural.sh` -> `structural.py`: structural respellings of the full split (`pvs_*.c`) and
  `q51_copy_k.c` (the reuse body with a fresh copy counter).
- **(C)(2) receipt**: `stmtcheck.py` (comments stripped, value declarations removed, values renamed
  back): `IDENTICAL statement lists` for all 18 twins (stmtcheck.txt).

## Values, writes and the target (asm/funcs/func_80057E84.s)
| var | value | C write | read by | target write |
|---|---|---|---|---|
| vtx ($t0) | vtx_a | `vtx = poly->vtx[i];` | `ax = vtx[0]; az = vtx[1];` | `addu $t0,$v1,$v0` 0x80057FC0 |
| | vtx_b | `vtx = poly->vtx[next];` | `bx = vtx[0]; bz = vtx[1];` | `addu $t0,$v1,$v0` 0x80057FEC |
| | vtx_dn | `vtx = poly->vtx[idx_dn];` | the dist_dn SquareRoot0 argument | `addu $t0,$v1,$v0` 0x800581DC |
| | vtx_up | `vtx = poly->vtx[idx_up];` | the dist_up SquareRoot0 argument | `addu $t0,$v1,$v0` 0x80058340 |
| node ($a1) | node_dn | `node = &route->node[c];` (down block) | the x / z / kind stores | `addu $a1,$t0,$v0` 0x80058270 |
| | node_up | `node = &route->node[c];` (up block) | the x / z / kind stores | `addu $a1,$t0,$v0` 0x800583D4 |
| route ($t0) | route_dn | `route = &path[0];` | count load/store, node_dn | `addiu $t0,$sp,0x28` 0x80058250 |
| | route_up | `route = &path[1];` | count load/store, node_up | `addiu $t0,$sp,0x5C` 0x800583B4 |
| | route_pick | `route = &path[0];` / `route = &path[1];` (tail arms) | the copy loop | `addiu $t0,$sp,0x28` 0x800584CC / `0x5C` 0x800584D0 |

(A) fresh locals, never address-taken; `vtx` and `node` declared at the top of the `iter` loop body
(the innermost scope enclosing the edge loop and both corner blocks), `route` at function scope (its
writes span the loop and the tail). No other declaration moved (v3 vs v2: both 0).
(B)(1) every write is read before the next write of its variable on every path (table).
(B)(2) records (Ruling 5 2(c) with the path-wise clarification; C semantics, calls not provably
distinct): a write that can re-store a held value, and a feasible path where the variable holds
something else:
- vtx_a at pass i >= 1 can re-store vtx_b's &vtx[next] of pass i-1 (next == i); func_80057CC8 receives
  `poly` in between. Other value: pass i = 0 of iteration iter >= 1, vtx holds the corner value
  &vtx[idx_up] (or &vtx[idx_dn]) of the previous iteration, e.g. nvtx = 4, idx_up = 2.
- vtx_b: holds &vtx[i]; writes &vtx[next] != &vtx[i] whenever nvtx >= 2 (e.g. nvtx = 3, i = 0).
- vtx_dn: holds the last edge pass's &vtx[next]; differs e.g. when both hits come on edge 0 (next = 1)
  and idx_dn = 3.
- vtx_up: holds &vtx[idx_dn] after a down corner; idx_up = idx_dn + 1 at entry (e.g. 1 and 2).
- node_dn: holds &path[1].node[c'] after an up corner of the previous iteration (another object).
- node_up: holds &path[0].node[c] after the down corner of the same iteration (another object).
- route_dn: can re-store &path[0] when only down corners ran since; holds &path[1] after an up corner
  of the previous iteration (both directions hit in iteration 0).
- route_up: holds &path[0] after the down corner of the same iteration; can re-store &path[1] when only
  up corners ran since.
- route_pick: holds &path[1] when the last corner was an up corner and dist_dn < dist_up picks &path[0].
No read of any of the three is reachable without a write on its path (each read follows its write
in the same block).
(C)(3) every value is address arithmetic in the target (`addu` / `addiu`, table); no constant or
copy values, no copy-clause value.

## (D)(4) results — sandbox --disable all (447-insn target), scores_b.txt / scores_s.txt
| body | score | insns |
|---|---|---|
| reuse (candidate.c) | **0** | 447 |
| pv_vtx / pvbs_vtx | 22 / 22 | 447 |
| pv_node / pvbs_node | 22 / 22 | 447 |
| pv_route / pvbs_route | 4 / 4 | 447 |
| pv_all / pvbs_all (full split) | 46 / 46 | 447 |
| r_vtx_a / r_vtx_b / r_vtx_dn / r_vtx_up | 3 / 3 / 8 / 8 | 447 |
| r_node_dn / r_node_up | 22 / 22 | 447 |
| r_route_dn / r_route_up / r_route_pick | 2 / 2 / 4 | 447 |
| pvs_noptr_vtx (vertices read as poly->vtx[k][0/1]) | 48 | 447 |
| pvs_noptr_node (route_X->node[c].x/.z/.kind) | 47 | 445 |
| pvs_noptr_route (path[0]/path[1] in the corner blocks) | 48 | 445 |
| pvs_noptr_all (the three) | 44 | 445 |
| pvs_tail_cond (tail as one ?: ) | 46 | 447 |
| pvs_node_early (node before the c >= 7 test) | 50 | 445 |
| pvs_vtx_struct (NavVtx {x, z} pointers; round 2 through a cast, evidence only) | 46 | 447 |
| pvs_vtab (vertex table loaded once per edge pass) | 76 | 446 |
| pvs_register (every split value `register`, no asm) | 46 | 447 |

Earlier-chassis receipts (PathWalker/u8* body, hypotheses.md; superseded but same mechanism): static
inline vertex helper = n0, struct view = n0, Ruling 4 node split-init 36/445, hoisted vertex table 44.

Permuter campaign from the full split (permuter.txt), round 2: split base 700, 6903 iterations, best 490
(borrows and junk; round 1: 6445 iterations, best 440, a vertex-pointer re-share); the reuse body scores
0 on the same scorer (perm_control.sh).

## (D)(1) dumps and command lines
`dump.sh <tag> <body>` (`dumpall.sh` for reuse, pv_vtx, pv_node, pv_route, pv_all, pvbs_all and the
nine r_*): splice with engine.inlineasm.substitute_body + prep_hdr.py's header -> `mipsel-linux-gnu-cpp`
with the build's CPP_FLAGS/CPP_DEFS -> `tools/decomp-permuter/strip_other_fns.py` -> build cc1
`tools/gcc-2.7.2/build/cc1 <the build's CC_FLAGS for text1b> -dr -dl -dg` -> instrumented
`tools/gcc-2.7.2/cc1` with `BB2_ALLOC_DEBUG=1 BB2_QTY_DEBUG=1`; the two compilers' func_80057E84 asm is
compared: IDENTITY OK for all 15. `findreg.sh <tag> <pseudo>...` (`findreg_all.sh`): instrumented
cc1 with `BB2_FINDREG_DEBUG=<pseudo>`. Banked: `pseudos.txt` (each value's pseudo, .lreg `Register`
line, ALLOCDBG line or local-alloc, .greg disposition, for 14 bodies), `qty.txt` (local-alloc QTYDBG
of the split values), `findreg.txt`, `lreg_registers.txt`, `alloc_<tag>.txt`.

Pseudo map (reuse): 132 vtx, 133 node, 103 route (write insns 190/223/497/760, 632/901,
600/869/1045/1053; pseudos.py checks each by UID).

## (D)(2) Mechanism (pass and source location; tools/gcc-2.7.2)
1. **flow.c** records each pseudo's block (`reg_basic_block`) and deaths (`reg_n_deaths`).
2. **local-alloc.c:472-475**: a pseudo used in one basic block that dies once gets
   `reg_qty = -2` (local allocation); every other pseudo `-1` (global.c).
3. **local-alloc.c combine_regs (:1784)**, called by block_alloc (:1123) at a local pseudo's birth
   insn, ties the set register into the quantity of a dying input (:1827-1851); it refuses when the
   set register is global (`reg_qty[sreg] == -1`, :1836).
4. **global.c:575/615/643** orders allocnos by `floor_log2(n_refs) * n_refs / live_length`;
   **find_reg (:952)** takes the lowest register not in `used` (conflicts, call-used set when the
   allocno crosses a call, :975; no REG_ALLOC_ORDER, :1060).

Applied (pseudos.txt, qty.txt, findreg.txt):
- **vtx.** Reuse: pseudo 132 `used 30 times across 18 insns; dies in 4 places` -> global; ALLOCDBG
  ord=0 hardreg=8; find_reg conflicts `2 3 4 5 6 7 16 29` (the union over its four sites), first free = 8
  ($t0) for all four values, as the target.
  Split (pv_vtx): each value `in block 10/31/40`, one death -> local; QTYDBG vtx_a qty refs 15 vs its
  own 9 (tied with its dying input), `got=3` ($v1); vtx_b $v1; vtx_dn / vtx_up `got=4` ($a0). One
  value split alone (r_vtx_*) leaves the rest one allocno in $t0 and the split one local-tied: 3/3/8/8.
- **node.** Reuse: 133 `dies in 2 places` -> global; ALLOCDBG ord=1 hardreg=5; conflicts `2 3 4 29`
  -> $a1 (5), as the target. Split: node_dn `in block 33` -> $v0, node_up `in block 42` -> $v1
  (QTYDBG got=2/3). Splitting either value leaves the other single-block, so r_node_dn = r_node_up =
  pv_node = 22.
- **route.** Reuse: 103 `across 52 insns; dies in 2 places`, conflicts `2 3 4 5 6 7 29` -> $t0; the
  $a1-$a3 conflicts come with the tail value (split, route_pick conflicts `2 3 4 5 6 7 18 29`), and
  one allocno carries them into both corner blocks. Split (pv_route): route_dn / route_up (103/104,
  multi-block, global) conflict only `2 3 4 29` -> $a1, against the target's $t0 at 0x80058250 /
  0x800583B4 (and node then cannot take $a1); route_pick (105) conflicts `2 3 4 5 6 7 18 29` -> $t0.

## (E)/(F)
Names are kind-names true of every value: every `vtx` value points at a vertex's x/z pair, every
`node` value at a CpuWaypoint being appended, every `route` value at a RouteBuf. Each declaration
carries the inline comment naming its values, citing Ruling 11 and this file.

## Q51 / Q53: the counter `i`
`i` counts the edge scan (`for (i = 0; i < nedges; i++)` with its `break`) and then counts the copy
loop down (`for (i = route->count - 1; i >= 0; i--)`), both in $s2 in the target (`addu $s2,$zero,$zero`
0x80057F84, `addu $s2,$v1,$zero` 0x8005818C; `addu $s2,$v0,$zero` 0x800584E4, 0x80058534). SOTN
`src/dra/42398.c` @aa53500 (config/splat.us.dra.yaml: `[0x42398, c, 42398]`; DebugCaptureScreen, no
version guard, no INCLUDE_ASM): `s32 i;` line 75 counts the file-search loop up with a `break`
(lines 85-95) and then counts the row loop down, `for (i = height - 1; i >= 0; i--)` line 131.
Match-motivated (Q53): a fresh copy counter `k` (q51_copy_k.c) scores 79 (it is call-free, takes $t0,
and every reload register rotates); annotated `/* FAKE: ... */` with the SOTN tag.
