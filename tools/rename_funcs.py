#!/usr/bin/env python3
"""
Atomic function rename tool for BB2 decompilation.
Renames func_XXXXXXXX -> semantic_name across all C and ASM files.
- C files: replaces all occurrences (definitions, externs, calls)
- ASM files: replaces only "jal old_name" patterns; skips glabel/endlabel
"""
import re, sys, os
from pathlib import Path

RENAMES = {
    # sound.c (56)
    "func_80046780": "snd_GetBgmId",
    "func_8004678C": "snd_GetSeId",
    "func_80046798": "stage_GetId",
    "func_800467A8": "stage_GetVariant",
    "func_800467B8": "snd_LoadBgm",
    "func_800468B0": "snd_PlayBgm",
    "func_800468DC": "snd_BgmCallback",
    "func_80046914": "snd_StopBgm",
    "func_80046934": "snd_AllocSe",
    "func_80046954": "snd_SeNullCallback",
    "func_8004695C": "snd_LoadSe",
    "func_800469A0": "snd_PlaySe",
    "func_800469C4": "snd_LoadSelection",
    "func_80046A60": "snd_StopSelection",
    "func_80046A80": "snd_SelectionCallback",
    "func_80046AA0": "snd_StopAll",
    "func_80046AE8": "snd_PlaySystemSe",
    "func_80046B20": "snd_StopSystemSe",
    "func_80046B44": "game_Init",
    "func_80046DA8": "game_StageInit",
    "func_80046DE4": "game_GetDummyFlag",
    "func_80046DEC": "game_GetPlayerData",
    "func_80046E18": "game_GetPlayerBase",
    "func_80046E44": "game_DisablePause",
    "func_80046E54": "game_SetPause",
    "func_80046E7C": "game_GetPause",
    "func_80046E8C": "game_ResetTimer",
    "func_80046EA0": "game_InitStageSound",
    "func_80046EDC": "game_StageCleanup",
    "func_80046F14": "stage_GetDataPtr",
    "func_80046F24": "camera_InitMatrix",
    "func_8004700C": "camera_Transform",
    "func_80047210": "camera_InitBoneData",
    "func_800472B0": "camera_GetBoneData",
    "func_800472C0": "camera_InitRotation",
    "func_80047384": "camera_CalcAngles",
    "func_8004746C": "game_EffInit",
    "func_8004748C": "game_EffCleanup",
    "func_800474AC": "game_AnimInit",
    "func_800474D0": "game_AnimCleanup",
    "func_800474F0": "game_EffInit2",
    "func_80047510": "game_EffCleanup2",
    "func_80047530": "game_SndInit",
    "func_80047550": "game_SndCleanup",
    "func_80047570": "camera_InitBone2",
    "func_80047738": "game_AnimStart",
    "func_8004775C": "game_AnimStop",
    "func_8004777C": "game_EffStart",
    "func_8004779C": "game_EffStop",
    "func_800477BC": "game_Stub1",
    "func_800477C4": "game_Stub2",
    "func_800477CC": "game_Stub3",
    "func_800477D4": "game_Stub4",
    "func_800477DC": "snd_SetVolume",
    "func_80047D94": "snd_CalcFade",
    "func_80047E5C": "snd_GetFadeCurve",
    "func_80047EC8": "snd_GetMaxFade",
    # ings.c (33)
    "func_80016514": "file_LoadAll",
    "func_800165F8": "file_LoadSectors",
    "func_800166C4": "disp_CalcFov",
    "func_80016768": "disp_SetFramebufferMode",
    "func_800167AC": "file_GetFlag0",
    "func_800167BC": "file_GetFlag1",
    "func_800167D4": "file_GetFlag2",
    "func_80016868": "gpu_EnableDisplay",
    "func_80016888": "gpu_InitDisplay",
    "func_800168D0": "gpu_DisableDisplay",
    "func_800168F0": "sys_StubEmpty",
    "func_800168F8": "sys_InitSound",
    "func_80016918": "disp_Init",
    "func_80016A18": "sys_Init",
    "func_80016C3C": "sys_Panic",
    "func_80016C74": "file_ResetDmaFlag",
    "func_80016C80": "file_LoadOverlay",
    "func_80016CF8": "file_LoadSoundData",
    "func_80016D78": "sys_GameInit",
    "func_80016E40": "gpu_SetDrawMode",
    "func_800171AC": "rng_SetSeed",
    "func_800171B8": "rng_Next",
    "func_80017714": "obj_ClearAll",
    "func_80017738": "obj_CalcOffset",
    "func_80017748": "math_Distance3D",
    "func_800177C8": "math_Distance3D_16",
    "func_80017E8C": "obj_Clear",
    "func_80017EB4": "obj_UpdatePosition",
    "func_80017EF4": "obj_AddValue",
    "func_80017F28": "scratchpad_Save",
    "func_80017F5C": "scratchpad_Restore",
    "func_80017F90": "sys_StubEmpty2",
    "func_80017F98": "sys_StubEmpty3",
    # ings2.c (21)
    "func_8008289C": "sys_SetVsyncMode",
    "func_800828B4": "sys_SetTimer",
    "func_80082AC0": "irq_DisableInterrupts",
    "func_80082AF0": "irq_EnableInterrupts",
    "func_80082B20": "irq_AcknowledgeVblank",
    "func_80082B50": "irq_SetAlarm",
    "func_80082B84": "irq_ClearAlarm",
    "func_80082BB4": "irq_Reset",
    "func_80082BE4": "irq_Dispatch",
    "func_80082C14": "sys_GetVblankCount",
    "func_80082C24": "sys_GetIrqCounter",
    "func_8008339C": "sys_MemClear",
    "func_80083644": "sys_MemClear2",
    "func_80083670": "sys_SetVideoMode",
    "func_80083688": "sys_GetVideoMode",
    "func_8008387C": "bios_FileRead",
    "func_8008393C": "bios_FileReadRaw",
    "func_80083954": "irq_ProcessPending",
    "func_80083A18": "sys_Shutdown",
    "func_80083B30": "spu_Reset",
    "func_80083B50": "spu_SetVolume",
    # Kengo size-match batch (38 renames, 2026-03-30)
    "func_8002CD58": "special_camera_Init",
    "func_8001F2E4": "md_game_rob_data_init",
    "func_8003E6D8": "DispHira",
    "func_80036940": "special_camera_Exec",
    "func_800863DC": "action_CheckHitZangeki",
    "func_80085A40": "AllocBukiRmd",
    "func_8002C22C": "PutRobShadow",
    "func_80086818": "md_game_end",
    "func_800880E8": "saTan2Main",
    "func_8003E2D8": "replay_camera_get_attack_number",
    "func_8003EDC0": "md_game_check_mode",
    "func_8001B748": "DispPracticeMenuTex_A",
    "func_80020E74": "DispPracticeMenuTex_B",
    "func_800290B8": "DispPracticeMenuTex_C",
    "func_800283D0": "saTan2KabutoWareMove",
    "func_8002D780": "saTan0KiWareMoveA",
    "func_8002DAD0": "saTan0KiWareMoveB",
    "func_80083E9C": "DispStuff",
    "func_800889D4": "DispUpdateStatusMessage",
    "func_80044800": "efc_rob_set_type_flash",
    "func_800338CC": "cpu_set_move_command_and_dir_for_no_action",
    "func_80017200": "cpu_set_move_command_and_dir_for_no_action_2",
    "func_8002EBDC": "DispSchoolBG",
    "func_800893D8": "coli_HitPauseKatana",
    "func_80089A48": "coli_HitPauseKatana_2",
    "func_80088D0C": "saTan0GaugeDraw",
    "func_80027640": "cpu_side_move_dir",
    "func_8003C040": "cpu_side_move_dir_2",
    "func_8008BF04": "cpu_side_move_dir_3",
    "func_80080DB0": "cpu_side_move_dir_4",
    "func_80018094": "marionation_camera_Exec",
    "func_800344B4": "DispSamnailWindow",
    "func_8003D52C": "DispSleepMenuTex",
    "func_8001B478": "myRobGeneiMove",
    "func_8002E838": "saSeInit",
    "func_800393C8": "saSeInit_2",
    "func_8003C714": "SetCurrentCursor",
    "func_8002E6B0": "pad_main_control",
    # Kengo affinity batch (38 renames, 2026-03-30)
    # HIGH — core engine, single exact affinity match
    "func_8001D790": "se_data_set",
    "func_8001FBE8": "single_game_CheckStatusUpDataTotalOver",
    "func_800204C0": "single_game_SetAbilityData",
    "func_800238C4": "camera_set_zoom",
    "func_8002FC80": "cpu_check_tubazeri",
    "func_8002FDB0": "coli_check_circle_hit_line",
    "func_80030580": "coli_hit_body_weapon",
    "func_8003032C": "cpu_get_dist",
    "func_80030A2C": "cpu_set_move_command_and_dir",
    "func_800307D0": "cpu_check_tubazeri_2",
    "func_800324D0": "cpu_get_dist_2",
    "func_80032314": "Pad_Prs",
    "func_8003339C": "cpu_check_same_dir_timer",
    "func_80036D98": "replay_camera_Init",
    "func_8003800C": "damage_DebugDisp",
    "func_800383A4": "pad_FuncAnalog",
    "func_8003A5A0": "pad_ClearAppliBuffer",
    "func_8003BCB4": "md_game_check_change_sub_mode",
    "func_800896A0": "exec_game",
    "func_8008A928": "md_game_check_change_main_mode_katinuki",
    # MED — affinity match
    "func_80040B44": "rob_calc_2d_position",
    "func_800401CC": "PutShadowRmd",
    "func_800408F8": "rob_life_ctrl",
    "func_80041188": "hirahira_w_ctrl",
    "func_80042504": "rob_life_ctrl_2",
    "func_80042684": "mot_data_set",
    "func_8004428C": "hirahira_w_frie",
    "func_80042C80": "hirahira_w_ctrl_2",
    "func_80044378": "calc_fc_frame",
    "func_80081BB0": "saEft01Init",
    "func_800804BC": "tslPolyF4Init",
    "func_80083C34": "saTan5TakeAnim2_2",
    "func_80084974": "saTan4GaugeInit",
    "func_80084CC0": "saTan0Main",
    "func_80085544": "SetBloodSpot",
    "func_80087E3C": "AddTbpOfst",
    "func_8008C1E8": "SetPacketData",
    "func_8002D518": "saTan5TakeAnim2",
    # Kengo near-miss batch (13 renames, ±5%, 2026-03-30)
    # HIGH — ≤5 insn diff, unique affinity match
    "func_800187F4": "single_game_setModeRequest",
    "func_800288C8": "saTan3MainJump",
    "func_8001EA84": "cpu_get_move_pattern_table_number",
    "func_80040594": "AllocRobRmd",
    "func_8008241C": "saEft00Add",
    "func_80018300": "cpu_check_run_attack",
    # MED — 5-19 insn diff, unique affinity match
    "func_8001CE60": "camera_set_target_zoom",
    "func_800174F4": "gnd_disp_loop_ctrl",
    "func_80036140": "special_camera_set_win_cam",
    "func_8008B488": "saTan1MainJump",
    "func_800812FC": "tslTm2LoadImage",
    "func_80082000": "tslTm2LoadImage_2",
    "func_8001DCB0": "mario_test_Exec",
    # Kengo near-miss batch LOW confidence (±10%, 2026-03-30)
    "func_80038C70": "motion_SetMotion",
    "func_80037348": "special_camera_get_rot_dir",
    "func_800836C8": "ang_hosei",
    "func_800833C8": "conv_matrix_rotation",
    # Expanded affinity batch (exact matches unlocked by new module entries)
    "func_80082C3C": "motion_make_table",
    "func_80083794": "motion_Open",
    "func_8002AB08": "calc_loc_mat_fw",
    "func_80081030": "marionation_Exec",
    "func_80035618": "replay_camera_rob_back_loose2",

    # === Item 6: readability renames (2026-04-10) ===
    # gpu.c — primitive inits (verified via GPU command codes)
    "func_8007A9B8": "initPolyF3",
    "func_8007A9CC": "initPolyFT3",
    "func_8007A9E0": "initPolyG3",
    "func_8007A9F4": "initPolyGT3",
    "func_8007AA08": "initPolyF4",
    "func_8007AA1C": "initPolyFT4",
    "func_8007AA30": "initPolyG4",
    "func_8007AA44": "initPolyGT4",
    "func_8007AA58": "initSprt8",
    "func_8007AA6C": "initSprt16",
    "func_8007AA80": "initSprt",
    "func_8007AA94": "initTile1",
    "func_8007AAA8": "initTile8",
    "func_8007AABC": "initTile16",
    "func_8007AAD0": "initTile",
    "func_8007AAE4": "initLineF2",
    "func_8007AAF8": "initLineG2",
    "func_8007AB0C": "initPolyF3_dither",
    "func_8007AB2C": "initPolyG3_dither",
    "func_8007AB4C": "initPolyF4_dither",
    "func_8007AB6C": "initPolyG4_dither",
    # gpu.c — GPU packet/OT functions
    "func_8007AB8C": "initDrawMode",
    "func_8007ABB8": "initLoadImage",
    "func_8007AC18": "initStoreImage",
    "func_8007AC84": "gpu_CatPacket",
    "func_8007A87C": "ot_GetTag",
    "func_8007A898": "ot_IsEnd",
    "func_8007A8B4": "ot_Link",
    "func_8007A8F0": "ot_Insert",
    "func_8007A92C": "ot_SetAddr",
    "func_8007A950": "ot_SetEnd",
    # gpu.c — TPage/Clut/debug/flags
    "func_8007A788": "gpu_CalcTPage",
    "func_8007A7C4": "gpu_CalcClut",
    "func_8007A7DC": "gpu_DebugTPage",
    "func_8007A83C": "gpu_DebugClut",
    "func_8007ACBC": "gpu_DebugDispEnv",
    "func_8007ADD0": "gpu_DebugDrawEnv",
    "func_8007A968": "gpu_SetSemiTransp",
    "func_8007A990": "gpu_SetRawTexture",
    # gpu.c — env init, mode/config
    "func_8007A694": "gpu_InitDrawEnv",
    "func_8007A74C": "gpu_InitDispEnv",
    "func_8007AE7C": "gpu_SetMode",
    "func_8007B000": "gpu_SetDither",
    "func_8007B114": "gpu_SetDebugLevel",
    "func_8007B178": "gpu_SetInterlace",
    "func_8007B224": "gpu_GetType",
    "func_8007B234": "gpu_GetDebugLevel",
    # gpu.c — texture loaders
    "func_8007A4D8": "gpu_LoadTexture",
    "func_8007A5C4": "gpu_LoadClut256",
    "func_8007A62C": "gpu_LoadClut16",
    # display.c — GTE functions (verified via COP2 control register assignments)
    "func_8007EEEC": "gte_SetRotMatrix",
    "func_8007EF1C": "gte_SetColorMatrix",
    "func_8007EF4C": "gte_SetTransVector",
    "func_8007EF6C": "gte_GetScreenXY",
    "func_8007EF8C": "gte_GetH",
    "func_8007EF9C": "gte_SetBackColor",
    "func_8007EFBC": "gte_SetFarColor",
    "func_8007EFDC": "gte_SetScreenOffset",
    # display.c — math (sin/cos lookup tables)
    "func_8007DF20": "math_Sin",
    "func_8007DF5C": "math_SinLookup",
    "func_8007DFEC": "math_Cos",
    # display.c — GPU state
    "func_8007DEE4": "bb2_memset",
    "func_8007C0B0": "gpu_GetDispEnv",
    "func_8007C0E8": "gpu_IsDrawing",
    "func_8007D308": "gpu_SendData",
    "func_8007D358": "gpu_StartDmaList",
    "func_8007D3A4": "gpu_GetInfo",
    # display.c — GPU packet inits
    "func_8007C118": "initClearImage",
    "func_8007C154": "initDrawArea",
    "func_8007C1D8": "initDrawOffset",
    "func_8007C21C": "initMaskBit",
    "func_8007C248": "initTexPage",
    # system.c — SKIPPED (GCC 2.7.2 register allocation sensitivity)
    # main.c — memcard / SPU
    "func_80087F00": "memcard_SetData",
    "func_80087F24": "memcard_ClearBusy",
    "func_80087F34": "memcard_SetSlot",
    "func_800885AC": "spu_Init",
    # config.c — SKIPPED (GCC 2.7.2 register allocation sensitivity)
}

dry_run = "--apply" not in sys.argv
root = Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile")

# Build combined pattern for efficiency
old_names = sorted(RENAMES.keys(), key=len, reverse=True)
pattern = re.compile(r"\b(" + "|".join(re.escape(k) for k in old_names) + r")\b")

def process_c_file(path, text):
    """Replace all occurrences in C files."""
    return pattern.sub(lambda m: RENAMES[m.group(0)], text)

def process_asm_file(path, text):
    """In ASM files, only replace jal/jalr targets, not glabel/endlabel."""
    lines = text.splitlines(keepends=True)
    result = []
    for line in lines:
        stripped = line.lstrip()
        # Skip glabel and endlabel lines — leave stub labels untouched
        if stripped.startswith("glabel ") or stripped.startswith("endlabel "):
            result.append(line)
        else:
            result.append(pattern.sub(lambda m: RENAMES[m.group(0)], line))
    return "".join(result)

total_files = 0
total_replacements = 0

c_files = list((root / "src").glob("*.c")) + list((root / "include").glob("*.h"))
asm_files = (
    list((root / "asm" / "funcs").glob("*.s")) +
    list((root / "asm").glob("*.s"))
)
# Data section .s files use .word symbol references — treat like C (replace all)
data_files = list((root / "asm" / "data").glob("*.s"))
# sdata files list functions by name — must be updated or GP-relative breaks
sdata_files = [
    p for p in [root / "sdata_funcs.txt", root / "sdata_exclude.txt"]
    if p.exists()
]

for path in sorted(c_files + asm_files + data_files + sdata_files):
    text = path.read_text(encoding="utf-8", errors="replace")
    if path.suffix == ".c" or path.parent.name == "data" or path.suffix == ".txt":
        new_text = process_c_file(path, text)
    else:
        new_text = process_asm_file(path, text)

    if new_text != text:
        count = sum(1 for _ in pattern.finditer(text))
        rel = path.relative_to(root)
        print(f"  [{count:3d} changes] {rel}")
        total_files += 1
        total_replacements += count
        if not dry_run:
            path.write_text(new_text, encoding="utf-8")

# Rename stub .s files for functions that are still INCLUDE_ASM stubs.
# When a stub is renamed, the INCLUDE_ASM argument (and thus the expected
# filename) changes, so the .s file must be renamed to match.  The glabel
# inside the file must also be updated — it is the symbol name the linker uses.
#
# Only process functions that are currently stubs — i.e. their NEW name appears
# in an INCLUDE_ASM call in a C file (the text replacements above already
# updated those calls).  Decompiled functions have no INCLUDE_ASM and their old
# .s files are kept as reference only; we leave those alone.
funcs_dir = root / "asm" / "funcs"
glabel_pattern = re.compile(r"^(glabel|endlabel) (func_[0-9A-Fa-f]{8})\b", re.MULTILINE)

# Build set of new names that appear in INCLUDE_ASM calls
include_asm_re = re.compile(r'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)')
active_stubs = set()
for c_path in (root / "src").glob("*.c"):
    c_text = c_path.read_text(encoding="utf-8", errors="replace")
    for m in include_asm_re.finditer(c_text):
        active_stubs.add(m.group(1))

for old_name, new_name in RENAMES.items():
    if new_name not in active_stubs:
        continue  # already decompiled — leave the reference .s file alone
    old_path = funcs_dir / f"{old_name}.s"
    new_path = funcs_dir / f"{new_name}.s"
    if not old_path.exists():
        continue  # file was already renamed in a previous run
    text = old_path.read_text(encoding="utf-8", errors="replace")
    # Replace glabel/endlabel lines (intentionally skipped in process_asm_file)
    new_text = glabel_pattern.sub(lambda m: f"{m.group(1)} {RENAMES.get(m.group(2), m.group(2))}", text)
    rel_old = old_path.relative_to(root)
    rel_new = new_path.relative_to(root)
    print(f"  [stub rename] {rel_old} → {rel_new}")
    total_files += 1
    if not dry_run:
        old_path.write_text(new_text, encoding="utf-8")
        old_path.rename(new_path)

mode = "DRY RUN" if dry_run else "APPLIED"
print(f"\n{mode}: {total_replacements} replacements across {total_files} files")
if dry_run:
    print("Re-run with --apply to write changes.")
