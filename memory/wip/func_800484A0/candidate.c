/*
 * WIP candidate for func_800484A0 (src/text1b.c).
 * Apply by replacing the HEAD body in src/text1b.c around line 359.
 *
 * Honest pure-C floor: 4 (HEAD: 5). build_insns 36 == target_insns 36.
 * Drops 0 of 5 regfix rules; src restructure cannot yet retire any rule
 * because GCC still emits the +12 combined and the addiu+move pair for
 * the buf reassignment (see meta.json / notes.md for the 4-diff breakdown).
 *
 * Key semantic discovery (this is the lever):
 *   Target asm shows that in the !decompress path ($v0 == 0 from
 *   func_800486FC), gpu_LoadImage's source ($a1) is the ORIGINAL arg0,
 *   not buf. So the HEAD body's `gpu_LoadImage(rect, (s32)buf)` is
 *   semantically wrong — buf would be uninitialized in that path.
 *   The correct expression is:
 *     - if decompress: write to buf, then upload buf
 *     - else: upload arg0 directly (as the sibling at line 280-296 does)
 *   `arg0 = (u8 *)buf;` inside the if reuses the parameter to express
 *   this "src = buf only after decompress" semantic, matching how
 *   target's $s0 is reassigned at insn 25 (only on the if-true path).
 *
 * Reviewer status: this construct passes the 6-test cheats-by-spelling
 * checklist (semantic purpose: yes; human-programmer: a real programmer
 * might write this when the parameter's lifetime ends mid-function;
 * GCC-internals: justified by target's actual behavior, not by RTL
 * mechanics; family check: not in any forbidden family; naming: arg0
 * keeps its name). See notes.md for the full vetting.
 */
void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {
    s16 rect[4];
    s16 buf[512];
    u32 dim;
    if (arg0[0] != 0x10) return;
    arg0 += 4;
    if ((*(s32 *)arg0 & 8) == 0) return;
    arg0 += 4;
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    arg0 += 4;
    rect[3] = dim >> 16;
    rect[2] = dim;
    if (func_800486FC() != 0) {
        func_8004876C((s32)arg0, rect[2], (s32)buf);
        arg0 = (u8 *)buf;
    }
    gpu_LoadImage(rect, (s32)arg0);
}
