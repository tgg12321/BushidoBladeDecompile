# PsyQ library code in SLUS_006.63 (Bushido Blade 2) -- verbatim-link scan

Scan date: 2026-07-09. Method: parse every module of every PsyQ 4.0 .LIB (sozud/psy-q mirror),
mask reloc-affected instruction fields (HI16/LO16 imm16, REL26 low-26, FULL32 whole word),
anchor-search the EXE with runs of >=4 fully-unmasked words, then verify 100% of unmasked bits
over the module's entire .text. Positive control: LIBCD BIOS.OBJ @0x80080828 (prior ground truth) reproduced.

EXE: `disc/SLUS_006.63`, loads at 0x80010000, 604160 bytes.

## Summary

- **177 distinct verbatim library module placements** (189 module identities incl. identical-code duplicates)
- **68208 bytes** of the EXE are bit-verbatim Sony PsyQ 4.0 library .text (11.3% of the 604,160-byte image)
- Contiguous span 0x80078948..0x8008D070 contains ALL of it (13 sub-regions, gaps = unmatched (newer-build) LIBSND/LIBSPU + glue)
- **92 queue items** (71 active, 21 parked) fall inside verbatim library modules

## (a) Verbatim-matched modules (link order)

All rows: 100% of non-reloc-masked bits identical over the full module .text.

| BB2 vaddr | end | lib/module | words | Sony symbols (.text XDEFs) |
|---|---|---|---|---|
| 0x80078948 | 0x80078958 | LIBAPI/C67 | 4 | Exec |
| 0x80078958 | 0x80078968 | LIBAPI/C112 / LIBCARD/C112 | 4 | _bu_init |
| 0x80078968 | 0x80078978 | LIBAPI/C159 | 4 | SetMem |
| 0x80078978 | 0x80078988 | LIBAPI/A08 | 4 | OpenEvent |
| 0x80078988 | 0x80078998 | LIBAPI/A09 | 4 | CloseEvent |
| 0x80078998 | 0x800789A8 | LIBAPI/A11 | 4 | TestEvent |
| 0x800789A8 | 0x800789B8 | LIBAPI/A12 | 4 | EnableEvent |
| 0x800789B8 | 0x800789C8 | LIBAPI/A36 | 4 | EnterCriticalSection |
| 0x800789C8 | 0x800789D8 | LIBAPI/A37 | 4 | ExitCriticalSection |
| 0x800789D8 | 0x800789E8 | LIBAPI/A39 | 4 | SetSp |
| 0x800789E8 | 0x800789F8 | LIBAPI/A50 | 4 | open |
| 0x800789F8 | 0x80078A08 | LIBAPI/A52 | 4 | read |
| 0x80078A08 | 0x80078A18 | LIBAPI/A53 | 4 | write |
| 0x80078A18 | 0x80078A28 | LIBAPI/A54 | 4 | close |
| 0x80078A28 | 0x80078A38 | LIBAPI/A65 | 4 | format |
| 0x80078A38 | 0x80078A48 | LIBAPI/A66 | 4 | firstfile |
| 0x80078A48 | 0x80078A58 | LIBAPI/A67 | 4 | nextfile |
| 0x80078A58 | 0x80078A68 | LIBAPI/A91 | 4 | ChangeClearPAD |
| 0x80078A68 | 0x80078BE0 | LIBAPI/COUNTER | 94 | StopRCnt, ResetRCnt, StartRCnt, GetRCnt, SetRCnt |
| 0x80078BE0 | 0x80078F00 | LIBAPI/PAD | 200 | SetInitPadFlag, InitPAD, StopPAD, ReadInitPadFlag, StartPAD, PAD_init |
| 0x80078F00 | 0x80078F10 | LIBAPI/A18 | 4 | InitPAD2 |
| 0x80078F10 | 0x80078F20 | LIBAPI/A19 | 4 | StartPAD2 |
| 0x80078F20 | 0x80078F30 | LIBAPI/A20 | 4 | StopPAD2 |
| 0x80078F30 | 0x80078F40 | LIBAPI/A21 | 4 | PAD_init2 |
| 0x80078F40 | 0x80078F50 | LIBAPI/L02 | 4 | SysEnqIntRP |
| 0x80078F50 | 0x80078F60 | LIBAPI/L03 | 4 | SysDeqIntRP |
| 0x80078F60 | 0x80078FF0 | LIBAPI/PATCH | 36 | _patch_pad, EnablePAD, DisablePAD |
| 0x80078FF0 | 0x80079000 | LIBAPI/C68 | 4 | FlushCache |
| 0x80079000 | 0x800790C0 | LIBAPI/SENDPAD | 48 | _send_pad, _SendPAD |
| 0x800790C0 | 0x80079120 | LIBAPI/CHCLRPAD | 24 | _remove_ChgclrPAD |
| 0x80079120 | 0x80079154 | LIBC2/MEMCPY | 13 | memcpy |
| 0x80079154 | 0x80079194 | LIBC2/RAND | 16 | srand, rand |
| 0x80079194 | 0x800791D8 | LIBC2/STRCPY | 17 | strcpy |
| 0x800791D8 | 0x80079208 | LIBC2/STRLEN | 12 | strlen |
| 0x80079208 | 0x80079244 | LIBC2/PRINTF | 15 | printf |
| 0x80079244 | 0x800798CC | LIBC2/PRNT | 418 | prnt |
| 0x800798CC | 0x8007992C | LIBC2/CTYPE | 24 | tolower, toupper |
| 0x8007992C | 0x8007997C | LIBC2/MEMCHR | 20 | memchr |
| 0x8007997C | 0x80079A30 | LIBC2/PUTCHAR | 45 | putchar |
| 0x80079A30 | 0x8007A28C | LIBC/SPRINTF / LIBC2/SPRINTF | 535 | sprintf |
| 0x8007A28C | 0x8007A2F8 | LIBC/MEMMOVE / LIBC2/MEMMOVE | 27 | memmove |
| 0x8007A2F8 | 0x8007A308 | LIBCARD/C171 | 4 | _card_info |
| 0x8007A308 | 0x8007A318 | LIBCARD/C172 | 4 | _card_load |
| 0x8007A318 | 0x8007A350 | LIBCARD/CARD | 14 | _card_clear |
| 0x8007A350 | 0x8007A360 | LIBCARD/A78 | 4 | _card_write |
| 0x8007A360 | 0x8007A370 | LIBCARD/A80 | 4 | _new_card |
| 0x8007A370 | 0x8007A428 | LIBCARD/INIT | 46 | StartCARD, InitCARD, StopCARD |
| 0x8007A428 | 0x8007A438 | LIBCARD/A74 | 4 | InitCARD2 |
| 0x8007A438 | 0x8007A448 | LIBCARD/A75 | 4 | StartCARD2 |
| 0x8007A448 | 0x8007A458 | LIBCARD/A76 | 4 | StopCARD2 |
| 0x8007A458 | 0x8007A4D8 | LIBCARD/END | 32 | _ExitCard |
| 0x8007A4D8 | 0x8007A788 | LIBGPU/EXT | 172 | LoadClut, LoadClut2, LoadTPage, SetDefDrawEnv, SetDefDispEnv |
| 0x8007A788 | 0x8007AE7C | LIBGPU/PRIM | 445 | SetSprt8, AddPrims, SetTile16, SetLineF2, SetLineG2, SetLineF3, DumpClut, SetLineG3 … |
| 0x8007AE7C | 0x8007DF10 | LIBGPU/SYS | 3109 | SetGraphDebug, SetDrawOffset, DrawOTagEnv, SetGraphQueue, DrawPrim, DrawSync, SetDrawArea, DrawSyncCallback … |
| 0x8007DF10 | 0x8007DF20 | LIBAPI/C73 | 4 | GPU_cw |
| 0x8007DF20 | 0x8007DFEC | LIBGTE/GEO_00 | 51 | rsin, sin_1 |
| 0x8007DFEC | 0x8007E08C | LIBGTE/GEO_01 | 40 | rcos |
| 0x8007E08C | 0x8007E11C | LIBGTE/MSC00 | 36 | InitGeom |
| 0x8007E11C | 0x8007E1AC | LIBGTE/MSC01 | 36 | SquareRoot0 |
| 0x8007E1AC | 0x8007E43C | LIBGTE/MSC06 | 164 | LoadAverage12, LoadAverage0, LoadAverageShort12, LoadAverageShort0, LoadAverageByte, LoadAverageCol |
| 0x8007E43C | 0x8007E4DC | LIBGTE/MSC09 | 40 | SquareRoot12 |
| 0x8007E4DC | 0x8007E5EC | LIBGTE/MTX_000 | 68 | MulMatrix0 |
| 0x8007E5EC | 0x8007E74C | LIBGTE/MTX_003 | 88 | CompMatrix |
| 0x8007E74C | 0x8007E8AC | LIBGTE/MTX_004 | 88 | ApplyMatrixLV |
| 0x8007E8AC | 0x8007E8DC | LIBGTE/MTX_005 | 12 | ApplyRotMatrix |
| 0x8007E8DC | 0x8007EA0C | LIBGTE/MTX_00A | 76 | ScaleMatrixL |
| 0x8007EA0C | 0x8007EB4C | LIBGTE/MTX_01 | 80 | ApplyRotMatrixLV |
| 0x8007EB4C | 0x8007EC5C | LIBGTE/MTX_03 | 68 | MulMatrix |
| 0x8007EC5C | 0x8007ED6C | LIBGTE/MTX_04 | 68 | MulMatrix2 |
| 0x8007ED6C | 0x8007EDBC | LIBGTE/MTX_05 | 20 | ApplyMatrix |
| 0x8007EDBC | 0x8007EEEC | LIBGTE/MTX_08 | 76 | ScaleMatrix |
| 0x8007EEEC | 0x8007EF1C | LIBGTE/MTX_09 | 12 | SetRotMatrix |
| 0x8007EF1C | 0x8007EF4C | LIBGTE/MTX_11 | 12 | SetColorMatrix |
| 0x8007EF4C | 0x8007EF6C | LIBGTE/MTX_12 | 8 | SetTransMatrix |
| 0x8007EF6C | 0x8007EF8C | LIBGTE/REG04 | 8 | ReadSZfifo3 |
| 0x8007EF8C | 0x8007EF9C | LIBGTE/REG09 | 4 | ReadGeomScreen |
| 0x8007EF9C | 0x8007EFBC | LIBGTE/REG10 | 8 | SetBackColor |
| 0x8007EFBC | 0x8007EFDC | LIBGTE/REG11 | 8 | SetFarColor |
| 0x8007EFDC | 0x8007EFFC | LIBGTE/REG12 | 8 | SetGeomOffset |
| 0x8007EFFC | 0x8007F00C | LIBGTE/REG13 | 4 | SetGeomScreen |
| 0x8007F00C | 0x8007F21C | LIBGTE/SMP_00 | 132 | LightColor, DpqColorLight, DpqColor3, Intpl, Square12, Square0, AverageZ3, AverageZ4 … |
| 0x8007F21C | 0x8007F24C | LIBGTE/SMP_02 | 12 | RotTransPers |
| 0x8007F24C | 0x8007F2AC | LIBGTE/SMP_03 | 24 | RotTransPers3 |
| 0x8007F2AC | 0x8007F2DC | LIBGTE/SMP_04 | 12 | RotTrans |
| 0x8007F2DC | 0x8007F35C | LIBGTE/CMB_00 | 32 | RotTransPers4 |
| 0x8007F35C | 0x8007F5EC | LIBGTE/FGO_01 | 164 | RotMatrix |
| 0x8007F5EC | 0x8007F87C | LIBGTE/FGO_03 | 164 | RotMatrixZYX |
| 0x8007F87C | 0x8007FA1C | LIBGTE/FGO_04 | 104 | RotMatrixX |
| 0x8007FA1C | 0x8007FBBC | LIBGTE/FGO_05 | 104 | RotMatrixY |
| 0x8007FBBC | 0x8007FD5C | LIBGTE/FGO_06 | 104 | RotMatrixZ |
| 0x8007FD5C | 0x8007FEDC | LIBGTE/RATAN | 96 | ratan2 |
| 0x8007FEDC | 0x8007FF7C | LIBGTE/PATCHGTE | 40 | _patch_gte |
| 0x8007FF7C | 0x8008008C | LIBCD/EVENT | 68 | CdInit |
| 0x8008008C | 0x8008009C | LIBAPI/A07 | 4 | DeliverEvent |
| 0x8008009C | 0x80080828 | LIBCD/SYS | 483 | CdComstr, CdStatus, CdIntstr, CdMode, CdDataCallback, CdGetSector, CdSync, CdSyncCallback … |
| 0x80080828 | 0x80082000 | LIBCD/BIOS | 1526 | CD_flush, CD_vol, CD_datasync, CD_initintr, CD_init, CD_sync, CD_set_test_parmnum, CD_getsector … |
| 0x80082000 | 0x80082050 | LIBC2/PUTS | 20 | puts |
| 0x80082050 | 0x800828CC | LIBCD/CDREAD | 543 | CdReadBreak, CdRead, CdReadCallback, CdReadMode, CdReadSync |
| 0x800828CC | 0x80082AB0 | LIBETC/VSYNC | 121 | VSync |
| 0x80082AB0 | 0x80082AC0 | LIBAPI/L10 | 4 | ChangeClearRCnt |
| 0x80082AC0 | 0x800831D0 | LIBETC/INTR | 452 | VSyncCallback, ResetCallback, GetIntrMask, SetIntrMask, VSyncCallbacks, StopCallback, InterruptCallback, DMACallback … |
| 0x800831D0 | 0x800831F0 | LIBAPI/C114 | 8 | _96_remove |
| 0x800831F0 | 0x80083200 | LIBAPI/A23 | 4 | ReturnFromException |
| 0x80083200 | 0x80083210 | LIBAPI/A24 | 4 | ResetEntryInt |
| 0x80083210 | 0x80083220 | LIBAPI/A25 | 4 | HookEntryInt |
| 0x80083220 | 0x800832A0 | LIBC2/SETJMP | 32 | setjmp, longjmp |
| 0x800832A0 | 0x800833C8 | LIBETC/INTR_VB | 74 | startIntrVSync |
| 0x800833C8 | 0x80083670 | LIBETC/INTR_DMA | 170 | startIntrDMA |
| 0x80083670 | 0x80083698 | LIBETC/VMODE | 10 | GetVideoMode, SetVideoMode |
| 0x8008386C | 0x8008387C | LIBAPI/C57 | 4 | InitHeap |
| 0x80083954 | 0x80083A18 | LIBSND/SSEND | 49 | SsEnd |
| 0x80083A18 | 0x80083A48 | LIBSND/SSINIT_C / LIBSND/SSINIT_H | 12 | SsInit, SsInitHot |
| 0x80083A48 | 0x80083B30 | LIBSND/SSINIT | 58 | _SsInit |
| 0x80083B30 | 0x80083B50 | LIBAPI/SEND / LIBGS/GS_106 / LIBSND/SSNOFF / LIBSND/SSQUIT | 8 | SendPAD, GsSetProjection, SsSetNoiseOff, SsQuit |
| 0x80083B50 | 0x80083BE4 | LIBSND/SSSATTR | 37 | SsSetSerialAttr |
| 0x80083BE4 | 0x80083C34 | LIBSND/SSSMV | 20 | SsSetMVol |
| 0x80083C34 | 0x80083F6C | LIBSND/SSSTART | 206 | SsStart2, SsStart |
| 0x80083F6C | 0x800841E0 | LIBSND/SSCALL | 157 | SsSeqCalledTbyT |
| 0x800848AC | 0x80084948 | LIBSND/PAUSE | 39 | _SsSndPause |
| 0x80084948 | 0x80084974 | LIBSND/PLAY | 11 | _SsSndPlay |
| 0x80085064 | 0x80085114 | LIBSND/MIDITIME | 44 | _SsReadDeltaValue |
| 0x80085114 | 0x80085210 | LIBSND/NEXT | 63 | _SsSndNextSep |
| 0x80085210 | 0x80085270 | LIBSND/REPLAY | 24 | _SsSndReplay |
| 0x8008541C | 0x80085448 | LIBSND/PLAY | 11 | _SsSndPlay |
| 0x80085448 | 0x80085544 | LIBSND/SSSV | 63 | SsSetSerialVol |
| 0x80085544 | 0x800856B0 | LIBSND/SSTICK | 91 | SsSetTickMode |
| 0x800856B0 | 0x800858D0 | LIBSND/TEMPO | 136 | _SsSndTempo |
| 0x800859F0 | 0x80085A40 | LIBSND/UT_GVBA | 20 | SsUtGetVBaddrInSB |
| 0x80085E4C | 0x80085EE4 | LIBSND/UT_RDEP | 38 | SsUtSetReverbDepth |
| 0x80085EE4 | 0x80085F98 | LIBSND/UT_REV | 45 | SsUtGetReverbType, SsUtSetReverbType |
| 0x80085F98 | 0x80085FB8 | LIBSND/UT_ROFF / LIBSPU/S_I | 8 | SsUtReverbOff, SpuInit |
| 0x80085FB8 | 0x80085FD8 | LIBSND/UT_RON / LIBSPU/S_IH | 8 | SsUtReverbOn, SpuInitHot |
| 0x80086B38 | 0x80086CF8 | LIBSND/VM_N2P | 112 | note2pitch, note2pitch2 |
| 0x80087E3C | 0x80087F00 | LIBSND/VM_VSU | 49 | _SsVmVSetUp |
| 0x80087F00 | 0x80087F10 | LIBSND/VS_AUTO | 4 | SsSetAutoKeyOffMode |
| 0x80087F10 | 0x80087F34 | LIBSND/VS_MONO | 9 | SsSetStereo, SsSetMono |
| 0x80087F34 | 0x80087F64 | LIBSND/VS_SRV | 12 | SsSetReservedVoice |
| 0x80087F64 | 0x80087FE8 | LIBSND/VS_VAB | 33 | SsVabClose |
| 0x80087FE8 | 0x80088058 | LIBSND/VS_VFB | 28 | SsVabFakeBody |
| 0x80088058 | 0x800884C4 | LIBSND/VS_VH | 283 | SsVabOpenHeadSticky, SsVabFakeHead, SsVabOpenHead |
| 0x800884C4 | 0x80088584 | LIBSND/VS_VTB | 48 | SsVabTransBody |
| 0x80088584 | 0x800885AC | LIBSND/VS_VTC | 10 | SsVabTransCompleted |
| 0x800885AC | 0x800885CC | LIBSND/UT_ROFF / LIBSPU/S_I | 8 | SsUtReverbOff, SpuInit |
| 0x800885CC | 0x80088740 | LIBSPU/S_INI | 93 | _SpuInit, SpuStart |
| 0x80088740 | 0x800892D4 | LIBSPU/SPU | 741 | _spu_FsetRXXa, _spu_Fr_, _spu_t, _spu_FsetPCR, _spu_FiDMA, _spu_FsetRXX, _spu_init, _spu_Fr … |
| 0x800892D4 | 0x800892F8 | LIBSPU/S_DCB | 9 | _SpuDataCallback |
| 0x800892F8 | 0x80089374 | LIBSPU/S_Q | 31 | SpuQuit |
| 0x80089374 | 0x80089384 | LIBAPI/A13 | 4 | DisableEvent |
| 0x80089384 | 0x800893D8 | LIBSPU/S_M_INIT | 21 | SpuInitMalloc |
| 0x800893D8 | 0x800896A0 | LIBSPU/S_M_M | 178 | SpuMalloc |
| 0x800896A0 | 0x800899A8 | LIBSPU/S_M_INT | 194 | _spu_gcSPU |
| 0x800899A8 | 0x80089A24 | LIBSPU/S_M_F | 31 | SpuFree |
| 0x80089A24 | 0x80089A48 | LIBSPU/S_SNV | 9 | SpuSetNoiseVoice |
| 0x80089D10 | 0x80089D60 | LIBSPU/S_SNC | 20 | SpuSetNoiseClock |
| 0x80089D60 | 0x80089E30 | LIBSPU/S_SR | 52 | SpuSetReverb |
| 0x80089E30 | 0x80089F3C | LIBSPU/S_M_UTIL | 67 | _SpuIsInAllocateArea_, _SpuIsInAllocateArea |
| 0x80089F3C | 0x8008A434 | LIBSPU/S_SRMP | 318 | SpuSetReverbModeParam |
| 0x8008A434 | 0x8008A904 | LIBSPU/S_SRA | 308 | _spu_setReverbAttr |
| 0x8008A904 | 0x8008A928 | LIBSPU/S_SRV | 9 | SpuSetReverbVoice |
| 0x8008A928 | 0x8008AAC4 | LIBSPU/S_CRWA | 103 | SpuClearReverbWorkArea |
| 0x8008AAC4 | 0x8008AAD4 | LIBAPI/A10 | 4 | WaitEvent |
| 0x8008AAD4 | 0x8008ACD0 | LIBSPU/S_SK | 127 | SpuSetKey |
| 0x8008ACD0 | 0x8008AD64 | LIBSPU/S_GKS | 37 | SpuGetKeyStatus |
| 0x8008AD64 | 0x8008ADC4 | LIBSPU/S_R / LIBSPU/S_W | 24 | SpuRead, SpuWrite |
| 0x8008ADC4 | 0x8008AE24 | LIBSPU/S_R / LIBSPU/S_W | 24 | SpuRead, SpuWrite |
| 0x8008AE24 | 0x8008AE7C | LIBSPU/S_STSA | 22 | SpuSetTransferStartAddr |
| 0x8008AE7C | 0x8008AEB0 | LIBSPU/S_STM | 13 | SpuSetTransferMode |
| 0x8008AEB0 | 0x8008AF58 | LIBSPU/S_ITC | 42 | SpuIsTransferCompleted |
| 0x8008AF58 | 0x8008AF9C | LIBSPU/S_IT | 17 | _spu_getInTransfer, _spu_setInTransfer |
| 0x8008AF9C | 0x8008B330 | LIBSPU/S_SCA | 229 | SpuSetCommonAttr |
| 0x8008B330 | 0x8008B488 | LIBSPU/SR_GAKS | 86 | SpuGetAllKeysStatus, SpuRGetAllKeysStatus |
| 0x8008BA94 | 0x8008BD88 | LIBSPU/S_N2P | 189 | _spu_note2pitch, _spu_pitch2note, _spu_2pitch |
| 0x8008BD88 | 0x8008BDE8 | LIBSPU/S_GVV | 24 | SpuGetVoiceVolume |
| 0x8008BDE8 | 0x8008BE04 | LIBSPU/S_GVEX | 7 | SpuGetVoiceEnvelope |
| 0x8008BE04 | 0x8008D050 | LIBCOMB/COMB | 1171 | ChangeClearSIO, AddCOMB, DelCOMB, _comb_control |
| 0x8008D050 | 0x8008D060 | LIBAPI/A71 | 4 | AddDrv |
| 0x8008D060 | 0x8008D070 | LIBAPI/A72 | 4 | DelDrv |

Sites listing multiple lib/module identities have byte-identical masked code (e.g. SpuRead/SpuWrite,
SsUtReverbOn/SpuInitHot stubs); link-order context suggests the LIBSND/LIBSPU identity in the sound block.
Full per-address Sony symbol map (XDEFs + static locals): `tmp/libscan/symbols.txt`.

Per-library rollup (by first-listed identity):

| lib | placements | bytes |
|---|---|---|
| LIBAPI | 43 | 2248 |
| LIBC | 2 | 2248 |
| LIBC2 | 11 | 2528 |
| LIBCARD | 10 | 480 |
| LIBCD | 4 | 10480 |
| LIBCOMB | 1 | 4684 |
| LIBETC | 5 | 3308 |
| LIBGPU | 3 | 14904 |
| LIBGTE | 37 | 8284 |
| LIBSND | 32 | 6944 |
| LIBSPU | 29 | 12100 |

## (b) Queue items inside verbatim library regions (92: 71 active, 21 parked)

| queue func | file | status | dist | addr | lib/module | Sony name |
|---|---|---|---|---|---|---|
| func_80078A68 | text1b_b | parked | 2 | 0x80078A68 | LIBAPI/COUNTER | SetRCnt |
| func_80078E58 | text1b_b | parked | 2 | 0x80078E58 | LIBAPI/PAD | _Pad1 (static) |
| func_80079244 | text1b_b | active | 416 | 0x80079244 | LIBC2/PRNT | prnt |
| func_80079A30 | text1b_b | parked | 533 | 0x80079A30 | LIBC/SPRINTF | sprintf |
| func_8007A28C | gpu | active | 17 | 0x8007A28C | LIBC/MEMMOVE | memmove |
| initDrawMode | gpu | parked | 5 | 0x8007AB8C | LIBGPU/PRIM | SetDrawTPage |
| gpu_SetMode | gpu | active | 0 | 0x8007AE7C | LIBGPU/SYS | ResetGraph |
| gpu_SetDispMask | display | active | 0 | 0x8007B2A0 | LIBGPU/SYS | SetDispMask |
| func_8007B6C8 | display | parked | 21 | 0x8007B6C8 | LIBGPU/SYS | MoveImage |
| func_8007B844 | display | parked | 7 | 0x8007B844 | LIBGPU/SYS | ClearOTagR |
| func_8007BC08 | display | parked | 296 | 0x8007BC08 | LIBGPU/SYS | PutDispEnv |
| func_8007C2A0 | display | parked | 4 | 0x8007C2A0 | LIBGPU/SYS | SetDrawEnv |
| func_8007C4B8 | display | active | 4 | 0x8007C4B8 | LIBGPU/SYS | SetDrawEnv2 (static) |
| func_8007C7A0 | display | parked | 20 | 0x8007C7A0 | LIBGPU/SYS | get_cs (static) |
| func_8007C86C | display | parked | 20 | 0x8007C86C | LIBGPU/SYS | get_ce (static) |
| func_8007CA00 | display | parked | 13 | 0x8007CA00 | LIBGPU/SYS | get_dx (static) |
| func_8007CBB0 | display | parked | 149 | 0x8007CBB0 | LIBGPU/SYS | _clr (static) |
| func_8007CE0C | display | parked | 48 | 0x8007CE0C | LIBGPU/SYS | _dws (static) |
| func_8007D048 | display | active | 159 | 0x8007D048 | LIBGPU/SYS | _drs (static) |
| func_8007D3F8 | display | active | 43 | 0x8007D3F8 | LIBGPU/SYS | _addque2 (static) |
| func_8007D6D8 | display | active | 185 | 0x8007D6D8 | LIBGPU/SYS | _exeque (static) |
| func_8007DC9C | display | parked | 9 | 0x8007DC9C | LIBGPU/SYS | get_alarm (static) |
| func_8007E1AC | display | active | 13 | 0x8007E1AC | LIBGTE/MSC06 | LoadAverage12 |
| func_8007E1FC | display | active | 13 | 0x8007E1FC | LIBGTE/MSC06 | LoadAverage0 |
| func_8007E4DC | display | active | 62 | 0x8007E4DC | LIBGTE/MTX_000 | MulMatrix0 |
| func_8007E74C | display | active | 85 | 0x8007E74C | LIBGTE/MTX_004 | ApplyMatrixLV |
| func_8007E8DC | display | active | 72 | 0x8007E8DC | LIBGTE/MTX_00A | ScaleMatrixL |
| func_8007EA0C | display | active | 75 | 0x8007EA0C | LIBGTE/MTX_01 | ApplyRotMatrixLV |
| func_8007EB4C | display | active | 62 | 0x8007EB4C | LIBGTE/MTX_03 | MulMatrix |
| func_8007ED6C | display | active | 16 | 0x8007ED6C | LIBGTE/MTX_05 | ApplyMatrix |
| func_8007F24C | display | active | 16 | 0x8007F24C | LIBGTE/SMP_03 | RotTransPers3 |
| func_8007F2DC | display | active | 23 | 0x8007F2DC | LIBGTE/CMB_00 | RotTransPers4 |
| single_game_getEnemyCharId | display | active | 0 | 0x8007FD5C | LIBGTE/RATAN | ratan2 |
| cdrom_GetCmdName | system | active | 0 | 0x80080180 | LIBCD/SYS | CdComstr |
| cdrom_GetResultName | system | active | 0 | 0x800801B4 | LIBCD/SYS | CdIntstr |
| func_80080258 | system | active | 25 | 0x80080258 | LIBCD/SYS | CdControl |
| func_80080390 | system | active | 23 | 0x80080390 | LIBCD/SYS | CdControlF |
| tslPolyF4Init | system | active | 8 | 0x800804BC | LIBCD/SYS | CdControlB |
| cdrom_FramesToBcd | system | active | 52 | 0x800806A4 | LIBCD/SYS | CdIntToPos |
| func_80080828 | system | active | 352 | 0x80080828 | LIBCD/BIOS | getintr (static) |
| cpu_side_move_dir_4 | system | active | 7 | 0x80080DB0 | LIBCD/BIOS | CD_sync |
| marionation_Exec | system | active | 56 | 0x80081030 | LIBCD/BIOS | CD_ready |
| tslTm2LoadImage | system | active | 262 | 0x800812FC | LIBCD/BIOS | CD_cw |
| cdrom_ConfigSPU | system | active | 0 | 0x80081880 | LIBCD/BIOS | CD_initvol |
| saEft01Init | system | active | 18 | 0x80081BB0 | LIBCD/BIOS | CD_datasync |
| tslTm2LoadImage_2 | system | active | 261 | 0x80082000 | LIBC2/PUTS | puts |
| saEft00Add | system | active | 18 | 0x8008241C | LIBCD/CDREAD | cd_read_retry (static) |
| saEft00Add_sub | system | parked | 4 | 0x80082630 | LIBCD/CDREAD | CdReadBreak |
| func_800826CC | system | active | 23 | 0x800826CC | LIBCD/CDREAD | CdRead |
| func_800827D0 | system | active | 0 | 0x800827D0 | LIBCD/CDREAD | CdReadSync |
| sys_SetTimer | ings2 | active | 0 | 0x800828B4 | LIBCD/CDREAD | CdReadMode |
| func_80082A14 | ings2 | parked | 0 | 0x80082A14 | LIBETC/VSYNC | v_wait (static) |
| func_80082C58 | ings2 | active | 0 | 0x80082C58 | LIBETC/INTR | startIntr (static) |
| func_80082D34 | ings2 | active | 295 | 0x80082D34 | LIBETC/INTR | trapIntr (static) |
| D_80083418 | ings2 | active | 13 | 0x80083418 | LIBETC/INTR_DMA | trapIntrDMA (static) |
| D_8008359C | ings2 | active | 20 | 0x8008359C | LIBETC/INTR_DMA | setIntrDMA (static) |
| func_80083A48 | ings2 | active | 19 | 0x80083A48 | LIBSND/SSINIT | _SsInit |
| saTan5TakeAnim2_2 | main | active | 19 | 0x80083C34 | LIBSND/SSSTART | _SsStart (static) |
| DispStuff | main | active | 207 | 0x80083E9C | LIBSND/SSSTART | SsStart |
| spu_ResetMotionEntry | main | active | 0 | 0x80085114 | LIBSND/NEXT | _SsSndNextSep |
| spu_SetMotionActive | main | active | 23 | 0x80085210 | LIBSND/REPLAY | _SsSndReplay |
| func_80085448 | main | active | 0 | 0x80085448 | LIBSND/SSSV | SsSetSerialVol |
| func_800856B0 | main | active | 65 | 0x800856B0 | LIBSND/TEMPO | _SsSndTempo |
| func_80086BFC | main | active | 40 | 0x80086BFC | LIBSND/VM_N2P | note2pitch2 |
| AddTbpOfst | main | parked | 3 | 0x80087E3C | LIBSND/VM_VSU | _SsVmVSetUp |
| saTan2Main | main | active | 245 | 0x800880E8 | LIBSND/VS_VH | SsVabOpenHeadWithMode (static) |
| spu_InitEx | main | parked | 0 | 0x800885CC | LIBSPU/S_INI | _SpuInit |
| func_80088740 | main | active | 56 | 0x80088740 | LIBSPU/SPU | _spu_init |
| DispUpdateStatusMessage | main | active | 204 | 0x800889D4 | LIBSPU/SPU | _spu_FwriteByIO (static) |
| saTan0GaugeDraw | main | active | 57 | 0x80088D0C | LIBSPU/SPU | _spu_t |
| func_800892F8 | main | active | 0 | 0x800892F8 | LIBSPU/S_Q | SpuQuit |
| spu_IrqHandler | main | active | 0 | 0x80089384 | LIBSPU/S_M_INIT | SpuInitMalloc |
| coli_HitPauseKatana | main | active | 58 | 0x800893D8 | LIBSPU/S_M_M | SpuMalloc |
| exec_game | main | active | 121 | 0x800896A0 | LIBSPU/S_M_INT | _spu_gcSPU |
| spu_DmaTransfer | main | parked | 4 | 0x800899A8 | LIBSPU/S_M_F | SpuFree |
| saEft03Start2 | main | active | 15 | 0x80089D60 | LIBSPU/S_SR | SpuSetReverb |
| func_80089E30 | main | active | 23 | 0x80089E30 | LIBSPU/S_M_UTIL | _SpuIsInAllocateArea |
| func_80089EB0 | main | active | 23 | 0x80089EB0 | LIBSPU/S_M_UTIL | _SpuIsInAllocateArea_ |
| func_80089F3C | main | active | 316 | 0x80089F3C | LIBSPU/S_SRMP | SpuSetReverbModeParam |
| md_game_check_change_main_mode_katinuki | main | parked | 3 | 0x8008A928 | LIBSPU/S_CRWA | SpuClearReverbWorkArea |
| func_8008AAD4 | main | active | 69 | 0x8008AAD4 | LIBSPU/S_SK | SpuSetKey |
| func_8008ADC4 | main | active | 0 | 0x8008ADC4 | LIBSPU/S_R | SpuRead/SpuWrite |
| func_8008AE7C | main | active | 0 | 0x8008AE7C | LIBSPU/S_STM | SpuSetTransferMode |
| func_8008AF9C | main | active | 279 | 0x8008AF9C | LIBSPU/S_SCA | SpuSetCommonAttr |
| func_8008B400 | main | active | 0 | 0x8008B400 | LIBSPU/SR_GAKS | SpuGetAllKeysStatus |
| func_8008BB24 | main | active | 39 | 0x8008BB24 | LIBSPU/S_N2P | _spu_note2pitch |
| func_8008BC60 | main | active | 64 | 0x8008BC60 | LIBSPU/S_N2P | _spu_pitch2note |
| func_8008BEA4 | main | active | 0 | 0x8008BEA4 | LIBCOMB/COMB | SioAnsyncRead (static) |
| cpu_side_move_dir_3 | main | active | 41 | 0x8008BF04 | LIBCOMB/COMB | SioSyncroRead (static) |
| func_8008C184 | main | active | 0 | 0x8008C184 | LIBCOMB/COMB | SioAnsyncWrite (static) |
| SetPacketData | main | active | 32 | 0x8008C1E8 | LIBCOMB/COMB | SioSyncroWrite (static) |
| func_8008C464 | main | parked | 761 | 0x8008C464 | LIBCOMB/COMB | _comb_control |

`(static)` = module-local symbol (not exported); `+0xNN` = address is inside that Sony function.

## (c) Near-matches (recompiled / different library revision -- NOT verbatim)

| lib/module | BB2 vaddr | words | differing unmasked words | verdict |
|---|---|---|---|---|
| LIBSND/UT_KEYV (SsUtKeyOnV, SsUtKeyOffV) | 0x80085A40..0x80085E4C | 259 | 4 (real opcode diffs: addu->subu x2, sll shift-amount x2) | newer LIBSND revision than 4.0; 3.3/3.5/3.6 match worse. Queue item `AllocBukiRmd` @0x80085A40 IS SsUtKeyOnV of that revision |
| LIBGS/GS_007 (GsInitVcount) | 0x80046AE8 | 14 | 1 | game func `snd_PlaySystemSe` -- coincidental (trivial code shape), NOT a library link |
| LIBGS/GS_111,GS_112 (GsDrawOt) | 0x80077A04 | 9 | 1 | game func `motion_state_reset_80077A04` -- coincidental |
| LIBSND/VM_DON | 0x8001B6E0 | 5 | 1 | too small; false positive in game code |

Two 9-word "verbatim" hits in game-code territory were likewise judged coincidental and EXCLUDED from all totals above:
LIBSPU/S_CB (_SpuCallback) @0x800469A0 = game func `snd_PlaySe`, LIBGS/GS_008/GS_009 (GsGetVcount) @0x80046B20 = `snd_StopSystemSe`
(trivial set-a-global / read-a-global shapes; the surrounding code is matched game source, and adjacent GS_007 fails full match).

## (d) Libraries checked with NO verbatim module in the EXE

| lib | note |
|---|---|
| LIBDS | 0/14 modules |
| LIBMATH | 0/48 |
| LIBSIO | 0/4 |
| LIBTAP | 0/1 |
| LIBPRESS | 0/5 |
| LIBGS | no genuine links (only the coincidental tiny hits above; GS_101/102/106/124/125 4-8-word stubs are byte-identical to LIBAPI/LIBSND stubs and subsumed) |
| LIBC | only SPRINTF + MEMMOVE -- byte-identical to the LIBC2 copies at the same addresses (game linked LIBC2) |
| LIBPAD, LIBHMD | NOT AVAILABLE in the sozud/psy-q mirror (404 in 3.3/3.5/3.6/4.0; repo only has those versions + cc_2.60) -- pad code here is LIBAPI PAD/SENDPAD/CHCLRPAD, which DID match |

## Unmatched gaps inside the library span (probable Sony code, unverifiable with available libs)

The library block 0x80083698..0x8008D070 (sound/sio) has ~15KB of unmatched code between verbatim
modules. 122 LIBSND and 93 LIBSPU 4.0 modules found no match, yet the gap positions sit exactly where
the LIBSND sequencer (SS*/VM_*) modules would link, and UT_KEYV near-matches with 4-word source diffs.
Conclusion: BB2 links a LIBSND/LIBSPU build newer than 4.0 (4.1-4.3 era) for those modules; the mirror
has no 4.1+ libs to prove it. 26 queue items fall in these gaps -- they are PROBABLE Sony sound-library
code but not verbatim-proven: func_800841E0, func_80084500, saTan4GaugeInit @0x80084974, func_80084A7C,
func_80085270, title_mv_exec2 @0x800858D0, AllocBukiRmd @0x80085A40 (= SsUtKeyOnV, near-proven),
func_80085FD8, func_80086014, func_80086080, func_80086130, tslGlobalMemFree_800861BC,
action_CheckHitZangeki, md_game_end, func_80086CF8, func_800871D4, func_800872A4, func_80087770,
func_80087CAC, func_80087D10, func_80087D58, coli_HitPauseKatana_2 @0x80089A48, saTan1MainJump @0x8008B488.
Also motion_Open/motion_Close/ang_hosei @0x800836C8-0x80083868 (gap between LIBETC and LIBSND blocks).

## Overlay note

The disc image contains a second PS-X EXE at raw offset 655,149,624 (load 0x801D8800, text 0x1E000,
pc 0x801DA084) -- the movie-overlay (MOVOVL) region prior work identified with its own LIBCD copy.
Not scanned here (out of scope); expect the same LIBCD/LIBETC/LIBGPU modules relinked at 0x801Dxxxx.

## Method caveats

- Verbatim = 100% of non-reloc-masked bits identical over the module's FULL .text. Reloc masking:
  HI16/LO16 imm16 (types 82/84), REL26 low 26 bits (74), FULL32 (16). No unknown reloc types were
  encountered in matched modules.
- 4-8-word modules carry weak evidence in isolation; every accepted tiny module is contiguous, in
  link order, with uniquely-anchored larger modules (LIBAPI syscall-stub chain at 0x80078948+,
  LIBGTE REG* chain, LIBCARD A7x chain, etc.). Tiny matches outside the library span were rejected
  (listed in (c)).
- Identical-code module pairs (S_R/S_W = SpuRead/SpuWrite; SSINIT_C/SSINIT_H; SEND/SSNOFF/SSQUIT/GS_106
  stub shape; UT_ROFF/S_I; UT_RON/S_IH; GS_101/102/124) cannot be distinguished by bytes -- both names
  are reported at the shared sites; the queue mapping shows both (e.g. func_8008ADC4 = SpuRead/SpuWrite,
  by link order almost certainly SpuWrite as SpuRead sits at 0x8008AD64).
- Only .text was located; library .data/.rdata live elsewhere (RCS id strings for sys.c 1.129,
  bios.c 1.86, intr.c 1.76 sit at 0x80015E28/0x80016274/0x80016328).
- Queue addresses resolved via asm/funcs glabels, build/bb2.map, and func_/D_-embedded hex; all 495
  queue items resolved.

## Files

- `tmp/libscan/matches.json` -- raw scan results, all libs/modules (incl. per-module status)
- `tmp/libscan/matches_versions.json` -- LIBSND/LIBGS 3.3/3.5/3.6 bracketing scan
- `tmp/libscan/queue_hits.json` -- regions, module placements, queue cross-reference
- `tmp/libscan/symbols.txt` -- full vaddr -> Sony symbol map (XDEFs + statics) for matched modules
- `tmp/libscan/scan.py`, `tmp/libscan/analyze.py` -- reproducible pipeline
