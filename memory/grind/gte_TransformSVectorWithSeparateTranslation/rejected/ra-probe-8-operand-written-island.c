/* SCRATCH RA-reachability probe only -- never a candidate (written asm = banned island).
 * Question: if all 8 values were C-level pseudos fed to one asm via "r", which hard
 * regs does GCC 2.7.2 assign? Target needs t0..t7 ($8-$15). */
typedef short s16; typedef int s32;
typedef struct { s32 m[5]; } M5;
typedef struct { s16 vx, vy, vz, pad; } SV;
void probe(M5 *rot, SV *tr, SV *v0, s32 *out) {
    s32 a = rot->m[0], b = rot->m[1], c = rot->m[2], d = rot->m[3], e = rot->m[4];
    s32 x = tr->vx, y = tr->vy, z = tr->vz;
    __asm__ volatile("ctc2 %0,$0;ctc2 %1,$1;ctc2 %2,$2;ctc2 %3,$3;ctc2 %4,$4;ctc2 %5,$5;ctc2 %6,$6;ctc2 %7,$7"
        : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(x), "r"(y), "r"(z));
    __asm__ volatile("lwc2 $0,0(%0);lwc2 $1,4(%0)" : : "r"(v0));
    __asm__ volatile("swc2 $9,0(%0);swc2 $10,4(%0);swc2 $11,8(%0)" : : "r"(out) : "memory");
}
