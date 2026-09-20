/* REJECTED (session 6): this is the s2-s5 body that scored 11 for three
 * sessions. It is SEMANTICALLY WRONG: when i == 0 and sel is one of the five
 * explicit jump-table cases {0,3,12,13,14}, the target overwrites s.p0 with
 * the default formula `s0 + sel*12` (asm .L8006EE98's
 * `sll v0,s1,16; beqz v0,.L8006EF14`), but this body leaves the case's own
 * s.p0 in place. It is also 2 instructions longer than target (211 vs 209).
 * Superseded by the session-6 chassis, which is semantically correct, exactly
 * 209 instructions, and scores 2. Do not restore this form.
 */
extern s16 D_800A3554;
extern s32 D_800A35B0;
extern u8 D_8009BC7C[];
extern u8 D_800A3560[];
extern u8 D_8009BC40[];
extern s16 D_800A3588[];
extern s16 D_800A358C[];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern s32 D_800A3568;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern void *D_800A35C4;
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect_8006ECF4;
extern Rect_8006ECF4 D_800A32F4;

void func_8006ECF4(s32 arg0) {
    S46C s;
    s32 v0;
    s32 s3;
    s32 s0;
    s32 sel;
    s32 a2;
    s16 i;
    Rect_8006ECF4 rectbuf;

    s.one14 = 0x14;
    s.c20 = 0x200;
    s.zero18 = 0;
    s.zero1C = 0;
    s.c24 = 0x100;

    v0 = *(s32 *)arg0;
    s3 = *(s32 *)(v0 + 0x54);
    s0 = s3 + 0xC;

    for (i = 0; i < D_800A3554 + 1 + D_800A35B0; i++) {
        s32 b = D_800A3588[i];
        s32 c = D_800A358C[i];
        s32 b2 = b * 2;
        sel = D_8009BC40[b2 + c * 12];
        if (D_8009BC7C[sel] & 1) {
            s.zero10 = 0;
            if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
                s.byte28 = 1;
            } else {
                s.byte28 = 0;
            }
            *((u8 *)&s + 0x29) = 0x94;
            *((u8 *)&s + 0x2A) = 0x80;
            *((u8 *)&s + 0x2B) = 0x6E;
        } else {
            s.zero10 = 1;
            s.byte28 = 1;
            *((u8 *)&s + 0x2B) = 0;
            *((u8 *)&s + 0x2A) = 0;
            *((u8 *)&s + 0x29) = 0;
        }

        if (sel < 15) {
            switch (sel) {
            case 12:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x84);
                s.p0 = (void *)(s0 + 0x108);
                break;
            case 13:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x88);
                s.p0 = (void *)(s0 + 0x114);
                break;
            case 14:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x8C);
                s.p0 = (void *)(s0 + 0x120);
                break;
            case 0:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x90);
                s.p0 = (void *)(s0 + 0x12C);
                break;
            case 3:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x94);
                s.p0 = (void *)(s0 + 0x138);
                break;
            default:
                s.p0 = (void *)(s0 + sel * 12);
                goto skip_load;
            }
            if (i != 0) {
                if (D_800A32E8 != sel || D_800A32E9 != D_800A3554) {
                    rectbuf = D_800A32F4;
                    LoadImage((s32)&rectbuf, a2);
                    DrawSync(0);
                }
            }
        skip_load:;
        } else {
            s.p0 = (void *)(s0 + sel * 12);
        }

        if (D_800A35B0 != 0) goto p1_idx;
        if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) goto p1_idx;
        if (D_800A35BC != 2) goto p1_fallback;
        if (*(s32 *)((s32)D_800A3568 + 0x14) & 0x20000) goto p1_idx;
    p1_fallback:
        s.p1 = (s32 *)*(s32 *)(s3 + 4);
        goto p1_done;
    p1_idx:
        s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
    p1_done:;

        s.ret = *(s32 *)(arg0 + 4);
        if (i != 0) {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        } else {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        }
    }
}
