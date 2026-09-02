/* s4 REJECTED - tblall: every D_8008E194 access respelled as a direct indexed read of
 * the global ((&D_8008E194)[arg1*7 + k]) with the tbl pointer local removed - FOUR
 * indexed sites on a SECOND global.  vars stays 8 (one slot) at bodydiff=64.  This
 * refines the s3 law: 'indexed global site' is NOT a general orphan generator; only
 * Judge's accesses are folded to (mem (plus (reg idx) (symbol_ref))), while
 * D_8008E194 / D_80106A78 keep a materialised base register (lui+addiu) in both ours
 * and the target, so their address pseudos never orphan.  tblglob0 = 0 bodydiff,
 * tblglob2 = 46, tblglob4 = 152, all vars=8.  */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s32 i;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * (&D_8008E194)[arg1 * 7 + 2]) >> 12;
    *(s32 *)(obj + 0x48) = (&D_8008E194)[arg1 * 7 + 3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * (&D_8008E194)[arg1 * 7 + 2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if ((&D_8008E194)[arg1 * 7] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(&D_8008E194 + arg1 * 7 + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if ((&D_8008E194)[arg1 * 7] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(&D_8008E194 + arg1 * 7 + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if ((&D_8008E194)[arg1 * 7] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(&D_8008E194 + arg1 * 7 + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    *(s32 *)(obj + 0x50) = 1;
    *(u8 *)(obj + 5) = 0;
    *(s16 *)obj = 0;
    return (s32 *)obj;
}
