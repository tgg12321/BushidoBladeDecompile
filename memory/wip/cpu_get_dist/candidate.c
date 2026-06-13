/* HONEST BASELINE — pure-C form, NO cheats (scores 21 cheat-stripped).
 *
 * HEAD's committed src for cpu_get_dist (code6cac_b.c) carries two
 * forbidden coercion cheats (see notes.md for evidence): a do-while(0)
 * wrapper around the sin/vz/rx/rz/v48 region, and a dead `rx = (vx*cos
 * + vz*sin) >> 12;` BEFORE `vz` is assigned (reads uninit vz). Both
 * are [[no-new-park-categories]] cheats-by-spelling — they have no
 * semantic purpose; the wrapper is a NOTE_INSN_LOOP_BEG scheduling
 * fence, and the dead store force-extends vz's live range upward to
 * bias register allocation. With those constructs HEAD's
 * cheat-stripped sandbox is 8 (closed by 4 regfix rules); without
 * them (this body) it's 21.
 *
 * This body is COMPLIANT C and is what the next session should resume
 * from when seeking a pure-C lever. The 13-point gap traces to a
 * single sched1 decision (see notes.md "RTL mechanism") that no
 * structural variant in the sweep flipped.
 */
void cpu_get_dist(s32 *a0, s16 *a1) {
    s32 angle = single_game_getEnemyCharId(a1[0], a1[2]);
    s16 cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
    s32 vx = *((s32 *)(((u8 *)a0) + 0x44));
    s16 sin_val = *((&Judge) + (angle & 0xFFF));
    s32 vz = *((s32 *)(((u8 *)a0) + 0x4C));
    s32 rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
    s32 rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
    s32 v48 = *((s32 *)(((u8 *)a0) + 0x48));
    *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
    *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
    if (v48 < 0) {
        v48 += 3;
    }
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
