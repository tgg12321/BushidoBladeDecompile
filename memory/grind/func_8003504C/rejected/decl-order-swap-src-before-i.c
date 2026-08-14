/* REJECTED — floor stays 24 and the normalized diff is BYTE-IDENTICAL to
 * baseline (all 26 differing positions unchanged). Completely codegen-neutral.
 * H1': declaring src before i lowers its allocno number and wins the priority
 * tie. Killed s1 — there is no tie: measured reg_n_refs are i=7 vs src=5, so
 * global.c's reg_n_refs/live_length ordering decides it outright and
 * declaration order never enters the comparison. Do not re-propose.
 */
    s32 *p;
    u8 *src;      /* <-- declared ahead of i */
    s32 i;
    u8 *ptr;
    s8 *base;
    s8 val;
    u8 tmp;
