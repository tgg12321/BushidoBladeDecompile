"""wave115 rows (func-second-pass-23): the D_800A322C flag setter / getter, func_8003F218 and func_8003F268.
Research input tmp/codex/func-second-pass-23-research.md (untrusted); every code claim re-derived at HEAD 0559b0765 (see notes.txt).
Lane naming-b is concurrently naming D_800A322C / D_800F665C (var-globals-p); these function names do not depend on them.

Re-derived code facts (src/main/2B344.c unless noted):
- Setter body :2312-2324: ignores (u32)a0 >= 2 and a0 == D_800A322C; stores a0 to D_800A322C (:2319); on a0 == 0 calls
  gpu_SetFarViewGridFogControlAndCoordinateShift(0) (:2321; that callee :2302-2310 sets g_gpu_grid_and_fog_control = 0,
  g_gpu_view_coordinate_shift = 1 for 0); then D_800F665C = (s16)D_800A322C (:2323; D_800F665C has no reader, only the writes
  :2323, 368E4.c:445 and asm func_80046B44.s:38). Getter :2326 returns D_800A322C.
- Setter arguments (all six jal sites): fight_Init 9F9C.c:1900 g_fight_tracked_view_setting (same function: FOV 0x50 if the setting != 0
  else 0x2D :1902; camera_InitTwoFighterView :2052; camera_InitTrackedFighterFocus :2054 only if the setting != 0);
  camera_ApplyFightView 9F9C.c:2101 `g_camera_fight_view_mode < 1`, inside the `g_fight_tracked_view_setting != 0` block (:2083-2118,
  else-branch uses g_camera_default_view_record), mode 0 -> g_camera_alternate_view_record :2095-2096 and FOV 0x50 :2103-2104, mode 1 / 2
  -> g_camera_default_view_record and FOV 0x2D; replay_Start 28708.c:1313, fight_InitNearestStartPairPlayback 2B344.c:1074,
  motion_StartPlayback 43500.c:812: constant 0, each followed by SetGeomScreen(math_FovToScreenDist(0x2D)).
  g_camera_alternate_view_record is the record camera_UpdateTrackedFighterView (9F9C.c:780-786) updates.
- Readers of D_800A322C: direct 2B344.c:1995 (grid mask table select D_800927C0 vs D_80090740) and :2071 (masked-grid pass gate);
  via the getter: 2B344.c:1480 (fog base 0x55F0 if set else 0x6590), :1545 (same pair), 32D04.c:1152 (OT cutoff 0xBE if set), :2343
  (result discarded).
Words: gpu_ (every reader is a gpu_ draw/fog/OT function; the callee is gpu_SetFarViewGridFogControlAndCoordinateShift; neighbours
:2298-2326 are gpu_ getters/setters). TrackedFighterView = the landed vocabulary (g_fight_tracked_view_setting,
camera_InitTrackedFighterFocus, camera_UpdateTrackedFighterView) for the state every writer's argument encodes (setting != 0 AND
view mode 0 / constant 0 on the third-person paths). "POV"/"FirstPerson" is left out: it is a menu / screenshot reading, not shown
at every writer or reader. D_800F665C (write-only mirror) is deliberately not named. Runtime capture 0353fb7ee
(pov-view-story-pov-rematch-1; capture.jsonl sha1 95982af2 verified here, D_800A322C writes 258 BIOS 0, 8714 fight_Init 1,
12548 replay_Start 0, 13120 fight_Init 1) corroborates the 0/1 timing only; the reviewer reads the captures under
.claude/rules/naming-runtime-evidence.md. Verdicts set from the pre-apply review (getter PASS; setter FAIL, replaced by the reviewer's suggested name). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (fresh default-FAIL reviewer, lane naming-c 2026-10-31)"
NOTES = {"0x8003F218": "suggested: the pre-apply reviewer FAILed gpu_SetTrackedFighterViewFlagAndDisableFarViewOnClear (R4: DisableFarView over-claims; the callee(0) resets grid/fog control, is_far_view is untouched) and suggested gpu_SetTrackedFighterViewFlag; R6 must judge it fresh at every use"}
DROPPED = {"0x8003F218": "R6 FAIL (N1): gpu_SetTrackedFighterViewFlag (the pre-apply reviewer's suggestion) omits the clear-side gpu_SetFarViewGridFogControlAndCoordinateShift(0) reset of g_gpu_grid_and_fog_control / g_gpu_view_coordinate_shift; R6 suggests gpu_SetTrackedFighterViewFlagAndResetGridFogOnClear as a later func row"}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")

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


_SET = (
    f"BODY: {B('func_8003F218', '2B344.c')}: `if ((u32)a0 >= 2) return; if (a0 == D_800A322C) return; D_800A322C = a0; if (!a0) "
    "gpu_SetFarViewGridFogControlAndCoordinateShift(0); D_800F665C = (s16)D_800A322C;`. "
    "CALLERS (5 functions, 5 jal, no table): fight_Init src/main/9F9C.c:1900 passes g_fight_tracked_view_setting (beside FOV 0x50 / 0x2D "
    "on the same setting :1902, then camera_InitTwoFighterView :2052 and, if the setting != 0, camera_InitTrackedFighterFocus :2054); "
    "camera_ApplyFightView 9F9C.c:2101 passes (g_camera_fight_view_mode < 1) only inside the g_fight_tracked_view_setting != 0 block, mode 0 "
    "= g_camera_alternate_view_record (the record camera_UpdateTrackedFighterView :780-786 updates) with FOV 0x50, modes 1/2 = "
    "g_camera_default_view_record with FOV 0x2D; replay_Start src/main/28708.c:1313, fight_InitNearestStartPairPlayback "
    "src/main/2B344.c:1074 and motion_StartPlayback src/main/43500.c:812 pass constant 0, each followed by FOV 0x2D. So after a "
    "camera_ApplyFightView frame the flag is (tracked setting != 0 && tracked-fighter view mode 0), and 0 on every third-person path. "
    "READERS of the stored value: 2B344.c:1995 (grid mask table), :2071 (masked-grid gate), getter callers :1480 / :1545 (fog base), "
    "32D04.c:1152 (OT cutoff 0xBE). "
    "WHY: Set = the 0/1 store (other values rejected, equal value returns early); TrackedFighterViewFlag = the condition encoded by "
    "every caller's argument, in the landed g_fight_tracked_view_setting / camera_*TrackedFighter* vocabulary. The clear-side "
    "gpu_SetFarViewGridFogControlAndCoordinateShift(0) (resets g_gpu_grid_and_fog_control / g_gpu_view_coordinate_shift, not the "
    "camera's is_far_view) and the write-only D_800F665C mirror are left out of the name rather than mis-worded (pre-apply review: "
    "the drafted suffix AndDisableFarViewOnClear FAILed R4). "
    "COUNTER-EVIDENCE: (1) the D_800F665C mirror store is not in the name; it is write-only (no reader anywhere), so no caller can "
    "depend on it. (2) fight_Init passes the raw setting, which the debug config menu can set to >= 2; the call is then a no-op and the "
    "next camera_ApplyFightView frame rewrites the flag, so the flag never holds a 1 outside the tracked view. (3) camera modes 1 / 2 "
    "(the clear path) are code-only: the capture shows no 0-write during the fight. (4) 'OnClear' means on a change to 0, not on every "
    "call with 0. (5) The setter selects no camera; the name says Flag, not View. "
    "TRACE (corroboration only; only capture.jsonl sha1 and the D_800A322C hits re-read in this draft): watch "
    "docs/naming/runtime/pov-view-story-pov-rematch-1 D_800A322C writes: frame 8714 stage_card pc 0x8003F238 ra 0x8001DD24 (fight_Init) "
    "value 1; 12548 t_ko ra 0x800397F4 (replay_Start) 0; 13120 t_rematch fight_Init 1; (258 BIOS clear 0). Knockout "
    "docs/naming/runtime/story-pov-rematch-view-flag-stub-1/func_8003F218 (research: stub from 8360 gives 0 px at 8900, 7477 px at "
    "9200, missing backdrop strip / left floor-edge wedge; not re-read here)."
)

_GET = (
    f"BODY: {B('func_8003F268', '2B344.c')}: `return D_800A322C;`. "
    "CALLERS (4 jal, no table): gpu_LatchFogDepthOnTransitionAndQueueMoves src/main/2B344.c:1480 (zero -> fog base 0x6590 else 0x55F0), "
    "gpu_QueueDepthSpacedMovesAndLinkTerminalPacket 2B344.c:1545 (same pair), gpu_PrepareViewAndFlushDrawQueue src/main/32D04.c:1152 "
    "(non-zero -> OT cutoff 0xBE regardless of gpu_GetRaisedOtCutoffRequested), gpu_ResetAndStampViewCenteredCellAttributes 2B344.c:2343 "
    "(result discarded). Every caller tests it only for zero / non-zero. "
    "WHY: a pure getter of the flag func_8003F218 sets; same noun as the setter (TrackedFighterViewFlag), Get matching "
    "gpu_GetLastFogOtIndex / gpu_GetLastGridPassViewYaw / gpu_GetRaisedOtCutoffRequested; gpu_ because all callers are gpu_ functions. "
    "COUNTER-EVIDENCE: (1) the readers alone (fog base, OT cutoff) would not establish 'tracked fighter view'; the noun comes from the "
    "writers (see the setter row), which the getter returns unchanged. (2) the discarded call at :2343 supports nothing either way. "
    "TRACE (research, not re-read here): docs/naming/runtime/story-pov-rematch-view-flag-stub-1/func_8003F268 stub returning 0 from 8360 "
    "gives 0 px at 8900 / 9200; adds only that the getter is live in the fight; no reader-side word is claimed from it."
)

FUNCS = [
    ("0x8003F268", "gpu_GetTrackedFighterViewFlag", "RENAME", _GET),
]

DATA = []
