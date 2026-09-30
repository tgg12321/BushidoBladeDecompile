#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "gte.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */
extern u8 D_8008E914[][8];
extern s32 D_8008EA00[][4];
extern s32 func_8001DB58(void);





/* Extern function declarations */


















extern u16 D_80101F32;





extern void player_SetCharId(s32, s32);







extern u8 D_800A3768;
extern u8 D_800A36A8;




extern s16 *func_8004678C(void);










extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);







extern u8 D_800A384C;
extern u8 D_8008E908[][5];
extern u8 D_8008EC24[][5];
extern s32 ratan2(s32, s32);
extern s32 rand(void);
extern void RotMatrixX(s32, s32 *);
extern void RotMatrixY(s32, s32 *);
extern void RotMatrixZ(s32, s32 *);



extern void eff_Init(void);

extern s32 stage_GetDataPtr(void);











extern s16 Judge;

extern u16 D_8008EBA0;



/* P1/P2 round scores and tiebreakers (per-file declarations: owner rulings Q21-Q25,
 * .claude/rules/no-new-park-categories.md aggregate-merge exception). Declared here
 * as single u8s: every measured counting aggregate spelling of func_800340A0
 * misses the shipped code (constant subscripts put element 0 behind a base
 * register; the index-variable and regrouped-condition spellings that avoid that
 * miss its round-result stores or compares; dummy-index and pointer-alias
 * spellings that match are refused/set aside, Q22/Q23). src/code6cac.c:2135
 * declares the same bytes as D_800A3898[2] / D_800A38AA[2] for func_8001CE60,
 * which indexes them by player and does not produce those accesses from single
 * bytes. The mismatch is kept because no single declaration compiles both files
 * with a counting spelling (Q22/Q23 set-asides excluded).
 * Evidence: memory/grind/func_8001CE60/evidence.md (s3),
 * probes/calib_800340A0/ (Q24-AGREEMENT*.txt). */
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A38AA;
extern u8 D_800A38AB;


extern u8 D_800F65F8;




extern u8 D_80106A78;


extern s32 D_80102410;
extern s32 D_80102408;
extern s32 D_80101FC4;
extern s32 D_80101FBC;
extern s16 D_800A3824;
extern s16 D_800A3876;
extern s16 D_800A38A8;
extern void func_8001F860(s16 *arg0, s32 arg1);
extern void func_8002AB08(s32 a0);

extern void func_800288C8(void);
extern s32 func_80029454(void);
extern void func_80031B24(void);
extern s32 D_801020D8;





extern s32 D_801020FC;






/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_80026DA4(void);
/* Called by func_8002C61C each frame for D_80101F32 modes 0xF and
 * 0x1C-0x21.  s0/s1 are the two 0x44C-stride records at D_80101EC8;
 * the tail passes func_80032854 the midpoint of the two records, offset
 * by the D_8008EB54 row picked from the mode, with D_8008EB6C[row] as
 * its arg1.
 *
 * func_80027A58 and func_80032854 are defined in code6cac_b_tu2.c (further down
 * this file before the 2026-09-30 TU split) and are called here with no
 * prototype in scope (implicit int).  The resulting
 * call_value sets of $v0 are what keep sched1 from giving the func_8002BEA0
 * call its birthing boost, so the `la s0` stays after the jal. */
void func_80026DA4(void) {
    u8 *s0;
    u8 *s1;
    u8 *p;
    s32 timer;
    s32 i;
    s32 idx;
    s32 dir;
    s32 kind;
    s32 pos[3];

    timer = func_8002BEA0();
    s0 = (u8 *)&D_80101EC8;
    D_800A3824 = -1;
    if (D_80101F32 == 0x1C) {
        if (D_80101F08 != 4) goto tail;
        idx = D_800A3876;
        if (idx == -1) goto tail;
        s0 = (u8 *)&D_80101EC8 + idx * 0x44C;
        s1 = (u8 *)&D_80101EC8;
        if (idx == 0) {
            s1 += 0x44C;
        }
        *(s16 *)(s0 + 0x286) = 3;
        *(s16 *)(s1 + 0x286) = 4;
    } else if (D_80101F32 != 0xF) {
        for (i = 0; i < 2; i++) {
            p = s0 + i * 0x44C;
            if (*(s32 *)(p + 0x30) & 0x20) {
                *(s32 *)(p + 0x28C) += *(s16 *)(p + 0x20);
            }
            if (*(s32 *)(p + 0x30) & 0x40) {
                *(s32 *)(p + 0x28C) += *(s16 *)(p + 0x20);
            }
            if (*(u16 *)(p + 0x6A) == 0x1D || *(u16 *)(p + 0x6A) == 0x1E ||
                *(u16 *)(p + 0x6A) == 0x20) {
                if (*(s32 *)(p + 0x2C) & 0x1000) {
                    dir = 1;
                } else if (*(s32 *)(p + 0x2C) & 0x4000) {
                    dir = -1;
                } else {
                    dir = 0;
                }
                *(s32 *)(p + 0x134) -= ((&Judge)[(*(s16 *)(p + 0x1CA) + 0x400) & 0xFFF] * dir) / 256;
                *(s32 *)(p + 0x13C) += ((&Judge)[*(u16 *)(p + 0x1CA) & 0xFFF] * dir) / 256;
            }
        }
        s0 = (u8 *)&D_80101EC8;
        s1 = s0 + 0x44C;
        D_800A389C++;
        if ((s16)D_800A389C >= 0x2B) {
            if (D_80102154 > D_801025A0) {
                D_8010214E = 3;
                D_8010259A = 4;
                if (D_8010237E == 0x21) {
                    func_80027A58((s32 *)s1);
                }
                func_80032854(1, 0x2D, s0 + 0x540, 0);
            } else if (D_801025A0 > D_80102154) {
                D_8010259A = 3;
                D_8010214E = 4;
                if (D_80101F32 == 0x21) {
                    func_80027A58((s32 *)s0);
                }
                func_80032854(0, 0x2D, s0 + 0xF4, 0);
            }
            *(s32 *)(s1 + 0x28C) = 0;
            *(s32 *)(s0 + 0x28C) = 0;
            D_800A389C = 0;
        }
        if (timer > 200) {
            *(s16 *)(s0 + 0x286) = 2;
            *(s16 *)(s1 + 0x286) = 2;
        } else {
            s32 diff = *(s32 *)(s0 + 0xF8) - *(s32 *)(s1 + 0xF8);
            if (diff < 0) diff = -diff;
            if (diff >= 1000) {
                *(s16 *)(s0 + 0x286) = 2;
                *(s16 *)(s1 + 0x286) = 2;
            }
        }
        if (*(u16 *)(s0 + 0x6A) == 0x1D) {
            if (*(s32 *)(s0 + 0x2C) & 0x8000) {
                if (*(s32 *)(s1 + 0x2C) & 0x8000) {
                    *(s16 *)(s0 + 0x286) = 2;
                    *(s16 *)(s1 + 0x286) = 2;
                } else {
                    *(s16 *)(s0 + 0x286) = 2;
                    *(s16 *)(s1 + 0x286) = 5;
                }
            } else if (*(s32 *)(s1 + 0x2C) & 0x8000) {
                *(s16 *)(s0 + 0x286) = 5;
                *(s16 *)(s1 + 0x286) = 2;
            }
        }
    }
    s0 = (u8 *)&D_80101EC8;
tail:
    s1 = s0 + 0x44C;
    if (D_800A3910 == 0) {
        switch (D_80101F32) {
        case 0xF:
            kind = -1;
            break;
        case 0x1C:
            kind = 0;
            break;
        case 0x21:
            kind = 1;
            break;
        case 0x20:
            kind = 2;
            break;
        case 0x1D:
            kind = 3;
            break;
        case 0x1E:
            kind = 4;
            break;
        case 0x1F:
            kind = 5;
            break;
        }
        if (kind >= 0) {
            pos[0] = (*(s32 *)(s0 + 0xF4) + *(s32 *)(s1 + 0xF4)) / 2;
            pos[1] = (*(s32 *)(s0 + 0xDC) + *(s32 *)(s1 + 0xDC)) / 2;
            pos[2] = (*(s32 *)(s0 + 0xFC) + *(s32 *)(s1 + 0xFC)) / 2;
            pos[0] += ((&Judge)[*(u16 *)(s0 + 0x1D8) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            pos[1] += D_8008EB54[kind].unk2;
            pos[2] += ((&Judge)[(*(s16 *)(s0 + 0x1D8) + 0x400) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            func_80032854(0, D_8008EB6C[kind], (u8 *)pos, 0);
        }
    } else {
        D_800A3910--;
    }
}
