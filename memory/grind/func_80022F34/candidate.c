/* func_80022F34 — MATCHED BODY (grind s10, rederive, 2026-09-05)
 * ============================================================================
 * STATUS: sandbox func_80022F34 --disable all == 0, rules_dropped 0,
 *         build_insns 70 == target_insns 70, and FULL-TREE verify-oracle
 *         build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (oracle),
 *         all measured THIS session with the three edits below in place.
 *         Zero cheat-asm, zero rules, zero FAKE constructs. Pure C.
 *
 * THIS IS AN INTEGRATION HANDOFF, not a candidate: two of the three required
 * edits are OUTSIDE a grind session's allowed surface, so the tree was
 * reverted to clean after measurement. The three edits are:
 *
 *   (1) include/code6cac.h:467   extern s32 D_801027BC;
 *                             -> extern s32 D_801027BC[][5];
 *   (2) include/code6cac.h:450   extern u8  D_80102782;
 *                             -> extern u8  D_80102782[];
 *   (3) maspsx_label_nop_funcs.txt: append a line `func_80022F34`
 *
 *   (4) src/code6cac.c:2467      INCLUDE_ASM("asm/funcs", func_80022F34);
 *                             -> the body below
 *
 * Exact diffs banked at tmp/grind/func_80022F34/s10/HANDOFF-header.diff and
 * .../HANDOFF-maspsx-label-nop.diff; the matched src file at
 * .../code6cac.c.matched; the score-0 objdump at .../matched_dis_score0.txt.
 *
 * ----------------------------------------------------------------------------
 * WHAT CHANGED vs s1-s9 (and why nine sessions measured 11)
 * ----------------------------------------------------------------------------
 * The whole 11-point residual was an OBJECT-MODEL defect, not a codegen defect.
 * D_801027BC is an array of 20-byte (5-word) records but was declared
 * `extern s32 D_801027BC;` — a plain scalar. Every previous form therefore had
 * to spell the access as a per-use pun, `(&D_801027BC)[idx * 5]`, i.e. a
 * pointer-arithmetic expression on the ADDRESS of a scalar.
 *
 * Those two spellings take DIFFERENT expand paths in GCC 2.7.2:
 *
 *   scalar + pun `(&D_801027BC)[idx*5]`  -> a PLUS tree handed to
 *     memory_address(), which runs break_out_memory_refs (explow.c:274-291)
 *     BEFORE GO_IF_LEGITIMATE_ADDRESS and unconditionally force_reg's the
 *     SYMBOL_REF into a pseudo (s9's kill, and it is correct FOR THAT SHAPE).
 *     combine later folds that pseudo back into the two mems, deleting its only
 *     definition, which orphans a REG_DEAD note; distribute_notes
 *     (combine.c:10829-10846) then emits `(use (reg N))` after the preceding
 *     CODE_LABEL, the pseudo gets no hard reg, reload homes it to a stack slot
 *     -> vars=8 -> `subu $sp,$sp,40` vs target's 32 -> 10 frame-offset diffs.
 *
 *   2-D array `D_801027BC[idx][0]`       -> the outer ARRAY_REF has ARRAY type,
 *     so expand builds the address symbolically and the mem is emitted DIRECTLY
 *     as `(mem/s:SI (plus:SI (symbol_ref "D_801027BC") (reg 98)))` at expand
 *     time — the mips.h:2325-2349 "pretend the MIPS supports a constant address
 *     + a register" clause, reached without ever passing through
 *     break_out_memory_refs. Evidence: tmp/grind/func_80022F34/s10/
 *     rtl-arrayref-no-address-pseudo.txt, insn 104. There is no address pseudo,
 *     so there is no combine fold, no orphaned REG_DEAD note, no `(use ...)`
 *     strand, no unallocated pseudo, and no phantom frame slot: vars=0,
 *     `subu $sp,$sp,32`, and the two per-access lui/%lo pairs are already
 *     present at expand rather than manufactured by combine.
 *
 * This FALSIFIES the class kill banked by s9 ("the mips.h const+reg clause is
 * unreachable at expand for ANY symbol+runtime-variable address in this fork").
 * It is unreachable for a PLUS built from &scalar; it is reached by an
 * ARRAY_REF on an array-typed declaration. The s8 chain's first link was
 * spelling-invariant only within the scalar-declaration object model.
 *
 * CONTROL MEASUREMENTS (this session, same chassis, maspsx gate ON so the
 * load-delay nop is not part of the score):
 *   flat 1-D `extern s32 D_801027BC[];` + `D_801027BC[idx * 5]` -> score 10
 *       (the +8 phantom slot is BACK: a 1-D ARRAY_REF with a runtime index
 *        collapses to the same PLUS tree as the pun and re-enters
 *        break_out_memory_refs).
 *   2-D  `extern s32 D_801027BC[][5];` + `D_801027BC[idx][0]`   -> score 0
 * The 2-D shape is load-bearing; it is not cosmetic.
 *
 * The residual 1 point with the header fix alone is the maspsx `.L`-label
 * load-delay blind spot, the documented store-value-consumer variant:
 *   `lhu $v0,0($s2)` / `.L80022FD0:` / `sh $v0,8($a0)`
 * — exactly the gnd_get_fog precedent in .claude/rules/maspsx-label-nop-gate.md.
 * Adding func_80022F34 to maspsx_label_nop_funcs.txt takes 1 -> 0, and
 * verify-oracle confirmed no index cascade into code6cac.c's siblings
 * (full build SHA1 == oracle).
 *
 * D_80102782[] (edit 2) is required for CHEAT COMPLIANCE, not for bytes: the
 * old `(&D_80102782)[i]` is a per-use declaration pun and an auto-FAIL under
 * the layer-1 INDEXED-ACCESS rule. Declaring the symbol as the array it
 * demonstrably is (the target indexes it with a computed register) is the
 * sanctioned declaration-level fix and is byte-neutral.
 * ============================================================================
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *rec = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(rec + 6) != 0) {
            s32 mode = D_800A38DC;

            switch (mode) {
                case 0:
                    *(s16 *)(rec + 8) = D_80102782[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(rec + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = *(s16 *)(rec + 0x4A);
                s32 val1 = D_801027BC[idx1][0];
                rec = *(u8 **)rec;
                {
                    s16 idx2 = *(s16 *)(rec + 0x4A);
                    func_80055138(i, val1, D_801027BC[idx2][0]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
