/* The game's boot and main loop: main(), the PCdrv host-file loaders
 * (pcdrv_LoadFile, pcdrv_LoadSectors), display setup wrappers, rng_SetSeed /
 * rng_Next and scratchpad_Save / scratchpad_Restore. .text 0x800164F8 (ROM
 * 0x6CF8). Start boundary: the first game .text object (LEGACY: a splat segment
 * edge). */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"
#include <psxsdk/libgte.h>
#include <psxsdk/libsn.h>

extern u8 D_800A3716;
extern s32 g_rng_state;
extern RECT g_gpu_clear_rect;
extern u32 g_scratchpad_save;
/* This file's .rodata: debug format strings, and the date string
 * ("Fri Aug  7 22:26:32 1998") that D_800A30E0 (below) points at. */
const char g_str_overflow[12] = "OVER FLOW\n";
const char g_str_eff_init[28] = "eff_init:%08x size:%08x\n";
const char g_str_limit[12] = "LIMIT:%08x\n";
const char g_str_prim_overflow[24] = "common prim over flow\n";
const char D_8001004C[28] = "Fri Aug  7 22:26:32 1998\n";

extern void printf();
extern void func_800164F8(void);

extern u8 g_softbody_slot_high_water;
extern RECT D_800A30D4;
extern s32 D_800A30DC;

extern void __main(void);
/* Not the bb2.h spelling: the definition takes an s16 first parameter, but
 * this call passes it unextended (an s16 prototype adds sll/sra here). */
extern s32 func_80060414(s32, s32, s32);

/* func_800164F8 -- polls the PC host 10000 times: pollhost() is PsyQ LIBSN.H's
 * `break 1024`, which ASPSX (and maspsx) encode as `break 1,0` (0x0001000D). */
void func_800164F8(void) {
    s32 i;

    for (i = 9999; i >= 0; i--) {
        pollhost();
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

s32 pcdrv_LoadSectors(s32 a0, u8 *dest, s32 arg2, s32 count) {
    s32 fd;
    s32 i;

    fd = PCopen(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    PClseek(fd, arg2 << 11, 0);
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

u32 func_800167AC(void) { return D_80106A50.flags & 1; }

u32 func_800167BC(void) { return (D_80106A50.flags >> 1) & 1; }

u32 func_800167D4(void) { return (D_80106A50.flags >> 2) & 1; }

void func_800167EC(void) {
    s32 i;
    /* FAKE: second handle to D_80106A50; using the global directly
     * everywhere: score 19; final times[0].unk_4 store through var_1: score 10
     */
    FileRecord *var_1 = &D_80106A50; /* SOTN: src/st/st0/2A218.c:48 @db41b28 */

    g_camera_distance_offset = 0;
    var_1->flags = 0;
    var_1->unk_00 = 0x7007;
    var_1->unk_04 = 0;
    for (i = 0; i < 3; i++) {
        var_1->times[i].unk_0 = 0;
        var_1->times[i].unk_1 = 0;
        var_1->times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}

void gpu_ResetGraphMode1(void) { ResetGraph(1); }

void gpu_HideResetAndClear(void) {
    SetDispMask(0);
    ResetGraph(1);
    ClearImage(&g_gpu_clear_rect, 0, 0, 0);
    DrawSync(0);
}

void gpu_SetDispMaskOn(void) { SetDispMask(1); }

void func_800168F0(void) {}

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
    D_800A3768 = DISP_DISABLED;
    D_800A36A8 = 0;
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
    cdrom_WaitIdle();
    cdrom_StartRead(cdrom_GetFileIndex(2, 0x61), (s32)arg0);
    cdrom_WaitIdle();
    PutDispEnv(&g_gpu_db[1].disp);
    DrawSync(0);
    LoadImage(&rect, (u32 *)(arg0 + 0x14));
    DrawSync(0);
    SetDispMask(1);

    for (i = 0; i < 0x96; i++) {
        if (i >= 0x79) {
            u16 *var_2 = (u16 *)(arg0 + 0x28);
            s32 j;

            for (j = 0; j < rect.w * rect.h; j++, var_2++) {
                u32 var_4 = *var_2;
                s32 value = var_4 & 0x1F;
                u32 temp;

                value >>= 1;
                /* FAKE: no-op mask of the u16 read; removed: score 1 (the andi
                 * goes) */
                var_4 &= 0xFFFF;
                temp = var_4 >> 1;
                temp &= 0x1E0;
                value += temp;
                var_4 >>= 1;
                var_4 &= 0x3C00;
                value += var_4;
                *var_2 = value;
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

void eff_ClearInitFlag(void) { D_800A3716 = 0; }

void eff_Init(void) {
    s32 size;

    if (D_800A3716 != 0) {
        return;
    }
    size = eff_LoadImagesAndSetup(0x801D8800, 0x8010E800);
    printf(g_str_eff_init, 0x8010E800, size);
    if (0xA000 < size) {
        sys_Panic();
    }
    D_800A3716 = 1;
}

extern void memcpy(u32, u32, s32);
extern s32 snd_VabFakeOpen(s32, s16);

void snd_InitAndLoadCommonVab(void) {
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

void func_80016D78(void) {
    printf(g_str_limit, 0x8010DB00);
    func_800167EC();
    motion_InitBuffers();
    D_800A3770[0] = 0x801D8800;
    D_800A3770[1] = 0x801EBC00;
    D_800A3798 = 0x13400;
    D_800A3716 = 0;
    D_800A3906 = 0;
    snd_InitAndLoadCommonVab();
    pad_ResetStateMarkValid();
    font_LoadImage();
    fight_InitSettings();
    D_800A36F9 = 0;
    D_800A3690 = 0;
    D_800A3744 = 0;
    D_800A3745 = 0;
    D_800A3746 = 0;
    func_80046B44();
    D_800A36F1 = 2;
    D_800A38C4[1] = 0;
    D_800A36B0 = 0;
    g_comb_abort_request = 0;
}

void gpu_WaitDrawSync(void) { DrawSync(0); }

void menu_RunPauseMenu(GpuDb *arg0, s32 arg1) {
    u32 var_0[2];
    GpuDb *var_1;
    s32 var_2;
    s32 var_3;
    s32 limit;
    u32 var_5;
    s32 idx;
    GpuDb *var_7;

    var_2 = 0;
    var_3 = 0;
    /* FAKE: pass-through handle on the parameter, used once at DrawOTag;
       moves the prologue's $s5 save/init group from first to third. Removed:
       score 4 (pointer-alias-fake-exception) */
    var_7 = arg0;
    if (D_800A38DC == 2) {
        var_3 = D_800A389A < 1;
    }
    limit = 3;
    if (var_3 != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    snd_QueueSound(3, 0x7F, 0x7F);
    var_5 = D_800A3770[D_800A36AC & 1];

    while (1) {
        idx = D_800A36AC & 1;
        g_prim_buf_cursor = (u32 *)(var_5 + (idx * 0x9A00));
        g_gpu_ot_ptr = &var_0[idx];
        var_1 = &g_gpu_db[idx];

        ClearOTagR(g_gpu_ot_ptr, 1);
        pad_Update();
        if (var_3 != 0) {
            func_8005C8A8(
                2, var_2 | (D_800A3788 << 16), (s32)g_prim_buf_cursor, 0);
        } else {
            func_8005C8A8(0, var_2, (s32)g_prim_buf_cursor, 0);
        }
        cdrom_Update();
        snd_FlushSoundQueue();
        DrawSync(0);
        VSync(2);
        /* FAKE: do-while(0) raises var_1's loop-depth weight above var_2's,
           seating var_1 in $s0 and var_2 in $s1; unwrapped: score 21
           (do-while-zero-exception) */
        do {
            PutDispEnv(&var_1->disp);
            PutDrawEnv(&var_1->draw);
        } while (0);
        DrawOTag(&var_7->ot[0x1007]);
        DrawOTag(g_gpu_ot_ptr);
        D_800A36AC++;

        if (g_pad_state.pressed & 0x100010) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_2 = 0;
            break;
        }
        if (g_pad_state.pressed & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            break;
        }
        if (g_pad_state.pressed & 0x10001000) {
            snd_QueueSound(0, 0x7F, 0x7F);
            var_2 = (var_2 == 0) ? limit - 1 : var_2 - 1;
        } else if (g_pad_state.pressed & 0x40004000) {
            snd_QueueSound(0, 0x7F, 0x7F);
            var_2 = (var_2 == limit - 1) ? 0 : var_2 + 1;
        }

        if ((var_3 != 0) && (var_2 >= 3)) {
            if (g_pad_state.pressed & 0x80008000) {
                snd_QueueSound(0, 0x7F, 0x7F);
                D_800A3788 |= 1 << (var_2 - 3);
            } else if (g_pad_state.pressed & 0x20002000) {
                snd_QueueSound(0, 0x7F, 0x7F);
                D_800A3788 &= ~(1 << (var_2 - 3));
            }
        }
    }

    if (var_2 != 0) {
        if (var_2 == 1) {
            D_800A31DA = 1;
            D_800A3834 = 8;
            cdrom_PauseAndWait();
        } else if (var_2 == 2) {
            D_800A3834 = 8;
            cdrom_PauseAndWait();
        }
    }

    DrawSync(0);
    ResetRCnt(0xF2000001);
    D_800A36B0 = 1;
}

void rng_SetSeed(s32 a0) { g_rng_state = a0; }

s32 rng_Next(void) {
    s32 var_0 = g_rng_state;
    s32 result = var_0 * 5497 + 0x7FA9;
    var_0 = (var_0 >> 16) ^ result;
    g_rng_state = var_0;
    return var_0 & 0x7FFF;
}

void main(void) {
    s32 idx;
    GpuDb *var_1;
    u32 *var_2;
    /* D_800A390D (frames left to skip presenting) as read this frame; the next
     * frame passes it to pad_Update. */
    s32 var_3;
    /* FAKE: second handle on D_800A3770 keeps its address in $s4 for the
     * whole loop; D_800A3770[idx] at both uses: score 14 */
    u32 *var_4;

    __main();
    SetSp(0x801FFF00);
    SetMem(2);
    sys_Init();
    func_80016D78();
    SetDispMask(1);
    func_80016A8C((u8 *)0x80118800, var_1, idx);

    var_4 = D_800A3770;
    D_800A3834 = 0xF;
    D_800A390D = 0;
    D_800A36AC = 0;

loop:
    idx = D_800A36AC & 1;
    var_1 = &g_gpu_db[idx];
    var_2 = var_1->ot;
    ClearOTagR(var_2, 0x1008);
    g_gpu_ot_ptr = var_2;
    g_prim_buf_cursor = (u32 *)var_4[idx];
    eff_SelectQuadBuffer(idx);
    font_BeginFrame();
    pad_Update(var_3);
    cdrom_Update();
    snd_FlushSoundQueue();

    if (g_comb_abort_request != 0) {
        cdrom_PauseAndWait();
        D_800A3768 = DISP_DISABLED;
        g_comb_abort_request = 0;
        D_800A31DA = 0;
        D_800A3834 = 8;
    }

    D_8008D090[D_800A3834]();
    font_AddTpage();

    do {
        if (GetRCnt(0xF2000001u) >= ((D_800A36F1 - 1) << 8) + 0x80)
            break;
        rand();
    } while (1);

    VSync(1);
    DrawSync(0);
    VSync(0);
    ResetRCnt(0xF2000001u);

    var_3 = D_800A390D;
    if (var_3 == 0) {
        PutDispEnv(&var_1->disp);
        PutDrawEnv(&var_1->draw);
    }

    {
        s32 remaining = var_4[idx] + 0x13400 - (s32)g_prim_buf_cursor;
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
        DrawOTag(&var_1->ot[0x1007]);
        D_800A36AC++;
    }

    if (D_800A3834 != 1)
        goto loop;
    if (var_3 != 0)
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
    menu_RunPauseMenu(var_1, idx);
    goto loop;
}

void func_800174F4(void) {
    u32 var_0[2];
    DRAWENV env;
    /* holds two values: the case-1/2 fade loop's iteration count, then the
     * case-20 D_800A37A8[] code passed to func_80060414 (owner ruling 11) */
    s32 temp;
    s32 var_3;
    /* holds two values: the D_800A3768 switch selector, then the case-20
     * D_800A37A0 limit (owner ruling 11) */
    s32 temp2;

    var_3 = (s32)D_800F33D8;
    if (D_800A3768 == DISP_DISABLED) {
        return;
    }
    SetDefDrawEnv(&env, 0, (D_800A36AC & 1) ? 0xF0 : 0, 0x280, 0xF0);
    env.isbg = 0;
    PutDrawEnv(&env);
    g_gpu_ot_ptr = var_0;
    ClearOTagR(var_0, 2);
    temp2 = D_800A3768;
    switch (temp2) {
    case 1:
    case 2:
        var_3 = func_8005D46C(var_3, temp2);
        if (D_800A36A8 != 0) {
            s32 i;
            temp = (rand() & 3) + 4;
            for (i = 0; i < temp; i++) {
                var_3 = func_8005D554(var_3, D_800A3768);
            }
        } else if ((rand() & 7) == 0) {
            func_8005D554(var_3, D_800A3768);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, var_3, 0);
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
            func_80060414(temp, var_3, 0);
        }
        break;
    }
    }
    DrawOTag(g_gpu_ot_ptr + 1);
    DrawSync(0);
}

void softbody_FreeAll(void) {
    s32 i;
    /* FAKE: the table walked by byte offset, the target's one induction
       variable; indexed by record: score 4 */
    for (i = 7 * sizeof(Func80017A44Output); i >= 0;
         i -= sizeof(Func80017A44Output)) {
        ((Func80017A44Output *)((u8 *)g_softbody_slots + i))->points = 0;
    }
}

s32 softbody_GetBufferSize(s32 a0, s32 a1) { return (a0 << 6) + (a1 << 4); }

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
 * non-negative field_18, or an edge a->b or b->a already exists. Returns 1
 * when an edge was added. */
s32 softbody_AddSpring(Func80017A44Output *out, s32 arg1, s32 a, s32 b) {
    s32 i;
    s32 dist;
    Func80017848Edge *edge;

    if (a == b) {
        return 0;
    }
    if (out->nodes[a].field_18 >= 0 && out->nodes[b].field_18 >= 0) {
        return 0;
    }
    for (i = 0; i < out->nodes[a].a_edge_count; i++) {
        if (out->edges[out->nodes[a].a_edges[i]].ends.node.b == b) {
            return 0;
        }
    }
    for (i = 0; i < out->nodes[a].b_edge_count; i++) {
        if (out->edges[out->nodes[a].b_edges[i]].ends.node.a == b) {
            return 0;
        }
    }
    dist = math_Distance3D(out->nodes[a].pos, out->nodes[b].pos);
    edge = &out->edges[out->edge_count];
    edge->dist = dist;
    edge->field_8 = dist * 3;
    edge->group_id = arg1;
    edge->ends.pair = (a << 16) | b;
    out->nodes[a].a_edges[out->nodes[a].a_edge_count++] = out->edge_count;
    out->nodes[b].b_edges[out->nodes[b].b_edge_count++] = out->edge_count;
    out->edge_count++;
    return 1;
}

void softbody_Build(Func80017A44Input *a0, Func80017A44Output *a1) {
    VECTOR pos;
    VECTOR var_1;
    s32 flag;
    Func80017A44Record *record_base;
    s16 *var_4;
    s32 var_5;
    s32 i;
    s32 j;
    s32 var_8;
    s32 var_9;
    s32 dist;
    s32 value;

    var_1.vx = 0;
    var_1.vy = 0;
    var_1.vz = 0;
    var_5 = 0;
    SetRotMatrix(a0->matrix);
    SetTransMatrix(a0->matrix);

    record_base = a1->nodes;
    for (i = 0; i < a0->count; i++) {
        record_base[i].field_18 = a0->points[i].pad;
        RotTrans(&a0->points[i], &pos, &flag);
        pos.vx <<= 7;
        pos.vy <<= 7;
        pos.vz <<= 7;
        if (record_base[i].field_18 >= 0) {
            var_5++;
            var_1.vx += pos.vx;
            var_1.vy += pos.vy;
            var_1.vz += pos.vz;
        }
        record_base[i].pos[0] = pos.vx;
        record_base[i].pos[1] = pos.vy;
        record_base[i].pos[2] = pos.vz;
        record_base[i].vel_x = 0;
        record_base[i].vel_y = 0;
        record_base[i].vel_z = 0;
        record_base[i].a_edge_count = 0;
        record_base[i].b_edge_count = 0;
    }

    var_1.vx /= var_5;
    var_1.vy /= var_5;
    var_1.vz /= var_5;

    for (i = 0; i < a0->count; i++) {
        value = math_Distance3D_16((s32 *)&var_1, record_base[i].pos) >> 8;
        if (value > 0x100) {
            value = 0x100;
        }
        record_base[i].field_34 = value;
    }

    var_4 = a0->groups;
    while ((var_8 = *var_4++) != 0) {
        var_9 = *var_4++;
        for (i = 0; i < var_8 - 1; i++) {
            dist = math_Distance3D_16((s32 *)&var_1, record_base[var_4[i]].pos);
            for (j = i + 1; j < var_8; j++) {
                if (dist < math_Distance3D_16(
                               (s32 *)&var_1, record_base[var_4[j]].pos)) {
                    softbody_AddSpring(a1, var_9, var_4[i], var_4[j]);
                } else {
                    softbody_AddSpring(a1, var_9, var_4[j], var_4[i]);
                }
            }
        }
        var_4 += var_8;
    }
}

s32 softbody_Create(Func80017A44Input *a0) {
    Func80017A44Output *p;
    s32 i;
    Func80017A44Record *c;

    p = g_softbody_slots;
    for (i = 0; i < 8; i++) {
        if (p->points == 0)
            break;
        p++;
    }
    if (i == 8)
        return -1;
    if (g_softbody_slot_high_water < i)
        g_softbody_slot_high_water = i;
    p->count = a0->count;
    p->points = a0->points;
    p->matrix = *a0->matrix;
    p->flags = a0->flags;
    c = (Func80017A44Record *)a0->buf;
    p->edge_count = 0;
    p->nodes = c;
    p->edges = (Func80017848Edge *)(c + p->count);
    softbody_Build(a0, p);
    return i;
}

void softbody_Free(s32 a0) { g_softbody_slots[a0].points = 0; }

void softbody_RebaseBuffer(s32 a0, s32 a1) {
    Func80017A44Output *ptr = &g_softbody_slots[a0];

    ptr->nodes = (Func80017A44Record *)((u8 *)ptr->nodes + a1);
    ptr->edges = (Func80017848Edge *)(ptr->nodes + ptr->count);
}

void softbody_RebasePoints(s32 a0, s32 a1) {
    Func80017A44Output *ptr = &g_softbody_slots[a0];
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

void func_80017F90(void) {}

void func_80017F98(s32 arg0, s32 arg1, s32 arg2) {}

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 D_800A30DC = 0x13400;
/* not named by any code or data: size from the gap */
s32 D_800A30E0[2] = {(s32)D_8001004C, 0x190};
u8 g_softbody_slot_high_water = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
u8 D_800A3690;
u8 D_800A36A8;
s32 D_800A36AC;
u8 D_800A36B0;
u8 D_800A36F1;
u8 D_800A36F9;
s16 g_camera_distance_offset;
u8 D_800A3713;
u8 D_800A3716;
u8 D_800A3744;
u32 *g_gpu_ot_ptr;
u8 D_800A3768;
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
u8 g_comb_abort_request;
