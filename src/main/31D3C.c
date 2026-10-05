/* 21 game functions. .text 0x8004153C (ROM 0x31D3C). Start boundary: LEGACY (inside the -G8 run,
 * no evidence either way). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

























/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */








extern u8 D_80094D40[];


s32 *func_8004153C(s32 a0) {
    return g_player_ptrs[a0];
}
s32 func_80041554(s32 a0) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        return ptr[4];
    }
    return -1;
}
s32 func_80041584(void) {
    s32 i;
    s32 ret = -1;
    for (i = 0; i < 3; i++) {
        if (g_player_ptrs[i] == 0) {
            ret = i;
            break;
        }
    }
    return ret;
}
void player_Destroy(s32 a0) {
    func_8004016C(a0);
    func_80045A50(a0);
    g_player_ptrs[a0] = 0;
}
void player_SetCharId(s32 a0, s32 a1) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        if ((val & 0x1F) != a1) {
            ptr[3] = -2;
        }
    }
    g_player_char_ids[a0] = a1;
}
s32 func_80041650(s32 a0) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        return val & 0x1F;
    }
    return -1;
}

/* func_80041688 (gnd_init_80041688): sets or clears bit 0 of the flag byte
 * in every bone record of player arg0, then pushes the player's colour
 * (greyscaled when func_800486FC() is set) through func_80041398. */
void func_80041688(s32 arg0, s32 arg1) {
    volatile u32 pre_pad[8]; /* !FAKE: target frame 0x38 keeps 0x20 leading
        bytes allocated-but-untouched (only the sp/ra/s0 offset immediates
        depend on it); phantom-frame-slot family, owner-sanctioned pad row
        (engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS). */
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;


    player = g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) {
        p[1] |= 1;
    } else {
        p[1] &= ~1;
    }

    i = 1;
loop1:
    p += 0x68;
    /* FAKE: guard staged through the existing local b (value consumed by
       the branch below; b's real color-byte assignment follows later and
       this staged value is dead before it), mechanism: sched.c
       adjust_priority -> birthing_insn_p reg_n_sets==1 launch-boost gate
       (second live set of b turns off the FALSE-arm lbu's LAUNCH_PRIORITY
       so it is picked last = emitted first, matching target [b,r,g]);
       staged-value-reused-variable. */
    b = *(s16 *)(p + 2) >= 0;
    if (b) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = math_Grayscale3(b, g, r);
        func_80041398((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        func_80041398(b | ((r << 16) | (g << 8)));
    }
}
typedef struct { s32 w[4]; } Block16;
extern void gte_SetMatrixRotTransIRVec(void *, void *, void *);
void func_800417D0(Unk80101DF0Record *a0) {
    AnimRotFunc func;

    if (a0->unk6 == 1) {
        return;
    }
    if (a0->unk6 != 2) {
        func = g_anim_func_table[a0->unk8];
        func(&a0->xf.rot, &a0->work);
    }
    if (a0->unkC != 0) {
        if (a0->unkC->unk6 == 0) {
            func_800417D0(a0->unkC);
        }
        gte_MulMatrix0ClearTrans(&a0->unkC->xf.mat, &a0->work, &a0->xf.mat);
        gte_SetMatrixRotTransIRVec(&a0->unkC->xf.mat, a0->work.t, a0->xf.mat.t);
    } else {
        a0->xf.mat = a0->work;
    }
    a0->unk6 = 1;
}
void func_800418D0(s32 *a0) {
    SVECTOR sp10;
    AnimRotFunc func;
    sp10.vx = -(u16)((u16 *)a0)[8];
    sp10.vy = -(u16)((u16 *)a0)[9];
    sp10.vz = -(u16)((u16 *)a0)[10];
    func = g_anim_func_table[((s16 *)a0)[4]];
    func(&sp10, (MATRIX *)(a0 + 14));
    ((Block16 *)(a0 + 6))[0] = ((Block16 *)(a0 + 14))[0];
    ((Block16 *)(a0 + 6))[1] = ((Block16 *)(a0 + 14))[1];
}
void func_80041988(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 mask_table;
    s32 bit;
    s32 i;
    s32 one;

    if ((u32)a0 >= 2) {
        return;
    }
    mask_table = D_80094D40[a1];
    bit = 0x10;
    i = 0;
    do {
        if (!(mask_table & bit) || !(a2 & bit)) {
            goto shift;
        }
        /* FAKE: constant-holder biasing RA (named-local-fake-exception,
         * SOTN `s16 three = 3;`). A literal `a0 == 1` materializes the 1 as a
         * compiler-generated pseudo (!REG_USERVAR_P) that loop.c move_movables
         * hoists to the preheader; reload then rematerializes it in-loop as $v1.
         * A named local (REG_USERVAR_P) is not movable, stays in-loop, and global
         * RA assigns $v0 — matching target's in-loop `addiu v0,zero,1`. The store
         * is live (feeds the compare). A literal if-chain or switch(a0) does
         * not reproduce it. */
        one = 1;
        if (a0 == 0) {
            goto case0;
        }
        if (a0 == one) {
            goto case1;
        }
        goto shift;
    case0:
        if (func_8003E2A0() == 0) {
            func_800480C0(a3, i + 1, 0, 0, -0x140, 0xE8);
        } else {
            func_80047EE8(a3, i + 1);
        }
        goto shift;
    case1:
        if (func_8003E2A0() == a0) {
            func_800480C0(a3, i + 1, 0x80, 0, -0x140, 0xE8);
        } else {
            func_80047FBC(a3, i + 1, 0x80, 0);
        }
    shift:
        bit >>= 1;
        i++;
    } while (i < 5);
}

extern s32 D_80094DF0[];
extern u8 D_80094E08[];
extern s16 D_800A9A20;
extern u16 g_gpu_store_buf;
void func_80041AC8(s16 *arg0)
{
  s16 rect[4];
  s16 *var_s0;
  u16 *var_s1;
  s32 var_s2;
  s32 var_s3;
  u16 v1_val;
  s16 *id_ptr;
  if (arg0[2] != 1)
  {
    return;
  }
  if (D_80094E08[arg0[4]] == 0xFF)
  {
    return;
  }
  /* id_ptr spelling: target's store->reload order (sh D_800A9A20 BEFORE
   * lh arg0[4]) is only producible when the reload is NOT spelled as plain
   * pointer-indexing (GCC 2.7.2 MEM_IN_STRUCT_P /s flag + sched.c escape
   * clause) — the original source provably used a non-indexed spelling.
   * One representative spelling sanctioned by owner policy; see
   * .claude/rules/proven-spelling-class-reconstruction.md. */
  id_ptr = &arg0[4];
  D_800A9A20 = arg0[4];
  var_s0 = (s16 *) D_80094DF0[D_80094E08[*id_ptr]];
  if (func_8003E2A0() != 1)
  {
    goto else_lbl;
  }
  var_s3 = -0x140;
  var_s2 = 0xF0;
  goto after_if;
  else_lbl:
  var_s3 = 0x80;

  var_s2 = 0;
  after_if:
  v1_val = (u16) (*var_s0);

  if ((*var_s0) >= 0)
  {
    s32 w = 0x10;
    s32 h = 1;
    var_s1 = &g_gpu_store_buf;
    do
    {
      u16 v0_val;
      rect[0] = v1_val + var_s3;
      v0_val = (u16) var_s0[1];
      rect[2] = w;
      rect[3] = h;
      rect[1] = v0_val + var_s2;
      StoreImage(rect, var_s1);
      var_s0 += 2;
      var_s1 += 0x10;
      v1_val = (u16) (*var_s0);
    }
    while ((*var_s0) >= 0);
  }
  DrawSync(0);
}
extern void LoadImage(s32, s32);
void func_80041BF4(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s32 r;
  s32 g;
  s32 b;
  s32 outer;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s32 idx;
  s32 x;
  /* FAKE: opaque constant-holder for the trailing `== 1` test, mechanism:
     local-alloc.c block_alloc / find_free_reg - the bare literal is
     rematerialized by reload into $v1, while a live pseudo carrying it is
     allocated $t0 exactly as target does; ordinary-C spellings of the test
     (subtract-compare-zero, double negation, switch, named call result) do
     not.  Family: .claude/rules/named-local-fake-exception.md (owner ruling). */
  int one;
  /* FAKE: oversized locals object - rect[0..3] is the live LoadImage RECT and
     rect[4..7] is the unwritten tail, mechanism: mips.c compute_frame_size /
     get_frame_size - ALIGN8(vars) + ALIGN8(args) + gp_regs.  Frame-math proof
     from the TARGET BYTES ALONE: target frame is 88 with ten callee-saves
     ($s0-$s7,$fp,$ra = 40 bytes) and a 24-byte outgoing-args area (the 6-arg
     func_80048A7C call), so the locals region is 88-40-24 = 24 bytes while the
     only stores into it are the 8 bytes of the RECT at sp+0x18.  The
     fully-written form (rect[4]) gives frame 80, so no fully-written locals
     set can produce target's 88.  ALIGN8 plus this frame's fixed 8-byte
     phantom slot make the declared size recoverable only as a RANGE: rect[5]
     through rect[8] are all byte-identical here - [8] is chosen.  Family:
     .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS carve-out (owner
     ruling); prong 2 is satisfied by extending the LIVE object - rect's
     address is passed to LoadImage - rather than adding a dead pad. */
  s16 rect[8];
  fp_ptr = func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  r = (a0 << 12) / 255;
  one = 1;
  g = (a1 << 12) / 255;
  b = (a2 << 12) / 255;
  if (func_800486FC()) {
    b = math_Grayscale3(r, g, b);
    g = b;
    r = b;
  }
  outer = 0;
  oloop:
  {
  if (outer == 0) {
    xoff = -0x140;
    yoff = 0xF0;
  } else {
    /* FAKE: single-level do-while(0) wrap on the else-arm offset defs --
       it lifts xoff's and yoff's weighted reference counts (flow.c weights
       REG_N_REFS by loop_depth) so global.c's allocno order matches target's
       and the two offsets land in $s5/$s4.  Arm swap, hoisted defs, a
       ternary, narrower declaration scopes and plain assignment do not.
       Family: .claude/rules/do-while-zero-exception.md (owner ruling,
       sanctioned for any codegen effect incl. register allocation). */
    do { xoff = 0x80; yoff = 0; } while (0);
  }
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  goto test;
  again:
  {
    s32 off = idx << 5;
    idx++;
    rect[0] = x + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&g_gpu_store_buf + off));
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  test:
  x = *(u16 *) tbl;
  if ((s16) x >= 0) goto again;
  outer++;
  }
  if (outer < 2) goto oloop;
  if (func_8003E2A0() == one) { func_8003E120(); }
}
extern s16 g_anim_select[3];
extern Vec4i32 D_800A9B28;
void func_80041E10(Vec4i32 *a0, s32 a1) {
    g_anim_select[0] = (s16)((((a1 >> 16) & 0xFF) << 12) / 255);
    g_anim_select[1] = (s16)((((a1 >> 8) & 0xFF) << 12) / 255);
    g_anim_select[2] = (s16)(((a1 & 0xFF) << 12) / 255);
    D_800A9B28 = *a0;
}
extern s32 rcos(s32);
extern s32 rsin(s32);
void func_80041EB0(s32 a0, s32 a1)
{
    Unk800F62E0Rec *fp_ptr;
    s32 outer;
    Vec4i32 *cam;
    Unk800F62E0Rec *tbl;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 angle;
    s32 cos_val;
    s32 *ptr;

    fp_ptr = D_800F62E0;
    outer = 0;
    cam = &D_800A9B28;

    do {
        tbl = fp_ptr;
        if (outer == 0) {
            ptr = (s32 *)a0;
        } else {
            ptr = (s32 *)a1;
            tbl = &D_800F62E0[1];
        }

        if (ptr == 0) { goto skip; }
        if (g_anim_select[0] < 0) { goto skip; }

        dx = ptr[0] - cam->vx;
        dy = ptr[1] - cam->vy;
        dz = ptr[2] - cam->vz;

        if (dx < 0) goto neg_dx;
        if (dx < 0x7000) goto check_dy;
        goto calc;
    neg_dx:
        if (-dx >= 0x7000) goto calc;
    check_dy:
        if (dy < 0) goto neg_dy;
        if (dy < 0x7000) goto check_dz;
        goto calc;
    neg_dy:
        if (-dy >= 0x7000) goto calc;
    check_dz:
        if (dz < 0) goto neg_dz;
        if (dz < 0x7000) goto dist_check;
        goto calc;
    neg_dz:
        if (-dz >= 0x7000) goto calc;

    dist_check:
        if (gte_SumSquares3(dx, dy, dz) > 0x17D7840) { goto skip; }

    calc:
        angle = ratan2(dx, dz);
        cos_val = rcos(angle);
        {
            s32 sin_val = rsin(angle);
            s32 cross = (cos_val * dz + sin_val * dx) >> 12;
            tbl->light[1].pitch = (s16)-ratan2(dy, cross);
        }
        tbl->light[1].yaw = (s16)angle;
        tbl->light[1].on = 1;
        tbl->cmat.m[0][1] = g_anim_select[0];
        tbl->cmat.m[1][1] = g_anim_select[1];
        tbl->cmat.m[2][1] = g_anim_select[2];
        goto end_loop;

    skip:
        tbl->light[1].on = 0;

    end_loop:
        outer++;
    } while (outer < 2);

    func_8004A1FC(fp_ptr);
    func_8004A1FC(&D_800F62E0[1]);
}
extern s16 g_anim_select[3];
/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s16 g_anim_hit_flags[2];
static s32 g_anim_hit_data[2];

void func_800420D0(void) {
    g_anim_hit_flags[1] = 0;
    g_anim_hit_flags[0] = 0;
    g_anim_select[0] = -1;
}
void func_800420E8(s32 a0, s32 a1) {
    if (a0 < 2) {
        g_anim_hit_flags[a0] = 1;
        g_anim_hit_data[a0] = a1;
    }
}
void func_8004211C(void) {
    s32 val = g_anim_hit_flags[0] * 2 + g_anim_hit_flags[1];
    switch (val) {
    case 1:
        func_80041EB0(0, g_anim_hit_data[1]);
        break;
    case 2:
        func_80041EB0(g_anim_hit_data[0], 0);
        break;
    case 3:
        func_80041EB0(g_anim_hit_data[0], g_anim_hit_data[1]);
        break;
    }
}
extern void func_80041EB0(s32, s32);
void func_800421A4(void) {
    func_80041EB0(0, 0);
}
extern void func_800422BC(s32, s32, s32, s32);
extern void func_80042478(s32);


extern s32 StageLight[];
void func_800421C8(s32 a0) {
    s32 *p = (s32 *)((u8 *)StageLight + a0 * 24);
    s32 val;
    func_800422BC(a0, *p++, 0, 0);
    func_800422BC(a0, *p++, 0, 1);
    func_800422BC(a0, *p++, 1, 0);
    func_800422BC(a0, *p++, 1, 1);
    val = *p;
    D_800F62E0[4].light[0].yaw = val & 0xFFF;
    D_800F62E0[1].light[0].yaw = val & 0xFFF;
    D_800F62E0[0].light[0].yaw = val & 0xFFF;
    val = *(s16 *)((u8 *)p + 2);
    D_800F62E0[4].light[0].pitch = val & 0xFFF;
    D_800F62E0[1].light[0].pitch = val & 0xFFF;
    D_800F62E0[0].light[0].pitch = val & 0xFFF;
    func_80042478(*(s32 *)((u8 *)p + 4));
}


void func_800422BC(s32 a0, s32 packed, s32 a2, s32 a3) {
    s32 r = (packed >> 16) & 0xFF;
    s32 g = (packed >> 8) & 0xFF;
    s32 b = packed & 0xFF;
    s16 r2;
    s16 g2;
    s16 b2;
    if (func_800486FC()) {
        b = math_Grayscale3(r, g, b);
        g = b;
        r = b;
    }
    if (a3 != 0) {
        goto raw;
    }
    r2 = (r << 12) / 255;
    g2 = (g << 12) / 255;
    b2 = (b << 12) / 255;
    if (a2 != 0) {
        goto alt_scale;
    }
    D_800F62E0[0].cmat.m[0][0] = r2;
    D_800F62E0[0].cmat.m[1][0] = g2;
    D_800F62E0[0].cmat.m[2][0] = b2;
    func_8004A1FC(&D_800F62E0[0]);
    D_800F62E0[1].cmat.m[0][0] = r2;
    D_800F62E0[1].cmat.m[1][0] = g2;
    D_800F62E0[1].cmat.m[2][0] = b2;
    func_8004A1FC(&D_800F62E0[1]);
    goto out;
alt_scale:
    D_800F62E0[4].cmat.m[0][0] = r2;
    D_800F62E0[4].cmat.m[1][0] = g2;
    D_800F62E0[4].cmat.m[2][0] = b2;
    func_8004A1FC(&D_800F62E0[4]);
    goto out;
raw:
    if (a2 != 0) {
        goto alt_raw;
    }
    D_800F62E0[0].back[0] = r;
    D_800F62E0[0].back[1] = g;
    D_800F62E0[0].back[2] = b;
    D_800F62E0[1].back[0] = r;
    D_800F62E0[1].back[1] = g;
    D_800F62E0[1].back[2] = b;
    goto out;
alt_raw:
    D_800F62E0[4].back[0] = r;
    D_800F62E0[4].back[1] = g;
    D_800F62E0[4].back[2] = b;
out:;
}
void func_80042478(s32 a0) {
    s32 r = (a0 >> 16) & 0xFF;
    s32 g = (a0 >> 8) & 0xFF;
    s32 b = a0 & 0xFF;
    if (func_800486FC()) {
        b = math_Grayscale3(r, g, b);
        g = b;
        r = b;
    }
    gpu_SetDrawEnvBg(1, r, g, b);
    SetFarColor(r, g, b);
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s16 g_anim_select[3] = { 0, 0, 0 };
