/* The game's boot and main loop: main(), the PCdrv host-file loaders
 * (pcdrv_LoadFile, pcdrv_LoadSectors), display setup wrappers, rng_SetSeed /
 * rng_Next and scratchpad_Save / scratchpad_Restore. .text 0x800164F8 (ROM
 * 0x6CF8). Start boundary: the first game .text object (LEGACY: a splat segment
 * edge). */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"
#include "gte.h"

extern u8 g_file_dma_flag;
extern s32 g_rng_state;
extern RECT g_gpu_clear_rect;
extern u32 g_scratchpad_save;
/* This file's .rodata: debug format strings, and the build date that D_800A30E0
 * (below) points at. */
const char g_str_overflow[12] = "OVER FLOW\n";
const char g_str_eff_init[28] = "eff_init:%08x size:%08x\n";
const char g_str_limit[12] = "LIMIT:%08x\n";
const char g_str_prim_overflow[24] = "common prim over flow\n";
const char g_str_build_date[28] = "Fri Aug  7 22:26:32 1998\n";

extern void printf();
extern void func_800164F8(void);
extern s32 PCread(s32, u8 *, s32);

extern u8 D_800A30E8;
extern RECT D_800A30D4;
extern s32 D_800A30DC;

extern void __main(void);
/* Not the bb2.h spelling: the definition takes an s16 first parameter, but
 * this call passes it unextended (an s16 prototype adds sll/sra here). */
extern s32 func_80060414(s32, s32, s32);

/* func_800164F8 -- spins 10000 times executing `break 1` (0x0001000D), spelled
 * as its word: maspsx cannot assemble `break` with a code operand. */
void func_800164F8(void) {
    s32 i;

    for (i = 9999; i >= 0; i--) {
        __asm__ volatile(".word 0x0001000D"); /* break 1 */
    }
}

s32 pcdrv_LoadFile(s32 a0, u8 *dest) {
    s32 fd;
    s32 total;
    s32 remaining;
    s32 chunk;

    fd = PCopen(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    total = PClseek(fd, 0, 2);
    remaining = total;
    PClseek(fd, 0, 0);
    if (total > 0) {
        do {
            chunk = 0x4000;
            if (remaining < 0x4001) {
                chunk = remaining;
            }
            if (PCread(fd, dest, chunk) != chunk) {
                close(fd);
                return -1;
            }
            remaining -= chunk;
            dest += chunk;
        } while (remaining > 0);
    }
    PCclose(fd);
    return total;
}

s32 pcdrv_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;

    fd = PCopen(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    PClseek(fd, sector << 11, 0);
    for (i = 0; i < count; i++) {
        if (PCread(fd, dest, 0x800) != 0x800) {
            close(fd);
            return -1;
        }
        dest += 0x800;
    }
    PCclose(fd);
    return count << 11;
}

s32 math_FovToScreenDist(s32 a0) {
    s32 tmp = (a0 << 12) / 360;
    s32 v1 = tmp / 2;
    s16 cos_val = Judge[(v1 + 0x400) & 0xFFF];
    s16 sin_val = Judge[v1 & 0xFFF];
    return (cos_val * 320) / sin_val;
}

void gpu_SetDrawEnvBg(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 i;

    for (i = 0; i < 2; i++) {
        g_gpu_db[i].draw.isbg = a0;
        setRGB0(&g_gpu_db[i].draw, a1, a2, a3);
    }
}

u32 file_GetFlag0(void) { return D_80106A50.flags & 1; }

u32 file_GetFlag1(void) { return (D_80106A50.flags >> 1) & 1; }

u32 file_GetFlag2(void) { return (D_80106A50.flags >> 2) & 1; }

void func_800167EC(void) {
    s32 i;
    /* FAKE: pointer to the record beside direct global writes (Q50, Q53):
     * the header stores go off rec's register and the loop walks a pointer
     * copied from it by 8; D_80106A50 directly in the loop: score 19. */
    FileRecord *rec = &D_80106A50; /* SOTN: src/st/st0/2A218.c:48 @db41b28 */

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}

void gpu_ResetGraphMode1(void) { ResetGraph(1); }

void gpu_InitDisplay(void) {
    SetDispMask(0);
    ResetGraph(1);
    ClearImage(&g_gpu_clear_rect, 0, 0, 0);
    DrawSync(0);
}

void gpu_SetDispMaskOn(void) { SetDispMask(1); }

void sys_StubEmpty(void) {}

void rcnt_StartCnt1Wrapper(void) { rcnt_StartCnt1(); }

extern void SetGraphDebug(s32);

void disp_Init(void) {
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
    InitGeom();
    SetGeomOffset(0x140, 0x78);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    SetDefDrawEnv(&g_gpu_db[0].draw, 0, 0, 0x280, 0xF0);
    SetDefDrawEnv(&g_gpu_db[1].draw, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[0].disp, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    ClearImage(&g_gpu_clear_rect, 0, 0, 0);
    DrawSync(0);
}

extern void InitPAD(u8 *, s32, u8 *, s32);

void sys_Init(void) {
    ResetCallback();
    InitPAD((u8 *)g_pad_buf[0], 8, (u8 *)g_pad_buf[1], 8);
    StartPAD();
    ChangeClearPAD(0);
    disp_Init();
    g_disp_enable = DISP_DISABLED;
    g_disp_fade = 0;
    cdrom_Init();
    memcard_Init();
    rcnt_StartCnt1Wrapper();
}

void func_80016A8C(u8 *arg0, void *arg1, s32 arg2) {
    RECT rect;
    s32 i;

    rect = D_800A30D4;

    SetDispMask(0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x140, 0xF0);
    game_FrameLoop();
    cdrom_StartRead(func_80036EA8(2, 0x61), (s32)arg0);
    game_FrameLoop();
    PutDispEnv(&g_gpu_db[1].disp);
    DrawSync(0);
    LoadImage(&rect, (u32 *)(arg0 + 0x14));
    DrawSync(0);
    SetDispMask(1);

    for (i = 0; i < 0x96; i++) {
        if (i >= 0x79) {
            u16 *pixels = (u16 *)(arg0 + 0x28);
            s32 j;

            for (j = 0; j < rect.w * rect.h; j++, pixels++) {
                u32 pixel = *pixels;
                /* FAKE: (s32) makes the shift signed; unsigned: score 1 (srl
                 * for the target's sra) */
                s32 value = (s32)(pixel & 0x1F) >> 1;
                u32 temp;

                /* FAKE: no-op mask of the u16 read; removed: score 1 (the andi
                 * goes) */
                pixel &= 0xFFFF;
                temp = pixel >> 1;
                temp &= 0x1E0;
                value += temp;
                pixel >>= 1;
                pixel &= 0x3C00;
                value += pixel;
                *pixels = value;
            }
            LoadImage(&rect, (u32 *)(arg0 + 0x14));
        }

        DrawSync(0);
        VSync(0);
    }

    SetDispMask(0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    SetDispMask(1);
}

void sys_Panic(void) {
    printf(g_str_overflow);
    while (1) {
        func_800164F8();
    }
}

void file_ResetDmaFlag(void) { g_file_dma_flag = 0; }

void eff_Init(void) {
    s32 size;

    if (g_file_dma_flag != 0) {
        return;
    }
    size = func_80060CB8(0x801D8800, 0x8010E800);
    printf(g_str_eff_init, 0x8010E800, size);
    if (0xA000 < size) {
        sys_Panic();
    }
    g_file_dma_flag = 1;
}

extern void memcpy(u32, u32, s32);
extern s32 snd_VabFakeOpen(s32, s16);

void file_LoadSoundData(void) {
    s32 size;

    snd_Init();
    size = snd_LoadCommonVab(0x801D8800);
    if (size >= 0xD01) {
        sys_Panic();
    }
    memcpy(0x8010DB00, 0x801D8800, size);
    snd_VabFakeOpen(0xFFF35300, 0);
    func_8005C614();
    D_800A3906 = 1;
}

extern u32 D_800A3798;
extern u8 D_800A3744;
extern u8 D_800A3745;
extern u8 D_800A3746;
extern u8 D_800A36B0;

void sys_GameInit(void) {
    printf(g_str_limit, 0x8010DB00);
    func_800167EC();
    func_80020D70();
    D_800A3770[0] = 0x801D8800;
    D_800A3770[1] = 0x801EBC00;
    D_800A3798 = 0x13400;
    g_file_dma_flag = 0;
    D_800A3906 = 0;
    file_LoadSoundData();
    pad_ResetStateMarkValid();
    func_8003D2C4();
    func_8001C444();
    D_800A36F9 = 0;
    D_800A3690 = 0;
    D_800A3744 = 0;
    D_800A3745 = 0;
    D_800A3746 = 0;
    game_Init();
    D_800A36F1 = 2;
    D_800A38C4[1] = 0;
    D_800A36B0 = 0;
    D_800A3928 = 0;
}

void gpu_WaitDrawSync(void) { DrawSync(0); }

void func_80016E60(GpuDb *arg0, s32 arg1) {
    u32 ot[2];
    GpuDb *env;
    s32 select;
    s32 special;
    s32 limit;
    u32 prim_base;
    s32 idx;
    GpuDb *ot_base;

    select = 0;
    special = 0;
    /* FAKE: pass-through handle on the parameter, used once at DrawOTag;
       moves the prologue's $s5 save/init group from first to third. Removed:
       score 4 (pointer-alias-fake-exception) */
    ot_base = arg0;
    if (D_800A38DC == 2) {
        special = D_800A389A < 1;
    }
    limit = 3;
    if (special != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    func_8005C650(3, 0x7F, 0x7F);
    prim_base = D_800A3770[D_800A36AC & 1];

    while (1) {
        idx = D_800A36AC & 1;
        g_prim_buf_cursor = (u32 *)(prim_base + (idx * 0x9A00));
        g_gpu_ot_ptr = &ot[idx];
        env = &g_gpu_db[idx];

        ClearOTagR(g_gpu_ot_ptr, 1);
        func_80019568();
        if (special != 0) {
            func_8005C8A8(
                2, select | (D_800A3788 << 16), (s32)g_prim_buf_cursor, 0);
        } else {
            func_8005C8A8(0, select, (s32)g_prim_buf_cursor, 0);
        }
        func_80036940();
        func_8005C6D0();
        DrawSync(0);
        VSync(2);
        /* FAKE: do-while(0) raises env's loop-depth weight above select's,
           seating env in $s0 and select in $s1; unwrapped: score 21
           (do-while-zero-exception) */
        do {
            PutDispEnv(&env->disp);
            PutDrawEnv(&env->draw);
        } while (0);
        DrawOTag(&ot_base->ot[0x1007]);
        DrawOTag(g_gpu_ot_ptr);
        D_800A36AC++;

        if (g_pad_state.pressed & 0x100010) {
            func_8005C650(1, 0x7F, 0x7F);
            select = 0;
            break;
        }
        if (g_pad_state.pressed & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            break;
        }
        if (g_pad_state.pressed & 0x10001000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == 0) ? limit - 1 : select - 1;
        } else if (g_pad_state.pressed & 0x40004000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == limit - 1) ? 0 : select + 1;
        }

        if ((special != 0) && (select >= 3)) {
            if (g_pad_state.pressed & 0x80008000) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3788 |= 1 << (select - 3);
            } else if (g_pad_state.pressed & 0x20002000) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3788 &= ~(1 << (select - 3));
            }
        }
    }

    if (select != 0) {
        if (select == 1) {
            D_800A31DA = 1;
            D_800A3834 = 8;
            func_800372C0();
        } else if (select == 2) {
            D_800A3834 = 8;
            func_800372C0();
        }
    }

    DrawSync(0);
    ResetRCnt(0xF2000001);
    D_800A36B0 = 1;
}

void rng_SetSeed(s32 a0) { g_rng_state = a0; }

s32 rng_Next(void) {
    s32 seed = g_rng_state;
    s32 result = seed * 5497 + 0x7FA9;
    seed = (seed >> 16) ^ result;
    g_rng_state = seed;
    return seed & 0x7FFF;
}

void main(void) {
    s32 idx;
    GpuDb *env;
    u32 *ot;
    /* D_800A390D (frames left to skip presenting) as read this frame; the next
     * frame passes it to func_80019568. */
    s32 skip;
    /* FAKE: second handle on D_800A3770 keeps its address in $s4 for the
     * whole loop; D_800A3770[idx] at both uses: score 14 */
    u32 *tbl;

    __main();
    SetSp(0x801FFF00);
    SetMem(2);
    sys_Init();
    sys_GameInit();
    SetDispMask(1);
    func_80016A8C((u8 *)0x80118800, env, idx);

    tbl = D_800A3770;
    D_800A3834 = 0xF;
    D_800A390D = 0;
    D_800A36AC = 0;

loop:
    idx = D_800A36AC & 1;
    env = &g_gpu_db[idx];
    ot = env->ot;
    ClearOTagR(ot, 0x1008);
    g_gpu_ot_ptr = ot;
    g_prim_buf_cursor = (u32 *)tbl[idx];
    func_80060E04(idx);
    func_8003D2F4();
    func_80019568(skip);
    func_80036940();
    func_8005C6D0();

    if (D_800A3928 != 0) {
        func_800372C0();
        g_disp_enable = DISP_DISABLED;
        D_800A3928 = 0;
        D_800A31DA = 0;
        D_800A3834 = 8;
    }

    g_module_func_tbl[D_800A3834]();
    func_8003D330();

    do {
        if (GetRCnt(0xF2000001u) >= ((D_800A36F1 - 1) << 8) + 0x80)
            break;
        rand();
    } while (1);

    VSync(1);
    DrawSync(0);
    VSync(0);
    ResetRCnt(0xF2000001u);

    skip = D_800A390D;
    if (skip == 0) {
        PutDispEnv(&env->disp);
        PutDrawEnv(&env->draw);
    }

    {
        s32 remaining = tbl[idx] + 0x13400 - (s32)g_prim_buf_cursor;
        if (remaining < D_800A30DC) {
            D_800A30DC = remaining;
        }
        if (remaining < 0) {
            printf(g_str_prim_overflow);
            while (1) {
                func_800164F8();
            }
        }
    }

    if (D_800A390D != 0) {
        D_800A390D--;
    } else {
        DrawOTag(&env->ot[0x1007]);
        D_800A36AC++;
    }

    if (D_800A3834 != 1)
        goto loop;
    if (skip != 0)
        goto loop;
    if (g_pad_state.pressed & 0x08000800u)
        goto call_func;
    if (D_800A38DC != 2)
        goto loop;
    if (D_800A3713 == 0)
        goto loop;
    D_800A3713--;
    if (D_800A3713 != 0)
        goto loop;
call_func:
    func_80016E60(env, idx);
    goto loop;
}

void func_800174F4(void) {
    u32 ot[2];
    DRAWENV env;
    /* holds two values: the case-1/2 fade loop's iteration count, then the
     * case-20 D_800A37A8[] code passed to func_80060414 (owner ruling 11) */
    s32 temp;
    s32 prim;
    /* holds two values: the g_disp_enable switch selector, then the case-20
     * D_800A37A0 limit (owner ruling 11) */
    s32 temp2;

    prim = (s32)D_800F33D8;
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    SetDefDrawEnv(&env, 0, (D_800A36AC & 1) ? 0xF0 : 0, 0x280, 0xF0);
    env.isbg = 0;
    PutDrawEnv(&env);
    g_gpu_ot_ptr = ot;
    ClearOTagR(ot, 2);
    temp2 = g_disp_enable;
    switch (temp2) {
    case 1:
    case 2:
        prim = func_8005D46C(prim, temp2);
        if (g_disp_fade != 0) {
            s32 i;
            temp = (rand() & 3) + 4;
            for (i = 0; i < temp; i++) {
                prim = func_8005D554(prim, g_disp_enable);
            }
        } else if ((rand() & 7) == 0) {
            func_8005D554(prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, prim, 0);
        break;
    case 20: {
        u8 cur;

        temp2 = D_800A37A0;
        cur = D_800A38F8;
        if ((u32)temp2 < cur) {
            break;
        }
        if (0xF0 / (temp2 + 1) >= ++D_800A37C0) {
            break;
        }
        /* FAKE: the common `D_800A38F8 = cur + 1` store written in both arms
         * (F7), as the target computes it per arm; one hoisted store: score 6
         * (131/136 insns). */
        if (cur == temp2) {
            D_800A38F8 = cur + 1;
        } else {
            u8 next = cur + 1;
            D_800A38F8 = next;
            D_800A37C0 = 0;
            temp = D_800A37A8[cur];
            if (next == temp2) {
                temp |= 0x8000;
            }
            func_80060414(temp, prim, 0);
        }
        break;
    }
    }
    DrawOTag(g_gpu_ot_ptr + 1);
    DrawSync(0);
}

void obj_ClearAll(void) {
    s32 i;
    /* FAKE: the table walked by byte offset, the target's one induction
       variable; indexed by record: score 4 */
    for (i = 7 * sizeof(Func80017A44Output); i >= 0;
         i -= sizeof(Func80017A44Output)) {
        ((Func80017A44Output *)((u8 *)g_file_data_buf + i))->points = 0;
    }
}

s32 obj_CalcOffset(s32 a0, s32 a1) { return (a0 << 6) + (a1 << 4); }

s32 math_Distance3D(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[3];

    in[0] = (a0[0] - a1[0]) >> 2;
    in[1] = (a0[1] - a1[1]) >> 2;
    in[2] = (a0[2] - a1[2]) >> 2;
    Square12(in, out);
    return SquareRoot12(out[0] + out[1] + out[2]) << 2;
}

s32 math_Distance3D_16(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[4];

    in[0] = (a0[0] - a1[0]) >> 4;
    in[1] = (a0[1] - a1[1]) >> 4;
    in[2] = (a0[2] - a1[2]) >> 4;
    Square12(in, out);
    return SquareRoot12(out[0] + out[1] + out[2]) << 4;
}

/* Adds an edge between records a and b unless a == b, both records have a
 * non-negative index, or an edge a->b or b->a already exists. Returns 1 when
 * an edge was added. */
s32 func_80017848(Func80017A44Output *out, s32 group_id, s32 a, s32 b) {
    s32 i;
    s32 dist;
    Func80017848Edge *edge;

    if (a == b) {
        return 0;
    }
    if (out->records[a].index >= 0 && out->records[b].index >= 0) {
        return 0;
    }
    for (i = 0; i < out->records[a].field_1C; i++) {
        if (out->edges[out->records[a].field_24[i]].ends.node.b == b) {
            return 0;
        }
    }
    for (i = 0; i < out->records[a].field_20; i++) {
        if (out->edges[out->records[a].field_2C[i]].ends.node.a == b) {
            return 0;
        }
    }
    dist = math_Distance3D(out->records[a].pos, out->records[b].pos);
    edge = &out->edges[out->edge_count];
    edge->dist = dist;
    edge->field_8 = dist * 3;
    edge->group_id = group_id;
    edge->ends.pair = (a << 16) | b;
    out->records[a].field_24[out->records[a].field_1C++] = out->edge_count;
    out->records[b].field_2C[out->records[b].field_20++] = out->edge_count;
    out->edge_count++;
    return 1;
}

void func_80017A44(Func80017A44Input *a0, Func80017A44Output *a1) {
    VECTOR pos;
    VECTOR center;
    s32 flag;
    Func80017A44Record *record_base;
    s16 *groups;
    s32 valid_count;
    s32 i;
    s32 j;
    s32 group_count;
    s32 group_id;
    s32 dist;
    s32 value;

    center.vx = 0;
    center.vy = 0;
    center.vz = 0;
    valid_count = 0;
    SetRotMatrix(a0->matrix);
    SetTransMatrix(a0->matrix);

    record_base = a1->records;
    for (i = 0; i < a0->count; i++) {
        record_base[i].index = a0->points[i].pad;
        RotTrans(&a0->points[i], &pos, &flag);
        pos.vx <<= 7;
        pos.vy <<= 7;
        pos.vz <<= 7;
        if (record_base[i].index >= 0) {
            valid_count++;
            center.vx += pos.vx;
            center.vy += pos.vy;
            center.vz += pos.vz;
        }
        record_base[i].pos[0] = pos.vx;
        record_base[i].pos[1] = pos.vy;
        record_base[i].pos[2] = pos.vz;
        record_base[i].field_C = 0;
        record_base[i].field_10 = 0;
        record_base[i].field_14 = 0;
        record_base[i].field_1C = 0;
        record_base[i].field_20 = 0;
    }

    center.vx /= valid_count;
    center.vy /= valid_count;
    center.vz /= valid_count;

    for (i = 0; i < a0->count; i++) {
        value = math_Distance3D_16((s32 *)&center, record_base[i].pos) >> 8;
        if (value > 0x100) {
            value = 0x100;
        }
        record_base[i].distance = value;
    }

    groups = a0->groups;
    while ((group_count = *groups++) != 0) {
        group_id = *groups++;
        for (i = 0; i < group_count - 1; i++) {
            dist =
                math_Distance3D_16((s32 *)&center, record_base[groups[i]].pos);
            for (j = i + 1; j < group_count; j++) {
                if (dist < math_Distance3D_16(
                               (s32 *)&center, record_base[groups[j]].pos)) {
                    func_80017848(a1, group_id, groups[i], groups[j]);
                } else {
                    func_80017848(a1, group_id, groups[j], groups[i]);
                }
            }
        }
        groups += group_count;
    }
}

s32 func_80017D84(Func80017A44Input *a0) {
    Func80017A44Output *p;
    s32 i;
    Func80017A44Record *c;

    p = g_file_data_buf;
    for (i = 0; i < 8; i++) {
        if (p->points == 0)
            break;
        p++;
    }
    if (i == 8)
        return -1;
    if (D_800A30E8 < i)
        D_800A30E8 = i;
    p->count = a0->count;
    p->points = a0->points;
    p->matrix = *a0->matrix;
    p->flags = a0->flags;
    c = (Func80017A44Record *)a0->buf;
    p->edge_count = 0;
    p->records = c;
    p->edges = (Func80017848Edge *)(c + p->count);
    func_80017A44(a0, p);
    return i;
}

void obj_Clear(s32 a0) { g_file_data_buf[a0].points = 0; }

void obj_UpdatePosition(s32 a0, s32 a1) {
    Func80017A44Output *ptr = &g_file_data_buf[a0];

    ptr->records = (Func80017A44Record *)((u8 *)ptr->records + a1);
    ptr->edges = (Func80017848Edge *)(ptr->records + ptr->count);
}

void obj_AddValue(s32 a0, s32 a1) {
    Func80017A44Output *ptr = &g_file_data_buf[a0];
    ptr->points = (SVECTOR *)((u8 *)ptr->points + a1);
}

void scratchpad_Save(void) {
    vu32 *src = (vu32 *)0x1F800000;
    u32 *dst = (u32 *)&g_scratchpad_save;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}

void scratchpad_Restore(void) {
    u32 *src = (u32 *)&g_scratchpad_save;
    vu32 *dst = (vu32 *)0x1F800000;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}

void sys_StubEmpty2(void) {}

void sys_StubEmpty3(s32 arg0, s32 arg1, s32 arg2) {}

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 D_800A30DC = 0x13400;
/* not named by any code or data: size from the gap */
s32 D_800A30E0[2] = {(s32)g_str_build_date, 0x190};
u8 D_800A30E8 = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
u8 D_800A3690;
u8 g_disp_fade;
s32 D_800A36AC;
u8 D_800A36B0;
u8 D_800A36F1;
u8 D_800A36F9;
s16 D_800A3710;
u8 D_800A3713;
u8 g_file_dma_flag;
u8 D_800A3744;
u32 *g_gpu_ot_ptr;
u8 g_disp_enable;
u32 D_800A3770[2];
s32 D_800A3784;
u8 D_800A3788;
u32 D_800A3798;
u8 D_800A37A0;
s32 D_800A37C0;
s16 D_800A3834;
u8 D_800A389A;
u32 *g_prim_buf_cursor;
s32 g_rng_state;
s16 D_800A38DC;
u8 D_800A38F8;
u8 D_800A3906;
u8 D_800A390D;
u8 D_800A3928;
