/* BEST ADAPTED BODY - src/system.c - func_80080828 == LIBCD/BIOS `getintr`
 * Reference: sotn-decomp src/main/psxsdk/libcd/bios.c:115-208 - transplanted almost
 * verbatim; the algorithm, the 8-byte result FIFO drain, the error mask and the
 * switch(nReg) arms all correspond 1:1.
 *
 * MEASURED: 30   (baseline 354 = INCLUDE_ASM).  build 345 / target 354.
 * This is the largest single reduction in the campaign.
 *
 * Symbol adaptation (SOTN -> BB2), read off the target asm and cross-checked against
 * BB2's already-decompiled CD_sync, which shares this vocabulary:
 *   libcd_CDRegister0/1/2/3 -> D_800A147C / D_800A1480 / D_800A1484 / D_800A1488
 *   Intr.sync/.ready/.c     -> D_800A1494 / D_800A1495 / D_800A1496
 *   D_80039260/68/70        -> D_800F19A0 / D_800F19A8 / D_800F19B0
 *   CD_status / CD_status1  -> D_800A11C4 (a WORD here) / D_800A11C8
 *   CD_nopen -> D_800A11CC ; CD_com -> D_800A11D5 ; debug level -> D_800A11C0
 *   D_80032C68 -> D_800A137C ; D_80032B68 -> D_800A127C ; cmd strings -> D_800A11DC
 *   CdlStatError|SeekError|IdError|ShellOpen -> the literal mask 0x1D
 *   _memcpy(dst, &buf, 8) is INLINED in BB2's revision as a byte loop guarded by a
 *   `dst != 0` test (cd_rescpy below) - the same spelling BB2's CD_sync already uses.
 * BB2 also gates BOTH DiskError printfs on `debug > 0` where SOTN leaves the first
 * (a puts) ungated.
 *
 * RESIDUAL (9 insns short): in three of the five switch arms the target materialises
 * the address of the Intr.sync byte into a register (lui+addiu+sb, 3 insns) where our
 * build uses the 2-insn %lo form (lui+sb); the other two arms agree with our build.
 * It is a per-arm address-in-register choice, not a structural difference - everything
 * before the switch (insns 0-155) is already identical.
 *
 * !! SCAFFOLD WARNING !! The `asm("D_800A...")` alias renames below are a MEASUREMENT
 * scaffold only, NOT part of the proposed body. They exist solely because src/system.c
 * re-declares these same symbols further down the file with different types. Alias
 * renames are in the cheat catalog ([[inline-asm-injection]] sibling family). Landing
 * this body for real requires RECONCILING those duplicate declarations to one
 * canonical decl per symbol, not shipping the aliases. The types used here (D_800A11C4
 * as s32, D_800A11D5 as u8, D_800A11DC as s32[]) are the ones the target asm loads.
 */

extern volatile u8 *D_800A147C_r0 asm("D_800A147C");
extern volatile u8 *D_800A1480;
extern volatile u8 *D_800A1484;
extern volatile u8 *D_800A1488;
extern u8 D_800A1494_s asm("D_800A1494");
extern u8 D_800A1495;
extern u8 D_800A1496;
extern s32 D_800A11C0;
extern s32 D_800A11C4_w asm("D_800A11C4");
extern s32 D_800A11C8;
extern s32 D_800A11CC;
extern u8 D_800A11D5_c asm("D_800A11D5");
extern s32 D_800A127C[];
extern s32 D_800A137C[];
extern s32 D_800A11DC_t[] asm("D_800A11DC");
extern u8 D_800F19A0_b;
extern u8 D_800F19A8_b asm("D_800F19A8");
extern u8 D_800F19B0_b asm("D_800F19B0");
extern u8 D_800161E4, D_800161F0, D_8001620C, D_80016220;
extern void puts(void *);
extern void printf(void *, ...);

static inline void cd_rescpy(u8 *dst, volatile u8 *src) {
    s32 i;
    u8 b;
    if (dst != 0) {
        i = 7;
        do {
            b = *src;
            src++;
            i--;
            *dst = b;
            dst++;
        } while (i != -1);
    }
}

s32 func_80080828(void) {
    volatile u8 nReg;
    volatile u8 buf[8];
    s32 i, j;
    s32 bHasError;

    *D_800A147C_r0 = 1;
    nReg = *D_800A1488 & 0x7;
    if (nReg == 0) {
        return 0;
    }
    bHasError = 0;
    while (nReg != (*D_800A1488 & 7)) {
        nReg = *D_800A1488 & 0x7;
    }
    for (i = 0; i < 8; i++) {
        if ((*D_800A147C_r0 & 0x20) == 0) {
            break;
        }
        buf[i] = *D_800A1480;
    }
    for (j = i; j < 8; j++) {
        buf[j] = 0;
    }
    *D_800A147C_r0 = 1;
    *D_800A1488 = 7;
    *D_800A1484 = 7;
    if (nReg != 3 || D_800A137C[D_800A11D5_c]) {
        if (!(D_800A11C4_w & 0x10) && (buf[0] & 0x10)) {
            D_800A11CC++;
        }
        D_800A11C4_w = buf[0];
        D_800A11C8 = buf[1];
        bHasError = D_800A11C4_w & 0x1D;
    }
    if (nReg == 5) {
        if (D_800A11C0 > 0) {
            printf(&D_800161E4);
        }
        if (D_800A11C0 > 0) {
            printf(&D_800161F0, D_800A11DC_t[D_800A11D5_c], D_800A11C4_w,
                   D_800A11C8);
        }
    }
    switch (nReg) {
    case 3:
        if (bHasError) {
            D_800A1494_s = 5;
            cd_rescpy(&D_800F19A0_b, buf);
            return 2;
        }
        if (D_800A127C[D_800A11D5_c]) {
            D_800A1494_s = 3;
            cd_rescpy(&D_800F19A0_b, buf);
            return 1;
        }
        D_800A1494_s = 2;
        cd_rescpy(&D_800F19A0_b, buf);
        return 2;
    case 2:
        D_800A1494_s = bHasError ? 5 : 2;
        cd_rescpy(&D_800F19A0_b, buf);
        return 2;
    case 1:
        if (bHasError && i == 1) {
            bHasError = 0;
        }
        D_800A1495 = bHasError ? 5 : 1;
        cd_rescpy(&D_800F19A8_b, buf);
        *D_800A147C_r0 = 0;
        *D_800A1488 = 0;
        return 4;
    case 4:
        D_800A1496 = 4;
        D_800A1495 = D_800A1496;
        cd_rescpy(&D_800F19B0_b, buf);
        cd_rescpy(&D_800F19A8_b, buf);
        return 4;
    case 5:
        D_800A1495 = 5;
        D_800A1494_s = D_800A1495;
        cd_rescpy(&D_800F19A0_b, buf);
        cd_rescpy(&D_800F19A8_b, buf);
        return 6;
    default:
        puts(&D_8001620C);
        printf(&D_80016220, nReg);
        return 0;
    }
}
