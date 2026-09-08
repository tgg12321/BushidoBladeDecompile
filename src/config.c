#include "common.h"
#include "include_asm.h"
#include "game.h"

/* Rodata owned by config.c per func_8003FA24's reference at asm/funcs/func_8003FA24.s:240-241.
 * Re-attributed from asm/data/101C.rodata_c2_post.s 2026-06-09 (rodata-cleanup project,
 * docs/rodata-cleanup-project.md). Named per named_syms.txt alias g_str_multipul_model_80010D8C.
 * Fixed [16] to match the asm/data block's exact byte content (14 chars + null + 1 pad). */
const char D_80010D8C[16] = "Multipul Model";

/* Forward declarations */
extern s32 stage_GetId(void);
extern void sys_StubEmpty3(s32, s32, s32);
extern void *func_8004153C(void);
extern void obj_ClearAll(void);
extern void sys_StubEmpty2(void);
extern void obj_Clear(s32);

/* Externs for globals */
extern s32 g_game_mode;
extern s32 g_game_player_count;
extern s32 g_game_flag_b;
extern s32 g_game_flag_a;
extern s32 g_stage_init_tbl;
extern void func_8001924C(s32 *, s32);
extern void func_80045A28(s32, s32);
extern void func_80052A20(s32 *, s32 *, s16 *);
extern void func_80052C10(void);

/* Externs for globals */
extern u8 g_char_data;
extern s16 g_game_p1_ctrl;
extern u8 g_stage_collision[];
extern s32 D_80094A6C[];
extern u8 *D_800A3708;
extern s32 g_stage_light_pos;
extern s32 D_800A93B4;
extern s32 D_800A93B8;
extern s32 g_stage_light_dir;
extern s32 D_800A93C0;
extern s32 D_800A93C4;

/* --- Functions 0x8003F168 - 0x8004019C --- */
void stage_ExecInitFunc(void) {
    s32 v0 = *(s32 *)((u32)&g_stage_init_tbl + (stage_GetId() << 3));
    if (v0) {
        (*(void (**)(void))((u32)&g_stage_init_tbl + (stage_GetId() << 3)))();
    }
}
s32 game_GetMode(void) {
    return g_game_mode;
}

void *game_GetCharData(void) {
    return &g_char_data;
}

void game_SetControllerPorts(s32 a0) {
    if (a0) {
        g_game_p1_ctrl = 3;
        g_game_p2_ctrl = 2;
    } else {
        g_game_p1_ctrl = 0;
        g_game_p2_ctrl = 1;
    }
}

void game_SetPlayerCount(s32 a0) {
    if ((u32)a0 >= 2) {
        return;
    }
    if (a0 == g_game_player_count) {
        return;
    }
    g_game_player_count = a0;
    if (!a0) {
        game_SetControllerPorts(0);
    }
    g_game_mirror_mode = (s16)g_game_player_count;
}
s32 game_GetPlayerCount(void) {
    return g_game_player_count;
}
void stage_InitCollision(void) {
    s32 i, j;
    s32 col_center, row_center;
    s32 data;
    s32 adj_i;
    s32 adj_j;
    s32 *ptr = (s32 *)g_stage_collision;

    i = 0xFF;
    do {
        *ptr = 0;
        i--;
        ptr++;
    } while (i >= 0);

    game_GetPlayerCount();

    col_center = (*(s32 *)(D_800A3708 + 0x4C) + 0x7D00) / 2000;
    row_center = (*(s32 *)(D_800A3708 + 0x54) + 0x7D00) / 2000;

    for (i = 0; i < 16; i++) {
        adj_i = i - 8;
        {
            u32 y = (u32)(row_center + adj_i);
            if (y < 0x20) {
                data = D_80094A6C[i];
                for (j = 0; j < 16; j++) {
                    adj_j = j - 8;
                    {
                        u32 x = (u32)(col_center + adj_j);
                        if (x < 0x20) {
                            s32 bits = (data >> ((15 - j) * 2)) & 3;
                            g_stage_collision[(y << 5) + x] |= bits;
                        }
                    }
                }
            }
        }
    }
}
void func_8003F388(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_stage_collision[y * 32 + x] |= 0x4;
    }
}
void func_8003F3D4(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_stage_collision[y * 32 + x] |= 0x8;
    }
}
void func_8003F420(s32 a0, s32 a1) {
    s32 s3, s2, s1;
    s32 s0;
    a0 += 0x7D00;
    a1 += 0x7D00;
    s3 = a0 / 2000;
    s1 = a0 - s3 * 2000;
    s0 = a1 / 2000;
    s2 = s0;
    s0 = a1 - s2 * 2000;
    if (s1 < 1000) {
        s1 = -1;
    } else {
        s1 = 1;
    }
    if (s0 < 1000) {
        s0 = -1;
    } else {
        s0 = 1;
    }
    stage_SetCollision(s3, s2, 2);
    stage_SetCollision(s3 + s1, s2, 2);
    stage_SetCollision(s3, s2 + s0, 2);
    stage_SetCollision(s3 + s1, s2 + s0, 2);
}

void stage_SetCollision(s32 a0, s32 a1, s32 a2) {
    g_stage_collision[a1 * 32 + a0] = a2 & 3;
}

u32 stage_GetCollision(s32 a0, s32 a1) {
    return g_stage_collision[a1 * 32 + a0];
}

void stage_ClearLighting(void) {
    g_game_flag_b = 0;
    g_game_flag_a = 0;
    D_800A93B8 = 0;
    D_800A93B4 = 0;
    g_stage_light_pos = 0;
    D_800A93C4 = 0;
    D_800A93C0 = 0;
    g_stage_light_dir = 0;
}

void stage_SetLightPosDir(s32 a0, s32 a1, s32 a2) {
    (&g_stage_light_pos)[a2] = a0;
    (&g_stage_light_dir)[a2] = a1;
}

void stage_ApplyLighting(void) {
    sys_StubEmpty3(g_stage_light_pos, g_stage_light_dir, 0);
    sys_StubEmpty3(D_800A93B4, D_800A93C0, 1);
    sys_StubEmpty3(D_800A93B8, D_800A93C4, 2);
}

void func_8003F62C(s32 *a0) {
    s16 *s0;
    s32 *s1 = a0;
    s0 = (s16 *)s1[9];
    if (s0 == 0) return;
    if (s0[3]) {
        func_8004016C(*(s16 *)((u8 *)s1 + 4));
        func_8003F824(s1, 0);
    }
    if (s0[1]) {
        func_8004001C((u8 *)s0);
    }
    func_8003F6D8(s0);
    func_8001924C((s32 *)((u8 *)s0 + 0x418), s0[0]);
    if (s0[1]) {
        func_80040068((u8 *)s0);
        s0[1] = 0;
    }
}
typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 *objs[5];
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ s32 quads[3][4];
} Func8003F6D8Inner; /* size 0xB4; sits at +0x1C of each 0xD0-byte record, records start at arg0+8 */

void func_8003F6D8(s16 *arg0) {
    s32 i;
    s32 j;

    for (i = 0; i < arg0[0]; i++) {
        s32 off = i * 0xD0 + 8;
        Func8003F6D8Inner *in = (Func8003F6D8Inner *)((u8 *)arg0 + off + 0x1C);
        for (j = 0; j < in->count; j++) {
            s32 *obj = in->objs[j] + 6;
            func_80052A20(obj, in->quads[j], in->pairs[j]);
            func_80052A20(obj, in->quads[j] + 2, in->pairs[j] + 8);
        }
    }
}

void func_8003F7F4(void) {
    obj_ClearAll();
    sys_StubEmpty2();
    g_game_flag_a = 0;
    g_game_flag_b = 0;
}
typedef struct {
    /* 0x00 */ s32 v[4];
} SceneQuad;

typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ SceneQuad quad;
    /* 0x14 */ u8 *obj;
    /* 0x18 */ u8 *cur;
    /* 0x1C */ Func8003F6D8Inner inner;
} SceneRec; /* size 0xD0 */

typedef struct {
    /* 0x000 */ s16 count;
    /* 0x002 */ s16 unk2;
    /* 0x004 */ s16 unk4;
    /* 0x006 */ s16 unk6;
    /* 0x008 */ SceneRec recs[5];
    /* 0x418 */ SceneQuad quads[5];
    /* 0x468 */ u8 data[1];
} Scene;

extern u8 *func_8003FA24(SceneRec *rec, s16 *cmds, u8 *cur);
void func_8003FECC(s32 *a0, s32 *a1, s16 *a2);

void func_8003F824(u8 *arg0, s32 arg1) {
    Scene *sc;
    s16 *cmds;
    u8 *cur;
    SceneRec *rec;
    u8 *obj;
    s32 i;
    s16 c;

    sc = *(Scene **)(arg0 + 0x24);
    if (sc == 0) return;
    cmds = *(s16 **)(arg0 + 0x28);
    cur = sc->data;
    if (*cmds == -3) {
        *(Scene **)(arg0 + 0x24) = 0;
        return;
    }
    sc->count = 0;
    sc->unk2 = 0;
    sc->unk4 = 0;
    sc->unk6 = 0;
    for (i = 0; *cmds != -3; i++) {
        c = *cmds;
        if (c != -2) {
            if (sc->count >= 5) {
                func_80052C10();
            }
            rec = &sc->recs[sc->count];
            obj = ((u8 **)(arg0 + 0x1A34))[i];
            rec->cur = cur;
            rec->obj = obj;
            cur = func_8003FA24(rec, cmds, cur);
            sc->quads[sc->count] = rec->quad;
            *obj = 0xD;
            sc->count++;
            if (*cmds >= 0) {
                cmds++;
                while (*cmds >= 0) {
                    cmds++;
                }
            }
            if (*cmds == -1) {
                cmds++;
                func_8003FECC((s32 *)arg0, (s32 *)rec, cmds);
            }
        }
        while (*cmds++ != -2) {
        }
    }
    if (arg1) {
        func_80045A28(*(s16 *)(arg0 + 4), cur - *(u8 **)(arg0 + 0x1C));
    }
}
INCLUDE_ASM("asm/funcs", func_8003FA24);
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2) {
    s32 i;
    i = 0;
    if (a1 > i) {
        s16 fill = -256;
        s16 *p = a0;
        for (i = 0; i < a1; i++) {
            *(s16 *)((u8 *)p + 6) = fill;
            p = (s16 *)((u8 *)p + 8);
        }
    }

    {
        s32 count;
        count = a2[0];
        a2++;
        if (count >= 0) {
            do {
                int val;
                val = a2[0];
                a2++;
                count--;
                while (count != -1) {
                    s32 addr;
                    i = a2[0];
                    a2++;
                    count--;
                    addr = (i << 3) + (s32)a0;
                    *(s16 *)(addr + 6) = val;
                }
                count = a2[0];
                a2++;
            } while (count >= 0);
        }
    }
    return (s16 *)a2;
}

void func_8003FECC(s32 *a0, s32 *a1, s16 *a2)
{
  s32 t2;
  u8 *new_var2;
  s16 v1;
  unsigned int t3;
  s32 *a3;
  s32 *t1;
  u16 t0;
  t2 = *((s32 *) (((u8 *) a1) + 0x1C));
  ;
  t0 = (u16) a2[0];
  a1 = (s32 *) (((u8 *) a1) + 0x1C);
  if (a2[0] == (-2))
  {
    goto end;
  }
  t3 = -2;
  a3 = (s32 *) ((t2 * 0x10) + (s32)(u8 *) a1);
  t1 = (s32 *) ((t2 * 4) + (s32)(u8 *) a1);
  loop:
  {
    a2++;
    {
      s32 v0;
      v1 = 1;
      v0 = (s32) (((u8 *) a0) + ((s32) ((((s16) t0) * 0x68) + 0x94)));
      t1[v1] = v0;
    }
    {
      u16 val = (u16) (*(a2++));
      t2++;
      *((u16 *) (((u8 *) a3) + 0x84)) = val;
    }
    *((u16 *) (((u8 *) a3) + 0x86)) = (u16) (*(a2++));
    *((u16 *) (((u8 *) a3) + 0x88)) = (u16) (*(a2++));
    *((u16 *) (((u8 *) a3) + 0x8C)) = (u16) (*(a2++));
    new_var2 = ((u8 *) a3) + 0x90;
    *((u16 *) (((u8 *) a3) + 0x8E)) = (u16) (*(a2++));
    *((u16 *) new_var2) = (u16) (*(a2++));
    {
      s32 val78 = (s32) (*(a2++));
      /* FAKE: split advance (+8 +8); the extra refs seat this pointer in $a3
         ahead of the halfword temp (combine re-merges to one addiu). */
      a3 = (s32 *) (((u8 *) a3) + 8);
      a3 = (s32 *) (((u8 *) a3) + 8);
      *((s32 *) (((u8 *) t1) + 0x78)) = val78;
    }
    t0 = (u16) (*a2);
    v1 = *a2;
    /* FAKE: split advance (+2 +2); the extra refs seat this pointer in $t1
       ahead of the entry counter (combine re-merges to one addiu). */
    t1 = (s32 *) (((u8 *) t1) + 2);
    t1 = (s32 *) (((u8 *) t1) + 2);
  }

  if (v1 != ((s16) t3))
  {
    goto loop;
  }
  end:
  a1[0] = t2;

  a1[5] = t2;
}
s32 func_8003FFA8(s32 a0) {
    if (a0 & 3) {
        a0 = (a0 + 3) & ~3;
    }
    return a0;
}
void func_8003FFC4(s32 *a0) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        v1[3] = 1;
    }
}
void func_8003FFE0(void) {
    s32 *v0 = (s32 *)func_8004153C();
    if (v0) {
        s16 *v1 = (s16 *)v0[9];
        if (v1) {
            v1[1] = 1;
        }
    }
}
void func_8004001C(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 1;
        a0[0xE + i * 0xD0] = 1;
    }
}
void func_80040068(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 0;
        a0[0xE + i * 0xD0] = 0;
    }
}
void func_800400B0(s32 *a0, s32 a1) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        s32 i;
        for (i = 0; i < v1[0]; i++) {
            *(s32 *)((u8 *)v1 + i * 0xD0 + 0x34) = a1;
        }
    }
}
/* Judge-ruled form (s2 grind, 2026-07-14): the variable compare `s2[0] > s0`
 * is load-bearing — target's 0x28 frame is the combine-leftover of the folded
 * guard (phantom slot sp+20); a literal `> 0` compare yields frame 0x20 + RA
 * swap. Every statement here is live. Do not respell. */
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}

void func_8004016C(void) {
    void *v0 = func_8004153C();
    if (v0) {
        func_800400F8(v0);
    }
}

void func_8004019C(s32 *a0, s32 a1) {
    s32 *v1 = (s32 *)a0[9];
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0[9] = (s32)v1;
        a0[10] = a0[10] + a1;
        *(s16 *)((s32)v1 + 6) = 1;
    }
}
