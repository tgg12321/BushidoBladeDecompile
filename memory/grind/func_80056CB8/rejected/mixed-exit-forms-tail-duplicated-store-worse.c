/* REJECTED s34 (rederive modality, 2026-09-16).
 * Mixed-exit-forms / duplicated-statement-into-arms rewrite of the flags==3
 * / flags==4 tail, matching target's actual control-flow shape (confirmed
 * via a direct read of asm/funcs/func_80056CB8.s lines 144-197: target
 * recomputes the store address `addu $v0,$s7,$s6` fresh at FIVE separate
 * exit points -- .L80056F0C, .L80056F54, .L80056F74, .L80056F8C, .L80056F94
 * -- rather than falling through to one shared store after the if/else-if,
 * and m2c's independent reconstruction (tmp/grind/func_80056CB8/s12/m2c_out.c)
 * shows the same shape: `var_v0 = arg0 + var_s6;` repeated at each of
 * goto block_19 / goto block_20 / the natural fallthroughs).
 *
 * Applied to the s22-s33-banked 38/204 chassis (func_80053614 s32-return
 * fix + header externs unchanged), duplicating the real store statement
 * `*(s8 *)(arg0 + 0x444 + i) = (s8)flags;` into each of the five exit arms
 * (each followed by `goto next;` to the loop-bottom label) instead of
 * falling through to one merged store after the if/else-if chain -- the
 * SOTN-sanctioned duplicated-statement-into-arms family
 * (.claude/rules/duplicated-statement-into-arms.md, owner ruling
 * 2026-07-01).
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 38 -> 72/204,
 * build_insns 198 -> 200 (WORSE, +2 real instructions -- duplicating the
 * store statement into 5 arms cost more in code size / register pressure
 * around the store address computation than it gained by matching target's
 * control-flow shape at the SOURCE level; whatever GCC-internal reason
 * target's compiler emitted 5 separate `addu $v0,$s7,$s6` recomputations,
 * writing the C source with 5 duplicated store statements does not
 * reproduce that RTL shape on THIS chassis).
 *
 * Reverted immediately via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s34 chassis
 * (s22-s33-banked 38/204 body + func_80053614 s32-return fix + header
 * externs, tail rewritten with the store statement duplicated into 5 exit
 * arms with `goto next;`, no other change, no FAKE constructs present).
 *
 * Below: the modified function body only (header/prologue unchanged from
 * candidate.c).
 */

        flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;
        if (flags == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                flags = 0;
                *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
                goto next;
            }
        }
        if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        flags = 5;
                        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
                        goto next;
                    }
                    *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
                    goto next;
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
                        goto next;
                    }
                    *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
                    goto next;
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
    next:;
    }
}
