#!/usr/bin/env python3
"""
Generate m2c context file with inferred function signatures.

Analyzes each asm stub to determine:
- If a0 is used as a struct pointer (GameObj*)
- If a1/a2 are used as struct pointers
- Return type (void vs s32)

Writes include/m2c_context.h with struct definitions + function signatures.

Usage: python3 tools/gen_m2c_context.py
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_DIR = ROOT / "asm" / "funcs"

# Types that are safe to use in m2c context (defined in the header)
KNOWN_TYPES = {
    'void', 'u8', 's8', 'u16', 's16', 'u32', 's32', 'u64', 's64',
    'vu8', 'vs8', 'vu16', 'vs16', 'vu32', 'vs32',
    'GameObj', 'Vec3s16', 'Vec3s32', 'Vec2s16',
}

# Struct definition — hardcoded to avoid circular read/overwrite issues
STRUCT_DEFS = r"""
/* -----------------------------------------------------------------------
 * GameObj: Polymorphic game object struct (0x100 bytes)
 *
 * Accessed via a0/s0 in ~340+ functions across the codebase.
 * Used for multiple object types (characters, cameras, effects, particles)
 * with field semantics varying by object type.
 *
 * NAMING EVIDENCE SOURCES:
 *   - Kengo (PS2 sequel) debug symbols: rob_*, motion_*, camera_*, coli_*
 *   - Named BB2 functions: cpu_get_dist, rob_life_ctrl, efc_rob_set_type_flash
 *   - Decompiled C code: func_8001A538 (rotation), func_8002EECC (matrix inverse)
 *   - Assembly instruction patterns across 1,466 functions
 *
 * POLYMORPHIC FIELD OVERLAY:
 *   Field 0x00 is accessed as lw (323 funcs), lh (180), lb (74):
 *     - As lw: pointer to linked object (opponent in rob, parent in effect)
 *     - As lbu: object type/status flag
 *     - As lhu: packed type+subtype
 *
 * OBJECT TYPE USAGE:
 *   Rob (character):  0x00=opponent_ptr, 0x0C=mode, 0x0E=sub_mode, 0x6A=action_id
 *   Camera:           0x60=target0_ptr, 0x64=target1_ptr, 0x68=target2_ptr
 *   Effect/particle:  0x04=active_flag, 0x2C=pos, 0x44=vel, 0x5C=ang_vel
 *   3x3 matrix:       0x00-0x10 as 9 s16 entries (3x3 rotation)
 *
 * ----------------------------------------------------------------------- */
typedef struct GameObj {
    /* ===== 0x00-0x03: Object identity / status flags (542 funcs total) ===== */

    /* 0x00 */ u8 type;             /* object type flag; lbu in 74 funcs (action_CheckHitZangeki,
                                       cpu_check_same_dir_timer, marionation_Exec)
                                       NOTE: also accessed as lw (323 funcs) for linked pointer
                                       and lhu (180 funcs) for packed type+sub_type */
    /* 0x01 */ u8 sub_type;         /* sub-type/variant; 37 funcs; cpu_check_same_dir_timer checks,
                                       marionation_Exec reads; md_game_check_mode clears */
    /* 0x02 */ s16 state;           /* object state; 120 funcs; sh in md_game_check_mode,
                                       efc_rob_set_type_flash reads as lhu; rob_life_ctrl reads;
                                       also used as matrix entry m00 in func_8002EECC */
    /* ===== 0x04-0x16: Core transform / matrix data (316+ funcs) ===== */
    /*
     * When used as a 3x3 rotation matrix (func_8002EECC matrix inverse):
     *   [m00 m01 m02]   [0x00 0x02 0x04]   (but 0x00 overlaps type field)
     *   [m10 m11 m12] = [0x06 0x08 0x0A]
     *   [m20 m21 m22]   [0x0C 0x0E 0x10]
     *
     * When used as rob/game object (most common usage):
     *   0x04-0x0A: secondary transform or collision data
     *   0x0C-0x0E: mode/sub_mode IDs
     *   0x10-0x14: rotation angles
     */

    /* 0x04 */ s16 field_04;        /* 316 funcs; heavily used in collision (coli_check_circle_hit_line),
                                       camera (camera_set_zoom), transform (calc_loc_mat_fw);
                                       also matrix entry m01; lh dominant */
    /* 0x06 */ s16 field_06;        /* 126 funcs; rob_life_ctrl, cpu_check_run_attack;
                                       efc: cleared to 0; also matrix entry m02 */
    /* 0x08 */ s16 field_08;        /* 238 funcs; rob_life_ctrl uses in limb index lookup;
                                       mot_data_set loads; pad_main_control accesses;
                                       also matrix entry m10 */
    /* 0x0A */ s16 field_0A;        /* 100 funcs; cpu_check_same_dir_timer, hirahira_w_ctrl_2;
                                       also matrix entry m11 */
    /* 0x0C */ s16 mode;            /* 181 funcs; md_game_rob_data_init compares against 0x1D, 0x1F, 0x0E
                                       (mode IDs); md_game_check_mode sets; saTan2KabutoWareMove reads;
                                       for matrix context = m20 */
    /* 0x0E */ s16 sub_mode;        /* 81 funcs; md_game_rob_data_init checks (lhu, subtract 6, range test);
                                       hirahira_w_ctrl_2, rob_life_ctrl;
                                       for matrix context = m21 */

    /* 0x10 */ s16 rot_x;           /* 149 funcs; CONFIRMED rotation X: func_8001A538 passes -field_10
                                       to func_8007F87C (RotateX); efc_rob_set_type_flash stores from
                                       data table; rob_life_ctrl reads via lhu;
                                       for matrix context = m22 */
    /* 0x12 */ s16 rot_y;           /* 55 funcs; CONFIRMED rotation Y: func_8001A538 passes -field_12
                                       to func_8007FA1C (RotateY); efc uses; rob_life_ctrl_2 uses
                                       as multiplier for limb scaling */
    /* 0x14 */ s16 rot_z;           /* 129 funcs; CONFIRMED rotation Z: func_8001A538 passes -field_14
                                       to func_8007FBBC (RotateZ); AllocRobRmd reads; DispPracticeMenuTex_A;
                                       rob_life_ctrl reads via lhu */
    /* 0x16 */ s16 field_16;        /* 25 funcs; AllocRobRmd reads; saTan1MainJump, saTan2Main reads lbu;
                                       low traffic, possibly padding or scale factor */

    /* ===== 0x18-0x2C: Position / distance (98 funcs at 0x18) ===== */

    /* 0x18 */ s32 dist;            /* 98 funcs; func_8001E6E4 camera code: local.dist = *(s32*)(s2+0x18);
                                       efc_rob_set_type_flash: dest for func_8007EC5C (transform);
                                       marionation_camera_Exec stores */
    /* 0x1C */ s32 field_1C;        /* 92 funcs; AllocRobRmd stores pointer; calc_loc_mat_fw;
                                       marionation_camera_Exec stores; myRobGeneiMove clears */
    /* 0x20 */ s32 field_20;        /* 54 funcs; calc_loc_mat_fw; marionation_camera_Exec;
                                       AllocRobRmd loads */
    /* 0x24 */ s32 field_24;        /* 50 funcs; calc_loc_mat_fw; marionation_camera_Exec;
                                       SetCurrentCursor stores */
    /* 0x28 */ s32 field_28;        /* 35 funcs; calc_loc_mat_fw; marionation_camera_Exec */
    /* 0x2C */ s32 pos_x;           /* 37 funcs; CONFIRMED position X: cpu_set_move_command_and_dir
                                       copies Vec3_copy here from a1; SetBloodSpot source position;
                                       efc_rob_set_type_flash copies; calc_loc_mat_fw reads */

    /* ===== 0x30-0x42: Extended transform / sub-object data ===== */

    /* 0x30 */ s16 pos_y;           /* 39 funcs; part of position vector with pos_x; md_game_check_mode
                                       stores; DispPracticeMenuTex_A; saTan1MainJump reads as lhu */
    /* 0x32 */ s16 pos_z;           /* 7 funcs; continues position triple; DispPracticeMenuTex_A;
                                       saTan1MainJump reads */
    /* 0x34 */ s16 field_34;        /* 37 funcs; calc_loc_mat_fw; md_game_check_mode stores;
                                       saTan1MainJump reads */
    /* 0x36 */ s16 field_36;        /* 7 funcs; saTan1MainJump reads */
    /* 0x38 */ s16 field_38;        /* 22 funcs; DispPracticeMenuTex_A; calc_loc_mat_fw;
                                       saTan1MainJump reads */
    /* 0x3A */ s16 field_3A;        /* 6 funcs; DispPracticeMenuTex_A; saTan1MainJump */
    /* 0x3C */ s16 field_3C;        /* 24 funcs; DispPracticeMenuTex_A; calc_loc_mat_fw;
                                       saTan1MainJump reads */
    /* 0x3E */ s16 field_3E;        /* 4 funcs; low traffic */
    /* 0x40 */ s16 field_40;        /* 25 funcs; calc_loc_mat_fw; cpu_check_same_dir_timer reads */
    /* 0x42 */ s16 field_42;        /* 8 funcs */

    /* ===== 0x44-0x50: Velocity / movement vector (21 funcs at 0x44) ===== */

    /* 0x44 */ s32 vel_x;           /* 21 funcs; CONFIRMED velocity X: cpu_get_dist rotates (vel_x,vel_z)
                                       by angle using sin/cos LUT; cpu_set_move_command_and_dir assigns
                                       randomized values; calc_loc_mat_fw stores */
    /* 0x48 */ s32 vel_y;           /* 20 funcs; CONFIRMED velocity Y: cpu_get_dist damps (>>2);
                                       cpu_set_move_command_and_dir assigns -(rng & 0x3F) - 0x80;
                                       rob_life_ctrl stores scaled value */
    /* 0x4C */ s32 vel_z;           /* 24 funcs; CONFIRMED velocity Z: cpu_get_dist rotates with vel_x;
                                       cpu_set_move_command_and_dir assigns; rob_life_ctrl stores */
    /* 0x50 */ s32 field_50;        /* 21 funcs; cpu_side_move_dir loads; rob_life_ctrl stores scaled value;
                                       single_game_CheckStatusUpDataTotalOver loads */

    /* ===== 0x54-0x60: Angular velocity / counters ===== */

    /* 0x54 */ s16 field_54;        /* 20 funcs; saTan4GaugeInit reads */
    /* 0x56 */ s16 field_56;        /* 6 funcs */
    /* 0x58 */ s32 timer;           /* 26 funcs; efc_rob_set_type_flash loads as frame index/timer;
                                       cpu_check_same_dir_timer loads; cpu_get_dist_2 loads;
                                       md_game_check_mode clears (sb zero) */
    /* 0x5C */ s16 ang_vel_x;      /* 17 funcs; CONFIRMED angular velocity: cpu_set_move_command_and_dir
                                       assigns random rotation speed (0x200..0x5FF or negative);
                                       efc_rob_set_type_flash reads */
    /* 0x5E */ s16 ang_vel_y;      /* 12 funcs; CONFIRMED: cpu_set_move_command_and_dir assigns
                                       (rng & 0x7FF) - 0x400; cpu_side_move_dir reads;
                                       single_game_CheckStatusUpDataTotalOver reads */
    /* 0x60 */ s32 target0_ptr;     /* 25 funcs; DUAL USE:
                                       - For effects: s16 ang_vel_z (cleared to 0 by cpu_set_move_command_and_dir)
                                       - For cameras: pointer to target0 position (special_camera_Init,
                                         saTan0KiWareMoveA/B load and dereference as Vec3) */
    /* 0x64 */ s32 target1_ptr;     /* 14 funcs; camera context: pointer to target1 position
                                       (special_camera_Init dereferences); calc_loc_mat_fw stores */
    /* 0x68 */ s32 target2_ptr;     /* 13 funcs; camera context: pointer to target2 position
                                       (special_camera_Init dereferences) */

    /* ===== 0x6C-0x80: Extended data / checksum ===== */

    /* 0x6C */ s32 field_6C;        /* 9 funcs */
    /* 0x70 */ s32 field_70;        /* 6 funcs */
    /* 0x74 */ s32 field_74;        /* 9 funcs; camera_set_zoom stores; single_game stores */
    /* 0x78 */ s32 field_78;        /* 10 funcs; damage_DebugDisp loads; calc_loc_mat_fw reads/writes */
    /* 0x7C */ s32 field_7C;        /* 11 funcs; calc_loc_mat_fw reads/writes */
    /* 0x80 */ s32 field_80;        /* 9 funcs; calc_loc_mat_fw reads/writes */

    /* ===== 0x84-0x96: Motion / animation frame data ===== */

    /* 0x84 */ s16 mot_frame_start; /* 11 funcs; func_80027A58: field_86 is reset to field_84 value;
                                       func_8002872C copies to field_86; animation frame start */
    /* 0x86 */ s16 mot_frame;       /* 8 funcs; func_80027A58: compared against field_88;
                                       loaded, checked, reset to field_84; current motion frame */
    /* 0x88 */ s16 mot_frame_end;   /* 18 funcs; func_80027A58: compared against field_86;
                                       cpu_check_tubazeri_2 reads; end frame of animation */
    /* 0x8A */ s16 mot_frame_delta; /* 8 funcs; func_80027A58: checked for non-zero, then cleared;
                                       frame increment/delta */
    /* 0x8C */ s32 field_8C;        /* 10 funcs; md_game_rob_data_init checks non-zero;
                                       saTan2KabutoWareMove reads; calc_loc_mat_fw writes */
    /* 0x90 */ s32 field_90;        /* 9 funcs; saTan4GaugeInit accesses */
    /* 0x94 */ s32 field_94;        /* 7 funcs; camera_set_zoom writes; single_game writes */
    /* 0x98 */ s32 field_98;        /* 5 funcs */
    /* 0x9C */ s32 field_9C;        /* 4 funcs */

    /* ===== 0xA0-0xAC: CPU AI / distance data ===== */

    /* 0xA0 */ s32 field_A0;        /* 5 funcs */
    /* 0xA4 */ s32 field_A4;        /* 6 funcs */
    /* 0xA8 */ s32 delta0_x;        /* 16 funcs; special_camera_Init: target1.x - target0.x;
                                       saTan0KiWareMoveA loads pair; DispSchoolBG stores;
                                       saSeInit computes; delta vector component X */
    /* 0xAC */ s32 delta0_y;        /* 16 funcs; special_camera_Init: target1.y - target0.y;
                                       delta vector component Y */
    /* 0xB0 */ s32 delta0_z;        /* 14 funcs; special_camera_Init: target1.z - target0.z;
                                       delta vector component Z */

    /* ===== 0xB4-0xC0: Secondary delta / transform vectors ===== */

    /* 0xB4 */ s32 field_B4;        /* 6 funcs; calc_loc_mat_fw loads */
    /* 0xB8 */ s32 delta1_x;        /* 20 funcs; special_camera_Init: target2.x - target0.x;
                                       camera_set_zoom loads; cpu_side_move_dir reads/writes;
                                       saTan0KiWareMoveA loads */
    /* 0xBC */ s32 delta1_y;        /* 23 funcs; special_camera_Init: target2.y - target0.y;
                                       camera_set_zoom loads; single_game loads */
    /* 0xC0 */ s32 delta1_z;        /* 18 funcs; special_camera_Init: target2.z - target0.z;
                                       camera_set_zoom loads; cpu_side_move_dir reads/writes */

    /* ===== 0xC4-0xD4: Interpolation / spline data ===== */

    /* 0xC4 */ s32 field_C4;        /* 5 funcs; calc_loc_mat_fw loads */
    /* 0xC8 */ s32 interp_x;        /* 10 funcs; camera_set_zoom loads; saTan0KiWareMoveB does
                                       interpolation: interp_x += (delta0_x - interp_x) >> shift */
    /* 0xCC */ s32 interp_y;        /* 8 funcs; same interpolation pattern as interp_x */
    /* 0xD0 */ s32 interp_z;        /* 13 funcs; same interpolation pattern; camera_set_zoom loads;
                                       calc_loc_mat_fw stores */
    /* 0xD4 */ s32 field_D4;        /* 4 funcs */

    /* ===== 0xD8-0xE8: Rotation matrix (3x3 s16 with padding) ===== */
    /* Pattern from DispSchoolBG/saSeInit/special_camera_Init:
     *   D8=val, DA=0, DC=0, DE=0, E0=val, E2=0, E4=0, E6=0, E8=val
     * This is a diagonal rotation matrix stored as 3 rows of {s16,s16,s16,pad} */

    /* 0xD8 */ s32 rot_m00_m01;     /* 19 funcs; packed s16 pair: rotation matrix [0][0] and [0][1];
                                       DispSchoolBG sets diag; saSeInit sets; special_camera_Init */
    /* 0xDC */ s32 rot_m02_m10;     /* 15 funcs; packed s16 pair: rotation matrix [0][2] and [1][0] */
    /* 0xE0 */ s32 rot_m11_m12;     /* 16 funcs; packed s16 pair: rotation matrix [1][1] and [1][2] */
    /* 0xE4 */ s32 rot_m20_m21;     /* 12 funcs; packed s16 pair: rotation matrix [2][0] and [2][1] */
    /* 0xE8 */ s32 rot_m22_pad;     /* 14 funcs; packed s16 pair: rotation matrix [2][2] and pad */

    /* ===== 0xEC-0xFC: World position / final state ===== */

    /* 0xEC */ s32 field_EC;        /* 2 funcs; low traffic */
    /* 0xF0 */ s32 field_F0;        /* 3 funcs */
    /* 0xF4 */ s32 world_pos_x;     /* 18 funcs; md_game_rob_data_init: loaded from self and opponent (via
                                       field_00 ptr) at 0xF4; distance squared computed;
                                       cpu_side_move_dir reads/writes; used in position calculations */
    /* 0xF8 */ s16 screen_x;        /* 20 funcs; saTan0KiWareMoveA/B: delta projected to screen coords;
                                       DispSchoolBG reads; saSeInit sets; special_camera_Init sets;
                                       GTE mvmva output stored here */
    /* 0xFA */ s16 screen_y;        /* 11 funcs; same pattern as screen_x; DispSchoolBG reads */
    /* 0xFC */ s32 world_pos_z;     /* 21 funcs; md_game_rob_data_init: same usage as world_pos_x;
                                       cpu_side_move_dir reads/writes;
                                       single_game_CheckStatusUpDataTotalOver loads */
} GameObj;

/* Short vector types used for angles, positions, etc. */
typedef struct Vec3s16 {
    s16 x;
    s16 y;
    s16 z;
} Vec3s16;

typedef struct Vec3s32 {
    s32 x;
    s32 y;
    s32 z;
} Vec3s32;

typedef struct Vec2s16 {
    s16 x;
    s16 y;
} Vec2s16;
"""


def get_field_accesses(asm_path, reg):
    """Get all offsets accessed via a given register in a function."""
    content = open(asm_path).read()
    offsets = {}
    for m in re.finditer(
        r'(lw|sw|lh|sh|lb|sb|lhu|lbu)\s+\$\w+,\s*(-?(?:0x[0-9a-fA-F]+|\d+))\(\$' + reg + r'\)',
        content
    ):
        instr, offset_str = m.groups()
        offset = int(offset_str, 16) if '0x' in offset_str.lower() else int(offset_str)
        if 0 <= offset < 512:
            if offset not in offsets:
                offsets[offset] = set()
            offsets[offset].add(instr)
    return offsets


def check_reg_saved(asm_path, src_reg, dst_reg):
    """Check if src_reg is saved to dst_reg early in the function."""
    content = open(asm_path).read()
    lines = content.split('\n')[:15]
    for line in lines:
        if re.search(r'addu\s+\$' + dst_reg + r',\s*\$' + src_reg + r',\s*\$zero', line) or \
           re.search(r'move\s+\$' + dst_reg + r',\s*\$' + src_reg, line) or \
           re.search(r'or\s+\$' + dst_reg + r',\s*\$' + src_reg + r',\s*\$zero', line):
            return True
    return False


def is_struct_pointer(asm_path, reg):
    """Check if reg is used as a GameObj struct pointer.

    Also checks if the register is saved to a callee-saved register
    (a0->s0, a1->s1, etc.) and then used from there.
    """
    offsets = get_field_accesses(asm_path, reg)

    # Check callee-saved register mapping
    save_map = {'a0': ['s0', 's1', 's2'], 'a1': ['s1', 's2', 's3'], 'a2': ['s2', 's3']}
    for dst in save_map.get(reg, []):
        if check_reg_saved(asm_path, reg, dst):
            saved_offsets = get_field_accesses(asm_path, dst)
            offsets.update(saved_offsets)
            break

    # Need 3+ distinct field accesses to be a struct pointer
    if len(offsets) < 3:
        return False

    # Check that offsets are in GameObj range (0x00-0xFC)
    gameobj_offsets = [o for o in offsets if 0 <= o <= 0xFC]
    return len(gameobj_offsets) >= 3


def infer_return_type(asm_path):
    """Check if function returns a value in $v0."""
    content = open(asm_path).read()
    lines = content.strip().split('\n')

    # Find the jr $ra instruction
    jr_idx = None
    for i, line in enumerate(lines):
        if re.search(r'jr\s+\$ra', line):
            jr_idx = i
            break

    if jr_idx is None:
        return 'void'

    # Check the delay slot and 2 instructions before jr $ra
    check_range = lines[max(0, jr_idx - 3):jr_idx + 2]
    for line in check_range:
        # v0 is explicitly set near the return
        if re.search(r'(lw|lh|lhu|lb|lbu|addiu|addu|subu|sll|srl|sra|ori|andi|sltu?i?|move)\s+\$v0,', line):
            return 's32'

    return 'void'


def count_args_conservative(asm_path):
    """Count arguments by checking which $aN regs are read before being overwritten.

    Conservative: only counts if the register is clearly used as input.
    """
    content = open(asm_path).read()
    lines = content.split('\n')

    # Track first use of each arg register
    first_read = {}
    first_write = {}
    line_num = 0

    for line in lines:
        # Skip non-instruction lines
        if not re.search(r'\*/\s+\w', line):
            continue
        line_num += 1

        for reg in ['a0', 'a1', 'a2', 'a3']:
            # Skip if both already found
            if reg in first_read and reg in first_write:
                continue

            # Check for read: used as base register or source operand
            is_read = False
            if re.search(r'\(\$' + reg + r'\)', line):
                is_read = True
            # Source in arithmetic/logic: op $dst, $aN, ... or op $dst, ..., $aN
            if re.search(r',\s*\$' + reg + r'(?:\s|,|$)', line):
                is_read = True
            # Used in sw/sh/sb as source: sw $aN, offset($X)
            if re.search(r'(sw|sh|sb)\s+\$' + reg + r',', line):
                is_read = True

            # Check for write: reg is destination
            is_write = re.search(
                r'(lw|lh|lhu|lb|lbu|addiu|addu|subu|sll|srl|sra|move|or|andi|ori|lui|mflo|mfhi)\s+\$' + reg + r',',
                line
            ) is not None

            if is_read and reg not in first_read:
                first_read[reg] = line_num
            if is_write and reg not in first_write:
                first_write[reg] = line_num

    # An arg register is an input if it's read before (or without) being written
    arg_regs = ['a0', 'a1', 'a2', 'a3']
    count = 0
    for reg in arg_regs:
        if reg in first_read:
            if reg not in first_write or first_read[reg] <= first_write[reg]:
                count += 1
            else:
                break  # Written before read = not an input arg
        else:
            break  # Not used at all = no more args

    return max(count, 0)


def infer_arg_type(asm_path, reg):
    """Infer the type of an argument register."""
    if is_struct_pointer(asm_path, reg):
        return 'GameObj *'
    return 's32'


def find_all_stubs():
    """Find all INCLUDE_ASM stubs."""
    stubs = []
    for src in sorted(ROOT.glob("src/*.c")):
        with open(src) as f:
            for line in f:
                m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
                if m:
                    stubs.append(m.group(1))
    return stubs


def sanitize_type(type_str):
    """Replace unknown types with s32 or void*."""
    # Strip pointer suffix for checking
    base = type_str.replace('*', '').strip()
    if base in KNOWN_TYPES or base == '':
        return type_str
    # Replace unknown pointer types with void*
    if '*' in type_str:
        return 'void *'
    return 's32'


def find_decompiled_signatures():
    """Extract function signatures from already-decompiled code in src/*.c."""
    sigs = {}
    for src in sorted(ROOT.glob("src/*.c")):
        content = open(src).read()
        for m in re.finditer(
            r'^(\w[\w\s\*]*?)\s+(func_[0-9A-Fa-f]+)\s*\(([^)]*)\)\s*\{',
            content, re.MULTILINE
        ):
            ret_type, name, params = m.groups()
            ret_type = ret_type.strip()

            # Sanitize types
            ret_type = sanitize_type(ret_type)
            if params.strip() and params.strip() != 'void':
                sanitized_params = []
                for param in params.split(','):
                    param = param.strip()
                    # Split type from name
                    parts = param.rsplit(' ', 1)
                    if len(parts) == 2:
                        ptype, pname = parts
                        ptype = sanitize_type(ptype.strip())
                        sanitized_params.append(f"{ptype} {pname}")
                    else:
                        sanitized_params.append(sanitize_type(param))
                params = ', '.join(sanitized_params)

            sigs[name] = f"{ret_type} {name}({params})"
    return sigs


def main():
    stubs = find_all_stubs()
    decompiled_sigs = find_decompiled_signatures()

    print(f"Stubs: {len(stubs)}, Decompiled signatures: {len(decompiled_sigs)}")

    # Infer signatures for stubs
    signatures = {}
    gameobj_count = 0

    for func in sorted(stubs):
        asm_file = ASM_DIR / f"{func}.s"
        if not asm_file.exists():
            continue

        nargs = count_args_conservative(asm_file)
        ret_type = infer_return_type(asm_file)

        args = []
        for reg in ['a0', 'a1', 'a2', 'a3'][:nargs]:
            arg_type = infer_arg_type(asm_file, reg)
            if arg_type == 'GameObj *':
                gameobj_count += 1
            args.append(arg_type)

        param_str = ', '.join(args) if args else 'void'
        signatures[func] = f"{ret_type} {func}({param_str})"

    # Merge with decompiled signatures
    all_sigs = {}
    all_sigs.update(decompiled_sigs)
    all_sigs.update(signatures)  # stubs override if conflict (shouldn't happen)

    print(f"Stubs with GameObj* parameter: {gameobj_count}")

    # Generate the context file
    lines = []
    lines.append("/* m2c context: auto-generated by tools/gen_m2c_context.py")
    lines.append(" *")
    lines.append(" * This file is NOT included by the build — it is only used by m2c")
    lines.append(" * via --context to improve decompilation output.")
    lines.append(" */")
    lines.append("")

    # Basic types
    for typedef in [
        "typedef unsigned char u8;",
        "typedef signed char s8;",
        "typedef unsigned short u16;",
        "typedef signed short s16;",
        "typedef unsigned int u32;",
        "typedef signed int s32;",
        "typedef unsigned long long u64;",
        "typedef signed long long s64;",
        "typedef volatile u8 vu8;",
        "typedef volatile s8 vs8;",
        "typedef volatile u16 vu16;",
        "typedef volatile s16 vs16;",
        "typedef volatile u32 vu32;",
        "typedef volatile s32 vs32;",
    ]:
        lines.append(typedef)
    lines.append("")

    # Struct definitions
    lines.append(STRUCT_DEFS.strip())
    lines.append("")

    # Global arrays (indexed access patterns found in asm)
    lines.append("/* Global arrays (indexed access patterns found in asm) */")
    lines.append("extern s16 D_800973FC[];       /* 4096-entry sine LUT, 19 funcs */")
    lines.append("extern s32 D_800A9A10[];       /* 7 funcs */")
    lines.append("extern s32 D_800A3860[];       /* 6 funcs */")
    lines.append("extern s32 D_80106F28[];       /* 6 funcs */")
    lines.append("extern s16 D_80101F12[];       /* 5 funcs */")
    lines.append("extern s32 D_800A11DC[];       /* 5 funcs */")
    lines.append("extern u8  D_80102A68[];       /* 5 funcs */")
    lines.append("extern s32 D_80103608[];       /* 4 funcs */")
    lines.append("extern s32 D_800F66A0[];       /* 5 funcs */")
    lines.append("")

    # Top globals
    lines.append("/* Top referenced globals */")
    globals_list = [
        ("D_800A374C", 320), ("D_800A38B4", 171), ("D_800A387C", 77),
        ("D_8009BC7C", 71), ("D_80099D88", 65), ("D_8009BF48", 58),
        ("D_800A3834", 56), ("D_8009BD38", 47), ("D_8009BF7C", 46),
        ("D_800A3820", 44), ("D_8009BF78", 42), ("D_800A2CDC", 39),
        ("D_800F116C", 36), ("D_800A36AC", 36), ("D_8009C928", 36),
        ("D_8009BE6C", 36), ("D_800F1180", 35), ("D_800A3560", 35),
        ("D_8009BE70", 35),
    ]
    for name, refs in globals_list:
        lines.append(f"extern s32 {name};     /* {refs} refs */")
    lines.append("")

    # Function signatures
    lines.append("/* Function signatures */")
    for func in sorted(all_sigs.keys()):
        sig = all_sigs[func]
        lines.append(f"{sig};")

    output = '\n'.join(lines) + '\n'
    out_path = ROOT / "include" / "m2c_context.h"
    out_path.write_text(output)
    print(f"\nWrote {out_path} ({len(all_sigs)} function signatures)")

    # Stats
    gameobj_all = sum(1 for s in all_sigs.values() if 'GameObj' in s)
    void_ret = sum(1 for s in all_sigs.values() if s.startswith('void'))
    s32_ret = sum(1 for s in all_sigs.values() if s.startswith('s32'))
    print(f"  Total with GameObj*: {gameobj_all}")
    print(f"  void return: {void_ret}")
    print(f"  s32 return: {s32_ret}")


if __name__ == "__main__":
    main()
