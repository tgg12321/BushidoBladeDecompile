/* HONEST BASELINE v2 — pure-C form, NO cheats (scores 15 cheat-stripped,
 * down from the prior candidate's 21). 2026-06-13 algebraic-factoring lever.
 *
 * The improvement over the score-21 candidate is PURE reassociation +
 * statement order, no semantic change, no cheat construct:
 *   - rz is computed BEFORE rx (statement order only).
 *   - rz's inner sum is written vz*cos FIRST: ((vz*cos) + ((-vx)*sin)).
 * Both are commutative-add / independent-statement reorderings a human could
 * write; GCC's sched1 then emits an instruction stream 6 masked-edits closer
 * to target than the rx-first/vx*cos-first natural form. This is the
 * "fundamentally different algebraic factoring" the prior WIP notes flagged as
 * the one remaining clean direction — it WORKS as a partial lever.
 *
 * HEAD's committed src still carries two forbidden coercion cheats (do-while(0)
 * scheduling fence + dead `rx = (vx*cos + uninit-vz*sin) >> 12;`). This body is
 * COMPLIANT C and is what the next session should resume from. Remaining gap
 * (masked 15): target's mult order is vx*cos, vz*sin, (-vx)*sin, vz*cos with vz
 * loaded LATE; the residual is sched1 register allocation. Next modality:
 * directed permuter re-seeded from THIS base (the prior two campaigns seeded
 * from the score-21 base and found only cheat-forms).
 */
void cpu_get_dist(s32 *a0, s16 *a1) {
    s32 angle = single_game_getEnemyCharId(a1[0], a1[2]);
    s16 cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
    s32 vx = *((s32 *)(((u8 *)a0) + 0x44));
    s16 sin_val = *((&Judge) + (angle & 0xFFF));
    s32 vz = *((s32 *)(((u8 *)a0) + 0x4C));
    s32 rz = -(((vz * cos_val) + ((-vx) * sin_val)) >> 12);
    s32 rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
    s32 v48 = *((s32 *)(((u8 *)a0) + 0x48));
    *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
    *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
    if (v48 < 0) {
        v48 += 3;
    }
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
