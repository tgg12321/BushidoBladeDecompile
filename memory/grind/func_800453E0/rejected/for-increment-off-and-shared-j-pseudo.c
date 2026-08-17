/* REJECTED — func_800453E0, session 1. Honest floor 18 (76/76 insns).
 *
 * This is the HYBRID that first got the instruction count exact and made insns
 * 13..61 structurally identical to target. It is banked because its three
 * remaining defects are each a separate, now-solved lesson — do not re-derive
 * them:
 *
 *  (1) `j = i + 1; func_80045294(j, ...); for (; j < ...; j++)` reuses ONE
 *      variable as the call argument and the shift-loop counter, so GCC
 *      coalesces them into a single pseudo. That lands `i` in $s1 and `j` in
 *      $s0 — the mirror image of target, which keeps i+1 in callee-save $s1
 *      across the jal and copies it into caller-save $t0 for the call-free
 *      inner loop (`addu $t0,$s1,$zero`). Splitting out `s32 next = i + 1;`
 *      and starting the loop `for (j = next; ...)` closed floor 18 -> 6.
 *
 *  (2) `for (i = 0, off = 0; i < D_800A33AC; i++, off += 0x10)` emits `i++`
 *      BEFORE the loop-bound load, leaving nothing to fill the lw's load-delay
 *      slot — maspsx inserts an unfillable nop, one instruction too many. Target
 *      orders it `lw` gp / `addiu $s0,$s0,1` / `slt` / `bnez`. Adopting the
 *      already-matching sibling func_80045510's do-while shape (body ends
 *      `count = D_800A33AC; i += 1;`) closed floor 6 -> 3.
 *
 *  (3) advancing `off` in the for-increment makes it a SOURCE induction
 *      variable, so `off = 0` is emitted at its source position AHEAD of the
 *      LICM-hoisted base addresses; target emits `addu $v1,$zero,$zero` AFTER
 *      them. Writing `off = i << 4;` at the top of the body makes it a
 *      strength-reduction giv whose zero-init loop.c creates in the pre-header
 *      after the movables. Closed floor 3 -> 1.
 *
 * A fourth, subtler point: `SUBTBL[j - 1] = SUBTBL[j]` produces the CORRECT
 * FINAL LINKED BYTES but emits a LO16 reloc against D_800EED10 with addend -16
 * where target's object encodes `%lo(D_800EED00)` with addend 0. score.py does
 * not mask data LO16 addends, so it reads as 1 false distance. Writing the loop
 * against the D_800EED00-based view (`SUBSLOT[j] = SUBSLOT[j + 1]`) is
 * semantically identical, derives both base pointers from one symbol, and closed
 * floor 1 -> 0.
 */

typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} SubEntry;

#define SUBTBL ((SubEntry *)D_800EED10)

void func_800453E0(s32 a0) {
    s32 i;
    s32 j;
    s32 last;
    s32 off;

    for (i = 0, off = 0; i < D_800A33AC; i++, off += 0x10) {   /* defects (2)+(3) */
        if (*(s16 *)((u8 *)D_800EED10 + off) == a0) {
            j = i + 1;                                          /* defect (1) */
            func_80045294(j, -*(s32 *)((u8 *)&D_800EED18 + off));
            if (i < D_800A33AC - 1) {
                for (; j < D_800A33AC; j++) {
                    SUBTBL[j - 1] = SUBTBL[j];
                }
            }
            last = D_800A33AC - 1;
            *(s16 *)((u8 *)D_800EED10 + (last << 4)) = -1;
            *(s32 *)((u8 *)D_800EED1C + (last << 4)) = 0;
            D_800A33AC = last;
            return;
        }
    }
}
