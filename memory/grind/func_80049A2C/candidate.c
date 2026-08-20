/* ===================================================================
 * s7 CORRECTION TO THE s6 HEADER BELOW - READ FIRST
 * ===================================================================
 * The s6 "CONSEQUENCE" paragraph below concludes that target's +8 frame
 * slot "is reachable ONLY through a wholly dead, memory-resident local".
 * That is FALSE and s7 measured it false. s6 enumerated only ONE of the two
 * sources of MIPS frame bytes (expand-time locals, get_frame_size()). The
 * second source is reload1.c:2404 alter_reg, which reserves an 8-byte-rounded
 * stack slot for any pseudo with reg_renumber < 0 and reg_n_refs > 0 - and a
 * pseudo whose insns were all absorbed by combine (combine.c:10836 leaves a
 * codegen-free `(use (reg))` after a CODE_LABEL to carry the orphaned REG_DEAD
 * note) is never allocated and never referenced. Result: vars=8 with ZERO
 * stack traffic, which is exactly target's shape.
 *
 * 70 functions in this repo's own oracle-matching source have that signature,
 * 26 of them loopless, including func_800493E4 in THIS FILE - ordinary C, no
 * dead local. s7 also killed the standing cross-ledger claim that the +8 is a
 * cc1psx-vs-fork divergence: cc1psx compiles this very body to the same
 * frame 40. Full record in evidence.md / hypotheses.md under [s7].
 *
 * This body is UNCHANGED from s6 and still scores 12 (126/126 insns; the 12
 * differing instructions are exactly the prologue adjust, five saves, five
 * restores and the epilogue adjust). It remains the best honest form.
 * ===================================================================
 */
/* func_80049A2C — session s6 (synthesis) BEST HONEST FORM.
 *
 * sandbox --disable all = 12 (frame 0x28 vs target 0x30; 126/126 insns, the
 * whole residual is sp-relative offset shift). This is the TRUE cheat-free
 * floor and it matches memory/grind/func_80049A2C/migration_pin.json (12).
 *
 * ===================================================================
 * s6 CORRECTION — the banked "sandbox = 0" of sessions s1..s5 was an
 * ARTIFACT of a cheat-stripper spelling hole, not a byte match.
 * ===================================================================
 * s1's candidate carried `s32 dummy[2];    /* LOAD-BEARING ... *[/]` with a
 * TRAILING COMMENT on the declaration line. engine/volatile_cheats.py's
 * orphaned-declaration closure (`_ORPHAN_DECL_RE`) is anchored `;[ 	]*$`,
 * so the trailing comment made the declaration invisible to the stripper
 * while `(void) dummy;` WAS stripped. The dead 8-byte local therefore
 * survived into the "cheat-invisible" build and produced target's
 * `addiu $sp,-0x30`. Removing only the comment (identical C otherwise)
 * scores 12. Verified reproducible 2x, and by reading the post-strip
 * source the sandbox actually compiles
 * (tmp/sandbox/func_80049A2C/src/text1b.c).
 *
 * The same applies to s4's H8 `struct { s32 a; s32 b; } dummy;` "sandbox=0":
 * NO detector covers a zero-reference struct-typed local, so it is a second
 * spelling hole (checklist test T4 — passes only because the detectors do
 * not catch THIS spelling).
 *
 * ===================================================================
 * s6 MECHANISM LAW for target's +8 frame slot (10 real-build measurements,
 * tmp/grind/func_80049A2C/s6/matrix.md)
 * ===================================================================
 * Frame = 40 with no locals; target = 48. The +8 appears iff a local
 * aggregate is BLKmode, i.e. its (size, alignment) does NOT admit a scalar
 * integer machine mode:
 *   char[1] (QI) / s16[1] (HI) / s32[1] (SI)  -> pseudo, NO frame bytes -> 40
 *   char[2] / char[3] / char[4] / s32[2] / struct{s16;s16;} -> BLKmode,
 *      assign_stack_local, get_frame_size()=2..8 -> ALIGN8 -> 8 -> 48
 * ALIGNMENT decides, not size (char[4] -> 48 but s32[1] -> 40, both 4 bytes).
 * `(void) dummy;` is NOT needed: a zero-reference BLKmode local alone gives 48.
 *
 * CONSEQUENCE (the closure this session establishes):
 *  - Any BLKmode local that is SEMANTICALLY LIVE emits stack traffic. Target
 *    is exactly 126 insns with ZERO `sw/lw` to 0x00..0x14($sp) (only the five
 *    s0-s3/ra saves). So a live one cannot exist.
 *  - Any aggregate small/aligned enough to avoid stack traffic is promoted to
 *    a pseudo and reserves NO frame bytes (measured: live `s16 a1_val_a[1]`
 *    carrying a1_val = 126 insns, frame 40).
 *  - Therefore the +8 slot is reachable ONLY through a wholly dead,
 *    memory-resident local — the `unused-local-array frame coercion` family
 *    in the forbidden catalog. Every "0" this function has ever scored came
 *    from one of those, hidden from the stripper by a spelling hole.
 *
 * Ladder status: phantom-slot mechanism KILLED with cc1 -da (s3, still valid —
 * it was a dump-level, not score-level, measurement); scalar widening KILLED
 * (s2); recomputation H3 KILLED (s2); canonical-asm certified LOW 0/8 (s4);
 * permuter infrastructure-blocked and independently reconfirmed (s4, s5);
 * BLKmode/mode-promotion law established and live-aggregate axis KILLED (s6).
 */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    u8 *new_var6;
    u8 *new_var5;
    s16 *new_var7;
    u8 temp_v1;
    u8 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    int new_var3;
    u8 *obj;
    u8 *vehicle;
    s16 a1_val;
    u8 *ot;

    new_var6 = D_80099CC8;
    {
        u8 *p = new_var6 + (arg0 * 2);
        temp_v1 = p[arg2];
    }
    if (temp_v1 == 0xFF) {
        return;
    }
    new_var3 = 8;
    new_var8 = (u8 *) D_800EF980;
    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));
    if ((*p_anim) < 0) {
        InitFadePanel();
    }
    vehicle = (u8 *) func_8004153C(arg1 >> 1);
    obj = D_800A38B4;
    obj[0] = 0;
    obj[1] = 0;
    a1_val = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + new_var3)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = a1_val;
    src = &D_80099D3C[(arg1 & 1) * 6];
    *((s32 *) (obj + 0x4C)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x50)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x54)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((u16 *) (obj + 0x10)) = (u16) (*src);
    src++;
    *((u16 *) (obj + 0x12)) = (u16) (*src);
    new_var2 = src[1];
    *((s32 *) (obj + 0xC)) = (s32) (vehicle + 0x50C);
    *((s16 *) (obj + 6)) = 0;
    *((u16 *) (obj + 0x14)) = (u16) new_var2;
    func_800417D0((s32 *) obj);
    ot = D_800A3820;
    D_800A3820 = ot + 4;
    *((u8 **) ot) = obj;
    obj += 0x68;
    new_var5 = obj + 0xA;
    a1_val = (*p_anim) * 2;
    obj[0] = 3;
    *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
    obj[1] = 0;
    new_var7 = (s16 *) (obj + 6);
    *((s16 *) (obj + new_var3)) = 0;
    *new_var7 = 1;
    *((s16 *) new_var5) = 0;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 2)) = (s16) (a1_val + 1);
    *((s32 *) (obj + 0x58)) = (s32) (*((s16 *) (vehicle + 0x1A84)));
    ot = D_800A3820;
    D_800A3820 = ot + 4;
    *((u8 **) ot) = obj;
    D_800A38B4 = obj + 0x68;
}
