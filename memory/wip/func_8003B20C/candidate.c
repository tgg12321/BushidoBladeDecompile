/* Candidate pure-C body for func_8003B20C — sandbox --disable all = 6.
 * Apply into src/code6cac_c_ab.c verbatim. No pin, no cheat-asm,
 * no dead-local-alias. Reaches sandbox 6 (down from HEAD's distance 5
 * under cheats present — note HEAD scored 5 only because the strip
 * count includes file-scope macros; the true cheat-free HEAD body has
 * its own deficit). All registers ($v0=0xFF, $v1=1, $a0=arg0_new) match
 * target; the residual 6 diffs are pure scheduling — GCC hoists the
 * array load past D_80102780/D_80102781 stores by 3 positions.
 *
 * The committed source uses `register s32 one asm("v1") = 1;` to anchor
 * scheduling AND a dead `u8 *new_var; ...= (new_var = &D_800900EC)` to
 * anchor the array load near its source position. Both are cheats
 * (register-asm pin + dead-store-coercion); neither acceptable per
 * [[no-new-park-categories]] / [[inline-asm-policy]].
 */
void func_8003B20C(s32 arg0) {
    D_80102780 = 0;
    D_80102781 = 1;
    D_800A3894 = 0;
    D_800A385C = 0;
    D_800A3836 = 0xFF;
    D_800A3915 = 0xFF;
    D_800A37C6 = 1;
    D_800A37A0 = 0;
    D_800A37A4 = 0;
    D_800A3844 = ((s32 *)&D_800900EC)[arg0];
    file_LoadOverlay();
    func_8003AE5C(D_800A3844);
    func_8003AF40(0);
    D_800A376C = (&D_8008D538)[(s8)D_8010277C];
}
