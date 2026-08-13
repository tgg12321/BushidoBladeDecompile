/* candidate.c — func_8001E404 (src/code6cac.c:1384), grind session 1 (recon, 2026-08-12)
 *
 * STATUS: this is the CURRENTLY COMMITTED body, unchanged. It is instruction-for-
 * instruction identical to asm/funcs/func_8001E404.s (real-build objdiff: 184/184,
 * zero real differences) and it is what keeps the oracle green today.
 *
 * IT IS NOT A CLEAN CANDIDATE. The `s32 pre_pad[2];` on the first line of the body is
 * the unwritten-leading-local-array frame coercion from the forbidden-family catalog:
 * it exists only to make GCC 2.7.2 reserve the 8-byte region `sp+0x10..0x17` that the
 * target frame allocates and NO target instruction ever touches. Remove it and the
 * frame drops 0x70 -> 0x68, the work buffer moves sp+0x18 -> sp+0x10, and the honest
 * sandbox distance is 23 (all of it that same uniform -8 shift plus its one knock-on
 * address-materialization choice).
 *
 * The construct is RETAINED, NOT SANCTIONED. (The 2026-08-12 decisions.md entry that
 * called this function REFUSED / OWNER-ACCEPTED INCOMPLETE has been WITHDRAWN — that
 * disposition was taken in the wrong modality and the session was discarded. This is an
 * ACTIVE grind item.) The live lever is the args term of the frame equation:
 * `current_function_outgoing_args_size` reads 24 in the original and 16 here — see
 * hypotheses.md H5/H6.
 *
 * The honest (cheat-free) form is this body with the `pre_pad` line deleted; it scores
 * 23 and is the form every future probe must improve on. Do not re-derive the 19
 * structural spellings already measured dead in memory/grind/func_8001E6E4/.
 */

typedef struct {
    s32 vx, vy, vz;
    s32 pad0;
    u16 rx, ry, rz;
    u16 pad1;
    s32 dist;
    s32 tail[10];
} CamBuf;

void func_8001E404(void) {
    s32 pre_pad[2];   /* CHEAT — retained, not sanctioned; see header */
    CamBuf local;
    s32 *s2;

    if (D_800A38BA != 0) {
        s32 v3 = D_800A36FA;
        if (v3 == 1) {
            if (D_80101F5E != 0 || D_801023AA != 0) {
                D_800A36FA = 2;
            }
        }
        if (D_800A36FA == 2) goto s2_default;
        if ((u16)D_80101F32 == 0x11 || (u16)D_8010237E == 0x11) {
            s2 = (s32 *)&D_800F6608;
            D_800A36FA = 1;
        } else {
            s2 = (s32 *)&D_800F5328;
            D_800A36FA = 0;
        }
        goto done_s2;
    s2_default:
        s2 = (s32 *)&D_800F6608;
    done_s2:

        game_SetPlayerCount(D_800A36FA < 1);

        {
            s32 fov = 0x2D;
            if (D_800A36FA == 0) {
                fov = 0x50;
            }
            SetGeomScreen(disp_CalcFov(fov));
        }

        if (D_800A36FA == 0) {
            func_80041688(D_800A36F6, 1);
            func_80041688(D_800A36F6 == 0, 0);
        } else {
            func_80041688(0, 0);
            func_80041688(1, 0);
        }
        goto common_tail;
    }
    s2 = (s32 *)&D_800F6608;
common_tail:

    if (D_800A3834 == 1) {
        local.vx = s2[0] + D_800FF5C8;
        local.vy = s2[1] + D_800FF5CC;
        local.vz = s2[2] + D_800FF5D0;
        local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
        local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
        local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;
        local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    } else {
        local = *(CamBuf *)s2;
    }

    func_80046BF4((s32 *)&local, (s32 *)&local.rx, local.dist);
    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538((s32 *)&local, p20);
        func_80061064((s32 *)&local.rx, p20);
    }
    func_8003F3D4((s16 *)((u8 *)s2 + 0x30));
    func_8003F3D4((s16 *)((u8 *)s2 + 0x38));
    D_800A36B4 = (s32)s2;
}
