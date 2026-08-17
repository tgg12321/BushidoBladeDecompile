/* CANDIDATE — func_80084A7C, grind session 3 (structural), 2026-08-17.
 *
 * HONEST SANDBOX DISTANCE 0 with NO unsanctioned construct anywhere in the
 * form (`sandbox func_80084A7C --disable all`: score 0, target_insns 145,
 * build_insns 145, 11 regfix rules dropped, 79 cheat-asm lines stripped).
 * The s1 normalized objdump diff shows the only token difference is the
 * unlinked sandbox object's unresolved D_80106F28 lui/addiu relocation pair.
 * Floor history: 26 (s1) -> 24 natural / 0-with-a-cheat (s2) -> 0 natural (s3).
 *
 * This SUPERSEDES session 2's candidate, which reached 0 only via the
 * arithmetic no-op `offset = -((s16)a1 * -0xB0)` and was refused by the Judge.
 * That negation is NOT in this form and is not needed.
 *
 * WHAT CHANGED vs the session-2 natural floor-24 form
 * (rejected/natural-offset-no-negation-floor24.c): the `s32 offset` local is
 * GONE.  The stride multiply `(s16)a1 * 0xB0` is written out at the `base`
 * computation and at each of the eight `+0x98` flag-word sites; cse shares the
 * single strength-reduced chain, so the emitted multiply is unchanged.
 *
 * WHY IT CLOSES (the RA mechanism, measured — see hypotheses.md H9/H10).  The
 * whole residual was one hard-reg decision: `offset` and `base_ptr` were two
 * global allocnos with equal `allocno_compare` priority (each ~10 refs over the
 * same span), so global.c's tiebreak fell to the allocno NUMBER and handed
 * `offset` $a1 (costing the failed-coalesce `move $a1,$a3`, the 146th
 * instruction) and swapped the $a2/$a3 pair against target.  Deleting the local
 * removes the tied allocno entirely: the multiply's value is a single-def
 * quantity whose def IS the chain's final `sll`, it lands in $a2 exactly as
 * target does, base_ptr keeps $a3, and no join copy exists to delete.
 *
 * The `shifted` + `addr` entry intermediates are retained from session 2 (H6):
 * they are the idiom of FOUR already-matched siblings in this same file
 * (src/main.c:304-305, 596-597, 615-616, 628-629) and they place the
 * D_80106F28 address load between `sll v0,a0,16` and `sra v0,v0,14`, target's
 * schedule slot.
 *
 * Self-vet: memory/grind/func_80084A7C/self_vet.md (no FAKE construct; the diff
 * is a net DELETION of one local plus the inlining of its real arithmetic).
 */
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void func_80084A7C(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    u8 *base = (u8 *)(*base_ptr + (s16)a1 * 0xB0);
    s32 val;
    u32 threshold;

    val = base[0x21] + 1;
    threshold = base[0x20];
    base[0x21] = val;

    if (threshold == 0) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
        }
        return;
    }

    if ((u8)val < threshold) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
            *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
            *(s32 *)(base + 8) = *(s32 *)(base + 4);
        }
        return;
    }

    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~1;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~2;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 0x200;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 4;
    base[0x14] = 0;

    if (*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) & 0x400) {
        *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
    } else {
        *(s32 *)(base + 8) = *(s32 *)(base + 4);
    }

    if (base[0x22] != 0xFF) {
        base[0x14] = 0;
        spu_ResetMotionEntry(base[0x22], base[0x23]);
        spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    }
    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    *(s32 *)(base + 0x90) = *(s16 *)(base + 0x54);
}
