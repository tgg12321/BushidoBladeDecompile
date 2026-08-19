/* ===========================================================================
 * DRAFT TRANSPLANT — _spu_FiDMA  ( = PsyQ libspu spu.c `_spu_FiDMA` )
 * 0x80088BA0 · src/main.c:1693 (currently INCLUDE_ASM) · dist 48 · 0 rules
 *
 * (a) psyz source: tmp/psyq_prov/psyz/decomp/src/libspu/spu.c:161-181
 * (b) Register-file spelling: see func_800889D4__spu_FwriteByIO.c section (b)
 *     — same offsets, same BB2 idiom, do not introduce a union.
 *
 * MAPPING DECISIONS SPECIFIC TO THIS FUNCTION
 *   psyz                     BB2                        addr        status
 *   -----------------------  -------------------------  ----------  ----------------
 *   D_800D1058 (static int)  D_800A2D2C                 0x800A2D2C  ALREADY DECLARED (main.c:90)
 *   _spu_transferCallback    D_800A2D14 (volatile s32)  0x800A2D14  ALREADY DECLARED (main.c:73)
 *   _spu_RXX                 D_800A2CDC                 0x800A2CDC  ALREADY DECLARED (main.c:74)
 *   DeliverEvent(...)        DeliverEvent(...)          —           PsyQ, already used in main.c
 *
 * (c) ASM CROSS-CHECK (asm/funcs/_spu_FiDMA.s, 48 insns, 1 back-edge):
 *   - `lw %lo(D_800A2D2C)` ; `bnez` -> skip the _spu_Fw1ts call    (D_800D1058 == 0)
 *   - `lhu 0x1AA` ; `andi 0xFFCF` ; `sh 0x1AA`                     (clear 0x30)
 *   - `lhu 0x1AA` ; `andi 0x30` ; timeout loop `sltiu $v1,0xF01`   (wait for clear)
 *   - `lw %lo(D_800A2D14)` ; `beqz` ; **RELOADS** `lw %lo(D_800A2D14)` before
 *     `jalr $v0` — that double load is the `volatile` on D_800A2D14, which is
 *     why the existing declaration is `extern volatile s32 D_800A2D14;` and why
 *     the call below goes through a cast of the volatile read rather than a
 *     function-pointer-typed global.  Do NOT re-type it as
 *     `void (*volatile)(void)`; that is a second C handle for the same memory
 *     ([[inline-asm-injection]] alias-rename family).
 *   - else `jal DeliverEvent` with $a0 = 0xF0000009, $a1 = 0x20 — the sweep's
 *     shared-constant evidence.
 *
 * (d) OPEN QUESTIONS FOR THE MEASURING SESSION
 *   1. The `timeout` loop has an empty body in psyz (`break` on overflow, no
 *      other effect).  If GCC deletes it, the residual will be exactly the
 *      loop's ~6 instructions; that is the signal to check that `spucnt` reads
 *      are really going through the volatile pointer expression.
 *   2. `_spu_transferCallback` is called with no arguments and returns void;
 *      the cast `((void (*)(void))D_800A2D14)()` reads the volatile a second
 *      time exactly as the asm does.  Verify the reload survives -O2.
 *   3. This is a 48-instruction function with 0 rules and no known lever — it
 *      is the cheapest of the six drafts to measure and the best first
 *      confidence check that the psyz seed transplants at all.  MEASURE THIS
 *      ONE FIRST.
 *   4. Replaces the INCLUDE_ASM at src/main.c:1693 ONLY.  Keep it between
 *      func_800889D4 and _spu_Fr_ in address order (see the warning comment at
 *      src/main.c:1683-1692 about the alabel and about queue.py's text scan).
 * ======================================================================== */

void _spu_FiDMA(void) {
    u32 timeout;

    if (D_800A2D2C == 0) {
        _spu_Fw1ts();
    }
    *(volatile u16 *)(D_800A2CDC + 0x1AA) =
        *(volatile u16 *)(D_800A2CDC + 0x1AA) & ~0x30;
    timeout = 0;
    while (*(volatile u16 *)(D_800A2CDC + 0x1AA) & 0x30) {
        timeout++;
        if (timeout > 0xF00) {
            break;
        }
    }
    if (D_800A2D14) {
        ((void (*)(void))D_800A2D14)();
        return;
    }
    DeliverEvent(0xF0000009, 0x20);
}
