/* REJECTED (grind s1, 2026-08-07) — sandbox distance 20 (48 insns, target 47).
 *
 * Hypothesis: reuse the `arg0` PARAM pseudo as the loop counter (its prologue
 * copy `(set pseudo (reg a0))` gives it an inherent {$a0} preference), keep
 * `sound` u8 so the snd_LoadBgm call needs no andi, and route stores through
 * an explicit `saved = arg0;` copy (hoped to become target's `move s0,a0`
 * delay-slot fill).
 *
 * KILL MECHANISM (read from the disassembly, tmp/grind/func_80048AD0/s1/):
 * copy-propagation replaces early uses of `saved` with the equal-valued
 * `arg0` pseudo (the `D_800A33E0 = saved;` store emits `sw s0` against
 * arg0's reg), so the arg0 pseudo stays live from the prologue THROUGH the
 * first call — reg_n_calls_crossed > 0 — and global.c bars it from
 * caller-saved $a0 entirely. It lands in $s0, `saved` needs a second
 * callee-save ($s1, +1 prologue/epilogue pair, frame grows to 32), and the
 * whole allocation shifts. Note the idx→$v0 half DID match in this variant
 * (lh v0 / addu at,at,v0): separating idx from the counter is correct; the
 * counter's pref source is the unsolved part.
 *
 * Lesson: ANY counter spelling whose pseudo is live across a call cannot
 * take $a0 — the pref route must come from a pseudo born after snd_LoadBgm.
 */
s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    u8 sound;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;
    s32 saved;

    saved = arg0;
    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    idx = *(s16 *)(temp_v0 + 8);
    D_800A33E0 = saved;
    sound = (&D_80099BCC)[idx];
    if (sound == 0xFF) return 0;
    base = (u8 *)snd_LoadBgm(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    for (arg0 = 0; arg0 < 0x11; arg0++) {
        *(s16 *)(q - 8 + arg0 * 0x68) = arg0;
        *(s16 *)(q - 6 + arg0 * 0x68) = 9;
        p[arg0 * 0x68] = 0xF;
        *(s8 *)(q - 9 + arg0 * 0x68) = 0;
        *(s16 *)(q + arg0 * 0x68) = (s16)saved;
    }
    snd_PlayBgm(delta + 0x6E8);
    return 1;
}
