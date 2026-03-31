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

c_files = list((root / "src").glob("*.c"))
asm_files = (
    list((root / "asm" / "funcs").glob("*.s")) +
    list((root / "asm").glob("*.s"))
)
# Data section .s files use .word symbol references — treat like C (replace all)
data_files = list((root / "asm" / "data").glob("*.s"))

for path in sorted(c_files + asm_files + data_files):
    text = path.read_text(encoding="utf-8", errors="replace")
    if path.suffix == ".c" or path.parent.name == "data":
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

mode = "DRY RUN" if dry_run else "APPLIED"
print(f"\n{mode}: {total_replacements} replacements across {total_files} files")
if dry_run:
    print("Re-run with --apply to write changes.")
