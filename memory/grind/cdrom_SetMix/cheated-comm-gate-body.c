/* cdrom_SetMix -- the COMPLETED-C body as landed, REVERTED 2026-09-30: its match depended on the
 * per-function maspsx COMMON gate (maspsx_comm_syms.txt), which the owner ruled a cheat
 * (docs/grind/decisions.md 2026-09-30 OWNER RULING). Row: `cdrom_SetMix: g_cd_atv`.
 * Kept as a lead, NOT a landable form. */
void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    g_cd_atv.val0 = (u8)arg0;
    g_cd_atv.val1 = (u8)arg1;
    g_cd_atv.val2 = (u8)arg2;
    g_cd_atv.val3 = (u8)arg3;
    CdMix(&g_cd_atv);
    D_800A3854 = 0;
}
