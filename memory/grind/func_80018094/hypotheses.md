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
