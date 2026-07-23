/* func_8001F938 +0x270 block — REJECTED (s8 rederive).
 * KILLED: score 9, build_insns 104 (WORSE than clean floor 8).
 *
 * Hypothesis (from decomp.me census): the clean, community-MATCHED (score 0)
 * gcc2.7.2 scratch func_8009AA68 (decomp.me CW0dj) emits adjacent
 * `lh $v0,0x6($s2); lhu $v1,0x6($s2)` of a SINGLE `s16 angle` field from
 * ORDINARY pure C — no dual-typed read, no union, no cast:
 *     if (entity->angle < 0x1000) {              // signed compare -> lh + slti
 *         s16 newAngle = entity->angle + 0x800;  // narrowing arith -> lhu
 *         entity->angle = newAngle; ...
 *     }
 * So a single-view field CAN produce two differently-signed loads. This block
 * mirrors that: re-read *(s16*)(+0x270) separately in the compare and the
 * else-arm (no reused `probe` local), hoping GCC reloads with lhu for the index.
 */
{
    s32 raw_or_3;
    if (*((s16 *)(arg0 + 0x270)) >= 4) {
        raw_or_3 = 3;
    } else {
        raw_or_3 = *((s16 *)(arg0 + 0x270));
    }
    idx = ((raw_or_3 << 16) >> 15);
}
/* MEASURED: sandbox --disable all = score 9, build_insns 104. Disasm:
 *     lh v1,624(a0); slti v0,v1,4; bnez; move v0,v1(delay); li v0,3;
 *     sll v0,v0,0x1; addu; lh v0,630(v0)
 * ONE signed load; the two re-reads CSE-merged; combine FOLDED (raw<<16)>>15
 * -> raw*2 (sll,1). Worse than clean floor 8 (dropped an insn to 104).
 *
 * WHY func_8009AA68 does NOT transfer (the decisive distinction):
 *   func_8009AA68's second use (`angle + 0x800` stored to s16) is
 *   sign-INSENSITIVE — only the low 16 bits survive the narrowing store, so
 *   GCC is FREE to load with lhu (zero-extend) for that use while the compare
 *   uses lh. func_8001F938's index `(field<<16)>>15` is sign-SENSITIVE: bit 15
 *   of the field becomes the RESULT's sign (s3: signed read folds/floor 8;
 *   unsigned read gives sltiu/floor 6). A signed-compare view therefore cannot
 *   also serve the index with lhu — the two views are NOT interchangeable here,
 *   they carry different values out of range. Hence the two loads are only
 *   reachable via a programmer-authored 2nd typed memory view = the pre-banned
 *   signedness-split family (s7 combine theorem, dump-proven). Confirmed.
 */
