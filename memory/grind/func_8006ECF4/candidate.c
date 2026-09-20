/* func_8006ECF4 — session 6 (SYNTHESIS). Floor 11 -> 2 this session.
 *
 * WHAT CHANGED VS THE s3/s4/s5 BANKED CHASSIS (which sat at 11 for three
 * sessions): the s2-s5 body was SEMANTICALLY WRONG in one place, and that
 * wrongness was the whole plateau. Re-reading asm/funcs/func_8006ECF4.s
 * lines 84-160 in this session showed the block at .L8006EE98 (reached after
 * EVERY one of the 5 explicit jump-table cases has already stored its own
 * s.p0) does `sll v0,s1,16; beqz v0,.L8006EF14` — i.e. when i == 0 the target
 * BRANCHES INTO THE SAME SHARED BLOCK the switch default and the
 * out-of-range path use (.L8006EF10/.L8006EF14) and OVERWRITES s.p0 with
 * `s0 + sel*12`. The s2 evidence entry had actually spotted this double-write
 * ("overwritten a SECOND time by the default formula whenever s1==0") but the
 * transcription never implemented it: the old body spelled the i==0 case as a
 * no-op (`if (i != 0) { ...LoadImage guard... }` with no else), so for i==0
 * with sel in {0,3,12,13,14} the old body computed a DIFFERENT s.p0 than the
 * game does. Fixing the semantics is what unlocked everything else.
 *
 * Measured ladder this session (every number from
 * `sandbox func_8006ECF4 --disable all`, edits live in src/text1b.c):
 *
 *   11  (s3/s4/s5 chassis re-confirmed at dispatch; 211 insns, 2 over target)
 *   28  H8  — SHARED-DEFAULT TOPOLOGY. Dropped the redundant outer
 *             `if (sel < 15) {...} else {...}` wrapper entirely and gave the
 *             i==0 path its real target: the switch default and the
 *             i==0 test both reach ONE `s.p0 = (void *)(s0 + sel*12);` block.
 *             Raw score went UP (11 -> 28) but build_insns went 211 -> 209,
 *             EXACTLY matching target for the first time, and the residual
 *             collapsed to one root cause (a callee-save rotation). This is
 *             why raw score alone is a bad stopping signal here: the
 *             11-chassis was 2 insns long AND semantically wrong; the
 *             28-chassis was length-exact and semantically right.
 *   24  H9  — LOOP-BOUND SPELLING. `D_800A35B0 + 1 + D_800A3554` (NOT the
 *             s3-killed `D_800A35B0 + D_800A3554 + 1`, and NOT s6's own first
 *             try `D_800A35B0 + (D_800A3554 + 1)`). Read our own cc1 output
 *             (tmp/grind/func_8006ECF4/dumps/text1b.s): fold reassociates
 *             `X + 1 + Y` into `Y + (X + 1)`, so to get target's
 *             `addiu v0,v0,1` on the D_800A3554 (lh) value and
 *             `addu v1,v1,v0` accumulating into the D_800A35B0 (lw) value,
 *             the SOURCE must read `D_800A35B0 + 1 + D_800A3554`. Closed both
 *             source-level lh/lw load-order hunks (head and loop-tail) at once.
 *    4  H10 — DUPLICATED DEFAULT STATEMENT (the allocator lever). The 20
 *             remaining operand-only hunks were ONE fact: target puts the loop
 *             counter in $s1 and the `s3 + 0xC` base in $s0; we had them
 *             swapped. Read tmp/grind/func_8006ECF4/dumps/text1b.greg:
 *             ";; 7 regs to allocate: 77 76 78 75 72 74 131" is global.c's
 *             allocno_compare order, and 78 (the s16 counter, n_refs 15,
 *             live_length 140) beat 75 (the base, n_refs 13, live_length 139)
 *             because floor_log2(15)*15/140 > floor_log2(13)*13/139. Writing
 *             `s.p0 = (void *)(s0 + sel * 12);` a SECOND time in the switch
 *             default arm (instead of jumping to the shared label from there)
 *             raises the base pseudo's loop-weighted n_refs from 13 to 17,
 *             floor_log2 3 -> 4, which flips the allocation order so the base
 *             takes $s0 and the counter takes $s1 — and jump2 cross_jump
 *             then re-merges the two identical `s.p0 = ...; goto p1_dispatch;`
 *             blocks back into the single block the target has, so the
 *             duplicate costs ZERO instructions. This is the SOTN-sanctioned
 *             duplicated-statement-into-arms family
 *             (.claude/rules/duplicated-statement-into-arms.md): a REAL
 *             statement, duplicated into a real arm, byte-neutral on re-merge.
 *    2  H11 — INDEX STAGING. Naming `c12 = c * 12` alongside the existing
 *             `b2 = b * 2` (so the index reads `D_8009BC40[b2 + c12]`) closed
 *             the last two operand-only hunks (`addu v1,v1,v0` vs
 *             `addu v0,v0,v1`, and the dependent `addu at,at,v1`). s3 and s5
 *             had both KILLED this region as unimprovable, but both measured
 *             it on the semantically-wrong 11-chassis; on the corrected
 *             chassis the same region moves. (`idx = b2 + c*12` as a single
 *             named intermediate also measures 2; `c12` was chosen as the
 *             more natural reading — c is the row, b the column of a 6-wide
 *             2-byte-entry table.)
 *
 * WHAT IS LEFT (score 2, and it is NOT a codegen question):
 *   `--diff` at score 2 reports 209 target insns / 209 ours, 24 hunks,
 *   **0 source-level, 1 operand-only, 23 not-scored**. The single scored hunk
 *   is the switch dispatch load: target `lw v0,%lo(jtbl_800159D0)(at)`,
 *   ours `lw v0,24(at)` with an `R_MIPS_LO16 .rodata` reloc — GCC put our
 *   ADDR_VEC at text1b.o(.rodata)+24, behind func_8006B578 24-byte table,
 *   while the original table lives at 0x800159D0, which today is still
 *   hand-transcribed as `const u32 jtbl_800159D0[15]` in
 *   src/text1a_b_mid_rodata.c:26. This is EXACTLY the residual sibling
 *   func_8006B578 carried for 12 sessions, and its 2026-09-16 integration
 *   handoff is already in the tree (see the header comment of
 *   src/text1a_b_mid_rodata.c and bb2.ld:59-62). It cannot be reached from C
 *   in this file: bb2.ld orders the rodata run
 *   text1a_b_pre_rodata.o -> text1b.o -> text1a_b_mid_rodata.o, so everything
 *   between 0x80015988 and 0x800159D0 must ALSO be emitted by text1b.o before
 *   our table can land at 0x800159D0. That means, in emission order:
 *     0x80015988 jtbl_80015988  — already compiler-emitted (func_8006B578, C)
 *     0x800159A0 "warning\n"    — D_800159A0, referenced by func_8007352C
 *     0x800159B0 jtbl_800159B0  — func_8006E534 switch table; func_8006E534
 *                                 is still INCLUDE_ASM (src/text1b.c:7451,
 *                                 queue-active, distance 222)
 *     0x800159D0 jtbl_800159D0  — OURS
 *   So this function is COUPLED to func_8006E534 (and to whichever TU emits
 *   D_800159A0). Nothing in func_8006ECF4 own C can move it.
 *
 * CONSTRUCT INVENTORY (for the session that submits this body):
 *   - `b`, `c`, `b2`, `c12`, `sel`, `a2`, `v0`, `s3`, `s0`, `i` — all carry
 *     real consumed values; none is dead, none is written-never-read.
 *   - The duplicated `s.p0 = (void *)(s0 + sel * 12);` in the switch default
 *     is the duplicated-statement-into-arms family (real statement, both
 *     copies semantically required by the arm they sit in, byte-neutral after
 *     cross_jump). Whether it needs a FAKE annotation under
 *     .claude/rules/duplicated-statement-into-arms.md is the ONE open
 *     classification question — resolve it (ruling-request if unsure) before
 *     any candidate-ready submission.
 *   - The `goto p1_idx` / `goto p1_fallback` / `goto p1_dispatch` topology is
 *     the mixed-exit-forms family (ordinary C, no annotation) and is unchanged
 *     from s3 H5.
 *   - No register pins, no __asm__, no volatile, no dead stores, no padding.
 *
 * NOTE FOR THE NEXT SESSION: do NOT "restore" the 11-scoring s2-s5 body. It is
 * banked at
 * memory/grind/func_8006ECF4/rejected/s5-shared-p0-missing-i0-overwrite.c
 * and it is WRONG (missing the i==0 p0 overwrite). Score 2 on a correct body
 * beats score 11 on an incorrect one.
 */

extern s16 D_800A3554;
extern s32 D_800A35B0;
extern u8 D_8009BC7C[];
extern u8 D_800A3560[];
extern u8 D_8009BC40[];
extern s16 D_800A3588[];
extern s16 D_800A358C[];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern s32 D_800A3568;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern void *D_800A35C4;
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect_8006ECF4;
extern Rect_8006ECF4 D_800A32F4;

void func_8006ECF4(s32 arg0) {
    S46C s;
    s32 v0;
    s32 s3;
    s32 s0;
    s32 sel;
    s32 a2;
    s16 i;
    Rect_8006ECF4 rectbuf;

    s.one14 = 0x14;
    s.c20 = 0x200;
    s.zero18 = 0;
    s.zero1C = 0;
    s.c24 = 0x100;

    v0 = *(s32 *)arg0;
    s3 = *(s32 *)(v0 + 0x54);
    s0 = s3 + 0xC;

    for (i = 0; i < D_800A35B0 + 1 + D_800A3554; i++) {
        s32 b = D_800A3588[i];
        s32 c = D_800A358C[i];
        s32 b2 = b * 2;
        s32 c12 = c * 12;
        sel = D_8009BC40[b2 + c12];
        if (D_8009BC7C[sel] & 1) {
            s.zero10 = 0;
            if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
                s.byte28 = 1;
            } else {
                s.byte28 = 0;
            }
            *((u8 *)&s + 0x29) = 0x94;
            *((u8 *)&s + 0x2A) = 0x80;
            *((u8 *)&s + 0x2B) = 0x6E;
        } else {
            s.zero10 = 1;
            s.byte28 = 1;
            *((u8 *)&s + 0x2B) = 0;
            *((u8 *)&s + 0x2A) = 0;
            *((u8 *)&s + 0x29) = 0;
        }

        switch (sel) {
        case 12:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x84);
            s.p0 = (void *)(s0 + 0x108);
            break;
        case 13:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x88);
            s.p0 = (void *)(s0 + 0x114);
            break;
        case 14:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x8C);
            s.p0 = (void *)(s0 + 0x120);
            break;
        case 0:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x90);
            s.p0 = (void *)(s0 + 0x12C);
            break;
        case 3:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x94);
            s.p0 = (void *)(s0 + 0x138);
            break;
        default:
            s.p0 = (void *)(s0 + sel * 12);
            goto p1_dispatch;
        }
        if (i == 0) goto default_p0;
        if (D_800A32E8 != sel || D_800A32E9 != D_800A3554) {
            rectbuf = D_800A32F4;
            LoadImage((s32)&rectbuf, a2);
            DrawSync(0);
        }
        goto p1_dispatch;
    default_p0:
        s.p0 = (void *)(s0 + sel * 12);
    p1_dispatch:;

        if (D_800A35B0 != 0) goto p1_idx;
        if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) goto p1_idx;
        if (D_800A35BC != 2) goto p1_fallback;
        if (*(s32 *)((s32)D_800A3568 + 0x14) & 0x20000) goto p1_idx;
    p1_fallback:
        s.p1 = (s32 *)*(s32 *)(s3 + 4);
        goto p1_done;
    p1_idx:
        s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
    p1_done:;

        s.ret = *(s32 *)(arg0 + 4);
        if (i != 0) {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        } else {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        }
    }
}
