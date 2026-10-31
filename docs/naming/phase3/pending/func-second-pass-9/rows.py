"""wave100 rows (func-second-pass-9): Q132 mechanical names for 3 of the 5 functions revived in the
func-second-pass-9 batch (trace-unnamed cluster-2 capture 150597cb3: far-view-versus-2p-kimigaura-1 watch +
versus-2p-kimigaura-far-view-stub-1 knockout, which give D_800F6658 its role: 1 near view, 2 far view).
func_8003F1E4's name uses the D_800F6658 noun of data row var-globals-o (g_gpu_view_coordinate_shift) and
must land in the same wave or after it; if that data row FAILs this row goes back to NO_NAME.
func_8001B6F4 and func_8004C404 stand on their own evidence.
Research input: tmp/codex/func-second-pass-9-research.md (3 RENAME, 2 NO_NAME over 5 targets; untrusted
proposals). Every use of the three functions was re-derived at 02a603673 from the C bodies, the C call sites,
the asm jal sites (asm/funcs), the registries and the comments. No name changed; 0 dropped. One research
count corrected: func_8003F1E4 has 10 call sites (7 in 9F9C.c, 3 in 2B344.c; 10 asm jal), not 11.
func_80046B44 and func_8004A940 stay NO_NAME (research verdicts, unchanged by this draft).
(DATA is empty.)
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 3/3 RENAME + 2 NO_NAME confirmed (2026-10-30, naming-a); func_8003F1E4 conditional on var-globals-o g_gpu_view_coordinate_shift landing in the same wave or before"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8003F1E4": ("gpu_SetFarViewGridFogControlAndCoordinateShift", "gpu_SetFarViewGridFogControlAndCoordinateShift", ""),
    "0x8001B6F4": ("camera_ResetViewsForReplay", "camera_ResetViewsForReplay", ""),
    "0x8004C404": ("gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets", "gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets", ""),
}
DROPPED = {}

for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new:
        NOTES[_a] = f"adjusted from research {_old}: {_why}."

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

# ---------------------------------------------------------------------------------------------------------
_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")

_COV = {}
_covp = os.path.join(WT, "docs", "naming", "runtime", "matrix", "coverage.tsv")
_rd = list(csv.reader(open(_covp, encoding="utf-8"), delimiter="\t"))
_CAPS = _rd[0][5:]
for _r in _rd[1:]:
    _COV[_r[0].lower()] = [_CAPS[i] for i in range(len(_CAPS)) if _r[5 + i] not in (".", "")]


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
    for k, l in enumerate(lines):
        if l.startswith("INCLUDE_ASM") and fn in l:
            return f"src/main/{f}:{k + 1} (INCLUDE_ASM, no C body)"
    raise SystemExit(f"no C definition of {fn}")


def T(addr):
    """Reach clause from coverage.tsv (verified at build time); no role rests on it."""
    h = _COV[addr.lower()]
    return (f"TRACE: not used; coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation (Q132 mechanical name).")


def ch(addr):
    if addr not in ADJUSTED:
        return ""
    old, new, why = ADJUSTED[addr]
    if old == new:
        return ""
    return f"CHANGED FROM RESEARCH: {old}; {why}. "


def R(addr, new, fn, f, body, callers, why, counter, prior, trace=None):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {trace or T(addr)}")
    return (addr, new, "RENAME", ev)


FUNCS = [
    R("0x8003F1E4", "gpu_SetFarViewGridFogControlAndCoordinateShift", "func_8003F1E4", "2B344.c",
      "if (a0) { g_gpu_grid_and_fog_control = 3; D_800F6658 = 2; } else { g_gpu_grid_and_fog_control = 0; D_800F6658 = 1; }; "
      "void, no calls, no return. g_gpu_grid_and_fog_control (named in var-globals-d) bit 0 selects the far grid mask "
      "table (2B344.c:1997), the masked-grid pass (:2071) and the latched fog depth (:1495-1504, :1542); bit 1 is only "
      "cleared elsewhere (func_8003E2AC, :1793). D_800F6658 (data row var-globals-o -> g_gpu_view_coordinate_shift) is read "
      "only as a right-shift count on node-minus-camera-root coordinates and the rotation matrix before GTE projection, "
      "with 3 - it as the depth rescale (asm/funcs/func_8004A940.s:765-773, :836-855; asm/funcs/func_8004C404.s:15-19, "
      ":27-64, :116-124): 1 near, 2 far",
      "10 C calls (7 in 9F9C.c, 3 in 2B344.c), 10 asm jal. The only non-constant argument: 9F9C.c:574 "
      "func_8003F1E4(view_record->is_far_view) in camera_UpdateTwoFighterView, where is_far_view is a hysteresis on "
      "eye_distance (9F9C.c:569-573: stays set while eye_distance >= 0x558D, becomes set at >= 0x55F1; eye_distance clamped "
      "to <= 100000 at :566-568), so it is true exactly when the camera is far. All other calls pass 0: 9F9C.c:740 "
      "(camera_InitTwoFighterView), :766 (camera_InitTrackedFighterFocus), :790 (camera_UpdateTrackedFighterView), :908 "
      "(func_8001B6F4), :1046 (camera_SetFighterFocusView), :1058 (camera_SetInterpolatedFighterView); 2B344.c:947 "
      "(fight_UpdateRotatingViewThreeOptionChoice), :1037 (fight_UpdateRotatingViewDecimalPresentation), :2321 "
      "(func_8003F218 when its view index becomes 0). asm jal: camera_UpdateTwoFighterView.s:312, "
      "camera_InitTwoFighterView.s:23, camera_InitTrackedFighterFocus.s:23, camera_UpdateTrackedFighterView.s:8, "
      "camera_SetFighterFocusView.s:8, camera_SetInterpolatedFighterView.s:11, func_8001B6F4.s:17, "
      "fight_UpdateRotatingViewThreeOptionChoice.s:4, fight_UpdateRotatingViewDecimalPresentation.s:4, func_8003F218.s:13. "
      "Declaration include/bb2.h:574. Comment mentions: 9F9C.c:456, symbol_addrs.txt:144 and :1981 (notes). "
      "tools/rename_funcs.py:344 is a dead pre-rename map entry (game_SetControllerPorts, historical, not applied). "
      "No table or address-taken use. No caller reads a result",
      "Set = two unconditional stores selected by the argument; FarView = the argument's one non-constant source is the "
      "far-camera hysteresis and every 0 caller sets the near state; GridFogControl = the accepted name of the first "
      "written global; CoordinateShift = the D_800F6658 noun (var-globals-o). gpu_ = both written globals are gpu render "
      "state (g_gpu_*; readers in the grid / fog code of 2B344.c and the node draw path in 3AB48.c). The name claims no "
      "values (3, 2, 1)",
      "most callers are camera code (camera_*), so a camera_ prefix is arguable; gpu_ is chosen because the body touches "
      "only gpu render state. DEPENDS ON data row var-globals-o (D_800F6658 -> g_gpu_view_coordinate_shift): land in the "
      "same wave or after it; if that row fails, 'CoordinateShift' has no anchor and this row goes back to NO_NAME "
      "('SetFarViewGridFogControl' alone omits a write)",
      "no_name.tsv w50/w50b/w82/w92/w97 (every round failed on the writes because D_800F6658 had no role); revive "
      "evidence: capture 150597cb3 and the var-globals-o name",
      trace="TRACE (Q124, corroborating, not the basis): docs/naming/runtime/far-view-versus-2p-kimigaura-1 watch on "
      "D_800F6658 records writes at pc 0x8003F20C (this body) with ra 0x8001B2F0 (camera_InitTwoFighterView, value 1) and ra "
      "0x8001ACAC (camera_UpdateTwoFighterView: 1, then 2 once the scripted inputs separate the fighters; values {1,2}); "
      "docs/naming/runtime/versus-2p-kimigaura-far-view-stub-1 stubbing this function from frame 8230 changes 77-95% of "
      "pixels from frame 8540 on. Far-to-near was not observed. The name rests on the code."),
    R("0x8001B6F4", "camera_ResetViewsForReplay", "func_8001B6F4", "9F9C.c",
      "node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0) and (1, 0) (clears bit 0 of primitive_table_select on "
      "every bone record of both slot objects and pushes their colour, src/main/31D3C.c); g_camera_fight_view_mode = 1; "
      "g_camera_default_view_record.replay_view_started = 0 and g_camera_alternate_view_record.replay_view_started = 0 "
      "(include/game.h:1992; read and set only by camera_UpdateReplayView, 9F9C.c:931-932, so clearing it makes each "
      "view record's replay view start afresh); func_8003F1E4(0) (near-view grid / fog state and coordinate shift); void, "
      "no return. g_camera_fight_view_mode != 0 is the mode in which camera_ApplyFightView clears both slots' select bit "
      "(9F9C.c:2105-2112), the pair of node calls here",
      "one C call, src/main/28708.c:1318 in replay_Start (after func_8003F218(0), SetGeomScreen and the same two node "
      "calls at :1316-1317; then eff_StopCommands, g_replay_playback_frame = 0, handler 5); asm jal asm/funcs/replay_Start.s:20. "
      "Declaration include/bb2.h:698. No comment, symbol_addrs.txt or named_syms.txt entry names it beyond RESET notes. "
      "No table or address-taken use. No caller reads a result",
      "camera_ = the state touched is the camera's (g_camera_fight_view_mode, both camera view records, and the "
      "near-view setter every camera_Init* / Set* calls); Reset = every store is a constant start value; Views = both view "
      "records plus the view mode and the near-view state; ForReplay = its only caller is replay_Start and the members it "
      "clears are the replay view's",
      "the two node calls are node_ code, covered only through camera_ApplyFightView's mode-1 pairing (a reviewer may want "
      "them named, e.g. ...AndObjectNodeSelect); 'ForReplay' rests on a single caller (call-graph role, N3 class 3); the "
      "mode value 1 is not encoded. Independent of the D_800F6658 name (func_8003F1E4 is described by effect only, and "
      "this row's name does not mention it). The pilot-fight-9f9c-2 FAIL refused 'RestoreNodes' (nothing is saved) and "
      "'RenderConfig0'; this name uses neither",
      "no_name.tsv w51/w62/w84/w92/w97 (blocked on func_8003F1E4 and on replay_view_started, since named, game.h:1992)"),
    R("0x8004C404", "gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets", "func_8004C404", "3AB48.c",
      "(hand-written asm, 356 insns; inline_asm_canonical.txt:201) (1) setup, asm/funcs/func_8004C404.s:14-111: scratchpad "
      "0x1F8001C6 = 3 - D_800F6658, 0x1F8001C8 = D_80103624, 0x1F8001C0 = D_800A3790; the a2 matrix copied to 0x1F800188.. "
      "and a copy shifted by D_800F6658 to 0x1F80019C..; the camera root position to 0x1F8001B4..BC; the light setup into "
      "the GTE. (2) per node, .L8004C5BC (.s:112-): node x / z (offsets 8 / 0xC, y 0) minus the camera position, shifted "
      "by D_800F6658, rotated into the GTE translation; the node's vertex block projected by rtpt (rtps for a count of 4) "
      "into scratchpad SXY / SZ; then the packet stream after it dispatched: each u16 word >> 3 indexes D_80015470 "
      "(.L8004C8D8, .s:316-331; jalr) until a 0 word; a nonzero trailer tail-jumps to "
      "sys_StoreZeroPastScratchpadOnFailedCheck (.s:332-339). (3) .L8004C930 (.s:340-354): while the next draw-queue entry "
      "(before g_draw_queue_cursor) is another node of kind 12, advance and repeat; return the queue pointer of the last node drawn",
      "one call, asm/funcs/func_8004A940.s:493 jal from the node-kind 12 case .L8004B0C8 (jtbl_8001541C[12], "
      "src/main/d_5BF0.c:58-61) with a0 = the draw-queue cursor $s7, a1 = the node $s2, a2 = the matrix $s6; the return "
      "replaces $s7 (.s:495). INCLUDE_ASM at src/main/3AB48.c:25. No C caller, no table or address-taken use, no registry "
      "line; also cited in known_psyq_stdlib.txt:35 and inline_asm_canonical.txt:201, and the layer-2 ledger "
      "memory/grind/func_8004C404/",
      "Project = GTE rtpt / rtps of each node's vertex block; QueuedNodeRun = it consumes consecutive draw-queue entries of "
      "its own node kind and returns the advanced queue pointer, which tells it from the strided sibling "
      "gpu_ProjectStridedNodeVerticesAndDispatchPackets (3AB48.c:34, one node per call); Vertices / DispatchPackets = the "
      "sibling's accepted wording for the projection plus D_80015470 stream dispatch. gpu_ = draw-queue node rendering, GTE, "
      "OT handlers (the gpu_ callees and the caller chain gpu_DrawQueuedNodes -> func_8004A940)",
      "the fixed light setup (g_light_setups[4]) and the coordinate-shift / depth-rescale scratch setup are not in the "
      "name (they prepare the projection and the handlers); 'Run' is the loop's reading, a reviewer may prefer "
      "'Consecutive'; the node kind value is not encoded; the neutral identities of D_80103624 / D_800A3790 / slot 0 do not "
      "block a name for what the body does (N4). Independent of the D_800F6658 name (only the setup reads it; the name does "
      "not mention the shift)",
      "no_name.tsv w59/w82/w93/w96 (per-node GTE / light setup plus the D_80015470 stream decode); revive evidence: "
      "D_800F6658 now has its role, and the sibling's accepted wording",
      trace="TRACE: not used; the capture watches writes of D_800F6658 only and this reader is not observed. The name "
      "rests on the asm body."),
]

DATA = []
