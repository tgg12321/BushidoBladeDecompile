# Naming Proposals -- High confidence
**High confidence**: deterministic pattern (BIOS jumptable, raw syscall, data-as-code, recognizable PsyQ stdlib idiom), pure single-load getter of an already-named global, literal empty stub, OR a Kengo `name-unique` match with diff <=1 instruction. These are the lowest-risk renames; review and apply in small batches with SHA1 verification.

**Risk**: BIOS A0/B0/C0 lookups are deterministic from the trampoline shape; PsyQ memcpy/memset proposals are address-suffixed to avoid name collisions if multiple wrappers exist for the same primitive.

**Workflow**: copy the proposed name into `named_syms.txt`, run `make setup && make`, verify SHA1 unchanged, commit.

Total High: **75**

## Primary evidence: `bios_jumptable` (38)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80078948` | `func_80078948` | `bios_Exec` | bios_jumptable=bios_Exec | [md](evidence/func_80078948.md) |
| `0x80078958` | `func_80078958` | `bios__bu_init_A0` | bios_jumptable=bios__bu_init_A0; kengo_pattern=coli_RobColliScaleEditReset_80078958; sole_caller_path=pad_press_control_helper_80078958 | [md](evidence/func_80078958.md) |
| `0x80078968` | `func_80078968` | `bios_SetMem` | bios_jumptable=bios_SetMem; sole_caller_path=cpu_set_move_command_and_dir_for_no_action_2_helper_80078968 | [md](evidence/func_80078968.md) |
| `0x80078978` | `func_80078978` | `bios_OpenEvent` | bios_jumptable=bios_OpenEvent; kengo_pattern=coli_RobColliScaleEditReset_80078978 | [md](evidence/func_80078978.md) |
| `0x80078988` | `func_80078988` | `bios_CloseEvent` | bios_jumptable=bios_CloseEvent; kengo_pattern=coli_RobColliScaleEditReset_80078988 | [md](evidence/func_80078988.md) |
| `0x80078998` | `func_80078998` | `bios_TestEvent` | bios_jumptable=bios_TestEvent; kengo_pattern=coli_RobColliScaleEditReset_80078998 | [md](evidence/func_80078998.md) |
| `0x800789A8` | `func_800789A8` | `bios_EnableEvent` | bios_jumptable=bios_EnableEvent; kengo_pattern=coli_RobColliScaleEditReset_800789A8 | [md](evidence/func_800789A8.md) |
| `0x800789E8` | `func_800789E8` | `bios_FileOpen_B` | bios_jumptable=bios_FileOpen_B; kengo_pattern=coli_RobColliScaleEditReset_800789E8 | [md](evidence/func_800789E8.md) |
| `0x800789F8` | `func_800789F8` | `bios_FileRead_B` | bios_jumptable=bios_FileRead_B; kengo_pattern=coli_RobColliScaleEditReset_800789F8 | [md](evidence/func_800789F8.md) |
| `0x80078A08` | `func_80078A08` | `bios_FileWrite_B` | bios_jumptable=bios_FileWrite_B; kengo_pattern=coli_RobColliScaleEditReset_80078A08 | [md](evidence/func_80078A08.md) |
| `0x80078A18` | `func_80078A18` | `bios_FileClose_B` | bios_jumptable=bios_FileClose_B; kengo_pattern=coli_RobColliScaleEditReset_80078A18 | [md](evidence/func_80078A18.md) |
| `0x80078A28` | `func_80078A28` | `bios_FormatDevice_B` | bios_jumptable=bios_FormatDevice_B; kengo_pattern=coli_RobColliScaleEditReset_80078A28 | [md](evidence/func_80078A28.md) |
| `0x80078A38` | `func_80078A38` | `bios_firstfile_B` | bios_jumptable=bios_firstfile_B; kengo_pattern=coli_RobColliScaleEditReset_80078A38 | [md](evidence/func_80078A38.md) |
| `0x80078A48` | `func_80078A48` | `bios_nextfile_B` | bios_jumptable=bios_nextfile_B; kengo_pattern=coli_RobColliScaleEditReset_80078A48 | [md](evidence/func_80078A48.md) |
| `0x80078A58` | `func_80078A58` | `bios_ChangeClearPad` | bios_jumptable=bios_ChangeClearPad; kengo_pattern=coli_RobColliScaleEditReset_80078A58 | [md](evidence/func_80078A58.md) |
| `0x80078F00` | `func_80078F00` | `bios_InitPad` | bios_jumptable=bios_InitPad; kengo_pattern=saTan0GaugeInit_80078F00 | [md](evidence/func_80078F00.md) |
| `0x80078F10` | `func_80078F10` | `bios_StartPad` | bios_jumptable=bios_StartPad; kengo_pattern=saTan0GaugeInit_80078F10 | [md](evidence/func_80078F10.md) |
| `0x80078F20` | `func_80078F20` | `bios_StopPad` | bios_jumptable=bios_StopPad; kengo_pattern=saTan0GaugeInit_80078F20; sole_caller_path=pad_Init_helper_80078F20 | [md](evidence/func_80078F20.md) |
| `0x80078F30` | `func_80078F30` | `bios_OutdatedPadInitAndStart` | bios_jumptable=bios_OutdatedPadInitAndStart; kengo_pattern=saTan0GaugeInit_80078F30 | [md](evidence/func_80078F30.md) |
| `0x80078F40` | `func_80078F40` | `bios_SysEnqIntRP` | bios_jumptable=bios_SysEnqIntRP; kengo_pattern=saTan0GaugeInit_80078F40 | [md](evidence/func_80078F40.md) |
| `0x80078F50` | `func_80078F50` | `bios_SysDeqIntRP` | bios_jumptable=bios_SysDeqIntRP; kengo_pattern=saTan0GaugeInit_80078F50 | [md](evidence/func_80078F50.md) |
| `0x8007A2F8` | `func_8007A2F8` | `bios_A0_0xAB_wrapper` | bios_jumptable=bios_A0_0xAB_wrapper | [md](evidence/func_8007A2F8.md) |
| `0x8007A308` | `func_8007A308` | `bios_A0_0xAC_wrapper` | bios_jumptable=bios_A0_0xAC_wrapper | [md](evidence/func_8007A308.md) |
| `0x8007A350` | `func_8007A350` | `bios__card_write` | bios_jumptable=bios__card_write | [md](evidence/func_8007A350.md) |
| `0x8007A360` | `func_8007A360` | `bios__new_card` | bios_jumptable=bios__new_card | [md](evidence/func_8007A360.md) |
| `0x8007A428` | `func_8007A428` | `bios_InitCard` | bios_jumptable=bios_InitCard | [md](evidence/func_8007A428.md) |
| `0x8007A438` | `func_8007A438` | `bios_StartCard` | bios_jumptable=bios_StartCard | [md](evidence/func_8007A438.md) |
| `0x8007A448` | `func_8007A448` | `bios_StopCard` | bios_jumptable=bios_StopCard | [md](evidence/func_8007A448.md) |
| `0x8007DF10` | `func_8007DF10` | `bios_GPU_cw` | bios_jumptable=bios_GPU_cw | [md](evidence/func_8007DF10.md) |
| `0x8008008C` | `func_8008008C` | `bios_DeliverEvent` | bios_jumptable=bios_DeliverEvent | [md](evidence/func_8008008C.md) |
| `0x80082AB0` | `func_80082AB0` | `bios_ChangeClearRCnt` | bios_jumptable=bios_ChangeClearRCnt | [md](evidence/func_80082AB0.md) |
| `0x800831D8` | `func_800831D8` | `bios_CdRemove_A0` | bios_jumptable=bios_CdRemove_A0; sole_caller_path=motion_make_table_helper_800831D8 | [md](evidence/func_800831D8.md) |
| `0x800831F0` | `func_800831F0` | `bios_ReturnFromException` | bios_jumptable=bios_ReturnFromException | [md](evidence/func_800831F0.md) |
| `0x80083200` | `func_80083200` | `bios_SetDefaultExitFromException` | bios_jumptable=bios_SetDefaultExitFromException | [md](evidence/func_80083200.md) |
| `0x80083210` | `func_80083210` | `bios_SetCustomExitFromException` | bios_jumptable=bios_SetCustomExitFromException | [md](evidence/func_80083210.md) |
| `0x8008386C` | `func_8008386C` | `bios_InitHeap` | bios_jumptable=bios_InitHeap; sole_caller_path=ang_hosei_helper_8008386C | [md](evidence/func_8008386C.md) |
| `0x80089374` | `func_80089374` | `bios_DisableEvent` | bios_jumptable=bios_DisableEvent; kengo_pattern=saTan0GaugeInit_80089374 | [md](evidence/func_80089374.md) |
| `0x8008D050` | `func_8008D050` | `bios_AddDevice_B` | bios_jumptable=bios_AddDevice_B; kengo_pattern=saTan0GaugeInit_8008D050; address_neighborhood=g_local_8008D050 | [md](evidence/func_8008D050.md) |

## Primary evidence: `manual_review` (23)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80017748` | `func_80017748` | `math_Distance3D_80017748` | vec3_magnitude_w_overflow_guard;legacy_rename_match | [md](evidence/func_80017748.md) |
| `0x80035480` | `func_80035480` | `scene_teardown_variant_80035480` | manual_review=Direct sibling of scene_teardown_80035DC8 (named in commit b6e72b8 as a misnomer-replacement alias). Same player_Destroy(0,1) + disp_load_config_from_buf(0x80118800) shape, but adds a few extras: conditional func_8003A41C call, conditional obj_InitAll + D_800A390E = -1 based o... | [md](evidence/func_80035480.md) |
| `0x80037774` | `func_80037774` | `memcard_event_pool_close_80037774` | manual_review=closes all 8 memcard events (4 in class 0xF4000001 + 4 in class 0xF0000011) opened by pad_press_control (which itself is a misnomer -- it does memcard init); was named game_event_shutdown_* before the OpenEvent owner was identified | [md](evidence/func_80037774.md) |
| `0x80037804` | `func_80037804` | `memcard_event_wait_class0xF4000001_with_timeout_80037804` | manual_review=spin-poll the 4 class-0xF4000001 events (D_800A37DC/F0/FC/3800) with 2-sec timeout via D_800A3924 counter; these are the memcard I/O-complete class opened by pad_press_control | [md](evidence/func_80037804.md) |
| `0x80037964` | `func_80037964` | `memcard_event_wait_class0xF0000011_80037964` | manual_review=spin-poll the 4 class-0xF0000011 events (D_800A3838/3C/48/50); these are the memcard error/notification class opened by pad_press_control; was named game_event_wait_any_* before the OpenEvent owner was identified | [md](evidence/func_80037964.md) |
| `0x80040510` | `func_80040510` | `player_rob_Init` | manual_review=stores in g_player_ptrs[idx]; chains AllocRobRmd + 4 rob_* helpers; single caller at fight-start (post-gpu-enable, post-sound-end) with (slot, model_id, heap_base=0x80190800) | [md](evidence/func_80040510.md) |
| `0x80041584` | `func_80041584` | `player_find_empty_slot_80041584` | manual_review=Iterates the 3 player slots in g_player_ptrs and returns the index of the first NULL slot (or -1 if all 3 are used). Standard 'find empty slot' pattern. | [md](evidence/func_80041584.md) |
| `0x80042478` | `func_80042478` | `disp_set_fade_color_80042478` | manual_review=Unpacks 0xRRGGBB from a0, optionally desaturates to grayscale (when func_800486FC -- likely 'is_grayscale_mode' -- returns true), then calls two named display helpers: disp_SetFramebufferMode(1, r, g, b) (sets the clear color) and gte_SetFarColor(r, g, b) (sets the GTE fog far... | [md](evidence/func_80042478.md) |
| `0x80044650` | `func_80044650` | `init_fade_panel_wrapper_80044650` | manual_review=1-line wrapper around named InitFadePanel(). Pure trampoline. | [md](evidence/func_80044650.md) |
| `0x80046954` | `func_80046954` | `empty_stub_80046954` | empty_jr_ra_nop | [md](evidence/func_80046954.md) |
| `0x80047210` | `func_80047210` | `camera_InitBoneData_80047210` | memcpy_default+halve_halfwords;legacy_rename_match | [md](evidence/func_80047210.md) |
| `0x80048744` | `func_80048744` | `set_color_mode_80048744` | manual_review=2-line setter for the named global g_color_mode to 0 or 1 based on input truthiness. Equivalent to g_color_mode = !!a0; but written as a branch (likely for codegen reasons). | [md](evidence/func_80048744.md) |
| `0x80065344` | `func_80065344` | `motion_ex_play_id1_80065344` | manual_review=Family member: calls motion_SetExMotion(1), increments per-id counter D_800F0BAA by 0x1C6, returns the motion id 1 result if counter < 0x11C8 (else 0). 0x11C8 / 0x1C6 = 10 -- so this motion can play 10 times before the counter saturates. | [md](evidence/func_80065344.md) |
| `0x80065394` | `func_80065394` | `motion_ex_play_id2_80065394` | manual_review=Family member: motion_SetExMotion(2), counter D_800F0BAC, same 10-play cap. Direct sibling of func_80065344 with different motion id and counter slot. | [md](evidence/func_80065394.md) |
| `0x80065434` | `func_80065434` | `motion_ex_play_id4_80065434` | manual_review=Family member: motion_SetExMotion(4), counter D_800F0BB0, step 0x19, threshold 0xC9. 0xC9 / 0x19 = ~10.5 -- so still ~10 plays but with smaller increments. | [md](evidence/func_80065434.md) |
| `0x80065540` | `func_80065540` | `motion_ex_play_id6_80065540` | manual_review=Family member: motion id 6, step 0x32, threshold 0x100. 0x100 / 0x32 = ~5.1 -- only ~5 plays. | [md](evidence/func_80065540.md) |
| `0x80065590` | `func_80065590` | `motion_ex_play_id7_80065590` | manual_review=Family member: motion id 7, identical structure to func_80065540 (5-play cap). | [md](evidence/func_80065590.md) |
| `0x80065630` | `func_80065630` | `motion_ex_play_idB_80065630` | manual_review=Family member: motion id 0xB (11), step 0x32, threshold 0x100 (5-play cap). | [md](evidence/func_80065630.md) |
| `0x80065760` | `func_80065760` | `motion_ex_play_id10_80065760` | manual_review=Family member: motion id 0x10 (16), counter D_800F0BC8, 10-play cap (same as id 1). | [md](evidence/func_80065760.md) |
| `0x80079154` | `func_80079154` | `bb2_rand` | manual_review=BSD libc LCG (0x41C64E6D * x + 0x3039, >> 16 & 0x7FFF); 29 callers across 6 files | [md](evidence/func_80079154.md) |
| `0x80079184` | `func_80079184` | `bb2_srand_80079184` | manual_review=1-line setter for D_800F1848 -- which is named g_rand_state (see [D_800F1848.md](../data_evidence/D_800F1848.md)) and is the LCG state of bb2_rand. | [md](evidence/func_80079184.md) |
| `0x80080600` | `func_80080600` | `cd_send_cmd_80080600` | manual_review=1-line wrapper around named cdrom_SendCmd() that always returns 1 (success). Adds nothing to the underlying helper -- likely an interface adapter to a dispatch table that requires s32 () return signature. | [md](evidence/func_80080600.md) |
| `0x80080620` | `func_80080620` | `cd_dma_idle_80080620` | manual_review=1-line inversion of cdrom_DmaToRam() == 0; returns 1 when no CD-DMA-to-RAM transfer is active | [md](evidence/func_80080620.md) |

## Primary evidence: `psyq_idiom` (5)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80016768` | `func_80016768` | `psyq_memset_80016768` | psyq_idiom=psyq_memset_80016768 | [md](evidence/func_80016768.md) |
| `0x8003043C` | `func_8003043C` | `psyq_memset_8003043C` | psyq_idiom=psyq_memset_8003043C; sole_caller_path=mario_test_Exec_helper_8003043C | [md](evidence/func_8003043C.md) |
| `0x80038148` | `func_80038148` | `psyq_memset_80038148` | psyq_idiom=psyq_memset_80038148; sole_caller_path=pad_FuncAnalog_helper_80038148 | [md](evidence/func_80038148.md) |
| `0x8005509C` | `func_8005509C` | `psyq_memset_8005509C` | psyq_idiom=psyq_memset_8005509C | [md](evidence/func_8005509C.md) |
| `0x8007DEE4` | `func_8007DEE4` | `psyq_memset_8007DEE4` | psyq_idiom=psyq_memset_8007DEE4 | [md](evidence/func_8007DEE4.md) |

## Primary evidence: `pure_getter` (3)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80046780` | `func_80046780` | `get_snd_bgm_id_80046780` | pure_getter=g_snd_bgm_id | [md](evidence/func_80046780.md) |
| `0x80046798` | `func_80046798` | `get_stage_id_80046798` | pure_getter=g_stage_id_signed | [md](evidence/func_80046798.md) |
| `0x80083688` | `func_80083688` | `get_sys_video_mode_80083688` | pure_getter=g_sys_video_mode | [md](evidence/func_80083688.md) |

## Primary evidence: `syscall_wrapper` (3)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x800164F8` | `func_800164F8` | `syscall_wrapper_break_800164F8` | syscall_wrapper=syscall_wrapper_break_800164F8 | [md](evidence/func_800164F8.md) |
| `0x80083698` | `func_80083698` | `syscall_wrapper_break_80083698` | syscall_wrapper=syscall_wrapper_break_80083698 | [md](evidence/func_80083698.md) |
| `0x8008393C` | `func_8008393C` | `syscall_wrapper_break_8008393C` | syscall_wrapper=syscall_wrapper_break_8008393C | [md](evidence/func_8008393C.md) |

## Primary evidence: `camera_InitRotation(&bone2)+g_cam_interp` (1)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80047570` | `func_80047570` | `camera_InitBone2_80047570` | camera_InitRotation(&bone2)+g_cam_interp=4;legacy_rename_match | [md](evidence/func_80047570.md) |

## Primary evidence: `data_as_code` (1)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x800164AC` | `func_800164AC` | `data_as_code_lb_table_800164AC` | data_as_code=data_as_code_lb_table_800164AC | [md](evidence/func_800164AC.md) |

## Primary evidence: `kengo_pattern` (1)
| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80060758` | `func_80060758` | `replay_camera_check_stage_80060758` | kengo_pattern=replay_camera_check_stage_80060758; sole_caller_path=SetCurrentCursor_helper_80060758 | [md](evidence/func_80060758.md) |
