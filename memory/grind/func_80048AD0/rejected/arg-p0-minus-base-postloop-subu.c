/* REJECTED s5 (forensics, 2026-08-07): snd_PlayBgm(p0 - base + 0x6E8) arg
 * respelling. MECHANISM WIN, BYTES LOSS: because the pref-carrier (the minus
 * temp) is born POST-loop, it does not conflict with the counter, so the
 * counter escapes the regs_someone_prefers repulsion and correctly lands
 * $a0 (ALLOCDBG: pseudo 76 -> hardreg 4), with the subu temp also on $4
 * after the counter dies. But the subu is emitted POST-loop (cse1/LICM never
 * hoist an outside-loop invariant), while target has subu $a2 PRE-loop, and
 * p0+base must both stay live across the loop (two regs vs target one),
 * shifting the 0xF/9 constant regs to $8/$9. Wrong bytes; honest form;
 * measured dead. Proves the s5 repulsion theorem causally: the ONLY honest
 * escape shape emits the wrong instruction placement. */
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
    u8 sound;
    u8 *p0;
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
    if (sound == 0xFF) return 0;
    base = (u8 *)snd_LoadBgm(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    p0 = p;
    for (idx = 0; idx < 0x11; idx++) {
        *(s16 *)(q - 8 + idx * 0x68) = idx;
        *(s16 *)(q - 6 + idx * 0x68) = 9;
        p[idx * 0x68] = 0xF;
        *(s8 *)(q - 9 + idx * 0x68) = 0;
        *(s16 *)(q + idx * 0x68) = (s16)arg0;
    }
    snd_PlayBgm(p0 - base + 0x6E8);
    return 1;
}
