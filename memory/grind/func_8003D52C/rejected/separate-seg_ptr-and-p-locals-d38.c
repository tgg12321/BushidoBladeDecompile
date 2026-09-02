/* REJECTED (measured d38, s1 2026-09-02): separate `seg_ptr` (loop 1) and `p` (loop 2) locals.
 * seg_ptr alone never crosses a call -> global.c allocates it $v0; target has it in $s1 because
 * the original reused one pointer for both loops. Also carried pct/seg_start/buf_ptr holders,
 * which are byte-neutral (v3 without them also scores 0). */
