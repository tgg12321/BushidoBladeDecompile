/* REJECTED s4: single-set SImode carrier copy (`bgm = sound;` before the gate,
 * passed as the snd_LoadBgm argument). Theory: bgm records
 * reg_last_set_nonzero_bits=0xFF in the load's label region and combine's
 * reg_n_sets==1 fast path then folds the truncation label-independently under
 * target layout. Measured: byte-identical to base (andi present, copy gone) —
 * cse1 puts bgm and sound in one quantity class and canon_reg rewrites the
 * truncation operand back to sound's pseudo, deleting the dead copy before
 * combine runs (k1 .cse dump in tmp/grind/func_80048AD0/s4/). Structurally
 * unavoidable: the 0xFF record requires same-cse-region placement, and
 * same-region copies are always canonicalized away. Post-gate placement (k2)
 * fails identically (also canon-reverted; across the label anyway). */
typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned short u16;
typedef signed char s8;
typedef unsigned char u8;

extern s32 snd_LoadBgm(u8);
extern s32 snd_PlayBgm(s32);
extern u8 D_80099BCC;
extern s32 D_800A33E0;
extern s32 D_800A33E4;
extern s32 func_8004153C(s32);

s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    s32 sound;
    s32 bgm;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    idx = *(s16 *)(temp_v0 + 8);
    D_800A33E0 = arg0;
    sound = (&D_80099BCC)[idx];
    bgm = sound;
    if (sound != 0xFF) goto body;
    return 0;
body:
    base = (u8 *)snd_LoadBgm(bgm);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    for (sound = 0; sound < 0x11; sound++) {
        *(s16 *)(q - 8 + sound * 0x68) = sound;
        *(s16 *)(q - 6 + sound * 0x68) = 9;
        p[sound * 0x68] = 0xF;
        *(s8 *)(q - 9 + sound * 0x68) = 0;
        *(s16 *)(q + sound * 0x68) = (s16)arg0;
    }
    snd_PlayBgm(delta + 0x6E8);
    return 1;
}
