/* REJECTED: dead-computation-cheat — permuter-found form using dead vars and
 * uninitialized arithmetic to defeat the fold. Not formally reviewed (clear cheat).
 * The form uses uninitialized variables in dead computations, (double) cast on a
 * boolean, and new_var = 0 to index p[0] indirectly. Not a legitimate pure-C form.
 * Permuter also found variants with long long new_var2 + empty if(new_var2){}.
 * All these are discarded. Do not re-derive. */
s32 func_80078EC0(void) {
    int new_var4;
    int new_var;
    unsigned int new_var3;
    unsigned int new_var2;
    s32 *p = (s32 *) D_8009BD88;
    if ((double) ((p[1] & 1) == 0)) { return 0; }
    new_var = 0;
    if ((p[new_var] & 1) != 0) { return 1; }
    new_var3 = new_var2 & new_var4;
    new_var4 = 0xFFFF; new_var2 = new_var; new_var2 = new_var2 & new_var4;
    new_var3 = new_var2;
    return new_var3;
}
