/* func_8001979C — BEST FORM as of session 1 (recon). Floor: sandbox
 * --disable all = 24 (target_insns 77, build_insns 75).
 *
 * This is byte-identical to what is already in src/code6cac.c at HEAD
 * (session 1 measured probes on top of it and reverted; none beat 24).
 * It is recorded here so later sessions have an explicit baseline to
 * diff against rather than re-reading git.
 *
 * Residual diff families at this floor (see evidence.md for the full
 * instruction-level table):
 *   D1  2 insns MISSING: target has `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero`
 *       (one per loop); we emit a single `subu $a3,$t2,$a0`.
 *   D2  $v0/$v1 role swap through the whole shift/or chain in both loops,
 *       and again on the final -2 fill loop ($v0/$v1 swapped there too).
 *   D3  walking pointer: target inits `addu $t1,$t3,$zero` and stores at
 *       `0xA($t1)` / `0x8E($t1)`; we init `addiu $t1,$t3,10` / `,142` and
 *       store at `0($t1)`. GCC eliminated the biv and folded the constant
 *       displacement into the induction variable's initial value.
 *   D4  preheader constant order: target emits `li $t4,0xC` then
 *       `li $t2,0x20`; we emit `li $t2,32` then `li $t4,12`.
 * Register ASSIGNMENT at this floor already matches target exactly
 * ($t3 = base, $t2 = 0x20, $t4 = nbits, $t1 = walker, $t0 = counter,
 * $a3 = bits_left, $a2 = cur, $a1 = arg1, $a0 = needed) — do not disturb
 * it casually; the session-1 probes showed it is one allocno away from
 * cascading.
 */
void func_8001979C(s32 arg0, u32 *arg1) {
    s32 bits_left;
    u32 base;
    s32 i;
    u32 cur;
    u32 hi;
    s32 needed;
    u32 dst;
    s32 val;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    dst = base;
    do {
        if (bits_left < 0xC) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0xA) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
        dst += 2;
    } while (i < 0x3F);

    i = 0;
    dst = base;
    do {
        if (bits_left < 2) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0x8E) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
        dst += 2;
    } while (i < 0x3F);

    val = -2;
    i = 3;
    dst = base + 0x348;
    do {
        *(s32 *)(dst + 0x110) = val;
        i--;
        dst -= 0x118;
    } while (i >= 0);
    *(s32 *)(base + 0x10C) = 0;
}
