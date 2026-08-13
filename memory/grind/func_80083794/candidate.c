/* func_80083794 — best honest pure-C form, session 1 (recon).
 * Honest sandbox floor: 18 (target_insns 28, build_insns 28).
 * Structure is instruction-for-instruction isomorphic to target; the residual is
 * (a) frame layout — see hypotheses.md H1, and (b) register assignment
 * (temp v0-vs-t0, and the s0/s1 roles being swapped relative to target).
 *
 * Identity: this is GCC's `__main` — the only caller is main() (src/ings.c:589,
 * first statement), and the body is libgcc2.c's `__main` with
 * `__do_global_ctors` inlined:
 *     static int initialized;
 *     if (!initialized) { initialized = 1; <walk ctor table, call each>; }
 * D_800A2668 is the `initialized` flag, D_8008D070 is the ctor table base,
 * D_00000000 is a link-time absolute symbol whose VALUE (not contents) is the
 * ctor count — hence the `la` (lui/addiu) rather than a load.
 *
 * NOTE: the 9 regfix rules still present for this function (regfix.txt:105-113)
 * were calibrated against the previous register-pin/inline-asm form that this
 * body replaces, so the integrated build is expected to disagree until those
 * rules are retired. The honest sandbox floor is the gradient that matters.
 */
extern s32 D_800A2668;
extern void (*D_8008D070)(void);
extern s32 D_00000000;

void func_80083794(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        while (count != 0) {
            (*p++)();
            count--;
        }
    }
}
