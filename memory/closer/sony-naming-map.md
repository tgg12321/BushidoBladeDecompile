
- func_8008AF9C → SpuSetCommonAttr (LIBSPU/S_SCA); its extent tail 0x8008B330 = SpuRGetAllKeysStatus (LIBSPU/SR_GAKS dead code, now a static C fn)
- func_80089F3C → SpuSetReverbModeParam (LIBSPU/S_SRMP)
- D_800A2888 → _spu_rev_attr (struct SpuRevAttr; members D_800A288C mode / D_800A2890-92 depth / D_800A2894 delay / D_800A2898 feedback)
- D_800A2D94 → Sony rev_param preset table (10 x 0x44 RevParamEntry)
coli_HitPauseKatana_2 -> _SpuSetAnyVoice (confirmed: shared static behind SpuSetNoiseVoice/SpuSetReverbVoice wrappers @0x80089A24/0x8008A904; kengo:HIGH name was an x2 size collision)
