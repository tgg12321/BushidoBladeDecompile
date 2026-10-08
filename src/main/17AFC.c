/* 80 game functions, among them math_LerpAngle. .text 0x800272FC (ROM 0x17AFC).
 * Start boundary: PHASE (rodata-align site 2). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"
#include "bb2_const.h"

/* P1/P2 round scores and tiebreakers, declared here as single u8s (per-file
 * declarations: Q21-Q25, no-new-park-categories aggregate-merge exception):
 * every counting aggregate spelling of func_800340A0 misses the shipped code
 * (constant subscripts put element 0 behind a base register; the
 * index-variable and regrouped-condition spellings miss its round-result
 * stores or compares); dummy-index and pointer-alias spellings are refused
 * (Q22/Q23). src/main/9F9C.c declares the same bytes as D_800A3898[2] /
 * D_800A38AA[2] for func_8001CE60, whose player-indexed accesses single bytes
 * do not produce; no single declaration compiles both files with a counting
 * spelling. */
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A38AA;
extern u8 D_800A38AB;

INCLUDE_RODATA("asm/rodata", D_80010478);

s32 func_800272FC(s32 a0) {
    s32 v1;
    if (a0 < 0) {
        a0 = -a0;
    }
    if (a0 < 0xA01) {
        if (a0 < 0x600) {
            v1 = 1;
        } else {
            v1 = 2;
        }
    } else {
        v1 = 0;
    }
    return v1;
}

void func_80027334(MotionFrame *arg0) {
    arg0->unk_0C[0x18] = 0x3F5;
    arg0->unk_0C[0x19] = 0x2B6;
    arg0->unk_0C[0x1A] = 0x77A;
    arg0->unk_0C[0x1B] = 0xBEE;
    arg0->unk_0C[0x1C] = 0x8C;
    arg0->unk_0C[0x1D] = 0x227;
    arg0->unk_0C[0x21] = 0x8A0;
    arg0->unk_0C[0x22] = 0xF0E;
    arg0->unk_0C[0x1E] = 0;
    arg0->unk_0C[0x1F] = 0;
    arg0->unk_0C[0x20] = 0;
    arg0->unk_0C[0x23] = 0xBCD;
}

/* D_800A376A / D_800A376B are one u8 per player (the hit-limb bits); a0 is
 * the player, and `*(&D_800A376A + a0)` reaches [1] through [0]'s address.
 * FAKE (Q97): two u8 scalars; as u8[2], func_8003B2C8 / func_8003B328 were
 * reported at 12 / 16 (not banked). */
void func_8002738C(s32 a0, s32 a1) {
    if (D_800A38DC != 0) {
        return;
    }
    switch (a1) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        *(&D_800A376A + a0) |= 0x10; /* FAKE: Q97 */
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        *(&D_800A376A + a0) |= 0x01; /* FAKE: Q97 */
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        *(&D_800A376A + a0) |= 0x02; /* FAKE: Q97 */
        break;
    case 14:
    case 15:
    case 16:
    case 17:
        *(&D_800A376A + a0) |= 0x04; /* FAKE: Q97 */
        break;
    case 18:
    case 19:
    case 20:
    case 21:
        *(&D_800A376A + a0) |= 0x08; /* FAKE: Q97 */
        break;
    }
}

void func_80027438(Unk80101EC8Record *a0, s32 a1, s16 a2) {
    s16 v1;
    v1 = D_800A38DC;
    if (v1 == 5) {
        return;
    }
    if ((u32)a1 >= 0x16) {
        return;
    }
    switch (a1) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        a0->unk_272 += a2;
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        a0->unk_26C = 0;
        a0->unk_90 = 0;
        a0->unk_8A = 0;
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        a0->unk_270 += a2;
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        a0->unk_26E += a2;
        break;
    }
}

void func_800274BC(s32 *arg0, s16 *arg1) {
    u32 dist_sq =
        (u32)((arg0[0] * arg0[0]) + (arg0[1] * arg0[1]) + (arg0[2] * arg0[2]));
    u32 log2_val;
    if (dist_sq < 0x400) {
        log2_val = ((u32)(g_sqrt_table_u8[dist_sq])) >> 3;
    } else {
        s32 sp_tmp;
        /* Hand-written GTE leading-zero count (LZCS in, LZCR out), as in
         * func_8001A67C. */
        __asm__ volatile(
            "addu   $t4, %1, $zero\n"
            "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
            "nop\n"
            "nop\n"
            "addu   $t4, $sp, $zero\n" /* &sp_tmp (at 0($sp)) */
            "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
            : "=m"(sp_tmp)
            : "r"(dist_sq)
            : "$12");
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = dist_sq >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(g_sqrt_table_u8[idx]));
            log2_val = (hi << 16) >> (0x13 - v1_m);
        }
    }
    arg1[0] = (s16)(((-arg0[0]) << 12) / ((s32)log2_val));
    arg1[1] = (s16)(((-arg0[1]) << 12) / ((s32)log2_val));
    arg1[2] = (s16)(((-arg0[2]) << 12) / ((s32)log2_val));
}

void func_80027640(Unk80101EC8Record *arg0) {
    VECTOR tgt;
    VECTOR dir;
    s32 idx;
    s16 *tbl;
    s32 rate;
    u8 cnt;
    u16 *r1;
    void *r2;
    s32 vx;
    s32 vz;

    idx = arg0->index;
    tbl = func_80046F14();
    cnt = arg0->unk_34C;
    if (cnt < 0x40) {
        arg0->unk_34C = cnt + 1;
    }
    rate = 0x3C - ((arg0->unk_34C - 1) * 4);
    if (rate < 10) {
        rate = 10;
    }
    if (D_800A36A4 == 3) {
        vx = 0x2EE0;
        if (arg0->other->unk_F4.x >= 0x3E9) {
            vx = -0x2710;
        }
        dir.vx = vx;
        vz = 0x1770;
        if (arg0->other->unk_F4.z > 0) {
            vz = -0x1770;
        }
        dir.vz = vz;
        tgt.vx = (dir.vx * rate + arg0->unk_F4.x * (100 - rate)) / 100;
        tgt.vz = (dir.vz * rate + arg0->unk_F4.z * (100 - rate)) / 100;
    } else {
        tbl += (D_800A36A4 * 12 + idx * 3);
        tgt.vx = tbl[0];
        tgt.vz = tbl[2];
    }
    tgt.vx -= arg0->unk_F4.x;
    tgt.vz -= arg0->unk_F4.z;
    arg0->unk_F4.x += tgt.vx;
    arg0->unk_F4.z += tgt.vz;
    arg0->unk_D8.x += tgt.vx;
    arg0->unk_D8.z += tgt.vz;
    arg0->unk_B8.vx += tgt.vx;
    arg0->unk_B8.vz += tgt.vz;
    arg0->unk_104.vx = 0;
    arg0->unk_104.vy = 0;
    arg0->unk_104.vz = 0;
    arg0->unk_134.vx = 0;
    arg0->unk_134.vy = 0;
    arg0->unk_134.vz = 0;
    r1 = func_80021424(arg0, arg0->unk_50->unk_00, &arg0->unk_5E);
    r2 = func_80021424(arg0, r1[0x1D], &arg0->unk_5E);
    func_80021A98(idx, r2, arg0->unk_5E);
    func_80032854(arg0->index, 0x30, &arg0->unk_F4.x, 0);
}

void func_800278C0(s32 a0, Unk80101EC8Record *ptr, s32 cmd, Tbl8008E194 *a3,
                   s32 *stack_a2, s32 stack_v1) {
    Unk80101EC8Record *chk_obj;

    if (a0 == 1) {
        return;
    }

    if (stack_v1 != 0) {
        func_80032854(ptr->other->index, 0x2B, stack_a2, (s16 *)0);
        return;
    }

    chk_obj = ptr->other;
    {
        s16 f86 = chk_obj->unk_86;
        s16 f8E = chk_obj->unk_8E;

        if (f86 == f8E) {
            func_80032854(chk_obj->index, 0x28, stack_a2, (s16 *)0);
            return;
        }

        {
            s16 f88 = chk_obj->unk_88;
            if (f86 == f88 && a3 != 0) {
                func_80032854(chk_obj->index, 0x27, stack_a2, (s16 *)0);
                return;
            }
        }
    }

    if (cmd == 0) {
        func_80032854(ptr->other->index, 0x22, stack_a2, (s16 *)0);
        return;
    }

    if (cmd < 6) {
        func_80032854(ptr->other->index, 0x23, stack_a2, (s16 *)0);
        return;
    }

    func_80032854(ptr->other->index, 0x24, stack_a2, (s16 *)0);
}

s32 func_8002798C(Unk80101EC8Record *a0) {
    s32 ret = 0;
    u16 v1 = a0->unk_6A;

    if (v1 == 6 || v1 == 0x25 || v1 == 0x33 || v1 == 4 || v1 == 0x14) {
        goto check_88;
    }
    {
        s16 v40 = a0->unk_40;
        if (v40 < a0->unk_A5) {
            goto check_88;
        }
        if (!(a0->unk_A6 < v40)) {
            goto final_1;
        }
    }

check_88: {
    s16 v88 = a0->unk_88;
    s32 a0_6a;
    if (v88 == -1) {
        goto done;
    }
    a0_6a = a0->unk_6A;
    if ((u16)a0_6a == 0xF) {
        goto final_1;
    }
    if ((u32)(a0_6a - 0x1C) < 2) {
        goto final_1;
    }
    if ((u32)(a0_6a - 0x1E) < 2) {
        goto final_1;
    }
    if ((u32)(a0_6a - 0x20) < 2) {
        goto final_1;
    }
    if ((u16)a0_6a == 0x2B) {
        goto final_1;
    }
    goto done;
}

final_1:
    ret = 1;
done:
    return ret;
}

void func_80027A58(Unk80101EC8Record *a0) {
    s16 v1 = a0->unk_86;
    if (v1 == a0->unk_88) {
        if (a0->unk_8A) {
            if (func_8002798C(a0)) {
                u8 *v0 = func_80046DEC(a0->index);
                func_80030900(a0, *(s32 *)(v0 + 0x4C) + 0x14);
                a0->unk_8A = 0;
                a0->unk_86 = a0->unk_84;
            }
        }
    }
}

extern u8 D_8008EB74[3][2][2];

s32 func_80027AD8(s32 pass, Unk80101EC8Record *ch, s32 limb, s32 thresh,
                  s32 flag, Tbl8008E194 *rec, s32 arg6, s32 *out) {
    /* FAKE: tbl, tbl_arg: copies of the stack-passed parameter `rec` (an
     * unmodified stack parameter gets half the register priority); rec used
     * directly: score 78 (Ruling 12) */
    Tbl8008E194 *tbl;     /* the record, read field by field */
    Tbl8008E194 *tbl_arg; /* the record, passed on to func_800278C0 */
    s16 *vec;
    s32 *scr;
    s32 player;
    Unk80101EC8Record *opp;
    s32 dot;
    s32 dot_lo;
    u16 st;
    s32 diff;
    s32 cat;
    u32 sign;
    s32 same;
    s32 code;

    /* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector reached by the
     * first's address, passed to func_800203B4 and read as vec[0..2] (Q96); as
     * s16[3] or {x,y,z}: score 2 */
    vec = &D_800A37E8;
    player = ch->index;
    opp = ch->other;
    tbl = rec;
    tbl_arg = rec;
    scr = &SPAD->unkA8[player][limb].x;
    *out = 0;
    if (ch->unk_0C == 0x1C) {
        dot = (ch->unk_1EC.x * D_800A37E8 + ch->unk_1EC.y * D_800A37EA +
               ch->unk_1EC.z * D_800A37EC) >>
              12;
        dot &= 0x1FFF;
        if (dot >= 0x1000) {
            dot -= 0x2000;
        }
        dot_lo = dot < 0x400;
        if (!((0xE >> limb) & 1) || dot_lo) {
            if (ch->unk_96 == 0) {
                ch->unk_286 = 0x1E;
            }
            if (pass == 0 && opp->unk_286 == -1) {
                opp->unk_286 = tbl == NULL ? 0xB : 0x19;
            }
            func_80032854(player, 0x12, scr, 0);
            return 0;
        }
    }
    if (ch->unk_0C == 0xD && ch->unk_96 == 0 && ch->unk_6A != 0x2E) {
        func_80027640(ch);
        return 2;
    }
    st = ch->unk_6A;
    if (st == 4 || st == 0x14) {
        func_800203B4(ch, limb, vec);
        func_800278C0(pass, ch, limb, tbl_arg, scr, arg6);
        return 1;
    }
    if (ch->unk_0C == 0x1F) {
        func_800203B4(ch, limb, vec);
        func_800278C0(pass, ch, limb, tbl_arg, scr, arg6);
        ch->unk_286 = 7;
        return 1;
    }
    if ((st == 2 || st == 0x1B || st == 0x28 || st == 0x26) &&
        ch->unk_40 < ch->unk_A0) {
        if (pass == 0) {
            diff = opp->unk_20 - ch->unk_20;
            cat = func_800272FC(diff);
            /* FAKE: diff's sign bit taken before the same / unk_B4 stores and
             * the call; read at its use, ch and limb swap $s0 / $s1: score 102
             */
            sign = (u32)diff >> 31;
            same = ch->unk_AF == opp->unk_AF;
            ch->unk_B4 = opp->unk_B4 = same;
            func_80032854(player, same ? 0xF : 2, scr, 0);
            code = ch->unk_286 = D_8008EB74[cat][sign][same];
            if (code == 0) {
                func_80032854(player == 0, 0x25, scr, 0);
                func_80032854(player, 0x25, scr, 0);
            } else if (code == 1) {
                func_80032854(player == 0, 0x26, scr, 0);
                func_80032854(player, 0x25, scr, 0);
                func_80032854(player, 0x2D, scr, 0);
            } else if (code == 2) {
                func_80032854(player == 0, 0x26, scr, 0);
                func_80032854(player, 0x26, scr, 0);
                func_80032854(player, 0x2D, scr, 0);
                func_80027A58(ch);
            } else {
                return 0;
            }
            if (ch->unk_286 != 2) {
                return 0;
            }
            if (vec[1] * vec[1] < vec[0] * vec[0] + vec[2] * vec[2]) {
                func_8001F860(ch, ratan2(vec[0], vec[2]));
                ch->unk_134.vx -= vec[0] / 16;
                ch->unk_134.vz -= vec[2] / 16;
            }
            return 0;
        }
        if (pass == 1) {
            if (tbl->unkC == 0) {
                func_80032854(player, 2, scr, 0);
                func_80032854(player, 0x25, scr, 0);
                ch->unk_286 = 0;
                *out = pass;
                return 0;
            }
            if (tbl->unkC == 1) {
                func_80032854(player, 2, scr, 0);
                func_80032854(player, 0x25, scr, 0);
                ch->unk_286 = 1;
                func_8001F860(ch, ratan2(vec[0], vec[2]));
                ch->unk_134.vx -= vec[0] / 16;
                ch->unk_134.vz -= vec[2] / 16;
                *out = pass;
                return 0;
            }
        }
    }
    if (D_800A38DC == 0 && player == 0) {
        func_8002738C(0, limb);
    }
    func_800278C0(pass, ch, limb, tbl_arg, scr, arg6);
    st = ch->unk_6A;
    if (st == 6 || st == 9) {
        if (flag) {
            ch->unk_286 = ch->unk_1DA ? 6 : 7;
            func_800203B4(ch, limb, vec);
            return 1;
        }
        ch->unk_286 = ch->unk_1DA ? 3 : 4;
        func_80032854(player, 3, scr, 0);
        func_80027438(ch, limb, 1);
        return 0;
    }
    if (pass == 1 && tbl->unk0 == 4) {
        s16 kind = ch->unk_0A;
        if (kind == 2 || kind == 4 || kind == 0xE || kind == 0xF) {
            func_80027438(ch, limb, tbl->unkD == 2 ? 2 : 1);
            ch->unk_286 = 0x1B;
            func_80032854(player, 3, scr, 0);
            return 0;
        }
    }
    switch (limb) {
    case 0:
        if (flag) {
            ch->unk_286 = thresh > 0x400 ? 6 : 9;
            func_800203B4(ch, limb, vec);
            func_80027A58(ch);
            return 1;
        }
        /* FAKE: count/return tail duplicated into each arm: the extra copies
         * raise tbl's reference count (allocation priority) before
         * cross-jumping re-merges them; cases 0, 1-3 jumping to the case
         * 4-5 copy: score 31 */
        if (pass == 1 && tbl->unkD == 2) {
            ch->unk_272++;
        }
        if (D_800A38DC != 5) {
            ch->unk_272++;
        }
        ch->unk_286 = 5;
        func_80032854(player, 3, scr, 0);
        func_80027A58(ch);
        return 0;
    case 1:
    case 2:
    case 3:
        if (flag) {
            ch->unk_286 = thresh > 0x400 ? 7 : 9;
            func_800203B4(ch, limb, vec);
            func_80027A58(ch);
            return 1;
        }
        /* FAKE: count/return tail duplicated into each arm: the extra copies
         * raise tbl's reference count (allocation priority) before
         * cross-jumping re-merges them; cases 0, 1-3 jumping to the case
         * 4-5 copy: score 31 */
        if (pass == 1 && tbl->unkD == 2) {
            ch->unk_272++;
        }
        if (D_800A38DC != 5) {
            ch->unk_272++;
        }
        ch->unk_286 = 5;
        func_80032854(player, 3, scr, 0);
        func_80027A58(ch);
        return 0;
    case 4:
    case 5:
        if (!flag) {
            /* FAKE: count/return tail duplicated into each arm: the extra
             * copies raise tbl's reference count (allocation priority) before
             * cross-jumping re-merges them; cases 0, 1-3 jumping to the case
             * 4-5 copy: score 31 */
            if (pass == 1 && tbl->unkD == 2) {
                ch->unk_272++;
            }
            if (D_800A38DC != 5) {
                ch->unk_272++;
            }
            ch->unk_286 = 5;
            func_80032854(player, 3, scr, 0);
            func_80027A58(ch);
            return 0;
        }
        ch->unk_286 = thresh > 0x400 ? 8 : 9;
        func_800203B4(ch, limb, vec);
        func_80027A58(ch);
        return 1;
    case 6:
    case 7:
    case 8:
    case 9:
        if (D_800A38DC != 5) {
            ch->unk_26C = 0;
        }
        ch->unk_286 = 4;
        ch->unk_90 = 0;
        func_80032854(player, 3, scr, 0);
        func_80027A58(ch);
        ch->unk_8A = 0;
        return 0;
    case 10:
    case 11:
    case 12:
    case 13:
        if (D_800A38DC != 5) {
            ch->unk_270++;
        }
        ch->unk_286 = 5;
        func_80032854(player, 3, scr, 0);
        func_80027A58(ch);
        return 0;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        if (D_800A38DC != 5) {
            ch->unk_26E++;
        }
        ch->unk_286 = 3;
        func_80032854(player, 3, scr, 0);
        func_80027A58(ch);
        return 0;
    }
}

s32 func_800283D0(Unk80101EC8Record *arg0, s32 *arg1) {
    s32 temp_a1;
    Unk80101EC8Record *temp_s4;
    s32 temp_v1;
    s32 var_s1;
    s16 var_v0;
    s32 ret;

    temp_s4 = arg0->other;
    ret = 1;
    temp_a1 = arg0->unk_6A;
    /* FAKE: redundant mask of the u16 state code, reproduces the target's andi
     * $v1,$a1,0xFFFF; without it score 9 */
    temp_v1 = temp_a1 & 0xFFFF;
    if (temp_v1 == 4) {
        goto ret_one;
    }
    if (temp_v1 == 0x14) {
        return ret;
    }
    {
        u16 temp_v0 = temp_s4->unk_6A;
        if (temp_v0 == 4) {
            goto ret_one;
        }
        if (temp_v0 == 0x14) {
            goto ret_one;
        }
        {
            s32 d_val;
            s32 temp_a1_2;
            s32 temp_s5;

            if (temp_v1 != 0x13) {
                if (((u32)(temp_a1 - 0x19) >= 2U) && (temp_v1 != 2) &&
                    (temp_v1 != 0x26) && (temp_v1 != 0x1B) &&
                    (temp_v1 != 0x15) && (temp_v1 != 0x25) &&
                    (temp_v1 != 0x2C) && (temp_v1 != 0xC)) {
                ret_one:
                    return 1;
                }
            }
            var_s1 = 0;
            d_val = D_800A3824;
            temp_a1_2 = (d_val >> arg0->index) & 1;
            temp_s5 = (d_val >> temp_s4->index) & 1;
            if (arg0->unk_8C != 0) {
                var_s1 = temp_a1_2 == 0;
            }
            if (var_s1 != 0) {
                s16 temp_v1_2 = arg0->unk_0C;
                if (temp_v1_2 != 0x1D) {
                    if (temp_v1_2 != 0xE) {
                        goto block_20;
                    }
                    return ret;
                }
                goto block_49;
            }
        block_20: {
            s16 temp_v1_3 = temp_s4->unk_288[temp_s5];
            if (temp_v1_3 == 0) {
                /* the target tests unk_288 signed here (lh + blez), against its
                 * u16 declaration */
                if ((s16)arg0->unk_288[temp_a1_2] > 0) {
                    var_v0 = 0x19;
                    if (var_s1 == 0) {
                    set_0xB:
                        var_v0 = 0xB;
                    }
                do_store_calls:
                    arg0->unk_286 = var_v0;
                do_calls:
                    func_80032854(arg0->index, 1, arg1, (s16 *)0);
                    func_80032854(arg0->index, 0x25, arg1, (s16 *)0);
                    return ret;
                }
                goto block_49;
            }
            {
                s16 temp_v0_3 = arg0->unk_288[temp_a1_2];
                s16 var_v0_2;
                if (temp_v0_3 == temp_v1_3) {
                    /* FAKE: do-while(0) loop depth weights these refs in
                     * register allocation; without it score 12
                     * (do-while-zero-exception) */
                    do {
                        func_80032854(arg0->index, 1, arg1, (s16 *)0);
                        func_80032854(arg0->index, 0x25, arg1, (s16 *)0);
                    } while (0);
                    /* FAKE: (s16) view of the u16 unk_288: the target reloads
                     * it with lh here; without the cast lhu, score 1 */
                    if ((s16)arg0->unk_288[temp_a1_2] == 5) {
                        if (arg0->unk_0E == 6 || arg0->unk_0E == 7 ||
                            temp_s4->unk_0E == 6 || temp_s4->unk_0E == 7) {
                            if (var_s1 != 0) {
                                goto sel19;
                            }
                            var_v0_2 = 0xB;
                            goto block_48;
                        sel19:
                            /* FAKE: store duplicated into this arm instead of
                             * sharing block_48's copy; cross-jumping picks
                             * which copy survives; sharing it scores 2 */
                            arg0->unk_286 = 0x19;
                            goto block_49;
                        }
                        D_800A38A8 = 1;
                        D_800A3876 = -1;
                        goto block_49;
                    }
                    {
                        /* FAKE: named intermediate for the selected constant
                         * keeps the two constants in the target's order (a
                         * plain ternary reorders them: score 2) */
                        s32 sel = (var_s1 == 0) ? 0xB : 0x19;
                        var_v0_2 = sel;
                    }
                    goto block_48;
                }
                if (temp_v1_3 < temp_v0_3) {
                    s16 var_v0_4 = 0x19;
                    if (var_s1 == 0) {
                        goto set_0xB;
                    }
                    /* FAKE: store duplicated into the `<` arm instead of
                     * sharing do_store_calls's copy (cross-jump tail merge);
                     * sharing it scores 4 */
                    arg0->unk_286 = var_v0_4;
                    goto do_calls;
                }
                func_80032854(arg0->index, 0x26, arg1, (s16 *)0);
                func_80032854(arg0->index, 0x2D, arg1, (s16 *)0);
                var_v0_2 = 0x1A;
                if (var_s1 == 0) {
                    s32 temp_v1_4 = -arg0->unk_1C8.vy;
                    s32 temp_v1_5 =
                        (s32)(Judge[(temp_v1_4 + 0x400) & 0xFFF] *
                                  temp_s4->unk_114[temp_s5].vx +
                              Judge[temp_v1_4 & 0xFFF] *
                                  temp_s4->unk_114[temp_s5].vz) >>
                        0xC;
                    s32 temp_a0_2 = temp_s4->unk_114[temp_s5].vy;
                    s32 var_a1 = temp_a0_2;
                    s32 var_v0_3;
                    if (temp_a0_2 < 0) {
                        var_a1 = -temp_a0_2;
                    }
                    var_v0_3 = temp_v1_5;
                    if (temp_v1_5 < 0) {
                        var_v0_3 = -temp_v1_5;
                    }
                    if (var_v0_3 < var_a1) {
                        var_v0_2 = 0x14;
                        if (temp_a0_2 > 0) {
                            var_v0_2 = 0x13;
                        }
                    } else {
                        var_v0_2 = 0x15;
                        if (temp_v1_5 <= 0) {
                            var_v0_2 = 0x16;
                        }
                    }
                }
            block_48:
                arg0->unk_286 = var_v0_2;
            }
        }
        block_49:
            return ret;
        }
    }
}

void func_8002872C(void) {
    s32 i = 0;
    Unk80101EC8Record *base;

    do {
        s32 cmp_a1;
        s32 cmp_a2;
        Unk80101EC8Record *ptr;
        s32 a0_raw;
        s32 v1;

        base = &D_80101EC8[i];

        cmp_a1 = base->unk_0C;
        if (cmp_a1 != 0x1B)
            goto next;

        if (base->unk_46 != 0)
            goto next;

        cmp_a2 = base->unk_6A;
        if (cmp_a2 != 0xB)
            goto next;

        if (base->unk_40 != base->unk_A7)
            goto next;

        ptr = base->other;
        a0_raw = ptr->unk_6A;
        v1 = a0_raw & 0xFFFF;

        if (v1 == 0x26)
            goto match;
        if (v1 == cmp_a1)
            goto match;
        if (v1 == 2)
            goto match;
        if (v1 == 0x15)
            goto match;
        if ((u32)(a0_raw - 0x24) < 2)
            goto match;
        if (v1 == 8)
            goto match;
        if ((u32)(a0_raw - 0x22) < 2)
            goto match;
        if (v1 == 0)
            goto match;
        if (v1 == 0x10)
            goto match;
        if (v1 == 0x13)
            goto match;
        if ((u32)(a0_raw - 0x30) < 2)
            goto match;
        if (v1 == 0x1A)
            goto match;
        if (v1 == cmp_a2)
            goto match;
        if (v1 == 0x12)
            goto match;
        if (v1 == 0x2A)
            goto match;
        if (v1 == 0xC)
            goto match;
        if (v1 != 0x19)
            goto next;

    match:
        if (!(D_800A387C < D_800A3134))
            goto next;

        base->other->unk_286 = 0x1C;
        func_80027A58(base->other);

        {
            Unk80101EC8Record *p2 = base->other;
            if (p2->unk_6A == 0x25) {
                p2->unk_86 = p2->unk_84;
            }
        }

    next:
        i++;
    } while (i < 2);
}

extern s32 D_800A3144[2];

/* Body-overlap push between the two characters (D_80101EC8[0] and [1]),
 * skipped while either's unk_6A is 0x28. Each character's two points (joint 1,
 * and the midpoint of joints 15 and 19) are tested pairwise with the radii
 * D_800A3144[0..1]; an overlap (at most 0x80) pushes the characters apart along
 * x/z, scaled by the distance. After a hit, the unk_6A states and the facing
 * difference pick the reaction (unk_286, D_800A38A8 / D_800A3876). The push is
 * added to unk_134.vx / .vz.
 * GTE: gte_Lzc (gte_ldlzc, gte_nop x2, gte_stlzc), inline_o.h class
 * (inline-asm-policy, owner ruling 2026-09-26). Everything else is ordinary C
 * except the three FAKE-annotated locals below (dist, tbl, shift). */
void func_800288C8(void) {
    s32 lzc_out;
    s32 hits;
    s32 i;
    s32 j;
    s32 c;
    s32 push1_x;
    s32 push1_z;
    s32 push2_x;
    s32 push2_z;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 reach;
    /* FAKE: two values -- the squared distance, then its square root (clamped
     * to 1); a separate squared-distance local reorders the copy and the r*r
     * test (score 7) */
    s32 dist;
    /* FAKE: two values -- the gte_Lzc input copy of the squared distance,
     * then the table byte; a fresh copy local (or dist itself) takes $v1, not
     * the target's $a0: score 13 (Q28) */
    s32 tbl;
    s32 pen;
    s32 bias;
    s32 flag1;
    s32 flag2;
    s32 facing;

    if (D_80101EC8[0].unk_6A == 0x28 || D_80101EC8[1].unk_6A == 0x28) {
        return;
    }
    for (c = 0; c < 2; c++) {
        SPAD->unk78[c][0].x = SPAD->unkA8[c][1].x;
        SPAD->unk78[c][0].y = SPAD->unkA8[c][1].y;
        SPAD->unk78[c][0].z = SPAD->unkA8[c][1].z;
        SPAD->unk78[c][1].x = (SPAD->unkA8[c][15].x + SPAD->unkA8[c][19].x) / 2;
        SPAD->unk78[c][1].y = (SPAD->unkA8[c][15].y + SPAD->unkA8[c][19].y) / 2;
        SPAD->unk78[c][1].z = (SPAD->unkA8[c][15].z + SPAD->unkA8[c][19].z) / 2;
    }
    hits = 0;
    push1_x = push1_z = push2_x = push2_z = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            dx = SPAD->unk78[0][i].x - SPAD->unk78[1][j].x;
            dy = SPAD->unk78[0][i].y - SPAD->unk78[1][j].y;
            dz = SPAD->unk78[0][i].z - SPAD->unk78[1][j].z;
            reach = D_800A3144[i] + D_800A3144[j];
            if (dx > reach || dx < -reach || dy > reach || dy < -reach ||
                dz > reach || dz < -reach) {
                continue;
            }
            dist = dx * dx + dy * dy + dz * dz;
            if (reach * reach < dist) {
                continue;
            }
            hits++;
            tbl = dist;
            if ((u32)dist < 0x400) {
                dist = (u32)g_sqrt_table_u8[dist] >> 3;
            } else {
                /* gte_Lzc(tbl, &lzc_out) -- gtemac.h :174-178: gte_ldlzc,
                 * gte_nop x2, gte_stlzc */
                __asm__ volatile ("move  $12,%0": :"r"(tbl):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                {
                    /* FAKE: two values -- the leading-zero count, then the
                     * table shift; as one expression `shift = 0x16 - (lzc_out &
                     * ~1)` the count lands in $a0 (target: $v1 / $v0) */
                    s32 shift;
                    shift = lzc_out;
                    shift = 0x16 - (shift & ~1);
                    tbl = g_sqrt_table_u8[(u32)dist >> shift];
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            pen = reach - dist;
            if (pen > 0x80) {
                pen = 0x80;
            }
            if (dist == 0) {
                dist = 1;
            }
            bias = (D_80101EC8[1].unk_20 - D_80101EC8[0].unk_20) / 4;
            push1_x += (dx * (pen * (0x400 + bias)) / dist) >> 10;
            push1_z += (dz * (pen * (0x400 + bias)) / dist) >> 10;
            push2_x -= (dx * (pen * (0x400 - bias)) / dist) >> 10;
            push2_z -= (dz * (pen * (0x400 - bias)) / dist) >> 10;
        }
    }
    if (hits == 0) {
        return;
    }
    flag1 = 0;
    if (D_80101EC8[0].unk_6A == 0x13 || D_80101EC8[0].unk_6A == 0x1B ||
        D_80101EC8[0].unk_6A == 0x30) {
        if (D_80101EC8[0].unk_14E > 0x9C4) {
            flag1 = 1;
        }
    }
    flag2 = 0;
    if (D_80101EC8[1].unk_6A == 0x13 || D_80101EC8[1].unk_6A == 0x1B ||
        D_80101EC8[1].unk_6A == 0x30) {
        if (D_80101EC8[1].unk_14E > 0x9C4) {
            flag2 = 1;
        }
    }
    facing = (D_80101EC8[0].unk_1C8.vy - D_80101EC8[1].unk_1C8.vy) & 0xFFF;
    if (facing >= 0x800) {
        facing = 0x1000 - facing;
    }
    if (flag1) {
        if (flag2) {
            if (facing > 0x600) {
                func_80032854(0, 0x21, &D_80101EC8[0].unk_F4.x, 0);
                D_80101EC8[1].unk_286 = 0xF;
                D_80101EC8[0].unk_286 = 0xF;
                func_80027A58(&D_80101EC8[0]);
                func_80027A58(&D_80101EC8[1]);
                hits = 0;
            }
        } else {
            if (facing < 0x400 && D_80101EC8[1].unk_6A == 0x15) {
                D_80101EC8[1].unk_286 = 0x10;
                func_80027A58(&D_80101EC8[1]);
            }
            if (facing > 0x600 && D_80101EC8[1].unk_6A == 0x15) {
                if (D_80101EC8[0].unk_0E == 6 || D_80101EC8[0].unk_0E == 7 ||
                    D_80101EC8[1].unk_0E == 6 || D_80101EC8[1].unk_0E == 7) {
                    func_80032854(1, 0x21, &D_80101EC8[1].unk_F4.x, 0);
                    func_80032854(1, 0x2D, &D_80101EC8[1].unk_F4.x, 0);
                    D_80101EC8[1].unk_286 = 5;
                } else {
                    D_800A38A8 = 1;
                    D_800A3876 = 0;
                }
            }
        }
    } else if (flag2) {
        if (facing < 0x400 && D_80101EC8[0].unk_6A == 0x15) {
            D_80101EC8[0].unk_286 = 0x10;
            func_80027A58(&D_80101EC8[0]);
        }
        if (facing > 0x600 && D_80101EC8[0].unk_6A == 0x15) {
            if (D_80101EC8[0].unk_0E == 6 || D_80101EC8[0].unk_0E == 7 ||
                D_80101EC8[1].unk_0E == 6 || D_80101EC8[1].unk_0E == 7) {
                func_80032854(0, 0x21, &D_80101EC8[0].unk_F4.x, 0);
                func_80032854(0, 0x2D, &D_80101EC8[0].unk_F4.x, 0);
                D_80101EC8[0].unk_286 = 5;
            } else {
                D_800A38A8 = 1;
                D_800A3876 = 1;
            }
        }
    }
    if (hits != 0) {
        D_80101EC8[0].unk_134.vx += push1_x;
        D_80101EC8[0].unk_134.vz += push1_z;
        D_80101EC8[1].unk_134.vx += push2_x;
        D_80101EC8[1].unk_134.vz += push2_z;
    }
}

/* One 16-byte entry of the list func_8004678C returns (terminated by a zero
 * `type`); func_8002906C clears every entry's `used`. */
typedef struct {
    s16 type;
    s16 used;
    s32 x;
    s32 y;
    s32 z;
} PosRec;

void func_8002906C(void) {
    PosRec *ptr = (PosRec *)func_8004678C();
    while (ptr->type != 0) {
        ptr->used = 0;
        ptr++;
    }
}

s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3);
s32 func_8002FC80(VECTOR *a0, VECTOR *a1, VECTOR *a2);
void func_80033550(LeafPos *arg0);

/* Tests the unused PosRec entries inside grid `idx`'s x/z bounds against the
 * grid's two triangles (points 0,1,2 and 1,2,3) with func_8002E6B0. `tbl` holds
 * two 2x2 grids of points (entry idx * 4 + row * 2 + col); the bounds go to the
 * scratch record at 0x1F8002B8. A type-2 hit restarts on grid 1 when `flag` is
 * set and idx is 0; otherwise it calls func_80044B30 / func_80033550, marks the
 * entry used and returns 0. Any other hit, or a miss with `flag` set and idx
 * not 0, stores the average of unk78.y and the top y in unk100[0].y and returns
 * 1. Returns 0 when the list runs out. */
s32 func_800290B8(s32 idx, s32 flag, LeafPos *tbl) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    PosRec *rec;
    LeafPos *a;
    LeafPos *b;
    LeafPos *c;
    s32 i;
    s32 n;
    /* two values: the grid row i / 2, then the list index passed to
     * func_80044B30 (Ruling 11) */
    s32 temp;
    /* two values: the grid column i & 1, then the triangle number
     * (Ruling 11) */
    s32 temp2;

    scr->unk84 = tbl[idx * 4];
    scr->unk78 = scr->unk84;

    for (i = 1; i < 4; i++) {
        temp = i / 2;
        temp2 = i & 1;
        n = idx * 4 + temp * 2 + temp2;
        if (tbl[n].x < scr->unk78.x) {
            scr->unk78.x = tbl[n].x;
        } else if (scr->unk84.x < tbl[n].x) {
            scr->unk84.x = tbl[n].x;
        }
        if (tbl[n].z < scr->unk78.z) {
            scr->unk78.z = tbl[n].z;
        } else if (scr->unk84.z < tbl[n].z) {
            scr->unk84.z = tbl[n].z;
        }
        if (tbl[n].y > scr->unk84.y) {
            scr->unk84.y = tbl[n].y;
        }
    }

    rec = (PosRec *)func_8004678C();
    for (temp = 0; rec->type != 0; temp++, rec++) {
        if (rec->used != 0)
            continue;
        if (rec->y > scr->unk84.y)
            continue;
        if (rec->x < scr->unk78.x)
            continue;
        if (scr->unk84.x < rec->x)
            continue;
        if (rec->z < scr->unk78.z)
            continue;
        if (scr->unk84.z < rec->z)
            continue;

        scr->unk100[0].x = rec->x;
        scr->unk100[0].z = rec->z;
        for (temp2 = 0; temp2 < 2; temp2++) {
            if (temp2 == 0) {
                a = &tbl[idx * 4];
                b = &tbl[idx * 4 + 1];
                c = &tbl[idx * 4 + 2];
            } else {
                a = &tbl[idx * 4 + 1];
                b = &tbl[idx * 4 + 2];
                c = &tbl[idx * 4 + 3];
            }
            if (func_8002E6B0(
                    (s32 *)a, (s32 *)b, (s32 *)c, &scr->unk100[0].x) != 0) {
                if (rec->type != 2)
                    goto hit;
                if (flag != 0 && idx == 0) {
                    idx = 1;
                    temp2 = -1;
                    continue;
                }
                func_80044B30(
                    temp, func_8002FC80((VECTOR *)a, (VECTOR *)b, (VECTOR *)c));
                func_80033550(a);
                rec->used = 1;
                return 0;
            }
        }
        if (flag == 0 || idx == 0)
            continue;
    hit:
        scr->unk100[0].y = (scr->unk78.y + scr->unk84.y) / 2;
        return 1;
    }
    return 0;
}

/* 1 when the box unk78 (min) / unk84 (max) and the box unk90 (min) / unk9C
 * (max) overlap on all three axes. */
static inline s32 box_overlap(Unk1F8002B8Rec *scr) {
    return scr->unk78.x <= scr->unk9C.x && scr->unk84.x >= scr->unk90.x &&
           scr->unk78.y <= scr->unk9C.y && scr->unk84.y >= scr->unk90.y &&
           scr->unk78.z <= scr->unk9C.z && scr->unk84.z >= scr->unk90.z;
}

/* Both are defined further down this file. */
extern s32 func_8002DAD0(Unk1F8002B8Rec *obj);
extern s32 func_8002DE20(
    Unk1F8002B8Rec *obj, LeafPos *p0, LeafPos *p1, LeafPos *p2);

/* Blade contact test between the two D_80101EC8 records (called by
 * func_8002C61C; the result goes to D_800A3824). Returns -1 unless both
 * records' +0x3C are at least 4. Borrows the first 16 SPAD->unkA8 points as
 * two 2x2 grids per record (func_800290B8's layout). A record whose +0xE is
 * not 6 or 7, in +0x6A state 2, 0x1B, 0x28 or 0x26, whose grid func_800290B8
 * hits plays effects 1, 0x26 and 0x2D and sets +0x286 to 0x19 or 0xB; the hit
 * counts only when +0x40 lies within +0xA1..+0xA3 (+0xA2..+0xA4 for grid 1,
 * which is tested only with +0x8C set). Then each record-0 triangle
 * func_8002DAD0 accepts is tested against each record-1 triangle (bounding
 * boxes, then func_8002DE20), the rejected ones again with the roles swapped. A
 * hit returns (record-1 grid << 1) | record-0 grid, else -1; the points are
 * restored. */
s32 func_80029454(void) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    LeafPos *ws = SPAD->unkA8[0];
    LeafPos saved[16];
    s32 count[2];
    /* i, j, n, k: loop counters, each shared by several loops */
    s32 i;
    s32 j;
    s32 n;
    u32 mask;
    s32 k;
    s32 *p;
    /* rec: the record pointer of the first record loop, then of the
     * second (Ruling 11, ordinary-c-judge-decidable) */
    Unk80101EC8Record *rec;

    if (D_80101EC8[0].unk_3C < 4) {
        return -1;
    }
    if (D_80101EC8[1].unk_3C < 4) {
        return -1;
    }
    for (k = 0; k < 16; k++) {
        saved[k] = SPAD->unkA8[0][k];
    }

    for (i = 0; i < 2; i++) {
        rec = &D_80101EC8[i];
        if (rec->unk_0E == 6 || rec->unk_0E == 7) {
            continue;
        }
        if (!(rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 ||
              rec->unk_6A == 0x26)) {
            continue;
        }
        if (rec->unk_0E == 4 || rec->unk_0E == 5) {
            ws[0] = SPAD->unk00[i][0];
            ws[1] = SPAD->unk00[i][2];
            ws[2] = rec->unk_210[0];
            ws[3] = rec->unk_210[2];
            ws[4] = SPAD->unk00[i][0];
            ws[5] = SPAD->unk00[i][1];
            ws[6] = rec->unk_210[0];
            ws[7] = rec->unk_210[1];
        } else {
            ws[0] = SPAD->unk00[i][0];
            ws[1] = SPAD->unk00[i][1];
            ws[2] = rec->unk_210[0];
            ws[3] = rec->unk_210[1];
            if (rec->unk_8C != 0) {
                ws[4] = SPAD->unk48[i][0];
                ws[5] = SPAD->unk48[i][1];
                ws[6] = rec->unk_234[0];
                ws[7] = rec->unk_234[1];
            }
        }
        if (rec->unk_40 >= rec->unk_A1[0] && rec->unk_40 <= rec->unk_A3[0] &&
            func_800290B8(0, rec->unk_0E == 4 || rec->unk_0E == 5, ws) != 0) {
            func_80032854(i, 1, &scr->unk100[0].x, 0);
            func_80032854(i, 0x26, &scr->unk100[0].x, 0);
            func_80032854(i, 0x2D, &scr->unk100[0].x, 0);
            rec->unk_286 = rec->unk_8C != 0 ? 0x19 : 0xB;
            rec->unk_AD = 0;
        } else if (
            rec->unk_8C != 0 && rec->unk_40 >= rec->unk_A1[1] &&
            rec->unk_40 <= rec->unk_A3[1] && func_800290B8(1, 0, ws) != 0) {
            func_80032854(i, 1, &scr->unk100[0].x, 0);
            func_80032854(i, 0x26, &scr->unk100[0].x, 0);
            func_80032854(i, 0x2D, &scr->unk100[0].x, 0);
            rec->unk_286 = 0xB;
            rec->unk_AD = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        LeafPos *dst = &ws[i * 8];
        rec = &D_80101EC8[i];
        dst[0] = SPAD->unk00[i][0];
        dst[1] = SPAD->unk00[i][1];
        dst[2] = rec->unk_210[0];
        dst[3] = rec->unk_210[1];
        count[i] = 2;
        if (rec->unk_96 != 0 || rec->unk_92 == 0 || rec->unk_0C == 0x1F) {
            dst[0].y = 100000;
            dst[1].y = 100000;
            dst[2].y = 100000;
            dst[3].y = 100000;
        }
        if (rec->unk_8C != 0) {
            dst[4] = SPAD->unk48[i][0];
            dst[5] = SPAD->unk48[i][1];
            dst[6] = rec->unk_234[0];
            dst[7] = rec->unk_234[1];
            count[i] += 2;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            p = (s32 *)((u8 *)ws + (i * 0x60 + j * 0x18));
            p[0] >>= 1;
            p[1] >>= 1;
            p[2] >>= 1;
            p[3] >>= 1;
            p[4] >>= 1;
            p[5] >>= 1;
        }
    }

    mask = 0;
    for (j = 0; j < count[0]; j++) {
        switch (j) {
        case 0:
            scr->unk60[0] = &ws[2];
            scr->unk60[1] = &ws[3];
            scr->unk60[2] = &ws[1];
            break;
        case 1:
            scr->unk60[0] = &ws[0];
            scr->unk60[1] = &ws[1];
            scr->unk60[2] = &ws[2];
            break;
        case 2:
            scr->unk60[0] = &ws[6];
            scr->unk60[1] = &ws[7];
            scr->unk60[2] = &ws[5];
            break;
        case 3:
            scr->unk60[0] = &ws[4];
            scr->unk60[1] = &ws[5];
            scr->unk60[2] = &ws[6];
            break;
        }
        if (func_8002DAD0(scr) == 0) {
            mask |= 1 << j;
            continue;
        }
        scr->unk84 = *scr->unk60[0];
        scr->unk78 = scr->unk84;
        for (k = 1; k < 3; k++) {
            if (scr->unk60[k]->x < scr->unk78.x) {
                scr->unk78.x = scr->unk60[k]->x;
            } else if (scr->unk84.x < scr->unk60[k]->x) {
                scr->unk84.x = scr->unk60[k]->x;
            }
            if (scr->unk60[k]->y < scr->unk78.y) {
                scr->unk78.y = scr->unk60[k]->y;
            } else if (scr->unk84.y < scr->unk60[k]->y) {
                scr->unk84.y = scr->unk60[k]->y;
            }
            if (scr->unk60[k]->z < scr->unk78.z) {
                scr->unk78.z = scr->unk60[k]->z;
            } else if (scr->unk84.z < scr->unk60[k]->z) {
                scr->unk84.z = scr->unk60[k]->z;
            }
        }
        for (n = 0; n < count[1]; n++) {
            switch (n) {
            case 0:
                scr->unk6C[0] = &ws[10];
                scr->unk6C[1] = &ws[11];
                scr->unk6C[2] = &ws[9];
                break;
            case 1:
                scr->unk6C[0] = &ws[8];
                scr->unk6C[1] = &ws[9];
                scr->unk6C[2] = &ws[10];
                break;
            case 2:
                scr->unk6C[0] = &ws[14];
                scr->unk6C[1] = &ws[15];
                scr->unk6C[2] = &ws[13];
                break;
            case 3:
                scr->unk6C[0] = &ws[12];
                scr->unk6C[1] = &ws[13];
                scr->unk6C[2] = &ws[14];
                break;
            }
            scr->unk9C = *scr->unk6C[0];
            scr->unk90 = scr->unk9C;
            for (k = 1; k < 3; k++) {
                if (scr->unk6C[k]->x < scr->unk90.x) {
                    scr->unk90.x = scr->unk6C[k]->x;
                } else if (scr->unk9C.x < scr->unk6C[k]->x) {
                    scr->unk9C.x = scr->unk6C[k]->x;
                }
                if (scr->unk6C[k]->y < scr->unk90.y) {
                    scr->unk90.y = scr->unk6C[k]->y;
                } else if (scr->unk9C.y < scr->unk6C[k]->y) {
                    scr->unk9C.y = scr->unk6C[k]->y;
                }
                if (scr->unk6C[k]->z < scr->unk90.z) {
                    scr->unk90.z = scr->unk6C[k]->z;
                } else if (scr->unk9C.z < scr->unk6C[k]->z) {
                    scr->unk9C.z = scr->unk6C[k]->z;
                }
            }
            if (box_overlap(scr) &&
                func_8002DE20(
                    scr, scr->unk6C[0], scr->unk6C[1], scr->unk6C[2]) != 0) {
                for (k = 0; k < 16; k++) {
                    SPAD->unkA8[0][k] = saved[k];
                }
                return ((n >> 1) * 2) | (j >> 1);
            }
        }
    }
    if (mask == 0) {
        for (k = 0; k < 16; k++) {
            SPAD->unkA8[0][k] = saved[k];
        }
        return -1;
    }

    for (j = 0; j < count[1]; j++) {
        switch (j) {
        case 0:
            scr->unk60[0] = &ws[10];
            scr->unk60[1] = &ws[11];
            scr->unk60[2] = &ws[9];
            break;
        case 1:
            scr->unk60[0] = &ws[8];
            scr->unk60[1] = &ws[9];
            scr->unk60[2] = &ws[10];
            break;
        case 2:
            scr->unk60[0] = &ws[14];
            scr->unk60[1] = &ws[15];
            scr->unk60[2] = &ws[13];
            break;
        case 3:
            scr->unk60[0] = &ws[12];
            scr->unk60[1] = &ws[13];
            scr->unk60[2] = &ws[14];
            break;
        }
        if (func_8002DAD0(scr) == 0) {
            continue;
        }
        scr->unk84 = *scr->unk60[0];
        scr->unk78 = scr->unk84;
        for (k = 1; k < 3; k++) {
            if (scr->unk60[k]->x < scr->unk78.x) {
                scr->unk78.x = scr->unk60[k]->x;
            } else if (scr->unk84.x < scr->unk60[k]->x) {
                scr->unk84.x = scr->unk60[k]->x;
            }
            if (scr->unk60[k]->y < scr->unk78.y) {
                scr->unk78.y = scr->unk60[k]->y;
            } else if (scr->unk84.y < scr->unk60[k]->y) {
                scr->unk84.y = scr->unk60[k]->y;
            }
            if (scr->unk60[k]->z < scr->unk78.z) {
                scr->unk78.z = scr->unk60[k]->z;
            } else if (scr->unk84.z < scr->unk60[k]->z) {
                scr->unk84.z = scr->unk60[k]->z;
            }
        }
        for (n = 0; n < count[0]; n++) {
            if (!(mask & (1 << n))) {
                continue;
            }
            switch (n) {
            case 0:
                scr->unk6C[0] = &ws[2];
                scr->unk6C[1] = &ws[3];
                scr->unk6C[2] = &ws[1];
                break;
            case 1:
                scr->unk6C[0] = &ws[0];
                scr->unk6C[1] = &ws[1];
                scr->unk6C[2] = &ws[2];
                break;
            case 2:
                scr->unk6C[0] = &ws[6];
                scr->unk6C[1] = &ws[7];
                scr->unk6C[2] = &ws[5];
                break;
            case 3:
                scr->unk6C[0] = &ws[4];
                scr->unk6C[1] = &ws[5];
                scr->unk6C[2] = &ws[6];
                break;
            }
            scr->unk9C = *scr->unk6C[0];
            scr->unk90 = scr->unk9C;
            for (k = 1; k < 3; k++) {
                if (scr->unk6C[k]->x < scr->unk90.x) {
                    scr->unk90.x = scr->unk6C[k]->x;
                } else if (scr->unk9C.x < scr->unk6C[k]->x) {
                    scr->unk9C.x = scr->unk6C[k]->x;
                }
                if (scr->unk6C[k]->y < scr->unk90.y) {
                    scr->unk90.y = scr->unk6C[k]->y;
                } else if (scr->unk9C.y < scr->unk6C[k]->y) {
                    scr->unk9C.y = scr->unk6C[k]->y;
                }
                if (scr->unk6C[k]->z < scr->unk90.z) {
                    scr->unk90.z = scr->unk6C[k]->z;
                } else if (scr->unk9C.z < scr->unk6C[k]->z) {
                    scr->unk9C.z = scr->unk6C[k]->z;
                }
            }
            if (box_overlap(scr) &&
                func_8002DE20(
                    scr, scr->unk6C[0], scr->unk6C[1], scr->unk6C[2]) != 0) {
                for (k = 0; k < 16; k++) {
                    SPAD->unkA8[0][k] = saved[k];
                }
                return ((j >> 1) * 2) | (n >> 1);
            }
        }
    }
    for (k = 0; k < 16; k++) {
        SPAD->unkA8[0][k] = saved[k];
    }
    return -1;
}

/* Sweeps a character's weapon segment (base -> tip, in the scratch record at
 * 0x1F8002B8) against its limb spheres. Takes the segment's pitch and yaw (a
 * negative square prints "ILLEGAL GUN MOTION" and returns) and unit direction
 * q, and pushes the tip out by q*4. If the yaw difference to the partner is
 * under 0x400, each of character `id`'s 22 hit records (D_800F5F68; records
 * 6..9 only when obj+0x26C is set) is tested: a hit sets bit i in *hit and,
 * only when the record's first halfword is nonzero, a second (deep) test sets
 * bit i in *deep. Then func_80053614 casts the segment against the stage; on
 * a stage hit (func_80054434() != 7), unless *hit is set and the stage point
 * is no nearer the base than obj+0xF4, the limb masks are cleared (when *hit
 * was set) and, unless `quiet`, effect 0xA plays at the hit point.
 * D_800A37E8.. receive -q.
 * GTE: gte_Lzc (gte_ldlzc, gte_nop x2, gte_stlzc) per square root, inline_o.h
 * class (inline-asm-policy, owner ruling 2026-09-26). Everything else is
 * ordinary C, except the five Ruling 11 locals (dx, dy, dz, temp, temp2) and
 * the do-while(0), each annotated below. */
extern char D_80010478[];
extern void printf();
extern void func_8002E838(Unk1F8002B8Rec *scr);
extern s32 func_8002EA24(
    Unk1F8002B8Rec *scr, LeafPos *pos, s32 threshold, s32 r_sq);
extern s32 func_80054434(void);

void func_8002A458(Unk80101EC8Record *obj, u32 *hit, u32 *deep, s32 quiet) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    s32 id = obj->index;
    Unk80101EC8Record *partner = obj->other;
    s32 work[64];
    s32 sp_tmp;
    s32 sp_tmp2;
    /* dx/dy/dz: a point's offset from the segment base -- the segment itself,
     * then the stage hit point and obj+0xF4 (Ruling 11) */
    s32 dx;
    s32 dy;
    s32 dz;
    s32 diff;
    /* temp: the squared horizontal length, then the segment length
     * (Ruling 11) */
    s32 temp;
    s32 hlen;
    /* temp2: the site-1 gte_ldlzc input copy of `temp`, then the
     * g_sqrt_table_u8 byte (Ruling 11, Q28) */
    s32 temp2;
    s32 len_sq;
    s32 hit_sq;
    s32 qx;
    s32 qy;
    s32 qz;
    LeafPos *p;
    BoneHitRec *rec;
    s32 i;

    scr->unk60[0] = &scr->unk00.unk00[0];
    scr->unk60[1] = &scr->unk00.unk00[1];
    scr->unkC8 = scr->unk00.unk00[1];
    dx = scr->unk60[1]->x - scr->unk60[0]->x;
    dy = scr->unk60[1]->y - scr->unk60[0]->y;
    dz = scr->unk60[1]->z - scr->unk60[0]->z;
    diff = (partner->unk_1D8 - ratan2(dx, dz)) & 0xFFF;
    if (diff >= 0x800) {
        diff = 0x1000 - diff;
    }
    temp = dx * dx + dz * dz - dy * dy;
    if (temp < 0) {
        printf(D_80010478, temp);
        return;
    }
    temp2 = temp;
    if ((u32)temp < 0x400) {
        hlen = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        /* gte_Lzc(temp2, &sp_tmp) -- gtemac.h :174-178: gte_ldlzc, gte_nop x2,
         * gte_stlzc */
        __asm__ volatile ("move  $12,%0": :"r"(temp2):"$12","$13","$14","$15","memory");
        __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
        {
            s32 lz = ~1;
            s32 shift;
            lz &= sp_tmp;
            shift = 0x16 - lz;
            temp2 = g_sqrt_table_u8[(u32)temp >> shift];
            hlen = (u32)(temp2 << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    scr->unkF8.vx = -ratan2(dy, hlen);
    scr->unkF8.vy = ratan2(dx, dz);
    scr->unkF8.vz = 0;
    if (quiet == 0) {
        func_80032854(id == 0, 0xB, &scr->unkC8.x, &scr->unkF8.vx);
    }
    len_sq = dx * dx + dy * dy + dz * dz;
    if ((u32)len_sq < 0x400) {
        temp = (u32)g_sqrt_table_u8[len_sq] >> 3;
    } else {
        /* gte_Lzc(len_sq, &sp_tmp2) -- gtemac.h :174-178: gte_ldlzc, gte_nop
         * x2, gte_stlzc */
        __asm__ volatile ("move  $12,%0": :"r"(len_sq):"$12","$13","$14","$15","memory");
        __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp2):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
        {
            s32 lz = ~1;
            s32 shift;
            s32 tbl;
            lz &= sp_tmp2;
            shift = 0x16 - lz;
            tbl = g_sqrt_table_u8[(u32)len_sq >> shift];
            temp = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    qx = (dx << 12) / temp;
    qy = (dy << 12) / temp;
    qz = (dz << 12) / temp;
    *scr->unk60[0] = *scr->unk60[1];
    scr->unk60[1]->x += qx * 4;
    scr->unk60[1]->y += qy * 4;
    scr->unk60[1]->z += qz * 4;
    if (diff < 0x400) {
        func_8002E838(scr);
        /* FAKE: do-while(0) puts id in $s7 / obj in $fp as in the target;
         * unwrapped they swap (16 instructions differ)
         * (do-while-zero-exception) */
        do {
            rec = D_800F5F68[id];
        } while (0);
        for (i = 0; i < 22; i++, rec++) {
            LeafPos *pos;
            if (obj->unk_26C == 0 && i >= 6 && i <= 9) {
                continue;
            }
            pos = &SPAD->unkA8[id][i];
            if (func_8002EA24(scr, pos, rec->unk_0C, rec->unk_0E) != 0) {
                s32 bit = 1 << i;
                *hit |= bit;
                if (rec->unk_00 != 0 &&
                    func_8002EA24(scr, pos, rec->unk_10, rec->unk_12) != 0) {
                    *deep |= bit;
                }
            }
        }
    }
    scr->unkA8.x = scr->unk60[0]->x - qx / 4;
    scr->unkA8.y = scr->unk60[0]->y - qy / 4;
    scr->unkA8.z = scr->unk60[0]->z - qz / 4;
    if (func_80053614(&scr->unkA8.x, &scr->unk60[1]->x, &scr->unk100[0].x,
                      &scr->unkF8.vx, (s32)work) != 0 &&
        func_80054434() != 7) {
        if (*hit != 0) {
            p = scr->unk60[0];
            dx = scr->unk100[0].x - p->x;
            dy = scr->unk100[0].y - p->y;
            dz = scr->unk100[0].z - p->z;
            hit_sq = dx * dx + dy * dy + dz * dz;
            dx = obj->unk_F4.x - p->x;
            dy = obj->unk_F4.y - p->y;
            dz = obj->unk_F4.z - p->z;
            if (hit_sq >= dx * dx + dy * dy + dz * dz) {
                goto done;
            }
            *deep = 0;
            *hit = 0;
        }
        if (quiet == 0) {
            func_80032854(id == 0, 0xA, &scr->unk100[0].x, 0);
        }
    }
done:
    D_800A37E8 = -qx;
    D_800A37EA = -qy;
    D_800A37EC = -qz;
}

extern void func_8002CA8C(Unk80101EC8Record *a0, s32 a1, s32 a2);
extern s32 func_8002CD58(Unk1F8002B8Rec *obj);

void func_8002AB08(s32 mode) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    /* FAKE: second handle to D_800A37E8, so the address is rematerialized at
     * each of the three calls as in the target; spelled directly, cse keeps it
     * in a callee-saved register: score 6 (pointer-alias-fake-exception;
     * vector: Q96) */
    s16 *vec = &D_800A37E8;
    s32 i;

    for (i = 0; i < 2; i++) {
        Unk80101EC8Record *other;
        Unk80101EC8Record *self;
        u32 hit;
        u32 deep;
        u32 mask_a;
        u32 mask_b;
        u32 mask_c;
        s32 npass;
        s32 pass;
        /* alt: the pass's blade (0: the unk00 / unk_210 points, 1: the
         * unk48 / unk_234 points; the knockback reads the last pass's), then
         * whether the nearest hit came from that blade. The 4/5 arm's `alt =
         * 0;` re-stores the held 0 (Q85; Ruling 11, reused-local-necessity) */
        s32 alt;
        /* temp1 / temp2: the indices of the pass's two points (per-branch
         * constants, Q20), then the opponent's squared distance to the blade's
         * first / second point. The unk_8C arm re-stores the held values (Q85,
         * Ruling 11) */
        s32 temp1;
        s32 temp2;
        s32 nseg;
        /* idx: the triangle index of the segment loop, then the nearest
         * hit's index, the Q34 plain copy `idx = temp3;` (Ruling 11) */
        s32 idx;
        /* dx / dy / dz: the offset between two points, one value per use --
         * the segment's length vector, a hit point from the reference point,
         * the blade's points from the opponent, the blade direction, the scaled
         * push (dy: the first two only) (Ruling 11) */
        s32 dx;
        s32 dy;
        s32 dz;
        s32 best;
        /* temp3: the hit-slot counter of the nearest-hit search, then
         * whether the nearest hit came from the alternate blade (Ruling 11) */
        s32 temp3;
        s32 j;
        BoneHitRec *rec;
        s32 near;
        s32 guard;
        s32 bit;
        s32 diff;
        s32 ang;
        /* work: the segment length squared, a hit's distance past its
         * radius, the facing difference to the opponent's velocity, then the
         * push weight (Ruling 11) */
        s32 work;
        s32 flag;
        s32 strong;

        if (mode == 1 && D_800A38AE == i) {
            continue;
        }
        self = &D_80101EC8[i];
        other = D_80101EC8;
        if (i == 0) {
            other++;
        }
        mask_a = 0;
        mask_b = 0;
        mask_c = 0;
        npass = 0;
        hit = 0;
        deep = 0;
        if (other->unk_96 == 0 && other->unk_92 != 0 && other->unk_3C != 0) {
            npass = other->unk_0C != 0x1F;
        }
        if (other->unk_8C != 0) {
            npass++;
        }
        if (npass != 0 && (other->unk_0E == 4 || other->unk_0E == 5)) {
            npass++;
        }
        if ((other->unk_0E == 6 || other->unk_0E == 7) &&
            (other->unk_6A == 2 || other->unk_6A == 0x1B ||
             other->unk_6A == 0x28 || other->unk_6A == 0x26) &&
            other->unk_40 == other->unk_A1[0]) {
            if (other->unk_34A != 0) {
                other->unk_34A -= 1;
                scr->unk00.unk00[0] = SPAD->unk00[i == 0][1];
                scr->unk00.unk00[1] = SPAD->unk00[i == 0][0];
                func_8002A458(self, &hit, &deep, 0);
                mask_a |= hit;
            } else {
                func_80032854(
                    i == 0, 0x32,
                    i == 0 ? &SPAD->unk00[1][1].x : &SPAD->unk00[0][1].x, 0);
                npass = 0;
            }
        }
        if ((other->unk_0C == 0x1D || other->unk_0C == 0xE) &&
            other->unk_26C != 0 &&
            (other->unk_6A == 2 || other->unk_6A == 0x1B ||
             other->unk_6A == 0x28 || other->unk_6A == 0x26) &&
            other->unk_40 == other->unk_A1[1] && other->unk_34A != 0) {
            other->unk_34A -= 1;
            scr->unk00.unk00[0] = SPAD->unk48[i == 0][1];
            scr->unk00.unk00[1] = SPAD->unk48[i == 0][0];
            func_8002A458(self, &hit, &deep, other->unk_0C == 0xE);
            func_80032854(
                i == 0, 0x2A,
                i == 0 ? &SPAD->unk00[1][1].x : &SPAD->unk00[0][1].x, 0);
            mask_a |= hit;
        }
        for (pass = 0; pass < npass; pass++) {
            /* deep_on: func_8002CA8C's third argument (enables the deep-hit
             * test), per-branch constants 1 / 1 / 0 (Q20); the unk_8C arm's
             * store is shared with pass 0 (Q85, Ruling 11) */
            s32 deep_on;

            if (pass == 0) {
                alt = 0;
                temp1 = 0;
                temp2 = 1;
                deep_on = 1;
            } else if (other->unk_8C != 0) {
                alt = 1;
                temp1 = 0;
                temp2 = 1;
                deep_on = 1;
            } else if (other->unk_0E == 4 || other->unk_0E == 5) {
                alt = 0;
                temp1 = 1;
                temp2 = 2;
                deep_on = 0;
            }
            if (alt == 0) {
                scr->unk00.unk00[0] = SPAD->unk00[i == 0][temp1];
                scr->unk00.unk00[1] = SPAD->unk00[i == 0][temp2];
                scr->unk00.unk00[2] = other->unk_210[temp1];
                scr->unk00.unk00[3] = other->unk_210[temp2];
            } else {
                scr->unk00.unk00[0] = SPAD->unk48[i == 0][temp1];
                scr->unk00.unk00[1] = SPAD->unk48[i == 0][temp2];
                scr->unk00.unk00[2] = other->unk_234[temp1];
                scr->unk00.unk00[3] = other->unk_234[temp2];
            }
            scr->unk00.unk00[4].x =
                (scr->unk00.unk00[0].x + scr->unk00.unk00[2].x) / 2;
            scr->unk00.unk00[4].y =
                (scr->unk00.unk00[0].y + scr->unk00.unk00[2].y) / 2;
            scr->unk00.unk00[4].z =
                (scr->unk00.unk00[0].z + scr->unk00.unk00[2].z) / 2;
            scr->unk00.unk00[5].x =
                (scr->unk00.unk00[1].x + scr->unk00.unk00[3].x) / 2;
            scr->unk00.unk00[5].y =
                (scr->unk00.unk00[1].y + scr->unk00.unk00[3].y) / 2;
            scr->unk00.unk00[5].z =
                (scr->unk00.unk00[1].z + scr->unk00.unk00[3].z) / 2;
            dx = scr->unk00.unk00[1].x - scr->unk00.unk00[3].x;
            dy = scr->unk00.unk00[1].y - scr->unk00.unk00[3].y;
            dz = scr->unk00.unk00[1].z - scr->unk00.unk00[3].z;
            work = dx * dx + dy * dy + dz * dz;
            nseg = 2;
            if (work > 6249999) {
                nseg = 4;
            }
            for (idx = 0; idx < nseg; idx++) {
                if (nseg == 2) {
                    if (idx == 0) {
                        scr->unk60[0] = &scr->unk00.unk00[2];
                        scr->unk60[1] = &scr->unk00.unk00[3];
                        scr->unk60[2] = &scr->unk00.unk00[1];
                    } else {
                        scr->unk60[0] = &scr->unk00.unk00[0];
                        scr->unk60[1] = &scr->unk00.unk00[1];
                        scr->unk60[2] = &scr->unk00.unk00[2];
                    }
                } else {
                    switch (idx) {
                    case 0:
                        scr->unk60[0] = &scr->unk00.unk00[3];
                        scr->unk60[1] = &scr->unk00.unk00[2];
                        scr->unk60[2] = &scr->unk00.unk00[5];
                        break;
                    case 1:
                        scr->unk60[0] = &scr->unk00.unk00[5];
                        scr->unk60[1] = &scr->unk00.unk00[4];
                        scr->unk60[2] = &scr->unk00.unk00[2];
                        break;
                    case 2:
                        scr->unk60[0] = &scr->unk00.unk00[5];
                        scr->unk60[1] = &scr->unk00.unk00[4];
                        scr->unk60[2] = &scr->unk00.unk00[1];
                        break;
                    case 3:
                        scr->unk60[0] = &scr->unk00.unk00[1];
                        scr->unk60[1] = &scr->unk00.unk00[0];
                        scr->unk60[2] = &scr->unk00.unk00[4];
                        break;
                    }
                }
                scr->unk84 = *scr->unk60[0];
                scr->unk78 = scr->unk84;
                for (j = 1; j < 3; j++) {
                    if (scr->unk60[j]->x < scr->unk78.x) {
                        scr->unk78.x = scr->unk60[j]->x;
                    } else if (scr->unk84.x < scr->unk60[j]->x) {
                        scr->unk84.x = scr->unk60[j]->x;
                    }
                    if (scr->unk60[j]->y < scr->unk78.y) {
                        scr->unk78.y = scr->unk60[j]->y;
                    } else if (scr->unk84.y < scr->unk60[j]->y) {
                        scr->unk84.y = scr->unk60[j]->y;
                    }
                    if (scr->unk60[j]->z < scr->unk78.z) {
                        scr->unk78.z = scr->unk60[j]->z;
                    } else if (scr->unk84.z < scr->unk60[j]->z) {
                        scr->unk84.z = scr->unk60[j]->z;
                    }
                }
                func_8002CA8C(self, func_8002CD58(scr), deep_on);
                hit |= scr->unkB4;
                deep |= scr->unkC4;
                if (alt != 0) {
                    mask_a |= scr->unkB4;
                }
                if (pass == 1 && alt == 0) {
                    mask_b |= scr->unkB4;
                } else {
                    mask_c |= scr->unkB4;
                }
            }
        }
        if (hit == 0) {
            continue;
        }
        rec = D_800F5F68[i];
        best = 0x7FFFFFFF;
        scr->unkC8.x = other->unk_210[1].x;
        scr->unkC8.y = other->unk_210[1].y;
        scr->unkC8.z = other->unk_210[1].z;
        for (temp3 = 0; temp3 < 22; temp3++, rec++) {
            if (hit & (1 << temp3)) {
                dx = SPAD->unkA8[i][temp3].x - scr->unkC8.x;
                dy = SPAD->unkA8[i][temp3].y - scr->unkC8.y;
                dz = SPAD->unkA8[i][temp3].z - scr->unkC8.z;
                work = dx * dx + dy * dy + dz * dz - rec->unk_0E;
                if (work < best) {
                    best = work;
                    idx = temp3;
                }
            }
        }
        if (mode == 1) {
            func_800274BC(&other->unk_114[0].vx, vec);
            func_80032854(i, 4, &SPAD->unkA8[i][idx].x, vec);
            return;
        }
        near = 0;
        if ((other->unk_40 >= other->unk_A1[0] - 3 &&
             other->unk_40 < other->unk_A1[0]) ||
            (other->unk_40 >= other->unk_A1[1] - 3 &&
             other->unk_40 < other->unk_A1[1])) {
            near = 1;
        }
        guard = 0;
        if ((other->unk_0E == 6 || other->unk_0E == 7 ||
             other->unk_0C == 0x1D || other->unk_0C == 0xE) &&
            (mask_a & (1 << idx))) {
            guard = 1;
        }
        if (guard == 0 && (!(other->unk_6A == 2 || other->unk_6A == 0x1B ||
                             other->unk_6A == 0x28 || other->unk_6A == 0x26) ||
                           near == 0)) {
            bit = 1 << idx;
            temp3 = (mask_a & bit) != 0;
            if (temp3) {
                dx = SPAD->unk48[i == 0][0].x - other->unk_F4.x;
                dz = SPAD->unk48[i == 0][0].z - other->unk_F4.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - other->unk_F4.x;
                dz = SPAD->unk48[i == 0][1].z - other->unk_F4.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - SPAD->unk48[i == 0][0].x;
                dz = SPAD->unk48[i == 0][1].z - SPAD->unk48[i == 0][0].z;
            } else if (mask_b & bit) {
                dx = SPAD->unk00[i == 0][2].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][2].z - other->unk_F4.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][1].z - other->unk_F4.z;
                temp2 = dx * dx + dz * dz;
                dx = (SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][2].x) / 4;
                dz = (SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][2].z) / 4;
            } else {
                dx = SPAD->unk00[i == 0][0].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][0].z - other->unk_F4.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][1].z - other->unk_F4.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][0].x;
                dz = SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][0].z;
            }
            if (temp2 < temp1) {
                dx = -dx;
                dz = -dz;
            }
            diff = (other->unk_1D8 - ratan2(dx, dz)) & 0xFFF;
            if (diff >= 0x800) {
                diff = 0x1000 - diff;
            }
            if (diff < 0x400) {
                work = (ratan2(other->unk_114[alt].vx, other->unk_114[alt].vz) +
                        0x800 - self->unk_1D8) &
                       0xFFF;
                if (work >= 0x800) {
                    work = 0x1000 - work;
                }
                work = 0x400 - work;
                if (work < 0) {
                    work = 0;
                }
                dx = dx / 4 + (other->unk_114[temp3].vx * work / 2 >> 11);
                dz = dz / 4 + (other->unk_114[temp3].vz * work / 2 >> 11);
                if (other->unk_6A == 3 || other->unk_6A == 7 ||
                    other->unk_6A == 0xD || other->unk_6A == 0x2C) {
                    self->unk_134.vx += dx / 2;
                    self->unk_134.vz += dz / 2;
                } else {
                    self->unk_134.vx += dx;
                    self->unk_134.vz += dz;
                }
                if ((self->unk_6A == 0x13 || self->unk_6A == 0x1B ||
                     self->unk_6A == 0x30) &&
                    other->unk_6A == 0x15) {
                    if (self->unk_0E == 6 || self->unk_0E == 7 ||
                        other->unk_0E == 6 || other->unk_0E == 7) {
                        other->unk_286 = 5;
                        func_80032854(i == 0, 0x21,
                                      i == 0 ? &D_80101EC8[1].unk_F4.x
                                             : &D_80101EC8[0].unk_F4.x,
                                      0);
                        func_80032854(i == 0, 0x2D,
                                      i == 0 ? &D_80101EC8[1].unk_F4.x
                                             : &D_80101EC8[0].unk_F4.x,
                                      0);
                    } else {
                        D_800A38A8 = 1;
                        D_800A3876 = i;
                    }
                }
            }
        }
        ang = (self->unk_1C8.vy - other->unk_1C8.vy) & 0xFFF;
        if (ang >= 0x800) {
            ang = 0x1000 - ang;
        }
        alt = (mask_a & (1 << idx)) != 0;
        if (other->unk_AE == 0) {
            continue;
        }
        if (other->unk_8C != 0 && (other->unk_40 < other->unk_A1[alt] ||
                                   other->unk_40 > other->unk_A3[alt])) {
            continue;
        }
        flag = 0;
        if (!(other->unk_0E == 6 || other->unk_0E == 7)) {
            if ((other->unk_0C == 0x1D || other->unk_0C == 0xE) && alt != 0) {
                flag = 1;
            } else {
                func_800274BC(&other->unk_114[alt].vx, vec);
            }
        }
        if (D_800A3140 == 0) {
            strong = deep & (1 << idx);
        } else {
            strong = 1;
        }
        if (mask_b & (1 << idx) & ~mask_c) {
            strong = 0;
        }
        /* func_80027AD8's sixth argument is a record pointer on its pass-1
         * calls (func_80031B24) and, on this pass-0 call, the 0/1
         * alternate-blade flag the target passes in that slot: its `tbl == NULL
         * ? 0xB : 0x19` picks the same reaction func_80029454 picks from +0x8C.
         */
        func_80027AD8(0, self, idx, ang, strong, (Tbl8008E194 *)alt, flag, 0);
        other->unk_AD = 0;
    }
}

s32 func_8002BC68(s32 arg0) {
    s32 temp_a3;
    s32 temp_t1;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    /* FAKE: pointer alias to D_80101EC8[0] / [1]: holds the table base in a
     * register as the target does; without it every access is an absolute
     * symbol+offset address (mips.h:2300 GO_IF_LEGITIMATE_ADDRESS accepts it
     * as is), one instruction more: score 14 (pointer-alias-fake-exception) */
    Unk80101EC8Record *t2_base;
    Unk80101EC8Record *t3_base;

    t2_base = D_80101EC8;
    t3_base = t2_base + 1;
    temp_a3 = t2_base->unk_D8.x - t3_base->unk_D8.x;
    temp_t1 = t2_base->unk_D8.z - t3_base->unk_D8.z;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32)(g_sqrt_table_u8[temp_a0])) >> 3;
    } else {
        s32 sp_tmp;
        /* GTE leading-zero-count island (no C form). The $13-$15 clobbers
         * reproduce the original's register footprint (not a pin): only with
         * them does reload pick the target's $24 for the mfhi. */
        __asm__ volatile(
            "addu   $t4, %1, $zero\n"
            "mtc2   $t4, $30\n"
            "nop\n"
            "nop\n"
            "addu   $t4, $sp, $zero\n"
            "swc2   $31, 0($t4)\n"
            : "=m"(sp_tmp)
            : "r"(temp_a0)
            : "$12", "$13", "$14", "$15");
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = temp_a0 >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(g_sqrt_table_u8[idx]));
            var_t0 = (hi << 16) >> (0x13 - v1_m);
        }
    }
    if (((s32)var_t0) < arg0) {
        var_a0 = ((arg0 - ((s32)var_t0)) * 0x50) / 100;
    } else {
        var_a0 = (arg0 - ((s32)var_t0)) / 16;
    }
    {
        s32 temp_v0 = arg0 - 0x64;
        s32 temp_v1_3 = -var_a0;
        t2_base->unk_134.vx = (temp_a3 * var_a0) / temp_v0;
        t2_base->unk_134.vz = (temp_t1 * var_a0) / temp_v0;
        t3_base->unk_134.vx = (temp_a3 * temp_v1_3) / temp_v0;
        t3_base->unk_134.vz = (temp_t1 * temp_v1_3) / temp_v0;
    }
    return (s32)var_t0;
}

s32 func_8002BEA0(void) {
    s32 temp_a3;
    s32 temp_t1;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    /* FAKE: pointer alias to D_80101EC8[0] / [1]: holds the table base in a
     * register as the target does; without it every access is an absolute
     * symbol+offset address (mips.h:2300 GO_IF_LEGITIMATE_ADDRESS accepts it
     * as is), one instruction more: score 14 (pointer-alias-fake-exception) */
    Unk80101EC8Record *t2_base;
    Unk80101EC8Record *t3_base;

    t2_base = D_80101EC8;
    t3_base = t2_base + 1;
    temp_a3 = t2_base->unk_F4.x - t3_base->unk_F4.x;
    temp_t1 = t2_base->unk_F4.z - t3_base->unk_F4.z;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32)(g_sqrt_table_u8[temp_a0])) >> 3;
    } else {
        s32 sp_tmp;
        /* GTE leading-zero-count island (no C form). The $13-$15 clobbers
         * reproduce the original's register footprint (not a pin): only with
         * them does reload pick the target's $24 for the mfhi. */
        __asm__ volatile(
            "addu   $t4, %1, $zero\n"
            "mtc2   $t4, $30\n"
            "nop\n"
            "nop\n"
            "addu   $t4, $sp, $zero\n"
            "swc2   $31, 0($t4)\n"
            : "=m"(sp_tmp)
            : "r"(temp_a0)
            : "$12", "$13", "$14", "$15");
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = temp_a0 >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(g_sqrt_table_u8[idx]));
            var_t0 = (hi << 16) >> (0x13 - v1_m);
        }
    }
    if (((s32)var_t0) < 0x44C) {
        var_a0 = ((0x44C - ((s32)var_t0)) * 0x50) / 100;
    } else {
        var_a0 = (0x44C - ((s32)var_t0)) / 16;
    }
    {
        s32 temp_v0 = 0x3E8;
        s32 temp_v1_3 = -var_a0;
        t2_base->unk_134.vx = (temp_a3 * var_a0) / temp_v0;
        t2_base->unk_134.vz = (temp_t1 * var_a0) / temp_v0;
        t3_base->unk_134.vx = (temp_a3 * temp_v1_3) / temp_v0;
        t3_base->unk_134.vz = (temp_t1 * temp_v1_3) / temp_v0;
    }
    return (s32)var_t0 - 0x44C;
}

void func_8002C0DC(void) {
    s32 i;
    Unk80101EC8Record *var_s0;
    s32 temp_s2;

    temp_s2 = func_8002BC68(D_800A371C);

    for (i = 0; i < 2; i++) {
        Unk80101EC8Record *e = &D_80101EC8[i];
        Unk80101EC8Record *ptr = e->other;
        s32 arg1 = ptr->unk_D8.x - e->unk_D8.x;
        s32 arg2 = ptr->unk_D8.z - e->unk_D8.z;
        func_8001F860(e, ratan2(arg1, arg2));
    }

    {
        s32 idx;
        s32 chk;
        idx = D_800A38AE;
        chk = D_800A376E;
        var_s0 = &D_80101EC8[idx];

        if (chk == 0) {
            if (D_800A3758 == 0xFF) {
                if (var_s0->unk_AA == var_s0->unk_40) {
                    func_8002AB08(1);
                }
            }
        }

        {
            s32 v1;
            v1 = var_s0->unk_40;
            if (v1 < (s32)D_800A38E8) {
                return;
            }
            if (v1 >= var_s0->unk_AA) {
                return;
            }
            if (D_800A371C + 0xC8 >= temp_s2) {
                return;
            }
            {
                Unk80101EC8Record *v1ptr;
                v1ptr = var_s0->other;
                var_s0->unk_286 = 4;
                v1ptr->unk_286 = 5;
            }
        }
    }
}

/* Accumulates a character's shadow vectors in the scratchpad record at
 * 0x1F8002B8 (`scr`): unkA8 sums two of the character's SPAD points (unk48[k]
 * when bit k of D_800A3824 is set, else unk00[k]); unkB8 sums the matching pair
 * func_8002C61C copied into D_80101EC8[k] (unk_234 / unk_210); the result goes
 * to unk13C. */
void func_8002C22C(void) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    /* FAKE: pointer alias to D_80101EC8[1]: the target holds record 1's base in
     * a register; without it every access is an absolute address
     * (GO_IF_LEGITIMATE_ADDRESS, gcc-2.7.2/config/mips/mips.h:2286) and the
     * direct D_80101EC8[1] form costs ten instructions (score 26)
     * (pointer-alias-fake-exception) */
    Unk80101EC8Record *rec1 = &D_80101EC8[1];

    scr->unkA8.x = 0;
    scr->unkA8.y = 0;
    scr->unkA8.z = 0;
    scr->unkB8.x = 0;
    scr->unkB8.y = 0;
    scr->unkB8.z = 0;

    if (D_800A3824 & 1) {
        scr->unkA8.x = SPAD->unk48[0][0].x;
        scr->unkA8.y = SPAD->unk48[0][0].y;
        scr->unkA8.z = SPAD->unk48[0][0].z;
        scr->unkA8.x += SPAD->unk48[0][1].x;
        scr->unkA8.y += SPAD->unk48[0][1].y;
        scr->unkA8.z += SPAD->unk48[0][1].z;
        scr->unkB8.x = D_80101EC8[0].unk_234[0].x;
        scr->unkB8.y = D_80101EC8[0].unk_234[0].y;
        scr->unkB8.z = D_80101EC8[0].unk_234[0].z;
        scr->unkB8.x += D_80101EC8[0].unk_234[1].x;
        scr->unkB8.y += D_80101EC8[0].unk_234[1].y;
        scr->unkB8.z += D_80101EC8[0].unk_234[1].z;
    } else {
        scr->unkA8.x = SPAD->unk00[0][0].x;
        scr->unkA8.y = SPAD->unk00[0][0].y;
        scr->unkA8.z = SPAD->unk00[0][0].z;
        scr->unkA8.x += SPAD->unk00[0][1].x;
        scr->unkA8.y += SPAD->unk00[0][1].y;
        scr->unkA8.z += SPAD->unk00[0][1].z;
        scr->unkB8.x = D_80101EC8[0].unk_210[0].x;
        scr->unkB8.y = D_80101EC8[0].unk_210[0].y;
        scr->unkB8.z = D_80101EC8[0].unk_210[0].z;
        scr->unkB8.x += D_80101EC8[0].unk_210[1].x;
        scr->unkB8.y += D_80101EC8[0].unk_210[1].y;
        scr->unkB8.z += D_80101EC8[0].unk_210[1].z;
    }
    if (D_800A3824 & 2) {
        scr->unkA8.x += SPAD->unk48[1][0].x;
        scr->unkA8.y += SPAD->unk48[1][0].y;
        scr->unkA8.z += SPAD->unk48[1][0].z;
        scr->unkB8.x += rec1->unk_234[0].x;
        scr->unkB8.y += rec1->unk_234[0].y;
        scr->unkB8.z += rec1->unk_234[0].z;
        scr->unkA8.x += SPAD->unk48[1][1].x;
        scr->unkA8.y += SPAD->unk48[1][1].y;
        scr->unkA8.z += SPAD->unk48[1][1].z;
        scr->unkB8.x += rec1->unk_234[1].x;
        scr->unkB8.y += rec1->unk_234[1].y;
        scr->unkB8.z += rec1->unk_234[1].z;
    } else {
        scr->unkA8.x += SPAD->unk00[1][0].x;
        scr->unkA8.y += SPAD->unk00[1][0].y;
        scr->unkA8.z += SPAD->unk00[1][0].z;
        scr->unkB8.x += rec1->unk_210[0].x;
        scr->unkB8.y += rec1->unk_210[0].y;
        scr->unkB8.z += rec1->unk_210[0].z;
        scr->unkA8.x += SPAD->unk00[1][1].x;
        scr->unkA8.y += SPAD->unk00[1][1].y;
        scr->unkA8.z += SPAD->unk00[1][1].z;
        scr->unkB8.x += rec1->unk_210[1].x;
        scr->unkB8.y += rec1->unk_210[1].y;
        scr->unkB8.z += rec1->unk_210[1].z;
    }
    scr->unk13C.x = ((scr->unkA8.x * 3) + scr->unkB8.x) >> 4;
    scr->unk13C.y = ((scr->unkA8.y * 3) + scr->unkB8.y) >> 4;
    scr->unk13C.z = ((scr->unkA8.z * 3) + scr->unkB8.z) >> 4;
}

/* Per-frame update of the two D_80101EC8 records.  The three
 * scratchpad points per character at SPAD->unk00 and the two at SPAD->unk48
 * are copied into each record's unk_210 / unk_234 (func_8002C22C reads them
 * back); unk_18C is the centroid of SPAD->unkA8[k][1..3] and unk_174 the
 * midpoint of SPAD->unkA8[k][4..5]. */
void func_8002C61C(void) {
    /* FAKE: pointer aliases to D_80101EC8[0] / [1]: the target keeps both
     * bases in $s1 / $s0 for unk_3C / unk_286 / unk_0C / unk_F4 / unk_28C;
     * without them those accesses become absolute lui pairs
     * (GO_IF_LEGITIMATE_ADDRESS, mips.h:2286): 12 more instructions, score 38
     * (pointer-alias-fake-exception) */
    Unk80101EC8Record *s1 = &D_80101EC8[0];
    Unk80101EC8Record *s0 = &D_80101EC8[1];
    s32 i;
    u16 mode;

    mode = D_80101EC8[0].unk_6A;

    if (mode == 0xF || mode == 0x1C || mode == 0x1D || mode == 0x1E ||
        mode == 0x1F || mode == 0x20 || mode == 0x21) {
        func_80026DA4();
    } else if (mode == 0x11) {
        func_8002C0DC();
    } else {
        func_8002872C();
        func_800288C8();
        D_800A3824 = func_80029454();
        if (D_800A3824 < 0)
            goto do_calc;
        func_8002C22C();
        if (D_800A3824 < 0)
            goto do_calc;
        if (s1->unk_AD != 0 || s0->unk_AD != 0) {
            func_800283D0(s1, &SPAD->unk2B8.rec.unk13C.x);
            func_800283D0(s0, &SPAD->unk2B8.rec.unk13C.x);
            s0->unk_AD = 0;
            s1->unk_AD = 0;
            goto after_calc;
        }
    do_calc:
        func_8002AB08(0);
    after_calc:

        if (s1->unk_3C >= 3 && s0->unk_3C >= 3 && D_800A38A8 != 0 &&
            s1->unk_286 == -1 && s0->unk_286 == -1 && s1->unk_0C != 0x1F &&
            s0->unk_0C != 0x1F) {
            s32 diff = s1->unk_F4.y - s0->unk_F4.y;
            if (diff < 0)
                diff = -diff;
            if (diff < 0x3E8) {
                s1->unk_286 = 0xA;
                s0->unk_286 = 0xA;
                s0->unk_28C = 0;
                s1->unk_28C = 0;
                D_800A3910 = 0;
                D_800A389C = 0;
            }
        }
    }

    if (D_80101EC8[0].unk_6A == 5) {
        D_800A3748 = 1;
        D_800A3834 = 0x1C;
    } else if (D_80101EC8[1].unk_6A == 5) {
        D_800A3748 = 0;
        D_800A3834 = 0x1C;
    }

    for (i = 0; i < 3; i++) {
        D_80101EC8[0].unk_210[i] = SPAD->unk00[0][i];
        D_80101EC8[1].unk_210[i] = SPAD->unk00[1][i];
    }

    for (i = 0; i < 2; i++) {
        D_80101EC8[0].unk_234[i] = SPAD->unk48[0][i];
        D_80101EC8[1].unk_234[i] = SPAD->unk48[1][i];
    }

    for (i = 0; i < 2; i++) {
        D_80101EC8[i].unk_18C.x =
            (SPAD->unkA8[i][1].x + SPAD->unkA8[i][2].x + SPAD->unkA8[i][3].x) /
            3;
        D_80101EC8[i].unk_18C.y =
            (SPAD->unkA8[i][1].y + SPAD->unkA8[i][2].y + SPAD->unkA8[i][3].y) /
            3;
        D_80101EC8[i].unk_18C.z =
            (SPAD->unkA8[i][1].z + SPAD->unkA8[i][2].z + SPAD->unkA8[i][3].z) /
            3;
        D_80101EC8[i].unk_174.x =
            (SPAD->unkA8[i][4].x + SPAD->unkA8[i][5].x) / 2;
        D_80101EC8[i].unk_174.y =
            (SPAD->unkA8[i][4].y + SPAD->unkA8[i][5].y) / 2;
        D_80101EC8[i].unk_174.z =
            (SPAD->unkA8[i][4].z + SPAD->unkA8[i][5].z) / 2;
    }

    {
        s16 saved = s1->unk_286;
        if (saved == -1) {
            func_80031B24();
            if (s1->unk_286 == saved) {
                func_80032314();
            }
        }
    }
}

extern s32 func_8002D320(
    s32 flag, Unk1F8002B8Rec *obj, LeafPos *pos, s32 threshold, s32 r_sq);
extern s32 func_8002D780(
    s32 flag, Unk1F8002B8Rec *obj, s32 *pos, s32 threshold, s32 r_sq);

void func_8002CA8C(Unk80101EC8Record *a0, s32 a1, s32 a2) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    s32 id = a0->index;
    BoneHitRec *recbase = D_800F5F68[id];
    BoneHitRec *rec;
    s32 hitMask = 0;
    s32 seenMask = 0;
    s32 i;

    for (i = 0, rec = recbase; i < 0x16; i++, rec++) {
        s32 x;
        s32 y;
        s32 z;
        s32 r;
        s32 hit;

        if (a0->unk_26C == 0 && i >= 6 && i <= 9) {
            continue;
        }

        r = rec->unk_0C;
        /* FAKE: the AABB reject flag is staged through `hit` (set on
         * reject, read by the `continue` test, then overwritten by the call); a
         * separate flag takes $a1 instead of the target's $s0 (score 3) */
        hit = 0;
        x = SPAD->unkA8[id][i].x;
        if (scr->unk84.x < x - r || x + r < scr->unk78.x) {
            hit = 1;
        } else {
            y = SPAD->unkA8[id][i].y;
            if (scr->unk84.y < y - r || y + r < scr->unk78.y) {
                hit = 1;
            } else {
                z = SPAD->unkA8[id][i].z;
                if (scr->unk84.z < z - r || z + r < scr->unk78.z) {
                    hit = 1;
                }
            }
        }
        if (hit != 0) {
            continue;
        }

        if (a1 != 0) {
            hit = func_8002D780(
                0, scr, (s32 *)&SPAD->unkA8[id][i], r, rec->unk_0E);
            if (hit != 0) {
                if (rec->unk_00 != 0 && a2 != 0) {
                    if (func_8002D780(
                            1, scr, (s32 *)0, rec->unk_10, rec->unk_12) != 0) {
                        hitMask |= 1 << i;
                    }
                }
            }
        } else {
            hit = func_8002D320(0, scr, &SPAD->unkA8[id][i], r, rec->unk_0E);
            if (hit != 0) {
                if (rec->unk_00 != 0 && a2 != 0) {
                    if (func_8002D320(1, scr, NULL, rec->unk_10, rec->unk_12) !=
                        0) {
                        hitMask |= 1 << i;
                    }
                }
            }
        }
        if (hit != 0) {
            seenMask |= 1 << i;
        }
    }

    scr->unkB4 = seenMask;
    scr->unkC4 = hitMask;
}

/* Orients a triangle's local frame. The vertices obj->unk60[0..2] give edge
 * vectors a = v1 - v0 (unkA8) and b = v2 - v0 (unkB8) and n = a x b (unkC8).
 * If n's components are within +-0x3FFF and |n| < 0x4000, the yaw (unkF8.vy)
 * and pitch (.vx) come from a and it returns 0; otherwise n is scaled down by
 * 64 and the angles come from n (returns 1). A rotation matrix built in unkD8
 * (RotMatrixY, RotMatrixX) then rotates a and b in place.
 * GTE islands (owner cop2 cluster grant, cop2-addressing-preamble-cluster,
 * cluster ruling 2026-08-17): PsyQ inline_o.h / gtemac.h macro text
 * (Xeeynamo/croc@f30ff1ee): gte_OuterProduct0 (gtemac.h:190-196) =
 * gte_ldopv1 (inline_o.h:595) / gte_ldopv2 / gte_op0 / gte_stlvnl; gte_sqr0 +
 * gte_stlvnl; gte_Lzc (gtemac.h:230-236) = gte_ldlzc / 2x gte_nop /
 * gte_stlzc, three times; gte_SetRotMatrix, gte_ldlv0, gte_rtv0, gte_stlvnl.
 * Operands go through the macros' own `move $12,%0`; cc1 materializes
 * &sp_tmpN before each gte_stlzc island. Island boundaries and clobbers
 * follow this file's gte_ldopv1 / gte_ldopv2 / gte_op0 (func_8002FDB0)
 * and gte_stlvnl / gte_SetRotMatrix / gte_ldlv0 / gte_rtv0 (func_8002E838).
 * Departures: the gte_ldopv2, gte_ldlv0 and gte_ldlzc islands carry the two
 * nops of the macro that follows (gte_op0 / gte_rtv0 / the gte_Lzc pair of
 * gte_nop), so the gte_op0 and gte_rtv0 islands are just their command words;
 * each `.word` is the real cop2 command for the DMPSX placeholder: op0
 * 0x0000127f -> 0x4B70000C, sqr0 0x00000f3f -> 0x4AA00428, rtv0 0x0000013f ->
 * 0x4A486012. gte_sqr0 and the gte_ldlzc / gte_stlzc islands have no
 * in-file precedent in this form. `len` and `temp` each hold two values
 * (Ruling 11). */
s32 func_8002CD58(Unk1F8002B8Rec *obj) {
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 sp_tmp3;
    s32 len_sq;
    s32 xz_sq;
    /* FAKE: temp holds two values: n.x*n.x + n.z*n.z, then the square-root
     * table byte; one local per value: score 6 (Ruling 11) */
    s32 temp;
    s32 nxz_len;

    obj->unkA8.x = obj->unk60[1]->x - obj->unk60[0]->x;
    obj->unkA8.y = obj->unk60[1]->y - obj->unk60[0]->y;
    obj->unkA8.z = obj->unk60[1]->z - obj->unk60[0]->z;
    obj->unkB8.x = obj->unk60[2]->x - obj->unk60[0]->x;
    obj->unkB8.y = obj->unk60[2]->y - obj->unk60[0]->y;
    obj->unkB8.z = obj->unk60[2]->z - obj->unk60[0]->z;

    /* gte_ldopv1(a) -- inline_o.h:595: a into the rotation matrix
     * diagonal (cop2 control $0/$2/$4). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"(&obj->unkA8) : "$12", "$13", "$14", "$15");
    /* gte_ldopv2(b) -- inline_o.h:626: b into IR1-IR3, then gte_op0's
     * two nops (inline_o.h:1866). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(&obj->unkB8) : "$12");
    /* gte_op0() command word -- inline_o.h:1866, OP sf=0. */
    __asm__ volatile(".word 0x4B70000C");
    /* gte_stlvnl(n) -- inline_o.h:2422: MAC1-MAC3 to obj->unkC8. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(&obj->unkC8) : "$12", "memory");

    if ((u32)(obj->unkC8.x + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.y + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.z + 0x3FFF) < 0x7FFF) {
        /* FAKE: len holds two values: |n| for the `< 0x4000` guard, then
         * |a.xz| for the pitch's ratan2; one local per value: score 3 (Ruling
         * 11) */
        s32 len;
        /* gte_sqr0() -- inline_o.h:1749: square IR1-IR3 (still n). */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n");
        /* gte_stlvnl -- inline_o.h:2422: the squares to obj->unk100[0]. */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(&obj->unk100[0]) : "$12", "memory");
        len_sq = obj->unk100[0].x + obj->unk100[0].y + obj->unk100[0].z;
        if ((u32)len_sq < 0x400) {
            len = (u32)g_sqrt_table_u8[len_sq] >> 3;
        } else {
            s32 lzcr = 0;
            if (len_sq >= 0) {
                /* gte_Lzc(len_sq, &sp_tmp) -- gtemac.h:230-236: gte_ldlzc
                 * (inline_o.h:645) + 2x gte_nop + gte_stlzc */
                __asm__ volatile(
                    "move   $12, %0\n"
                    "mtc2   $12, $30\n"
                    "nop\n"
                    "nop\n"
                    :: "r"(len_sq) : "$12");
                __asm__ volatile(
                    "move   $12, %0\n"
                    "swc2   $31, 0($12)\n"
                    :: "r"(&sp_tmp) : "$12", "memory");
                lzcr = sp_tmp;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                len = (u32)(g_sqrt_table_u8[(u32)len_sq >> shift] << 16) >>
                      (0x13 - ((u32)shift >> 1));
            }
        }
        if ((u32)len < 0x4000) {
            obj->unkF8.vy = 0x800 - ratan2(obj->unkA8.x, obj->unkA8.z);
            xz_sq = obj->unkA8.x * obj->unkA8.x + obj->unkA8.z * obj->unkA8.z;
            if ((u32)xz_sq < 0x400) {
                len = (u32)g_sqrt_table_u8[xz_sq] >> 3;
            } else {
                s32 lzcr = 0;
                if (xz_sq >= 0) {
                    /* gte_Lzc(xz_sq, &sp_tmp2) -- gte_ldlzc (inline_o.h:645) +
                     * 2x gte_nop (:3068), gte_stlzc (:2999); slot sp+0x14. */
                    __asm__ volatile(
                        "move   $12, %0\n"
                        "mtc2   $12, $30\n"
                        "nop\n"
                        "nop\n"
                        :: "r"(xz_sq) : "$12");
                    __asm__ volatile(
                        "move   $12, %0\n"
                        "swc2   $31, 0($12)\n"
                        :: "r"(&sp_tmp2) : "$12", "memory");
                    lzcr = sp_tmp2;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    len = (u32)(g_sqrt_table_u8[(u32)xz_sq >> shift] << 16) >>
                          (0x13 - ((u32)shift >> 1));
                }
            }
            obj->unkF8.vx = 0x800 - ratan2(obj->unkA8.y, len);
            obj->unkD8.m[0][0] = 0x1000;
            obj->unkD8.m[0][1] = 0;
            obj->unkD8.m[0][2] = 0;
            obj->unkD8.m[1][0] = 0;
            obj->unkD8.m[1][1] = 0x1000;
            obj->unkD8.m[1][2] = 0;
            obj->unkD8.m[2][0] = 0;
            obj->unkD8.m[2][1] = 0;
            obj->unkD8.m[2][2] = 0x1000;
            RotMatrixY(obj->unkF8.vy, &obj->unkD8);
            RotMatrixX(obj->unkF8.vx, &obj->unkD8);
            /* gte_SetRotMatrix(&obj->unkD8) -- inline_o.h:860: the five
             * packed rotation-matrix words into cop2 control $0..$4. */
            __asm__ volatile(
                "move   $12, %0\n"
                "lw     $13, 0($12)\n"
                "lw     $14, 4($12)\n"
                "ctc2   $13, $0\n"
                "ctc2   $14, $1\n"
                "lw     $13, 8($12)\n"
                "lw     $14, 12($12)\n"
                "lw     $15, 16($12)\n"
                "ctc2   $13, $2\n"
                "ctc2   $14, $3\n"
                "ctc2   $15, $4\n"
                :: "r"(&obj->unkD8) : "$12", "$13", "$14", "$15");
            /* gte_ldlv0(a) -- inline_o.h:277: pack VX0/VY0, VZ0 via lwc2,
             * then gte_rtv0's two nops (inline_o.h:1353). */
            __asm__ volatile(
                "move   $12, %0\n"
                "lhu    $14, 4($12)\n"
                "lhu    $13, 0($12)\n"
                "sll    $14, $14, 16\n"
                "or     $13, $13, $14\n"
                "mtc2   $13, $0\n"
                "lwc2   $1, 8($12)\n"
                "nop\n"
                "nop\n"
                :: "r"(&obj->unkA8) : "$12", "$13", "$14");
            /* gte_rtv0() command word -- inline_o.h:1353, MVMVA sf=1,
             * rotation x V0. */
            __asm__ volatile(".word 0x4A486012");
            /* gte_stlvnl(a) -- inline_o.h:2422: rotated a written back. */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(&obj->unkA8) : "$12", "memory");
            /* gte_ldlv0(b) (inline_o.h:277) / gte_rtv0() (:1353) /
             * gte_stlvnl(b) (:2422): same for b. */
            __asm__ volatile(
                "move   $12, %0\n"
                "lhu    $14, 4($12)\n"
                "lhu    $13, 0($12)\n"
                "sll    $14, $14, 16\n"
                "or     $13, $13, $14\n"
                "mtc2   $13, $0\n"
                "lwc2   $1, 8($12)\n"
                "nop\n"
                "nop\n"
                :: "r"(&obj->unkB8) : "$12", "$13", "$14");
            __asm__ volatile(".word 0x4A486012");
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(&obj->unkB8) : "$12", "memory");
            return 0;
        }
    }

    obj->unkF8.vy = 0x800 - ratan2(obj->unkC8.x, obj->unkC8.z);
    obj->unkC8.x >>= 6;
    obj->unkC8.y >>= 6;
    obj->unkC8.z >>= 6;
    temp = obj->unkC8.x * obj->unkC8.x + obj->unkC8.z * obj->unkC8.z;
    if ((u32)temp < 0x400) {
        nxz_len = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        s32 lzcr = 0;
        if (temp >= 0) {
            /* gte_Lzc(temp, &sp_tmp3) -- gte_ldlzc (inline_o.h:645) +
             * 2x gte_nop (:3068), gte_stlzc (:2999); slot sp+0x18. */
            __asm__ volatile(
                "move   $12, %0\n"
                "mtc2   $12, $30\n"
                "nop\n"
                "nop\n"
                :: "r"(temp) : "$12");
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $31, 0($12)\n"
                :: "r"(&sp_tmp3) : "$12", "memory");
            lzcr = sp_tmp3;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            temp = g_sqrt_table_u8[(u32)temp >> shift];
            nxz_len = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, nxz_len);
    obj->unkD8.m[0][0] = 0x1000;
    obj->unkD8.m[0][1] = 0;
    obj->unkD8.m[0][2] = 0;
    obj->unkD8.m[1][0] = 0;
    obj->unkD8.m[1][1] = 0x1000;
    obj->unkD8.m[1][2] = 0;
    obj->unkD8.m[2][0] = 0;
    obj->unkD8.m[2][1] = 0;
    obj->unkD8.m[2][2] = 0x1000;
    RotMatrixY(obj->unkF8.vy, &obj->unkD8);
    RotMatrixX(obj->unkF8.vx, &obj->unkD8);
    /* gte_SetRotMatrix (inline_o.h:860), then gte_ldlv0 (:277) / gte_rtv0
     * (:1353) / gte_stlvnl (:2422) for a and for b: the same islands as the
     * first path. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(&obj->unkD8) : "$12", "$13", "$14", "$15");
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(&obj->unkA8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(&obj->unkA8) : "$12", "memory");
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(&obj->unkB8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(&obj->unkB8) : "$12", "memory");
    return 1;
}

s32 func_8002D320(
    s32 flag, Unk1F8002B8Rec *obj, LeafPos *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        SVECTOR *vin;
        Vec3i32 *vout;
        obj->unkF8.vx = pos->x - obj->unk60[0]->x;
        obj->unkF8.vy = pos->y - obj->unk60[0]->y;
        obj->unkF8.vz = pos->z - obj->unk60[0]->z;
        vin = &obj->unkF8;
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "lwc2 $0, 0($t4)\n"
            "lwc2 $1, 4($t4)\n"
            "nop\n"
            "nop\n"
            ".word 0x4A486012"
            : : "r"(vin) : "$12", "memory");
        vout = &obj->unk100[0];
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "swc2 $25, 0($t4)\n"
            "swc2 $26, 4($t4)\n"
            "swc2 $27, 8($t4)"
            : : "r"(vout) : "$12", "memory");
    }
    {
        /* x: unk100[0].x, then x * x + y * y, then r_sq minus that, then its
         * square root (the sphere's half-chord along z) (Ruling 11) */
        s32 x;
        s32 y;
        s32 sp_var;
        s32 min_z;
        s32 max_z;
        s32 az;
        s32 bz;
        s32 z;
        s32 ret;
        s32 neg_threshold = -threshold;

        x = obj->unk100[0].x;
        if (x < neg_threshold || threshold < x)
            return 0;
        y = obj->unk100[0].y;
        if (y < neg_threshold || threshold < y)
            return 0;

        x = x * x + y * y;
        if (r_sq < x)
            return 0;
        x = r_sq - x;

        if ((u32)x < 0x400) {
            x = (u32)g_sqrt_table_u8[x] >> 3;
        } else {
            s32 lzcr = 0;
            if (x >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, $sp, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(x) : "$12");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)x >> shift];
                x = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_z = 0;
        min_z = 0;
        az = obj->unkA8.z;
        if (az < 0) {
            min_z = az;
        } else if (min_z < az) {
            max_z = az;
        }
        bz = obj->unkB8.z;
        if (bz < min_z) {
            min_z = bz;
        } else if (max_z < bz) {
            max_z = bz;
        }
        z = obj->unk100[0].z;
        if (max_z < z - x)
            return 0;
        if (z + x < min_z) {
            /* FAKE: dead store, overwritten by `ret = 0;`: the two-set arm
             * keeps the target's branch diamond instead of slt + xori
             * (dead-store-fake-exception) */
            ret = 1;
            ret = 0;
        } else {
            ret = 1;
        }
        return ret;
    }
}

/* Segment/circle test in x/z. Returns 0 when p1 and p2 lie on the same side
 * outside the +-threshold band in x or in z; otherwise solves the quadratic for
 * where p1 -> p2 meets the circle of squared radius r_sq about the origin
 * (terms >> 9; square root by g_sqrt_table_u8, GTE LZCS island for
 * discriminants >= 0x400) and returns whether both Q8 intersection parameters
 * are in range. Load-bearing: `ud = disc; lzcr = 0;` in that order, `disc`
 * holding the discriminant, the square root and the <<9 result, `num1` before
 * `denom`, and `result` kept apart from the comparison flag. The `ud`
 * re-store is a FAKE, explained at its site. */
s32 func_8002D518(s32 threshold, s32 r_sq, s32 *p1, s32 *p2) {
    s32 x1, z1, x2, z2;

    x1 = p1[0];
    x2 = p2[0];

    if (x1 < x2) {
        if (x2 < -threshold)
            return 0;
        if (threshold >= x1)
            goto z_check;
        return 0;
    } else {
        if (x1 < -threshold)
            return 0;
        if (threshold < x2)
            return 0;
    }

z_check:
    z1 = p1[1];
    z2 = p2[1];

    if (z1 < z2) {
        if (z2 < -threshold)
            return 0;
        if (threshold >= z1)
            goto dist_calc;
        return 0;
    } else {
        if (z1 < -threshold)
            return 0;
        if (threshold < z2)
            return 0;
    }

dist_calc: {
    s32 ax = p2[0] - p1[0];
    s32 x1r = p1[0];
    s32 az = p2[1] - p1[1];
    s32 z1r = p1[1];

    s32 ax_sq = ax * ax;
    s32 az_sq = az * az;
    s32 cx = ax * x1r;
    s32 cz = az * z1r;
    s32 x1_sq = x1r * x1r;
    s32 z1_sq = z1r * z1r;

    s32 dot2 = (cx + cz) * 2;
    s32 dot2_9 = dot2 >> 9;
    s32 c_val = ((x1_sq + z1_sq) - r_sq) >> 9;
    s32 dist_sq = ax_sq + az_sq;

    s32 disc = (dot2_9 * dot2_9) - ((dist_sq >> 9) * (c_val << 2));
    s32 result;

    if (disc < 0) {
        result = 0;
    } else {
        if ((u32)disc < 0x400u) {
            disc = g_sqrt_table_u8[disc] >> 3;
        } else {
            s32 sp_tmp;
            s32 lzcr;
            u32 ud;
            ud = disc;
            lzcr = 0;
            if (disc >= 0) {
                /* FAKE: same-value re-store of `ud` (it already holds disc):
                 * with a single def cse merges ud into disc and deletes the
                 * copy the target keeps in $a0; the store emits nothing;
                 * without it score 3 */
                ud = disc;
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu   $t4, $sp, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp)
                    : "r"(ud)
                    : "$12");
                lzcr = sp_tmp;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tval = g_sqrt_table_u8[ud >> shift];
                s32 half = (u32)shift >> 1;
                disc = (u32)(tval << 16) >> (0x13 - half);
            }
        }
        {
            s32 neg_b = -dot2;
            s32 num1;
            s32 denom;
            s32 t1_val;
            s32 t2_val;
            disc <<= 9;
            num1 = (neg_b + disc) << 8;
            denom = dist_sq * 2;
            t1_val = num1 / denom;
            t2_val = ((neg_b - disc) << 8) / denom;

            {
                s32 flag = 0;
                if (t1_val >= 0) {
                    flag = t2_val < 0x101;
                }
                result = flag;
            }
        }
    }
    return result;
}
}

s32 func_8002D780(
    s32 flag, Unk1F8002B8Rec *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        obj->unkF8.vx = pos[0] - obj->unk60[0]->x;
        obj->unkF8.vy = pos[1] - obj->unk60[0]->y;
        obj->unkF8.vz = pos[2] - obj->unk60[0]->z;
        /* gte_ApplyRotMatrix(&obj->unkF8, &obj->unk100[0]) -- gtemac.h 4.3 =
         * inline_o.h gte_ldv0, gte_rtv0, gte_stlvnl, one statement per header
         * line incl. its `move $12,%0` (inline_o.h class, inline-asm-policy);
         * one departure: gte_rtv0's placeholder 0x0000013f is the post-DMPSX
         * word 0x4A486012 (owner Q61 grant) */
        __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("move  $12,%0": :"r"(&obj->unk100[0]):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
    }

    {
        s32 y = obj->unk100[0].z;
        if (y < -threshold || threshold < y)
            return 0;
    }

    {
        s32 x0 = obj->unkA8.x;
        s32 x2 = obj->unkB8.x;
        s32 z0 = obj->unkA8.y;
        s32 z2 = obj->unkB8.y;
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = obj->unk100[0].x;
        s32 pz = obj->unk100[0].y;
        /* FAKE: cross_center / cross_point hold one value per side test: the
         * cross product of a triangle edge with the centroid's / query point's
         * offset, for the edges (0,0)-(x0,z0), (0,0)-(x2,z2), (x0,z0)-(x2,z2);
         * a pair per test: score 36 (Ruling 11) */
        s32 cross_center = z0 * cx - x0 * cz;
        s32 cross_point = z0 * px - x0 * pz;

        if ((cross_center ^ cross_point) >= 0) {
            cross_center = z2 * cx - x2 * cz;
            cross_point = z2 * px - x2 * pz;
            if ((cross_center ^ cross_point) >= 0) {
                /* FAKE: ax holds cx - x0 ahead of the dx set-up; in the product
                 * the subtraction moves (score 2). */
                s32 ax = cx - x0;
                s32 dx;
                /* FAKE: the third edge's z2 - z0 is staged through the finished
                 * `flag` parameter (unread after the mode test); a block-local
                 * dz takes dx's $v1: score 9 */
                flag = z2 - z0;
                dx = x2 - x0;
                cross_center = (flag * ax) - (dx * (cz - z0));
                cross_point = (flag * (px - x0)) - (dx * (pz - z0));
                if ((cross_center ^ cross_point) >= 0)
                    return 1;
            }
        }
    }

    {
        s32 y = obj->unk100[0].z;
        s32 sp_var;
        s32 dist = r_sq - y * y;
        s32 sqrt_val;
        Vec3i32 *p118;
        Vec3i32 *p124;
        Vec3i32 *p10C;

        if ((u32)dist < 0x400) {
            sqrt_val = (u32)g_sqrt_table_u8[dist] >> 3;
        } else {
            s32 m = dist;
            s32 lzcr = 0;
            if (dist >= 0) {
                /* FAKE: same-value re-store of `m`: breaks the m == dist
                 * equivalence across the skipped arm so the shift below reads
                 * the $a0 copy, not dist ($s1), as in func_8002D518; without
                 * it: score 4 */
                m = dist;
                /* gte_Lzc(m, &sp_var) -- gtemac.h :174-178: gte_ldlzc, gte_nop
                 * x2, gte_stlzc */
                __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&sp_var):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                sqrt_val = (u32)(g_sqrt_table_u8[(u32)m >> shift] << 16) >>
                           (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = &obj->unk118[0];
        p124 = &obj->unk118[1];
        p118->x = obj->unkA8.x - obj->unk100[0].x;
        p118->y = obj->unkA8.y - obj->unk100[0].y;
        p124->x = obj->unkB8.x - obj->unk100[0].x;
        p124->y = obj->unkB8.y - obj->unk100[0].y;
        if (func_8002D518(sqrt_val, dist, (s32 *)p118, (s32 *)p124) != 0)
            return 1;

        p10C = &obj->unk100[1];
        p10C->x = -obj->unk100[0].x;
        p10C->y = -obj->unk100[0].y;
        if (func_8002D518(sqrt_val, dist, (s32 *)p10C, (s32 *)p118) != 0)
            return 1;

        if (func_8002D518(sqrt_val, dist, (s32 *)p10C, (s32 *)p124) != 0)
            return 1;
        return 0;
    }
}

/* GTE islands: equivalent expansions of the Sony PsyQ DMPSX inline_o.h
 * macros gte_ldopv1, gte_ldopv2 + gte_op0, gte_stlvnl, the gtemac.h gte_Lzc
 * (inline_o.h gte_ldlzc + gte_nop x2 + gte_stlzc), gte_SetRotMatrix,
 * gte_ldlv0 + gte_rtv0, a macro's statements joined into
 * one __asm__ (gte_op0 shares gte_ldopv2's; gte_rtv0's two nops ride with
 * gte_ldlv0 and its command word stands alone); admitted by the owner cop2
 * cluster grant (cop2-addressing-preamble-cluster, owner-instructed row
 * 2026-09-25), which covers the joined spellings. The LZC load and store are
 * separate macro statements: GCC supplies the &sp_tmp register and materializes
 * its stack address outside the island. */
s32 func_8002DAD0(Unk1F8002B8Rec *obj) {
    MATRIX *mat;
    s32 sp_tmp;
    s32 dist_sq;
    s32 dist;

    obj->unkA8.x = obj->unk60[1]->x - obj->unk60[0]->x;
    obj->unkA8.y = obj->unk60[1]->y - obj->unk60[0]->y;
    obj->unkA8.z = obj->unk60[1]->z - obj->unk60[0]->z;

    obj->unkB8.x = obj->unk60[2]->x - obj->unk60[0]->x;
    obj->unkB8.y = obj->unk60[2]->y - obj->unk60[0]->y;
    obj->unkB8.z = obj->unk60[2]->z - obj->unk60[0]->z;

    /* gte_ldopv1(vecA) -- inline_o.h:192-200, equivalent expansion: the OP
     * diagonal (RT11/RT22/RT33) into cop2 control $0/$2/$4
     * (cop2-addressing-preamble-cluster). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"(&obj->unkA8) : "$12", "$13", "$14", "$15");

    /* gte_ldopv2(vecB) -- inline_o.h:201-206, equivalent expansion
     * (`addu $12,%0,$zero` for its `move`): IR1-IR3; then gte_op0()
     * (inline_o.h:711-715 / inline_c.h:784-787, outer product) with the real
     * cop2 word 0x4B70000C for the DMPSX placeholder 0x0000127f, as
     * include/gte.h's gte_mvmva does for MVMVA. */
    __asm__ volatile(
        "addu   $12, %0, $zero\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word 0x4B70000C\n"
        :: "r"(&obj->unkB8) : "$12");

    /* gte_stlvnl -- inline_o.h:904-909, equivalent expansion: MAC1-MAC3
     * (the cross product) to obj->unkC8. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(&obj->unkC8) : "$12", "memory");

    if ((u32)(obj->unkC8.x + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.y + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.z + 0x3FFF) < 0x7FFF) {
        return 0;
    }

    obj->unkF8.vy = 0x800 - ratan2(obj->unkC8.x, obj->unkC8.z);
    obj->unkC8.x = obj->unkC8.x >> 6;
    obj->unkC8.y = obj->unkC8.y >> 6;
    /* FAKE: the scaled z delta is staged through `dist` (dead until the
     * if/else below); a block-local temp moves dist_sq from the target's $a0 to
     * $a1: score 6 */
    dist = obj->unkC8.z;
    dist >>= 6;
    dist_sq = obj->unkC8.x * obj->unkC8.x + dist * dist;
    obj->unkC8.z = dist;

    if ((u32)dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            /* gte_Lzc(r1,r2) -- gtemac.h:174-178: gte_ldlzc
             * (inline_o.h:207-211) + 2x gte_nop (inline_o.h:1095) +
             * gte_stlzc (inline_o.h:1074-1077), each an equivalent
             * expansion (`addu $t4,%0,$zero` for `move $12,%0`); the store's
             * operand is &sp_tmp. LZCS in, LZCR out. */
            __asm__ volatile(
                "addu   $t4, %0, $zero\n"
                "mtc2   $t4, $30\n"
                "nop\n"
                "nop\n"
                :: "r"(dist_sq)
                : "$12", "$13", "$14", "$15", "memory");
            __asm__ volatile(
                "addu   $t4, %0, $zero\n"
                "swc2   $31, 0($t4)\n"
                :: "r"(&sp_tmp)
                : "$12", "$13", "$14", "$15", "memory");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            dist = (u32)(g_sqrt_table_u8[(u32)dist_sq >> shift] << 16) >>
                   (0x13 - ((u32)shift >> 1));
        }
    }

    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, dist);
    mat = &obj->unkD8;

    /* identity 3x3 rotation matrix obj->unkD8 */
    obj->unkD8.m[0][0] = 0x1000;
    obj->unkD8.m[0][1] = 0;
    obj->unkD8.m[0][2] = 0;
    obj->unkD8.m[1][0] = 0;
    obj->unkD8.m[1][1] = 0x1000;
    obj->unkD8.m[1][2] = 0;
    obj->unkD8.m[2][0] = 0;
    obj->unkD8.m[2][1] = 0;
    obj->unkD8.m[2][2] = 0x1000;
    RotMatrixY(obj->unkF8.vy, mat);
    RotMatrixX(obj->unkF8.vx, mat);

    /* gte_SetRotMatrix(r1) -- inline_o.h:272-284, expanded to equivalent
     * instructions (all 11, incl. the $13/$14 re-use order): the 5 packed
     * rotation-matrix words into cop2 control $0..$4. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");

    /* gte_ldlv0(vecA) -- inline_o.h:95-103, equivalent expansion (the
     * trailing nop pair is gte_rtv0's, not gte_ldlv0's), then gte_rtv0
     * (inline_o.h:426-430;
     * MVMVA `.word 0x4A486012` = gte_mvmva(1,0,0,3,0) under
     * include/gte.h) and gte_stlvnl (inline_o.h:904-909): vecA rotated
     * in place. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(&obj->unkA8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(&obj->unkA8) : "$12", "memory");

    /* The same gte_ldlv0 (inline_o.h:95-103) + gte_rtv0 (inline_o.h:426-430)
     * + gte_stlvnl (inline_o.h:904-909) equivalent expansions, rotating vecB
     * in place. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(&obj->unkB8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(&obj->unkB8) : "$12", "memory");

    return 1;
}

/* func_8002DE20 GTE islands: PsyQ 4.3 inline_o.h gte_ldv0, gte_rtv0,
 * gte_stlvnl and gtemac.h gte_ApplyRotMatrix, statement for statement. One
 * departure: gte_rtv0's DMPSX placeholder 0x0000013f is carried as the
 * post-DMPSX word 0x4A486012 (owner Q11 grant). cross_a / cross_b: Ruling 11
 * (see their declaration). */
extern s32 D_800A314C;

/* Rotates the three points p0/p1/p2 (relative to the origin) by the current GTE
 * rotation matrix, cuts the rotated triangle with the plane z = 0, and returns
 * 1 if the resulting segment (x1,y1)-(x2,y2) touches the triangle (0,0) / A / B
 * in the x-y plane: either endpoint inside it (same side of every edge as the
 * centroid), or the segment crossing one of its edges. */
s32 func_8002DE20(Unk1F8002B8Rec *obj, LeafPos *p0, LeafPos *p1, LeafPos *p2) {
    s32 i;
    s32 max_i;
    s32 min_i;
    s32 mid_i;
    s32 min_z;
    s32 max_z;
    s32 z;
    s32 dz_a;
    s32 dz_b;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 cx;
    s32 cy;
    /* cross_a / cross_b: one value per same-side test, the 2D cross
     * product (edge) x (point - edge start) for the pair's first / second point
     * (Ruling 11, ordinary-c-judge-decidable) */
    s32 cross_a;
    s32 cross_b;
    /* edge A-B x ((x2,y2) - A) */
    s32 cross_ab2;

    obj->unkF8.vx = p0->x - obj->unk60[0]->x;
    obj->unkF8.vy = p0->y - obj->unk60[0]->y;
    obj->unkF8.vz = p0->z - obj->unk60[0]->z;
    /* gte_ldv0(&obj->unkF8): inline_o.h 4.3 :16-20 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    /* gte_rtv0(): inline_o.h 4.3 :426-430; word 0x4A486012 for the
     * placeholder 0x0000013f */
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    obj->unkF8.vx = p1->x - obj->unk60[0]->x;
    obj->unkF8.vy = p1->y - obj->unk60[0]->y;
    obj->unkF8.vz = p1->z - obj->unk60[0]->z;
    /* gte_stlvnl(&obj->unk118[0]): inline_o.h 4.3 :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unk118[0]):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
    /* gte_ldv0(&obj->unkF8): inline_o.h 4.3 :16-20 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    /* gte_rtv0(): inline_o.h 4.3 :426-430; word 0x4A486012 for the
     * placeholder 0x0000013f */
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    obj->unkF8.vx = p2->x - obj->unk60[0]->x;
    obj->unkF8.vy = p2->y - obj->unk60[0]->y;
    obj->unkF8.vz = p2->z - obj->unk60[0]->z;
    /* gte_stlvnl(&obj->unk118[1]): inline_o.h 4.3 :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unk118[1]):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
    /* gte_ApplyRotMatrix(&obj->unkF8, &obj->unk118[2]): gtemac.h 4.3 :354-357,
     * i.e. gte_ldv0 / gte_rtv0 / gte_stlvnl as above */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unk118[2]):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    max_i = 0;
    min_i = 0;
    max_z = obj->unk118[0].z;
    min_z = max_z;
    for (i = 1; i < 3; i++) {
        z = obj->unk118[i].z;
        if (z < min_z) {
            min_i = i;
            min_z = z;
        } else if (max_z < z) {
            max_i = i;
            max_z = z;
        }
    }
    if (min_z > 0 || max_z < 0) {
        return 0;
    }
    if (min_z == 0 && max_z == 0) {
        min_i = 0;
        max_i = 1;
        obj->unk118[0].z--;
        obj->unk118[1].z++;
    }

    /* where the min->max edge crosses z = 0 */
    dz_a = obj->unk118[max_i].z - obj->unk118[min_i].z;
    if (dz_a == 0) {
        D_800A314C++;
        dz_a = 1;
    }
    x1 = obj->unk118[min_i].x +
         (-obj->unk118[min_i].z *
          (obj->unk118[max_i].x - obj->unk118[min_i].x)) /
             dz_a;
    y1 = obj->unk118[min_i].y +
         (-obj->unk118[min_i].z *
          (obj->unk118[max_i].y - obj->unk118[min_i].y)) /
             dz_a;

    /* where the edge through the middle point crosses z = 0 */
    mid_i = 3 - min_i - max_i;
    if (obj->unk118[mid_i].z >= 0) {
        dz_b = obj->unk118[mid_i].z - obj->unk118[min_i].z;
        if (dz_b == 0) {
            D_800A314C++;
            dz_b = 1;
        }
        x2 = obj->unk118[min_i].x +
             (-obj->unk118[min_i].z *
              (obj->unk118[mid_i].x - obj->unk118[min_i].x)) /
                 dz_b;
        y2 = obj->unk118[min_i].y +
             (-obj->unk118[min_i].z *
              (obj->unk118[mid_i].y - obj->unk118[min_i].y)) /
                 dz_b;
    } else {
        dz_b = obj->unk118[max_i].z - obj->unk118[mid_i].z;
        if (dz_b == 0) {
            D_800A314C++;
            dz_b = 1;
        }
        x2 = obj->unk118[mid_i].x +
             (-obj->unk118[mid_i].z *
              (obj->unk118[max_i].x - obj->unk118[mid_i].x)) /
                 dz_b;
        y2 = obj->unk118[mid_i].y +
             (-obj->unk118[mid_i].z *
              (obj->unk118[max_i].y - obj->unk118[mid_i].y)) /
                 dz_b;
    }

    cx = (obj->unkA8.x + obj->unkB8.x) / 3;
    cy = (obj->unkA8.y + obj->unkB8.y) / 3;

    /* (x1,y1) inside the triangle */
    cross_a = obj->unkA8.y * cx - obj->unkA8.x * cy;
    cross_b = obj->unkA8.y * x1 - obj->unkA8.x * y1;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = obj->unkB8.y * cx - obj->unkB8.x * cy;
        cross_b = obj->unkB8.y * x1 - obj->unkB8.x * y1;
        if ((cross_a ^ cross_b) >= 0) {
            cross_a = (obj->unkB8.y - obj->unkA8.y) * (cx - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (cy - obj->unkA8.y);
            cross_b = (obj->unkB8.y - obj->unkA8.y) * (x1 - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (y1 - obj->unkA8.y);
            if ((cross_a ^ cross_b) >= 0) {
                return 1;
            }
        }
    }
    /* (x2,y2) inside the triangle */
    cross_a = obj->unkA8.y * cx - obj->unkA8.x * cy;
    cross_b = obj->unkA8.y * x2 - obj->unkA8.x * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = obj->unkB8.y * cx - obj->unkB8.x * cy;
        cross_b = obj->unkB8.y * x2 - obj->unkB8.x * y2;
        if ((cross_a ^ cross_b) >= 0) {
            cross_a = (obj->unkB8.y - obj->unkA8.y) * (cx - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (cy - obj->unkA8.y);
            cross_b = (obj->unkB8.y - obj->unkA8.y) * (x2 - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (y2 - obj->unkA8.y);
            if ((cross_a ^ cross_b) >= 0) {
                return 1;
            }
        }
    }
    /* the segment against edge (0,0)-A */
    cross_a = obj->unkA8.y * x1 - obj->unkA8.x * y1;
    cross_b = obj->unkA8.y * x2 - obj->unkA8.x * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a =
            (y2 - y1) * (obj->unkA8.x - x1) - (x2 - x1) * (obj->unkA8.y - y1);
        cross_b = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    /* the segment against edge (0,0)-B */
    cross_a = obj->unkB8.y * x1 - obj->unkB8.x * y1;
    cross_b = obj->unkB8.y * x2 - obj->unkB8.x * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a =
            (y2 - y1) * (obj->unkB8.x - x1) - (x2 - x1) * (obj->unkB8.y - y1);
        cross_b = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    /* the segment against edge A-B */
    cross_a = (obj->unkB8.y - obj->unkA8.y) * (x1 - obj->unkA8.x) -
              (obj->unkB8.x - obj->unkA8.x) * (y1 - obj->unkA8.y);
    cross_ab2 = (obj->unkB8.y - obj->unkA8.y) * (x2 - obj->unkA8.x) -
                (obj->unkB8.x - obj->unkA8.x) * (y2 - obj->unkA8.y);
    if ((cross_a ^ cross_ab2) >= 0) {
        cross_a =
            (y2 - y1) * (obj->unkA8.x - x1) - (x2 - x1) * (obj->unkA8.y - y1);
        cross_b =
            (y2 - y1) * (obj->unkB8.x - x1) - (x2 - x1) * (obj->unkB8.y - y1);
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    return 0;
}

/* Point-in-triangle test: is arg3 on the same side of each triangle edge as
 * the centroid? For each of the three edges (arg0->arg1, arg0->arg2,
 * arg1->arg2) the 2D cross product of the edge vector with the centroid
 * offset and with the query-point offset must have the same sign; the sign
 * agreement is tested as `(cc ^ cp) >= 0`. Same idiom as the triangle test
 * inside func_8002D780 in this file. */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
    s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
    s32 cross_center;
    s32 cross_point;

    {
        s32 dz = arg1[2] - arg0[2];
        s32 dx = arg1[0] - arg0[0];
        cross_center =
            (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
        cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
    }
    if ((cross_center ^ cross_point) >= 0) {
        {
            s32 dz = arg2[2] - arg0[2];
            s32 dx = arg2[0] - arg0[0];
            cross_center =
                (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
            cross_point =
                (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
        }
        if ((cross_center ^ cross_point) >= 0) {
            {
                s32 dz = arg2[2] - arg1[2];
                s32 dx = arg2[0] - arg1[0];
                cross_center =
                    (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
                cross_point =
                    (dz * (arg3[0] - arg1[0])) - (dx * (arg3[2] - arg1[2]));
            }
            if ((cross_center ^ cross_point) >= 0) {
                return 1;
            }
        }
    }
    return 0;
}

void func_8002E838(Unk1F8002B8Rec *scr) {
    s32 sp_tmp;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 dist_sq;
    s32 angle;
    s32 dist;

    scr->unkA8.x = scr->unk60[1]->x - scr->unk60[0]->x;
    scr->unkA8.y = scr->unk60[1]->y - scr->unk60[0]->y;
    scr->unkA8.z = scr->unk60[1]->z - scr->unk60[0]->z;
    angle = ratan2(scr->unkA8.x, scr->unkA8.z);
    dist_sq = scr->unkA8.x * scr->unkA8.x + scr->unkA8.z * scr->unkA8.z;
    scr->unkF8.vy = 0x800 - angle;

    if ((u32)dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            /* Hand-written GTE leading-zero count (LZCS in, LZCR out), as in
             * func_8001A67C. */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"  /* &sp_tmp */
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                : "=m"(sp_tmp)
                : "r"(dist_sq)
                : "$2", "$12");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = g_sqrt_table_u8[(u32)dist_sq >> shift];
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    angle = ratan2(scr->unkA8.y, dist);
    mat = &scr->unkD8;
    scr->unkF8.vx = 0x800 - angle;

    /* identity 3x3 rotation matrix at scr->unkD8 */
    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixY(scr->unkF8.vy, mat);
    RotMatrixX(scr->unkF8.vx, mat);

    /* gte_SetRotMatrix(r): the 5 packed rotation-matrix words into cop2
     * control $0..$4 (as in func_800203B4). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    vec = &scr->unkA8;
    /* PsyQ libgte inline macro gte_ldlv0(r) (the long-vector gte_ldv0) ---
     * pack VX0/VY0 into one word, mtc2 to $0, lwc2 VZ0 into $1, then the
     * 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "$13", "$14");
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r (rotated vector written back in place). */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");
}

s32 func_8002EA24(Unk1F8002B8Rec *scr, LeafPos *pos, s32 threshold, s32 r_sq) {
    SVECTOR *vin;
    Vec3i32 *vout;
    scr->unkF8.vx = pos->x - scr->unk60[0]->x;
    scr->unkF8.vy = pos->y - scr->unk60[0]->y;
    scr->unkF8.vz = pos->z - scr->unk60[0]->z;
    vin = &scr->unkF8;
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : "r"(vin) : "$12", "memory");
    vout = &scr->unk100[0];
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(vout) : "$12", "memory");

    {
        s32 y;
        /* a0_var: r_sq - (x * x + y * y), then its square root (the
         * sphere's half-chord along z) (Ruling 11) */
        s32 a0_var;
        s32 sp_var;
        s32 min_z;
        s32 max_z;
        s32 az;
        s32 z;
        s32 x;
        s32 neg_threshold = -threshold;
        s32 sq;

        x = scr->unk100[0].x;
        if (x < neg_threshold || threshold < x)
            return 0;
        y = scr->unk100[0].y;
        if (y < neg_threshold || threshold < y)
            return 0;

        sq = x * x + y * y;
        if (r_sq < sq)
            return 0;
        a0_var = r_sq - sq;

        if ((u32)a0_var < 0x400) {
            a0_var = (u32)g_sqrt_table_u8[a0_var] >> 3;
        } else {
            s32 lzcr;
            if (a0_var < 0) {
                lzcr = 0;
            } else {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, $sp, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(a0_var) : "$12");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)a0_var >> shift];
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_z = 0;
        min_z = 0;
        az = scr->unkA8.z;
        if (az < 0) {
            min_z = az;
        } else {
            max_z = az;
        }
        z = scr->unk100[0].z;
        if (max_z < z - a0_var || z + a0_var < min_z)
            return 0;
        return 1;
    }
}

void func_8002EBDC(s16 *vec_in, s16 *dir, s32 *out, s32 scale_z, s32 scale_xy) {
    s32 sp_tmp;
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 angle1;
    s32 angle2;
    s32 dist_sq;
    s32 dist;

    angle1 = ratan2(dir[0], dir[2]);
    scr->unkF8.vy = 0x800 - angle1;
    dist_sq = dir[0] * dir[0] + dir[2] * dir[2];

    if ((u32)dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            /* gte_Lzc -- gtemac.h:174-178: gte_ldlzc, gte_nop, gte_nop,
             * gte_stlzc */
            __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            dist = (u32)(g_sqrt_table_u8[(u32)dist_sq >> shift] << 16) >>
                   (0x13 - ((u32)shift >> 1));
        }
    }

    angle2 = ratan2(dir[1], dist);
    mat = &scr->unkD8;
    scr->unkF8.vx = 0x800 - angle2;

    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixY(scr->unkF8.vy, mat);
    RotMatrixX(scr->unkF8.vx, mat);

    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(mat):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430; gte_rtv0's
     * command word is the post-DMPSX 0x4A486012 for the placeholder
     * 0x0000013f (Q61) */
    __asm__ volatile ("move  $12,%0": :"r"(vec_in):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    vec = &scr->unkA8;
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    vec->z = (vec->z * scale_z) / 256;
    vec->x = (vec->x * scale_xy) / 256;
    vec->y = (vec->y * scale_xy) / 256;

    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixX(-scr->unkF8.vx, mat);
    RotMatrixY(-scr->unkF8.vy, mat);

    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(mat):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430; gte_rtv0's
     * command word is the post-DMPSX 0x4A486012 for the placeholder
     * 0x0000013f (Q61) */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(out):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
}

void func_8002EECC(void *arg0, void *arg1) {
    MATRIX *src = arg0;
    MATRIX *dst = arg1;
    s16 temp_a3;
    s16 temp_t0;
    s16 temp_t1;
    s16 temp_t2;
    s16 temp_v0;
    s16 temp_v1;
    s32 temp_a2;
    s32 temp_v1_2;

    temp_t2 = src->m[1][2];
    temp_t1 = src->m[2][1];
    temp_t0 = src->m[1][1];
    temp_a3 = src->m[2][2];
    temp_a2 = (temp_t2 * temp_t1) - (temp_t0 * temp_a3);
    temp_v1 = src->m[0][1];
    temp_v0 = src->m[0][2];
    temp_v1_2 =
        ((src->m[0][0] * (temp_a2 >> 0xC)) +
         (src->m[1][0] * (((temp_v1 * temp_a3) - (temp_v0 * temp_t1)) >> 0xC)) +
         (src->m[2][0] *
          (((temp_v0 * temp_t0) - (temp_v1 * temp_t2)) >> 0xC))) >>
        0xC;
    dst->m[0][0] = (temp_a2 / temp_v1_2);
    dst->m[0][1] =
        (((src->m[0][1] * src->m[2][2]) - (src->m[0][2] * src->m[2][1])) /
         temp_v1_2);
    dst->m[0][2] =
        (((src->m[0][2] * src->m[1][1]) - (src->m[0][1] * src->m[1][2])) /
         temp_v1_2);
    dst->m[1][0] =
        (((src->m[1][0] * src->m[2][2]) - (src->m[1][2] * src->m[2][0])) /
         temp_v1_2);
    dst->m[1][1] =
        (((src->m[0][2] * src->m[2][0]) - (src->m[0][0] * src->m[2][2])) /
         temp_v1_2);
    dst->m[1][2] =
        (((src->m[0][0] * src->m[1][2]) - (src->m[0][2] * src->m[1][0])) /
         temp_v1_2);
    dst->m[2][0] =
        (((src->m[1][1] * src->m[2][0]) - (src->m[1][0] * src->m[2][1])) /
         temp_v1_2);
    dst->m[2][1] =
        (((src->m[0][0] * src->m[2][1]) - (src->m[0][1] * src->m[2][0])) /
         temp_v1_2);
    dst->m[2][2] =
        (((src->m[0][1] * src->m[1][0]) - (src->m[0][0] * src->m[1][1])) /
         temp_v1_2);
}

void func_8002F2D0(MATRIX *a0, s16 *a1);

void func_8002F2D0(MATRIX *a0, s16 *a1) {
    MATRIX *m;
    Unk1F8002B8Rec *scr;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 c0, c1, c2;
    /* work: the 3x3 determinant (divisor of the six cofactors), then the
     * square root of i0*i0 + i1*i1 (Ruling 11) */
    s32 work;
    s32 d0;
    s32 i0, i1, i2;
    s32 r0, r1, r2;
    s32 ang_z, ang_y;
    /* temp: i0*i0 + i1*i1, then the square-root table byte (Ruling 11) */
    s32 temp;
    s32 sp_tmp;

    m = &SPAD->unk2B8.rec.unkD8;
    *m = *a0;

    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    d0 = m->m[0][0] * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    work = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    i0 = c0 / work;
    i1 = c1 / work;
    i2 = c2 / work;
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / work;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / work;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / work;

    ang_z = -ratan2(i1, i0);
    scr = &SPAD->unk2B8.rec;
    temp = i0 * i0 + i1 * i1;
    if ((u32)temp < 0x400) {
        work = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        s32 lzcr = 0;
        if (temp >= 0) {
            /* gte_Lzc -- gtemac.h:174-178: gte_ldlzc, gte_nop, gte_nop,
             * gte_stlzc */
            __asm__ volatile ("move  $12,%0": :"r"(temp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            temp = g_sqrt_table_u8[(u32)temp >> shift];
            work = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    ang_y = ratan2(i2, work);
    mat = &scr->unkD8;
    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixZ(ang_z, mat);
    RotMatrixY(ang_y, mat);

    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(mat):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    vec = &scr->unkA8;
    vec->x = r0;
    vec->y = r1;
    vec->z = r2;
    /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430; gte_rtv0's
     * command word is the post-DMPSX 0x4A486012 for the placeholder
     * 0x0000013f (Q61) */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    a1[0] = ratan2(vec->z, vec->y);
    a1[1] = -ang_y;
    a1[2] = -ang_z;
}

void func_8002F770(s16 *angles, s32 z, s32 y, s32 x) {
    MATRIX *m;
    Unk1F8002B8Rec *scr;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 c0, c1, c2;
    /* work: the 3x3 determinant (divisor of the six cofactors), then the
     * square root of i0*i0 + i1*i1 (Ruling 11) */
    s32 work;
    s32 d0;
    s32 i0, i1, i2;
    s32 r0, r1, r2;
    s32 ang_z, ang_y;
    /* temp: i0*i0 + i1*i1, then the square-root table byte (Ruling 11) */
    s32 temp;
    s32 sp_tmp;

    scr = &SPAD->unk2B8.rec;
    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixX(x, &scr->unkD8);
    RotMatrixY(y, &scr->unkD8);
    RotMatrixZ(-z, &scr->unkD8);
    RotMatrixX(angles[0], &scr->unkD8);
    RotMatrixY(angles[1], &scr->unkD8);
    RotMatrixZ(angles[2], &scr->unkD8);

    m = &scr->unkD8;
    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    d0 = m->m[0][0] * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    work = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    i0 = c0 / work;
    i1 = c1 / work;
    i2 = c2 / work;
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / work;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / work;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / work;

    ang_z = -ratan2(i1, i0);
    temp = i0 * i0 + i1 * i1;
    if ((u32)temp < 0x400) {
        work = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        s32 lzcr = 0;
        if (temp >= 0) {
            /* gte_Lzc -- gtemac.h:174-178: gte_ldlzc, gte_nop, gte_nop,
             * gte_stlzc */
            __asm__ volatile ("move  $12,%0": :"r"(temp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            temp = g_sqrt_table_u8[(u32)temp >> shift];
            work = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    ang_y = ratan2(i2, work);
    mat = &scr->unkD8;
    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixZ(ang_z, mat);
    RotMatrixY(ang_y, mat);

    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(mat):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    vec = &scr->unkA8;
    vec->x = r0;
    vec->y = r1;
    vec->z = r2;
    /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430; gte_rtv0's
     * command word is the post-DMPSX 0x4A486012 for the placeholder
     * 0x0000013f (Q61) */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    angles[0] = ratan2(vec->z, vec->y);
    angles[1] = -ang_y;
    angles[2] = -ang_z;
}

s32 func_8002FC80(VECTOR *a0, VECTOR *a1, VECTOR *a2) {
    VECTOR *p;
    s32 ret;

    /* Difference vectors (a1 - a0) and (a2 - a0) into the scratchpad VECTORs
     * at SCR[0x60..0x68] / SCR[0x70..0x78], the GTE operands below. */
    ((VECTOR *)0x1F800360)->vx = a1->vx - a0->vx;
    ((VECTOR *)0x1F800360)->vy = a1->vy - a0->vy;
    ((VECTOR *)0x1F800360)->vz = a1->vz - a0->vz;
    ((VECTOR *)0x1F800370)->vx = a2->vx - a0->vx;
    ((VECTOR *)0x1F800370)->vy = a2->vy - a0->vy;
    ((VECTOR *)0x1F800370)->vz = a2->vz - a0->vz;

    /* gte_ldopv1(r) (the 3-word load, not gte_SetRotMatrix's 5): the packed
     * words at r into cop2 control R11R12 / R22R23 / R33 ($0 / $2 / $4). */
    __asm__ volatile(
        "move   $12, %0
"
        "lw     $13, 0($12)
"
        "lw     $14, 4($12)
"
        "ctc2   $13, $0
"
        "lw     $15, 8($12)
"
        "ctc2   $14, $2
"
        "ctc2   $15, $4
"
        :: "r"((VECTOR *)0x1F800360) : "$12", "$13", "$14", "$15");
    /* gte_ldopv2(r) (not gte_ldlvl, which loads IR1 first): r into IR1-IR3
     * ($9/$10/$11), IR3 first; then gte_op0's two nops. */
    __asm__ volatile(
        "move   $12, %0
"
        "lwc2   $11, 8($12)
"
        "lwc2   $9, 0($12)
"
        "lwc2   $10, 4($12)
"
        "nop
"
        "nop
"
        :: "r"((VECTOR *)0x1F800370) : "$12");
    /* GTE OP (cross product of the IR vector with the rotation matrix
     * diagonal), sf=0: cop2 command 0x0170000C. */
    __asm__ volatile(".word 0x4B70000C");
    /* gte_stlvnl(r): MAC1/MAC2/MAC3 ($25/$26/$27) to r. */
    p = (VECTOR *)0x1F800380;
    __asm__ volatile(
        "move   $12, %0
"
        "swc2   $25, 0($12)
"
        "swc2   $26, 4($12)
"
        "swc2   $27, 8($12)
"
        :: "r"(p) : "$12");
    /* Angle of the cross product in the XZ plane, +0x800 (180 deg) when its
     * Y component is positive. */
    ret = ratan2(p->vx, p->vz);
    if (p->vy > 0) {
        ret += 0x800;
    }
    return ret;
}

s32 func_8002FDB0(Unk80101EC8Record *arg0) {
    LeafPos *pt;

    /* The character's points 2 and 3 relative to its point 1, into the
     * collision record's unkA8 and unkB8 (0x1F800360 / 0x1F800370, the GTE
     * operands below); the GTE's product lands in unkC8. */
    pt = SPAD->unkA8[arg0->index];
    SPAD->unk2B8.rec.unkA8.x = pt[2].x - pt[1].x;
    SPAD->unk2B8.rec.unkA8.y = pt[2].y - pt[1].y;
    SPAD->unk2B8.rec.unkA8.z = pt[2].z - pt[1].z;
    SPAD->unk2B8.rec.unkB8.x = pt[3].x - pt[1].x;
    SPAD->unk2B8.rec.unkB8.y = pt[3].y - pt[1].y;
    SPAD->unk2B8.rec.unkB8.z = pt[3].z - pt[1].z;

    /* gte_ldopv1(r) (the 3-word load, not gte_SetRotMatrix's 5): the packed
     * words holding the diagonal into cop2 control R11R12 / R22R23 / R33
     * ($0 / $2 / $4). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"((s32 *)0x1F800360) : "$12", "$13", "$14", "$15");
    /* gte_ldopv2(r) (not gte_ldlvl, which loads IR1 first): r into IR1-IR3
     * ($9/$10/$11), IR3 first; then gte_op0's two nops. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"((s32 *)0x1F800370) : "$12");
    /* GTE OP (cross product of the IR vector with the rotation matrix
     * diagonal), sf=0: cop2 command 0x0170000C. */
    __asm__ volatile(".word 0x4B70000C");
    /* gte_stlvnl(r): MAC1/MAC2/MAC3 ($25/$26/$27) to r. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"((s32 *)0x1F800380) : "$12");
    /* unkC8.y (MAC2) > 0 */
    return 0 < SPAD->unk2B8.rec.unkC8.y;
}

/* GTE islands: the PsyQ 4.5 inline_c.h macro bodies gte_SetRotMatrix,
 * gte_ldlv0, gte_rtv0 (MVMVA `.word 0x4A486012`), gte_stlvnl (owner Ruling A
 * 2026-09-02, cop2-addressing-preamble-cluster); the three operand islands
 * (gte_SetRotMatrix, gte_ldlv0, gte_stlvnl) carry the materialize-then-copy
 * preamble the widened cop2 owner grant of 2026-09-01 covers
 * (cop2-addressing-preamble-cluster). `vec` is one local for the gte_ldlv0
 * (long-VECTOR gte_ldv0) and gte_stlvnl operands, so its address is
 * materialized once. */
void func_8002FF20(Obj80106A78 *arg0, s16 arg1) {
    s32 mat_local[8];
    s32 *playerData;
    s32 *s2_ptr;
    MATRIX *rot_mat;
    Vec3i32 *vec;

    arg0->unk_08 = 1;
    arg0->unk_09 = arg1;
    playerData = func_80046DEC(arg0->owner < 1);
    rot_mat = &arg0->mtx;
    s2_ptr = (s32 *)playerData[arg0->unk_09];

    /* 3x3 identity rotation in the record's matrix. */
    arg0->mtx.m[0][0] = 0x1000;
    arg0->mtx.m[0][1] = 0;
    arg0->mtx.m[0][2] = 0;
    arg0->mtx.m[1][0] = 0;
    arg0->mtx.m[1][1] = 0x1000;
    arg0->mtx.m[1][2] = 0;
    arg0->mtx.m[2][0] = 0;
    arg0->mtx.m[2][1] = 0;
    arg0->mtx.m[2][2] = 0x1000;
    RotMatrixX(arg0->rot[0], rot_mat);
    RotMatrixY(arg0->rot[1], rot_mat);
    RotMatrixZ(arg0->rot[2], rot_mat);
    func_8002EECC(s2_ptr, mat_local);
    MulMatrix0((MATRIX *)mat_local, rot_mat, rot_mat);

    /* Subtract opponent reference position from self position. */
    arg0->pos.x -= s2_ptr[5];
    arg0->pos.y -= s2_ptr[6];
    arg0->pos.z -= s2_ptr[7];

    /* gte_SetRotMatrix(r0) -- PsyQ 4.5 inline_c.h: the 5 packed
     * rotation-matrix words into cop2 control $0..$4. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat_local) : "$12", "$13", "$14", "$15");
    vec = &arg0->pos;
    /* gte_ldlv0(r0) -- PsyQ 4.5 inline_c.h:101-110: VX0/VY0 packed into
     * one word, VZ0 into $1; the 2-cycle GTE load delay as explicit nops. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "$13", "$14");
    /* PsyQ 4.5 inline_c.h macro gte_rtv0(): MVMVA sf=1, mx=rotation,
     * v=V0, cv=none (`.word 0x4A486012`). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ 4.5 inline_c.h macro gte_stlvnl(r0) in the `move $12,%0`
     * spelling: MAC1/MAC2/MAC3 ($25/$26/$27) to r0. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12");

    /* Halve x, y, z (signed arithmetic shift). */
    arg0->pos.x >>= 1;
    arg0->pos.y >>= 1;
    arg0->pos.z >>= 1;
}

/* GTE rotate+translate of the object's local vector, then dispatch.
 * GTE islands: the PsyQ 4.5 inline_c.h macros gte_SetRotMatrix, gte_ldlv0,
 * gte_rtv0 (MVMVA `.word 0x4A486012`), gte_stlvnl; the three operand islands
 * use the older-SDK `move $12,%0` materialize-then-copy spelling (owner cop2
 * cluster grant,
 * cop2-addressing-preamble-cluster); gte_ldlv0's lhu/lhu/sll/or pack is the
 * macro's own text (inline_c.h:101-110; owner Ruling A 2026-09-02,
 * cop2-addressing-preamble-cluster). One
 * FAKE: the do-while(0) wrap around gte_stlvnl (see its annotation;
 * without it score 13). */
void func_800300B4(Obj80106A78 *arg0) {
    s32 mac[3];
    s16 dir[4];
    MATRIX mtx;
    s32 *playerData;
    MATRIX *mat;
    s32 lookup;

    playerData = func_80046DEC(arg0->owner < 1);
    mat = (MATRIX *)playerData[arg0->unk_09];

    /* PsyQ 4.5 inline_c.h macro gte_SetRotMatrix(r): the 5 packed
     * rotation-matrix words into cop2 control R11R12..R33. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    /* gte_ldlv0(r) -- PsyQ 4.5 inline_c.h:101-110: the VECTOR at r into V0
     * (vx/vy packed into VXY0, vz into VZ0); then the 2-cycle GTE load delay
     * and gte_rtv0() (MVMVA sf=1, rotation x V0, no translation). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(&arg0->pos) : "$12", "$13", "$14");
    /* PsyQ 4.5 inline_c.h macro gte_rtv0(): MVMVA sf=1, mx=rotation,
     * v=V0, cv=none (cop2 command 0x0486012). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ 4.5 inline_c.h macro gte_stlvnl(r): MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    /* FAKE: do-while(0) around gte_stlvnl weights &mac's refs so it
     * takes $s2 ahead of arg0; without it score 13
     * (do-while-zero-exception) */
    do {
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(mac) : "$12", "memory");
    } while (0);

    /* Add the matrix translation to the rotated vector. */
    mac[0] += mat->t[0];
    mac[1] += mat->t[1];
    mac[2] += mat->t[2];

    MulMatrix0(mat, &arg0->mtx, &mtx);
    func_8002F2D0(&mtx, dir);

    lookup = D_8008EB80[arg0->kind];
    func_80049718(lookup, 1, mac, dir);
    func_800393C8(arg0->slot, lookup, mac, dir);
}

void func_80030208(void) {
    Obj80106A78 *obj;
    s32 i;
    s16 kind;
    s32 lookup;

    for (obj = D_80106A78, i = 0; i < 12; i++, obj++) {
        kind = obj->kind;
        if (kind == -1) {
            continue;
        }
        if (obj->unk_08 != 0) {
            func_800300B4(obj);
            continue;
        }
        if (obj->unk_00 < 2) {
            continue;
        }
        lookup = D_8008EB80[kind];
        if (kind >= 0x12 && kind < 0x1E) {
            lookup = obj->unk_0B;
        } else if (kind == 0xE && obj->unk_05 == 2) {
            lookup += 3;
        }
        func_80049718(lookup, 1, &obj->pos.x, obj->rot);
        func_800393C8(obj->slot, lookup, &obj->pos, obj->rot);
    }
}

void func_8003032C(s32 *a0, s16 *a1) {
    s32 angle;
    s32 cos_val;
    s32 sin_val;
    s32 vx;
    s32 vz;
    s32 rx;
    s32 rz;
    s32 v48;
    angle = ratan2(a1[0], a1[2]);
    cos_val = Judge[(angle + 0x400) & 0xFFF];
    sin_val = Judge[angle & 0xFFF];
    vx = a0[0x11];
    vz = a0[0x13];
    rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
    rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
    v48 = a0[0x12];
    a0[0x11] = ((rx * cos_val) - (rz * sin_val)) >> 15;
    a0[0x13] = ((rx * sin_val) + (rz * cos_val)) >> 15;
    if (v48 < 0) {
        v48 += 3;
    }
    a0[0x12] = v48 >> 2;
}

void func_8003043C(void) {
    s32 i;

    for (i = 0; i < 12; i++) {
        D_80106A78[i].kind = -1;
        D_80106A78[i].slot = 0xFF;
    }
}

void func_8003047C(Unk80101EC8Record *a0) {
    s32 i;
    s16 val;

    a0->unk_330 = 0;
    i = 0;
    do {
        val = D_8008E338[a0->unk_0A][i];
        a0->unk_332[i] = val;
        if (val != -1) {
            a0->unk_330++;
        }
        i++;
    } while (i < 5);
    D_800A36F2[a0->index] = D_8008E338[a0->unk_0A][0];
}

void func_80030524(void) {
    s32 i = 0;
    do {
        if (D_80106A78[i].kind != -1) {
            if (D_80106A78[i].unk_08 != 0) {
                D_80106A78[i].kind = -1;
            }
        }
    } while (++i < 12);
}

Obj80106A78 *func_80030580(Unk80101EC8Record *arg0, s32 arg1) {
    /* FAKE: unwritten pad reserving the 16 untouched local bytes of the
     * target frame (target vars=24, ours 8 without the pad) (phantom-frame pad,
     * owner ruling 2026-08-18; row granted by owner ruling 2026-09-02). SOTN
     * precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. */
    volatile u32 pre_pad[4]; /* !FAKE: without it score 2 */
    Obj80106A78 *obj;
    Tbl8008E194 *tbl;
    s32 i;

    obj = D_80106A78;
    for (i = 0; i < 12; i++, obj++) {
        if (obj->kind == -1 && obj->slot == 0xFF)
            break;
    }
    obj->slot = i;
    obj->kind = arg1;
    obj->unk_07 = 0;
    obj->unk_08 = 0;
    obj->unk_04 = 1;
    obj->owner = arg0->index;
    obj->pos.x = arg0->unk_F4.x;
    obj->pos.y = arg0->unk_F4.y - arg0->unk_1A / 32;
    obj->pos.z = arg0->unk_F4.z;
    tbl = &D_8008E194[arg1];
    obj->vel.x = (Judge[arg0->unk_1C8.vy & 0xFFF] * tbl->unk4) >> 12;
    obj->vel.y = tbl->unk6;
    obj->vel.z = (Judge[(arg0->unk_1C8.vy + 0x400) & 0xFFF] * tbl->unk4) >> 12;
    obj->pos.x += obj->vel.x;
    obj->pos.y += obj->vel.y;
    obj->pos.z += obj->vel.z;
    obj->pos.x += obj->vel.x / 2;
    obj->pos.y += obj->vel.y / 2;
    obj->pos.z += obj->vel.z / 2;
    obj->prev_pos = obj->pos;
    obj->rot[0] = 0;
    obj->rot[1] = arg0->unk_1C8.vy;
    obj->rot[2] = 0;
    if (tbl->unk0 == 1) {
        obj->rot_vel[0] = 0;
        obj->rot_vel[1] = tbl->unk8;
        obj->rot_vel[2] = 0;
    } else if (tbl->unk0 == 2) {
        obj->rot_vel[0] = tbl->unk8;
        obj->rot_vel[1] = 0;
        obj->rot_vel[2] = 0;
    } else if (tbl->unk0 == 3) {
        obj->rot_vel[0] = 0;
        obj->rot_vel[1] = tbl->unk8;
        obj->rot_vel[2] = 0;
    } else {
        obj->rot_vel[0] = 0;
        obj->rot_vel[1] = 0;
        obj->rot_vel[2] = 0;
    }
    obj->unk_50 = 1;
    obj->unk_05 = 0;
    obj->unk_00 = 0;
    return obj;
}

extern Obj80106A78 *func_80030580(Unk80101EC8Record *, s32);

s32 func_800307D0(Unk80101EC8Record *a0) {
    s32 count;
    s32 idx;
    s32 kind;
    s32 id;
    Obj80106A78 *obj;
    s32 i;

    count = a0->unk_330;
    if (count == 0) {
        return -1;
    }
    idx = 0;
    if (count >= 2 && a0->unk_88 != -1) {
        idx = a0->unk_332[0] == a0->unk_14;
    }
    id = a0->unk_332[idx];
    obj = func_80030580(a0, id);
    for (i = idx; i < a0->unk_330 - 1; i++) {
        a0->unk_332[i] = a0->unk_332[i + 1];
    }

    a0->unk_330--;
    kind = obj->kind;
    if (kind == 0xE) {
        func_80032854((D_800A36F2[0] ^ 0xE) != 0, 0x2F, &obj->pos.x, 0);
    } else {
        func_80032854((kind ^ D_800A36F2[0]) != 0, 0x2A, &obj->pos.x, 0);
    }
    return id;
}

typedef struct {
    s32 x, y, z;
} Vec3_copy;

extern Obj80106A78 *func_80030580(Unk80101EC8Record *, s32);

void func_80030900(Unk80101EC8Record *a0, Vec3i32 *a1) {
    Obj80106A78 *p;
    s32 rnd;
    s32 i;

    p = func_80030580(a0, a0->unk_332[0]);
    p->unk_04 = 0;
    p->pos = *a1;
    p->vel.x = (rng_Next() & 0xFF) - 0x80;
    p->vel.y = -(rng_Next() & 0x3F) - 0x80;
    p->vel.z = (rng_Next() & 0xFF) - 0x80;
    rnd = rng_Next();
    if (rnd & 0x1000) {
        p->rot_vel[0] = (rnd & 0x3FF) + 0x200;
    } else {
        p->rot_vel[0] = -(rnd & 0x3FF) - 0x200;
    }
    p->rot_vel[1] = (rng_Next() & 0x7FF) - 0x400;
    p->rot_vel[2] = 0;
    p->unk_07 = 1;
    for (i = 0; i < a0->unk_330 - 1; i++) {
        a0->unk_332[i] = a0->unk_332[i + 1];
    }
    a0->unk_330--;
}

void func_80030A2C(Unk80101EC8Record *a0, s32 a1, Vec3i32 *a2) {
    Obj80106A78 *p;
    s32 rnd;

    p = func_80030580(a0, a1);
    p->unk_04 = 0;
    p->pos = *a2;
    p->vel.x = (rng_Next() & 0xFF) - 0x80;
    p->vel.y = -(rng_Next() & 0x3F) - 0x80;
    p->vel.z = (rng_Next() & 0xFF) - 0x80;
    rnd = rng_Next();
    if (rnd & 0x1000) {
        p->rot_vel[0] = (rnd & 0x3FF) + 0x200;
    } else {
        p->rot_vel[0] = -(rnd & 0x3FF) - 0x200;
    }
    p->rot_vel[1] = (rng_Next() & 0x7FF) - 0x400;
    p->rot_vel[2] = 0;
    p->unk_07 = 1;
    p->unk_0B = a0->unk_12;
}

s32 func_80030B10(Unk80101EC8Record *arg0, s32 arg1) {
    s32 count = arg0->unk_330;
    s16 c;
    if (count == 0xC) {
        return 0;
    }
    if (arg0->unk_88 == -1) {
        goto skip_shift;
    }
    if (arg1 != arg0->unk_14) {
        goto skip_shift;
    }
    if (count > 0) {
        s32 i = count;
        do {
            arg0->unk_332[i] = arg0->unk_332[i - 1];
            i--;
        } while (i > 0);
    }
    {
        s16 cc = arg0->unk_330;
        arg0->unk_332[0] = arg1;
        arg0->unk_330 = cc + 1;
        goto done;
    }
skip_shift:
    c = arg0->unk_330;
    arg0->unk_330 = c + 1;
    arg0->unk_332[c] = arg1;
done:
    return 1;
}

s32 func_80030BA8(Unk80101EC8Record *arg0) {
    Obj80106A78 *p;
    s32 i;
    s32 old_val;

    for (i = 0, p = D_80106A78; i < 12; i++, p++) {
        s16 kind = p->kind;
        if (kind >= 0x12 && kind < 0x1E) {
            continue;
        }
        if (kind == -1) {
            continue;
        }
        if (p->unk_50 != 0) {
            continue;
        }
        {
            s32 bc = arg0->unk_B8.vy;
            s32 py = p->pos.y;
            if (bc - 0x64 >= py) {
                continue;
            }
            if (py >= bc + 0x64) {
                continue;
            }
        }
        {
            s32 dx = arg0->unk_F4.x - p->pos.x;
            s32 dz = arg0->unk_F4.z - p->pos.z;
            if (dx * dx + dz * dz > 0xF423F) {
                continue;
            }
        }
        if (func_80030B10(arg0, kind) == 0) {
            return -1;
        }
        old_val = p->kind;
        p->kind = -1;
        if (old_val == 0xE) {
            func_80032854((D_800A36F2[0] ^ 0xE) != 0, 0x2F, &arg0->unk_F4.x, 0);
        } else {
            func_80032854(arg0->index, 0x11, &arg0->unk_F4.x, 0);
        }
        return old_val;
    }
    return -1;
}

void func_80030D04(void) {
    s32 i = 0;
    do {
        if (D_80106A78[i].kind >= 0x12 && D_80106A78[i].kind < 0x1E) {
            D_80106A78[i].kind = -1;
        }
    } while (++i < 12);
}

void func_80030D48(void) {}

s32 math_LerpAngle(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = (arg0 - arg1) & 0xFFF;
    if (arg0 >= 0x800) {
        arg0 -= 0x1000;
    }
    return arg1 + ((arg0 * arg2) >> 12);
}

/* Per-frame update of the twelve 0x64-byte records at D_80106A78: velocity
 * turn, spin decay, gravity, collision test through func_8005344C, reflection
 * off the returned normal, func_80032854 cues, and the rest / re-hop logic.
 * Scratch vectors live in its layout of the scratchpad area at 0x1F8002B8
 * (Unk1F8002B8_8005344C). */
void func_80030D7C(void) {
    Unk1F8002B8_8005344C *scr;
    Obj80106A78 *obj;
    s32 i;
    s32 half;
    s32 dot;
    s32 spd;
    s16 kind;
    s16 *nrm;

    scr = &SPAD->unk2B8.v8005344C;
    obj = D_80106A78;
    for (i = 0; i < 12; i++, obj++) {
        /* work: the clamped turn amount (turn block), then the bounce
         * restitution factor D_8008E194[kind].unkA (Ruling 11,
         * ordinary-c-judge-decidable) */
        s32 work;
        /* temp: the ratan2() heading of the velocity (turn block), then
         * the func_8005344C() collision result (cleared when func_80054434() ==
         * 7) (Ruling 11, ordinary-c-judge-decidable) */
        s32 temp;

        if (obj->kind == -1) {
            obj->slot = 0xFF;
            continue;
        }
        if (obj->unk_08 != 0) {
            continue;
        }
        obj->prev_pos = obj->pos;
        if (obj->unk_50 != 0) {
            obj->unk_00++;
        }
        if (obj->kind == 0x10 && obj->unk_50 != 0 && obj->unk_05 == 0 &&
            obj->unk_00 >= 14) {
            temp = ratan2(obj->vel.x, obj->vel.z);
            work = (0x4E - obj->unk_00) * 96 / 64;
            if (work < 0) {
                work = 0;
            } else if (work > 0x42) {
                work = 0x42;
            }
            scr->unk10.x = (Judge[(temp + 0x400) & 0xFFF] * obj->vel.x -
                            Judge[temp & 0xFFF] * obj->vel.z) >>
                           12;
            scr->unk10.y = obj->vel.y;
            scr->unk10.z = (Judge[temp & 0xFFF] * obj->vel.x +
                            Judge[(temp + 0x400) & 0xFFF] * obj->vel.z) >>
                           12;
            scr->unk20.x = scr->unk10.x;
            half = work / 2;
            scr->unk20.y = (Judge[(half + 0x400) & 0xFFF] * scr->unk10.y -
                            Judge[half & 0xFFF] * scr->unk10.z) >>
                           12;
            scr->unk20.z = (Judge[half & 0xFFF] * scr->unk10.y +
                            Judge[(half + 0x400) & 0xFFF] * scr->unk10.z) >>
                           12;
            obj->vel.x = (Judge[(work - temp + 0x400) & 0xFFF] * scr->unk20.x -
                          Judge[(work - temp) & 0xFFF] * scr->unk20.z) >>
                         12;
            obj->vel.y = scr->unk20.y;
            obj->vel.z =
                (Judge[(work - temp) & 0xFFF] * scr->unk20.x +
                 Judge[(work - temp + 0x400) & 0xFFF] * scr->unk20.z) >>
                12;
            obj->rot_vel[0] = obj->rot_vel[0] * 63 / 64;
            obj->rot_vel[1] = obj->rot_vel[1] * 63 / 64;
            obj->rot_vel[2] = obj->rot_vel[2] * 63 / 64;
        } else {
            obj->rot_vel[0] = obj->rot_vel[0] * 15 / 16;
            obj->rot_vel[1] = obj->rot_vel[1] * 15 / 16;
            obj->rot_vel[2] = obj->rot_vel[2] * 15 / 16;
        }
        obj->rot[0] += obj->rot_vel[0];
        obj->rot[1] += obj->rot_vel[1];
        obj->rot[2] += obj->rot_vel[2];
        obj->vel.y += 13;
        scr->unk00.x = obj->pos.x + obj->vel.x;
        scr->unk00.y = obj->pos.y + obj->vel.y;
        scr->unk00.z = obj->pos.z + obj->vel.z;
        obj->pos.y -= 8;
        nrm = scr->unk30;
        temp = func_8005344C(
            &obj->pos.x, &scr->unk00.x, &scr->unk10.x, nrm, (s32)&scr->unk38);
        if (temp != 0 && func_80054434() == 7) {
            temp = 0;
        }
        if (temp != 0) {
            if (obj->kind == 0xF) {
                func_80032854(obj->owner, 0xE, &scr->unk10.x, nrm);
                obj->kind = -1;
                continue;
            }
            dot = (obj->vel.x * nrm[0] + obj->vel.y * nrm[1] +
                   obj->vel.z * nrm[2]) /
                  2048;
            obj->vel.x -= nrm[0] * dot / 4096;
            obj->vel.y -= nrm[1] * dot / 4096;
            obj->vel.z -= nrm[2] * dot / 4096;
            obj->pos = scr->unk10;
            kind = obj->kind;
            work = D_8008E194[kind].unkA;
            if (obj->unk_50 != 0) {
                obj->vel.x = obj->vel.x * work / 4096;
                obj->vel.y = obj->vel.y * work / 4096;
                obj->vel.z = obj->vel.z * work / 4096;
                spd = obj->vel.x * obj->vel.x + obj->vel.z * obj->vel.z;
                if (scr->unk30[1] >= -0x7FF) {
                    obj->rot_vel[1] += (rng_Next() & 1) ? spd / 64 : -spd / 64;
                    if (obj->kind != 0xE && obj->unk_04 != 0) {
                        func_80032854(obj->owner, 1, &obj->pos.x, 0);
                    }
                }
                if (obj->kind == 0xE) {
                    if (obj->unk_04 != 0) {
                        func_80032854((obj->kind ^ D_800A36F2[0]) != 0, 0x2F,
                                      &obj->pos.x, 0);
                    }
                } else if (obj->kind < 0x12) {
                    if (spd > 0x10) {
                        func_80032854((obj->kind ^ D_800A36F2[0]) != 0, 0x2C,
                                      &obj->pos.x, 0);
                    }
                } else if (obj->kind >= 0x12 && obj->kind < 0x1E) {
                    if (spd > 0x10) {
                        func_80032854(obj->owner, 0x29, &obj->pos.x, 0);
                    }
                }
            } else if (scr->unk30[1] >= -0x7FF && kind == 0xE) {
                if (obj->unk_05 == 1) {
                    obj->unk_50 = 1;
                    obj->rot[1] += 0x780 + (rng_Next() & 0xFF);
                    obj->vel.x = Judge[obj->rot[1] & 0xFFF] / 64;
                    obj->vel.y = -150;
                    obj->vel.z = Judge[(obj->rot[1] + 0x400) & 0xFFF] / 64;
                    obj->unk_05 = 2;
                    func_80032854(
                        (obj->kind ^ D_800A36F2[0]) != 0, 0x2F, &obj->pos.x, 0);
                }
            }
            obj->unk_04 = 0;
            if (obj->unk_07 == 1) {
                obj->unk_07 = 2;
            }
            if (obj->unk_05 == 2) {
                obj->unk_05 = 3;
            }
        } else {
            obj->pos = scr->unk00;
        }
        if (obj->vel.y >= -15 && obj->vel.y <= 15 && obj->vel.x >= -3 &&
            obj->vel.x <= 3 && obj->vel.z >= -3 && obj->vel.z <= 3) {
            if (temp != 0) {
                if (D_800A38DC == 3) {
                    obj->kind = -1;
                } else {
                    obj->unk_50 = 0;
                    obj->unk_05 = 1;
                    obj->vel.x = 0;
                    obj->vel.y = 0;
                    obj->vel.z = 0;
                }
            }
        } else {
            obj->unk_50 = 1;
        }
        if (obj->kind == 0xE) {
            if (obj->unk_05 == 1 && !(rng_Next() & 0x133)) {
                obj->unk_50 = 1;
                obj->rot_vel[1] += (rng_Next() & 0x7F) - 0x40;
                obj->vel.x = Judge[obj->rot[1] & 0xFFF] / 64;
                obj->vel.y = -150;
                obj->vel.z = Judge[(obj->rot[1] + 0x400) & 0xFFF] / 64;
                obj->unk_05 = 2;
                func_80032854(
                    (obj->kind ^ D_800A36F2[0]) != 0, 0x2F, &obj->pos.x, 0);
            }
        }
        if (obj->unk_07 == 2) {
            obj->rot[1] = math_LerpAngle(obj->rot[1], 0, 0x800);
            obj->rot[2] = math_LerpAngle(obj->rot[2], 0x400, 0xE00);
            obj->rot_vel[1] = obj->rot_vel[1] * 3 / 4;
        } else if (obj->unk_04 == 0 && D_8008E194[obj->kind].unk0 == 2) {
            obj->rot[0] = math_LerpAngle(obj->rot[0], 0, 0xE00);
            obj->rot[1] = math_LerpAngle(obj->rot[1], 0, 0x600);
            obj->rot[2] = math_LerpAngle(obj->rot[2], -0x400, 0xE00);
        }
        if (obj->pos.y > 0x3A98) {
            obj->kind = -1;
        }
    }
    func_80030208();
}

/* GTE rotate-velocity-by-table-angle (sibling of func_8002E838) */
void func_80031890(Unk1F8002B8Rec *scr, Obj80106A78 *ent, s32 idx) {
    MATRIX *mat;
    Vec3i32 *vec;
    s32 angle1;
    s32 angle2;
    s32 sum_sq;
    s32 adj;

    if (ent->kind != 0xE) {
        s32 vx = ent->vel.x;
        s32 vz = ent->vel.z;
        s32 av = ent->rot_vel[1];
        sum_sq = vx * vx + vz * vz;
        if (rng_Next() & 1) {
            adj = sum_sq / 64;
        } else {
            adj = -sum_sq / 64;
        }
        ent->rot_vel[1] = av + adj;
    }

    mat = &scr->unkD8;
    angle1 = D_8008EBA0[idx] & 0xFFF;
    angle2 =
        (((ent->pos.x * 16) + ent->pos.y + (ent->pos.z * 8)) & 0x7FF) - 0x400;
    /* identity 3x3 rotation matrix at scr->unkD8 */
    scr->unkD8.m[0][0] = 0x1000;
    scr->unkD8.m[0][1] = 0;
    scr->unkD8.m[0][2] = 0;
    scr->unkD8.m[1][0] = 0;
    scr->unkD8.m[1][1] = 0x1000;
    scr->unkD8.m[1][2] = 0;
    scr->unkD8.m[2][0] = 0;
    scr->unkD8.m[2][1] = 0;
    scr->unkD8.m[2][2] = 0x1000;
    RotMatrixY(angle1, mat);
    RotMatrixX(angle2, mat);

    /* gte_SetRotMatrix(r): the 5 packed rotation-matrix words into cop2
     * control $0..$4 (as in func_800203B4 / func_8002E838). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    vec = &ent->vel;
    /* PsyQ libgte inline macro gte_ldlv0(r) (the long-vector gte_ldv0) ---
     * pack VX0/VY0 into one word, mtc2 to $0, lwc2 VZ0 into $1, then the
     * 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "$13", "$14");
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r (rotated vector written back in place). */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");

    if ((u32)(angle1 - 0x401) < 0x7FFU) {
        ent->vel.x /= 8;
        ent->vel.y /= 8;
        ent->vel.z /= 8;
    } else {
        ent->vel.x /= 4;
        ent->vel.y /= 4;
        ent->vel.z /= 4;
    }
    ent->pos.x += ent->vel.x / 2;
    ent->pos.y += ent->vel.y / 2;
    ent->pos.z += ent->vel.z / 2;
}

extern s32 func_80027AD8(
    s32, Unk80101EC8Record *, s32, s32, s32, Tbl8008E194 *, s32, s32 *);

void func_80031B24(void) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    s32 i;
    s32 deep;
    Obj80106A78 *obj;
    Vec3i32 *seg = scr->unk00.unk00;
    Unk80101EC8Record *ch;
    s32 other;
    u16 st;
    BoneHitRec *rec;
    s32 j;
    s32 hit;
    s32 diff;
    s32 r;
    s32 kind;
    s32 flag;

    obj = D_80106A78;
    for (i = 0; i < 12; i++, obj++) {
        if (obj->kind == -1)
            continue;
        if (obj->unk_04 == 0)
            continue;
        if (obj->unk_50 == 0)
            continue;
        other = obj->owner == 0;
        ch = &D_80101EC8[other];
        st = ch->unk_6A;
        if (st == 4 || st == 0x14 || st == 0xF || st == 0x1C || st == 0x1D ||
            st == 0x1E || st == 0x1F || st == 0x20 || st == 0x21 ||
            st == 0x11) {
            continue;
        }

        seg[0] = obj->prev_pos;
        seg[1] = obj->pos;
        scr->unk60[0] = &seg[0];
        scr->unk60[1] = &seg[1];
        func_8002E838(scr);

        hit = 0;
        rec = D_800F5F68[other];
        for (j = 0; j < 22; j++, rec++) {
            LeafPos *pos;
            if (ch->unk_26C == 0 && j >= 6 && j <= 9)
                continue;
            pos = &SPAD->unkA8[other][j];
            hit = func_8002EA24(scr, pos, rec->unk_0C, rec->unk_0E);
            if (hit != 0) {
                deep = 0;
                if (rec->unk_00 != 0 && D_8008E194[obj->kind].unkD == 0) {
                    deep =
                        func_8002EA24(scr, pos, rec->unk_10, rec->unk_12) != 0;
                }
                break;
            }
        }
        if (hit == 0)
            continue;

        diff = (ch->unk_1C8.vy - ratan2(obj->vel.x, obj->vel.z)) & 0xFFF;
        if (diff >= 0x800)
            diff = 0x1000 - diff;
        func_800274BC(
            (s32 *)&obj->vel,
            /* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector reached
             * by the first's address, passed to func_800274BC (Q96 / Q117; as
             * s16[3] or {x,y,z}, func_80027AD8 scores 2) */
            &D_800A37E8);
        obj->pos.x -= obj->vel.x / 2;
        obj->pos.y -= obj->vel.y / 2;
        obj->pos.z -= obj->vel.z / 2;
        r = func_80027AD8(
            1, ch, j, diff, deep, &D_8008E194[obj->kind], 0, &flag);
        if (r == 2)
            continue;
        if (r != 0) {
            func_80032854((obj->kind ^ D_800A36F2[0]) != 0, 0x2B,
                          &SPAD->unkA8[other][j].x, 0);
            func_8002FF20(obj, rec->bone);
            obj->unk_04 = 0;
            st = ch->unk_6A;
            if (st == 8 || st == 0x23) {
                g_disp_fade = 1;
            }
            continue;
        }
        kind = obj->kind;
        if (kind == 0xF) {
            func_80032854(
                other ^ 1, 0xE, &SPAD->unkA8[other][j].x,
                /* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector reached
                 * by the first's address, passed to func_80032854 (Q96 / Q117;
                 * as s16[3] or {x,y,z}, func_80027AD8 scores 2) */
                &D_800A37E8);
            obj->kind = -1;
            continue;
        }
        if (kind == 0xE) {
            func_80032854((D_800A36F2[0] ^ 0xE) != 0, 0x2F, &obj->pos.x, 0);
        } else if (flag == 0) {
            func_80032854(
                (kind ^ D_800A36F2[0]) != 0, 0x2B, &SPAD->unkA8[other][j].x, 0);
        }
        func_80031890(scr, obj, j);
        obj->unk_04 = 0;
    }
}

void func_80032040(void) {
    s32 i;
    for (i = 0; i < 4; i++) {
        D_80104E88[i].unk_00 = 0;
    }
}

Unk80104E88Rec *func_80032064(Unk80101EC8Record *src, s32 type) {
    /* FAKE: constant holders for the speed (0x50) and the initial y velocity
     * (-0xC8); as literals the li pair reorders (speed: score 12; vel_y: score
     * 7). */
    s32 speed = 0x50;
    s32 vel_y = -0xC8;
    s32 i = 0;
    Unk80104E88Rec *ptr = D_80104E88;
    Unk80104E88Rec *s0;
    s16 sp_area[3];

    for (; i < 4; i++) {
        s0 = ptr;
        if (s0->unk_00 == 0)
            break;
        ptr = s0 + 1;
    }
    if (i == 4)
        return 0;

    s0->unk_00 = type;
    s0->unk_01 = 1;
    s0->unk_02 = 0;
    s0->unk_03 = src->index;
    s0->unk_04.x = src->unk_F4.x;
    {
        s32 v1 = src->unk_1A;
        if (v1 < 0)
            v1 += 0x1F;
        s0->unk_04.y = src->unk_B8.vy - (v1 >> 5);
    }
    s0->unk_04.z = src->unk_F4.z;
    s0->unk_1C.x = ((s32)Judge[src->unk_1C8.vy & 0xFFF] * speed) >> 12;
    s0->unk_1C.y = vel_y;
    s0->unk_1C.z =
        ((s32)Judge[(src->unk_1C8.vy + 0x400) & 0xFFF] * speed) >> 12;
    s0->unk_10 = s0->unk_04;
    s0->unk_28 = src->unk_B8.vy;
    sp_area[1] = src->unk_1C8.vy;
    {
        /* FAKE: unk_B2 read into a0_arg before cmd is chosen; read at the call
         * the argument registers rotate (score 20). */
        s32 a0_arg = src->unk_B2;
        s32 cmd = 0xD;
        if (type == 1)
            cmd = 0xC;
        func_80032854(a0_arg, cmd, &s0->unk_04.x, sp_area);
    }
    return s0;
}

void func_800321E8(void) {
    Unk1F8002B8_8005344C *scr = &SPAD->unk2B8.v8005344C;
    Unk80104E88Rec *base = D_80104E88;
    s32 i;

    i = 0;
    do {
        if (base->unk_00 != 0) {
            base->unk_02 += 1;
            base->unk_10 = base->unk_04;
            base->unk_1C.y += 0xD;
            scr->unk00.x = base->unk_04.x + base->unk_1C.x;
            scr->unk00.y = base->unk_04.y + base->unk_1C.y;
            scr->unk00.z = base->unk_04.z + base->unk_1C.z;
            if (func_8005344C(&base->unk_04.x, &scr->unk00.x, &scr->unk10.x,
                              scr->unk30, (s32)&scr->unk38) != 0 ||
                base->unk_04.y > base->unk_28) {
                base->unk_00 = 0;
            } else {
                base->unk_04 = scr->unk00;
            }
        }
        i++;
        base++;
    } while (i < 4);
}

void func_80032314(void) {
    Unk80104E88Rec *t0 = D_80104E88;
    s32 t1 = 0;
    /* FAKE: a second, byte cursor a3 at each record's unk_02, read for unk_02
       (*a3), unk_03 (a3 + 1) and unk_04 (a3 + 2..); reading them through t0
       drops the second induction register (score 21). */
    u8 *a3 = &D_80104E88[0].unk_02;
    Unk80101EC8Record *ent;
    s32 state;

loop:
    if (t0->unk_00 == 0)
        goto next;
    {
        /* FAKE: named intermediate `v1_v` holds the 0/1 record index;
         * written directly (&D_80101EC8[cmp]) the multiply by a comparison
         * becomes a branch: score 13 (no-new-park-categories
         * named-intermediate) */
        s32 v1_v = (*(u8 *)(a3 + 1) == 0);
        ent = &D_80101EC8[v1_v];
    }
    state = (u16)ent->unk_6A;
    if ((state & 0xFFFF) == 4)
        goto next;
    /* FAKE: do-while(0) loop depth re-ranks register allocation into
     * the target's $a1/$a2/$a3 seating; without it: score 15
     * (do-while-zero-exception) */
    do {
        if ((state & 0xFFFF) == 0x14)
            goto next;
        if ((state & 0xFFFF) == 0xF)
            goto next;
        if ((u32)(state - 0x1C) < 2)
            goto next;
        if ((u32)(state - 0x1E) < 2)
            goto next;
        if ((u32)(state - 0x20) < 2)
            goto next;
        if ((state & 0xFFFF) == 0x11)
            goto next;
        {
            s32 dx = ent->unk_F4.x - *(s32 *)(a3 + 2);
            s32 dy = ent->unk_F4.y - *(s32 *)(a3 + 6);
            s32 dz = ent->unk_F4.z - *(s32 *)(a3 + 0xA);
            u32 dist_sq;
            u32 log2_val;
            dist_sq = (u32)(dx * dx + dy * dy + dz * dz);
            if (dist_sq < 0x400) {
                log2_val = (u32)(g_sqrt_table_u8[dist_sq]) >> 3;
            } else {
                s32 clz = 0;
                s32 sp_tmp;
                if ((s32)dist_sq >= 0) {
                    /* Hand-written GTE leading-zero count (LZCS in, LZCR out),
                     * as in func_800274BC. */
                    __asm__ volatile(
                        "addu   $t4, %1, $zero\n"
                        "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                        "nop\n"
                        "nop\n"
                        "addu   $t4, $sp, $zero\n" /* &sp_tmp (at 0($sp)) */
                        "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                        : "=m"(sp_tmp)
                        : "r"(dist_sq)
                        : "$12");
                    clz = sp_tmp;
                }
                {
                    u32 v1_m;
                    u32 idx;
                    u32 hi;
                    v1_m = 0x16 - (clz & (u32)-2);
                    idx = dist_sq >> v1_m;
                    v1_m = v1_m >> 1;
                    hi = (u32)((u8)(g_sqrt_table_u8[idx]));
                    log2_val = (hi << 16) >> (0x13 - v1_m);
                }
            }
            {
                if (log2_val < (u32)(*a3 * 30 + 0x1F4)) {
                    ent->unk_286 = 5;
                    t0->unk_00 = 0;
                }
            }
        }
    } while (0);
next:
    t1 += 1;
    a3 += 0x2C;
    t0++;
    if (t1 < 4)
        goto loop;
}

void func_800324D0(Unk80101EC8Record *rec) {
    u8 *ptr;
    u8 c;
    u8 val;

    ptr = rec->unk_58;
    rec->unk_A1[0] = 0xFF;
    rec->unk_A3[0] = 0xFF;
    rec->unk_A1[1] = 0xFF;
    rec->unk_A3[1] = 0xFF;
    rec->unk_AA = 0;
    rec->unk_A7 = 0;
    rec->unk_A8 = 0;
    rec->unk_A5 = 0;
    rec->unk_A6 = 0xFF;
    rec->unk_AB = 0xFF;
    rec->unk_AC = 0xFF;

    c = ptr[4];
    /* FAKE: `ptr += 5` spelled as a four-step chain: combine folds it
     * back to one addiu, but the extra refs seat the walker in $v1 */
    ptr++;
    ptr++;
    ptr++;
    ptr += 2;
    while (c != 0) {
        if (c == 0xFF) {
            /* FAKE: the same foldable chain for the 0xFF command's
             * `ptr += 6` */
            ptr++;
            ptr += 5;
        } else if (c < 0x80) {
            ptr++;
        } else {
            val = *ptr;
            ptr++;
            /* FAKE: the loop tail `c = *ptr; ptr++;` duplicated into the
             * first five arms; the extra walker refs steer register allocation
             * and cross-jumping re-merges the tails; a plain `break` in each
             * scores 27 */
            switch (c - 0x80) {
            case 0:
                rec->unk_A1[0] = val;
                c = *ptr;
                ptr++;
                continue;
            case 1:
                rec->unk_A3[0] = val;
                c = *ptr;
                ptr++;
                continue;
            case 2:
                rec->unk_A7 = val;
                c = *ptr;
                ptr++;
                continue;
            case 3:
                rec->unk_A8 = val;
                c = *ptr;
                ptr++;
                continue;
            case 4:
                rec->unk_A9 = val;
                c = *ptr;
                ptr++;
                continue;
            case 5:
                rec->unk_A5 = val;
                break;
            case 6:
                rec->unk_A6 = val;
                break;
            case 7:
                rec->unk_A1[1] = val;
                break;
            case 8:
                rec->unk_A3[1] = val;
                break;
            case 9:
                rec->unk_AA = val;
                break;
            case 10:
                rec->unk_AB = val;
                break;
            case 11:
                rec->unk_AC = val;
                break;
            }
        }
        c = *ptr;
        ptr++;
    }
}

/* 3D positional sound pan/volume: listener-relative delta of *arg1,
 * distance attenuation via the g_sqrt_table_u8 byte-LUT square root (GTE
 * LZCS/LZCR island for
 * >= 0x400), then a sin/cos left/right pan scaled by distance, clamped to 0x7F,
 * sent to func_8005C650(arg0, L, R). The island is an enumerated carrier
 * under the owner cop2 cluster grant (cop2-addressing-preamble-cluster);
 * everything outside it is ordinary C with no FAKE constructs. */
void func_800325E0(s32 arg0, s32 *arg1) {
    s32 sp_tmp;
    s32 dx, dy, dz;
    u32 dist_volume;
    s32 distance_scale;
    s16 listener_angle;
    s32 projected_pan;
    u32 pan_sign;
    s32 pan_L;
    s32 pan_R;

    dx = g_listener_cam->w20 - arg1[0];
    dy = g_listener_cam->w24 - arg1[1];
    dz = g_listener_cam->w28 - arg1[2];

    if (((u32)(dx + 0x9C40) > 0x13880U) || ((u32)(dz + 0x9C40) > 0x13880U)) {
        dist_volume = 0x9C40;
    } else {
        u32 dist_sq = (u32)((dx * dx) + (dy * dy) + (dz * dz));
        if (dist_sq < 0x400U) {
            dist_volume = (u32)(u8)(g_sqrt_table_u8[dist_sq]) >> 3;
        } else {
            /* gte_Lzc(dist_sq, &sp_tmp) -- gtemac.h:174-178: gte_ldlzc
             * (inline_c.h:228-231) + 2x gte_nop (inline_c.h:1346-1347) +
             * gte_stlzc (inline_c.h:1318-1322). The `addu $t4,%1,$zero` and
             * `addiu $v0,$sp,0x10` / `addu $t4,$v0,$zero` materialisations
             * are the cop2 addressing preamble the owner cluster grant
             * covers (cop2-addressing-preamble-cluster); nothing else outside
             * the macro body is in the island. Same spelling as
             * func_800274BC / func_80032314 / func_8002E838. */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"  /* &sp_tmp */
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                : "=m"(sp_tmp)
                : "r"(dist_sq)
                : "$2", "$12");
            {
                /* FAKE: v0_m holds the -2 mask (li -2 into v0, and v0,v1,v0);
                   as a literal the mask lands in v1 and the clz in v0 (score
                   3). */
                u32 v0_m = (u32)-2;
                u32 v1_m;
                u32 idx;
                u32 hi;
                v0_m &= sp_tmp;
                v1_m = 0x16 - v0_m;
                idx = dist_sq >> v1_m;
                v1_m = v1_m >> 1;
                hi = (u32)((u8)(g_sqrt_table_u8[idx]));
                dist_volume = (hi << 16) >> (0x13 - v1_m);
            }
        }
    }

    distance_scale = (s32)((0x9C40 - dist_volume) << 11) / 32000;
    if (distance_scale < 0) {
        distance_scale = 0;
    }

    listener_angle = g_listener_cam->unk_10.vy;
    projected_pan = (s32)((dx * (s32)Judge[(listener_angle + 0x400) & 0xFFF]) +
                          (dz * (s32)Judge[listener_angle & 0xFFF])) >>
                    12;
    pan_sign = ~(u32)projected_pan;
    pan_sign >>= 31;
    if (projected_pan < 0) {
        projected_pan = -projected_pan;
    }

    pan_L = (s32)((0x7530 - projected_pan) << 12) / 30000;
    pan_R = (s32)((0x2710 - projected_pan) << 12) / 10000;
    if (pan_L < 0)
        pan_L = 0;
    if (pan_R < 0)
        pan_R = 0;

    {
        s32 tmp = pan_L;
        if (pan_sign != 0) {
            pan_L = pan_R;
            pan_R = tmp;
        }
    }

    pan_L = (s32)(distance_scale * pan_L) >> 16;
    pan_R = (s32)(distance_scale * pan_R) >> 16;
    if (pan_L >= 0x80)
        pan_L = 0x7F;
    if (pan_R >= 0x80)
        pan_R = 0x7F;

    func_8005C650(arg0, pan_L, pan_R);
}

void func_80032854(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    s32 s0;
    s32 s4;

    if (D_800A38DC == 3 && arg0 == 1 &&
        ((u32)(arg1 - 0x22) < 2 || (u32)(arg1 - 0x24) < 2 ||
         (u32)(arg1 - 0x26) < 2 || (u32)(arg1 - 0x28) < 2)) {
        s4 = 0;
        s0 = 0;
    } else {
        s0 = arg0 * 40;
        s4 = 0x28;
    }

    if (D_800A38DC == 3 && arg0 == 1 && (u32)(arg1 - 0x2D) < 2) {
        s0 += D_8008EBCC[D_800A384C];
    }

    func_800395B4(arg0, arg1, arg2, arg3);

    switch (arg1) {
    case 4:
        if (D_800A38DC == 2 && D_800A389A == 0) {
            func_800617C8(arg2);
            break;
        }
        if (D_800A38DC == 5) {
            func_800617C8(arg2);
            break;
        }
        func_800618B4(arg2, arg3);
        break;
    case 3:
        func_80061250(arg2);
        break;
    case 2:
        func_80061658(arg2, arg0);
        break;
    case 15:
        func_80061710(arg2, arg0);
        break;
    case 1:
        func_8006156C(arg2);
        break;
    case 5:
        func_800325E0(0x3C, arg2);
        func_800325E0(s4 + 0x3C, arg2);
        D_800A3910 = func_8006133C(arg2);
        break;
    case 6:
        func_800325E0(0x3C, arg2);
        func_800325E0(s4 + 0x3C, arg2);
        D_800A3910 = func_800613C8(arg2);
        break;
    case 7:
        func_800325E0(0x3C, arg2);
        func_800325E0(s4 + 0x3C, arg2);
        D_800A3910 = func_80061454(arg2);
        break;
    case 8:
        func_800325E0(0x3C, arg2);
        func_800325E0(s4 + 0x3C, arg2);
        D_800A3910 = func_800614E0(arg2);
        break;
    case 9:
        func_800619A4(arg2);
        break;
    case 10:
        func_800619F0(arg2);
        break;
    case 11:
        func_800325E0(s0 + 0x31, arg2);
        func_800611A4(arg2, arg3);
        break;
    case 12:
        func_800325E0(0x78, arg2);
        func_80061C00(arg2, arg3[1]);
        break;
    case 13:
        func_800325E0(0x79, arg2);
        func_80061D74(arg2, arg3[1]);
        break;
    case 14:
        func_800325E0(s0 + 0x22, arg2);
        func_80061ACC(arg2, arg3);
        break;
    case 17:
        func_800325E0(s0 + 0x39, arg2);
        break;
    case 18:
        func_80061EC0(arg2);
        func_800325E0(s0 + 0x22, arg2);
        break;
    case 33:
        func_800325E0(0x7A, arg2);
        break;
    case 34:
        func_800325E0(s0 + 0x35, arg2);
        break;
    case 35:
        func_800325E0(s0 + 0x35, arg2);
        break;
    case 36:
        func_800325E0(s0 + 0x34, arg2);
        break;
    case 37:
        func_800325E0(s0 + 0x3A, arg2);
        break;
    case 38:
        func_800325E0(s0 + 0x3B, arg2);
        break;
    case 39:
        func_800325E0(s0 + 0x22, arg2);
        break;
    case 40:
        func_800325E0(s0 + 0x3E, arg2);
        break;
    case 41:
        func_800325E0(s0 + 0x40, arg2);
        break;
    case 42:
        func_800325E0(s0 + 0x21, arg2);
        break;
    case 43:
        func_800325E0(s0 + 0x22, arg2);
        break;
    case 44:
        func_800325E0(s0 + 0x23, arg2);
        break;
    case 45:
        func_800325E0(s0 + 0x29, arg2);
        break;
    case 46:
        func_800325E0(s0 + 0x2B, arg2);
        break;
    case 47:
        func_800325E0(s0 + 0x22, arg2);
        break;
    case 48:
        func_800325E0(s0 + 0x22, arg2);
        break;
    case 49:
        func_800325E0(s0 + 0x32, arg2);
        break;
    case 50:
        func_800325E0(s0 + 0x33, arg2);
        break;
    }
}

void func_80032C50(Unk80101EC8Record *obj, s32 kind) {
    s32 pos[3];
    s32 base;
    s32 base2;
    s32 row;
    s32 row2;
    s32 tri;
    s32 tri2;

    if (D_800A38DC == 3 && obj->index == 1 &&
        ((u32)(kind - 7) < 2 || (u32)(kind - 9) < 2 || (u32)(kind - 11) < 2 ||
         (u32)(kind - 13) < 2 || (u32)(kind - 15) < 2 || kind == 17)) {
        base2 = 0;
        base = 0;
    } else {
        base = obj->index * 40;
        base2 = obj->other->index * 40;
    }

    if (D_800A38DC == 3 &&
        ((u32)(kind - 0x15) < 2 || (u32)(kind - 0x17) < 2 ||
         (u32)(kind - 0x19) < 2 || kind == 0x26 || (u32)(kind - 0x36) < 2 ||
         (u32)(kind - 0x38) < 2 || (u32)(kind - 0x3A) < 2 || kind == 0x47)) {
        if (obj->index == 1) {
            base += D_8008EBCC[D_800A384C];
        } else {
            base2 += D_8008EBCC[D_800A384C];
        }
    }

    row = base + obj->unk_B2 * 4;
    tri = obj->unk_B2 * 3;
    row2 = base2 + obj->other->unk_B2 * 4;
    tri2 = obj->other->unk_B2 * 3;

    switch (kind) {
    case 0:
        if (D_800A36A4 == 11) {
            if (obj->unk_1B0[0] - 600 < obj->unk_198[0].y) {
                pos[0] = obj->unk_198[0].x;
                pos[1] = obj->unk_1B0[0];
                pos[2] = obj->unk_198[0].z;
                func_80061A3C(pos, obj->unk_1BA,
                              D_8008EBE0[D_8008E5A8[obj->unk_0C]], obj->index);
                if ((0x60 >> obj->unk_B1) & 1) {
                    func_80032854(obj->index, 9, pos, 0);
                }
            }
        } else if (D_800A36A4 == 14) {
            if (((0x60 >> obj->unk_B1) & 1) &&
                obj->unk_1B0[0] - 600 < obj->unk_198[0].y) {
                pos[0] = obj->unk_198[0].x;
                pos[1] = obj->unk_1B0[0];
                pos[2] = obj->unk_198[0].z;
                func_80032854(obj->index, 9, pos, 0);
            }
        }
        break;
    case 1:
        if (D_800A36A4 == 11) {
            if (obj->unk_1B0[1] - 600 < obj->unk_198[1].y) {
                pos[0] = obj->unk_198[1].x;
                pos[1] = obj->unk_1B0[1];
                pos[2] = obj->unk_198[1].z;
                func_80061A3C(pos, obj->unk_1C2,
                              D_8008EBE0[D_8008E5A8[obj->unk_0C]], obj->index);
                if ((0x60 >> obj->unk_B1) & 1) {
                    func_80032854(obj->index, 9, pos, 0);
                }
            }
        } else if (D_800A36A4 == 14) {
            if (((0x60 >> obj->unk_B1) & 1) &&
                obj->unk_1B0[1] - 600 < obj->unk_198[1].y) {
                pos[0] = obj->unk_198[1].x;
                pos[1] = obj->unk_1B0[1];
                pos[2] = obj->unk_198[1].z;
                func_80032854(obj->index, 9, pos, 0);
            }
        }
        break;
    case 2:
        func_80032854(obj->index, 10, &obj->unk_180.x, 0);
        break;
    case 3:
        func_80032854(obj->index, 10, &obj->unk_18C.x, 0);
        break;
    case 4:
        func_80032854(obj->index, 10, &obj->unk_174.x, 0);
        break;
    case 7:
        func_800325E0(base + 0x31, &obj->unk_F4.x);
        break;
    case 8:
        func_800325E0(base + 0x32, &obj->unk_F4.x);
        break;
    case 9:
        func_800325E0(base + 0x33, &obj->unk_F4.x);
        break;
    case 10:
        func_800325E0(base + 0x37, &obj->unk_F4.x);
        break;
    case 11:
        func_800325E0(base + 0x38, &obj->unk_F4.x);
        break;
    case 12:
        func_800325E0(base + 0x39, &obj->unk_F4.x);
        break;
    case 13:
        func_800325E0(base + 0x3D, &obj->unk_F4.x);
        break;
    case 14:
        func_800325E0(base + 0x3E, &obj->unk_F4.x);
        break;
    case 15:
        func_800325E0(base + 0x3D, &obj->unk_F4.x);
        break;
    case 16:
        func_800325E0(base + 0x3F, &obj->unk_F4.x);
        break;
    case 17:
        func_800325E0(base + 0x40, &obj->unk_F4.x);
        break;
    case 18:
        func_800325E0(base + 0x22, &obj->unk_F4.x);
        break;
    case 19:
        func_800325E0(base + 0x23, &obj->unk_F4.x);
        break;
    case 20:
        func_800325E0(base + 0x24, &obj->unk_F4.x);
        break;
    case 21:
        func_800325E0(base + 0x25, &obj->unk_F4.x);
        break;
    case 22:
        func_800325E0(base + 0x26, &obj->unk_F4.x);
        break;
    case 23:
        func_800325E0(base + 0x27, &obj->unk_F4.x);
        break;
    case 24:
        func_800325E0(base + 0x28, &obj->unk_F4.x);
        break;
    case 25:
        func_800325E0(base + 0x29, &obj->unk_F4.x);
        break;
    case 26:
        func_800325E0(base + 0x2A, &obj->unk_F4.x);
        break;
    case 27:
        func_800325E0(row + 0x41, &obj->unk_F4.x);
        break;
    case 28:
        func_800325E0(row + 0x42, &obj->unk_F4.x);
        break;
    case 29:
        func_800325E0(row + 0x43, &obj->unk_F4.x);
        break;
    case 30:
        func_800325E0(row + 0x44, &obj->unk_F4.x);
        break;
    case 31:
        func_800325E0(tri + 0x71, &obj->unk_F4.x);
        break;
    case 32:
        func_800325E0(tri + 0x72, &obj->unk_F4.x);
        break;
    case 33:
        func_800325E0(tri + 0x73, &obj->unk_F4.x);
        break;
    case 34:
        func_800325E0(0x77, &obj->unk_F4.x);
        break;
    case 35:
        func_800325E0(0x7A, &obj->unk_F4.x);
        break;
    case 36:
        func_800325E0(base + 0x3A, &obj->unk_F4.x);
        break;
    case 37:
        func_800325E0(base + 0x36, &obj->unk_F4.x);
        break;
    case 38:
        func_800325E0(base + 0x2B, &obj->unk_F4.x);
        break;
    case 39:
        func_800325E0(base + 0x2C, &obj->unk_F4.x);
        break;
    case 40:
        func_800325E0(base2 + 0x31, &obj->other->unk_F4.x);
        break;
    case 41:
        func_800325E0(base2 + 0x32, &obj->other->unk_F4.x);
        break;
    case 42:
        func_800325E0(base2 + 0x33, &obj->other->unk_F4.x);
        break;
    case 43:
        func_800325E0(base2 + 0x37, &obj->other->unk_F4.x);
        break;
    case 44:
        func_800325E0(base2 + 0x38, &obj->other->unk_F4.x);
        break;
    case 45:
        func_800325E0(base2 + 0x39, &obj->other->unk_F4.x);
        break;
    case 46:
        func_800325E0(base2 + 0x3D, &obj->other->unk_F4.x);
        break;
    case 47:
        func_800325E0(base2 + 0x3E, &obj->other->unk_F4.x);
        break;
    case 48:
        func_800325E0(base2 + 0x3D, &obj->other->unk_F4.x);
        break;
    case 49:
        func_800325E0(base2 + 0x3F, &obj->other->unk_F4.x);
        break;
    case 50:
        func_800325E0(base2 + 0x40, &obj->other->unk_F4.x);
        break;
    case 51:
        func_800325E0(base2 + 0x22, &obj->other->unk_F4.x);
        break;
    case 52:
        func_800325E0(base2 + 0x23, &obj->other->unk_F4.x);
        break;
    case 53:
        func_800325E0(base2 + 0x24, &obj->other->unk_F4.x);
        break;
    case 54:
        func_800325E0(base2 + 0x25, &obj->other->unk_F4.x);
        break;
    case 55:
        func_800325E0(base2 + 0x26, &obj->other->unk_F4.x);
        break;
    case 56:
        func_800325E0(base2 + 0x27, &obj->other->unk_F4.x);
        break;
    case 57:
        func_800325E0(base2 + 0x28, &obj->other->unk_F4.x);
        break;
    case 58:
        func_800325E0(base2 + 0x29, &obj->other->unk_F4.x);
        break;
    case 59:
        func_800325E0(base2 + 0x2A, &obj->other->unk_F4.x);
        break;
    case 60:
        func_800325E0(row2 + 0x41, &obj->other->unk_F4.x);
        break;
    case 61:
        func_800325E0(row2 + 0x42, &obj->other->unk_F4.x);
        break;
    case 62:
        func_800325E0(row2 + 0x43, &obj->other->unk_F4.x);
        break;
    case 63:
        func_800325E0(row2 + 0x44, &obj->other->unk_F4.x);
        break;
    case 64:
        func_800325E0(tri2 + 0x71, &obj->other->unk_F4.x);
        break;
    case 65:
        func_800325E0(tri2 + 0x72, &obj->other->unk_F4.x);
        break;
    case 66:
        func_800325E0(tri2 + 0x73, &obj->other->unk_F4.x);
        break;
    case 67:
        func_800325E0(0x77, &obj->other->unk_F4.x);
        break;
    case 68:
        func_800325E0(0x7A, &obj->other->unk_F4.x);
        break;
    case 69:
        func_800325E0(base2 + 0x3A, &obj->other->unk_F4.x);
        break;
    case 70:
        func_800325E0(base2 + 0x36, &obj->other->unk_F4.x);
        break;
    case 71:
        func_800325E0(base2 + 0x2B, &obj->other->unk_F4.x);
        break;
    case 72:
        func_800325E0(base2 + 0x2C, &obj->other->unk_F4.x);
        break;
    }
}

/* Tail word after func_80032C50's 73-entry compiler-generated switch table. */
const u32 D_800107BC[1] = {0x00000000};

void func_8003339C(Unk80101EC8Record *base) {
    u8 *s0;
    s32 a1val;
    u8 *p;

    p = base->unk_58;
    a1val = p[4];
    if (a1val == 0)
        goto done;
    s0 = p + 5;

loop:
    a1val = a1val & 0xFF;
    if (a1val == 0xFF) {
        u8 b0 = s0[0];
        u8 b1 = s0[1];
        u8 b2 = s0[2];
        u8 b3 = s0[3];
        s32 packed;
        s16 shift;
        s32 mask;

        packed = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
        shift = base->unk_0A;
        mask = 1 << shift;
        if ((packed & mask) != 0) {
            s0 += 4;
        } else {
            s0 += 6;
        }
        goto next;
    }

    /* FAKE: do-while(0) puts a loop note on `done:` so reorg keeps the
     * target's branch sense for the `bnez done` below; without it score 1
     * (do-while-zero-exception) */
    do {
        if ((u32)a1val < 0x80) {
            u8 val = s0[0];
            s16 dir = base->unk_40;
            s0 += 1;
            if ((val & 0xFF) == dir) {
                func_80032C50(base, a1val - 1);
                goto next;
            }
            if (dir < val) {
                goto done;
            }
            goto next;
        }

        s0 += 1;

    next:
        a1val = *s0;
        s0 += 1;
    } while (0);

    if (a1val != 0)
        goto loop;

done:
    return;
}

s32 func_80033498(void) {
    s16 idx = D_800A36A4 - 2;
    switch (idx) {
    case 0:
        return 0;
    case 2:
        return 1;
    case 5:
        return 2;
    case 6:
        return 3;
    case 16:
        return 4;
    case 22:
        return 5;
    default:
        return 0xFF;
    }
}

extern u8 D_800A391D;

void func_80033510(void) {
    s32 i = 3;
    s16 *p1 = &D_800A3750[3];
    do {
        *p1 = 0;
        i--;
        p1--;
    } while (i >= 0);
    i = 5;
    {
        u8 *p2 = &D_800A391D;
        do {
            *p2 = 0;
            i--;
            p2--;
        } while (i >= 0);
    }
}

void func_80033550(LeafPos *arg0) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (D_800A3918[i] == 0) {
            break;
        }
    }
    if (i == 6) {
        return;
    }
    D_800A3918[i] = 1;
    D_80107850[i] = *arg0;
}

void func_800335D8(void) {
    s32 i;
    u8 *tbl = D_8008E914[D_800A36A4];

    for (i = 0; i < 6; i++) {
        if (D_800A3918[i] != 0) {
            s32 cat = func_80033498();
            u8 cur = D_800A3918[i];

            if (cur == 1) {
                func_800325E0(D_8008EBF4[cat], &D_80107850[i].x);
            } else if (cur == D_8008EBFC[cat].a) {
                func_800325E0(D_8008EBF4[cat] + 1, &D_80107850[i].x);
            } else if (cur == D_8008EBFC[cat].b) {
                func_800325E0(D_8008EBF4[cat] + 2, &D_80107850[i].x);
            }

            {
                s32 val = D_800A3918[i] + 1;
                D_800A3918[i] = val;
                if ((u32)D_8008EBFC[cat].b < (u32)(val & 0xFF)) {
                    D_800A3918[i] = 0;
                }
            }
        }
    }

    if (func_8001DB58() != 0) {
        s16 *buf = D_800A3750;

        for (i = 0; i < 4; i++, tbl += 2) {
            u8 *data = tbl + 1;
            s32 rng;
            s32 type;
            s32 rnd;

            rng = rng_Next();
            rnd = rng & 0x3FF;
            type = *tbl;

            if (type == 1)
                goto handle_type_1;
            if (type < 2)
                goto skip;
            if (type < 7)
                goto handle_type_2_6;
            continue;

        handle_type_1:
            if (buf[i] != 0)
                goto skip;
            buf[i] = 1;
            goto call_default;

        handle_type_2_6: {
            s32 adj = type - 2;
            s32 count;
            s32 scount;
            s32 thresh;
            s32 limit;

            count = (u16)buf[i] + 1;
            buf[i] = count;
            scount = (s16)count;
            thresh = D_8008EA44[adj].a;
            limit = thresh * 30;

            if (limit < scount) {
                s32 ratio = ((scount - limit) << 10) / (D_8008EA44[adj].b * 30);

                if (rnd < ratio) {
                    buf[i] = 0;
                    type = *tbl;
                    if ((u32)(type - 5) < 2) {
                        func_800325E0(0x7B + i, D_8008EA00[type]);
                        continue;
                    }
                    goto call_default;
                }
            }
            goto skip;
        }

        call_default: {
            u8 val = *data;
            func_8005C650(0x7B + i, val, val);
        }
        skip:;
        }
    }
}

void func_80033898(void) {
    gpu_ResetGraphMode1();
    D_800A37B8 = 0;
    D_800A3834 = 3;
}

void func_800338CC(void) {
    s32 sp[2];
    s32 count;
    s32 i;
    s32 idx1;
    u8 *ptr;
    s32 one;

    {
        s32 mask = ~(1 << D_8008D538[(s8)D_80102778.unk_4[0]]) & 0x3EF3DF;
        s32 bits;
        count = 0;
        i = 0;
        bits = D_80106A50.unk_00 & mask;
        /* FAKE: `one` carries the constant 1 of `1 << i`; with the literal
           the test becomes `(bits >> i) & 1` (srav/andi instead of the
           target's `li a2,1` + sllv/and), score 4. */
        one = 1;
        ptr = D_801077B0;
        do {
            if (bits & (one << i)) {
                *ptr = i;
                ptr++;
                count++;
            }
            i++;
        } while (i < 0x1B);
    }

    i = 0;
    do {
        i++;
        idx1 = rand() % count;
        {
            s32 idx2 = rand() % count;
            s32 tmp = D_801077B0[idx1];
            D_801077B0[idx1] = D_801077B0[idx2];
            D_801077B0[idx2] = tmp;
        }
    } while (i < 0x6C);

    if (D_80106A50.unk_00 & 0x10020) {
        s32 v1 = D_80106A50.unk_00 & 0x20;
        s32 v0 = D_80106A50.unk_00 & 0x10000;
        sp[0] = v1;
        sp[1] = v0;
        if (v1 == 0) {
            sp[0] = v0;
            sp[1] = 0;
            goto block_12;
        }
        if (v0 != 0) {
            if (rand() & 1) {
                s32 t1 = sp[1];
                s32 t2 = sp[0];
                sp[0] = t1;
                sp[1] = t2;
            }
        }
    block_12:
        if (count >= 0xB) {
            i = count;
            do {
                D_801077B0[i] = D_801077B0[i - 1];
                i--;
            } while (i >= 0xB);
            {
                s32 dir = 0x10;
                if (sp[0] & 0x20) {
                    dir = 5;
                }
                D_801077B0[10] = dir;
            }
        } else {
            s32 dir = 0x10;
            if (sp[0] & 0x20) {
                dir = 5;
            }
            D_801077B0[count] = dir;
        }
        count++;
        if (sp[1] != 0) {
            u8 *a1 = &D_801077B0[count];
            count++;
            {
                s32 dir = 0x10;
                if (sp[1] & 0x20) {
                    dir = 5;
                }
                *a1 = dir;
            }
        }
    }
    {
        u8 lookup = D_8008D9EC[D_8008D538[(s8)D_80102778.unk_4[0]]];
        s32 val;
        if (lookup != 0) {
            if (D_80106A50.unk_00 & 0x04000000) {
                val = 0x1A;
                goto append_last;
            }
        } else {
            if (D_80106A50.unk_00 & 0x01000000) {
                val = 0x18;
                goto append_last;
            }
        }
        goto set_count;
    append_last:
        D_801077B0[count] = val;
        count++;
    }
set_count:
    D_800A391F = count;
    D_800A3783 = 0;
    D_800A37BC = 0;
}

void func_80033BC0(void) {
    u8 a0 = D_800A3783;
    u8 b = D_800A391F;

    if (a0 == b) {
        g_disp_enable = DISP_DISABLED;
        g_disp_fade = 0;
        if (a0 == 0x14) {
            u8 z = D_8008D9EC[D_80101EC8[0].unk_0A];
            s32 val = 2;
            if (z != 0)
                val = 3;
            D_800A38A4 = val;
            D_800A3834 = 0x12;
        } else {
            D_800A3834 = 0x20;
        }
    } else {
        u8 a1;
        D_800A376B = 0;
        D_800A3783 = a0 + 1;
        a1 = D_801077B0[a0];
        D_80102778.unk_4[1] = (&D_8008D55C)[a1];
        if (a1 == 5) {
            D_80102778.unk_4[3] = 6;
        } else if (a1 == 0x10) {
            D_80102778.unk_4[3] = 7;
        } else {
            u8 x = D_800A37BC;
            s8 y;
            D_800A37BC = x + 1;
            y = (&D_8008E748)[x];
            D_80102778.unk_4[3] = y;
            if (y == 4) {
                u8 v = D_8008D9EC[a1];
                if (v != 0) {
                    D_80102778.unk_4[3] = 5;
                }
            }
        }
        D_80102778.unk_0[1] = 0x800;
        {
            s16 v = (&D_8008E75C)[a1];
            D_800A3834 = 0;
            D_800A36A4 = v;
        }
    }
}

void func_80033D38(void) {
    /* FAKE: pointer to the record, admitted on SOTN precedent (Q50, Q53);
     * mechanism: its register (t1) is the base of every times[] access and of
     * the shift loop's pointer; written directly, each D_80106A50.times[]
     * access is its own lui/addu/%lo: score 34. */
    FileRecord *rec = &D_80106A50; /* SOTN: src/dra/4CE2C.c:63 @db41b28 */
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (rec->times[j].unk_4 < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            rec->times[k] = rec->times[k - 1];
        }
        rec->times[n].unk_0 = (u8)D_80101EC8[0].unk_0A;
        rec->times[n].unk_1 = (u8)D_80101EC8[0].unk_0E;
        rec->times[n].unk_4 = D_800A3858;
    }
}

s32 func_80033DF4(void) {
    u8 state;
    s32 tableIndex;

    state = D_800A38E2;
    tableIndex = state & 0xFF;
    if (tableIndex == 0x64) {
        D_800A36F0 = 0;
        D_800A3781 = 0;

        if (D_800A3858 < 0x6979) {
            s32 *flags;
            s32 word;
            s32 mask;

            mask = 0x20;
            if (D_8008D9EC[D_8008D538[(s8)D_80102778.unk_4[0]]] != 0) {
                mask = 0x10000;
            }
            flags = &D_80106A50.unk_00;
            word = *flags;
            D_800A36F0 = (u32)(word & mask) < 1;
            *flags = word | mask;
        }

        if (D_800A380C == 0) {
            s32 *flags;
            s32 word;
            s32 mask;

            mask = 0x1000000;
            if (D_8008D9EC[D_8008D538[(s8)D_80102778.unk_4[0]]] != 0) {
                mask = 0x4000000;
            }
            flags = &D_80106A50.unk_00;
            word = *flags;
            D_800A3781 = (u32)(word & mask) < 1;
            *flags = word | mask;
        }

        func_80033D38();
        D_800A3834 = 4;
        return 0;
    } else {
        u8 *table = cpu_practice_honmokuroku_data_tbl[tableIndex];
        u8(*ranks)[5] = D_8008EC24;
        u8(*moves)[5] = D_8008E908;
        s32 entry;
        s32 row;

        D_800A38E2 = state + 1;
        D_800A376B = 0;
        entry = table[0];
        D_800A384C = entry;
        row = D_8008D9EC[D_8008D538[(s8)D_80102778.unk_4[0]]] == 0;
        entry &= 0xFF;
        D_800A38DE = ranks[row][entry];
        D_800A38EC = table[1];
        D_800A38ED = table[2];
        D_800A38EE = table[3];
        D_80102778.unk_4[1] = moves[row][entry];
        return 1;
    }
}

void func_80033FE4(void) {
    s32 v1;
    if (D_800A36F0 != 0) {
        v1 = 6;
        if (D_8008D9EC[D_80101EC8[0].unk_0A] != 0) {
            v1 = 7;
        }
        D_800A38A4 = v1;
        D_800A3834 = 0x12;
        return;
    }
    if (D_800A3781 != 0) {
        v1 = 8;
        if (D_8008D9EC[D_80101EC8[0].unk_0A] != 0) {
            v1 = 9;
        }
        D_800A38A4 = v1;
        D_800A3834 = 0x12;
        return;
    }
    if (D_800A38E9 < 3) {
        D_800A3834 = 0x1A;
    } else {
        D_800A3834 = 8;
    }
}

void func_800340A0(void) {
    u8 p1, p2, round;

    p1 = D_800A3898;
    if ((u8)p1 == D_800A37F8) {
        round = D_800A3874;
        D_800A377C[round] = 0;
    } else {
        p2 = D_800A3899;
        if ((u8)p2 == D_800A37F8) {
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else if ((u8)p2 < (u8)p1) {
            round = D_800A3874;
            D_800A377C[round] = 0;
        } else if ((u8)p1 < (u8)p2) {
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else {
            if ((u8)D_800A38AA < (u8)D_800A38AB) {
                D_800A3898 = p1 + 1;
                round = D_800A3874;
                D_800A377C[round] = 0;
            } else if ((u8)D_800A38AB < (u8)D_800A38AA) {
                D_800A3899 = p2 + 1;
                round = D_800A3874;
                D_800A377C[round] = 1;
            } else {
                round = D_800A3874;
                D_800A377C[round] = 2;
            }
        }
    }
    D_800F65F8[D_800A3874][0] = D_800A3898;
    D_800F65F8[D_800A3874][1] = D_800A3899;
    D_800A3874 = D_800A3874 + 1;
}

void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    u8 n;
    s32 innerBound;
    u8 *base;

    g_disp_enable = DISP_LOADING;
    n = D_800A389B;
    if (i < n) {
        innerBound = D_800A3874;
        do {
            s32 useReal;
            s32 j;
            base = D_800F65F8[i];
            useReal = (i < innerBound);
            j = 0;
            do {
                if (useReal) {
                    acc |= base[j] << shift;
                } else {
                    acc |= 3 << shift;
                }
                shift += 2;
                j++;
            } while (j < 2);
            i++;
            n = D_800A389B;
        } while (i < n);
    }

    D_800A3784 = acc;
}

extern void func_80034200(void);

void func_800342A0(void) {
    func_80034200();
    if (D_800A3874 == D_800A389B) {
        g_disp_enable = DISP_DISABLED;
        g_disp_fade = 0;
        D_800A3834 = 0x14;
    } else {
        s32 v1;
        u8 *a1 = &D_801027A0;
        u8 *a0 = &D_801027D8;
        D_800A38F4 = 0;
        D_800A3899 = 0;
        D_800A3898 = 0;
        D_800A38AB = 0;
        D_800A38AA = 0;
        D_800A3816 = 0;
        D_800A391E = 0x50;
        D_800A381E = 0;
        D_800A37E1 = 0;
        D_800A36E8 = 0;
        D_800A38B8 = 0;
        D_800A3920 = 0;
        v1 = 0;
    loop: {
        s32 v0 = D_800A3874;
        v0 = v0 << 1;
        v0 = v0 + (s32)a1;
        v0 = *((u8 *)v0 + v1);
        D_80102778.unk_4[v1] = v0;
    }
        {
            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a0;
            v0 = *((u8 *)v0 + v1);
            D_80102778.unk_4[2 + v1] = v0;
        }
        v1++;
        if (v1 < 2)
            goto loop;
        func_80016888();
        func_800372C0();
        D_800A3834 = 0;
    }
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 D_800A3140 = 0;
/* address taken (lui/addiu) in func_800288C8: size from the blob label */
s32 D_800A3144[2] = {0x15e, 0x172};
s32 D_800A314C = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
u8 D_800A36F2[2];
