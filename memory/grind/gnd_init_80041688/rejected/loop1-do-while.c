/* s12 KILLED: loop1 as natural do-while regresses score 2 -> 7 (+1 insn, 82 -> 83).
   GCC 2.7.2's front-end lowers `do {...} while (cond);` to different RTL than the
   manual `goto` loop — reorg.c picks a different rotation and adds a compensating
   insn. The manual goto/label form is codegen-optimal here; natural loop shapes
   are strictly worse. Empirically KILLS the "loop1-shape" axis. */

    i = 1;
    do {
        p += 0x68;
        if (*(s16 *)(p + 2) >= 0) {
            if (arg1) p[1] |= 1;
            else      p[1] &= ~1;
        }
        i++;
    } while (i < 18);
