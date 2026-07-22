/* s10 KILLED (score 21, from floor 6). Region A' attempt: co-bracket sum AND bp
 * in one do-while(0), j=0 outside/last, hoping sched1 emits sum,bp then j (target
 * A' order) while keeping sum's RA win.
 *   do { sum = 0; bp = base + offset; } while (0);
 *   j = 0;
 * RESULT: bp is the inner-loop walking pointer (pseudo 78 = $v1, high ref count);
 * bracketing it raises bp's loop-depth-weighted refs and cascades the whole inner
 * allocation -> 21 diffs. Bracketing anything other than sum ALONE disturbs the
 * matched registers. Confirms s5/s6/s7: the +1 ref must be sum-only; A' coupling holds.
 */
