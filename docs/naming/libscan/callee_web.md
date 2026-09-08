# libscan-xref callee web (accepted verbatim placements, main block)

| Sony callee | EXE target | fn start? | our name(s) | called from |
|---|---|---|---|---|
| AddDrv | 0x8008D050 | yes | AddDrv | LIBCOMB/COMB@0x8008BE04 |
| CD_cw | 0x800812FC | yes | CD_cw | LIBCD/SYS@0x8008009C |
| CD_datasync | 0x80081BB0 | yes | CD_datasync | LIBCD/SYS@0x8008009C |
| CD_flush | 0x800817A0 | NO (interior of CD_cw) | CD_flush | LIBCD/SYS@0x8008009C |
| CD_getsector | 0x80081D1C | NO (interior of CD_datasync) | CD_getsector | LIBCD/SYS@0x8008009C |
| CD_getsector2 | 0x80081E1C | NO (interior of CD_datasync) | CD_getsector2 | LIBCD/SYS@0x8008009C |
| CD_init | 0x800819C4 | NO (interior of CD_cw) | CD_init | LIBCD/SYS@0x8008009C |
| CD_initintr | 0x80081974 | NO (interior of CD_cw) | CD_initintr | LIBCD/SYS@0x8008009C |
| CD_initvol | 0x80081880 | NO (interior of CD_cw) | CD_initvol | LIBCD/SYS@0x8008009C |
| CD_ready | 0x80081030 | yes | CD_ready | LIBCD/SYS@0x8008009C |
| CD_sync | 0x80080DB0 | yes | CD_sync | LIBCD/SYS@0x8008009C |
| CD_vol | 0x80081718 | NO (interior of CD_cw) | CD_vol | LIBCD/SYS@0x8008009C |
| CdControl | 0x80080258 | yes | CdControl | LIBCD/CDREAD@0x80082050 |
| CdControlB | 0x800804BC | yes | CdControlB | LIBCD/CDREAD@0x80082050 |
| CdControlF | 0x80080390 | yes | CdControlF | LIBCD/CDREAD@0x80082050 |
| CdDataCallback | 0x80080660 | yes | CdDataCallback | LIBCD/CDREAD@0x80082050 |
| CdDataSync | 0x80080684 | NO (interior of CdDataCallback) | CdDataSync | LIBCD/CDREAD@0x80082050 |
| CdFlush | 0x80080148 | yes | CdFlush | LIBCD/CDREAD@0x80082050 |
| CdGetSector | 0x80080620 | yes | CdGetSector | LIBCD/CDREAD@0x80082050 |
| CdGetSector2 | 0x80080640 | NO (interior of CdGetSector) | CdGetSector2 | LIBCD/CDREAD@0x80082050 |
| CdLastPos | 0x800800CC | yes | CdLastPos | LIBCD/CDREAD@0x80082050 |
| CdMode | 0x800800AC | yes | CdMode | LIBCD/CDREAD@0x80082050 |
| CdPosToInt | 0x800807A8 | NO (interior of CdDataCallback) | CdPosToInt | LIBCD/CDREAD@0x80082050 |
| CdReadCallback | 0x8008289C | yes | CdReadCallback | LIBCD/EVENT@0x8007FF7C |
| CdReadMode | 0x800828B4 | yes | CdReadMode | LIBCD/EVENT@0x8007FF7C |
| CdReady | 0x80080208 | NO (interior of CdIntstr) | CdReady | LIBCD/CDREAD@0x80082050 |
| CdReadyCallback | 0x80080240 | yes | CdReadyCallback | LIBCD/CDREAD@0x80082050; LIBCD/EVENT@0x8007FF7C |
| CdReset | 0x800800DC | NO (interior of CdLastPos) | CdReset | LIBCD/EVENT@0x8007FF7C |
| CdStatus | 0x8008009C | yes | CdStatus | LIBCD/CDREAD@0x80082050 |
| CdSyncCallback | 0x80080228 | yes | CdSyncCallback | LIBCD/CDREAD@0x80082050; LIBCD/EVENT@0x8007FF7C |
| ChangeClearPAD | 0x80078A58 | yes | ChangeClearPAD | LIBAPI/PAD@0x80078BE0; LIBCARD/INIT@0x8007A370; LIBETC/INTR@0x80082AC0; LIBETC/VSYNC@0x800828CC |
| ChangeClearRCnt | 0x80082AB0 | yes | ChangeClearRCnt | LIBETC/INTR@0x80082AC0; LIBETC/VSYNC@0x800828CC |
| CheckCallback | 0x80082C14 | NO (interior of RestartCallback) | CheckCallback | LIBCD/BIOS@0x80080828 |
| CloseEvent | 0x80078988 | yes | CloseEvent | LIBSPU/S_Q@0x800892F8 |
| DMACallback | 0x80082B20 | NO (interior of ResetCallback) | DMACallback | LIBCD/SYS@0x8008009C; LIBGPU/SYS@0x8007AE7C; LIBSPU/S_DCB@0x800892D4 |
| DelDrv | 0x8008D060 | yes | DelDrv | LIBCOMB/COMB@0x8008BE04 |
| DeliverEvent | 0x8008008C | yes | DeliverEvent | LIBCD/EVENT@0x8007FF7C; LIBCOMB/COMB@0x8008BE04; LIBSPU/SPU@0x80088740 |
| DisableEvent | 0x80089374 | yes | DisableEvent | LIBSPU/S_Q@0x800892F8 |
| DisablePAD | 0x80078F74 | yes | DisablePAD | LIBAPI/PAD@0x80078BE0 |
| EnableEvent | 0x800789A8 | yes | EnableEvent | LIBSPU/S_INI@0x800885CC |
| EnablePAD | 0x80078F60 | yes | EnablePAD | LIBAPI/PAD@0x80078BE0 |
| EnterCriticalSection | 0x800789B8 | yes | EnterCriticalSection | LIBAPI/CHCLRPAD@0x800790C0; LIBAPI/PAD@0x80078BE0; LIBAPI/PATCH@0x80078F60; LIBAPI/SENDPAD@0x80079000; LIBCARD/END@0x8007A458; LIBCARD/INIT@0x8007A370; LIBCOMB/ |
| ExitCriticalSection | 0x800789C8 | yes | ExitCriticalSection | LIBAPI/CHCLRPAD@0x800790C0; LIBAPI/PAD@0x80078BE0; LIBAPI/SENDPAD@0x80079000; LIBCARD/END@0x8007A458; LIBCARD/INIT@0x8007A370; LIBCOMB/COMB@0x8008BE04; LIBETC/I |
| FlushCache | 0x80078FF0 | yes | FlushCache | LIBAPI/CHCLRPAD@0x800790C0; LIBAPI/PATCH@0x80078F60; LIBAPI/SENDPAD@0x80079000; LIBCARD/END@0x8007A458; LIBCOMB/COMB@0x8008BE04; LIBGTE/PATCHGTE@0x8007FEDC |
| GPU_cw | 0x8007DF10 | yes | GPU_cw | LIBGPU/SYS@0x8007AE7C |
| GetClut | 0x8007A7C4 | NO (interior of gpu_LoadClut16) | GetClut | LIBGPU/EXT@0x8007A4D8 |
| GetTPage | 0x8007A788 | NO (interior of gpu_LoadClut16) | GetTPage | LIBGPU/EXT@0x8007A4D8 |
| GetVideoMode | 0x80083688 | yes | sys_GetVideoMode, GetVideoMode | LIBGPU/EXT@0x8007A4D8; LIBGPU/SYS@0x8007AE7C; LIBSND/SSTICK@0x80085544 |
| HookEntryInt | 0x80083210 | yes | HookEntryInt | LIBETC/INTR@0x80082AC0 |
| InitCARD2 | 0x8007A428 | yes | InitCARD2 | LIBCARD/INIT@0x8007A370 |
| InitPAD2 | 0x80078F00 | yes | InitPAD2 | LIBAPI/PAD@0x80078BE0 |
| InterruptCallback | 0x80082AF0 | NO (interior of ResetCallback) | InterruptCallback | LIBCD/BIOS@0x80080828; LIBETC/INTR_DMA@0x800833C8; LIBETC/INTR_VB@0x800832A0; LIBSND/SSEND@0x80083954; LIBSND/SSSTART@0x80083C34 |
| LoadImage | 0x8007B600 | yes | LoadImage | LIBGPU/EXT@0x8007A4D8 |
| OpenEvent | 0x80078978 | yes | OpenEvent | LIBSPU/S_INI@0x800885CC |
| PAD_init2 | 0x80078F30 | yes | PAD_init2 | LIBAPI/PAD@0x80078BE0 |
| ReadInitPadFlag | 0x80078BF0 | yes | ReadInitPadFlag | LIBCARD/INIT@0x8007A370 |
| ResetCallback | 0x80082AC0 | yes | ResetCallback | LIBCD/BIOS@0x80080828; LIBGPU/SYS@0x8007AE7C; LIBSPU/S_INI@0x800885CC |
| ResetEntryInt | 0x80083200 | yes | ResetEntryInt | LIBETC/INTR@0x80082AC0 |
| ResetGraph | 0x8007AE7C | yes | ResetGraph | LIBCOMB/COMB@0x8008BE04 |
| ResetRCnt | 0x80078BA8 | yes | ResetRCnt | LIBSND/SSSTART@0x80083C34 |
| ReturnFromException | 0x800831F0 | yes | ReturnFromException | LIBETC/INTR@0x80082AC0 |
| SetIntrMask | 0x80082C3C | yes | SetIntrMask | LIBGPU/SYS@0x8007AE7C |
| SetRCnt | 0x80078A68 | yes | SetRCnt | LIBSND/SSSTART@0x80083C34 |
| SpuClearReverbWorkArea | 0x8008A928 | yes | SpuClearReverbWorkArea | LIBSPU/S_SRMP@0x80089F3C |
| SpuFree | 0x800899A8 | yes | spu_DmaTransfer, SpuFree | LIBSND/VS_VAB@0x80087F64 |
| SpuIsTransferCompleted | 0x8008AEB0 | yes | SpuIsTransferCompleted | LIBSND/VS_VTC@0x80088584 |
| SpuMalloc | 0x800893D8 | yes | SpuMalloc | LIBSND/VS_VH@0x80088058 |
| SpuSetCommonAttr | 0x8008AF9C | yes | SpuSetCommonAttr | LIBSND/SSSATTR@0x80083B50; LIBSND/SSSMV@0x80083BE4; LIBSND/SSSV@0x80085448 |
| SpuSetReverb | 0x80089D60 | yes | SpuSetReverb | LIBSND/UT_REV@0x80085EE4 |
| SpuSetReverbModeParam | 0x80089F3C | yes | SpuSetReverbModeParam | LIBSND/UT_RDEP@0x80085E4C; LIBSND/UT_REV@0x80085EE4 |
| SpuSetTransferMode | 0x8008AE7C | yes | SpuSetTransferMode | LIBSND/VS_VTB@0x800884C4 |
| SpuSetTransferStartAddr | 0x8008AE24 | yes | SpuSetTransferStartAddr | LIBSND/VS_VTB@0x800884C4 |
| SpuWrite | 0x8008ADC4 | yes | SpuWrite | LIBSND/VS_VTB@0x800884C4 |
| StartCARD2 | 0x8007A438 | yes | StartCARD2 | LIBCARD/INIT@0x8007A370 |
| StartPAD2 | 0x80078F10 | yes | StartPAD2 | LIBAPI/PAD@0x80078BE0 |
| StopCARD2 | 0x8007A448 | yes | StopCARD2 | LIBCARD/INIT@0x8007A370 |
| StopPAD2 | 0x80078F20 | yes | StopPAD2 | LIBAPI/PAD@0x80078BE0 |
| SysDeqIntRP | 0x80078F50 | yes | SysDeqIntRP | LIBAPI/PAD@0x80078BE0; LIBCOMB/COMB@0x8008BE04 |
| SysEnqIntRP | 0x80078F40 | yes | SysEnqIntRP | LIBAPI/PAD@0x80078BE0; LIBCOMB/COMB@0x8008BE04 |
| TestEvent | 0x80078998 | yes | TestEvent | LIBSPU/S_ITC@0x8008AEB0 |
| VSync | 0x800828CC | yes | VSync | LIBCD/BIOS@0x80080828; LIBCD/CDREAD@0x80082050; LIBGPU/SYS@0x8007AE7C |
| VSyncCallback | 0x80082B50 | NO (interior of ResetCallback) | VSyncCallback | LIBSND/SSEND@0x80083954; LIBSND/SSSTART@0x80083C34 |
| WaitEvent | 0x8008AAC4 | yes | WaitEvent | LIBSPU/S_CRWA@0x8008A928 |
| _96_remove | 0x800831D8 | yes | _96_remove | LIBETC/INTR@0x80082AC0 |
| _ExitCard | 0x8007A458 | yes | _ExitCard | LIBCARD/INIT@0x8007A370 |
| _SpuDataCallback | 0x800892D4 | yes | spu_SetCallback, _SpuDataCallback | LIBSPU/S_INI@0x800885CC; LIBSPU/S_Q@0x800892F8 |
| _SpuIsInAllocateArea_ | 0x80089EB0 | yes | _SpuIsInAllocateArea_ | LIBSPU/S_CRWA@0x8008A928; LIBSPU/S_SR@0x80089D60; LIBSPU/S_SRMP@0x80089F3C |
| _SpuSetAnyVoice | 0x80089A48 | yes | func_80089A48 **<- differs** | LIBSPU/S_SNV@0x80089A24; LIBSPU/S_SRV@0x8008A904 |
| _SsSndCrescendo | 0x800841E0 | yes | func_800841E0, DispStuff_helper_800841E0 **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _SsSndDecrescendo | 0x80084500 | yes | func_80084500, DispStuff_helper_80084500 **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _SsSndPause | 0x800848AC | yes | spu_SetMotionState **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _SsSndPlay | 0x80084948 | yes | spu_SetMotionCallback, _SsSndPlay | LIBSND/SSCALL@0x80083F6C |
| _SsSndReplay | 0x80085210 | yes | spu_SetMotionActive **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _SsSndStop | 0x80085270 | yes | _SsSndStop | LIBSND/SSCALL@0x80083F6C |
| _SsSndTempo | 0x800856B0 | yes | _SsSndTempo | LIBSND/SSCALL@0x80083F6C |
| _SsVmDamperOff | 0x800863CC | yes | spu_ResetCounter, set_global_800863CC **<- differs** | LIBSND/SSSTOP@0x80085270 |
| _SsVmFlush | 0x800863DC | yes | func_800863DC **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _SsVmInit | 0x80086818 | yes | func_80086818 **<- differs** | LIBSND/SSINIT@0x80083A48 |
| _SsVmSeqKeyOff | 0x80087DA0 | yes | spu_NotifyChannel, find_id_in_table_80087DA0 **<- differs** | LIBSND/PAUSE@0x800848AC; LIBSND/SSSTOP@0x80085270 |
| _card_write | 0x8007A350 | yes | _card_write | LIBCARD/CARD@0x8007A318 |
| _new_card | 0x8007A360 | yes | _new_card | LIBCARD/CARD@0x8007A318 |
| _patch_gte | 0x8007FEDC | yes | _patch_gte | LIBGTE/MSC00@0x8007E08C |
| _patch_pad | 0x80078F88 | yes | _patch_pad | LIBAPI/PAD@0x80078BE0 |
| _remove_ChgclrPAD | 0x800790C0 | yes | _remove_ChgclrPAD | LIBAPI/PAD@0x80078BE0 |
| _send_pad | 0x80079028 | yes | _send_pad | LIBAPI/PAD@0x80078BE0 |
| _spu_FsetRXX | 0x8008908C | yes | _spu_FsetRXX | LIBSPU/S_INI@0x800885CC; LIBSPU/S_SRMP@0x80089F3C |
| _spu_FsetRXXa | 0x800890D4 | yes | _spu_FsetRXXa | LIBSPU/S_STSA@0x8008AE24 |
| _spu_gcSPU | 0x800896A0 | yes | _spu_gcSPU | LIBSPU/S_M_F@0x800899A8; LIBSPU/S_M_M@0x800893D8 |
| _spu_getInTransfer | 0x8008AF84 | yes | _spu_getInTransfer | LIBSND/VS_VH@0x80088058 |
| _spu_init | 0x80088740 | yes | _spu_init | LIBSPU/S_INI@0x800885CC |
| _spu_setInTransfer | 0x8008AF58 | yes | _spu_setInTransfer | LIBSND/VS_VFB@0x80087FE8; LIBSND/VS_VH@0x80088058; LIBSND/VS_VTB@0x800884C4 |
| _spu_setReverbAttr | 0x8008A434 | yes | _spu_setReverbAttr | LIBSPU/S_SRMP@0x80089F3C |
| _spu_t | 0x80088D0C | yes | _spu_t | LIBSPU/S_CRWA@0x8008A928 |
| memchr | 0x8007992C | yes | memchr | LIBC/SPRINTF@0x80079A30; LIBC2/PRNT@0x80079244; LIBC2/SPRINTF@0x80079A30 |
| memcpy | 0x80079120 | yes | memcpy | LIBGPU/SYS@0x8007AE7C |
| memmove | 0x8007A28C | yes | memmove | LIBC/SPRINTF@0x80079A30; LIBC2/SPRINTF@0x80079A30 |
| printf | 0x80079208 | yes | printf | LIBCD/BIOS@0x80080828; LIBCD/EVENT@0x8007FF7C; LIBETC/INTR@0x80082AC0; LIBETC/INTR_DMA@0x800833C8; LIBGPU/SYS@0x8007AE7C; LIBSPU/SPU@0x80088740 |
| prnt | 0x80079244 | yes | prnt | LIBC2/PRINTF@0x80079208 |
| putchar | 0x8007997C | yes | putchar | LIBC2/PRNT@0x80079244; LIBC2/PUTS@0x80082000 |
| puts | 0x80082000 | yes | puts | LIBCD/BIOS@0x80080828; LIBCD/CDREAD@0x80082050; LIBETC/VSYNC@0x800828CC |
| setjmp | 0x80083220 | yes | setjmp | LIBETC/INTR@0x80082AC0 |
| startIntrDMA | 0x800833C8 | yes | startIntrDMA | LIBETC/INTR@0x80082AC0 |
| startIntrVSync | 0x800832A0 | yes | startIntrVSync | LIBETC/INTR@0x80082AC0 |
| strlen | 0x800791D8 | yes | strlen | LIBC/SPRINTF@0x80079A30; LIBC2/PRNT@0x80079244; LIBC2/SPRINTF@0x80079A30 |
| write | 0x80078A08 | yes | write | LIBC2/PUTCHAR@0x8007997C |
