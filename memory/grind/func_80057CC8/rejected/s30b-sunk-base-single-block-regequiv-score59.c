/* REJECTED s30b 2026-08-27 (forensics). The base read (and the centre cx/cy reads
 * that depend on it) SUNK below both index if-blocks, so every reference to the
 * base pseudo lives in one basic block, plus const-qualification. MEASURED:
 * sandbox --disable all = 59, target_insns 111, build_insns 104. This is the form
 * that PROVES the update_equiv_regs gates: with the references confined to one
 * block AND the MEM marked /u, the .lreg dump now carries exactly one
 * `REG_EQUIV (mem/u:SI (plus:SI (reg/v:SI 72) (const_int 4)))` note (formB and the
 * const-only variant carry zero). It also proves the note is NOT sufficient: global
 * alloc still hands the base pseudo a callee-save ($16), so reload never falls back
 * to the equivalent MEM and only ONE `lw ...,0x4(...)` is emitted.
 * SECOND, INDEPENDENT FINDING: 104 < 111. The s29 closing claim that the
 * ban-compliant regime is pinned at 112 and therefore "structurally short" is FALSE
 * as stated -- ban-compliant single-read forms span at least 104..112 instructions
 * depending only on WHERE the single base read is placed. The barrier is shape, not
 * instruction budget. (Non-const twin measured identically: 59 / 104, so the const
 * qualifier contributes nothing to codegen here; sinking alone does.) */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    s32 ni;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    table = *(s16 *const *)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ni = (s16) next_idx;
    ang_next = ratan2(table[ni * 2] - (s16) cx, table[ni * 2 + 1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
