/* s2: SpuUnion model + COMPLETE aggregate merge of D_800F7420 into it (SpuSetKey rewritten as
 * D_800F7298.rxx.key_on[k]/key_off[k], _spu_init as D_800F7298.raw[0xC4 + channel]).
 * func_80089A48 stays 0/178 but SpuSetKey (COMPLETED-C at HEAD, 0) regresses to 14: the target
 * addresses key_on[0]/key_off[0] register-base (lui/addiu %lo(D_800F7420); sh 0(v0)) which HEAD gets
 * from D_800F7420[0] (offset-0 access of its own symbol), while sym+0x188 through the union is
 * emitted as the symbol+const form (lui at; sh %lo(D_800F7298+0x188)(at)). Merge as spelled is NOT
 * byte-neutral for the sibling -> aggregate-merge prong (c)/(e) unsatisfied by this spelling.
 * Full TU snapshot: tmp/grind/func_80089A48/s2/main_vB_merge.c */
s32 func_80089A48(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (D_800A2CD4 & 1) {
        var_t0 = ((D_800F7298.raw[addr2] & 0xFF) << 16) | D_800F7298.raw[addr1];
    } else {
        var_t0 = ((((SpuUnion *)D_800A2CDC)->raw[addr2] & 0xFF) << 16) | ((SpuUnion *)D_800A2CDC)->raw[addr1];
    }
    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] |= bits;
            D_800F7298.raw[addr2] |= (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)D_800A2CDC)->raw[addr1] |= bits;
            ((SpuUnion *)D_800A2CDC)->raw[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] &= ~bits;
            D_800F7298.raw[addr2] &= ~((bits >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)D_800A2CDC)->raw[addr1] &= ~bits;
            ((SpuUnion *)D_800A2CDC)->raw[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] = bits;
            D_800F7298.raw[addr2] = (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)D_800A2CDC)->raw[addr1] = bits;
            ((SpuUnion *)D_800A2CDC)->raw[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
