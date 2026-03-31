#!/usr/bin/env python3
"""
Add /* kengo: ... */ confidence comments after INCLUDE_ASM lines for
renamed stubs. Comments go on the line immediately below the INCLUDE_ASM.
Skips stubs that already have a kengo comment.
"""
import re, sys, glob
from pathlib import Path

ROOT = Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile")
DRY = "--apply" not in sys.argv

# (new_name, kengo_module, insn_count, confidence, note)
ANNOTATIONS = [
    # HIGH — core Marionation engine, single exact match
    ("special_camera_Init",    "nm_special_cam", 370, "HIGH", ""),
    ("special_camera_Exec",    "nm_special_cam", 274, "HIGH", ""),
    ("md_game_rob_data_init",  "md_game",        351, "HIGH", ""),
    ("md_game_end",            "md_game",        249, "HIGH", ""),
    ("md_game_check_mode",     "md_game",        234, "HIGH", ""),
    ("action_CheckHitZangeki", "is_action",      271, "HIGH", ""),
    ("efc_rob_set_type_flash", "is_efc_rob",     204, "HIGH", ""),
    ("pad_main_control",       "is_pad",          98, "HIGH", ""),
    ("replay_camera_get_attack_number", "nm_replay_cam", 242, "HIGH", ""),
    # HIGH with collision — multiple BB2 stubs share the same Kengo size
    ("coli_HitPauseKatana",    "is_coli",        178, "HIGH", "x2 size collision"),
    ("coli_HitPauseKatana_2",  "is_coli",        178, "HIGH", "x2 size collision"),
    ("cpu_set_move_command_and_dir_for_no_action",   "nm_cpu", 189, "HIGH", "x2 size collision"),
    ("cpu_set_move_command_and_dir_for_no_action_2", "nm_cpu", 189, "HIGH", "x2 size collision"),
    ("cpu_side_move_dir",   "nm_cpu", 160, "HIGH", "x4 size collision"),
    ("cpu_side_move_dir_2", "nm_cpu", 160, "HIGH", "x4 size collision"),
    ("cpu_side_move_dir_3", "nm_cpu", 160, "HIGH", "x4 size collision"),
    ("cpu_side_move_dir_4", "nm_cpu", 160, "HIGH", "x4 size collision"),
    # MED — plausible but unverified
    ("PutRobShadow",            "am_rmd",    252, "MED", ""),
    ("AllocBukiRmd",            "am_rmd",    259, "MED", ""),
    ("DispHira",                "am_rmd",    299, "MED", ""),
    ("marionation_camera_Exec", "nm_mario_cam", 155, "MED", ""),
    ("saSeInit",                "sa_se",     123, "MED", "x2 size collision"),
    ("saSeInit_2",              "sa_se",     123, "MED", "x2 size collision"),
    ("saTan2KabutoWareMove",    "sa_tan2",   215, "MED", ""),
    ("saTan0KiWareMoveA",       "sa_tan0",   212, "MED", "x2 size collision"),
    ("saTan0KiWareMoveB",       "sa_tan0",   212, "MED", "x2 size collision"),
    ("saTan0GaugeDraw",         "sa_tan0",   164, "MED", ""),
    ("saTan2Main",              "sa_tan2",   247, "MED", ""),
    ("myRobGeneiMove",          "my_eff",    134, "MED", ""),
    # Kengo affinity batch HIGH (2026-03-30)
    ("single_game_CheckStatusUpDataTotalOver", "nm_single_game", 289, "HIGH", ""),
    ("camera_set_zoom",                        "nm_camera",      219, "HIGH", ""),
    ("exec_game",                              "md_game",        194, "HIGH", ""),
    ("pad_FuncAnalog",                         "is_pad",         173, "HIGH", ""),
    ("coli_hit_body_weapon",                   "is_coli",        148, "HIGH", ""),
    ("single_game_SetAbilityData",             "nm_single_game", 124, "HIGH", ""),
    ("Pad_Prs",                                "is_pad",         111, "HIGH", ""),
    ("md_game_check_change_main_mode_katinuki","md_game",        103, "HIGH", ""),
    ("se_data_set",                            "md_game",         93, "HIGH", ""),
    ("coli_check_circle_hit_line",             "is_coli",         92, "HIGH", ""),
    ("pad_ClearAppliBuffer",                   "is_pad",          87, "HIGH", ""),
    ("md_game_check_change_sub_mode",          "md_game",         87, "HIGH", ""),
    ("damage_DebugDisp",                       "is_damage_calc",  79, "HIGH", ""),
    ("cpu_check_tubazeri",   "nm_cpu",  76, "HIGH", "x2 size collision"),
    ("cpu_check_tubazeri_2", "nm_cpu",  76, "HIGH", "x2 size collision"),
    ("cpu_get_dist",         "nm_cpu",  68, "HIGH", "x2 size collision"),
    ("cpu_get_dist_2",       "nm_cpu",  68, "HIGH", "x2 size collision"),
    ("cpu_check_same_dir_timer",       "nm_cpu",          63, "HIGH", ""),
    ("cpu_set_move_command_and_dir",   "nm_cpu",          57, "HIGH", ""),
    ("replay_camera_Init",             "nm_replay_cam",   39, "HIGH", ""),
    # Kengo affinity batch MED (2026-03-30)
    ("saTan0Main",       "sa_tan0",    233, "MED", ""),
    ("SetPacketData",    "am_rmd",     159, "MED", ""),
    ("saTan5TakeAnim2",   "sa_tan5",  154, "MED", "x2 size collision"),
    ("saTan5TakeAnim2_2", "sa_tan5",  154, "MED", "x2 size collision"),
    ("hirahira_w_ctrl",   "my_hirahira", 132, "MED", "x2 size collision"),
    ("hirahira_w_ctrl_2", "my_hirahira", 132, "MED", "x2 size collision"),
    ("mot_data_set",     "se_fc",      110, "MED", ""),
    ("rob_life_ctrl",    "my_rob",      96, "MED", "x2 size collision"),
    ("rob_life_ctrl_2",  "my_rob",      96, "MED", "x2 size collision"),
    ("rob_calc_2d_position", "my_rob",  93, "MED", ""),
    ("SetBloodSpot",     "am_rmd",      91, "MED", ""),
    ("saEft01Init",      "sa_eft",      91, "MED", ""),
    ("tslPolyF4Init",    "tsl_pkt",     81, "MED", ""),
    ("PutShadowRmd",     "am_rmd",      78, "MED", ""),
    ("calc_fc_frame",    "se_fc",       72, "MED", ""),
    ("saTan4GaugeInit",  "sa_tan4",     66, "MED", ""),
    ("hirahira_w_frie",  "my_hirahira", 59, "MED", ""),
    ("AddTbpOfst",       "am_rmd",      49, "MED", ""),
]

# Build lookup: func_name -> comment line
comments = {}
for name, module, insns, conf, note in ANNOTATIONS:
    parts = [f"kengo:{conf}", f"{module}/{name}", f"{insns}i"]
    if note:
        parts.append(note)
    comments[name] = "/* " + "  |  ".join(parts) + " */"

include_re = re.compile(
    r'^(\s*INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)\s*;[^\n]*)\n',
    re.MULTILINE
)

total_added = 0

for c_path in sorted((ROOT / "src").glob("*.c")):
    text = c_path.read_text(encoding="utf-8", errors="replace")
    changed = False
    new_lines = []

    for line in text.splitlines(keepends=True):
        new_lines.append(line)
        m = re.match(r'\s*INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)\s*;', line)
        if m:
            func_name = m.group(1)
            if func_name in comments:
                # Check if comment already present on next line
                # (we'll check after building; simpler: check if it's already there)
                indent = len(line) - len(line.lstrip())
                comment_line = " " * indent + comments[func_name] + "\n"
                new_lines.append(comment_line)
                total_added += 1
                changed = True

    if changed:
        new_text = "".join(new_lines)
        # Deduplicate: remove double-inserted comments (idempotent re-runs)
        # If the comment appears twice in a row, keep one
        dup_re = re.compile(r'(/\* kengo:[^\n]+\n)\1')
        new_text = dup_re.sub(r'\1', new_text)

        rel = c_path.relative_to(ROOT)
        print(f"  {rel}")
        if not DRY:
            c_path.write_text(new_text, encoding="utf-8")

mode = "DRY RUN" if DRY else "APPLIED"
print(f"\n{mode}: {total_added} comments added")
if DRY:
    print("Pass --apply to write.")
