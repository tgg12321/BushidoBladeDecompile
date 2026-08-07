/* WIP candidate body for special_camera_Exec (src/code6cac_b2.c)
 *
 * NOT IN BUILD. memory/wip/ is outside the compile/link pipeline.
 * To apply: paste this body in place of the existing `void special_camera_Exec(void) {}` stub,
 *           and REMOVE the asmfix bridge `special_camera_Exec: replace_with_asmfile ...`.
 * Expected sandbox `--disable all` floor: 69 (target 274 insns / build 284 insns).
 *
 * STRUCTURAL BLOCKER: jtbl-rodata-split-infrastructure.
 *   target's `jtbl_80010978` lives at 0x80010978 in asm/data/101C.rodata_pre.s
 *   (linked at bb2.ld:29-31 — far earlier than code6cac_b2.o(.rodata) at bb2.ld:50).
 *   GCC emits a NEW jtbl in code6cac_b2.o(.rodata) for the switch below,
 *   which cannot land at 0x80010978. Removing the asmfix bridge causes:
 *     ld: 101C.rodata_pre.o:(.rodata+0x110): undefined reference to `.L80036D74'
 *     (+13 more — the external jtbl_80010978 references the deleted asm labels)
 *   See [[jtbl-rodata-split-infrastructure]] — exactly the same shape as
 *   `replay_camera_rob_back_loose2` (only other current jtbl-infra case in BB2).
 *
 * Required externs (not already in code6cac.h):
 *   extern s32 Vu0SetLightColMatrix_800801E8(s32, void *);
 *   extern void myRobGeneiDraw3(s32, void *);
 *   extern void _DispCharacterName(s32, s32, s32);
 *   extern u8  D_800A3760;
 */

void special_camera_Exec(void) {
    u8 buf[4];
    s32 r;

    if (D_80101E62 >= 0x10) {
        special_camera_set_win_cam();
        return;
    }
    if ((u32)D_80101E62 >= 0xE) {
        return;
    }
    switch (D_80101E62) {
    case 2:
        if (D_80101E68 != 0) {
            D_80101E62 = 0;
            return;
        }
        buf[0] = 0xA0;
        myRobGeneiDraw3(0xE, buf);
        D_80101E66 = 0;
        D_80101E98 = 0;
        D_80101E62 = 3;
        return;

    case 3:
        r = Vu0SetLightColMatrix_800801E8(1, &D_800A3760);
        if (r == 2) {
            D_80101E62 = 4;
            D_80101E8C = 0;
            return;
        }
        if (r == 5) {
            D_80101E62 = 5;
            return;
        }
        D_80101E98++;
        if ((s16)D_80101E98 < 0x3D) {
            return;
        }
        D_80101E62 = 0xA;
        return;

    case 4:
        D_80101E8C++;
        if (D_80101E8C < 3) {
            return;
        }
        D_80101E84 = D_80101E7C;
        D_80101E80 = D_80101E78;
        D_80101EA0 = cdrom_BcdToFrames(D_80101E7C);
        _DispCharacterName(2, D_80101E7C, 0);
        D_80101E98 = 0;
        D_80101E62 = 5;
        return;

    case 5:
        r = Vu0SetLightColMatrix_800801E8(1, &D_800A3760);
        if (r == 2) {
            cdrom_SetCallbackB((s32)marionation_camera_Init_80036064);
            D_80101E98 = 0;
            buf[0] = 0x40;
            myRobGeneiDraw3(6, buf);
            D_80101E62 = 6;
            return;
        }
        if (r == 5) {
            D_80101E62 = 9;
            return;
        }
        D_80101E98++;
        if ((s16)D_80101E98 < 0x3D) {
            return;
        }
        cdrom_SetCallbackB(0);
        D_80101E62 = 0xA;
        return;

    case 6:
        r = Vu0SetLightColMatrix_800801E8(1, &D_800A3760);
        if (r == 2) {
            if (D_80101E80 == 0) {
                D_80101E62 = 8;
                return;
            }
            if (D_80101E80 < 0) {
                D_80101E62 = 9;
                return;
            }
            D_80101E98++;
            if ((s16)D_80101E98 < 0x3D) {
                return;
            }
            cdrom_SetCallbackB(0);
            D_80101E62 = 0xA;
            return;
        }
        if (r == 5) {
            cdrom_SetCallbackB(0);
            D_80101E62 = 9;
            return;
        }
        D_80101E98++;
        if ((s16)D_80101E98 < 0x3D) {
            return;
        }
        cdrom_SetCallbackB(0);
        D_80101E62 = 0xA;
        return;

    case 8:
        if (D_800A3760 & 0x10) {
            D_80101E62 = 0xA;
            return;
        }
        D_80101E62 = 0xC;
        return;

    case 9:
        myRobGeneiDraw3(1, 0);
        D_80101E62 = 0xB;
        D_80101E5C = 0;
        return;

    case 10:
        r = Vu0SetLightColMatrix_800801E8(1, &D_800A3760);
        if (r == 2) {
            if (D_800A3760 & 0x10) {
                D_80101E62 = 0xA;
                return;
            }
            D_80101E62 = 0xC;
            return;
        }
        if (r == 5) {
            D_80101E62 = 0xA;
            return;
        }
        D_80101E5C++;
        if (D_80101E5C < 0xB) {
            return;
        }
        func_80080148();
        D_80101E62 = 0xA;
        return;

    case 12:
        myRobGeneiDraw3(0x13, 0);
        D_80101E62 = 0xD;
        D_80101E5C = 0;
        return;

    case 13:
        r = Vu0SetLightColMatrix_800801E8(1, &D_800A3760);
        if (r == 2) {
            D_80101E62 = 2;
            sys_VSync(4);
            sys_VSync(4);
            sys_VSync(4);
            sys_VSync(4);
            return;
        }
        if (r == 5) {
            D_80101E62 = 9;
            return;
        }
        D_80101E5C++;
        if (D_80101E5C < 0x1F) {
            return;
        }
        func_80080148();
        D_80101E62 = 0xA;
        return;
    }
}
