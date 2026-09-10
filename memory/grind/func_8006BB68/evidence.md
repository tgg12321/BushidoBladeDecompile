# Evidence bank — func_8006BB68

## s1 (2026-09-10, recon) — MATCHED, sandbox distance 0
- canonical: verdict C, hand_coded_tier LOW (no S1/S2/S6), distance 112 = size only. Ordinary pure-C target.
- OBJECT MODEL: D_800A374C (g_dma_buf_base, `extern s32 D_800A374C;` in m2c_context.h) — MATCHES. Target does
  `lw a0,%lo(D_800A374C); addiu a0,a0,0x28` = scalar load + integer add; the s32 scalar declaration reproduces it
  byte-for-byte in the distance-0 form (same spelling `D_800A374C + 0x28` as sibling func_8006A3CC uses `+ 4`).
  No struct/aggregate signal; no declaration change needed. D_800A34F8 / D_800A34FC (gp-relative s32 externs,
  already declared in text1b.c) also MATCH (srl/andi on the load; lh 0xE off the pointer).
- Shape: struct local `S69E18 s` at sp+0x18 (existing typedef at text1b.c ~5557, same one sibling func_80069E18 uses),
  u16 rect[4] at sp+0x48, callee-saved: s0=arg0, s1=i, s2=q (table ptr), s3=const 1 (hoisted loop store value).
- Floor progression this session (all sandbox --disable all, no FAKE constructs at any step):
  v1 21 -> v2 16 -> v3 2 -> v4 0.
- v1->v2 (21->16): a FUNCTION-scope temp `p` used in 3 blocks became pseudo 75 (11 refs/12 insns, 3 live ranges)
  -> global.c colored it a0 (v0/v1 conflicting) -> forced `addiu a0,sp,0x18` late (sched2 anti-dep) and p+0xC into
  v0. Fix: per-block temp `p0` (local-alloc -> v0 / v1 as target) + FUNCTION-scope `p1 = p0 + 0xC` (global.c).
  Dumps: tmp/grind/func_8006BB68/dumps/text1b.lreg (pseudo 75 line), .sched.
- v2->v3 (16->2): `do{}while` with `i = 0` written before the first call put i=0 (uid 53, pri 1) into the sched1
  load-delay hole after `lw p` (sched.c is a BACKWARD list scheduler; ties by LUID). That kept a0 free during p1's
  range so p1 got a0 (ra_solver TRACE: a=75 hard_conf=[v0,v1,sp] best=a0) and also swapped i/q priorities
  (q 10 refs/55 vs i 9/51 -> q won s1). Writing the loop as `for (i = 0; i < 3; i++)` moves the i=0 emission after
  the first call block: `&s` now fills the hole, a0 conflicts with p1 -> p1 gets a1, and i/q land in s1/s2.
  Artifacts: tmp/grind/func_8006BB68/s1/inverse_v2.txt (ra_solver simulate/inverse), perturb_v2.txt,
  classify_v2.txt, sbs_v1..v3.txt (side-by-side).
- v3->v4 (2->0): loop-bottom order `addiu a0,sp,0x18` BEFORE `addiu s2,s2,4` requires `q++` emitted after the
  call statement in RTL (LUID tie-break): `q++;` as the last statement of the loop body.
- Signature: `void func_8006BB68(s32 *arg0)`; the stale `extern void func_8006BB68(s32);` prototype after the
  INCLUDE_ASM block (only caller func_8006C168 passes an s32 array) was removed — it conflicted with the definition.
- Solver notes: sched_solver goal-mapper reported moved:0 (register-only replace) — the i=0/&s swap is a
  same-position replacement in text, so use --goal-before by hand when a swap is pure seating consequence.
