/*
 * REJECTED — func_8003C714, s4 (2026-09-01/02), permuter modality.
 *
 * Reading the three `*(s32 *)(src + 4)` values once into a local `s32 v` is NOT
 * a byte-neutral tidy-up of candidate.c. Measured through the validated s4
 * mini-TU pipeline (tmp/grind/func_8003C714/s4/mkobj.sh, which mirrors
 * engine/pipeline.py for the code6cac_c2 stem): this body emits **88**
 * instructions against the target's **104** (-16), and its permuter base score
 * is 4040 versus candidate.c's 590.
 *
 * candidate.c's DUPLICATED reads are load-bearing structure, not incidental
 * style. Do not "clean this up" in a future session — see hypotheses.md K14.
 *
 * (Corroborating datum: the s4-vlocal-basin permuter campaign, seeded on THIS
 * body, spent 39,955 iterations and its best-ever score was exactly 590 —
 * reached by a mutant that had restored the three separate reads and left `v`
 * dead. The randomizer walked back to candidate.c on its own.)
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 v;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        v = *(s32 *)(src + 4);
        dst[0x21] = v / 1800;
        dst[0x22] = (v / 30) % 60;
        dst[0x23] = ((v % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
