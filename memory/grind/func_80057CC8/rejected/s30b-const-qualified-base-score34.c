/* REJECTED s30b 2026-08-27 (forensics). formB with the base read const-qualified
 * (`*(s16 *const *)(arg0 + 4)`) to set RTX_UNCHANGING_P on the MEM. MEASURED:
 * sandbox --disable all = 34 (WORSE than formB's 30), target_insns 111,
 * build_insns 112. The const DOES take effect at RTL level (the .flow/.lreg dump
 * prints the load as `(mem/u:SI (plus:SI (reg/v:SI 72) (const_int 4)))`), which is
 * the gate at local-alloc.c:583 (`GET_CODE (insn) == CALL_INSN && ! RTX_UNCHANGING_P
 * (memref)` -> validate_equiv_mem returns 0). But NO REG_EQUIV note is produced for
 * the base pseudo (reg 89) in this function anyway, because the other gate --
 * `reg_basic_block[regno] >= 0` in update_equiv_regs -- fails: the base is
 * referenced in the entry block (centre vertex) and again in the tail block
 * (neighbour vertices). Banked so no future session re-derives const as a lever. */
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
    table = *(s16 *const *)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

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
