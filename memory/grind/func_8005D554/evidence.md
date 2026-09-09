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
