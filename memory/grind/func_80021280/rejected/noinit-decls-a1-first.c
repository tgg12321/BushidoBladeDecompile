/*
 * REJECTED (s1, 2026-08-03): block-scope no-initializer declarations with
 * assignments in TARGET order (`a1 = 0;` first, then t1..t0):
 *
 *   u16 t1; s32 t4, t3, t2, mode; u8 t0;
 *   a1 = 0; t1 = val; t4 = 4; t3 = 3; t2 = 1; mode = D_800A38DC; t0 = D_800A384C;
 *
 * -> floor 19 (72/72 insns): the $a1/$a2 register swap returns.
 *
 * MECHANISM: with `a1 = 0;` textually before `t1 = val;`, a1 is live
 * across t1's birth -> a1<->t1 pseudo conflict added -> the allocator's
 * low-register tie flips against a1 (the same swap the fresh-`i` HEAD
 * form has). Moving `a1 = 0;` to SECOND (after `t1 = val;`) keeps RA
 * correct (floor 2) and the emitted order follows source exactly, so the
 * boundary is precisely the a1<->t1 conflict — NOT function-vs-block
 * scope (WIP's function-scope no-init rejection is a different, weaker
 * datum). Any future form must zero a1 ahead of t1 WITHOUT creating
 * that pseudo conflict at RA time.
 *
 * Also neutral (not a kill, just useless): splitting `a1 << 2` into its
 * own named local `s32 sh2 = a1 << 2;` — floor stays 2, identical bytes
 * (WIP next_avenues item 3, now measured).
 */
