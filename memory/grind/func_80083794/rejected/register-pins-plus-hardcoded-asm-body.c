/* REJECTED — func_80083794, session 1.
 *
 * Why it is dead: this is the form that was sitting in src/ings2.c on arrival.
 * It is a cheat by three separate spellings at once —
 *   - `register T x asm("s0"/"s1"/"t0")` register pins (forbidden family:
 *     register-asm pins),
 *   - a hardcoded-`$17` single-instruction `__asm__` (forbidden family:
 *     lost-codegen / inline-asm injection, .claude/rules/inline-asm-injection.md),
 *   - an `__asm__("jalr %0")` standing in for the actual C call.
 * The cheat-invisible sandbox strips all of it, so it is also USELESS: it scored
 * 23 with build_insns 16 out of 28 — i.e. after stripping, the loop body did not
 * exist at all. The ordinary C rewrite scored 22 on first try and 18 after one
 * statement-order change.
 *
 * Do not reconstruct any part of this, in any spelling.
 */
void func_80083794(void) {
    register void (**p)(void) asm("s0");
    register s32 count asm("s1");

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                register void (*f)(void) asm("t0") = *p;
                p++;
                __asm__ volatile("jalr %0" :: "r"(f) : "ra", "memory");
                __asm__ volatile("addiu $17, $17, -1" : "=r"(count) : "0"(count));
            } while (count != 0);
        }
    }
}
