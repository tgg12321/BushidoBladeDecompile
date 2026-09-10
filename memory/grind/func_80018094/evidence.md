# Evidence bank — func_80018094

## s1 (recon, 2026-09-09) — floor 81 -> 21, chassis -mel -msoft-float, no FAKE constructs

- OBJECT MODEL: D_8008D118 (census g_isqrt_lut, byte LUT) — MATCHES. Declared `extern u8 D_8008D118;` at
  src/code6cac.c:19 (TU-local, the same declaration the COMPLETED sibling func_8001A67C uses); the
  target's two LUT reads (`lui at,%hi; addu at,at,idx; lbu v0,%lo(D_8008D118)(at)`) are byte-identical in
  the s1 candidate (pairdiff lines 70-72 and 87-89 equal, tmp/grind/func_80018094/s1/v6a_pairdiff.txt).
  No header declaration exists (decl NONE) but none is needed for the bytes; no MISMATCH signal.
- Function shape (asm/funcs/func_80018094.s, 153 insns): gte_SetRotMatrix(arg0[1]) island,
  gte_SetTransMatrix(arg0[1]) island (arg0[1] re-read between them -> "memory" clobber on island 1, the
  func_80019310 spelling), call func_80017FA0(arg0), dx/dy/dz = ((MATRIX *)arg0[1])->t[k] - arg1[10+k]
  each stored to scratchpad 0x1F800024/28/2C (`lui at; sw` = GCC `sw $2,C` expanded by as), sum of
  squares, scale = 0x100 if sum > 250000 / 0 if sum < 0 / LUT-based otherwise (LZCS/LZCR island in the
  func_8001A67C template, local at 0x10($sp)), then the three scratchpad words scaled `(w*scale)>>1`
  (`lui v0; lw v0,C(v0)` = GCC `lw $2,C`), a 32-byte MATRIX block copy arg0[1] -> arg1+0x14 through a
  copy pseudo of arg1 that is also the func_80018300 argument (`move a0,s1`), tail call func_80018300.
- Scratchpad spelling is load-bearing (tools/gcc-2.7.2/sched.c true_dependence): a constant-address
  store that is NOT MEM_IN_STRUCT_P has no dependence against an in-struct varying load, so plain
  `*(s32 *)0x1F800024 = dx` lets the next statement's loads hoist above it (sandbox 81). A struct member
  at the constant base (`((ScrV *)0x1F800000)->x`, same-file-family precedent src/code6cac_b.c:919
  `#define SCR ((ProbeScr *)0x1F800078)`) is in-struct: the stores stay in place AND each read/write keeps
  its own constant address (cse folds `(plus reg_equiv_const off)` to a constant; the bare `(mem (reg))`
  form of `*(s32 *)C` is force_reg'd by explow.c memory_address, shared by cse canon_reg between the lw
  and the sw, and combine cannot fold a 2-use reg). A `s32 *scr = (s32 *)0x1F800000` variable instead
  hoists `lui` into s2 across the call (sandbox 62).
- `scale = 0x100` must be the ELSE arm (`if (sum > 250000) scale = 0x100; else if (sum < 0) ... else`):
  the target's `li v1,256` sits in the bnez delay slot and WRITES the register the branch reads, which
  reorg only does for an insn stolen from the branch-target thread (the else block), never from before
  the branch. With `scale = 0x100` before the if, sched1 adjust_priority (tools/gcc-2.7.2/sched.c:2562-2600,
  birthing insn -> max_priority) places the li between `mult dz` and `sw dz`; dz's local-alloc qty then
  spans 14 suids (pri floor_log2(6)*6*4/14*10000 = 34285) and loses to the 2-ref loads (40000), seating
  dz in $3 and cascading ($4 for the 250000 constant, sum in $4, sched2 anti-dependence on $2 blocks the
  arg1[12]/mat[7] load swap). Ground truth: tools/ra_solver/local_extract.py ->
  tmp/ra_solver_work/code6cac.local.json (blk 0: qty11 first_reg 77 birth 40 death 54 refs 6 got 3).
  Else-arm form: block 1 and the branch skeleton byte-identical.
- The 32-byte MATRIX copy (`*(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1]` with `s32 *dst = arg1` also passed
  to func_80018300) reproduces the target's 4 lw / 4 sw / 4 lw / 4 sw through `a0` on the first try:
  mips.c expand_block_move copy_addr_to_reg (tools/gcc-2.7.2/config/mips/mips.c:2351) makes the copy
  pseudo, combine folds `(plus dst 20)` back into the movstrsi_internal address, and the call-argument
  copy ties dst to $4.
- Island-input copy: the target reads the LZCS input from a0 = `move a0,a1` (sum stays in a1 for the
  slt/bgez/slti/LUT/srav uses). All 21 LZC sites in the binary read a0; the COMPLETED siblings had the
  value already seated in a0 (func_8001A67C: no copy). A named local copy (`lz_in = sum_sq`) and a
  `static inline` helper with a parameter both compile to NO move (tmp/grind/func_80018094/s1/v7a.s,
  v7b.s): the copy is removed before RA. The mechanism of the target's surviving copy is OPEN.
- Frame: target vars=16 (0x10..0x20), ours 8. BB2_FRAME_DEBUG (instrumented tools/gcc-2.7.2/cc1) shows a
  single put_reg_into_stack of 4 bytes (sp_tmp, the "=m" operand) then round_frame to 8 — identical to
  the COMPLETED sibling func_8001A67C (also 8). The extra 8 bytes are a phantom/temp the original had
  and ours lacks (memory phantom-frame-slots-gcc272). OPEN.
- Sandbox strips the 20 island insns on both sides (cheat_asm_stripped 20); canonical verdict
  ASM-PARTIAL "10/153 insns canonical-asm (ctc2/mtc2/swc2)". Owner-cluster member per
  .claude/rules/cop2-addressing-preamble-cluster.md:60 (SetRotMatrix/long-vector sub-family); not yet
  in tools/grinder/owner_cluster_grants.txt (grep empty). The island grant is a distance-0 check, not a
  lever.
- Tooling notes for the next session: tmp/grind/func_80018094/s1/splice2.py <variant.c> replaces the
  INCLUDE_ASM line (run `git checkout src/code6cac.c` first); cc.sh <v...> compiles variants with the
  build cc1 to s1/<v>.s; cc_dbg.sh <v> ENV=1 runs the instrumented cc1 with -da dumps + BB2_* hooks.
  splice.py (the first version) re-inserts the header typedef on every call — do not reuse it.

- [s1] OBJECT MODEL: D_8008D118 MATCHES with the TU-local `extern u8 D_8008D118;` (src/code6cac.c:19); both LUT reads byte-identical in s1/v6a_pairdiff.txt; no header decl needed for the bytes.

- [s1] canonical func_80018094 = ASM-PARTIAL, 10/153 insns canonical (ctc2/mtc2/swc2); sandbox strips 20 island insns (cheat_asm_stripped 20); islands spelled as gte_SetRotMatrix / gte_SetTransMatrix (func_80019310 / func_800300B4 spelling, ADDED memory clobber precedent src/code6cac_b.c:935) and the authorized func_8001A67C LZC template (inline_asm_canonical.txt:266).

- [s1] Scratchpad accesses must be MEM_IN_STRUCT_P (struct member at the 0x1F800000 base, precedent src/code6cac_b.c:919): sched.c true_dependence exempts a non-struct constant-address store from conflicting with in-struct varying loads (v1 = 81); a pointer variable hoists lui into s2 (v2 = 62); the struct form is byte-identical (v6a).

- [s1] scale = 0x100 must be the else arm of `if (sum_sq > 250000)`: the target's `li v1,256` occupies the bnez delay slot while writing the register the branch reads (reorg thread-steal only); scale-before-if extends the dz qty to 14 suids via sched1 adjust_priority and cascades four seats (measured with local_extract: qty11 got $3).

- [s1] The 32-byte MATRIX copy `*(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1]` with `s32 *dst = arg1` also the func_80018300 argument matches the target's 4 lw / 4 sw / 4 lw / 4 sw through a0 (mips.c expand_block_move copy_addr_to_reg + combine refolding the offset + call-arg tie).

- [s1] The LZCS island input in the target is a copy (`move a0,a1`; sum stays in a1); all 21 LZC sites in the binary read a0, and the completed siblings had the value seated there naturally. A named local copy and a static inline helper both compile with the copy eliminated pre-RA (s1/v7a.s, v7b.s).

- [s1] Frame: target vars=16, candidate vars=8; BB2_FRAME_DEBUG shows one 4-byte put_reg_into_stack (sp_tmp) + round_frame, identical to the completed func_8001A67C; the extra 8 bytes are an unidentified phantom/temp (memory phantom-frame-slots-gcc272).

- [s1] Residual 21 = prologue/epilogue frame offsets (8) + island-input copy and sum seat a1 (9) + LZCR-read seats `lw v1,16(sp); li v0,-2; and v0,v1,v0` (3) + log2_val seat a1 (1).

- [s1] Tooling: s1/splice2.py <variant.c> replaces the INCLUDE_ASM line after `git checkout src/code6cac.c`; s1/cc.sh <v...> compiles variants with the build cc1; s1/cc_dbg.sh <v> ENV=1 runs the instrumented cc1 with -da dumps and BB2_* hooks. The first splice.py re-inserts the header typedef on every call and must not be reused.
