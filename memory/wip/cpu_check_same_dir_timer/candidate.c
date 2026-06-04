/*
 * cpu_check_same_dir_timer — cheat-removed candidate at masked floor 1
 * (2 actual byte diffs: bnez/beqz + j-target pair, branch-sense flip)
 *
 * Apply to src/code6cac_b.c replacing the existing function body, then
 *   & tools/eng.ps1 sandbox cpu_check_same_dir_timer --disable all
 * → expect score 1 (=2 masked-asm diffs: insn 48 branch sense + insn 50
 * jump-target swap; semantically equivalent but bytes differ).
 *
 * Cheats removed from HEAD:
 *   1. `register s32 a1val asm("a1");` — pin (cheat-asm)
 *   2. `register s32 *arg0 asm("a0") = base; asm("" : "=r"(arg0) : "0"(arg0));` — pin + barrier
 *   3. `asm volatile("");` — scheduling barrier
 *
 * The branch-sense flip is the residual ALL pure-C variants tested
 * converged on. See notes.md for the lever ledger.
 */
void cpu_check_same_dir_timer(s32 *base) {
    u8 *s0;
    s32 a1val;
    s32 *p;

    p = *(s32 **)((u8 *)base + 0x58);
    a1val = *(u8 *)((u8 *)p + 4);
    if (a1val == 0) goto done;
    s0 = (u8 *)p + 5;

loop:
    a1val = a1val & 0xFF;
    if (a1val == 0xFF) {
        u8 b0 = s0[0];
        u8 b1 = s0[1];
        u8 b2 = s0[2];
        u8 b3 = s0[3];
        s32 packed;
        s16 shift;
        s32 mask;

        packed = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
        shift = *(s16 *)((u8 *)base + 0xA);
        mask = 1 << shift;
        if ((packed & mask) != 0) {
            s0 += 4;
        } else {
            s0 += 6;
        }
        goto next;
    }

    if ((u32)a1val < 0x80) {
        u8 val = s0[0];
        s16 dir = *(s16 *)((u8 *)base + 0x40);
        s0 += 1;
        if ((val & 0xFF) == dir) {
            func_80032C50((s32)base, a1val - 1);
            goto next;
        }
        if (dir < val) {
            goto done;
        }
        goto next;
    }

    s0 += 1;

next:
    a1val = *s0;
    s0 += 1;
    if (a1val != 0) goto loop;

done:
    return;
}
