/* PsyQ 4.0 LIBSND VM_VSU: _SsVmVSetUp. .text 0x80087E3C..0x80087F00, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

s32 _SsVmVSetUp(s32 a0, s32 a1) {
    s16 vabId = a0;
    s16 prog = a1;
    if (vabId < 0 || vabId >= 0x10 || _svm_vab_used[vabId] != 1 || prog >= kMaxPrograms) {
        return -1;
    }
    _svm_vh = _svm_vab_vh[vabId];
    _svm_pg = _svm_vab_pg[vabId];
    _svm_tn = _svm_vab_tn[vabId];
    _svm_cur.vabId = vabId;
    _svm_cur.prog = prog;
    _svm_cur.field_7_fake_program = _svm_pg[prog].reserved1;
    return 0;
}
