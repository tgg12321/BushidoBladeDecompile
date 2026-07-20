/* REJECTED s1 (recon, 2026-07-20): in-loop named pointer, member stores kept,
 * arg routed through the name. Score 2 — BYTE-IDENTICAL to HEAD form.
 * Why dead: combine merges `P = sp+24` into the call-site arg load whenever P
 * dies at the arg use (LOG_LINKS intra-block), so the addiu keeps the call-site
 * LUID and both schedulers slot it 3rd (priority-1 three-way tie broken by
 * LUID; see s1 sched/sched2 traces). Source position/naming of the pointer has
 * ZERO effect — confirmed twice: this pure form, and a scratch-only diagnostic
 * with `register s32 *dst asm("$4")` set first in source (identical output).
 * Any spelling where the pointer's only real use is the call arg is this form.
 */
    do {
        s32 *dst = &s.sp18;
        s32 v = *q;
        q++;
        s.sp18 = v;
        s.sp1C = v + 0xC;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)dst);
        i++;
    } while (i < 3);
