/* REJECTED s2 (2026-08-07) — sandbox 6. Frontier lever (b) probe: strip
 * delta's {$a0} preference by naming the snd_PlayBgm argument as its own
 * local (`t = delta + 0x6E8; snd_PlayBgm(t);`) with u8 sound (andi-free
 * call) and a separate fresh s32 counter.
 *
 * WHY DEAD (measured + mechanism): the disassembly shows
 * `subu a0,v1,v0` + `addiu a0,a0,1768` — t coalesced with delta ON $a0.
 * delta DIES in `(set t (plus delta 0x6E8))`; global.c expand_preferences
 * (:829-874) merges preferences BIDIRECTIONALLY between the SET allocno and
 * any REG_DEAD allocno of the same insn, so t's {$a0} copy-preference flows
 * straight back to delta. Generalization (banked as a KILL for the whole
 * lever): delta must semantically die feeding the $a0 argument chain, and
 * every insn in that chain SETs the next pseudo, so ANY pure-C respelling
 * returns the {$a0} preference to delta via the dying-merge. Lever (b) is
 * closed, not just this spelling.
 */
s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    u8 sound;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;
    s32 i;
    s32 t;

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
    for (i = 0; i < 0x11; i++) {
        *(s16 *)(q - 8 + i * 0x68) = i;
        *(s16 *)(q - 6 + i * 0x68) = 9;
        p[i * 0x68] = 0xF;
        *(s8 *)(q - 9 + i * 0x68) = 0;
        *(s16 *)(q + i * 0x68) = (s16)arg0;
    }
    t = delta + 0x6E8;
    snd_PlayBgm(t);
    return 1;
}
