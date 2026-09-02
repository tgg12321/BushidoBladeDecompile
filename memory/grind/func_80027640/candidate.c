/* CANDIDATE -- func_80027640 -- s1 (2026-09-01) -- sandbox --disable all == 0 (158/158 insns
 * instruction-identical; target 160 words incl. the two mfhi->mult hazard nops objdump collapses).
 * Pure C, no FAKE constructs, no volatile. The retired chassis body (8 volatile s32 locals,
 * 4 never referenced) is replaced by two local VECTORs (include/gte.h, libgte canonical):
 *   tgt at sp+0x10 (vx@0x10, vz@0x18; vy/pad untouched)  -- the blended target position
 *   dir at sp+0x20 (vx@0x20, vz@0x28; vy/pad untouched)  -- the push direction
 * GCC 2.7.2 keeps aggregate locals in memory, which reproduces every sp-slot round trip.
 * The direction constants MUST be spelled `v = A; if (cond) v = B; dir.v = v;` -- ternary
 * flips branch sense (rejected/ternary-direction-flips-branch-sense.c), and storing into
 * dir.vx inside if/else arms forces a frame reload (rejected/struct-store-in-both-arms-reloads.c).
 * REQUIRES the fidelity gate entry `func_80027640` in maspsx_label_nop_funcs.txt -- an OPERATOR
 * surface (add-scope-allow denylist), NOT landed by any grind session (s1 was discarded for it):
 * target has `lh $v0,4($a0)` @0x800277A0 -> `.L800277A4:` -> `nop` -> `sw $v0,0x18($sp)`;
 * maspsx drops that load-delay nop across a .L merge label unless the function is listed
 * (.claude/rules/maspsx-label-nop-gate.md). Without the entry: sandbox 1 (that nop). With it
 * (proven in tmp/ by s2: tmp/grind/func_80027640/s1/build_gated.sh + link_gated.sh): 160/160
 * words and full-build SHA1 == oracle. Handoff entry: docs/grind/decisions.md:20004.
 * Commit tag: [infra-rule: maspsx-label-nop] + site citation asm/funcs/func_80027640.s:95-98. */
void func_80027640(s32 arg0)
{
    VECTOR tgt;
    VECTOR dir;
    s32 idx;
    s16 *tbl;
    s32 rate;
    u8 cnt;
    void *r1;
    void *r2;
    s32 vx;
    s32 vz;

    idx = *(s16 *)(arg0 + 4);
    tbl = (s16 *)stage_GetDataPtr();
    cnt = *(u8 *)(arg0 + 0x34C);
    if (cnt < 0x40) {
        *(u8 *)(arg0 + 0x34C) = cnt + 1;
    }
    rate = 0x3C - ((*(u8 *)(arg0 + 0x34C) - 1) * 4);
    if (rate < 10) {
        rate = 10;
    }
    if (D_800A36A4 == 3) {
        vx = 0x2EE0;
        if (*(s32 *)(*(s32 *)arg0 + 0xF4) >= 0x3E9) {
            vx = -0x2710;
        }
        dir.vx = vx;
        vz = 0x1770;
        if (*(s32 *)(*(s32 *)arg0 + 0xFC) > 0) {
            vz = -0x1770;
        }
        dir.vz = vz;
        tgt.vx = (dir.vx * rate + *(s32 *)(arg0 + 0xF4) * (100 - rate)) / 100;
        tgt.vz = (dir.vz * rate + *(s32 *)(arg0 + 0xFC) * (100 - rate)) / 100;
    } else {
        tbl += (D_800A36A4 * 12 + idx * 3);
        tgt.vx = tbl[0];
        tgt.vz = tbl[2];
    }
    tgt.vx -= *(s32 *)(arg0 + 0xF4);
    tgt.vz -= *(s32 *)(arg0 + 0xFC);
    *(s32 *)(arg0 + 0xF4) += tgt.vx;
    *(s32 *)(arg0 + 0xFC) += tgt.vz;
    *(s32 *)(arg0 + 0xD8) += tgt.vx;
    *(s32 *)(arg0 + 0xE0) += tgt.vz;
    *(s32 *)(arg0 + 0xB8) += tgt.vx;
    *(s32 *)(arg0 + 0xC0) += tgt.vz;
    *(s32 *)(arg0 + 0x104) = 0;
    *(s32 *)(arg0 + 0x108) = 0;
    *(s32 *)(arg0 + 0x10C) = 0;
    *(s32 *)(arg0 + 0x134) = 0;
    *(s32 *)(arg0 + 0x138) = 0;
    *(s32 *)(arg0 + 0x13C) = 0;
    r1 = func_80021424(arg0, **(u16 **)(arg0 + 0x50), arg0 + 0x5E);
    r2 = func_80021424(arg0, *(u16 *)((s32)r1 + 0x3A), arg0 + 0x5E);
    func_80021A98(idx, r2, *(s16 *)(arg0 + 0x5E));
    func_80032854(*(s16 *)(arg0 + 4), 0x30, (s32 *)(arg0 + 0xF4), 0);
}
