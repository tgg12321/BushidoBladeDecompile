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
