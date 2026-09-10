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
