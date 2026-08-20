/* Candidate: func_80038170 (src/code6cac_c_mid.c) — PROVEN FORM, session s4
 * (forensics, 2026-08-19). Measured THIS session on the post-asm-until-matched
 * chassis: `sandbox func_80038170 --disable all` = **0** (141/141 insns) and a
 * full `build` = SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE.
 *
 * STATUS 2026-08-20 (s5, forensics): SUBMITTED as an ordinary candidate-ready.
 * The pipeline scope grant landed on 2026-08-19 (tools/grinder/scope_allow.txt:26,
 * `func_80038170 include/code6cac.h`), so the one-line companion declaration fix
 * below is now IN SCOPE and is staged with the Match commit. Re-measured on the
 * current chassis this session with both edits in place: full `build` SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE, and
 * `sandbox func_80038170 --disable all` = **0** (141/141 insns).
 *
 * CHASSIS GOTCHA measured this session and worth inheriting: the FIRST sandbox
 * run after applying the edits reported score 1, not 0. That residual is a STALE
 * build/ REFERENCE, not a code difference — the sandbox scores the fresh .o
 * against the reference objects in build/, which still held the pre-edit
 * `D_8008F19D` relocation spelling. A full `build` regenerates build/ and the
 * very next identical sandbox invocation returns 0 with no source change at all.
 * This is exactly the "floor 1" that s1-s3 recorded and could not dissolve; it
 * was never a property of the C. ORDER OF OPERATIONS for any future session
 * touching this function: apply -> `build` -> `sandbox`, never sandbox first.
 *
 * The src-only spelling of the same reads is a BANNED construct
 * (state.json banned_constructs[2], layer-1 FAIL 2026-08-19 21:23) and must not
 * be resurrected: with the header corrected it is not needed.
 *
 * REQUIRED COMPANION EDIT (include/code6cac.h:80-81):
 *     -extern u8 D_8008F19C;
 *     -extern u8 D_8008F19D;
 *     +extern u8 D_8008F19C[];
 * Both symbols are referenced by this function ONLY (verified by grep over
 * src/ + include/ this session), so the correction is complete and TU-external
 * fallout is zero.
 *
 * ============================================================================
 * WHY THE HEADER IS FACTUALLY WRONG — the s4 forensic proof (this is the new
 * result; s1-s3 asserted the shared-base shape mattered but never named the
 * pass, and the "phantom frame temp inside the conditional scope" story in the
 * s5-era candidate header is WRONG in its details).
 * ============================================================================
 * Target frame is `addiu $sp,$sp,-0x38` => cc1 must report `vars= 16`. Measured
 * with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_FRAME_DEBUG=1):
 *
 *   ONE base + source-level +1 addend (this file):
 *     .frame $sp,56 # vars= 16, regs= 5/0, args= 16   <= TARGET
 *     FRAMEDBG ctx=spill_new_p98  size=8 frame_offset=8
 *     FRAMEDBG ctx=spill_new_p120 size=8 frame_offset=16
 *   TWO separate bases (D_8008F19C / D_8008F19D, the pre-migration cheat form):
 *     .frame $sp,48 # vars=  8, regs= 5/0, args= 16   sandbox 13
 *     FRAMEDBG ctx=spill_new_p98  size=8 frame_offset=8
 *
 * The two forms' emitted bodies differ in EXACTLY ONE instruction —
 * `lbu $2,D_8008F19C+1($3)` vs `lbu $2,D_8008F19D($3)` — plus the frame size
 * and the save offsets it shifts. Everything else is byte-identical.
 *
 * The 8 extra bytes are a reload spill slot for a pseudo that no longer has a
 * SET. Pass-by-pass, from the -da dumps under tmp/grind/func_80038170/dumps/:
 *   .rtl/.cse/.flow  insn 238 `(set (reg:SI 120) (plus (reg:SI 119) (reg:SI 117)))`
 *                    insn 240 `(set (reg:QI 121) (mem (reg:SI 120)))`
 *                    — expand refused `reg + CONST(PLUS(symbol,1))` as a legal
 *                      MIPS address, so memory_address() forced the sum into a
 *                      pseudo. With a bare SYMBOL_REF (the two-base form) it is
 *                      a legal `lbu sym($r)` operand and no such pseudo exists.
 *   .combine         try_combine folds the address back INTO the mem operand
 *                    (`(mem (plus (reg:SI 119) (const (plus (symbol_ref
 *                    "D_8008F19C") (const_int 1)))))`) and leaves the dead
 *                    setter behind as a bare `(insn 439 (use (reg:SI 120)))`.
 *   .greg            global.c sees reg 120 with reg_n_refs > 0 but no SET, so it
 *                    forms no allocno and reg_renumber[120] stays < 0.
 *   reload1.c:2403   alter_reg's `from_reg == -1` arm therefore calls
 *                    assign_stack_local(mode, total_size=8, -1) for it —
 *                    ctx=spill_new_p120 above — and frame_offset goes 8 -> 16.
 *                    No spill store or load is ever emitted; the slot is pure
 *                    reservation. (Same family as [[phantom-frame-slots-gcc272]],
 *                    but the creator here is named: combine's address-fold
 *                    residue, not an expand-time temp.)
 *
 * CONSEQUENCE — this is program-model evidence, not codegen steering. The
 * target's own frame size can only be produced if the ORIGINAL source expressed
 * the second read as a +1 addend on the SAME symbol, i.e. the original C indexed
 * ONE array `D_8008F19C[]` with stride 2. splat's per-byte D_8008F19C /
 * D_8008F19D naming is an artifact with no evidentiary weight
 * ([[splat-symbol-names-are-not-evidence]]); the sibling table one entry later,
 * D_8008F1A8, is ALREADY declared `extern u8 D_8008F1A8[];` in the same header
 * and is read with the identical `[sN*2+0]` / `[sN*2+1]` stride-2 shape.
 *
 * The body below is plain C: no dead local, no volatile, no inline asm, no
 * register pin, no frame coercion, no scheduling construct, no pointer pun, no
 * rule. It claims NO sanctioned family and owes NO annotation.
 */

void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s3 = 0;
    s2 = 0;
    s1 = 0;
    mask = D_80106A50;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = D_8008F204[i];
            switch (v) {
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    strcpy(out + 4, D_8008F1C0);

    out[0x22] = D_8008F1A8[s1 * 2 + 0];
    out[0x23] = D_8008F1A8[s1 * 2 + 1];
    out[0x3C] = D_8008F1A8[s2 * 2 + 0];
    out[0x3D] = D_8008F1A8[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = D_8008F19C[s3 * 2 + 0];
        out[0x43] = D_8008F19C[s3 * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        u16 *src = (u16 *)&D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}
