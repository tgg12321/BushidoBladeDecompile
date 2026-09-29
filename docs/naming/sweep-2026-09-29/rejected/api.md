# api vein — examined and dropped (2026-09-29)

Method: `triage.py` re-derived every target's callees from the `jal` WORDS in `asm/funcs/<glabel>.s`
(each word asserted equal to the EXE word at that vaddr), mapped each target address to the CURRENT census
(`docs/naming/function-names.csv`) tier, and rescanned lui/addiu string refs (`triage.csv` / `triage.txt`).
targets.csv's `[?]` callees were stale names; the table below uses the recomputed tiers.
"INF/AUTO callee" = the body calls at least one INFERRED/AUTO function whose effect is not established,
so an api-restatement name could not cover the whole body.

| addr | insns | idea examined | why dropped |
|---|---|---|---|
| 0x80016A8C | 108 | display/VRAM init restatement (SetDispMask/SetDefDispEnv/PutDispEnv/LoadImage/DrawSync/VSync) | also calls game_FrameLoop x2 [INFERRED], mode_helper [INFERRED], cdrom_StartRead [CORROBORATED]; any name would summarise control flow |
| 0x800174F4 | 136 | frame-render restatement | 09-24 reason stands: func_8005D554 [AUTO] + 3 gnd_* [INFERRED] callees |
| 0x8001B294, 0x8001B478 | 75/134 | — | ratan2 only verified call + func_8003F1E4 [AUTO]; game logic |
| 0x8001C820 | 47 | — | rand + mk_helper [INFERRED] |
| 0x8001D790, 0x8001D904, 0x8001D998 | 93/37/37 | teardown restatement | memcpy + gpu_ResetGraphMode1 [CORR] + several AUTO/INFERRED (func_80020D38, sys_Panic, func_8005B* ...) |
| 0x8001DCB0 | 469 | — | 34-callee setup body, mostly AUTO/INFERRED |
| 0x8001F2E4, 0x80022224, 0x800233AC, 0x800238C4, 0x80027AD8, 0x8002AB08, 0x80031B24 | — | — | ratan2/rand/rng_Next plus AUTO/INFERRED callees on game structs; 09-24 reason stands |
| 0x8002CD58, 0x8002DAD0, 0x8002E838, 0x8002EBDC | 370/212/123/188 | computation name (all callees VERIFIED: ratan2, RotMatrixX/Y) | read/write fields of a game struct reached through a0 (+0x60/+0x64 point pointers, +0xA8 vector, +0xD8 matrix, +0xF8/+0xFA angles; e.g. func_8002E838.s 8002E84C-8002EA08). The struct layout is game data, not Sony, so any "look-at"-style name needs game semantics. 09-24 reason stands |
| 0x8002FC80 | 76 | — | ratan2 only, but 22 global refs: game computation |
| 0x800338CC, 0x800645B0, 0x80055138, 0x800571C0, 0x8005D554, 0x80067200 | — | — | rand/RotMatrix on game tables and globals (e.g. func_800645B0 touches D_800F0BCC.., videoDec); no restatement |
| 0x80034708 | 544 | in-binary-string ("%sCR%3d\n" @0x80010834, "%sCO%3d\n" @0x80010840, "~c777" @0x800A3178, "%s%5d" ...) | debug-overlay format strings; none states the function's role. 16 calls to func_8003D52C [AUTO] + mode_helper [INFERRED] |
| 0x80035828 | 360 | — | ~25 AUTO/INFERRED callees |
| 0x80036140 | 512 | cdrom_* restatement (only VERIFIED Cd* + cdrom_SetMix [CORR]) | a 512-insn jump-table state machine (jtbl_80010938 on D_80101E62); a name would summarise control flow (apiscan README left this family out for the same reason) |
| 0x80036940 | 274 | cdrom state-machine tick | calls func_80036140 [AUTO]; same control-flow objection |
| 0x80037110 | 39 | cdrom_StartAudio wrapper | func_80036EA8 [INFERRED mode_helper] computes the file index; table D_8008F13C meaning not pinned |
| 0x80038170 | 141 | `memcard_BuildSaveHeader` (writes 'S','C', 0x11, 1 at +0..+3; strcpy of the Shift-JIS title D_8008F1C0 to +4; 16 halfwords D_800109EC to +0x60; 0x80 bytes D_80010A2C to +0x80) | the "SC / icon flag / block count / title / CLUT / icon" meaning is the BIOS memory-card file-header format (psx-spx / BIOS), not a PsyQ header struct and not consumed by any verified Sony code in the EXE. Not an admitted class (sony-struct-restatement fails (b) and (c); api-restatement covers only the strcpy). Also: the "OVER FLOW\n" string hit @0x80010000 is a scanner false positive (lui 0x8001 base of D_800109EC). Also reads pad-state globals (D_80106A50 bit loop) |
| 0x800383A4 | 173 | memcard dispatcher | memcard_* [CORR] x4 but also 5 AUTO helpers (func_80037AA4/37B00/37F40/38148/38170); 09-24 reason stands |
| 0x8003AB44 | 93 | comb_* link-handshake restatement | _comb_control/SetDispMask [VERIFIED] + comb_* [CORR], but it is an 8-state jump-table machine (jtbl_80010CA4 on D_800A38AC) with game flags D_800A37B8/D_800A38A0/D_80102794/D_800A3916; a name summarises control flow |
| 0x8003ACB8 | 105 | — | many INFERRED/AUTO callees |
| 0x8003D2C4 | 12 | LoadImage one-liner | 09-24 reason stands (name would assert what the fixed 16x36 image is). No new evidence |
| 0x8003D52C | 146 | `dbg_Printf`-style name (sprintf/strlen per '%' segment) | the formatted text is then consumed by globals D_800A335C/D_800A3360 and func_8003D39C [AUTO]; only the formatting half is restatable |
| 0x8003E164, 0x80040594, 0x80041AC8, 0x80041BF4, 0x80045B68 | — | VRAM move/store restatements | AUTO/INFERRED callees (func_8003E22C, func_8003E2A0, gnd_helper, get_global ...); 09-24 reasons stand |
| 0x8003E6D8, 0x80041EB0, 0x80044504, 0x80044800, 0x80046BF4, 0x800475A4, 0x80048BA4, 0x8004A940 | — | matrix restatements | each also calls INFERRED helpers (camera_*, calc_*, stage_*, gte_rtpt, efc_*) or jalr through data |
| 0x80044CCC | 70 | computation name (rsin/rcos/LoadAverage12, arg memory only) | builds (-a, r*sin(t)>>12, -(r*cos(t)>>12)) from two s16[3] then LoadAverage12; the sign convention is ad hoc, any name would be invented shorthand. 09-24 reason stands |
| 0x80044FA0 | 56 | in-binary-string "Marionation over flow. No.%d (-%dbyte)\n" @0x8001528C | error message on the overflow path only (then spins in func_800164F8); names the failure, not the function. 5 AUTO/INFERRED callees |
| 0x800477E8 | 170 | GetClut/GetTPage texture-attr restatement | efc_effect_dispatch_child_recursive [INFERRED] callee; game texture records |
| 0x800482C8 | 69 | `gpu_LoadTim` | new since 09-24: math_GrayscaleRgb555 is CORROBORATED, but gnd_helper (0x800486FC) is still INFERRED and gates the grayscale path; and the "TIM" reading (byte 0 == 0x10, flag bit 3 = CLUT) is the TIM file format, not consumed by verified Sony code in the EXE (no OpenTIM/ReadTIM linked) - sony-struct (c) fails |
| 0x80048864 | 134 | `gpu_RecolorVramLine` (all callees VERIFIED: DrawSync, StoreImage, LoadImage) | body = StoreImage(RECT{a1,a2,a3,1}) -> per-pixel transform chosen by a0 (0: each 5-bit channel *k>>12 &0x1F with no clamp; 1: 1351-weighted gray then *k>>12; other: unchanged; 0x0000 pixels kept) -> LoadImage at (stack 0x24, 0x28). A short name would summarise a 3-way mode switch; not emulation-proven. Dropped rather than MEDIUM |
| 0x800548DC | 12 | — | DrawSync + mode_helper [INFERRED] + func_80046A60 [AUTO] |
| 0x80055B60 | 1110 | — | game logic, INFERRED/AUTO callees |
| 0x80057CC8 | 111 | computation name (ratan2 x2 + Judge, arg memory only) | reads a game record (+2 scale byte, +3 count byte, +4 s16-pair array ptr) and offsets point i by 40*scale along the half-angle of the directions to its ring neighbours. The record layout is game data; a name ("loop vertex bisector offset") would describe it. Not emulated |
| 0x8005B644, 0x8005B6AC, 0x8005B6FC | — | SsVabClose one-liners | apiscan README:42-43 and 09-25 followups precedent (hard-coded VAB ids); B644/B6AC also call func_800858D0 [AUTO]. No new evidence |
| 0x8005BDF0 | 37 | `snd_CloseVabs` (SsVabClose over the 3 ids at D_8009AD18, zeroing g_vab_rec_ptr[id] / g_vab_vb_sbaddr[id]) | same precedent: a VAB-id list from a data table is still "which VABs"; left auto like the one-liners |
| 0x8005BA8C | 169 | — | multi-step VAB load (cdrom_StartRead, game_FrameLoop [INFERRED], func_8005C2A8 [AUTO] ...) |
| 0x8005BE84 | 46 | snd_SetReverbType | 09-07 PLAUSIBLE-not-applied (arg indexes D_8009AD1C) and func_800858D0 [AUTO]. No new evidence |
| 0x8005C2A8 | 134 | in-binary-string "vab id:%d mistake\n" @0x800158CC | error message; README left-out multi-step VAB transfer. No new evidence |
| 0x8005C614 | 15 | `snd_InitMaster` (SsSetMVol(0x7F,0x7F); func_800858D0(0); SsSetStereo(); SsSetAutoKeyOffMode(0)) | func_800858D0 is still AUTO (owner-held PROBABLE SsUtAllKeyOff); 09-25 precedent (0x8005BF3C snd_ReverbOff rejected for the same callee). Re-propose if the lib vein verifies func_800858D0 |
| 0x8005C6D0 | 118 | snd key-on flush | 09-25 followups rejection stands (SsUtKeyOnV is CORROBORATED libscan-near, not caller-pinned) |
| 0x8005E098, 0x8005F1C8, 0x8006A564, 0x8006C21C, 0x80070188, 0x80070F78, 0x80073200, 0x800753D8, 0x80075830 | — | prim-builder restatements | satan_sprite_batch_render [INFERRED] + func_8006E480 [AUTO] (and mode_helper/gnd_helper) in every body |
| 0x80063084, 0x80065800 | 667/1456 | — | large GTE render bodies with 100+ game-global refs (0x80063084 also calls func_80052C28 [AUTO]) |
| 0x800784E4 | 30 | — | ClearOTagR + 3 INFERRED helpers; 09-24 reason stands |
| 0x80087770 | 335 | — | inside the LIBSND VM block (between _SsVmKeyOnNow 0x800872A4 and _SsVmGetSeqVol 0x80087CAC); library code carries Sony's name - lib vein, not api-restatement |
| 0x8008B488 | 387 | — | owner-held PROBABLE SpuSetVoiceAttr (LIBSPU region); lib vein |
| 0x8003553C | 43 | `gpu_AddPolyG4` (HIGH) | downgraded to MEDIUM `gpu_AddPolyG4_640x240`, see candidates.csv: the calls alone support only "AddPolyG4", which reads like a generic helper for a caller's prim; the 640x240 part is a POLY_G4 field restatement whose consumer is the GPU, not verified Sony code (sony-struct (c)) |
