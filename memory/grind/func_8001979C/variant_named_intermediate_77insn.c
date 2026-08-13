/* func_8001979C — VARIANT (session 1, probe D). NOT the floor form.
 *
 * Measured: sandbox --disable all score 33 (baseline is 24), but build_insns
 * == target_insns == 77 for the first time. Every residual diff in this form
 * is a REGISTER-NAMING diff plus the store-displacement fold; there is no
 * longer any missing/extra instruction.
 *
 * What it establishes: the two `addu $a3,$v0,$zero` copies that target has and
 * the plain baseline lacks are produced by routing the new bits_left through a
 * named intermediate (`val`) instead of assigning `bits_left = 0x20 - needed;`
 * directly. The copy lands in EXACTLY target's position (between
 * `subu $v0,$t2,$a0` and `sllv`).
 *
 * Why it scores worse: the extra allocno raises loop register pressure by one,
 * which cascades the whole allocation up a register — base $t3->$t4,
 * consts $t2/$t4 -> $t3/$t5, walker $t1->$t2, and critically `hi` is evicted
 * from $v1/$v0 into $t1 (baseline keeps `hi` sharing a caller-save temp with
 * the `0x20 - bits_left` sub-expression, exactly as target does with $v1).
 *
 * Next-session job: keep this shape, recover baseline's allocation.
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
    s16 *dp;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    dp = (s16 *)base;
    do {
        if (bits_left < 0xC) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            val = 0x20 - needed;
            bits_left = val;
            dp[5] = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            dp[5] = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
        dp++;
    } while (i < 0x3F);

    i = 0;
    dst = base;
    do {
        if (bits_left < 2) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            val = 0x20 - needed;
            bits_left = val;
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
