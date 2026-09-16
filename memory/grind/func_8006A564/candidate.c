/* func_8006A564 -- src/text1b.c
 * Session 1 (recon). sandbox --disable all score = 137 (target_insns 199,
 * build_insns 204). Pure C, zero cheat constructs, zero FAKE annotations.
 * Structure: draws 3 TILE primitives (SetTile/SetSemiTrans/AddPrim) whose
 * color bytes are chosen by (D_800A34F8 & 0xF == arg2), then loads a second
 * object's field_1C->field_28 into arg1's packet, halves/sets its color
 * bytes under the same condition, advances two fields, calls
 * func_8007352C(arg1) twice (feeding two different source structs' data
 * through the same packet), and finishes with a SetDrawMode/AddPrim of
 * *(arg0+0x1C), advancing that field by 0xC.
 *
 * KNOWN GAP (turn budget spent measuring, not yet closed): the compiled
 * register allocation differs from target. Target uses s2=arg0, s1=arg1,
 * s3=arg2, s4=(the local I call `s4`), with NO s5 and a 0x30-byte frame
 * (asm/funcs/func_8006A564.s:2 `addiu $sp,$sp,-0x30`, saves s0-s4+ra).
 * Our build (tmp/sandbox/func_8006A564/text1b.o, objdump'd s1 turn 12)
 * allocates s3=arg0, s2=arg1, s4=arg2, s5=(the `s4` local), s0/s1 as
 * scratch, and needs a 0x38-byte frame (saves s0-s5+ra) -- one extra
 * callee-save register, +5 instructions overall (204 vs 199).
 *
 * The srl-vs-sra byte-level diff (color >>1 ops) was fixed this session by
 * casting through (u32) before the shift -- confirmed via objdump that our
 * build now emits `srl` matching target's `srl s0,s0,0x1` (was `sra`).
 *
 * NOT yet attempted: reordering local declarations / call-argument
 * expression order to steer allocno priority toward target's s1/s2/s3/s4
 * mapping (see hypotheses.md frontier). No permuter run yet.
 */
extern s32 D_800A34F8;
extern s32 D_800A374C;
extern void SetTile(void *);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, void *);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {
    u8 *tile;
    u8 *obj2;
    s32 v0;
    s32 s4;

    tile = *(u8 **)(arg0 + 0x18);
    SetTile(tile);
    v0 = D_800A34F8 & 0xF;
    if (v0 == arg2) {
        v0 = *(u8 *)(arg1 + 0x29);
        tile[5] = 0;
        tile[4] = v0;
        v0 = *(u8 *)(arg1 + 0x2B);
        s4 = 0;
    } else {
        v0 = 0x50;
        tile[4] = v0;
        tile[5] = v0;
        s4 = 0x20;
    }
    tile[6] = v0;
    v0 = 0x5F;
    *(s16 *)(tile + 8) = v0;
    v0 = *(s32 *)(arg1 + 0x1C);
    v0 = v0 + 0xF;
    *(s16 *)(tile + 0xA) = v0;
    v0 = *(s32 *)(arg1 + 0x18);
    *(s16 *)(tile + 0xE) = 1;
    v0 = v0 + 0x19;
    *(s16 *)(tile + 0xC) = v0;
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    v0 = D_800A34F8 & 0xF;
    if (v0 == arg2) {
        v0 = *(u8 *)(arg1 + 0x29);
        tile[5] = 0;
        tile[4] = v0;
        v0 = *(u8 *)(arg1 + 0x2B);
        tile[6] = v0;
    } else {
        v0 = 0x20;
        tile[4] = v0;
        tile[5] = v0;
        tile[6] = v0;
    }
    v0 = *(s32 *)(arg1 + 0x18);
    v0 = v0 + 0x78;
    *(s16 *)(tile + 8) = v0;
    v0 = *(s32 *)(arg1 + 0x1C);
    *(s16 *)(tile + 0xC) = 0x78;
    *(s16 *)(tile + 0xE) = 1;
    v0 = v0 + 0xE;
    SetSemiTrans(tile, 1);
    *(s16 *)(tile + 0xA) = v0;
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    v0 = D_800A34F8 & 0xF;
    if (v0 == arg2) {
        v0 = *(u8 *)(arg1 + 0x29);
        tile[5] = 0;
        v0 = (u32)v0 >> 1;
        tile[4] = v0;
        v0 = *(u8 *)(arg1 + 0x2B);
        v0 = (u32)v0 >> 1;
    } else {
        v0 = 0x10;
        tile[4] = v0;
        tile[5] = v0;
    }
    tile[6] = v0;
    v0 = *(s32 *)(arg1 + 0x18);
    v0 = v0 + 0x40;
    *(s16 *)(tile + 8) = v0;
    v0 = *(s32 *)(arg1 + 0x1C);
    *(s16 *)(tile + 0xC) = 0x38;
    *(s16 *)(tile + 0xE) = 1;
    v0 = v0 + 0xD;
    SetSemiTrans(tile, 1);
    *(s16 *)(tile + 0xA) = v0;
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);

    obj2 = *(u8 **)(arg0 + 4);
    tile = *(u8 **)(arg0 + 0x18) + 0x30;
    *(u8 **)(arg0 + 0x18) = tile;
    tile = *(u8 **)(obj2 + 0x1C);
    v0 = *(s32 *)(tile + 0x28);
    *(s32 *)(arg1 + 0) = v0;

    v0 = D_800A34F8 & 0xF;
    if (v0 == arg2) {
        v0 = *(u8 *)(arg1 + 0x29);
        v0 = (u32)v0 >> 1;
        *(u8 *)(arg1 + 0x2A) = 0;
        *(u8 *)(arg1 + 0x29) = v0;
        v0 = *(u8 *)(arg1 + 0x2B);
        v0 = (u32)v0 >> 1;
        *(u8 *)(arg1 + 0x2B) = v0;
    } else {
        v0 = 0x28;
        *(u8 *)(arg1 + 0x2B) = v0;
        *(u8 *)(arg1 + 0x2A) = v0;
        *(u8 *)(arg1 + 0x29) = v0;
    }

    v0 = *(s32 *)(arg1 + 0);
    v0 = *(s32 *)(arg1 + 0x1C);
    *(s32 *)(arg1 + 0x18) = 0;
    v0 = *(s32 *)(arg1 + 0) + 0xC;
    *(s32 *)(arg1 + 0x1C) = *(s32 *)(arg1 + 0x1C) + 0xF;
    *(s32 *)(arg1 + 4) = v0;

    v0 = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg1 + 8) = v0;
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);

    v0 = *(s32 *)(tile + 0x2C);
    *(s32 *)(arg1 + 0) = v0;
    *(s32 *)(arg1 + 4) = v0 + 0xC;
    v0 = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg1 + 8) = v0;
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}
