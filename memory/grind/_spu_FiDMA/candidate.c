/* CANDIDATE — _spu_FiDMA — sandbox distance 0, full-build SHA1 == oracle
 * (session 1, recon, 2026-09-07). Splices into src/main.c in place of
 * INCLUDE_ASM("asm/funcs", _spu_FiDMA); at the address-ordered slot between
 * _spu_FwriteByIO and _spu_Fr_. Requires the HEAD declarations
 *   extern s32 D_800A2D2C;  extern s32 D_800A2CDC;
 *   extern volatile s32 _spu_transferCallback;  extern void _spu_Fw1ts(void);
 * (all already present in main.c; the volatile is the ratified
 * volatile_extern_allowlist.txt:33 grant, not introduced here).
 * Provenance: the psyz transplant seed banked at
 * memory/grind/_spu_FiDMA/psyz-seed.c, applied verbatim modulo the
 * D_800A2D14 -> _spu_transferCallback rename. NO FAKE constructs.
 */
/* PsyQ LIBSPU spu.c `_spu_FiDMA` (C ref: Xeeynamo/psyz decomp/src/libspu/spu.c:161).
   SPU DMA-completion interrupt handler: waits for the transfer-mode bits
   (0x30) in SPUCNT (_spu_RXX + 0x1AA) to clear with a bounded spin, then
   dispatches either the installed transfer callback or the SPU DMA event. */
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
    if (_spu_transferCallback) {
        ((void (*)(void))_spu_transferCallback)();
        return;
    }
    DeliverEvent(0xF0000009, 0x20);
}
