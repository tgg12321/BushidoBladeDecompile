/* func_80021A98 — SANDBOX-0 form (session 2, structural). 158/158, distance 0
 * measured this session with these exact edits in place in src/code6cac.c.
 * ALSO REQUIRED: include/code6cac.h:442 prototype changed to
 *   extern void func_80021A98(s32, s32, s32);   (arg1 u8* -> s32)
 * OPEN CLASSIFICATION QUESTION (why session 2 returned ruling-request instead
 * of candidate-ready): the closing lever spells the two second-table sums with
 * INCONSISTENT operand order between the arms —
 *   if-arm : v0 = *(u16*)(v0+2) + D_80102768;            (offset + base)
 *   else   : v0 = *(s32*)(...+arg0) + *(u16*)(v0+2);     (base + offset)
 * Both-base-first measures 6 (v1 lands $5); both-offset-first measures 2
 * (both addus emit operand-swapped); ONLY the mixed arrangement measures 0.
 * The derivation referenced global.c set_preference/expand_preferences
 * (first-PLUS-operand hard-reg preference feeding the v1/v0 web), which is a
 * GCC-internals justification for an operand-order choice in a commutative
 * `+` — possibly inside the or-tree-shape-shift forbidden family, possibly
 * ordinary expression spelling (2-operand sum; both orders appear all over
 * this file).  RESOLVED: Judge ruled PASS 2026-08-07 22:21
 * (docs/grind/decisions.md:4073-4075, commit 9b326242) — ordinary expression
 * spelling, outside or-tree-shape-shift (scoped to 3+-operand chains), no FAKE
 * annotation required.  Session 3 re-applied this body + the header prototype
 * to src and re-measured sandbox 0 (158/158); self_vet.md written;
 * candidate-ready returned.
 * Levers landed this session (all measured, in order):
 *   +  arg0 = a3*5*4 reuse (dead param, $4 home-pref)      20 -> 15
 *   +  s32 v1 with v1 <<= 2 per arm (in-place shift)       15 -> 12 (stacked)
 *   +  split-init arg0 chain (arg0=a3<<2; arg0+=a3; ...)   12 -> 10
 *   +  arg1 param s32 + reuse for the a1_val byte          10 -> 6
 *   +  mixed operand order on the second-table sums         6 -> 0
 * Apply this body over func_80021A98 in src/code6cac.c (+ the header decl)
 * to resume. */
void func_80021A98(s32 arg0, s32 arg1, s32 arg2) {
    u8 *s0 = ((u8 *) (&D_80101EC8)) + (arg0 * 1100);
    s32 a3;
    if ((*((s16 *) (s0 + 0x4C))) != 0) {
        a3 = *((s16 *) ((*((s32 *) s0)) + 0x4A));
    } else {
        a3 = *((s16 *) (s0 + 0x4A));
    }
    *((s16 *) (s0 + 0x4C)) = 0;
    *((s32 *) (s0 + 0x50)) = arg1;
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
            arg1 = *((u8 *) (v0_50 + 6));
            *((s16 *) (s0 + 0x6C)) = old_kind;
            {
                s32 v1_58 = *((s32 *) (s0 + 0x58));
                s32 li1 = 1;
                *((s16 *) (s0 + 0x42)) = 0;
                *((s16 *) (s0 + 0x7A)) = li1;
                *((s32 *) (s0 + 0x7C)) = 0;
                *((s16 *) (s0 + 0x46)) = 0;
                *((s16 *) (s0 + 0x40)) = arg1;
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
