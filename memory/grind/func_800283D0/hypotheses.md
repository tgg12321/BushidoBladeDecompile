# Hypothesis ledger — func_800283D0

## Session 1 (2026-08-19, recon) — measured

| id | hypothesis | mechanism | probe | result | verdict |
|---|---|---|---|---|---|
| H1 | s32 raw holder (872C sibling spelling) makes the & 0xFFFF reach RTL and survive as andi while the load stays lhu | tree-level fold-const only folds by declared type; combine won't substitute a multi-use AND | `u16 temp_a1` -> `s32 temp_a1`, sandbox + objdump | andi v1,raw,0xffff emitted, lhu kept; 44 flat (a1/a2 swap remained) | CONFIRMED |
| H2 | li-4 vs lhu hard-reg swap is global.c allocno_compare priority: const wins by live length 14 vs 15; equalizing lengths flips via ascending-allocno tiebreak | global.c:635-655 (pri = floor_log2(refs)*refs*10000/length; tie -> lower allocno) | moved `var_s1 = 0;` after the range-check if (dead on the return path) | 44 -> 39; slots 6/12/14/27 all match | CONFIRMED |
| H2b | source statement order of the loads changes lreg live length | (refuted) sched1 re-hoists loads and live lengths are recomputed post-sched1 | moved the lhu statement after ret=1 | no change | KILLED |
| H3 | range-check exit is an inline `return 1;` (mixed exit forms), not the shared block_13 | jump2 cross-jump vs distinct exit tails | `goto block_13` -> `return 1;` | 39 -> 37 | CONFIRMED |
| H4 | target recomputes temp_a1_2*2 in-arm; the named local caused the hoist | cse does not extend across the branch when the expression is written per-arm | delete `temp_v0_2`, inline `(temp_a1_2 * 2)` at both uses | 37 -> 30 (also fixed the bne/slt operand diffs downstream) | CONFIRMED |

## Frontier (for session 2)

1. **Tail a0/a1 swap (~6 diffs)** — same shape as the SOLVED a1/a2 swap. Probe: regenerate
   dumps at floor 30, find the two pseudos (the `sll s5,0x4`/addu pointer vs the Judge-table
   `lh` value), compute allocno_compare priorities, look for a semantics-clean statement
   placement that equalizes/flips live lengths (the H2 recipe). Mechanism: global.c
   allocno_compare (global.c:635-655).
2. **Diamond 2 (+4 insns): defeat the cross-jump merge of the second var_v0_2 selection copy**
   (C ~585-599). Read .claude/rules/cross-jump-store-tail-merge.md FIRST (coupled-fixpoint
   warning). Candidate spellings: duplicate the real store+goto differently per arm / invert
   the || range-check so the D_800A38A8 block is the fallthrough (target's beqz sense);
   the v0/v1 rename on the 1/-1 stores (3 diffs) should follow as downstream. Mechanism:
   jump2 find_cross_jump.
3. **s2/s3 prologue rotation (~12 diffs)** — the known-hard arg-home wall (shared with
   func_8008C1E8, brief-shared §5). Do NOT open frontally, BUT: the addu operand-order clue
   (target `addu s3,v0,s0` = X + arg0, ours `addu s2,s0,v0` = arg0 + X at the temp_s3 site)
   is a cheap commutativity respelling probe worth ONE measurement before treating the whole
   cluster as the prologue wall — the rotation may be partly downstream of that spelling.

## [s1] An s32 holder for the raw *(u16*) load (the func_8002872C sibling spelling) makes the & 0xFFFF mask reach RTL and survive as a real andi while the load stays lhu
- mechanism: The old u16 holder let front-end fold-const delete the mask before RTL existed (grep 65535 in .rtl: absent) - the prior sessions' combine/nonzero_bits framing was moot, and P0's u32 conclusion does not generalize. With s32, the multi-use AND survives combine (same-TU sibling func_8002872C proves it in the .combine dump).
- probe: u16 temp_a1 -> s32 temp_a1; sandbox + objdump of the sandbox .o
- result: lhu + andi v1,raw,0xffff emitted matching target's shape; score flat 44 because the a1/a2 hard-reg swap remained
- verdict: CONFIRMED

## [s1] The li-4 vs lhu a1/a2 swap is a global.c allocno_compare priority margin of exactly one insn of live length, and equalizing the lengths flips it via the ascending-allocno tiebreak
- mechanism: global.c:635-655: pri = floor_log2(refs)*refs*10000*size/live_length; pseudo 74 (raw, 3 refs/15) lost to pseudo 81 (const 4, 3 refs/14); tie goes to the lower allocno (74). The var_s1=0 store was the one insn inside 74's range but outside 81's, and block_13 (return) never reads var_s1, so moving the store below the range-check if is semantics-clean.
- probe: Move var_s1 = 0; after the range-check if; sandbox + pairdiff
- result: 44 -> 39; li a2,4 / lhu a1 / andi v1,a1 / addiu v0,a1,-25 all match target
- verdict: CONFIRMED

## [s1] Source statement order of the loads changes lreg live length
- mechanism: sched1 re-hoists loads and reg_live_length is recomputed post-sched1, so load statement order is laundered
- probe: Moved the lhu statement after ret=1
- result: No change in registers or score
- verdict: KILLED

## [s1] The rejection-chain exit is an inline return 1 (mixed exit forms), not the shared block_13 return
- mechanism: jump2 cross-jump merges identical exit tails; a distinct inline return produces target's j + li v0,1 tail. ret is the constant 1 so return 1; is semantics-identical ordinary C (SOTN-sanctioned mixed exit forms).
- probe: goto block_13 -> return 1; at the range-check exit
- result: 39 -> 37, diamond-1 closed
- verdict: CONFIRMED

## [s1] Target recomputes temp_a1_2*2 inside the arm; the named local temp_v0_2 caused our hoisted sll + copy
- mechanism: With the expression written per-use, cse does not extend it across the branch; the named single-def local forced one early sll plus a carrier copy
- probe: Delete s32 temp_v0_2, inline (temp_a1_2 * 2) at both use sites
- result: 37 -> 30; also resolved the bne/slt operand-order diffs downstream
- verdict: CONFIRMED

## Session 2 (2026-08-26, structural) — measured

| id | hypothesis | mechanism | probe | result | verdict |
|---|---|---|---|---|---|
| H5 | The s1 frontier-3 clue (target `addu s3,v0,s0` = X+arg0 vs ours `addu s2,s0,v0` = arg0+X) is a C-level commutativity spelling | operand order of the plus feeds the RTL plus operand order | `temp_s3 = (temp_a1_2 * 2) + arg0`, sandbox + pairdiff | flat 30, emitted `addu s2,s0,v0` UNCHANGED; pointer-arith expansion canonicalizes int+ptr back to plus(ptr,int) | KILLED |
| H6 | Diamond 2's +4 is jump2's cross-jump merging the two identical `var_v0_2 = 0x19/0xB` selection copies | jump.c:2020 chain-partner arm of find_cross_jump; both jumps have JUMP_LABEL == block_48 | per-pass `const_int 25)` census (8 through greg, 6 at jump2) + instrumented cc1 `BB2_XJUMP_DEBUG=1` trace | `DO_CROSS_JUMP jump=362 newjpos=351 newlpos=386` after 3 backward MATCHes; merge localised to jump2 exactly | CONFIRMED |
| H7 | Making ONE copy's tail insns differ defeats find_cross_jump and restores target's insn count | WIN needs min<=0, i.e. >=2 matching insns from the jump backwards; a mismatch on insn 1 aborts | tail copy respelled `if (var_s1 != 0) {0x19} else {0xB}` | 30 -> 28 and **build_insns 211 -> 215 == target exactly**; mirror placement (if/else on the in-range copy) measures identically 28/215 | CONFIRMED |
| H7b | A distinct TERMINATOR on the tail copy (explicit store + `return ret;` instead of `goto block_48`) defeats the pairing without disturbing the selection order | find_cross_jump pairs only jumps sharing a JUMP_LABEL | tail spelled as explicit `*(s16*)(arg0+0x286) = var_v0_2; return ret;` | 29 / 211 insns — jump2 cross-jumps the store+return into block_48 first, sets `next = insn`, and merges the selections on the re-scan | KILLED |
| H8 | Diamond 2 and the s2/s3 rotation are coupled (fixing the insn count would change live lengths and flip the allocation) | reg_live_length is recomputed per pass | re-dumped .lreg/.greg with the 215-insn body | pseudo 73 still 7/92, 143 still 3/14, allocation-order string and every disposition byte-identical. jump2 runs AFTER reload, so it cannot feed back | KILLED |
| H9 | The s2/s3 rotation is a global.c allocno_compare margin; the flip needs live_length(temp_s3) raised from 14 into the window 20..21 | pri = floor_log2(refs)*refs*10000*size/live_length, ties -> lower allocno; MIPS defines no REG_ALLOC_ORDER so find_reg walks hard regs ascending and first-allocated takes the lowest free callee-saved reg. pri(143)=2142 > pri(73)=1521 > pri(75)=1363; 143 must land strictly between 75 and 73 or temp_s4 steals s3 | arithmetic derived from the .lreg ref/length report and the .greg allocation-order string | model reproduces the observed assignment exactly (72->s0, 77->s1, 143->s2, 73->s3, 75->s4, 90->s5, 79->s6); the required window is narrow but decidable | CONFIRMED (model), flip NOT yet achieved |
| H9a | Hoisting the temp_s3 definition above `if (temp_v1_3 == 0)` buys the needed live length | more insns between def and last use | def moved above the branch, sandbox + re-dump | 28 -> 63, 214 insns: cse folds the ==0 arm's own `arg0 + temp_a1_2*2 + 0x288` into temp_s3 (target keeps a separate `addu v0,v0,s0`), pseudo 143 renumbered away, callee-saved map re-shuffles (72 s0->s1, 77 s1->s2) | KILLED |

## Frontier (for session 3)

1. **Finish diamond 2 byte-exactly (8 diffs).** Target has TWO IDENTICAL copies of
   `v0=0x19; bnez s1,block_48; v0=0xB; j block_48` that jump2 did NOT merge. Under the
   traced mechanism identical copies ALWAYS merge once paired, so the original's
   non-merge must be a PAIRING failure, not a comparison failure. Read jump.c:1996-2033:
   the chain-partner loop is guarded by `if (INSN_UID (JUMP_LABEL (insn)) < max_uid)` and
   walks `jump_chain[]` stopping at the first WIN. Probe: re-run
   `BB2_XJUMP_DEBUG=1` on the CANONICAL (both-copies-identical) body, read which
   partners are tried for e1 in what order, and look for a statement placement that
   either pushes block_48's label UID past max_uid or gives e1 an earlier chain partner
   that wins first. Mechanism: jump.c find_cross_jump / jump_chain ordering.
2. **s2/s3 rotation (11 diffs) — H9's window.** Need live_length(pseudo 143, temp_s3)
   = 20 or 21 (currently 14) WITHOUT letting cse merge the `temp_v1_3 == 0` arm's own
   address computation into it (H9a's failure mode). Untried: keep the def where it is
   and push the LAST use later — e.g. sink the `*(s16*)(temp_s3 + 0x288) == 5` test
   below one of the two func_80032854 calls if a semantics-clean ordering exists, or
   find a spelling in which the `== 5` re-read is consumed a few insns later.
   Mechanism: global.c:635-655 allocno_compare.
3. **Tail a0/a1 swap (7 diffs) — not yet modelled.** These are CALLER-saved regs in the
   final block (`sll a0,s5,4`/`addu a0,s4,a0` pointer, `lh a1,Judge(at)`), so
   global.c priority is the wrong lens; this is local-alloc (local-alloc.c qty ordering)
   or reload. Probe: read the .lreg qty report for that block before hypothesising.

## [s2] The s1 frontier-3 clue (target `addu s3,v0,s0` vs ours `addu s2,s0,v0`) is a C-level commutativity spelling of the plus
- mechanism: C operand order of `arg0 + X` vs `X + arg0` feeds the RTL plus operand order and hence the emitted addu operand order
- probe: Respelled `u8 *temp_s3 = (temp_a1_2 * 2) + arg0;`, sandbox --disable all + pairdiff
- result: Flat 30 and the emitted `addu s2,s0,v0` is byte-identical: GCC's pointer-arithmetic expansion canonicalizes int+ptr back to plus(ptr,int). The operand-order diff is downstream of the s2/s3 register assignment, not an independent lever.
- verdict: KILLED

## [s2] Diamond 2's +4-insn gap is jump2's cross-jump pass merging the two identical `var_v0_2 = 0x19/0xB` selection copies (in-range arm and the `!= 5` tail)
- mechanism: jump.c:2020 chain-partner arm of find_cross_jump - both copies' terminating jumps share JUMP_LABEL == block_48, entered with minimum=2, and each backward matching insn decrements it
- probe: Per-pass `const_int 25)` census inside the function region across all -da dumps, then tools/gcc-2.7.2/cc1 with BB2_XJUMP_DEBUG=1
- result: Census: 8 copies through .greg, 6 at .jump2 - the merge is localised to jump2 exactly. Trace: `enter e1=362 e2=397 min=2 (chain-partner)` then three MATCHes (set(reg<-11), set, set(reg<-25)) driving min to -1, then `DO_CROSS_JUMP jump=362 newjpos=351 newlpos=386`.
- verdict: CONFIRMED

## [s2] Making the first backward-compared insn differ between the two copies defeats find_cross_jump and restores the target insn count
- mechanism: find_cross_jump WINs at min<=0, i.e. needs >=2 matching insns walking back from the jump; a PAT-MISMATCH on insn 1 aborts the pairing
- probe: Tail copy respelled `if (var_s1 != 0) { var_v0_2 = 0x19; } else { var_v0_2 = 0xB; }` instead of `var_v0_2 = 0x19; if (var_s1 == 0) var_v0_2 = 0xB;`
- result: Floor 30 -> 28 and build_insns 211 -> 215, exactly the target count. Mirror placement (if/else on the in-range copy, canonical on the tail) measures identically 28/215, so the lever is the asymmetry itself, not which copy carries it.
- verdict: CONFIRMED

## [s2] Giving the tail copy a distinct TERMINATOR (explicit store + return instead of `goto block_48`) defeats the pairing while leaving both copies in target's canonical order
- mechanism: find_cross_jump's chain-partner arm only pairs jumps that share a JUMP_LABEL; a store+return tail has no jump to block_48
- probe: Tail spelled `*(s16 *)(arg0 + 0x286) = var_v0_2; return ret;`
- result: 29 with build_insns back to 211. jump2 cross-jumps the store+return tail INTO block_48 first (restoring `j block_48`), sets `next = insn` and re-scans, then merges the two selections anyway. The terminator lever is self-defeating.
- verdict: KILLED

## [s2] Diamond 2 and the s2/s3 prologue rotation are coupled - correcting the insn count changes live lengths and could flip the callee-saved assignment
- mechanism: reg_live_length is recomputed per pass, so +4 insns inside pseudo 73's live range would lower its allocno priority
- probe: Re-ran pwsh tools/grinder/dump.ps1 with the 215-insn body and re-read .lreg/.greg
- result: Pseudo 73 still `7 refs / 92 insns`, pseudo 143 still `3 refs / 14 insns`, the `;; 25 regs to allocate:` order string byte-identical, every disposition unchanged. jump2 runs AFTER reload so it cannot feed back into allocation. The two clusters are strictly independent.
- verdict: KILLED

## [s2] The s2/s3 rotation is a single global.c allocno_compare decision whose flip requires live_length(temp_s3) to land in the window 20..21
- mechanism: global.c:635-655 pri = floor_log2(n_refs)*n_refs*10000*size/live_length with an ascending-allocno tiebreak; MIPS defines no REG_ALLOC_ORDER (verified absent from tools/gcc-2.7.2) so find_reg walks hard regs ascending and the first-allocated call-crossing pseudo takes the lowest free callee-saved register
- probe: Derived pri from the .lreg ref/length report and checked it against the .greg allocation-order string and dispositions
- result: pri(143 temp_s3)=1*3*10000/14=2142, pri(73 arg1 home)=2*7*10000/92=1521, pri(75 temp_s4)=2*6*10000/88=1363. Model reproduces the observed map exactly (72->s0, 77->s1, 143->s2, 73->s3, 75->s4, 90->s5, 79->s6). For target's map, 143 must sort strictly between 75 and 73, i.e. 30000/L in (1363,1521) => L must be 20 or 21. n_refs(73) cannot honestly reach 8 - arg1 is genuinely referenced exactly 7 times.
- verdict: CONFIRMED

## [s2] Hoisting the temp_s3 definition above `if (temp_v1_3 == 0)` buys the +6/+7 insns of live length the allocno model needs
- mechanism: more insns between the def and the last use raises reg_live_length and lowers allocno priority
- probe: Moved `u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` above the branch, sandbox + re-dump of .lreg/.greg
- result: 28 -> 63 with 214 insns. cse folds the `temp_v1_3 == 0` arm's own `arg0 + temp_a1_2*2 + 0x288` address into temp_s3 (target keeps a SEPARATE `addu v0,v0,s0` there), pseudo 143 is renumbered out of the lreg report entirely, and the callee-saved map re-shuffles (72 s0->s1, 77 s1->s2).
- verdict: KILLED

## [s3] The tail a0/a1 swap is a local-alloc quantity-order decision (NOT global.c allocno_compare), and a priority TIE is enough to flip it
- mechanism: local-alloc.c qty_compare_1 (lines 1659-1685) sorts quantities by pri = floor_log2(n_refs)*n_refs*size/(qty_death-qty_birth)*10000 and breaks ties by ascending qty number; find_free_reg then walks hard regs ascending because MIPS defines no REG_ALLOC_ORDER
- probe: Ran tools/gcc-2.7.2/cc1 with BB2_QTY_DEBUG=1 (harness tmp/grind/func_800283D0/s3/qty.sh) and read the block-41 quantity order
- result: qty0 (temp_a0 pointer, birth 2 death 32 refs 6) pri 4000; qty3 (the Judge[] element, birth 16 death 20 refs 2) pri 5000. qty3 allocates first and takes $a0; qty0 takes $a1. Target is the mirror. Because qty0 < qty3, pri(qty0) >= 5000 suffices: span(qty0) <= 24 (3 fewer insns, from 30) OR span(qty3) >= 6 (one more insn between the `lh Judge` and the `mult`). refs(qty0)=8 would also do it but there is no honest 8th reference.
- verdict: CONFIRMED

## [s3] The declaration-order / local-elimination axis can move the birth or death of the pseudos the two allocation clusters hinge on
- mechanism: naming a value in a local, or not naming it, changes where cse/expand creates the pseudo and hence its birth index and live length
- probe: Six measured variants from the s2 candidate body — A_s3_int (integer-typed plus for temp_s3), B_a0_int (same for temp_a0), D_a0_2_late (declare temp_a0_2/var_a1 after temp_v1_5), E_a0_decl_late (declare temp_a0 after temp_v1_4), F_no_a0_local (delete temp_a0, spell the address at all three loads), N_no_s3_local (delete temp_s3, spell the address at both sites)
- result: ALL SIX measure exactly 28 / 215 insns — byte-neutral. GCC 2.7.2 rebuilds the same pseudo with the same birth via cse + sched1 no matter where the C names the value, and int+ptr canonicalizes back to plus(ptr,int) so the `addu` operand order does not follow the C either. The cheap end of frontier items 2 and 3 is spent: the remaining levers must change the amount of WORK inside the live range, not the declaration site.
- verdict: KILLED

## [s3] Giving the tail var_v0_2 selection its own internal label (two explicit gotos to block_48) prevents jump2 from pairing the two copies while keeping both in target's canonical order
- mechanism: find_cross_jump's chain-partner arm (jump.c:2012-2021) walks jump_chain[] for jumps that share a JUMP_LABEL; an extra intermediate label was expected to change the chain membership or the branch senses
- probe: M_tail_two_gotos — `if (var_s1 == 0) goto tail_0B; var_v0_2 = 0x19; goto block_48; tail_0B: var_v0_2 = 0xB; goto block_48;`
- result: 28 / 215 and the pairdiff HUNK SET is byte-identical to the base body's (23 hunks, same slots). jump.c threads the intermediate label away before the cross-jump phase and rebuilds exactly the same two-simplejump shape, so the pairing is attempted identically. Together with s2's KILLED distinct-terminator probe, both "make the copies structurally different at the jump" ideas are spent.
- verdict: KILLED

## [s3] Spelling the tail selection as a ternary `var_v0_2 = (var_s1 != 0) ? 0x19 : 0xB;` produces different RTL from the if/else and might emit target's canonical constant order
- mechanism: C-level conditional-expression expansion vs if-statement expansion
- probe: L_ternary_tail, measured with sandbox --disable all
- result: 28 / 215 — identical score to the if/else form. GCC 2.7.2 lowers `?:` on two integer constants through the same jump-around path as the if/else, so it is the same construct in a different skin.
- verdict: KILLED

## [s3] Swapping the multiply operand orders inside temp_v1_5, or sinking the temp_a0_2 abs below the temp_v1_5 abs, are free re-associations
- mechanism: source operand order feeds RTL operand order and hence emission order; statement order between two independent abs computations was assumed free
- probe: C_mul_swap (field-load first, Judge[] second in both products) and G_abs_late (`if (temp_a0_2 < 0) ...` moved below `var_v0_3 = temp_v1_5;`)
- result: C_mul_swap 29 / 215 (target emits the `lh Judge` before the `lw 0x114`, so our base order is already right). G_abs_late 31 / 216 — it costs a WHOLE INSTRUCTION, because reorg loses the shared bgez/negu layout between the two abs computations. Both banked in memory/grind/func_800283D0/rejected/. The relative order of the two abs computations is load-bearing, not free.
- verdict: KILLED

## [s3] qty0's span reaches 32 only because var_a1's pseudo is fused onto the pointer's quantity by the `var_a1 = temp_a0_2;` copy; removing the copy should drop the death to the `lw 0x118` and win the tie
- mechanism: local-alloc.c combine_regs fuses the dest and source quantities of a reg-to-reg copy, extending qty_death of the fused quantity to the copy's last use
- probe: P_var_a1_ifelse - `s32 var_a1;` declared uninitialised and both arms written explicitly (`if (temp_a0_2 < 0) { var_a1 = -temp_a0_2; } else { var_a1 = temp_a0_2; }`), then sandbox plus a fresh BB2_QTY_DEBUG=1 trace
- result: 28 / 215 and the block-41 quantity table is BYTE-IDENTICAL (qty0 birth 2 death 32 refs 6 -> $5; qty3 birth 16 death 20 refs 2 -> $4). GCC's if-conversion/expand rebuilds the same `move` insn from the two-arm form, so the copy - and therefore the fusion - survives. The 'break the var_a1 copy' axis is dead; qty0's span must be attacked some other way.
- verdict: KILLED

## [s3] The tail a0/a1 swap (7 diffs) is a local-alloc quantity-order decision, not a global.c allocno_compare one, and a priority TIE is enough to flip it.
- mechanism: local-alloc.c qty_compare_1 (lines 1659-1685) sorts quantities by pri = floor_log2(n_refs)*n_refs*size/(qty_death-qty_birth)*10000 and breaks ties by ASCENDING QTY NUMBER (`return *q1 - *q2;`); find_free_reg then walks hard regs ascending because MIPS defines no REG_ALLOC_ORDER.
- probe: Ran tools/gcc-2.7.2/cc1 with BB2_QTY_DEBUG=1 (harness tmp/grind/func_800283D0/s3/qty.sh, trace tmp/grind/func_800283D0/s3/qty_trace.txt) and read the block-41 quantity order, then re-derived the priorities by hand from the printed birth/death/refs.
- result: qty0 (temp_a0 pointer, birth 2 death 32 refs 6) pri = 2*6/30*10000 = 4000; qty3 (the Judge[] element, birth 16 death 20 refs 2) pri = 1*2/4*10000 = 5000. qty3 allocates first and takes $a0 (regs 2 and 3 are already held by qty2/qty1), qty0 takes $a1. Target is the exact mirror (asm/funcs/func_800283D0.s:175-199). Because qty0 < qty3, pri(qty0) >= 5000 SUFFICES: span(qty0) <= 24 (3 fewer insns, from 30) OR span(qty3) >= 6 (one more insn between the `lh Judge` and the `mult`). refs(qty0)=8 would also do it but there is no honest 8th reference. Note qty0 is a COMBINED quantity (sll producer + temp_a0 + var_a1), which is why the .lreg summary line '4 times across 14 insns' understates it.
- verdict: CONFIRMED

## [s3] The declaration-order / local-elimination axis can move the birth or death of the pseudos the two allocation clusters (s2/s3 rotation, tail a0/a1) hinge on.
- mechanism: Naming a value in a local, or not naming it, changes where cse/expand creates the pseudo and hence its birth index and live length; and C-level operand order of a pointer plus was expected to feed the emitted addu operand order.
- probe: Six variants measured with `sandbox --disable all` from the s2 candidate body, all banked in tmp/grind/func_800283D0/s3/v/: A_s3_int (integer-typed plus for temp_s3), B_a0_int (same for temp_a0), D_a0_2_late (declare temp_a0_2/var_a1 after temp_v1_5), E_a0_decl_late (declare temp_a0 after temp_v1_4), F_no_a0_local (delete temp_a0, spell temp_s4 + temp_s5*0x10 + off at all three loads), N_no_s3_local (delete temp_s3, spell arg0 + temp_a1_2*2 + 0x288 at both sites).
- result: ALL SIX measure exactly 28 / 215 insns - byte-neutral. GCC 2.7.2 rebuilds the same pseudo with the same birth via cse + sched1 regardless of where the C names the value, and int+ptr canonicalizes back to plus(ptr,int) so the addu operand order does not follow the C either. The cheap end of frontier items 2 and 3 is spent: the remaining levers must change the amount of WORK inside the live range, not the declaration site.
- verdict: KILLED

## [s3] Giving the tail var_v0_2 selection its own internal label (two explicit gotos to block_48) prevents jump2 from pairing the two selection copies while leaving both in target's canonical `v=0x19; if(!s1) v=0xB;` order.
- mechanism: find_cross_jump's chain-partner arm (jump.c:2012-2021) only pairs jumps that share a JUMP_LABEL and walks jump_chain[] stopping at the first WIN; an extra intermediate label was expected to change chain membership or the branch senses.
- probe: M_tail_two_gotos - tail spelled `if (var_s1 == 0) goto tail_0B; var_v0_2 = 0x19; goto block_48; tail_0B: var_v0_2 = 0xB; goto block_48;`. Measured with sandbox and then diffed the pairdiff hunk set against the base body's (tmp/grind/func_800283D0/s3/pd_M.txt vs pd_start.txt).
- result: 28 / 215 and the pairdiff HUNK SET is byte-identical to the base body's (23 hunks, same slots). jump.c threads the intermediate label away before the cross-jump phase and rebuilds exactly the same two-simplejump shape, so the pairing is attempted identically. Together with s2's KILLED distinct-terminator probe, both 'make the two copies structurally different at the jump' ideas are now spent; only the UID / jump_chain-order route survives.
- verdict: KILLED

## [s3] Spelling the tail selection as a ternary `var_v0_2 = (var_s1 != 0) ? 0x19 : 0xB;` produces different RTL from the if/else and may emit target's canonical constant order while still defeating the merge.
- mechanism: C-level conditional-expression expansion vs if-statement expansion.
- probe: L_ternary_tail, measured with sandbox --disable all.
- result: 28 / 215 - identical score to the if/else form. GCC 2.7.2 lowers `?:` over two integer constants through the same jump-around path as the if/else, so it is the same construct in a different skin.
- verdict: KILLED

## [s3] Swapping the multiply operand orders inside temp_v1_5, and sinking the temp_a0_2 abs below the temp_v1_5 abs, are free re-associations that can shift the tail register assignment at no cost.
- mechanism: Source operand order feeds RTL operand order and hence emission order; the two abs computations are independent so their statement order was assumed free.
- probe: C_mul_swap (field-load first, Judge[] second in both products) and G_abs_late (`if (temp_a0_2 < 0) var_a1 = -temp_a0_2;` moved below `var_v0_3 = temp_v1_5;`).
- result: C_mul_swap 29 / 215 - target emits the `lh Judge` before the `lw 0x114`, so the base order is already right. G_abs_late 31 / 216 - it costs a WHOLE INSTRUCTION because reorg loses the shared bgez/negu layout between the two abs computations. Both banked in memory/grind/func_800283D0/rejected/. The relative order of the two abs computations is load-bearing, not free.
- verdict: KILLED

## [s3] qty0's span reaches 32 only because var_a1's pseudo is fused onto the temp_a0 pointer's quantity by the `var_a1 = temp_a0_2;` copy; removing the copy drops qty0's death to the `lw 0x118` and wins the priority tie.
- mechanism: local-alloc.c combine_regs fuses the dest and source quantities of a reg-to-reg copy, extending qty_death of the fused quantity to the copy's last use.
- probe: P_var_a1_ifelse - `s32 var_a1;` declared uninitialised with both arms written explicitly (`if (temp_a0_2 < 0) { var_a1 = -temp_a0_2; } else { var_a1 = temp_a0_2; }`), measured with sandbox and re-traced with BB2_QTY_DEBUG=1.
- result: 28 / 215 and the block-41 quantity table is BYTE-IDENTICAL to the base body's (qty0 birth 2 death 32 refs 6 -> $5; qty3 birth 16 death 20 refs 2 -> $4). GCC rebuilds the same `move` insn from the two-arm form, so the copy and therefore the fusion survive.
- verdict: KILLED

## Session 4 (2026-08-27, permuter) — measured

| id | hypothesis | mechanism | probe | result | verdict |
|---|---|---|---|---|---|
| H10 | decomp-permuter on the s2/s3 candidate chassis can find an ordinary-C respelling below 28 | random source perturbation over the whole body | 4 campaigns (perm_a canonical chassis, perm_b mirror chassis, perm_c do-while(0) chassis, perm_d chained) — ~20k iterations total, all harvested + stopped | ONE semantically-valid improving find: a `do {} while (0)` wrap, 28 -> 26. Every other find was byte-neutral or semantically invalid | CONFIRMED (with a caveat that makes it unusable — see H11) |
| H11 | The `do { } while (0)` wrap that scores 26 works through the sanctioned reorg.c / LABEL_OUTSIDE_LOOP_P interaction | `.claude/rules/do-while-zero-exception.md` scope | re-dumped .lreg with the wrapped body and diffed the per-pseudo ref/length report against tmp/grind/func_800283D0/s4/lreg_base28.txt | REFUTED as to mechanism: EVERY live length is byte-identical (72:155, 73:92, 75:88, 77:73, 90:32, 143:14) and only REG_N_REFS moves, by exactly +1 per reference sited inside the wrapped region (72: 19->24, 73: 7->9, 75: 6->7, 77: 9->11, 90: 3->4, 128: 4->5) = flow.c `REG_N_REFS (regno) += loop_depth`. That is a global.c allocno-priority lever, OUT OF SCOPE for the carve-out | KILLED (as a usable construct) |
| H12 | The 22-scoring permuter find (temp_s3 reused as an `arg0 + 0x286` store pointer) is a legitimate variable-reuse spelling | local-alloc / global.c ref+span effects of an extra pointer def | read the body against the CFG, then checked target's store form | SEMANTICALLY INVALID (block_48 is reached on paths where temp_s3 still holds `arg0 + temp_a1_2*2`, so the store lands at the wrong address) AND directionally wrong (target emits `sh $v0, 0x286($s0)` at all three sites, asm/funcs/func_800283D0.s:92,162,222 — never register-indirect) | KILLED |
| H13 | The permuter's `if (temp_a0_2 > (var_s1 = 0))` dead-store-in-condition contributes real distance | dead store to a local perturbing liveness | ablation: v_nodeadstore.c (same body, condition restored to `> 0`) | 22 — IDENTICAL. The construct contributes EXACTLY ZERO; it is pure permuter noise | KILLED |

## [s4] decomp-permuter can find a semantically-valid ordinary-C respelling of the candidate body that scores below 28
- mechanism: random source perturbation (decomp-permuter) over four chassis, scored by its own weighted metric and then RE-MEASURED with `sandbox --disable all`, because the permuter metric and the engine distance disagree freely.
- probe: Four campaigns, all launched and harvested through tools/permuter_campaign.py with telemetry: perm_a (label s4a-canonical-chassis, the candidate body), perm_b (s4b-mirror-chassis, the if/else spelled on the in-range copy instead of the tail copy), perm_c (s4c-dowhile0-chassis-floor26, seeded from perm_a's find), perm_d (s4d-chain-floor22, seeded from perm_c's find). ~20k iterations total (perm_c 8859, perm_d 7000). Every novel find was re-measured with sandbox --disable all.
- result: Exactly ONE semantically-valid improving find in ~20k iterations: the `do {} while (0)` wrap (28 -> 26 at 215 insns). Everything else was either byte-neutral against the engine metric despite a lower permuter score (perm_b's `arg0 - -(temp_a1_2*2)` double negation: permuter 468 -> 458, sandbox 28 -> 28) or semantically invalid (perm_c's 22 and perm_d's 400-score find both mis-target the block_48 store). CONCLUSION FOR LATER SESSIONS: this function's residual is NOT permuter-shaped. All of the remaining distance is callee-saved register-assignment permutation, and the permuter's local source edits reach that only through constructs that are out-of-family or semantically wrong. A further permuter session on this chassis is not a good use of a modality slot.
- verdict: CONFIRMED

## [s4] The `do { ... } while (0)` wrap that scores 26 works through the sanctioned reorg.c / LABEL_OUTSIDE_LOOP_P interaction, so it sits inside the do-while-zero carve-out
- mechanism: `.claude/rules/do-while-zero-exception.md` sanctions the wrap for the LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY.
- probe: Applied the wrapped body (`do { ... } while (0);` around the region from `if (temp_v1_3 < temp_v0_3)` through `block_48: *(s16*)(arg0+0x286) = var_v0_2;`), ran `pwsh tools/grinder/dump.ps1 func_800283D0`, and diffed the .lreg per-pseudo "used N times across M insns" report against the pre-wrap snapshot tmp/grind/func_800283D0/s4/lreg_base28.txt.
- result: REFUTED as to mechanism. Every live length is byte-identical (72:155, 73:92, 75:88, 77:73, 90:32, 128:7, 143:14). Only REG_N_REFS moves, and it moves by exactly +1 per reference sited inside the wrapped region: 72 19->24, 73 7->9, 75 6->7, 77 9->11, 90 3->4, 128 4->5. That is flow.c's loop-depth ref weighting (`REG_N_REFS (regno) += loop_depth`, loop_depth 1 -> 2 inside the NOTE_INSN_LOOP_BEG/END pair the do-while leaves behind), feeding global.c's allocno_compare priority — NOT the reorg.c label interaction. So the construct would be a FIRST REACH of an unsanctioned mechanism wearing a sanctioned family's syntax, and the no-new-park-categories non-extension clause forbids generalizing the carve-out to it. It is also not a match (26, not 0) and s4 has no modality-ladder exhaustion, which a FAKE construct independently requires. Banked NOT adopted: memory/grind/func_800283D0/rejected/dowhile0-refweight-out-of-scope.c.
- verdict: KILLED (as a usable construct); the mechanism measurement itself is CONFIRMED and is the session's main asset.

## [s4] The callee-saved cluster is a single coupled permutation whose lever is REG_N_REFS, not only reg_live_length
- mechanism: global.c:635-655 allocno_compare pri = floor_log2(n_refs)*n_refs*10000*size/live_length. s2's H9 explored only the live_length denominator; the wrap experiment moves the numerator with live_length pinned, which isolates the n_refs axis cleanly for the first time.
- probe: pairdiff of the wrapped (26) body vs target, compared hunk-for-hunk against the s3 base pairdiff (tmp/grind/func_800283D0/s3/pd_start.txt vs tmp/grind/func_800283D0/s4/pd_dowhile.txt).
- result: The prologue hunks ours[3:5] and ours[10:11] — the arg1-home s2/s3 rotation the ledger has chased since s2 — DISAPPEAR entirely: our `sw s3,36(sp)` / `move s3,a1` become target's `sw s2,32(sp)` / `move s2,a1`. But the win is paid for elsewhere: temp_s4's pseudo lands in $s5 (target $s4) and temp_s5's in $s3 (target $s5), creating six NEW hunks at ours[13:14], [20:21], [52:53], [59:60], [69:71], [121:122]. Net 28 -> 26. So the seven callee-saved pseudos are allocated as ONE ordered permutation, and any lever that reorders the priority list trades one cluster for another rather than adding a partial fix. The honest search is therefore NOT "find a live_length window" (s2 and s3 spent that axis) but "find an ordinary-C restructure that changes the REFERENCE COUNT of pseudo 73 (arg1 home, 7 refs, pri 1521) or pseudo 75 (temp_s4, 6 refs, pri 1363)" — the adjacent pair in the priority order — while leaving the other five pseudos' priorities where they are.
- verdict: CONFIRMED

## [s4] The 22-scoring permuter find (temp_s3 reused as an `arg0 + 0x286` store pointer) is a legitimate lower-floor body
- mechanism: an extra pointer def inside the tail region changes both the ref count and the span of pseudo 143 and of the arg0 pointer.
- probe: Read the body against the CFG; ablated the three constructs separately (tmp/grind/func_800283D0/s4/v_nowrap.c = 25, v_nodeadstore.c = 22, v_nos3reuse.c = 26).
- result: KILLED on SEMANTICS before any family question arises. The find assigns `temp_s3 = arg0 + 0x286;` inside the `temp_v1_3 < temp_v0_3 && var_s1 != 0` arm and then spells the block_48 store as `*(s16 *)temp_s3 = var_v0_2;`, but block_48 is also reached from the `temp_v1_3 >= temp_v0_3` (var_v0_2 = 0x1A) path where temp_s3 still holds `arg0 + temp_a1_2 * 2` — the store would land at the wrong address. decomp-permuter's randomizer does not guarantee semantic equivalence; every find must be read against the CFG before it is measured for meaning. It is ALSO directionally wrong: the target emits `sh $v0, 0x286($s0)` at all three store sites (asm/funcs/func_800283D0.s:92, 162, 222), i.e. base+offset off the arg0 pointer, never register-indirect — so no store-address pointer local can ever be the answer here, whatever it does to the allocator. The ablation is still useful and is banked: the do-while wrap contributes 3 (25 -> 22), the store-pointer reuse contributes 4 (26 -> 22), and the `if (temp_a0_2 > (var_s1 = 0))` dead-store-in-condition contributes EXACTLY 0.
- verdict: KILLED

## Frontier (for session 5)

1. **REG_N_REFS is the live axis for the callee-saved permutation.** The s2/s3 rotation
   and the temp_s4/temp_s5 assignment are ONE ordered permutation over seven pseudos, and
   the wrap experiment proves the order moves on ref counts with live lengths pinned.
   Adjacent pair in the priority order: pseudo 73 (arg1 home, 7 refs / 92 insns, pri 1521)
   and pseudo 75 (temp_s4, 6 refs / 88 insns, pri 1363). Probe: enumerate ordinary-C
   restructures that legitimately change one of those two ref counts by one — e.g. reading
   `*(u8 **)(arg0)` a second time at a site where target plausibly re-loads it instead of
   keeping temp_s4 live, or the reverse (hoisting a temp_s4 use so one reference is folded
   away). Re-dump .lreg after each and check the printed ref count actually moved before
   reading the score. Mechanism: flow.c REG_N_REFS + global.c:635-655 allocno_compare.
2. **Do NOT spend another permuter slot on this chassis.** ~20k iterations across four
   chassis produced exactly one semantically-valid improving find and it is out-of-family.
   The residual is register-assignment permutation, which the permuter's local source edits
   cannot address honestly.
3. **Diamond 2's last 8 diffs** (unchanged from s3): both selection copies must be in
   target's canonical `v=0x19; if(!s1) v=0xB;` order AND unmerged, which needs the jump2
   pairing never to be ATTEMPTED. Next probe unchanged: instrument xjump.sh to also print
   INSN_UID(JUMP_LABEL) and max_uid on the canonical both-copies-identical body.

## [s4] decomp-permuter on the func_800283D0 candidate chassis can find a semantically-valid ordinary-C respelling that scores below the 28 floor.
- mechanism: Random source perturbation (decomp-permuter) over four chassis, scored by the permuter's own weighted metric and then RE-MEASURED with `sandbox --disable all`, because the two metrics disagree freely.
- probe: Four campaigns launched and harvested+stopped in-session via tools/permuter_campaign.py: perm_a (s4a-canonical-chassis, the candidate body), perm_b (s4b-mirror-chassis, if/else spelled on the in-range copy instead of the tail copy), perm_c (s4c-dowhile0-chassis-floor26, seeded from perm_a's find, 8859 iterations), perm_d (s4d-chain-floor22, seeded from perm_c's find, 7000 iterations). ~20k iterations total; every novel find re-measured with sandbox and read against the CFG.
- result: Exactly ONE semantically-valid improving find: a `do { } while (0);` wrap around the block_48 region, 28 -> 26 at 215 insns. perm_b's find (`arg0 - -(temp_a1_2*2)`) improved the permuter score 468->458 but was byte-neutral at sandbox 28. perm_c's 22 and perm_d's find are both semantically INVALID (they mis-target the block_48 store). Conclusion: this function's residual is not permuter-shaped - all remaining distance is callee-saved register-assignment permutation, which the permuter's local source edits reach only through out-of-family or wrong constructs.
- verdict: CONFIRMED

## [s4] The `do { } while (0)` wrap that scores 26 works through the LABEL_OUTSIDE_LOOP_P / reorg.c interaction, i.e. it sits inside the sanctioned do-while-zero carve-out.
- mechanism: .claude/rules/do-while-zero-exception.md scopes the wrap to the LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY.
- probe: Applied the wrapped body, ran `pwsh tools/grinder/dump.ps1 func_800283D0`, and diffed the .lreg per-pseudo 'used N times across M insns' report against the pre-wrap snapshot tmp/grind/func_800283D0/s4/lreg_base28.txt.
- result: REFUTED as to mechanism. Every live length is byte-identical (72:155, 73:92, 75:88, 77:73, 90:32, 128:7, 143:14) and only REG_N_REFS moves, by exactly +1 per reference sited inside the wrapped region: 72 19->24, 73 7->9, 75 6->7, 77 9->11, 90 3->4, 128 4->5. That is flow.c's loop-depth ref weighting (REG_N_REFS (regno) += loop_depth; the do-while leaves a NOTE_INSN_LOOP_BEG/END pair so loop_depth is 2 inside) feeding global.c:635-655 allocno_compare - a global.c allocno-priority lever, NOT the reorg.c label interaction. So adopting it would be a first reach of an unsanctioned mechanism wearing a sanctioned family's syntax, which the no-new-park-categories non-extension clause forbids. It is also not a match (26, not 0) and a FAKE construct independently requires demonstrated modality-ladder exhaustion, which four sessions in does not have. Banked NOT adopted; src restored to the 28 body.
- verdict: KILLED

## [s4] The 22-scoring permuter find (temp_s3 reused as an `arg0 + 0x286` store pointer) is a legitimate lower-floor body.
- mechanism: An extra pointer def inside the tail region changes both the ref count and the span of pseudo 143 and of the arg0 pointer.
- probe: Read the body against the CFG; checked target's store form in asm/funcs/func_800283D0.s; ablated the three constructs separately (tmp/grind/func_800283D0/s4/v_nowrap.c, v_nodeadstore.c, v_nos3reuse.c).
- result: KILLED on semantics before any family question arises. It assigns `temp_s3 = arg0 + 0x286;` inside the `temp_v1_3 < temp_v0_3 && var_s1 != 0` arm and spells the block_48 store as `*(s16 *)temp_s3 = var_v0_2;`, but block_48 is also reached from the `temp_v1_3 >= temp_v0_3` (var_v0_2 = 0x1A) path where temp_s3 still holds `arg0 + temp_a1_2 * 2` - the store would land at the wrong address. ALSO directionally impossible: target emits `sh $v0, 0x286($s0)` at all three store sites (asm/funcs/func_800283D0.s:92, 162, 222), base+offset off arg0, never register-indirect, so no store-address pointer local can ever be the answer here. Ablation banked: the wrap is worth 3 (25->22), the invalid reuse 4 (26->22).
- verdict: KILLED

## [s4] The permuter's `if (temp_a0_2 > (var_s1 = 0))` dead-store-in-condition contributes real distance.
- mechanism: A dead store to a local perturbing liveness / ref counts around the tail comparison.
- probe: Ablation variant tmp/grind/func_800283D0/s4/v_nodeadstore.c - same body with the condition restored to `temp_a0_2 > 0` - measured with sandbox --disable all.
- result: 22, IDENTICAL to the body that carries it. The construct contributes EXACTLY ZERO; it is pure permuter noise. Dead-store-in-condition is dead as a lever for this function and should not be re-proposed (it would also have needed the dead-store FAKE family for nothing).
- verdict: KILLED

## [s4] The callee-saved cluster's assignment is driven by REG_N_REFS as well as reg_live_length, and the seven pseudos are allocated as one coupled permutation rather than independently fixable clusters.
- mechanism: global.c:635-655 allocno_compare, pri = floor_log2(n_refs)*n_refs*10000*size/live_length. Sessions 2-3 explored only the live_length denominator; the do-while experiment moves the numerator with every live length pinned, isolating the n_refs axis for the first time.
- probe: pairdiff of the 26 body vs target (tmp/grind/func_800283D0/s4/pd_dowhile.txt) compared hunk-for-hunk against the s3 base pairdiff (tmp/grind/func_800283D0/s3/pd_start.txt).
- result: The prologue hunks ours[3:5] and ours[10:11] - the arg1-home s2/s3 rotation chased since session 2 - DISAPPEAR (our `sw s3,36(sp)` / `move s3,a1` become target's `sw s2,32(sp)` / `move s2,a1`). But temp_s4's pseudo moves to $s5 (target $s4) and temp_s5's to $s3 (target $s5), creating six new hunks at ours[13:14], [20:21], [52:53], [59:60], [69:71], [121:122]. Net 28 -> 26. The order therefore moved purely on ref counts, and any priority-list reorder trades one cluster for another instead of fixing part of it. Session 2's H9 arithmetic ('n_refs(73) cannot honestly reach 8') is correct but was the wrong search: the adjacent pair in the priority order is pseudo 73 (arg1 home, 7 refs, pri 1521) and pseudo 75 (temp_s4, 6 refs, pri 1363), and a one-reference honest change to either is the live question.
- verdict: CONFIRMED

## Session 5 (2026-08-26, synthesis) — measured

| id | hypothesis | mechanism | probe | result | verdict |
|---|---|---|---|---|---|
| H14 | The hand-derived allocno_compare model (s2/s3) is exact, and the callee-saved map is a pure function of the priority order (no preference term) | global.c:605 post-sort table; find_reg pass-0 masks `regs_used_so_far` and `regs_someone_prefers`, pass 1 walks ascending; copy/full preferences (global.c:1096-1163) can override best_reg | `BB2_ALLOC_DEBUG=1` on tools/gcc-2.7.2/cc1 (s5/alloc.sh, trace s5/alloc_trace.txt) + read of the seed set | Table matches every hand-derived number exactly. Seed excludes $s0-$s7 so all callee-saved assignment is pass-1 ascending; no callee-saved hard reg appears in pre-reload RTL so every preference set is empty | CONFIRMED |
| H15 | The reference counts feeding the callee-saved priorities can be moved by ordinary C (the s4 frontier's route (a)/(b)) | REG_N_REFS is the numerator of allocno_compare's pri | counted register references directly in asm/funcs/func_800283D0.s: $s2 7, $s3 3, $s4 6, $s5 3 | IDENTICAL to our pseudos 73/143/75/90. The original fed allocno_compare the same numerators, so any ref-count lever moves AWAY from the original's RTL (exactly what the s4 do-while wrap did) | KILLED |
| H16 | The s2/s3 flip requires live_length(143) in the narrow window {20,21} (s2 H9) | pri(143)=30000/L143 must land strictly between pri(75)=1363 and pri(73)=1521 | re-derivation noting that insns inside 143's range also lengthen 75's range but NOT 73's (arg1 dies at the equal-arm's second call), then a measured liveness dial (s5/dial.sh, n=1..6) | SUPERSEDED, window is wider: k=6/8/10 added insns all FLIP to target's map (73->$s2, 143->$s3, 75->$s4); k=2/4 too few, k=12 breaks it (75 overtakes). Correct constraint: L143 >= 20 AND L143 < (88+k)/4 | CONFIRMED (model); flip achievable only with 6-10 REAL insns |
| H17 | Some ordinary-C spelling can supply those 6-10 insns of liveness | live length is accumulated over every insn where the pseudo is live | target's own asm has NOTHING between the equal-arm's second call and the `== 5` load (lines 113-122), and $s3 is referenced exactly 3 times, all on the equal path — so no use is reachable from the not-equal arm to buy liveness for free | KILLED for the current whole-function RTL: the cluster is foreclosed under spelling and becomes a forensics question (where does our RTL still differ from the original's) |
| H18 | Deleting the shared `block_48` label and writing the store+return into each selection arm makes jump2 CREATE the shared block itself, giving its label a UID >= max_uid so the two selection blocks never enter jump_chain as partners | jump.c:2012-2021 guards the chain-partner loop with `INSN_UID (JUMP_LABEL (insn)) < max_uid`; jump_chain is only populated for pre-existing UIDs | both copies respelled canonical + own `*(s16 *)(arg0 + 0x286) = var_v0_2; return ret;`, no label anywhere (s5/v_nolabel.c) | 30 / 211 insns — jump2 cross-jumps the store+return TAILS first, rebuilds the shared block, and merges the selections on the rescan (same failure mode as s2's H7b). Banked rejected/diamond2-no-shared-label-remerges.c | KILLED |

## Frontier (for session 6)

1. **The s2/s3 rotation is foreclosed under SPELLING; treat it as a forensics target.**
   Mechanism and window are now exact and instrumented: the flip needs 6-10 insns of extra
   liveness strictly inside pseudo 143's range and outside pseudo 73's, and ref counts are
   pinned by target's own asm. Next probe: instead of proposing another spelling, use
   `s5/dial.sh` in reverse as a WINDOW MAP — insert the 2-insn filler at each candidate site in
   the function and record which pseudos' live lengths it moves — then look for a site whose
   liveness signature is "143 only". If no such site exists (expected), the residual is
   evidence that our whole-function RTL differs from the original's, and the next modality
   should be forensics/rederive on the regions we have never questioned (the rejection chain,
   the block_15 shift/mask computation), not another allocation probe.
2. **Diamond 2 needs compiler instrumentation a grind session may not write.** All four
   source-structural routes are dead (distinct terminator, two goto labels, ternary,
   no-shared-label). The remaining question — is block_48's label UID below `max_uid`, and which
   partners does `jump_chain` offer for e1 — requires a print inside `tools/gcc-2.7.2/jump.c`,
   outside the allowed surface. Route as an operator instrumentation request; do not spend
   another session on selection-copy spellings.
3. **The tail a0/a1 quantity cluster (7 diffs) is untouched since s3** and is the only cluster
   whose model has never been dialled. Requirement: pri(qty0) >= 5000, i.e. span(qty0) <= 24
   (from 30) or span(qty3) >= 6 (from 4). Build the local-alloc analogue of s5/dial.sh
   (`BB2_QTY_DEBUG=1`, harness s3/qty.sh) and dial filler insns between the `lh Judge($at)` and
   the `mult` to confirm the window empirically before proposing any C — the s5 experience is
   that a hand-derived window can be wrong once the co-moving denominators are accounted for.

## [s5] The hand-derived allocno_compare model carried since s2 is exact, and the callee-saved register map is a pure function of the priority ORDER (no preference term participates).
- mechanism: global.c:605 prints the post-sort allocno table; find_reg (global.c:1052-1163) masks pass 0 with IOR_COMPL_HARD_REG_SET(used, regs_used_so_far) plus regs_someone_prefers, then walks hard regs ascending in pass 1 (MIPS defines no REG_ALLOC_ORDER), and can afterwards override best_reg from hard_reg_copy_preferences / hard_reg_preferences.
- probe: BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1 via tmp/grind/func_800283D0/s5/alloc.sh, trace banked at s5/alloc_trace.txt; plus a read of find_reg's pass structure and of the ALLOCDBG seed set.
- result: Table reproduces every hand-derived number exactly: 72(19 refs/155/pri 4903)->$s0, 77(9/73/3698)->$s1, 143(3/14/2142)->$s2, 73(7/92/1521)->$s3, 75(6/88/1363)->$s4, 90(3/32/937)->$s5, 79(6/322/372)->$s6. seed_used excludes $s0-$s7, so all callee-saved assignment happens in pass 1 ascending; no callee-saved hard register appears in pre-reload RTL so every preference set is empty and the preference axis is inert here.
- verdict: CONFIRMED

## [s5] The reference counts feeding the callee-saved priorities are a workable ordinary-C axis (the s4 frontier's routes (a) and (b): re-read *(u8**)(arg0) at a later temp_s4 site, or the mirror).
- mechanism: REG_N_REFS is the numerator of allocno_compare's priority, so +/-1 reference moves a pseudo in the sorted order.
- probe: Counted register references directly in asm/funcs/func_800283D0.s: $s2 def line 6 + uses 95,100,113,118,164,169 = 7; $s3 def 107 + uses 108,122 = 3; $s4 def 15 + uses 22,57,77,132,177 = 6; $s5 def 64 + uses 76,175 = 3.
- result: Target's own reference counts are IDENTICAL to our pseudos 73/143/75/90. The original compile fed allocno_compare the same numerators, so any construct that moves a reference count moves away from the original's RTL - which is precisely what the s4 do-while(0) wrap did (fixed the arg1 home, broke temp_s4/temp_s5).
- verdict: KILLED

## [s5] The s2/s3 flip requires live_length(pseudo 143) in the narrow window {20,21} (s2's H9).
- mechanism: pri = floor_log2(n_refs)*n_refs*10000*size/live_length; pri(143)=30000/L143 must land strictly between pri(75) and pri(73), ties going to the lower allocno.
- probe: Re-derived noting that real insns added inside 143's range also lengthen 75's range (temp_s4 spans nearly the whole function) but NOT 73's (arg1 is dead after the equal-arm's second func_80032854 call), then measured with a liveness dial: tmp/grind/func_800283D0/s5/dial.sh inserts n 2-insn stores to distinct arg0-relative offsets immediately before the `== 5` test and re-dumps the ALLOCDBG table for n=1..6.
- result: Window is WIDER than recorded. k=2 (L143=16) and k=4 (18) leave 143 in $s2; k=6 (20), k=8 (22) and k=10 (24) all FLIP to target's map (73->$s2, 143->$s3, 75->$s4); k=12 (26) breaks it as temp_s4 overtakes. Correct constraint: L143 >= 20 AND L143 < (88+k)/4. The model predicted k in {6,8,10} before the dial ran and the dial reproduced both ends exactly.
- verdict: CONFIRMED

## [s5] Some ordinary-C spelling can supply the 6-10 insns of extra liveness the flip needs.
- mechanism: reg_live_length is accumulated over every insn in every block where the pseudo is live, so either real instructions inside the range or a use reachable from a sibling arm would buy it.
- probe: Read target's emitted code between the equal-arm's second call and the `== 5` load (asm/funcs/func_800283D0.s:113-122) and counted every $s3 reference in the whole function.
- result: Target has NOTHING between those two points, so there is no honest place for 6-10 instructions in a build that already sits at exactly 215 == 215; and $s3 is referenced exactly 3 times, all on the equal path, so no byte-neutral spelling can make pseudo 143 live-out along the not-equal arm (the only zero-instruction liveness mechanism).
- verdict: KILLED

## [s5] Deleting the shared block_48 label and writing the 0x286 store + return into each selection arm defeats the jump2 pairing while keeping both copies in target's canonical order.
- mechanism: jump.c:2012-2021 guards the chain-partner loop with `INSN_UID (JUMP_LABEL (insn)) < max_uid`, and jump_chain is populated only for jumps/labels whose UIDs pre-date jump optimization - so a shared block CREATED by jump2's own cross-jump should be invisible to the chain.
- probe: Both copies respelled `var_v0_2 = 0x19; if (var_s1 == 0) var_v0_2 = 0xB; *(s16 *)(arg0 + 0x286) = var_v0_2; return ret;` with no label anywhere (tmp/grind/func_800283D0/s5/v_nolabel.c), measured with sandbox --disable all.
- result: 30 / 211 insns. jump2 cross-jumps the store+return TAILS first, rebuilds the shared block, and merges the two selections on the rescan - the same failure mode as s2's H7b. Banked at memory/grind/func_800283D0/rejected/diamond2-no-shared-label-remerges.c.
- verdict: KILLED

## Session 6 (2026-08-26, synthesis) — measured

| id | hypothesis | mechanism | probe | result | verdict |
|---|---|---|---|---|---|
| H19 | The s5 H17 refutation is invalid because `reg_live_length` is a FLOW-TIME count while H17 argued from EMITTED asm; named-intermediate copies inside pseudo 143's range could raise live_length and then vanish at `final` as no-op moves | flow.c life_analysis increments REG_LIVE_LENGTH per insn where the pseudo is live; final.c never emits a set whose SRC == DEST after allocation, so a coalesced copy is byte-invisible | two byte-neutral named-intermediate spellings inside the equal arm (`s16 id1/id2` for the two call first-args; `u8 *p1 = arg1`), ALLOCDBG table read for pseudos 72/77/143/73/75/90/79 | Every row byte-identical to base (143: 3 refs/14/2142; 73: 7/92/1521; 75: 6/88/1363). cse/combine delete the copies BEFORE life_analysis runs, so they never reach the counter | KILLED — and this CLOSES the escape hatch: any insn that raises live_length in our RTL is also an emitted insn, which breaks 215==215. s5's H17 now rests on a mechanism, not an asm-line count |
| H20 | The s2/s3 flip has a SECOND solution branch nobody derived: lower pri(73) above pri(143) instead of lowering pri(143) below pri(73) | allocno_compare pri = floor_log2(n_refs)*n_refs*10000*size/live_length; target's map needs the sorted order 72,77,73,143,75,90,79. That holds iff pri(77)=3698 > pri(73) > pri(143)=2142, i.e. 140000/L73 in (2142,3698) | closed-form re-derivation against the measured ALLOCDBG table (s6/alloc_trace.txt) | **L73 in [38,65] flips the whole callee-saved map to target's** (currently 92). A 28-value window vs H16's 3-value one, and it needs liveness REMOVED, not 6-10 instructions added — so it is not blocked by the 215==215 constraint that killed H17 | CONFIRMED (model); untried |
| H21 | The 4-diff "reorg j/nop wobble" at slots 45-48, dismissed since s1 as near-neutral, is really a CFG difference: target's return-1 exit block is SHARED and LABELLED, ours is unlabelled | target `.L80028488` is a 2-insn block (`j .L80028700` / `addiu $v0,1`) with THREE branch predecessors (asm/funcs/func_800283D0.s:17,24,26) plus fallthrough; a label forbids reorg from sinking the `li` into the preceding beq's delay slot. Ours emits the same two insns unlabelled, so reorg's eager fill takes `li v0,1` into the beq slot and leaves the `j` a nop | read of asm/funcs/func_800283D0.s:12-53 + 213-226 against the sandbox objdump of the base body | CONFIRMED. Target keeps BOTH exit blocks: `.L80028488` (constant 1, 3 early exits + chain fallthrough) and `.L800286FC` (`addu $v0,$s6,$zero`, the block_48/block_49 tails). Ours cross-jumps the three early exits into the `move v0,s6` block instead, because our C returns `ret` there where the original returned the literal 1 | CONFIRMED |
| H22 | Spelling the three early exits as constant-1 returns reproduces target's shared `.L80028488` block | jump2 cross-jump merges identical return-1 tails into one block | v4: `block_13: return ret;` -> `return 1;`. v5: single shared `ret_one:` label at the chain fallthrough, `goto ret_one;` from block_13 and from the temp_v0 exits | BOTH measure 34 / **216** insns. The content hypothesis is right but jump2 keeps the LATE copy: the chain's last `beq v1,v0` inverts to `bne v1,v0,<end>` with `li v0,1` in its slot and the shared block relocates to the function end. Banked rejected/early-exit-return-const1-relocates-shared-block.c and rejected/shared-ret-one-label-relocates-shared-block.c | KILLED (these two spellings); the CLUSTER is now a placement question, not an attribution one |

## Frontier (for session 7)

1. **Cluster B (4 diffs) — newly attributed and newly actionable.** The residual is no longer
   "a reorg fill wobble"; it is that our three early exits (`temp_v1 == 4`, `temp_v0 == 4`,
   `temp_v0 == 0x14`) return `ret` and therefore cross-jump into the shared `move v0,s6`
   block, while the original returned the literal 1 into a shared `li v0,1 / j` block placed
   at the rejection chain's fallthrough. Both constant-1 spellings tried this session relocate
   that block to the function end (+1 insn, 216). Next probe: dump `.jump2` for v5
   (`tmp/grind/func_800283D0/s6/v5.c`) and read WHICH of the two candidate blocks
   find_cross_jump keeps and why the chain's `beq` inverts — the mechanism is the same
   `jump_chain` ordering question as diamond 2, so one instrumented read may settle both.
   Mechanism: jump.c find_cross_jump / block placement, then reorg.c fill_simple_delay_slots.
2. **s2/s3 (12 diffs) — the L73 branch of the window (H20), never tried.** L73 in [38,65]
   flips the whole map; L73 is currently 92 and needs liveness REMOVED, so unlike H17's route
   it is not blocked by the 215==215 insn budget. Next probe: reverse the s5 dial — measure
   which regions of arg1's live range contribute how many insns to L73 (insert/delete probes
   at each of the six `func_80032854` call sites and read ALLOCDBG livelen for 73), and look
   for a region where arg1 could honestly be dead. Mechanism: global.c:635-655 allocno_compare
   with flow.c-computed reg_live_length.
3. **Tail a0/a1 qty cluster (7 diffs, cluster E) — still never dialled** (carried unspent from
   the s5 frontier). Requirement: pri(qty0 pointer) >= pri(qty3 Judge[] element) = 5000, i.e.
   span(qty0) <= 24 (from 30) or span(qty3) >= 6 (from 4). Build the local-alloc analogue of
   s5/dial.sh with `BB2_QTY_DEBUG=1` (harness tmp/grind/func_800283D0/s3/qty.sh) and dial
   filler between the `lh Judge($at)` and the `mult` BEFORE proposing any C — qty0 is a
   COMBINED quantity (sll producer + temp_a0 + var_a1) so the co-moving denominators must be
   read from the trace, not assumed. Mechanism: local-alloc.c qty_compare_1.

## [s6] s5's H17 refutation is unsound because reg_live_length is a FLOW-TIME count while H17 argued from emitted asm; named-intermediate copies inside pseudo 143's live range could raise live_length and then vanish at final as no-op moves, buying the s2/s3 flip for free.
- mechanism: flow.c life_analysis increments REG_LIVE_LENGTH once per insn in every block where the pseudo is live; final.c never emits a SET whose SRC == DEST after hard-register assignment, so a copy that coalesces is byte-invisible in the object.
- probe: Two byte-neutral named-intermediate spellings applied inside the equal arm on the s5 base body and read back through the instrumented cc1 with BB2_ALLOC_DEBUG=1: (a) s16 id1/id2 carrying the two func_80032854 calls' first argument, (b) u8 *p1 = arg1 carrying the third. ALLOCDBG rows for pseudos 72/77/143/73/75/90/79 compared against base.
- result: Every row byte-identical to base (143: 3 refs / livelen 14 / pri 2142; 73: 7/92/1521; 75: 6/88/1363; 72: 19/155/4903; 77: 9/73/3698; 90: 3/32/937; 79: 6/322/372). cse/combine delete the copies before life_analysis runs, and GCC 2.7.2 has no later coalescing pass that could preserve them, so they never reach the counter.
- verdict: KILLED

## [s6] The s2/s3 callee-saved flip has a second solution branch nobody has derived: raise pri(pseudo 73, arg1's home) above pri(143) instead of lowering pri(143) below pri(73).
- mechanism: global.c:635-655 allocno_compare, pri = floor_log2(n_refs)*n_refs*10000*size/live_length with ties to the lower allocno; MIPS defines no REG_ALLOC_ORDER so find_reg walks hard registers ascending and the sorted allocno order alone determines the callee-saved map. Target's map is the sorted order 72, 77, 73, 143, 75, 90, 79, which holds iff pri(77)=3698 > pri(73) > pri(143)=2142.
- probe: Closed-form re-derivation against the ALLOCDBG table measured this session (tmp/grind/func_800283D0/s6/alloc_trace.txt), solving 2142 < 140000/L73 < 3698.
- result: L73 in [38,65] flips the entire callee-saved map to target's (73->$s2, 143->$s3, 75->$s4). L73 is currently 92. This is a 28-value window against H16's 3-value one, and it is satisfied by REMOVING arg1 liveness rather than by adding 6-10 instructions, so it is not blocked by the 215==215 instruction budget that foreclosed the L143 route.
- verdict: CONFIRMED

## [s6] The 4-diff 'near-neutral reorg j/nop wobble' at emitted slots 45-48, logged since s1 and never probed, is really a CFG difference: target's return-1 exit block is shared and labelled where ours is unlabelled.
- mechanism: reorg.c fill_simple/fill_eager_delay_slots may sink a fallthrough insn into a preceding conditional branch's delay slot only when that insn is not the body of a multiply-reached labelled block. Target's .L80028488 (asm/funcs/func_800283D0.s:48-50) is `j .L80028700` / `addiu $v0,$zero,0x1` with three branch predecessors (s.17, 24, 26 - the temp_v1==4, temp_v0==4 and temp_v0==0x14 early exits) plus fallthrough from the last rejection-chain beq, so the label forbids the sink and target emits `beq / nop` then `j / li v0,1`.
- probe: Read asm/funcs/func_800283D0.s:12-53 and 213-226 against an objdump of the cheat-invisible sandbox object for the base body, plus a fresh masked pairdiff (tmp/grind/func_800283D0/s6/pd_base.txt).
- result: Confirmed. We emit the same two instructions at the same position but unlabelled, so reorg sinks `li v0,1` into the beq slot and leaves the `j` a nop. Target keeps BOTH exit blocks - .L80028488 (constant 1) and .L800286FC (addu $v0,$s6,$zero, for the block_48/block_49 tails) - whereas our three early exits return `ret` and therefore cross-jump into the `move v0,s6` block, so no constant-1 block ever forms. The cluster is neither near-neutral nor downstream of diamond 2.
- verdict: CONFIRMED

## [s6] Spelling the three early exits as constant-1 returns reproduces target's shared .L80028488 block and closes cluster B.
- mechanism: jump2 find_cross_jump merges identical return-1 tails into one shared block; with the tails constant-1 rather than `return ret` they can no longer merge into the `move v0,s6` convergence block.
- probe: v4: `block_13: return ret;` respelled `return 1;` (tmp/grind/func_800283D0/s6/v4.c). v5: a single shared `ret_one:` label sited at the rejection chain's fallthrough, reached by `goto ret_one;` from block_13 and from the temp_v0 exits (s6/v5.c). Both measured with sandbox --disable all.
- result: Both measure 34 / 216 insns. jump2 cross-jumps the two return-1 tails and keeps the LATE copy: the chain's last `beq v1,v0,<block_15>` inverts to `bne v1,v0,<end>` with `li v0,1` in its delay slot, and the shared exit block relocates to the function end, costing one instruction. The content of the hypothesis is correct - target does want a constant-1 exit block - but PLACEMENT is the residual, and it is the same jump_chain-ordering question as diamond 2.
- verdict: KILLED

## Session 7 (2026-08-26, solver) — measured

Chassis: candidate body applied to src/code6cac_b.c, sandbox --disable all = **28 / 215 insns**
(build_insns 215 == target 215). Every number below is from the validated solver suite
(`tools/ra_solver`), ground-truthed against the instrumented cc1's own dumps this session.

## [s7] The residual's FIRST divergence is not RA at all — a triage run settles which layer owns it
- mechanism: `inverse_compose.classify` / `goal_from_tgt.py classify` compares register-blanked
  instruction MULTISETS (pre-RA), then exact texts (RA), then order (SCHED). Only the first
  layer that differs can own the residual.
- probe: `python3 tools/ra_solver/goal_from_tgt.py classify code6cac_b func_800283D0`
  (object-level path; the text path is unusable because src now carries our C body, so
  `<stem>.tgt.s` would be OUR build, not target's).
- result: **PRE-RA**, and the whole pre-RA delta is ONE instruction shape:
  `ours only: beqz #,@ x1` / `target only: bnez #,@ x1`. Everything else is a rename or a move.
  Alignment: |A|=|B|=215, equal 187, replace 24, moved 4, delete 0, insert 0.
  So the 28-point residual decomposes EXACTLY as: 1 branch-sense insn (cluster D/B),
  4 scheduler moves, 21 register renames, 2 immediate/reloc-differing pairs.
- verdict: CONFIRMED

## [s7] The global (callee-saved) cluster is a pure 73<->143 exchange and is reachable ONLY through those two allocnos
- mechanism: global.c allocno_compare sorts by
  `pri = floor_log2(n_refs)*n_refs*size/reg_live_length*10000`, ties -> lower allocno number;
  the sorted order maps 1:1 onto $s0..$s6. Our order puts 143 at slot 18 and 73 at slot 20;
  target's map is 72,77,73,143,75,90,79 — i.e. 73 and 143 exchanged.
- probe: extract.py -> model.json (25 allocnos), `goal_from_tgt.py goal --model` -> goal
  `{73: $s2, 143: $s3, 72: ...}`, then an EXHAUSTIVE single-atom sweep of the validated
  `simulate.Sim` over every allocno x livelen 1..250 x nrefs 1..24
  (tmp/grind/func_800283D0/s7/sweep_all.py).
- result: exactly TWO allocnos have any single-atom vector at all — 73 and 143. Every other
  allocno in the function is inert for this goal at any live length and any ref count.
- verdict: CONFIRMED

## [s7] The flip windows for the global cluster, measured exhaustively (this supersedes every hand-derived window)
- mechanism: as above; the sweep evaluates the real forward model, not the priority formula by hand.
- probe: sweep_all.py + joint.py (2-D sweeps) against `tmp/ra_solver_work/func_800283D0.model.json`
  (base: pseudo 73 nrefs=7 livelen_flow=92 reg_live_length=132 calls=3;
   pseudo 143 nrefs=3 livelen_flow=14 reg_live_length=22 calls=2).
- result (single-atom):
    * `L73` 92 -> **[38,65]** flips (confirms s6's derivation exactly)
    * `L143` 14 -> **[20,21]** flips — i.e. +6 or +7 and NO MORE; +8 overshoots
    * `R73` 7 -> **[8,11]** flips
    * `R143` 3 -> **2** flips
  result (joint, the region no prior session had):
    * L143=10..21 each admit an L73 range whose LOWER bound is a constant 38 and whose upper
      bound rises 46,51,56,60,65,70,74,79,84,88,93,98 — so the cluster is a 2-D diagonal
      region, not two isolated dials, and small partial moves in BOTH variables compose.
    * R73=8 admits L73 in [65,112] — base L73=92 sits INSIDE it, which is why refs_up 73 is a
      single-atom solution at the current live length.
    * R143=4 admits L143 in [53,58]; R143=2 admits only L143=14 (the base).
- verdict: CONFIRMED
- NOTE for the next session: `inverse.py global` reports NO livelen vector for this goal. That is
  a TOOL ARTEFACT, not a fact: its search bounds are "live length +/-2,4,8", and the true L143
  window is +6/+7 — a hole in the delta set. Sweep `simulate.Sim` directly (sweep_all.py) rather
  than trusting inverse.py's negative on livelen.

## [s7] The tail a0/a1 cluster (E) is a LOCAL-alloc decision, its two participants are named, and its window is measured
- mechanism: local-alloc.c `block_alloc`: `qty_compare_1` priority
  `floor_log2(refs)*refs*size/(death-birth)*10000`, ties -> ascending qty number; then
  `find_free_reg` scans ascending.
- probe: (a) read the .lreg RTL to NAME the participants instead of guessing —
  `(reg/v:SI 184)` is the tail pointer `temp_s4 + temp_s5*0x10` (its uses are the +276/+280/+284
  loads) and `(reg:SI 201)` / `(reg:SI 211)` are the two `(&Judge)[...]` elements; NEITHER 184 nor
  201 appears in the global allocno order, so both are block-local quantities.
  (b) `local_extract.py code6cac_b` + `inverse.py local ... --func func_800283D0 --block 41
  --swap 0,3 --depth 2 --top 40`.
- result: block 41 baseline is qty0 = reg 184 (birth 2, death 32, span 30, refs 6) -> `$a1`,
  qty3 = reg 201 (birth 16, death 20, span 4, refs 2) -> `$a0`; target wants them exchanged.
  The EXHAUSTIVE single-atom vector set is:
    * `span(qty0)` 30 -> **<= 24** (any of birth 2->8..19, or death 32->16..26)
    * `refs(qty0)` 6 -> **8, 9 or 10**
  and NOTHING else. s3's hand-derived "span(qty0) <= 24" is CONFIRMED; s3's alternative
  "span(qty3) >= 6" is **REFUTED** — no perturbation of qty3 alone reaches the goal.
- verdict: CONFIRMED

## [s7] Delaying the pointer's birth by reordering the tail declarations moves the local dial, but only 4 of the needed 6 insns
- mechanism: the qty's birth is its first reference in the POST-sched1 insn stream; putting an
  independent computation textually first gives sched1 a reason to emit it first.
- probe: variant B_ptr_late — `s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);` declared BEFORE
  `u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);` (pure declaration reorder, no new locals).
  Measured with sandbox --disable all AND with a fresh BB2_QTY_DEBUG block-41 dump.
- result: **28 / 215 — byte-neutral in score**, but the local model MOVED: the pointer quantity
  becomes qty1 / first_reg 188 with **birth 6 (was 2), death 32, span 26 (was 30)**. That is
  4 of the 6 insns the window needs; the assignment does not flip yet (`got` still `$a1`=5).
  This is the first C lever ever shown to move this cluster's dial in the right direction.
- verdict: CONFIRMED (partial — window not yet reached)

## [s7] Hoisting the temp_a0_2 abs above the temp_v1_5 product is the other span-shrink route
- mechanism: moving the last use of the pointer earlier would shrink span(qty0) from the death side
  (window: death 32 -> <= 26).
- probe: variants A_abs_first and C_both (`s32 temp_v1_5;` declared, assigned after the abs `if`).
- result: **44 / 214 insns** for BOTH — one instruction FEWER than target and a 16-point
  regression; C_both scores identically to A_abs_first, so the loss is owned by the abs hoist
  alone and not by the pointer reorder. Banked as
  rejected/tail-abs-hoisted-above-product-loses-insn.c. Together with s3's
  tail-abs-sunk-below-shift-adds-insn.c (31/216) BOTH directions of moving the abs are now dead,
  so the death side of the qty0 window must be reached some other way (or not at all).
- verdict: KILLED

## [s7] The 28-point residual's FIRST divergence layer is not RA; a triage run will say which layer owns it and how much of the stream is instruction-selection rather than renaming.
- mechanism: inverse_compose/goal_from_tgt classify compares register-blanked instruction MULTISETS (pre-RA), then exact texts (RA), then order (SCHED); only the first differing layer can own the residual. The object-level path is required because src now carries our C body, so mkasm_honest's .tgt.s half would rebuild OUR code, not target's.
- probe: python3 tools/ra_solver/goal_from_tgt.py classify code6cac_b func_800283D0, plus the same tool's `goal --show` alignment report.
- result: PRE-RA, and the entire pre-RA delta is one instruction shape: ours only `beqz #,@` x1, target only `bnez #,@` x1. Alignment of the two 215-insn streams: equal 187, replace 24, moved 4, delete 0, insert 0 - i.e. 1 branch-sense insn + 4 scheduler moves + 21 register renames + 2 immediate/reloc-differing pairs, with no hidden selection difference anywhere else.
- verdict: CONFIRMED

## [s7] The callee-saved (s2/s3) cluster is a pure exchange of allocnos 73 and 143, and some third allocno might offer a cheaper lever.
- mechanism: global.c allocno_compare sorts by pri = floor_log2(n_refs)*n_refs*size/reg_live_length*10000 with ties broken by lower allocno number; the sorted order maps 1:1 onto $s0..$s6, so any flip must come from a priority change on some allocno.
- probe: extract.py -> model.json (25 allocnos); goal_from_tgt goal --model -> the 73/143 exchange; then an exhaustive single-atom sweep of the validated simulate.Sim over EVERY allocno x livelen 1..250 x nrefs 1..24 (tmp/grind/func_800283D0/s7/sweep_all.py).
- result: Exactly TWO allocnos have any vector at all - pseudo 73 (= arg1, 7 refs: the prologue `move s3,a1` plus six `move a2,s3`) and pseudo 143 (= temp_s3 = arg0 + temp_a1_2*2, RTL insn 277). Every other allocno in the function is inert for this goal at every live length and every ref count. No third lever exists.
- verdict: CONFIRMED

## [s7] The flip windows for the global cluster are wider / differently shaped than the hand-derived ones the ledger carries (s5: '6-10 insns of extra live length on 143'; s6: 'L73 in [38,65]').
- mechanism: Same allocno_compare model, but evaluated by the validated forward simulator rather than by hand, and swept jointly in two variables instead of one at a time.
- probe: sweep_all.py (single-atom, exhaustive) and joint.py (2-D sweeps) against tmp/ra_solver_work/func_800283D0.model.json (base: 73 nrefs=7 livelen=92 rll=132 calls=3; 143 nrefs=3 livelen=14 rll=22 calls=2).
- result: Single-atom: L73 92 -> [38,65] (confirms s6 exactly); L143 14 -> [20,21] ONLY (+6 or +7; +8 already overshoots, correcting s5's '6-10'); R73 7 -> [8,11]; R143 3 -> 2. Joint: L143 = 10..21 admits L73 upper bounds 46,51,56,60,65,70,74,79,84,88,93,98 with a constant lower bound of 38, so the region is a diagonal and partial moves in both variables compose; R73=8 admits L73 in [65,112], which contains the base 92 - that is why one extra reference to arg1 is a single-atom solution today. Tool note banked: inverse.py global reports NO livelen vector here purely because its search bounds are +/-2,4,8 and the true window sits in the 6/7 hole - a tool artefact, not a foreclosure.
- verdict: CONFIRMED

## [s7] The tail a0/a1 cluster (7 diffs, cluster E) is a local-alloc quantity-order decision between the temp_a0 pointer and a Judge[] element, and its flip window has never been measured.
- mechanism: local-alloc.c block_alloc: qty_compare_1 priority floor_log2(refs)*refs*size/(death-birth)*10000, ties by ascending qty number, then find_free_reg scans ascending.
- probe: Read the .lreg RTL to NAME the participants instead of guessing (reg/v:SI 184 = temp_s4 + temp_s5*0x10, uses +276/+280/+284; reg:SI 201 / 211 = the two (&Judge)[...] elements; none of 184/186/201/211 appears in the global allocno order, so all are block-local). Then local_extract.py code6cac_b + inverse.py local --func func_800283D0 --block 41 --swap 0,3 --depth 2 --top 40.
- result: Block 41 baseline: qty0 = reg 184 (birth 2, death 32, span 30, refs 6) -> $a1; qty3 = reg 201 (birth 16, death 20, span 4, refs 2) -> $a0; target wants them exchanged. The COMPLETE single-atom vector set is span(qty0) 30 -> <= 24 (from either end: birth 2 -> 8..19, or death 32 -> 16..26) or refs(qty0) 6 -> 8/9/10, and nothing else. s3's hand-derived span(qty0) <= 24 is CONFIRMED; s3's alternative 'span(qty3) >= 6' is REFUTED - no qty3-only perturbation reaches the goal.
- verdict: CONFIRMED

## [s7] An ordinary-C declaration reorder that lets an independent computation precede the tail pointer will delay the pointer quantity's birth enough to reach span(qty0) <= 24.
- mechanism: A quantity's birth is its first reference in the post-sched1 insn stream; giving sched1 an independent, textually-earlier computation moves the pointer's ashift/addu later without adding instructions.
- probe: Variant B_ptr_late (regenerable via tmp/grind/func_800283D0/s7/probe.py B_ptr_late): declare `s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);` BEFORE `u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);` - no new locals, no new statements. Measured with sandbox --disable all AND re-derived from a fresh BB2_QTY_DEBUG block-41 dump (the local-mode caveat demands the dump, not the model's claim).
- result: 28 / 215 - byte-neutral in score - but the local model MOVED: the pointer quantity becomes qty1 / first_reg 188 with birth 6 (was 2), death 32, span 26 (was 30). That is 4 of the 6 insns the window needs; `got` is still $a1, so the assignment has not flipped. First C lever ever shown to move this cluster's dial in the right direction; 2 more insns of birth delay are needed.
- verdict: CONFIRMED

## [s7] The death side of the qty0 window is reachable by hoisting the temp_a0_2 abs above the temp_v1_5 product, so the pointer's last use dies earlier.
- mechanism: Same qty_compare_1 span; moving the last reference of the pointer earlier shrinks death from 32 toward the <= 26 the window needs.
- probe: Variants A_abs_first and C_both (declare `s32 temp_v1_5;` and assign it after the abs `if`), sandbox --disable all.
- result: BOTH measure 44 at 214 insns - one instruction FEWER than target's 215 and a 16-point regression; C_both scores identically to A_abs_first, so the loss is owned by the abs hoist alone and not by the pointer reorder. Banked as rejected/tail-abs-hoisted-above-product-loses-insn.c. Combined with s3's tail-abs-sunk-below-shift-adds-insn.c (31/216), BOTH directions of moving the abs are now dead and the abs is pinned where it is.
- verdict: KILLED


## Session 8 (2026-08-26, forensics) - measured

## [s8] A per-function BB2_XJUMP_DEBUG trace can be obtained despite the hook's whole-TU, uid-restarting output.
   probe: build iso.c (same TU, all other function bodies emptied), compile with the instrumented cc1, diff the func_800283D0 segment of .jump2 against the full-TU dump.
   result: segments identical except CODE_LABEL_NUMBER (TU-global .LNN counter).  211 XJDBG lines, all this function's.
   verdict: CONFIRMED

## [s8] The cross-jump pass that merges the two var_v0_2 selection copies runs before register allocation, so RA and cross-jump levers interact.
   probe: read toplev.c around the .jump2 dump; read the compared insns in .sched2.
   result: toplev.c:3141 calls jump_optimize(insns,1,1,0) AFTER sched2 and after local/global alloc + reload, immediately before reorg.  The compared insns already carry hard registers.
   verdict: KILLED - jump2 is post-RA; clusters B/D are independent of clusters A/E.

## [s8] s7's frontier item 3: one .jump2 read settles clusters B, C and the beqz/bnez insn together, because find_cross_jump's choice of surviving copy inverts the chain's last branch sense.
   probe: full XJDBG trace of the current body plus the canonical-both body.
   result: the beqz/bnez at slot 138 is the source's branch shape expanded into RTL long before jump2; the cross-jump decisions logged for the return-1 blocks (uids 84/613/618) are separate decisions with their own arithmetic.
   verdict: KILLED as stated (cluster B remains open on its own terms).

## [s8] The merge that costs 4 insns is a single find_cross_jump decision whose arithmetic can be read exactly.
   probe: BB2_XJUMP_DEBUG on the canonical-both body.
   result: e1=362 vs chain partner e2=397, minimum starts at 2, three MATCHes (both constant loads AND the two conditional branches) plus a LABEL-BONUS take it to -2.  Merge fires with 3 units of slack; only a mismatch on the FIRST compared insn defeats it.
   verdict: CONFIRMED

## [s8] Because rtx_renumbered_equal_p compares GET_MODE, giving one copy's constant loads a different machine mode defeats the merge while leaving the emitted bytes identical.
   probe: route the tail copy's selection through an s32 intermediate (both copies otherwise canonical); measure sandbox and re-diff the emitted stream.
   result: 28 -> 25, build_insns 215 == 215.  Cluster D closed: slots 127-130 and 138-141 both match target, canonical bnez / 0x19-then-0xB order.
   verdict: CONFIRMED

## [s8] The sanctioned variable-reuse family can express the mode split by borrowing the existing s32 local d_val instead of inventing an intermediate.
   probe: d_val = 0x19; if (var_s1 == 0) d_val = 0xB; var_v0_2 = d_val;
   result: 33 / 216 - the copy does not coalesce (d_val's pseudo has real conflicts), so it survives as a 216th instruction.
   verdict: KILLED

## Frontier (for session 9)
1. CLASSIFY OR RESPELL THE MODE SPLIT.  The 25-floor body's only new construct is a fresh, multi-write, once-read s32 intermediate whose mechanism is jump.c's mode comparison.  It fits no frozen family (named-intermediate excludes multi-write; variable-reuse measured dead).  Either obtain a ruling, or find an ordinary-C spelling that produces an SImode selection in the tail copy for program reasons - e.g. a tail-copy selection expression that is naturally int-typed and only narrowed at the shared store.
2. CLUSTER B with the new attribution.  Target's shared, labelled `j / li v0,1` exit block is a POST-RA cross-jump product, not a source-CFG accident: in the trace our return-1 blocks reach uid 618-vs-84 with MATCH set(reg<-1) + LABEL-BONUS = min 0 => WIN, i.e. we merge a pair whose survivor placement differs from target's (the slot 45/47 nop/li swap is what survives).  The same mode lever may apply, and the trace names the exact insns (82, 616) to differentiate.
3. CLUSTERS A and E are now provably independent of everything jump2 does, so s7's exhaustive RA windows (L73 92->[38,65], L143 14->[20,21]; span(qty0) 30 -> <=24) remain the live model - but re-run them ON THE 25-FLOOR BODY, since the mode split changed the tail block's RTL and may have moved the live lengths.

## [s8] A per-function BB2_XJUMP_DEBUG trace can be obtained even though the hook's output is whole-TU and insn UIDs restart per function.
- mechanism: Compile an isolated TU (same file, every other function body emptied to '{ }'); if the function's .jump2 RTL segment is unchanged, the stderr trace is attributable in full.
- probe: tmp/grind/func_800283D0/s8/isolate.py + dumpiso.sh; diff the func_800283D0 segment of iso.jump2 against the full-TU code6cac_b.jump2.
- result: Segments identical except CODE_LABEL_NUMBER values (the TU-global .LNN counter). 211 XJDBG lines, all belonging to func_800283D0.
- verdict: CONFIRMED

## [s8] The cross-jump pass that merges the two var_v0_2 selection copies runs before register allocation, so RA/scheduling levers and cross-jump levers interact.
- mechanism: GCC 2.7.2 pass order in toplev.c decides whether find_cross_jump sees pseudos or hard registers.
- probe: Read toplev.c around the .jump2 dump block; read the compared insns in the .sched2 dump.
- result: toplev.c:3141 calls jump_optimize(insns,1,1,0) AFTER sched2 and after local/global allocation + reload, immediately before reorg (.dbr). The compared insns already carry hard registers ((reg/v:HI 2 v0)).
- verdict: KILLED

## [s8] s7 frontier item 3: one instrumented .jump2 read settles clusters B, C and the beqz-vs-bnez instruction together, because find_cross_jump's choice of surviving copy inverts the rejection chain's last branch sense.
- mechanism: jump.c find_cross_jump / do_cross_jump redirecting one of two identical tails.
- probe: Full XJDBG trace of the current body and of the canonical-both body; slot-level normalized diff of the emitted stream against target.
- result: The beqz/bnez at emitted slot 138 is the source's branch shape expanded into RTL long before jump2 (it flips with the source spelling, not with the merge); the return-1 block decisions (uids 84/613/618) are separate decisions with their own min arithmetic. The three clusters do not share one lever.
- verdict: KILLED

## [s8] The 4-insn loss on the canonical-both body is a single find_cross_jump decision whose arithmetic can be read exactly.
- mechanism: find_cross_jump starts at minimum=2 for a jump_chain partner (1 for own-label), subtracts 1 per matching insn and 1 more on reaching a CODE_LABEL in stream 1; the merge fires at <= 0.
- probe: BB2_XJUMP_DEBUG on the canonical-both body (tmp/grind/func_800283D0/s8/xjump_CANON2.log).
- result: enter e1=362 e2=397 min=2; MATCH 358/393 set(reg<-11) -> 1; MATCH 354/389 set (the two bnez) -> 0; MATCH 351/386 set(reg<-25) -> -1; LABEL-BONUS i1=347 -> -2; WIN; DO_CROSS_JUMP jump=362 newjpos=351 newlpos=386. Three units of slack: only a mismatch on the FIRST compared insn defeats it.
- verdict: CONFIRMED

## [s8] Because rtx_renumbered_equal_p compares GET_MODE, giving one copy's constant loads a different machine mode defeats the merge while leaving the emitted bytes identical.
- mechanism: var_v0_2 is s16, so both copies emit (set (reg/v:HI 2 v0) (const_int 25)) / movhi_internal2; an s32 intermediate emits movsi_internal2 instead, which assembles to the same 'addiu $v0,$zero,0x19' byte but is a distinct RTL pattern, so find_cross_jump PAT-MISMATCHes on insn 1.
- probe: Tail copy spelled 's32 sel = 0x19; if (var_s1 == 0) sel = 0xB; var_v0_2 = sel;' with BOTH copies otherwise canonical; sandbox --disable all plus a normalized slot diff.
- result: score 25 (from 28), build_insns 215 == target 215. Cluster D closed: emitted slots 127-130 and 138-141 now match target exactly, including target's bnez sense and canonical 0x19-then-0xB order that the s2 if/else spelling could never produce. First floor movement since session 2.
- verdict: CONFIRMED

## [s8] The sanctioned variable-reuse family can express the same mode split by borrowing the existing s32 local d_val instead of inventing an intermediate.
- mechanism: Borrowed local is already SImode, so the same movsi-vs-movhi mismatch should arise with no new declaration.
- probe: d_val = 0x19; if (var_s1 == 0) d_val = 0xB; var_v0_2 = d_val; measured with sandbox --disable all.
- result: 33 / 216. d_val's pseudo carries real conflicts from its earlier live range, so reload does not coalesce the copy and it survives as a 216th instruction. Banked as rejected/reuse-dval-selection-holder-no-coalesce.c.
- verdict: KILLED


## Session 9 (2026-08-26, forensics) - measured

## [s9] The s8 mode split can be spelled with a ONCE-WRITTEN, once-read, int-typed intermediate, which removes the classification blocker without costing a byte.
- mechanism: find_cross_jump compares patterns with rtx_renumbered_equal_p, which compares
  GET_MODE.  The tail copy needs SImode constant loads while the ==5 copy keeps HImode ones.
  s8 achieved that with `s32 sel = 0x19; if (var_s1 == 0) sel = 0xB; var_v0_2 = sel;` - a
  multi-write local, which fits no frozen SOTN family.  A single-assignment ternary
  initialiser produces the same SImode pseudo in one write.
- probe: `{ s32 sel = (var_s1 == 0) ? 0xB : 0x19; var_v0_2 = sel; }` (variant Q1), sandbox
  --disable all plus a slot-for-slot comparison of the emitted stream against s8's body.
- result: 25 / 215, and the emitted instruction stream is BYTE-IDENTICAL to s8's `sel` body
  (215/215 lines equal).  The intermediate is now fresh, once-written, once-read, int-typed,
  and carries a real consumed value - the named-intermediate SHAPE.
- verdict: CONFIRMED

## [s9] The ternary's arm order, not the ternary itself, decides the emitted branch sense.
- mechanism: expand_expr lowers COND_EXPR by branching on the negated condition with the
  ':' arm in the fallthrough/delay position, so the C arm order maps directly onto which
  constant lands in the branch's delay slot.
- probe: three arm orders measured on the 25-floor body -
  `(var_s1 != 0) ? 0x19 : 0xB`, `var_s1 ? 0x19 : 0xB`, `(var_s1 == 0) ? 0xB : 0x19`.
- result: the first two measure 28 / 215 - the merge IS defeated but the block emits
  `beqz $s1 / li 0xB / j / li 0x19` against target's `bnez $s1 / li 0x19 / j / li 0xB`
  (3 diffs at slots 138-141).  Only `(var_s1 == 0) ? 0xB : 0x19` reproduces target's sense
  and reaches 25.  Banked as rejected/tern-no-intermediate-inverts-branch-sense.c.
- verdict: CONFIRMED

## [s9] The intermediate is load-bearing: an int-typed ternary written straight into the s16 var_v0_2 would carry the same mode split.
- mechanism: if the RHS is int-typed, the constant loads might stay SImode even when the
  destination is HImode.
- probe: `var_v0_2 = (var_s1 == 0) ? 0xB : 0x19;` with no intermediate; and separately
  declaring var_v0_2 itself as `s32` with both copies canonical.
- result: BOTH measure 30 / 211 - i.e. the 4-insn cross-jump merge is back.  The C-level
  conversion to short is folded into the selection, so the constant loads are HImode and
  match the ==5 copy; and making var_v0_2 SImode moves BOTH copies to SImode, so they still
  match each other.  The lever is a mode SPLIT between the two copies, not SImode per se.
  Banked as rejected/tern-no-intermediate-canonical-order-remerges.c and
  rejected/var-v0-2-declared-s32-both-copies-remerge.c.
- verdict: KILLED

## [s9] The cross-jump merge can be defeated STRUCTURALLY - by giving one selection copy its own store and return so the two blocks no longer share the block_48 tail - which would need no mode trick at all.
- mechanism: find_cross_jump walks backward from two jumps; if the insn immediately before
  one jump is a store and before the other is a constant load, it should PAT-MISMATCH on the
  first compared insn exactly as the mode split does.
- probe: four spellings measured - ==5 copy stores directly through a fresh s16 local
  (P2_eq5_direct), ==5 copy stores through the existing outer s16 var_v0 (P2b), tail copy
  stores directly (P3), and ==5-direct combined with the tail mode split (P4).  P2 was then
  re-compiled with the instrumented cc1 and BB2_XJUMP_DEBUG via the s8 isolate/dumpiso
  recipe (tmp/grind/func_800283D0/s9/xjump_P2.log, 289 XJDBG lines).
- result: P2 = 30 / 211, P2b = 30 / 211, P3 = 29 / 211 - the merge STILL fires.  The trace
  names the decision: `enter e1=372 e2=407 min=2 (chain-partner)`, then
  `MATCH i1=360 i2=403 set(reg<-11)`, `MATCH i1=356 i2=399 set`,
  `MATCH i1=353 i2=396 set(reg<-25)`, `LABEL-BONUS i1=347`, `min=-2 => WIN`,
  `DO_CROSS_JUMP jump=372 newjpos=353 newlpos=396`.  The duplicated store+return tail has
  ALREADY been merged back into block_48 by an earlier jump pass before jump2 runs, so the
  two blocks are identical again by the time find_cross_jump looks at them.  P4 emits a
  stream byte-identical to Q1's, confirming the same re-merge.  The structural route is
  CLOSED; the mode split is the only known lever for cluster D.
- verdict: KILLED

## [s9] Cluster C (the 4-diff v0-vs-v1 rename at emitted slots 126/132/133/135) is a register-allocation consequence of sched1 hoisting the block's return-value copy to the head of the block.
- mechanism: local-alloc cannot assign a hard register that is already live across the
  quantity's range.  If `(set (reg/i:SI 2 v0) <ret>)` is scheduled FIRST in the block, v0 is
  live from the top and the two HImode constant pseudos must take v1.
- probe: read the D_800A38A8 block in the pre-sched1 dumps (.combine, .flow), in the
  post-sched1 dump (.sched) and in .lreg / .greg, all regenerated for this body with
  `pwsh tools/grinder/dump.ps1 func_800283D0`.
- result: pre-sched1 the block is 368 (li HI 1) -> 370 (sh D_800A38A8) -> 373 (li HI -1) ->
  375 (sh D_800A3876) -> 378 (set (reg/i:SI 2 v0) (reg/v:SI 79)); .sched shows sched1 hoists
  378 to the FRONT (378, 368, 370, 373, 375); .greg then shows
  `(set (reg:HI 3 v1) (const_int 1))` and `(set (reg:HI 3 v1) (const_int -1))`.  Target has
  no such copy at the head of that block and its constants take v0.  The pass is sched1, the
  decision is the hoist, and the RA outcome is downstream of it.
- verdict: CONFIRMED

## [s9] Deleting the block-local return-value copy - exiting the globals block with `goto block_49;` instead of an inline `return ret;` - frees v0 and closes cluster C.
- mechanism: with no `(set (reg/i:SI 2 v0) ...)` in the block there is nothing for sched1 to
  hoist and nothing holding v0 live, so local-alloc gives v0 to the two constant pseudos.
  `goto block_49;` is ordinary C - block_49 is an already-existing shared `return ret;` label
  in this body.
- probe: variant D4_goto49 on top of Q1; sandbox --disable all plus a difflib alignment of
  the emitted stream against target.
- result: FLOOR 25 -> 23.  Slots 131-136 now read `li v0,1 / lui at / sh v0 / li v0,-1 /
  lui at / sh v0` - target's registers exactly.  Cluster C is CLOSED.
- verdict: CONFIRMED

## [s9] The 23-floor body's only new cost is one unfilled delay slot, and it is a reorg decision, not an extra computation.
- mechanism: reorg's steal-from-branch-target fill.  On the 25-floor body reorg steals the
  branch target's `li v1,1` into the delay slot of `beqz v0,<globals block>` (slot 125).  On
  the 23-floor body the same candidate insn is `li v0,1` - and v0 is the register the branch
  itself reads - and reorg declines, emitting `nop` at slot 126 and leaving the `li v0,1` in
  the block at slot 131.  That is the entire 216-vs-215 delta.
- probe: difflib alignment of the D4 stream against target: the ONLY insert is the nop at
  slot 126; everything from 131 onward re-aligns one slot late against target 126..135.
- result: target performs exactly the steal ours declines - `beqz v0,.L800285DC` with
  `addiu v0,zero,1` in its delay slot - so the fill is reachable in this compiler; the
  reason ours declines has not been read out of reorg.c/resource.c yet.
- verdict: CONFIRMED (as a measurement; the reorg reason is OPEN)

## [s9] The inline-return -> shared-goto change is a general policy that will help at the other early-exit sites too.
- mechanism: same argument as the globals block: fewer block-local return-value copies.
- probe: apply `goto block_49;` to the `temp_v1_2 == 0xE` early return (E1_goto49b) and to
  the do_calls return (G2_docalls).
- result: E1 = 38 / 216, a 15-point REGRESSION; G2 = 23 / 216, byte-identical to D4 (jump2
  re-merges it).  The goto-vs-inline-return choice is PER SITE and must be measured per
  site.  s1's finding that the range-check exit wants an INLINE return still stands.  Banked
  as rejected/shared-goto-for-0xE-early-return.c.
- verdict: KILLED

## [s9] Cheap source-level reorderings inside the globals block can reach cluster C without changing the exit form.
- mechanism: swapping the two independent global stores, or returning the literal 1, changes
  what sched1 sees.
- probe: C1/G1 (store order swapped) and C2 (`return 1;`), measured on both the 25- and
  23-floor bodies.
- result: the store swap is byte-neutral at 25 (it only swaps which constant reaches v1
  first) and is 24 - i.e. WORSE - on the 23-floor body, because target stores 1 to
  D_800A38A8 before -1 to D_800A3876 and the source order is the emitted order.
  `return 1;` measures 26 and leaves 126/132/133/135 unchanged: the return-value set is
  still hoisted by sched1 and still holds v0.  Deleting the copy is the only lever.  Banked
  as rejected/globals-block-store-order-swapped.c and rejected/globals-block-return-const1.c.
- verdict: KILLED

## Frontier (for session 10)
1. THE NOP AT SLOT 126 IS THE WHOLE 216-vs-215 DELTA.  Read reorg.c /
   resource.c in tools/gcc-2.7.2 and, if needed, instrument the delay-slot fill the way s8
   instrumented find_cross_jump, to learn why the steal of `li v0,1` into the delay slot of
   `beqz v0,...` is declined for us when target performs it.  If it is a conservative
   resource conflict with the branch's own source register, the C question becomes: what
   source shape lets the globals block's first insn be something other than a write of the
   branch's condition register?  Killing the nop puts the body at 215 insns with cluster C
   already closed.
2. CLUSTER B (slots 45/47) with the s9 exit-form result in hand.  s9 showed that the
   goto-vs-inline-return choice at a given exit is worth up to 4 slots and is decided per
   site; cluster B is exactly an exit-form/exit-block-sharing residual and the two respellings
   the ledger banked (s6) both moved the shared block wholesale.  The untried axis is the
   THIRD exit-form dimension: which of the several `return ret;` / `goto block_49;` sites
   feeds target's labelled `.L80028488` block.  Enumerate the 2^N exit-form assignments over
   the four early-exit sites (s9 measured only two of them) and measure.
3. CLUSTERS A and E on the 23-floor body.  s7's exhaustive RA windows (global.c: L73 92 ->
   [38,65], L143 14 -> [20,21]; local-alloc: span(qty0) 30 -> <=24 or refs(qty0) 6 -> 8..10)
   have NOT been re-extracted since the s8 mode split or the s9 exit-form change, both of
   which changed the RTL of blocks those quantities live in.  Re-run tools/ra_solver/extract.py
   plus tmp/grind/func_800283D0/s7/qty41.py before spending any spelling probe on them.

## [s9] The s8 mode split (which closes cluster D) can be spelled with a fresh, ONCE-WRITTEN, once-read, int-typed intermediate instead of s8's multi-write local, removing the classification blocker without costing a byte.
- mechanism: jump.c find_cross_jump compares patterns with rtx_renumbered_equal_p, which compares GET_MODE; the tail copy needs SImode constant loads while the ==5 copy keeps HImode ones. A single-assignment ternary initialiser produces the same SImode pseudo in one write.
- probe: { s32 sel = (var_s1 == 0) ? 0xB : 0x19; var_v0_2 = sel; } (variant Q1); sandbox --disable all plus a slot-for-slot comparison of the emitted stream against s8's body.
- result: 25 / 215 and the emitted stream is BYTE-IDENTICAL to s8's (all 215 lines equal). The intermediate is now fresh, once-written, once-read, int-typed and carries a real consumed value - the named-intermediate shape.
- verdict: CONFIRMED

## [s9] The ternary's ARM ORDER, not the ternary itself, decides the emitted branch sense.
- mechanism: expand_expr lowers COND_EXPR by branching on the negated condition with the ':' arm in the fallthrough/delay position, so the C arm order maps onto which constant lands in the branch's delay slot.
- probe: Three arm orders on the 25-floor body: (var_s1 != 0) ? 0x19 : 0xB, var_s1 ? 0x19 : 0xB, (var_s1 == 0) ? 0xB : 0x19.
- result: The first two measure 28 / 215 - the merge IS defeated but the block emits `beqz $s1 / li 0xB / j / li 0x19` against target's `bnez $s1 / li 0x19 / j / li 0xB` (3 diffs at slots 138-141). Only (var_s1 == 0) ? 0xB : 0x19 reproduces target and reaches 25.
- verdict: CONFIRMED

## [s9] An int-typed ternary written straight into the s16 var_v0_2, or declaring var_v0_2 itself s32, carries the same mode split without needing an intermediate.
- mechanism: If the RHS is int-typed the constant loads might stay SImode even with an HImode destination; and SImode everywhere would be the natural int spelling.
- probe: var_v0_2 = (var_s1 == 0) ? 0xB : 0x19; with no intermediate; and separately `s32 var_v0_2;` with both copies canonical.
- result: BOTH measure 30 / 211 - the 4-insn cross-jump merge is back. The C conversion to short is folded into the selection so the loads are HImode again; and s32 var_v0_2 moves BOTH copies to SImode so they still match each other. The lever is a mode SPLIT between the copies, not SImode per se.
- verdict: KILLED

## [s9] The cross-jump merge can be defeated STRUCTURALLY - by giving one selection copy its own store and return so the blocks no longer share the block_48 tail - needing no mode trick at all.
- mechanism: find_cross_jump walks backward from two jumps; a store before one jump and a constant load before the other should PAT-MISMATCH on the first compared insn.
- probe: Four spellings (==5 copy direct-store via a fresh s16 local; via the existing outer var_v0; tail copy direct-store; ==5-direct combined with the tail mode split), then the ==5-direct variant re-compiled with the instrumented cc1 under BB2_XJUMP_DEBUG via the s8 isolate/dumpiso recipe.
- result: 29-30 at 211 insns in every spelling - the merge still fires. The trace (tmp/grind/func_800283D0/s9/xjump_P2.log) names the decision: enter e1=372 e2=407 min=2 (chain-partner); MATCH 360/403 set(reg<-11); MATCH 356/399 set; MATCH 353/396 set(reg<-25); LABEL-BONUS 347; DO_CROSS_JUMP jump=372 newjpos=353 newlpos=396. The duplicated store+return tail is already tail-merged back into block_48 by an EARLIER jump pass, so the blocks are identical again by the time jump2 looks. Structural route CLOSED; the mode split is the only known lever for cluster D.
- verdict: KILLED

## [s9] Cluster C (the 4-diff v0-vs-v1 rename at emitted slots 126/132/133/135) is a register-allocation consequence of sched1 hoisting the block's return-value copy to the head of the block.
- mechanism: local-alloc cannot assign a hard register already live across a quantity's range; if (set (reg/i:SI 2 v0) <ret>) is scheduled FIRST, v0 is live from the top of the block and the two HImode constant pseudos must take v1.
- probe: Read the D_800A38A8 block in .combine / .flow (pre-sched1), .sched (post-sched1) and .lreg / .greg, all regenerated with `pwsh tools/grinder/dump.ps1 func_800283D0`.
- result: Pre-sched1 the order is 368 (li HI 1) -> 370 (sh D_800A38A8) -> 373 (li HI -1) -> 375 (sh D_800A3876) -> 378 (set (reg/i:SI 2 v0) (reg/v:SI 79)); .sched:4853 shows sched1 hoists 378 to the FRONT; .greg:3150 then shows (set (reg:HI 3 v1) (const_int 1)). Target has no such copy at the head of that block and its constants take v0.
- verdict: CONFIRMED

## [s9] Deleting the block-local return-value copy - exiting the globals block with `goto block_49;` instead of an inline `return ret;` - frees v0 and closes cluster C.
- mechanism: With no (set (reg/i:SI 2 v0) ...) in the block there is nothing for sched1 to hoist and nothing holding v0 live, so local-alloc gives v0 to the two constant pseudos. block_49 is an already-existing shared `return ret;` label in this body, so the change is plain ordinary C.
- probe: Variant D4_goto49 on top of Q1; sandbox --disable all plus a difflib alignment of the emitted stream against target.
- result: FLOOR 25 -> 23. Slots 131-136 now read `li v0,1 / lui at / sh v0 / li v0,-1 / lui at / sh v0` - target's registers exactly. Cluster C is CLOSED. Cost: build_insns 216 vs target 215 (see the next hypothesis).
- verdict: CONFIRMED

## [s9] The 23-floor body's entire 216-vs-215 delta is one unfilled delay slot, and it is a reorg decision rather than an extra computation.
- mechanism: reorg's steal-from-branch-target delay-slot fill. On the 25-floor body reorg steals the branch target's `li v1,1` into the delay slot of `beqz v0,<globals block>` at slot 125; on the 23-floor body the same candidate is `li v0,1`, a write of the register the branch itself reads, and reorg declines.
- probe: difflib alignment of the D4 stream against target.
- result: The ONLY insertion is a `nop` at slot 126; from 131 onward our stream re-aligns one slot late against target 126..135. Target performs exactly the steal ours declines (`beqz v0,.L800285DC` with `addiu v0,zero,1` in the delay slot), so the fill is reachable in this compiler. The reorg.c/resource.c reason has not been read out yet.
- verdict: CONFIRMED

## [s9] The inline-return -> shared-goto change is a general policy that helps at the other early-exit sites too.
- mechanism: Same argument as the globals block: fewer block-local return-value copies for sched1 to hoist.
- probe: Apply `goto block_49;` to the temp_v1_2 == 0xE early return (E1_goto49b) and to the do_calls return (G2_docalls).
- result: E1 = 38 / 216, a 15-point REGRESSION; G2 = 23 / 216, byte-identical to D4 (jump2 re-merges it). The goto-vs-inline-return choice is PER SITE and must be measured per site; s1's finding that the range-check exit wants an INLINE return still stands.
- verdict: KILLED

## [s9] Cheap source-level reorderings inside the globals block can reach cluster C without changing the exit form.
- mechanism: Swapping the two independent global stores, or returning the literal 1, changes what sched1 sees in the block.
- probe: Store order swapped, and `return 1;`, measured on both the 25- and 23-floor bodies.
- result: The store swap is byte-neutral at 25 (it only swaps which constant reaches v1 first) and WORSE (24) on the 23-floor body, because target stores 1 to D_800A38A8 before -1 to D_800A3876 and source order is emitted order. `return 1;` measures 26 and leaves 126/132/133/135 unchanged - the return-value set is still hoisted by sched1 and still holds v0. Deleting the copy is the only lever.
- verdict: KILLED

## [s10] A fresh m2c re-derivation yields a structurally DIFFERENT C shape that beats the ledger body.
- mechanism: The ledger body descends from an s1-era m2c run that has been hand-edited across nine sessions; a fresh `m2c --target mipsel-gcc-c --valid-syntax` on asm/funcs/func_800283D0.s reproduces the compiler's own view of the CFG and may pick different exit forms, label placement and CSE boundaries.
- probe: Fresh m2c (tmp/grind/func_800283D0/s10/m2c_fresh.c), transcribed to real typed field accesses while KEEPING the two proven levers (SImode `sel` intermediate in the tail selection, `goto block_49;` out of the globals block) = variant R1; then each of the five individual structural deltas measured separately on the 23-floor body (D1..D5).
- result: R1 (all five deltas at once) = 65 / 214 - a 42-point regression. Every delta is individually regressive: D1 hoisting `temp_v0_2 = temp_a1_2 * 2` above the `temp_v1_3 == 0` test = 30 / 217; D2 moving `var_s1 = 0;` above the eight-way range check = 29 / 216; D3 moving the shared store+calls tail from the `temp_v1_3 == 0` arm into the `temp_v1_3 < temp_v0_3` arm (m2c's block_23/block_24 placement, ours' set_0xB/do_store_calls/do_calls placement inverted) = 49 / 216; D4 spelling the range-check exit as `goto block_13;` = 25 / 214; D5 dropping the `ret` carrier for a literal `return 1;` at every exit = 33 / 211. m2c's canonical shape is NOT closer to target than the ledger body on any axis; the ledger body is a strict local optimum against the whole m2c re-derivation.
- verdict: KILLED

## [s10] The exit-form dimension (frontier item 2) still has an unmeasured assignment that reaches below 23.
- mechanism: Each inline `return ret;` creates a block-local `(set (reg/i:SI 2 v0) ...)` that sched1 places; a `goto` to the shared labelled return does not. s9 measured only two of the four sites' sixteen assignments, and target's `.L80028488` is a labelled shared `j / li v0,1` block with three early-exit predecessors, so SOME assignment should reproduce it.
- probe: Complete the enumeration on the 23-floor body. Site 1 = range-check exit (inline / `goto block_13` / `goto block_49`), site 2 = `temp_v1_2 == 0xE` exit, site 3 = do_calls exit, site 4 = globals block. Variants D4, E5, E3, E4 measured this session; sites 2 and 4 carry s9/s8 measurements.
- result: ENUMERATION COMPLETE AND 23 IS THE MINIMUM. Site 1: inline `return 1` = 23 / 216 (BASE), `goto block_13` = 25 / 214 (D4), `goto block_49` = 25 / 214 (E5) - the two goto spellings are score-identical and both trade 2 insns for 2 points. Site 2: inline = 23, goto = 38 (s9, E1). Site 3: inline = 23 / 216 (BASE), `goto block_49` = 23 / 216 (E3, byte-identical - jump2 re-merges it). Site 4: `goto block_49` = 23 (s9), inline `return ret` = 25 (s8). Crossed: site1-goto + site3-goto = 25 / 214 (E4), i.e. the two axes do not interact. No assignment over the four sites goes below 23, and cluster B (slots 45/47) does NOT yield to the exit-form dimension. Frontier item 2 is CLOSED.
- verdict: KILLED

## [s10] The short-circuit `||` in the `== 5` guard can be respelled to change the block layout at the unfilled-delay-slot site.
- mechanism: De Morgan turns `if (A || B) { sel } else { globals }` into `if (!A && !B) { globals } else { sel }`, inverting both branch senses and swapping which block is the beqz's fall-through - which is exactly the geometry that decides reorg's steal-from-target.
- probe: R2 (de Morgan, selection spelled inline in the else) and R3 (de Morgan, selection reached by `goto block_sel;` so the two selection copies stay merged into one).
- result: R2 = 31 / 216, R3 = 27 / 212. Both regress. The short-circuit `||` spelling with the selection in the TRUE arm is pinned; target's `bnez v0,.L800285CC` / `beqz v0,.L800285DC` branch senses only come out of that spelling.
- verdict: KILLED

## [s10] The reorg refusal at the unfilled delay slot (frontier item 1) is a resource conflict with the branch's own condition register.
- mechanism: s9's stated guess was that reorg declines to steal `li v0,1` because the `beqz v0` reads v0. The actual predicate in GCC 2.7.2 reorg.c:3468 is `! insn_sets_resource_p (trial, &opposite_needed, 1)`, where `opposite_needed = mark_target_live_regs (fallthrough_insn)` - i.e. it is about liveness on the FALL-THROUGH path, not about the branch's own read.
- probe: Re-ran the s9 isolate.py/dumpiso.sh recipe on the 23-floor body with BB2_DBR_DEBUG=1 (the instrumented cc1 already carries the DBRDBG hooks in fill_slots_from_thread and mark_target_live_regs). Log: tmp/grind/func_800283D0/s10/iso_dumps/dbr.log.
- result: s9's guess is WRONG in its stated mechanism, and the true predicate is now measured exactly. The branch is insn 344 (`beqz v0 -> label 365`, the globals block). `DBRDBG thr insn=344 thread=368 opp=686 own=1 likely=0 tif=1 oppregs=20630084_00000000 oppmem=1` - 0x20630084 = {v0, a3, s0, s1, s5, s6, sp}, so **v0 is reported LIVE at the fall-through insn 686**. Every one of the four trials in the globals block then loses on `setsopp=1`: 368 (`li v0,1`), 370 (`sh v0,D_800A38A8`), 373 (`li v0,-1`), 375 (`sh v0,D_800A3876`) - the two stores also lose because `oppmem=1`. The fall-through thread was never even scanned (no `thr insn=344 thread=686` line), so `own_fallthrough` was 0. Nothing in that block is stealable while v0 is in opposite_needed. NOTE the open sub-question: the fall-through insn 686 is the sequence at code_label 347 whose delay insn is `li v0,25`, and insn 344 carries `REG_DEAD (reg:SI 2 v0)`, so by the letter of mark_target_live_regs (reorg.c:2709, a CODE_LABEL flushes pending_dead_regs) v0 should have been cleared at label 347. `DBRDBG mtlr target=686 block=28` shows find_basic_block walked back past label 347 to the previous BARRIER and returned the SAME block as insn 339, so the whole scan is one long forward walk - yet the flush at 347 is on that walk. Why v0 survives it is not yet read out.
- verdict: CONFIRMED (the predicate and the measured live set); the sub-question of WHY v0 is live is open.

## [s10] A fresh m2c re-derivation yields a structurally different C shape that beats the nine-session-old ledger body.
- mechanism: The ledger body descends from an s1-era m2c run hand-edited across nine sessions; a fresh m2c run reproduces the compiler's own view of the CFG and may pick different exit forms, label placement and CSE boundaries.
- probe: tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax on asm/funcs/func_800283D0.s; transcribed the whole fresh shape to typed field accesses while keeping the two proven levers (variant R1), then measured each of the five individual structural deltas separately on the 23-floor body (D1..D5) with sandbox --disable all.
- result: R1 (all five deltas at once) = 65 / 214, a 42-point regression. Individually: D1 hoisting temp_a1_2*2 above the temp_v1_3==0 test = 30/217; D2 var_s1=0 written above the eight-way range check = 29/216; D3 store+calls tail owned by the temp_v1_3<temp_v0_3 arm (m2c's block_23/block_24 ownership, ours inverted) = 49/216; D4 range-check exit as goto block_13 = 25/214; D5 no ret carrier, literal return 1 everywhere = 33/211. m2c's canonical shape is not closer to target on any axis. Sibling/transplant route also empty: the five other functions touching D_800A38A8/D_800A3876 (func_8002006C, func_80026DA4, func_800288C8, func_8002AB08, func_8002C61C) are all still INCLUDE_ASM.
- verdict: KILLED

## [s10] Some assignment of goto-vs-inline-return over the four early-exit sites reaches below 23 and closes cluster B (frontier item 2).
- mechanism: Each inline return creates a block-local (set (reg/i:SI 2 v0) ...) that sched1 places and later passes may cross-jump into a shared exit block; a goto to a shared labelled return does not. Target's .L80028488 is a labelled shared j/li v0,1 block with three early-exit predecessors.
- probe: Completed the enumeration on the 23-floor body: site 1 (range-check exit) as goto block_13 (D4) and as goto block_49 (E5); site 3 (do_calls exit) as goto block_49 (E3); the crossed site1-goto + site3-goto (E4). Sites 2 and 4 carry s9/s8 measurements.
- result: Site 1: inline 23 / goto block_13 25 / goto block_49 25. Site 2: inline 23 / goto 38. Site 3: inline 23 / goto 23 (byte-identical, jump2 re-merges it). Site 4: goto 23 / inline 25. Crossed site1+site3 goto = 25/214, so the axes do not interact. No assignment scores below 23 and cluster B (slots 45/47) does not move under any of them. Both site-1 goto spellings trade exactly 2 instructions (216 to 214) for exactly 2 points.
- verdict: KILLED

## [s10] Respelling the short-circuit || in the ==5 guard via de Morgan changes the block layout at the unfilled-delay-slot site favourably.
- mechanism: if (!A && !B) { globals } else { selection } inverts both branch senses and swaps which block is the beqz's fall-through, which is exactly the geometry that decides reorg's steal-from-target.
- probe: R2 (selection inline in the else arm) and R3 (selection reached by goto block_sel so the two selection copies stay merged), both measured with sandbox --disable all on the 23-floor body.
- result: R2 = 31 / 216, R3 = 27 / 212. Both regress. Target's bnez v0,.L800285CC / beqz v0,.L800285DC branch senses only come out of the short-circuit || spelling with the selection in the TRUE arm.
- verdict: KILLED

## [s10] reorg refuses the delay-slot steal at the 216-vs-215 site because the stolen insn writes the register the branch itself reads (s9's stated mechanism).
- mechanism: s9 guessed a conservative resource conflict in resource.c against the branch's own condition register. The actual gate in GCC 2.7.2 reorg.c:3466-3468 is ! insn_sets_resource_p (trial, &opposite_needed, 1), where opposite_needed = mark_target_live_regs (fall-through insn) - i.e. it is about liveness on the FALL-THROUGH path, not about the branch's own read.
- probe: Re-ran the s9 isolate.py/dumpiso.sh recipe on the 23-floor body against the instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_DBR_DEBUG=1; read tmp/grind/func_800283D0/s10/iso_dumps/dbr.log.
- result: s9's stated mechanism is wrong; the true predicate is now measured. Branch = insn 344 (beqz v0 -> label 365, the globals block). Log line: 'DBRDBG thr insn=344 thread=368 opp=686 own=1 likely=0 tif=1 oppregs=20630084_00000000 oppmem=1' - 0x20630084 decodes to {v0, a3, s0, s1, s5, s6, sp}, so v0 is reported LIVE at the FALL-THROUGH insn 686. All four trials in the globals block then lose on setsopp=1 (368 li v0,1; 370 sh v0,D_800A38A8; 373 li v0,-1; 375 sh v0,D_800A3876 - the two stores lose additionally on oppmem=1). There is no 'thr insn=344 thread=686' line at all, so own_fallthrough was 0 and the fall-through thread was never scanned either: the slot is unfillable from both directions while v0 sits in opposite_needed. mostly_true_jump returns 0 here (EQ condition, no rarity difference), so fill_eager_delay_slots takes the prediction<=0 branch.
- verdict: CONFIRMED

## [s11] The rederive shape axis is exhausted: the ledger body's control flow and instruction selection already equal target's, so no C reshaping can move the floor - only register seats can.
- mechanism: An index-aligned objdump diff of the 23-floor body against asm/funcs/func_800283D0.s aligns the two streams instruction-for-instruction. Every surviving difference is a register NAME ($s2/$s3, $a0/$a1), a delay-slot fill decision, or the single extra nop. Instruction selection and CFG are identical, so the remaining distance lives entirely in global.c/local-alloc.c seats and reorg.c fills.
- probe: Built tmp/grind/func_800283D0/s11/mkdiff.py (normalised objdump-vs-target index alignment) and read the full diff; then measured two independent shape rewrites - R1, the eight-way state test respelled in the matched sibling func_8002798C's positive-goto idiom, and R2, `(temp_a1_2 * 2) + arg0` for target's addu operand order.
- result: Both rewrites measured 23 / 216 - byte-for-byte score-identical to the ledger body. The author-idiomatic dispatch spelling is free, and the addu operand order is a consequence of the register assignment rather than an independent diff.
- verdict: CONFIRMED

## [s11] s7's cluster-A allocno windows are chassis-relative and have moved under the s8 mode split and the s9 exit-form change (frontier item 2).
- mechanism: Both changes altered the RTL of blocks the callee-saved quantities live in, and global.c allocno_compare consumes live lengths and reference counts computed on that RTL.
- probe: Re-ran tools/ra_solver/extract.py on the 23-floor body and swept simulate.Sim exhaustively (every allocno x livelen 1..250 x nrefs 1..24) with tmp/grind/func_800283D0/s11/sweep.py, then diffed the result against s7's table.
- result: KILLED - the windows are IDENTICAL to s7's. livelen(73) 92 -> [38,65]; nrefs(73) 7 -> [8,9,10,11]; livelen(143) 14 -> [20,21]; nrefs(143) 3 -> [2]; every other allocno inert at every value; baseline map 72:$s0 77:$s1 143:$s2 73:$s3 75:$s4 90:$s5 79:$s6 unchanged. s7's cluster-A model is LIVE on the 23-floor chassis and may be spent directly. (The cluster-E local-alloc half was not re-measured this session.)
- verdict: KILLED

## [s11] Duplicating a shared call pair into the branch arm that reaches it by goto raises nrefs(arg1) into s7's [8,11] window and flips the $s2/$s3 callee-saved seat.
- mechanism: global.c allocno_compare ranks allocnos by pri = floor_log2(nrefs)*nrefs/allocno_live_length*10000*size. On the 23-floor body pri(143, temp_s3) = 3/14 -> 2142 beats pri(73, arg1) = 14/92 -> 1521, so temp_s3 sorts first and takes $s2. jump.c cross-jumping runs in jump2, AFTER reload, so a source-level duplicate of the two func_80032854 calls is real RTL while global.c sorts and is only re-merged before emission - it raises arg1's reference count without surviving into the bytes.
- probe: Rewrote the `temp_v1_3 < temp_v0_3` arm to carry its own selection, store, copy of both calls and `return ret;` instead of `goto do_calls;` (tmp/grind/func_800283D0/s11/variants/R4_dup_calls_lt_arm.c), measured the sandbox, then re-extracted the ra_solver model on the new body.
- result: CONFIRMED - floor 23 -> 20, build_insns 212. nrefs_flow(73) rose 7 -> 9, the forward model reports `base meets goal? True`, the prologue is now target's (`sw $s2,0x20($sp) / addu $s2,$a1,$zero ... sw $s3,0x24($sp)`), and all ten $s2<->$s3 slots (4, 85, 90, 96, 97, 102, 107, 111, 148, 153) are correct. Cluster A is CLOSED.
- verdict: CONFIRMED

## [s11] The duplication can be spelled so the arm survives cross-jumping the way target's does, recovering the 3 lost instructions.
- mechanism: jump2's find_cross_jump walks the common tail; target's arm survives because its inner branch has the inverted sense (`beqz $s1`) relative to the shared block's (`bnez $s1`), which is what the s9 `goto set_0xB;` spelling produced for free.
- probe: Measured two inverted-sense duplication spellings - R6 (`goto set_0xB;` retained plus duplicated calls) and R7 (duplicate in the 0xB path with `goto do_calls;` kept alive for the 0x19 path) - plus R5, which forces the duplicated arm to be the fall-through via `if (temp_v1_3 >= temp_v0_3) goto ge_block;`.
- result: PARTIAL/KILLED as spelled. R6 = 20 / 219, R7 = 20 / 222, R5 = byte-identical 212. The score is FLAT at 20 across every duplication spelling: keeping the inverted branch stops the over-merge but kills the `do_calls` label, and the `sh $v0,0x286($s0)` store then sinks out of its target position in front of `addiu $a1,$zero,1` into the following jal's delay slot, costing back what the arm gains. The open requirement is now exact: duplicate the calls AND keep the inner branch inverted AND keep a live label between the store and the argument set-up.
- verdict: KILLED

## [s11] Hoisting temp_s3 above the `temp_v1_3 == 0` test so both reads share one pointer lengthens its live range toward s7's livelen(143) 20/21 window.
- mechanism: A single shared pointer local was a plausible original-source shape and would extend the pointer's live range across the branch.
- probe: Built variants/R3_shared_ptr_hoist.c and measured the sandbox.
- result: KILLED - 59 / 215, a 36-point regression. The hoist gives the pointer a FOURTH reference, and at nrefs 4 the model requires livelen(143) in [53,58], which is unreachable from 14. Banked rejected/shared-ptr-hoist-adds-4th-ref-59.c.
- verdict: KILLED

## [s11] The rederive shape axis is exhausted: the ledger body's control flow and instruction selection already equal target's, so no C reshaping can move the floor - only register seats and reorg fills can.
- mechanism: An index-aligned objdump diff of the 23-floor body against asm/funcs/func_800283D0.s aligns the two streams instruction-for-instruction. Every surviving difference is a register NAME ($s2/$s3, $a0/$a1), a delay-slot fill decision, or the single extra nop. Instruction selection and CFG are identical, so the remaining distance lives entirely in global.c / local-alloc.c seats and reorg.c fills.
- probe: Built tmp/grind/func_800283D0/s11/mkdiff.py (normalised objdump-vs-target index alignment) and read the full diff; then measured two independent shape rewrites - R1, the eight-way state test respelled in the matched sibling func_8002798C's positive-goto idiom, and R2, `(temp_a1_2 * 2) + arg0` for target's addu operand order.
- result: Both rewrites measured 23 / 216 - score-identical to the ledger body. The author-idiomatic dispatch spelling is free, and target's `addu $s3,$v0,$s0` operand order is a consequence of the register assignment rather than an independent diff.
- verdict: CONFIRMED

## [s11] s7's cluster-A allocno windows are chassis-relative and have moved under the s8 mode split and the s9 exit-form change (standing frontier item 2).
- mechanism: Both changes altered the RTL of blocks the callee-saved quantities live in, and global.c allocno_compare consumes live lengths and reference counts computed on that RTL.
- probe: Re-ran tools/ra_solver/extract.py func_800283D0 code6cac_b on the 23-floor body and swept simulate.Sim exhaustively (every allocno x livelen 1..250 x nrefs 1..24) with tmp/grind/func_800283D0/s11/sweep.py, then diffed against s7's table.
- result: KILLED - the windows are IDENTICAL to s7's: livelen(73) 92 -> [38,65]; nrefs(73) 7 -> [8,9,10,11]; livelen(143) 14 -> [20,21]; nrefs(143) 3 -> [2]; every other allocno inert; baseline map 72:$s0 77:$s1 143:$s2 73:$s3 75:$s4 90:$s5 79:$s6 unchanged. s7's cluster-A model is LIVE on this chassis and may be spent directly. The cluster-E local-alloc half was not re-measured.
- verdict: KILLED

## [s11] Duplicating a shared call pair into the branch arm that currently reaches it by goto raises nrefs(arg1) into s7's [8,11] window and flips the $s2/$s3 callee-saved seat.
- mechanism: global.c allocno_compare ranks by pri = floor_log2(nrefs)*nrefs/allocno_live_length*10000*size. On the 23-floor body pri(143, temp_s3) = 3/14 -> 2142 beats pri(73, arg1) = 14/92 -> 1521, so temp_s3 sorts first and takes $s2 - a ~40% inversion, not a tie. jump.c cross-jumping runs in jump2, AFTER reload, so a source-level duplicate of the two func_80032854 calls is real RTL while global.c sorts allocnos and is only re-merged before emission: it raises arg1's reference count without surviving into the bytes.
- probe: Rewrote the `temp_v1_3 < temp_v0_3` arm to carry its own selection, store, copy of both func_80032854 calls and `return ret;` instead of `goto do_calls;` (tmp/grind/func_800283D0/s11/variants/R4_dup_calls_lt_arm.c), measured the sandbox, then re-extracted the ra_solver model on the new body.
- result: CONFIRMED - floor 23 -> 20, build_insns 212. nrefs_flow(73) rose 7 -> 9, the forward model reports `base meets goal? True`, the prologue is now target's (`sw $s2,0x20($sp) / addu $s2,$a1,$zero ... sw $s3,0x24($sp)`), and all ten $s2<->$s3 slots (4, 85, 90, 96, 97, 102, 107, 111, 148, 153) are correct. Cluster A is CLOSED.
- verdict: CONFIRMED

## [s11] The duplication can be spelled so the arm survives cross-jumping the way target's does, recovering the 3 instructions the 20-floor body is short.
- mechanism: jump2's find_cross_jump walks the common tail; target's arm survives because its inner branch has the inverted sense (`beqz $s1`) relative to the shared block's (`bnez $s1`), which is what the s9 `goto set_0xB;` spelling produced for free.
- probe: Measured two inverted-sense duplication spellings - R6 (`goto set_0xB;` retained plus duplicated calls) and R7 (duplicate in the 0xB path with `goto do_calls;` kept alive for the 0x19 path) - plus R5, which forces the duplicated arm to be the fall-through via `if (temp_v1_3 >= temp_v0_3) goto ge_block;`.
- result: KILLED as spelled. R6 = 20 / 219, R7 = 20 / 222, R5 = byte-identical 212. The score is FLAT at 20 across every duplication spelling: keeping the inverted branch stops the over-merge but kills the `do_calls` label, and the `sh $v0,0x286($s0)` store then sinks out of its target position in front of `addiu $a1,$zero,1` into the following jal's delay slot, costing back exactly what the arm gains.
- verdict: KILLED

## [s11] Hoisting temp_s3 above the `temp_v1_3 == 0` test so both reads share one pointer lengthens its live range toward s7's livelen(143) [20,21] window.
- mechanism: A single shared pointer local is a plausible original-source shape and would extend the pointer's live range across the branch, which is the cheaper of the two cluster-A dials (+6/+7).
- probe: Built tmp/grind/func_800283D0/s11/variants/R3_shared_ptr_hoist.c and measured the sandbox.
- result: KILLED - 59 / 215, a 36-point regression. The hoist gives the pointer a FOURTH reference, and at nrefs 4 the model requires livelen(143) in [53,58], which is unreachable from 14.
- verdict: KILLED

---

## s12 (2026-08-26) — structural

**CONFIRMED — H-s12-1.** The `<` arm's duplicated call pair and target's 4-insn
arm are NOT mutually exclusive (s11's framing); which of the two survives is
decided by where `find_cross_jump`'s pairwise walk-back stops, and that stop
point is controlled at C level by the INITIALISATION ORDER of the arm's
selection variable relative to path1's. Initialising the arm's variable to 0xB
and overwriting with 0x19 (path1 does the reverse) makes the walk differ one
insn before the branch, so only the tail from the store onward merges.
Probe: V6 = the s11 body with `s16 var_v0_4 = 0xB; if (var_s1 != 0) var_v0_4 =
0x19;`. Result: 20 -> **17**, all twelve $s2/$s3 slots correct AND the arm's
`beqz $s1 / li 0x19 / j / sh` reproduced. Nothing else changed.

**KILLED — H-s12-2.** "A third entry into the shared block that keeps `do_calls`
alive while the calls are still duplicated" (s11's frontier-1 next probe) is not
reachable by any of the enumerable placements. Probes: V2/V3 (arm duplicates only
the first call and re-enters at a `do_call2` label before the second) 22/219 and
22/212; V7 (0xB edge duplicates, 0x19 edge stores then `goto do_calls`) 20/222;
V8 (mirror) 20/219; P2 (path1's store duplicated per edge with a shared
`do_calls`) 19/222. All strictly worse than 17. The label-liveness route to
pinning the store is dead; the store must be pinned by the walk-back stopping one
insn earlier on the 0x19 edge, not by a hand-placed extra entry.

**KILLED — H-s12-3.** A second, independent source of `arg1` references (so the
arm could be spelled with pure gotos and still hold nrefs(73) >= 8) does not
exist in this function. Probe: P1 duplicates path1's OWN store+calls into its two
selection edges — the only other site whose emitted bytes could re-merge —
29/224. V9 confirms the complementary half: the inverted initialisation without
the duplicate scores 25, i.e. the inverted init is a walk-back stop, not a dial.

**KILLED — H-s12-4.** The `addu $s3,$v0,$s0` operand order and the tail's
declaration order are not levers on this chassis. Probes: Q1
`(temp_a1_2 * 2) + arg0` — 17, byte-for-byte neutral (re-confirms s11's R2 after
the chassis moved); Q4 computing the Judge product before the `+0x118` load — 17,
neutral, cluster E unmoved.

**CONFIRMED (methodology) — H-s12-5.** The unnormalized objdump-vs-target diff
used by s11 is unreadable (every `li`/`addiu`, `move`/`addu`, hex/dec and
`%hi()`/`lui` pair reads as a divergence). `tmp/grind/func_800283D0/s12/norm.py`
canonicalises both sides first and resolves the residual into named clusters that
sum to the score. Every future probe on this function should use
`bash tmp/grind/func_800283D0/s12/dif.sh`.

## [s12] The '<' arm's duplicated func_80032854 pair (needed as the only reachable dial on nrefs_flow(pseudo 73 = arg1), 7 -> 9, into s7's [8,11] window) and target's surviving 4-instruction arm are not mutually exclusive; which of the two survives is decided by where find_cross_jump's pairwise walk-back stops, and that stop point is controlled at C level by the initialisation order of the arm's selection variable relative to path1's.
- mechanism: jump.c find_cross_jump compares the two blocks' insns pairwise from their ends and stops at the first difference. With path1 spelled 'v0=0x19; if (s1==0) v0=0xB;' and the arm spelled 'v0=0xB; if (s1!=0) v0=0x19;', the post-reload streams are 'li v0,0x19 / bne s1,0,Lsh / li v0,0xB / Lsh: sh / calls / j end' versus 'li v0,0xB / beq s1,0,Lsh / li v0,0x19 / Lsh: sh / calls / j end'. The walk matches 'j end', both calls and the 'sh', then compares 'li v0,0x19' against 'li v0,0xB', differs and stops - so only the tail from the store onward merges and the arm keeps its two-insn selection plus the redirected (hence inverted) branch. With s11's spelling both blocks initialised to 0x19, so the walk also matched the branch and the 'li' and the whole arm collapsed to a single 'bnez', losing 3 instructions. The duplicate still survives to global-alloc, which is where the nrefs dial is read.
- probe: V6 = the s11 candidate with the arm's 's16 var_v0_4 = 0x19; if (var_s1 == 0) var_v0_4 = 0xB;' replaced by 's16 var_v0_4 = 0xB; if (var_s1 != 0) var_v0_4 = 0x19;'. Nothing else changed. sandbox func_800283D0 --disable all, then the normalized diff (tmp/grind/func_800283D0/s12/dif.sh).
- result: score 20 -> 17, 216 insns (was 212). All twelve $s2/$s3 slots (emitted 3, 4, 10, 85, 90, 96, 97, 102, 107, 111, 149, 154) now correct, AND the arm reproduces target's 'beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)'.
- verdict: CONFIRMED

## [s12] s11's frontier-1 next probe - keep a THIRD entry into the shared call block so the do_calls label survives while the calls are still duplicated, thereby pinning the 'sh $v0,0x286($s0)' store in front of the argument set-up - is reachable by some placement of the duplicate.
- mechanism: The store sinks into the jal's delay slot because nothing separates it from 'addiu $a1,$zero,1'; in target .L80028520 sits between them. If a third C-level entry point kept that label alive, sched2 could not move the store past the argument set-up (the inverse of defer-store-past-later-compute-into-jal-delay).
- probe: Five placements measured on this chassis: V2 (arm duplicates only the FIRST call and re-enters at a do_call2 label placed before the second call) 22/219; V3 (same with a selection variable) 22/212; V7 (0xB edge duplicates store+calls, 0x19 edge stores then 'goto do_calls') 20/222; V8 (mirror: 0xB edge 'goto set_0xB', 0x19 edge duplicates) 20/219; P2 (path1's store duplicated per selection edge with a shared do_calls) 19/222.
- result: Every placement scores strictly worse than 17 (19-22 vs 17). The hand-placed extra entry either re-merges the arm anyway or blocks the merge entirely and adds 3-7 instructions.
- verdict: KILLED

## [s12] A second, independent source of arg1 references exists in this function, so the arm could be spelled with pure gotos (target's exact bytes) while nrefs_flow(73) still reaches 8.
- mechanism: nrefs(73) >= 8 is what inverts allocno_compare's priority ordering (pri = floor_log2(nrefs)*nrefs/live_length) so arg1 sorts before temp_s3 and takes $s2. Any C site whose emitted bytes re-merge in jump2 would add references at global-alloc time without costing instructions.
- probe: V1 (arm spelled purely with 'goto set_0xB' + 'goto do_calls', no duplicate) - measured, then tools/ra_solver/extract.py + the s11 single-atom sweep re-run on it. P1 duplicates path1's OWN store+calls into its two selection edges, the only other site whose bytes could re-merge. V9 isolates the inverted initialisation without any duplicate.
- result: V1: 23/216 - the arm's bytes are EXACTLY target's but nrefs_flow(73) falls back to 7, the sim reports 'base meets goal? False', and all twelve $s2/$s3 slots invert. P1: 29/224. V9: 25/216. No second source exists; the inverted initialisation is a walk-back stop, not a dial in its own right.
- verdict: KILLED

## [s12] The 'addu $s3,$v0,$s0' operand order and the tail block's declaration order are independent levers on the current chassis.
- mechanism: s11's R2 measured the operand order neutral at floor 23; the chassis has since moved twice, and cluster E (the tail $a0/$a1 exchange) is a local-alloc seat that declaration order plausibly reaches via qty birth/death spans.
- probe: Q1 'u8 *temp_s3 = (temp_a1_2 * 2) + arg0;' instead of 'arg0 + (temp_a1_2 * 2)'. Q4 moves the temp_v1_5 Judge product above the '*(s32 *)(temp_a0 + 0x118)' load in declaration order.
- result: Q1: 17, byte-for-byte neutral - the operand order is a consequence of register assignment, re-confirmed on this chassis. Q4: 17, neutral - cluster E unmoved.
- verdict: KILLED


## s13 (structural, 2026-08-26)

**H-s13-1 - KILLED (as stated).**  "The store sink is the same jump2
find_cross_jump walk-back mechanism s12 exploited, one instruction further
along."  Probe: `pwsh tools/grinder/dump.ps1 func_800283D0` on the banked
17-floor body, then track the store insn's chain position across `.combine`,
`.flow`, `.sched`, `.jump2`.  Result: the store is block-head in `.combine` and
`.flow` and already below all four argument set-ups in `.sched`; jump2's anchor
(label 223 = `do_store_calls`) inherits that position.  Verdict: KILLED - the
pass is sched1, not jump2.  See E-s13-1.

**H-s13-2 - CONFIRMED.**  "A source-level label immediately after the store
pins it, because sched1 cannot move an insn across a basic-block boundary and
jump2 (which could manufacture such a label) runs after sched1."  Probe:
variant A, `<` arm = `if (var_s1 == 0) goto set_0xB; store 0x19; goto
do_calls;`.  Result: emitted slots 83-88 and 144-147 become byte-identical to
target - the store sits at the head of the shared block, `move a3,zero` fills
the jal delay slot, and the arm is target's exact four instructions.  Verdict:
CONFIRMED.  See E-s13-2.

**H-s13-3 - CONFIRMED (as a negative coupling).**  "The pinning `goto` and the
arm's duplicated call pair are mutually exclusive in one edge, so the store pin
and cluster A's nrefs dial cannot both be paid by the `<` arm's 0x19 edge."
Probe: A (23/216, pin held / seats lost), B (20/219), C (20/222, pin AND seats
held but +6 insns), F (27/223, refs restored from path1's 0x19 edge).  Verdict:
CONFIRMED.  See E-s13-3..5.

**Open sub-question created by C (highest-value probe left on the store
cluster).**  Variant C proves the two requirements are simultaneously
satisfiable; the whole remaining cost is that jump2 merges its 0xB edge down to
three instructions instead of zero.  Target pays zero for that edge because it
jumps INTO path1's `li v0,0xB` (`.L80028518`).  So the question is no longer
"can we have both" but "can the arm's 0xB edge supply two C-level `arg1`
references while still merging to a bare jump into path1's selection".  A
`goto set_0xB` merges to zero but supplies no refs; a duplicated call pair
supplies the refs but costs three.  Any third site for the two refs must be
byte-neutral - F shows path1's own 0x19 edge is not.

## [s13] The store sink into the jal delay slot is the same jump2 find_cross_jump walk-back mechanism s12 exploited, one instruction further along.
- mechanism: s12 frontier item 1 claimed the arm's 0x19 edge merging AT the store (rather than after it) leaves the only live label before the `sh`, letting sched2/reorg sink it.
- probe: pwsh tools/grinder/dump.ps1 func_800283D0 on the banked 17-floor body; tracked the store insn's RTL chain position (prev-insn field) across .combine, .flow, .sched and .jump2 in tmp/grind/func_800283D0/dumps/.
- result: In .combine and .flow the store is `(insn 226 223 227 ...)` - prev is code_label 223 (`do_store_calls`), i.e. block head. In .sched it is `(insn 226 240 242 ...)` - prev is the `a3=0` argument set-up. jump2's merge anchor is label 223, which by then already precedes the argument set-up.
- verdict: KILLED

## [s13] A label that already exists when sched1 runs - i.e. a SOURCE-level goto, since jump2 is downstream of sched1 - placed immediately after the store pins it at the head of the shared block and reproduces target's ordering.
- mechanism: sched1 (schedule_insns, pre-reload) schedules per basic block; a code_label ends the block, so the store cannot be moved below the argument set-up. In target the pin is .L80028520, the re-entry point of the `<` arm's 0x19 edge.
- probe: Variant A: `<` arm spelled `if (var_s1 == 0) goto set_0xB; *(s16*)(arg0+0x286) = 0x19; goto do_calls;`. Applied to src/code6cac_b.c, scored with sandbox --disable all, verified by direct objdump of tmp/sandbox/func_800283D0/code6cac_b.o.
- result: Emitted slots 83-95 and 144-157 become byte-identical to target: `sh v0,0x286(s0)` at the head of the shared block, `move a3,zero` in the jal delay slot, and the arm emits target's exact four instructions `beqz s1,.L80028518 / li v0,0x19 / j .L80028520 / sh v0,0x286(s0)`. Two clusters closed at once. Total 23/216 - see the next hypothesis for why it is not lower.
- verdict: CONFIRMED

## [s13] The pinning goto and the arm's duplicated func_80032854 pair are mutually exclusive on the same edge, so the store pin and cluster A's nrefs dial cannot both be paid by the `<` arm's 0x19 edge.
- mechanism: `goto do_calls` is exactly the construct that removes the arm's two C-level references to arg1, dropping nrefs_flow(pseudo 73) from 9 to 7 and out of s7's [8,11] window; a label manufactured by jump2 cross-jumping a duplicated call pair arrives after sched1 and pins nothing.
- probe: Four measured spellings, all banked in memory/grind/func_800283D0/rejected/: A (goto set_0xB + own store + goto do_calls), B (var_v0=0xB + goto do_store_calls; 0x19 edge stores + duplicates the calls), C (0x19 edge stores + goto do_calls; 0xB edge stores + duplicates the calls), F (variant A's arm plus path1's own 0x19 selection edge duplicating the call pair).
- result: A = 23/216 (pin held, all twelve seats revert to $s3). B = 20/219 (jump2-made label, no pin). C = 20/222 (pin AND $s2 seats both held - the only body so far to do so - but jump2 merges its 0xB edge only down to three insns). F = 27/223 (path1's duplicated 0x19 edge does not merge).
- verdict: CONFIRMED


## s14 (synthesis, 2026-08-26) - measured

## [s14] Cluster E's local-alloc birth window is unreachable by declaration reordering because a NAMED POINTER LOCAL pins its own RTL to the head of its block; deleting the local lets CSE birth the pseudo at first use and reaches the window.
mechanism: local-alloc.c block_alloc ranks quantities by qty_compare and assigns
by find_free_reg over regs_live_at[birth..death]; the tail pointer quantity's
span (30) loses $a0 to the Judge element. inverse.py local --block 42 --swap 0,3
gives birth in [8,19] as the complete birth-end vector set. A named local's
initializer is emitted where it is declared, at the block head, so the best any
reordering achieves is birth 6 (s7's B_ptr_late). Without the local, the pointer
pseudo is created by CSE at its first USE, which the C can place after the two
(&Judge)[...] index computations.
probe: delete `u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);`, spell the +0x114,
+0x11C and +0x118 reads from `temp_s4 + (temp_s5 * 0x10) + <off>`, and write
temp_v1_4 and the temp_v1_5 product before the +0x118 read; sandbox twice, then
local_extract.py + QTYDBG on block 42.
result: 17 -> 11 / 216. QTYDBG block 42 main pass: pointer quantity birth 2 -> 12,
span 30 -> 20, got $a0 (target); Judge element got $a1 (target). Cluster E gone
from the normalized diff.
verdict: CONFIRMED

## [s14] Cluster A ($s2/$s3) might be reachable through a preference or conflict perturbation rather than the ref-count dial, which no session had tested.
mechanism: global.c's find_reg consults copy preferences and the conflict graph
after priority ordering, so a change in either could flip the 73/143 exchange
without touching reg_n_refs or reg_live_length.
probe: inverse.py global on variant A's model with --swap 73,143 --depth 3 --top 30.
result: exactly five vectors returned, all ref-count atoms: refs(73) 7->8/9/10/11
and refs(143) 3->2. No pref, conflict or birth-order vector at any depth <= 3.
verdict: KILLED

## [s14] refs(143) 3 -> 2 - the cheapest cluster-A atom - is reachable by recomputing temp_s3's address at the `== 5` test instead of reusing the cached pointer.
mechanism: dropping one reference to pseudo 143 raises 73 above it in
allocno_compare's priority order, which is atom #2 of the inverse solver's set.
probe: spell the `== 5` test as `*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288)` on
both the 17-floor body and variant A's body; sandbox twice each, then extract.py
and read flow[143].nrefs_flow.
result: 17 (neutral) and 23 (neutral); nrefs_flow(143) still 3. CSE re-merges the
recomputed address into the same pseudo before flow.c counts references.
verdict: KILLED

## [s14] Closing cluster E changes the balance of the store-pin / cluster-A tension, so variant A's pinning arm may now be worth more than the duplicated call pair.
mechanism: variant A's `goto do_calls` supplies the pre-sched1 basic-block
boundary that stops the store sinking (E-s13-1/2) but removes the arm's two
arg1 references, dropping nrefs_flow(73) 9 -> 7 and losing all twelve $s2 seats.
probe: t4 = variant A's arm + the no-pointer tail; sandbox twice.
result: 17 / 216 versus t3's 11 / 216. The pin is still exactly -6 / +6. The
coupling is unaffected by the tail fix.
verdict: KILLED (for this chassis; the tension is unchanged, not resolved)

## [s14] The two commutative `addu` operand-order diffs left at 11 (slots 96 and 162) are independent C levers.
mechanism: the emitted operand order of a commutative addu follows the RTL PLUS
operand order, which follows the C expression's operand order.
probe: `(temp_a1_2 * 2) + arg0`, `(temp_s5 * 0x10) + temp_s4`, and both together,
on the 11-floor body; sandbox twice each.
result: 11, 11, 11 - all neutral. Extends s11's R2 / s12's Q1 to the tail site.
verdict: KILLED

## Frontier (for session 15)

## [s14] The store sink is the largest remaining cluster (~4 pts) and its only known C form costs exactly as much in cluster A as it gains; a THIRD incoming edge to the `do_calls` label - one that does not come from the `<` arm - would pay for the pin without spending the arm's duplicated calls.
mechanism: sched1 sinks `sh $v0,0x286($s0)` past the four argument set-ups
because store and set-ups share one basic block (E-s13-1, dumps read). A
pre-sched1 label between them requires a source `goto do_calls` from somewhere.
Target has exactly one such edge and it is the arm's (E-s14-4), which is also the
only site that can supply refs(73) 7 -> 9. The two known third sites are already
dead: path1's own 0x19 selection edge (s13's F, 27/223) and path1's store
duplicated per edge (s12's P2, 19/222).
next probe: the untried third site is the `==` block, whose call pair is the SAME
(a1=1, a1=0x25) pair as the shared block's. Restructure so the `==` block reaches
the shared calls by a source goto and returns to its continuation - e.g. by
moving the `== 5` test and everything after it into the path that currently falls
out of the shared block, so the shared call site has two source predecessors and
the arm can keep its duplicate. Score every candidate with two sandbox runs then
tmp/grind/func_800283D0/s12/dif.sh (E-s13-6 staleness note applies).

## [s14] Cluster B (~4 pts) is the whole 216-vs-215 insn surplus and is the only residual with NO measured C dial after fourteen sessions; s10's open sub-question about `opposite_needed` at insn 686 is still the highest-value forensics probe left.
mechanism: target fills the dispatch chain's last `beq $v0` delay slot with the
`li $v0,0x1` belonging to the fall-through block - speculating a value into the
register the branch just read - and leaves the following `j` unfilled; we do the
reverse at emitted 45-48 and again at 126/131. reorg.c's mark_target_live_regs /
opposite_needed decides this. s10 proved it does not move under any of the
sixteen exit-form assignments.
next probe: locate the dispatch chain's final `beq` insn UID in
tmp/grind/func_800283D0/s10/iso_dumps/iso.dbr, grep that UID in
tmp/grind/func_800283D0/s10/iso_dumps/dbr.log for its thr/simp lines, and settle
whether the steal is granted there for the same reason it is refused at insn 344.
Re-run the dumps on the NEW 11-floor body first - the tail rewrite changed the
RTL of every block after the dispatch chain.

## [s14] The named-local-pins-birth-order finding (E-s14-1) is a GENERAL local-alloc lever that has never been applied anywhere else in this function, and cluster A's own participants are locals of exactly the same kind.
mechanism: pseudo 143 = `temp_s3 = arg0 + temp_a1_2 * 2` is a named pointer local
whose RTL is emitted at the head of its block. Its global-alloc inputs are
livelen 14/22 and nrefs 3, and s7's sweep gives a livelen window of [20,21] with
nrefs held at 3 - a +6/+7 lengthening that no session has reached because every
attempt to hoist the declaration added a fourth reference
(rejected/hoist-temp-s3-cses-arm-addu.c, shared-ptr-hoist-adds-4th-ref-59.c).
next probe: apply the inverse of E-s14-1 - since deleting a named local moves a
pseudo's birth LATER, adding one where the C currently inlines an expression
should move a birth EARLIER. The `temp_v1_3 == 0` path already recomputes
`arg0 + temp_a1_2 * 2` inline; check with extract.py whether that recompute is a
separate pseudo on the 11-floor body, and if so whether unifying the two under
one earlier-born pointer reaches livelen(143) 20/21 with nrefs still 3 rather
than 4. Verify nrefs from extract.py BEFORE spending a sandbox run.

## [s14] Cluster E's local-alloc birth window is unreachable by declaration reordering because a named pointer local pins its own RTL to the head of its block; deleting the local lets CSE birth the pseudo at first use and reaches the window.
- mechanism: local-alloc.c block_alloc assigns by find_free_reg over regs_live_at[birth..death]; the tail pointer quantity's span 30 loses $a0. inverse.py local --block 42 --swap 0,3 gives birth in [8,19] as the complete birth-end vector set. A named local's initializer is emitted where declared, at the block head (best any reorder achieves is birth 6 = s7's B_ptr_late). Without the local, CSE creates the pointer pseudo at its first USE, which the C can place after both (&Judge)[...] index computations.
- probe: Delete `u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);`; spell the +0x114/+0x11C/+0x118 reads from `temp_s4 + (temp_s5 * 0x10) + <off>`; write temp_v1_4 and the temp_v1_5 product before the +0x118 read. sandbox --disable all twice, then local_extract.py QTYDBG on block 42.
- result: 17 -> 11 / 216. QTYDBG block 42 main pass: pointer quantity birth 2 -> 12, span 30 -> 20, got $a0 (target); Judge element got $a1 (target). Cluster E gone from the normalized diff.
- verdict: CONFIRMED

## [s14] Cluster A ($s2/$s3) may be reachable through a preference or conflict perturbation rather than the ref-count dial - an axis no session had tested.
- mechanism: global.c's find_reg consults copy preferences and the conflict graph after priority ordering, so a change in either could flip the 73/143 exchange without touching reg_n_refs or reg_live_length.
- probe: inverse.py global on variant A's model, --swap 73,143 --depth 3 --top 30.
- result: Exactly five vectors, all ref-count atoms: refs(73) 7->8/9/10/11 and refs(143) 3->2. No pref, conflict or birth-order vector at any depth <= 3.
- verdict: KILLED

## [s14] refs(143) 3 -> 2, the cheapest cluster-A atom, is reachable by recomputing temp_s3's address at the `== 5` test instead of reusing the cached pointer.
- mechanism: Dropping one reference to pseudo 143 raises 73 above it in allocno_compare's priority order (inverse solver atom #2).
- probe: Spell the `== 5` test as `*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288)` on both the 17-floor body and variant A's body; sandbox twice each; extract.py and read flow[143].nrefs_flow.
- result: 17 (neutral) and 23 (neutral); nrefs_flow(143) still 3 - CSE re-merges the recomputed address into the same pseudo before flow.c counts references.
- verdict: KILLED

## [s14] Closing cluster E changes the balance of the store-pin / cluster-A tension, so variant A's pinning arm may now be worth more than the arm's duplicated call pair.
- mechanism: variant A's `goto do_calls` supplies the pre-sched1 basic-block boundary that stops the store sinking (E-s13-1/2) but removes the arm's two arg1 references, dropping nrefs_flow(73) 9 -> 7 and losing all twelve $s2 seats.
- probe: t4 = variant A's arm + the no-pointer tail; sandbox twice.
- result: 17 / 216 versus t3's 11 / 216 - the pin is still exactly -6 (cluster A) / +6 (store region). The coupling is unaffected by the tail fix.
- verdict: KILLED

## [s14] The two commutative `addu` operand-order diffs left at 11 (emitted slots 96 and 162) are independent C levers.
- mechanism: The emitted operand order of a commutative addu follows the RTL PLUS operand order, which follows the C expression's operand order.
- probe: `(temp_a1_2 * 2) + arg0`, `(temp_s5 * 0x10) + temp_s4`, and both together, on the 11-floor body; sandbox twice each.
- result: 11, 11, 11 - all neutral. Extends s11's R2 / s12's Q1 to the tail site.
- verdict: KILLED


## s15 (synthesis) - frontier reset

**KILLED this session**
- H-s15-a: "livelen(143) can be lengthened by hoisting temp_s3 to the head of
  block_20 so its live range covers path1."  KILLED - 53 / 215; CSE unifies the
  hoisted address with path1's own `arg0 + temp_a1_2*2 + 0x288`, pseudo 143
  leaves the global allocno set entirely and path1 loses an insn.
  (rejected/s3-hoist-block20-drops-143-from-global-53.c)
- H-s15-b (RE-FRAMED, not killed by measurement but by deduction from target's
  own asm): "the store pin and cluster A are genuinely coupled, so the way
  forward is a third source edge into `do_calls` that pays for the arm's
  duplicated call pair" (s14 frontier item 1).  Target's `.L80028520` is a
  source label and its arm carries no duplicate, so target compiles with
  nrefs(73) = 7 and wins the seat anyway.  There is no third edge to find; the
  coupling is an artefact of OUR ref-count workaround.  De-prioritised.

**CONFIRMED this session**
- H-s15-c: cluster A's decision is `pri(73) > pri(143)` under global.c's
  `allocno_compare`, with pri = floor_log2(nrefs)*nrefs/livelen*10000*size.
  Verified digit-for-digit against ALLOCDBG on the store-pinned body.

**Frontier for the next pass (in priority order)**

1. **livelen(143) 14 -> >= 20 from the DEATH end, on variant A's (store-pinned)
   body.**  This is the only route consistent with target's own asm (E-s15-5)
   and it closes the store sink and cluster A together, leaving only cluster B.
   The birth end is CSE-foreclosed (E-s15-6); the death end is untried.
   Concrete probes: (i) move the post-call `*(s16 *)(temp_s3 + 0x288) == 5`
   re-read later inside the `==` block - e.g. compute the two `0xE` window
   tests into named locals FIRST and test `== 5` afterwards where the semantics
   permit (the `||` is side-effect free, so hoisting both reads above the `== 5`
   test is a legal reordering; check the emitted order against target lines
   48-63 before spending a sandbox run); (ii) give temp_s3 a genuinely later
   REAL use so the range extends - but note nrefs would then be 4, which by the
   closed form needs livelen >= 53, so (ii) is only viable if the extra use
   also extends the range that far.  Read ALLOCDBG (extract.py) for
   nrefs/livelen/pri BEFORE spending a sandbox run: the pass/fail condition is
   now arithmetic.

2. **Cluster B (~4 pts, and the entire 216-vs-215 surplus).**  Emitted 126/131:
   target fills the `beqz $v0,.L800285DC` delay slot with `addiu $v0,0x1`, the
   first insn of the BRANCH TARGET block (`steal_delay_list_from_target`), and
   we emit a `nop` there and the `li v0,1` afterwards.  The insn order in the
   target block is identical in both builds, so the refusal is in reorg.c's
   eligibility test, not in the C statement order: the two candidates are
   `LABEL_NUSES(.L800285DC) != 1` (dbr must redirect the branch to label+1 and
   will not if the label has other references) and `mark_target_live_regs`
   believing `$v0` live on the fall-through path.  Probe: re-run
   `pwsh tools/grinder/dump.ps1 func_800283D0` on the 11-floor body, find that
   branch's UID in the `.dbr` dump and grep the DBRDBG log for its thr/simp
   lines.  Emitted 45-48 (ours `j / nop`, target `nop / j`) is the same pass.

3. **The arm's walk-back stop point (~2 pts), independent of both above.**
   On the 11-floor body BOTH arm edges land on 0x6a8 because jump2's walk-back
   swallowed the arm's own store; target's 0x19 edge keeps its store and lands
   at `.L80028520`.  s12 already proved the walk-back's stopping point is
   steered by the initialisation order of the arm's selection variable relative
   to path1's, so this is a live, cheap dial that has been swept only once (at
   floor 20, before the tail rewrite).  Re-sweep the four init-order
   combinations on the 11-floor body and read the arm's two branch targets out
   of the objdump rather than the normalized diff.

## [s15] Cluster A ($s2/$s3) is decided by GCC 2.7.2 global.c allocno_compare, so the seat exchange is exactly the inequality pri(73) > pri(143) with pri = floor_log2(nrefs)*nrefs/livelen*10000*size, and the complete single-input solution set is nrefs(73)>=8, nrefs(143)=2, livelen(143)>=20, livelen(73)<=65.
- mechanism: global.c:allocno_compare sorts allocnos by descending pri and find_reg assigns in that order; the two participants' pris were read out of the instrumented ALLOCDBG dump of the store-pinned body (t4 = variant A's arm + s14's no-pointer tail) and reproduce the formula exactly: 143 = 1*3/14*10000 = 2142 (ord 19 -> $s2), 73 = 2*7/92*10000 = 1521 (ord 21 -> $s3).
- probe: Read tools/gcc-2.7.2/global.c allocno_compare verbatim; ran tools/ra_solver/extract.py on the t4 body (tmp/grind/func_800283D0/s15/model_t4.json) and compared the dumped pri values against the formula; cross-checked with inverse.py global --swap 73,143 --depth 2 --top 40 (tmp/grind/func_800283D0/s15/inv_t4.txt).
- result: Formula matches the dump digit-for-digit for both allocnos. inverse.py returns six single-atom vectors: refs_up 73 -> 8/9/10/11, refs_down 143 -> 2, and live_extend 143 14 -> 22. This supersedes s7's empirical '[8,11] window' framing with the underlying arithmetic and corrects s14's E-s14-2 claim that only ref-count atoms exist (that run predated the tail rewrite).
- verdict: CONFIRMED

## [s15] Target reaches the $s2 seat with nrefs(arg1) = 7 - i.e. through livelen(temp_s3) >= 20, not through a duplicated call pair - so the store pin and cluster A are NOT coupled in target's source and there is no 'third do_calls edge' to find (s14 frontier item 1 is de-prioritised).
- mechanism: Direct objdump of the 11-floor body's arm (0x7a0-0x7ac) shows BOTH edges landing on the same address 0x6a8: jump2's cross-jump walk-back swallowed the arm's own `sh $v0,0x286($s0)` together with the duplicated call pair. Target's arm keeps its own store and lands one insn further on, at .L80028520. A walk-back that does not stop in front of an identical store in our build cannot have stopped in front of one in target's, so .L80028520 was already a label before jump2; and a label that also survives sched1 as a block boundary (which is what keeps target's store at the head of the shared block, E-s13-1) can only come from a source goto. Hence target's arm is variant A's arm, which supplies zero duplicated arg1 references, and target's final seven $s2 references are also its pre-jump2 count.
- probe: mipsel-linux-gnu-objdump of tmp/sandbox/func_800283D0/code6cac_b.o at the 11-floor body vs asm/funcs/func_800283D0.s .L80028608; combined with the closed form of hypothesis 1 and with E-s15-3 (livelen(73)=92 is a structural floor because 73 is the arg1 parameter, live from its prologue home to the last of three call sites).
- result: Consistent only with livelen(143) >= 20. The correct attack becomes 'lengthen pseudo 143's live range from 14 to >= 20 with nrefs held at 3, on top of variant A's arm', which would close the store sink (~4 pts) and cluster A together and leave only cluster B.
- verdict: CONFIRMED

## [s15] livelen(143) can be lengthened from the BIRTH end by hoisting `u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` to the head of block_20 so its range covers path1's block (+~7 insns, exactly the required lengthening).
- mechanism: reg_live_length is summed over the basic blocks in which the pseudo is live; making temp_s3 dominate path1 adds path1's ~7 insns to its range without adding a use.
- probe: Probe h1 (tmp/grind/func_800283D0/s15/probes/h1_s3_hoist_block20.c) applied to the store-pinned t4 body; two sandbox --disable all runs plus a fresh extract.py ALLOCDBG dump (tmp/grind/func_800283D0/s15/model_h1.json).
- result: 53 / 215. Pseudo 143 disappears from the global allocno set entirely - CSE unifies the hoisted address with path1's own `arg0 + temp_a1_2*2 + 0x288`, the merged pseudo drops to local-alloc, and path1 loses its own `addu` (215 vs 216). Pseudo 73 stays at ord 21 / $s3. The birth end is CSE-foreclosed for the same reason the refs(143) 3->2 dial is (E-s14-3); the death end remains untried.
- verdict: KILLED


## FRONTIER RESET - s16 (2026-08-26), floor 10 / 216

s15's frontier item 1 (lengthen `livelen(temp_s3)` from the death end to >= 20
on top of variant A's store-pinned arm) is **RETIRED, not carried forward**:
E-s16-3 shows target reaches the cluster-A seat with `nrefs(73) = 9`, exactly
as the current candidate does, so there is nothing left to buy on that dial and
the store-pin / cluster-A tension that motivated it does not exist in target's
source.  s15's item 2 (cluster B / reorg) and item 3 (the arm's jump2 walk-back
stop point) both survive and are re-stated below against the new body.

1. **The arm's jump2 walk-back consumes path1's store on BOTH edges; target
   keeps it on the 0x19 edge.  Stopping it one insn earlier closes emitted 148,
   emitted 83-88, or both (~3-4 pts).**
   *Mechanism.*  On the 10-floor body the arm is
   `beqz s1,T / li v0,25 / j T / li a1,1` - both edges land before path1's
   `sh $v0,0x286($s0)`, so the arm owns no store and reorg fills its `j` delay
   slot by stealing `li a1,1` from the branch target block.  Target's arm is
   `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)`:
   the 0x19 edge stops in front of the store, keeps it, and rejoins one insn
   later.  A stop one insn earlier also puts a label immediately after path1's
   store, which would end that basic block and prevent sched1 from sinking the
   store into the first jal's delay slot (emitted 83-88, the ~2-pt store sink).
   s12 proved the stopping point is steered by the initialisation order of the
   arm's selection variable relative to path1's - that is what took the floor
   20 -> 17 - and that dial has never been swept on an arm carrying the
   `goto set_0xB` shape.
   *Next probe.*  On `candidate.c`, sweep the four init-order / branch-sense
   combinations of the arm's `s16 var_v0_4` against path1's `var_v0`
   (`= 0x19` + `if (var_s1 == 0) goto set_0xB` is the current one; also try
   `= 0xB` + `if (var_s1 != 0)` with a `set_0x19` label in path1, and both
   with path1's own order inverted).  Read the arm's TWO branch target
   ADDRESSES straight out of
   `mipsel-linux-gnu-objdump -d tmp/sandbox/func_800283D0/code6cac_b.o`
   (offsets around 0x7a0-0x7b0) rather than out of the normalized diff, which
   hides which address each edge lands on - that is the whole signal.  Note the
   E-s13-6 staleness trap: run `sandbox --disable all` twice before objdumping.
   `tmp/grind/func_800283D0/s16/mk.sh` regenerates the normalized diff and
   `al.py` re-checks that the seats did not move.

2. **Cluster B (~4 pts, and the entire 216-vs-215 insn surplus) is a reorg.c
   `steal_delay_list_from_target` refusal at the `beqz $v0,.L800285DC` branch,
   not a C statement-order question.**
   *Mechanism.*  Target steals `addiu $v0,$zero,0x1` - the first insn of the
   branch TARGET block - into that delay slot and redirects the branch past it;
   our build has the same insn order in the target block yet emits a nop and
   the `li v0,1` afterwards.  dbr will only steal from a target block when it
   can redirect the branch to label+1, which it refuses when `LABEL_NUSES != 1`,
   and when `mark_target_live_regs` proves the clobbered register dead on the
   fall-through path.  Emitted 45-48 (ours `j / nop`, target `nop / j`) is the
   same pass.
   *Next probe.*  Run `pwsh tools/grinder/dump.ps1 func_800283D0` on the
   10-floor body (every dump older than s14 is stale - the tail rewrite and now
   the arm rewrite changed the RTL of the blocks after the dispatch chain),
   locate that branch's insn UID in the `.dbr` dump, and grep the DBRDBG log for
   its thr/simp lines to settle which of the two refusals fires.  If it is
   LABEL_NUSES, the C dial is the number of source gotos landing on the
   `D_800A38A8` block; if it is `mark_target_live_regs`, the dial is what the
   fall-through arm does to `$v0`.

3. **The two commutative `addu` operand orders (emitted 96 `addu s3,s0,v0` vs
   target `addu s3,v0,s0`, and emitted 162 `addu a0,a0,s4` vs target
   `addu a0,s4,a0`) are the cheapest remaining points but have never been
   attacked as a pair on a body that already wins cluster A.**
   *Mechanism.*  Both are the operand order GCC emits for a `plus` whose
   operands are a pseudo and a hard-reg-allocated pseudo; earlier sessions
   measured each spelling swap score-neutral, i.e. the C operand order does not
   reach them (`rejected/addu-commutativity-laundered.c`,
   `s3-operand-order-neutral-11.c`, `tail-ptr-operand-order-neutral-11.c`).
   Emitted 96 is the birth insn of pseudo 143, whose seat just changed this
   session, so the earlier neutrality measurement is chassis-stale for THAT
   site.
   *Next probe.*  Re-measure only the emitted-96 site
   (`u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` vs `(temp_a1_2 * 2) + arg0`) on
   `candidate.c` and read emitted 96 out of the objdump; if it is still
   laundered, record the site as foreclosed on this chassis and stop re-probing
   commutativity for good.

## [s16] The cluster-A seat exchange has a second, unmeasured input besides allocno_compare priority: find_reg's pass-0 gating (regs_used_so_far), regs_someone_prefers, and hard_reg_copy_preferences could steer which hard reg each allocno takes independently of allocation order.
- mechanism: global.c find_reg runs two passes; pass 0 ORs in the complement of regs_used_so_far plus regs_someone_prefers[allocno], so a pseudo can be pushed off $s2 onto $s3 without any change to pri. If any of those sets were non-empty for pseudo 73 or 143, cluster A would have a preference dial that the E-s15-1 closed form does not model.
- probe: Compiled the store-pinned t4 body with the instrumented cc1 under BB2_FINDREG_DEBUG=143 and =73 and read the dumped exclusion sets (tmp/grind/func_800283D0/s16/findreg_t4_143.txt, findreg_t4_73.txt), then read tools/gcc-2.7.2/global.c:583-599 to confirm the allocation loop has no second phase.
- result: someone_prefers, own_copy_prefs and own_full_prefs are ALL EMPTY for both allocnos. regs_used_so_far contains neither 18 nor 19 when 143 is allocated, so both allocnos fall through pass 0 into pass 1 and take the first non-conflicting register in reg_alloc_order. The allocation loop is one qsort on allocno_compare followed by one sequential find_reg call per allocno.
- verdict: KILLED

## [s16] E-s15-5's re-attribution is wrong: target's `<` arm is NOT variant A's arm (nrefs(73)=7 plus livelen(143)>=20); it is a DUPLICATED-CALLS arm (nrefs(73)=9) whose 0xB edge is a source `goto` into path1's selection statement and whose 0x19 edge keeps its own store, with jump2 merging only the common `calls; return` tail.
- mechanism: Target's .L80028610 is `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)`. The 0xB edge lands ON path1's `addiu $v0,0xB` (value assignment AND store merged); the 0x19 edge keeps its own value insn and its own store and rejoins one insn after path1's store. That asymmetry is what jump2's cross-jump produces when an arm duplicates `store; calls; return` and the walk-back goes one insn further on one edge than the other - not what an arm with no duplicated calls produces.
- probe: Spelled that arm in C on the 11-floor body (`s16 var_v0_4 = 0x19; if (var_s1 == 0) { goto set_0xB; } store; calls(1); calls(0x25); return ret;`), measured `sandbox --disable all`, and read the resulting seat table with a new ALLOCDBG dumper (tmp/grind/func_800283D0/s16/al.py).
- result: 10 / 216 - a NEW FLOOR (was 11 / 216). ALLOCDBG on the new body: pseudo 73 (arg1) ord 17, nrefs 9, livelen 98, pri 2755 -> $s2; pseudo 143 (temp_s3) ord 20, nrefs 3, livelen 14, pri 2142 -> $s3 - target's seats. The duplicated call pair is target's own cluster-A mechanism, so livelen(143) >= 20 is not required and the store-pin / cluster-A tension recorded since E-s13-3 does not exist in target's source.
- verdict: CONFIRMED

## [s16] The arm's control flow alone is what buys the point; the spelling of the 0x19 value (literal store vs a local carrier) is incidental.
- mechanism: jump2's cross-jump merges identical instruction tails. If the arm stores a literal, the pre-store insn sequence still ends in `li $v0,0x19; sh $v0,0x286($s0)` and ought to merge with path1's identically-shaped tail.
- probe: Measured the same arm with the store written as `*(s16 *)(arg0 + 0x286) = 0x19;` (v1) against the local-carrier spelling (v2), and a third variant whose 0xB edge lands on path1's STORE rather than on path1's value assignment (vA).
- result: v1 = 14 / 219 - jump2 refuses to merge the duplicated call pair AT ALL, costing 3 insns; v2 = 10 / 216; vA = 10 / 216 (codegen-equivalent to v2). The local value carrier is load-bearing for the merge; which of path1's two labels the 0xB edge targets is not.
- verdict: KILLED

## [s16] With the new arm in place, jumping the 0x19 edge to the shared `do_calls` label (dropping the duplicated pair) would pin path1's store at a block boundary at no cost to the seats.
- mechanism: A source goto landing after path1's store makes that store end a basic block, which is what keeps target's store at the head of the shared block instead of in the jal delay slot (E-s13-1). If the seats were held by something other than nrefs, the pair could be dropped for free.
- probe: vD - candidate's arm with `*(s16 *)(arg0 + 0x286) = var_v0_4; goto do_calls;` replacing the duplicated call pair. Measured sandbox, then re-read the seat table.
- result: 17 / 216 and the $s2/$s3 seats swap back (nrefs(73) 9 -> 7, pri 2755 -> ~1521 < 2142). Third independent confirmation of the E-s15-1 pri arithmetic on this chassis: the duplicated pair is worth exactly the seat, so it cannot be traded for the store pin this way.
- verdict: KILLED


## s17 (solver) - hypotheses closed and opened

**H-s17-A (CONFIRMED).**  The cluster-A seat exchange and path1's store pin are
NOT mutually exclusive.  tC holds both at 12 / 221; only the arm's four
argument-setup insns fail to cross-jump.  E-s13-3's "provably coupled" statement
is superseded.

**H-s17-B (CONFIRMED, mechanism).**  A duplicated call pair merges 100% under
jump2 only when the arm's block and path1's block are SCHEDULED IDENTICALLY,
i.e. their stores are both pinned or both sunk.  Traced with BB2_XJUMP_DEBUG:
find_cross_jump PAT-MISMATCH at the arm's own `sh` against path1's
`move $a3,$zero`.

**H-s17-C (CONFIRMED).**  nrefs(73) = 8 is sufficient for the seat (pri 2376 >
2142).  Only ONE extra byte-free arg1 reference is needed, not two.

**H-s17-D (KILLED).**  E-s16-3's identification of target's arm as a
duplicated-calls arm, deduced from the emitted arm shape.  Variant A's arm emits
the identical four instructions, so the emitted arm discriminates nothing; and
variant A additionally reproduces emitted 83-88, which the duplicated-calls arm
does not.

**H-s17-E (KILLED).**  "Pin both sides identically and the duplicated pair will
merge into target's shape."  tD pins both and jump2 then collapses the WHOLE
arm (11 / 212, three insns short of target).  Identical pinning over-merges.

**OPEN (frontier).**
1. Find a path1 spelling whose 0x19 edge has NO store of its own (so it is
   textually distinct from the arm's 0x19 edge) while path1's single store is
   still pinned - target's asm says that is exactly target's shape.  tD's
   split-store pin gets the pin for free but gives the 0x19 edge a store.
2. On tA (variant A arm, byte-exact at 83-88 and 144-148), find ONE byte-free
   extra `arg1` reference outside the arm.  Sites whose tails can cross-jump
   100% are the only candidates; the arm is occupied by the pin.
3. Cluster B (emitted 45-48 and 126/131) is untouched since s10 and is still a
   reorg.c `steal_delay_list_from_target` eligibility question.

## [s17] inverse_compose.py classify is the correct solver triage entry point for this function.
- mechanism: classify compares <stem>.hon.s against <stem>.tgt.s, where .tgt.s is 'current src + regfix + asmfix'. Under asm-until-matched func_800283D0 carries ZERO rules, so the 'target' stream is our own build and the comparison is self-referential.
- probe: Ran `inverse_compose.py classify code6cac_b func_800283D0` after mkasm_honest.sh; then ran the object-level `goal_from_tgt.py classify code6cac_b func_800283D0`.
- result: inverse_compose reported FIRST DIVERGENCE: IDENTICAL, 213 vs 213 insns. goal_from_tgt reported ours 216 / target 215, FIRST DIVERGENCE: PRE-RA, with 'ours only: li #,1 x1 + nop x1' and 'target only: sh #,646(#) x1'.
- verdict: KILLED

## [s17] E-s16-3: target's `<` arm is identified bit-for-bit as a DUPLICATED-CALLS arm, therefore target compiles with nrefs(73)=9 and the livelen(143) route must not be spent.
- mechanism: s16 read target's four arm insns (beqz/li 0x19/j/sh) and asserted only a duplicated-calls arm plus a jump2 walk-back can produce them.
- probe: Built tA (the s16 candidate with the arm's 0x19 edge changed to `store; goto do_calls;` = variant A) and diffed its normalized objdump against asm/funcs/func_800283D0.s.
- result: tA = 17/216 and its diff has NO entry at emitted 83-88 and NO entry at 144-148 - variant A's arm emits target's four instructions EXACTLY (as E-s13-2 already recorded) AND reproduces target's store block. The emitted arm discriminates nothing; emitted 83-88 discriminates AGAINST the duplicated-calls arm.
- verdict: KILLED

## [s17] The cluster-A seat exchange requires nrefs(73) >= 9 (two extra arg1 references).
- mechanism: global.c allocno_compare pri = floor_log2(nrefs)*nrefs/livelen*10000*size; every prior body that won the seat carried a duplicated CALL PAIR.
- probe: Built tB, whose arm's 0xB edge duplicates only the FIRST call and jumps to a `do_call2:` label between path1's two calls, then dumped ALLOCDBG.
- result: tB: pseudo 73 nrefs 8 livelen 101 pri 2376 -> $s2; pseudo 143 nrefs 3 livelen 14 pri 2142 -> $s3. The seats FLIP at nrefs 8. One extra reference is sufficient; the assumed requirement was double the real one.
- verdict: KILLED

## [s17] E-s13-3: the store pin and cluster A are provably coupled - the pinning `goto` must sit on the same arm edge that would carry the duplicated call pair, so the two cannot be held together.
- mechanism: The pin needs a label predating sched1, hence a source goto; that goto deletes the edge's call duplicates.
- probe: Built tC: the arm's 0xB edge does its own store then `goto do_calls` (supplying the pin), while the 0x19 edge keeps its own store and BOTH duplicated calls.
- result: tC = 12/221 with NO cluster-A entry in the diff (all twelve $s2 seats and the prologue sw pair correct) AND path1's store at the head of the shared block (emitted 83 correct). Both held simultaneously at a competitive score; the coupling is not a law, it is a consequence of which edge carries the goto.
- verdict: KILLED

## [s17] On a body where path1's store is pinned, an arm carrying a duplicated call pair will cross-jump back to zero cost, exactly as it does on the unpinned 10-floor body.
- mechanism: jump2's find_cross_jump walks two streams backwards and merges while patterns match.
- probe: Ran the instrumented cc1 with BB2_XJUMP_DEBUG=1 on tC and read the trace for the arm's return.
- result: The walk matched 6 insns (call 0x25's block and call 1's jal) then PAT-MISMATCH i1=432 (the ARM's own `sh $v0,0x286($s0)`) vs i2=240 (path1's `move $a3,$zero`); do_cross_jump merged only from the jal (newjpos=447 newlpos=242). path1's store is pinned at its block head while the arm's is unpinned and sched1 sinks it before the arm's jal, so the two blocks are scheduled differently and the argument setup cannot merge. THE LAW: the duplicate merges 100% only when both blocks are scheduled identically - both stores pinned or both sunk.
- verdict: CONFIRMED

## [s17] If BOTH path1's store and the arm's store are pinned the same way, the duplicated call pair will merge and the arm will emit target's four insns.
- mechanism: Identical scheduling on both sides should let find_cross_jump walk back to the arm's own pin label and stop there, leaving the arm [li 0x19, sh] plus a jump to path1's a1 setup - exactly target's .L80028520 shape.
- probe: Built tD (path1 and the arm each respelled so every selection edge does its own store and the 0xB edge ends `goto <calls label>`), measured and diffed.
- result: tD = 11/212. Path1's emitted 80-88 is BYTE-EXACT with target, but jump2 collapses the ENTIRE `<` arm (emitted `bnez $v0` where target has `beqz $v0`; target's four arm insns absent, build 3 insns SHORT). Identical pinning makes the arm's two edges textually identical to path1's two edges, so the merge does not stop at the pin - it eats the whole arm.
- verdict: KILLED

## s18 (forensics) - hypotheses closed and opened

## [s18] CLOSED-CONFIRMED: E-s15-5's livelen route is real - livelen(143) >= 20 flips the cluster-A seats with nrefs(73) still 7.
- mechanism: global.c allocno_compare sorts by pri = floor_log2(nrefs)*nrefs/livelen*10000*size. On tA pri(143) = 1*3/14*10000 = 2142 and pri(73) = 2*7/92*10000 = 1521, so 143 is allocated first and takes $s2. Pushing livelen(143) to 20 drops pri(143) to 1500 < 1521, reversing the sort order so 73 takes $s2 and 143 takes $s3 - target's seats.
- probe: V6 (tA with the two `(u32)(*(u16*)(...+0xE)-6) < 2U` booleans hoisted into `s32 c0, c1` right after the shared call pair). ALLOCDBG: 73 -> $s2 nrefs 7 livelen 92 pri 1521; 143 -> $s3 nrefs 3 livelen 20 pri 1500. Normalized diff has ZERO cluster-A entries.
- verdict: CONFIRMED. Cluster A has two disjoint solutions, not one.

## [s18] CLOSED-KILLED (on this chassis): the livelen route is affordable.
- mechanism: reg_live_length counts one per insn inside the def-to-last-use span, so lengthening 143 by 6 requires SIX insns scheduled between the last `jal` and the `== 5` reload. Target places those insns AFTER the reload, so any hoist moves 6 insns across a 4-insn reload/compare group.
- probe: V6 = 19/214 (cluster A closed, ~14 diff slots at emitted 111-127); V10 = 38/207 (hoisting the shared selection additionally lets jump2 dedupe target's two identical 4-insn selection blocks). Seat-flipped tA is worth ~5; cheapest displacement prices at ~10.
- verdict: KILLED as a floor-lowering move on a store-pinned body. It stays live only if a displacement can be found whose six insns land in target's own positions - no such site exists in this function's shape.

## [s18] CLOSED-KILLED: nrefs(143) 3 -> 2 by respelling the address (the s14 recompute hypothesis, both directions).
- mechanism: cse2 re-unifies any recomputed `arg0 + temp_a1_2*2` with the existing pointer pseudo, so the ref lands back on 143.
- probe: V3 (recompute at the `== 5` test) and V4 (recompute at the FIRST load) are ALLOCDBG-IDENTICAL to tA - 3 refs, livelen 14, pri 2142. V7 (a differently-based `s16 *base288 = (s16*)(arg0+0x288)` with `base288[temp_a1_2]`) DOES split the pseudo (145: 3 refs, 146: 2 refs) but adds an eighth callee-saved allocno, puts arg1 in $s4 and costs a prologue save.
- verdict: KILLED. Do not re-propose address respellings for nrefs(143).

## [s18] CLOSED-KILLED: lengthening livelen(143) from the BIRTH end (the s15 declaration hoist).
- mechanism: the def cannot move above the `temp_v1_3 == 0` branch without cse merging it with the `== 0` path's identical address; the merged pseudo has nrefs 4 / livelen 16 / pri 5000, which outranks arg0 (4871) and takes $s0.
- probe: V1. Every callee-saved seat reshuffles; 143 disappears as an allocno.
- verdict: KILLED. Birth-end lengthening is structurally impossible here; only the death end is available.

## [s18] CLOSED-CONFIRMED: cluster B's dbr refusal is `insn_sets_resource_p (trial, &opposite_needed)` (mark_target_live_regs), NOT a LABEL_NUSES refusal.
- mechanism: fill_slots_from_thread will only take a trial insn from the target thread if it does not clobber a register live on the OPPOSITE thread, because the MIPS port has no annulled slots. insn 368 (`v0 = 1`, head of the D_800A38A8 block) writes $v0, and $v0 is in mark_target_live_regs(insn 731) - the fall-through `bnez $s1 -> 632` + delay `v0 = 0x19` selection sequence.
- probe: BB2_DBR_DEBUG=1 on the base body, `DBRDBG thr insn=344 trial=368 refset=0 setset=0 setneed=0 setsopp=1 trap=0` -> LOSE, with `DBRDBG mtlr target=731 block=27` proving the block was found (no conservative fallback).
- verdict: CONFIRMED. The s17 frontier's LABEL_NUSES alternative is eliminated; the C dial is what the fall-through 0x19/0xB selection does to $v0.

## [s18] OPEN: the over-approximation in basic_block_live_at_start[27] is itself the lever - $a3 is marked live at insn 731 and is provably dead there, so $v0's liveness there may also be an artifact of a stale block boundary rather than a real dataflow fact.
- mechanism: reorg.c computes basic_block_live_at_start once and mark_target_live_regs returns it verbatim when the target IS the block head. Block 27 begins at code_label 347, whose block sets $v0 before any use of it, so a correct live-in would exclude $v0. If the over-approximation comes from label 347 having a second predecessor (the `bnez $v0` from the first 0xE test) or from the block head having moved when sched2 formed the sequence, then changing the SHAPE of the first 0xE test - so that label 347 has exactly one predecessor, or so that the fall-through block's first insn is not the one dbr already consumed into a sequence - would make $v0 dead there and let the steal happen.
- next probe: on the base body, re-run `bash tmp/grind/func_800283D0/s18/dbr.sh <tag>` (it writes both the DBRDBG log and the -da dumps) with the `||` restructured so the first 0xE test branches somewhere other than label 347 (e.g. `if (h0 >= 2U) { if (h1 >= 2U) { D_800A38A8 = 1; ... } } selection...`), and diff the `oppregs` word at `DBRDBG thr insn=<the beqz> trial=<the v0=1>`. If bit 0x4 clears, the steal fires and cluster B's ~4 points plus the 216-vs-215 surplus close. Read the branch's UID out of `tmp/grind/func_800283D0/s18/code6cac_b.i.dbr` (search for the insn that sets `(mem (symbol_ref "D_800A38A8"))` and walk back to the preceding jump_insn).

## [s18] OPEN: on a store-pinned body the byte-free nrefs(73) route needs a TAIL-mergeable duplication site, and E-s18-1's ref map says only three call-pair sites exist.
- mechanism: jump2 runs after global_alloc/reload, so a duplicated call pair counts toward reg_n_refs at allocation time and can still cross-jump to zero emitted cost (E-s18-6). But find_cross_jump merges TAILS, so the duplicate must be the last thing both arms do. path1's tail is `store; calls; return`; the `==` arm's calls are followed by the `== 5` subtree; the `>` path's calls are followed by the 0x1A selection. Only the `<` arm can tail-merge with path1 - and on a pinned body that arm is spending its goto on the pin (E-s17-5's scheduling law).
- next probe: this is one measurement away from a clean FORECLOSURE. Take tD's self-supplied path1 pin (E-s17-6, emitted 80-88 byte-exact) and give the `<` arm a duplicated call pair whose block is scheduled IDENTICALLY to path1's - i.e. path1's calls block must also begin with its store, which tD's per-edge-store spelling already guarantees. If that body still stalls at 12/221 like tC/tE/tH, the nrefs route is FORECLOSED on every pinned body and the function's whole remaining budget belongs to cluster B.

## [s18] E-s15-5's route is real: livelen(pseudo 143 / temp_s3) >= 20 flips the cluster-A callee-saved seats without any extra arg1 reference.
- mechanism: GCC 2.7.2 global.c allocno_compare sorts allocnos by pri = floor_log2(nrefs)*nrefs/livelen*10000*size. On tA pri(143) = 1*3/14*10000 = 2142 and pri(73) = 2*7/92*10000 = 1521, so 143 is allocated first and takes $s2. Raising livelen(143) to 20 drops pri(143) to 1500 < 1521, reversing the sort and giving 73 the $s2 seat and 143 the $s3 seat - exactly target's assignment.
- probe: Body V6 = tA with the two `(u32)(*(u16*)(...+0xE) - 6) < 2U` sub-expressions hoisted out of the `== 5` block into `s32 c0, c1` evaluated immediately after the shared call pair. ALLOCDBG via the instrumented cc1 (BB2_ALLOC_DEBUG=1).
- result: ALLOCDBG on V6: ord=23 pseudo=73 $s2 nrefs=7 livelen=92 pri=1521; ord=24 pseudo=143 $s3 nrefs=3 livelen=20 pri=1500. The normalized objdump diff has ZERO cluster-A entries - all twelve `move $a2,$s2` sites and the prologue sw $s2 / sw $s3 pair are byte-exact. First body in 18 sessions to reach target's seats without a duplicated call pair.
- verdict: CONFIRMED

## [s18] The livelen route is affordable enough to lower the floor on a store-pinned body.
- mechanism: reg_live_length counts one per insn scheduled inside the def-to-last-use span, so +6 livelen requires SIX insns moved between the last `jal` and the `== 5` reload. Target places all of those insns AFTER the reload, so any hoist drags six insns across a four-insn reload/compare group.
- probe: Six bodies measured with both sandbox --disable all and ALLOCDBG: tA (nothing hoisted) livelen 14, V8 (selection, 3 insns) 17, V5 (two 0xE loads + subtract, 4 insns) 18, V9 (5 insns) 19, V10 (6 insns) 20 FLIP, V6 (6 insns) 20 FLIP.
- result: V6 = 19 / 214 and V10 = 38 / 207. The dial is exactly linear (1 displaced insn = +1 livelen) and the threshold is exactly 20 as the closed form predicted, but the cheapest displacement prices at ~10 diff points while a seat-flipped tA is only worth ~5 (tA 17 minus cluster A ~12). V10 is worse still because hoisting the shared 0x19/0xB selection lets jump2 dedupe target's two IDENTICAL 4-insn selection blocks (.L800285CC / .L800285F8), costing 8 insns.
- verdict: KILLED

## [s18] nrefs(143) can be lowered from 3 to 2 by respelling one of the two 0x288 loads (the s14 recompute hypothesis).
- mechanism: pri(143) at nrefs 2 would be 1*2/14*10000 = 1428 < pri(73) = 1521, so a single removed reference would flip the seats with no insn displacement at all.
- probe: V3 (recompute the address at the `== 5` test), V4 (recompute at the FIRST load instead), V7 (a differently-based `s16 *base288 = (s16*)(arg0 + 0x288)` with `base288[temp_a1_2]`). ALLOCDBG on each.
- result: V3 and V4 are ALLOCDBG-IDENTICAL to tA - nrefs 3, livelen 14, pri 2142 - because cse2 re-unifies the recomputed address with the pointer pseudo in both directions. V7 does split the pseudo (145: 3 refs, 146: 2 refs) but adds an EIGHTH callee-saved allocno, lands arg1 in $s4 and costs an extra prologue save.
- verdict: KILLED

## [s18] livelen(143) can be lengthened from the BIRTH end by hoisting the `u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` declaration to the head of block_20 (the s15 hypothesis).
- mechanism: Moving the def above the `temp_v1_3 == 0` branch would let the pointer's live range cover the extra block without adding a reference.
- probe: Body V1, ALLOCDBG.
- result: Destructive, not merely inert: cse merges the hoisted def with the `== 0` path's identical `arg0 + temp_a1_2*2` address into ONE pseudo (123) with nrefs 4 / livelen 16 / pri 5000, which outranks arg0 (pri 4871) and takes $s0. Every callee-saved seat reshuffles and 143 ceases to exist as an allocno. The def cannot leave the != 0 block without merging the two paths' addresses.
- verdict: KILLED

## [s18] Cluster B's delay-slot refusal is one of two candidates - a LABEL_NUSES != 1 refusal, or a mark_target_live_regs refusal (the s17 frontier's open binary question).
- mechanism: reorg.c's fill_slots_from_thread takes an insn from the target thread only if it passes five predicates; the MIPS port has no annulled slots, so a trial that clobbers a register live on the OPPOSITE thread can never be taken.
- probe: BB2_DBR_DEBUG=1 on the instrumented tools/gcc-2.7.2/cc1 over the 10-floor base body, cross-referenced against the -da .dbr RTL to identify the branch and the stolen insn by UID.
- result: The branch is jump_insn 344 (`beqz $v0,<label 365>`, the `(u32)(*(u16*)(temp_s4+0xE)-6) < 2U` test); the insn target steals is insn 368 (`(set (reg:HI 2 v0) (const_int 1))`, head of the D_800A38A8 block). Trace: `DBRDBG thr insn=344 trial=368 refset=0 setset=0 setneed=0 setsopp=1 trap=0` then `LOSE`. FOUR of five predicates pass; the sole failure is insn_sets_resource_p(trial, &opposite_needed) - insn 368 writes $v0 and $v0 (bit 0x4 of oppregs=0x20630084) is live in mark_target_live_regs(opposite_thread = insn 731, the already-filled `bnez $s1 -> 632` + delay `v0 = 0x19` selection sequence). `DBRDBG mtlr target=731 block=27` proves the block was found, so this is a real liveness answer, not the find_basic_block()==-1 conservative fallback.
- verdict: CONFIRMED

## s19 (forensics, 2026-08-27)

### H-s19-1 CONFIRMED - cluster B's empty delay slot is closable by a pure-C dial
Statement: the refusal of `jump_insn 344`'s delay slot is not a compiler wall; it
is caused by the fall-through selection block owning a `$v0` write that
`fill_simple_delay_slots` steals backward into the block's own branch, and
removing that write from the block removes `$v0` from `opposite_needed`.
Mechanism: reorg.c `update_block` leaves `(use (insn N))` at the stolen insn's
original position; `mark_target_live_regs`'s forward walk dereferences the USE
and `note_stores`/`update_live_status` re-marks the destination LIVE after
`code_label 347` has already flushed the REG_DEAD note carried by `jump_insn 344`.
Probe: body W1 (`var_v0_2 = 0x19;` hoisted above the `||` test), `BB2_DBR_DEBUG=1`.
Result: `oppregs` 0x20630084 -> 0x20630088 ($v0 bit cleared), opposite thread
becomes the plain `jump_insn 354` instead of `sequence 731`, `setsopp=0`, no LOSE,
delay slot filled with `v0 = 1` byte-exactly as target.
Verdict: CONFIRMED.

### H-s19-2 KILLED - the s18 frontier's reading that block 27's live-in is over-approximate
Statement (s18): `basic_block_live_at_start[27]` marks `$v0` and `$a3` live even
though both are provably dead there, so the over-approximation itself is the lever.
Probe: read `dump_flow_info`'s per-block "Registers live at start" out of the
`.lreg` dump and renumber through the `.greg` dispositions.
Result: block 27's live-in is pseudos 29 30 72 73 75 77 79 89 90 128 =
sp/fp/s0/s2/s4/s1/s6/a1/s5/v1.  Neither `$v0` nor `$a3` is in it.  The two bits
are introduced by the forward walk in `mark_target_live_regs`, not by flow.
Verdict: KILLED (the premise was wrong; the real mechanism is H-s19-1).

### H-s19-3 KILLED - the s18 frontier's probe (a): "give the first 0xE test its own branch target so label 347 has exactly ONE predecessor"
Statement: cluster B is a LABEL_NUSES / predecessor-count question about the
fall-through label.
Probe: trace `find_basic_block` (reorg.c:2236) and the walk bounds directly.
Result: the block resolution ignores label predecessor counts entirely - it
scans back to the previous BARRIER and forward over the labels that follow it.
Adding or removing a predecessor of `code_label 347` cannot move the walk's
start, which is `code_label 270` regardless.  What matters is only which insns
between the barrier-derived head and the opposite thread write `$v0`.
Verdict: KILLED.

### H-s19-4 CONFIRMED (as a constraint, not a solution) - slots 344 and 354 are mutually exclusive on this block shape
Statement: no body whose fall-through selection block owns its own `$v0` write
can have both `jump_insn 344`'s and `jump_insn 354`'s delay slots filled.
Mechanism: `fill_simple_delay_slots`'s backward search is bounded by
`stop_search_p`, which stops at a `CODE_LABEL`, so the insn that fills 354's
slot is always drawn from between `code_label 347` and the branch - exactly the
window whose `update_block` marker poisons 344's `opposite_needed`.
Probe: W1's normalized diff, emitted 120 (`li v1,25` vs target `nop`) and
emitted 128 (`nop` vs target `li v0,25`).
Result: W1 filled 344 and lost 354, as predicted.  W1 = 32 / 216.
Verdict: CONFIRMED.  Target has BOTH slots filled, so target's RTL must satisfy
one of: (a) 354's slot filled by fill_eager from its TARGET (`INSN_FROM_TARGET_P`
suppresses the marker entirely), (b) the filling insn writes a register other
than `$v0`, (c) `code_label 347` absent at reorg time (refuted by target's asm).

## [s19] Cluster B's empty delay slot on jump_insn 344 is closable by a pure-C dial: the fall-through selection block owns a $v0 write that fill_simple_delay_slots steals backward into that block's own branch, and removing that write from the block removes $v0 from opposite_needed.
- mechanism: reorg.c update_block (reorg.c:2270) leaves a `(use (insn N))` marker at the stolen insn's ORIGINAL position, between code_label 347 and the new opposite thread. mark_target_live_regs' forward walk dereferences that USE to the real insn and applies note_stores/update_live_status (reorg.c:2392), which SETs the destination LIVE - undoing the pending_dead_regs flush that code_label 347 had just performed for the REG_DEAD (reg:SI 2 v0) note carried by jump_insn 344. insn_sets_resource_p(trial=368, &opposite_needed) is then 1 and the slot is refused.
- probe: Body W1 = the banked candidate with `var_v0_2 = 0x19;` hoisted above the `||` test, so the fall-through block no longer owns a $v0 write. BB2_DBR_DEBUG=1 on the instrumented tools/gcc-2.7.2/cc1 (tmp/grind/func_800283D0/s19/dbr_W1.txt), plus sandbox --disable all.
- result: dbr_W1.txt lines 603-604: `thr insn=347 thread=368 opp=354 own=1 likely=0 tif=1 oppregs=20630088_00000000` then `trial=368 refset=0 setset=0 setneed=0 setsopp=0 trap=0` with NO LOSE line. oppregs loses bit 0x4 ($v0) and gains bit 0x8 ($v1); the opposite thread is the PLAIN jump_insn 354 instead of the filled sequence 731. The delay slot is filled with `v0 = 1` and the normalized diff has no entry at the `beqz v0 / li v0,1` pair. Score 32 / 216.
- verdict: CONFIRMED

## [s19] s18's reading that basic_block_live_at_start[27] is over-approximate (marking $v0 and $a3 live although both are provably dead there), making the over-approximation itself the lever.
- mechanism: s18 inferred the live set from the oppregs word alone, without reading the pre-RA CFG.
- probe: Read dump_flow_info's per-block 'Registers live at start' out of tmp/grind/func_800283D0/s19/code6cac_b.i.lreg and renumber through the .greg dispositions.
- result: Block 27 live-in = pseudos 29 30 72 73 75 77 79 89 90 128 = sp/fp/s0/s2/s4/s1/s6/a1/s5/v1. Neither $v0 nor $a3 is present. Blocks 30/31/33 are likewise clean. Both bits are introduced by mark_target_live_regs' forward walk, not by flow.
- verdict: KILLED

## [s19] s18's frontier probe (a): cluster B is a predecessor-count question - give the first 0xE test its own branch target so code_label 347 has exactly ONE predecessor.
- mechanism: Assumed find_basic_block/mark_target_live_regs resolve the fall-through block via its label's predecessors.
- probe: Trace find_basic_block (reorg.c:2236) and the walk bounds in mark_target_live_regs directly against the .sched2/.dbr RTL.
- result: find_basic_block ignores predecessor counts entirely: it scans BACKWARD to the previous BARRIER, then forward over the run of CODE_LABELs following it, returning the first that is a basic_block_head. Conditional branches emit no barrier, so the fall-through of jump_insn 344 always resolves to block 27 (head code_label 270), never to its own block 31 (head code_label 347). Adding or removing predecessors of label 347 cannot move the walk's start.
- verdict: KILLED

## [s19] On this block shape, jump_insn 344's and jump_insn 354's delay slots are MUTUALLY EXCLUSIVE - a body cannot have both filled.
- mechanism: fill_simple_delay_slots' backward search is bounded by stop_search_p, which stops at a CODE_LABEL, so whatever fills 354's slot from before the branch is necessarily drawn from between code_label 347 and the branch - exactly the window whose update_block marker poisons 344's opposite_needed. fill_simple runs before fill_eager, so 354 always wins first.
- probe: W1's normalized objdump diff vs target at emitted 120 and 128.
- result: W1 emitted 120 = `li v1,25` where target has `nop`, and emitted 128 = `nop` where target has `li v0,25`: W1 filled slot 344 and lost slot 354, exactly as predicted. Target has BOTH filled, so target's RTL must satisfy (a) 354's slot filled by fill_eager from its TARGET (INSN_FROM_TARGET_P suppresses the marker - update_block returns early), (b) the filling insn writes a register other than $v0, or (c) code_label 347 absent at reorg time (refuted by target's own asm, where .L800285CC is the bnez target of the first 0xE test).
- verdict: CONFIRMED


## [s20] Cluster B's ~4 points and the 216-vs-215 insn surplus close for FREE once W1's incidental cost is removed by a fresh local scoped to the `== 5` subtree.
- mechanism: W1's 32/216 = ~20 points of pure register renaming in the `>` path caused by var_v0_2 leaving `$v0`, plus a structural 2-point slot trade at emitted 120/128. A local declared inside the `== 5` block cannot participate in the `>` path's allocation, so the ~20 points should vanish and the body should land near 12 with cluster B's slot correct.
- probe: four spellings of the fresh local, each applied with tmp/grind/func_800283D0/s20/apply.py and scored with `sandbox --disable all` (base re-measured twice per E-s13-6): W2a (arm stores `sel5` itself, `goto block_49`), W2b (`var_v0_2 = sel5; goto block_48`), W2c (W2b with `s32 sel5`), W2d (`var_v0_2 = sel5` written before the `var_s1` test). Normalized objdump diffs read with s19/mk.sh.
- result: HALF CONFIRMED, HALF KILLED. The incidental cost is real and fully removable - W1 = 32/216, W2b = W2a = W2c = 12/216. But 12 is WORSE than the base 10, so cluster B does NOT close for free: it costs a net +2. W2b's diff has no entry at emitted 126 (`li v0,1` is in the `beqz` delay slot exactly as target), so cluster B IS byte-closed; the price is two `move v0,v1` copies (emitted 128, 131) because `sel5` is allocated `$v1` - its live range spans the `||` test, which computes in `$v0` - plus the structural slot trade at emitted 120 that E-s19-7 predicted and that survives every respelling. W2d folds the fresh local away entirely (byte-identical to base, 10/216).
- verdict: KILLED (as a floor-lowering route); CONFIRMED (as the mechanism prediction)

## [s20] Target closes both delay slots by removing the fall-through block's own `$v0` write, i.e. target's source hoists the 0x19 the way W1/W2b do.
- mechanism: E-s19-4 named `update_block`'s `(use (insn N))` marker as the poisoner of `jump_insn 344`'s slot; removing the block's `$v0` write removes the marker. If target had both slots filled, target's source would have to avoid that write.
- probe: read target's own emitted asm at the region (asm/funcs/func_800283D0.s, normalized lines 126-131 in tmp/grind/func_800283D0/s19/target.txt) and check whether any `0x19` materialisation exists before the `||` test.
- result: REFUTED. Target emits `beqz v0,.L800285DC` / `li v0,1` (slot) / `bnez s1,.L800286F8` / `li v0,25` (slot) / `j .L800286F8` / `li v0,11` (slot). The `li v0,25` in `jump_insn 354`'s slot is a BACKWARD steal from inside the fall-through block - the marker IS present in target - and target still fills `jump_insn 344`. Target has no insn before the `||` test to hoist. The entire hoist family can close cluster B but can never reproduce target's emitted code here.
- verdict: KILLED

## [s20] A disjoint-path variable split isolates var_v0_2's live range, so the `>` path keeps `$v0` while the `== 5` path carries the hoisted 0x19.
- mechanism: the `== 5` subtree and the `>` path are on mutually exclusive control-flow paths, so two distinct pseudos with two distinct `*(s16*)(arg0+0x286)` stores should get two distinct seats and the hoist's renaming cost should not reach the `>` path.
- probe: V5 = base with the whole `>` path tail rewritten onto a fresh `s16 var_v0_5` plus its own store and `goto block_49`; V4 = V5 plus W1's hoist. Scored with `sandbox --disable all`.
- result: KILLED. V5 = 10 / 216, byte-identical to base - jump2 cross-jumps the two stores back together and the pseudos coalesce, so the split is a source-level no-op. V4 = 15 / 218 (better than W1's 32, but two insns longer and 5 worse than base). A source-level variable split is not a live-range lever on this body.
- verdict: KILLED

## [s20] Inverting the `== 5` selection so the 0x19 edge is the branch TARGET makes the branch's slot fillable only from its target (INSN_FROM_TARGET_P leaves no `update_block` marker), closing cluster B without the hoist.
- mechanism: s19 frontier item 2 / E-s19-7 option (a): `update_block` returns early without emitting a marker when `INSN_FROM_TARGET_P(insn)` is set, so an insn that arrives in a slot via `steal_delay_list_from_target` cannot poison a preceding branch.
- probe: W3 = the `== 5` selection spelled `if (var_s1 != 0) { var_v0_2 = 0x19; goto block_48; } var_v0_2 = 0xB; goto block_48;`. Scored, and the normalized diff read.
- result: KILLED. 13 / 216. GCC does not lay the arms out in source order here: it re-inverts the branch sense, so the emitted `beqz s1` diverges from target's `bnez s1` and the region gains divergences rather than losing them. W2a exhibits the same inversion. Any respelling that moves the 0x19 initialisation out of the fall-through position flips the emitted branch sense away from target.
- verdict: KILLED

## [s20] The emitted-45-47 slot swap (ours `li v0,1` in the last `beq`'s slot, target `li v0,1` in the `j`'s slot) is an exit-FORM question that a distinct trailing `return 1;` label fixes.
- mechanism: the range check currently spells its exit as an inline `return 1;`. If the `li v0,1` were owned by a separate labelled exit block, fill_simple's forward steal into the `beq`'s slot could not reach it and the `j`'s backward steal would take it instead - which is target's placement.
- probe: E4 = base with `goto ret_one;` in the range check and a trailing `ret_one: return 1;` after `block_13:`. Scored with `sandbox --disable all`.
- result: KILLED. 37 / 215. The insn count reaches target's 215 (the extra label removes the surplus insn) but the score more than triples - the trailing label re-shapes the whole epilogue region. The exit-form dimension is closed on the 10-floor chassis, re-confirming s10's D4/D5 kills on the 23-floor chassis.
- verdict: KILLED

## [s20] Cluster B's ~4 points and the 216-vs-215 insn surplus close for FREE once W1's incidental cost is removed by a fresh local scoped to the `== 5` subtree.
- mechanism: W1's 32/216 = ~20 points of pure register renaming in the `>` path caused by var_v0_2 leaving $v0, plus a structural 2-point slot trade at emitted 120/128. A local declared inside the `== 5` block cannot participate in the `>` path's allocation, so the ~20 points should vanish and the body should land near 12 with cluster B's slot correct.
- probe: Four spellings of the fresh local, each applied with tmp/grind/func_800283D0/s20/apply.py and scored with `sandbox --disable all` (base re-measured twice per E-s13-6): W2a (arm stores sel5 itself, goto block_49), W2b (var_v0_2 = sel5; goto block_48), W2c (W2b with s32 sel5), W2d (var_v0_2 = sel5 written before the var_s1 test). Normalized objdump diffs read with s19/mk.sh.
- result: W1 = 32/216; W2a = W2b = W2c = 12/216; W2d = 10/216 (the fresh local const-folds away, byte-identical to base). W2b's normalized diff has NO entry at emitted 126 - `li v0,1` sits in the beqz delay slot byte-exactly as target - so cluster B IS closed. Its price: two `move v0,v1` copies at emitted 128/131 (sel5 is allocated $v1 because its live range spans the `||` test, which itself computes in $v0, so the sel5 -> var_v0_2 copy does not coalesce) plus the structural slot trade at emitted 120 that E-s19-7 predicted and that survives every respelling.
- verdict: KILLED

## [s20] Target closes both delay slots by removing the fall-through block's own $v0 write - i.e. target's source hoists the 0x19 the way W1/W2b do.
- mechanism: E-s19-4 named update_block's `(use (insn N))` marker as the poisoner of jump_insn 344's slot; removing the block's $v0 write removes the marker, so a target with both slots filled would have to avoid that write.
- probe: Read target's own emitted asm for the region (asm/funcs/func_800283D0.s, normalized as tmp/grind/func_800283D0/s19/target.txt lines 126-131) and check for any 0x19 materialisation before the `||` test.
- result: REFUTED. Target emits `beqz v0,.L800285DC` / `li v0,1` (slot) / `bnez s1,.L800286F8` / `li v0,25` (slot) / `j .L800286F8` / `li v0,11` (slot). The `li v0,25` in jump_insn 354's slot is a BACKWARD steal from inside the fall-through block, so the update_block marker IS present in target - and target still fills jump_insn 344. Target has no insn before the `||` test to hoist. The whole hoist family can close cluster B but can never reproduce target's emitted code in this region.
- verdict: KILLED

## [s20] A disjoint-path variable split isolates var_v0_2's live range, so the `>` path keeps $v0 while the `== 5` path carries the hoisted 0x19.
- mechanism: The `== 5` subtree and the `>` path are mutually exclusive control-flow paths, so two distinct pseudos with two distinct *(s16*)(arg0+0x286) stores should get two distinct seats and the hoist's renaming cost should not reach the `>` path.
- probe: V5 = base with the whole `>` path tail rewritten onto a fresh `s16 var_v0_5` plus its own store and `goto block_49`; V4 = V5 plus W1's hoist. Both scored with `sandbox --disable all`.
- result: V5 = 10 / 216, byte-identical to base - jump2 cross-jumps the two stores back together and the pseudos coalesce, so the split is a source-level no-op. V4 = 15 / 218 (better than W1's 32 but two insns longer and 5 worse than base). A source-level variable split is not a live-range lever on this body.
- verdict: KILLED

## [s20] Inverting the `== 5` selection so the 0x19 edge is the branch TARGET makes the branch's slot fillable only from its target (INSN_FROM_TARGET_P leaves no update_block marker), closing cluster B without the hoist.
- mechanism: s19 frontier item 2 / E-s19-7 option (a): update_block returns early without emitting a marker when INSN_FROM_TARGET_P(insn) is set, so an insn that arrives in a slot via steal_delay_list_from_target cannot poison a preceding branch.
- probe: W3 = the `== 5` selection spelled `if (var_s1 != 0) { var_v0_2 = 0x19; goto block_48; } var_v0_2 = 0xB; goto block_48;`. Scored, normalized diff read.
- result: 13 / 216. GCC does not lay the arms out in source order here - it re-inverts the branch sense, so the emitted `beqz s1` diverges from target's `bnez s1` and the region gains divergences. W2a shows the same inversion. Any respelling that moves the 0x19 initialisation out of the fall-through position flips the emitted branch sense away from target.
- verdict: KILLED

## [s20] The emitted-45-47 slot swap (ours `li v0,1` in the last `beq`'s slot, target `li v0,1` in the `j`'s slot) is an exit-FORM question that a distinct trailing `return 1;` label fixes.
- mechanism: The range check currently spells its exit as an inline `return 1;`. If the `li v0,1` were owned by a separate labelled exit block, fill_simple's forward steal into the beq's slot could not reach it and the j's backward steal would take it instead - which is target's placement.
- probe: E4 = base with `goto ret_one;` in the range check and a trailing `ret_one: return 1;` after `block_13:`. Scored with `sandbox --disable all`.
- result: 37 / 215. The insn count reaches target's 215 (the extra label removes the surplus insn) but the score more than triples - the trailing label re-shapes the whole epilogue region. The exit-form dimension is closed on the 10-floor chassis, re-confirming s10's D4/D5 kills on the 23-floor chassis.
- verdict: KILLED


## s21 (rederive, 2026-08-27)

**H-s21-1 - Target's `<` arm is a source-level double-goto (variant A), not a
duplicated-call arm.**  CONFIRMED (E-s21-1).  Read off target's asm insns
159-162 plus a whole-body `$s2`/`$s3` ref census.  This REFUTES the s16 note in
candidate.c's header claiming the duplicated call pair is target's own cluster-A
mechanism; the s16 body wins cluster A by an accident of ref counting, not by
reproducing target's structure.

**H-s21-2 - Porting target's arm onto the 10-floor body byte-closes the store
sink and the arm store.**  CONFIRMED (E-s21-2).  Body V1 = 17 / 216 with no diff
entry at emitted 83-88 or 148.  The `goto do_calls` label is the pin; sched1
cannot sink a store out of a block it is alone in.  Two residual clusters open
since s13 are closed.

**H-s21-3 - Cluster A is the single inequality pri(arg1-carrier) > 2142, with
pri = floor_log2(n)*n*10000/livelen from global.c:635.**  CONFIRMED (E-s21-3),
reproduced digit-for-digit on five different bodies this session.

**H-s21-4 - Solution (a), nrefs(carrier) >= 8, flips the seats and leaves only
cluster B.**  CONFIRMED by probe P2 (E-s21-4): 13 / 222 with seven insns of
known semantic defect, and a residual diff containing ONLY cluster B and one
commutative `addu`.  Prices the prize at roughly 4-6 / 215.

**H-s21-5 - Solution (b), a later-defined carrier, flips the seats with
semantics preserved.**  CONFIRMED but PRICED NEGATIVE by body P4 (E-s21-5):
16 / 216.  The flip is real (livelen 92 -> 58, pri 2413) and the tail is clean,
but any assignment point late enough to shorten the range is after the
range-check chain, so `$a1` is pinned across it and the prologue diverges by
~10 points.  Family capped at 16; not a route to target.

**H-s21-6 - livelen(temp_s3) can be driven to >= 20 by hoisting its definition
or by reordering blocks.**  KILLED both ways (E-s21-6).  Hoisting hands path1's
load to the same pseudo via cse1 (nrefs 4, livelen 16, pri 5000, takes $s0);
block reordering changes nothing at all because `reg_live_length` is
path-sensitive, not layout-sensitive (P3's ALLOCDBG is identical to V1's).
Target's own window is 14 RTL insns wide, so target does not use this solution
either.

**H-s21-7 - The `>` path's call pair, duplicated into the arms of
`if (var_s1 == 0)`, can supply solution (a) byte-neutrally.**  KILLED (E-s21-7).
The allocation half works (nrefs 9, pri 2700, target seats) but the body scores
81 / 183: `temp_s5` loses its callee-saved seat, `ret` slides $s6 -> $s5, and the
`>` tail collapses.  The arms diverge immediately after the calls so jump2 has
no mergeable tail.  The duplicated-statement-into-arms family is closed for this
residual.

## [s21] Target's `<` arm is a source-level double-goto (s13's variant A), not the duplicated-call arm the s16 header attributes to it.
- mechanism: Read directly off asm/funcs/func_800283D0.s insns 159-162: `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)` - a goto into path1's selection plus a goto into path1's call block with the arm's own store in the delay slot. Four insns, zero calls. Corroborated by a whole-body register census: $s2 has 1 def + 6 `addu $a2,$s2,$zero` call arguments = 7 refs, $s3 has 1 def + 2 `lh ...,0x288($s3)` = 3 refs, and the 0x38 frame carries no stack argument (no sw below 0x18($sp)), so there is no hidden fifth call pair.
- probe: grep '$s2' / '$s3' over asm/funcs/func_800283D0.s plus a read of insns 87-94 and 156-172.
- result: Target's arm is variant A verbatim; nrefs(arg1)=7 and nrefs(temp_s3)=3 in target's shipped bytes. The s16 claim in candidate.c's header ('the duplicated call pair IS target's own mechanism for the cluster-A seat') is refuted.
- verdict: CONFIRMED

## [s21] Porting target's arm onto the 10-floor chassis byte-closes residual cluster 2 (the store sink) and cluster 3 (the arm store), which have been open since s13.
- mechanism: `goto do_calls` makes `do_calls` a REFERENCED label sitting between path1's store and path1's call-argument setup, so the store is alone in its basic block and sched1 has nowhere to sink it. Without the goto the label is unreferenced, GCC deletes it, store and setup share one block, and sched1 sinks the store into the first jal's delay slot (E-s13-1).
- probe: Body V1 = candidate.c with the `<` arm's `store; call(1); call(0x25); return ret;` replaced by `store; goto do_calls;`. Scored, then normalized-objdump diffed against target.
- result: 17 / 216, and the normalized diff has NO entry at emitted 83-88 and NO entry at emitted 148. Both clusters byte-closed. V1's whole remaining residual is cluster A (the $s2/$s3 seat swap) plus cluster B plus the two known commutative addu operand orders. Rejected at 17 only because dropping the arm's duplicated calls takes nrefs(arg1) 9 -> 7 and loses cluster A.
- verdict: CONFIRMED

## [s21] Cluster A on the store-pinned chassis is exactly the inequality pri(arg1-carrier) > 2142, with pri = floor_log2(n_refs)*n_refs*10000/live_length.
- mechanism: tools/gcc-2.7.2/global.c:635-656 allocno_compare sorts allocnos by that quotient descending; of two conflicting allocnos the first-sorted takes $s2 and the second $s3. On V1: pseudo 143 (temp_s3) nrefs=3 livelen=14 pri=2142 -> $s2, pseudo 73 (arg1) nrefs=7 livelen=92 pri=1521 -> $s3, i.e. both seats inverted from target.
- probe: BB2_ALLOC_DEBUG=1 ALLOCDBG dumps on five distinct bodies (V1, P1, P2, P3, P4, P5) compared against the formula read out of global.c.
- result: The formula reproduces every measured pri digit-for-digit. Given livelen(temp_s3)=14 is immovable, the inequality has exactly two solutions: (a) nrefs(carrier) >= 8 at livelen 92 (pri 2608), or (b) livelen(carrier) <= 65 at nrefs 7 (pri 2153).
- verdict: CONFIRMED

## [s21] Solution (a) - one extra arg1 reference - flips both seats on the store-pinned chassis and leaves an otherwise clean diff.
- mechanism: nrefs 7 -> 8 raises floor_log2(n)*n from 14 to 24, so pri goes 1521 -> 2474 and overtakes temp_s3's 2142; the sort order inverts and the carrier takes $s2 while temp_s3 falls to $s3.
- probe: Probe P2 = V1 + one extra `func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);` in the `<` arm before the goto. SEMANTICALLY WRONG (calls once too often) - a measurement only, not banked as a form. ALLOCDBG + score + normalized diff.
- result: Seats become target's exactly (73 pri 2474 -> $s2, 143 pri 2142 -> $s3). Score 13 / 222; after discounting the seven insns the bogus call adds, the residual diff contains ONLY cluster B (emitted 45-48 j/nop inversion, emitted 126 nop vs target's stolen `li v0,1`, and the surplus `li v0,1` at 131) and the single commutative `addu s3,s0,v0` vs `addu s3,v0,s0`. First measurement in 21 sessions showing the store pin and cluster A are simultaneously satisfiable; prices a legitimate solution at roughly 4-6 / 215.
- verdict: CONFIRMED

## [s21] Solution (b) - a later-defined carrier for arg1 - flips the seats with semantics fully preserved.
- mechanism: Assigning the carrier after the top range-check chain shortens its live range from 92 to 58 insns while keeping nrefs at 7, so pri = 2*7*10000/58 = 2413 > 2142.
- probe: Body P4 = V1 + `u8 *p;` declared with the other block locals, assigned `p = arg1;` at block_15, used at all six call sites. ALLOCDBG + score + normalized diff. Banked at rejected/s21-late-arg1-carrier-flips-seats-but-pins-a1-16.c.
- result: Seats correct (91 pri 2413 -> $s2, 144 pri 2142 -> $s3) and everything from emitted 87 to the end matches target except cluster B and the one operand order. But 16 / 216: the residual moves wholesale into the PROLOGUE, because the carrier's source operand is the incoming $a1, which must stay live from entry to block_15. That pins $a1 across the range-check chain (ours `lhu a3,106(s0)` / `li t0,4` vs target `lhu a1,106(s0)` / `li a2,4`, cascading through the andi/beq/addiu chain) and puts `move s2,a1` at emitted 48 instead of target's 4. Any assignment point late enough to shorten the range is by definition after the chain, so the family is structurally capped near 16 and is NOT a route to target.
- verdict: CONFIRMED

## [s21] livelen(temp_s3) can be driven to >= 20 (the third route to the inequality) by hoisting its definition or by reordering the blocks.
- mechanism: Raising livelen(temp_s3) from 14 to 20 would put pri(temp_s3) at 1500, below arg1's 1521, flipping the sort with no change to arg1 at all.
- probe: Body P1 = V1 with `u8 *temp_s3 = arg0 + temp_a1_2*2;` hoisted above the `temp_v1_3 == 0` test and path1's load routed through it. Body P3 = V1 with the three-way compare inverted so the `!=` case is the if-body and the `==` handling is the tail. ALLOCDBG on both.
- result: KILLED both ways. P1: cse1 commons path1's load into the hoisted pointer, so nrefs goes 3 -> 4 while the range stays short - nrefs=4 livelen=16 pri=5000, which outranks arg0 itself, takes $s0 and displaces every callee-saved seat in the function. Any spelling that moves the def up necessarily hands path1's load to the same pseudo, so the direction is wrong by construction. P3: ALLOCDBG comes back IDENTICAL to V1 digit-for-digit (72: 19/156/4871, 77: 9/74/3648, 143: 3/14/2142, 73: 7/92/1521) - flow.c's reg_live_length accumulates only over blocks in which the pseudo is actually live, and temp_s3 is dead on the `<` and `>` paths however they are laid out. Block reordering can never move it. Counting target's own window (insns 107-122: def + load + branch + two 5-insn calls + load = 14 RTL insns) shows target does not use this solution either.
- verdict: KILLED

## [s21] The `>` path's 0x26/0x2D call pair, duplicated into both arms of `if (var_s1 == 0)`, can supply solution (a) byte-neutrally while leaving the `<` arm free to be target's pure double-goto.
- mechanism: It is the only other call site in the function that could add arg1 references without touching the arm that carries the store pin; jump2 cross-jumping would be expected to re-merge the two copies post-reload the way it merges the s16 body's duplicated arm calls.
- probe: Body P5, ALLOCDBG + score. Banked at rejected/s21-gt-path-calls-duplicated-into-arms-collapses-81.c.
- result: KILLED. The allocation half works exactly as the closed form predicts - nrefs(arg1) 7 -> 9, pri = 3*9*10000/100 = 2700 > 2142, seats become target's - but the body scores 81 / 183. The duplication is not byte-neutral: temp_s5 loses its callee-saved seat entirely (only six callee-saved allocnos survive and `ret` slides $s6 -> $s5) and the `>` tail collapses to 183 insns against target's 215. The two arms diverge immediately after the calls, so jump2 never gets a mergeable tail. The duplicated-statement-into-arms family is closed for this residual.
- verdict: KILLED


## [s22] The `do { } while (0)` RA-weighting construct s4 discarded is SANCTIONED policy of record, and it solves cluster A's inequality directly
- mechanism: flow.c increments `REG_N_REFS (regno)` by `loop_depth`, which is 2 inside the NOTE_INSN_LOOP_BEG/END pair a `do { } while (0);` leaves behind. That numerator feeds global.c:635-656 `allocno_compare`'s `pri = floor_log2(n_refs)*n_refs*10000*size/live_length`. s21 reduced the whole $s2/$s3 seat swap to `pri(arg1-carrier) > 2142`, solvable by `nrefs(carrier) >= 8` at livelen 92. A wrap around an existing arg1-referencing call pair supplies exactly that, and supplies it at ZERO emitted cost because loop notes are not instructions.
- probe: re-read `.claude/rules/do-while-zero-exception.md` end-to-end (owner ruling 2026-07-06 abolishes the pre-existing scoping to the reorg.c label-note mechanism and sanctions the wrap for ANY codegen effect incl. register allocation, subject to an inline FAKE annotation); then wrapped each of the three existing `func_80032854(..., arg1, ...)` call pairs on s21's V1 chassis and read ALLOCDBG from the instrumented cc1 for each.
- result: CONFIRMED. Wrapping the `==` arm's pair (body A3) moves nrefs(arg1) 7 -> 9 and nrefs(arg0) 19 -> 21, leaves EVERY live length byte-identical, and flips exactly the two cluster-A seats (arg1 pri 1521 -> 2934 -> $s2; temp_s3 2142 -> $s3, i.e. target's). Score 17 / 216 -> **6 / 216**, closing cluster A, the store sink and the arm store together. Any one of the three pairs gives the identical 6 (A3 = B2 = C2); any two give 6; all three give 24. s4's kill of this construct was a POLICY conclusion under the superseded pre-2026-07-06 scoping and is retracted; its measurements stand.
- verdict: CONFIRMED

## [s22] Wrapping only ONE call of a pair is enough for the seats but not for the score
- mechanism: the loop note then lands BETWEEN the two calls, inside arg1's live range, instead of bracketing both.
- probe: A1/A2 (`==` arm), B1 (`>` path), C1 (shared `do_calls`) - one call each; ALLOCDBG + sandbox.
- result: seats are already target's at nrefs 8 (pri 2553), but livelen(arg1) grows 92 -> 94 and the call block's scheduling is perturbed: all four score 10 / 216 against A3's 6.
- verdict: KILLED (as a form; the measurement localises the extra 4 points to the note's placement, not to allocation)

## [s22] s20's W2b cluster-B hoist becomes free once the store is no longer sunk into the first jal delay slot
- mechanism: s20 attributed W2b's +2 to two non-coalescing `move v0,v1` copies caused by `sel5` getting `$v1`, and hypothesised that on a store-pinned chassis the copies would land in different scheduling positions and coalesce.
- probe: F1 = A3 + s20's W2b construction verbatim; sandbox.
- result: REFUTED. F1 = 8 / 216 against A3's 6 - exactly the same +2 W2b paid on the s16 chassis (10 -> 12). The hoist family's price is chassis-independent.
- verdict: KILLED

## [s22] Cluster B's delay-slot refusal can be flipped by making the branch predicted-taken, via a NOTE_INSN_LOOP_BEG immediately before its target label
- mechanism: reorg.c:1379 `mostly_true_jump` scans back from the branch's target label through NOTEs and returns 2 (strongly predicted taken) on a NOTE_INSN_LOOP_BEG; `fill_eager_delay_slots` (reorg.c:3784) uses that prediction to choose the TARGET thread, which is exactly where target's stolen `li v0,1` comes from at emitted 126, and symmetrically at emitted 45-48.
- probe: six bodies. G1 (wrap the `D_800A38A8 = 1; D_800A3876 = -1;` target block), G2/G3 (wrap the 0x19/0xB selection), G5 (wrap the range-check `return 1;`), G6 (wrap the whole range-check `if`); then the purpose-built H4, which respells the `||` test as `if (!(A || B)) goto rare5;` with `do { rare5: D_800A38A8 = 1; D_800A3876 = -1; } while (0);` so the branch's OWN target label sits immediately after the loop note; and H5/H6, the same shape on the range-check chain.
- result: REFUTED. G1/G2/G3/G5 are byte-neutral at 6 / 216 (G1's neutrality has a named cause: `expand_end_cond` emits the if-false label before the following statement expands, so the note lands on the wrong side of the backward scan). G6 = 14 / 213. H4 - the shape that DOES place the note correctly - is also byte-neutral at 6 / 216. H5/H6 = 10 / 213, regressive: the `return 1` cross-jumps into the shared epilogue and the insn count falls below target's 215. Prediction is not the dial; E-s18-8's `insn_sets_resource_p (trial, &opposite_needed)` attribution stands, and that test runs whichever way the branch is predicted.
- verdict: KILLED

## [s22] The two residual commutative `addu` operand orders are a source-order lever on the 6-floor chassis
- mechanism: emitted 96 `addu s3,s0,v0` vs target `addu s3,v0,s0` mirrors the source's `arg0 + (temp_a1_2 * 2)`.
- probe: F2 = A3 with the addends written `(temp_a1_2 * 2) + arg0`; sandbox + normalized objdump diff.
- result: REFUTED, byte-identical at 6 / 216. GCC canonicalises PLUS operands before RA; the emitted order is a consequence of allocation, not of source order. Re-confirms the same conclusion earlier sessions reached on the 10-floor and 23-floor chassis.
- verdict: KILLED

## [s22] The do { } while (0) RA-weighting construct s4 measured and discarded in 2026-07 is sanctioned policy of record, and it solves s21's cluster-A inequality pri(arg1-carrier) > 2142 directly and at zero emitted cost.
- mechanism: flow.c increments REG_N_REFS(regno) by loop_depth, which is 2 inside the NOTE_INSN_LOOP_BEG/END pair a do-while(0) leaves behind; that numerator feeds global.c:635-656 allocno_compare's pri = floor_log2(n_refs)*n_refs*10000*size/live_length. Wrapping an EXISTING arg1-referencing call pair supplies s21's solution (a) (nrefs >= 8 at livelen 92) without adding an instruction, which is exactly the property s21's P5 duplicated-call route could not have.
- probe: Re-read .claude/rules/do-while-zero-exception.md end-to-end (owner ruling 2026-07-06 abolishes the pre-existing scoping to the reorg.c label-note mechanism and sanctions the wrap for ANY codegen effect incl. register allocation, with a mandatory inline FAKE annotation and no exhaustion gate for single-level wraps). Then wrapped each of the three existing func_80032854(..., arg1, ...) call pairs on s21's V1 chassis (bodies A1/A2/A3/B1/B2/C1/C2/D1-D4), read ALLOCDBG from the instrumented cc1 for each, and scored every body with sandbox --disable all.
- result: Body A3 (V1 + one wrap around the `temp_v0_3 == temp_v1_3` arm's two calls) = 6 / 216 against V1's 17 and the inherited candidate's 10. ALLOCDBG: nrefs(arg1) 7 -> 9 and nrefs(arg0) 19 -> 21, EVERY live length byte-identical, arg1 pri 1521 -> 2934 takes $s2 and temp_s3 stays 2142 and takes $s3 - target's seats - with every other callee-saved seat ($s0/$s1/$s4/$s5/$s6) unchanged. The normalized objdump diff has no entry at emitted 83-88 or 148, so cluster A, the store sink and the arm store are closed simultaneously. Any one of the three call pairs gives the identical 6 (A3 = B2 = C2); any two give 6; all three give 24.
- verdict: CONFIRMED

## [s22] Wrapping only ONE call of a pair is enough, since nrefs 8 already satisfies the inequality.
- mechanism: pri at nrefs 8 / livelen 92 is 2608 > 2142, so the seats should flip with half the wrap.
- probe: A1/A2 (== arm), B1 (> path), C1 (shared do_calls); ALLOCDBG + sandbox on each.
- result: The seats DO flip (nrefs 8, livelen 94, pri 2553 -> $s2), but all four bodies score 10 / 216 against A3's 6. The loop note lands BETWEEN the two calls, inside arg1's live range: livelen(arg1) grows 92 -> 94 and the call block's scheduling is perturbed. The wrap must bracket both calls.
- verdict: KILLED

## [s22] s20's W2b cluster-B hoist becomes free once the store is no longer sunk into the first jal delay slot (s20's frontier item 3).
- mechanism: s20 attributed W2b's +2 to two non-coalescing `move v0,v1` copies caused by sel5 getting $v1, and predicted they would land in different scheduling positions - and coalesce - on a store-pinned chassis.
- probe: F1 = A3 + s20's W2b construction verbatim (s16 sel5 = 0x19 scoped to the == 5 subtree, sel5 = 0xB in the var_s1 == 0 arm, var_v0_2 = sel5 before goto block_48); sandbox.
- result: F1 = 8 / 216 against A3's 6 - exactly the same +2 W2b paid on the s16 chassis (10 -> 12). The hoist family's price is a property of the family, not of the chassis.
- verdict: KILLED

## [s22] Cluster B's delay-slot refusal can be flipped by making the branch predicted-taken, via a NOTE_INSN_LOOP_BEG immediately preceding its target label.
- mechanism: reorg.c:1379 mostly_true_jump scans back from the branch's target label through NOTEs and returns 2 (strongly predicted taken) on a NOTE_INSN_LOOP_BEG; fill_eager_delay_slots (reorg.c:3784) uses that prediction to fill from the TARGET thread, which is exactly where target's stolen `li v0,1` comes from at emitted 126 and, symmetrically, at emitted 45-48.
- probe: Six bodies. G1 (wrap the D_800A38A8 = 1 / D_800A3876 = -1 target block), G2/G3 (wrap the 0x19/0xB selection), G5 (wrap the range-check `return 1;`), G6 (wrap the whole range-check `if`); then H4, built specifically to place the note on the correct side - the `||` test respelled `if (!(A || B)) goto rare5;` with `do { rare5: D_800A38A8 = 1; D_800A3876 = -1; } while (0);` so the branch's own target label sits immediately after the loop note; and H5/H6, the same shape applied to the range-check chain.
- result: G1/G2/G3/G5 are byte-neutral at 6 / 216. G1's neutrality has a named cause: expand_end_cond emits the if-false label BEFORE the following statement expands, so a `do {` written after the `if` puts the note on the far side of the backward scan. G6 = 14 / 213. H4, which DOES place the note correctly, is also byte-neutral at 6 / 216. H5/H6 = 10 / 213, regressive - the `return 1` cross-jumps into the shared epilogue and the insn count falls below target's 215. Prediction is therefore not cluster B's dial, and E-s18-8's insn_sets_resource_p(trial, &opposite_needed) attribution stands unchallenged (that test runs whichever way the branch is predicted).
- verdict: KILLED

## [s22] The two residual commutative addu operand orders are a source-order lever on the 6-floor chassis.
- mechanism: emitted 96 `addu s3,s0,v0` vs target `addu s3,v0,s0` mirrors the source's `arg0 + (temp_a1_2 * 2)`.
- probe: F2 = A3 with the addends written `(temp_a1_2 * 2) + arg0`; sandbox + normalized objdump diff.
- result: Byte-identical at 6 / 216. GCC canonicalises PLUS operands before RA; the emitted order is a consequence of allocation, not of source order. Re-confirms the same conclusion earlier sessions reached on the 23- and 10-floor chassis.
- verdict: KILLED

## [s23] E-s19-7's option (a) is spellable in pure C: putting the fall-through selection block's `$v0` write on the OTHER SIDE of its own branch (as that branch's TARGET thread) fills BOTH delay slots, because a target-thread steal leaves no update_block marker.
- mechanism: reorg.c:2270 `update_block` returns EARLY when `INSN_FROM_TARGET_P (insn)`, so a `fill_slots_from_thread` steal out of a branch's TARGET thread leaves no `(use (insn N))` marker in the stream, whereas `fill_simple_delay_slots`' backward steal always does. The marker between `code_label 347` and the fall-through selection is the only thing that re-sets `$v0` after the label flush in `mark_target_live_regs`' walk, which is what makes `insn_sets_resource_p (trial, &opposite_needed)` refuse `li v0,1` for the `beqz $v0` slot (E-s19-4). Ordering matters too: `fill_eager_delay_slots` walks insns in order, so the `beqz` is processed BEFORE the selection branch it is poisoned by, and a selection branch still unfilled at that moment cannot poison anything.
- probe: three bodies on the s22 A3 chassis (floor 6 / 216), all semantics-preserving respellings of the `== 5` selection only. X1: `if (var_s1 != 0) goto sel19; var_v0_2 = 0xB; goto block_48; sel19: var_v0_2 = 0x19; goto block_48;`. X2: the same but the `sel19` arm carries its own copy of the (real, already-existing) store, `*(s16 *)(arg0 + 0x286) = 0x19; goto block_49;`. X3: the mirrored sense, 0xB edge as branch target.
- result: **X2 = 4 / 215 - the floor drops 6 -> 4 and the instruction count matches target for the first time in 23 sessions.** The normalized diff has NO entry at the `beqz v0` slot and NO surplus `li v0,1`: cluster B, open since s3 and named since s18, is CLOSED, and unlike the whole W1/W2b/F1 hoist family it costs ZERO (the hoist family's +2 is chassis-independent, s22 F1). X1 = 6 (byte-identical to s22: with the arm going to the SHARED block_48 store, the `0x19` write is still emitted before the branch, so fill_simple still steals it backward and still leaves the marker - the arm MUST carry its own store). X3 = 9 (GCC re-inverts the branch sense away from target's `bnez s1`, same re-inversion as s20's W3).
- verdict: CONFIRMED

## [s23] The last residual delay-slot divergence (emitted 45-47) can be bought by denying jump_insn 78 a stealable fall-through, i.e. by relocating the range check's `return 1;` away from the chain's fall-through position.
- mechanism: DBRDBG on the 4-floor body shows `simp insn=84 trial=82 elig=1`, i.e. fill_simple ALREADY puts `li v0,1` (insn 82) into the `j`'s slot exactly as target does; a later pass then runs `fill_slots_from_thread` on jump_insn 78 (the chain's last `beq`) with `thread=84 opp=270 own=1 tif=0` and STEALS the delay insn back out of the fall-through sequence (`WINNER insn=78 trial=84`). If the `return 1;` block is moved elsewhere, insn 78's fall-through becomes a bare `j` with nothing stealable.
- probe: Q1 = the 4-floor body with the range-check exit spelled `goto ret1;` and `ret1: return 1;` placed at the very end of the function; Q2 = the same with `ret1: return 1;` placed immediately before `block_13`.
- result: KILLED. Q1 = 10 / 215, Q2 = 8 / 216. Relocating the constant-1 exit block costs 4-6 points elsewhere to buy at most the 2 points at emitted 45-47 - the same direction s5's H22 measured for shared-return relocation on the 34-floor chassis, now re-measured on the 4-floor chassis. The steal is governed by `opposite_needed` (the flow live-in of the `beq`'s TARGET block, oppregs=0x20000380, no `$v0`), so the lever is a LIVENESS question at that block, not a placement question.
- verdict: KILLED

## [s23] E-s19-7's option (a) is spellable in pure C: moving the fall-through selection block's $v0 write to the OTHER SIDE of its own branch (making it that branch's target-thread head) fills BOTH delay slots, because a target-thread steal leaves no update_block marker.
- mechanism: reorg.c:2270 update_block returns EARLY when INSN_FROM_TARGET_P, so fill_slots_from_thread steals out of a TARGET thread leave no `(use (insn N))` marker, whereas fill_simple_delay_slots' backward steal always emits one. That marker, sitting between code_label 347 and the fall-through selection, is the only thing that re-sets $v0 after the label flush in mark_target_live_regs' walk, which is what makes insn_sets_resource_p(trial,&opposite_needed) refuse `li v0,1` for the beqz's slot (E-s19-4). Ordering reinforces it: fill_eager_delay_slots walks insns in order, so the beqz is processed while the selection branch is still unfilled and cannot poison it.
- probe: Three semantics-preserving respellings of the `== 5` selection only, on the s22 A3 chassis (6/216). X1: `if (var_s1 != 0) goto sel19; var_v0_2 = 0xB; goto block_48; sel19: var_v0_2 = 0x19; goto block_48;`. X2: same but the sel19 arm carries its own copy of the existing store, `*(s16 *)(arg0 + 0x286) = 0x19; goto block_49;`. X3: mirrored sense (0xB edge as branch target). sandbox --disable all + normalized objdump diff for each; BB2_DBR_DEBUG dump plus -da dumps on X2.
- result: X2 = 4 / 215 - floor 6 -> 4 and the instruction count matches target for the first time in 23 sessions; the normalized diff has NO entry at the beqz slot and no surplus `li v0,1`. Emitted 128-131 stay byte-identical to target (the duplicated store re-merges in jump2), so the duplication is invisible in the bytes. X1 = 6 (byte-identical to s22 - with the arm going to the SHARED store, jump2 re-merges and `li v0,25` returns to before the branch, restoring fill_simple's marker; the arm MUST own its store). X3 = 9 (GCC re-inverts the branch sense away from target's `bnez s1`, same as s20's W3/W2a).
- verdict: CONFIRMED

## [s23] The last delay-slot divergence (emitted 45-47) can be bought by denying jump_insn 78 a stealable fall-through, i.e. by relocating the range check's `return 1;` out of the chain's fall-through position.
- mechanism: DBRDBG on the 4-floor body shows fill_simple ALREADY produces target's arrangement (`simp insn=84 trial=82 refset=0 setset=0 setneed=0`, `elig=1`: `li v0,1` goes into the `j`'s slot). A later fill_slots_from_thread on jump_insn 78 (`thr insn=78 thread=84 opp=270 own=1 tif=0 oppregs=20000380`, then `WINNER insn=78 trial=84`) steals the delay insn back out of the fall-through sequence via steal_delay_list_from_fallthrough. Relocating the `return 1;` block would leave insn 78's fall-through a bare `j` with nothing stealable.
- probe: Q1 = the 4-floor body with the range-check exit spelled `goto ret1;` and `ret1: return 1;` at the very end of the function; Q2 = the same with `ret1: return 1;` immediately before block_13. sandbox --disable all on both.
- result: KILLED. Q1 = 10 / 215, Q2 = 8 / 216. The relocation costs 4-6 points elsewhere to buy at most the 2 points at emitted 45-47 (the same direction s5's H22 measured on the 34-floor chassis, now re-measured on the 4-floor chassis). The steal is gated by opposite_needed = 0x20000380, the exact flow live-in of the beq's TARGET block (resolved cleanly from barrier 85, not the find_basic_block -1 fallback), which lacks $v0 - so the lever is LIVENESS at that block, not placement.
- verdict: KILLED
