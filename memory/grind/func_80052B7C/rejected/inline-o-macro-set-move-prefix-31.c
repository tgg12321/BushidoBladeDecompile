/* s20: inline_o.h macro set -- sandbox 31 (legal chassis is 19). Every pointer-taking
 * inline_o.h macro starts with move $12,%0 (inline_o.h:273); the target has no move insns. */
#include <psxsdk/inline_o.h>
void func_80052B7C(MATRIX *rot, SVECTOR *tr, SVECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    gte_ldtr((s32)tr->vx, (s32)tr->vy, (s32)tr->vz);
    gte_ldv0(v0);
    gte_rt();
    gte_stlvl(out);
}
