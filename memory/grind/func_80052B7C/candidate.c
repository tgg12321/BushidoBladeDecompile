/* func_80052B7C — DRAFT, UNMEASURED (banked by s1/recon; not applied to src/ this session).
 * See memory/grind/func_80052B7C/evidence.md and hypotheses.md H1 for the instruction-level
 * decode and the vendored-macro mapping this draft is built from.
 *
 * Signature guess from the asm (asm/funcs/func_80052B7C.s):
 *   a0 -> MATRIX*  (5 lw -> ctc2 CR0-CR4, matches gte_SetRotMatrix exactly, byte offsets
 *                   0/4/8/C/10 = MATRIX.m[3][3]+pad)
 *   a1 -> SVECTOR* (3 lh -> ctc2 CR5-CR7; NO vendored PsyQ macro loads TRX/TRY/TRZ from
 *                   sign-extended halfwords — every inline_o.h site touching CR5-7 uses lw
 *                   or register-to-register ops. This chunk is the one hand-written piece;
 *                   written here as a literal island citing the existing whole-function
 *                   canonical-asm grant, inline_asm_canonical.txt:340, until a
 *                   ruling-request (frontier item 3) says otherwise.)
 *   a2 -> VECTOR*  (2 lwc2 -> DR0/DR1, matches gte_ldv0 exactly)
 *   a3 -> VECTOR*  (3 swc2 <- DR9/DR10/DR11, matches gte_stlvl exactly; mvmva word
 *                   0x4A480012 matches gte_rt() exactly)
 *
 * Needs: #include <psxsdk/inline_o.h> added to src/main/3AB48.c (not present yet; the
 * file's `#include <psxsdk/libgte.h>` only declares prototypes, no asm macros).
 */
void func_80052B7C(MATRIX *rot, SVECTOR *bias, VECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    __asm__ volatile(
        "lh     $13, 0(%0)\n"
        "lh     $14, 2(%0)\n"
        "lh     $15, 4(%0)\n"
        "ctc2   $13, $5\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(bias) : "$13", "$14", "$15");
    gte_ldv0(v0);
    gte_rt();
    gte_stlvl(out);
}
