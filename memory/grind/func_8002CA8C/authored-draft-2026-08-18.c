/* =====================================================================
 * func_8002CA8C  -  AUTHORING DRAFT (not a candidate; never commit as-is)
 * ---------------------------------------------------------------------
 * PROVENANCE
 *   target      : asm/funcs/func_8002CA8C.s  (179 insns, 0 cop2 ops)
 *   tree state  : src/code6cac_b.c:1020 - body is an EMPTY STUB; the real
 *                 179 instructions live in asmfix.txt as a whole-body blob
 *                 anchored on ^\.frame / ^\.end (ROBUST anchors, so partial
 *                 C is oracle-safe and committable).
 *   raw m2c     : tmp/authoring/func_8002CA8C.m2c.c
 *                 (tools/m2c/m2c.py --valid-syntax
 *                  --context include/m2c_context.h --target mipsel-gcc-c)
 *   scanner     : tier=LOW, 0/8 - no canonical-asm grant path. PURE C ONLY.
 *
 * SYMBOL / TYPE MAPPING (m2c name -> project spelling)
 *   arg0/arg1/arg2       -> a0 / a1 / a2, keeping the tree signature
 *                           void func_8002CA8C(u8 *a0, s32 a1, s32 a2)
 *   &D_800F5F68          -> extern u8 D_800F5F68;  (spelling already used at
 *                           src/code6cac.c:2121 as (u8 *)&D_800F5F68 + id*0x1B8)
 *   0x1F8002B8 base      -> u8 *scr = (u8 *)0x1F8002B8;  - the TU's accepted
 *                           scratchpad spelling (src/code6cac_b.c:1576)
 *   0x1F8000A8 indexed   -> *(s32 *)((u8 *)0x1F8000A8 + off)  - the TU's
 *                           accepted lui/addu/lw(%lo) spelling
 *                           (src/code6cac_b.c:1881-1893)
 *   func_8002D780/D320   -> already DEFINED in this TU with exactly the
 *                           signature this call site needs:
 *                             s32 f(s32 flag, u8 *obj, s32 *pos,
 *                                   s32 threshold, s32 r_sq)
 *                           (src/code6cac_b.c:1229 / :1027). Add forward
 *                           declarations above this function; do NOT invent
 *                           new prototypes with fresh types.
 *   M2C_FIELD(...,0x26C) -> *(s16 *)(a0 + 0x26C)
 *   var_s5 / var_s2      -> folded into ONE record base + literal field
 *                           offsets (see [S3] below)
 *
 * DERIVED LAYOUT (read off the asm, not guessed)
 *   id = *(s16 *)(a0 + 4)
 *   D_800F5F68 : per-id block of 0x1B8 bytes = 22 records x 0x14 bytes
 *                record+0x00 : s16  enable/valid flag   (lh 0($s5))
 *                record+0x0C : u16  radius              (lhu -6($s2), $s2=$s5+0x12)
 *                record+0x0E : u16  arg5 of call #1     (lhu -4($s2))
 *                record+0x10 : u16  arg4 of call #2     (lhu -2($s2))
 *                record+0x12 : u16  arg5 of call #2     (lhu  0($s2))
 *   0x1F8000A8 : per-id block of 0x108 bytes = 22 entries x 0xC bytes {s32 x,y,z}
 *   0x1F8002B8 : +0x78/0x7C/0x80 = min x/y/z ; +0x84/0x88/0x8C = max x/y/z
 *                +0xB4 = "pass" bitmask out ; +0xC4 = "second-hit" bitmask out
 *
 * SUSPICIOUS SPOTS / m2c ARTIFACTS (what the measuring session checks first)
 *   [S1] m2c invented four stack objects sp18/sp20/sp28/sp30 at an 8-BYTE
 *        stride. Those are NOT source-level locals - MIPS BIGGEST_ALIGNMENT is
 *        64 bits, so GCC 2.7.2 lays every spilled pseudo 8 bytes apart. They
 *        are the spills of a0, a1, a2 and the seenMask accumulator under
 *        register pressure. DO NOT model them as a struct (that is exactly the
 *        mistake baked into func_80056CB8's harness). Let pressure create them.
 *   [S2] m2c's "temp_a0_3 = ..., (cond != 0)" comma chain renders a
 *        short-circuit OR whose operands share a reload. Spelled below as a
 *        nested if/else that lands all six failures on one rejected = 1
 *        (GCC cross-jumps them into the single .L8002CBD8 block). Note the
 *        literal compare order in target: slt max, (c - r), i.e. the bound is
 *        the LEFT operand -> spell it "max < c - r", not "c - r > max"
 *        ([[compare-operand-order-register]]).
 *   [S3] TWO induction variables in target: $s7 = i*0xC and $s4 = base + i*0xC.
 *        They exist because the LOADS use the "constant + register" form
 *        (lui/addu/lw %lo) while the CALL ARG materializes the whole pointer
 *        (lui/ori, then addu $s7, then addu $fp). The draft preserves that by
 *        writing the two expressions differently - loads via
 *        (u8 *)0x1F8000A8 + off, the call arg via integer arithmetic
 *        i * 0xC + 0x1F8000A8 + base. If the residual shows a missing/extra
 *        IV, this is the knob. Per the func_800393C8 lesson, do NOT add
 *        hand-written walking pointers - that ADDS IVs.
 *   [S4] The .L8002CCD8 "beqz $s0" re-tests a value already known non-zero, and
 *        the a2==0 path jumps PAST it to .L8002CCE0. That asymmetry is a
 *        cross-jump / jump-thread artifact. The draft uses the flat spelling;
 *        if the residual is exactly that one branch, try the duplicated-arm
 *        spelling instead ([[duplicated-statement-into-arms]]):
 *            if (hit) { if (rec0) { if (a2) {...} else { seen |= m; continue; } } }
 *            if (hit) seen |= m;
 *   [S5] The "i >= 6 && i <= 9" range test is what GCC folds to
 *        addiu $v0,$s3,-6 / sltiu $v0,$v0,4. Written as a continue-guard so
 *        the taken edge lands on the increment block (.L8002CCF8).
 *   [S6] No cop2/GTE instruction anywhere in the target - nothing here is
 *        canonical-asm eligible, and the scanner agrees (0/8).
 *   [S7] The two call arms are duplicated in the asm (two separate jal
 *        sequences with near-identical tails, merged only at .L8002CCC4).
 *        The draft duplicates them in source too. If GCC instead hoists the
 *        common tail differently, the alternative is a function-pointer-free
 *        restructure - but do NOT reach for a function pointer, target has two
 *        direct jals.
 *
 * FIRST MEASUREMENT: see tmp/authoring/func_8002CA8C_notes.md
 * ===================================================================== */

extern u8 D_800F5F68;
extern s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);
extern s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);

void func_8002CA8C(u8 *a0, s32 a1, s32 a2) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 id = *(s16 *)(a0 + 4);
    u8 *recbase = (u8 *)&D_800F5F68 + id * 0x1B8;
    s32 base = id * 0x108;
    s32 hitMask = 0;
    s32 seenMask = 0;
    s32 i;

    for (i = 0; i < 0x16; i++) {
        u8 *rec = recbase + i * 0x14;
        s32 off = base + i * 0xC;
        s32 rejected;
        s32 c;
        u16 r;
        s32 hit;

        if (*(s16 *)(a0 + 0x26C) == 0 && i >= 6 && i <= 9) {
            continue;
        }

        r = *(u16 *)(rec + 0xC);
        rejected = 0;
        c = *(s32 *)((u8 *)0x1F8000A8 + off);
        if (*(s32 *)(scr + 0x84) < c - r || c + r < *(s32 *)(scr + 0x78)) {
            rejected = 1;
        } else {
            c = *(s32 *)((u8 *)0x1F8000AC + off);
            if (*(s32 *)(scr + 0x88) < c - r || c + r < *(s32 *)(scr + 0x7C)) {
                rejected = 1;
            } else {
                c = *(s32 *)((u8 *)0x1F8000B0 + off);
                if (*(s32 *)(scr + 0x8C) < c - r || c + r < *(s32 *)(scr + 0x80)) {
                    rejected = 1;
                }
            }
        }
        if (rejected != 0) {
            continue;
        }

        if (a1 != 0) {
            hit = func_8002D780(0, scr, (s32 *)(i * 0xC + 0x1F8000A8 + base),
                                r, *(u16 *)(rec + 0xE));
            if (hit != 0) {
                if (*(s16 *)rec != 0 && a2 != 0) {
                    if (func_8002D780(1, scr, (s32 *)0,
                                      *(u16 *)(rec + 0x10),
                                      *(u16 *)(rec + 0x12)) != 0) {
                        hitMask |= 1 << i;
                    }
                }
                seenMask |= 1 << i;
            }
        } else {
            hit = func_8002D320(0, scr, (s32 *)(i * 0xC + 0x1F8000A8 + base),
                                r, *(u16 *)(rec + 0xE));
            if (hit != 0) {
                if (*(s16 *)rec != 0 && a2 != 0) {
                    if (func_8002D320(1, scr, (s32 *)0,
                                      *(u16 *)(rec + 0x10),
                                      *(u16 *)(rec + 0x12)) != 0) {
                        hitMask |= 1 << i;
                    }
                }
                seenMask |= 1 << i;
            }
        }
    }

    *(s32 *)(scr + 0xB4) = seenMask;
    *(s32 *)(scr + 0xC4) = hitMask;
}
