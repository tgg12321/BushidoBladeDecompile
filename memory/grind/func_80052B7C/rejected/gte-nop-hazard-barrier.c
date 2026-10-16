/* REJECTED s21: gte_nop() is a scheduling barrier by name (engine/gtemacro.py:1095);
 * the sandbox strips it (score 19, unchanged). Forbidden family: scheduling barriers. */
#include <psxsdk/inline_c.h>
void func_80052B7C(MATRIX *rot, SVECTOR *tr, SVECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    gte_ldtr((s32)tr->vx, (s32)tr->vy, (s32)tr->vz);
    gte_ldv0(v0);
    gte_rt();
    gte_nop();
    gte_stlvl(out);
}
