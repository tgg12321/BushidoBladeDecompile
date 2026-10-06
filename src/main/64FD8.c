/* Game functions func_800747D8 .. func_800788B0. .text 0x800747D8 (ROM 0x64FD8). Start boundary:
 * PHASE (site 5). Ends where the PsyQ 4.0 LIBAPI C67 module starts (0x80078948, LIBSCAN, Q106
 * D3); the library modules that followed are in src/main/psxsdk/libapi/ and libc2/. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

/* ---- merged from text1b_tu2.c (owner ruling Q67: one original file) ---- */
/* Declarations from the file this TU was split from (text1b.c). */
extern u32 *ClearOTagR(u32 *, s32);
extern s32 rsin();
extern void AddPrim(void *, void *);
/* func_800747D8: the duplicated `sound = 4;` below is the
 * duplicated-statement-into-arms shape and carries its FAKE annotation inline. */
/* 0x800A35D0: one {s16, s16} pair per player (two words,
 * asm/data/91C98.data.s:4279-4282), passed to func_800692C0 beside SelWork
 * f40[player]; func_800768DC indexes it by player * 4 (0x80076948/5C). */

/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s16 D_800A35D0[2][2];
static Unk8006E49CRec *D_800A35D8;
static s8 D_800A35DC;
static s32 D_800A35E0;
static s32 D_800A35E4;
static s32 D_800A35E8;
static s32 D_800A35EC;  /* not named by any code or data: size from the gap */
static s32 D_800A35F0;
static Unk8006E49CRec *D_800A35F4;
static Ctx77D94 *D_800A35F8;
static s32 D_800A35FC;
static s32 D_800A3600;
static s32 D_800A3604;  /* not named by any code or data: size from the gap */
static s32 D_800A3608;
static Unk8006E49CRec *D_800A360C;
static Unk80078824Rec *D_800A3610;
static s32 D_800A3614;

s32 func_800747D8(u32 input) {
    SelWork *base;
    s32 sp10;
    s32 ret;
    s32 state;
    s32 result;
    s32 i;
    u8 row;
    s32 sound;

    base = SELWORK;
    result = 0;
    if (base->f10.word == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, base->f40[0], D_800A35D0[0]);
        switch (ret >> 16) {
    case 1:
        SELWORK->f34 = 0;
        SELWORK->f3C[0] += 1;
        if (SELWORK->f3C[0] >= 5) {
            SELWORK->f3C[0] = 0;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    case 2:
        SELWORK->f34 = 0;
        SELWORK->f3C[0] -= 1;
        if (SELWORK->f3C[0] < 0) {
            SELWORK->f3C[0] = 4;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
        }

        state = SELWORK->f3C[0];
        switch (state) {
    case 0:
        switch (ret & 0xFF) {
        case 1:
            if (SELWORK->f65 == SELWORK->f64) {
                SELWORK->f65 = 0;
            } else {
                SELWORK->f65 += 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:` -- reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold. */
            sound = 4;
            goto selection_sound;
        case 2:
            if (SELWORK->f65 == 0) {
                SELWORK->f65 = SELWORK->f64;
            } else {
                SELWORK->f65 -= 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:` -- reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold. */
            sound = 4;
            goto selection_sound;
        }
        goto confirm;
selection_sound:
        if (SELWORK->f64 != 0) {
            sound = 0;
        }
        func_8005C650(sound, 0x7F, 0x7F);
        goto confirm;
    case 1:
        if ((ret & 0xFF) != 0) {
            SELWORK->f67 += 1;
            SELWORK->f67 &= 1;
            SELWORK->f66 = D_8009BD20[SELWORK->f67][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
    case 2:
        if ((ret & 0xFF) != 0) {
            row = SELWORK->f67;
            D_800A35DC += 1;
            SELWORK->f66 = D_8009BD20[row][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
confirm:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK->f3C[0] = 3;
        }
        break;
    case 3:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            for (i = 0; i < 2; i++) {
                SELWORK->f10.half[i] = 3;
                SELWORK->f18[i] = 1;
                SELWORK->f38[i] = 0;
            }
        }
        break;
    case 4:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            result = -1;
        }
        break;
        }
        if (input & 0x100010) {
            func_8005C650(2, 0x7F, 0x7F);
            result = -1;
        }
    }
    return result;
}




void func_80074B18(Unk8006EACCRec *arg0, s32 arg1, s32 arg2) {
    TILE *p;
    Rec_8006C21C *t;
    s16 i;
    s16 j;
    s16 n;
    s32 ot;

    n = 5;
    if (arg2 != 0) {
        n = 8;
    }
    p = arg0->unk_04.unk_10;
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        t = SELWORK->f04->unk_3C;
        for (j = 0; j < n; j++) {
            SetTile(p);
            p->r0 = t->r;
            p->g0 = t->g;
            p->b0 = t->b;
            p->w = t->w;
            p->h = t->h;
            SetSemiTrans(p, 0);
            if (arg2 != 0) {
                p->x0 = t->x + arg1 * 240;
                p->y0 = t->y + i * 34 + 0x2B;
            } else {
                p->x0 = t->x + arg1 * 240;
                p->y0 = t->y + i * 17 + 0x7C;
            }
            ot = 0xB;
            if (arg1 != 0) {
                ot = 0x15;
            }
            AddPrim(g_gpu_ot_ptr + ot * 4, p);
            p++;
            t++;
        }
    }
    arg0->unk_04.unk_10 = p;
}

void func_80074D2C(Unk8006EACCRec *arg0, s32 arg1, s32 arg2) {
    Unk8007352CEnv s;
    s32 ot;

    ot = 0xC;
    s.semi = 0;
    s.has_color = 0;
    s.header = arg0->unk_00.v80076FF8->unk_1C[(s16)arg2];
    s.x = arg1 * 0xF0;
    s.y = 0;
    s.table = s.header->cells;
    if (arg1 != 0) {
        ot = 0x16;
    }
    s.ot_idx = ot;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + ot * 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
}
void func_80074E08(Unk8006EACCRec *arg0, s32 arg1) {
    Unk8007352CEnv s;
    RECT rect;
    u16 offset[2];
    Unk8009B0E0Record **records;
    TILE *prim;
    s32 ot_idx;
    /* work holds two values (owner Ruling 11; the first under its Q20
       per-branch-constant clause): the backdrop TILE's OT index (0xE for
       player 1, 4 for player 0), then ot_idx * 4, the OT byte offset the last
       three AddPrim calls add. FAKE: one local for both; a second local for the byte
       offset: score 22. */
    s32 work;
    s32 rect_x;
    /* the sprite sheet's cell array (8-byte Unk8009B400Record cells), which starts
       just past the sheet's one 12-byte Unk8009B0E0Record header (+0xC) on each of
       the four root+0x18 sheets this function draws: one meaning at one
       constant offset, written once per sheet (owner Ruling 9). FAKE: the holder between the header's cells and s.table; stored directly: score 12. */
    Unk8009B400Record *cells;
    s16 i;

    prim = arg0->unk_04.unk_10;
    SetTile(prim);
    SetSemiTrans(prim, 0);
    prim->r0 = 0xD0;
    prim->g0 = 0xC8;
    prim->b0 = 0xB8;
    prim->x0 = arg1 * 0xF0 + 0x62;
    prim->y0 = 0x14;
    prim->w = 0xCC;
    prim->h = 0xC8;
    if (arg1 != 0) {
        work = 0xE;
    } else {
        work = 4;
    }
    AddPrim(g_gpu_ot_ptr + work * 4 + 0x24, prim);
    prim++;
    arg0->unk_04.unk_10 = prim;

    records = arg0->unk_00.v80076FF8->unk_18;
    s.semi = 0;
    s.has_color = 0;
    s.x = arg1 * 0xF0;
    s.ot_idx = 2;
    i = 0;
    do {
        s.y = (0xD2 - SELWORK->f0C[arg1]) * i;
        s.header = records[3];
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        s.header = records[2];
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        s.table += s.header->count;
        s.scale_x = 0xCE00;
        s.scale_y = 0x100;
        s.ft4_out = arg0->unk_04.unk_00;
        arg0->unk_04.unk_00 = func_80073728(&s, 0);
        i++;
    } while (i < 2);

    s.x = arg1 * 0xF0;
    s.y = 0;
    s.header = records[0];
    cells = s.header->cells;
    s.table = cells;
    if (arg1 != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    s.header = records[1];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    if (arg1 != 0) {
        ot_idx = 0xE;
        rect_x = 0x14E;
    } else {
        ot_idx = 4;
        rect_x = 0x5E;
    }
    rect.x = SELWORK->f24->draw.clip.x + rect_x;
    rect.y = SELWORK->f24->draw.clip.y + 0x14;
    rect.w = 0xD4;
    rect.h = 0xC8 - SELWORK->f0C[arg1];
    SetDrawArea(arg0->unk_04.unk_18, &rect);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0->unk_04.unk_18);
    arg0->unk_04.unk_18++;

    rect.x = SELWORK->f24->draw.clip.x;
    rect.y = SELWORK->f24->draw.clip.y;
    rect.w = SELWORK->f24->draw.clip.w;
    rect.h = SELWORK->f24->draw.clip.h;
    SetDrawArea(arg0->unk_04.unk_18, &rect);
    work = ot_idx * 4;
    AddPrim(g_gpu_ot_ptr + work, arg0->unk_04.unk_18);
    arg0->unk_04.unk_18++;

    offset[0] = SELWORK->f24->draw.ofs[0];
    offset[1] = SELWORK->f24->draw.ofs[1]
              - SELWORK->f08[arg1];
    SetDrawOffset(arg0->unk_04.unk_1C, offset);
    AddPrim(g_gpu_ot_ptr + work + 0x24, arg0->unk_04.unk_1C);
    arg0->unk_04.unk_1C++;

    offset[0] = SELWORK->f24->draw.ofs[0];
    offset[1] = SELWORK->f24->draw.ofs[1];
    SetDrawOffset(arg0->unk_04.unk_1C, offset);
    AddPrim(g_gpu_ot_ptr + work, arg0->unk_04.unk_1C);
    arg0->unk_04.unk_1C++;
}
void func_8007526C(void) {
    SelWork *base;
    s32 i;

    base = SELWORK;
    i = 0;
    do {
        switch ((u8)base->f10.half[i]) {
        case 1:
            base->f08[i] += 0xA;
            base->f0C[i] += 0xA;
            if (base->f0C[i] >= 0xC8) {
                if (((u16)base->f10.half[i] >> 8) == 0) {
                    base->f10.half[i] += 1;
                }
                base->f08[i] = 0;
                base->f0C[i] = 0xC8;
                base->f14.half[i] = base->f18[i];
                base->f3C[i] = base->f38[i];
            }
            break;
        case 3:
            base->f0C[i] += 0xA;
            if (base->f0C[i] >= 0xC8) {
                base->f08[i] = 0xC8;
                base->f0C[i] = 0xC8;
                base->f14.half[i] = base->f18[i];
                base->f3C[i] = base->f38[i];
                if (((u16)base->f10.half[i] >> 8) == 0) {
                    base->f10.half[i] += 1;
                }
            }
            break;
        case 2:
            base->f0C[i] -= 0xA;
            if (base->f0C[i] <= 0) {
                base->f08[i] = 0;
                base->f0C[i] = 0;
                base->f10.half[i] = 0;
            }
            break;
        case 4:
            base->f08[i] -= 0xA;
            base->f0C[i] -= 0xA;
            if (base->f0C[i] <= 0) {
                base->f08[i] = 0;
                base->f0C[i] = 0;
                base->f10.half[i] = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}



extern s32 rsin(s32);

/* Per-player gauge draw: fills the 0x2C-byte sprite descriptor `s` (the same
   descriptor shape func_8006DD94 / func_80069F80 hand to func_8007352C) for the
   player's gauge frame, two fixed sub-elements from the 0x14 table, and two
   layered elements from the 0x2C table, emitting a SetDrawMode prim after each
   group. x position is arg1 * 240 (player 1 sits one screen-half right); the
   frame's y is the per-player 0x68 counter * 90 plus the per-player s16 at
   0x42; the frame colour breathes with rsin of the 5-bit timer at 0x34. */
void func_800753D8(Unk8006EACCRec *arg0, s32 arg1) {
    Unk8007352CEnv s;
    Unk8009B0E0Record **tbl;
    s16 i;
    /* FAKE: the holder between the header's cells and s.table; stored directly: score 15. */
    Unk8009B400Record *body;
    /* FAKE: constant-holder — the 0 passed to both func_8006E480 calls is
       kept live in callee-save $s5 (target: `addu $s5,$zero,$zero` in the
       prologue, `addu $a1,$s5,$zero` at both call sites) instead of being
       re-materialized as `li $a1,0`; mechanism: global.c allocates the
       once-set constant pseudo a callee-save because it crosses calls and
       cse.c only folds the constant within the entry extended basic block.
       SOTN ships this exact shape: src/dra/7879C.c:2067 `s32 zero = 0;`. The literal
       at both calls: score 9. */
    s32 zero;

    zero = 0;
    s.semi = 0;
    if (arg1 != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    tbl = arg0->unk_00.v80076FF8->unk_2C;
    s.header = tbl[arg1 + 2];
    s.x = arg1 * 240;
    body = s.header->cells;
    s.table = body;
    s.y = SELWORK->f68[arg1] * 90 + SELWORK->f40[arg1][1];
    s.col_r = s.col_g = s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
    s.has_color = 1;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    s.has_color = 0;
    tbl = arg0->unk_00.v80076FF8->unk_14;
    i = 0;
    s.header = tbl[0];
    s.x = arg1 * 240 + 0x9D;
    s.y = 0x36;
    body = s.header->cells;
    s.table = body;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    s.y = 0x90;
    s.table += s.header->count;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    tbl = arg0->unk_00.v80076FF8->unk_2C;
    do {
        s.header = tbl[i];
        s.x = arg1 * 240;
        s.y = 0;
        body = s.header->cells;
        if (arg1 != 0) {
            s.ot_idx = 0x16;
        } else {
            s.ot_idx = 0xC;
        }
        s.table = body;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, zero), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
        i++;
        arg0->unk_04.unk_14++;
    } while (i < 2);
}
void func_80075670(s32 arg0, s32 arg1) {
    s16 i;
    SelWork *base;
    SelWork *work;

    base = SELWORK;
    if (base->f10.word != 0) {
        return;
    }
    if ((func_800692C0(&arg0, arg1, base->f40[arg1], D_800A35D0[arg1]) >> 16) != 0) {
        SELWORK->f34 = 0;
        SELWORK->f68[arg1] = SELWORK->f68[arg1] + 1;
        SELWORK->f68[arg1] &= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    work = SELWORK;
    work->f3C[arg1] = work->f68[arg1];
    if (arg0 & (0x40 << (arg1 * 16))) {
        for (i = 0; i < 2; i++) {
            work->f38[i] = 0;
            work->f10.half[i] = 1;
            work->f18[i] = 2;
        }
        SELWORK->f68[(arg1 + 1) & 1] = (SELWORK->f68[arg1] + 1) & 1;
        func_8005C650(1, 0x7F, 0x7F);
        return;
    }
    if (arg0 & (0x10 << (arg1 * 16))) {
        if (work->f14.half[(arg1 != 0) ? 0 : 1] == 1) {
            for (i = 0; i < 2; i++) {
                work->f38[i] = 0;
                work->f10.half[i] = 3;
                work->f18[i] = 0;
            }
            func_8005C650(2, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
void func_80075830(Unk8006EACCRec *arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8007352CEnv s;
    s16 var_a1;
    s16 var_a2;
    s.semi = arg3;
    s.col_r = s.col_g = s.col_b = ((s32) (rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;
    s.header = arg0->unk_00.v80076FF8->unk_14[21];
    s.table = s.header->cells;
    if (arg1 < 0xA) {
        var_a1 = arg1 / 5;
        var_a2 = arg1 % 5;
    } else {
        var_a1 = (arg1 - 0xA) / 5;
        var_a2 = (arg1 - 0xA) % 5;
    }
    if (SELWORK->f1C.half[arg2] == var_a1 && SELWORK->f20.half[arg2] == var_a2) {
        s.has_color = 1;
    } else {
        s.has_color = 0;
    }
    s.x = arg2 * 0xF0 + var_a1 * 0x64;
    s.y = var_a2 * 16;
    if (arg2 != 0) {
        s.ot_idx = 0x13;
    } else {
        s.ot_idx = 9;
    }
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
}
/* 0x8009BCE4: 20 flag bytes indexed by entry id (asm/data/7D920.data.s:23732-23753;
 * func_800768DC lbu/sb at 0x80076B90/0x80076BA4). */
extern u8 D_8009BCE4[20];

/* Character-select grid renderer (select-screen case 2 of func_80077374; the
 * draw half of func_80075F80), called once per player per frame.
 *   arg0 = draw context (arg0->unk_00 root table, arg0->unk_04.unk_0C sprite chain,
 *          arg0->unk_04.unk_14 DR_MODE cursor)
 *   arg1 = select page into D_8009BCF8 (10 cells per page)
 *   arg2 = this player's pick list (-1 = cleared slot)
 *   arg3 = player index (0/1)
 * Draws the page frame, the page's 10 character cells (selectable cells as
 * sprites with the cursor cell highlighted, taken cells via func_80075830,
 * unselectable cells via func_80075830), the picks made so far, then the
 * f65+3 slot sprites, and closes with two DR_MODE/AddPrim pairs. */
void func_800759D0(Unk8006EACCRec *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    Unk8007352CEnv s;
    /* table holds three values, each a sprite-sheet pointer table read from the
     * root at arg0->unk_00: the page table at +0x14 (the head; read by the head and
     * loops 1-2), the slot table at +0x20 + f65 * 4 (reloaded every loop-3
     * iteration), and the page table at +0x14 again for the closing DR_MODE
     * pair.  One local, not three (owner Ruling 11). */
    Unk8009B0E0Record **table;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
       func_8006E480's second argument at all three call sites is kept in
       callee-save $fp (target: `addu $fp,$zero,$zero` in the entry block,
       `addu $a1,$fp,$zero` at each call) instead of being re-materialized;
       global.c gives the once-set constant pseudo a callee-save because it
       crosses every call, and cse only folds a constant within the entry
       extended basic block. Same shape as the siblings
       func_800753D8 (`zero`) and func_8007636C (`mode`). The literal at the three
       calls: score 30. */
    s32 zero;
    s16 i;
    /* the sheet's cell array. The code treats each sheet it draws here as
       12-byte Unk8009B0E0Record headers followed by 8-byte Unk8009B400Record cells: the head
       treats the table[0] page sheet as one header (cells at +0xC), and
       loops 1-3 treat their sheets as three headers, normal then one cursor
       highlight per player at +12/+24 (cells at +0x24). Loop 2 applies that
       three-header view to every pick slot, including the 0x14 placeholder
       func_80075F80 stores for an unavailable cell, whose sheet the code
       elsewhere draws as one header (func_80075830); there the +0x24 runs
       past the sheet into bytes nothing references (owner Ruling 9).
       FAKE: the holder between the header's cells and s.table; stored directly: score 25. */
    Unk8009B400Record *cells;

    zero = 0;
    s.semi = 0;
    s.has_color = 0;
    table = arg0->unk_00.v80076FF8->unk_14;
    s.header = table[0];
    s.x = arg3 * 240 + 0x88;
    s.y = 0x33;
    cells = s.header->cells;
    s.table = cells;
    if (arg3 != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    if (arg1 != 0) {
        s.table += s.header->count;
    }
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    s.col_r = s.col_g = s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    if (arg3 != 0) {
        s.ot_idx = 0x14;
    } else {
        s.ot_idx = 0xA;
    }
    s.x = arg3 * 240;
    for (i = arg1 * 10; i < arg1 * 10 + 10; i++) {
        /* i indexes the whole table flat (arg1 * 10 + cell), from the first record. */
        /* SOTN: src/dra/62DEC.c:1091 @aa53500 */
        /* FAKE: pointer form of the flat read (Q53).  `&D_8009BCF8[0][0] + i`
         * forms the address as its own value, so cse reuses it for the second
         * read and loop.c hoists the table base into $s6 (the target's
         * `lui/addiu $s6` and 2-byte step); D_8009BCF8[0][i].unk0 at the three
         * sites folds the symbol into each load (`lui $at; addu; lbu %lo`): score 44. */
        u8 entry = (&D_8009BCF8[0][0] + i)->unk0;

        if (D_8009BCE4[entry] & 1) {
            s.header = table[entry + 1];
            cells = s.header[2].cells;
            s.table = cells;
            if (D_8009BCF8[arg1][SELWORK->f1C.half[arg3] * 5 + SELWORK->f20.half[arg3]].unk0 == (&D_8009BCF8[0][0] + i)->unk0) {
                s.has_color = 1;
                s.header += 1 + arg3;
            } else {
                s.has_color = 0;
            }
            s.y = 0;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
            if (D_8009BCE4[(&D_8009BCF8[0][0] + i)->unk0] & (4 << arg3)) {
                func_80075830(arg0, i, arg3, 1);
            }
        } else {
            func_80075830(arg0, i, arg3, 0);
        }
    }

    for (i = 0; i < SELWORK->f3C[arg3] + 1; i++) {
        if (arg2[i] >= 0) {
            s.header = table[arg2[i] + 1];
            cells = s.header[2].cells;
            if (i != SELWORK->f3C[arg3]) {
                s.has_color = 0;
            } else {
                s.has_color = 1;
                s.header += 1 + arg3;
            }
            s.y = i * 17;
            s.table = cells;
            s.table += s.header->count;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
        }
    }

    s.has_color = 0;
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        table = arg0->unk_00.v80076FF8->unk_20[SELWORK->f65];
        s.header = table[i];
        cells = s.header[2].cells;
        if (i == SELWORK->f3C[arg3]) {
            s.header += 1 + arg3;
        }
        s.table = cells;
        s.x = arg3 * 240;
        s.y = i * 17;
        if (arg3 != 0) {
            s.ot_idx = 0x14;
        } else {
            s.ot_idx = 0xA;
        }
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
    }

    table = arg0->unk_00.v80076FF8->unk_14;
    s.header = table[1];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4 - 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
}

/* Character-select cursor / pick handler; called once per player per frame.
 *   arg0 = this frame's pad bits, both players packed (player N in bits N*16)
 *   arg1 = select page into D_8009BCF8 (2 rows x 5 columns per page)
 *   arg2 = this player's pick list (character ids, -1 = cleared slot)
 *   arg3 = player index (0/1)
 * SELWORK is the shared select work area (per-player pairs); D_8009BCE4[] is
 * the per-character flag byte (bit 0 = selectable, bit 4<<player = already
 * taken by that player). */
void func_80075F80(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    u8 *flag;
    s32 result;
    s32 bit;
    s32 i;

    if (SELWORK->f10.half[arg3] != 0) {
        return;
    }

    if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK->f3C[arg3] != 0) {
            arg2[SELWORK->f3C[arg3]] = -1;
            SELWORK->f3C[arg3]--;
            D_8009BCE4[arg2[SELWORK->f3C[arg3]]] &= ~(4 << arg3);
            return;
        }
        /* Neither player has a pick yet.  fold merges the two adjacent halfword
         * tests into the one `lw 0x3C` word test the target has at 0x80076060
         * (tools/gcc-2.7.2/fold-const.c:2687 fold_truthop). */
        if (SELWORK->f3C[0] != 0 || SELWORK->f3C[1] != 0) {
            return;
        }
        if (SELWORK->f14.half[(arg3 != 0) ? 0 : 1] == 2) {
            SELWORK->f10.half[1] = 3;
            SELWORK->f10.half[0] = 3;
            SELWORK->f18[1] = 1;
            SELWORK->f18[0] = 1;
        }
        return;
    }

    result = func_800692C0(&arg0, arg3, SELWORK->f40[arg3], D_800A35D0[arg3]);
    if (arg0 & (0xF000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    {
        s32 low;

        low = result & 0xFF;
        if (low < 3 && low != 0) {
            SELWORK->f1C.half[arg3] = (SELWORK->f1C.half[arg3] + 1) & 1;
        }
    }

    switch (result >> 16) {
    case 1: {
        s16 value;

        value = SELWORK->f20.half[arg3];
        if (value == 4) {
            SELWORK->f20.half[arg3] = 0;
        } else {
            SELWORK->f20.half[arg3] = value + 1;
        }
    } break;
    case 2: {
        s16 value;

        value = SELWORK->f20.half[arg3];
        if (value == 0) {
            SELWORK->f20.half[arg3] = 4;
        } else {
            SELWORK->f20.half[arg3] = value - 1;
        }
    } break;
    }

    {
        u8 entry;

        entry = D_8009BCF8[arg1][SELWORK->f1C.half[arg3] * 5 + SELWORK->f20.half[arg3]].unk0;
        flag = &D_8009BCE4[entry];
        if ((*flag & 1) != 0) {
            bit = 4 << arg3;
            if ((*flag & bit) == 0) {
                u16 count;

                arg2[SELWORK->f3C[arg3]] = entry;
                if (arg0 & (0x40 << (arg3 * 16))) {
                    func_8005C650(1, 0x7F, 0x7F);
                    *flag |= bit;
                    count = SELWORK->f3C[arg3];
                    SELWORK->f3C[arg3] = count + 1;
                    if (SELWORK->f3C[arg3] == SELWORK->f65 + 3) {
                        SELWORK->f10.half[arg3] = 1;
                        SELWORK->f3C[arg3] = count;
                        SELWORK->f38[arg3] = 0;
                        SELWORK->f18[arg3] = 3;
                        for (i = 0; i < SELWORK->f60[arg3]; i++) {
                            SELWORK->f48[arg3][i] = i;
                        }
                        if (arg1 != 0) {
                            SELWORK->f48[arg3][4] = 5;
                        }
                    }
                }
                return;
            }
            arg2[SELWORK->f3C[arg3]] = 0x14;
            if (arg0 & (0x40 << (arg3 * 16))) {
                func_8005C650(4, 0x7F, 0x7F);
            }
        } else {
            /* FAKE: the placeholder tail is written in both arms (duplicated-
             * statement-into-arms, calls per Q47: the store and the call are real on
             * both paths).  Each arm is then reached by one path, so cse carries the
             * work-area address it formed for the cursor read into the 0x3C load;
             * jump2's cross-jump (tools/gcc-2.7.2/jump.c find_cross_jump) merges the two
             * tails back into the target's one copy (`lh v0,0x3C($a1)` at 0x8007630C,
             * 0x14 set in each predecessor).  One shared tail after the if recomputes
             * the address. */
            arg2[SELWORK->f3C[arg3]] = 0x14;
            if (arg0 & (0x40 << (arg3 * 16))) {
                func_8005C650(4, 0x7F, 0x7F);
            }
        }
    }
}
void func_8007636C(Unk8006EACCRec *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    Unk8007352CEnv s;
    s32 ot;
    Unk8009B0E0Record **table;
    /* the sprite sheet's cell array (8-byte Unk8009B400Record cells), which starts
       just past the sheet's 12-byte Unk8009B0E0Record headers: one header on the
       single-state sheets (+0xC), three (normal, then one cursor highlight
       per player) on the highlightable ones (+0x24). FAKE: the holder between the header's cells and s.table; stored directly: score 6. */
    Unk8009B400Record *cells;
    s16 i;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
     * func_8006E480's second argument at both call sites. Set once and live
     * past the first loop, so cse substitutes its pseudo into that loop's
     * `(s16)i < f65 + 3` entry guard (slt needs a register operand) and reload
     * rematerializes it as the target's `move t0,zero; slt` (0x800764A0); a
     * literal 0 lets combine fold the guard to a beqz. The case-2
     * sibling func_800759D0 holds this same argument's zero in $fp (asm lines
     * 20/56/334/356). The literal: score 3. */
    s32 mode;
    u16 idx;

    mode = 0;
    ot = 10;
    s.semi = 0;
    if (arg3 != 0) {
        ot = 20;
    }
    table = arg0->unk_00.v80076FF8->unk_30;
    if (SELWORK->f14.half[arg3] < 4) {
        s.header = table[12];
        s.has_color = 0;
        s.x = arg3 * 240;
        cells = s.header->cells;
        s.table = cells;
        s.y = SELWORK->f3C[arg3] * 34;
        s.ot_idx = ot;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, mode), 0);
        AddPrim(g_gpu_ot_ptr + ot * 4, arg0->unk_04.unk_14);
        arg0->unk_04.unk_14++;
    }

    s.has_color = 0;
    s.col_r = s.col_g = s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    table = arg0->unk_00.v80076FF8->unk_14;
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        s.has_color = 0;
        s.header = table[arg2[i] + 1];
        cells = s.header[2].cells;
        if (SELWORK->f3C[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.header += 1 + arg3;
        }
        s.table = cells;
        s.table += s.header->count * 2;
        s.x = arg3 * 240;
        s.y = i * 34;
        s.ot_idx = ot;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
    }

    table = arg0->unk_00.v80076FF8->unk_30;
    for (i = 0; i < SELWORK->f3C[arg3] + 1; i++) {
        if (SELWORK->f3C[arg3] != i || SELWORK->f14.half[arg3] >= 4) {
            idx = SELWORK->f7E[arg3][i];
            s.has_color = 0;
        } else {
            idx = SELWORK->f48[arg3][SELWORK->f5C[arg3]];
            s.has_color = 1;
        }
        s.header = table[(s16)idx * 2];
        cells = s.header[2].cells;
        if (SELWORK->f3C[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.header += 1 + arg3;
        }
        s.x = arg3 * 240;
        s.table = cells;
        s.y = i * 34;
        s.ot_idx = ot;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        s.header = table[(s16)idx * 2 + 1];
        s.has_color = 0;
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
    }

    table = arg0->unk_00.v80076FF8->unk_20[SELWORK->f65];
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        s.has_color = 0;
        s.header = table[i];
        cells = s.header[2].cells;
        if (SELWORK->f3C[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.header += 1 + arg3;
        }
        s.table = cells;
        s.table += s.header->count;
        s.x = arg3 * 240;
        s.y = i * 34;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
    }

    table = arg0->unk_00.v80076FF8->unk_14;
    s.header = table[1];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + ot * 4, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
}

void func_800768DC(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 i;
    s16 j;

    if (SELWORK->f10.half[arg3] != 0) {
        return;
    }
    if (arg0 & (0xA000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    switch (func_800692C0((u32 *)&arg0, arg3, SELWORK->f40[arg3], D_800A35D0[arg3]) & 0xFF) {
    case 1:
        SELWORK->f5C[arg3]++;
        if (SELWORK->f5C[arg3] >= SELWORK->f60[arg3]) {
            SELWORK->f5C[arg3] = 0;
        }
        break;
    case 2:
        SELWORK->f5C[arg3]--;
        if (SELWORK->f5C[arg3] < 0) {
            SELWORK->f5C[arg3] = SELWORK->f60[arg3] - 1;
        }
        break;
    }

    if (arg0 & (0x40 << (arg3 * 16))) {
        if (SELWORK->f3C[arg3] < SELWORK->f65 + 3) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK->f7E[arg3][SELWORK->f3C[arg3]] = SELWORK->f48[arg3][SELWORK->f5C[arg3]];
            if (SELWORK->f3C[arg3] == SELWORK->f65 + 2) {
                SELWORK->f14.half[arg3] = 4;
                return;
            }
            SELWORK->f60[arg3]--;
            for (i = SELWORK->f5C[arg3]; i < SELWORK->f60[arg3]; i++) {
                SELWORK->f48[arg3][i] = SELWORK->f48[arg3][i + 1];
            }
            SELWORK->f5C[arg3] = 0;
            SELWORK->f3C[arg3]++;
        }
    } else if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK->f3C[arg3] == 0) {
            SELWORK->f10.half[arg3] = 3;
            SELWORK->f18[arg3] = 2;
            SELWORK->f38[arg3] = SELWORK->f65 + 2;
            D_8009BCE4[arg2[SELWORK->f38[arg3]]] &= ~(4 << arg3);
            SELWORK->f60[arg3] = 5;
            for (i = 0; i < SELWORK->f60[arg3]; i++) {
                SELWORK->f48[arg3][i] = i;
            }
            if (arg1 != 0) {
                SELWORK->f48[arg3][4] = 5;
            }
            return;
        }
        /* Put the last taken entry back into the list in sorted position. */
        for (i = 0; i < SELWORK->f60[arg3]; i++) {
            if (SELWORK->f48[arg3][i] > SELWORK->f7E[arg3][SELWORK->f3C[arg3] - 1]) {
                for (j = SELWORK->f60[arg3] - 1; j >= i; j--) {
                    SELWORK->f48[arg3][j + 1] = SELWORK->f48[arg3][j];
                }
                SELWORK->f48[arg3][i] = SELWORK->f7E[arg3][SELWORK->f3C[arg3] - 1];
                break;
            }
            if (i == SELWORK->f60[arg3] - 1) {
                SELWORK->f48[arg3][i + 1] = SELWORK->f7E[arg3][SELWORK->f3C[arg3] - 1];
            }
        }
        SELWORK->f60[arg3]++;
        SELWORK->f3C[arg3]--;
    }
}
/* func_80076D74: select-screen fade-out.  Raises SELWORK->f36 by 8 per frame
 * (capped at 0xFF) and, once it reaches 0xFF, fills the result record at
 * SELWORK->f00 (f65, f66, f67, f68 into its unk14 bit fields; per player and pick,
 * the picked entry's D_8009BCF8 column 1 and its f7E value).  Every frame it
 * draws the full-screen TILE with f36 as its colour. */
s32 func_80076D74(Unk8006EACCRec *arg0) {
    TILE *p;
    Unk8009BD24Block *hdr;
    u16 *cnt;
    s16 v;
    s16 i;
    s16 j;
    s32 sel;
    s32 ret;

    ret = 0;
    cnt = &SELWORK->f36;
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        hdr = SELWORK->f00;
        hdr->unk14_10 = SELWORK->f65;
        ret = 1;
        if (SELWORK->f66 < 3) {
            sel = SELWORK->f66 - 1;
        } else {
            sel = SELWORK->f66 - 2;
        }
        hdr->unk14_12 = sel;
        hdr->unk14_14 = SELWORK->f67;
        hdr->unk14_15 = SELWORK->f68[0] + SELWORK->f68[1] * 2;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < SELWORK->f65 + 3; j++) {
                /* flat character index into both pages, through row 0. */
                /* SOTN: src/st/e_grave_keeper.h:534 @aa53500 */
                hdr->unk00[i][j].chr = D_8009BCF8[0][SELWORK->f6A[i][j]].unk1;
                hdr->unk00[i][j].unk1 = SELWORK->f7E[i][j];
            }
        }
    }
    p = arg0->unk_04.unk_10;
    SetTile(p);
    p->r0 = *cnt;
    p->g0 = *cnt;
    p->b0 = *cnt;
    p->x0 = 0;
    p->y0 = 0;
    p->w = 0x280;
    p->h = 0xF0;
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr, p);
    p++;
    arg0->unk_04.unk_10 = p;
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    return ret;
}
s32 func_80076FF8(s32 *a0) {
    func_8006920C(a0, a0[5]);
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    func_8006920C(a0, a0[11]);
    func_8006920C(a0, a0[12]);
    func_8006920C(a0, a0[13]);
    func_8006920C(a0, a0[14]);
    return a0[1];
}
Unk8006E49CRec *func_80077098(s32 a0) {
    return &D_800A35D8[a0];
}


extern s32 func_80076FF8(s32 *);

s32 func_800770B8(s32 arg0, Unk8009BD24Block *arg1, s32 arg2) {
    s16 sp[2];
    /* work holds two values: the entry list pointer (arg0 + 0x58, passed to func_8006E950 /
       func_80076FF8 and stored as the work area's f04), then the work area func_8006E49C returns
       (stored to D_800A36A0). Owner Ruling 11 with owner ruling Q78. */
    void *work;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap (do-while-zero-exception). Effect: its NOTE_INSN_LOOP_BEG/END
       pair bounds sched2's region at this point, so the five frame-save stores are not interleaved
       with the first body insns as in the target's prologue order. */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR((u32 *)g_gpu_ot_ptr, 0x1008);
    work = (void *)(arg0 + 0x58);
    D_800A35D8 = (Unk8006E49CRec *)arg0;
    snd_StopAll();
    func_8006E950(6, work);
    r = func_80076FF8(work);
    {
        Unk80076FF8Rec *list = work;
        work = (void *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = work;
        SELWORK->f04 = list;
        /* FAKE: dead store (dead-store-fake-exception; owner ruling Q78): work's
           restored value is never read. Effect: cse.c make_regs_eqv puts work and the call's $v0 in
           one quantity with work canonical, so the D_800A36A0 reloads for the 0x30/0x34 clears below
           would become work ($s1); this store takes work out of that class first (cse.c
           delete_reg_equiv), the reloads resolve to the f04 store's reload copy of the call result
           instead, and the clears use $v0 as in the target (0x80077144/48). */
        work = list;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
    }
    t0 = 0;
    do {
        SelWork *base = SELWORK;
        /* FAKE: typed row pointer to D_800A35D0 (pointer-alias-fake-exception, the
           `s16 (*p)[] = &D_xxx` re-view), kept so the same-value re-set below has a pseudo to re-set
           (loop.c keeps its lui/addiu inside the outer loop). */
        s16 (*row)[2];
        a2 = 0;
        base->f10.half[t0] = 0;
        base->f08[t0] = 0;
        base->f0C[t0] = 0;
        base->f14.half[t0] = 0;
        base->f3C[t0] = 0;
        row = D_800A35D0;
        row[t0][1] = 0;
        row[t0][0] = 0;
        base->f40[t0][1] = 0;
        base->f40[t0][0] = 0;
        base->f68[t0] = t0;
        do {
            SELWORK->f6A[t0][a2] = -1;
            SELWORK->f7E[t0][a2] = 0;
            a2 = (s16)(a2 + 1);
        } while (a2 < 5);
        SELWORK->f5C[t0] = 0;
        SELWORK->f60[t0] = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] | 1);
                sp[t0] += 1;
                /* FAKE: same-value dead store (dead-store-fake-exception): row is not read after
                   this. Effect: a second set of row's pseudo in another basic block makes loop.c
                   count_loop_regs_set mark it may_not_move (loop.c:3040-3041), so the lui/addiu of
                   D_800A35D0 stays inside the outer loop where the target builds it instead of
                   being hoisted to the pre-header. */
                row = D_800A35D0;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        SelWork *p = SELWORK;
        p->f20.word = 0;
        p->f1C.word = 0;
        if (sp[0] < sp[1]) {
            p->f64 = sp[0] - 3;
        } else {
            p->f64 = sp[1] - 3;
        }
    }
    if (SELWORK->f64 >= 3) {
        SELWORK->f64 = 2;
    }
    {
        SelWork *q = SELWORK;
        q->f00 = arg1;
        q->f65 = 0;
    }
    SELWORK->f67 = 1;
    SELWORK->f66 = D_8009BD20[SELWORK->f67][1];
    D_800A35DC = 1;
    return 1;
}

/* Tail word after func_800747D8's five-entry compiler-generated switch table. */
const u32 D_80015A20[1] = { 0x00000000 };

s32 func_80077374(s32 arg0, Unk8006EACCRec *arg1) {
    s32 ret;
    s16 i;

    ret = 0;
    if (SELWORK->f14.word == 0x40004) {
        SELWORK->f14.half[1] = 5;
        SELWORK->f14.half[0] = 5;
        SELWORK->f36 = 0;
    }
    func_80074220(arg1, SELWORK->f14.half[0]);
    if (SELWORK->f14.half[0] != 5) {
        func_8007526C();
    }

    for (i = 0; i < 2; i++) {
        switch (SELWORK->f14.half[i]) {
        case 0:
            if (i == 0) {
                ret = func_800747D8(arg0);
                func_80074488(arg1);
            }
            break;
        case 1:
            func_80075670(arg0, i);
            func_80074D2C(arg1, i,
                          (s16)(SELWORK->f14.half[i] - 1));
            func_800753D8(arg1, i);
            func_80074E08(arg1, i);
            break;
        case 2:
            func_80075F80(arg0, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074D2C(arg1, i,
                          (s16)(SELWORK->f14.half[i] - 1));
            func_80074B18(arg1, i, 0);
            func_800759D0(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074E08(arg1, i);
            break;
        case 3:
            func_800768DC(arg0, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074D2C(arg1, i,
                          (s16)(SELWORK->f14.half[i] - 1));
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074E08(arg1, i);
            break;
        case 4:
            func_80074D2C(arg1, i, 2);
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074E08(arg1, i);
            if (arg0 & (0x10 << (i * 16))) {
                func_8005C650(2, 0x7F, 0x7F);
                SELWORK->f14.half[i] = 3;
            }
            break;
        case 5:
            func_80074D2C(arg1, i, 2);
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            ret = func_80076D74(arg1);
            func_80074E08(arg1, i);
            break;
        }
    }
    return ret;
}

s32 func_80077724(s32 arg0, s32 arg1) {
    Unk8006EACCRec s;
    Unk8006E49CRec *p;
    s32 temp_v1;
    SELWORK->f24 = &g_gpu_db[D_800A36AC & 1];
    temp_v1 = SELWORK->f30 + 1;
    SELWORK->f34 = SELWORK->f34 + 1;
    SELWORK->f30 = temp_v1;
    p = func_80077098(temp_v1 & 1);
    SELWORK->f2C = p;
    s.unk_00.v80076FF8 = SELWORK->f04;
    s.unk_04.unk_00 = p->unk_00;
    s.unk_04.unk_04 = p->unk_04;
    s.unk_04.unk_08 = p->unk_08;
    s.unk_04.unk_0C = p->unk_10;
    s.unk_04.unk_10 = p->unk_0C;
    s.unk_04.unk_14 = p->unk_14;
    s.unk_04.unk_18 = p->unk_18;
    s.unk_04.unk_1C = p->unk_1C;
    s.unk_24 = p->unk_20;
    return func_80077374(arg1, &s);
}


s32 func_80077820(s32 a0) {
    func_80068F70(a0, &D_8009BD24);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A35E4 = 0;
    return 1;
}


s32 func_80077860(void) {
    if (((s32 (*)())func_80069250)() == 1) {
        D_800A35E4 = 0;
        return 1;
    }
    return 0;
}
s32 func_80077894(s32 held, s32 pressed) {
    s32 ret;
    s32 result;

    ret = 0;
    result = func_800693CC(held, pressed);
    if (result >= 0) {
        ret = 1;
        D_800A35E4 = 0;
        D_8009BD24.unk14_0 = result;
    } else if (result == -2) {
        ret = -1;
    }
    return ret;
}
s32 func_80077904(void) {
    D_800A35E4 = 0;
    D_800A35E0 = D_8009BD58[D_8009BD24.unk14_0][1];
    return D_8009BD58[D_8009BD24.unk14_0][0];
}
void func_80077940(s32 arg0) {
    D_800A35E8 = (arg0 & 0x3FF) + ((u32) (arg0 & 0x3FF000) >> 2) + ((u32) (arg0 & 0x01000000) >> 4) + ((u32) (arg0 & 0x04000000) >> 5);
}

s32 func_80077984(s32 a0) {
    func_8006E534(a0, D_800A35E0, &D_8009BD24, D_800A35E8);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

s32 func_800779C8(s32 arg0, s32 arg1) {
    s32 ret = func_8006EACC(arg0, arg1);
    if (ret) {
        snd_CloseVab1();
    }
    return ret;
}

s32 func_80077A04(s32 a0, s32 a1) {
    D_800A35E4 = 0;
    return func_8006D74C(a0, a1);
}
void func_80077A28(void) {
    D_800A35E4 = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    func_8006D7FC();
}

s32 func_80077A60(s32 arg0, s32 arg1) {
    return func_8006E068(arg0, arg1);
}

s32 func_800770B8(s32, Unk8009BD24Block *, s32);
s32 func_80077A80(s32 a0) {
    func_800770B8(a0, &D_8009BD24, D_800A35E8);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}


s32 func_80077AC0(s32 arg0, s32 arg1) {
    return func_80077724(arg0, arg1);
}

void func_80077AE0(void) {
    func_8006E10C();
}

void func_80077B00(void) {
    func_8006E2A8();
}
void func_80077B20(void) {
    D_800A35E4 = 1;
}

/* ---- merged from text1b_b.c (owner ruling Q67: one original file) ---- */
#define NULL ((void *)0)

typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;










































































































































extern s32 column;











































































    extern s32 rand(void);







































































































































































/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */




s32 func_80077B30(s32 arg0, s32 arg1) {
    s32 s2;
    s32 result;

    if ((u32)D_800A35E4 >= 6) goto end;
    switch (D_800A35E4) {
    case 0:
        s2 = 0;
        result = func_8006B898(arg0, arg1);
        switch (result) {
        case 1: s2 = -1; goto end;
        case 2: D_800A35E4 = 1; goto end;
        case 3:
            D_8009BD24.unk17[0] = D_8009BD24.unk1D[0];
            D_8009BD24.unk17[1] = D_8009BD24.unk1D[1];
            D_8009BD24.unk17[2] = D_8009BD24.unk1D[2];
            D_800A35E4 = 4;
            goto end;
        }
        goto end;
    case 1:
        s2 = 0;
        result = func_8006C1FC(arg0, arg1);
        if (result == 1) { D_800A35E4 = 0; goto end; }
        if (result == 2) { D_800A35E4 = result; goto end; }
        if (result == 3) { D_800A35E4 = result; }
        goto end;
    case 2:
        s2 = 2;
        goto end;
    case 3:
        s2 = 3;
        goto end;
    case 4:
        func_8006D324();
        D_800A35E4 = D_800A35E4 + 1;
        /* fall through */
    case 5:
        s2 = 0;
        result = func_8006D338(arg0, arg1);
        if (result == 1) {
            D_800A35E4 = 0;
            D_8009BD24.unk1D[0] = D_8009BD24.unk17[0];
            D_8009BD24.unk1D[1] = D_8009BD24.unk17[1];
            D_8009BD24.unk1D[2] = D_8009BD24.unk17[2];
            goto end;
        }
        if (result == -1) { D_800A35E4 = 0; }
        goto end;
    }
end:
    return s2;
}
Unk8009BD24Block *func_80077D00(void) {
    return &D_8009BD24;
}
s32 func_80077D10(s32 *a0) {
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    return a0[1];
}
Unk8006E49CRec *func_80077D74(s32 a0) {
    return &D_800A35F4[a0];
}
extern void LoadImage(s32, s32);

typedef struct {
    s16 x, y, w, h;
} Rect77D94;
extern Rect77D94 D_800A32FC;

void func_80077D94(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    Rect77D94 rect;
    s16 *in;
    s16 *out;
    Unk8009B400Record *table;
    /* FAKE: constant-holder local (named-local-fake-exception). abr is the
       tpage blend-mode bits func_8006E480 adds to every sprite's tpage.
       Mechanism: a pseudo live across the draw calls is seated once in
       callee-saved $s7 (`addiu s7,zero,0x20` in the prologue branch's
       delay slot, then `addu a1,s7,zero` at all 5 call sites). The inline
       literal re-materializes `li a1,0x20` per call.
       Same shape as func_80078654's `zero` and func_80070C70's `c60`.
       The literal at every call: score 12. */
    s32 abr;
    Unk8009B0E0Record **hp;
    s32 i;
    s32 x;
    s32 v;
    Win77D94 *w;
    s32 *img;

    s.ot_idx = 2;
    s.y = 0x1E;
    s.semi = 0;
    in = D_800A35F8->in30;
    table = D_800A35F8->table;
    out = D_800A35F8->out34;
    abr = 0x20;
    /* FAKE (duplicated-statement-into-arms): both fade arms store their own
       has_color/r/g/b; the compiler cross-jumps the identical tails. The
       shared-tail spelling (arms set v only, the else skips the stores with
       a goto) does not match: score 23. */
    if (D_800A35F0 < in[D_800A3600] + 60 && D_800A35F0 >= in[D_800A3600]) {
        v = ((D_800A35F0 - in[D_800A3600]) << 7) / 60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else if (D_800A35F0 >= out[D_800A3600] && D_800A35F0 < out[D_800A3600] + 60) {
        v = ((60 - (D_800A35F0 - out[D_800A3600])) << 7) / 60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else {
        s.has_color = 0;
    }

    if (D_800A35F0 < out[5] + 60) {
        switch (D_800A3600) {
        case 0:
            if (D_800A35F0 + 1 >= out[D_800A3600] + 60) {
                if (D_800A35F0 + 1 >= in[D_800A3600 + 1]) {
                    D_800A3600 = D_800A3600 + 1;
                }
            } else if (D_800A35F0 >= in[D_800A3600]) {
                hp = D_800A35F8->hdr18;
                for (i = 0; i < 5; hp++, i++) {
                    s.header = *hp;
                    s.table = table;
                    s.x = i << 7;
                    s.sprt_out = arg0->unk_04.unk_0C;
                    arg0->unk_04.unk_0C = func_8007352C(&s);
                    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, abr), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
                    arg0->unk_04.unk_14++;
                }
            }
            break;
        case 2:
            if (D_800A35F0 + 1 >= out[D_800A3600] + 60) {
                if (D_800A35F0 + 1 >= in[D_800A3600 + 1]) {
                    D_800A3600 = D_800A3600 + 1;
                }
                break;
            }
            hp = D_800A35F8->hdr24;
            for (i = 0, x = 0x156; i < 2; x += 0x80, hp++, i++) {
                s.header = *hp;
                s.x = x;
                s.table = table;
                s.sprt_out = arg0->unk_04.unk_0C;
                arg0->unk_04.unk_0C = func_8007352C(&s);
                SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, abr), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
                arg0->unk_04.unk_14++;
            }
            /* fall through */
        case 1:
            if (D_800A35F0 + 1 >= in[D_800A3600 + 1] && D_800A3600 == 1) {
                D_800A3600 = 2;
            }
            if (D_800A35F0 < out[2] && D_800A35F0 >= in[2]) {
                s.has_color = 0;
            }
            hp = D_800A35F8->hdr20;
            for (i = 0, x = 0x2B; i < 2; x += 0x80, hp++, i++) {
                s.header = *hp;
                s.x = x;
                s.table = table;
                s.sprt_out = arg0->unk_04.unk_0C;
                arg0->unk_04.unk_0C = func_8007352C(&s);
                SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, abr), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
                arg0->unk_04.unk_14++;
            }
            break;
        case 3:
            rect = D_800A32FC;
            for (i = 0; i < 5; i++) {
                /* FAKE: named intermediate for the TIM-pointer slot's address.
                   Mechanism: loop.c strength-reduces it to the target's giv
                   ($s1 = 0x38, += 4, added to D_800A35F8). Written inline
                   (`D_800A35F8->img38[i]`), fold moves 0x38 into the load
                   displacement (`sll; addu; lw a1,56(v0)`) and the giv is not
                   reduced (score 9). */
                img = &D_800A35F8->img38[i];
                LoadImage((s32)&rect, *img + 0x220);
                DrawSync(0);
                rect.x += 0x40;
            }
            D_800A3600 = D_800A3600 + 1;
            break;
        case 4:
            if (D_800A35F0 + 1 < out[D_800A3600] + 60) {
                hp = D_800A35F8->hdr1C;
                for (i = 0; i < 5; hp++, i++) {
                    s.header = *hp;
                    s.table = table;
                    s.x = i << 7;
                    s.sprt_out = arg0->unk_04.unk_0C;
                    arg0->unk_04.unk_0C = func_8007352C(&s);
                    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, abr), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
                    arg0->unk_04.unk_14++;
                }
            }
            break;
        }
    }

    s.ot_idx = 1;
    s.y = 0;
    s.x = 0;
    hp = D_800A35F8->hdr28;
    for (i = 0; i < 21; i++) {
        /* FAKE: the window address is an integer sum converted to Win77D94 *: as
           `&D_800A35F8->win2C[i]` win2C comes first in the addu (`addu a0,v0,s0` for
           the target's `addu a0,s0,v0`): score 1; `w = win2C; w += i`, 2. */
        w = (Win77D94 *)(i * 4 + (s32)D_800A35F8->win2C);
        if (D_800A35F0 < w->off + 60 && D_800A35F0 >= w->on) {
            /* FAKE (duplicated-statement-into-arms): each arm stores its own
               r/g/b chain; the compiler cross-jumps the identical tails back
               into one. One shared chain after the if/else puts the value in
               a separate pseudo and costs a `move` at the join (shared tail: score 54). */
            if (D_800A35F0 < w->on + 60) {
                s.has_color = 1;
                v = ((D_800A35F0 - w->on) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else if (D_800A35F0 >= w->off && D_800A35F0 < w->off + 60) {
                /* FAKE: named intermediate for the frame 60 ticks back.
                   Mechanism: inline, fold reassociates off - (cnt - 60)
                   into (off + 60) - cnt; the target computes cnt - 60
                   first (`addiu v0,a1,-60`); neither the inline form nor
                   60 - (cnt - off) matches (inline: score 16). */
                s32 t = D_800A35F0 - 60;
                s.has_color = 1;
                v = ((w->off - t) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else {
                s.has_color = 1;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = 0x72;
            }
            s.header = hp[i];
            s.table = s.header->cells;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
            SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, abr), 0);
            AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0->unk_04.unk_14);
            arg0->unk_04.unk_14++;
        }
    }
}
s32 func_800784E4(s32 arg0) {
    Ctx77D94 *s0;
    s32 r;

    ClearOTagR((u32 *)g_gpu_ot_ptr, 0x1008);
    s0 = (Ctx77D94 *)(arg0 + 0x58);
    D_800A35F4 = (Unk8006E49CRec *)arg0;
    D_800A35F8 = s0;
    func_8006E950(0x32, &s0->unk_00);
    r = func_80077D10((s32 *)s0);
    func_8006E49C(r, D_800A35F4);
    D_800A35FC = 0;
    D_800A35F0 = 0;
    D_800A3600 = 0;
    return 1;
}

extern void func_80077D94(Unk8006EACCRec *);

s32 func_8007855C(s32 arg0) {
    Unk8006EACCRec s;
    Unk8006E49CRec *p;
    s32 c;
    c = D_800A35FC + 1;
    D_800A35FC = c;
    p = func_80077D74(c & 1);
    s.unk_04.unk_00 = p->unk_00;
    s.unk_04.unk_08 = p->unk_08;
    s.unk_04.unk_0C = p->unk_10;
    s.unk_04.unk_10 = p->unk_0C;
    s.unk_04.unk_14 = p->unk_14;
    s.unk_04.unk_18 = p->unk_18;
    s.unk_04.unk_1C = p->unk_1C;
    if (D_800A35F0 > 0 && (arg0 & 0x40)) {
        return 1;
    }
    func_80077D94(&s);
    {
        s16 *out = D_800A35F8->out34;
        s32 new_val = D_800A35F0 + 1;
        int cond = new_val < out[5];
        D_800A35F0 = new_val;
        return cond ? 0 : 1;
    }
}
s32 func_80078628(Unk80078824Rec *a0) {
    return a0->unk_00.unk_04;
}
Unk8006E49CRec *func_80078634(s32 a0) {
    return &D_800A360C[a0];
}


void func_80078654(Unk800788B0Rec *arg0) {
    Unk8007352CEnv s;
    Unk8009B0E0Record **var_s0;
    /* FAKE: constant-holder local, kept live across the SetDrawMode /
       func_8006E480 / AddPrim call sequence so the 0 argument comes out of a
       register instead of being re-materialized at each use.  Mechanism:
       local-alloc/global-alloc seat the constant in a call-saved quantity;
       with the literal 0 the function comes out 3 instructions short of
       the target (score 7). */
    s32 zero;

    zero = 0;
    s.ot_idx = 2;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.header = D_800A3610->unk_14[10];
    s.y = 0;
    s.table = s.header->cells;
    var_s0 = D_800A3610->unk_14;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.has_color = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.col_r = s.col_g = s.col_b = sv;
        }
        s.sprt_out = arg0->unk_0C;
        arg0->unk_0C = func_8007352C(&s);
        SetDrawMode(arg0->unk_14, 1, 0, func_8006E480(s.header, zero), 0);
        AddPrim(g_gpu_ot_ptr + (s.ot_idx * 4), arg0->unk_14);
        arg0->unk_14++;
    }
    s.has_color = 0;
    goto check;
loop:
    /* FAKE: 8-deep do-while(0) wrap around the walk-pointer read block.
       Effect: seats the table-walk pointer var_s0 in $s0 and the parameter
       arg0 in $s1 (loop-note reference weighting -> allocno priority).
       Mechanism: flow.c:2081 counts each register mention as loop_depth
       references, so the wrap multiplies var_s0's reg_n_refs without
       emitting an instruction; global.c's allocno priority then ranks
       var_s0 (13 refs / 91 live, pri 4285) above arg0 (13 / 98, pri 3979).
       A single level is insufficient: depth 1 yields 6 of the 13
       references the priority inversion requires; depth 8 is the minimum
       that reaches 13 at the only wrap site that costs no delay slot. Unwrapped:
       score 19. */
    do { do { do { do { do { do { do { do {
    s.header = var_s0[0];
    s.table = s.header->cells;
    s.y = -D_800A3608;
    } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    s.sprt_out = arg0->unk_0C;
    arg0->unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_14, 1, 0, func_8006E480(s.header, zero), 0);
    AddPrim(g_gpu_ot_ptr + (s.ot_idx * 4), arg0->unk_14);
    var_s0++;
    arg0->unk_14++;
check:
    if (var_s0[1] != (Unk8009B0E0Record *)-1) goto loop;
}

extern s32 D_800A3304;

s32 func_80078824(s32 arg0) {
    Unk80078824Rec *s0;

    ClearOTagR((u32 *)g_gpu_ot_ptr, 0x1008);
    s0 = (Unk80078824Rec *)(arg0 + 0x58);
    D_800A360C = (Unk8006E49CRec *)arg0;
    D_800A3610 = s0;
    func_8006E950(0x5F, &s0->unk_00);
    func_8006E49C(func_80078628(s0), D_800A360C);
    D_800A3304 = 0;
    D_800A3608 = 0;
    D_800A3614 = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

void func_80078654(Unk800788B0Rec *);
s32 func_800788B0(void) {
    Unk800788B0Rec buf;
    Unk8006E49CRec *v0;
    D_800A3304++;
    v0 = func_80078634(D_800A3304 & 1);
    buf.unk_00 = v0->unk_00;
    buf.unk_08 = v0->unk_08;
    buf.unk_0C = v0->unk_10;
    buf.unk_10 = v0->unk_0C;
    buf.unk_14 = v0->unk_14;
    buf.unk_18 = v0->unk_18;
    buf.unk_1C = v0->unk_1C;
    func_80078654(&buf);
    D_800A3608++;
    return D_800A3608 >= 0xB40;
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A3304 = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
u8 * D_800A36A0;
