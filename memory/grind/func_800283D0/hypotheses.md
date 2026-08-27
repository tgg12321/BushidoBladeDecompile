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
