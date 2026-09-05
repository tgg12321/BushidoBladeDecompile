struct Chr {
    union { s32 status; u16 index; } st;
    u32 pad04;
    u32 pad08;
    s32 *srcW;
    u16 *srcH;
    s8 *outByte;
    u16 w18;
    u16 w1A;
    u16 w1C;
    u16 pad1E;
    s32 w20;
    s32 w24;
    s32 w28;
};
extern struct Chr *D_800A3468;

void func_80060A68(void) {
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60[];
    extern s32 D_800F10D0[];
    extern s32 chractar_use_pset_combo_id_table[];
    s32 result;

    D_800F10D0[D_800A3468->st.index] = 0;
    D_800A3468->w20 = D_800A3468->srcW[0];
    D_800A3468->w24 = D_800A3468->srcW[1];
    D_800A3468->w28 = D_800A3468->srcW[2];
    D_800A3468->w18 = D_800A3468->srcH[0];
    D_800A3468->w1A = D_800A3468->srcH[1];
    D_800A3478 = (s32)&D_800A3468->w18;
    D_800A3468->w1C = D_800A3468->srcH[2];
    D_800A347C = (s32)&D_800A3468->w20;

    result = ((s32 (*)(void)) chractar_use_pset_combo_id_table[
                  D_8009BA60[D_800A3468->st.index]
                  + D_800F10D0[D_800A3468->st.index]])();
    *D_800A3468->outByte = result;

    if (D_800A3468->st.status & 0x200000) {
        D_800A32BC = 0xA;
    }
}
