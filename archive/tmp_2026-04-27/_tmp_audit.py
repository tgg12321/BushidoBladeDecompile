"""
Quick sanity check on the 38 renames:
- What module is each from?
- How many BB2 stubs share the same Kengo name (indicates collision)?
- What are the first few instructions / stack frame size?
"""
import re, os, glob

BASE = r"C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile"

RENAMES = {
    "special_camera_Init":    "nm_special_cam",
    "md_game_rob_data_init":  "md_game",
    "DispHira":               "am_rmd",
    "special_camera_Exec":    "nm_special_cam",
    "action_CheckHitZangeki": "is_action",
    "AllocBukiRmd":           "am_rmd",
    "PutRobShadow":           "am_rmd",
    "md_game_end":            "md_game",
    "saTan2Main":             "sa_tan2",
    "replay_camera_get_attack_number": "nm_replay_cam",
    "md_game_check_mode":     "md_game",
    "DispPracticeMenuTex_A":  "su_menu_tuto",
    "DispPracticeMenuTex_B":  "su_menu_tuto",
    "DispPracticeMenuTex_C":  "su_menu_tuto",
    "saTan2KabutoWareMove":   "sa_tan2",
    "saTan0KiWareMoveA":      "sa_tan0",
    "saTan0KiWareMoveB":      "sa_tan0",  # same kengo name as A!
    "DispStuff":              "su_menu_ending",
    "DispUpdateStatusMessage":"su_menu_home",
    "efc_rob_set_type_flash": "is_efc_rob",
    "cpu_set_move_command_and_dir_for_no_action":   "nm_cpu",
    "cpu_set_move_command_and_dir_for_no_action_2": "nm_cpu",  # same kengo name!
    "DispSchoolBG":           "su_menu_single",
    "coli_HitPauseKatana":    "is_coli",
    "coli_HitPauseKatana_2":  "is_coli",  # same kengo name!
    "saTan0GaugeDraw":        "sa_tan0",
    "cpu_side_move_dir":      "nm_cpu",
    "cpu_side_move_dir_2":    "nm_cpu",   # same kengo name!
    "cpu_side_move_dir_3":    "nm_cpu",   # same kengo name!
    "cpu_side_move_dir_4":    "nm_cpu",   # same kengo name!
    "marionation_camera_Exec":"nm_mario_cam",
    "DispSamnailWindow":      "su_menu_vs",
    "DispSleepMenuTex":       "su_menu_home",
    "myRobGeneiMove":         "my_eff",
    "saSeInit":               "sa_se",
    "saSeInit_2":             "sa_se",    # same kengo name!
    "SetCurrentCursor":       "su_menu_edit",
    "pad_main_control":       "is_pad",
}

# Categories
PS2_UI = {"su_menu_tuto", "su_menu_home", "su_menu_vs", "su_menu_edit",
          "su_menu_single", "su_menu_ending"}
CORE_ENGINE = {"nm_special_cam", "md_game", "is_action", "is_coli",
               "nm_cpu", "is_efc_rob", "is_pad", "nm_replay_cam"}
RENDERING = {"am_rmd", "my_eff"}
SOUND = {"sa_tan0", "sa_tan1", "sa_tan2", "sa_se"}
CAMERA = {"nm_mario_cam"}

def get_stack_frame(asm_path):
    """Extract stack frame size from first addiu $sp instruction."""
    try:
        with open(asm_path, encoding="utf-8", errors="replace") as f:
            for line in f:
                m = re.search(r'addiu\s+\$sp,\s+\$sp,\s+(-0x[0-9a-fA-F]+)', line)
                if m:
                    return m.group(1)
    except FileNotFoundError:
        return "FILE NOT FOUND"
    return "no frame"

# Group by base kengo name (strip _2, _3, _4 suffixes)
from collections import defaultdict
groups = defaultdict(list)
for new_name in RENAMES:
    base = re.sub(r'_[234]$', '', new_name)
    groups[base].append(new_name)

print(f"{'Name':<50} {'Module':<20} {'BB2 file':<20} {'Frame':<12} {'Confidence'}")
print("-" * 125)

for base, variants in sorted(groups.items()):
    module = RENAMES[variants[0]]

    if module in PS2_UI:
        conf = "LOW - PS2 UI"
    elif module in CORE_ENGINE:
        conf = "HIGH - core engine"
    elif module in RENDERING:
        conf = "MED - rendering"
    elif module in SOUND:
        conf = "MED - sound"
    elif module in CAMERA:
        conf = "MED - camera"
    else:
        conf = "?"

    # Multiple BB2 stubs sharing same kengo name = collision
    if len(variants) > 1:
        conf += f" (x{len(variants)} collision!)"

    for v in variants:
        asm = os.path.join(BASE, "asm", "funcs", f"{v}.s")
        frame = get_stack_frame(asm)
        # find which bb2 file
        bb2_file = "?"
        for c in glob.glob(os.path.join(BASE, "src", "*.c")):
            with open(c, encoding="utf-8", errors="replace") as f:
                if v in f.read():
                    bb2_file = os.path.basename(c)
                    break
        print(f"  {v:<48} {module:<20} {bb2_file:<20} {frame:<12} {conf}")
    print()
