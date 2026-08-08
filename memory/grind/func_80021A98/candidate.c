/* func_80021A98 — SANDBOX-0 form, SRC-ONLY (session 8, structural).
 * 158/158, distance 0 measured 2026-08-07 with these exact edits in place in
 * src/code6cac.c and include/code6cac.h UNTOUCHED (arg1 keeps its
 * header-declared type u8 * — the prototype at include/code6cac.h:442 stays
 * `extern void func_80021A98(s32, u8 *, s32);`).
 *
 * WHY THIS SUPERSEDES THE s2-s7 BANKED FORM: the driver's Judge constraint
 * (banked s7->s8) rules the include/code6cac.h:442 prototype edit
 * (u8* -> s32) OUT OF SCOPE — candidates may only edit src/code6cac.c. The
 * fix: keep arg1 as u8* and spell the P11 arg1-reuse lever with the two
 * casts the C type system requires for integer round-trip through a
 * pointer-typed variable:
 *     arg1 = (u8 *) *((u8 *) (v0_50 + 6));      // int-to-pointer, required
 *     *((s16 *) (s0 + 0x40)) = (s32) arg1;      // pointer-to-int, required
 * Both casts are 32-bit-to-32-bit (no width change, NOT F2); the RTL is
 * identical to the s32-typed spelling (same SImode pseudo, same $5
 * copy-preference from the prologue copy), so the register-allocation
 * mechanism is unchanged and sandbox measures 0 directly.
 *
 * Levers (all measured, sessions 2+8):
 *   +  arg0 = a3*5*4 reuse via split-init chain (dead param, $4 home-pref)
 *   +  s32 v1 with v1 <<= 2 per arm (in-place shift)
 *   +  arg1 (u8*, type unchanged) reused for the a1_val byte via casts
 *   +  byte-offset table casts *(s32*)((u8*)&D_801027B4 + arg0)
 *   +  mixed second-sum operand order (Judge PASS
 *      docs/grind/decisions.md:4073-4075, commit 9b326242)
 * Apply this body over func_80021A98 in src/code6cac.c to resume — NO
 * header edit needed or allowed. */
void func_80021A98(s32 arg0, u8 *arg1, s32 arg2) {
    u8 *s0 = ((u8 *) (&D_80101EC8)) + (arg0 * 1100);
    s32 a3;
    if ((*((s16 *) (s0 + 0x4C))) != 0) {
        a3 = *((s16 *) ((*((s32 *) s0)) + 0x4A));
    } else {
        a3 = *((s16 *) (s0 + 0x4A));
    }
    *((s16 *) (s0 + 0x4C)) = 0;
    *((s32 *) (s0 + 0x50)) = (s32) arg1;
    {
        s32 v1 = *((u16 *) (arg1 + 4));
        *((s16 *) (s0 + 0x5C)) = v1;
        if (arg2 != 0) {
            s32 v0;
            v1 <<= 2;
            v0 = D_80102764 + v1;
            *((s32 *) (s0 + 0x54)) = v0;
            v0 = *((u16 *) (v0 + 2)) + D_80102768;
            *((s32 *) (s0 + 0x58)) = v0;
        } else {
            s32 v0;
            arg0 = a3 << 2;
            arg0 += a3;
            arg0 <<= 2;
            v1 <<= 2;
            v0 = *(s32 *)((u8 *)&D_801027B4 + arg0) + v1;
            *((s32 *) (s0 + 0x54)) = v0;
            v0 = *(s32 *)((u8 *)&D_801027B8 + arg0) + *((u16 *) (v0 + 2));
            *((s32 *) (s0 + 0x58)) = v0;
        }
    }
    {
        s32 v0_50 = *((s32 *) (s0 + 0x50));
        u16 old_kind = *((u16 *) (s0 + 0x6A));
        s32 a0_58 = *((s32 *) (s0 + 0x58));
        *((u8 *) (s0 + 0x60)) = (u8) arg2;
        do { } while (0);
        *((u8 *) (s0 + 0x61)) = (u8) a3;
        {
            arg1 = (u8 *) *((u8 *) (v0_50 + 6));
            *((s16 *) (s0 + 0x6C)) = old_kind;
            {
                s32 v1_58 = *((s32 *) (s0 + 0x58));
                s32 li1 = 1;
                *((s16 *) (s0 + 0x42)) = 0;
                *((s16 *) (s0 + 0x7A)) = li1;
                *((s32 *) (s0 + 0x7C)) = 0;
                *((s16 *) (s0 + 0x46)) = 0;
                *((s16 *) (s0 + 0x40)) = (s32) arg1;
                *((s16 *) (s0 + 0x6A)) = *((u8 *) a0_58);
                *((s16 *) (s0 + 0x6E)) = *((u8 *) (v1_58 + 2));
            }
        }
        {
            s32 v0_50b = *((s32 *) (s0 + 0x50));
            s32 kind = *((u16 *) (s0 + 0x6A));
            *((s16 *) (s0 + 0x70)) = (*((u8 *) (v0_50b + 9))) & 3;
            {
                s32 a0_flag = 0;
                if ((((kind == 2) || (kind == 0x1B)) || (kind == 0x28)) || (kind == 0x26)) {
                    a0_flag = 1;
                }
                *((u8 *) (s0 + 0xAD)) = a0_flag;
            }
            func_800324D0(s0);
            {
                s32 kind2 = *((u16 *) (s0 + 0x6A));
                s32 v1k = kind2 & 0xFFFF;
                if (v1k == 9) {
                    *((s16 *) (s0 + 0x152)) = 1;
                    *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1CA));
                    goto end;
                }
                if (v1k == 2) {
                    if ((*((s16 *) (s0 + 0x152))) != 0) goto clear_152;
                    if ((*((s16 *) (s0 + 0x6C))) == 0x13) goto clear_152;
                    *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1D8));
                    goto clear_152;
                }
                if (((u32) (kind2 - 0x19)) >= 2U) goto not_in_range;
                if ((*((s16 *) (s0 + 0x152))) == 0) goto set_154;
                if (v1k != 0x19) goto set_152;
                if ((*((s16 *) (s0 + 0x6C))) != v1k) goto set_152;
                goto set_154;
                set_154:
                *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1D8));
                goto set_152;
                not_in_range:
                if (v1k != 0x11) goto clear_152;
                set_152:
                *((s16 *) (s0 + 0x152)) = 1;
                goto end;
                clear_152:
                *((s16 *) (s0 + 0x152)) = 0;
            }
            end:
            {
                u16 v1f = *((u16 *) (s0 + 0x6A));
                if ((((v1f == 2) || (v1f == 0x1B)) || (v1f == 0x28)) || (v1f == 0x26)) {
                    *((u8 *) (s0 + 0xAF)) = ((*((u8 *) (s0 + 0xB0))) & 0xF) != 5;
                }
            }
        }
    }
}
