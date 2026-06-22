/* func_80061710 — best pure-C floor (masked-distance 9, no rules, no pins).
 *
 * The HEAD form pins t->$2 and mask->$3. With pins stripped, GCC's natural
 * allocator emits the SAME instruction schedule as target byte-for-byte
 * EXCEPT the v0/v1 registers are swapped on every load/mask insn (9 such
 * insns -> masked Levenshtein = 9). All pure-C variants explored this
 * session reproduce that same swap or land at a worse floor. Save the
 * cleanest no-pins natural form so the next agent resumes from here, not
 * HEAD.
 */
void func_80061710(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        p = &D_800F115C + 2;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000F;
        p = &D_800F115C + 3;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0x10FF10;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
