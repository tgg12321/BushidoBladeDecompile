/* REJECTED s1: sandbox 19 (62 insns). Direct (&D_800F1159)[0]/[1] spelling.
 * Tail matched target exactly; head diverged — with a plain-symbol anchor
 * (addend 0) GCC 2.7.2 cse folds EVERY flag access to absolute lui/%lo macro
 * addressing and no shared base register forms (target holds one la'd base in
 * $v1 with 0/1(reg) offsets). Not a construct problem — a cse anchor problem;
 * fixed by the D_800F1154+5 compound-const anchor (see candidate.c). */
extern u8 D_800F1159;
void func_80061250(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if ((&D_800F1159)[0] != 0) {
        if ((&D_800F1159)[1] != 0) {
            (&D_800F1159)[1] = 0;
            (&D_800F1159)[0] = 0;
        }
        if ((&D_800F1159)[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)&D_800F1159;
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if ((&D_800F1159)[1] == 0) {
        D_800F1180 = (s32)((&D_800F1159) + 1);
        *v1 = 0x21000A;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFF0060;
}
