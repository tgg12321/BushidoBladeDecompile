/* s5 (forensics, 2026-08-07): floor still 1; body unchanged. Fresh-counter
 * mechanism named: find_reg pass-0 regs_someone_prefers repulsion -- delta's
 * full-pref {$a0} (set_preference on the one-insn addiu arg copy) excludes
 * $a0 from the counter's pass-0 search; this body's reuse counter OWNS the
 * {$a0} copy-pref instead, which is why it allocates like target. MEASURED:
 * with ONLY the snd_LoadBgm extern corrected to the definition signature
 * (src/sound.c:133 `s32 *snd_LoadBgm(s32)`), THIS body scores sandbox 0
 * (47/47). That extern edit is the twice-refused (A) -- NOT applied here;
 * ruling-request filed s5. See evidence.md s5 for the full theorem. */
/* s4 (forensics, 2026-08-07): floor still 1; body unchanged. The s3 "cse1
 * path" law is RE-ATTRIBUTED: cse1's path DOES reach the truncation under
 * target layout (P8 .cse: "Processing block from 2 to 94") but cse never folds
 * it in any layout — the fold is COMBINE's, via nonzero_bits(sound)=0xFF, and
 * combine.c:6887's fast path is label_tick-scoped: with `sound` multi-set (the
 * counter reuse below), the fold needs load and truncation in one label region,
 * which target layout forbids (call block at a branch target). The
 * reg_nonzero_bits fallback is poisoned to full mask by the counter's own sets
 * (combine.c:717-790). Single-set carriers (`bgm = sound;` as arg) are
 * canon-reverted by cse1 before combine (rejected/carrier-copy-cse-canon-
 * reverted.c). Full route partition (all dead) in evidence.md s4. */
/* s3 (permuter, 2026-08-07) addendum: floor still 1. New structural law measured:
 * the andi folds IFF the call block is fall-through-reachable on cse1 path;
 * target CFG puts the call at a branch target (bne 0x44). jump1 canonicalizes
 * every diamond spelling before cse1 into one of two stable forms — target
 * layout WITH andi, or fall-through layout WITHOUT andi but a 2-insn CFG miss
 * (rejected/andifree-fallthrough-wrong-cfg.c). Two fresh-seed campaigns dry:
 * P1 andi-free chassis 64k iters (one cheat-flavored -30 find), m1 idx-reuse
 * chassis 68k iters ZERO finds. Open crack for forensics: cse.c
 * cse_end_of_basic_block follow_jumps/skip_blocks — can a cse path ever cross
 * the 2-insn return-0 block into the branch target? See evidence.md s3. */
/* func_80048AD0 — HONEST FLOOR: sandbox distance 1 (47/47 insns), re-confirmed
 * s2-permuter 2026-08-07. Sole residual: `andi $a0,$a0,0xff` in the
 * snd_LoadBgm jal delay slot where target has `nop` (caller-side truncation
 * of the s32 `sound` local against the u8 prototype in text1b.c).
 *
 * THIS is the best NON-BANNED form. The previous candidate.c (sandbox 0 via
 * `snd_LoadBgm((&D_80099BCC)[idx])` — duplicate-read call argument) was
 * layer-1 FAILED as a respelling of the twice-refused declaration edit (A)
 * and is BANNED for this function; it is preserved at
 * rejected/layer1-fail-0807-1829.c. Do not re-propose it.
 *
 * s2-permuter closure of the andi-removal space (see evidence.md s2-permuter):
 *   - decl mutations (u8 -> int / volatile int): banned (A)-respelling family.
 *   - compare-site `(u8)sound == 0xFF` cast: folds the andi (CSE unifies the
 *     call-site truncation with the compare temp; nonzero_bits folds it into
 *     the lbu) BUT the arg copy then sources the QImode temp, not `sound`, so
 *     the {$a0} copy-preference never reaches the reused counter: counter/delta
 *     allocate $a2/$a0 (7 reg diffs, permuter 35). The fold and the pref-loss
 *     are COUPLED through the same CSE temp. Also semantically redundant
 *     (sound is lbu-loaded, always < 0x100) = F2 forbidden family anyway.
 *   - call-site `(u8)sound` cast alone: andi REMAINS (no CSE partner) — and
 *     is F2-redundant too.
 *   - 46k random permuter iterations from this chassis surfaced NO other
 *     andi-free spelling.
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
    base = (u8 *)snd_LoadBgm(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    /* FAKE: the record counter reuses `sound` rather than a fresh local.
       snd_LoadBgm's argument copy gives `sound` a hard-reg $a0 preference;
       global.c expand_preferences propagates it to the counter, which stops
       prune_preferences making the counter yield $a0 to `delta`. With a
       separate counter the pair allocates $a2/$a0 instead of target's
       $a0/$a2. Measured exhaustion: ~60 variants over 8 sweeps + kills in
       grind s1/s2 — see memory/grind/func_80048AD0/evidence.md. */
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
