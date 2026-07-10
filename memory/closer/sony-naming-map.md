# Sony naming map (func/data → PsyQ canonical name)

One line each; append as adopted. Renames deferred to an owner-signed pass.

## Functions
- cdrom_GetCmdName → CdComstr (LIBCD/SYS)
- cdrom_GetResultName → CdIntstr (LIBCD/SYS)
- Vu0SetLightColMatrix_800801E8 → CdSync (LIBCD/SYS)
- Vu0SetLightColMatrix_80080208 → CdReady (LIBCD/SYS)
- tslTmlGetHeda → CdDataCallback (LIBCD/SYS)
- Vu0SetLightColMatrix_80080684 → CdReadyCallback-adjacent wrapper (LIBCD/SYS; saEft01Init = CD_datasync)
- cdrom_ConfigSPU → CD_initvol (LIBCD/BIOS)
- saEft00Add → cd_read_retry (LIBCD/CDREAD static)
- saEft00Add_sub → CdReadBreak (LIBCD/CDREAD)
- func_800826CC → CdRead (LIBCD/CDREAD)
- func_800827D0 → CdReadSync (LIBCD/CDREAD)
- sys_SetTimer → CdReadMode (LIBCD/CDREAD)
- spu_ResetMotionEntry → _SsSndNextSep (LIBSND/NEXT)
- D_80082050 (fn) → cb_read (LIBCD/CDREAD static)
- func_80080828 → getintr (LIBCD/BIOS static)
- cpu_side_move_dir_4 → CD_sync (LIBCD/BIOS)
- marionation_Exec → CD_ready (LIBCD/BIOS)
- tslTm2LoadImage → CD_cw (LIBCD/BIOS)
- tslTm2LoadImage_2 → puts (LIBC2/PUTS)
- saEft01Init → CD_datasync (LIBCD/BIOS)
- func_80080258 → CdControl, func_80080390 → CdControlF,
  tslPolyF4Init → CdControlB (LIBCD/SYS)

## Data (CDREAD module .data block)
- D_800A14CC → CD_ReadCallbackFunc
- D_800A14D0 → cdread state struct (SOTN D_80032DBC): +0 sectors, +4 buf,
  +8 p, +0xC mode, +0x10 size, +0x14 cnt, +0x18 t2, +0x1C t1, +0x20 pos,
  +0x24 cbsync, +0x28 cbready, +0x2C cbdata, +0x30 tslmode(=g_sys_timer)

## Data (LIBGPU SYS module, .data base 0x8009BE24)
- D_8009BE6C → gpu dispatch-table pointer (SOTN D_8002C260)
- D_8009BE70 → GPU_printf (XDEF at .data+0x4C)
- D_8009BE76 → debug level byte (v1.129 layout)
- 0x8009BEE0 → cached DISPENV (memset target in SetDispMask, +0x6A from
  debug byte — one Sony object; struct design pending)
- cdrom_FramesToBcd -> CdIntToPos (LIBCD/SYS)
- func_80089E30 -> _SpuIsInAllocateArea (LIBSPU/S_M_UTIL)
- func_80089EB0 -> _SpuIsInAllocateArea_ (LIBSPU/S_M_UTIL)
- spu_SetMotionActive -> _SsSndReplay (LIBSND/REPLAY)
- func_80083A48 -> _SsInit (LIBSND/SSINIT)
- func_80086BFC -> note2pitch2 (LIBSND/VM_N2P)
- saEft03Start2 -> SpuSetReverb (LIBSPU/S_SR)
- md_game_check_change_main_mode_katinuki -> SpuClearReverbWorkArea (LIBSPU/S_CRWA)
- spu_DmaTransfer -> SpuFree (LIBSPU/S_M_F)
- coli_HitPauseKatana -> SpuMalloc (LIBSPU/S_M_M)
- saTan5TakeAnim2_2 -> _SsStart (LIBSND/SSSTART; struct-adopted, byte-identical)
- SetBloodSpot -> SsSetTickMode-adjacent tick-env writer (LIBSND/SSTICK window)
- exec_game -> _spu_gcSPU (LIBSPU/S_M_INT; candidate banked at 30)

## Data (LIBSND/LIBSPU adopted this session)
- D_800A26CC -> _snd_seq_tick_env (SndSeqTickEnv struct: +0 unk0 tick mode, +4 unk4, +0xC unk12 saved cb, +0x10/11/12/13 u8 unk16..unk19)
- D_800A2D14 -> _spu_transferCallback (= g_spu_init_flag @0x800A2D14; volatile per Sony libspu_internal.h)
- D_800A2D3C -> D_80033560 (spu memList top index; = g_spu_voice_key_b)
- D_800A2D40 -> _spu_memList (= g_spu_voice_key_c; SpuMemRec {u32 addr; u32 size;})
- D_800A2D38 -> D_8003355C (spu memList block count; = g_spu_voice_key_a)
- D_800A2D04 -> _spu_mem_mode_plus, D_800A2D0C -> _spu_mem_mode_unitM
- D_800A2D44 -> _spu_rev_startaddr[] , D_800A2884 -> _spu_rev_offsetaddr, D_800A2880 -> _spu_rev_reserve_wa
- D_800A2CDC -> _spu_RXX (SPU register block pointer; spucnt at +0x1AA, MMIO volatile)
- D_800A287C -> _spu_rev_flag, D_800A2CF8 -> _spu_transMode, D_800A2870 -> _spu_EVdma, D_800A28D4 -> reverb-clear zero buffer (D_800330F8)
- D_80101BC8 -> _svm_tn (VagAtr*), D_801027F7 -> _svm_cur.field_7_fake_program, D_801027FC -> _svm_cur.field_C_vag_idx, D_800A26E4 -> pitch table (D_80032F14)
- func_8007A28C -> memmove (LIBC2/MEMMOVE)
- func_80082C58 -> startIntr (LIBETC/INTR static)
- func_80083220 -> setjmp (LIBC2/SETJMP) — RENAMED in-tree 2026-07-10 (load-bearing: GCC returns-twice keys on the literal name)
- func_80082A14 -> v_wait (LIBETC/VSYNC static; adopted 2026-07-10 s4, Ruling 3)
- func_80078E58 -> _Pad1 (LIBAPI/PAD static; adopted 2026-07-10 s4, Ruling 3)
- func_8008BEA4 -> SioAnsyncRead, func_8008C184 -> SioAnsyncWrite (LIBCOMB statics; adopted 2026-07-10 s4, granted volatile)
- HandleSio (SIO ISR core) = static @0x8008C9F4 inside asm/funcs/func_8008C464.s; AddCOMB @0x8008BE04
- D_800A1578 -> intrEnv (intrEnv_t, LIBETC intr.c), D_800A15B4 = intrEnv.buf[1] (JB_SP)
- D_800A2604 -> i_stat ptr static, g_sys_irq_counter -> i_mask (g_InterruptMask) ptr static, D_800A260C -> d_pcr ptr static (LIBETC intr.c)
- D_80083418 -> trapIntrDMA, D_8008359C -> setIntrDMA (LIBETC/INTR_DMA statics; adopted 2026-07-10 s4)
- conv_matrix_rotation -> startIntrDMA (LIBETC/INTR_DMA; already matched, respelled for the volatile-MMIO-pointee decl)
- D_800A263C -> DMA Interrupt Register ptr static (0x1F8010F4; Sony declares volatile u_long*), D_800A2640 -> DMA callback table[8], D_800A2660 -> DMA base regs ptr (0x1F801080)
- saEft01Init -> CD_datasync (LIBCD/BIOS; NOT adopted -- window is the twins' knot, see phase3 s4 findings)
D_800A2CD4 -> _spu_env (LIBSPU S_SK relocs)
D_800F7420 -> _spu_RQ (u16[4]; D_800F7424 = _spu_RQ+4, same object)
D_800A28A0 -> _spu_RQmask
D_800A289C -> _spu_RQvoice
D_800A2874 -> _spu_keystat
D_800A2CDC -> _spu_RXX
func_8008AAD4 -> SpuSetKey
