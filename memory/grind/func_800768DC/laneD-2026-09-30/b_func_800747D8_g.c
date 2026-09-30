s32 func_800747D8(u32 input) {
    SelWork *base;
    s32 sp10;
    s32 ret;
    s32 state;
    s32 result;
    s32 i;
    s32 sound;

    base = SELWORK;
    result = 0;
    if (base->f10.word == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, base->f40[0], D_800A35D0[0]);
        switch (ret >> 16) {
    case 1:
        SELWORK->f34 = 0;
        SELWORK->f3C.half[0] += 1;
        if (SELWORK->f3C.half[0] >= 5) {
            SELWORK->f3C.half[0] = 0;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    case 2:
        SELWORK->f34 = 0;
        SELWORK->f3C.half[0] -= 1;
        if (SELWORK->f3C.half[0] < 0) {
            SELWORK->f3C.half[0] = 4;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
        }

        state = SELWORK->f3C.half[0];
        switch (state) {
    case 0:
        switch (ret & 0xFF) {
        case 1:
            if (SELWORK->f65 == SELWORK->f64) {
                SELWORK->f65 = 0;
            } else {
                SELWORK->f65 += 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        case 2:
            if (SELWORK->f65 == 0) {
                SELWORK->f65 = SELWORK->f64;
            } else {
                SELWORK->f65 -= 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        }
        goto confirm;
selection_sound:
        if (SELWORK->f64 != 0) {
            sound = 0;
        }
        func_8005C650(sound, 0x7F, 0x7F);
        goto confirm;
    case 1:
        if ((ret & 0xFF) != 0) {
            SELWORK->f67 += 1;
            SELWORK->f67 &= 1;
            SELWORK->f66 = D_8009BD20[SELWORK->f67][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
    case 2:
        if ((ret & 0xFF) != 0) {
            D_800A35DC += 1;
            SELWORK->f66 = D_8009BD20[SELWORK->f67][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
confirm:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK->f3C.half[0] = 3;
        }
        break;
    case 3:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            for (i = 0; i < 2; i++) {
                SELWORK->f10.half[i] = 3;
                SELWORK->f18[i] = 1;
                SELWORK->f38[i] = 0;
            }
        }
        goto tail;
    case 4:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            result = -1;
        }
        goto tail;
        }
tail:
        if (input & 0x100010) {
            func_8005C650(2, 0x7F, 0x7F);
            result = -1;
        }
    }
    return result;
}
