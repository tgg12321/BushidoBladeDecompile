/* REJECTED s2 (2026-07-13): sandbox distance 6 (55 vs 56 insns).
   (&D_8010277C)[1] is a compile-time-constant address; GCC folds it straight
   back to a symbolic MEM (D_8010277C+1) -> same $at macro form. Only a pointer
   VARIABLE (RA-visible pseudo) produces the pre-branch lui/addiu $v1 +
   register-indirect sb the target requires. */
    if (a1 != 0) {
        (&D_8010277C)[1] = 0xE;
    } else {
        (&D_8010277C)[1] = 0x1D;
    }
