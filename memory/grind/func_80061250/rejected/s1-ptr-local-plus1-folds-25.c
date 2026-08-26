/* REJECTED s1: sandbox 25 (59 insns). u8 *flag = &D_800F1159; pointer local.
 * Base register forms for the [0] accesses and the stored address value, but
 * cse substitutes the plain-symbol equiv into the +1 addresses: all three
 * flag[1] accesses fold to absolute D_800F1159+1, and the two 0x2100xx
 * constants share one hoisted `lui 0x21` in beqz delay slots (target
 * materializes each arm's constant fully). Same root cause as the 19-form:
 * plain-symbol (addend-0) cse equivalence. */
extern u8 D_800F1159;
void func_80061250(s32 *arg0) {
    u8 *flag = &D_800F1159;
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (flag[0] != 0) {
        if (flag[1] != 0) {
            flag[1] = 0;
            flag[0] = 0;
        }
        if (flag[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)flag;
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if (flag[1] == 0) {
        D_800F1180 = (s32)(flag + 1);
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
