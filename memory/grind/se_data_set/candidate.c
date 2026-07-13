/* se_data_set — best form, session 2 (structural). sandbox --disable all = 0,
 * register-exact vs asm/funcs/se_data_set.s (verified by objdump of the sandbox .o,
 * tmp/grind/se_data_set/s2/final_form.txt).
 *
 * Delta vs HEAD (both flagged constructs from the 2026-06-22 regression audit):
 *   1. `D_800A390E = (s8)(u16)D_800A36A4;`  ->  `D_800A390E = D_800A36A4;`
 *      CLOSED CLEAN. The lhu falls out of the plain truncating store; both casts
 *      were m2c transcription artifacts. Zero cast, still register-exact 0.
 *   2. `s8 *p = (s8 *)&D_8010277C;` -> `u8 *p = &D_8010277C;` with the (s8) VALUE
 *      cast moved to the use sites, identical to the condition's spelling.
 *      This removes the "signed re-view through an lvalue" objection (the pointer
 *      is now type-correct for the u8 global and the sign semantics are spelled
 *      the same way in the condition and the body). The address cache itself is
 *      RETAINED as a pointer-alias-fake-exception candidate — see the FAKE
 *      annotation; PENDING RULING (the 2026-06-22 judge constraint predates the
 *      2026-07-01 sanction of the family).
 */
void se_data_set(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        /* FAKE: block-local address cache for D_8010277C. The direct spelling
         * `(s8)D_8010277C` at both body sites re-materializes the symbol
         * address instead of holding it in a callee-save register across the
         * func_8005BA8C call (sandbox 6). GCC only creates an address pseudo
         * from a C-level `&` bound to a variable; cse.c will not manufacture
         * one. Declaring the pointer at function scope also fails (6), as does
         * using it in the condition (8). See memory/grind/se_data_set/. */
        u8 *p = &D_8010277C;

        EndADRSound();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        s1 = func_8005BA8C(s2, D_800A36A4, *(&D_8008E5A8 + (s8)*p), *(&D_8008E5A8 + D_8010277D));

        D_800A390E = D_800A36A4;
        D_800A30FC = *(&D_8008E5A8 + (s8)*p);
        D_800A30FD = *(&D_8008E5A8 + D_8010277D);

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        bb2_memcpy(s0, s2, s1);
        func_8005BD30((s32)s0 - s2);
    }
}
