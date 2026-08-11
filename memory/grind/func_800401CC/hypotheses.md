# Hypothesis ledger — func_800401CC

## Session 1 (recon, 2026-08-11) — floor 20 → 7

### CONFIRMED
- **H1: staging-local removal fixes both $2<->$3 swap regions.** Mechanism:
  multi-block user local loses $v0 to local-alloc temps; inline global reads
  produce short temps taking ascending regs. Probe: removed `s32 v0;` staging,
  inlined `D_800A36AC & 1` and `D_800A3234 * 240`. Result: 20 → 7 (with H2).
  CONFIRMED.
- **H2: D_800A3378 update belongs AFTER the ot-link statement.** Mechanism:
  sw-symbol vs lw-pointer ambiguity — source order picks the dependence
  direction; store-last makes the lw an anti-dep so sched floats the sw between
  the tail ANDs, matching target insns 70-77 structurally. Probe: moved the
  statement last. Result: tail structure byte-identical modulo the $6/$7 mask
  rename. CONFIRMED.

### KILLED (do not re-propose)
- **K1: first-encounter/birth order decides the $6/$7 mask assignment.**
  Probes: (a) stmt1 OR-operand flip → 7→9, load order perturbed, masks
  unchanged; (b) single named `addr_mask = 0xFFFFFF` local init before the
  packet statements → 7, masks unchanged; (c) two named mask locals init
  FFFFFF-first → 7, masks unchanged (QTYDBG: FF000000 still shorter-lived,
  still wins $6). Allocation follows qty_compare_1 priority, not birth.
- **K2: separating the two tail AND deaths in SOURCE moves them at
  local-alloc.** Probes: v0-split of stmt2 (late and early placement), store
  moved between; QTYDBG shows sched1 re-packs the ANDs adjacent (death gap 2)
  in every variant. Source-level death separation does not survive sched1.
- **K3: stmt2 OR-operand flip (`(pkt & 0xFFFFFF) | (ot & 0xFF000000)`).**
  Kills structure: GCC reuses the FIRST or-operand's reg as dest → `or $4,...`
  where target has `or $v0,...` (ot-word reg first), plus addiu/sw reshuffle;
  7→12. Target's or-dest PROVES original operand order is ot-first. Dead.
- **K4 (analytic): FF000000-allocated-first-but-takes-$7.** Impossible —
  FF000000's range is a strict subset of FFFFFF's, so the first-allocated of
  the pair always gets $6. The ONLY path is FFFFFF allocated first.

### FRONTIER (for session 2)
- **F1: find a natural C spelling that lifts refs(0xFFFFFF) to 4** (or drops
  refs(FF000000) to 2). With refs=4: pri = 2*4*4/len ≈ 32/30 beats FF000000's
  12/24 → FFFFFF allocated first → $6, remaining 7 close. Constraint: emitted
  bytes must stay identical (still exactly lui/ori + lui + 4 ands). reg_n_refs
  is counted by flow.c on pre-RA RTL; a use that later folds/merges (cross-jump,
  combine) may still count. Candidate spellings to probe: duplicated
  mask-consuming statement into the a0!=0 arms (sanctioned family
  duplicated-statement-into-arms — needs byte-neutrality proof + FAKE
  annotation + exhaustion, LAST RESORT); before that, look for an honest
  4th-ref shape (e.g. computing the OT-link low word once into a named local
  used by both statements changes and-counts — must keep 4 ands total).
- **F2: make sched1 emit the FF000000 li FIRST** (birth gap flips sign; then
  FF000000 is longer-lived and FFFFFF wins $6 outright). sched1 orders the
  const loads by chain-length priority (FFFFFF's li+ori chain is longer, always
  first). Probe idea: lengthen FF000000's dependency chain by 1 without new
  bytes (e.g. a spelling where the FF000000 li feeds through one more
  combine-folded op). Read sched.c priority computation first
  (INSN_PRIORITY / launch), instrumented hooks BB2_SCHED_DEBUG exist per
  [[sched-solver-campaign-2026-08-05]] memory.
- **F3: permuter sweep of the tail** with a clean single-function target
  (build target.o from asm/funcs/func_800401CC.s + prelude per
  difficult-is-not-impossible §3), seeded from the floor-7 candidate. The
  search space is small (tail spellings only) and the score gradient is pure
  reg-diff — good directed-PERM territory. Vet any closing form against the
  cheat checklist before adopting.

## [s1] Both $2<->$3 swap regions (regfix @1-5, @14-21) are caused by staging global reads through a reused multi-block local v0
- mechanism: Multi-block user pseudo goes to global-alloc and loses $v0 to block-local temps; inline reads become short local-alloc temps taking ascending regs
- probe: Removed s32 v0 staging; inlined D_800A36AC & 1 and D_800A3234 * 240 reads
- result: 20 -> 7; both regions byte-match
- verdict: CONFIRMED

## [s1] D_800A3378 update must come AFTER the ot-link statement
- mechanism: GCC 2.7.2 cannot disambiguate sw-symbol vs lw-through-pointer; source order sets the dependence direction, store-last makes the ot re-read an anti-dep so sched floats the sw between the tail ANDs like target
- probe: Moved D_800A3378 = (s32)(pkt + 6); to last statement
- result: Tail structure byte-identical to target modulo the $6/$7 rename
- verdict: CONFIRMED

## [s1] Birth/first-encounter order decides which mask constant gets $6
- mechanism: Presumed local-alloc birth-order allocation
- probe: OR-operand flips (stmt1, stmt2), single and dual named mask locals init FFFFFF-first, v0-splits early/late; QTYDBG on instrumented cc1 for each
- result: Assignment never flipped; QTYDBG shows qty_compare_1 priority = floor_log2(refs)*refs*size/length, FF000000 always ~2 luids shorter-lived (FFFFFF li+ori born 4 luids earlier by sched1 chain priority, ANDs re-packed adjacent), so FF000000 always allocated first and takes $6. stmt2 flip also breaks structure (or-dest reg) proving original operand order is ot-first
- verdict: KILLED

## Session 2 (structural, 2026-08-11) — floor 7 → 0

### CONFIRMED
- **H3 (from F1-adjacent): removing FF000000 from the local qty pool flips the
  remaining allocation.** Mechanism: with only ONE mask qty in block 5,
  qty_compare_1 ordering is moot; the FFFFFF qty takes the first free ascending
  reg = $6 (target). Probe: any spelling holding 0xFF000000 in a multi-block
  (global-alloc'd) variable. Result: P2/P3 both show FFFFFF→$6. CONFIRMED.
- **H4: the FF000000 holder lands $7 iff it has no other hard-reg commitment.**
  Mechanism: global.c allocno with tail-only range conflicts $2-$6 (local
  qtys), $7 free; the pre-existing $a3 call-arg copy preference of `v` points
  at $7 anyway. Probe: v = 0xFF000000 (v widened s32). Result: score 7 → 2,
  head byte-identical. CONFIRMED — this is the closing lever.
- **H5: constant-load emission order is source-set-order-biased when the
  holders are explicit sets.** Probe: lowmask = 0xFFFFFF; set before v's set.
  Result: 2 → 0. CONFIRMED (measured; sched internals not fully instrumented).

### KILLED
- **K5: param a2 as mask holder (either mask).** a2's pseudo spans head+tail;
  global-alloc must give both ranges ONE reg; local-alloc runs first and a
  local mask qty always takes $6 before a2's preference is considered → a2
  lands $7 → head parity cluster breaks (4 diffs). P1 score 12, P2 score 7.
  Generalizes: ANY head-register-committed variable (a1/tbl/u) has the same
  bind; only a variable whose existing hard-reg home IS $7 escapes — that is
  exactly `v`.

### FRONTIER
- (empty — sandbox 0 reached; candidate-ready submitted s2. If the Judge
  bounces the staged-mask construct, the fallback frontier is s1's F1
  (honest 4th-ref search) and F3 (directed permuter sweep from the floor-7
  form), both still unexhausted as pure-C search spaces.)
