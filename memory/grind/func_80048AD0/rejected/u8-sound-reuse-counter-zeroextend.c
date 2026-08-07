/* REJECTED (grind s1, 2026-08-07) — sandbox distance 25 (55 insns, target 47).
 *
 * Hypothesis: keep `sound` u8 (so the snd_LoadBgm call is andi-free under the
 * u8 prototype) AND still reuse it as the record counter, accepting whatever
 * QImode arithmetic costs appear.
 *
 * KILL MECHANISM: every SImode consumer of the u8 counter (the five scaled
 * store addresses `sound * 0x68`, the stored value, the `< 0x11` compare)
 * forces a zero_extend — the loop bloats by 8 instructions (55 vs 47).
 * The reuse trick strictly requires the counter to be a full-width s32,
 * which is exactly what re-introduces the caller-side andi at the call under
 * the u8 prototype. This closes the reuse × type matrix: s32-reuse = 1,
 * u8-reuse = 25, idx-reuse (single pseudo doubling as index+counter, u8
 * sound) = 2 (re-measured this session; = prior variant m1).
 */
s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    u8 sound;
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
