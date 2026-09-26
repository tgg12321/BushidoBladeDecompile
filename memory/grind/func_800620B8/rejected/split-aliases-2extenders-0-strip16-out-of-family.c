/* Draw the D_800F1198 effect particles (see func_80062020): per record until
   the terminator, pick a sprite size and an animation frame from the record's
   type, rotate/translate its position, project it, and when it lands in range
   emit one textured POLY_FT4 billboard and link it into the OT at its depth. */
void func_800620B8(s16 *pos, s32 *trans) {
    typedef struct {
        u32 tag;
        u8 r0, g0, b0, code;
        s16 x0, y0;
        u8 u0, v0;
        u16 clut;
        s16 x1, y1;
        u8 u1, v1;
        u16 tpage;
        s16 x2, y2;
        u8 u2, v2;
        u16 pad1;
        s16 x3, y3;
        u8 u3, v3;
        u16 pad2;
    } POLY_FT4;
    extern s32 D_800A32B8;
    extern s32 D_800A37D4;
    extern s32 D_800A3720;
    extern s32 D_8009BD44[];
    extern u16 D_8009BA00[6][4];
    extern u16 D_8009BA30[4][4];
    extern u16 D_8009BA50[4];
    extern u16 D_8009BA58[4];
    extern void ApplyRotMatrixLV(VECTOR *, VECTOR *);
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    extern void SetPolyFT4(void *);
    extern s32 SetShadeTex(s32, s32);
    extern s32 SetSemiTrans(void *, s32);
    extern s32 rand(void);
    u8 *base;
    s16 *w;
    s16 *h;
    VECTOR *tv;
    VECTOR *v;
    SVECTOR *sv;
    s32 *flag;
    u32 *z;
    POLY_FT4 *prim;
    s32 outer;
    u16 *dst16;
    s32 *dst32;
    u8 *rot;
    s16 i;
    s32 proj_w;
    s32 proj_h;
    s16 width;
    s16 height;
    /* FAKE: pointer aliases of the four sprite tables (pointer-alias-fake-exception).
       All four stay live across the loop; strip32 is set at the loop top so loop.c
       hoists it into the pre-header, after the entry test, where the target sets $fp.
       The other three lose global allocation and reload rebuilds each address in
       $t0 at its use, as the target does. Using the tables directly: 47. */
    u16 (*strip32)[4]; /* FAKE: alias of D_8009BA00 */
    u16 *alt32; /* FAKE: alias of D_8009BA50 */
    u16 (*strip16)[4]; /* FAKE: alias of D_8009BA30 */
    u16 *alt16; /* FAKE: alias of D_8009BA58 */

    func_80060E38((s32)pos, (s32)trans);
    outer = D_800A3468;
    dst16 = (u16 *)D_800A346C;
    dst16[0] = (*(u16 **)(outer + 4))[0];
    prim = (POLY_FT4 *)D_800A37D4;
    dst16[1] = (*(u16 **)(outer + 4))[1];
    dst16[2] = (*(u16 **)(outer + 4))[2];
    dst32 = (s32 *)D_800A3470;
    rot = (u8 *)D_800A3474; /* matrix func_80061FAC builds from pos */
    dst32[0] = (*(s32 **)(outer + 8))[0];
    base = (u8 *)D_800A34EC;
    dst32[1] = (*(s32 **)(outer + 8))[1];
    dst32[2] = (*(s32 **)(outer + 8))[2];
    D_800A32B8++;
    func_80061FAC(dst16, (s32)dst32, rot);
    SetRotMatrix((u8 *)D_800A3474);
    w = (s16 *)(base + 0x10);
    h = (s16 *)(base + 0x12);
    tv = (VECTOR *)(base + 0x14);
    v = (VECTOR *)(base + 0x24);
    sv = (SVECTOR *)(base + 0x34);
    flag = (s32 *)(base + 0x3C);
    z = (u32 *)(base + 0x44);
    sv->vz = 0;
    sv->vy = 0;
    sv->vx = 0;
    *(s32 *)D_800A34B0 = ReadGeomScreen() << 8;
    *(s32 *)D_800A3490 = 0x2F;
    alt32 = D_8009BA50; /* FAKE: alias */
    strip16 = D_8009BA30; /* FAKE: alias */
    alt16 = D_8009BA58; /* FAKE: alias */
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
        strip32 = D_8009BA00; /* FAKE: alias, set here so loop.c hoists it */
        switch (D_800F1198[i].unk4 & 7) {
        case 3:
            *(s16 *)D_800A34A8 = 0x151;
            *(s16 *)D_800A34AC = 0xA8;
            goto sel_a;
        case 0:
            *(s16 *)D_800A34A8 = 0x1C2;
            *(s16 *)D_800A34AC = 0xE1;
        sel_a:
            /* FAKE: `- strip32 + strip32` round trip (chain-extender): combine folds
               it away (zero bytes), but flow.c has already counted the two extra
               uses of strip32, so global.c ranks it above sv/flag and gives it $fp,
               as the target has it. Without it: 15. */
            D_800A348C = D_800A3488 = (s32)strip32[(u32)D_800A32B8 % 6] - (s32)strip32 + (s32)strip32;
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)alt32;
            }
            *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 0x1F;
            *(u16 *)D_800A34A4 = ((u16 *)D_800A3488)[3] + 0x1F;
            break;
        case 2:
            *(s16 *)D_800A34A8 = 0x50;
            *(s16 *)D_800A34AC = 0x3C;
            goto sel_b;
        case 1:
            *(s16 *)D_800A34A8 = 0x64;
            *(s16 *)D_800A34AC = 0x78;
        sel_b:
            /* FAKE: the same round trip (chain-extender); combine re-emits the add
               with the index first, `addu $v0,$v0,$t0` as the target. Without it: 1. */
            D_800A348C = D_800A3488 = (s32)strip16[D_800A32B8 & 3] - (s32)strip16 + (s32)strip16;
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)alt16;
            }
            *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 0xF;
            *(u16 *)D_800A34A4 = ((u16 *)D_800A3488)[3] + 0x13;
            break;
        }
        v->vx = D_800F1198[i].unk0 / 2 - ((s32 *)D_800A3470)[0];
        v->vy = D_800F1198[i].unk4 / 8 - ((s32 *)D_800A3470)[1];
        v->vz = D_800F1198[i].unk8 - ((s32 *)D_800A3470)[2];
        ApplyRotMatrixLV(v, tv);
        /* SetTransMatrix reads only m->t (+0x14): hand it the address 0x14
           below tv so tv is loaded as the translation (base+0x10/0x12 hold
           w/h -- there is no whole MATRIX here). */
        SetTransMatrix((u8 *)tv - 0x14);
        RotTransPers(sv, (s32 *)D_800A34B8, flag, (s32 *)D_800A34CC);
        /* gte_stsz(r0) --- inline_c.h :1042-1046 */
        __asm__ volatile(
            "swc2   $19, 0(%0)\n"
            :: "r"(D_800A34D0) : "memory");
        *z = func_80052C28(*(s32 *)D_800A34D0, 0);
        if (*z < 0x1005 && (*(s32 *)D_800A34B0 / 256 >> 4) < *z) {
            *(s32 *)D_800A3494 = (u16)(((((u16 *)D_800A348C)[0] >> 4) & 0x3F) + (((u16 *)D_800A348C)[1] << 6));
            *(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
            *(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
            *(s32 *)D_800A34D0 = *(s32 *)D_800A34D0 ? *(s32 *)D_800A34D0 : 1;
            *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 / *(s32 *)D_800A34D0;
            proj_w = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4;
            width = proj_w > 0x200 ? proj_w >> 8 : 2;
            *w = width / 2;
            proj_h = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4;
            height = proj_h > 0x200 ? proj_h >> 8 : 2;
            *h = height;
            *h += (*h * rand() / 10) >> 14;
            SetPolyFT4(prim);
            prim->tpage = *(s32 *)D_800A3490;
            prim->clut = *(s32 *)D_800A3494;
            prim->r0 = 0xFF;
            prim->g0 = 0x80;
            prim->b0 = 0x80;
            prim->x0 = *(s32 *)D_800A34B8 - *w;
            prim->y0 = (*(s32 *)D_800A34B8 >> 16) - *h;
            prim->x1 = *w + *(s32 *)D_800A34B8;
            prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *h;
            prim->x2 = *(s32 *)D_800A34B8 - *w;
            prim->y2 = *(s32 *)D_800A34B8 >> 16;
            prim->x3 = *w + *(s32 *)D_800A34B8;
            prim->y3 = *(s32 *)D_800A34B8 >> 16;
            prim->u0 = *(u16 *)D_800A3498;
            prim->v0 = *(u16 *)D_800A34A0;
            prim->u1 = *(u16 *)D_800A349C;
            prim->v1 = *(u16 *)D_800A34A0;
            prim->u2 = *(u16 *)D_800A3498;
            prim->v2 = *(u16 *)D_800A34A4;
            prim->u3 = *(u16 *)D_800A349C;
            prim->v3 = *(u16 *)D_800A34A4;
            SetShadeTex((s32)prim, 1);
            SetSemiTrans(prim, 1);
            if (prim - (POLY_FT4 *)D_800A3720 < 0x1C1) {
                D_800A34E4 = g_gpu_ot_ptr + *z * 4;
                D_800A34E8 = (s32)prim;
                *(u32 *)D_800A34E8 = (*(u32 *)D_800A34E8 & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);
                *(u32 *)D_800A34E4 = (D_800A34E8 & 0xFFFFFF) | (*(u32 *)D_800A34E4 & 0xFF000000);
                prim++;
            }
        }
    }
    D_800A37D4 = (s32)prim;
}
