/* REJECTED (s10, rederive modality) — replacing the second switch
 * (`switch ((u32)D_800A34F8 >> 10 & 7) { case 0..5: ... }`) with a semantically
 * identical if-else chain (`s32 dk = ...; if (dk==0) {...} else if (dk==1) {...} ...`).
 *
 * Hypothesis: a structurally different dispatch shape might reach the target's
 * jtbl_80015988 dispatch bytes through a different GCC codegen path, sidestepping
 * the cross-TU rodata-symbol residual documented in candidate.c / hypotheses.md H13/H16/H18.
 *
 * Measured: sandbox --disable all score REGRESSED 2 -> 16, insn count 200 -> 206.
 * GCC 2.7.2's stmt.c expand_end_case only synthesizes an ADDR_VEC jump table for a
 * real `switch`; an if-else chain compiles to a linear beq/bne compare chain and never
 * emits the jump table at all. The target itself dispatches through the identical kind
 * of ADDR_VEC jump table (asm/funcs/func_8006B578.s:83-88), so this rederivation moves
 * AWAY from target structure, not toward it. KILLED (instance) — see hypotheses.md H18.
 *
 * Full body for reference (apply candidate.c and substitute only the middle block below
 * to reproduce this exact regression):
 */
    {
        s32 dk = (u32)D_800A34F8 >> 10 & 7;
        if (dk == 0) {
            if ((ret & 0xFF) != 0) {
                s32 *p = (s32 *)D_800A3524;
                u32 f = (u32)p[8];
                u32 a3 = f & ~1u;
                u32 bit = f & 1;
                bit ^= 1;
                a3 |= bit;
                p[8] = (s32)a3;
                func_8005C650(0, 0x7F, 0x7F);
            }
            goto shared_400040;
        } else if (dk == 1) {
            if ((ret & 0xFF) != 0) {
                s32 *p = (s32 *)D_800A3524;
                u32 f = (u32)p[8];
                u32 a3 = f & ~2u;
                u32 bit = (f >> 1) & 1;
                bit ^= 1;
                bit <<= 1;
                a3 |= bit;
                p[8] = (s32)a3;
                func_8005C650(0, 0x7F, 0x7F);
            }
            goto shared_400040;
        } else if (dk == 2) {
            if ((ret & 0xFF) != 0) {
                s32 *p = (s32 *)D_800A3524;
                u32 f = (u32)p[8];
                u32 a3 = f & ~4u;
                u32 bit = (f >> 2) & 1;
                bit ^= 1;
                bit <<= 2;
                a3 |= bit;
                p[8] = (s32)a3;
                func_8005C650(0, 0x7F, 0x7F);
            }
        shared_400040:
            if (*(u32 *)arg1 & 0x400040) {
                func_8005C650(1, 0x7F, 0x7F);
                {
                    u32 f2 = D_800A34F8;
                    u32 m2 = f2 & ~0x1C00u;
                    s32 c2 = ((f2 >> 10) & 7) + 1;
                    D_800A34F8 = m2 | ((c2 & 7) << 10);
                }
            }
            goto tail;
        } else if (dk == 3) {
            if (*(u32 *)arg1 & 0x400040) {
                func_8005C650(1, 0x7F, 0x7F);
                D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
                var_s2 = 2;
            }
            goto tail;
        } else if (dk == 4) {
            if (*(u32 *)arg1 & 0x400040) {
                func_8005C650(1, 0x7F, 0x7F);
                var_s2 = 3;
            }
            goto tail;
        } else if (dk == 5) {
            if (*(u32 *)arg1 & 0x400040) {
                func_8005C650(1, 0x7F, 0x7F);
                var_s2 = 1;
            }
            goto tail;
        }
    }
