/* func_800204C0 — rec->unk_350 is a frame counter func_800203B4 starts at 1. On the tick
 * where (counter & 7) == 2, the vector func_800203B4 stored at rec->unk_354 is rotated by
 * matrix rec->unk_352 of game_GetPlayerData(pid) (GTE MVMVA), scaled by (150 - counter) / 150,
 * negated when its y exceeds 0x800, and handed to func_80032854 together with scratchpad
 * point SPAD->unkA8[pid][rec->unk_352]. The counter is cleared on every call that finds it
 * running (the original stores 0 under `>= 150` and then again unconditionally).
 * GTE islands: PsyQ Run-time Library Release 4.3 inline_o.h statements, character for
 * character (engine/gtemacro.py PINNED). */
void func_800204C0(PracticeMenuRec *rec) {
    s32 mac[3];
    s16 out[3];
    s32 pid;
    s32 mul;
    MATRIX **bones;

    pid = rec->unk_04;
    if (rec->unk_350 != 0) {
        rec->unk_350 += 1;
        if ((rec->unk_350 & 7) == 2) {
            bones = (MATRIX **)game_GetPlayerData(pid);
            /* inline_o.h: gte_SetRotMatrix :272-284 */
            __asm__ volatile ("move  $12,%0": :"r"(bones[rec->unk_352]):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
            /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430; gte_rtv0's command word is the
             * post-DMPSX word .word 0x4A486012 in place of the header's DMPSX placeholder
             * .word 0x0000013f (MVMVA sf=1 mx=rot v=V0 cv=none lm=0; per-function grant Q92, ded098133) */
            __asm__ volatile ("move  $12,%0": :"r"(&rec->unk_354):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
            mul = ((0x96 - rec->unk_350) << 12) / 150;
            /* inline_o.h: gte_stlvnl :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(mac):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            out[0] = (mac[0] * mul) / 0x1000;
            out[1] = (mac[1] * mul) / 0x1000;
            out[2] = (mac[2] * mul) / 0x1000;
            if (out[1] > 0x800) {
                out[0] = -out[0];
                out[1] = -out[1];
                out[2] = -out[2];
            }
            func_80032854(pid, 4, &SPAD->unkA8[pid][rec->unk_352].x, out);
        }
        if (rec->unk_350 >= 0x96) {
            rec->unk_350 = 0;
        }
        rec->unk_350 = 0;
    }
}
