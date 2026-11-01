"""wave116 rows (func-second-pass-24): func_8003F218 (the tracked-fighter-view flag setter), func_800430E4 (per-matrix-section draw
wrapper) and func_8004DDB4 (INCLUDE_ASM, the per-matrix-section colour / project / dispatch routine).
Research input tmp/codex/func-second-pass-24-research.md (untrusted); every code claim re-derived at HEAD b307048d0 (see notes.txt).
Lane naming-a is concurrently renaming D_800A3790 -> g_gpu_draw_option_mask (var-globals-q); none of these names depend on it.

Re-derived code facts:
- func_8003F218 (src/main/2B344.c:2313-2325): `if ((u32)a0 >= 2) return; if (a0 == g_gpu_tracked_fighter_view_active) return;
  g_gpu_tracked_fighter_view_active = a0; if (!a0) gpu_SetFarViewGridFogControlAndCoordinateShift(0); D_800F665C = (s16)flag;`.
  The callee (:2303-2311) writes control 3 / shift 2 for non-zero and control 0 / shift 1 for zero, and camera_UpdateTrackedFighterView's
  sibling code passes it view_record->is_far_view (9F9C.c:575), so the 0 arm is the callee's own non-far-view setting. R6's
  ...ResetGridFogOnClear (wave115) fails on both words: the shift (readers are the node projectors, not grid/fog code) is not grid/fog,
  and the init path func_80046B44 stores shift 2, so 1 is a mode setting, not a reset.
- func_800430E4 (32D04.c:486-511; asm/funcs/func_800430E4.s): t0 = *(0x1F800008); *(0x1F800008) = 3 - t0 (that word is the OT depth
  shift: gpu_RemapDepthToOtIndex 32D04.c:526 `depth >> *(s32 *)0x1F800008`; not restored); copies g_gpu_flush_view_matrix to 0x1F8003A0,
  halves m[1][0..2] and t[1] (asm func_800430E4.s:46-63); g_light_selected_setup = &g_light_setups[a2]; handler table 0x1F80001C = D_80095280 if
  arg3[1] & 1 else D_800951D8; func_8004DDB4(a0, a1, 0x1F8003A0, t0); restores 0x1F80001C = g_gpu_default_primitive_handlers. One caller.
- func_8004DDB4 (INCLUDE_ASM src/main/3AB48.c:35; asm/funcs/func_8004DDB4.s:1-513): a0 -> fp matrix-pointer array, a1 -> s0 stream,
  a2 -> s7 view matrix, a3 -> s1 shift. gte_ColorGroupedNormalStreamWithSelectedMatrices(stream + 2, a0) (:22-24); 13 sections
  (s5 = 0xC down to -1, fp += 4 per section; :17, :497), each reading *fp as a MATRIX (halfwords 0-0x10, words 0x14-0x1C minus
  g_view_camera_root_ptr->0x2C/0x30/0x34, srav by the shift; :24-52+), gte_MulMatrix0ClearTrans (:77), mvmva / rtps / rtpt vertex groups
  into the scratchpad buffers at 0x1F800020 (+0x224), then (count,type) packets through the table at 0x1F80001C via jalr (:446).
  One caller (asm/funcs/func_800430E4.s:82).
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 3/3 (fresh default-FAIL reviewer, lane naming-b 2026-11-01)"
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


def IA(fn, f):
    """src/main/<file>:<line> of the INCLUDE_ASM line of fn."""
    for k, l in enumerate(_SRC[f]):
        if l.startswith("INCLUDE_ASM") and re.search(r'\b%s\)' % fn, l):
            return f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no INCLUDE_ASM of {fn}")


_SET = (
    f"BODY: {B('func_8003F218', '2B344.c')}: `if ((u32)a0 >= 2) return; if (a0 == g_gpu_tracked_fighter_view_active) return; "
    "g_gpu_tracked_fighter_view_active = a0; if (!a0) gpu_SetFarViewGridFogControlAndCoordinateShift(0); D_800F665C = (s16)g_gpu_tracked_fighter_view_active;`. "
    "The callee (src/main/2B344.c:2303-2311) writes g_gpu_grid_and_fog_control = 3 / g_gpu_view_coordinate_shift = 2 for non-zero and 0 / 1 "
    "for zero; the camera passes it view_record->is_far_view (9F9C.c:575), so the zero arm is the callee's own non-far-view setting and "
    "writes both words. "
    "CALLERS (5 functions, 5 jal, no table): fight_Init src/main/9F9C.c:1900 (asm fight_Init.s:30) passes g_fight_tracked_view_setting; "
    "camera_ApplyFightView 9F9C.c:2101 (asm camera_ApplyFightView.s:66) passes g_camera_fight_view_mode < 1 inside the "
    "g_fight_tracked_view_setting != 0 block; replay_Start src/main/28708.c:1313 (asm replay_Start.s:8), "
    "fight_InitNearestStartPairPlayback src/main/2B344.c:1074 (asm :19), motion_StartPlayback src/main/43500.c:812 (asm :125) pass 0. "
    "WHY: Set...Flag = the guarded 0/1 store (other values and an unchanged value return early); TrackedFighterView = the landed noun "
    "of g_gpu_tracked_fighter_view_active and gpu_GetTrackedFighterViewFlag, which every caller's argument encodes; "
    "AndApplyNonFarViewGridFogAndShiftOnClear = the zero-only call, worded off the callee's own name for its zero arm "
    "(grid/fog control AND coordinate shift, so both are in the name; no 'Reset', since func_80046B44 (368E4.c:442-443) stores shift 2 "
    "at init and this arm stores 1). The write-only mirror D_800F665C (no reader in src or asm) states no effect and is left out. "
    "COUNTER-EVIDENCE: (1) camera code calls the same callee every frame (9F9C.c:575 and others), so the clear-side call can be "
    "overwritten later the same frame; the name claims the call, not a lasting state. (2) 'OnClear' = on a change to 0 (the unchanged-value "
    "early return precedes the call), not on every call with 0. (3) Long name (the census already holds 78-80 character names)."
)

_DRAW = (
    f"BODY: {B('func_800430E4', '32D04.c')}: t0 = *(s32 *)0x1F800008; *(s32 *)0x1F800008 = 3 - t0; the view matrix g_gpu_flush_view_matrix is "
    "copied to 0x1F8003A0 and m[1][0..2] and t[1] are halved (>>= 1); g_light_selected_setup = &g_light_setups[light_setup_index]; "
    "*(s32 *)0x1F80001C = (arg3[1] & 1) ? D_80095280 : D_800951D8; func_8004DDB4(arg0, arg1, dst, t0); *(s32 *)0x1F80001C = "
    "g_gpu_default_primitive_handlers. 0x1F800008 is the OT depth shift (gpu_RemapDepthToOtIndex src/main/32D04.c:526 `depth >> "
    "*(s32 *)0x1F800008`) and is left complemented (not restored); the callee receives the old value t0 as its coordinate shift. "
    "USE (1 jal, no table): asm/funcs/func_8004A940.s:485 (0x8004B0B4) in case L8004AF84 of jtbl_8001541C, with a0 = record+0x58, "
    "a1 = g_reloc_block_entry_tables[..][..] (the stream), a2 = record s16@0xA, a3 = the record (:473-486); v0 returned as the stream cursor (:487). "
    "WHY: Draw = the call to func_8004DDB4 (per matrix section, see its row) under the installed handler table, which is restored afterwards; "
    "PerMatrixSections = the callee's matrix-pointer-array walk; WithHalvedYView = the scratch view matrix with row 1 and t[1] halved that "
    "the callee receives; AndComplementedDepthShift = the 3 - x store to the OT depth shift word. Light-setup selection and the table choice "
    "are inputs to the callee's colour step and dispatch, so Draw covers them. "
    "COUNTER-EVIDENCE: why Y is halved is not shown, so the name states only the mechanics. 'PerMatrixSections' rests on the func_8004DDB4 row "
    "(same wave, land together or drop this row). No visual words: nothing in the code types the record as a fighter."
)

_PROJ = (
    f"BODY (INCLUDE_ASM {IA('func_8004DDB4', '3AB48.c')}; asm/funcs/func_8004DDB4.s): a0 = fp matrix-pointer array, a1 = s0 stream, a2 = s7 view matrix, "
    "a3 = s1 shift (:13-16). (1) jal gte_ColorGroupedNormalStreamWithSelectedMatrices(stream+2, a0) (:22-24); s0 = returned end - 2 (:25). "
    "(2) 13 sections: s5 = 0xC (:17), fp += 4 and s5 -= 1 per section (:497). Per section s2 = *fp read as a MATRIX: rotation halfwords at 0-0x10, "
    "translation words at 0x14-0x1C minus g_view_camera_root_ptr->0x2C/0x30/0x34, srav by the shift (:27-52+), gte_MulMatrix0ClearTrans "
    "(:77), then count-prefixed vertex groups through mvmva (:102, :141, :202, :262) / rtps (:226, :318) / rtpt (:370, :374) into the "
    "scratchpad buffers 0x1F800020 (+0x224), then (count,type) packets dispatched through the table at 0x1F80001C by jalr (:446), then "
    "0xFFFF-terminated slot-copy blocks (:453-483). Returns the stream end. "
    "USE (1 jal, no table): asm/funcs/func_800430E4.s:82 (= src/main/32D04.c:508, decl 32D04.c:479). "
    "WHY: ColorNormals = its first act, worded off the callee gte_ColorGroupedNormalStreamWithSelectedMatrices; ProjectVertices = the "
    "mvmva/rtps/rtpt groups into the vertex buffers; PerMatrixSection = one pass per matrix of the pointer array; AndDispatchPackets = the "
    "jalr loop over the 0x1F80001C handler table; the siblings gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets / "
    "gpu_ProjectStridedNodeVerticesAndDispatchPackets (3AB48.c:25, :34) carry the same project-and-dispatch words. "
    "COUNTER-EVIDENCE: the meaning of the slot-copy blocks between sections (seam vertices?) is not shown; they are bookkeeping on the "
    "buffers the name already names. Some groups stay in view space (mvmva only), the same looseness the siblings' names carry. "
    "No visual words."
)

FUNCS = [
    ("0x8003F218", "gpu_SetTrackedFighterViewFlagAndApplyNonFarViewGridFogAndShiftOnClear", "RENAME", _SET),
    ("0x800430E4", "gpu_DrawPerMatrixSectionsWithHalvedYViewAndComplementedDepthShift", "RENAME", _DRAW),
    ("0x8004DDB4", "gpu_ColorNormalsAndProjectPerMatrixSectionVerticesAndDispatchPackets", "RENAME", _PROJ),
]

DATA = []
