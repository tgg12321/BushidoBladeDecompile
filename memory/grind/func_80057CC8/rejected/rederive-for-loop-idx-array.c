/* REJECTED 2026-07-19 (s9 rederive) — REGRESSION 3 -> 77.
 * Form: `s32 i; for (i=0; i<2; i++) { p = ...; ang[i] = ...; }` with
 * `unsigned short idx[2]` and `s32 ang[2]` replacing prev_idx/next_idx and
 * ang_prev/ang_next. Rationale for trying: fresh rederive angle — the two
 * pointer-compute + call blocks are semantically identical (compute angle
 * from current point to arena[idx[i]]); a human writing the loop-shape from
 * spec would collapse them. Two-iteration loop lets GCC pick a fresh RTL
 * pseudo per iteration via loop RA.
 * Result: sandbox --disable all == 77, build_insns=118 vs target 111.
 * GCC 2.7.2 at -O2 does NOT unroll a `for(i=0;i<2;i++)` (needs -funroll-loops,
 * which is NOT in the project's canonical flag set per compiler-flags-canonical).
 * The loop compiles as an actual loop with branch and induction, adding ~7
 * insns and preventing the scheduling window target uses. Not a lever. KILLED.
 * See s9 ledger; hypothesis KILLED — loop-collapse over the two call sites is
 * not a valid rederive-modality axis for this function under -O2. */
extern s32 single_game_getEnemyCharId(s32, s32);
extern s16 Judge;
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short idx[2];
    s32 ang[2];
    s32 ang_mid, scale, base, half;
    u16 cx, cy;
    s16 new_var;
    s16 *p;
    s16 *table;
    s32 i;

    idx[0] = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);
    if ((s16) idx[0] < 0) idx[0] = arg0[3] - 1;
    { s32 tmp = arg1 + 1; idx[1] = tmp;
      if ((s16) tmp >= (s32)arg0[3]) idx[1] = 0; }

    for (i = 0; i < 2; i++) {
        p = (s16 *)((((s32)(idx[i] << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
        ang[i] = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    }

    if (ang[1] < ang[0]) {
        base = ang[0] + 0x800;
        half = (s32)(ang[0] - ang[1]) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang[1] - ang[0]) / 2) + ang[0];
    }
    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
