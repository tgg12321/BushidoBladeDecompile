# PsyQ Library Match Proposals

High-confidence proposals that match PsyQ stdlib/BIOS idioms. These are the lowest-risk renames in the corpus. The detection heuristics in `tools/propose_function_names.py` are conservative and require both shape signals AND tight loop body pairing.


- PsyQ memcpy/memset: **18**
- BIOS A0/B0/C0 jumptable wrappers: **38**
- Raw syscall/break wrappers: **3**

## BIOS jumptable wrappers

| address | current | proposed BIOS name | evidence |
|---|---|---|---|
| `0x80078948` | `func_80078948` | `bios_Exec` | bios_jumptable=bios_Exec |
| `0x80078958` | `func_80078958` | `bios__bu_init_A0` | bios_jumptable=bios__bu_init_A0; kengo_pattern=coli_RobColliScaleEditReset_80078958; sole_caller_path=pad_press_control_helper_80078958 |
| `0x80078968` | `func_80078968` | `bios_SetMem` | bios_jumptable=bios_SetMem; sole_caller_path=cpu_set_move_command_and_dir_for_no_action_2_helper_80078968 |
| `0x80078978` | `func_80078978` | `bios_OpenEvent` | bios_jumptable=bios_OpenEvent; kengo_pattern=coli_RobColliScaleEditReset_80078978 |
| `0x80078988` | `func_80078988` | `bios_CloseEvent` | bios_jumptable=bios_CloseEvent; kengo_pattern=coli_RobColliScaleEditReset_80078988 |
| `0x80078998` | `func_80078998` | `bios_TestEvent` | bios_jumptable=bios_TestEvent; kengo_pattern=coli_RobColliScaleEditReset_80078998 |
| `0x800789A8` | `func_800789A8` | `bios_EnableEvent` | bios_jumptable=bios_EnableEvent; kengo_pattern=coli_RobColliScaleEditReset_800789A8 |
| `0x800789E8` | `func_800789E8` | `bios_FileOpen_B` | bios_jumptable=bios_FileOpen_B; kengo_pattern=coli_RobColliScaleEditReset_800789E8 |
| `0x800789F8` | `func_800789F8` | `bios_FileRead_B` | bios_jumptable=bios_FileRead_B; kengo_pattern=coli_RobColliScaleEditReset_800789F8 |
| `0x80078A08` | `func_80078A08` | `bios_FileWrite_B` | bios_jumptable=bios_FileWrite_B; kengo_pattern=coli_RobColliScaleEditReset_80078A08 |
| `0x80078A18` | `func_80078A18` | `bios_FileClose_B` | bios_jumptable=bios_FileClose_B; kengo_pattern=coli_RobColliScaleEditReset_80078A18 |
| `0x80078A28` | `func_80078A28` | `bios_FormatDevice_B` | bios_jumptable=bios_FormatDevice_B; kengo_pattern=coli_RobColliScaleEditReset_80078A28 |
| `0x80078A38` | `func_80078A38` | `bios_firstfile_B` | bios_jumptable=bios_firstfile_B; kengo_pattern=coli_RobColliScaleEditReset_80078A38 |
| `0x80078A48` | `func_80078A48` | `bios_nextfile_B` | bios_jumptable=bios_nextfile_B; kengo_pattern=coli_RobColliScaleEditReset_80078A48 |
| `0x80078A58` | `func_80078A58` | `bios_ChangeClearPad` | bios_jumptable=bios_ChangeClearPad; kengo_pattern=coli_RobColliScaleEditReset_80078A58 |
| `0x80078F00` | `func_80078F00` | `bios_InitPad` | bios_jumptable=bios_InitPad; kengo_pattern=saTan0GaugeInit_80078F00 |
| `0x80078F10` | `func_80078F10` | `bios_StartPad` | bios_jumptable=bios_StartPad; kengo_pattern=saTan0GaugeInit_80078F10 |
| `0x80078F20` | `func_80078F20` | `bios_StopPad` | bios_jumptable=bios_StopPad; kengo_pattern=saTan0GaugeInit_80078F20; sole_caller_path=pad_Init_helper_80078F20 |
| `0x80078F30` | `func_80078F30` | `bios_OutdatedPadInitAndStart` | bios_jumptable=bios_OutdatedPadInitAndStart; kengo_pattern=saTan0GaugeInit_80078F30 |
| `0x80078F40` | `func_80078F40` | `bios_SysEnqIntRP` | bios_jumptable=bios_SysEnqIntRP; kengo_pattern=saTan0GaugeInit_80078F40 |
| `0x80078F50` | `func_80078F50` | `bios_SysDeqIntRP` | bios_jumptable=bios_SysDeqIntRP; kengo_pattern=saTan0GaugeInit_80078F50 |
| `0x8007A2F8` | `func_8007A2F8` | `bios_A0_0xAB_wrapper` | bios_jumptable=bios_A0_0xAB_wrapper |
| `0x8007A308` | `func_8007A308` | `bios_A0_0xAC_wrapper` | bios_jumptable=bios_A0_0xAC_wrapper |
| `0x8007A350` | `func_8007A350` | `bios__card_write` | bios_jumptable=bios__card_write |
| `0x8007A360` | `func_8007A360` | `bios__new_card` | bios_jumptable=bios__new_card |
| `0x8007A428` | `func_8007A428` | `bios_InitCard` | bios_jumptable=bios_InitCard |
| `0x8007A438` | `func_8007A438` | `bios_StartCard` | bios_jumptable=bios_StartCard |
| `0x8007A448` | `func_8007A448` | `bios_StopCard` | bios_jumptable=bios_StopCard |
| `0x8007DF10` | `func_8007DF10` | `bios_GPU_cw` | bios_jumptable=bios_GPU_cw |
| `0x8008008C` | `func_8008008C` | `bios_DeliverEvent` | bios_jumptable=bios_DeliverEvent |
| `0x80082AB0` | `func_80082AB0` | `bios_ChangeClearRCnt` | bios_jumptable=bios_ChangeClearRCnt |
| `0x800831D8` | `func_800831D8` | `bios_CdRemove_A0` | bios_jumptable=bios_CdRemove_A0; sole_caller_path=motion_make_table_helper_800831D8 |
| `0x800831F0` | `func_800831F0` | `bios_ReturnFromException` | bios_jumptable=bios_ReturnFromException |
| `0x80083200` | `func_80083200` | `bios_SetDefaultExitFromException` | bios_jumptable=bios_SetDefaultExitFromException |
| `0x80083210` | `func_80083210` | `bios_SetCustomExitFromException` | bios_jumptable=bios_SetCustomExitFromException |
| `0x8008386C` | `func_8008386C` | `bios_InitHeap` | bios_jumptable=bios_InitHeap; sole_caller_path=ang_hosei_helper_8008386C |
| `0x80089374` | `func_80089374` | `bios_DisableEvent` | bios_jumptable=bios_DisableEvent; kengo_pattern=saTan0GaugeInit_80089374 |
| `0x8008D050` | `func_8008D050` | `bios_AddDevice_B` | bios_jumptable=bios_AddDevice_B; kengo_pattern=saTan0GaugeInit_8008D050; address_neighborhood=g_local_8008D050 |

## PsyQ memcpy/memset

| address | current | proposed | evidence |
|---|---|---|---|
| `0x80016768` | `func_80016768` | `psyq_memset_80016768` | psyq_idiom=psyq_memset_80016768 |
| `0x80016C80` | `func_80016C80` | `psyq_memcpy_80016C80` | psyq_idiom=psyq_memcpy_80016C80; string_adjacent=eff_init_func_80016C80 |
| `0x8003043C` | `func_8003043C` | `psyq_memset_8003043C` | psyq_idiom=psyq_memset_8003043C; sole_caller_path=mario_test_Exec_helper_8003043C |
| `0x80035438` | `func_80035438` | `psyq_memcpy_80035438` | psyq_idiom=psyq_memcpy_80035438 |
| `0x80035FE0` | `func_80035FE0` | `psyq_memcpy_80035FE0` | psyq_idiom=psyq_memcpy_80035FE0 |
| `0x80038148` | `func_80038148` | `psyq_memset_80038148` | psyq_idiom=psyq_memset_80038148; sole_caller_path=pad_FuncAnalog_helper_80038148 |
| `0x800397A0` | `func_800397A0` | `psyq_memcpy_800397A0` | psyq_idiom=psyq_memcpy_800397A0 |
| `0x8003A42C` | `func_8003A42C` | `psyq_memset_8003A42C` | psyq_idiom=psyq_memset_8003A42C |
| `0x8005509C` | `func_8005509C` | `psyq_memset_8005509C` | psyq_idiom=psyq_memset_8005509C |
| `0x800550E8` | `func_800550E8` | `psyq_memcpy_800550E8` | psyq_idiom=psyq_memcpy_800550E8; sole_caller_path=cpu_get_move_pattern_table_number_helper_800550E8 |
| `0x80079194` | `func_80079194` | `psyq_memcpy_80079194` | psyq_idiom=psyq_memcpy_80079194 |
| `0x8007A28C` | `func_8007A28C` | `psyq_memcpy_8007A28C` | psyq_idiom=psyq_memcpy_8007A28C |
| `0x8007DEE4` | `func_8007DEE4` | `psyq_memset_8007DEE4` | psyq_idiom=psyq_memset_8007DEE4 |
| `0x8008339C` | `func_8008339C` | `psyq_memset_8008339C` | psyq_idiom=psyq_memset_8008339C |
| `0x80083644` | `func_80083644` | `psyq_memset_80083644` | psyq_idiom=psyq_memset_80083644 |
| `0x80086014` | `func_80086014` | `psyq_memcpy_80086014` | psyq_idiom=psyq_memcpy_80086014 |
| `0x80087F34` | `func_80087F34` | `psyq_memset_80087F34` | psyq_idiom=psyq_memset_80087F34 |
| `0x8008AE7C` | `func_8008AE7C` | `psyq_memset_8008AE7C` | psyq_idiom=psyq_memset_8008AE7C |

## Syscall / break wrappers

| address | current | proposed | evidence |
|---|---|---|---|
| `0x800164F8` | `func_800164F8` | `syscall_wrapper_break_800164F8` | syscall_wrapper=syscall_wrapper_break_800164F8 |
| `0x80083698` | `func_80083698` | `syscall_wrapper_break_80083698` | syscall_wrapper=syscall_wrapper_break_80083698 |
| `0x8008393C` | `func_8008393C` | `syscall_wrapper_break_8008393C` | syscall_wrapper=syscall_wrapper_break_8008393C |

