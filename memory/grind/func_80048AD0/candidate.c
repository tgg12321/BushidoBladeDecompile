/* func_80048AD0 — SANDBOX DISTANCE 0 (measured s2, 2026-08-07) — 47/47 insns,
 * every opcode/operand/offset verified against asm/funcs/func_80048AD0.s
 * (disassembly diff: tmp/grind/func_80048AD0/s2/sandbox0_disasm.txt).
 *
 * The s1 residual (andi $a0,$a0,0xff in the snd_LoadBgm jal delay slot, from
 * passing the s32 `sound` to the u8-prototyped snd_LoadBgm) is closed WITHOUT
 * any declaration edit — the reviewer's path (ii). The call argument is the
 * u8-typed table element read directly: `snd_LoadBgm((&D_80099BCC)[idx])`.
 * Being u8 → u8, no caller-side truncation is emitted (the lbu IS the
 * zero-extension); CSE folds this read into `sound`'s cached load pseudo, so
 * the emitted code has exactly ONE lbu and the argument copy still hands the
 * {$a0} preference to `sound` (the reused counter). Target delay slot: nop —
 * matched.
 *
 * Requires the surrounding declarations exactly as at HEAD (UNTOUCHED —
 * refused edit (A) is NOT re-filed):
 *   extern s32 snd_LoadBgm(u8);  extern s32 snd_PlayBgm(s32);
 *   extern u8 D_80099BCC;  extern s32 D_800A33E0;  extern s32 D_800A33E4;
 *   extern s32 func_8004153C(s32);
 *
 * The FAKE reuse below is the (B) construct layer-2 confirmed as genuine on
 * its merits in s1 (RTL-verified $a0-preference mechanism, ~60-variant
 * exhaustion). Self-vet: memory/grind/func_80048AD0/self_vet.md.
 */
s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    s32 sound;
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
    base = (u8 *)snd_LoadBgm((&D_80099BCC)[idx]);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    /* FAKE: the record counter reuses `sound` rather than a fresh local.
       snd_LoadBgm's argument copy gives `sound` a hard-reg $a0 preference;
       global.c expand_preferences propagates it to the counter, which stops
       prune_preferences making the counter yield $a0 to `delta` (delta gets
       its own $a0 preference from `(set (reg a0) (plus (reg delta) 0x6E8))`
       via set_preference's PLUS-first-operand rule). With a separate counter
       the pair allocates $a2/$a0 instead of target's $a0/$a2. Measured
       exhaustion: ~60 variants over 8 sweeps + 3 fresh kills in grind s1 —
       see memory/grind/func_80048AD0/evidence.md. */
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
