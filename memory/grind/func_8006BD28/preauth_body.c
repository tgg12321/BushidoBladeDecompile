/* func_8006BD28 -- pre-authorization C body, banked 2026-10-01 at de-authorization (owner ruling,
 * docs/grind/decisions.md "inline-asm audit"). Measured: sandbox func_8006BD28 --disable all
 * --candidate <this file> -> honest floor in migration_pin.json. Volatile frame pads removed;
 * any asm barrier is stripped by the sandbox. Starting context only, not a vetted candidate. */
typedef struct { s32 a; s32 b; } _Pair_BD28;
extern u8 *D_800A36E0;
extern u8 *D_800A36E4;
extern s32 saMotionSet();
extern s32 func_8007352C();
extern void initTexPage();
extern s32 D_800A374C;
#define arg2 ((s32)arg2_p)
void func_8006BD28(s32 arg0, s32 arg1, s32 *arg2_p, s32 arg3) {
    s32 i, j, n;
    s32 v;
    s32 base;
    s32 offset;
    _Pair_BD28 *p;

    offset = arg0 * 8;
    base = *(s32 *)(*(s32 *)((s32)D_800A34FC + 0x24) + 0x20);

    *(s8 *)(arg2 + 0x2B) = 0x30;
    *(s8 *)(arg2 + 0x2A) = 0x30;
    *(s8 *)(arg2 + 0x29) = 0x30;

    i = 0;
    do {
        s32 idx = i * 4 + base;
        __asm__ __volatile__("" : "=r"(idx) : "0"(idx));
        v = *(s32 *)(idx + offset);
        *(s32 *)(arg2 + 0) = v;
        if (v == -1) return;

        n = 1;
        if (arg0 == 0x12) {
            n = 3;
        }

        j = 0;
        if (n != 0) {
            do {
                if (arg0 == 0x12 && j != arg3 && j != 2) {
                    *(s8 *)(arg2 + 0x28) = 1;
                } else {
                    *(s8 *)(arg2 + 0x28) = 0;
                }
                *(s32 *)(arg2 + 0x18) = 0;
                *(s32 *)(arg2 + 0x1C) = arg1;
                *(s32 *)(arg2 + 0x14) = 8;
                *(s32 *)(arg2 + 0x10) = 0;
                *(s32 *)(arg2 + 8) = D_800A36E4;
                p = (_Pair_BD28 *)((s32)*(s32 *)(arg2 + 0) + 0xC);
                *(s32 *)(arg2 + 4) = (s32)&p[j];
                D_800A36E4 = func_8007352C(arg2);
                j++;
            } while (j < n);
        }

        initTexPage(D_800A36E0, 1, 0, saMotionSet(*(s32 *)(arg2 + 0), 0), 0);
        ot_Link(D_800A374C + 0x20, D_800A36E0);
        i++;
        D_800A36E0 += 12;
    } while (i < 2);
}
#undef arg2
