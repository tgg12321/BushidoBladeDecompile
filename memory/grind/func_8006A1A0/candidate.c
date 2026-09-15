/* func_8006A1A0 — session 1 candidate, sandbox --disable all = 0 (139/139 insns).
 * Requires the file-scope typedef S_69F80 + externs already on main above
 * func_80069F80 in src/text1b.c (shared with that sibling). */
void func_8006A1A0(s32 *arg0, s32 arg1) {
    /* FAKE: oversized locals object - `s` is the LIVE descriptor whose address is
       passed to func_80073728 and func_8007352C (addiu $a0,$sp,0x18 at three
       sites: 8006A2B8, 8006A30C, 8006A350); the S_69F80 tail sp44/sp48/sp4C/sp50
       is this call site's UNWRITTEN PADDING.  Nothing in this function or its
       callees' asm reads it; it is NOT asserted to be a field of a shared
       descriptor type.
       mechanism: mips.c compute_frame_size / get_frame_size -
       frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs).  Frame-math proof
       from the TARGET BYTES ALONE (asm/funcs/func_8006A1A0.s): target frame is
       0x70 with six callee-saves ($s0-$s4,$ra at sp+0x58..0x6C => ALIGN8(24) =
       0x18) and a 0x18 outgoing-args area (the 5-arg SetDrawMode call stores at
       sp+0x10), so the locals region is 0x70 - 0x18 - 0x18 = 0x40 = 64 bytes,
       while the only bytes ever read, written or addressed in that region are
       sp+0x18..0x43 (the 0x2C-byte descriptor; sp+0x44..0x57 is untouched
       anywhere in the target).  The fully-written form (a 0x2C descriptor) gives
       ALIGN8(44)+0x18+0x18 = 0x60 != 0x70 (measured this function: score 14, all
       14 diffs are the prologue/epilogue frame and save-slot offsets shifted by
       0x10 - memory/grind/func_8006A1A0/hypotheses.md s1 H2), so no fully-written
       locals set can produce the target frame.
       n.b.! ALIGN8 makes the declared descriptor size recoverable only as a
       RANGE: 0x39..0x40 bytes all give vars = 0x40; 0x3C is the smallest whole-word
       (s32-member) size in that range and is the one declared (S_69F80, shared
       with the sibling func_80069F80 whose frame equation is identical).
       Family: .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS carve-out
       (owner ruling 2026-07-13); prong 2 is satisfied by extending the LIVE object
       rather than adding a dead pad local.  In-tree precedent: func_80069F80
       (this TU, Judge PASS docs/grind/decisions.md:27053) and func_8006DD94
       (this TU, Judge PASS docs/grind/decisions.md:26632).
       Lever-exhaustion: memory/grind/func_8006A1A0/hypotheses.md - s1 H2 (0x2C
       form scores 14, every save/restore offset wrong; no other residual). */
    S_69F80 s;
    s32 *ptr;
    s32 x0;
    s32 c;
    s32 p1;
    s32 p2;
    s32 tbl;

    if (arg1 & 1) {
        ptr = *(s32 **)(arg0[1] + 0x1C);
        s.sp18 = ptr[2];
        if (arg1 & 2) {
            s.sp30 = -1;
            s.sp40 = 1;
        } else {
            s.sp30 = 0x4E;
            s.sp40 = 0;
        }
        x0 = s.sp30;
        if (!(((s32 *)D_800A3524)[8] & 8)) {
            if (arg1 & 2) {
                s.sp30 = x0 + *(s16 *)(D_800A34FC + 0xC);
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.sp43 = (s8)c;
                s.sp42 = (s8)c;
                s.sp41 = (s8)c;
            }
            s.sp34 = 0;
            s.sp3C = 0x100;
            s.sp38 = 0x100;
        } else {
            s.sp30 = x0 + 0x32;
            s.sp43 = 0x70;
            s.sp42 = 0x70;
            s.sp41 = 0x70;
            s.sp3C = 0x80;
            s.sp38 = 0x80;
            s.sp34 = 0xA;
        }
        s.sp28 = 0;
        s.sp2C = 2;
        tbl = s.sp18 + 0xC;
        s.sp1C = tbl;
        s.sp24 = arg0[2];
        arg0[2] = func_80073728((s32)&s, 0);
        s.sp34 = 0;
        s.sp30 = x0;
        s.sp40 = 0;
        p1 = ptr[3];
        tbl = p1 + 0xC;
        s.sp18 = p1;
        s.sp1C = tbl;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        if (arg1 & 2) {
            s.sp2C = 2;
            s.sp28 = 0;
            p2 = ptr[6];
            s.sp30 = 0;
            s.sp34 = 0;
            s.sp40 = 1;
            tbl = p2 + 0xC;
            s.sp18 = p2;
            s.sp1C = tbl;
            s.sp20 = arg0[5];
            arg0[5] = func_8007352C((s32)&s);
        }
        SetDrawMode(arg0[7], 1, 0, func_8006E480(s.sp18, 0), 0);
        AddPrim(D_800A374C + 8, arg0[7]);
        arg0[7] += 0xC;
    }
}
