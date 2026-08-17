/* func_800453E0 — src/text1a_c.c — MATCHING FORM
 *
 * Session 1 (2026-08-17, modality: recon). Honest pure-C distance:
 *   26 (inherited, with cheat) -> 0.
 * `sandbox func_800453E0 --disable all` == 0 with all 7 regfix rules dropped
 * and cheat-asm stripped; all 76 instructions match target register-for-register
 * and mnemonic-for-mnemonic. No cheats: zero inline asm, zero register pins,
 * zero volatile, zero dead stores, zero unused locals. Every local is read.
 *
 * SEMANTICS (recovered, not transcribed): this is "remove the table entry whose
 * id == a0". D_800EED10 is an array of 16-byte records with D_800A33AC live
 * entries: { s16 id; s16 unk2; s32 unk4; s32 amt; void (*fn)(s16,s32); }
 * (field offsets 0/2/4/8/0xC, corroborated by the five sibling functions in this
 * file that walk the same table with a 0x10 stride). On a hit it calls
 * func_80045294(index+1, -amt) to undo the entry's contribution, shifts every
 * following record down one slot, clears the now-unused tail slot (id = -1,
 * fn = NULL) and decrements the count.
 *
 * WHY EACH PIECE IS SHAPED THIS WAY (each measured, see evidence.md):
 *  - The struct + a real 16-byte struct assignment for the shift-down loop is
 *    what produces target's 4×lw / 4×sw block plus the exact copy-loop
 *    scheduling (GCC expands it via movstrsi as one unit). The previous C's
 *    four hand-written named word temps cost 10 scheduling diffs.
 *  - The search loop and the tail-clear read/write through a source-level byte
 *    offset (`off = i << 4`, then `(u8 *)SYM + off`) — the idiom every sibling
 *    in this file uses. This keeps the base as a symbol so GCC re-materializes
 *    %hi(SYM)+offset per access, as target does. Binding a local
 *    `SubEntry *tbl` instead makes GCC strength-reduce to a walking pointer and
 *    CSE one base register across the whole function, which costs 7
 *    instructions.
 *  - `next` is a variable distinct from the shift loop's `j`. Target keeps
 *    index+1 in a callee-save ($s1, live across the call) and copies it into a
 *    caller-save ($t0) for the loop; reusing one variable for both merges them
 *    into a single pseudo and mis-assigns $s0/$s1.
 *  - `off = i << 4` inside the loop body (rather than a second induction
 *    variable advanced in the for-increment) makes it a strength-reduction giv,
 *    so its zero-init is emitted into the loop pre-header AFTER the hoisted base
 *    addresses — matching target's ordering.
 *  - The do-while with `count` re-read at the bottom of the body, and the
 *    `i >= count` entry guard, are copied from the adjacent already-matching
 *    sibling func_80045510 (same file). Reading the global into `count` before
 *    the increment fills the load-delay slot with `i += 1`; putting the
 *    increment first leaves an unfillable maspsx nop (1 diff).
 *  - The shift-down loop is written against SUBSLOT (the D_800EED00 base, the
 *    record immediately preceding the table) so BOTH of its base pointers derive
 *    from one symbol: $s2 = %lo(D_800EED00), $s3 = $s2 + 0x10. Spelling it
 *    `SUBTBL[j-1] = SUBTBL[j]` produces the same final linked bytes but emits a
 *    LO16 reloc against D_800EED10 with addend -16, which the sandbox scorer
 *    counts as 1 false distance.
 *
 * NOTE FOR INTEGRATION: the 7 regfix.txt rules for func_800453E0 (lines
 * 214-224) were written for the OLD C and must be deleted; a full build with
 * them in place will corrupt this function. Retirement is the operator's /
 * driver's step, not this session's.
 */

typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} SubEntry;

/* The subtitle/effect slot table. D_800EED00 is the 16-byte slot immediately
   preceding it, so SUBSLOT[n + 1] == SUBTBL[n]; the shift-down loop below is
   written against SUBSLOT so both its base pointers derive from one symbol. */
#define SUBTBL ((SubEntry *)D_800EED10)
#define SUBSLOT ((SubEntry *)D_800EED00)

void func_800453E0(s32 a0) {
    s32 i;
    s32 next;
    s32 j;
    s32 last;
    s32 off;
    s32 count;

    i = 0;
    count = D_800A33AC;
    if (i >= count) {
        return;
    }
    do {
        off = i << 4;
        if (*(s16 *)((u8 *)D_800EED10 + off) == a0) {
            next = i + 1;
            func_80045294(next, -*(s32 *)((u8 *)&D_800EED18 + off));
            if (i < D_800A33AC - 1) {
                for (j = next; j < D_800A33AC; j++) {
                    SUBSLOT[j] = SUBSLOT[j + 1];
                }
            }
            last = D_800A33AC - 1;
            *(s16 *)((u8 *)D_800EED10 + (last << 4)) = -1;
            *(s32 *)((u8 *)D_800EED1C + (last << 4)) = 0;
            D_800A33AC = last;
            return;
        }
        count = D_800A33AC;
        i += 1;
    } while (i < count);
}
