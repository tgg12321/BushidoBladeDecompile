# Evidence bank — _spu_2pitch

## s1 (2026-09-02, recon) — MATCHED, sandbox 0

- Identity: PsyQ 4.0 LIBSPU/S_N2P `_spu_2pitch(atten, rem)`, 36 insns, leaf, verdict C
  (canonical: pure-C distance 7 on the first draft). It is the interpolation tail that
  `_spu_note2pitch` inlines (see `memory/grind/_spu_note2pitch/retired-chassis-2026-08/body.c`
  lines 37-62 for the same loop inside the sibling). No call sites in BB2 besides being an
  exported entry point (dead code pulled in by whole-object linking).
- Algorithm: `lower = atten*ratio^n`, `upper = atten*ratio^(n+1)` with ratio = 0x103B/0x1000
  (2^(1/48)), n = rem>>5; result = (lower + ((upper-lower)>>5)*(rem&0x1F)) >> 12. No clamp
  (the 0x3FFF clamp lives in the sibling only).
- The `upper` value lives in a stack slot (sw in both branch delay slots, lw after the loop)
  in EVERY spelling measured — it is a natural reload spill, not a volatile/address-taken
  local. Do not re-derive: the sibling's `volatile s32 sp8` was never needed for this.
- Frame: target `.frame 16` with the spill at +8. Plain `if (n) do..while` guard forms give
  `.frame 8`, spill at +0 (score 7). A `for (i = 0; i < n; i++)` loop gives `.frame 16`,
  spill at +8 — the rotated-loop guard compare orphans a pseudo that reload's alter_reg pays
  off with an untouched slot (producer 1 of `.claude/rules/phantom-slot-frame-lever.md`).
  `while (i < n)` also gives frame 16 but score 3 (slt/guard shape differs).
- Guard: `s32 steps = rem >> 5` (srl) + `for` guard emits `beqz` (cc1 knows the srl result is
  non-negative, so `0 < steps` folds to `steps != 0`); loop-bottom compare is signed `slt`
  because both `i` and `steps` are s32. `u32 steps` gives `sltu` (score 3).
- Last residual (score 2): the constant-multiply shift-add chain's final `subu` must target
  `ratio`'s own register (`subu v1,v0,v1; srl v1,v1,12`), not the chain temp
  (`subu v0,v0,v1; srl v1,v0,12`). Single expression `ratio = (ratio*0x103B)>>12` puts the
  product in a fresh temp pseudo (score 2); the two-statement `ratio *= 0x103B; ratio >>= 12;`
  assigns the product to `ratio` itself → score 0. Both `u32 ratio` (srl) forms — s32 params
  (`v8`) and all-u32 params (`v9`) — score 0; the all-unsigned form was kept (zero casts).
- Sandbox `--disable all` = 0 on the final form; `.frame $sp,16,$31 # vars= 16` matches.
- Artifacts: `tmp/grind/_spu_2pitch/s1/` (v1..v9 variants, `v2.s` cc1 output, `v2_sandbox.dis`,
  `patch.py` frame-gradient instrument, `sweep.ps1` batch scorer, `scorediff.py` scorer-level diff).
