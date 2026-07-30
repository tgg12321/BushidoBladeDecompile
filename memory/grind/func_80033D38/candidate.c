/* func_80033D38 -- src/code6cac_b.c -- CANDIDATE, honest pure-C distance 0.
 *
 * Session s1 (recon modality, 2026-07-29): floor 17 -> 9 -> 4 -> 1 -> 0.
 * `sandbox func_80033D38 --disable all` reports masked score 0, and an
 * independent register-INCLUSIVE aligned diff against asm/funcs/func_80033D38.s
 * (tmp/grind/func_80033D38/s1/cmp.py) reports 47/47 instructions with 0
 * differing -- so the match is not a masked-0 hiding a register diff.
 * Zero register-asm pins, zero inline asm, zero volatile, zero dead stores,
 * zero dead declarations. Every local is read.
 *
 * WHAT THE FUNCTION DOES
 * D_80106A50 is viewed as an array of 8-byte records { u8 x; u8 y; s32 t; }
 * (t at +4, stride 8). Slots 1..3 hold a short history of recent events, sorted
 * so that slot 3 is the oldest. The function finds the highest slot n whose
 * record is older than the current time D_800A3858, publishes n to the u8
 * D_800A38E9, shifts records upward to open slot n+1, and writes a fresh record
 * { D_80101ED2, D_80101ED6, D_800A3858 } into it. (Record 0 overlaps the
 * bit-flags word that the rest of code6cac_b.c reads as `D_80106A50 & mask`;
 * this function never touches slot 0.)
 *
 * WHY THIS EXACT SPELLING -- the four levers, each measured
 *
 * 1. TWO loop variables, n and j, not one. The natural single-variable scan
 *    `for (i = 3; i > 0; i--) if (recs[i].t < D_800A3858) break;` is a hard
 *    plateau at distance 9 (45 insns, two SHORT of target): loop.c strength-
 *    reduces the single DEST_ADDR giv into a walking pointer
 *    (`addiu v1,t1,24` / `addiu v1,v1,-8`), so the in-loop `sll`+`addu` index
 *    math disappears. Target keeps that math, and its `addu a3,v1,zero` is a
 *    plain register COPY -- the signature of a source-level `n = j;`. The
 *    invariant at the load is a3 == v1 + 1, i.e. j is the loop's own biv
 *    (2,1,0) and the record examined is recs[j + 1]. So the original source
 *    carried the scan index and the insertion slot as separate live variables.
 *
 * 2. `j = n - 1;` is the FIRST statement of the loop body, not the last.
 *    Target has `addiu v1,a3,-1` TWICE -- once in the preheader and once in the
 *    bgtz delay slot -- which is reorg.c stealing the loop-top instruction into
 *    the backedge delay slot and duplicating it into the preheader. With the
 *    decrement written last instead (`... n = j; if (n <= 0) break; j = n - 1;`)
 *    the preheader copy sits in the same basic block as `n = 3;`, cse folds it
 *    to a literal `addiu v1,zero,2`, and the form stalls at distance 4.
 *
 * 3. `while (1) { ... if (n <= 0) break; }`, NOT `do { ... } while (n > 0);`.
 *    Both express the same loop, but the do/while spelling makes GCC PEEL the
 *    first array test out of the loop (`lw v0,28(v0)` on a constant-folded
 *    address) and rotate the array compare onto the backedge: 51 insns,
 *    distance 24-25. The while(1)-with-breaks spelling keeps the load at the
 *    loop top and the `n > 0` test on the backedge, exactly as target. Also
 *    load-bearing: the exit test must be `n <= 0` (which becomes the `bgtz`
 *    backedge). Writing the equivalent `n < 1` emits slti+beqz instead and
 *    costs distance 25.
 *
 * 4. The element address is materialised in a NAMED POINTER, in both the loop
 *    and the tail. `recs[j + 1].t` and `(recs + j + 1)->t` build the address as
 *    plus(scaled_index, base) and emit `addu v0,v0,t1`; target has
 *    `addu v0,t1,v0` (base first). Assigning the address to a pointer variable
 *    first (`p = recs + j + 1;` / `ins = recs + n + 1;`) flips the operand
 *    order to base-first and matches. This was the last instruction in the loop
 *    and the last in the tail -- two independent occurrences of one mechanism.
 *
 * INTEGRATION NOTE: regfix.txt still carries the 6 legacy bridge rules for this
 * function (lines ~3447-3452). They were written against the old pinned goto
 * body and are now wrong; the operator/driver must retire them (`retire
 * func_80033D38`) for the full build to link byte-identical. Nothing in this
 * candidate depends on them -- the sandbox scored it with all 6 dropped.
 */
void func_80033D38(void) {
    struct HitRec {
        u8 x;
        u8 y;
        s32 t;
    };
    struct HitRec *recs = (struct HitRec *)&D_80106A50;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        struct HitRec *p;
        j = n - 1;
        p = recs + j + 1;
        if (p->t < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        struct HitRec *ins;
        for (k = 2; k > n; k--) {
            recs[k + 1] = recs[k];
        }
        ins = recs + n + 1;
        ins->x = (u8)D_80101ED2;
        ins->y = (u8)D_80101ED6;
        ins->t = D_800A3858;
    }
}
