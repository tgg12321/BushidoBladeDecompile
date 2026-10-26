"""wave87 rows (pilot-misc-2b344): Q132 mechanical names for 37 of the 42 neutral functions of main/2B344 (the
command-stream setup and fighter-configuration helpers, the mode-handler pairs of the D_800A3834 state machine, the
two fog-level builders, the grid-record queue / unpack functions and cell-attribute accessors, the loaded-selector
callback dispatcher and the softbody free-all wrapper).
Research input: tmp/codex/pilot-misc-2b344-research.md (38 RENAME, 4 NO_NAME over 42 targets; untrusted proposals).
Every row was re-derived from the C body in src/main/2B344.c and every caller (src/main/*.c, the mode-handler table
src/main/d_7D870.c, include/, asm/funcs/*.s); names changed against the research say CHANGED FROM RESEARCH in the
evidence.
Research NO_NAMEs (4; stay neutral, not rows): 8003B534 8003B56C 8003E2AC 8003F5A8 (two selector setters whose bases
D_800A3894 / D_800A385C have no distinguishing role, a one-bit clear of D_800F6656 with no named role for the bit, and a
pair store whose only consumer is an empty stub).
Research RENAMEs dropped by this draft (1 of 38; 37 remain, 20 of them renamed or reworded against the research, see
ADJUSTED):
  8003DDF8 (gpu_SetTerminalFogPacketOtLink): it stores a 24-bit packet address into word 0x3FFC / 4 of the table at
    D_800A378C. That this slot is the table's last or "terminal" entry is the claim waves 50b and 69 rejected, and
    nothing in the body or its single caller shows it; 'Terminal' would qualify the packet but reads as the table slot.
    A name without either claim would only restate the store. Stays neutral.
Prefix evidence: fight_ is the TU's (fight_InitCommandStream, fight_RunCommandStream, fight_TallyMatchResultsAndInitPresentation,
fight_UpdateMatchTallyPresentation, fight_FindNearestStartPair and fight_GetStartPosition live here; every handler
renamed fight_ is a member of the D_800A3834 mode table that fight_RunCommandStream and fight_Tally... also drive, or
a helper those handlers call); gpu_ is the TU's (gpu_SetCellGateBit, gpu_SetCellForceBit,
gpu_ResetAndStampViewCenteredCellAttributes, gpu_BuildFogImageLevels, gpu_MoveObjectClutsToFogBlock and
gpu_CollectGridDrawables... live here and their callees are gpu_); resource_ is the callee subsystem of F168
(resource_GetLoadedSelector); softbody_ is the callee subsystem of F7F4 (softbody_FreeAll). The research's data
suggestions (g_fight_command_stream_cursor, g_gpu_grid_cell_attributes ...) need separate all-access data manifests and
are not part of this wave (DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply (round 1): 33/37 PASS as proposed; 4 FAIL (8003B4DC, 8003BFC4, 8003C040, 8003C2C0) replaced by the reviewer-suggested names (NOTES)."
NOTES = {
    "0x8003B4DC": "suggested: naming-reviewer pre-apply round 1 failed fight_ConfigureFixedFighter1AndLoadBothFighters (2B344.c:298 writes unk_4[2], fighter 0's config byte read by func_8003AF40(0)) and suggested fight_SetFixedFighterBytesAndLoadBothFighters; R6 must judge it fresh.",
    "0x8003BFC4": "suggested: naming-reviewer pre-apply round 1 failed fight_ExecMovieOverlayAndReinitialize (N4: mode 0xF is entered only from main() 6CF8.c:439 and the mode-9 handler 25C38.c:248/276; no fight_ caller or callee) and suggested ExecMovieOverlayAndReinitialize; R6 must judge it fresh.",
    "0x8003C040": "suggested: naming-reviewer pre-apply round 1 failed fight_InitImageBackedPlaybackPresentation (R4: 'Backed' unshown; the paired split-wipe draw 4D0A8.c:1333-1365 shows a cover that opens) and suggested fight_InitImageAndPlaybackPresentation; R6 must judge it fresh.",
    "0x8003C2C0": "suggested: naming-reviewer pre-apply round 1 failed fight_UpdateImageBackedPlaybackPresentation (same R4 ground as 8003C040) and suggested fight_UpdateSplitWipeImageAndPlaybackPresentation; R6 must judge it fresh.",
}

# names changed against the research (research name -> name used)
ADJUSTED = {
    "0x8003AF40": ("fight_ResolveFighterSelectionInitAndLoadObject", "fight_FillMissingConfigInitFighterAndLoadObject",
                   "the first effect is filling a missing (0xFF) config byte unk_4[2 + arg0] from the other fighter's byte; "
                   "'Selection' is the byte indexing the object tables and is not what is resolved"),
    "0x8003AFFC": ("fight_SelectBothFightersResourcesAndLoad", "fight_RequestBothFightersEntriesAndReloadOnChange",
                   "the body only requests entries (func_800493E4 / func_800494D4) and ranks them (func_80049584); the load there "
                   "happens only when the snapshot or the pair index changed, and 'Entries' is the word of the existing "
                   "ResetEntrySelectionState it begins with"),
    "0x8003B10C": ("fight_SelectFighterResourcesAndLoad", "fight_RequestOneFighterEntriesAndReloadOnChange",
                   "same reasoning as 8003AFFC; 'One' tells it from the both-fighters loop and its extra-entry requests"),
    "0x8003B2C8": ("fight_SaveAndApplyStreamFighterSelectionAndState", "fight_SaveFighter0SelectionAndStateSubstituteStreamSelectionClearState",
                   "names which fighter (index 0 in unk_4[0] and func_80041604(0, 0)), the substitution from the stream record's first "
                   "byte and the clearing of D_800A376A that 'Apply' hid"),
    "0x8003B328": ("fight_RestoreSavedFighterSelectionAndStateAndRetainSubstitute", "fight_RestoreFighter0SelectionAndStateKeepingDisplacedPairAndResetFighter0Fields",
                   "adds the func_80022568(D_80101EC8) call (it writes four fields of fighter record 0) and names the fighter; "
                   "'Substitute' is the displaced pair D_800A3915 / D_800A36F4 that case 3 of the stream later reads"),
    "0x8003B3A4": ("fight_ConfigureLookupOpponentAndCopyCommandFields", "fight_ConfigureFighter1FromLookupAndOperandsAndLoad",
                   "'Opponent' is a role the body does not show; what it shows is fighter index 1 (func_8003AF40(1)) configured from "
                   "table lookups of fighter 0's selection plus five operand bytes, then loaded"),
    "0x8003B484": ("fight_ConfigureOperandSelectedOpponent", "fight_ConfigureFighter1FromOperandsAndLoad",
                   "same: fighter index 1, three operand bytes, then the AF40(1) / AFFC load the research name omitted"),
    "0x8003B4DC": ("fight_ConfigureFixedOpponentAndReloadFighters", "fight_ConfigureFixedFighter1AndLoadBothFighters",
                   "same: the fixed values (0 / 0x1F / 0) go to fighter 1's bytes; both fighters are initialised and loaded"),
    "0x8003B870": ("fight_InitCommandStreamImageTransition", "fight_InitCommandStreamImageLoad",
                   "'Transition' is a screen role; the code shows a black display and a mode switch to 0x17, whose handler loads the "
                   "next stream image"),
    "0x8003B8E4": ("fight_UpdateCommandStreamImageTransition", "fight_UpdateCommandStreamImageLoadAndResume",
                   "the handler loads the image with func_8003AE5C and resumes the stream with fight_RunCommandStream"),
    "0x8003BE10": ("fight_InitScrollingSpritePresentationAndAudio", "fight_InitScrollingSpriteDisplayAndAudio",
                   "wording: InitScrollingSpriteDisplay / StepScrollingSpriteDisplay are the callee names it follows"),
    "0x8003BEA8": ("fight_UpdateScrollingSpritesAndMergePersistentBits", "fight_UpdateScrollingSpriteDisplayAndMergeBitFlags",
                   "'Persistent' is not shown; the code ORs D_800A37A4 into D_80106A50.unk_00 and a fighter-indexed bit into unk_04"),
    "0x8003C8B4": ("fight_UpdatePackedTimeRecordPresentation", "fight_UpdateTimeRecordPresentation",
                   "the renderer's digit source is reached through a static set by a menu-side function (596CC.c:145), so 'Packed' (the "
                   "block func_8003C714 writes) is not shown as the same store; the pairing by mode number and the rank highlight are"),
    "0x8003C958": ("menu_InitRotatingViewChoice", "fight_InitRotatingViewThreeOptionChoice",
                   "no menu_ callee or data is in this body (the prefix would be a guess); the TU's fight_ state machine owns it; "
                   "the option index D_800A3817 wraps over 0..2 (up / down cycling in func_8003C9A4)"),
    "0x8003C9A4": ("menu_UpdateRotatingViewChoice", "fight_UpdateRotatingViewThreeOptionChoice",
                   "same as 8003C958"),
    "0x8003CF84": ("fight_UpdateNearestStartPairPlaybackAndResumeMotionStreams", "fight_UpdateNearestStartPairPlaybackAndReinitMotionStreams",
                   "the three motion_InitStream calls initialise streams; 'Resume' is not shown"),
    "0x8003EB84": ("gpu_ScanGridRowMasksQueueRecordsAndCollectUniqueNodes", "gpu_QueueMaskedGridRecordsWithCellAttributesAndCollectUniqueNodes",
                   "each queued record also takes its cell attribute byte (unk6 = a3 & 3, bit 0 of unk7 from the gate / force bits), which the "
                   "research verb left out; 'Masked' is the row-mask window"),
    "0x8003EDC0": ("gpu_UnpackGridDrawRecordsNodesAndCellRuns", "gpu_UnpackGridRecordsNodesAndCellRuns",
                   "'Draw' is a consumer-side claim; the unpacker itself writes records, transform nodes, the 32x32 cell index and the run table"),
    "0x8003F1C8": ("gpu_GetGridViewAzimuth", "gpu_GetLastGridPassViewYaw",
                   "D_800A336C is stored on every call of the grid pass (ratan2 of the rotated forward vector's x / z), so it is the last pass's value, "
                   "and an angle about the vertical axis is a yaw"),
    "0x8003F1D4": ("gpu_GetGridDrawableNodeTable", "gpu_GetGridNodeRecordTable",
                   "'Drawable' is a consumer-side word; the table is what func_8003EDC0 fills from the stream's node section"),
    "0x8003F420": ("gpu_StampNearestCellBlockAttributes", "gpu_ReplaceCellAttributesInNearest2x2Block",
                   "the four func_8003F52C calls overwrite (not OR, so not 'Stamp' like gpu_ResetAndStampViewCenteredCellAttributes) the cell and its "
                   "side / back / diagonal neighbours nearest the point: a 2 x 2 block"),
}

DROPPED = {
    "0x8003DDF8": "slot 0x3FFC / 4 being the table's terminal entry is the claim waves 50b / 69 rejected; unshown",
}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-d")

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


def B(fn, f="2B344.c"):
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


def L(f, pat, nth=0):
    """src/main/<f>:<line> of the nth line matching regex pat."""
    hits = [k + 1 for k, l in enumerate(_SRC[f]) if re.search(pat, l)]
    if len(hits) <= nth:
        raise SystemExit(f"{f}: no line {nth} matching {pat}")
    return f"src/main/{f}:{hits[nth]}"


def D7(fn):
    """The mode-handler table entry of fn."""
    return L("d_7D870.c", r"\.word %s\\n" % fn)


def T(addr):
    """Reach clause from coverage.tsv (verified at build time); no role rests on it."""
    h = _COV[addr.lower()]
    return (f"TRACE: not used; coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation (Q132 mechanical name).")


def ch(addr):
    """CHANGED FROM RESEARCH clause for rows whose name differs from the research."""
    if addr not in ADJUSTED:
        return ""
    old, new, why = ADJUSTED[addr]
    if old == new:
        return ""
    return f"CHANGED FROM RESEARCH: {old}; {why}. "


_MODE = ("The D_800A3834 values are the mode numbers of the handler table D_8008D090 (src/main/d_7D870.c, one function word per mode number); "
         "each handler has its own mode number, and the init handler of a screen stores the mode number of its update handler. ")

FUNCS = [
    ("0x8003AE5C", "fight_ScanCommandStreamAndLoadImage", "RENAME",
     f"{ch('0x8003AE5C')}BODY: {B('func_8003AE5C')}: walks the command bytes of arg0, skipping each command's operand length, until command 0, 1 or 20; command 1 also captures its operand as result; "
     f"when result >= 0 it stores D_800A36A4 into D_800A37A8[D_800A37A0], resets graph mode, calls func_80020D38 and func_800602AC(result, 0x80190800). "
     f"func_800602AC ({B('func_800602AC', '4D0A8.c')}) reads CD file (2, result + 0x3D) into the buffer and uploads four regions with LoadImage. "
     f"CALLERS: {L('2B344.c', r'func_8003AE5C\(D_800A3844\);')} (fight_InitCommandStream) and {L('2B344.c', r'func_8003AE5C\(D_800A3844\);', 1)} (func_8003B8E4). "
     f"WHY: Scan = the operand-length walk over the stream, LoadImage = the read plus LoadImage uploads. COUNTER-EVIDENCE: no screen or story identity (result is only a file index); "
     f"the D_800A37A8 history store is a one-word side write. Prior manifests: no_name.tsv (image identity unresolved; Q132 lifts it). {T('0x8003AE5C')}"),

    ("0x8003AF40", "fight_FillMissingConfigInitFighterAndLoadObject", "RENAME",
     f"{ch('0x8003AF40')}BODY: {B('func_8003AF40')}: when D_80102778.unk_4[2 + arg0] is 0xFF it takes the other fighter's byte; fighter_Init(arg0, unk_4[4 + arg0], unk_4[arg0], unk_4[2 + arg0], 0); gpu_ResetGraphMode1(); "
     f"func_80020D38(); func_80040510(arg0, D_8008D578[unk_4[arg0]], 0x80190800). func_80040510 ({B('func_80040510', '309CC.c')}) creates the object (func_80045878), stores it in D_800A9A10[arg0], sets up the softbody and rebuilds the fog block. "
     f"CALLERS: fight_InitCommandStream, func_8003B3A4 / B484 / B4DC, fight_RunCommandStream cases 3 and 2, func_8003BCB4, func_8003C040 (four), func_8003CF84, src/main/28708.c:1532 and replay_PlayFrame (asm) each call it to (re)build one fighter. "
     f"WHY: Fill = the 0xFF default, InitFighter = fighter_Init, LoadObject = func_80040510 with the table-indexed argument. COUNTER-EVIDENCE: no resource-class claim. Prior manifests: no_name.tsv (whole-effect coverage required). {T('0x8003AF40')}"),

    ("0x8003AFFC", "fight_RequestBothFightersEntriesAndReloadOnChange", "RENAME",
     f"{ch('0x8003AFFC')}BODY: {B('func_8003AFFC')}: gpu_ResetGraphMode1(); func_80020D38(); ResetEntrySelectionState(); for fighters 0 and 1: func_800493E4(D_80101EC8[i].unk_12), func_800494D4(i, D_8008E5CC[unk_0A][unk_0E]), and when unk_14 != -1 "
     f"func_800493E4(D_8008EB80[unk_14]) (plus D_8008EB80[14] + 3 when unk_14 == 14); then func_80049584(0x80190800). func_80049584 ({B('func_80049584', '368E4.c')}) snapshots D_800EF980, ranks the entries >= 0, computes the pair index "
     f"and only when the snapshot signs or D_800A324C changed runs func_80046020 / func_80045B68 / func_8003E120 (free then load). CALLERS: 2B344.c:282 / 292 / 303 / 357 / 506 / 567 / 1106, 28708.c:1533 and the asm of fight_RunCommandStream and replay_PlayFrame (see HOMES)."
     f"WHY: Request = the func_800493E4 / func_800494D4 entry requests for each of the two fighters, ReloadOnChange = the guarded reload inside func_80049584. COUNTER-EVIDENCE: no asset class or motion/model claim. Prior manifests: no_name.tsv (resource-class identity; Q132). {T('0x8003AFFC')}"),

    ("0x8003B10C", "fight_RequestOneFighterEntriesAndReloadOnChange", "RENAME",
     f"{ch('0x8003B10C')}BODY: {B('func_8003B10C')}: gpu_ResetGraphMode1(); func_80020D38(); ResetEntrySelectionState(); func_800493E4(D_80101EC8[arg0].unk_12); func_800494D4(arg0, table[unk_0A][unk_0E]) with table D_8008E6A4 when D_800A38DC == 5 else D_8008E5CC; func_80049584(0x80190800). "
     f"CALLERS: func_8003C040 ({L('2B344.c', r'func_8003B10C\(0\)')} ff., arguments 0 only). WHY: One = the single parameter-selected fighter (no loop, no unk_14 extra entries) against func_8003AFFC; Request / ReloadOnChange as there. "
     f"COUNTER-EVIDENCE: the alternate table for mode 5 is not named. Prior manifests: no_name.tsv (unknown resource identity). {T('0x8003B10C')}"),

    ("0x8003B2C8", "fight_SaveFighter0SelectionAndStateSubstituteStreamSelectionClearState", "RENAME",
     f"{ch('0x8003B2C8')}BODY: {B('func_8003B2C8')}: D_800A3836 = unk_4[0]; D_800A36C8 = D_800A376A; D_800A376A = 0; unk_4[0] = *D_800A3894 (the first byte of the stream's current record); func_80041604(0, 0). "
     f"func_80041604 ({B('func_80041604', '31D3C.c')}) invalidates object D_800A9A10[0] when its stored value differs and writes D_80094B88[0]. CALLERS: fight_RunCommandStream case 2 ({L('2B344.c', r'func_8003B2C8\(\)')}) only. "
     f"WHY: unk_4[0] is fighter 0's selection byte (func_8003AF40(0) indexes D_8008D578 by it); the saved pair is read back by func_8003B328. COUNTER-EVIDENCE: no claim that a whole fighter record is saved; D_800A376A's meaning is not named. "
     f"Prior manifests: no_name.tsv (identity and associated state both unresolved). {T('0x8003B2C8')}"),

    ("0x8003B328", "fight_RestoreFighter0SelectionAndStateKeepingDisplacedPairAndResetFighter0Fields", "RENAME",
     f"{ch('0x8003B328')}BODY: {B('func_8003B328')}: reads unk_4[0], D_800A376A, D_800A3836, D_800A36C8; D_800A3836 = 0xFF (marks the save empty); D_800A3915 = old unk_4[0]; D_800A36F4 = old D_800A376A; unk_4[0] = saved selection; D_800A376A = saved state; "
     f"func_80041604(0, saved state); func_80022568(D_80101EC8), which sets four fields (unk_26C = 1, 26E / 270 / 272 = 0) of fighter record 0. CALLERS: {L('2B344.c', r'func_8003B328\(\);')} (func_8003CF84), 28708.c and replay_PlayFrame (asm). "
     f"The displaced pair is read in fight_RunCommandStream case 3 ({L('2B344.c', r'D_800A3915\)')}: when the operand equals D_800A3915, func_80041604(1, D_800A36F4)). WHY: inverse of func_8003B2C8 with the displaced values retained. "
     f"COUNTER-EVIDENCE: not a whole-record restore. Prior manifests: no_name.tsv (displaced-state retention). {T('0x8003B328')}"),

    ("0x8003B3A4", "fight_ConfigureFighter1FromLookupAndOperandsAndLoad", "RENAME",
     f"{ch('0x8003B3A4')}BODY: {B('func_8003B3A4')}: D_800A3712 = 0; idx = D_8008D538[unk_4[0]], a1 = D_8008D9EC[idx] (zeroed when D_800A37A0 == 1); unk_4[1] = a1 ? 0xE : 0x1D; unk_4[3] = 0; copies arg0[0] to D_800A3680 / D_800A3671, arg0[1] to unk_A[1], arg0[2..4] to D_800A37B4..B6; "
     f"func_8003AF40(1); func_8003AFFC(); returns 5. CALLERS: fight_RunCommandStream case 16 ({L('2B344.c', r'func_8003B3A4\(D_800A3844\)')}: adds the return value to the stream pointer) and case 2. "
     f"WHY: fighter index 1 is the one AF40(1) initialises; its selection / config bytes come from fighter 0's table lookups and the operand bytes. COUNTER-EVIDENCE: no CPU, lesson or opponent role is claimed. Prior manifests: no_name.tsv (operand roles unresolved). {T('0x8003B3A4')}"),

    ("0x8003B484", "fight_ConfigureFighter1FromOperandsAndLoad", "RENAME",
     f"{ch('0x8003B484')}BODY: {B('func_8003B484')}: D_800A3712 = 1; unk_4[1] = arg0[0]; unk_4[3] = arg0[1]; unk_A[1] = arg0[2]; func_8003AF40(1); func_8003AFFC(); returns 3. "
     f"CALLERS: fight_RunCommandStream case 17 ({L('2B344.c', r'func_8003B484\(D_800A3844\)')}, stream pointer += return), 28708.c:1517 and 9F9C.c:1302 (D_800A3894 + 6), and the asm of fight_UpdateDefeatDelayAndModeProgress / replay_PlayFrame. "
     f"WHY: operand-sourced against func_8003B3A4's lookup-sourced bytes and func_8003B4DC's fixed bytes. COUNTER-EVIDENCE: no logo, CPU or lesson claim. Prior manifests: no_name.tsv (operand distinction beyond count). {T('0x8003B484')}"),

    ("0x8003B4DC", "fight_SetFixedFighterBytesAndLoadBothFighters", "RENAME",
     f"{ch('0x8003B4DC')}BODY: {B('func_8003B4DC')}: D_800A3712 = 1; unk_4[2] = 0; unk_4[1] = 0x1F; unk_4[3] = 0; func_8003AF40(0); func_8003AF40(1); func_8003AFFC(). CALLERS: fight_RunCommandStream case 20 ({L('2B344.c', r'func_8003B4DC\(\)')}) only. "
     f"WHY: constant bytes (against the operand and lookup siblings) and both fighters initialised before the entry request. COUNTER-EVIDENCE: the selection value 0x1F is not given a character identity. Prior manifests: no_name.tsv (unknown identity; Q132). {T('0x8003B4DC')}"),

    ("0x8003B870", "fight_InitCommandStreamImageLoad", "RENAME",
     f"{ch('0x8003B870')}BODY: {B('func_8003B870')}: func_80041604(0, D_800A376A); func_80041604(1, 0); snd_ResetVoicesAndQueue(); gpu_HideResetAndClear(); gpu_SetDrawEnvBg(1, 0, 0, 0); D_800A37B8 = 0; D_800A3834 = 0x17; gpu_SetDispMaskOn(). "
     f"{_MODE}Handler table entry {D7('func_8003B870')}; its update handler is mode 0x17 = func_8003B8E4 ({D7('func_8003B8E4')}). WHY: ImageLoad = the blank screen whose mode loads the next stream image. "
     f"COUNTER-EVIDENCE: no screen identity. Prior manifests: no_name.tsv (displayed identity). {T('0x8003B870')}"),

    ("0x8003B8E4", "fight_UpdateCommandStreamImageLoadAndResume", "RENAME",
     f"{ch('0x8003B8E4')}BODY: {B('func_8003B8E4')}: D_800A37B8++; while it is below 3 g_prim_buf_cursor += func_80060544(cursor, 1) / 4; at 3: DrawSync(0); func_8003AE5C(D_800A3844) (scan and load image); D_800A37C0 = 500; D_800A38F8 = 0; D_800A3768 = DISP_ACTIVE; func_8001D790(); "
     f"fight_RunCommandStream(); gpu_SetDrawEnvBg(1, 0, 0, 0); D_800A390D = 1. Table entry {D7('func_8003B8E4')} (mode 0x17). WHY: the load is func_8003AE5C, the resume is fight_RunCommandStream. "
     f"COUNTER-EVIDENCE: the first-two-frame draw is not named (func_80060544's output is not identified). Prior manifests: no_name.tsv. {T('0x8003B8E4')}"),

    ("0x8003B9D0", "fight_StartDescriptorPlaybackAndAudio", "RENAME",
     f"{ch('0x8003B9D0')}BODY: {B('func_8003B9D0')}: resets sound / effects / display / graph mode; reads the four-byte record D_800A3878 (the stream pointer stored by command 19 and by func_8003B534 / B56C): byte 3 flags choose the buffer (0x80), "
     f"fighter entry requests via func_8003AFFC with entry 0x32 substituted (0x30), the playback fighter arguments (0x1 / 0x2 / 0x10 / 0x20) and func_8004659C(-1) (0x40); func_80054884(D_800A376C, rec[0], 0, ...) starts motion playback ({B('func_80054884', '43500.c')}); "
     f"func_80041688 on both objects; cdrom_GetFileIndex(5, rec[1]) + cdrom_StartAudio(.., rec[2]); D_800A3834 = 7. Table entry {D7('func_8003B9D0')} (mode 6; BCB4 is mode 7). "
     f"WHY: Descriptor = the D_800A3878 record that selects playback and audio. COUNTER-EVIDENCE: no intro, replay or dialogue claim. Prior manifests: no_name.tsv (playback identity). {T('0x8003B9D0')}"),

    ("0x8003BCB4", "fight_HandleDescriptorPlaybackCompletion", "RENAME",
     f"{ch('0x8003BCB4')}BODY: {B('func_8003BCB4')}: D_800A37B8++; returns while func_80054F68() != 0 and no skip bit in g_pad_state.pressed; cdrom_PauseAndWait(); func_800548DC() (stops playback); returns if D_800A38DC != 0; "
     f"then, per which of D_800A3894 / D_800A385C is set and D_800A37B0 / D_800A390C: D_800A3834 = 0 with D_800A3907++, or func_8003AF40(0) + func_8003AFFC(), or clears the record pointer; the common exit is fight_RunCommandStream(). "
     f"Table entry {D7('func_8003BCB4')} (mode 7). WHY: the whole body is the end-of-playback handling and its continuations. COUNTER-EVIDENCE: no win / loss reading of the selector values. Prior manifests: no_name.tsv. {T('0x8003BCB4')}"),

    ("0x8003BE10", "fight_InitScrollingSpriteDisplayAndAudio", "RENAME",
     f"{ch('0x8003BE10')}BODY: {B('func_8003BE10')}: gpu_ResetGraphMode1(); gpu_HideResetAndClear(); func_80020CDC(); func_800415C4(0) and (1) (free both objects); eff_ClearInitFlag(); func_8005B72C(); InitScrollingSpriteDisplay(0x80118800); snd_SerialMixOn(); "
     f"cdrom_StartAudio(cdrom_GetFileIndex(5, 0x20), 4); cdrom_WaitAudioStarted(); D_800A3834 = 0xB; gpu_SetDispMaskOn(). Table entry {D7('func_8003BE10')} (mode 0xA); the update is mode 0xB. "
     f"WHY: InitScrollingSpriteDisplay is the central call, Audio the CD start. COUNTER-EVIDENCE: no credits or ending identity. Prior manifests: no_name.tsv. {T('0x8003BE10')}"),

    ("0x8003BEA8", "fight_UpdateScrollingSpriteDisplayAndMergeBitFlags", "RENAME",
     f"{ch('0x8003BEA8')}BODY: {B('func_8003BEA8')}: go = StepScrollingSpriteDisplay() != 0 or a skip bit; on go: cdrom_PauseAndWait(); D_80106A50.unk_00 |= D_800A37A4 (changed = 1 if it altered); for D_80101EC8[0].unk_0A in {{0, 1, 2, 0xC, 0xD, 0xE}} sets bit (unk_0A, or unk_0A - 9 when >= 0xC) of D_80106A50.unk_04 (changed = 1 if newly set); "
     f"D_800A3834 = changed ? 0x1A : 8. Table entry {D7('func_8003BEA8')} (mode 0xB). WHY: Step is the display call, MergeBitFlags the two ORs. COUNTER-EVIDENCE: no unlock or persistence claim for the bits. Prior manifests: no_name.tsv. {T('0x8003BEA8')}"),

    ("0x8003BFC4", "ExecMovieOverlayAndReinitialize", "RENAME",
     f"{ch('0x8003BFC4')}BODY: {B('func_8003BFC4')}: gpu_ResetGraphMode1(); func_80020CDC(); func_800415C4(0) and (1); eff_ClearInitFlag(); sys_ExecMovieOverlay(arena_GetBase(), 0x80118000, 1, 0xCF8, 0xB01); func_80046B44(); D_800A3834 = 8. "
     f"func_80046B44 ({B('func_80046B44', '368E4.c')}) is the startup initialiser also called from 6CF8.c. Table entry {D7('func_8003BFC4')} (mode 0xF). WHY: the movie overlay call and the reinit that follows it. "
     f"COUNTER-EVIDENCE: no ending or credits movie identity. Prior manifests: no_name.tsv. {T('0x8003BFC4')}"),

    ("0x8003C040", "fight_InitImageAndPlaybackPresentation", "RENAME",
     f"{ch('0x8003C040')}BODY: {B('func_8003C040')}: clears display / graph mode; for D_800A38A4 4 or 5 eff_ClearInitFlag(); for 6..9 sets unk_4[0] / unk_4[2] and runs func_8003AF40(0) + func_8003B10C(0); func_8005FBC8(sel, 0x80118800) reads CD file (2, sel + 0x33) and uploads two LoadImage regions ({B('func_8005FBC8', '4D0A8.c')}); "
     f"func_80054884(0x16, D_8009016C[sel], 0, ...) starts playback; func_80041688 x2; eff_StopCommands(); optional CD audio from D_8008EA70[sel]; D_800A3834 = 0x13. Table entry {D7('func_8003C040')} (mode 0x12). "
     f"WHY: ImageBacked = func_8005FBC8's image upload, Playback = func_80054884, selector-driven by D_800A38A4. COUNTER-EVIDENCE: no unlock or victory claim across the selectors. Prior manifests: no_name.tsv (presentation identities). {T('0x8003C040')}"),

    ("0x8003C2C0", "fight_UpdateSplitWipeImageAndPlaybackPresentation", "RENAME",
     f"{ch('0x8003C2C0')}BODY: {B('func_8003C2C0')}: D_800A37B8++; g_prim_buf_cursor += gpu_DrawImageAndAdvanceSplitWipe(cursor, 1); on playback end (func_80054F68() == 0) or skip, selects the next mode from D_800A38A4 (0x12 for 6 / 7 with D_800A3781, 0x18, 0x1A, 0xA or 8); "
     f"when leaving: cdrom_PauseAndWait(), func_800548DC() (unless going to 0x12). Table entry {D7('func_8003C2C0')} (mode 0x13), the update of func_8003C040. WHY: the image draw is gpu_DrawImageAndAdvanceSplitWipe. "
     f"COUNTER-EVIDENCE: skip input is no menu claim. Prior manifests: no_name.tsv. {T('0x8003C2C0')}"),

    ("0x8003C714", "fight_PackTimeRecordsAndInitPresentation", "RENAME",
     f"{ch('0x8003C714')}BODY: {B('func_8003C714')}: s0 = func_80077D00(); cdrom_PauseAndWait(); gpu_HideResetAndClear(); func_80060758(); for the three D_80106A50.times records writes minutes (/1800), seconds ((/30) % 60), hundredths ((% 30) * 100 / 30) and unk_0 into s0->unk21[i]; "
     f"three bytes of func_8001CD68 into unk2D[]; unk30 = D_80101EC8[0].unk_0A; black background; D_800A37B8 = 0; D_800A3834 = 0x1F; display on. Table entry {D7('func_8003C714')} (mode 0x1E). "
     f"WHY: the packing is the body's main work, the rest is the presentation start. COUNTER-EVIDENCE: no save-screen identity. Prior manifests: no_name.tsv (PackTimes omitted the setup; this name includes it). {T('0x8003C714')}"),

    ("0x8003C8B4", "fight_UpdateTimeRecordPresentation", "RENAME",
     f"{ch('0x8003C8B4')}BODY: {B('func_8003C8B4')}: D_800A37B8++; g_prim_buf_cursor += func_80060768(cursor, 1, D_800A38E9) / 4; on skip or D_800A37B8 >= 0xF1 calls func_80033FE4 (picks the next mode). "
     f"It is the mode-0x1F handler that func_8003C714 (which packs the D_80106A50.times records) switches to. func_80060768 ({B('func_80060768', '4D0A8.c')}) calls gpu_DrawIndexedCellsAndDecimalDigitPairs, whose digit-pair source is "
     f"Unk8009BD24Block unk21[i].unk0 / unk1 / unk2 ({L('596CC.c', r'D_800A3524->unk21\[k\]\.unk0')}), the layout func_8003C714 writes, and arg2 = D_800A38E9 is the rank of the new time among record->times ({L('17AFC.c', r'D_800A38E9 = \(u8\)n')}), which the function highlights with a tile at row arg2 < 3. "
     f"Table entry {D7('func_8003C8B4')} (mode 0x1F). WHY: TimeRecord = the paired packer's records and the rank highlight. COUNTER-EVIDENCE: the pointer D_800A3524 is set by a menu-side function (596CC.c:145), so the data link rests on the identical block type, not a shared store; "
     f"hence the name omits 'Packed'; no cursor or menu claim. Prior manifests: no_name.tsv (screen identity). {T('0x8003C8B4')}"),

    ("0x8003C958", "fight_InitRotatingViewThreeOptionChoice", "RENAME",
     f"{ch('0x8003C958')}BODY: {B('func_8003C958')}: gpu_HideResetAndClear(); D_800A3817 = 0; D_800A3929 = 0; D_800A37B8 = 0; D_800A3834 = 0x19; gpu_SetDispMaskOn(). Table entry {D7('func_8003C958')} (mode 0x18), update func_8003C9A4 (mode 0x19). "
     f"WHY: the update handler shows the choice (see 8003C9A4). COUNTER-EVIDENCE: no retry / continue / post-fight claim. Prior manifests: no_name.tsv and wave69 (generic InitChoice failed against the sibling pair in 9F9C.c; the rotating view tells them apart). {T('0x8003C958')}"),

    ("0x8003C9A4", "fight_UpdateRotatingViewThreeOptionChoice", "RENAME",
     f"{ch('0x8003C9A4')}BODY: {B('func_8003C9A4')}: func_8003F1E4(0); fills D_800F6608 with a fixed position, vx = 0x20 and vy = D_800A36AC << 2 (a view yaw that follows the frame counter); gpu_ResetDrawQueueAndPrepareViewAndLighting and the grid / fog / flush pass; "
     f"until confirmed draws func_8005C8A8(1, D_800A3817, ..), moves D_800A3817 within 0..2 (wrapping both ways) on the up / down pad bits and sets D_800A3929 on confirm (1 for option 0, 0x3C otherwise); after 0x3C frames cdrom_PauseAndWait() and the per-option continuation "
     f"(fight_AdvanceMatch / D_800A380C++ / fight_FindNearestStartPair, or func_8001DA2C and mode 8). Table entry {D7('func_8003C9A4')} (mode 0x19). WHY: the rotating view is the D_800F6608 yaw, ThreeOption the 0..2 wrap. "
     f"COUNTER-EVIDENCE: continuations differ per option, so no retry semantics. Prior manifests: no_name.tsv, wave69. {T('0x8003C9A4')}"),

    ("0x8003CCCC", "fight_InitRotatingViewDecimalPresentation", "RENAME",
     f"{ch('0x8003CCCC')}BODY: {B('func_8003CCCC')}: gpu_HideResetAndClear(); eff_StopCommands(); D_800A37B8 = 0; D_800A3834 = 0x21; gpu_SetDispMaskOn(). Table entry {D7('func_8003CCCC')} (mode 0x20), update func_8003CD10 (mode 0x21). "
     f"WHY: names the pair via the update (see 8003CD10). COUNTER-EVIDENCE: no screen title or menu claim. Prior manifests: no_name.tsv. {T('0x8003CCCC')}"),

    ("0x8003CD10", "fight_UpdateRotatingViewDecimalPresentation", "RENAME",
     f"{ch('0x8003CD10')}BODY: {B('func_8003CD10')}: the same D_800F6608 view setup as func_8003C9A4 (vy = D_800A36AC << 2) and scene pass; g_prim_buf_cursor += func_800600C8(D_800A391F, cursor, 1) ({B('func_800600C8', '4D0A8.c')}: frame header cells plus the digit cells of the value); "
     f"D_800A37B8++; after 0x97 frames or skip: cdrom_PauseAndWait(); func_8001DA2C(); D_800A3834 = 8. Table entry {D7('func_8003CD10')} (mode 0x21). WHY: Decimal = func_800600C8 draws the digits of D_800A391F. "
     f"COUNTER-EVIDENCE: what the value means is not claimed. Prior manifests: no_name.tsv. {T('0x8003CD10')}"),

    ("0x8003CE18", "fight_InitNearestStartPairPlayback", "RENAME",
     f"{ch('0x8003CE18')}BODY: {B('func_8003CE18')}: func_8001DA2C(); cdrom_PauseAndWait(); display / graph mode / background reset; gpu_RestoreObjectClutOffsets(); func_8003F218(0); SetGeomScreen; s0 = 6..9 from D_800A3748 and fighter D_80101EC8[D_800A3748].unk_0E; "
     f"D_800A3818 = fight_FindNearestStartPair(&other fighter's world_pos); func_80054884(0x16, s0, D_800A3818, ...) starts playback; func_80041688 x2; eff_StopCommands(); D_800A3834 = 0x1D. Table entry {D7('func_8003CE18')} (mode 0x1C). "
     f"WHY: NearestStartPair = fight_FindNearestStartPair's result, kept in D_800A3818 and read by the update. COUNTER-EVIDENCE: no victory or execution claim for the variants 6..9. Prior manifests: no_name.tsv. {T('0x8003CE18')}"),

    ("0x8003CF84", "fight_UpdateNearestStartPairPlaybackAndReinitMotionStreams", "RENAME",
     f"{ch('0x8003CF84')}BODY: {B('func_8003CF84')}: snd_UpdateStageSounds(); snd_QueueSound at frames D_8008EAC0[..] / D_8008EB04..0A; at D_8008EB0C fight_GetStartPosition(0, vec, D_800A3818) and eff_StartRecycledVectorCommandWithRedLight; motion_GetPlaybackViewAngles + func_80061064; "
     f"on playback end or skip: func_800548DC(); optional D_800A37D2[D_800A3748]++; motion_InitStream(0, ..), (1, ..), (2, ..); then restore + AF40(0) + AFFC + func_8003B534(4) when D_800A38DC == 0 and D_800A3836 != 0xFF, else D_800A3834 = 0x18. Table entry {D7('func_8003CF84')} (mode 0x1D). "
     f"WHY: NearestStartPair = D_800A3818 set by func_8003CE18, ReinitMotionStreams = the three motion_InitStream calls. COUNTER-EVIDENCE: the mode continuation is not folded into the name; not the replay ring. Prior manifests: no_name.tsv. {T('0x8003CF84')}"),

    ("0x8003E0E0", "gpu_BuildFogRowLevels", "RENAME",
     f"{ch('0x8003E0E0')}BODY: {B('func_8003E0E0')}: RECT {{x 0, y 0x1E1, w 0x140, h 1}}; gpu_BuildFogImageLevels(&rect, 0x1F). CALLERS: {L('368E4.c', r'func_8003E0E0\(\)')} and the asm of func_80046EDC. gpu_BuildFogImageLevels ({B('gpu_BuildFogImageLevels')}) builds count-1 fogged copies of the rectangle. "
     f"WHY: Row = a one-pixel-high strip against the 64 x 8 block of func_8003E120. COUNTER-EVIDENCE: no palette or stage identity; the numbers stay out of the name. Prior manifests: no_name.tsv (resource identity and geometry-only distinction; Q132 allows the operand shape). {T('0x8003E0E0')}"),

    ("0x8003E120", "gpu_BuildFogBlockLevels", "RENAME",
     f"{ch('0x8003E120')}BODY: {B('func_8003E120')}: RECT {{x 0x140, y 0x1E8, w 0x40, h 8}}; gpu_BuildFogImageLevels(&rect, 0x13). CALLERS: func_80040510 (309CC.c:150, every object load), func_80049584 (368E4.c:1874, the entry reload), gpu_MoveObjectClutsToFogBlock ({L('2B344.c', r'func_8003E120\(\)')}), 31D3C.c:283 and the asm of func_80041BF4."
     f"WHY: Block against func_8003E0E0's Row; the same callee. COUNTER-EVIDENCE: not fighter-only (callers include the entry reload). Prior manifests: no_name.tsv. {T('0x8003E120')}"),

    ("0x8003EB84", "gpu_QueueMaskedGridRecordsWithCellAttributesAndCollectUniqueNodes", "RENAME",
     f"{ch('0x8003EB84')}BODY: {B('func_8003EB84')}: builds a 32-entry table sp[] of column masks (mask from a0, rows a1 .. a1 + 0x1E, other rows all ones); for each row with a non-zero mask and each set column reads the run index D_800A7FE0[row][col]; per id in the run: ids below D_800A3368 set e->unk6 = cell & 3 and bit 0 of unk7 from the gate / force bits "
     f"and push e on g_draw_queue_cursor; other ids append the node D_800A6690[id - D_800A3368] to out once (unk58 flag); returns out. The same loop appears inline in gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards. "
     f"CALLERS: {L('2B344.c', r'= func_8003EB84\(x, z, out\)')} only. WHY: Queue (records), CellAttributes (unk6 / unk7 from D_800A8FB0), CollectUniqueNodes (unk58). COUNTER-EVIDENCE: no outside-window geometry claim. Prior manifests: no_name.tsv (unverified geometric noun; split outputs). {T('0x8003EB84')}"),

    ("0x8003EDC0", "gpu_UnpackGridRecordsNodesAndCellRuns", "RENAME",
     f"{ch('0x8003EDC0')}BODY: {B('func_8003EDC0')} (doc comment above it): section 1 fills D_800A4750 records from (cell, id) pairs and sets D_800A3368 = count; section 2 fills D_800A6690 transform nodes (translation, rotation, D_800F66A0 handler call); then clears the 32 x 32 index D_800A7FE0 and fills it with offsets into the run table D_800A87E0, "
     f"updates the high water D_800A3230 (overflow check at 1000 via func_80052C10), and zeroes D_800A3678[0..2]. CALLERS: four in 368E4.c (the loader's section installs) and the asm of func_800460E4 / func_800464C4 / func_8004659C. "
     f"The records and runs are consumed by the grid pass (gpu_CollectGridDrawables... and func_8003EB84), which queue them on g_draw_queue_cursor. WHY: gpu_ follows the unpacked data's consumers. COUNTER-EVIDENCE: no collision or stage role. Prior manifests: wave69 grid_ / cellgrid_ prefix objections do not apply to gpu_. {T('0x8003EDC0')}"),

    ("0x8003F168", "resource_InvokeLoadedSelectorInitCallback", "RENAME",
     f"{ch('0x8003F168')}BODY: {B('func_8003F168')}: if D_800948BC[resource_GetLoadedSelector()].init != 0 calls it. CALLERS: 368E4.c (two, inside func_800460E4) and the asm of func_800460E4. "
     f"WHY: Invoke / Callback = the guarded indirect call; LoadedSelector = resource_GetLoadedSelector, the selector index. COUNTER-EVIDENCE: no stage identity (the member is called init, nothing more). Prior manifests: no_name.tsv (stage_ prefix unsupported). {T('0x8003F168')}"),

    ("0x8003F1C8", "gpu_GetLastGridPassViewYaw", "RENAME",
     f"{ch('0x8003F1C8')}BODY: {B('func_8003F1C8')}: returns D_800A336C. Its only writer is the grid pass ({L('2B344.c', r'D_800A336C = ang')}): ratan2(res[0], res[2]) of {{0, 0, 0x1000}} rotated by the matrix of D_800A3708's rotation, then (ang >> 6) & 0x3F indexes the row-mask tables. "
     f"CALLERS: none in src/ or asm. WHY: written once per grid pass, so Last; Yaw = ratan2 of the horizontal components. COUNTER-EVIDENCE: the registry note 'game mode / state' is not evidence and no reader treats it so. Prior manifests: wave69 (no caller). {T('0x8003F1C8')}"),

    ("0x8003F1D4", "gpu_GetGridNodeRecordTable", "RENAME",
     f"{ch('0x8003F1D4')}BODY: {B('func_8003F1D4')}: returns D_800A6690. Filled by func_8003EDC0's node section ({L('2B344.c', r'c = &D_800A6690\[n\+\+\]')}); read by the grid passes (unk58 flag, queue) and by 32D04.c:1187 (D_800A9CF8.unk10, 0x68-byte records) and 35000.c:67 / 129 (transform edits). "
     f"CALLERS: 32D04.c:1187 and the asm of func_8004473C. WHY: NodeRecordTable = the transform-node records of the grid stream. COUNTER-EVIDENCE: not a collision table. Prior manifests: wave69 grid_ territory (this is the node table, not a cell grid). {T('0x8003F1D4')}"),

    ("0x8003F420", "gpu_ReplaceCellAttributesInNearest2x2Block", "RENAME",
     f"{ch('0x8003F420')}BODY: {B('func_8003F420')}: (a0, a1) + 0x7D00 divided by 2000 gives the cell (s3, s2); the remainder against 1000 picks the neighbouring cell on the nearer side per axis (+-1); func_8003F52C(cell, 2) is called for (s3, s2), (s3 + s1, s2), (s3, s2 + s0), (s3 + s1, s2 + s0). "
     f"CALLERS: none in src/ or asm. WHY: Replace = func_8003F52C overwrites, Nearest2x2Block = the four cells nearest the point. COUNTER-EVIDENCE: the written value 2 and the attribute meaning are not named. Prior manifests: no_name.tsv (unknown fixed mode). {T('0x8003F420')}"),

    ("0x8003F52C", "gpu_ReplaceCellAttributesWithMaskedValue", "RENAME",
     f"{ch('0x8003F52C')}BODY: {B('func_8003F52C')}: D_800A8FB0[a1 * 32 + a0] = a2 & 3 (overwrite, clearing the gate 4 / force 8 bits that gpu_SetCellGateBit / gpu_SetCellForceBit OR in). CALLERS: the four calls in func_8003F420. "
     f"The byte is read by the grid passes (e->unk6 = a3 & 3, gate / force tests) and filled by gpu_ResetAndStampViewCenteredCellAttributes, so it is per-cell attribute data. WHY: Replace against the OR setters; MaskedValue = & 3. COUNTER-EVIDENCE: no collision or visibility meaning. Prior manifests: wave69 grid_ / cellgrid_ (cell attributes are shown by the consumers). {T('0x8003F52C')}"),

    ("0x8003F54C", "gpu_GetCellAttributes", "RENAME",
     f"{ch('0x8003F54C')}BODY: {B('func_8003F54C')}: returns D_800A8FB0[a1 * 32 + a0], the whole byte (not only the & 3 part). CALLERS: none in src/ or asm. The byte is the cell attribute data of func_8003F52C / gpu_SetCellGateBit / gpu_SetCellForceBit. "
     f"WHY: Get against the Replace / Set accessors. COUNTER-EVIDENCE: an uncalled getter is named from the data's other uses. Prior manifests: wave69 (no caller). {T('0x8003F54C')}"),

    ("0x8003F7F4", "softbody_FreeAllAndClearWriteOnlyWords", "RENAME",
     f"{ch('0x8003F7F4')}BODY: {B('func_8003F7F4')}: softbody_FreeAll(); func_80017F90() (an empty function, 6CF8.c:796); D_800A3370 = 0; D_800A3374 = 0. Both words are file-static in 2B344.c (lines 1127-1128) and are written only here and in ClearSavedWordPairsAndTwoWriteOnlyWords; no read exists in src/, include/ or asm. "
     f"CALLERS: {L('368E4.c', r'func_8003F7F4\(\)')} (func_80046B44). WHY: FreeAll is the substantive effect; the empty call adds none and the two words are write-only. COUNTER-EVIDENCE: no cloth or physics init is claimed. Prior manifests: no_name.tsv (callee analysis beyond transform context). {T('0x8003F7F4')}"),
]

DATA = []
