/* func_8008AAD4 = PsyQ 4.0 LIBSPU S_SK SpuSetKey — PROVEN candidate (Closer
 * phase-3 session 5, 2026-07-10). Standalone masked mnemonic stream IDENTICAL
 * to build/src/main.o (registers included, 127/127 insns); only diffs are
 * %lo(D_800F7420+4/+6) vs %lo(D_800F7424+0/+2) reloc addend spellings =
 * byte-identical after link (splat split Sony's one _spu_RQ object into two
 * D_ symbols).
 *
 * GROUND TRUTH (tmp/libscan/psyq40/LIBSPU.LIB S_SK relocs, psyq_lib.py):
 *   D_800A2CD4 = _spu_env      (volatile mode flag)
 *   D_800F7420 = _spu_RQ       (ONE symbol, addends 0/2/4/6 = u16[4]:
 *                               key-on pending [0..1], key-off pending [2..3])
 *   D_800A28A0 = _spu_RQmask   D_800A289C = _spu_RQvoice
 *   D_800A2874 = _spu_keystat  D_800A2CDC = _spu_RXX (SPU MMIO base ptr)
 *
 * BLOCKED ON: volatile allowlist grants for D_800F7420/D_800A28A0/
 * D_800A2CD4/D_800A289C (proposal in memory/closer/volatile-grant-proposals.md
 * s2) + the merged u16[4] redeclaration (drop the D_800F7424 extern; only
 * main.c references it). RXX struct volatile members are MMIO (0x1F801D88
 * KEY-ON reg) = type-level legit, no grant needed.
 *
 * Load-bearing discoveries (measured, in order):
 *  - merged _spu_RQ[4] decl: the +4/+6 const-plus addresses are never
 *    address-CSE'd by cse.c -> at-form; the split-symbol spelling base-caches
 *    (that is what HEAD's 42 regfix rules were faking).
 *  - u16 lo/hi copy vars (SOTN new_var idiom) keep the move a2/a3 copies.
 *  - SpuRXX struct-member stores (MEM_IN_STRUCT_P) let sched hoist the
 *    scalar _spu_keystat lw above them in the case-1 else arm.
 *  - volatile RXX members (Sony: volatile SPU_RXX rxx): block reorg
 *    delay-slot steals (case-0 beqz slot nop; j slot nop) and keep the
 *    else-arm nor un-stolen.
 *  - case-1 else arm stages keystat (stat = D_800A2874|vb; stores; store) so
 *    the final sw cross-jump-merges with case 0's tail; case-0 else arm is
 *    the direct RMW form.
 *  - symmetric ~voice_bit everywhere (no notmask local): case-0's shared a0
 *    nor is CSE, case-1's fresh nors fall out (no prior ~ value on that path).
 */
typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned short u16;

extern s32 D_800A2874;
extern volatile s32 D_800A289C;
extern volatile s32 D_800A28A0;
extern volatile s32 D_800A2CD4;
extern s32 D_800A2CDC;
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];   /* +0x188 */
    volatile u16 key_off[2];  /* +0x18C */
} SpuRXX;
extern volatile u16 D_800F7420[4];  /* Sony _spu_RQ (S_SK relocs: one symbol, addends 0/2/4/6) */

void func_8008AAD4(s32 on_off, u32 voice_bit) {
    u16 lo;
    u16 hi;
    u32 hi2;

    voice_bit &= 0xFFFFFF;
    lo = voice_bit;
    hi2 = voice_bit >> 16;
    hi = hi2;

    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7420[0] = lo;
            D_800F7420[1] = hi;
            D_800A28A0 |= 1;
            D_800A289C |= voice_bit;
            if (D_800F7420[2] & voice_bit) {
                D_800F7420[2] &= ~voice_bit;
            }
            if (D_800F7420[3] & hi2) {
                D_800F7420[3] &= ~hi2;
            }
        } else {
            u32 stat = D_800A2874 | voice_bit;
            ((SpuRXX *)D_800A2CDC)->key_on[0] = lo;
            ((SpuRXX *)D_800A2CDC)->key_on[1] = hi;
            D_800A2874 = stat;
        }
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7420[2] = lo;
            D_800F7420[3] = hi;
            D_800A28A0 |= 1;
            D_800A289C &= ~voice_bit;
            if (D_800F7420[0] & voice_bit) {
                D_800F7420[0] &= ~voice_bit;
            }
            if (D_800F7420[1] & hi2) {
                D_800F7420[1] &= ~hi2;
            }
        } else {
            ((SpuRXX *)D_800A2CDC)->key_off[0] = lo;
            ((SpuRXX *)D_800A2CDC)->key_off[1] = hi;
            D_800A2874 &= ~voice_bit;
        }
        break;
    }
}
