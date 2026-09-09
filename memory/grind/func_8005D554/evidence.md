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
