/* REJECTED as a closing form (floor 24 -> 28) but MEASUREMENT-POSITIVE —
 * this is the session-1 proof that cluster 1 IS reachable. Read before probing.
 *
 * H3: cluster 1's inversion is a global.c allocno-priority problem
 * (priority = reg_n_refs / live_length, hard regs handed out in MIPS allocation
 * order), so extending i's live range past loop 1 drops its ratio below src's
 * and hands src the earlier register.
 *
 * MEASURED: the intended flip HAPPENED — positions 12, 38, 39 went clean and
 * `src` landed in $a2 exactly as target wants. But i over-shot past `base`:
 *     got:    ptr=$a1, src=$a2, base=$a3, i=$t0
 *     target: ptr=$a1, src=$a2, i=$a3,    base=$t0
 * So this exact extension is one discrete position too strong. The next probe
 * needs a SMALLER ratio drop for i (or a matching drop for base) — e.g. reuse i
 * as loop 2's INNER counter instead of its outer one (flow.c weights
 * reg_n_refs by loop depth, so inner-loop refs raise the numerator while the
 * live range grows by roughly the same span), or reuse it for the inner `off`.
 * Re-run refs.py + the greg dispositions after each variant rather than
 * inferring from the score alone.
 *
 * NB: this construct is NOT a cheat — `i` is genuinely dead after loop 1 and is
 * reused as a live loop counter with no dead store anywhere. It was rejected on
 * SCORE, not on policy.
 */
        {
            u8 *dst_d = &D_801027D8;
            u8 *dst_a = &D_801027A0;
            i = 0;                  /* reuse loop-1's counter; `s32 j` deleted */
            do {
                s32 k = 0;
                u8 *da = dst_d;
                u8 *db = dst_a;
                s32 off = i << 1;
            loop_inner:
                {
                    u8 *pp = (u8 *)p + off;
                    *db = (&D_8008D55C)[pp[0]];
                    off += 10;
                    k++;
                    *da = pp[1];
                    db++;
                    da++;
                }
                if (k < 2) goto loop_inner;
                dst_d += 2;
                i++;
                dst_a += 2;
            } while (i < 5);
        }
