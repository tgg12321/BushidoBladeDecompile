/*
 * func_8003B5A4 — CANDIDATE (sandbox --disable all == 0, session s1, 2026-08-07)
 *
 * This exact body is IN PLACE in src/code6cac_c_ab.c (lines ~522-620) and scored
 * distance 0 with all 10 regfix rules dropped and cheat-asm stripped. The only
 * residual in a raw objdump diff is the %lo(jtbl_80010D1C) relocation addend
 * (116 in our object vs 0 in the normalized target listing) — a link-layer
 * artifact of the jump table's offset inside the TU's .rodata, not a codegen
 * difference; the engine masks it and the integrated link resolves it.
 *
 * The closing lever (vs the previous form): cases 3 and 1 each use a fresh
 * POINTER LOCAL `u8 *p = (u8 *)D_800A3844;` with the target's statement order —
 * store p+1 back to the global BEFORE loading the byte through p. The pointer
 * dies at its last load, so GCC reuses its register for the loaded byte
 * (target: case 3 p->$a0, case 1 p->$a1 because $a0 is held by &D_80101EC8).
 * The old form cached both bytes early through an s32 `data` local, creating
 * extra live ranges that rotated the whole allocation ($3<->$4 swap cluster)
 * and forced early lbu scheduling (reorder rules). Pure C, no FAKE constructs.
 */
void func_8003B5A4(void) {
    s32 done;
    u8 *chardata;

    obj_InitChars();
    done = 0;
    chardata = &D_8010277D;

    do {
        s32 ptr = D_800A3844;
        s32 cmd;
        D_800A3844 = ptr + 1;
        cmd = *(u8 *)ptr;

        switch (cmd) {
            case 16: {
                s32 ret = func_8003B3A4((u8 *)D_800A3844);
                D_800A3844 += ret;
                break;
            }

            case 17: {
                s32 ret = func_8003B484((u8 *)D_800A3844);
                D_800A3844 += ret;
                break;
            }

            case 3: {
                u8 *p = (u8 *)D_800A3844;
                u8 byte0;
                D_800A3844 = (s32)(p + 1);
                byte0 = p[0];
                D_800A3844 = (s32)(p + 2);
                chardata[0] = byte0;
                chardata[2] = p[1];
                if ((s8)byte0 == D_800A3915) {
                    player_SetCharId(1, D_800A36F4);
                }
                func_8003AF40(1);
                func_8003AFFC();
                break;
            }

            case 1: {
                u8 byte;
                u8 *p = (u8 *)D_800A3844;
                u8 counter = D_800A37A0;
                D_800A3844 = (s32)(p + 1);
                byte = *p;
                D_800A37A0 = counter + 1;
                D_800A36A4 = byte;
                (&D_800A37A8)[counter] = byte;
                D_800A3834 = 22;
                done = 1;
                func_80022568(&D_80101EC8);
                D_800A3907 = 0;
                break;
            }

            case 0:
                if (D_800A380C == 0) {
                    D_800A38A4 = ((&D_8008D9EC)[(s16)D_80101ED2] != 0);
                    D_800A3834 = 18;
                } else {
                    D_800A3834 = 10;
                }
                done = 1;
                break;

            case 19:
                D_800A3834 = 6;
                D_800A3878 = D_800A3844;
                D_800A3844 += 4;
                done = 1;
                break;

            case 2:
                D_800A3894 = (u8 *)D_800A3844;
                D_800A3844 += 0x1D;
                func_8003B2C8();
                func_8003AF40(0);
                func_8003B3A4(D_800A3894 + 1);
            case 18:
                D_800A3834 = 0;
                D_800A3907++;
                done = 1;
                break;

            case 20:
                D_800A385C = (u8 *)D_800A3844;
                D_800A3844 += 12;
                done = 1;
                func_8003B4DC();
                func_8003B56C(1);
                D_800A38BA = 0;
                break;
        }
    } while (!done);
}
