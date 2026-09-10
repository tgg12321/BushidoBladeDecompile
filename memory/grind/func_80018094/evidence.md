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

## s2 (structural, 2026-09-09) — floor 21 -> 18, chassis -mel -msoft-float, no FAKE constructs

- SIBLING TRANSPLANT (mandated): the COMPLETED-C sibling func_8001A67C (src/code6cac.c:834-878) shares
  the whole LZCS/LZCR else-branch with this function. Transplanting its LZCR-read statement block —
  `{ s32 lw_v1 = sp_tmp; s32 li_v0 = -2; li_v0 = lw_v1 & li_v0; shift_a = 0x16 - li_v0; }`
  (src/code6cac.c:869-873) — in place of `shift_a = 0x16 - (sp_tmp & -2);` emits
  `lw $3,16($sp); li $2,-2; and $2,$3,$2` = the target's `lw v1,16(sp); li v0,-2; and v0,v1,v0`
  byte-identically. sandbox 21 -> 18 (tmp/grind/func_80018094/s2/v8a.c). The sibling's OTHER spellings
  do NOT transplant: its u32 typing of the else branch (dist_sq/log2_val/shift_a/shift_b unsigned)
  turns the target's `sra v1,v1,0x1` / `srav v0,a1,v1` into `srl` (s2/v9a.s) — func_80018094 needs the
  SIGNED spelling because sum_sq also carries the `> 250000` and `< 0` tests.
- RESIDUAL 18 = 8 prologue/epilogue frame-offset insns (vars 8 vs target 16) + a 10-insn register-seat
  cluster. The seat cluster is ONE phenomenon: the target keeps sum_sq in a1 for every use
  (`addu a1,v0,a3`, `slt`, `bgez a1`, `slti a1,1024`, `addu at,at,a1`, `srav v0,a1,v1`, and log2_val
  reuses a1 too) and feeds the LZC island a SEPARATE pseudo in a0 produced by a `move a0,a1` copy that
  reorg parks in the `beqz` delay slot (ours: a nop there, sum_sq in a0, no copy at all).
- PASS ATTRIBUTION (dumps, not guesswork): a C-level copy `lz_in = sum_sq;` exists as
  `(insn 124 (set (reg/v:SI 108) (reg/v:SI 78)))` in s2/v8b.rtl AND in s2/v8b.jump, and is GONE in
  s2/v8b.cse, where the asm_operands input has been rewritten from reg108 to reg78. The deleting pass
  is cse.c (canon_reg returns qty_first_reg, which is the older reg78 even when the block starts a
  fresh cse path, so the copy always becomes dead and is deleted). This closes the s1 frontier
  question "cse vs combine": it is cse, and it is position-independent — the copy is deleted equally
  when it sits at the top of the else-big block (s1 v7a) and when it sits in the pre-branch block
  (s2 v11b, whose asm is byte-identical to v8a).
- BINARY-WIDE CENSUS of the LZC island: grep for `mtc2 ... $30` over asm/funcs/*.s finds 28 files;
  EVERY inlined site emits `addu $t4, $a0, $zero` — the input is in $a0 at all 21 inlined sites
  regardless of the surrounding code (the three library leaves Lzc / SquareRoot0 / SquareRoot12 use
  `mtc2 $a0,$30` directly, a0 being their first parameter). The original's island input is
  register-FIXED to $a0; it is not an allocation coincidence.
- MECHANISM PROOF (diagnostic only, banned family, never proposed): a local register variable pinned
  to $4 (`register s32 lz_in __asm__("$4"); lz_in = sum_sq;` placed in the pre-branch block, the island
  reading it) reproduces the target's ENTIRE seat cluster exactly — `addu $5,$2,$7`, `slt $3,$3,$5`,
  `bgez $5`, `slt $2,$5,1024`, `move $4,$5`, `lbu $2,D_8008D118($5)`, `sra $2,$5,$3` (s2/v11a.s vs
  s2/v8a.s). It is INERT as a lever: the sandbox strips it (cheat_asm_stripped 20 -> 22) and the honest
  score goes UP to 24, exactly as the anti-cheat design intends. Banked at
  rejected/lzc-input-copy-register-pin-a0-sandbox-strips-24.c.
- The one non-pin construct that makes a copy SURVIVE is a tied output/input pair on the island
  (`: "=m"(sp_tmp), "=r"(lz_in) : "1"(sum_sq)`): reload emits a real `move $3,$4` (s2/v10b.s). It seats
  WRONG — the copy is a block-local qty and local-alloc's ascending find_free_reg gives it $3 (the
  island's own clobber list blocks $2), leaving sum_sq in $4; the target needs the copy in $4 and
  sum_sq in $5. It is also a dead asm output (the island never writes that register), so it is a
  coercion smell, not ordinary C. Banked at rejected/lzc-input-tied-output-copy-seats-v1-not-a0.c.
- RA-SOLVER GROUND TRUTH (tools/ra_solver/extract.py func_80018094 code6cac, model at
  tmp/ra_solver_work/func_80018094.model.json): global allocation order
  [79, 78, 120, 99, 72, 96, 127, 132, 137, 94, 73, 93]; dispositions 79 -> $3, 78 (sum_sq) -> $4,
  93 -> $5. sum_sq is allocated second and takes the first free register ($2 unavailable, $3 taken by
  79). For sum_sq to land in $5, some allocno CONFLICTING with it must take $4 first — in the target
  that allocno is precisely the island-input copy. So the frame residual and the seat residual are
  independent, but the entire 10-insn seat cluster is downstream of ONE question: how the original
  source fixed the island input to $a0.
- Structural levers that are byte-NEUTRAL on this body (all compiled; s2/*.s identical to v8a.s):
  sp_tmp declared in the innermost block (v9b), declaration-order permutation with dst first and
  sp_tmp last (v9c), log2_val hoisted to function scope (v9e), and the pre-branch copy local (v11b).
  Frame `vars=` stayed 8 for every one of them, so none of them is the missing 8 frame bytes.
- Tooling: tmp/grind/func_80018094/s2/{cc.sh,cc_dbg.sh} are the s1 scripts re-pointed at s2 and fixed
  to use splice2.py with a `git checkout src/code6cac.c` before every splice. They must be run under
  WSL (`bash tools/wsl.sh 'bash tmp/grind/func_80018094/s2/cc.sh <v>'`) — the Windows-side Git Bash
  cannot exec the Linux cc1 ("Exec format error").

- [s2] Floor 21 -> 18 on the chassis measured this session; candidate.c re-verified at 18 (153/153 insns, rules_dropped 0, cheat_asm_stripped 20) with no FAKE constructs and no register pins.

- [s2] Residual 18 = 8 prologue/epilogue frame-offset insns (vars 8 vs the target's 16) + a single 10-insn register-seat cluster: the target keeps sum_sq in a1 for every use and feeds the LZC island a separate a0 pseudo via a `move a0,a1` copy that reorg parks in the beqz delay slot; ours keeps sum_sq in a0, emits no copy, and leaves that slot a nop.

- [s2] Binary-wide census: grep of `mtc2 ... $30` over asm/funcs/*.s finds 28 files, and every one of the 21 inlined LZC sites emits `addu $t4, $a0, $zero` regardless of surrounding code (the three library leaves Lzc / SquareRoot0 / SquareRoot12 read $a0 as their first parameter). The original's island input is register-fixed to $a0, not allocated there by chance.

- [s2] Pass attribution by dump, not inference: the C-level copy is insn 124 in v8b.rtl and v8b.jump and is gone in v8b.cse with the asm operand rewritten to the source pseudo — cse.c canon_reg deletes it, in both the top-of-else-block position (s1 v7a) and the pre-branch position (s2 v11b).

- [s2] RA-solver ground truth (tmp/ra_solver_work/func_80018094.model.json): global order [79, 78, 120, 99, 72, 96, 127, 132, 137, 94, 73, 93]; 79 -> $3, 78 (sum_sq) -> $4, 93 -> $5. sum_sq is allocated second and takes the first free register, so it can only land in $5 if a conflicting allocno takes $4 first — in the target that allocno is exactly the island-input copy. The frame residual and the seat residual are independent problems.

- [s2] Structural levers measured byte-neutral on this body: sp_tmp in the innermost block, declaration-order permutation, log2_val at function scope, pre-branch copy local — all four assemble identically to the candidate and all four keep `vars= 8`.

- [s2] Tooling: tmp/grind/func_80018094/s2/{cc.sh,cc_dbg.sh,splice2.py} are the s1 scripts re-pointed at s2 and fixed to git-checkout src/code6cac.c before each splice; they must run under WSL (`bash tools/wsl.sh 'bash tmp/grind/func_80018094/s2/cc.sh <v>'`) because Windows-side Git Bash cannot exec the Linux cc1.
