/* func_8001A820 fidelity micro-tests: signed halfword read through a register that holds a
 * constant base, inside a loop that calls a function (the base lives in a callee-saved reg).
 * Run: bash memory/grind/func_8001A820/fidelity/micro.sh memory/grind/func_8001A820/fidelity/micro_all.c */
typedef struct { unsigned short a[512]; } S;
typedef struct { short vx, vy, vz, pad; } SV;
typedef struct { char pad[0x58]; SV nrm; } U;
typedef struct { char pad[0x58]; int x : 16; int y : 16; } B;
extern int g(void);
extern void h(int);
/* base = 64K-aligned constant (lui only) */
int t4(void) { S *s = (S *)0x1F800000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && (short)s->a[45] < -800) n++; } return n; }
int t6(void) { short *s = (short *)0x1F800000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && s[45] < -800) n++; } return n; }
int t8(void) { short *s = (short *)0x1F810000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && s[45] < -800) n++; } return n; }
int t9(void) { short *s = (short *)0x80100000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && s[45] < -800) n++; } return n; }
int tF(void) { U *s = (U *)0x1F800000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && s->nrm.vy < -800) n++; } return n; }
/* same, base NOT 64K-aligned (lui+ori): both compilers fold to lh */
int t5(void) { S *s = (S *)0x1F8001B0; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && (short)s->a[45] < -800) n++; } return n; }
int t7(void) { short *s = (short *)0x1F8001B0; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && s[45] < -800) n++; } return n; }
/* same, base is a parameter: both fold to lh */
int t41(S *s) { int i, n = 0; for (i = 0; i < 2; i++) { if (g() && (short)s->a[45] < -800) n++; } return n; }
/* respellings tried on our cc1 (all fold to lh; cc1psx keeps lhu+sll+sra) */
int tA(void) { S *s = (S *)0x1F800000; int i, n = 0; unsigned short ny; for (i = 0; i < 2; i++) { if (g() && (ny = s->a[45], (short)ny < -800)) n++; } return n; }
int tE(void) { S *s = (S *)0x1F800000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && (short)s->a[45] + 800 < 0) n++; } return n; }
int tG(void) { S *s = (S *)0x1F800000; int i, n = 0; short y; for (i = 0; i < 2; i++) { if (g()) { y = s->a[45]; if (y < -800) n++; } } return n; }
int tH(void) { S *s = (S *)0x1F800000; int i, n = 0, y; for (i = 0; i < 2; i++) { y = g(); if (y && (short)s->a[45] < -800) n++; } return n; }
int tB(void) { B *s = (B *)0x1F800000; int i, n = 0; for (i = 0; i < 2; i++) { if (g() && s->y < -800) n++; } return n; }
