/* func_80035F78 -- the COMPLETED-C body as landed, REVERTED 2026-09-30: its match depended on the
 * per-function maspsx COMMON gate (maspsx_comm_syms.txt), which the owner ruled a cheat
 * (docs/grind/decisions.md 2026-09-30 OWNER RULING). Row: `func_80035F78: D_800A36B8`.
 * Kept as a lead, NOT a landable form. */
void func_80035F78(s16 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A36B8.val0 = (u8)arg1;
    D_800A36B8.val1 = (u8)arg2;
    D_800A36B8.val2 = (u8)arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36B8.val3 = (u8)arg4;
}
