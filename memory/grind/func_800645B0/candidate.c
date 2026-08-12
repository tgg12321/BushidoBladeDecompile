/* func_800645B0 (src/text1b.c) — grind session 2 (structural modality), 2026-08-12.
 *
 * HONEST PURE-C SANDBOX DISTANCE: 1  (`sandbox func_800645B0 --disable all`,
 * target_insns 78 / build_insns 78, rules_dropped 1).  Session 1's floor was
 * 3; this is variant "AA" of tmp/grind/func_800645B0/s2/sweep6.py.
 *
 * DELTA vs the session-1 candidate: the first `rand()` call is named into the
 * existing `last` temp, and the *3 (12-byte-stride) word index is then
 * assigned back into `idx`:
 *
 *      idx2 = idx << 1;
 *      last = rand();
 *      idx  = idx2 + idx;
 *      *((s32 *)((s32)&D_800F0D78 + (idx << 2))) = base[0] + (last & 0xFF) - 0x7F;
 *
 * Naming a call result into a temp before the statement that consumes it is
 * the same spelling the session-1 form already uses for the s16 store
 * (`last = rand(); val = D_800A3444; *sh = last & 7;`), and reusing `idx` for
 * the derived word index is the sanctioned variable-reuse family
 * ([[defeat-licm-hoist-var-reuse]]) — the target's own allocation does exactly
 * this ($s0 = i+j, then $s1+$s0, then $s0<<2).
 *
 * WHY IT MOVES THE FLOOR (two independent effects, both measured):
 *  1. `idx = idx2 + idx;` is a REAL computation, so reg_n_sets[idx] == 2 and
 *     sched.c's birthing_insn_p bonus no longer fires on `addu idx,i,j` at the
 *     inner-loop top; the block emits `addu` first and reorg.c steals it into
 *     the back-edge delay slot, exactly as target.  (Session 2 proved that a
 *     second assignment CSE/copy-prop can fold away — a copy, a split-init, a
 *     recomputed `idx << 1` — does NOT do this: sweep 5's five variants all
 *     produce a byte-identical .sched ready list to the single-set form.)
 *  2. Naming the first `rand()` puts the CALL ahead of the sum in source
 *     order, so GCC emits the sum AFTER the `jal` and reorg.c fills the jal
 *     delay slot with the `sll` — the target's shape.  With the sum ahead of
 *     the call (session 1's variant K) the sum fills the slot instead.
 *
 * WHAT REMAINS (1 point, mechanism fully identified — see evidence.md):
 *   idx | target              | this build
 *   20  | addu s0,s1,s0       | addu s0,s0,s1
 * optabs.c:399-417 (expand_binop) swaps a commutative operand pair whenever
 * `target == op1`, so ANY assignment of the form `idx = <x> + idx;` expands to
 * (plus idx x) — visible already in the .rtl dump, before cse ever runs.  The
 * target's operand order therefore requires the sum's DESTINATION pseudo to
 * differ from `idx` — which costs effect 1 above (variant CA: operand order
 * right, loop top wrong, score 3) or, if the byte offset is also routed
 * through a multi-block pseudo, flips the $s0/$s1 assignment of `idx`/`idx2`
 * and costs 12 (variant DA).
 *
 * Byte-identical alternatives measured this session (all score 1 / 78):
 * naming every `rand()` into `last` (AB), and spelling the sum `idx = idx * 3`
 * (AC).
 *
 * INTEGRATION NOTE: regfix.txt:2521 (`func_800645B0: reorder 3,1,2 @ 1-3`) is
 * the function's only rule; the sandbox drops it.  Retiring it is the
 * operator/driver's job, not a grind session's.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
