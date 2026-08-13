/* candidate.c — func_8001E404 (src/code6cac.c:1384), grind session 2 (structural, 2026-08-12)
 *
 * STATUS: **sandbox --disable all == 0 was MEASURED THIS SESSION with this exact body
 * in src/code6cac.c** (score 0, target_insns 184, build_insns 184, 0 rules dropped;
 * normalized objdiff against asm/funcs/func_8001E404.s = 184/184 with only the four
 * known `addiu s2,s2,%lo(SYM)` relocation-display artifacts). The `s32 pre_pad[2];`
 * unwritten-leading-local-array cheat is GONE, and the frame partition is now
 * mechanically identical to the target's: cc1 reports
 *     .frame $sp,112,$31   # vars= 72, regs= 4/0, args= 24, extra= 0
 * (the committed pre_pad form reports vars= 80 / args= 16 — same total frame, wrong
 * partition; see evidence.md s1 for why the partition, not the total, is the defect).
 *
 * IT IS NOT SUBMITTED AS candidate-ready. The construct that produces args=24 is a
 * COMPILED-OUT CALL SITE — `if (0) { bb2_dbg_probe(0,0,0,0,0,0); }` — i.e. dead code.
 * That is a first reach of a family no sanctioned SOTN carve-out covers, so session 2
 * returned `ruling-request` rather than self-approving it. Do NOT commit this body
 * until the owner rules. The tree was reverted to the committed pre_pad form at the
 * end of session 2; re-apply from here in one edit if the ruling is favourable.
 *
 * WHY THE DEAD CALL IS THE ONLY MECHANISM (measured, session 2):
 *   - o32 has OUTGOING_REG_PARM_STACK_SPACE with a 16-byte register-home floor, so a
 *     LIVE call raises current_function_outgoing_args_size above 16 only by having a
 *     5th+ argument word — and every such word is genuinely STORED into sp+0x10..0x17.
 *     The target stores nothing anywhere in sp+0x10..0x17 (s1 $sp map).
 *   - expand_call raises current_function_outgoing_args_size during RTL expansion;
 *     compute_frame_size reads it at final, long after jump.c/cse.c may have deleted
 *     the call's insns. Deleting the call does NOT lower the args partition.
 *   => args>16 with zero stores in the args region <=> the >4-word call was expanded
 *      and then deleted. The original translation unit therefore CONTAINED a >=5-word
 *      call site that compiled away; the frame is its fossil.
 *
 * Every dead spelling measured produces args=24 identically (tmp/grind/func_8001E404/
 * s2/probe2.c): `if (0) {...}`, `s32 mode = 0; if (mode) {...}`, a zero-trip `for`,
 * statements after `return`, and a `goto` over the call. 5-word and 6-word dead calls
 * both give args=24; a 7-word dead call gives args=32 — which is exactly the 16-byte
 * hole of the third family member func_8003CF84 (tmp/.../s2/probe3.c).
 *
 * The second edit — `s32 *lp = (s32 *)&local;` reused by the two consumers — is
 * ordinary live C and is required for the last 3 points: with `local` at vars offset 0
 * its address IS the frame base, so GCC rematerializes `addiu $a0,$sp,0x18` at each
 * consumer (183 insns) where the target holds it in $s0 (`addiu $s0,$sp,0x18` +
 * `move $a0,$s0` x2, 184 insns). Naming the pointer restores the target's form.
 * Measured gradient this session: 23 (honest baseline) -> 3 (dead call alone) -> 0.
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
    extern void bb2_dbg_probe();   /* UNRULED: stand-in for the original's compiled-out callee */
    CamBuf local;
    s32 *s2;
    s32 *lp;

    if (0) {                       /* UNRULED: reconstructs current_function_outgoing_args_size == 24 */
        bb2_dbg_probe(0, 0, 0, 0, 0, 0);
    }

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

    lp = (s32 *)&local;
    func_80046BF4(lp, (s32 *)&local.rx, local.dist);
    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538(lp, p20);
        func_80061064((s32 *)&local.rx, p20);
    }
    func_8003F3D4((s16 *)((u8 *)s2 + 0x30));
    func_8003F3D4((s16 *)((u8 *)s2 + 0x38));
    D_800A36B4 = (s32)s2;
}
