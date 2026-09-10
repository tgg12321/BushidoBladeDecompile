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

## s3 (structural, 2026-09-09) — the residual is exactly {8 frame insns} + {10 seat insns}, and they are independent

- Chassis re-measured this session: `sandbox func_80018094 --disable all` on the committed
  memory/grind/func_80018094/candidate.c = **18** (153/153 insns, rules_dropped 0,
  cheat_asm_stripped 20). The ledger's floor 18 stands on the current chassis.
- `tools/pairdiff.py code6cac func_80018094` (tmp/grind/func_80018094/s3/base_pairdiff.txt) resolves
  the 18 into two disjoint sets:
  - **Frame (8 insns)**: ours `addiu sp,sp,-40` / `sw s0,24(sp)` / `sw s1,28(sp)` / `sw ra,32(sp)` and
    the four matching epilogue insns; target `-48` / `32` / `36` / `40`. Nothing else in the body
    depends on them.
  - **Seats (10 insns)**: indices 60, 61, 64, 65, 69, 71, 74, 85, 93, 96. The target keeps sum_sq in
    **$a1** across the whole if-chain (`addu a1,v0,a3`, `slt v1,v1,a1`, `bgez a1`, `slti v0,a1,1024`,
    `addu at,at,a1`, `srl a1,v0,3`, `srav v0,a1,v1`, `srav a1,a0,v0`, `sll v1,a1,6`) and fills the
    delay slot of `beqz v0` with **`move a0,a1`**, where ours emits `nop`. So the island-input copy is
    a real insn that reorg.c stole into the delay slot — it is not part of the hand-asm island.
- The two sets are decoupled: s2's register-pin body (s2/v11a.s), which reproduces the whole seat
  cluster byte-for-byte, still prints `.frame $sp,40 # vars= 8`. Fixing either does not fix the other.

### Frame: the search space is now bounded on three sides
- Frame equation (mips.c compute_frame_size, MIPS_STACK_ALIGN=8):
  `48 = ALIGN8(vars) + ALIGN8(args=16) + ALIGN8(gp_regs=12)` => `ALIGN8(vars)=16`, i.e. raw vars 9..16.
  Ours is raw 4 -> round_frame 8. The missing amount is 5..12 raw bytes; 8 is the natural fit.
- **Position**: the target's island writes `addiu $v0,$sp,0x10` and the C-side read is `lw $v1,0x10($sp)`,
  so sp_tmp is at vars offset 0 with args=16. Any extra object must therefore be allocated *after*
  sp_tmp's `put_reg_into_stack`, i.e. by a construct expanded after the LZC island statement. Measured
  both ways: the same 8-byte struct placed before the island (p2.c) pushes sp_tmp to `24($sp)` and
  breaks the island; placed after it (p3.c) it keeps `0x10` and prints `vars= 16, frame 48` — the
  target's exact frame.
- **Invisibility**: the target contains no sp-relative access other than that one 0x10 pair, so the
  extra slot is never read or written. It is a phantom (memory phantom-frame-slots-gcc272), which
  excludes every LIVE aggregate spelling (p3 costs 3 real memory insns).
- **Mode**: a FRAMEDBG census of the entire src/code6cac_c2.c TU with the instrumented cc1
  (`BB2_FRAME_DEBUG=1 tools/gcc-2.7.2/cc1`, tmp/grind/func_80018094/s3/code6cac_c2.fd — 53 slot
  events) finds only three contexts in this codebase: `round_frame` (39), `stack_temp` (11) and
  `spill_new_pNN` (3). **Every one of the 11 `stack_temp` events is `mode=26` (BLKmode)**, sizes
  8/16/32/256/1024. No SImode or HImode stack temp is ever allocated in this TU. So the construct to
  hunt is an 8-byte **aggregate-typed** temp in the post-island tail, not a scalar.
- Ruled out this session as frame levers on this body: HImode narrowing of the LZCR-read block or of
  shift_a/shift_b (v12a/v12b/v12c — vars stays 8 and costs 2 sign-extension insns), and named-
  intermediate scalar splits of the products / shift arithmetic / division (v14a-v14d — vars stays 8;
  v14b additionally burns a 4th callee-saved register). Combined with s2's H15 (declaration scope,
  declaration order, function-scope hoisting all frame-neutral), **no scalar-level structural lever
  moves this frame** — the answer is an aggregate object model in the tail.

### Seat cluster: the $a0 census is 29/29
- `grep -B3 'mtc2 .*\$30' asm/funcs/*.s` over the whole binary: every one of the 29 inlined LZCS sites
  is preceded by `addu $t4, $a0, $zero`. By contrast the SetRotMatrix/SetTransMatrix preambles in this
  same function read `$a2` (`lw $a2,4($s0); addu $t4,$a2,$zero`), i.e. that macro's register genuinely
  varies with its operand. The LZC macro's does not. That asymmetry is the evidence that the LZC
  island's asm text names `$a0` itself, and it is why every "r"-constraint C spelling seats the input
  wherever the allocator likes ($a0 here, by luck) instead of forcing the copy the target has.

- [s3] Chassis re-measured this session: sandbox func_80018094 --disable all on the committed candidate.c = 18 (153/153 insns, rules_dropped 0, cheat_asm_stripped 20). The ledger floor 18 stands on the current -mel -msoft-float chassis.

- [s3] The 18 residual insns are exactly 8 frame-offset insns (prologue/epilogue, ours frame 40 vs target 48) and 10 register-seat insns (target keeps sum_sq in $a1 and fills the `beqz v0` delay slot with `move a0,a1`, where ours emits `nop`).

- [s3] The two residuals are independent: s2's register-pin body v11a reproduces the entire seat cluster byte-for-byte and still prints vars= 8.

- [s3] Frame equation for the target: 48 = ALIGN8(vars) + ALIGN8(args=16) + ALIGN8(gp_regs=12) => ALIGN8(vars)=16, i.e. raw vars 9..16; ours is raw 4 rounded to 8. The missing amount is 5..12 raw bytes and 8 is the natural fit.

- [s3] Position is fixed by the island: the target writes `addiu $v0,$sp,0x10` and reads `lw $v1,0x10($sp)`, so sp_tmp occupies vars offset 0 and any extra slot must be allocated after it. Measured both ways with the same 8-byte struct: before the island it pushes sp_tmp to 24($sp) (wrong); after the island it keeps 0x10 and yields frame 48 / vars 16 (exactly right).

- [s3] The extra slot is a PHANTOM: the target has no sp-relative access other than that single 0x10 pair, so the object is allocated and never touched — every live-aggregate spelling is excluded by the target's own bytes (p3 costs 3 real memory insns).

- [s3] A FRAMEDBG census of the whole src/code6cac_c2.c TU (53 slot events) shows all 11 non-spill non-round_frame slots are mode=26 BLKmode aggregates; there is no SImode/HImode stack temp anywhere in this codebase, which explains why every scalar-level lever (s2 H15 declaration scope/order/hoisting, s3 HImode narrowing, s3 named-intermediate splits) is frame-neutral.

- [s3] Binary-wide census: all 29 `mtc2 $t4,$30` sites in asm/funcs are preceded by `addu $t4, $a0, $zero`, while the SetRotMatrix/SetTransMatrix preamble in this same function reads $a2 (`lw $a2,4($s0); addu $t4,$a2,$zero`). That asymmetry is the evidence that the LZC island's asm text names $a0 itself, and it is why no `r`-constraint C spelling forces the target's `move a0,a1` copy.

## s4 (permuter, 2026-09-09) - floor 18 -> 7: the frame residual is CLOSED and 3 of the 10 seat insns fell to a permuter find

- Chassis re-measured at session start on the committed s2/s3 candidate.c: `sandbox func_80018094
  --disable all` = **18** (153/153 insns, rules_dropped 0, cheat_asm_stripped 20). The dispatch brief's
  "measurement unavailable" is resolved: the ledger floor 18 was correct.
- **STANDALONE CHASSIS IS EXACT.** A 7-line minimal TU (typedefs + `extern u8 D_8008D118;` + the two
  callee prototypes + the candidate body, tmp/grind/func_80018094/s4/m0.c) compiles to asm that is
  identical to the full-TU code6cac.c compile except for `.L` label NUMBERS. That is what made a
  permuter workspace possible at all - pycparser cannot parse the full TU's file-scope `INCLUDE_ASM`
  asm blocks, and it parses the minimal TU (including the three inline-asm islands) without a patch.
  Workspaces: tmp/perm_80018094{,b,c}; builders tmp/grind/func_80018094/s4/mkperm{,2,3}.sh.
- **FRAME CLOSED (8 of the 18 insns), at ZERO insn cost.** The missing 8 frame bytes are the unwritten
  TAIL of the LZC output local itself, not a separate temp: spelling `s32 sp_tmp;` as `s32 sp_tmp[4];`
  (island operand `"=m"(sp_tmp[0])`, read `sp_tmp[0]`) prints `.frame $sp,48 # vars= 16, regs= 3/0,
  args= 16` - the target's exact frame - while leaving the other 145 insns bit-identical. Measured
  sandbox 18 -> **10**. This makes s3's frontier hypothesis (an 8-byte BLKmode stack TEMP created in
  the post-island tail) WRONG in its mechanism: no temp is needed, the sp_tmp slot itself is oversized
  in the original.
- **The four spellings of that slot are byte-identical** (tmp/grind/func_80018094/s4/v20{b,c,d,g,h}.s
  all diff-clean against each other): `struct { s32 a, b, c; }`, `s32 [3]`, `s32 [4]` with `"=m"(sp_tmp)`
  and `s32 [4]` with `"=m"(sp_tmp[0])`. So the declared size is recoverable only as the RANGE 9..16
  raw bytes - exactly the situation the OVERSIZED-LOCALS carve-out's range-annotation prerequisite
  (.claude/rules/dead-vars-local-array.md, owner ruling 2026-07-13) was written for.
- **Frame-math proof (carve-out prerequisite 1), from the target bytes alone**: target frame 0x30 =
  outgoing args 0x10 + locals 0x10 + callee-saves 0x10 (s0/s1/ra at 0x20/0x24/0x28); the ONLY locals
  traffic anywhere in asm/funcs/func_80018094.s is the island's `swc2 $31,0($t4)` with `$t4 = $sp+0x10`
  and the matching `lw $v1,0x10($sp)` - 4 bytes written of a 16-byte locals region. A fully-written
  4-byte locals set yields ALIGN8(4)+16+16 = 0x28 != 0x30, so no fully-written locals set can produce
  the target frame. Prerequisite 2 (prefer extending a LIVE object over a dead pad) is satisfied by
  construction here: sp_tmp[0] is the live LZC output, and the carve-out's own precedent shape
  (func_80037540's `s32 sp[8]` with only sp[0..5] stored) is the same written-prefix-buffer form.
- **PERMUTER FIND (3 more insns).** Campaign `s4-v20g-framefixed` (tmp/perm_80018094b, base score 245,
  --stack-diffs, -j 8) produced output-230-1 at 24,345 iterations / ~9.5 min. Its ONE semantic delta
  from the base is staging the LZCR-read result through the enclosing block's `log2_val`:
  `li_v0 = lw_v1 & li_v0; log2_val = li_v0; shift_a = 0x16 - log2_val;`. Applied to the candidate this
  measures sandbox 10 -> **7**, and it fixes precisely the seats s2/s3 predicted: sum_sq now sits in
  **$a1** for every use (`addu at,at,a1`, `srav v0,a1,v1` now byte-identical to the target).
  Banked source: tmp/grind/func_80018094/s4/perm_find_230.c.
- **THE RESIDUAL IS NOW 7 INSNS AND ALL 7 ARE DOWNSTREAM OF ONE MISSING COPY**
  (tmp/grind/func_80018094/s4/v21a_pairdiff.txt): index 69 `nop` vs `move a0,a1` (the island-input
  copy reorg parks in the `beqz v0` delay slot), 74/75 (`srl a0` vs `srl a1`; `move t4,a1` vs
  `move t4,a0`), 82/84 (li_v0 in `$a0` vs the target's `$v0`), 93/96 (log2_val in `$a0` vs `$a1`).
  The target reserves `$a0` for the copy across that whole region, which is what pushes li_v0 to $v0
  and log2_val to $a1. This is the same single question s2/s3 identified, now with the frame and the
  sum_sq seat removed from around it: how the original fixed the LZC island's input to `$a0`.
- **Permuter exhaustion on the post-find chassis**: campaign `s4-v21a-log2staged` (tmp/perm_80018094c,
  base score 230) ran **45,681 iterations / ~18 min with ZERO novel finds** before harvest --stop.
  Combined with `s4-v8a-chassis` (8,906 iterations, 0 improvements) that is 78,932 iterations across
  three chassis this session; only the frame-fixed chassis yielded, and it yielded in ~9 minutes,
  exactly as the basins-yield-early rule predicts.

- [s4] Floor 18 -> 7 this session. candidate.c is the s2 v8a body + `s32 sp_tmp[4]` (OVERSIZED-LOCALS carve-out, closes the 8 frame insns at zero insn cost) + the permuter's `log2_val` staging of the LZCR-read result (closes 3 seat insns). Both constructs are FAKE-annotated in candidate.c.

- [s4] s3's frontier mechanism is REFUTED: the missing 8 frame bytes are not a post-island BLKmode stack temp, they are the unwritten TAIL of the LZC output local. `s32 sp_tmp[4]` with `"=m"(sp_tmp[0])` gives `.frame $sp,48 # vars= 16, args= 16, regs= 3/0` with all 145 non-frame insns unchanged.

- [s4] The declared size of that locals object is recoverable only as a RANGE (raw 9..16 bytes): `struct {s32 a,b,c;}`, `s32[3]`, `s32[4]` with `"=m"(sp_tmp)` and `s32[4]` with `"=m"(sp_tmp[0])` all assemble identically (s4/v20{b,c,d,g,h}.s).

- [s4] Frame-math proof for the carve-out: target frame 0x30 = args 0x10 + locals 0x10 + saves 0x10; the only locals traffic in the entire target is `swc2 $31,0($sp+0x10)` / `lw $v1,0x10($sp)` (4 bytes of 16); a fully-written 4-byte locals set gives ALIGN8(4)+16+16 = 0x28 != 0x30.

- [s4] A minimal 7-line-header standalone TU reproduces the full-TU codegen for this function exactly (labels aside), which is what makes decomp-permuter usable here - pycparser cannot parse code6cac.c's file-scope INCLUDE_ASM blocks but parses the minimal TU with all three inline-asm islands intact.

- [s4] Permuter telemetry: 78,932 iterations over three chassis. s4-v8a-chassis 8,906 it / 0 improvements; s4-v20g-framefixed 24,345 it / one improvement (245 -> 230) at ~9.5 min = the log2_val staging; s4-v21a-log2staged 45,681 it / 0 novel finds in 18 min (harvested --stop).

- [s4] The remaining 7 insns are ALL downstream of the single missing `move a0,a1` island-input copy: with $a0 reserved for it the target is forced to put li_v0 in $v0 and log2_val in $a1, which is exactly the other 6 diffs (s4/v21a_pairdiff.txt).

- [s4] Chassis re-measured at session start on the committed s2/s3 candidate.c: sandbox func_80018094 --disable all = 18 (153/153 insns, rules_dropped 0, cheat_asm_stripped 20). The dispatch brief's 'measurement unavailable' is resolved — the ledger floor 18 was correct.

- [s4] Floor moved 18 -> 10 -> 7 this session, each step re-measured with the sandbox on the spliced src/code6cac.c; src was reverted to INCLUDE_ASM at session end and the floor-7 body is saved to memory/grind/func_80018094/candidate.c.

- [s4] A 7-line-header standalone TU (typedefs + extern u8 D_8008D118 + the two callee prototypes + the candidate body, tmp/grind/func_80018094/s4/m0.c) reproduces the full-TU code6cac.c codegen for this function EXACTLY except for .L label numbers. That is what makes decomp-permuter usable here: pycparser cannot parse code6cac.c's file-scope INCLUDE_ASM asm blocks, but it parses the minimal TU with all three inline-asm islands intact.

- [s4] Frame closed at ZERO insn cost by spelling the LZC output local as `s32 sp_tmp[4]` with island operand "=m"(sp_tmp[0]) and read sp_tmp[0]: .frame $sp,48 # vars= 16, regs= 3/0, args= 16, with the other 145 insns bit-identical to the scalar chassis.

- [s4] The declared size of that locals object is recoverable only as a RANGE (raw 9..16 bytes): struct{s32 a,b,c;}, s32[3], s32[4] with "=m"(sp_tmp) and s32[4] with "=m"(sp_tmp[0]) all assemble identically (tmp/grind/func_80018094/s4/v20{b,c,d,g,h}.s) — the exact situation the OVERSIZED-LOCALS carve-out's range-annotation prerequisite was written for.

- [s4] OVERSIZED-LOCALS prerequisite 1 (frame-math proof from the target bytes alone) holds: target frame 0x30 = outgoing args 0x10 + locals 0x10 + callee-saves 0x10 (s0/s1/ra at 0x20/0x24/0x28); the only locals traffic anywhere in asm/funcs/func_80018094.s is `swc2 $31,0($t4)` with $t4 = $sp+0x10 and the matching `lw $v1,0x10($sp)`, i.e. 4 bytes written of a 16-byte locals region; a fully-written 4-byte locals set yields ALIGN8(4)+16+16 = 0x28 != 0x30.

- [s4] OVERSIZED-LOCALS prerequisite 2 (prefer extending a LIVE object over a dead pad) is satisfied by construction: sp_tmp[0] is the live LZC output, so this is the carve-out's written-prefix-buffer shape (its own precedent is func_80037540's s32 sp[8] with only sp[0..5] stored), not a dead pad local.

- [s4] Both constructs in candidate.c carry /* FAKE: what + named GCC-pass mechanism + lever-exhaustion pointer */ annotations, and the sp_tmp declaration additionally carries the SOTN `n.b.!` range annotation with the frame derivation, per carve-out prerequisite 3.

- [s4] Permuter telemetry (all campaigns harvested --stop, 0 live at session end): s4-v8a-chassis 8,906 iterations / base 303 / 0 improvements; s4-v20g-framefixed 24,345 iterations / base 245 / one improvement to 230 at ~9.5 min (the log2_val staging); s4-v21a-log2staged 45,681 iterations / base 230 / 0 novel finds in ~18 min. 78,932 iterations total.

- [s4] The residual is now 7 insns and every one of them is a consequence of the missing island-input copy: index 69 nop vs `move a0,a1`, 74/75, 82/84, 93/96 (tmp/grind/func_80018094/s4/v21a_pairdiff.txt).

## s5 (permuter, 2026-09-09) — the island-input copy is now MATERIALISED; the residual is one register name

Chassis at dispatch: candidate.c re-measured on HEAD = sandbox 7 (153/153 insns, rules_dropped 0,
20 island insns stripped on both sides). Unchanged from s4.

**What moved.** s4's frontier said "all 7 remaining insns fall together the moment the LZC island's
input is fixed to $a0" and framed a ruling-request about naming $a0 in the island's asm text. That
ruling request is NOT needed: the copy can be produced by ordinary operand plumbing. A tied asm
output on the island (`: "=m"(sp_tmp[0]), "=r"(lz_in) : "1"(sum_sq)`) makes GCC emit a real
`move lz_in,sum_sq` that cse.c cannot delete, and reorg.c parks it in the `beqz v0` delay slot in
exactly the target's position. Adding it alone costs 3 insns (sandbox 10); the s5 permuter campaign
found that a `do { ... } while (0);` wrap around the inner if-chain plus its `scale = ...` statement
gives all 3 back (sandbox 7). That body — `rejected/tied-copy-dowhile-wrap-copy-seats-v1-not-a0-
equal-floor-7.c` — is the same floor as candidate.c but a strictly better structural position:

| | ours (w6) | target |
|---|---|---|
| 69 (delay slot) | `move v1,a1` | `move a0,a1` |
| 75 (island input) | `move t4,v1` | `move t4,a0` |
| 74 | `srl a0,v0,3` | `srl a1,v0,3` |
| 82/84 (li_v0) | `$a0` | `$v0` |
| 93/96 (log2_val) | `$a0` | `$a1` |

sum_sq is seated in $a1 on both sides. Every one of the 7 differing insns is now a consequence of
ONE fact: our copy takes $v1, the target's takes $a0.

**Why it takes $v1 (measured, not inferred).** tools/ra_solver, run on the spliced body:
- global model (`extract.py` + `simulate.py`): sort order MATCH, dispositions 10/12. The three
  contested globals are pseudo 77 = sum_sq (nrefs 8, livelen 16), 98 = log2_val (nrefs 5, livelen 7,
  prefs [4]), 78 = scale (nrefs 6, livelen 15). The copy is NOT among them.
- local model (`local_extract.py` + `local_alloc.py`, order 6/6 blocks, assign 29/34): the copy is
  `blk 6 qty 0` — birth 4, death 5, refs 2, got 3. It is a BLOCK-LOCAL quantity, allocated by
  local_alloc BEFORE global_alloc runs, and `find_free_reg` scans ascending over
  `fixed_reg_set | union(regs_live_at[4..5])`. $2 is in that set (the island's own clobber list);
  nothing else is; $3 is therefore the first free register. The next qty in the block is born at 6,
  so no reordering of block 6's own quantities can occupy $3 across [4,5].
- `inverse.py local --block 6 --goal '{"0": 4}' --depth 2` returns exactly ONE minimal vector:
  **[live_extend] qty 0 dies later (5 -> 9)**.

**The structural consequence.** For the target's copy to take $a0 it cannot be a block-local qty of
the else block at all — it must be a cross-block GLOBAL allocno, defined in the pre-branch block and
used at the island (which is also the simplest reading of the target's delay slot: the copy is the
insn immediately before `beqz`, not a reorg hoist out of the branch target). A tied asm operand
structurally cannot produce that: GCC emits the copy immediately before the asm, inside the else
block. And s1 H7 already killed the direct spelling (`lz_in = sum_sq;` before the branch) — cse.c
canon_reg deletes it.

**The live-extend vector, spelled honestly, overshoots.** The only real consumer of sum_sq's value
after the island is the LUT index `sum_sq >> shift_a` at position ~18, not ~9. Re-sourcing it from
lz_in (s5/w8.c) makes sum_sq die at the copy, cse re-unifies the two values, and the copy insn
disappears: sandbox 12, 148 emitted lines. Banked as
`rejected/lzc-input-tied-copy-live-extend-to-lut-index-cse-refolds.c`.

**Campaign telemetry.** s5-w1-tiedcopy (tmp/perm_80018094d, base score 55, --stack-diffs, -j 8):
38,542 iterations, three finds — output-35-1 at **23 s** (the do-while(0) wrap; the entire yield),
output-45-1 at 880 s and output-55-1 at 942 s (both non-improving). Harvested with --stop; 0 live
campaigns at session end. Consistent with the fresh-seed rule: this basin yielded immediately or
not at all.

**Inert on this chassis (measured, do not re-derive):** declaration scope of the tied output pseudo
(function-top vs else-arm: byte-identical); the permuter's split sum accumulation
(`new_var = dx*dx + dy*dy; sum_sq = new_var + dz*dz;` — sandbox 10, i.e. no change from the tied
form alone); a dead `log2_val = lz_in;` store as a live-extend carrier (sandbox 7, diagnostic only).

- [s5] HEAD floor re-measured at dispatch: candidate.c spliced into src/code6cac.c gives sandbox --disable all == 7 (153/153 insns, rules_dropped 0, cheat_asm_stripped 20). Unchanged from s4.

- [s5] s4's frontier framed a ruling-request asking whether the authorized LZC island may name $a0 in its own asm text. That request is NOT needed and should not be filed: a tied asm output operand produces the missing `move a0,a1`-shaped copy with ordinary operand plumbing, in the target's exact delay-slot position, with no register pin.

- [s5] On the s5 w6 chassis all 7 differing insns are consequences of ONE register name: our island-input copy takes $v1, the target's takes $a0. sum_sq is seated in $a1 on both sides; the four downstream hunks (srl a0 vs a1, li_v0 in $a0 vs $v0, log2_val in $a0 vs $a1 twice) follow from $a0 being free in ours.

- [s5] ra_solver global model on this body: sort order MATCH, dispositions 10/12; the two misses are pseudo 77 (sum_sq, sim=5 dump=4) and pseudo 98 (log2_val, sim=4 dump=5) - the simulator already predicts the TARGET's seats for both, i.e. global.c is not where the divergence lives.

- [s5] ra_solver local model: the copy is blk 6 qty 0 (birth 4, death 5, refs 2, got $3), allocated last in the block; $2 is in find_free_reg's `used` set via the island's clobber list, nothing else is, and the block's next qty is born at 6 - so $3 is unavoidable for a block-local copy here.

- [s5] Structural conclusion the next session should start from: the target's copy is a cross-block GLOBAL allocno (defined in the pre-branch block, used at the island). A tied asm operand cannot produce that shape, and s1 H7 already killed the direct `lz_in = sum_sq;` pre-branch spelling (cse.c canon_reg deletes it). The open question is what ordinary C puts a cse-surviving copy in the PRE-BRANCH block.

- [s5] Inert on this chassis, do not re-derive: tied-output declaration scope (function-top vs else-arm, byte-identical); the permuter's split sum accumulation `new_var = dx*dx + dy*dy; sum_sq = new_var + dz*dz;` (sandbox 10, no change from the tied form alone); a dead `log2_val = lz_in;` store as a live-extend carrier (sandbox 7, diagnostic only).

- [s5] Campaign telemetry: s5-w1-tiedcopy, base score 55, 38,542 iterations, 3 finds, entire useful yield at 23 s. Harvested with --stop; `permuter_campaign.py status` reports 0 live campaigns at session end.

- [s5] src/code6cac.c was reverted to its INCLUDE_ASM state at session end; the tree carries only memory/grind ledger changes.

## s6 (structural, 2026-09-09) — the s5 frontier's central mechanism is REFUTED; the residual is a global.c conflict-set problem, not a local-vs-global one

Chassis re-measured at dispatch: candidate.c spliced into src/code6cac.c gives
`sandbox func_80018094 --disable all` == **7** (rules_dropped 0, cheat_asm_stripped 20).
The s5 w6 body (rejected/tied-copy-dowhile-wrap-…-equal-floor-7.c) re-measures **7** as well.
Unchanged from s4/s5.

**1. The pass that deletes the pre-branch copy is cse.c, and the predicate is now named.**
s1 H7 ("`lz_in = sum_sq;` before the branch is deleted") was re-measured on the floor-7 chassis at
two declaration scopes (else-arm block, function top): both are BYTE-IDENTICAL to candidate.c
(s6/v1.s == s6/v2.s == s6/w0.s). Dump attribution, not inference: in `s6/d_v1/v1.i.jump` the copy
is `insn 103 (set (reg/v:SI 99) (reg/v:SI 77))` and the island asm's input operand is `reg/v:SI 99`;
in `v1.i.cse` the operand is `reg/v:SI 77` and insn 103 is gone. The substitution is cse's
extended-path walk: `cse_end_of_basic_block`'s follow-jumps arm
(tools/gcc-2.7.2/cse.c:8102-8117) extends the path THROUGH the `beqz` into the island block
because (a) `LABEL_NUSES (island label) == 1` and (b) the insn before the label is the BARRIER
emitted after the LUT arm's `j` to the merge. Both predicates are properties of the target's own
control-flow shape, so no declaration-scope respelling touches them.
Attempting to break (a) with a redundant second conjunct (`sum_sq < 0x400 && sum_sq >= 0`,
s6/d1.c, s6/d2.c) does nothing: jump optimisation folds the redundant test away and the output is
byte-identical to candidate.c.

**2. THE REFUTATION.** s5's frontier said: "the target's copy must be a cross-block GLOBAL allocno
defined in the pre-branch block; a tied asm operand structurally cannot produce that shape, and
that is why ours takes $v1". Measured with a diagnostic compile (scratch only, never a build
path): `cc1 … -fno-cse-follow-jumps -fno-cse-skip-blocks` on s6/v1.c makes the pre-branch copy
SURVIVE, and `v1.i.lreg` confirms reg 99 is not a local qty while `v1.i.greg` lists it in
";; 13 regs to allocate: 98 78 120 77 99 …" — i.e. it is exactly the cross-block GLOBAL allocno
s5 asked for, and reorg parks it in the `beqz` delay slot in the target's exact position
(s6/v1.diag.s:95 `move $3,$5` under `beq $2,$0,.L6`). **It still takes $3, not $a0.**
So local-vs-global allocation is NOT the discriminator. Root cause: `tools/gcc-2.7.2/config/mips/mips.h`
defines no `REG_ALLOC_ORDER`, so global.c's `find_reg` and local-alloc's `find_free_reg` both scan
hard regs ascending; with $2 blocked by the island's clobber list, $3 is the first free register
for either allocator.

**3. What the greg dump says the target's seat actually requires.** From `s6/d2_v1/v1.i.greg`
(the surviving-copy chassis): allocation order `98 78 120 77 99 95 …`; `98 preferences: 4`;
`98 conflicts: 72 73 77 98 2 3 29`; `99 conflicts: 72 73 77 99 2 12 29`;
dispositions `77 in 5, 78 in 3, 98 in 4, 99 in 3`. The copy (99) reaches $3 because it does NOT
conflict with 78 (`scale`, seated in $3). The target's seats are copy=$a0(4), log2_val=$a1(5),
sum_sq=$a1(5), li_v0=$v0(2). Reproducing them requires TWO simultaneous conflict-set changes:
(i) 99 must conflict with 78 so $3 is closed to it, and (ii) 98 must stop conflicting with 77 so
it can share $5 with sum_sq. Neither is a spelling of the copy — both are liveness facts about
`scale` and `log2_val`.

**4. The s4 log2_val staging is load-bearing and is also (ii)'s obstacle.** Removing it from the
tied chassis (s6/t4.c) measures sandbox **11**; removing it from the copy chassis moves sum_sq to
$a0 (s6/v5.diag.s `move $3,$4`). The staging is precisely what makes 98 conflict with 77 — which
is what keeps sum_sq at $a1 (worth 3 insns since s4) and simultaneously forces 98 onto its
preferred $4, the register the target needs for the copy.

**5. Declaration-order levers measured inert or worse.** Hoisting `s32 li_v0 = -2;` above the
island (so its qty is born before the copy) leaves the copy at $3 and pushes it OUT of the delay
slot (s6/t1.s:108 vs t0.s:95) — worse. Hoisting only the declaration (s6/t2.c) is byte-identical
to w6. Swapping the inner arms so the island is the fall-through (s6/v3.c) costs +1 insn, and the
pre-branch copy on top of it (s6/v4.c) is byte-identical to v3 (cse folds it without needing the
follow-jumps path at all).

- [s6] HEAD chassis re-measured: candidate.c == sandbox 7; the s5 w6 tied body == sandbox 7.
- [s6] cse.c is the pass that deletes the pre-branch copy; the gate is cse_end_of_basic_block's follow-jumps arm at tools/gcc-2.7.2/cse.c:8102 (LABEL_NUSES==1 + preceding BARRIER). Dump-proven via v1.i.jump vs v1.i.cse.
- [s6] REFUTED (s5 frontier item 1): making the copy a cross-block GLOBAL allocno does NOT give it $a0. Diagnostic -fno-cse-follow-jumps build keeps it global (greg allocno 99) in the target's exact delay-slot position and it still takes $3.
- [s6] Mechanism for that: MIPS has no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so both allocators scan ascending; $2 is blocked by the island clobber and $3 is the first free reg.
- [s6] The target's seats need TWO conflict-set changes, both about OTHER pseudos: the copy must conflict with 78 (scale, $3) and 98 (log2_val) must stop conflicting with 77 (sum_sq) so both can sit at $5.
- [s6] The s4 log2_val staging is load-bearing (removing it: tied chassis sandbox 7 -> 11) AND is what creates the 98-77 conflict blocking (ii).

- [s6] HEAD chassis re-measured this session: candidate.c spliced into src/code6cac.c gives `sandbox func_80018094 --disable all` == 7 (rules_dropped 0, cheat_asm_stripped 20). The s5 w6 tied body re-measures 7 as well. The dispatch brief's 'measurement unavailable' is resolved: the ledger floor 7 is correct.

- [s6] PASS ATTRIBUTION (dump-proven): the pre-branch island-input copy is deleted by cse.c, not by combine or flow. tmp/grind/func_80018094/s6/d_v1/v1.i.jump carries `insn 103 (set (reg/v:SI 99) (reg/v:SI 77))` with the asm operand as reg 99; v1.i.cse has the operand as reg 77 and no insn 103.

- [s6] The gate is cse_end_of_basic_block's follow-jumps arm (tools/gcc-2.7.2/cse.c:8102-8117): it extends cse's path through the `beqz` into the island block because LABEL_NUSES(island label)==1 and the insn preceding the label is the BARRIER emitted after the LUT arm's `j` to the merge. Both are properties of the target's own control-flow shape.

- [s6] REFUTATION of the s5 frontier: a diagnostic compile with -fno-cse-follow-jumps -fno-cse-skip-blocks makes the pre-branch copy survive as a genuine cross-block GLOBAL allocno (absent from v1.i.lreg's local-qty list; present in v1.i.greg's ';; 13 regs to allocate: 98 78 120 77 99 ...'), parked by reorg in the `beqz` delay slot in the target's exact position - and it is still assigned $3, not $a0.

- [s6] Mechanism for that refutation: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so global.c find_reg and local-alloc find_free_reg both scan hard registers ascending. With $2 closed by the island's clobber list, $3 is the first free register for either allocator. Local-vs-global is not the discriminator for this seat.

- [s6] greg conflict data on the surviving-copy chassis (s6/d2_v1/v1.i.greg): allocation order `98 78 120 77 99 95 127 132 137 72 93 73 92`; `98 preferences: 4`; `98 conflicts: 72 73 77 98 2 3 29`; `99 conflicts: 72 73 77 99 2 12 29`; dispositions `77 in 5, 78 in 3, 98 in 4, 99 in 3`. The copy reaches $3 only because it does NOT conflict with 78 (scale).

- [s6] The target's seats are copy=$a0, log2_val=$a1, sum_sq=$a1, li_v0=$v0. Reproducing them needs two simultaneous conflict-set changes about OTHER pseudos - 99 must conflict with 78 (scale, $3) so $3 closes, and 98 must stop conflicting with 77 so it can share $5 with sum_sq. Neither is a spelling of the copy itself.

- [s6] The s4 log2_val staging is load-bearing AND is the obstacle to the second of those changes: removing it costs 4 insns on the tied chassis (sandbox 7 -> 11) and moves sum_sq to $a0 on the copy chassis, because the staging is exactly what creates the 98-77 conflict.

- [s6] Declaration-order levers measured: hoisting `s32 li_v0 = -2;` above the island keeps the copy at $3 and pushes it out of the delay slot (worse); hoisting only its declaration is byte-identical; swapping the inner arms costs +1 insn and lets cse fold the copy without the follow-jumps path.

- [s6] src/code6cac.c was reverted to its INCLUDE_ASM state at session end; the tree carries only memory/grind ledger changes and tmp/ scratch.
