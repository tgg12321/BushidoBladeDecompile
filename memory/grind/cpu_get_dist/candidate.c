/* SANDBOX-0 CANDIDATE (s2, 2026-07-21) — pure C, zero rules, zero cheat-asm.
 * sandbox --disable all = 0 (build 62 == target 62); unmasked diff (s2/diff_t3.txt)
 * is instruction+register identical to target modulo the 6 assembler-stage
 * hazard/latency nops the scorer normalizes on both sides.
 *
 * Constructs (all sanctioned families, FAKE-annotated at site):
 *  - do-while(0) fence: sanctioned per do-while-zero-exception.md (owner FINAL
 *    ruling 2026-07-06 — any body, any codegen effect incl. RA).
 *  - two dead stores to local rx reading only INITIALIZED locals: sanctioned per
 *    dead-store-fake-exception.md (2026-07-01) after documented lever exhaustion
 *    (24-form prior sweep all >=21; s2's 30+ additional clean forms plateau at 15).
 *
 * Mechanism (validated on THIS form: s2/iso_t3.c.sched, .greg, iso_t3.s):
 *  sched.c pins the first in-loop insn as the loop-note fence pivot. The FAKE
 *  `rx = cos_val * cos_val;` first-statement gives cos_val an in-loop first use,
 *  so combine's shared extendhisi2 (the cos lh) materializes there and BECOMES
 *  the pivot (insn 47); lw vx (insn 54) then truly depends on it — forcing
 *  lh-before-lw (defeating the rank_for_schedule class-3 hoist, the documented
 *  wall) and making cos_idx + vx share $v1 (greg: pseudos 77/89/92 -> reg 3).
 *  Both dead stores are DCE'd: zero emitted trace, bytes all from compiled C.
 *  The second FAKE dead store (post-sin) is load-bearing for the tail schedule
 *  (removing it: 25/64-insn regression, see s2 t2 log).
 */
void cpu_get_dist(s32 *a0, s16 *a1) {
    s32 angle;
    s16 cos_val;
    s16 sin_val;
    s32 vx;
    s32 vz;
    s32 rx;
    s32 rz;
    s32 v48;
    angle = single_game_getEnemyCharId(a1[0], a1[2]);
    cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
    /* FAKE: do-while(0) scheduling fence */
    do {
        rx = cos_val * cos_val; /* FAKE: dead store */
        vx = *((s32 *)(((u8 *)a0) + 0x44));
        sin_val = *((&Judge) + (angle & 0xFFF));
        rx = ((vx * cos_val) + (vx * sin_val)) >> 12; /* FAKE: dead store */
        vz = *((s32 *)(((u8 *)a0) + 0x4C));
        rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
        rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
        v48 = *((s32 *)(((u8 *)a0) + 0x48));
        *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
        *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
        if (v48 < 0) {
            v48 += 3;
        }
    } while (0);
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
