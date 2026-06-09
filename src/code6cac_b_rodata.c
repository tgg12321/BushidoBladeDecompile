/* Rodata sub-TU split out for the 101C.rodata_pre cluster (rodata-cleanup
 * project, docs/rodata-cleanup-project.md, 2026-06-09).
 *
 * MULTI-FILE cluster: the 10 symbols below are referenced from functions
 * spanning code6cac_b.c (jtbl_8001086C/jtbl_8001089C → func_80034708),
 * code6cac_b2.c (5 jtbls → replay_camera_rob_back_loose2 + func_80035828 +
 * special_camera_set_win_cam + special_camera_Exec), code6cac_c.c
 * (D_800109B0/D_800109BC → func_80037A20/func_80037B90/func_80037C34), and
 * code6cac_c_mid.c (D_800109C8 → motion_LoadPreCalcData_80037F08).
 *
 * Splitting these across all four owner files would be invasive — multiple
 * cross-file source restructures + matching bb2.ld reorders. The sub-TU
 * approach isolates ALL the bytes into one file that takes the asm/data
 * block's exact link slot (between code6cac_b.o(.rodata) and
 * code6cac_c0.o(.rodata)). The owning functions stay in their .c files; the
 * jtbl/string symbols become defined in code6cac_b_rodata.o and resolve
 * across TUs at link time.
 *
 * Jtbl entries reference asm-bridged stub function bodies' internal `.L<n>`
 * labels — those labels aren't C-visible, so the jtbls use literal hex
 * addresses (cc1 produces identical bytes either form).
 *
 * NOTE: replay_camera_rob_back_loose2's 24-rule jtbl-infra cluster
 * (`[[jtbl-rodata-split-infrastructure]]`) is independent of this
 * retirement — those rules bridge GCC's emitted jtbl in code6cac_b2.o
 * to the external jtbl_800108CC symbol, which now resolves to this
 * sub-TU's definition (instead of the asm/data block's). The function
 * itself remains INCOMPLETE (still parked under jtbl-infra). */
#include "common.h"

/* Leading 4 bytes of zero padding — matches the original asm/data block's
 * leading `.word 0x00000000` that aligns the first jtbl to its target
 * address. Declared at the top of the file so it lands at offset 0 of
 * this .o's .rodata section. */
static const u32 _bb2_101C_pre_lead = 0;

const u32 jtbl_8001086C[12] = {
    0x80034B3C, 0x80034B50, 0x80034B64, 0x80034B78,
    0x80034B98, 0x80034BAC, 0x80034BCC, 0x80034BE0,
    0x80034D3C, 0x80034D50, 0x80034BFC, 0x80034C1C,
};

const u32 jtbl_8001089C[12] = {
    0x80034C7C, 0x80034C90, 0x80034CA4, 0x80034CB8,
    0x80034CD8, 0x80034CEC, 0x80034D0C, 0x80034D20,
    0x80034D3C, 0x80034D50, 0x80034D64, 0x80034D84,
};

const u32 jtbl_800108CC[8] = {
    0x80035644, 0x80035784, 0x8003568C, 0x80035694,
    0x800357DC, 0x800357B0, 0x800356D4, 0x80035808,
};

const u32 jtbl_800108EC[14] = {
    0x8003595C, 0x80035AD8, 0x80035C04, 0x80035C38,
    0x80035C64, 0x80035C98, 0x800358C0, 0x800358F0,
    0x80035934, 0x80035D30, 0x80035A50, 0x80035A9C,
    0x80035B9C, 0x80035BD0,
};

const u32 jtbl_80010924[5] = {
    0x80035B18, 0x80035D9C, 0x80035B18, 0x80035B6C,
    0x80035B84,
};

const u32 jtbl_80010938[16] = {
    0x80036360, 0x800363A4, 0x800363DC, 0x80036434,
    0x80036490, 0x800364C0, 0x80036634, 0x8003683C,
    0x8003686C, 0x80036880, 0x800368CC, 0x800368E0,
    0x800367B0, 0x800367D8, 0x80036808, 0x00000000,
};

const u32 jtbl_80010978[14] = {
    0x80036D74, 0x80036D74, 0x80036990, 0x800369DC,
    0x80036A54, 0x80036AC8, 0x80036B50, 0x80036D74,
    0x800369A4, 0x80036C24, 0x80036C40, 0x80036C54,
    0x80036CB8, 0x80036CE0,
};

const char D_800109B0[12] = "bu%1d%1d:*";
const char D_800109BC[12] = "bu%1d%1d:%s";
const char D_800109C8[16] = "bu%1d%1d:";
