/* func_80036FD4 — grind candidate, session 2 (structural).
 *
 * Honest pure-C floor: 9 -> 2 (sandbox --disable all).  build_insns 78 -> 79
 * == target 79.  Zero cheats in the body: no pins, no __asm__, no volatile
 * coercion, no dead stores.  (Session 1 had already removed the pre-existing
 * `asm volatile("" ::: "memory")` barrier; nothing was added back.)
 *
 * ===== THE REMAINING 2 IS NOT AN INSTRUCTION DIFFERENCE =====
 * All 79 instructions match target in opcode AND register.  The two scored
 * words are RELOCATION ADDENDS:
 *
 *     ours    lui at, %hi(D_80101E60+12) ; sw a0, %lo(D_80101E60+12)(at)
 *     target  lui at, %hi(D_80101E6C)    ; sw a0, %lo(D_80101E6C)(at)
 *     ours    lui at, %hi(D_80101E60+16) ; sw a1, %lo(D_80101E60+16)(at)
 *     target  lui at, %hi(D_80101E70)    ; sw a1, %lo(D_80101E70)(at)
 *
 * 0x80101E60 + 12 == 0x80101E6C and + 16 == 0x80101E70, and GNU ld resolves an
 * o32 R_MIPS_HI16 using the addend of the *following* R_MIPS_LO16, so both
 * spellings link to byte-identical words.  The residual exists only because
 * splat named every word at 0x80101E60.. as its own symbol in
 * asm/funcs/func_80036FD4.s, and the sandbox compares unlinked object words.
 * See docs/grind/decisions.md (2026-07-29) — this is an INTEGRATION HANDOFF,
 * not a pure-C gap.  The 8 regfix rules must be retired first: with them
 * ENABLED the score is 4 (they now actively corrupt correct codegen).
 *
 * ===== WHY A SHARED-BASE RECORD IS THE LEVER (the mechanism) =====
 * Session 1 established that an aggregate (BLKmode) store is what flushes
 * cse's memory table and so restores target's reload of D_80101E60 (floor
 * 17 -> 9).  Session 2's finding is that the SAME record must ALSO be the
 * base of the halfword accesses, for a completely different reason — the
 * instruction scheduler:
 *
 *   - cc1's post-reload scheduler (sched2, -O2) schedules this block
 *     backwards.  Target needs the pre-jal `lh` to sit AFTER both `sw`s and
 *     to pay a load-delay `nop`.  With distinct symbols the block-move insn
 *     is the only insn ready in that load-delay slot, so sched2 sinks it past
 *     the `lh` (dump line: ";; launching 62 before 53 with no stalls at
 *     T-15").  That removes the nop AND makes the reload's pseudo live across
 *     the block move, so reload gives the movstrsi scratches $a1/$a2 instead
 *     of target's $a0/$a1.  ONE cause, both symptoms.
 *   - The fix is a true memory dependence block-move -> lh.  sched.c:817
 *     true_dependence() -> memrefs_conflict_p(): SIZE_FOR_MODE(BLKmode) == 0,
 *     and for two constant addresses the conflict test ends at
 *     `rtx_equal_for_memref_p (x, y) && (xsize == 0 || ...)` — so a BLKmode
 *     store conflicts with a load ONLY when they share a base symbol.  Two
 *     distinct symbol_refs can never conflict, at any record shape.
 *   - Declaring the block at 0x80101E60 as ONE record makes the store
 *     (base+0xC, BLKmode) conflict with the load (base+0), the dependence
 *     appears, sched2 is forced into target's order, the nop appears, and the
 *     scratches fall into $a0/$a1 with no register work at all.
 *
 * Independent justification for the record (NOT "it schedules well"):
 *   - this function already hands a pointer to the block's interior to another
 *     function: `base = (u8 *)&D_80101E62 - 0xA` (== 0x80101E58) is passed to
 *     tslPolyF4Init as a struct pointer;
 *   - code6cac_b2_post.c:277 passes `&D_80101E6C` to cdrom_BcdToFrames /
 *     cdrom_FramesToBcd as an 8-byte buffer, i.e. `pair` is one object;
 *   - target's own call passes `&SpecialCam + i*8`, so SpecialCam is an array
 *     of that same 8-byte record.
 *
 * ===== WHY THE LOCALS ARE IN THIS ORDER (also measured, not cosmetic) =====
 * The last 2 scored *instructions* (floor 4 -> 2) were an adjacent-pair
 * reorder: target emits `sll v0,a0,16 / lui+addiu v1,SpecialCam / sra v0,v0,13`
 * — the table base is materialised BETWEEN the sign-extend's sll and its sra.
 * sched.c rank_for_schedule() breaks priority ties on INSN_LUID, i.e. on
 * original RTL order, so the C statement order decides.  Splitting the index
 * read (`idx = rec->unk00;` -> sll16/sra16) from the scaling (`idx * 8` ->
 * sll3, which combine fuses with the sra16 into sra13 AT THE sll3's position)
 * and putting `cam = &SpecialCam;` between them yields exactly sll, sym, sra.
 * Measured: symbol-first (`u8 *cam = &SpecialCam;` as the first initialiser)
 * gives sym,sll,sra -> score 4; no local at all gives sll,sra,sym -> score 4;
 * only the split-with-symbol-between gives target's order -> score 2.
 *
 * ===== CLEANUP STILL OWED BEFORE A COMPLETION CLAIM =====
 * `(ReplayCamRec *)&D_80101E60` is a typed re-view of a global (pointer-alias
 * family).  It is a PROBE SPELLING.  The committable form is to declare the
 * record ONCE in include/code6cac.h and replace the per-word externs
 * (D_80101E60/E62/E64/E68/E6A/E6C/E70/E74) with member accesses.  That is an
 * owner-scope refactor: those symbols are referenced from other files, and
 * unifying them into one object changes their mutual aliasing, so it must be
 * done with a full-build SHA1 check.  It does NOT change this function's
 * score — the addend spelling is identical either way.
 */

extern void tslPolyF4Init(s32, u8 *, s32);
typedef struct {
    s32 a;
    s32 b;
} CamPair;
typedef struct {
    s16 unk00; /* 0x80101E60 */
    s16 unk02; /* 0x80101E62 */
    s16 unk04; /* 0x80101E64 */
    s16 unk06;
    s16 unk08; /* 0x80101E68 */
    s16 unk0A; /* 0x80101E6A */
    CamPair pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14; /* 0x80101E74 */
} ReplayCamRec;
s32 func_80036FD4(s32 arg0, s32 arg1) {
    s16 *s0 = &D_80101E62;

    if (*s0 != 0) {
        return 0;
    }

    {
        ReplayCamRec *rec = (ReplayCamRec *)&D_80101E60;
        s32 idx;
        u8 *cam;
        CamPair *entry;

        rec->unk00 = arg0;
        idx = rec->unk00;
        cam = &SpecialCam;
        entry = (CamPair *)(cam + idx * 8);
        rec->pair = *entry;
    }

    {
        extern u8 SpecialCam;
        D_80101E74 = cdrom_BcdToFrames((s32)(&SpecialCam + D_80101E60 * 8)) + (*(u32 *)((u8 *)&D_8008EC38 + (D_80101E60 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        D_80101E94 = 0;
        D_80101E90 = 5;
    } else {
        u8 *base = (u8 *)s0 - 0xA;
        D_80101E94 = 1;
        *base = 1;
        D_80101E59 = arg1;
        tslPolyF4Init(0xD, base, 0);
        D_80101E90 = 0xC8;
    }

    D_80101E64 = 0;
    D_80101E68 = 0;
    D_80101E6A = 0;
    D_80101E62 = 0x10;

    return 1;
}
