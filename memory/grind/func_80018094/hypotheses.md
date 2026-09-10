# Hypothesis ledger — func_80018094

## s1 (recon, 2026-09-09) — chassis -mel -msoft-float; islands = SDK-macro spellings + func_8001A67C LZC template; NO FAKE constructs

| # | hypothesis | probe | result | verdict |
|---|---|---|---|---|
| H1 | D_8008D118 declaration (DATA MODEL signal: no header decl) needs fixing | pairdiff of the LUT reads | byte-identical with the TU-local `extern u8` (code6cac.c:19) | MATCHES (no fix needed) |
| H2 | plain `*(s32 *)0x1F8000xx` scratchpad access | sandbox v1 | 81; loads hoist above the stores, li-shared address in block 2 | KILLED (instance) |
| H3 | `s32 *scr = (s32 *)0x1F800000` pointer variable | sandbox v2 | 62; lui hoisted into s2 across the call, block 2 through s2 | KILLED (instance) |
| H4 | struct member at constant base `((ScrV *)0x1F800000)->x/y/z` (in-struct MEM) | cc1 v3d + sandbox v6a | block 1 stores in place, block 2 constant addresses, byte-identical | CONFIRMED |
| H5 | `scale = 0x100` before the if, `if (sum <= 250000)` nest | cc1 v3d + local_extract | li 256 between mult dz / sw dz -> dz life 14 -> seat cascade | KILLED (instance) |
| H6 | `if (sum > 250000) scale = 0x100; else if (sum < 0) scale = 0; else {...}` | sandbox v6a | 21; block 1 + branch skeleton byte-identical | CONFIRMED |
| H7 | island-input copy via named local `lz_in = sum_sq` | cc1 v7a | copy eliminated pre-RA, no move emitted | KILLED (instance) |
| H8 | island-input copy via `static inline` helper parameter | cc1 v7b | copy eliminated, frame unchanged 8 | KILLED (instance) |
| H9 | frame vars: single `s32 sp_tmp` ("=m") reaches the target's 16 | BB2_FRAME_DEBUG v3d | 4 bytes -> 8; target 16 | KILLED (instance) |
| H10 | 32-byte MATRIX copy through `s32 *dst = arg1` shared with the call | sandbox v1 | matched first try | CONFIRMED |

Not measured with the sandbox (cc1-only, s1/*.s): v3b (locals-first block 2, same as v3d), v3c
(const-base array indexing, li form), v3f (struct block 1 + plain block 2, li form), v6b (nested `<=`
with else, same block 1 as v6a, downstream untested), x5 (matching-constraint diagnostic: no SDK basis,
dropped).

## Frontier (next session)
1. Island-input copy `move a0,a1` + sum_sq seat a1 (9 residual insns). Regenerate v7a dumps
   (`bash tmp/grind/func_80018094/s1/cc_dbg.sh v7a`) and read v7a.cse vs v7a.combine to name the pass
   that deletes the copy; then a form whose second pseudo is not cse-equivalent. Also adopt the COMPLETED
   sibling's committed LZCR-read statement shape (func_8001A67C, src/code6cac.c:
   `lw_v1 = sp_tmp; li_v0 = -2; li_v0 = lw_v1 & li_v0;`) for the `lw v1,16(sp); li v0,-2; and v0,v1,v0`
   seats (3 insns) — ordinary C, same-file precedent.
2. Frame 16: BB2_FRAME_DEBUG is the gradient (`cc_dbg.sh <v> BB2_FRAME_DEBUG=1`, grep
   `FRAMEDBG func=func_80018094`). Candidates: a 12-byte local for the LZCR store, a DImode temp; check
   psyz / SOTN usage of gte_stlzc for the local's natural type before spelling anything.
3. After (1) exists as a spelling, `tools/ra_solver/extract.py func_80018094 code6cac` +
   `inverse_compose.py classify` for sum -> $5 / log2_val -> $5.

## [s1] OBJECT MODEL: D_8008D118 (g_isqrt_lut, no header decl) needs a declaration fix to match the two LUT reads
- mechanism: DATA MODEL signal: decl NONE in include/*.h; TU-local `extern u8 D_8008D118;` at src/code6cac.c:19 is the same declaration the COMPLETED sibling func_8001A67C compiles against
- probe: pairdiff of the sandbox object vs the reference (tmp/grind/func_80018094/s1/v6a_pairdiff.txt)
- result: MATCHES: both `lui at,%hi; addu at,at,idx; lbu v0,%lo(D_8008D118)(at)` sites are byte-identical (pairdiff lines 70-72, 87-89 equal); no MISMATCH signal, nothing to fix
- verdict: CONFIRMED

## [s1] plain `*(s32 *)0x1F800024/28/2C` casts for the three scratchpad stores and the three read-modify-writes, measured on this candidate body
- mechanism: sched.c true_dependence: a constant-address store that is not MEM_IN_STRUCT_P has no dependence against an in-struct varying load, so the next statement's loads hoist above the store; block-2 addresses are force_reg'd (explow.c memory_address), shared by cse canon_reg and unfoldable by combine (2 uses)
- probe: sandbox v1 (tmp/grind/func_80018094/s1/v1.c, v1_pairdiff.txt)
- result: 81/153; block-1 stores sink below all three subtractions, block 2 emits `lui/ori addr; lw 0(addr) ... sw 0(addr)`
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v1 body: SDK-macro SetRot/SetTrans islands + func_8001A67C LZC template, scale=0x100 before the if, no FAKE constructs

## [s1] a `s32 *scr = (s32 *)0x1F800000;` pointer variable indexed scr[9..11] for all six scratchpad accesses, measured on this candidate body
- mechanism: the pointer pseudo is live across func_80017FA0 and the join label: its `lui` is hoisted into a callee-saved s2 (extra save/restore) and the block-2 accesses (after the join, cse table flushed) address through s2 instead of constant addresses
- probe: sandbox v2 (s1/v2.c, v2_pairdiff.txt)
- result: 62/153; block-1 stores now in place, but `sw s2,32(sp); lui s2,0x1f80` in the prologue and `lw v0,36(s2)` forms in block 2
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v2 body: islands as v1, scale=0x100 before the if, no FAKE constructs

## [s1] scratchpad accesses spelled as members of a struct at the constant base (`((ScrV *)0x1F800000)->x/y/z`, the src/code6cac_b.c:919 SCR-macro shape) keep the stores in place and give each block-2 read/write its own constant address
- mechanism: MEM_IN_STRUCT_P set on both sides -> sched.c true_dependence conflict keeps the arg0[1]/arg1[k] loads below the previous store; cse folds `(plus reg_equiv_const 36)` to a CONST_INT address per access, so nothing is shared and combine has nothing to fold
- probe: cc1 v3d (s1/v3d.s) then sandbox v6a
- result: block 1 store order and block 2 (`lui v0; lw v0,36(v0)` reads, `lui at; sw` writes, target insn order) byte-identical in v6a
- verdict: CONFIRMED

## [s1] `scale = 0x100;` assigned before `if (sum_sq <= 250000) { if (sum_sq < 0) scale = 0; else {...} }`, measured on this candidate body
- mechanism: sched1 adjust_priority (tools/gcc-2.7.2/sched.c:2562-2600) raises the birthing `li 256` to max_priority so it is scheduled between `mult dz` and `sw dz`; the dz local-alloc qty then spans 14 suids (pri floor_log2(6)*6*4/14*10000 = 34285) and is allocated after the 2-ref loads (40000), seating dz in $3; cascade: 250000 constant -> $4, sum -> $4, sched2 anti-dependence on $2 blocks the arg1[12]/mat[7] load swap
- probe: cc1 v3d + tools/ra_solver/local_extract.py (tmp/ra_solver_work/code6cac.local.json: blk 0 qty11 first_reg 77 birth 40 death 54 refs 6 got 3) + sched dump s1/v3d.sched (insn 75 li-256 scheduled before insn 61 sw-dz)
- result: dz triple `lw v1,28(v0); lw v0,48(s1)` (target `lw v1,48(s1); lw v0,28(v0)`), `subu $3`, `li $4,0x30000`, sum in $4; the target's `li v1,256` in the bnez delay slot writes the register the branch reads, which reorg only does for an insn stolen from the branch-target thread (an else block)
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v3d body: struct-macro scratchpad, islands as v1, no FAKE constructs

## [s1] `if (sum_sq > 250000) scale = 0x100; else if (sum_sq < 0) scale = 0; else { LUT / LZC path }` reproduces the target's block 1, branch skeleton and delay-slot fills
- mechanism: scale=0x100 is its own else block -> not in block 1's sched1 ready list -> dz life 12 -> dz $2, arg1[12] $3, constant $3; reorg steals the block's single `li v1,256` into the bnez delay slot and redirects the branch to the join
- probe: sandbox v6a (s1/v6a.c, v6a_pairdiff.txt)
- result: 21/153; everything from the prologue body through `bgez a1` / `j; move v1,zero` / `beqz v0` and the whole block 2 + MATRIX copy + tail call byte-identical
- verdict: CONFIRMED

## [s1] a named local copy `lz_in = sum_sq;` as the LZCS island input, measured on this candidate body, produces the target's `move a0,a1` before `addu $t4,$a0,$zero`
- mechanism: the copy would need to survive cse canon_reg (which rewrites the asm input to the equivalent older pseudo) and combine (which folds a single-use copy into the asm)
- probe: cc1 v7a (s1/v7a.s)
- result: no move emitted; island reads the sum register directly (`addu $t4,$4,$zero`), output identical to v6a
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v7a body = v6a + `s32 lz_in` local, no FAKE constructs

## [s1] a `static inline s32 lzc_of(s32 v)` helper wrapping the LZC island (parameter copy + its own `s32 r` local), measured on this candidate body, produces the input copy and/or the target's 16-byte vars area
- mechanism: integrate.c parameter copies and callee frame embedding
- probe: cc1 v7b (s1/v7b.s)
- result: no move emitted (copy eliminated), `.frame` still vars= 8
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v7b body = v6a with the island moved into a static inline helper, no FAKE constructs

## [s1] a single `s32 sp_tmp;` "=m" operand for the LZCR store reaches the target's vars=16 frame, measured on this candidate body
- mechanism: function.c put_reg_into_stack allocates 4 bytes; reload1.c round_frame rounds to 8
- probe: instrumented cc1 BB2_FRAME_DEBUG=1 on v3d (s1/v3d.dbg: `FRAMEDBG func=func_80018094 ctx=put_reg_into_stack size=4 frame_offset=4` then `round_frame frame_offset=8`)
- result: vars= 8 (frame 40) vs target 48 (vars 16, sp_tmp at 0x10, 0x14..0x1F untouched); the COMPLETED sibling func_8001A67C shows the identical 4->8 profile, so the extra 8 bytes come from something the original had and this body lacks
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v3d/v6a bodies, no FAKE constructs

## s2 (structural, 2026-09-09) — chassis -mel -msoft-float; body = s1 v6a + sibling LZCR-read block

| # | hypothesis | probe | result | verdict |
|---|---|---|---|---|
| H11 | sibling func_8001A67C's `lw_v1/li_v0` LZCR-read block fixes the 3 read seats | sandbox v8a | 21 -> 18; `lw $3,16($sp); li $2,-2; and $2,$3,$2` = target | CONFIRMED |
| H12 | the sibling's u32 typing of the else branch transplants too | cc1 v9a | `sra` -> `srl` at 2 sites (target has sra) | KILLED (instance) |
| H13 | cse (not combine) deletes the island-input copy | dumps v8b.rtl / .jump / .cse | insn 124 present in .rtl and .jump, gone in .cse, asm operand rewritten to reg78 | CONFIRMED |
| H14 | the copy survives if placed in the pre-branch block (global allocno) | cc1 v11b | asm byte-identical to v8a; cse deletes it there too | KILLED (instance) |
| H15 | frame vars 16 comes from a declaration-scope / order / hoisting structural lever | cc1 v9b / v9c / v9e / v11b `.frame` | vars= 8 in every variant | KILLED (instance) |
| H16 | split accumulation `sum=dx*dx; sum+=dy*dy; sum+=dz*dz;` moves the sum seat | cc1 v9d | mflo pair swaps, sum still $4, block-1 add order breaks | KILLED (instance) |
| H17 | a $4-pinned register variable is a usable lever for the seat cluster | cc1 v11a + sandbox | asm reproduces the target's 10 seat insns EXACTLY, but the sandbox strips it (22 vs 20 stripped) and the honest score rises to 24 | KILLED (instance) as a lever; the asm output stands as mechanism proof |
| H18 | a tied output/input pair on the island produces a surviving copy seated in $4 | cc1 v10b | copy survives reload (`move $3,$4`) but seats $3 with sum_sq still $4; also a dead asm output | KILLED (instance) |

## Frontier (next session)
1. The 10-insn seat cluster reduces to ONE question: how the original fixed the LZC island's input to
   $a0 (binary-wide: all 21 inlined sites read $a0). Ordinary C copies cannot survive cse.c canon_reg
   (H13/H14 here, s1 H7/H8 before). The register-pin spelling reproduces the bytes but is a banned
   family and is stripped by the sandbox (H17). The open RULING question is whether the input-register
   fixing belongs to the AUTHORIZED canonical island for this cop2-cluster member
   (.claude/rules/cop2-addressing-preamble-cluster.md:60, inline_asm_canonical.txt:266) rather than to
   the C — i.e. whether the island template for func_80018094 may be spelled with $a0 in its own text
   plus an operand form that makes the compiler materialise the value there.
2. Frame vars 8 -> 16 (8 insns) is INDEPENDENT of the seat cluster and is untouched by declaration
   scope, declaration order and function-scope hoisting (H15). Next: BB2_FRAME_DEBUG on bodies that
   introduce a stack TEMP rather than a stack VARIABLE (memory phantom-frame-slots-gcc272 records that
   an HImode bitwise expression over two HImode locals is the known minimal trigger for exactly 8
   phantom bytes) — e.g. narrowing the `sp_tmp & -2` block or the `0x16 - x` / `0x13 - y` shift
   arithmetic to s16 intermediates, then reading `vars=` from the `.frame` comment.
3. If (1) is ruled C-side, the remaining pin-free mechanism to test is a copy that reload MUST
   materialise: the tied-operand form of H18 with $3 made busy across the copy's block-local range so
   local-alloc's ascending find_free_reg lands on $4 and pushes sum_sq to $5.

## [s2] the COMPLETED-C sibling func_8001A67C's LZCR-read statement block (`s32 lw_v1 = sp_tmp; s32 li_v0 = -2; li_v0 = lw_v1 & li_v0; shift_a = 0x16 - li_v0;`, src/code6cac.c:869-873) replaces `shift_a = 0x16 - (sp_tmp & -2);` and emits the target's three read seats exactly
- mechanism: the split named intermediates give local-alloc a 2-address `and` whose destination qty is the li-constant qty, so the load lands in $3 and the constant in $2 (target `lw v1,16(sp); li v0,-2; and v0,v1,v0`)
- probe: sandbox func_80018094 --disable all on tmp/grind/func_80018094/s2/v8a.c (= s1 v6a + the sibling block)
- result: 21 -> 18, 153/153 insns, rules_dropped 0, cheat_asm_stripped 20; the three insns are byte-identical; this is the s1 frontier item 1 sibling transplant, banked as the new candidate.c
- verdict: CONFIRMED

## [s2] cse.c, not combine.c, is the pass that deletes a C-level island-input copy `lz_in = sum_sq;` on this body
- mechanism: cse.c canon_reg rewrites the asm_operands input to qty_first_reg (the older pseudo reg78) even when the copy opens a fresh cse path, after which the copy insn is dead and is deleted
- probe: tmp/grind/func_80018094/s2/cc_dbg.sh v8b, then grep `reg/v:SI 108` across v8b.rtl / v8b.jump / v8b.cse
- result: (insn 124 (set (reg/v:SI 108) (reg/v:SI 78))) is present in .rtl and .jump and absent in .cse, where the asm input reads reg78; this closes the s1 open question 'cse vs combine'
- verdict: CONFIRMED

## [s2] placing the island-input copy `lz_in = sum_sq;` in the pre-branch block (before `if (sum_sq < 0x400)`) so that it becomes a cross-block allocno, measured on this candidate body, makes the copy survive to the assembly
- mechanism: a copy live across a branch would be a global.c allocno conflicting with sum_sq and would take $4 before it, pushing sum_sq to $5
- probe: cc1 v11b (tmp/grind/func_80018094/s2/v11b.c, v11b.s) diffed against v8a.s
- result: v11b.s is byte-identical to v8a.s: cse deletes the copy in that position as well, so no copy reaches RA and sum_sq keeps $4
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v8a body (s1 v6a + sibling LZCR-read block), no FAKE constructs

## [s2] declaration-scope / declaration-order / hoisting structural levers (sp_tmp declared in the innermost block, dst-first + sp_tmp-last permutation, log2_val hoisted to function scope) move the frame from vars=8 toward the target's vars=16, measured on this candidate body
- mechanism: function.c assign_stack_local / put_reg_into_stack ordering plus reload1.c round_frame
- probe: cc1 v9b, v9c, v9e and v11b; read the `.frame` comment (vars= IS get_frame_size) and diff each .s against v8a.s
- result: all four print `vars= 8` and all four assemble byte-identically to v8a.s — these levers are frame-neutral and codegen-neutral here
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v8a body, no FAKE constructs

## [s2] transplanting the sibling func_8001A67C's u32 typing of the LZC else branch (log2_val / shift_a / shift_b unsigned) onto this body keeps the target's shift forms
- mechanism: unsigned shift operands select lshiftrt (srl) instead of ashiftrt (sra) in the RTL expander
- probe: cc1 v9a diffed against v8a.s
- result: `sra $3,$3,1` -> `srl $3,$3,1` and `sra $3,$4,$2` -> `srl $3,$4,$2`; the target has sra at both sites, so func_80018094 needs the signed spelling (sum_sq also carries the `> 250000` and `< 0` tests)
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v8a body, no FAKE constructs

## [s2] re-associating the sum as split accumulation (`sum_sq = dx*dx; sum_sq += dy*dy; sum_sq += dz*dz;`) moves the sum_sq seat off $4, measured on this candidate body
- mechanism: changing the order in which the three products are born changes which partial-product qty local-alloc reuses for the accumulator
- probe: cc1 v9d diffed against v8a.s
- result: the two mflo destinations swap ($5/$4 -> $4/$5) and the adds become `addu $4,$4,$5; addu $4,$4,$7`; sum_sq still lands in $4 and block 1 loses the target's `addu v0,a1,a0` form
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v8a body, no FAKE constructs

## [s2] a local register variable pinned to $4 carrying the island input is a usable lever for the 10-insn seat cluster on this body
- mechanism: the hard-reg range conflicts with sum_sq's allocno so global.c find_reg pushes sum_sq to $5 and reload materialises `move $4,$5`
- probe: cc1 v11a (asm compared against asm/funcs/func_80018094.s) then sandbox func_80018094 --disable all
- result: the assembly reproduces the target's whole seat cluster exactly (addu $5,$2,$7 / slt $3,$3,$5 / bgez $5 / slt $2,$5,1024 / move $4,$5 / lbu $2,D_8008D118($5) / sra $2,$5,$3), but the register pin is a banned family AND is inert: the sandbox strips it (cheat_asm_stripped 20 -> 22) and the honest score RISES to 24. Kept only as mechanism proof in rejected/lzc-input-copy-register-pin-a0-sandbox-strips-24.c; never proposed as a candidate
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v8a body plus one register-pin declaration (the banned construct under test)

## [s2] a tied output/input pair on the island (`: "=m"(sp_tmp), "=r"(lz_in) : "1"(sum_sq)`) produces a surviving input copy seated in $4 with sum_sq pushed to $5, measured on this candidate body
- mechanism: reload must satisfy the matching constraint while sum_sq is live afterwards, so it emits a real reg-reg copy that cse cannot touch
- probe: cc1 v10b diffed against v8a.s, plus tools/ra_solver/extract.py on the v10b body
- result: a copy does survive (`move $3,$4`, island reads $3) but it is a block-local qty and local-alloc's ascending find_free_reg gives it $3 ($2 is blocked by the island's own clobber), leaving sum_sq in $4; the model shows order [78, 79, ...] with 78 -> $4. It is also a dead asm output (the island never writes that register), i.e. a coercion smell rather than ordinary C
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), v8a body plus a tied asm output operand (the construct under test)

## s3 (structural, 2026-09-09) — chassis -mel -msoft-float; body = candidate.c (s2 v8a), baseline sandbox 18 re-measured this session

| # | hypothesis | probe | result | verdict |
|---|---|---|---|---|
| H19 | the 18 residual insns are 8 frame-offset insns + 10 seat insns, and the two are independent | pairdiff base_pairdiff.txt + `.frame` of s2/v11a (the register-pin body that reproduces the seat cluster) | v11a still prints vars= 8: the pin fixes the seats and leaves the frame wrong — the two residuals are decoupled | CONFIRMED |
| H20 | HImode (s16) narrowing of the sibling LZCR-read block raises vars from 8 toward 16 (memory phantom-frame-slots-gcc272 names an HImode bitwise expression as the minimal 8-byte trigger) | cc1 v12a/v12b/v12c | all three print vars= 8 and add 2 sign-extension insns (`sll $2,$2,16; sra $2,$2,16`) plus swap the lw/li seat pair | KILLED (instance) |
| H21 | extra named-intermediate locals (split products, split shift arithmetic, split scale computation) raise vars | cc1 v14a/v14b/v14c/v14d | all print vars= 8; v14b additionally forces a 4th saved register (regs= 4/0) and loses 3 insns of shape | KILLED (instance) |
| H22 | +8 bytes of frame is the right magnitude and IS reachable in this body | cc1 p2 (a live 8-byte struct before the island) | vars= 16, frame= 48 — the target's exact frame equation (16 args + 16 vars + 16 gp) | CONFIRMED |
| H23 | the extra 8 bytes must be allocated AFTER sp_tmp's slot, i.e. by a construct expanded after the LZC island statement | cc1 p2 (aggregate before the island) vs p3 (same aggregate after it) | p2 pushes sp_tmp to `24($sp)` (target reads `16($sp)`) — WRONG; p3 keeps `addiu $v0,$sp,0x10` / `lw $3,16($sp)` AND prints vars= 16 — RIGHT | CONFIRMED |
| H24 | a live 8-byte aggregate in the tail is byte-usable as the frame source | cc1 p3 | frame layout exact, but the aggregate is real memory: +3 insns (`sw $4,24($sp)` etc.), and the target has NO sp-relative access other than the 0x10 pair, so the target's 8 bytes are a PHANTOM (allocated, never accessed) | KILLED (instance) |

## Frontier (next session)
1. FRAME (8 of the 18 insns, fully independent of the seat cluster). The search is now sharply bounded
   by three measured facts: (a) the slot is 5..12 raw bytes (ALIGN8 -> 16) — an 8-byte object is the
   natural fit; (b) it is allocated AFTER the island's `put_reg_into_stack` for sp_tmp, so it cannot be
   any object declared/expanded before the island (H23); (c) it is never accessed by any emitted insn
   (H24), i.e. it is a BLKmode stack TEMP, not a stack VARIABLE. A FRAMEDBG census of the whole
   src/code6cac_c2.c TU (s3/code6cac_c2.fd, 53 slots) shows EVERY non-spill, non-round_frame slot in
   this codebase is `ctx=stack_temp mode=26` (BLKmode) — so the construct to hunt is an 8-byte
   AGGREGATE-typed temp in the post-island tail whose accesses are optimized away: candidates are
   expr.c:6105 (TARGET_EXPR, keep-level 2 so it is never reused), calls.c:1083 (a struct argument
   copied by value), expr.c:4531 / expr.c:5090 (a struct-valued rvalue with no target). PsyQ's only
   8-byte aggregate is SVECTOR (4 shorts); MATRIX is 32 and VECTOR is 16, both of which overshoot.
2. SEATS (10 insns). Unchanged and now precisely characterized by base_pairdiff.txt: the target keeps
   sum_sq in $a1 for the whole if-chain and materialises `move a0,a1` in the DELAY SLOT of `beqz v0`
   (ours emits `nop` there), so the copy is a real insn reorg stole into the slot, not an island
   artifact. A binary census of all 29 `mtc2 $t4,$30` sites shows `addu $t4, $a0, $zero` at 29/29 —
   the LZC island's asm text names $a0, which no "r"-constraint spelling can guarantee. This remains
   the standing ruling question from s2.
3. If the frame temp is found, the floor drops 18 -> 10 with no change to the seat question.

## [s3] the 18-insn residual splits into 8 frame-offset insns and 10 register-seat insns that are independent levers
- mechanism: the frame insns are the 4 prologue / 4 epilogue sp-offset-bearing insns (addiu sp / sw s0,s1,ra / lw ra,s1,s0 / addiu sp); the seat insns are every place the target names $a1 for sum_sq plus the `move a0,a1` delay-slot fill
- probe: tools/pairdiff.py code6cac func_80018094 on the spliced candidate (tmp/grind/func_80018094/s3/base_pairdiff.txt) plus `grep .frame` on s2/v11a.s (the register-pin body that reproduces the whole seat cluster)
- result: the pairdiff hunks are exactly {0,1,3,5,147,148,149,150} (frame) and {60,61,64,65,69,71,74,85,93,96} (seats); v11a fixes the second set and still prints `vars= 8`, so neither residual is downstream of the other
- verdict: CONFIRMED

## [s3] HImode (s16) narrowing of the LZCR-read block or of shift_a/shift_b raises the frame from vars=8 toward the target's 16, measured on this candidate body
- mechanism: memory phantom-frame-slots-gcc272 records "two HImode locals feeding an HImode bitwise expression" as the minimal 8-byte phantom-slot trigger measured on func_80037540
- probe: cc1 v12a (lw_v1/li_v0 as s16), v12b (+ shift_a/shift_b s16), v12c (shift_b s16 only); read the `.frame` comment and diff against s2/v8a.s
- result: all three print `vars= 8, frame 40`; each also costs 2 insns (`sll $2,$2,16; sra $2,$2,16` re-extending the HImode `and` result for the SImode `0x16 - x`) and swaps the target's `lw $3 / li $2` seat pair to `lw $2 / li $3`
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body (s2 v8a), no FAKE constructs

## [s3] adding named-intermediate scalar locals (split products for the sum, a split `0x16 - li_v0`, a split `(log2_val<<6)/500`) raises the frame, measured on this candidate body
- mechanism: GCC 2.7.2 can reserve locals bytes for a computation it later register-allocates away (phantom-frame-slots-gcc272), so more live named pseudos might buy the slot
- probe: cc1 v14a (named products p1/p2/p3), v14b (named `li_c` / `li_v1` around the shift arithmetic), v14c (named `num` / `q` around the division), v14d (all combined)
- result: every variant prints `vars= 8`; scalar locals never take a frame slot here. v14b is strictly worse (regs= 4/0, 145 insns, a 4th callee-saved register)
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body, no FAKE constructs

## [s3] the target's extra 8 frame bytes belong to a slot allocated AFTER sp_tmp's, so no object expanded before the LZC island can be their source
- mechanism: function.c assign_stack_local hands out increasing frame_offsets; sp_tmp's slot comes from put_reg_into_stack at the moment the `"=m"` asm is expanded, and the target's island reads `addiu $v0,$sp,0x10` / `lw $v1,0x10($sp)` — i.e. sp_tmp sits at vars offset 0 with args=16
- probe: the same live 8-byte struct placed before the island (p2.c) and after it (p3.c); read `.frame` and the sp offsets in p2.s / p3.s
- result: p2 gives `vars= 16` but moves sp_tmp to `24($sp)` (the aggregate took offsets 0..7) — the island's hand-written 0x10 no longer names sp_tmp; p3 gives `vars= 16, frame 48` AND keeps `addiu $v0,$sp,0x10` / `lw $3,16($sp)`, reproducing the target's frame equation exactly (args 16 + vars 16 + gp regs 16 = 48)
- verdict: CONFIRMED

## [s3] a live 8-byte aggregate local in the post-island tail is a byte-usable source for the missing frame bytes
- mechanism: an aggregate local is never a register candidate in GCC 2.7.2, so it occupies its stack slot and every member access is a real load/store
- probe: cc1 p3 (`struct { s32 a, b; } q;` carrying scale and SCRV->x through the first scaling statement) diffed against s2/v8a.s and against the target
- result: the frame is exact but the body grows to 151 insns with real `sw $4,24($sp)`-class accesses, while the target contains NO sp-relative access other than the `addiu $v0,$sp,0x10` / `lw $v1,0x10($sp)` pair — so the target's extra 8 bytes are a phantom slot that is allocated and never touched, and any LIVE aggregate spelling is excluded by the target's own bytes
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body plus one live 8-byte struct (the construct under test), no FAKE constructs

## [s3] The 18-insn residual splits into 8 frame-offset insns and 10 register-seat insns, and the two are independent levers on this candidate body
- mechanism: the frame insns are the 4 prologue / 4 epilogue sp-offset-bearing insns (addiu sp / sw s0,s1,ra and their restores); the seat insns are every site where the target names $a1 for sum_sq plus the `move a0,a1` that reorg.c stole into the delay slot of `beqz v0` (ours emits `nop` there)
- probe: tools/pairdiff.py code6cac func_80018094 on the spliced candidate (tmp/grind/func_80018094/s3/base_pairdiff.txt), plus `grep .frame` on tmp/grind/func_80018094/s2/v11a.s, the s2 register-pin body that reproduces the whole seat cluster
- result: the pairdiff hunks are exactly {0,1,3,5,147,148,149,150} (frame) and {60,61,64,65,69,71,74,85,93,96} (seats); v11a fixes the second set and still prints `.frame $sp,40 # vars= 8`, so neither residual is downstream of the other and either can be closed alone
- verdict: CONFIRMED

## [s3] HImode (s16) narrowing of the sibling LZCR-read block, or of shift_a/shift_b, raises the frame from vars=8 toward the target's vars=16 on this candidate body
- mechanism: memory phantom-frame-slots-gcc272 records 'two HImode locals feeding an HImode bitwise expression' as the minimal 8-byte phantom-slot trigger measured on func_80037540, and the LZCR-read block (`lw_v1 & li_v0`) is exactly that shape here
- probe: cc1 v12a (lw_v1/li_v0 as s16), v12b (+ shift_a/shift_b s16), v12c (shift_b s16 only) via tmp/grind/func_80018094/s3/cc.sh; read the `.frame` comment and diff each .s against s2/v8a.s
- result: all three print `vars= 8, frame 40` — unchanged — and each costs 2 extra insns (`sll $2,$2,16; sra $2,$2,16` re-extending the HImode `and` result for the SImode `0x16 - x`) while swapping the target's `lw $3 / li $2` seat pair to `lw $2 / li $3`. Banked as rejected/lzcr-read-himode-narrowing-adds-sign-extend.c
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body (s2 v8a: s1 v6a + the sibling LZCR-read block), no FAKE constructs

## [s3] Adding named-intermediate scalar locals (split products for the sum, a split 0x16 - li_v0, a split (log2_val<<6)/500) raises the frame on this candidate body
- mechanism: GCC 2.7.2 can reserve locals bytes for a computation it later register-allocates away (phantom-frame-slots-gcc272), so more live named pseudos were the cheapest candidate source of the slot
- probe: cc1 v14a (named products), v14b (named li_c/li_v1 around the shift arithmetic), v14c (named num/q around the division), v14d (all combined); read `.frame` and the insn count
- result: every variant prints `vars= 8`; scalar locals never take a frame slot in this function. v14b is strictly worse (regs= 4/0, i.e. a 4th callee-saved register, and 145 insns). Banked as rejected/shift-arith-named-intermediates-extra-saved-reg.c
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body, no FAKE constructs

## [s3] The target's extra 8 frame bytes belong to a slot allocated AFTER sp_tmp's slot, so no object expanded before the LZC island can be their source on this body
- mechanism: function.c assign_stack_local hands out increasing frame_offsets, and sp_tmp's slot comes from put_reg_into_stack at the moment the `=m` asm is expanded; the target's island writes `addiu $v0,$sp,0x10` and its C-side read is `lw $v1,0x10($sp)`, pinning sp_tmp to vars offset 0 with args=16
- probe: the same live 8-byte struct placed before the island (tmp/grind/func_80018094/s3/p2.c) and after it (p3.c); read `.frame` plus the sp offsets in p2.s / p3.s
- result: p2 gives `vars= 16` but moves sp_tmp to `24($sp)`, which breaks the island's hand-written 0x10; p3 gives `vars= 16, frame 48` AND keeps `addiu $v0,$sp,0x10` / `lw $3,16($sp)` — the target's exact frame equation (args 16 + vars 16 + gp regs 16 = 48). The +8 magnitude is therefore correct and reachable, and its position is fixed
- verdict: CONFIRMED

## [s3] A live 8-byte aggregate local in the post-island tail is a byte-usable source for the missing frame bytes on this candidate body
- mechanism: an aggregate local is never a register candidate in GCC 2.7.2, so it occupies its stack slot and every member access is emitted as a real load/store
- probe: cc1 p3 (`struct { s32 a, b; } q;` carrying scale and SCRV->x through the first scaling statement) diffed against s2/v8a.s and against asm/funcs/func_80018094.s
- result: the frame is exact but the body grows to 151 insns with real `sw $4,24($sp)`-class accesses, while the target contains NO sp-relative access other than the `addiu $v0,$sp,0x10` / `lw $v1,0x10($sp)` pair — so the target's extra 8 bytes are a phantom slot that is allocated and never touched, and the LIVE-aggregate spelling is excluded by the target's own bytes. Banked as rejected/frame-8byte-aggregate-after-island-costs-3-insns.c
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body plus one live 8-byte struct (the construct under test), no FAKE constructs

## [s3] Every non-spill stack slot GCC 2.7.2 allocates in this codebase is a BLKmode (aggregate) temp, so the missing frame bytes come from an aggregate-typed object rather than any scalar
- mechanism: function.c:896 sets ctx=stack_temp for assign_stack_temp; the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_FRAME_DEBUG=1) prints mode/size/frame_offset for every slot handed out
- probe: BB2_FRAME_DEBUG=1 compile of the whole src/code6cac_c2.c TU (tmp/grind/func_80018094/s3/fd.sh code6cac_c2 -> code6cac_c2.fd, 53 slot events)
- result: only three contexts appear — round_frame (39), stack_temp (11), spill_new_pNN (3) — and all 11 stack_temp events are `mode=26` (BLKmode) at sizes 8/16/32/256/1024. No SImode or HImode stack temp is ever allocated in this TU, which is why every scalar lever tried in s2 and s3 is frame-neutral
- verdict: CONFIRMED

## [s4] The target's extra 8 frame bytes are the unwritten TAIL of the LZC output local itself, not a separate post-island stack temp
- mechanism: mips.c compute_frame_size / function.c assign_stack_local - get_frame_size returns the raw locals size and MIPS_STACK_ALIGN rounds it; a 4-byte scalar rounds to 8, a 9..16-byte object stays 16, and the object still starts at vars offset 0 so the island's hand-written `addiu $v0,$sp,0x10` keeps naming it
- probe: compile `s32 sp_tmp[4]` / `s32 sp_tmp[3]` / `struct {s32 a,b,c;}` variants (tmp/grind/func_80018094/s4/v20{b,c,d,g,h}.c) with s4/cc.sh, read `.frame`, diff against m0.s; then splice the array form into src/code6cac.c and run `sandbox func_80018094 --disable all`
- result: every variant prints `.frame $sp,48 # vars= 16, regs= 3/0, args= 16` and differs from the scalar chassis ONLY in the 8 frame-offset insns; sandbox 18 -> 10. All four spellings are byte-identical to each other, so the original size is recoverable only as the range raw 9..16. s3's "post-island BLKmode stack temp" mechanism is refuted - no temp is involved
- verdict: CONFIRMED

## [s4] Staging the LZCR-read result through the enclosing block's dead `log2_val` local seats sum_sq in $a1 for every use
- mechanism: staged-value-reused-variable (owner ruling 2026-07-03) - the extra reference on log2_val's allocno changes local-alloc/global.c ordering so the sum_sq allocno is no longer first to the ascending free-register scan; found by decomp-permuter, not by hand
- probe: campaign s4-v20g-framefixed (tmp/perm_80018094b, base 245, --stack-diffs -j 8) output-230-1 at 24,345 iterations; the single semantic delta (`log2_val = li_v0; shift_a = 0x16 - log2_val;`) transplanted onto candidate.c as tmp/grind/func_80018094/s4/cand_v21a.c and measured with the sandbox
- result: sandbox 10 -> 7; `addu at,at,a1` and `srav v0,a1,v1` become byte-identical to the target, i.e. sum_sq is now seated in $a1 across the whole if-chain exactly as s2/s3 predicted it must be
- verdict: CONFIRMED

## [s4] The decomp-permuter did not reach the remaining 7 insns from the log2-staged chassis in 45,681 iterations
- mechanism: the residual is a register-seat problem whose only known solver is fixing the island input to $a0; the permuter's randomization operates on C statements/types and never emits a construct that materialises a value in a named hard register
- probe: campaign s4-v21a-log2staged (tmp/perm_80018094c, base score 230, --stack-diffs, -j 8), harvested with --stop
- result: 45,681 iterations / ~18 minutes with ZERO novel finds (no output-* dirs at all). The earlier chassis campaigns bracket it: s4-v8a-chassis 8,906 iterations / 0 improvements, s4-v20g-framefixed one improvement at ~9.5 min. 78,932 iterations total this session
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 (s32 sp_tmp[4] + log2_val staging, both FAKE-annotated), decomp-permuter default randomization with --stack-diffs, -j 8

## [s4] All 7 remaining residual insns are downstream of the single missing `move a0,a1` island-input copy
- mechanism: with $a0 reserved for the copy allocno across the else-branch, local-alloc's ascending find_free_reg is forced to give li_v0 $v0 and log2_val $a1; without the copy both land in $a0 and the island reads sum_sq's $a1 directly
- probe: tools/pairdiff.py code6cac func_80018094 on the spliced floor-7 candidate (tmp/grind/func_80018094/s4/v21a_pairdiff.txt)
- result: the 7 hunks are index 69 (`nop` vs `move a0,a1`), 74/75 (`srl a0` vs `srl a1`, `move t4,a1` vs `move t4,a0`), 82/84 (li_v0 in $a0 vs $v0), 93/96 (log2_val in $a0 vs $a1) - one cause, six consequences. s2's register-pin diagnostic (v11a) already reproduced this whole cluster, and s2 proved cse.c canon_reg deletes every pin-free C-level copy
- verdict: CONFIRMED

## [s4] The target's extra 8 frame bytes are the unwritten TAIL of the LZC output local itself, not a separate post-island stack temp
- mechanism: mips.c compute_frame_size / function.c assign_stack_local — get_frame_size returns the raw locals size and MIPS_STACK_ALIGN rounds it, so a 4-byte scalar rounds to 8 while a 9..16-byte object stays 16; the object still starts at vars offset 0, so the island's hand-written `addiu $v0,$sp,0x10` keeps naming it
- probe: compiled `s32 sp_tmp[4]`, `s32 sp_tmp[3]` and `struct {s32 a,b,c;}` variants (tmp/grind/func_80018094/s4/v20{b,c,d,g,h}.c) with s4/cc.sh, read the `.frame` comment and diffed each against m0.s; then spliced the array form into src/code6cac.c and ran `sandbox func_80018094 --disable all`
- result: every variant prints `.frame $sp,48 # vars= 16, regs= 3/0, args= 16` — the target's exact frame — and differs from the scalar chassis ONLY in the 8 frame-offset insns; sandbox 18 -> 10. All four spellings are byte-identical to one another, so the original's declared size is recoverable only as the range raw 9..16 bytes. s3's frontier mechanism (an 8-byte BLKmode stack TEMP allocated in the post-island tail) is refuted: no temp is involved, the sp_tmp slot itself was oversized in the original. Frame-math proof for the OVERSIZED-LOCALS carve-out (.claude/rules/dead-vars-local-array.md, owner ruling 2026-07-13): target frame 0x30 = args 0x10 + locals 0x10 + saves 0x10, the only locals traffic in the whole target is `swc2 $31,0($sp+0x10)` / `lw $v1,0x10($sp)` (4 bytes of 16), and a fully-written 4-byte locals set gives ALIGN8(4)+16+16 = 0x28 != 0x30
- verdict: CONFIRMED

## [s4] Staging the LZCR-read result through the enclosing block's currently-dead log2_val local seats sum_sq in $a1 for every one of its uses
- mechanism: staged-value-reused-variable (owner ruling 2026-07-03) — the extra reference on log2_val's allocno changes local-alloc / global.c find_reg ordering so the sum_sq allocno is no longer first to the ascending free-register scan and no longer takes $a0
- probe: permuter campaign s4-v20g-framefixed (tmp/perm_80018094b, base score 245, --stack-diffs, -j 8) produced output-230-1 at 24,345 iterations / ~9.5 min; its single semantic delta (`log2_val = li_v0; shift_a = 0x16 - log2_val;`) was transplanted onto candidate.c as tmp/grind/func_80018094/s4/cand_v21a.c and measured with the sandbox
- result: sandbox 10 -> 7. `addu at,at,a1` and `srav v0,a1,v1` become byte-identical to the target, i.e. sum_sq is now seated in $a1 across the whole if-chain exactly as s2 and s3 predicted it must be. Banked source: tmp/grind/func_80018094/s4/perm_find_230.c
- verdict: CONFIRMED

## [s4] The decomp-permuter did not reach the remaining 7 insns from the log2-staged chassis in 45,681 iterations
- mechanism: the residual is a register-seat problem whose only known solver is fixing the island input to $a0; the permuter's randomization operates on C statements and types and never emits a construct that materialises a value in a named hard register
- probe: campaign s4-v21a-log2staged (tmp/perm_80018094c, base score 230, --stack-diffs, -j 8), waited in-turn with `permuter_campaign.py wait` and harvested with --stop
- result: 45,681 iterations / ~18 minutes with ZERO novel finds (no output-* directories at all). The other two campaigns bracket it: s4-v8a-chassis 8,906 iterations / 0 improvements, s4-v20g-framefixed 24,345 iterations / one improvement at ~9.5 min. 78,932 iterations total this session, all three campaigns harvested and stopped (0 live at session end)
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 (s32 sp_tmp[4] + the log2_val staging, both FAKE-annotated), decomp-permuter default randomization with --stack-diffs, -j 8

## [s4] All 7 remaining residual insns are downstream of the single missing `move a0,a1` island-input copy
- mechanism: with $a0 reserved for the copy allocno across the else branch, local-alloc's ascending find_free_reg is forced to give li_v0 $v0 and log2_val $a1; without the copy both land in $a0 and the island reads sum_sq's $a1 directly
- probe: tools/pairdiff.py code6cac func_80018094 on the spliced floor-7 candidate (tmp/grind/func_80018094/s4/v21a_pairdiff.txt)
- result: the 7 hunks are index 69 (`nop` vs `move a0,a1`, the copy reorg parks in the `beqz v0` delay slot), 74/75 (`srl a0` vs `srl a1`, `move t4,a1` vs `move t4,a0`), 82/84 (li_v0 in $a0 vs the target's $v0) and 93/96 (log2_val in $a0 vs $a1) — one cause and six consequences. s2's register-pin diagnostic (v11a) already reproduced this whole cluster, and s2's dump attribution proved cse.c canon_reg deletes every pin-free C-level copy
- verdict: CONFIRMED

## [s5] Re-measuring the s2 tied-asm-output island-input copy on the CURRENT floor-7 chassis materialises the copy but seats it in $v1
- mechanism: a tied asm operand pair (`: "=m"(sp_tmp[0]), "=r"(lz_in) : "1"(sum_sq)`) forces GCC to emit a real `move lz_in,sum_sq` before the asm because the output pseudo is distinct from sum_sq's, which cse.c canon_reg cannot delete (unlike every pin-free C-level copy killed in s1 H7 / s2 H13); reorg.c then parks that copy in the `beqz v0` delay slot exactly as the target does. s2 measured this lever BEFORE the s4 frame fix and the log2_val staging, so its allocno ordering was stale
- probe: four variants on the current chassis (tmp/grind/func_80018094/s5/w1..w4.c: lz_in declared in the else arm vs at function top, each with and without the log2_val staging), compiled with s5/cc.sh and each spliced into src/code6cac.c for `sandbox func_80018094 --disable all`
- result: the copy IS materialised and IS delay-slot-parked in all four, but takes $v1, and adding it displaces sum_sq from $a1 to $a0 — sandbox 7 -> 10 (with staging, w1/w2) and 7 -> 11 (without, w3/w4). Declaration scope is inert (w1 == w2 byte-for-byte, w3 == w4), which is itself the signal that the copy pseudo is block-local. Banked: rejected/tied-copy-dowhile-wrap-copy-seats-v1-not-a0-equal-floor-7.c (the improved w6 form below)
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 (s32 sp_tmp[4] + log2_val staging, both FAKE-annotated) plus one tied asm output operand (the construct under test)

## [s5] A do{...}while(0) wrap around the inner if-chain recovers the 3 insns the tied-operand copy costs, giving a floor-7 chassis that CARRIES the copy
- mechanism: found by the decomp-permuter, not by hand — campaign s5-w1-tiedcopy (tmp/perm_80018094d, base score 55) produced output-35-1 at 23s combining a do-while(0) wrap of `if (sum_sq < 0x400) {...} else {...} scale = ...` with a split sum accumulation (`new_var = dx*dx + dy*dy; sum_sq = new_var + dz*dz;`). Family: do-while-zero-exception (owner ruling 2026-07-06, ANY codegen effect incl. register allocation)
- probe: the two deltas split apart and measured separately (s5/w5.c = split sum only, w6.c = do-while only, w7.c = both), each spliced into src/code6cac.c and sandboxed; then `tools/pairdiff.py code6cac func_80018094` on w6 (s5/w6_pairdiff.txt)
- result: the split sum is inert (w5 = 10, same as w1); the do-while(0) wrap alone is the whole find (w6 = 7, w7 = 7). w6 is a genuinely different chassis at the SAME floor: the copy now exists, reorg parks it in the `beqz v0` delay slot, and sum_sq is correctly seated in $a1 — the 7 differing insns become a single register-naming question (`move v1,a1` / `move t4,v1` where the target has `move a0,a1` / `move t4,a0`, plus the four consequences that follow from $a0 being free for log2_val and li_v0)
- verdict: CONFIRMED

## [s5] The island-input copy this chassis emits is a block-LOCAL quantity, so no reordering of the else block's own quantities can move it off $v1
- mechanism: local_alloc's `find_free_reg` scans ascending over `used = fixed_reg_set | union(regs_live_at[birth..death))` and runs BEFORE global_alloc, so the copy claims its register before sum_sq/scale/log2_val exist as assignments
- probe: tools/ra_solver/extract.py + simulate.py (global model: sort order MATCH, dispositions 10/12; pseudo 77 = sum_sq, 98 = log2_val, 78 = scale) and local_extract.py + local_alloc.py (order 6/6 blocks, assign 29/34) on the spliced w1 and w6 bodies; then tools/ra_solver/inverse.py local --block 6 --goal '{"0": 4}' --depth 2
- result: the copy is `code6cac.local.json func_80018094 blk 6 qty 0` — birth 4, death 5, refs 2, got 3. $2 is in `used` (the island's clobber list), nothing else is, so $3 is the first free register. NO other local qty of block 6 overlaps [4,5] (the next birth is 6), so the block's own allocation order cannot occupy $3 there. The target's copy must therefore be a cross-block GLOBAL allocno (defined in the pre-branch block, used at the island), which the tied-operand spelling structurally cannot produce — GCC emits a tied-operand copy immediately before the asm, inside the else block. inverse.py returns exactly ONE minimal vector: [live_extend] qty 0 dies later (5 -> 9)
- verdict: CONFIRMED

## [s5] The only honest spelling of the inverse solver's live-extend vector available in this body overshoots and lets CSE refold the copy
- mechanism: [live_extend] qty 0 death 5 -> 9 needs the copy's value read once, shortly after the island and before `li $v0,-2`; the sole real consumer of sum_sq's value in that region is the post-island LUT index `sum_sq >> shift_a`, which sits at ~18, not ~9
- probe: s5/w8.c (`(u8)(*(&D_8008D118 + (lz_in >> shift_a)))` in place of the sum_sq read), compiled and sandboxed; s5/w9.c (a dead `log2_val = lz_in;` store) run only as a diagnostic of the same vector
- result: w8 = 12 — with lz_in carrying the later read, sum_sq dies at the copy, cse re-unifies the two values and the copy insn vanishes altogether (148 emitted lines vs 153). w9 (dead store, diagnostic only, not committable) = 7, i.e. the vector is not reached by a same-block extra reference either. Banked as rejected/lzc-input-tied-copy-live-extend-to-lut-index-cse-refolds.c
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), the s5 w6 body (candidate.c + tied asm output + do-while(0) wrap, three FAKE constructs) with the LUT-index read re-sourced from lz_in

## [s5] The decomp-permuter reached the do-while(0) find quickly on the tied-copy chassis but nothing further in 38,542 iterations
- mechanism: fresh-seed discipline — a basin yields early or not at all; the tied-copy chassis is structurally different from the s4 floor-7 chassis (the copy insn exists), so it is a valid re-seed under the 2026-09-01 chassis rule
- probe: campaign s5-w1-tiedcopy (tmp/perm_80018094d, base score 55, --stack-diffs, -j 8), waited in-turn with `permuter_campaign.py wait`, harvested with --stop
- result: three finds — output-35-1 at 23s (the do-while(0) wrap, transplanted above), output-45-1 at 880s and output-55-1 at 942s (both non-improving relative to 35). 38,542 iterations total, campaign stopped, 0 live at session end. The whole yield of the basin arrived in the first 25 seconds
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), the s5 w1 body (candidate.c + tied asm output, permuter base score 55), decomp-permuter default randomization with --stack-diffs, -j 8

## [s5] Re-measuring the s2 tied-asm-output island-input copy on the current floor-7 chassis materialises the copy but seats it in $v1 and displaces sum_sq from $a1 to $a0
- mechanism: a tied asm operand pair (`: "=m"(sp_tmp[0]), "=r"(lz_in) : "1"(sum_sq)`) forces GCC to emit a real `move lz_in,sum_sq` before the asm because the output pseudo is distinct from sum_sq's, which cse.c canon_reg cannot delete (unlike every pin-free C-level copy killed in s1 H7 / s2 H13); reorg.c then parks that copy in the `beqz v0` delay slot exactly where the target has it. s2 measured this lever before the s4 frame fix and log2_val staging, so its allocno ordering was stale
- probe: four variants on the current chassis (tmp/grind/func_80018094/s5/w1..w4.c: lz_in declared in the else arm vs at function top, each with and without the log2_val staging), compiled with s5/cc.sh and each spliced into src/code6cac.c for `sandbox func_80018094 --disable all`
- result: the copy is materialised and delay-slot-parked in all four, but takes $v1; sandbox 7 -> 10 with the staging (w1/w2) and 7 -> 11 without it (w3/w4). Declaration scope is inert (w1 == w2 byte-for-byte, w3 == w4), which is itself the signal that the copy pseudo is block-local rather than global
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 (s32 sp_tmp[4] + log2_val staging, both FAKE-annotated) plus one tied asm output operand (the construct under test)

## [s5] A do{...}while(0) wrap around the inner if-chain recovers the 3 insns the tied-operand copy costs, giving a floor-7 chassis that carries the copy with sum_sq correctly seated in $a1
- mechanism: found by the decomp-permuter, not by hand - campaign s5-w1-tiedcopy (tmp/perm_80018094d, base score 55) produced output-35-1 at 23s combining a do-while(0) wrap of `if (sum_sq < 0x400) {...} else {...} scale = ...` with a split sum accumulation. Family: do-while-zero-exception (owner ruling 2026-07-06, ANY codegen effect incl. register allocation)
- probe: the two deltas split apart and measured separately (s5/w5.c = split sum only, w6.c = do-while only, w7.c = both), each spliced into src/code6cac.c and sandboxed; then tools/pairdiff.py code6cac func_80018094 on w6 (s5/w6_pairdiff.txt)
- result: the split sum is inert (w5 = 10, same as w1); the do-while(0) wrap alone is the whole find (w6 = 7, w7 = 7). w6 is a different chassis at the same floor: copy present, reorg-parked in the `beqz v0` delay slot, sum_sq in $a1 - and all 7 differing insns reduce to one register name (ours `move v1,a1` / `move t4,v1` vs target `move a0,a1` / `move t4,a0`, plus the four consequences of $a0 being free for log2_val and li_v0). Banked as rejected/tied-copy-dowhile-wrap-copy-seats-v1-not-a0-equal-floor-7.c
- verdict: CONFIRMED

## [s5] The island-input copy this chassis emits is a block-LOCAL quantity of the else block (blk 6 qty 0, birth 4, death 5, refs 2, got $3), so no reordering of that block's own quantities can move it off $v1
- mechanism: local_alloc's find_free_reg scans ascending over `used = fixed_reg_set | union(regs_live_at[birth..death))` and runs BEFORE global_alloc, so the copy claims its register before sum_sq/scale/log2_val exist as assignments; $2 is in `used` (the island's own clobber list) and nothing else is
- probe: tools/ra_solver/extract.py + simulate.py (global model: sort order MATCH, dispositions 10/12; pseudo 77 = sum_sq nrefs 8 livelen 16, 98 = log2_val nrefs 5 livelen 7 prefs [4], 78 = scale nrefs 6 livelen 15 - the copy is NOT among them) and local_extract.py + local_alloc.py (order 6/6 blocks, assign 29/34) on the spliced w1 and w6 bodies; then tools/ra_solver/inverse.py local --block 6 --goal '{"0": 4}' --depth 2
- result: the copy is code6cac.local.json func_80018094 blk 6 qty 0; the next qty in the block is born at 6, so nothing can overlap [4,5]. The target's copy must therefore be a cross-block GLOBAL allocno (defined in the pre-branch block, used at the island), which a tied operand structurally cannot produce - GCC emits a tied-operand copy immediately before the asm. inverse.py returns exactly ONE minimal vector: [live_extend] qty 0 dies later (5 -> 9)
- verdict: CONFIRMED

## [s5] The only honest spelling of the inverse solver's live-extend vector available in this body (re-sourcing the post-island LUT index from the tied output lz_in) overshoots and lets cse refold the copy away
- mechanism: [live_extend] qty 0 death 5 -> 9 needs the copy's value read shortly after the island and before `li $v0,-2`; the sole real consumer of sum_sq's value in that region is the LUT index `sum_sq >> shift_a`, which sits at ~18, not ~9
- probe: s5/w8.c (`(u8)(*(&D_8008D118 + (lz_in >> shift_a)))` in place of the sum_sq read), compiled with s5/cc.sh and sandboxed; s5/w9.c (a dead `log2_val = lz_in;` store) run only as a diagnostic of the same vector, never as a candidate
- result: w8 = 12 - with lz_in carrying the later read, sum_sq dies at the copy, cse re-unifies the two values and the copy insn vanishes (148 emitted lines vs 153). The dead-store diagnostic w9 = 7, i.e. an extra same-block reference does not reach the vector either. Banked as rejected/lzc-input-tied-copy-live-extend-to-lut-index-cse-refolds.c
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), the s5 w6 body (candidate.c + tied asm output + do-while(0) wrap, three FAKE constructs present) with the LUT-index read re-sourced from lz_in

## [s5] The decomp-permuter reached the do-while(0) find in the first 25 seconds on the tied-copy chassis and nothing further in 38,542 iterations
- mechanism: fresh-seed discipline - a basin yields early or not at all; the tied-copy chassis is structurally different from the s4 floor-7 chassis (the copy insn exists in the stream), so it is a valid re-seed under the 2026-09-01 chassis rule
- probe: campaign s5-w1-tiedcopy (tmp/perm_80018094d, base score 55, --stack-diffs, -j 8), waited in-turn with `permuter_campaign.py wait --dir`, harvested with --stop
- result: three finds - output-35-1 at 23 s (the do-while(0) wrap, transplanted and measured above), output-45-1 at 880 s and output-55-1 at 942 s (both non-improving relative to 35). 38,542 iterations; campaign stopped, 0 live at session end
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), the s5 w1 body (candidate.c + tied asm output, permuter base score 55), decomp-permuter default randomization with --stack-diffs, -j 8

## s6 (structural, 2026-09-09)

- **H27 — KILLED (class, cse.c:8102).** A plain pre-branch copy `lz_in = sum_sq;` in the else arm,
  with the island reading `"r"(lz_in)`, survives to the island in some declaration scope.
  Measured at two scopes (else-arm block s6/v1.c; function top, assigned immediately after sum_sq,
  s6/v2.c): both byte-identical to candidate.c. Dump-attributed to cse.c — the copy is live in
  v1.i.jump and gone in v1.i.cse. Gate: `cse_end_of_basic_block`'s follow-jumps arm
  (tools/gcc-2.7.2/cse.c:8102) extends cse's path through the `beqz` into the island block because
  LABEL_NUSES(island)==1 and a BARRIER precedes the label — both properties of the target's own
  control-flow shape, unaffected by how the copy is spelled. Banked:
  rejected/lzc-input-prebranch-copy-cse-follow-jumps-deletes-remeasured-s6.c.
- **H28 — KILLED (instance).** Breaking cse's LABEL_NUSES==1 predicate from C by giving the LUT
  arm a second (redundant) conjunct `sum_sq >= 0`. Measured (s6/d1.c with the copy, s6/d2.c
  without): jump optimisation folds the redundant test and both outputs are byte-identical to
  candidate.c, so the second label reference never materialises.
- **H29 — KILLED (instance), and it REFUTES the s5 frontier.** "The target's island-input copy is
  a cross-block GLOBAL allocno defined in the pre-branch block; make a copy survive to that shape
  and it takes $a0." Measured with a scratch-only diagnostic compile
  (`-fno-cse-follow-jumps -fno-cse-skip-blocks` on s6/v1.c — never a build path, see
  .claude/rules/no-compiler-divergence.md; used solely to isolate the pass): the copy survives,
  `v1.i.lreg` shows reg 99 is not a local qty, `v1.i.greg` lists it among the global allocnos, and
  reorg parks it in the `beqz` delay slot in the target's exact position — and it is assigned
  **$3**, not $a0 (s6/v1.diag.s:95). Mechanism: `tools/gcc-2.7.2/config/mips/mips.h` defines no
  REG_ALLOC_ORDER, so global.c `find_reg` and local-alloc `find_free_reg` both scan ascending;
  $2 is closed by the island's clobber list and $3 is the first free register for either allocator.
- **H30 — KILLED (instance).** The s4 `log2_val` staging is removable now that the copy exists.
  Measured: tied chassis without the staging (s6/t4.c) = sandbox **11** (vs 7); on the copy chassis
  it moves sum_sq to $a0 (s6/v5.diag.s). Banked:
  rejected/tied-copy-chassis-without-log2-staging-sandbox-11.c.
- **H31 — KILLED (instance).** Declaration-order lever: hoisting `s32 li_v0 = -2;` above the island
  so its qty is born before the copy and overlaps it in find_free_reg's `used` set. Measured
  (s6/t1.c): the copy still takes $3 and is pushed out of the `beqz` delay slot — strictly worse.
  Hoisting only the declaration (s6/t2.c) is byte-identical to w6. Banked:
  rejected/minus2-const-holder-hoisted-above-island-copy-leaves-delay-slot.c.
- **H32 — KILLED (instance).** Swapping the inner arms (`if (sum_sq >= 0x400) { island } else { LUT }`,
  s6/v3.c) so the island becomes the fall-through: +1 insn (149 vs 148 emitted lines), 21 asm lines
  moved; the target's shape is ours (asm/funcs/func_80018094.s:71 branches TO the island). Adding
  the pre-branch copy on top (s6/v4.c) is byte-identical to v3. Banked:
  rejected/inner-arms-swapped-island-fallthrough-plus-one-insn.c.

## [s6] A plain pre-branch copy `lz_in = sum_sq;` in the else arm, with the island reading "r"(lz_in), can be made to survive to the island by choosing a different declaration scope for lz_in.
- mechanism: cse.c's extended-path walk: cse_end_of_basic_block's follow-jumps arm extends the path through the `beqz` into the island block whenever LABEL_NUSES(target label)==1 and the insn before the label is a BARRIER. Inside that extended path reg 99 is a known equivalent of reg 77, so canon_reg rewrites the asm operand and the copy dies. Both predicates are properties of the target's own control-flow shape (the LUT arm ends in `j` to the merge, which emits the BARRIER), so no spelling of the copy touches them.
- probe: Built the copy at two declaration scopes - else-arm block (tmp/grind/func_80018094/s6/v1.c) and function top assigned immediately after sum_sq (s6/v2.c) - compiled both with the canonical cc1 flags; both .s outputs are byte-identical to candidate.c (s6/v1.s == s6/v2.s == s6/w0.s). Pass attribution from -da dumps: s6/d_v1/v1.i.jump still has `insn 103 (set (reg/v:SI 99) (reg/v:SI 77))` with the asm operand as reg 99; s6/d_v1/v1.i.cse has the operand as reg 77 and no insn 103.
- result: Both scopes byte-identical to the floor-7 candidate; the deletion is cse.c, dump-proven rather than inferred. Re-confirms s1 H7 on the current chassis and names the gate. Banked as rejected/lzc-input-prebranch-copy-cse-follow-jumps-deletes-remeasured-s6.c.
- verdict: KILLED
- kill_scope: class
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 (s32 sp_tmp[4] + log2_val staging, both FAKE-annotated), plus the copy under test
- predicate_cite: tools/gcc-2.7.2/cse.c:8102

## [s6] The target's island-input copy takes $a0 because it is a cross-block GLOBAL allocno defined in the pre-branch block, so producing a copy in that shape (rather than the tied-asm block-local copy of s5) would seat it at $a0.
- mechanism: s5 read the ra_solver local model (copy = blk 6 qty 0, find_free_reg ascending, $2 closed by the island clobber, $3 first free) and concluded local_alloc was the discriminator, so a global allocno would be allocated by global.c find_reg instead and could reach $a0.
- probe: Scratch-only diagnostic compile of s6/v1.c with -fno-cse-follow-jumps -fno-cse-skip-blocks (never a build path; used solely to isolate the pass). The copy survives; s6/d2_v1/v1.i.lreg shows reg 99 is not a local qty; v1.i.greg lists it in ';; 13 regs to allocate: 98 78 120 77 99 ...' with ';; 99 conflicts: 72 73 77 99 2 12 29'; reorg parks it in the `beqz` delay slot in the target's exact position (s6/v1.diag.s:95 `move $3,$5` under `beq $2,$0,.L6`). Dispositions: 77 in 5, 78 in 3, 98 in 4, 99 in 3.
- result: The copy IS the global cross-block allocno s5 asked for, in the right slot, and it is still assigned $3 rather than $a0. Local-vs-global allocation is not the discriminator here: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so global.c find_reg and local-alloc find_free_reg both scan hard regs ascending and both land on $3 once $2 is closed by the island's clobber list. The real requirement, read off the greg conflict sets, is two conflict-set changes about OTHER pseudos: allocno 99 must conflict with 78 (scale, seated $3) to close $3, and 98 (log2_val) must stop conflicting with 77 (sum_sq) so both can share $5.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 plus a pre-branch copy, compiled with cse follow-jumps/skip-blocks disabled as a diagnostic

## [s6] cse.c's LABEL_NUSES(island label)==1 predicate can be broken from C by giving the LUT arm a second conjunct (`sum_sq < 0x400 && sum_sq >= 0`), which would emit a second branch to the island label.
- mechanism: cse_end_of_basic_block only extends the path when the branch target label has exactly one use; a compound && condition emits two conditional branches around the then-arm.
- probe: s6/d1.c (compound condition + pre-branch copy) and s6/d2.c (compound condition alone), compiled with the canonical flags.
- result: Both are byte-identical to candidate.c - jump optimisation folds the redundant `sum_sq >= 0` test away (it is implied by the enclosing else-if), so the second label reference never exists and the copy is still deleted.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7, FAKE constructs sp_tmp[4] + log2_val staging present

## [s6] With the island-input copy materialised by the s5 tied asm output, the s4 log2_val staging is no longer needed and dropping it improves or holds the floor.
- mechanism: s4 introduced the staging to seat sum_sq at $a1; with a copy present the seat might be held by the copy relationship instead.
- probe: s6/t4.c = the s5 w6 body (tied output + do-while(0) wrap) with `log2_val = li_v0; shift_a = 0x16 - log2_val;` replaced by `shift_a = 0x16 - li_v0;`. Spliced into src/code6cac.c and scored with `sandbox func_80018094 --disable all`.
- result: sandbox 11 (vs 7 for w6 and for candidate.c). The staging is load-bearing on both chassis; on the surviving-copy chassis dropping it moves sum_sq to $a0 (s6/v5.diag.s `move $3,$4`). The greg dump explains why: the staging is what makes pseudo 98 (log2_val) conflict with 77 (sum_sq), which keeps 77 off $a0 - while simultaneously forcing 98 onto its preferred $4, the register the target reserves for the copy. Banked as rejected/tied-copy-chassis-without-log2-staging-sandbox-11.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), the s5 w6 body (candidate.c + tied asm output + do-while(0) wrap, three FAKE constructs) with the staging removed

## [s6] Hoisting the `s32 li_v0 = -2;` constant holder above the LZC island makes its quantity overlap the island-input copy in find_free_reg's `used` set and pushes the copy off $v1.
- mechanism: local-alloc's find_free_reg excludes hard regs live across the quantity's range; a longer-lived quantity born before the copy and allocated first would take $3 and force the copy to the next free register.
- probe: s6/t1.c (declaration + initialiser hoisted above the asm) and s6/t2.c (declaration only, initialiser left in place), compiled with the canonical flags and compared against s6/t0.s (the w6 body).
- result: t1: the copy still takes $3 and is additionally pushed OUT of the `beqz` delay slot (s6/t1.s:108 vs t0.s:95) - strictly worse. t2: byte-identical to w6. Banked as rejected/minus2-const-holder-hoisted-above-island-copy-leaves-delay-slot.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), the s5 w6 body (three FAKE constructs) with the constant holder hoisted

## [s6] Swapping the inner arms so the LZC island is the fall-through and the small-LUT arm is the branch target changes which basic block the copy is born in and closes part of the residual.
- mechanism: Frontier probe (a) from s5: the sibling shape at src/code6cac_b.c:1385 nests the tests differently, which changes block membership and therefore which allocator sees the copy.
- probe: s6/v3.c (`if (sum_sq >= 0x400) { island } else { LUT }`) and s6/v4.c (v3 plus the pre-branch copy), compiled with the canonical flags.
- result: v3 emits 149 lines vs 148 for candidate.c (+1 insn) and moves 21 asm lines; the target's own shape is ours (asm/funcs/func_80018094.s:71 `beqz $v0,.L800181C0` branches TO the island). v4 is byte-identical to v3 - with the island as the fall-through, cse folds the copy without even needing the follow-jumps path. Banked as rejected/inner-arms-swapped-island-fallthrough-plus-one-insn.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis 2026-09-09 (-mel -msoft-float), candidate.c body at floor 7 with the inner arms swapped
