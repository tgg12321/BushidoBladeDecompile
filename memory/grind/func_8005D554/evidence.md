# Evidence bank — func_8005D554

## s1 (2026-09-08, recon) — chassis: HEAD main @ fabcda5a, cc1 -mel -msoft-float, 0 rules

- **Baseline on this chassis:** `canonical` verdict C (hand_coded_tier LOW, no S1/S2/S6),
  target 176 insns. The retired-chassis body (`retired-chassis-2026-08/body.c`) scores
  **65** (`sandbox --disable all`, 176/176 insns, frame 104 vs target 120). Residual
  families of that body: frame -16 (24 lines), s2/s3 seat swap (i vs 0x100), `&D_8009B388`
  rematerialized in-loop instead of held in s7, `base+0xC` in fp instead of spilled to
  sp+0x40, fold reassociation of `(r5 - K) + rnd` into `r5 + (rnd - K)` (4 sites),
  `move a1,zero` placement (2 sites). Diff: `tmp/grind/func_8005D554/s1/pairdiff_retired.txt`.

- **Cross-knowledge recovered:** the legacy WIP checkpoint `memory/wip/func_8005D554/`
  (deleted from the tree at 3e8527af, recoverable via `git show 2651e2e5:memory/wip/func_8005D554/notes.md`
  and `.../candidate_8.diff`; copies in `tmp/grind/func_8005D554/s1/wip_notes.md` + `candidate_8.diff`)
  banked a **score-8** form on the 2026-08-05 chassis (post -mel, pre -msoft-float). Its levers:
  loop guard written in the loop variable (`if (i < ((D_800A326C + 1) * 2))` — jump.c
  `duplicate_loop_exit_test` shape; births a comparison pseudo combine folds to `blez`, which
  reload gives an 8-byte phantom slot: this is the target's 16-byte frame delta, measured 0-insn
  cost), named displaced pointer `p_b2ec = p_b2e0 + 0xC` (raises `p_b388`'s allocno priority
  ordering), `p_b390 = p_b388 + 2` (bumps `p_b388` refs to 4 so it wins s7 over `p_b2e0`,
  which then takes fp and pushes `base_offset` to the sp+0x40 spill — exactly the target
  seats), and split-init accumulation `off = r5 - K; off += rnd;` on all four offset sites
  (sanctioned ordinary C per [[split-init-accumulation-sanctioned]]).

- **Score-8 form re-measured on THIS chassis: 7** (176/176). The old "group 3" residual
  (`lui/addiu %lo(D_8009B390)` vs `lui %hi(D_8009B388); addiu a3,a3,8`) is GONE on the
  soft-float chassis — `p_b390 = p_b388 + 2` now emits the target's `%lo(D_8009B390)` form
  (score masks the symbol+8 addend the same way). Diff: `s1/pairdiff_cand8.txt`.

- **Operand-order lever CONFIRMED (7 -> 6):** `s.p0 = (void *)(stride + (s32)p_b2e0);`
  (integer-typed sum) emits the target's `addu v0,s0,fp` (stride first). The pointer-typed
  spellings `p_b2e0 + stride`, `stride + p_b2e0`, `&p_b2e0[stride]` all emit `addu v0,fp,s0`
  because `pointer_int_sum` canonicalizes to PLUS(ptr,int) regardless of source order; an
  integer PLUS keeps source operand order. Form: `tmp/grind/func_8005D554/s1/p1_intorder.c`
  = `memory/grind/func_8005D554/candidate.c` (**floor 6**, 176/176, frame 120 == target).

- **Residual at 6 (13 differing lines, one class, two instances):** after the 3rd and 7th
  `rand` calls the target orders `addiu a0,sp,16; move a1,zero; lw v1,D_800A3418; addiu a2,s4,-K;
  sw zero,32(sp); ...` while ours orders `addiu a2,s4,-K; addiu a0,sp,16; lw v1; move a1,zero; sw zero,32(sp); ...`.
  RTL UIDs on the candidate (sched2 dump `s1/sched2_p1.txt`): first half 211 (a2 init),
  240 (a0=sp+16), 205 (lw), 242 (a1=0), 228/231/234 (stores), 206 (xor); second half
  316 / 346 / 310 / 348 / 334-340.

- **Probes measured (all on the candidate chassis, floor 6/7):**
  - `p2_a2init_late.c` (a2 init statement moved to just before its `+=`): **7** — no change.
  - `p2b_stores_before_xor.c` (struct stores zero10/one14/ret moved before the a2-feeding
    `D_800A3418 ^= rand()`): **32** — much worse. Dead spelling.
  - `q2_stores_before_a2init.c` (stores between the xor and the a2 init): **6** — no change.
  - `q3_single_off.c` (one `off` local for both offset sites): **12** — worse.
  - `p4_typed_array.c` (OBJECT-MODEL probe, below): **21**.

- **OBJECT MODEL:** D_8009B2E0 — census `g_eff_particle_template_table` (0x3C stride, arg1
  indexes it), no header decl. MISMATCH measured: a typed `extern EffTmpl D_8009B2E0[]`
  (struct {s32 hdr[3]; s32 sub[2][3]; s32 rest[6];}) with `s.p0 = &D_8009B2E0[idx]` /
  `&D_8009B2E0[idx].sub[D_800A3418 & 1]` scores **21** vs 6 for the byte-pointer form: array
  indexing re-associates `idx*60` with the base per use and loses the hoisted `stride` /
  `base+0xC` invariants the target keeps in s0 and sp+0x40. The target bytes are consistent
  with the byte-pointer-plus-stride model (TU-local `extern u8 D_8009B2E0`, same as the matched
  siblings func_8005D46C/func_8005FA98 in this file). D_8009B388 / D_8009B390 — MATCHES:
  `extern s32` scalars, `&D_8009B388` held in s7 across the loop and `&D_8009B390` emitted as
  `%lo(D_8009B390)` (the `p_b388 + 2` spelling scores identically to the symbol on this chassis;
  the +8 addend is what the target encodes either way). No declaration fix is on the frontier.

- Loop.c on this chassis: `Insn 251: regno 149 (life 1), move-insn savings 1 not desirable`
  — the in-loop `&D_8009B390` materialization is NOT hoisted (threshold 29 with loop_has_call,
  insn_count 98), matching the target's in-loop `lui a3/addiu a3`. The `&D_8009B388` pseudo
  in the retired body had refs==2 → `local-alloc.c:1079` marked it `reg_equiv_replacement`,
  global skipped it, reload rematerialized it in-loop (`greg`: "Spilling reg 6" for insn 128).
  The `p_b390 = p_b388 + 2` reference is what lifts refs to 4 and keeps it allocated (s7).

### s1 addendum — the last-6 residual is a sched1 LUID tie, proven on the exact model

- reorg.c is ruled out: `tools/gcc-2.7.2/config/mips/mips.md:125-138` defines delay slots only
  for branch/jump/call, and the `.dbr` order equals the `.sched2` order for the whole loop
  block (`s1/dbr_p1.txt` vs `s1/sched2_p1.txt`); the lw "delay slot" content is sched output.
- sched2 is a pure replay: in the pass-2 model all of {a2 init, a0=&s, a1=0, lw} carry
  priority 3, so their order is the post-sched1 LUID order.
- sched1 pass-1 block 6 (`s1/picks.py 1 6 ...`): reverse list scheduling; the lw (uid 205,
  LAUNCH priority as a birth) is not ready until clock 65 (memory-unit blockage from the
  three struct stores picked at 61-63). At clock 64 the ready array is [242, 240, 211] — all
  priority 3, all class 3 vs last_scheduled 228 — and `rank_for_schedule` picks by
  INSN_LUID (`tools/gcc-2.7.2/sched.c:2464`); 211 (luid 31) loses to 242 (luid 43). The ready
  array carries over between cycles (simulate.py `sched_sort`, d==0 means no re-sort), so
  insertion history matters: 211 was inserted at clock 55 behind 228 by luid.
- Exact-model mutations (`s1/mutate.py`, output `s1/mutate_out.txt`), forward order of the
  first-half region, OURS = [201, 211, 240, 205, 242, 228, 231, 234, 206, ...], TARGET =
  [201, 240, 242, 205, 211, 228, 231, 234, 206, ...]:
  - luid(211) := 43.5 -> TARGET (both halves) [F1]
  - luid(240,242) := 30.5/30.6 or 28.5/28.6 -> TARGET [F1, equivalent statement]
  - a1-only-low, a0-only-low, init luid 42.5 -> ours (both arg loads must precede the init)
  - pri(211)=4 + 228 depends on 211 -> TARGET (half 1) [F2 variant]
  - LAUNCH(211) + 228 depends on 211 -> TARGET (both) [F2]
  - LAUNCH(211) alone -> init lands right before its add (wrong)
  - pri4 alone, dep228 alone, dep on lw, stores pri 4, lw pri 4, no store blockage,
    211 anti-dep on stores, dropping 211 (reload-born-init theory) -> all wrong.
- Expand-time RTL (`s1/rtl_p1.txt`): 237 (sw a2->44), 240 (a0 = reg 69), 242 (a1 = 0), 244 call —
  nothing is emitted between the arg loads and the call except what `emit_queue`
  (`tools/gcc-2.7.2/calls.c:1910`, queued post-inc/dec side effects) flushes, so a statement
  before the call cannot reach luid > 43 (H4 class kill in hypotheses.md).
- Perturber note: `perturb.py --goal-from-target` skips block 6 ("goal is not a topological
  order: duplicate instruction text mis-paired" — the two loop halves are near-identical);
  hand goals via `--goal-before` at depth 2 did not finish in 10+ min and were stopped. The
  mutation harness is the faster instrument for this block.

- [s1] OBJECT MODEL: D_8009B2E0 MISMATCH (measured 21) for a typed 0x3C-stride struct array; the byte-pointer-plus-stride model (extern u8, as in matched siblings func_8005D46C/func_8005FA98) reproduces the target seats. D_8009B388/D_8009B390 MATCH as extern s32 scalars (&D_8009B388 in s7, &D_8009B390 as %lo(D_8009B390) via p_b388 + 2 on this chassis).

- [s1] Legacy WIP memory/wip/func_8005D554 (deleted at 3e8527af) held a score-8 form; recovered via git show 2651e2e5 into tmp/grind/func_8005D554/s1/{wip_notes.md,candidate_8.diff}; it re-measures 7 on this chassis and 6 with the int-typed p0 sum.

- [s1] Frame 120 == target on the candidate (guard-in-i phantom slot); frame is no longer a residual.

- [s1] Residual at 6 = 2 x (a0=sp+16, a1=0, lw, addiu a2,s4,-K order after the a2-site rand call); reorg.c ruled out (no load define_delay in mips.md:125-138; .dbr order == .sched2 order); sched2 is a pure LUID replay; the pick is sched1 clock 64 with ready [242,240,211] all priority 3/class 3 -> INSN_LUID.

- [s1] Exact-model routes to the target order: (F1) init luid > a1-load luid (both arg loads must precede the init; a1-only or a0-only does not); (F2) LAUNCH-priority init (single-set birth, sched.c:2505) plus a dependence of the s.zero10 store on it. LAUNCH alone, pri 4 alone, dropping the init (reload-born theory), lw priority, store priority/blockage changes all fail.

- [s1] loop.c on this chassis: in-loop &D_8009B390 'not desirable' (threshold 29, insn_count 98) matches the target's in-loop lui/addiu; the retired body's refs==2 &D_8009B388 pseudo was rematerialized by reload after local-alloc.c:1079 marked it replaceable.

- [s1] perturb.py --goal-from-target skips loop block 6 (duplicate instruction text mis-pairs the halves); depth-2 hand goals did not finish in 10 min; s1/mutate.py (direct simulate.py mutations) is the working instrument for this block.

## s2 (2026-09-08, structural) — chassis: HEAD main @ 8d3c3235, cc1 -mel -msoft-float, 0 rules

- **Chassis re-measured:** `memory/grind/func_8005D554/candidate.c` applied to src/text1b.c
  scores **6**, 176/176 insns — the s1 floor is intact on this chassis. Scored diff regenerated
  as `tmp/grind/func_8005D554/s2/pairdiff_base.txt`: **12 differing instructions, exactly two
  identical 3-insn rotations**, one per loop half. Ours emits
  `addiu a2,s4,-K / addiu a0,sp,16 / lw v1,0(gp) / move a1,zero`; the target emits
  `addiu a0,sp,16 / move a1,zero / lw v1,0(gp) / addiu a2,s4,-K`. Nothing else differs.

- **Target asm read directly** (`asm/funcs/func_8005D554.s:91-109`, first half): the
  `jal rand` at :91 has `sw a0,0x28(sp)` (`s.zero18 = a0_offset`) in its delay slot; the
  `func_80073728` arg loads `addiu a0,sp,0x10` / `addu a1,zero,zero` sit at :93-94 —
  **15 insns ahead of their `jal` at :108** and ahead of both the `lw` of D_800A3418 (:95)
  and the a2 init (:96). The a2 sum reaches its consumer as `addu a2,a2,v0` (:106) and is
  stored by `sw a2,0x2C(sp)` in the call's delay slot (:109); `s` is at sp+0x10 so
  0x2C == `s.zero1C`, 0x20 == `s.zero10`, 0x24 == `s.one14`, 0x1C == `s.ret`. Field mapping
  and store order in our candidate are already identical to the target.

### s2 probes — 16 structural spellings, ALL measured (probe log `s2/probe_log.txt`)

Every one is `sandbox func_8005D554 --disable all` on the s1 candidate chassis, no FAKE
constructs present. Score 6 == the floor (no change), lower is better; none went below 6.

| form | score | insns | note |
|---|---|---|---|
| `base.c` (= candidate.c) | 6 | 176 | floor |
| `v1_three_split` — `a2_offset = (s32)r4; a2_offset -= K; a2_offset += rnd;` | 6 | 176 | neutral; the `-= K` insn stays in the same slot |
| `v2_rnd_first` — `a2_offset = rnd; a2_offset += (s32)r4 - K;` | 15 | 176 | the `-K` moves into the `+=` expression and the sum reassociates |
| `v3_unsplit_kfirst` — `a2_offset = ((s32)r4 - K) + rnd;` | 15 | 176 | fold reassociates to `r4 + (rnd - K)` (the retired body's residual) |
| `v4_unsplit_rndfirst` — `a2_offset = rnd + ((s32)r4 - K);` | 15 | 176 | same reassociation, operand order irrelevant |
| `v5_fresh_singleset` — per-site `a2b1 = (s32)r4 - K; a2_offset = a2b1 + rnd;` | 54 | **178** | +2 insns |
| `v6_declorder` — `a2_offset` declared before `a0_offset` | 6 | 176 | neutral |
| `v7_r4_signed` — `r4`/`r5` declared `s32` instead of `u32` | 6 | 176 | neutral |
| `w1_singleset_direct` — per-site once-set base, sum stored straight into `s.zero1C` | 54 | **178** | +2 insns |
| `w2_shared_direct` — one shared base var, sum stored straight into `s.zero1C` | 10 | 176 | worse |
| `w3_distinct_split` — distinct accumulator per a2 site (`a2p`/`a2q`), split kept | 6 | 176 | neutral |
| `w4_field_accum` — accumulate in the field: `s.zero1C = (s32)r4 - K; s.zero1C += rnd;` | 54 | **178** | +2 insns; `s` is address-taken so the field cannot stay in a register |
| `w5_store_order` — `s.zero1C` store before the `zero10/one14/ret` group | 6 | 176 | neutral |
| `w6_decl_first` — `a2_offset` declared first of all locals | 6 | 176 | neutral |
| `x1_blockscope` — offset locals declared at do-while block scope | 6 | 176 | neutral |
| `x2_perhalf_blocks` — nested block per loop half, own offset locals in each | 6 | 176 | neutral |
| `x3_a2_unsigned` — `a2_offset` typed `u32`, cast at the store | 6 | 176 | neutral |

- **The +2-insn signature is the same in v5/w1/w4:** any spelling that gives the `r4 - K`
  value its own once-written carrier (a fresh local, or the struct field itself) costs two
  extra instructions — GCC does not coalesce the extra pseudo/memory copy away. This is the
  measured price of F2's `reg_n_sets == 1` LAUNCH-birth precondition, and it is unaffordable
  at 176/176.

- **F1 is now closed as a class, mechanically.** s1 proved (a) sched1's clock-64 pick among
  the equal-priority, equal-class ready set is INSN_LUID order (`tools/gcc-2.7.2/sched.c:2464`)
  and the target order needs `luid(a2 init) > luid(a1 = 0)`, and (b) `expand_call` emits
  nothing between `load_register_parameters` and the call except `emit_queue`
  (`tools/gcc-2.7.2/calls.c:1909-1910`, "Perform postincrements before actually calling the
  function"). s2 reads the remaining half: `emit_queue` can only flush insns put there by
  `enqueue_insn`, and the sole caller is `expand_increment`'s post-inc/dec fast path
  (`tools/gcc-2.7.2/expr.c:8641`), which queues `GEN_FCN (icode) (op0, op0, op1)` — an insn
  whose **destination is the incremented variable itself**, while the enclosing expression's
  value is the *pre*-increment copy (that is the whole point of `QUEUED` /
  `protect_from_queue`, `expr.c:402-418`). The a2 residual insn is `a2 = r4 - K` whose result
  is consumed by the `addu` two insns later in the SAME iteration; a queued post-decrement can
  never be that insn. Independently the constants forbid a decrement chain anyway: the two a2
  sites need `r4 - 0xC` and `r4 - 0x19` from a loop-invariant `r4`, i.e. a delta of 13 with a
  per-iteration reset, not a monotone walk.

- **Sibling `main` in src/ings.c (COMPLETED-C, floor 0) is SPENT and shares nothing.**
  `grep -n "D_800A326C\|D_800A3418\|D_8009B2E0\|func_80073728" src/ings.c` returns zero hits:
  no global, no callee, and no block is common with func_8005D554, so there is no spelling to
  transplant. Recorded so no later session re-opens it.

### s2 addendum — register allocation already MATCHES the target; the callee is genuinely 2-arg

- **The candidate's hard-register assignment in the residual region is byte-identical to the
  target.** cc1 `.s` dump (`pwsh tools/grinder/dump.ps1 func_8005D554` ->
  `tmp/grind/func_8005D554/dumps/text1b.s`, func body lines 134-147):
  `addu $6,$20,-12 / addu $4,$sp,16 / lw $3,D_800A3418 / move $5,$0 / sw $0,32($sp) /
  sw $22,36($sp) / sw $17,28($sp) / ... / addu $6,$6,$2 / sw $3,D_800A3418`. Target
  (`asm/funcs/func_8005D554.s:93-107`) uses exactly the same registers: `a2`($6) for the
  offset, `s4`($20) for r4, `a0`($4), `a1`($5), `v1`($3), `s6`($22), `s1`($17). So there is
  **no allocation residual left at all** — frontier item G2 (attack the a2 pseudo's seat) is
  dead on arrival, and any future session should treat the last 6 as a pure emission-order
  question, not an RA question.

- **func_80073728 does NOT take a third argument.** `grep '\$a2' asm/funcs/func_80073728.s`
  returns only `mflo $a2` at :194 and :235 — `$a2` is never read before being written, so the
  incoming ABI arity is (a0, a1), matching the existing `extern s32 func_80073728(s32, s32);`
  at src/text1b.c:2642 and every other call site in the file. The tempting idea that the
  target's `addiu a2,s4,-K` sits after the a0/a1 loads because it is a *third argument* set up
  by `load_register_parameters` is therefore false: `a2` is a scratch seat for the offset
  pseudo in both our build and the target.

- [s2] Chassis re-measured: memory/grind/func_8005D554/candidate.c applied to src/text1b.c scores 6 at 176/176 insns on HEAD main @ 8d3c3235 (cc1 -mel -msoft-float, 0 rules) - the s1 floor is intact.

- [s2] The scored diff (tmp/grind/func_8005D554/s2/pairdiff_base.txt) is exactly 12 differing instructions = two identical 3-insn rotations, one per loop half. Ours: addiu a2,s4,-K / addiu a0,sp,16 / lw v1,0(gp) / move a1,zero. Target: addiu a0,sp,16 / move a1,zero / lw v1,0(gp) / addiu a2,s4,-K. Nothing else in the function differs.

- [s2] Target asm read directly (asm/funcs/func_8005D554.s:91-109): the func_80073728 arg loads sit 15 insns ahead of their jal, in the slot right after the preceding rand call; the a2 sum reaches addu a2,a2,v0 at :106 and is stored by sw a2,0x2C(sp) in the call's delay slot at :109. With s at sp+0x10 that maps 0x2C=s.zero1C, 0x20=s.zero10, 0x24=s.one14, 0x1C=s.ret - our candidate's field mapping and store order already match the target exactly.

- [s2] 16 structural spellings measured this session, all recorded with score and insn count in evidence.md s2 and tmp/grind/func_8005D554/s2/probe_log.txt: 11 neutral at 6/176, three at 15/176 (fold reassociation), one at 10/176, three at 54/178.

- [s2] The +2-instruction signature is identical across v5/w1/w4: any spelling that gives the r4 - K value its own once-written carrier costs two extra instructions that GCC does not coalesce away, which prices F2's reg_n_sets == 1 precondition out at 176/176.

- [s2] F1 is closed mechanically: emit_queue (calls.c:1909-1910) can only flush insns from enqueue_insn, whose sole caller is expand_increment's post-inc/dec fast path (expr.c:8641) emitting op0 = op0 <op> op1; the enclosing expression's value is the pre-increment copy (expr.c:402-418), so a queued insn can never be the a2 init whose result the following addu consumes.

- [s2] Register allocation is byte-identical to the target across the residual window ($6/$20/$4/$5/$3/$22/$17), so nothing is left but sched1 emission order.

- [s2] func_80073728's ABI arity is (a0, a1): $a2 appears only as mflo destinations at asm/funcs/func_80073728.s:194 and :235.

## s3 (2026-09-08, permuter) — chassis: HEAD main @ 1ddb0a8c, cc1 -mel -msoft-float, 0 rules

**HONEST FLOOR 6 -> 0.** `sandbox func_8005D554 --disable all` prints `"score": 0,
"build_insns": 176` for `tmp/grind/func_8005D554/s3/g23_stage_ret.c` (now
`memory/grind/func_8005D554/candidate.c`). The form is NOT submitted this session: its
closing construct is a ruling-request (see hypotheses.md H16).

### Permuter workspaces built this session (both faithful to the full-TU build)

`tmp/perm_5d554/` (s1/s2 candidate chassis) and `tmp/perm_5d554_z3/` (the z3
guard+while chassis). Each is a STANDALONE base.c (typedef prelude + the function only)
compiled with the exact Makefile pipeline for `text1b` — `cc1 -O2 -G0 -funsigned-char
-quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | prologue_fix |
maspsx (MASPSX_FLAGS incl. --prefill-label-funcs) | multu_pad | as`. Fidelity proven:
`objdump -d base.o` vs `target.o` differs in exactly the 12 known residual instructions
plus the `addiu a3,a3,8` vs `addiu a3,a3,0` relocation-addend line that `engine/score.py`
masks. Standalone compilation is therefore a valid permuter context for this function —
no full-TU wrapper is needed (contrast `tools/mar_perm_workspace.sh`).

Both campaigns carried the un-maskable `a3` addend line, so permuter base_score was 310
for a sandbox distance of 6; permuter scores here are NOT comparable to sandbox scores.

- **Campaign 1** (`--label s3-candidate-chassis`, -j 8, 676 s): **21,569 iterations, 0
  finds**. The s1/s2 candidate chassis is a dead basin for random search.
- **Campaign 2** (`--label s3-z3-guard-while-chassis`, -j 8, ~1100 s): **30,885
  iterations, 1 find** — `tmp/perm_5d554_z3/output-160-1/` (permuter score 310 -> 160),
  found at ~413 s. Sandbox-measured: **3** at 176 insns. Its two mutations were
  (a) `new_var = ((s32) r4) - 0xC;` hoisted to sit between `a0_offset += ...` and
  `s.zero18 = a0_offset;` in loop half 1, with `a2_offset = new_var;` after the rand call,
  and (b) `new_var = 0; ... s.zero10 = new_var;` in loop half 2. Reseeding a structurally
  different chassis is what produced the find — the same random search on the original
  chassis produced nothing in 21k iterations.

### The closing lever, isolated by hand ablation (13 forms, all `sandbox --disable all`)

| form | score | insns | note |
|---|---|---|---|
| `base.c` (= s2 candidate) | 6 | 176 | floor at session start |
| `f1_perm160.c` (the raw permuter find) | **3** | 176 | half the residual closed |
| `g1_hoist_h1` / `g2_hoist_h2` (fresh carrier, one half, single-set) | 37 | 178 | |
| `g3_hoist_both_samevar` (one carrier, both halves, base only) | 31 | 178 | |
| `g4_hoist_both_distinct` (two carriers, base only) | 54 | 178 | |
| `g7_reorder_only` / `g8_reorder_h1` (move `a2_offset = r4-K` earlier, no carrier) | 31 | 178 | plain reorder is NOT enough |
| `g13_a2_reuse_zero` (`a2_offset` itself reused for the zero10 store) | 31 | 178 | |
| `g14_reuse_only` (zero10 via `a2_offset`, no hoist) | 6 | 176 | neutral |
| `g15_staged_base` (one `base` staged for all four a0/a2 sites) | 35 | 178 | |
| `g16_single_base` (one carrier, base + zero, both halves) | 31 | 178 | |
| `g18_four_locals` (base and zero in SEPARATE once-written locals) | 54 | 178 | |
| `g19_shared_zero` (per-half base locals + one shared zero local) | 54 | 178 | |
| `g20`/`g21_borrow_a0off` (borrow `a0_offset` AFTER its zero18 store) | 35 | 178 | position is load-bearing |
| `g22_perhalf_v0v3` (borrow the existing dead `v0`/`v3`) | 63 | 178 | |
| `g6_borrow_v0v3` (cross-half layout, existing `v0`/`v3`) | 61 | 178 | |
| `g5_cross_reuse` (two fresh carriers, base in one half + zero in the other) | **0** | 176 | match |
| `g17_perhalf_base` (two fresh carriers, base + zero within the same half) | **0** | 176 | match |
| `g23_stage_ret` (two fresh carriers, base + `ret`, within the same half) | **0** | 176 | match, no constant staging — the saved candidate |

Read together these price the lever exactly: the closing form needs, per loop half, a
carrier that (i) is a FRESH local — every existing-local borrow measured 178 — that (ii)
receives the a2 base `(s32)r4 - K` at the position between `a0_offset += ...` and
`s.zero18 = a0_offset;` (moving it anywhere later, or storing it through `a2_offset`
itself, is 178), and (iii) is written a SECOND time in the same block with another real,
immediately-consumed value. Any spelling that leaves the carrier single-set costs exactly
+2 instructions, which is the same `reg_n_sets == 1` price s2 measured for H8. The second
write does not have to be a constant: `nv = ret; s.ret = nv;` (`g23`) matches identically
to `nv = 0; s.zero10 = nv;` (`g17`), so the construct stages two real values and contains
no dead code at all.

### Frontier items 2 and 3 measured out

- Loop chassis (frontier item 2): `z1_while` (plain `while`, guard not duplicated) 25/174,
  `z2_for` 25/174, `z4_for_incslot` 25/174 — all lose the jump.c phantom slot and drop to
  174 instructions. `z3_guard_while` (guard duplicated outside AND as the `while` test)
  is **6/176**, i.e. floor-equivalent to the do-while but with a genuinely different
  emission order in the setup block (`li s3,256` and the `&D_8009B2E0` lui/addiu hoisted
  ahead of the first `lw v1,0(gp)`; objdump diff in `s3/z3.o` vs `s3/base_sb.o`). That
  structural difference is what made campaign 2 productive where campaign 1 was not.
- a2-base source expression (frontier item 3): `y2_neg_add` (`(s32)r4 + -0xC`) 6/176 and
  `y3_unsigned_sub` (`(s32)(r4 - 0xC)`) 6/176 are neutral; `y4_ptr_disp` (base carried as
  a `u8 *` walked by `- 0xC`) 8/177 and `y5_second_inv` (a second loop-invariant local
  holding `(s32)r4`) 8/177 both cost an instruction. Re-spelling the subtraction does not
  move the `addiu`; only the carrier's set-count and the statement position do.
- `y1_direct_b390` (`p_b390 = &D_8009B390;` instead of `p_b388 + 2`) is **43/174** — the
  `p_b388 + 2` spelling is load-bearing for `p_b388`'s ref count exactly as s1 recorded,
  and the resulting `addiu a3,a3,8` vs the target's `%lo(D_8009B390)` is a masked
  relocation addend, not a real residual.

- [s3] HONEST FLOOR 6 -> 0: memory/grind/func_8005D554/candidate.c (= s3/g23_stage_ret.c) scores 0 at 176/176 on HEAD main @ 1ddb0a8c. Not submitted — the closing construct is a ruling-request (fresh multi-set staging carrier).
- [s3] Permuter campaign 1 on the s1/s2 candidate chassis: 21,569 iterations, 0 finds (tmp/perm_5d554). Campaign 2 on the z3 guard+while chassis: 30,885 iterations, 1 find at permuter score 160 = sandbox 3 (tmp/perm_5d554_z3/output-160-1).
- [s3] A STANDALONE permuter workspace (typedef prelude + the function only, exact text1b pipeline flags) is byte-faithful to the full-TU build for this function; no full-TU wrapper is needed.
- [s3] The closing lever: per loop half, a FRESH local that receives `(s32)r4 - K` between `a0_offset += ...` and `s.zero18 = a0_offset;` and is written a second time with another real, immediately-consumed value. Existing-local borrows (v0/v3, a0_offset, a2_offset) all measure 178; single-set carriers all measure 178.
- [s3] Frontier item 2 (loop chassis): while/for without the duplicated guard are 25/174; the guard-duplicated while (z3) is 6/176 with a different setup emission order. Frontier item 3 (a2-base source expression): neutral (6) or +1 insn (8); killed.

- [s3] The `ret`-borrow probes (h1/h2/h4) all score 33. `ret` is the only existing local
  whose value can be made dead at the required staging position, and borrowing it compiles
  at 176 instructions (h1) — i.e. an existing-local borrow CAN satisfy the +2-free
  requirement, unlike the v0/v3/a0_offset borrows. What kills it is that freeing `ret`
  requires hoisting the `s.ret` struct store out of the target's fixed store slot
  (between the one14 and zero1C stores), which costs 33. Forms:
  `rejected/borrow-ret-hoisted-sret-store-scores-33.c`, probe log
  `tmp/grind/func_8005D554/s3/probe_log.txt`.
- [s3] Existing-local carrier space at the required staging position is now fully
  enumerated on this chassis: `a0_offset` (live there), `v0`/`v3` (dead; 61-63 at 178),
  `a2_offset` (31 at 178), `ret` (33 at 175-176), and the remaining locals
  (`i`, `stride`, `c100`, `c1`, `r4`, `r5`, `p_b2e0`, `p_b2ec`, `p_b388`, `p_b390`,
  `base_offset`) are loop-carried and cannot legally be overwritten there.

## s3b (permuter, post-Judge-FAIL re-dispatch) — 2026-09-08, HEAD main @ 4e7ad872

### Disposition of the s3 result
The s3 body that reached honest distance 0 used the fresh multi-write carriers `nv`/`nw` and was
FAILed by the Judge (docs/grind/decisions.md, entry "2026-09-08 22:31 — func_8005D554 — ruling
… **FAIL**"): non-membership in every frozen family, not a format defect. `candidate.c` is now
the clean 176/176 score-6 body; the FAILed body is preserved verbatim at
`rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c`. Because the driver keys
review verdicts by BODY, that text is permanently unsubmittable — comment changes do not make a
new body.

### The residual, stated exactly (unchanged, re-verified this session)
Ours, per loop half, after the `jal rand` delay slot `sw a0,0x28(sp)`:

    addiu a2,s4,-K      addiu a0,sp,0x10
    addiu a0,sp,0x10    addu  a1,zero,zero
    lw    v1,gp(...)    lw    v1,gp(...)
    move  a1,zero       addiu a2,s4,-K        <- target (asm/funcs/func_8005D554.s:4DEB4-4DEC0)

Two halves x 3 differing lines = the whole score of 6. Registers, frame (0x78) and instruction
count (176) already match the target everywhere (H12).

### s3b probe table (all `sandbox func_8005D554 --disable all`, forms in tmp/grind/func_8005D554/s3/)

| form | what it changes vs candidate.c | insns | score |
|---|---|---|---|
| `k0_base.c` | (= candidate.c, the clean score-6 body) | 176 | 6 |
| `k1_perhalf_acc_hoist.c` | per-half accumulators `a2a`/`a2b`, base init hoisted before `s.zero18` | 178 | 47 |
| `k2_perhalf_acc_nohoist.c` | per-half accumulators, no hoist (control) | 170 | 53 |
| `k3_half1_only.c` | half-1 only: `a2a` hoisted, half 2 unchanged | 178 | 23 |
| `g7_reorder_only.c` (s3) | ordinary-C hoist of the shared `a2_offset` base init | 178 | 31 |
| `k4_g7_noc1.c` | g7 + `s.one14 = 1;` (drop the `c1` holder) | 178 | 32 |
| `k5_g7_noc100.c` | g7 + literal `0x100` stores (drop the `c100` holder) | 178 | 33 |
| `k7_g7_noc1_noc100.c` | g7 + both holders dropped | 178 | 34 |
| `k8_k0_noc1.c` | candidate + `s.one14 = 1;` (control, no hoist) | 176 | 8 |
| `k9_g7_nopb390.c` | g7 + drop the `p_b390 = p_b388 + 2` local | 176 | 27 |
| `k11_g7_nopb2e0.c` | g7 + drop the `p_b2e0` local | 176 | 28 |
| `k10_k0_nopb390.c` | candidate + drop `p_b390` (control, no hoist) | 174 | 45 |
| `k12_k0_nopb2e0.c` | candidate + drop `p_b2e0` (control, no hoist) | 174 | 47 |

### SEAT BUDGET (the finding that explains the whole +2 column)
The target's prologue saves `s0-s7`, `fp`, `ra` into a 0x78 frame
(`asm/funcs/func_8005D554.s:4DD58-4DD84`) — nine callee-saved registers, exactly what our build
uses. Any spelling that hoists the a2-site base above the `rand()` call makes that value live
across a call and so demands a TENTH callee-saved seat; GCC evicts `p_b2e0` (or `p_b388`) and
rematerializes it with extra `lui/addiu` pairs in the pre-loop block, which is the +2 in every
178-instruction row above. Deleting one pointer local hands the seat back and restores 176
(k9/k11) — but the resulting code holds the base in `s0` across the call
(`s3/k11.asm` body insn 75: `addiu s0,s4,-12` in the rand delay slot), whereas the target holds
it in caller-saved `$a2` computed AFTER the call. So no source-level hoist is target-shaped.

### THE MECHANISM CORRECTION (supersedes the s3 H15 story)
s3 recorded the win as `reg_n_sets > 1` defeating `sched.c:2505 birthing_insn_p`. That reading is
wrong: `k1`'s `a2a` is written twice at exactly the s3 statement positions (base, then `+=`) and
scores 47/178. What separates the FAILed `g17`/`g23` bodies from `k1` is the presence of a
SEPARATE pseudo whose value is copied into the accumulator (`nv = r4 - K; … a2_offset = nv;`),
not the write count — i.e. some later pass folds the copy and re-emits the arithmetic at the
copy site, after the arg loads, which is precisely the LUID condition H4/H7 said no C spelling
could reach. H7's class kill is therefore contradicted by measurement and must be re-opened;
naming the responsible pass (cse.c / combine.c / flow.c) is frontier item P1.

### Permuter campaign (this session's mandated modality)
Workspace `tmp/perm_5d554_g7` (base.c = the `g7` ordinary-C hoist chassis, permuter base score
940), label `s3b-g7-ordinary-hoist-chassis`, 8 jobs, 24,879 iterations, ~13 minutes, stopped and
harvested in-session. Best find 655; no approach to the 160-class basin the `z3` campaign
reached. Chassis ledger for permutation: k0/candidate chassis spent (21,569 iters, 0 finds),
`z3` spent (30,891 iters, the banned `new_var` find), `g7` spent (24,879 iters, best 655).

- [s3] The s3 body that reached distance 0 was FAILed by the Judge (docs/grind/decisions.md, '2026-09-08 22:31 - func_8005D554 - ruling ... FAIL') for non-membership, not format. candidate.c has been reset to the clean 176/176 score-6 body; the FAILed text is preserved verbatim at memory/grind/func_8005D554/rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c and is permanently unsubmittable because the driver keys review verdicts by body.

- [s3] SEAT BUDGET: the target's prologue saves s0-s7, fp and ra into a 0x78 frame (asm/funcs/func_8005D554.s:4DD58-4DD84) - nine callee-saved registers, exactly what our build uses. There is no free seat, so any spelling that makes the a2-site base live across the rand() call pays +2 instructions of pointer rematerialization. This is a single fact that explains every 178-instruction row in the s1/s2/s3 probe tables.

- [s3] The target computes the a2-site base in caller-saved $a2 AFTER the rand call (asm/funcs/func_8005D554.s:4DEC0 addiu $a2,$s4,-0xC, three insns later than ours). Our candidate does the same thing three insns too early: ours emits [addiu a2][addiu a0,sp][lw v1][move a1] where the target emits [addiu a0,sp][addu a1,zero,zero][lw v1][addiu a2]. Two halves x 3 lines = the entire score of 6.

- [s3] MECHANISM CORRECTION: the s3 ledger attributed the distance-0 win to reg_n_sets > 1 defeating sched.c:2505 birthing_insn_p. k1_perhalf_acc_hoist writes its accumulator twice at exactly those statement positions (reg_n_sets == 2) and scores 47/178, so that reading is disproven. What separates the FAILed nv/nw forms is a SEPARATE pseudo copied into the accumulator, which implies a later pass folds the copy and re-emits the arithmetic at the copy site - i.e. exactly the LUID condition H7 class-killed as unreachable by any C spelling. H7's class kill is contradicted by measurement and should be treated as re-opened.

- [s3] Full s3b probe table (13 forms, insns and scores) is in memory/grind/func_8005D554/evidence.md under 's3b'; every form is on disk under tmp/grind/func_8005D554/s3/ and the five decisive ones are banked in memory/grind/func_8005D554/rejected/.

## s4 (enumerate) — 2026-09-08, HEAD main @ f95f6b8a, chassis floor re-measured = 6/176

### Chassis re-verification (first act of the session)
`candidate.c` applied to `src/text1b.c` and measured with
`sandbox func_8005D554 --disable all`: **score 6, build_insns 176, target_insns 176** — the
ledger's floor of 6 is intact on the current HEAD chassis, so every s1–s3 spelling conclusion
remains chassis-valid.

### The residual, re-proven instruction-by-instruction (artifact: s4/ours.dis, s4/pairdiff.py)
The sandbox object for `candidate.c` was disassembled and aligned against
`asm/funcs/func_8005D554.s`. The two bodies are identical in all 176 slots except six, which are
two copies of one 4-slot window (one per loop half). Half 1 (target indices T88–T91 vs ours
O88–O91):

    slot  target                    ours
    88    addiu a0, sp, 0x10        addiu a2, s4, -0xC     <- differs
    89    addu  a1, zero, zero      addiu a0, sp, 0x10     <- differs
    90    lw    v1, %gp_rel(...)    lw    v1, 0(gp)           identical
    91    addiu a2, s4, -0xC        move  a1, zero         <- differs

Half 2 is the same window with `-0x19` (target T134–T137 vs ours O129–O132). Everything else —
prologue, the four `rand` calls and their delay slots, all ten struct stores, both
`jal func_80073728` sites, the `sw a2, 0x2C(sp)` delay-slot store, the epilogue — is byte-aligned.
So the function's ENTIRE remaining distance is: our scheduler picks the a2 base insn FIRST out of
the {a2 base, a0 arg, a1 arg} triple, the target picks it LAST.

### THE EXHAUSTIVE SWEEP (1,224 spellings measured this session; artifacts s4/sweep{1,2,3}.json)

Sweep 1 — `tmp/grind/func_8005D554/enum1`, 168 spellings (generator `s4/gen1.py`).
Axes: 12 spellings of the a2 arithmetic (split-init base-first, split-init random-first,
three-way splits, single-expression reassociations, `= -K; += r4; += rnd`, `= r4-K; = rnd + a2`),
x 2 cast spellings of the `r4` read (`(s32)r4` vs the raw `u32 r4`), x 4 insertion points of the
a2 statements among the tail stores, x 2 positions of `s.zero1C = a2_offset` — applied identically
to both halves. Result histogram, ALL at 176 instructions:

    score 6  : 71 spellings   (every form whose FIRST written term is the `r4 - K` base:
                               a1, a3, a9, a11, a12 — i.e. the `addiu a2,s4,-K` insn survives)
    score 15 : 98 spellings   (every form whose first written term is the random product, or
                               that folds the sum into one expression: a2, a4, a5, a6, a7, a8, a10)

The cast axis and BOTH placement axes are completely inert — every one of the 8 (cast x placement
x zero1C) combinations of a given arithmetic form scores the same. No spelling of the a2
arithmetic scores below 6.

Sweep 2 — `tmp/grind/func_8005D554/enum2`, 256 spellings (generator `s4/gen2.py`).
Axis: the full power set of which of the seven movable struct stores
{`byte28`, `c24`, `c20`, `p1`, `zero10`, `one14`, `ret`} is relocated out of its current
basic block into the tail block that follows the last `rand()` call, x 2 positions for `i += 1`.
(`s.p0` is excluded: in half 2 it reads `D_800A3418 & 1` and moving it past a `rand()` changes the
value.) Result: 9 spellings at 6/176, the rest spread 10..30 (some at 177). Per-axis costs:

    move `zero10` alone           6/176   (neutral — preserves the tail store order)
    move `one14` alone           10/176   (transposes the tail order to one14,zero10,ret)
    move `ret` alone             10/176   (transposes to ret,zero10,one14)
    move `byte28` / `c24` / `c20` 10/176  each (crosses a rand() block boundary)
    move `p1`                    13/176
    move `i += 1`                 6/176   (neutral, either position)
    all seven moved            28-30/177

The only neutral moves are the ones that keep `zero10`,`one14`,`ret` in that order inside the
existing tail block. Two facts fall out: (a) the tail store order
`sw zero,0x20 / sw s6,0x24 / sw s1,0x1C` is byte-fixed and any transposition costs +4; (b) every
setup store's basic block is byte-fixed — pushing `byte28`/`c24`/`c20`/`p1` past a `rand()`
costs +4 each.

Sweep 3 — `tmp/grind/func_8005D554/enum3`, 800 spellings (generator `s4/gen3.py`).
The full cross product of every axis that sweeps 1 and 2 found NEUTRAL, stacked:
5 neutral arithmetic forms for the a0 site x 5 for the a2 site x 4 declaration scopes for
`a0_offset`/`a2_offset` (function top in either order, top of the `do` body, per-half nested
`{ }` blocks) x 2 positions of `s.zero1C` x 4 neutral tail-store move sets.
**Result: all 800 score exactly 6 at 176 instructions.** The score is completely invariant under
the stacked-neutral space, so frontier item P3's premise ("a stacked-neutral chassis has a
different emission order") is measurably false for these four axes: they do not perturb the
emission order at all, they are pure no-ops at the byte level.

### WHY THE REGION IS FLAT — the scheduler predicate, read out of cc1's own source
`rank_for_schedule` (`tools/gcc-2.7.2/sched.c:2408`) orders the ready list by, in order:
1. `INSN_PRIORITY` descending (`sched.c:2418`);
2. dependence class w.r.t. `last_scheduled_insn` (`sched.c:2424-2458`);
3. `INSN_LUID` ASCENDING — original insn order — as the stable tie-break (`sched.c:2464`).

`priority()` (`sched.c:1434`, the max at `sched.c:1499`) sets `INSN_PRIORITY` to the longest
dependence path from the insn to the end of the block. For ANY spelling in which the target's
`addiu a2,s4,-K` exists at all, that insn's path is
`addiu a2 -> addu a2,a2,v0 -> sw a2,0x2C(sp) -> jal`, strictly longer than the a0/a1 argument
setup insns' path (`addiu a0 -> jal`). So priority always ranks the a2 base ABOVE the arg setup,
and the LUID tie-break at `sched.c:2464` is never even consulted between them — the a2 base wins
on term 1. The one mechanism in cc1 that could raise the arg setups to parity is
`adjust_priority`'s birthing bump (`sched.c:2584`), but `birthing_insn_p` (`sched.c:2526`) returns
nonzero only when `reg_n_sets[REGNO(dest)] == 1`, and both hard arg registers are set at two call
sites in this function, so the bump is unavailable to them. (This also finally settles the s3 /
s3b dispute about `reg_n_sets`: the flag is real and it lives in `birthing_insn_p`, but it gates
a bump the arg-setup insns cannot get, not the accumulator's write count.)

The consequence is a structural one, not a spelling one: the target's ordering requires the a2
base insn to sit BELOW the call's argument setup in the insn stream, and since `expand_call`
emits the argument setup at the call statement — after every statement that computes the value
being stored into the struct — no ordinary source-level statement order can put it there. Only a
LATER pass re-emitting the arithmetic below the arg setup can (frontier P1: the `cse.c`
re-materialization / `combine.c` copy-fold that the Judge-FAILed `nv`/`nw` bodies triggered).
That is why 1,224 spellings of the region are quantized to exactly two values.

- [s4] Floor re-measured at 6/176 on HEAD main @ f95f6b8a with candidate.c applied, so every s1-s3 spelling conclusion stays chassis-valid.

- [s4] 1,224 complete function bodies were generated and swept this session (enum1 168, enum2 256, enum3 800). NOTHING scores below 6. The a2-site local spelling space collapses to exactly two values, 6 (any form whose first written term is the `r4 - K` base) and 15 (any random-first or single-expression reassociation); cast spelling, statement placement among the tail stores and the position of the s.zero1C store are all completely inert.

- [s4] The 800-form cross product of EVERY axis that measured neutral (5 a0 arithmetic forms x 5 a2 arithmetic forms x 4 declaration scopes for a0_offset/a2_offset x 2 zero1C positions x 4 tail-store move sets) is 800/800 at exactly 6/176 -- these axes are byte-level no-ops, not weak levers, which retires frontier item P3 (a stacked-neutral permuter seed would start in the already-spent k0 basin).

- [s4] MECHANISM, read out of cc1's own source: rank_for_schedule (sched.c:2408) sorts by INSN_PRIORITY (sched.c:2418), then dependence class, then INSN_LUID ascending (sched.c:2464). priority() (sched.c:1434/1499) is the longest path to the block end, so the a2 base insn (addiu a2 -> addu a2 -> sw a2,0x2C -> jal) ALWAYS outranks the a0/a1 argument setup (addiu a0 -> jal) on term 1 -- the LUID tie-break is never reached between them. adjust_priority's birthing bump (sched.c:2584) could raise the arg setup to max_priority, but birthing_insn_p (sched.c:2526) requires reg_n_sets[dest] == 1 and both hard argument registers are set at TWO func_80073728 call sites here. This is the correct home of the reg_n_sets flag that s3 mis-attributed.

- [s4] Two structural consequences for the next sessions: (a) the target order needs LUID(a2 base) > LUID(a0 arg setup), which no source statement order can give because expand_call emits arg setup at the call -- only a post-expand pass re-emitting the arithmetic can (frontier F1); (b) the untested alternative is to make the ARG SETUP birthing by giving the function a single func_80073728 call site in the source, which is ordinary C and never been measured (frontier F2).

- [s4] Floor re-measured at 6/176 (target_insns 176, build_insns 176) on HEAD main @ f95f6b8a with candidate.c applied, so every s1-s3 spelling conclusion remains chassis-valid.

- [s4] 1,224 complete function bodies were generated and swept this session (enum1 168, enum2 256, enum3 800). Nothing scored below 6.

- [s4] The a2-site local spelling space collapses to exactly two values: 6 for any form whose first written term is the `r4 - K` base, 15 for any random-first or single-expression reassociation. Cast spelling of the r4 read, statement placement among the tail stores and the position of the s.zero1C store are completely inert.

- [s4] The tail store order sw zero,0x20 / sw s6,0x24 / sw s1,0x1C is byte-fixed: transposing any two of them costs +4. Each setup store's basic block is byte-fixed too: pushing byte28, c24 or c20 past a rand() costs +4, and p1 costs +7.

- [s4] The 800-form cross product of every axis that measured neutral is 800/800 at exactly 6/176 -- these axes are byte-level no-ops, which retires the stacked-neutral permuter seed idea (frontier P3).

- [s4] MECHANISM (read out of cc1's own source, not inferred): rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) sorts the ready list by INSN_PRIORITY (sched.c:2418), then dependence class, then INSN_LUID ascending (sched.c:2464). priority() (sched.c:1434, max at sched.c:1499) is the longest dependence path to the block end, so the a2 base insn always outranks the a0/a1 argument setup on term 1 and the LUID tie-break is never reached between them.

- [s4] adjust_priority's birthing bump (sched.c:2584) is the only mechanism in cc1 that could raise the argument setup to parity, and birthing_insn_p (sched.c:2526) gates it on reg_n_sets[REGNO(dest)] == 1. Both hard argument registers are set at TWO func_80073728 call sites in this function, so the bump cannot fire. This is the correct home of the reg_n_sets flag that s3 mis-attributed to sched.c:2505 birthing_insn_p's write-count of the accumulator.

- [s4] Consequence: the target's order requires INSN_LUID(a2 base) > INSN_LUID(a0 argument setup). expand_call emits argument setup at the call statement, i.e. after every statement that computes the value stored into the struct, so no source-level statement order can produce that inequality -- which is exactly why only the Judge-FAILed nv/nw bodies (a copy re-materialized by a post-expand pass) ever reached it, and why 1,224 spellings are quantized to two values.

## s5 (synthesis, 2026-09-09) — dumps read, s3/s4 mechanism stories corrected

Chassis re-measured at dispatch: `candidate.c` applied to `src/text1b.c` on HEAD main
@ 4f43e5cf scores **6 / 176 build_insns** under `sandbox func_8005D554 --disable all`
(target_insns 176). The floor is unchanged from s2/s3b/s4.

### CORRECTION 1 — s4's closing claim is false
s4 recorded: "the target's order requires INSN_LUID(a2 base) > INSN_LUID(a0 argument setup)
... no source-level statement order can produce that inequality." The dumps show the opposite.
In the Judge-FAILed `nv`/`nw` body (the only measured body at distance 0) the a2 base insn is
`(insn 198 ...)`, born BEFORE the third `rand` call (`call_insn 204`), i.e. with a LUID far
BELOW the argument setup's — and sched1 still schedules it last, sinking it across the call to
sit immediately after the `D_800A3418` load (`dumps_nvnw/f.lreg`: the insn is chained
`208 -> 198 -> 209`). The residual is therefore not a LUID inequality problem at all; the
scheduler's placement is driven by the base pseudo's live range and dependence notes, not by
its source position. Any future session that re-derives the "must out-LUID the arg loads"
framing is re-deriving a disproven claim.

### CORRECTION 2 — s3's `reg_n_sets`/`birthing_insn_p` story is dead twice over
s3 H15 attributed the FAILed body's win to `reg_n_sets > 1` on the carrier. s3's own H19
already contradicted it (a per-half accumulator with `reg_n_sets == 2` scores 47). s5 finishes
it: `grep -c 'set (reg/v:SI 83)' dumps_nvnw/f.combine` returns **1** — after `combine.c` the
winning carrier is SINGLE-set, with a `REG_DEAD` note at the consuming `addu`
(`dumps_nvnw/f.combine:572-574`). The source-level second write (`nv = ret; s.ret = nv;`) is
erased by combine; its only job is to exist while `loop.c` runs.

### The actual causal chain (H23), pass by pass
1. `loop.c` — a loop-invariant value written ONCE is a movable and is hoisted out of the loop.
   Proof: with a fresh single-set per-half carrier, the base insn is renumbered and relocated
   into the pre-loop block (`dumps_freshsingle/f.combine:339` — `(insn 393 133 394 (set (reg/v:SI 83)
   (plus (reg/v:SI 78) (const_int -12))))`), `dumps_freshsingle/f.greg:391` seats it in `$a2`
   there, and the loop body is 178 instructions (score 54). A carrier written TWICE in source
   is not a movable and stays in the loop.
2. `combine.c` — erases the redundant second write and the `a2_offset = carrier` copy, folding
   the copy into the consuming add so the surviving pair is
   `(set regB (plus r4 -K))` ... `(set regA (plus regB rnd))`. The base insn is NOT merged into
   the add because the base pseudo is multi-set when combine examines it.
3. `sched1` — with `regB` single-set, single-use and dying at the add, the base insn sinks to
   immediately after the `D_800A3418` load: the target's slot.
4. `local-alloc` — `regB`'s live range is now confined to one loop half, so it is seated in
   caller-saved `$a2`, matching `asm/funcs/func_8005D554.s:4DEC0`. No callee-saved seat is
   consumed, so no pointer local is evicted and the body stays at 176 instructions.

Every step of that chain is reproduced or refuted by a banked dump; nothing here is inferred.

### What the sixteen new measurements say (H24)
The a2-site shape axis is a byte-level no-op and the seat axis is a fixed +/-2:

| spelling | plain | `p_b390` freed | `p_b2e0` freed | both freed |
|---|---|---|---|---|
| base after `s.zero18` store, direct-store sum (v1) | 31/178 | 27/176 | 28/176 | 53/174 |
| base at the `nv` slot, direct-store sum (v2) | 31/178 | 27/176 | 28/176 | 53/174 |
| base at the `nv` slot, RMW accumulate (v4) | 31/178 | 27/176 | 28/176 | 53/174 |
| base at the natural late slot, direct-store sum (v3) | 10/176 | — | — | — |
| base after `s.zero18`, RMW accumulate (v5) | 31/178 | — | — | — |
| sum into `a0_offset` instead of `a2_offset` (v6/v7) | 30/178 | — | — | — |
| FRESH single-set per-half carrier (LICM-hoisted) | 54/178 | 32/176 | 35/176 | — |
| `a0_offset` borrowed for the base | 35/178 | 33/176 | 32/176 | — |
| `a2_offset` borrowed + `X = ret; s.ret = X;` restage | 30/178 | 30/176 | 27/176 | — |
| `a0_offset` borrowed + `X = ret; s.ret = X;` restage | 35/178 | 33/176 | 32/176 | — |

Reading: the direct-store vs accumulate distinction, and the choice of which local receives
the sum, are exact no-ops (v1 == v2 == v4 == v5 to the byte). The ONLY thing that moves the
score is where the base pseudo lives. Freeing a pointer local buys the two instructions back
but leaves the base in a callee-saved register across the `rand` call
(`dumps_v2/f.greg:455` — `(set (reg/v:SI 16 s0) (plus (reg:SI 20 s4) (const_int -12)))`),
so those 176-instruction forms are structurally further from the target than the 6-point
candidate is.

Critically, the `X = ret; s.ret = X;` restage — the exact source construct that erases the
second set in the FAILed body — does NOT reproduce the win when `X` is an existing
function-scope local. That is the direct experimental separation between the Judge-banned
construct and the sanctioned existing-local borrow quadrant: the borrow fails not on the write
count but on the live range, because every function-scope local here is read again in the
other loop half.

### s4 frontier F2 (single call site) is closed on target evidence
`grep -n 'jal' asm/funcs/func_8005D554.s` — nine `jal`, seven to `rand`, and exactly two to
`func_80073728` (lines 108 and 155). A source with one call site emits one `jal`; GCC 2.7.2
does not unroll at `-O2`. The `birthing_insn_p` bump on the argument registers is therefore
unreachable by construction, not merely unmeasured.

### Kill re-audit
`rejected/shared-a2-base-direct-store-scores-10.c` re-measures 10/176 (as `s5/enum/v3.c`) and
`rejected/hoist-plus-freed-pb390-seat-scores-27.c` re-measures 27/176 (as
`s5/enum2/v4_nob390.c`) on the current chassis — both banked kills reproduce.
`tools/fake_ablate.py` on the FAILed body reports "no FAKE-annotated constructs found", so no
banked measurement on this function was taken with a FAKE carrier occupying the a2 pseudo.

### Artifacts
`tmp/grind/func_8005D554/s5/` — `dumps_cand/`, `dumps_nvnw/`, `dumps_v2/`,
`dumps_freshsingle/` (each with `f.rtl f.jump f.cse f.loop f.combine f.flow f.lreg f.greg
f.sched f.sched2` per-function extracts), `chain.py` (post-sched1 window reader), `ext.sh`,
`gen.py gen2.py gen3.py gen4.py gen5.py`, `enum{,2,3,4,5}/` (25 complete bodies),
`sweep{,2,3,4,5}.json`.

## s5b (synthesis, 2026-09-09) — 55 new measured spellings; floor holds at 6/176

- [s5b] The floor was RE-MEASURED at 6/176 on `memory/grind/func_8005D554/candidate.c` on
  HEAD main @ 4f43e5cf; the control body in every sweep this session (`s5b/enum/q2_late_none.c`,
  `s5b/enum2/shared_accum_before.c`, `s5b/enum4/dowhile_split_tail.c`) reproduces 6/176 exactly.

- [s5b] The 4-local chassis (separate `a0a`/`a2a` and `a0b`/`a2b` offset locals per loop half)
  is a byte-level no-op at the floor: 6/176, identical to the 2-local candidate. Early-birthing
  the a2 base on top of it is strictly worse than on the 2-local chassis (47/178 in all four
  measured cells vs 31/178). s5 frontier Q1 is retired.

- [s5b] Carrier IDENTITY for the a2 base is invisible in the accumulate shape: the shared
  `a2_offset`, one fresh local shared by both halves, and two fresh per-half locals all give
  6/176 when the base is born late and the value is accumulated. In the direct-store shape
  (`s.zero1C = X + rnd`) the same three carriers give 10/176, 10/176 and 54/178 — the last
  because a fresh PER-HALF carrier is set exactly once in the loop and `loop.c` hoists the
  invariant `(s32)r4 - K` out of the loop entirely.

- [s5b] The `loop.c` gate is now named exactly: a movable is moved when
  `threshold * savings * lifetime >= insn_count` (`tools/gcc-2.7.2/loop.c:1631`), with
  `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (`loop.c:532`),
  `savings = n_times_used[regno]` and `lifetime = luid(last use) - luid(first def)`
  (`loop.c:790-793`). Every spelling measured across s3/s4/s5/s5b attacks the SET COUNT (which
  decides whether a movable is recorded at all); `insn_count`, `lifetime` and the invariance
  test itself are untried.

- [s5b] Reproducing the permuter's only sub-6 find (`tmp/perm_5d554_z3/output-160-1`, sandbox 3:
  a carrier that holds the half-1 a2 base and is later re-written to 0 to feed `s.zero10`) with
  the dead PARAMETERS `arg0`/`arg1` or the dead locals `v0`/`v3` as the carrier costs 1-3
  instructions in all nine measured bodies (25/178, 37/177, 42/178, 43/177 x3, 43/179, 50/179,
  61/178 x2). The sanctioned staged-value quadrant does not reach 176 instructions here.

- [s5b] The LOOP CHASSIS is measured out. `do { } while`, the s2 `z3` guard-duplicated `while`,
  and `for (;;) { ...; if (!cond) break; }` produce the same 6/176 in every measured column;
  `jump.c`'s `duplicate_loop_exit_test` normalises all three to the same RTL. An explicit
  label + `goto` loop loses the phantom compare pseudo and drops to 175 instructions (37/175).

- [s5b] Routing the call's literal second argument through a zero constant-holder local costs
  2-3 instructions (16/178 function-top holder, 19/179 guard-block holder, 21/179 when the
  holder also feeds `s.zero10`, 30/179 for both). The target's `addu $a1, $zero, $zero` is
  exactly what GCC emits for a literal `0` argument, which the candidate already emits.

- [s5b] The callee `func_80073728` genuinely READS the +0x10 and +0x14 fields of its argument
  struct (`asm/funcs/func_80073728.s:314` `lw $a1, 0x10($s2)`, `:317` `lw $v0, 0x14($s2)`,
  `:323` `sw $t0, 0x14($s2)`), so `s.zero10 = 0` and `s.one14 = 1` are real input stores of
  literal constants in the original source — not staging sites for another value. That removes
  the semantic justification for the permuter's shared-pseudo trick.

- [s5b] The residual window is unchanged and precisely located: ours emits
  `[addiu a2,s4,-K][addiu a0,sp,0x10][lw v1,gp][move a1,zero]`, the target emits
  `[addiu a0,sp,0x10][addu a1,zero,zero][lw v1,gp][addiu a2,s4,-K]`
  (`asm/funcs/func_8005D554.s:93-96` and `:139-142`), twice — once per loop half.

- [s5] Floor RE-MEASURED at 6/176 on memory/grind/func_8005D554/candidate.c on HEAD main @ 4f43e5cf; every sweep this session carried a control body reproducing 6/176 exactly.

- [s5] The previous session's work (H23-H26 pass attribution, dumps under tmp/grind/func_8005D554/s5/) survives on disk and was re-read rather than re-derived; that session was discarded on an outcome-JSON kill-scope wording defect, not a measurement defect.

- [s5] The loop.c hoist gate is now named exactly: a movable is moved when threshold * savings * lifetime >= insn_count (tools/gcc-2.7.2/loop.c:1631), with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) (loop.c:532), savings = n_times_used[regno] and lifetime = luid(last use) - luid(first def) (loop.c:790-793).

- [s5] Every spelling measured across s3/s4/s5/s5b attacks the carrier's SET COUNT, which decides only whether a movable is recorded at all; insn_count, lifetime and the invariant_p test itself have never been attacked.

- [s5] Carrier identity for the a2 base is invisible in the accumulate shape (shared local, fresh shared local and fresh per-half locals all give 6/176) and only decides 176 vs 178 in the direct-store shape.

- [s5] The loop chassis is measured out: do-while, the z3 guard-duplicated while, and for(;;)+break are all 6/176 because jump.c's duplicate_loop_exit_test normalises them to one RTL stream; label+goto drops to 175 instructions.

- [s5] The residual window is unchanged and precisely located: ours emits [addiu a2,s4,-K][addiu a0,sp,0x10][lw v1,gp][move a1,zero]; the target emits [addiu a0,sp,0x10][addu a1,zero,zero][lw v1,gp][addiu a2,s4,-K] at asm/funcs/func_8005D554.s:93-96, repeated once per loop half.

- [s5] src/text1b.c was left byte-clean at the end of the session (func_8005D554 still INCLUDE_ASM); every measurement went through tools/sweep_variants.py, which restores the file.

## s6 (solver, 2026-09-09, HEAD main @ 07aadcc3) — the residual is typed, and the statement-order axis is closed with a predicate

Floor RE-MEASURED at **6/176** with candidate.c applied to src/text1b.c.

**Typing.** `python3 tools/ra_solver/inverse_compose.py classify text1b func_8005D554
--target-object build/src/text1b.o --ours-object tmp/sandbox/func_8005D554/text1b.o` reports
`FIRST DIVERGENCE: SCHED — same instructions and registers, 6 slot(s) in a different order`
(honest 176, target 176). There is no PRE-RA component and no RA component: register-blanked
multisets are equal AND the register-bearing texts are equal as a multiset. Every future session
can take this as settled — the whole residual lives in `sched.c`.

**The goal, in UIDs.** The two rotations are symmetric. Half 1 (uids from
`tmp/sched_solver_work/text1b.i.dbr`):

| uid | insn | pass-1 LUID (= source order) | pass-2 LUID (= sched1 output) |
|---|---|---|---|
| 201 | `jal rand` | 26 | 24 |
| 205 | `lw v1, D_800A3418` | 28 | 27 |
| 211 | `addiu a2, s4, -0xC` (the a2 base) | 31 | 25 |
| 225 | `addu a2, a2, v0` (the accumulate) | 37 | 38 |
| 228/231/234 | `s.zero10` / `s.one14` / `s.ret` stores | 38/39/40 | 29/30/31 |
| 237 | `s.zero1C = a2_offset` | 41 | 40 |
| 240 | `addiu a0, sp, 16` (call arg 1) | 42 | 26 |
| 242 | `move a1, zero` (call arg 2) | 43 | 28 |
| 244 | `jal func_80073728` | 44 | 41 |

Half 2 is the same shape with 306/310/316/331/334/337/340/343/346/348/350.
Ours emits `211, 240, 205, 242`; the target emits `240, 242, 205, 211`
(objdump positions 88-91 of `build/src/text1b.o`). In `perturb.py`'s pick space (reverse
emission) the goal is `211 before 205 before 242 before 240`.

**Depth-1, all atoms.** `perturb.py --pass 1 --block 6 --depth 1 --max 500` over all 12,558
single atoms (`add_dep`, `del_dep`, `luid`, `luid_move`, `cost`) returns exactly 102 hits and
every one is a LUID move of insn 211 to a slot at or after uid 244. Zero `add_dep`, zero
`del_dep`, zero `cost` hits. The cheapest hit is `luid_move 211 -> immediately before 244`.

**Why that is unspellable.** `sched.c:2464` makes `INSN_LUID (tmp) - INSN_LUID (tmp2)` the last
tie-break in `rank_for_schedule`, and here the four insns are mutually independent, all at
INSN_PRIORITY 3, all in the same dependence class against the last-scheduled insn — so LUID
alone decides. Insn 211 would have to be born after 240 and 242, which `expand_call` emits at the
call, while 211's own consumer 225 is born at LUID 37. No C statement order can do that.

**The exhaustive check.** `tmp/grind/func_8005D554/s6/stmtperm.py` enumerates every
dependence-legal permutation of the eight half-1 source statements (S1 `D_800A3418 ^= rand()`
tail, S2 a2 base, S3 a2 accumulate, S4/S5/S6 the three struct stores, S7 the `s.zero1C` store,
S8 the call), 420 of them, and simulates sched1 for each: 0 reach the goal.
`tmp/grind/func_8005D554/s6/compose.py` replays the same 420 through the FULL funnel — sched1,
then sched2 with its LUIDs rebuilt from that permutation's sched1 emission order — and again
reaches 0. compose.py self-checks that the control replay reproduces the real sched2 dump
bit-for-bit (`real sched2 exact: True`, `replay == real: True`), so this is an exact result, not
a modelling approximation.

**Compiler agreement.** Four bodies swept with `tools/sweep_variants.py`
(`tmp/grind/func_8005D554/s6/sweep.json`): control 6/176;
`v3_a2base_last_legal_slot` (a2 base moved to the last slot the dependence graph allows, right
after the zero10/one14/ret stores — the maximum LUID the model says is reachable) **6/176**,
exactly as predicted; `v4_base_folded_into_store` 15/176; `v2_struct_via_pointer` 60/179.

**What this leaves.** Since the insn multiset and the register assignment are both already
target-identical, and statement order is now closed with a predicate, the remaining lever must
change the DEPENDENCE GRAPH or the INSN_PRIORITY values while keeping the multiset — i.e. a
different value-flow, not a different order. The depth-1 search says no single edge does it; the
depth-2 window search over the same eight statements' insns was launched this session
(`tmp/grind/func_8005D554/s6/depth2_window.py`) and is the natural first probe for s7.

- [s6] Floor RE-MEASURED at 6/176 this session on HEAD main @ 07aadcc3 with candidate.c applied to src/text1b.c; the chassis has not moved since s5.

- [s6] inverse_compose.py classify types the entire residual as SCHED: 176 honest vs 176 target instructions, register-blanked multisets equal, register-bearing texts equal as a multiset, 6 slots in a different order. No PRE-RA and no RA component remains.

- [s6] The half-1 window in RTL uids (tmp/sched_solver_work/text1b.i.dbr): 201 jal rand, 205 lw v1 D_800A3418, 211 addiu a2 s4 -0xC, 217-225 the shift chain and a2 += v0, 228/231/234 the zero10/one14/ret stores, 237 the zero1C store, 240 addiu a0 sp 16, 242 move a1 zero, 244 jal func_80073728. Half 2 is the same shape at 306/310/316/331/334/337/340/343/346/348/350.

- [s6] Pass-1 LUIDs (source order) are 205=28, 211=31, 225=37, 228=38, 231=39, 234=40, 237=41, 240=42, 242=43, 244=44; pass-2 LUIDs (sched1 output order) are 201=24, 211=25, 240=26, 205=27, 242=28, 228=29, 231=30, 234=31, 206=32, 225=38, 237=40, 244=41. sched2 is a no-op in this window: the final emission order equals sched1's output.

- [s6] Depth-1 perturb over ALL 12,558 single atoms of sched1 block 6 yields 102 hits, all of them LUID moves of uid 211 to a slot at or after uid 244, and zero hits from add_dep, del_dep or cost.

- [s6] All 420 dependence-legal permutations of the eight half-1 source statements fail at sched1 AND after a sched2 replay whose control is self-checked bit-for-bit against the real sched2 dump.

- [s6] Four bodies swept against the real compiler: control 6/176, v3_a2base_last_legal_slot 6/176 (the model-predicted maximum LUID placement, byte-level no-op as predicted), v4_base_folded_into_store 15/176, v2_struct_via_pointer 60/179.

- [s6] The pass-2 search shows the rotation is completed by sched2 for free if sched1 emits uid 211 after the three struct stores -- so a lever that changes only sched1's OUTPUT (not the source order) is sufficient; it does not have to reach the final order directly.

## s7 (forensics, 2026-09-09, HEAD main @ 8a6f96b1) — the residual measured inside the compiler: rank_for_schedule's first two criteria are structurally tied, and only INSN_LUID is left

Floor RE-MEASURED at **6/176** with candidate.c applied to src/text1b.c (`sandbox func_8005D554
--disable all` -> `"score": 6`).

**Tool health.** The instrumented cc1 is `tools/gcc-2.7.2/cc1` (NOT `tools/gcc-2.7.2/build/cc1`,
which carries no `BB2_*` hooks — confirmed by `grep -ao "BB2_[A-Z_]*"` on both binaries).
`tmp/grind/func_8005D554/s7/run_dump.sh` pins it explicitly. SELF-CHECK: the assembly the
instrumented cc1 emits for the whole TU is **byte-identical** to the assembly the build cc1 emits
(`diff -q s7/dumps/text1b.s s7/dumps2/text1b.s` -> identical), so every number below is a
measurement of the real build, not of a differently-behaving binary.

**The rank function, in full.** `rank_for_schedule` (tools/gcc-2.7.2/sched.c:2408) decides the
order of the ready list with exactly three criteria, in order:
  1. `INSN_PRIORITY (tmp) - INSN_PRIORITY (tmp2)` (sched.c:2418),
  2. the last-scheduled-insn CLASS (1 = data dep on last scheduled, 2 = anti/output dep,
     3 = independent-or-latency-1; sched.c:2429),
  3. `INSN_LUID (tmp) - INSN_LUID (tmp2)` (sched.c:2464).
Higher priority wins, then higher class wins, then higher LUID wins (the ready array is sorted so
that the larger-LUID insn is picked first, and GCC 2.7.2 schedules the block bottom-up, so
picked-first == emitted-last).

**Criterion 1 measured (BB2_PRIO_DEBUG, `s7/prio.log`, 45,426 lines).** Pass-1 priorities for the
half-1 window, with the derivation each one came from:

| uid | insn (RTL from s7/dumps2/text1b.flow) | priority | why |
|---|---|---|---|
| 201 | `jal rand` | 3 | max over its anti-deps on the preceding block insns |
| 205 | `(set (reg 140) (mem (symbol_ref "D_800A3418")))` | 3 | anti-dep on 201, cost 1 |
| 206 | `(set (reg 141) (xor (reg 140) (reg 139)))` | **4** | **data dep on the LOAD 205, cost 2** |
| 211 | `(set (reg/v 82) (plus (reg/v 78) (const_int -12)))` — the a2 base | 3 | anti-dep on 201, cost 1 |
| 217/220/222 | the shift chain | 4 | inherited from 206 |
| 225 | `(set (reg/v 82) (plus (reg/v 82) (reg 148)))` | 4 | data dep on 223 |
| 228/231/234 | the `zero10`/`one14`/`ret` stores | 3 | anti-dep on 201, cost 1 |
| 237 | `(set (mem (fp+44)) (reg/v 82))` — the `zero1C` store | 4 | data dep on 225 |
| 240 | `(set (reg 4 a0) (plus (reg 30 $fp) (const_int 16)))` | 3 | anti-dep on 201, cost 1 |
| 242 | `(set (reg 5 a1) (const_int 0))` | 3 | anti-dep on 201, cost 1 |
| 244 | `jal func_80073728` | 4 | inherited from 206/208 |

Half 2 is the same shape shifted by the block's accumulated priority (the uid map in the s6 ledger
is off by one entry: **343 is half-2's `s.zero1C` store, 346 is `a0 = fp+16`, 348 is `a1 = 0`**):
306 = 6, 310 (the lw) = 6, **316 (the a2 base) = 6**, 331 = 7, **343 = 7**, **346 = 6**, **348 = 6**,
350 = 7. So in both halves the four contested insns — the load, the a2 base, the `a0` setup and
the `a1` setup — sit at *exactly the same* priority.

**Why the priority tie is structural.** `priority()` (sched.c:1434) computes
`prev_priority = priority (x) + insn_cost (x, prev, insn) - 1` (sched.c:1497) over the insn's
LOG_LINKS, i.e. over its PREDECESSORS, and takes the max. A call sets `reg_pending_sets_all`
(sched.c:1991/2095/2236), so **every** register-setting insn after a call in the same block gets an
anti-dependence on that call. `insn_cost` is 1 for every MIPS ALU insn and 2 only for a load's
data-dependent consumer. Therefore every insn after the last call that is not a load consumer
inherits *exactly* the call's priority, and the only way to be one higher is to be a data consumer
of a load. The a2 base is `reg78 - K` where reg78 is the loop-invariant `r4` living in a
callee-saved register: it has no load predecessor in any spelling that keeps the multiset.

**The variable-reuse route to a priority bump is measured DEAD, not argued dead.** Half-2's a2 base
insn 316 *already* carries the anti-deps that C-level variable reuse manufactures — `pred=225`
(pri 4) and `pred=237` (pri 4), because `a2_offset` (pseudo 82) is shared by both halves — and both
contribute 4, which is dominated by the `pred=306` call contribution of 6. A reused-variable
anti-dep always points BACKWARD to an insn with a lower accumulated priority than the nearest
preceding call, so it can never raise an insn above the call's level.

**Criterion 2 measured (BB2_RANK_DEBUG, `s7/rank.log`, 9,689 lines).** Across **203** rank
comparisons whose two endpoints both carry window uids, the class delta is **0 in every single
one** (`cls=3 x=... cls2=3 val=0`). The class test at sched.c:2429 assigns class 3 whenever the
insn is not a LOG_LINK predecessor of `last_scheduled_insn` *or* its cost is 1; every insn in the
window is an ALU insn or a store with cost 1, so all of them are class 3 against every
last-scheduled insn that occurs. Class can never separate them.

**What is left.** Criterion 3, `INSN_LUID`, alone. The measured LUIDs are 205=28, 211=31, 240=42,
242=43, and the target requires 211 to be picked before 240/242, i.e. to hold the LARGEST LUID of
the four. 240 and 242 are emitted by `expand_call` immediately before the call insn, and the a2
base must be computed before the `s.zero1C` store that the call reads, so 211's LUID is bounded
above by 237's. This is the same wall s6 found with its model — s7 confirms it *inside the
compiler*, and additionally rules out the two criteria that sit ABOVE LUID in the same comparison.

**Structural sweep (`s7/sweep.json` via tools/sweep_variants.py, control 6/176).** Frontier item 3
is measured out: `v3_structptr_mixed` (a TU-local `struct EffEnt { u8 a[0xC]; u8 b[0xC];
u8 rest[0x24]; }` pointer replacing `base_offset`, multiset preserved at 176) scores **10/176**;
`v2_structptr` (the full replacement of the byte-pointer model) scores **43/172** — it loses four
instructions. `v4_a2base_born_after_shift` (`a2_offset = shifted; a2_offset += (s32)r4 - K;`, the
maximum-LUID legal birth point for insn 211) scores **15/176**.

- [s7] Floor RE-MEASURED at 6/176 on HEAD main @ 8a6f96b1 with memory/grind/func_8005D554/candidate.c applied to src/text1b.c.
- [s7] tools/gcc-2.7.2/cc1 (the BB2-instrumented build) emits byte-identical assembly to tools/gcc-2.7.2/build/cc1 for the whole text1b TU, so its dumps describe the real build exactly.
- [s7] Measured pass-1 INSN_PRIORITY: 205=3, 211=3, 240=3, 242=3 (half 1) and 310=6, 316=6, 346=6, 348=6 (half 2). The four contested insns tie in BOTH halves.
- [s7] Priority is inherited only from LOG_LINK predecessors via priority(pred) + insn_cost(pred) - 1 (sched.c:1497); insn_cost is 2 only for a load's data-dependent consumer, so the a2 base (an addiu off a callee-saved loop invariant) cannot exceed the priority of the call it hangs off.
- [s7] Half-2's a2 base insn 316 already carries the variable-reuse anti-deps (pred=225 pri 4, pred=237 pri 4) that a C-level variable borrow would manufacture, and both are dominated by the call anti-dep (pred=306 pri 6). The reused-variable priority lever is measured inert.
- [s7] Across 203 rank_for_schedule comparisons in the window the class criterion (sched.c:2429) returns delta 0 every time — all endpoints are class 3. Only INSN_LUID (sched.c:2464) is left to decide.
- [s7] The corrected half-2 uid map: 343 is the s.zero1C store (priority 7), 346 is a0 = fp+16 (6), 348 is a1 = 0 (6). The s6 ledger's half-2 list was shifted by one entry; 316 <-> 211, 331 <-> 225, 343 <-> 237, 346 <-> 240, 348 <-> 242, 350 <-> 244.
- [s7] Frontier item 3 (the p_b2e0 declaration-pun object model) is MEASURED OUT: a TU-local struct pointer replacing the byte-pointer model scores 43/172 in full form and 10/176 in the mixed form that keeps the multiset — never below the control's 6/176.
- [s7] Reassociating the a2 sum so the base subtraction is born after the shift chain scores 15/176 — the maximum-LUID legal birth point for insn 211 costs 9 points and still does not reach the target order, matching s6's v3_a2base_last_legal_slot result from the other direction.
- [s7] src/text1b.c was left byte-clean (func_8005D554 back to INCLUDE_ASM); the only tracked-file change from this session is the three new rejected/ forms.

- [s7] Floor RE-MEASURED at 6/176 on HEAD main @ 8a6f96b1 with memory/grind/func_8005D554/candidate.c applied to src/text1b.c ('sandbox func_8005D554 --disable all' -> "score": 6).

- [s7] TOOL HEALTH: the instrumented cc1 is tools/gcc-2.7.2/cc1, NOT tools/gcc-2.7.2/build/cc1 — 'grep -ao "BB2_[A-Z_]*"' finds 15 hook names in the former and none in the latter, and engine/buildconfig.py:19 points CC1 at build/cc1, so any -da run that does not override CC1 produces NO BB2 diagnostics. tmp/grind/func_8005D554/s7/run_dump.sh pins the instrumented binary.

- [s7] SELF-CHECK: the assembly the instrumented cc1 emits for the whole text1b TU is byte-identical to the assembly the build cc1 emits (diff -q s7/dumps/text1b.s s7/dumps2/text1b.s), so all s7 numbers describe the real build.

- [s7] rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) has exactly three criteria in order: INSN_PRIORITY (sched.c:2418), the last-scheduled-insn CLASS (sched.c:2429), INSN_LUID (sched.c:2464). GCC 2.7.2 schedules the block bottom-up, so picked-first == emitted-last, and the ready array is ordered so the larger-LUID insn is picked first.

- [s7] Measured pass-1 INSN_PRIORITY for the contested insns: half 1 205=3, 211=3, 240=3, 242=3; half 2 310=6, 316=6, 346=6, 348=6. The tie is present in both halves, which is why the residual is exactly two identical 3-insn rotations.

- [s7] The only priority-4 insns in the half-1 window are 206 (the xor, a data consumer of the load 205 at cost 2), 217/220/222 (its shift-chain downstream), 225, 237 and 244. Priority can only be raised by consuming a load; the a2 base is an addiu off the callee-saved loop invariant r4 (reg/v 78) and consumes none.

- [s7] Half-2's a2 base insn 316 already carries the anti-deps that a C-level variable borrow manufactures (pred=225 pri 4, pred=237 pri 4, because a2_offset is the shared pseudo reg/v 82) and both are dominated by the call anti-dep pred=306 pri 6, so its final priority is 6 — identical to the argument-setup insns 346 and 348. The variable-reuse priority lever is measured inert, not argued inert.

- [s7] Across 203 rank_for_schedule comparisons whose both endpoints carry window uids, the class criterion returns delta 0 every time (all endpoints class 3). Only INSN_LUID is left to decide the order of {205, 211, 240, 242}.

- [s7] CORRECTION to the s6 ledger: the half-2 uid list was shifted by one entry. The true map is 306<->201, 310<->205, 316<->211, 331<->225, 343<->237 (the s.zero1C store, priority 7), 346<->240 (a0 = fp+16, priority 6), 348<->242 (a1 = 0, priority 6), 350<->244. RTL identities read from tmp/grind/func_8005D554/s7/dumps2/text1b.flow.

- [s7] RTL identities of the window (s7/dumps2/text1b.flow): 205 = (set (reg 140) (mem (symbol_ref "D_800A3418"))); 211 = (set (reg/v 82) (plus (reg/v 78) (const_int -12))); 225 = (set (reg/v 82) (plus (reg/v 82) (reg 148))); 237 = (set (mem (fp+44)) (reg/v 82)); 240 = (set (reg 4 a0) (plus (reg 30 $fp) (const_int 16))); 242 = (set (reg 5 a1) (const_int 0)); 244 = the func_80073728 call.

- [s7] Structural sweep against the real compiler (tmp/grind/func_8005D554/s7/sweep.json): control 6/176, v1_control 6/176, v3_structptr_mixed 10/176, v4_a2base_born_after_shift 15/176, v2_structptr 43/172.

- [s7] src/text1b.c was left byte-clean (func_8005D554 back to INCLUDE_ASM); the only tracked-file changes from this session are the ledger updates and three new rejected/ forms.
