# Hypothesis ledger — func_8002CA8C

## s1 (recon, 2026-09-15) — floor 177 (empty stub) -> 5
| id | hypothesis | measured | verdict |
|---|---|---|---|
| s1-H1 | `u16 r` local emits an `andi` widening; `s32 r` matches the target's bare lhu | 95 -> 89 | CONFIRMED |
| s1-H2 | hit re-tested after the call2 block (`if (hit) seenMask |= …` as a separate statement) reproduces the redundant `beqz $s0` | 89 -> 86 (per-arm form) | CONFIRMED (shape), superseded by s1-H5 |
| s1-H3 | record pointer walks as its own IV (`rec += 0x14` in the for-increment) -> separate $s5 (biv) and $s2 (= rec+0x12, combined DEST_ADDR giv) | 86 -> 55 | CONFIRMED |
| s1-H4 | call arg spelled `&SCR[id].j[i + 4]` (struct-array ADDR_EXPR) defeats fold's constant re-association, so the 0x1F8000A8 load stays in-loop and only i*12 is reduced | 55 -> 30 | CONFIRMED |
| s1-H4k | any integer / pointer-cast arithmetic spelling of the call arg (i*0xC+C+base; (u8*)C+i*0xC+base; (u8*)(i*0xC+C)+base; base+(i*0xC+C)) | all 55, identical bytes | KILLED (instance: v3a chassis, no FAKE; fold-const.c:893 split_tree strips casts) |
| s1-H5 | `if (hit) seenMask |= 1<<i` written ONCE after the if/else (2 const-1 sites -> not hoisted; fewer seenMask refs -> it becomes the spilled pseudo and base takes $fp) | 30 -> 5 | CONFIRMED |
| s1-H6 | loads spelled `SCR[id].j[i+4].x` (struct member reads) | 84 (address giv not reduced, base+C hoisted and spilled) | KILLED (instance: v7a chassis, no FAKE) |
| s1-H7 | a2==0 -> seen edge needs an `else { seenMask |= 1<<i; continue; }` duplicate | 63 (per-arm seen, v3b) / 42 (shared seen, v8b) — both worse than the plain shape | KILLED (instance: v3a/v8a chassis, no FAKE) |

### Live frontier (for s2)
1. `rejected` flag seat: the target's $s0 is the same seat as `hit`. Probe, ordinary C first: (a) `rejected = (max < c - r || c + r < min || …)` assigned as an expression value; (b) `rejected` declared at function scope; (c) a single status variable for both (`hit = 0; if (AABB fail) hit = 1; if (hit) continue; hit = callX(...)`) — if only (c) works it is the variable-reuse family (FAKE + `.claude/rules/defeat-licm-hoist-var-reuse.md`), so exhaust (a)/(b) and classify with `tools/ra_solver/inverse_compose.py classify` on the .greg dump before burning spellings.
2. `lhu $a3,-6($s2)` before vs after the first x-load: run `tools/sched_solver` on the AABB block (sched1 tie between two loads feeding the same subu, or a sched2 anti-dependence on $a1 that disappears once `rejected` moves to $s0). Probably resolves with (1).
3. At 0: write self_vet.md (constructs expected: none — all ordinary C), then candidate-ready.

## [s1] u16 r local emits an andi widening; s32 r matches the bare lhu
- mechanism: zero_extend of an HImode pseudo used in SImode subtraction
- probe: v1_s32r sandbox --disable all
- result: 95 -> 89
- verdict: CONFIRMED

## [s1] record pointer walking as its own for-increment IV yields a separate $s5 biv and a $s2=rec+0x12 combined DEST_ADDR giv
- mechanism: loop.c strength_reduce / combine_givs on a pointer biv
- probe: v3a_walkrec
- result: 86 -> 55
- verdict: CONFIRMED

## [s1] call arg spelled &SCR[id].j[i + 4] keeps the 0x1F8000A8 constant in-loop and reduces only i*12 (s7), matching the target's addu a2,s7,a2 / addu a2,fp,a2
- mechanism: fold-const.c:3703 associate rule rewrites integer spellings to i*12+(base+C) so loop.c hoists C and base+C (loop.c:1631 move test) and reduces the whole giv; the struct ADDR_EXPR folds to id*0x108+(i*12+C) instead, C is force_reg'd adjacent (savings*lifetime too small to move), and i*12+Creg is not a giv (loop.c:2758 invariant_p==0)
- probe: v7a_scrcallarg + tmp/grind/func_8002CA8C/dumps/code6cac_b.loop read
- result: 55 -> 30
- verdict: CONFIRMED

## [s1] The four integer / pointer-cast arithmetic spellings of the call arg measured (i*0xC+C+base; (u8*)C+i*0xC+base; (u8*)(i*0xC+C)+base; base+(i*0xC+C)) all produce identical bytes at 55 on the v3a chassis
- mechanism: fold-const.c:893 split_tree strips NOP/CONVERT_EXPR before associating, so casts do not block the constant re-association
- probe: v4A/v4B/v4C sandbox
- result: all 55, byte-identical to v3a
- verdict: KILLED
- kill_scope: instance
- measured_on: v3a chassis (walking rec ptr, per-arm seen tail), no FAKE constructs

## [s1] loads spelled SCR[id].j[i+4].x/y/z on the v7a chassis
- mechanism: the mem address becomes (i*12+C)+base which is not a giv, so no reduced address IV and base+C gets hoisted and spilled
- probe: v7b_scrloads
- result: 84 (vs 30 with integer-offset loads)
- verdict: KILLED
- kill_scope: instance
- measured_on: v7a chassis, no FAKE constructs

## [s1] seenMask |= 1<<i written once after the if/else (shared tail) instead of per arm
- mechanism: 2 const-1 movables instead of 4 (combine_movables sums savings, loop.c:1283) keep addiu t0,1 in-loop; fewer seenMask refs make it the spilled pseudo so base takes $fp
- probe: v8a_sharedseen
- result: 30 -> 5, 179/179 insns
- verdict: CONFIRMED

## [s1] an explicit else { seenMask |= 1<<i; continue; } duplicate on the a2 test (per-arm seen: v3b; shared seen: v8b)
- mechanism: the a2==0 -> .L8002CCE0 edge comes from jump threading of the plain shape, not from cross-jumped source duplication
- probe: v3b / v8b sandbox
- result: 63 / 42, both worse than the unduplicated shape
- verdict: KILLED
- kill_scope: instance
- measured_on: v3a and v8a chassis, no FAKE constructs

## s2 (structural + annotation-fix, 2026-09-15) — floor 5 -> 0
The structural work was measured in the first s2 attempt (Judge FAIL 2026-09-15 21:32 on annotation
presence only; docs/grind/decisions.md:27436) whose ledger entries were never banked here — only its
rejected/ forms, self_vet.md and tmp/grind/func_8002CA8C/s2/diff_*.txt survived. This section restores
them from those artifacts so the FAKE annotation's lever-exhaustion pointer (s1-H5..s2-H3) resolves.
Chassis for every row: s1 v8a (walking `rec` IV, `&SCR[id].j[i+4]` call arg, shared seen tail), 179 insns.

| id | hypothesis | measured | verdict |
|---|---|---|---|
| s2-H1 | `rejected = (max < c - r OR c + r < min OR ...)` as an expression value (ordinary C) moves the flag to $s0 | 13/185 (tmp/grind/func_8002CA8C/s2/diff_v9a_rejexpr.txt; rejected/rejected-as-expression-value-13.c): 0/1 materialization + 6 extra insns, seat still $a1 | KILLED (instance: v8a chassis, no FAKE) |
| s2-H2 | a separate `rejected` flag pseudo (no call crossed) can be seated in $s0 by declaration placement: block scope (v8a) and function scope (v9b) | both 5/179, byte-identical (diff_v8a_base.txt, diff_v9b_rejfnscope.txt; rejected/separate-rejected-var-takes-a1-seat-5.c, rejected-declared-at-function-scope-neutral-5.c). Mechanism read in global.c: an allocno with `allocno_calls_crossed == 0` starts from `fixed_reg_set` (global.c:972), `used` adds the complement of `regs_used_so_far` (global.c:1000), and find_reg pass 0 takes the lowest-numbered free register in that set (global.c:1058-1076) = $a1 here; a callee-saved seat is reached only by a call-crossing allocno (`call_used_reg_set`, global.c:974), which in this function is `hit`. .greg dump (tmp/grind/func_8002CA8C/dumps/code6cac_b.greg:2768): the flag pseudo 102 conflicts only with {73, 97, 2, 3, 29}. | KILLED (class: every separate non-call-crossing flag pseudo fails the global.c:972 predicate; predicate_cite tools/gcc-2.7.2/global.c:972) |
| s2-H3 | one status local for both values (`hit = 0; AABB fail -> hit = 1; if (hit) continue; hit = callX(...)`), variable-reuse family, FAKE-annotated | 2/179 (diff_v9c_onevar.txt; rejected/one-status-var-but-multiset-c-load-2.c): seat fixed ($s0), residual = the lhu/lw order | CONFIRMED (seat), FAKE construct |
| s2-H4 | the remaining lhu-after-x-load reorder is sched.c `birthing_insn_p` (sched.c:2505): a three-times-set `c` never gets the single-set LAUNCH_PRIORITY bump; per-axis once-written `x`/`y`/`z` locals do | 0/179 (diff_v10a_xyz.txt; tmp/grind/func_8002CA8C/s2/sched_v9c_fn.txt) | CONFIRMED (ordinary C, semantic reading: three coordinates) |
| s2-H5 | `extern u8 D_800F5F68[]` + `&D_800F5F68[id * 0x1B8]` (byte-array declaration, removes the scalar-address pun) is byte-neutral | 0/179 (diff_v11a_xyz_arraydecl.txt, diff_v12_final.txt) | CONFIRMED (neutral) |
| s2-H6 | (annotation-fix) the FAKE comment spliced above `hit = 0;` in src is byte-neutral; the cp1252 0x97 byte in the comment replaced by ASCII `-` | 0/179 this session, sandbox with the edits in place in src/code6cac_b.c | CONFIRMED |
