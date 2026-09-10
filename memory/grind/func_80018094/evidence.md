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

## s7 (enumerate, 2026-09-09) — the LZC arm's local spelling space is SWEPT AND FLAT at 7; the s6 frontier's `scale`-liveness lever is KILLED

Chassis re-measured at dispatch by every sweep's own baseline splice: candidate.c spliced into
src/code6cac.c gives `sandbox func_80018094 --disable all` == **7** (build_insns 153). Unchanged
from s4/s5/s6. (The dispatch brief's "measurement unavailable" is again resolved to 7.)

**Method.** `tools/spelling_enum.py` + `tools/sweep_variants.py` per the systematic-spelling-sweep
mandate. Five ENUM regions were marked on the floor-7 chassis (candidate.c body), each written in
FULLY-NAMED form (every sub-expression that could be a local IS a local), and swept exhaustively.
sweep_variants was run through a repo-pinned wrapper `tmp/grind/func_80018094/s7/run_sweep.sh`
(absolute `git rev-parse --show-toplevel` cd inside the script) invoked via `bash tools/wsl.sh`,
because `tools/wteng.ps1` only forwards to `engine.cli`/`make` and the contamination guard blocks an
unpinned sweep invocation. src/code6cac.c is byte-restored by the tool after every sweep and the tree
was verified clean (`git status --porcelain` shows only metrics/events.jsonl) at session end.

**ENUMERATION RESULTS (1,039 measured spellings this session).**

| region (ENUM source) | axis | N | best | at floor 7 | histogram |
|---|---|---|---|---|---|
| `s7/e1_src.c` — LZC arm SUFFIX (shift_a, shift_b, idx, lut, wide, rsh) on the STAGED chassis | decl-keep/inline + order | 161 | **7** | 79 | 7x79, 15x46, 12x36 |
| `s7/e1_src.c` — same region | + commutative operand swaps | 296 | **7** | 79 | 7x79, 15x46, 27x46, 24x40, 12x36, 22x29, 28x16, 20x4 |
| `s7/e2_src.c` — LZC arm PREFIX (`lw_v1`, `li_v0`, `lz` feeding the staging) | decl-keep/inline + order | 10 | **7** | 10 | 7x10 |
| `s7/e3_src.c` — the shared `scale = ((log2_val << 6) / 500) + 0xC0;` tail | decl-keep/inline + order | 4 | **7** | 1 | 7x1 (the fully-inlined form = baseline), 54x3 |
| `s7/e4_src.c` — PRE-BRANCH block (dx/dy/dz, the three squares, sum_sq) | decl-keep/inline + order | 550 | 42 | 0 | 50x296, 44x104, 42x69, 47x44, 45x28, 43x9 |
| `s7/enum_src.c` — whole LZC arm, staging replaced by a FRESH named local `stage` | 2 endpoints sampled | 2 | 13 | 0 | 13x2 (fully-named and fully-inlined) |
| `s7/hand/` — `scale`-liveness forms (the s6 frontier's item 2) | hand | 4 | 30 | 0 | see below |
| `s7/hand2/` — declaration scope / arm order / staging-carrier identity | hand | 8 | **7** | 3 | see below |
| `s7/hand3/` — pre-branch island-input copy carried by a REUSED local | hand | 4 | **7** | 2 | see below |

**1. The LZC arm's local spelling space is EXHAUSTED and flat.** 471 spellings of the arm's two
sub-regions (161 + 296 with swaps + 10 prefix + 4 tail) contain no form better than 7, and 169 of
them sit exactly AT 7 — i.e. the residual is completely insensitive to which sub-expressions of the
LZC arm are named, in what order the declarations sit, and which way round each commutative operand
pair is written. Whatever the last 7 insns are, they are not a naming/ordering/commutativity fact
about that block. This is the strongest evidence to date that the residual lives OUTSIDE the arm's
local spelling space (block structure, the island's own operand plumbing, or the object model).

**2. The s6 frontier's second item — make `scale` live at the island entry — is KILLED, and badly.**
Every form that makes `scale` live earlier costs 15-47 insns over the floor:
  * `scale = ((log2_val << 6)/500) + 0xC0;` duplicated into BOTH inner arms with the shared tail
    removed (duplicated-statement-into-arms, a sanctioned family): **54**
    (`rejected/scale-tail-duplicated-into-both-inner-arms-costs-47.c`)
  * `scale = 0;` written before the outer if-chain: **41**
    (`rejected/scale-prechain-default-write-costs-34.c`)
  * `scale = 0x100;` written before the chain: **38**
  * per-arm `s32 arm_scale;` copied out at the merge (`scale = arm_scale;`): **30**
    (`rejected/per-arm-scale-local-merge-copy-costs-23.c`)
  * `scale` used as the pre-branch carrier of the island input (`scale = sum_sq;` + `"r"(scale)`),
    which is the cheapest possible way to make it live there: **22**
    (`rejected/prebranch-island-input-carrier-scale-costs-15.c`)
  * naming the scale tail's intermediates at all (`s32 sh6 = log2_val << 6; s32 dv = sh6/500;`): **54**
    (`rejected/named-scale-tail-intermediates-cost-47.c`)
So s6's conflict-set requirement (i) — "99 must conflict with 78 (`scale`, $3) so $3 closes to the
copy" — is reachable in C, but every C form that reaches it restructures the merge and pays 15-47
insns. The lever is real and the price is prohibitive on this chassis.

**3. The staging carrier's IDENTITY is load-bearing, not just its presence.** Re-staging the LZCR
read through a different currently-dead local instead of `log2_val` costs insns:
`dx` / `dy` / `dz` -> **14** (build_insns 154, i.e. an extra insn is emitted);
`scale` -> **17**; a FRESH named local (`stage`, the s7/enum_src.c family) -> **13**.
Only `log2_val` gives 7. This sharpens s6's finding: the staging works because the carrier is the
pseudo whose `preferences: 4` the greg dump shows, not merely because an extra reference exists.
(`rejected/staging-carrier-dx-instead-of-log2val-costs-7.c`)

**4. Declaration scope and block structure inside the else arm are INERT (all at 7).**
`s32 log2_val;` hoisted to the function's top declaration list: 7. Removing the redundant inner
`{ ... }` block that scopes `log2_val`: 7. Spelling the LZCR mask constant `~1` instead of `-2`: 7.
Do not re-derive these.

**5. Outer arm ORDER is not free.** Reordering the outer chain to `if (sum_sq < 0) ... else if
(sum_sq > 250000) ... else ...` costs **20** (`rejected/outer-arm-order-swap-lt0-first-costs-13.c`).
The target's chain order (`> 250000` first) is fixed.

**6. The pre-branch diff/sum block: the INTERLEAVED store shape is worth ~35 insns and no spelling
of the deferred shape recovers it.** spelling_enum classifies `SCRV->x = dx;` as an ANCHOR (it is
not a bare `name = expr;`), so all 550 e4 variants emit the three scratchpad stores AFTER the whole
diff+square+sum computation instead of interleaved with it as candidate.c does. Every one of those
550 scores 42-50 (best 42, build_insns 155). CAVEAT for the next session: e4 therefore measured the
DEFERRED-STORE block shape, not candidate.c's interleaved shape — the pre-branch block's naming
space in the interleaved shape is still UNSWEPT and would need the stores pinned in place (either a
spelling_enum change that treats a memory store as a reorderable statement, or hand-built variants).
What e4 does prove is that the interleaved store placement is load-bearing to the tune of ~35 insns
and is not recoverable by any renaming of the deferred form.
(`rejected/prebranch-diff-block-deferred-scrv-stores-costs-35.c`)

**7. A pre-branch island-input copy carried by a REUSED dead local is folded away exactly like the
fresh-local spelling (s1 H7).** `dx = sum_sq;` / `dz = sum_sq;` before the outer chain with the
island fed `"r"(dx)` / `"r"(dz)` both score **7 with build_insns 153** — identical to candidate.c,
i.e. cse deletes the copy just as it does for a fresh local. The carrier being an existing local
does not change cse's canon_reg substitution. `log2_val = sum_sq;` as carrier costs 9.

**8. The DATA-MODEL "declaration pun" flag on `D_8008D118` is a FALSE POSITIVE for this function.**
The dispatch brief flags candidate.c:89/124 (`*(&D_8008D118 + sum_sq)`) as a declaration pun that
would FAIL layer-1. But `src/code6cac.c:19` already carries `extern u8 D_8008D118;` and the
**COMPLETED-C** sibling in the same file uses the byte-identical spelling at src/code6cac.c:756 and
:783 (`log2_val = ((u32)((u8)(*((&D_8008D118) + dist_sq)))) >> 3;`). src/code6cac_b.c does the same
in six more places. The spelling is the project-established, oracle-proven form for this LUT; it is
not a pun this function invented.

- [s7] HEAD chassis: candidate.c == sandbox 7 (build_insns 153), unchanged from s4/s5/s6.
- [s7] ENUMERATION: 1,039 spellings measured; best 7; 169 spellings sit at the floor; ZERO improvements.
- [s7] ENUMERATION (LZC arm, both sub-regions, incl. commutative swaps): 471 spellings, best 7, 169 at the floor — the arm's local naming/order/commutativity space is exhausted and flat.
- [s7] KILLED (s6 frontier item 2): every C form that makes `scale` live at the island entry costs 15-47 insns (duplicate-into-arms 54, prechain 0 -> 41, prechain 0x100 -> 38, per-arm local + merge copy 30, scale-as-island-carrier 22). Sanctioned family, prohibitive price.
- [s7] The staging carrier must be `log2_val` specifically: dx/dy/dz -> 14, scale -> 17, a fresh local -> 13, log2_val -> 7.
- [s7] Inert at 7 (do not re-derive): log2_val declared at function top; the inner `{}` scope block removed; `~1` for `-2`.
- [s7] Outer arm order swap (`< 0` first) costs 20 — the target's chain order is fixed.
- [s7] A pre-branch island-input copy carried by an existing dead local (dx, dz) is folded by cse exactly like the fresh-local spelling: 7, build_insns 153, byte-identical to candidate.c. Carrier identity does not defeat cse canon_reg.
- [s7] The pre-branch block's 550-spelling sweep measured the DEFERRED-STORE shape (spelling_enum treats `SCRV->x = dx;` as an anchor and moves it last): best 42. The interleaved shape's naming space is still UNSWEPT.
- [s7] The `D_8008D118` declaration-pun flag is a false positive: `extern u8 D_8008D118;` + `*(&D_8008D118 + i)` is the spelling the COMPLETED-C sibling at src/code6cac.c:756/783 ships.
- [s7] src/code6cac.c restored byte-exact by sweep_variants; tree carries only memory/grind ledger changes and tmp/ scratch.

- [s7] HEAD chassis re-measured this session by every sweep's own baseline splice: candidate.c spliced into src/code6cac.c gives sandbox func_80018094 --disable all == 7 (build_insns 153). Unchanged from s4/s5/s6; the brief's 'measurement unavailable' resolves to 7.

- [s7] ENUMERATION: 1,039 spellings measured this session; best 7; 169 spellings sit exactly at the floor; ZERO improvements.

- [s7] ENUMERATION (LZC arm, both sub-regions, with and without the commutative-swap axis, plus the shared scale tail): 471 spellings, best 7, 169 at the floor. The arm's local naming / declaration-order / commutativity space is exhausted and flat, so the residual is not a spelling fact about that block.

- [s7] ENUMERATION (pre-branch diff/sum block, deferred-store shape): 550 spellings, best 42, worst 50. The interleaved SCRV store placement candidate.c uses is worth ~35 insns and is not recoverable by renaming.

- [s7] The s6 frontier's scale-liveness lever is measured and prohibitively expensive: duplicated-statement-into-arms 54, pre-chain `scale = 0;` 41, pre-chain `scale = 0x100;` 38, per-arm local + merge copy 30, scale carrying the island input 22, naming the scale tail's intermediates 54.

- [s7] The staging carrier must be log2_val specifically: dx/dy/dz -> 14, scale -> 17, a fresh named local -> 13, log2_val -> 7.

- [s7] Inert at 7 (do not re-derive): log2_val declared at the function top; the redundant inner brace scope removed; `~1` spelled for `-2`.

- [s7] Outer arm order swap (`sum_sq < 0` tested first) costs 20 — the target's chain order (`> 250000` first) is fixed.

- [s7] A pre-branch island-input copy carried by an existing dead local (dx, dz) is folded by cse exactly like the fresh-local spelling of s1 H7: 7 with build_insns 153, byte-identical to candidate.c.

- [s7] src/code6cac.c was restored byte-exact by sweep_variants after every sweep; git status --porcelain shows only metrics/events.jsonl plus the memory/grind ledger changes at session end.

- [s7] Tooling note for later sessions: tools/wteng.ps1 only forwards to engine.cli/make, so sweep_variants cannot be pinned through it. The working pattern is a repo-pinned wrapper script (tmp/grind/<func>/sN/run_sweep.sh, absolute `cd "$(git rev-parse --show-toplevel)"` inside) invoked as `bash tools/wsl.sh 'bash tmp/.../run_sweep.sh --variants ...'`. Throughput is ~1.05 s per variant, single-threaded.

## s8 (synthesis, 2026-09-09) — the residual reduces to ONE dump-derived requirement: the island-input copy must CONFLICT with `scale`

Chassis re-measured at dispatch (`tools/fake_ablate.py`, tmp/grind/func_80018094/s8/ablate.json):
candidate.c == sandbox **7** (build_insns 153). The brief's "measurement unavailable" resolves to 7
again, unchanged since s4. FAKE-ablation of the one detected FAKE unit (the `log2_val = li_v0;`
staging) gives sandbox **26** (build_insns 152) — the staging is worth 19 insns, not the 4 the s6
note implied, and it is the single most load-bearing construct in the body. (The `sp_tmp[4]`
oversized-locals FAKE marker sits on a comment-body continuation line, so fake_ablate does not
treat it as an ablatable unit; it was not re-measured this session.)

### 1. THE SYNTHESIS: the seat problem is ONE conflict, and the dump already says which

Reading the target bytes (asm/funcs/func_80018094.s:60-114) together with the s6 greg dump
(tmp/grind/func_80018094/s6/d2_v1/v1.i.greg) closes the question s5-s7 kept circling:

| pseudo | ours (surviving-copy chassis) | target | evidence |
|---|---|---|---|
| 77 `sum_sq` | $a1 (5) | $a1 (5) | `.s:91 srav $v0,$a1,$v1` — SAME |
| 78 `scale` | $v1 (3) | $v1 (3) | `.s:103 sll $v1,$a1,6`, `.s:109 addiu $v1,$v0,0xC0` — SAME |
| 98 `log2_val` | $a0 (4), `98 preferences: 4` | $a1 (5) | `.s:77 srl $a1,$v0,3`, `.s:99 srav $a1,$a0,$v0` |
| 99 island-input copy | $v1 (3) | $a0 (4) | `.s:72 addu $a0,$a1,$zero` (reorg-parked in the `beqz` delay slot) |

So `sum_sq` and `scale` are ALREADY seated correctly. Only two pseudos are wrong, and the greg
conflict lines say exactly why: `99 conflicts: 72 73 77 99 2 12 29` — 99 does NOT conflict with 78,
and 78 holds $3, so global.c's ascending `find_reg` scan hands 99 the first free register, $3
($2 is closed by the island's own clobber list). **For 99 to reach $a0 the ONLY change needed is
that 99 conflicts with 78** — $3 then closes and $4 is the next free register (99 does not conflict
with 98, so sharing $4 with `log2_val` is legal; the second, cosmetic half of the s6 requirement,
98 moving off $4 to share $5 with 77, is what puts `log2_val` at $a1).

This is a strictly SHARPER statement than s6's "two simultaneous conflict-set changes": requirement
(i) is necessary AND sufficient for the copy's seat, and requirement (ii) only moves `log2_val`.

### 2. The preference channel is closed (mechanism read, not guessed)

Could 99 reach $4 without conflicting with 78, i.e. by *preferring* $4?
- local-alloc: `qty_phys_copy_sugg` / `qty_phys_sugg` (tools/gcc-2.7.2/local-alloc.c:1860-1898) are
  populated ONLY by `combine_regs` when one side of a copy is a HARD register. There is no hard-reg
  traffic in the LZC block, so a block-local copy there can never carry a suggestion, and
  `find_free_reg`'s `just_try_suggested` path (local-alloc.c:2206-2213) never fires for it.
- global.c: preferences propagate through `expand_preferences` (tools/gcc-2.7.2/global.c:838-871)
  only across a `single_set` whose SOURCE carries a REG_DEAD note. Our copy's source is `sum_sq`,
  which is still live afterwards (the target re-uses it as the LUT index, `.s:91`), so no note, no
  propagation. The dump confirms it: 99 has no `preferences:` line at all.

Both allocators therefore fall through to the ascending scan for this pseudo, and the ONLY lever on
its seat is the conflict set. This retires the whole "make the copy materialise differently"
family (s5's tied `"=r"/"1"` operand, s6's cross-block global copy): s6 already proved a genuine
cross-block GLOBAL copy also takes $3, and the mechanism above says why no spelling of the copy
itself can change that.

**Consequence for the s7 frontier: the tied-operand ruling-request is NOT worth filing.** Its
premise ("the tied operand is the thing that gets the copy into the target's position") is true but
irrelevant — the copy's POSITION was never the residual, its SEAT is, and the seat is decided by a
conflict with `scale` that the tied operand does not create. `docs/reference/sotn-construct-index.md`
carries no inline-asm-operand class at all, so there is also no precedent to cite for it.

### 3. Three families measured and flat this session

**(a) The UNSTAGED family (s7 frontier item 3) — sampled 344 of 3,440, best 13.**
The pre-generated `tmp/grind/func_80018094/s7/enum/` set (LZC arm written with a FRESH named local
`stage` carrying the LZCR read instead of reusing `log2_val`) was swept in ten chunks; chunk 0
(344 spellings, `tmp/grind/func_80018094/s8/res_chunk_00.json`) came back **13 x141, 15 x105,
18 x98** — nothing at or below the floor, and 13 is exactly the value s7 measured for both sampled
endpoints. Throughput was ~2 s/variant (not the 1.05 s s7 recorded), so the remaining nine chunks
were killed rather than blow the session; the chunk-0 histogram is a 10% sample of the family with
a 6-insn gap to the floor. Combined with s7's staged-family sweep this is a two-family statement:
BOTH sides of s6's conflict-requirement (ii) are flat.

**(b) The pre-branch INTERLEAVED naming space (s7 frontier item 1) — 96 spellings, best 7, 12 ties.**
s7 could not sweep this because `spelling_enum` anchors memory stores; `tmp/grind/func_80018094/s8/gen_pre.py`
generates the interleaved shape directly and enumerates five axes over it: naming `((s32 *)arg0[1])`
as a pointer local, naming the three `arg1[10..12]` reads, naming the three squares, four sum
associations ((a+b)+c / a+(b+c) / (a+c)+b / split accumulation `sum=..; sum+=..;`), and three
diff/store shapes (diff-into-local-then-store / store-then-read-back / store-only-with-squares-read-
from-SCRV). Histogram: 7x12, 9x12, 20x3, 23x3, 25x12, 26x12, 27x6, 28x3, 30x6, 36x6, 38x3, 39x6,
40x6, 41x6. Twelve spellings tie the floor, none beat it. The pre-branch block is therefore in the
same state as the LZC arm: naming/order/association-insensitive.

**(c) NEW and KILLED: `scale` and `log2_val` spelled as ONE variable — 17 on both chassis.**
This is the shape the target's own seats suggest (its $a1 carries the arm result AND feeds the
`(x<<6)/500 + 0xC0` tail, and its $v1 carries `scale`), it is ordinary C, and no prior session had
tried it. Measured: **17** on the floor-7 body and **17** on the s5 w6 tied body, both at
build_insns 153 — 10 insns of pure seat churn. It does not create the 99<->78 conflict, because the
merged pseudo is still DEFINED inside each arm and so is not live at the island entry. Banked as
`rejected/merge-scale-and-log2val-one-variable-costs-10.c` (+ `-tied-chassis-` twin).

### 4. What this leaves

Requirement (i) — `scale` live across the island-input copy — is the whole residual, and s7 priced
every spelling that reaches it by making `scale` live EARLIER at 15-47 insns (duplicate-into-arms 54,
pre-chain writes 41/38, per-arm local + merge copy 30, scale-as-carrier 22). s8 adds the merge form
at 17, which is the cheapest member of that family so far by 5 insns. The untried direction is the
OTHER side of the same conflict: extend the COPY's live range forward into `scale`'s range instead of
extending `scale`'s backward — which is precisely the one minimal vector s5's `inverse.py` returned
(`[live_extend] qty 0 dies later (5 -> 9)`) and which s5 only ever spelled as an overshoot (consuming
the tied output at the LUT index, position ~18, where cse refolds it: sandbox 12).

- [s8] Chassis re-measured by fake_ablate: candidate.c == sandbox 7 (build_insns 153). Ablating the one detected FAKE unit (the log2_val staging) gives 26 (build_insns 152) — the staging is worth 19 insns.
- [s8] SYNTHESIS (dump + target bytes): `sum_sq` ($a1) and `scale` ($v1) are already seated exactly as the target seats them. Only `log2_val` (ours $a0, target $a1) and the island-input copy (ours $v1, target $a0) differ.
- [s8] The single necessary-and-sufficient change for the copy's seat is that allocno 99 CONFLICTS with allocno 78 (`scale`): 78 holds $3, $2 is closed by the island clobber, so with $3 closed the ascending scan hands 99 $4 — the target's seat. Sharing $4 with 98 is legal (99 and 98 do not conflict).
- [s8] The PREFERENCE channel is mechanically closed for this pseudo: local-alloc suggestions need hard-reg copy traffic in the same block (tools/gcc-2.7.2/local-alloc.c:1860-1898) and there is none; global.c preference propagation needs a REG_DEAD note on the copy's source (tools/gcc-2.7.2/global.c:838-871) and `sum_sq` outlives the copy. The greg dump shows 99 with no `preferences:` line. So no spelling OF THE COPY can move its seat — only a conflict can.
- [s8] Therefore the s5/s7 frontier item "file a ruling-request about the tied =r/1 island operand" should be DROPPED: the tied operand fixes the copy's position, which was never the residual, and sotn-construct-index.md has no inline-asm-operand class to cite anyway.
- [s8] KILLED (instance): the UNSTAGED LZC-arm family. 344 of the 3,440 pre-generated spellings swept; histogram 13x141, 15x105, 18x98; best 13, six insns above the floor and equal to s7's two sampled endpoints.
- [s8] KILLED (instance): the pre-branch INTERLEAVED naming space (s7 frontier item 1). 96 spellings over 5 axes (pointer naming, arg-read naming, square naming, 4 sum associations, 3 diff/store shapes): best 7, twelve ties, nothing better.
- [s8] KILLED (instance): spelling `scale` and `log2_val` as ONE variable — 17 on the floor-7 body and 17 on the s5 w6 tied body (build_insns 153 both). Ordinary C, never previously tried, and the cheapest member of the scale-liveness family so far, but still +10.
- [s8] Tooling note: sweep throughput this session was ~2.0 s/variant, not s7's 1.05 s — budget 3,440-variant sweeps at ~2 hours, i.e. they do NOT fit in one session. Chunk them (tmp/grind/func_80018094/s8/sweep_enum.sh writes one JSON per chunk so a kill keeps the partial histogram).
- [s8] src/code6cac.c was restored with `git checkout --` after the killed sweep; `git status --porcelain` shows only metrics/events.jsonl plus the memory/grind ledger changes at session end.

- [s8] Chassis re-measured by fake_ablate at dispatch: candidate.c == sandbox 7 (build_insns 153); ablating the log2_val staging gives 26 (build_insns 152), so the staging is worth 19 insns.

- [s8] Target-vs-dump seat table: sum_sq $a1 = target $a1; scale $v1 = target $v1; log2_val ours $a0 vs target $a1; island-input copy ours $v1 vs target $a0. Only two pseudos are misseated.

- [s8] greg line `99 conflicts: 72 73 77 99 2 12 29` omits 78 (scale, at $3); with $2 closed by the island clobber and MIPS having no REG_ALLOC_ORDER, the ascending find_reg scan hands the copy $3. Closing $3 by making 99 conflict with 78 is necessary and sufficient for the copy to reach $a0.

- [s8] The preference channel is closed for that pseudo: local-alloc suggestions require hard-reg copy traffic in the same block (tools/gcc-2.7.2/local-alloc.c:1860-1898; none exists in the LZC block) and global.c preference propagation requires a REG_DEAD note on the copy's source (tools/gcc-2.7.2/global.c:838-871; sum_sq outlives the copy). 99 carries no `preferences:` line in the dump while 98 carries `preferences: 4`.

- [s8] Consequence: the s5/s7 frontier item proposing a ruling-request on the tied "=r"/"1" island operand is dropped — it changes the copy's position, not its seat, and sotn-construct-index.md has no inline-asm-operand class.

- [s8] UNSTAGED family, 344 of 3,440 spellings swept: 13x141, 15x105, 18x98; best 13, zero at or below the floor.

- [s8] PRE-BRANCH interleaved naming space, 96 spellings over 5 axes: best 7 with 12 ties, zero improvements.

- [s8] scale/log2_val spelled as ONE variable: 17 on the floor-7 body and 17 on the s5 w6 tied body, build_insns 153 on both — 10 insns of pure seat churn.

- [s8] Tooling: sweep throughput this session was ~2.0 s/variant, not s7's 1.05 s. A 3,440-variant sweep is a ~2-hour job and does not fit one session; tmp/grind/func_80018094/s8/sweep_enum.sh chunks it so a kill preserves the partial histogram.

- [s8] src/code6cac.c was restored with `git checkout --` after the killed sweep and re-verified at 7 with the updated candidate.c; git status shows only metrics/events.jsonl plus memory/grind ledger changes.

## s9 (solver, 2026-09-09) — FLOOR 7 -> 0. The residual is fully decomposed; one construct is unclassified

Chassis re-measured at dispatch (`tools/sweep_variants.py`): candidate.c (the s4 body) == **7**,
rejected/tied-copy-dowhile-wrap-copy-seats-v1-not-a0-equal-floor-7.c (the s5 w6 tied body) == **7**,
both at build_insns 153. The brief's "measurement unavailable" resolves to 7 again.

### 1. Triage (solver operational rule 1)

`inverse_compose.py classify code6cac func_80018094 --target-object build/src/code6cac.o
--ours-object tmp/sandbox/func_80018094/code6cac.o` on the w6 chassis returns **FIRST
DIVERGENCE: RA** — 153 vs 153 insns, identical multiset, seven register substitutions. This is
the first time the residual has been typed by the classifier rather than inferred; it licenses
the whole session as an RA-model search and rules the pre-RA and scheduler models out.

The seven, read off the aligned objdump (tmp/grind/func_80018094/s9/ours.txt vs tgt.txt):

| i | ours | target | pseudo |
|---|---|---|---|
| 69 | `move v1,a1` | `move a0,a1` | island-input copy |
| 74 | `srl a0,v0,0x3` | `srl a1,v0,0x3` | log2_val (small arm) |
| 75 | `move t4,v1` | `move t4,a0` | island reads the copy |
| 82 | `and a0,v1,v0` | `and v0,v1,v0` | the staged LZCR value |
| 84 | `subu v1,v1,a0` | `subu v1,v1,v0` | consumer of the staged value |
| 93 | `srav a0,a0,v0` | `srav a1,a0,v0` | log2_val (big arm) |
| 96 | `sll v1,a0,0x6` | `sll v1,a1,0x6` | the scale tail reads log2_val |

Reading the lreg/greg dumps (tmp/grind/func_80018094/s9/w6.lreg.txt, w6.greg.txt) named the
pseudos exactly: the tied asm output is `reg/v:SI 107`, carrying `REG_UNUSED`, allocated by
**local-alloc as blk 6 qty 0** (birth 4, death 5, refs 2) and given $v1; `log2_val` is global
allocno **98** with `preferences: 4`; `sum_sq` is **77**. The s4 staging is what merges the
staged LZCR value into 98, which is why 98 conflicts with 77 and cannot share $a1 with it —
the target keeps them as two pseudos in two registers ($v0 and $a1).

`inverse.py local ... --block 6 --goal {"0": 4} --depth 3` returns the same single minimal
vector s5 saw: **[live_extend] qty 0 dies later (5 -> 9)**, i.e. the copy must still be live at
the `li -2` / `and` insns.

### 2. The four steps from 7 to 0

Every step was measured with `tools/sweep_variants.py` (each score is `sandbox --disable all`
with the body spliced into src/code6cac.c; build_insns 153 throughout).

**b4 (7 -> 5) — the live_extend vector, spelled by giving the tied output to an EXISTING
allocno.** `"=r"(log2_val)` instead of `"=r"(lz_in)`. The copy stops being a block-local qty
(local-alloc, ascending scan, $2 clobbered -> $3) and becomes global allocno 98, which carries
`preferences: 4`, has nrefs 12 / pri 40000 and is allocated FIRST — so the copy lands on the
target's $a0, and 77 (which conflicts with 98) keeps $a1. Diffs i69 and i75 close.

**d1 (5 -> 5, structurally decisive) — move the tied output off log2_val onto a real new local.**
`s32 lut;` is named for the LUT byte the arm already computes
(`lut = (u8)(*(&D_8008D118 + (sum_sq >> shift_a))); sum_sq = (lut << 16) >> ...`) and takes the
tied output. Naming `lut` alone is byte-neutral (d5 == 5). `lut` is now global allocno 107 with
`preferences: 4` at $a0; the copy is no longer fused with log2_val.

**e1 (5 -> 3) — DROP the s4 log2_val staging.** With the copy seated, `log2_val = li_v0;
shift_a = 0x16 - log2_val;` becomes `shift_a = 0x16 - li_v0;`. This RETIRES one of the body's
three FAKE constructs (the staged-value-reused-variable) and closes diffs i82 and i84: the
staged value becomes its own short local qty at $v0, exactly as the target has it. The s6/s7/s8
finding that the staging is worth 19 insns was chassis-relative and is void here.

**f1 (3 -> 0) — spell `log2_val` and `sum_sq` as ONE variable.** The remaining three diffs were
all "our log2_val is $v1, the target's is $a1", and $a1 is the register `sum_sq` occupies and
vacates one insn earlier in BOTH arms. Reusing the variable is what the target's own registers
say. `inverse.py global --goal {"98": 5}` had returned exactly one minimal vector,
`[pref_reroute] pseudo 98: preference ['$v1','$a0'] -> [$a1] (REPLACE the copy relationship)`;
the variable merge is that reroute, because the two values become one allocno.

**sandbox --disable all == 0, target_insns 153, build_insns 153, rules_dropped 0,
cheat_asm_stripped 20** — measured with f1 spliced into src/code6cac.c this session.

### 3. Ablations at the zero chassis (all three remaining devices are load-bearing)

| ablation | score | file |
|---|---|---|
| drop the tied `"=r"(lut)` / `"1"(sum_sq)` operand pair | 10 | s9/g1.c |
| `s32 sp_tmp[4]` -> `s32 sp_tmp` | 8 | s9/g3.c |
| drop the `do{...}while(0)` wrap | 13 | s9/g4.c |
| both of the last two | 21 | s9/g5.c |
| honest `lz_in = sum_sq;` copy instead of the tied operand (3 declaration scopes) | 10, 10, 10 | s9/h1.c, h2.c, h3.c |

### 4. Why this is a ruling-request and not a submission

The tied operand pair declares an asm OUTPUT (`lut`) that the island template never writes; its
only effect is to make reload emit the target's `move $a0,$a1`. No frozen SOTN family covers an
inline-asm operand device, and `docs/reference/sotn-construct-index.md` carries no
inline-asm-operand class at all (s8 recorded the same negative). Per the first-reach rule the
correct move is a ruling-request, not a candidate-ready.

The honest alternative is measured dead on this chassis too: a plain `lz_in = sum_sq;` copy is
deleted by cse (`tools/gcc-2.7.2/cse.c:8102`, the s6 class kill) at all three declaration
scopes, costing 10. And the copy cannot be part of the authorized island: reorg parks it in the
`beqz` delay slot, which it could not do to an insn inside a volatile asm block, so it is a
compiler-emitted schedulable insn, not island scaffolding.

- [s9] classify (object path) types the whole residual as **RA**: 153/153 insns, same multiset, 7 register substitutions. Pre-RA and scheduler models are out.
- [s9] The tied island output is `reg 107`, `REG_UNUSED`, blk 6 qty 0 in local-alloc (birth 4, death 5, refs 2) -> $v1. `inverse.py local --block 6` with the goal qty0 -> $a0 gives one minimal vector: extend its death 5 -> 9.
- [s9] CONFIRMED by measurement: handing the tied output to an existing GLOBAL allocno (b4) is that live-extension and seats the copy at $a0 — floor 7 -> 5.
- [s9] Naming the LUT byte `lut` is byte-neutral (d5 == 5); `lut` then carries the tied output without fusing it into log2_val (d1 == 5).
- [s9] KILLED (instance): the s4 `log2_val` staging is NOT load-bearing once the copy is seated — dropping it takes 5 -> 3 (e1). The s6/s7/s8 "the staging is worth 19 insns" measurement was chassis-relative and is void on the b4/d1 chassis.
- [s9] CONFIRMED: `log2_val` and `sum_sq` spelled as ONE variable closes the last three insns — sandbox 0, 153/153. (s8's `scale`/`log2_val` merge at +10 was the wrong pair.)
- [s9] Ablations at 0: tied operand -> 10, scalar sp_tmp -> 8, no do-while(0) -> 13, both -> 21.
- [s9] The s6 cse class kill (cse.c:8102) re-confirmed on the NEW chassis: an honest `lz_in = sum_sq;` copy scores 10 at all three declaration scopes (big-arm block, else-arm head, function top).
- [s9] The target's `move $a0,$a1` is NOT island scaffolding: reorg parks it in the `beqz` delay slot, which reorg cannot do to an insn inside a volatile `__asm__` block. It is a compiler-emitted, schedulable insn, so the cop2-addressing-preamble-cluster island grant does not cover it.
- [s9] Also measured and rejected: tied output on a fresh local reused as the `-2` carrier (9), as `shift_a` (11), on the dead pre-branch local `dz` (24); the sum_sq merge with the staging retained (12); a separate `res` variable for the arm result (11); dropping the staging on b4 without the `lut` split (11).
- [s9] src/code6cac.c restored with `git checkout --` at session end; `git status --porcelain` shows only metrics/events.jsonl plus the memory/grind ledger changes.

## s9b (solver, 2026-09-09) — honest floor 7 -> 2

**Chassis at dispatch.** HEAD is `INCLUDE_ASM("asm/funcs", func_80018094);` (src/code6cac.c:309).
The s9a body banked at candidate.c measured `sandbox --disable all == 0` but the Judge FAILed its
tied asm operand (docs/grind/decisions.md, 2026-09-09 23:11); dropping the operand from that body
scores 10, and the honest floor carried in the ledger was 7 (an older, unmerged chassis).

**The residual, stated exactly.** With the tied operand removed the objdump diff against
asm/funcs/func_80018094.s is ONE structural difference plus its register fallout: the target has
`addu $a0,$a1,$zero` in the `beqz` delay slot at .L800181A4 and carries the sum in $a1; we emit a
`nop` there and carry the sum in $a0. Instruction counts are equal (153/153) either way — the
delay slot exists in both, so the copy is FREE.

**Pass attribution (dumps, not hypothesis).** `pwsh tools/grinder/dump.ps1 func_80018094` with the
honest copy spliced in; the copy's pseudo counted per pass with tmp/grind/func_80018094/s9/ext.py:
- plain copy (s9/k0.c): pseudo 107 present in .rtl, absent by .cse -> **cse1 deletes it**.
- copy wrapped in `do { } while (0)` (s9/k1.c): present in .rtl/.jump/.cse/.loop, one occurrence
  left in .cse2, gone in .flow -> **cse1 is blocked, cse2 deletes it**.
The asymmetry is `cse_end_of_basic_block`'s `if (! after_loop && ... NOTE_INSN_LOOP_END) break;`
(tools/gcc-2.7.2/cse.c:8053-8055): cse1 runs after_loop = 0, cse2 after_loop = 1.

**The gate that closes both.** Same function, the follow-jumps test at cse.c:8100-8125: cse extends
the basic block across a conditional jump only when `LABEL_NUSES (JUMP_LABEL (p)) == 1` and the
backward walk from the target label lands on a BARRIER. The walk skips NOTEs *except*
NOTE_INSN_LOOP_END / NOTE_INSN_SETJMP, and has no `after_loop` guard. Spelling the small arm as an
`if` with NO else whose body is `do { ...; goto lzc_done; } while (0);` puts the loop-end note
between the small arm's BARRIER and the LZC arm's label, so neither cse run extends into the LZC
block, the asm operand keeps its own pseudo, and the copy survives to RA. An `if/else` can never
produce this layout — `expand_start_else` emits its jump+BARRIER immediately before the else label.

**Measured ladder this session** (all `sandbox --disable all`, build_insns 153 unless noted):
| body | score | what it shows |
|---|---|---|
| s9/k0-k3 (honest copy, four scopes/wraps) | 10 | cse kills the copy at every scope |
| s9/m3 (goto, no do-while(0)) | 10 | the `goto` alone is not the lever |
| s9/m1, m2 (goto + do-while(0)) | 13 | **the copy exists**, registers swapped |
| s9/m4, m5 (separate short-lived copy var) | 11 | copy becomes block-local -> $v1 |
| s9/n5 (`lut` carries the -2 mask) | 5 | seats correct, -2 block moves |
| s9/n8, n9 (`lut` carries the LUT index) | 4 | seats correct, index block moves |
| s9/n1 (`lut` holds the small arm's byte) | **2** | all seats correct |
| s9/n13 (n1 + `u32 lut` + `(s32)` cast) | **2** | also fixes `sra` -> `srl`; **banked** |
| s9/n4 (copy above the `sum_sq < 0` test) | 14 (154) | adds an instruction |
| s9/n10, q8 (separate result variable) | 8 (154) | adds an instruction |
| s9/p1, n7 (sll written back into `lut`) | 13 | 5 refs is not enough to flip the sort |
| s9/p5 (small arm indexes through `lut`) | 13 | sort flips back |

**Allocation facts (m1's .greg, tmp/grind/func_80018094/s9/m1.greg.txt).** `sum_sq` (77) and the
copy (80) have IDENTICAL conflict sets `{72 73 77 80 2 3 12 29}` and BOTH carry `preferences: 4`.
$2 is closed by the island's clobber list and $3 by the conflict set, so the first one allocated
takes $4 and the second $5. The priority order printed by global.c is `118 77 80 78 ...`. The
entire remaining residual is therefore one comparison in `allocno_compare` — a references/live-
length fact, not a conflict fact and not a preference fact. This supersedes the s6/s8/s9a framing
("allocno 99 must conflict with allocno 78"), which was a property of the tied-operand chassis.

**Remaining 2 instructions.** Target `lbu $v0,0($at)` / `srl $a1,$v0,3` in the small arm; ours
`lbu $a0,0($at)` / `srl $a1,$a0,3`, because the byte is parked in `lut` purely to win the sort.

- [s9] The whole residual after the Judge's FAIL of the tied operand was ONE structural insn: the target's `addu $a0,$a1,$zero` in the `beqz` delay slot at .L800181A4, plus nine register-name diffs that follow from sum_sq sitting in $a1 rather than $a0. Instruction counts are equal (153/153) with or without it, because the delay slot exists either way - the copy is free.

- [s9] cse deletes an honest reg-to-reg copy of a live value at EVERY declaration scope because cse_end_of_basic_block extends the basic block across the conditional jump into the LZC arm; the copy and the asm then sit in one extended block and canon_reg rewrites the asm operand back to sum_sq.

- [s9] That extension is gated at tools/gcc-2.7.2/cse.c:8100-8125 on LABEL_NUSES(JUMP_LABEL) == 1 AND the insn preceding the target label being a BARRIER; the backward walk that looks for the BARRIER stops early on NOTE_INSN_LOOP_END and NOTE_INSN_SETJMP and has no after_loop guard, so a loop-end note placed there blocks cse1 and cse2 alike.

- [s9] `if (cond) { do { ...; goto label; } while (0); }` with NO else is the only C shape that puts a NOTE_INSN_LOOP_END between the then-arm's BARRIER and the if's false label; `if/else` cannot, because expand_start_else emits its jump and BARRIER immediately before the else label.

- [s9] With the gate closed the copy reaches RA as a global allocno and reorg parks it in the beqz delay slot exactly as the target does (measured, tmp/grind/func_80018094/s9/m1.c).

- [s9] m1's .greg shows sum_sq (77) and the copy (80) with identical conflict sets {72 73 77 80 2 3 12 29} and both carrying `preferences: 4`; $2 is closed by the island clobber list and $3 by the conflict set, so allocation ORDER alone decides which takes $a0.

- [s9] The honest floor is now 2 and the two remaining instructions are the small arm's LUT byte temp (target $v0, ours $a0), which is parked in the copy's variable only to win global.c's priority sort.

- [s9] memory/grind/func_80018094/candidate.c carries NO asm-operand device: the island operand list is exactly the granted form : "=m"(sp_tmp[0]) : "r"(lut) : "$2", "$12", so the Judge's 2026-09-09 constraint is satisfied.

## s10 (forensics, 2026-09-09) — the honest floor reaches 0

**The allocno priority arithmetic, read out of the compiler instead of inferred.** The
instrumented cc1 (`tools/gcc-2.7.2/cc1`) carries a `BB2_ALLOC_DEBUG` hook at
tools/gcc-2.7.2/global.c:605 that prints, for every allocno in priority order,
`ord / pseudo / hardreg / nrefs / livelen / pri` where
`pri = floor_log2(allocno_n_refs)*allocno_n_refs / allocno_live_length * 10000 * allocno_size`
— i.e. exactly the key `allocno_compare` (tools/gcc-2.7.2/global.c) sorts on. Captured with
tmp/grind/func_80018094/s10/allocdbg.py (splices a body into src/code6cac.c, runs the project's
own cpp | cc1 front half with BB2_ALLOC_DEBUG=1 and -da, restores src):

| body | score | pseudo 80 (`lut`, the island-input copy) | pseudo 77 (`sum_sq`) | seats |
|---|---|---|---|---|
| s9/m1.c (byte in its own pseudo) | 13 | nrefs 8, len 7, **pri 34285** | nrefs 19, len 21, **pri 36190** | 77 -> $a0, 80 -> $a1 |
| s9/n13.c = s9b candidate (byte parked in `lut`) | 2 | nrefs 14, len 9, **pri 46666** | nrefs 19, len 21, pri 36190 | 80 -> $a0, 77 -> $a1 |
| s10/e1.c (byte in its own pseudo + third wrap) | **0** | nrefs 11, len 7, **pri 47142** | nrefs 21, len 21, pri 40000 | 80 -> $a0, 77 -> $a1 |

**reg_n_refs is LOOP-DEPTH WEIGHTED, and that is the lever.** flow.c computes
`basic_block_loop_depth` by counting NOTE_INSN_LOOP_BEG/END while it partitions the insn stream
(tools/gcc-2.7.2/flow.c:440-471), and every reference is then accumulated as
`reg_n_refs[regno] += loop_depth` (flow.c:2081, and again at 2329 / 2515 / 2725). A
`do { ... } while (0);` wrap emits exactly one LOOP_BEG/LOOP_END pair, so it multiplies the
weight of every reference inside it. On the m1 chassis the accounting checks out exactly:
`lut` has 4 real references (the copy at insn 106, the island operand at 144, the LUT-byte set at
169, the `<<16` use at 171), all at depth 2 -> nrefs 8; `sum_sq` has 3 references at depth 1
(before the outer wrap), 5 at depth 2 and 2 at depth 3 (inside the small arm's wrap) -> 19.

**The closing form.** Wrapping the LZC ARM's body in a third `do { ... } while (0);` lifts
`lut`'s three in-arm references from weight 2 to weight 3 (nrefs 8 -> 11) at UNCHANGED
live_length 7, while `sum_sq` only gains its two in-arm references (19 -> 21) against an unchanged
live_length 21. `floor_log2(11)*11/7 = 47142 > floor_log2(21)*21/21 = 40000`, so `lut` is
allocated FIRST, takes $a0, `sum_sq` takes $a1, and the small arm's LUT byte is free to stay in
its own short-lived pseudo, which lands on $v0 exactly as the target has it. The priorities were
PREDICTED from the m1/candidate arrays before the form was written and came out exact.

**Every construct is individually load-bearing (ablations, all at build_insns 153).** Drop the
outer do-while(0) -> 13 (s10/f1.c). Drop the small-arm do-while(0) -> 10 (s10/f3.c). Drop the
LZC-arm do-while(0) -> 13 (s10/a0.c). `s32 sp_tmp` scalar instead of `s32 sp_tmp[4]` -> 8
(s10/f2.c). This discharges the do-while-zero-exception's nested-wrap prerequisite by measurement.

- [s10] The instrumented cc1's BB2_ALLOC_DEBUG hook (global.c:605) prints allocno_n_refs, allocno_live_length and the allocno_compare priority for every allocno in sort order; that is the direct read of the sort that nine sessions inferred from `;; N regs to allocate:` ordering alone.
- [s10] reg_n_refs is accumulated as `reg_n_refs[regno] += loop_depth` (tools/gcc-2.7.2/flow.c:2081) with loop_depth taken from basic_block_loop_depth, which counts NOTE_INSN_LOOP_BEG/END (flow.c:440-471) — so a do-while(0) wrap is a REFERENCE-WEIGHT multiplier for everything inside it, not only a note-placement device. This is a general lever for any residual that reduces to global.c's priority sort.
- [s10] cse folds a branch test written on the copy (`if ((lut = sum_sq) < 0x400)` or `lut = sum_sq; if (lut < 0x400)`) back to `sum_sq` in the same block, so neither spelling moves a single reference between the two allocnos (both measured 13, identical ALLOCDBG arrays to m1).
- [s10] Folding the LZC tail's constant subtraction into shift_b (`shift_b = 0x13 - (shift_a >> 1)`) LENGTHENS `lut`'s live range from 7 to 9 instead of shortening it, dropping pri to 26666 (s10/a1.c, score 13; the same fold costs the winning s9b body 2 insns, s10/a2.c score 4).
- [s10] The honest floor is 0: `sandbox func_80018094 --disable all` == 0, target_insns 153, build_insns 153, rules_dropped 0, cheat_asm_stripped 20, with memory/grind/func_80018094/candidate.c spliced into src/code6cac.c, and with the LZC island's operand list at exactly the granted honest form.

- [s10-refile] The s10 result was RE-VALIDATED from scratch on 2026-09-09 after the driver discarded the first s10 write-up on a self-vet CITATION-FORMAT defect only (five `FAMILY:` blocks but only four verbatim SCOPE quotes; the fifth block was the ordinary-C `*(&D_8008D118 + i)` byte-LUT read, which claims no exception at all). No C changed: memory/grind/func_80018094/candidate.c was re-spliced into src/code6cac.c with tmp/grind/func_80018094/s9/splice.py and `sandbox func_80018094 --disable all` printed score 0 / target_insns 153 / build_insns 153 / rules_dropped 0 / cheat_asm_stripped 20 again. The self-vet's fifth block is now an ORDINARY-C NOTE (not a family claim), and three stale precedent line numbers were corrected against the post-splice tree: the sibling func_8001A538's LUT reads are src/code6cac.c:953 and :980 (not :879/:906) and its `lw_v1`/`li_v0` statements are src/code6cac.c:974-977 (not :911-914). Lesson for any later session on any function: line-number precedents in a self-vet must be re-derived AFTER the candidate is spliced in, because splicing a ~120-line body into the TU shifts every later line in the same file.
