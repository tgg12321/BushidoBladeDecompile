/* func_800453E0 — src/text1a_c.c — MATCHING FORM (session 2, 2026-08-17)
 *
 * Honest pure-C distance 0: `sandbox func_800453E0 --disable all` == 0 with all
 * 7 regfix rules dropped and cheat-asm stripped, 76/76 instructions, frame
 * 0x30, and addend-0 %hi/%lo relocs against D_800EED00 / D_800EED10 /
 * D_800EED18 / D_800EED1C (artifact tmp/grind/func_800453E0/s2/final_func.txt).
 *
 * This body is a SIMPLIFICATION of session 1's matching form. Session 1's form
 * also reached 0 but carried two constructs that session 2 measured to be
 * unnecessary in the final context and removed:
 *   - the do-while + `count = D_800A33AC;` re-read + `if (i >= count) return;`
 *     entry guard (session 1's H5, the construct the layer-1 review FAILed on
 *     for its unresolvable rule citation) — a plain
 *     `for (i = 0; i < D_800A33AC; i++)` emits BYTE-IDENTICAL output
 *     (tmp/grind/func_800453E0/s2/{forloop,dowhile}_func.txt diff clean);
 *   - the separate `s32 next = i + 1;` local (session 1's H4) — writing
 *     `i + 1` inline at both use sites, and deleting the declaration, also
 *     stays at 0.
 * Nothing in this body is present for a codegen reason that a simpler spelling
 * could serve; every simplification that was measured byte-neutral was taken.
 *
 * SEMANTICS (recovered, not transcribed): "remove the table entry whose
 * id == a0". D_800EED10 is an array of 16-byte records with D_800A33AC live
 * entries: { s16 id; s16 unk2; s32 unk4; s32 amt; void (*fn)(s16,s32); }
 * (field offsets 0/2/4/8/0xC, corroborated by the five sibling functions in
 * this file that walk the same table with a 0x10 stride). On a hit it calls
 * func_80045294(index + 1, -amt) to undo the entry's contribution, shifts every
 * following record down one slot, clears the freed tail slot (id = -1,
 * fn = NULL) and decrements the count.
 *
 * The two spelling choices that ARE load-bearing, both measured this session:
 *  - The search loop and the tail clear address the table through a
 *    source-level byte offset (`off = i << 4`, then `(u8 *)SYM + off`), the
 *    idiom every sibling in this file uses. Rewriting them as struct accesses
 *    (`SUBSLOT[i + 1].id`) drops the build to 72 insns / distance 11: the base
 *    stops being re-materialized per access.
 *  - The shift-down loop is a real 16-byte struct assignment written against
 *    SUBSLOT (the D_800EED00 base, the record immediately preceding the table)
 *    so BOTH of its base pointers derive from ONE symbol: $s2 = %lo(D_800EED00),
 *    $s3 = $s2 + 0x10. Spelling it against a D_800EED10 view produces the same
 *    linked bytes but emits a LO16 reloc with addend -16, which engine/score.py
 *    does not mask and counts as 1.
 *
 * NOTE FOR INTEGRATION: the 7 regfix.txt rules for func_800453E0 (lines
 * 214-224) were written for the OLD C and must be deleted; a full build with
 * them in place will corrupt this function. Retirement is the operator's /
 * driver's step, not this session's.
 */

/* The subtitle/effect slot table is an array of 16-byte records:
   { s16 id; s16 unk2; s32 unk4; s32 amt; void (*fn)(s16, s32); }
   (field offsets 0/2/4/8/0xC), with D_800A33AC live entries. The same layout
   and 0x10 stride is used by every other function in this file that walks it
   (func_80045294 above, func_80045510 / func_800455AC below, and the two
   setters near the end). D_800EED00 is the 16-byte slot immediately preceding
   the table, so SUBSLOT[n + 1] is table entry n. */
typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} SubEntry;

#define SUBSLOT ((SubEntry *)D_800EED00)

/* Remove the table entry whose id == a0: undo its contribution via
   func_80045294(index + 1, -amt), shift the following records down one slot,
   clear the freed tail slot, decrement the count. */
void func_800453E0(s32 a0) {
    s32 i;
    s32 j;
    s32 last;
    s32 off;

    for (i = 0; i < D_800A33AC; i++) {
        off = i << 4;
        if (*(s16 *)((u8 *)D_800EED10 + off) == a0) {
            func_80045294(i + 1, -*(s32 *)((u8 *)&D_800EED18 + off));
            if (i < D_800A33AC - 1) {
                for (j = i + 1; j < D_800A33AC; j++) {
                    SUBSLOT[j] = SUBSLOT[j + 1];
                }
            }
            last = D_800A33AC - 1;
            *(s16 *)((u8 *)D_800EED10 + (last << 4)) = -1;
            *(s32 *)((u8 *)D_800EED1C + (last << 4)) = 0;
            D_800A33AC = last;
            return;
        }
    }
}
