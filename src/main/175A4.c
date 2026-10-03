/* One game function, func_80026DA4. .text 0x80026DA4 (ROM 0x175A4). Start boundary: LEGACY (a
 * tooling split, no evidence either way); also the first file of the EXPAND_LB run. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"
#include "bb2_const.h"

/* Extern data declarations */
extern u8 D_8008E914[][8];
extern s32 D_8008EA00[][4];
extern s32 func_8001DB58(void);





/* Extern function declarations */



















extern void player_SetCharId(s32, s32);











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















/* P1/P2 round scores and tiebreakers (per-file declarations: owner rulings Q21-Q25,
 * .claude/rules/no-new-park-categories.md aggregate-merge exception). Declared here
 * as single u8s: every counting aggregate spelling of func_800340A0 misses the
 * shipped code (constant subscripts put element 0 behind a base register; the
 * index-variable and regrouped-condition spellings that avoid that miss its
 * round-result stores or compares; dummy-index and pointer-alias spellings are
 * refused, Q22/Q23). src/code6cac.c declares the same bytes as D_800A3898[2] /
 * D_800A38AA[2] for func_8001CE60, which indexes them by player and does not
 * produce those accesses from single bytes. The mismatch is kept because no single
 * declaration compiles both files with a counting spelling.
 * Evidence: pre-slim-2026-10-01:memory/grind/func_8001CE60/evidence.md. */
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A38AA;
extern u8 D_800A38AB;








extern void func_8001F860(s16 *arg0, s32 arg1);
extern void func_8002AB08(s32 a0);

extern void func_800288C8(void);
extern s32 func_80029454(void);
extern void func_80031B24(void);






/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

/* Updates the two active player records in practice modes. The timed contest
 * chooses a winner, the distance/input checks choose follow-up states, and
 * the tail emits a mode-specific effect at the players' midpoint.
 *
 * FAKE: leave the two void callees undeclared, as in the matched PS1 SOTN
 * caller below. Their unused implicit-int call_value results make v0 have
 * multiple sets, removing sched.c's birthing boost from the timer call and
 * keeping the record-base load after it. Correct void declarations miss 7
 * instructions.
 * SOTN: src/main/psxsdk/libsnd/ssclose.c:8 and
 * src/main/psxsdk/libsnd/vmanager.c:1107 @db41b28eee52969244a52cc269c8163d1ed8826a */
void func_80026DA4(void) {
    Unk80101EC8Record *record;
    Unk80101EC8Record *partner;
    Unk80101EC8Record *current;
    s32 timer;
    s32 i;
    s32 idx;
    s32 dir;
    s32 kind;
    s32 pos[3];

    timer = func_8002BEA0();
    record = D_80101EC8;
    D_800A3824 = -1;
    if ((u16)D_80101EC8[0].unk_6A == 0x1C) {
        if (D_80101EC8[0].unk_40 != 4) goto tail;
        idx = D_800A3876;
        if (idx == -1) goto tail;
        /* FAKE: reuse the record pointers for the selected pair, then restore
         * the fixed pair at the tail join. This keeps the selected pointers
         * in the call-preserved allocation used by the original.
         * SOTN: src/st/lib/unk_3B53C.c:41-55 @db41b28eee52969244a52cc269c8163d1ed8826a */
        record = &D_80101EC8[idx];
        partner = D_80101EC8;
        if (idx == 0) {
            partner++;
        }
        record->unk_286 = 3;
        partner->unk_286 = 4;
    } else if ((u16)D_80101EC8[0].unk_6A != 0xF) {
        for (i = 0; i < 2; i++) {
            current = record + i;
            if (current->unk_24.pressed & 0x20) {
                current->unk_28C += current->unk_20;
            }
            if (current->unk_24.pressed & 0x40) {
                current->unk_28C += current->unk_20;
            }
            if ((u16)current->unk_6A == 0x1D || (u16)current->unk_6A == 0x1E ||
                (u16)current->unk_6A == 0x20) {
                if (current->unk_24.held & 0x1000) {
                    dir = 1;
                } else if (current->unk_24.held & 0x4000) {
                    dir = -1;
                } else {
                    dir = 0;
                }
                current->unk_134.vx -= (Judge[(current->unk_1C8.vy + 0x400) & 0xFFF] * dir) / 256;
                current->unk_134.vz += (Judge[(u16)current->unk_1C8.vy & 0xFFF] * dir) / 256;
            }
        }
        /* FAKE: re-materialize the same record base after the drift loop;
         * GCC cse.c's basic-block boundary retains the original base load.
         * SOTN: src/weapon/w_011.c:343-348 @db41b28eee52969244a52cc269c8163d1ed8826a */
        record = D_80101EC8;
        partner = record + 1;
        D_800A389C++;
        if ((s16)D_800A389C >= 0x2B) {
            if (D_80101EC8[0].unk_28C > D_80101EC8[1].unk_28C) {
                D_80101EC8[0].unk_286 = 3;
                D_80101EC8[1].unk_286 = 4;
                if ((u16)D_80101EC8[1].unk_6A == 0x21) {
                    func_80027A58((s32 *)partner);
                }
                func_80032854(1, 0x2D, (u8 *)&partner->unk_F4, (s16 *)0);
            } else if (D_80101EC8[1].unk_28C > D_80101EC8[0].unk_28C) {
                D_80101EC8[1].unk_286 = 3;
                D_80101EC8[0].unk_286 = 4;
                if ((u16)D_80101EC8[0].unk_6A == 0x21) {
                    func_80027A58((s32 *)record);
                }
                func_80032854(0, 0x2D, (u8 *)&record->unk_F4, (s16 *)0);
            }
            partner->unk_28C = 0;
            record->unk_28C = 0;
            D_800A389C = 0;
        }
        if (timer > 200) {
            record->unk_286 = 2;
            partner->unk_286 = 2;
        } else {
            s32 diff = record->unk_F4.y - partner->unk_F4.y;
            if (diff < 0) diff = -diff;
            if (diff >= 1000) {
                record->unk_286 = 2;
                partner->unk_286 = 2;
            }
        }
        if ((u16)record->unk_6A == 0x1D) {
            if (record->unk_24.held & 0x8000) {
                if (partner->unk_24.held & 0x8000) {
                    record->unk_286 = 2;
                    partner->unk_286 = 2;
                } else {
                    record->unk_286 = 2;
                    partner->unk_286 = 5;
                }
            } else if (partner->unk_24.held & 0x8000) {
                record->unk_286 = 5;
                partner->unk_286 = 2;
            }
        }
    }
    /* FAKE: restore record zero at the three-path join after selected-pair
     * work; preserve the pointer reuse documented above.
     * SOTN: src/st/lib/unk_3B53C.c:41-55 @db41b28eee52969244a52cc269c8163d1ed8826a */
    record = D_80101EC8;
tail:
    /* FAKE: restore the second fixed record with the first at this join.
     * SOTN: src/st/lib/unk_3B53C.c:41-55 @db41b28eee52969244a52cc269c8163d1ed8826a */
    partner = record + 1;
    if (D_800A3910 == 0) {
        switch ((u16)D_80101EC8[0].unk_6A) {
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
            pos[0] = (record->unk_F4.x + partner->unk_F4.x) / 2;
            pos[1] = (record->unk_D8.y + partner->unk_D8.y) / 2;
            pos[2] = (record->unk_F4.z + partner->unk_F4.z) / 2;
            pos[0] += (Judge[(u16)record->unk_1D8 & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            pos[1] += D_8008EB54[kind].unk2;
            pos[2] += (Judge[(record->unk_1D8 + 0x400) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            func_80032854(0, D_8008EB6C[kind], (u8 *)pos, (s16 *)0);
        }
    } else {
        D_800A3910--;
    }
}
