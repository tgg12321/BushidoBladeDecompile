/* func_8006CCC8 — src/text1b.c — session s4 (permuter modality, structural work)
 * Sandbox --disable all floor at end of s4: 39 (target_insns=189, build_insns=184).
 * DOWN FROM 77 (s2/s3 floor) via three structural fixes this session, all derived
 * by hand-reading asm/funcs/func_8006CCC8.s directly (frontier item 1 from s3 —
 * the WSL engine.score.normalized_insns diff — plus a full manual disassembly
 * read) rather than by further hand-waving over "matched by eye."
 *
 * This candidate IS applied to src/text1b.c as of end of s4 (2026-09-16) — it is
 * the resident WORKING-TREE body (never committed; asm-until-matched keeps main
 * as INCLUDE_ASM until sandbox reaches 0). NOTE: at s4 dispatch, src/text1b.c
 * STILL carried INCLUDE_ASM (a repeat of the s1/s2/s3 stale-HEAD pattern flagged
 * by this session's CONSISTENCY WARNING) — the s3 candidate had never actually
 * been re-applied to HEAD. Re-applied verbatim at s4 start, re-verified score 77
 * (matches ledger), then made the changes below.
 *
 * s4 changes from the s3 body (each measured independently, same chassis):
 *
 * 1. NAMED CONSTANT `nib` for the repeated `0xF` mask literal (ordinary C,
 *    named-intermediate family — [[no-new-park-categories]] SOTN-accepted
 *    "named-intermediate declaration order" entry; fresh local, once-written,
 *    read twice with a real consumed value: `nib << fade` at both the +0x1A and
 *    +0x1D call sites). Declared and initialized in the SAME relative position
 *    target's own `li $s6, 0xF` occupies (right after `i = 0`, before `fade = 0`
 *    / `shift = 0` / `mask = 0`) — GCC's loop-invariant motion hoists the shared
 *    constant to the same LUID slot as target when the source names it there.
 *    Also re-split `for (i=0, shift=0, mask=0; ...)` into separate leading
 *    statements so `i`, `nib`, `fade`, `shift`, `mask` initialize in that exact
 *    order (target: s1, s6, s5, s2, s3).
 *
 * 2. `field` NAMED LOCAL for the single-read `*(s16 *)(mask + (u8*)D_800A34FC +
 *    0x28)` used by all three field28 compares (==3 / <4 / ==4). Target reads it
 *    ONCE (`lh $v1, 0x28($v0)`) and reuses that register for all three
 *    compares — matching a single C read into a local, reused verbatim,
 *    instead of re-reading the same expression 3x per compare (which is what
 *    the s2/s3 body did, and which is legal C but does not by itself force the
 *    single-read reuse the target shows once combined with #3 below).
 *
 * 3. IF-CHAIN RESTRUCTURE to a nested `!=4 { !=3 { <4 { default } } else {
 *    case3 } } else { case4 }` shape instead of the flat `if(==3){}else
 *    if(<4){}else if(==4){}` shape. This was derived directly from the target's
 *    branch senses: every one of the three field28 compares in
 *    asm/funcs/func_8006CCC8.s (`beq $v1,$v0,.L8006CE80` for ==3-equivalent
 *    branch-when-true, etc.) branches FORWARD (jumps AWAY) when its case is
 *    reached, with the "closest" block (nearest label, .L8006CE44) being the
 *    field<4 default case and the other two case bodies pushed further down —
 *    in target's OWN file order: default, then the +0x1A body (field==3), then
 *    the +0x1D body (field==4). A flat if/else-if chain (branch-when-FALSE to
 *    next test, fallthrough on TRUE) does not reproduce this; the closing form
 *    is the nested if/else shape checked in above, which reproduces it exactly
 *    (verified via the masked-insn diff: the block ORDER now matches target
 *    for all three regions, and the branch senses at every field28 compare now
 *    match target's `beq`/`bne`/`bnez` exactly). Ordinary nested if/else — no
 *    dead code, no coercion; every arm's condition is a real, necessary guard
 *    (field<4 still needed to exclude field>=5, which target skips entirely).
 *
 * MEASURED DEAD END (same session): combining the innermost `field < 4` and
 * `field >= 0` tests into one `&&` expression (`if (field < 4 && field >= 0 &&
 * (*arg1 & ...))`) INSTEAD of the nested `if (field<4) { if (field>=0 && ...) }`
 * form regressed the floor 39 -> 65 (measured, this session). Kept the nested
 * form. See rejected/combined-lt4-ge0-condition.c.
 *
 * KNOWN REMAINING GAP (floor 39): the masked-insn diff (WSL
 * engine.score.normalized_insns via tmp/grind/func_8006CCC8/s4/diff_probe.py)
 * now isolates the residual to exactly two shapes, each appearing twice
 * (symmetrically, once per field28==3 and once per field28==4 arm):
 *
 *   (a) THREE `addu` operand-order flips (target `addu $a0,$s3,$v0` /
 *       `addu $v1,$s3,$v0` / `addu $v0,$s3,$v0` — mask-register-first — vs our
 *       `addu $a0,$v0,$s3` etc — base-pointer-first) at the three
 *       `mask + (u8*)D_800A34FC + 0x28` address computations (the two
 *       increment/decrement branches' first use, plus the fresh `field` read).
 *       MEASURED this session: rewriting the C-level pointer-arithmetic operand
 *       order (`mask + (u8*)D_800A34FC + 0x28` instead of
 *       `(u8*)D_800A34FC + mask + 0x28`) did NOT flip the emitted `addu` operand
 *       order — GCC's tree-level commutative-operand canonicalization
 *       (fold-const.c) re-sorts pointer-plus-int operands independent of
 *       source spelling, so this is not a source-level lever in this shape.
 *       Untried: forcing association via an intermediate pointer local
 *       (`u8 *base = (u8*)D_800A34FC; ... base + mask ...` vs `mask + base`)
 *       — same risk of the same canonicalization, not yet measured.
 *
 *   (b) A register-allocation difference INSIDE the `for (j = 0; j < 3; j++)`
 *       record loop (identical in both the +0x1A and +0x1D arms): target
 *       computes `addu $v1, $v0, $a1` (record pointer) then `lbu $v0, N($v1)`
 *       (masked byte) then reuses `$v1` again for `lbu $v0, 0x17($v1)` (the
 *       byte17 read) and for the shift-mask `and $a0, $v0, $a2` — i.e. target
 *       keeps the mask-shift result in `$a2` (the SAME register that also
 *       holds the loop's shift-amount source, `sllv $a2, $s6, $s5`) and the
 *       record pointer in `$v1`. Our build allocates the record pointer to
 *       `$a1`, the mask-shift result to `$a3` (a DIFFERENT register from the
 *       loop-counter-adjacent computation), and re-reads `$v1`/re-lbu's
 *       `byte17` into a fresh register rather than reusing the same one target
 *       does. This is a classic local-alloc register-pressure/preference tie —
 *       NOT yet isolated to a specific C lever this session; candidate for
 *       directed permuter (this session's mandated modality) on the `for (j...)`
 *       loop body specifically, or an RA-solver pass
 *       (`tools/ra_solver`/`inverse_compose.py classify`) in a future solver
 *       session.
 *
 * Structure derived from m2c --valid-syntax over asm/funcs/func_8006CCC8.s
 * (s1 provenance) + a full manual read of the target disassembly this session
 * (all 207 lines, asm/funcs/func_8006CCC8.s). Field layout cross-checked
 * against sibling func_8006CBD4 (same TU, same D_800A34FC / D_800A3524 struct
 * idiom, unchanged since s1).
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 mask;
    s32 nib;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 t;
    s32 masked;
    u8 byte17;
    s16 field;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    fade = 0;
    shift = 0;
    mask = 0;
    for (; i < 2; i++, shift += 0x10, mask += 2) {
        t = arg2;
        lim = ((t >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) <= 0) {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = lim;
            } else {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = (s16)(*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) >= lim) {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = 0;
            } else {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = (s16)(*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) + 1);
            }
        }

        field = *(s16 *)(mask + (u8 *)D_800A34FC + 0x28);
        if (field != 4) {
            if (field != 3) {
                if (field < 4) {
                    if (field >= 0 && (*arg1 & (0xF0 << shift))) {
                        func_8005C650(0, 0x7F, 0x7F);
                        t = i;
                        func_8006CBD4(t, *arg1);
                    }
                }
            } else {
                if (*arg1 & (0x40 << shift)) {
                    func_8005C650(1, 0x7F, 0x7F);
                    for (j = 0; j < 3; j++) {
                        rec = (u8 *)D_800A3524 + j;
                        masked = *(rec + 0x1A) & (nib << fade);
                        byte17 = *(rec + 0x17);
                        if (i == 0) {
                            *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
                        } else {
                            *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
                        }
                    }
                }
            }
        } else {
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & (nib << fade);
                    byte17 = *(rec + 0x17);
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
                    } else {
                        *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
                    }
                }
            }
        }
        fade += 4;
    }
    return ret;
}
