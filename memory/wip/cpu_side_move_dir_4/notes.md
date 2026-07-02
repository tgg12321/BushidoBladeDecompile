# cpu_side_move_dir_4 (system.c) — WIP, masked floor 7, PARKED

## TL;DR (2026-07-01 session)
The residual is ONE block (debug_printf/do_timeout) and splits into TWO
mechanisms: (1) SCHEDULING — the `lw a1, D_800F19C0` must emit early
(target pos 51-52): **SOLVED in pure C** by candidate.c's arg5-first +
split-index structure (no volatile/pins/dead stores). (2) REGISTER — the
arg4 index chain must live in **a0** (target) not v1: local-alloc gives
the tied chain-qty (density 6refs/11insns) v1 before arg5val is colored;
target's coloring needs the chain's range stretched to the call (density
<0.35 → colored last → v0/v1 blocked → a0), which also produces the
target's interleave (a0-serialization). No clean C form found yet that
stretches the chain WITHOUT breaking the file-level s-reg balance.

## Ruled out this session (do NOT re-derive)
- **volatile on idx_1494** (pins idx-pair order, v15): prong-1 FAILS —
  D_800A1494's only writer is `func_80080828` (sb at
  asm/funcs/func_80080828.s:244), a SYNCHRONOUS callee (called from this
  fn's poll loop, marionation_Exec, tslTm2LoadImage, func_80081E1C; no
  IRQ registration found). Not carve-out-eligible.
- **v16 (t0 + arg4 inline in call)** and **v17 (address precompute p4)**:
  both STRETCH the chain correctly (sll/addu/lw spread ✓) but flip the
  file-level base reg s2→s1 (global s-reg rebalance; the committed
  idx_1495 chain lever is delicately ref-balanced). Any block shape must
  preserve idx_1494/tbl_125c ref counts exactly.
- **Inline-all args** (v1/v8/v9): right-to-left eval gives arg4-last ✓
  but idx1-first ✗ and 11D5-early ✗; u8 single-use locals get
  forward-substituted (cse/combine sink the lbu) — a bare local does NOT
  hold load position.
- **Standalone permuter (permuter/csmd4_v8)**: UNFAITHFUL — the v16-shape
  scores 270 there with s2 intact, but the same shape regresses to s1 in
  the full file. Do not trust its scores for this function; a faithful
  workspace needs the engine-sandbox pipeline as compile.sh (slow).
- FAKE dead stores/reads can't bump local-alloc qty density — flow.c
  deletes them BEFORE regclass/local-alloc; only REAL uses count.

## Resume here
1. Apply candidate.c (v5 form); confirm masked 7 + block shape via
   tmp/probe_csmd4.py (apply-variant → sandbox → objdump block).
2. The win condition, precisely: keep candidate's structure AND stretch
   the arg4 chain's last op (lw a3) to just before the fmt-la, WITHOUT
   changing idx_1494/tbl_125c/idx_1495 ref counts. Chain-extender-style
   detours through OTHER live stores in the block are the untried lever
   class (e.g. routing an existing store's value through arg4's address
   pseudo with a combine-foldable delta — verify zero-byte fold).
3. If a form colors the chain to a0 in the FULL file: expect the
   interleave to follow (a0-serialization vs fmt-la); then retire all
   5 rules + full SHA1.
4. Faithful permuter fallback: compile.sh = engine sandbox build of
   system.o with THIS function's rules dropped; target.o =
   build/src/system.o; base = candidate. ~30s/iter — directed PERM only.

## Pointers
- tmp/probe_csmd4.py + tmp/csmd4_variants/ + tmp/csmd4_v2/ (this
  session's probes; gitignored, regenerate from this note if gone).
- memory/project/register-alloc-deep-dive.md — sessions 4-9 ledger
  (chain-extender now SANCTIONED per dead-store-fake-exception §2026-07-01).
- Target block: asm/funcs/cpu_side_move_dir_4.s lines 49-73.
