#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern s32 func_80046798(void);
extern void func_80017F98(s32, s32, s32);
extern void *func_8004153C(void);
extern void func_80017714(void);
extern void func_80017F90(void);
extern void func_80017E8C(s32);

/* Externs for globals */
extern s32 D_800A336C;
extern s32 D_800A322C;
extern s32 D_800A3374;
extern s32 D_800A3370;
extern s32 D_800948BC;
extern s16 D_800F665C;
extern void func_8001924C(s32 *, s32);
extern void func_80045A28(s32, s32);
extern void func_80052A20(s32 *, s32 *, s16 *);
extern void func_80052C10(void *);

/* Externs for globals */
extern u8 D_800A6690;
extern s16 D_800F6656;
extern s16 D_800F6658;
extern u8 D_800A8FB0[];
extern s32 D_800A93B0;
extern s32 D_800A93B4;
extern s32 D_800A93B8;
extern s32 D_800A93BC;
extern s32 D_800A93C0;
extern s32 D_800A93C4;

/* --- Functions 0x8003F168 - 0x8004019C --- */
void func_8003F168(void) {
    s32 v0 = *(s32 *)((u32)&D_800948BC + (func_80046798() << 3));
    if (v0) {
        (*(void (**)(void))((u32)&D_800948BC + (func_80046798() << 3)))();
    }
}
s32 func_8003F1C8(void) {
    return D_800A336C;
}

void *func_8003F1D4(void) {
    return &D_800A6690;
}

void func_8003F1E4(s32 a0) {
    if (a0) {
        D_800F6656 = 3;
        D_800F6658 = 2;
    } else {
        D_800F6656 = 0;
        D_800F6658 = 1;
    }
}

void func_8003F218(s32 a0) {
    if ((u32)a0 >= 2) {
        return;
    }
    if (a0 == D_800A322C) {
        return;
    }
    D_800A322C = a0;
    if (!a0) {
        func_8003F1E4(0);
    }
    D_800F665C = (s16)D_800A322C;
}
s32 func_8003F268(void) {
    return D_800A322C;
}
INCLUDE_ASM("asm/funcs", func_8003F274);
void func_8003F388(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        D_800A8FB0[y * 32 + x] |= 0x4;
    }
}
void func_8003F3D4(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        D_800A8FB0[y * 32 + x] |= 0x8;
    }
}
void func_8003F420(s32 a0, s32 a1) {
    s32 s3, s2, s1;
    int new_var;
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
        s0 = (new_var = -1);
    } else {
        s0 = 1;
    }
    func_8003F52C(s3, s2, 2);
    func_8003F52C(s3 + s1, s2, 2);
    func_8003F52C(s3, s2 + s0, 2);
    func_8003F52C(s3 + s1, s2 + s0, 2);
}

void func_8003F52C(s32 a0, s32 a1, s32 a2) {
    D_800A8FB0[a1 * 32 + a0] = a2 & 3;
}

u32 func_8003F54C(s32 a0, s32 a1) {
    return D_800A8FB0[a1 * 32 + a0];
}

void func_8003F568(void) {
    D_800A3374 = 0;
    D_800A3370 = 0;
    D_800A93B8 = 0;
    D_800A93B4 = 0;
    D_800A93B0 = 0;
    D_800A93C4 = 0;
    D_800A93C0 = 0;
    D_800A93BC = 0;
}

void func_8003F5A8(s32 a0, s32 a1, s32 a2) {
    (&D_800A93B0)[a2] = a0;
    (&D_800A93BC)[a2] = a1;
}

void func_8003F5CC(void) {
    func_80017F98(D_800A93B0, D_800A93BC, 0);
    func_80017F98(D_800A93B4, D_800A93C0, 1);
    func_80017F98(D_800A93B8, D_800A93C4, 2);
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
INCLUDE_ASM("asm/funcs", func_8003F6D8);
void func_8003F7F4(void) {
    func_80017714();
    func_80017F90();
    D_800A3370 = 0;
    D_800A3374 = 0;
}
INCLUDE_ASM("asm/funcs", func_8003F824);
INCLUDE_ASM("asm/funcs", func_8003FA24);
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2) {
    register s32 i asm("a3");
    i = 0;
    if (a1 > i) {
        register s16 t0 asm("t0") = -256;
        register s16 *v1 asm("v1") = a0;
        for (i = 0; i < a1; i++) {
            *(s16 *)((u8 *)v1 + 6) = t0;
            v1 = (s16 *)((u8 *)v1 + 8);
        }
    }

    {
        register s32 count asm("v1");
        count = a2[0];
        a2++;
        if (count >= 0) {
            register s32 neg1 asm("t1") = -1;
            do {
                register int val asm("a1");
                val = a2[0];
                do { count--; } while(0);
                a2++;
                if (count != neg1) {
                    register s32 neg1b asm("t0") = -1;
                    do {
                        register s32 addr asm("v0");
                        i = a2[0];
                        a2++;
                        count--;
                        addr = (i << 3) + (s32)a0;
                        *(s16 *)(addr + 6) = val;
                    } while (count != neg1b);
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
  s16 *new_var;
  s32 *t1;
  u16 t0;
  t2 = *((s32 *) (((u8 *) a1) + 0x1C));
  ;
  t0 = (u16) a2[0];
  a1 = (s32 *) (((u8 *) a1) + 0x1C);
  new_var = &a2[0];
  if (a2[(*new_var) * 0] == (-2))
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
      a3++;
      a3--;
      t1++;
      t1--;
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
      {
        s32 *p_next = (s32 *) (((u8 *) a3) + 0x10);
        a3 = p_next;
      }
      *((s32 *) (((u8 *) t1) + 0x78)) = val78;
    }
    t0 = (u16) (*a2);
    v1 = (s16) t0;
    t1 = (s32 *) (((u8 *) t1) + 4);
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
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            s0 = 0;
            do {
                func_80017E8C(s1[4]);
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
