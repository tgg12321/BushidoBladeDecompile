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
