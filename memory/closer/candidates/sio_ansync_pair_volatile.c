/* PROVEN CANDIDATE (Closer phase-3 session 3, 2026-07-10) — BLOCKED on:
 *  (1) owner volatile grant for D_800F1B00 D_800F1B04 D_800F1AF0
 *      D_800F1AF4 D_800F1AF8 (proposal: memory/closer/volatile-grant-proposals.md)
 *  (2) label-drift coupling: asmfix.txt:139-140 anchor SetPacketData rules on
 *      .L761/.L764; removing the fake wrappers in func_8008BEA4/func_8008C184
 *      shifts every later .L number in main.c (measured -5/-6), silently
 *      no-oping those rules and breaking the oracle. Landing this requires the
 *      COMB module's Syncro pair (SetPacketData, cpu_side_move_dir_3) to shed
 *      their label-anchored rules in the same change (module-coherent close).
 *
 * PROOF: tmp/closer/diffsa.sh tmp/closer/sio_test.c func_8008BEA4 build/src/main.o
 *   -> 25/25 IDENTICAL (masked mnemonic stream, registers included)
 *   func_8008C184 -> identical modulo one trailing blank line (25 vs 26).
 * Whole-file parity (tmp/closer/parity_main.sh): only intended insn changes
 *   (sw $4,D_800F1AF0 placement + one #nop) + label renumbering.
 *
 * KEY METHOD FINDING: the banked s-2 measurement "volatile did NOT hold store
 * order" was a SANDBOX-STRIPPER ARTIFACT — un-granted `extern volatile` is
 * cheat-invisible (stripped before scoring; cheat_asm_stripped went 96->98).
 * Real volatile compiles to EXACTLY the target order. Measure volatile physics
 * standalone (diffsa.sh), never through the sandbox.
 *
 * The fake `if ((flag && flag) && flag) { do { } while (0); }` wrappers are
 * REPLACED by honest volatile stores (Sony original semantics: the SIO state
 * is mutated at interrupt time by HandleSio, static @0x8008C9F4 inside the
 * func_8008C464 asm blob; write sites 0x8008CA54/CBFC-CC00/CD28/CE5C/C880).
 */
typedef int s32;
typedef unsigned short u16;

extern s32 D_800F1AFC;
extern volatile s32 D_800F1B00;
extern volatile s32 D_800F1B04;
extern s32 D_800A3044;

extern s32 D_800F1AEC;
extern volatile s32 D_800F1AF0;
extern volatile s32 D_800F1AF4;
extern volatile s32 D_800F1AF8;

/* SioAnsyncRead (LIBCOMB comb static) */
s32 func_8008BEA4(int a0, int a1) {
    s32 *flag = &D_800F1AFC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1B04 = a1;
    D_800F1B00 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        {
            volatile u16 *ptr = (volatile u16 *)(reg + 0xA);
            *ptr |= 0x800;
            *ptr |= 0x20;
        }
    }
    return 0;
}

/* SioAnsyncWrite (LIBCOMB comb static) */
s32 func_8008C184(int a0, int a1) {
    s32 *flag = &D_800F1AEC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1AF4 = a1;
    D_800F1AF0 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        D_800F1AF8 = *(volatile u16 *)(reg + 4) & 0x80;
        *(volatile u16 *)(reg + 0xA) |= 0x400;
    }
    return 0;
}
