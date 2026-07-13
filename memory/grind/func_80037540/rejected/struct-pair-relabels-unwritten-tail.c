/* REJECTED (s2) — "CamEntry sp[2]" struct-pair reshape.
 *
 * sandbox --disable all == 0 (byte-identical to the flagged s32 sp[8]).
 * KILLED by cheat-reviewer (layer-2, default-FAIL), 2026-07-13.
 *
 * WHY IT IS DEAD — do not re-propose:
 *  1. It emits BYTE-IDENTICAL output to s32 sp[8]. A reshape that changes zero
 *     bytes has changed zero semantics — it only relabels the SAME 8 unwritten
 *     bytes as two struct fields. Pure cheat-by-spelling.
 *  2. The data model is incoherent. The six payload words are
 *     {&SpecialCam+i*8, a3, a0, a1, &SpecialCam+j*8, a4} — entry[0] fills all
 *     four fields, entry[1] fills only two. Genuinely parallel records would
 *     fill the same fields. These six words are the FLAT argv payload for
 *     bios_Exec(hdr, argc=6, argv) — a parameter list, not an array of records.
 *  3. Field names w0/w1/w2 are invented placeholders; dead-vars-local-array
 *     lists 'w' among the names that announce coercion intent.
 *  4. STRICTLY WORSE than the flat array: it launders the frame pad behind a
 *     fabricated type AND injects a false data model permanently into the tree.
 *
 * This was s1's top frontier lead. It is now closed.
 */
typedef struct {
    s32 cam;
    s32 w0;
    s32 w1;
    s32 w2;
} CamEntry;

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    CamEntry sp[2];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0].cam = (s32)&SpecialCam + v0 * 8;
    sp[0].w0 = a3;
    sp[0].w1 = a0;
    sp[0].w2 = a1;
    v0 = func_80036EA8(6, 2);
    sp[1].cam = (s32)&SpecialCam + v0 * 8;
    sp[1].w0 = a4;
    /* sp[1].w1, sp[1].w2 NEVER written — the same dead 8 bytes, relabelled. */
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, (s32 *)sp, v0 + 0x7FC);
}
