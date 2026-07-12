/* REJECTED — the two constructs the judge flagged as codegen-coercion cheats.
   Both byte-identical to the clean replacement in candidate.c, but fail the
   human-programmer test (no semantic reason to introduce a typed intermediate
   or a signed cast that changes nothing observable). Kept only as a record of
   what NOT to reintroduce.
*/

/* construct 1 — s16 named intermediate used only to steer SImode constant
   materialization (58368 unsigned vs -7168 signed -> ori vs addiu).
   Superseded by: g_file_vram_timer's signed *(s16 *)& store view (candidate.c). */
if ((s16)g_file_vram_timer < -0x1C00) {
    { s16 tmp = -0x1C00; g_file_vram_timer = tmp; }
}

/* construct 2 — explicit (s32) cast on a 32-bit AND mask; SImode canonicalizes
   (s32)0xFFFEFFFE, 0xFFFEFFFE, and ~0x10001 to the identical const_int, so the
   cast has zero effect on the emitted `lui/ori/and`. MEASURED this session:
   swapping to `& ~0x10001` alone left sandbox distance at 0 (byte-neutral).
   Superseded by: *arg0 = *arg0 & ~0x10001; (candidate.c) */
*arg0 = *arg0 & (s32)0xFFFEFFFE;
