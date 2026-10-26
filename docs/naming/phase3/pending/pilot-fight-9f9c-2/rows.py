"""wave84 rows (pilot-fight-9f9c-2): Q132 mechanical names for 28 formerly NO_NAME functions of main/9F9C
(fight / camera / motion / start-position code). Research input: tmp/codex/pilot-fight-9f9c-2-research.md
(Codex, 28 RENAME proposals; untrusted). Every row was re-derived from the C bodies in this tree and every
caller (src/, include/, asm/funcs, symbol_addrs.txt, named_syms.txt). Names changed against the research say
CHANGED FROM RESEARCH in the evidence (ADJUSTED lists them); DROPPED lists research rows that stay neutral.
Each LEDGER clause quotes the old no_name.tsv reason and says why it no longer blocks. The research's data
suggestions (D_800A3858, D_800A38C4, D_8008EB1C ...) are not part of this wave (DATA is empty). TRACE clauses are
reach only (docs/naming/runtime/<capture>/hitmap.tsv + the SHA1 recorded in that capture's capture.manifest.json);
no name leans on a runtime role.
"""
import csv
import glob
import json
import os
import re

VERDICT = "PENDING"
VERIFIER_NOTE = ""

NOTES = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

# ---------------------------------------------------------------------------------------------------------
_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")

_NN = {}
for _r in csv.DictReader(open(os.path.join(WT, "docs", "naming", "phase3", "no_name.tsv"), encoding="utf-8"),
                         delimiter="\t"):
    _NN[_r["address"].lower()] = _r

_RT = os.path.join(WT, "docs", "naming", "runtime")
CAPS = ["versus-2p-rematch-1", "slash-mode-1", "slash-mode-clear-1", "lib-story-final-fight-retail-1",
        "story-pov-rematch-1", "group-battle-hitmap-1"]
_HIT = {}
_SHA = {}
for _c in CAPS:
    _rows = list(csv.reader(open(os.path.join(_RT, _c, "hitmap.tsv"), encoding="utf-8"), delimiter="\t"))
    _ph = _rows[0][2:]
    for _r in _rows[1:]:
        _HIT.setdefault(_r[1].lower(), {})[_c] = [_ph[i] for i in range(len(_ph)) if _r[2 + i] not in (".", "")]
    _m = json.load(open(os.path.join(_RT, _c, "capture.manifest.json"), encoding="utf-8"))
    for _o in _m.get("outputs", []) if isinstance(_m.get("outputs"), list) else []:
        pass
    _txt = open(os.path.join(_RT, _c, "capture.manifest.json"), encoding="utf-8").read()
    _mm = re.search(r'%s"\s*,\s*"sha1":\s*"([0-9a-f]{40})"' % re.escape(_c + "\\\\capture.jsonl"), _txt)
    assert _mm, _c
    _SHA[_c] = _mm.group(1)

_COV = {}
_rd = list(csv.reader(open(os.path.join(_RT, "matrix", "coverage.tsv"), encoding="utf-8"), delimiter="\t"))
_CAPS = _rd[0][5:]
for _r in _rd[1:]:
    _COV[_r[0].lower()] = [_CAPS[i] for i in range(len(_CAPS)) if _r[5 + i] not in (".", "")]


def B(fn):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    for f, lines in _SRC.items():
        for k, l in enumerate(lines):
            if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
                continue
            j = k
            while j < len(lines) and "{" not in lines[j] and ";" not in lines[j]:
                j += 1
            if j >= len(lines) or ";" in lines[j] and "{" not in lines[j]:
                continue
            if lines[j].rstrip().endswith("}"):
                return f"src/main/{f}:{k + 1}-{j + 1}"
            while j < len(lines) and lines[j] != "}":
                j += 1
            return f"src/main/{f}:{k + 1}-{j + 1}"
    raise SystemExit(f"no C definition of {fn}")


def C(fn):
    """Every C call site / table reference of fn outside its own definition and extern declarations, plus asm jal sites."""
    out = []
    for f, lines in sorted(_SRC.items()):
        for k, l in enumerate(lines):
            if re.search(r"\b%s\b" % fn, l) and not re.match(r"^(extern|[A-Za-z_][\w \*]*\b%s\()" % fn, l):
                out.append(f"src/main/{f}:{k + 1}")
    for p in sorted(glob.glob(os.path.join(WT, "asm", "funcs", "*.s"))):
        for k, l in enumerate(open(p, encoding="utf-8", errors="replace"), 1):
            if re.search(r"\bjal\s+%s\b" % fn, l):
                out.append(f"asm/funcs/{os.path.basename(p)}:{k}")
    return "; ".join(out) if out else "none"


def L(addr, why):
    r = _NN[addr.lower()]
    old = r["reason"].replace("\t", " ")
    return f"LEDGER: no_name.tsv ({r['waves']}) said '{old}'. {why}"


def T(addr):
    """Reach clause: coverage.tsv count plus the per-capture hit phases of the newer captures (with capture SHA1)."""
    h = _COV[addr.lower()]
    parts = []
    for c in CAPS:
        ph = _HIT.get(addr.lower(), {}).get(c, [])
        if ph:
            parts.append(f"{c} (capture.jsonl SHA1 {_SHA[c]}) phases {','.join(ph)}")
    hit = "; ".join(parts) if parts else "no hit in the six newer captures"
    return (f"TRACE: matrix/coverage.tsv reach {len(h)} of {len(_CAPS)} registered captures; "
            f"docs/naming/runtime/<capture>/hitmap.tsv: {hit}. Reach only (no only-X test run), "
            f"no role or identity is taken from it (Q124/Q132).")


# ---------------------------------------------------------------------------------------------------------
FUNCS = [
    ("0x8001A67C", "math_StoreFloorDiv2000XZOfPointStepTowards", "RENAME",
     f"CHANGED FROM RESEARCH: math_WriteFloorDividedNormalizedOffsetXZ ('Normalized' alone hides the fixed step length and the direction). "
     f"BODY: {B('func_8001A67C')}: dx,dz = arg1 - arg2 (x,z); both halved until each is within +-0x4000; dist_sq -> an approximate length var_3 "
     f"(g_sqrt_table_u8 lookup, with a gte_Lzc exponent for dist_sq >= 0x400); arg0[0] = math_FloorDiv2000(arg2[0] + (dx << 10) / var_3) and "
     f"arg0[2] likewise with dz and arg2[2]; arg0[1] is never written. That is the point 1024 units from arg2 along the approximately normalized "
     f"x/z direction toward arg1, divided by 2000 with floor (math_FloorDiv2000). CALLERS: {C('func_8001A67C')}: both pass (var_2->h30[p], &scr->eye, scr->hit), "
     f"the same roles. RETURN USE: void. WHY: math_ because it reads only the three supplied vectors and a lookup table and calls math_FloorDiv2000 "
     f"(no fighter, camera or global state; math_ names such as math_NegNormalize3D are the same shape); StepTowards = the 1024 offset toward the first vector. "
     f"COUNTER-EVIDENCE: not an angle or direction return; the y slot is skipped; the length is table-approximated; no camera role is claimed even though both callers are in "
     f"camera_UpdateTwoFighterView. "
     + L("0x8001A67C", "That objection (output coordinate convention) asked what the stored values mean; the name states exactly what is computed (a floor-divided x/z of an offset point) "
        "without claiming a convention, which Q132 permits.") + " " + T("0x8001A67C")),
    ("0x8001B6F4", "fight_SetViewMode1RestoreNodesClearCameraB1FAndSelectRenderConfig0", "RENAME",
     f"CHANGED FROM RESEARCH: fight_ConfigureReplayCameraAndRendering; 'Replay' is a role inferred from the sole caller, 'Camera/Rendering' hides that the body writes view mode 1 where every camera init writes 0 "
     f"and names none of the writes. BODY: {B('func_8001B6F4')}: func_80041688(0,0) and func_80041688(1,0) (clears bit 0 of the node flags of the nodes with unk2 >= 0 and of the list at +0x10D5, then reloads the "
     f"stored or grayscale colour; 31D3C.c:59-120); g_camera_fight_view_mode = 1 (9F9C.c:706,727 write 0; 1950-1963 write 0/1/2); D_800F6608.b1F = 0 and D_800F5328.b1F = 0 (the b1F byte of the two camera records, "
     f"set to 1 at 9F9C.c:894); func_8003F1E4(0) (2B344.c:2192-2200: D_800F6656 = 0, D_800F6658 = 1, the 3/2 pair for nonzero). CALLER: {C('func_8001B6F4')}: replay_Start only, once, "
     f"after its own func_80041688 pair. RETURN USE: void. WHY: fight_ follows g_camera_fight_view_mode and the two camera records it writes; every write is spelled in the name (the wave81 failure "
     f"was exactly 'Reset' being false and the b1F clears being omitted). COUNTER-EVIDENCE: not a reset (view mode 1 is not the init value); no meaning of b1F or of render config 0 is claimed "
     f"(func_8003F1E4 stays neutral); no replay role is claimed. "
     + L("0x8001B6F4", "The unnamed callees are now read (func_80041688 above, func_8003F1E4 two stores) and each effect is in the name, so a camera-only name omitting non-camera effects is not what is proposed.")
     + " " + T("0x8001B6F4")),
    ("0x8001C4C0", "fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11", "RENAME",
     f"CHANGED FROM RESEARCH: fight_ConditionallyRestoreDefaultScriptBeforeOpponentReinit ('BeforeOpponentReinit' and 'Restore' are context/guesses; the gate is the discriminator). "
     f"BODY: {B('func_8001C4C0')}: if D_80101EC8[0].move_kind is 0x32 or 0x11: motion_ResetVariantIndex(0); v0 = motion_GetDefaultVariantScript(0); D_80101EC8[0].move_is_common = 0; "
     f"fighter_StartMoveScript(0, v0, 0); otherwise nothing. CALLER: {C('func_8001C4C0')} (func_8001C51C first statement). RETURN USE: void. "
     f"WHY: fight_ (it works on the fight records D_80101EC8); the identical ResetVariantIndex / GetDefaultVariantScript / move_is_common = 0 / StartMoveScript sequence is the one fight_Init uses at 9F9C.c:1872-1877 for fighter 0. "
     f"COUNTER-EVIDENCE: not a general fighter reset; fixed record 0; the move kinds are named by their literal values, no identity is claimed. "
     + L("0x8001C4C0", "Q132: an unknown identity of the gate values is no ground for neutral; the literal values are in the name, which also keeps the narrow gate and fixed record the old objection asked for.")
     + " " + T("0x8001C4C0")),
    ("0x8001C51C", "fight_RestartFighter0ReinitFighter1ClearDelayAndSelectionResetObjectsCameraReplay", "RENAME",
     f"CHANGED FROM RESEARCH: fight_RestoreDefaultMovesReinitOpponentAndResetObjectsViewAndReplay ('Opponent' and 'Restore' are guesses; the two scalar clears were omitted). "
     f"BODY: {B('func_8001C51C')}: func_8001C4C0() (the move-kind-gated restart of fighter 0's default script); fighter_Init(1, D_80102778.unk_4[5], unk_4[1], unk_4[3], flag) with flag 0 when D_800A38DC == 3 && D_800A3728 != 0, else 1; "
     f"fighter_ConfigureCpuControl(); motion_ResetVariantIndex(1) and the default variant script started for fighter 1; D_800A382E = 0 (symbol_addrs.txt:2119: the frame counter gating fight_UpdateDefeatDelayAndModeProgress at 0x3D); "
     f"D_800A3748 = -1 (symbol_addrs.txt:657: index into D_80101EC8, -1 == none); object_FreeAttachedObjects() (17AFC.c:3778: kind = -1 for every pool object with is_attached) and func_80030D04() (17AFC.c:4002: kind = -1 for pool kinds 0x12..0x1D); "
     f"camera_InitTwoFighterView(&rec0,&rec1); replay_Reset(); fighter_AssignMotionSlotAndScanDataIdNibbles(1). CALLERS: {C('func_8001C51C')}: both inside fight_UpdateDefeatDelayAndModeProgress "
     f"after the 0x3D delay, each time as the replacement step before the fight proceeds; neither caller depends on a result (void). WHY: each sub-effect is a name part in execution order. "
     f"COUNTER-EVIDENCE: no next-round, rematch or opponent semantics are claimed; the object reset is selective (attached objects and kinds 0x12..0x1D), not a pool clear. "
     + L("0x8001C51C", "The name carries the session counters, objects, camera and replay effects the old fighter-one-only name omitted, and claims no round or opponent identity.")
     + " " + T("0x8001C51C")),
    ("0x8001CD68", "fight_SplitFrameCounterIntoMinutesSecondsHundredths", "RENAME",
     f"CHANGED FROM RESEARCH: fight_FormatElapsedTimeMinutesSecondsHundredths ('Format' suggests text; 'Elapsed' is an inference from the counter's role, the code shows a frame counter). "
     f"BODY: {B('func_8001CD68')}: val = D_800A3858; above 0x2BF1F stores 99 / 59 / 99 into unk_0 / unk_2 / unk_3; else unk_0 = val / 1800, unk_2 = val / 30 - unk_0 * 60, unk_3 = (val % 30) * 100 / 30 "
     f"(game.h:1936 documents the record). CALLERS: {C('func_8001CD68')}: 9F9C.c:1456 and 2126 pass a local buffer to func_8005D814 (draws it), 2B344.c:853 copies the three fields into the save record "
     f"(+0x2D..+0x2F); all three use the three fields as minutes / seconds / hundredths of the counter. RETURN USE: void. "
     f"WHY: fight_ because D_800A3858 is incremented by fight_UpdateModeDisplaysAndMatchProgress (9F9C.c:1451), penalised by func_8001EA84, cleared in 24BF0.c:69 and inserted by fight_InsertBestTime (17AFC.c:5479-5508); "
     f"1800 = 30 * 60 and 30 per second is the arithmetic itself. COUNTER-EVIDENCE: global input, so no math_; not rounding (hundredths truncate); 30 Hz is the code's constant, not a claim about elapsed real time. "
     f"LEDGER: no_name.tsv has no row for this address (a free target), so no earlier objection applies. "
     + T("0x8001CD68")),
    ("0x8001D790", "fight_ResetGraphAndReloadSelectorResourcesIfSelectionsChanged", "RENAME",
     f"CHANGED FROM RESEARCH: resource_ResetGraphAndReloadChangedFightResources (resource_ has only the 368E4 selector uses and is unexplained for this body; 'Fight' is the caller's TU, 'Changed' is vague). "
     f"BODY: {B('func_8001D790')}: gpu_ResetGraphMode1() always; if D_800A36A4 != D_800A390E or D_8008E5A8[(s8)D_80102778.unk_4[0]] != D_800A30FC or the [1] analogue != D_800A30FD: func_80020D38(); "
     f"func_80046EDC(D_800A36A4, 0x80190800) (368E4.c:542: func_800460E4, func_800421C8, func_8003E0E0); func_8002906C() (17AFC.c:948: clears `used` of every position record); snd_CloseListedVabs(); "
     f"s1 = func_8005BA8C(0x80190800, D_800A36A4, D_8008E5A8[..0], D_8008E5A8[..1]) (4BC3C.c:244-298: closes the three listed VABs, reads CD file D_800A36A4 + 9, installs the billboard list, loads 2-3 VAB packs and packs them back to back); "
     f"the three cache words are stored; s1 >= 0x2519 -> sys_Panic; memcpy(&D_800FF6A8, 0x80190800, s1); snd_RebaseListedVabs(D_800FF6A8 - 0x80190800). CALLERS: {C('func_8001D790')}: "
     f"fight_Init (9F9C.c:1852) and the 2B344.c:440 loading phase, both only run it for its reload. RETURN USE: void. WHY: fight_ because the first caller is fight_Init and the cached selectors are the fight's D_80102778 picks; "
     f"the name keeps the unconditional ResetGraph, the guard, and 'Resources' (sound and non-sound) rather than a sound-only word, which the billboard install at 4BC3C.c:267 would contradict. "
     f"COUNTER-EVIDENCE: no stage or character identity is claimed for D_800A36A4 or the D_8008E5A8 values (symbol_addrs.txt:857 calls D_800A36A4 only 'selector'); the ResetGraph runs even when nothing reloads. "
     + L("0x8001D790", "Q132: the concise resource identity is replaced by naming the demonstrated mechanics (guard on the three cached selections, ResetGraph, reload, rebase), with the non-sound content included.")
     + " " + T("0x8001D790")),
    ("0x8001D904", "snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVab9", "RENAME",
     f"CHANGED FROM RESEARCH: resource_ResetGraphLoadCopyAndRebaseVabPack (resource_ unexplained; 'Rebase' is the callee snd_VabFakeOpen9's spelling, and the VAB number is what tells it from its sibling). "
     f"BODY: {B('func_8001D904')}: gpu_ResetGraphMode1(); func_80020D38() (forgets slot 1's marker and ndata pack, 9F9C.c:3018); func_8005B9C4() (4BC3C.c:211-216: SsVabClose(9), clears g_vab_rec_ptr[9] and g_vab_vb_sbaddr[9]); "
     f"s1 = func_8005B9FC(0x80190800) (4BC3C.c:218-228: CD file index 8 read to the buffer, snd_LoadVabPack(.., 9, ..)); s1 >= 0xE81 -> sys_Panic; memcpy(&D_80104F38, 0x80190800, s1); snd_VabFakeOpen9(D_80104F38 - 0x80190800). "
     f"CALLER: {C('func_8001D904')}: fight_Init only, under D_800A38DC == 5 (9F9C.c:1853-1854); func_8001DA2C closes VAB 9 for the same mode. RETURN USE: void. WHY: snd_ because the body is a VAB 9 load: SsVabClose, snd_LoadVabPack, snd_VabFakeOpen9 "
     f"(existing sibling name encodes the same number); the graph reset and the slot-1 forget are explicit parts. COUNTER-EVIDENCE: not audio playback or a CD read alone; no mode name is claimed. "
     + L("0x8001D904", "the old objection forbade guessing VAB numbers or modes; here the number is the literal in the callee that the existing name snd_VabFakeOpen9 already carries, and the mode is not named.")
     + " " + T("0x8001D904")),
    ("0x8001D998", "snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVabs8And4", "RENAME",
     f"CHANGED FROM RESEARCH: resource_ResetGraphLoadCopyAndRebaseConcatenatedVabPacks (resource_ unexplained). "
     f"BODY: {B('func_8001D998')}: gpu_ResetGraphMode1(); func_80020D38(); func_8005B868() (4BC3C.c:171-179: SsVabClose(8) and (4), clears their g_vab_rec_ptr and g_vab_vb_sbaddr); "
     f"s1 = func_8005B8B8(0x80190800) (4BC3C.c:181-204: CD file index 0x5D to the buffer then snd_LoadVabPack(.., 8, ..), file 0x5E after the first pack's length then snd_LoadVabPack(.., 4, ..), returns the sum); "
     f"s1 >= 0x1B19 -> sys_Panic; memcpy(&D_80104F38, 0x80190800, s1); snd_VabFakeOpen8And4(D_80104F38 - 0x80190800) (4BC3C.c:206-209: snd_VabFakeOpen(.., 8) and (.., 4)). "
     f"CALLER: {C('func_8001D998')}: fight_Init only, under D_800A38DC == 3 (9F9C.c:1856-1857); func_8001DA2C closes VABs 8 and 4 for that mode. RETURN USE: void. "
     f"WHY: twin of func_8001D904 (instruction shape identical) but the VABs are 8 and 4, as in the existing snd_VabFakeOpen8And4. COUNTER-EVIDENCE: no mode or content is claimed. "
     + L("0x8001D998", "same as func_8001D904: the literal VAB numbers are the sibling distinction and match the existing snd_VabFakeOpen8And4 callee name.")
     + " " + T("0x8001D998")),
    ("0x8001DA2C", "snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode", "RENAME",
     f"CHANGED FROM RESEARCH: snd_ResetVoicesQueueAndReverbAndCloseModeVabs (the VAB numbers and which mode value picks which are in the code). "
     f"BODY: {B('func_8001DA2C')}: snd_ResetVoicesAndQueue() (4BC3C.c:91: g_snd_queue[0..23].req = 0, volumes 0x7F, func_80086130 per voice); snd_StopVoicesAndReverb() (4BC3C.c:353: SsUtReverbOff, type 0, depth 0); "
     f"D_800A38DC == 5 -> func_8005B9C4() (SsVabClose(9)); D_800A38DC == 3 -> func_8005B868() (SsVabClose(8) and (4)). CALLERS: {C('func_8001DA2C')}: all are leaving or cleaning-up sites that run it for its own effect (no result). "
     f"RETURN USE: void. WHY: snd_ because every effect is a sound operation (voices, queue, reverb, VAB close); the modes 5 and 3 match the modes in which fight_Init loads VAB 9 and VABs 8 and 4 (9F9C.c:1853-1857). "
     f"COUNTER-EVIDENCE: the closure is conditional; it does not unload anything in other modes; no mode names are claimed. "
     + L("0x8001DA2C", "the unnamed callees are now read (voice/queue reset, reverb off, SsVabClose), so the whole effect is spelled; the exit-path frequency is not used as evidence.")
     + " " + T("0x8001DA2C")),
    ("0x8001DB9C", "motion_LoadNdataPackIntoSlot1BufferAndMarkSlot1Id", "RENAME",
     f"CHANGED FROM RESEARCH: resource_LoadPackByFighterKindAndMarkSharedBuffer (resource_ unexplained; 'FighterKind' is the D_8008D9EC flag, not a kind; 'SharedBuffer' is slot 1's buffer, which motion_InitBuffers names). "
     f"BODY: {B('func_8001DB9C')}: ndata_LoadPack(D_8008D9EC[D_80101EC8[0].unk_0A] < 1, 0x80190800) (35000.c:278: ndata_Read(index + 0x25, 0x80190800); g_ndata_pack = 0x80190800; g_ndata_pack_loaded = 1); "
     f"D_800A38C4[1] = 0xFFFF. 0x80190800 is g_motion_fighter_buffers[1] (motion_InitBuffers, 9F9C.c:3032); the 9F9C.c:2999-3003 comment says D_800A38C4[i] is the motion id loaded into slot i and 0xFFFF in slot 1 marks an ndata pack there. "
     f"CALLER: {C('func_8001DB9C')}: fight_Init only (9F9C.c:1858), just before motion_LoadFighterData. RETURN USE: void. WHY: motion_ because the destination is a motion fighter buffer and the marker is the motion-slot id table "
     f"(read by motion_LoadFighterData 3107-3154 and by func_80020CDC / func_80020D38); the pack choice comes from fighter 0's D_8008D9EC flag. COUNTER-EVIDENCE: no pack content (dialogue, motion) is claimed. "
     + L("0x8001DB9C", "the pack/marker relation is now shown: the load address is slot 1's buffer and the 0xFFFF store is the documented ndata marker that func_80020CDC / func_80020D38 test.")
     + " " + T("0x8001DB9C")),
    ("0x8001E800", "fight_QueueTrackedFighterNodesFromRecordFlagsAndInitOt", "RENAME",
     f"CHANGED FROM RESEARCH: fight_BuildTrackedFighterViewQueueNodesAndLinkOrderingTable (view-matrix and 'link' claims restated from the callee; the discriminator is which fighter and which flags). "
     f"BODY: {B('func_8001E800')}: ptr = D_80101EC8[D_800A36F6]; a1 = ptr->unk_0E when ptr->unk_62 & 1 else -1; func_80048BA4(D_800F5328.h1C, a1, (ptr->unk_62 & 4) > 0). func_80048BA4 (368E4.c:1563-1634) uses "
     f"mat_tbl = node_GetSlotMatrixPointerArray(D_800A33E0): D_800A33E0 is written only by func_80048AD0, whose single call (9F9C.c:1814) is guarded by D_800A38BA != 0 && D_800A36F6 == i, so the slot is the tracked fighter's; "
     f"it builds the D_800FF558 transform from the angle h1C, queues one node per index of D_80099C14 on g_draw_queue, plus nodes 18 (selected by a1) and 19 (when the flag is set), then clears and re-links g_gpu_ot256. "
     f"CALLER: {C('func_8001E800')}: fight_Update only, in the D_800A38BA != 0 && g_camera_fight_view_mode == 0 branch (9F9C.c:2076-2077) where the else branch calls gpu_SetViewOriginSegmentCellsGateBit. RETURN USE: void. "
     f"WHY: 'Tracked' is the word camera_UpdateTrackedFighterView already uses for D_80101EC8[D_800A36F6]; the queue and ot parts are the callee's writes; the flags are the record's unk_62 bits. "
     f"COUNTER-EVIDENCE: no shadow, reflection or weapon identity of the nodes is claimed; the unk_62 bit meanings are not claimed; it is not the whole fighter update. "
     + L("0x8001E800", "the rendered-node identity is not needed: the name states whose slot, which record flags select the extra nodes, and that the callee queues nodes and initialises the OT (callee body read).")
     + " " + T("0x8001E800")),
    ("0x8001EA04", "fight_RestoreNodesStopEffectsClearUnk31AAndInitThreeItemMenu", "RENAME",
     f"CHANGED FROM RESEARCH: fight_InitChoiceLoopRestoreNodeColoursAndStopEffects ('ChoiceLoop' and the omitted unk_31A clears). "
     f"BODY: {B('func_8001EA04')}: func_80041688(0,0) and (1,0) (node flag bit 0 cleared, colours reloaded; 31D3C.c:59-120); eff_StopCommands(); unk_31A of both records = 0; D_800A37B8 = 0 (frame counter); D_800A3929 = 0; "
     f"D_800A3834 = 0xD; D_800A3804 = D_800A3817 = (D_800A38D4 < 1). D_800A3817 is documented (symbol_addrs.txt:1868) as the cursor of a three-item pad menu with lower bound D_800A3804, D_800A3929 as its confirm delay "
     f"(0 while choosing); phase 0xD is index 13 of the handler table D_8008D090 (d_7D870.c:30-60), which is func_8001EA84. CALLERS: {C('func_8001EA04')}: only the handler table slot 12 (d_7D870.c:42), "
     f"entered by writes of D_800A3834 = 0xC (28708.c:1527,1544,1552, and func_8001EFA0's own D_800A3834 = var_v0 = 0xC); no direct C call. RETURN USE: void. WHY: the name lists the writes and the menu state it initialises; fight_ because it runs the fight records "
     f"(80041688 on both fighter slots) and the handler-table pair is the fight's. COUNTER-EVIDENCE: no meaning of the three items or of why the menu appears is claimed; unk_31A is a raw field. "
     + L("0x8001EA04", "the display helpers are now resolved (func_80041688 body) and the selector is shown as D_800A38D4 < 1 initialising D_800A3804 / D_800A3817; no continue/retry labels are used.")
     + " " + T("0x8001EA04")),
    ("0x8001EA84", "fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange", "RENAME",
     f"CHANGED FROM RESEARCH: fight_UpdateFighterFocusChoiceLoopAndDelayedTransitions ('ChoiceLoop' unsupported; the menu is the documented three-item pad menu). "
     f"BODY: {B('func_8001EA84')}: D_800A37B8++; camera_SetFighterFocusView(&D_80101EC8[D_800A3748 == 0], D_800A37B8 << 3); camera_ApplyFightView; the simulation frame with pad_ClearStateBits'd input "
     f"(replay_BeginFrame, fight_UpdateSeparationAndPairedMoves, fighter_Update x2, fight_UpdateFighterInteractionsAndPointCaches, object_UpdatePool, fight_UpdateBallisticRecords, replay_AdvanceFrame, the draw-queue flush, snd_UpdateStageSounds); "
     f"in D_800A38DC == 3 draws the func_8001CD68 time; while D_800A3929 == 0 draws the menu (func_8005C8A8 with D_800A3817), steps D_800A3817 on the 0x10001000 / 0x40004000 pad bits with wrap and a queued sound, "
     f"and on 0x400040 sets D_800A3929 = 1 (item 0) or 0x3C, clearing unk_B3 of the focused fighter and adding 0x384 (capped 0x2BF20) to D_800A3858 in mode 3 for item 0; once D_800A3929 reaches 0x3C it writes D_800A3834: "
     f"item 0 -> D_800A3670 = 1, D_800A380C++, D_800A38DF = fight_FindNearestStartPair(..), fighter_HalveCpuTally(1) when cpu_controlled, phase 0; items 1 and 2 -> cdrom_PauseAndWait, func_8001DA2C, phase 8 (item 1 also D_800A31DA = 1). "
     f"CALLER: {C('func_8001EA84')}: handler table slot 13 only (d_7D870.c:43), entered from func_8001EA04. RETURN USE: void. WHY: every phase of the body is a name part; camera_SetFighterFocusView is the existing camera name. "
     f"COUNTER-EVIDENCE: not merely a menu draw; no meaning of the three items is claimed, nor of the time penalty. "
     + L("0x8001EA84", "the choices need no identity: they are named by structure (three-item menu, confirm delay, phase change) and the simulation, penalty and delayed exits the old note warned about are in the name's parts.")
     + " " + T("0x8001EA84")),
    ("0x8001EEB4", "fight_StartIndexedCommonScriptOnSelectedFighterIfGatedAndEnterInterpolatedViewPhase", "RENAME",
     f"CHANGED FROM RESEARCH: fight_StartConditionalCommonMoveAndTimedFocusPhase (the script start is the indexed common script of func_80021A3C, not a 'move'; 'Timed' is the frame-counter interpolation). "
     f"BODY: {B('func_8001EEB4')}: entry = D_80101EC8[D_800A3748]; when move_kind is not 0xA, 0x17, 0x18, unk_72 == 0 and is_defeated == 0: motion_ResetVariantIndex(D_800A3748); ret = func_80021A3C(D_800A3748, entry->unk_0A); "
     f"entry->move_is_common = 1; fighter_StartMoveScript(D_800A3748, ret, 1); entry->unk_26C = 1; always eff_StopCommands(); D_800A37B8 = 0; D_800A3834 = 0x11. Phase 0x11 is table slot 17 = func_8001EFA0, whose camera is "
     f"camera_SetInterpolatedFighterView(&D_80101EC8[D_800A3748], D_800A37B8 * 4096 / 105). CALLERS: {C('func_8001EEB4')}: handler table slot 16 (d_7D870.c:46) only, entered by D_800A3834 = 0x10 (28708.c:1550). RETURN USE: void. "
     f"WHY: 'Selected fighter' is D_80101EC8[D_800A3748] (symbol_addrs.txt:657); the gate, the indexed common script (func_80021A3C, below) and the phase write are the three parts. "
     f"COUNTER-EVIDENCE: no winner, victory or move identity is claimed (D_800A3748 is set to the other fighter of the first one defeated, 9F9C.c:5160, but the name does not rely on that); the phase and effect writes are unconditional. "
     + L("0x8001EEB4", "the shared trigger need not be named: the two operations are named separately (gated indexed common script start; unconditional interpolated-view phase entry).")
     + " " + T("0x8001EEB4")),
    ("0x8001EFA0", "fight_UpdateInterpolatedFighterViewAndSelectNextPhaseByModeOnTimeoutOrPadPress", "RENAME",
     f"CHANGED FROM RESEARCH: fight_UpdateTimedFighterFocusAndDispatchModeProgress ('FighterFocus' is camera_SetFighterFocusView, a different function; this body calls camera_SetInterpolatedFighterView). "
     f"BODY: {B('func_8001EFA0')}: D_800A37B8++; camera_SetInterpolatedFighterView(&D_80101EC8[D_800A3748], (D_800A37B8 << 12) / 105); camera_ApplyFightView; the same neutral-input simulation frame as func_8001EA84; "
     f"if the selected fighter is_defeated in mode 1 the counter is set to 0x69; at counter >= 0x69 or the 0x400040 pad bits: mode 4/6 -> phase 0xC; mode 1 -> if D_800A3748 == 0 func_8001DA2C, D_800A3768 = 2, fight_AdvanceOpponentOrder and return, else phase 0xC; "
     f"other modes -> func_8001DA2C and phase 2. CALLER: {C('func_8001EFA0')}: handler table slot 17 only (d_7D870.c:47), entered from func_8001EEB4. RETURN USE: void. "
     f"WHY: the view call is the discriminator against func_8001EA84; the dispatch is the switch over D_800A38DC. COUNTER-EVIDENCE: no menu_ prefix (the pad bits only skip the timer), no replay_ prefix (no replay ring access), no scene identity. "
     + L("0x8001EFA0", "scene identity is not claimed: the name carries the interpolated view, the timeout-or-pad condition and the mode switch; the rejected menu_ / replay_ prefixes are not used.")
     + " " + T("0x8001EFA0")),
    ("0x8001FB34", "fighter_PassesPairedMoveEntryModeOpponentUnk0CObjectAndUnk26CGates", "RENAME",
     f"CHANGED FROM RESEARCH: fighter_PassesPairedMoveEntryStateGates ('State' was vague; the body's four gates are listed). "
     f"BODY: {B('func_8001FB34')}: returns 0 when D_800A38DC is 2, 5 or 3; or D_800A38DC == 0 && D_800A385C != 0; or arg0->other->unk_0C is 0xD or 0x1C; or arg0->unk_0A == 0xE and (carried_object_count == 0 or carried_object_kinds[0] != 0xA); "
     f"then arg1 != 0 ? arg0->unk_26C != 0 : 1. CALLER: {C('func_8001FB34')}: fight_UpdatePairedMoves (9F9C.c:2706) passes the entry's (data[3] & 0x80), and afterwards tests the distance window, height and opponent move kind itself. RETURN USE: s32 tested "
     f"with == 0 -> continue. WHY: the name lists what it gates and that it is only the entry-state part of paired-move eligibility. COUNTER-EVIDENCE: not a complete paired-move capability test; the optional unk_26C requirement is caller-selected; "
     f"no character or object identity is named (0xE and 0xA stay raw). "
     + L("0x8001FB34", "the precise eligible class is not claimed: the four gates are named by their fields, and the name says they are the entry gates only, so neither the optional requirement nor the character-specific datum is hidden.")
     + " " + T("0x8001FB34")),
    ("0x800203B4", "fighter_StoreBoneLocalVectorAndArmEventTrigger", "RENAME",
     f"CHANGED FROM RESEARCH: fighter_ArmBoneLocalVectorEvent. BODY: {B('func_800203B4')}: rec->unk_350 = 1; rec->unk_352 = D_8008D59C[arg1].bone; src = node_GetSlotMatrixPointerArray(rec->index)[rec->unk_352]; "
     f"math_InvertMatrix3x3(src, mat); the vector arg2 (3 x s16) is rotated by the inverse (gte_rtv0) and stored at rec->unk_354, i.e. the vector in that bone's local frame. CALLERS: {C('func_800203B4')}: all inside "
     f"fighter_ResolveBoneHit (17AFC.c:404,409,477,499,509,520), each time with the hit's bone group index and the D_800A37E8 vector, followed by fighter_DispatchBoneHitSoundEvent or fighter_TryDropQueuedHeadAtBone; "
     f"RETURN USE: ignored. CONSUMER: func_800204C0 reads exactly unk_350 / unk_352 / unk_354 and calls func_80032854(rec->index, 4, point, vector), which records a replay event (replay_RecordEvent, 17AFC.c:4689) and starts a command. "
     f"WHY: Store = the three field writes; BoneLocal = the inverse-matrix rotation; ArmEventTrigger = unk_350 = 1, consumed by the dispatcher below. COUNTER-EVIDENCE: it emits nothing itself; no visual or sound effect (the wave81 'Feedback' coinage) is claimed. "
     + L("0x800203B4", "the eventual operation's identity is not needed: the name states the stored vector and that a trigger is armed for the event dispatcher; 'Event' is the word of replay_RecordEvent / func_80032854's own argument, not a visual.")
     + " " + T("0x800203B4")),
    ("0x800204C0", "fighter_DispatchArmedBoneVectorEventAndDisarm", "RENAME",
     f"CHANGED FROM RESEARCH: fighter_DispatchArmedBoneVectorEventAndClearTrigger (same meaning; 'Disarm' pairs with the producer's 'Arm'). BODY: {B('func_800204C0')}: when rec->unk_350 != 0: unk_350 += 1; on (unk_350 & 7) == 2 the stored vector unk_354 "
     f"is rotated by matrix unk_352 of node_GetSlotMatrixPointerArray(rec->index) (gte_rtv0), scaled by (0x96 - unk_350) * 4096 / 150 / 0x1000, negated when out[1] > 0x800, and passed to func_80032854(rec->index, 4, &SPAD->unkA8[..].x, out); "
     f"after the `>= 0x96` test the counter is stored 0 unconditionally, so every armed call ends disarmed. CALLER: {C('func_800204C0')}: fighter_Update (9F9C.c:5222). RETURN USE: void. "
     f"WHY: Dispatch = func_80032854 call; Armed = the guard; BoneVector = the unk_352 / unk_354 pair the producer wrote; Disarm = the unconditional unk_350 = 0. COUNTER-EVIDENCE: not a persistent emitter or timer (the final store always clears); "
     f"no effect identity (blood, spark) is claimed; 'Event' is replay_RecordEvent's word. "
     + L("0x800204C0", "the old note warned that the unconditional clear defeats a timer or emitter reading; the name says Disarm (one dispatch per arming) and uses no effect identity.")
     + " " + T("0x800204C0")),
    ("0x80020CDC", "motion_InvalidateMarkedNdataPackAndResetLoadCaches", "RENAME",
     f"CHANGED FROM RESEARCH: motion_InvalidateAllLoadCachesAndSharedPack ('Shared' is a guess; the ndata part is conditional on the marker). BODY: {B('func_80020CDC')}: if D_800A38C4[1] == 0xFFFF ndata_InvalidatePack(); "
     f"g_motion_common_loaded = 0; D_800A38C4[1] = D_800A38C4[0] = 0; g_motion_frame_set_ids[1] = g_motion_frame_set_ids[0] = 0xFF. All three tables guard loads in motion_LoadFighterData (9F9C.c:3076-3155: D_800A38C4 compares at 3107-3121, "
     f"the common flag and frame-set ids in the loaders), and the 9F9C.c:2999 comment states D_800A38C4 holds the motion id loaded into each slot. CALLERS: {C('func_80020CDC')}: motion_InitBuffers (9F9C.c:3033) and the loading / cleanup sites in 25C38.c and 2B344.c; "
     f"every one runs it for the invalidation, none uses a result. RETURN USE: void. WHY: motion_ because every global is a motion-load cache and the name of its first caller (motion_InitBuffers) agrees. "
     f"COUNTER-EVIDENCE: nothing is wiped or streamed, only metadata; the pack invalidation is conditional. "
     + L("0x80020CDC", "the global roles are now each shown by their load-guard uses in motion_LoadFighterData, so motion_ is established by the data and not by one caller.")
     + " " + T("0x80020CDC")),
    ("0x80020D38", "motion_InvalidateMarkedNdataPackAndClearSlot1Id", "RENAME",
     f"CHANGED FROM RESEARCH: resource_ForgetSharedFighterBufferContents (resource_ unexplained). BODY: {B('func_80020D38')}: if D_800A38C4[1] == 0xFFFF ndata_InvalidatePack(); D_800A38C4[1] = 0. "
     f"It is the slot-1-only part of func_80020CDC: it leaves D_800A38C4[0], g_motion_common_loaded and the frame-set ids alone. Slot 1's buffer is 0x80190800 (motion_InitBuffers), which func_8001DB9C and the "
     f"VAB loaders func_8001D790 / D904 / D998 use as their scratch address. CALLERS: {C('func_80020D38')}: the resource loaders above, fight_Init (9F9C.c:1810) and 2B344.c loading functions; each calls it before reusing "
     f"0x80190800 or loading fighter data. RETURN USE: void. WHY: the name is the two writes; motion_ for the same reason as func_80020CDC; Slot1 distinguishes it from that all-slot sibling by the code's own index. "
     f"COUNTER-EVIDENCE: no memory is cleared; no mode-only lifetime is claimed. "
     + L("0x80020D38", "the objection wanted a role other than an ordinal; here the distinguishing fact is the code's own slot index 1 against the all-caches sibling, and the buffer-sharing chain is in the evidence, not asserted as a lifetime.")
     + " " + T("0x80020D38")),
    ("0x800213A0", "motion_AdvanceVariantIfAtNibble4Or5Position", "RENAME",
     f"CHANGED FROM RESEARCH: motion_SkipVariantMatchingTrackedNibblePosition. BODY: {B('func_800213A0')}: a1 = rec->unk_86 (the current variant; motion_ResetVariantIndex sets it from unk_84); if a1 equals unk_88 or unk_8E: "
     f"unk_86 = (a1 + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14; else returns. unk_88 / unk_8E are written by fighter_AssignMotionSlotAndScanDataIdNibbles (9F9C.c:3172-3200) as the nibble positions i at which "
     f"motion_data_id holds the value 4 and 5 (and are -1 from fighter_Init, 3650). CALLER: {C('func_800213A0')}: motion_ResolveMoveData (9F9C.c:3264) under D_800A38DC == 3 && cpu_controlled, once. RETURN USE: void. "
     f"WHY: the guard values 4 and 5 are literal in the scan that fills the two fields; Advance = the modular increment. COUNTER-EVIDENCE: advances once, not until a valid entry; no equipment or weapon identity of nibbles 4 and 5. "
     + L("0x800213A0", "what the skipped entries are is not claimed: the name states the guard by the nibble values the scan stores, and Advance (not a generic NextVariant) carries the conditional.")
     + " " + T("0x800213A0")),
    ("0x800219E4", "motion_GetCommonScriptAtFighterBufferOffset", "RENAME",
     f"CHANGED FROM RESEARCH: motion_GetScalarOffsetCommonScript. BODY: {B('func_800219E4')}: &g_motion_common_sections.unk_00[g_motion_fighter_buffers[D_80101EC8[a0].motion_slot]->f16], i.e. the script at the fighter buffer's single "
     f"offset f16 inside the common section (the script then starts with move_is_common = 1). CALLERS: {C('func_800219E4')}: fight_Init (9F9C.c:1893,1898) for fighters 0 and 1, each time with move_is_common = 1 and fighter_StartMoveScript(.., 1). "
     f"RETURN USE: passed to fighter_StartMoveScript. WHY: motion_ with the motion_Get*Script family (motion_GetDefaultVariantScript, motion_GetCurrentVariantScript return fighter-section scripts; these two return common-section scripts); "
     f"the sibling func_80021A3C indexes an offset array, this one reads one offset. COUNTER-EVIDENCE: no entrance, idle or opening identity; startup placement is not evidence of one. "
     + L("0x800219E4", "the selected script's identity is not claimed; the name states the section (common) and the lookup shape (one buffer offset).")
     + " " + T("0x800219E4")),
    ("0x80021A3C", "motion_GetCommonScriptAtFighterBufferIndexedOffset", "RENAME",
     f"CHANGED FROM RESEARCH: motion_GetIndexedOffsetCommonScript. BODY: {B('func_80021A3C')}: &g_motion_common_sections.unk_00[g_motion_fighter_buffers[D_80101EC8[a0].motion_slot]->f18[a1]]: the offset is element a1 of the "
     f"f18 array. CALLER: {C('func_80021A3C')}: func_8001EEB4 (9F9C.c:2203) with a1 = entry->unk_0A, then move_is_common = 1 and fighter_StartMoveScript(.., ret, 1). RETURN USE: passed to fighter_StartMoveScript. "
     f"WHY: twin of func_800219E4 differing by the array index. COUNTER-EVIDENCE: no win / death / result script identity. "
     + L("0x80021A3C", "the table's semantic distinction is not claimed: scalar versus indexed is the whole difference and it is in the name.")
     + " " + T("0x80021A3C")),
    ("0x80021DB0", "fight_SearchRadialFlatSurfacePointOrUseStartPositionPair", "RENAME",
     f"CHANGED FROM RESEARCH: fight_SearchRadialSupportPositionOrUseStartPair ('Support' is unproved; 'FlatSurface' is the exact test: normal y == -0x1000). BODY: {B('func_80021DB0')}: base = pos with y - 500; for 8 directions from a random angle: "
     f"a 4000-unit segment must not hit (collision_TestSegmentWithWork), then up to 40 steps via func_8005344C advance *out; then a 4000-unit downward segment from *out must hit with normal y == -0x1000, and a downward refinement in 100-unit steps "
     f"returns out->y = the contact y; if no direction succeeds: mode 3 uses start-pair index D_800A38E0, otherwise the pair nearest pos by the second endpoint; out = that pair's second endpoint (offset +3). The first argument is unused. "
     f"CALLER: {C('func_80021DB0')}: fighter_Init (9F9C.c:3703) under D_800A38DC 2 or 3 with arg4 != 0, passing the opponent's base_pos. RETURN USE: void (writes *out). WHY: Search / Radial / FlatSurface / Point = the loop; "
     f"OrUseStartPositionPair = the fallback via fight_GetStartPositionPairTable (existing name, same table as fight_GetStartPosition). COUNTER-EVIDENCE: no safe-spawn or separation guarantee (the old failure); func_8005344C extends short probes backward, "
     f"so contact may lie behind the probe; fallback base handling differs between branches. "
     + L("0x80021DB0", "no guarantee is named; the search / probe / fallback mechanics and the unused argument are described, as Q132 asks.")
     + " " + T("0x80021DB0")),
    ("0x80022224", "fight_PickRandomStartPositionExcludingNearestAndFarthest", "RENAME",
     f"CHANGED FROM RESEARCH: fight_SelectRandomStartPointAfterDiscardingNearestThenFarthest (shortened; the order is irrelevant to the result). BODY: {B('func_80022224')}: for the 4 pairs of fight_GetStartPositionPairTable block D_800A36A4, "
     f"dists[i] = squared x/z distance from arg2 to the second endpoint (offset +3); the minimum index is set to -1, then the maximum of the rest is set to -1; picks[] = the two remaining indices; out = the second endpoint of picks[rand() & 1]. "
     f"The first argument is unused. CALLER: {C('func_80022224')}: fighter_Init (9F9C.c:3693) under D_800A38DC == 0 with arg4 != 0 and arg2 = the opponent's base_pos. RETURN USE: void (writes arg1). "
     f"WHY: Pick = random choice; Excluding... = the two discards; same table and endpoint as func_80021DB0's fallback. COUNTER-EVIDENCE: ties: only one index per extreme is excluded; not simply nearest, farthest or fully random. "
     + L("0x80022224", "the objection was that nearest / farthest / random alone are false; the name states the combined mechanic.")
     + " " + T("0x80022224")),
    ("0x800224E0", "motion_SelectVariantNibblePositionMatchingPreferredPairInOpponentValue", "RENAME",
     f"CHANGED FROM RESEARCH: motion_SelectOpponentNibbleVariantByPreferredValues. BODY: {B('func_800224E0')}: p = D_8008EB1C[D_800A384C] (two preferred values); val = D_8008DB1C[other->unk_0A][other->unk_0E]; for j in 0..1, for i in 0..2: "
     f"if p[j] == (val >> (i * 4)) & 0xF returns i; else 0. CALLER: {C('func_800224E0')}: fighter_Init (9F9C.c:3666) stores it into p->unk_84 (the default variant: motion_ResetVariantIndex and motion_GetDefaultVariantScript read unk_84) "
     f"for fighter 1 under D_800A38DC == 3. RETURN USE: the stored position is the result domain. WHY: Select (not Find): 0 is both a real first-position match and the fallback, so there is no distinct not-found value; "
     f"the three nibbles of the opponent's D_8008DB1C word and the two preferred values are the loop. COUNTER-EVIDENCE: no meaning of the nibble values or of D_8008EB1C is claimed. "
     + L("0x800224E0", "the domain identity is shown by the sole use (default variant unk_84 of fighter 1 in mode 3) and the verb avoids the Find semantics the old note rejected.")
     + " " + T("0x800224E0")),
    ("0x80022568", "fighter_ResetBoneGroupHitState", "RENAME",
     f"BODY: {B('func_80022568')}: unk_26C = 1; unk_26E = unk_270 = unk_272 = 0. fighter_AccumulateOrClearByBoneGroup (17AFC.c:113-157) updates exactly these fields by bone group: bones 0-5 add to unk_272, bones 6-9 clear unk_26C (and unk_90 / unk_8A), "
     f"10-13 add to unk_270, 14-21 add to unk_26E; fighter_ResolveBoneHitTail (17AFC.c:330-343, unk_272++) and fighter_ResolveBoneHit (17AFC.c:528 unk_26C = 0, 541 unk_270++, 556 unk_26E++) do the same. CALLERS: {C('func_80022568')}: fighter_Init (9F9C.c:3773), fight_RunCommandStream and func_8003B328, all as a per-fighter state reset (void). "
     f"WHY: this exact name was the wave81 R6 suggestion after fighter_ResetBoneGroupHitAccumulators failed for omitting the unk_26C = 1 write; State covers the re-enabled gate (26C) and the three accumulators. "
     f"COUNTER-EVIDENCE: these fields also feed paired moves, motion and object interactions (9F9C.c:2562, 4402, 4510C.c:1819-1823); no uniform damage or anatomy meaning is claimed; unk_8A / unk_90 are not reset here. "
     + L("0x80022568", "the group's shared role is established by its producers (bone group hit routines) and the name makes no claim beyond them.")
     + " " + T("0x80022568")),
    ("0x80023DB8", "fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface", "RENAME",
     f"CHANGED FROM RESEARCH: fighter_TestUpwardFacingContactWithExtendedDownwardProbe ('Extended' is the callee's backward extension, not part of this body). BODY: {B('func_80023DB8')}: scratch point (0x1F8001E0) = collision_pos with vy + 5 "
     f"(positive y is down: func_80021DB0 at 9F9C.c:3461,3487,3492 starts above the point, probes down with +y and tests a -0x1000 normal); func_8005344C(collision_pos, point, 0x1F8001B0, 0x1F8001C0, 0x1F8002B8); on contact returns (normal y at 0x1F8001C2) < -0x800, else 0. "
     f"CALLER: {C('func_80023DB8')}: fighter_Update (9F9C.c:4574) under `move_kind == 0xA || unk_72 != 0`; a nonzero result enters the saved_collision_y comparison and the motion_ResolveMoveData of cur_move->event_move_table_id. RETURN USE: tested != 0. WHY: Probe = the 5-unit segment; HitsUpwardFacingSurface = the contact plus the normal threshold. "
     f"COUNTER-EVIDENCE: not exact-position groundedness (func_8005344C (43500.c:192-215) extends short segments backward, so contact may precede the point); no numeric cutoff or surface identity is named. "
     + L("0x80023DB8", "the old objection wanted the probe and slope thresholds named; the name says downward probe from the fighter's collision position and an upward-facing normal, with the numbers in the evidence, and claims no grounded state.")
     + " " + T("0x80023DB8")),
]

DATA = []

# TRACE_USED: captures cited per row come from hitmap.tsv at build time (see T()); all are reach only.
TRACE_USED = {a: "reach-only (hitmap.tsv + capture SHA1 from capture.manifest.json)" for a, *_ in FUNCS}

ADJUSTED = {
    "0x8001A67C": "math_WriteFloorDividedNormalizedOffsetXZ -> math_StoreFloorDiv2000XZOfPointStepTowards (fixed 1024 step and direction stated)",
    "0x8001B6F4": "fight_ConfigureReplayCameraAndRendering -> fight_SetViewMode1RestoreNodesClearCameraB1FAndSelectRenderConfig0 (Replay is a caller-derived role; every write named)",
    "0x8001C4C0": "fight_ConditionallyRestoreDefaultScriptBeforeOpponentReinit -> fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11 (gate literal, no Restore/Opponent role)",
    "0x8001C51C": "fight_RestoreDefaultMovesReinitOpponentAndResetObjectsViewAndReplay -> fight_RestartFighter0ReinitFighter1ClearDelayAndSelectionResetObjectsCameraReplay (Opponent guess dropped; two scalar clears added)",
    "0x8001CD68": "fight_FormatElapsedTimeMinutesSecondsHundredths -> fight_SplitFrameCounterIntoMinutesSecondsHundredths (Format/Elapsed not shown)",
    "0x8001D790": "resource_ResetGraphAndReloadChangedFightResources -> fight_ResetGraphAndReloadSelectorResourcesIfSelectionsChanged (resource_ unexplained)",
    "0x8001D904": "resource_ResetGraphLoadCopyAndRebaseVabPack -> snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVab9 (prefix explained by callees; sibling told apart by VAB number)",
    "0x8001D998": "resource_ResetGraphLoadCopyAndRebaseConcatenatedVabPacks -> snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVabs8And4",
    "0x8001DA2C": "snd_ResetVoicesQueueAndReverbAndCloseModeVabs -> snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode (VAB numbers stated)",
    "0x8001DB9C": "resource_LoadPackByFighterKindAndMarkSharedBuffer -> motion_LoadNdataPackIntoSlot1BufferAndMarkSlot1Id (resource_ unexplained; FighterKind was a flag)",
    "0x8001E800": "fight_BuildTrackedFighterViewQueueNodesAndLinkOrderingTable -> fight_QueueTrackedFighterNodesFromRecordFlagsAndInitOt (tracked-slot proof via D_800A33E0 chain)",
    "0x8001EA04": "fight_InitChoiceLoopRestoreNodeColoursAndStopEffects -> fight_RestoreNodesStopEffectsClearUnk31AAndInitThreeItemMenu (ChoiceLoop replaced by the documented three-item menu; unk_31A clears added)",
    "0x8001EA84": "fight_UpdateFighterFocusChoiceLoopAndDelayedTransitions -> fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange",
    "0x8001EEB4": "fight_StartConditionalCommonMoveAndTimedFocusPhase -> fight_StartIndexedCommonScriptOnSelectedFighterIfGatedAndEnterInterpolatedViewPhase",
    "0x8001EFA0": "fight_UpdateTimedFighterFocusAndDispatchModeProgress -> fight_UpdateInterpolatedFighterViewAndSelectNextPhaseByModeOnTimeoutOrPadPress (FighterFocus is another camera function)",
    "0x8001FB34": "fighter_PassesPairedMoveEntryStateGates -> fighter_PassesPairedMoveEntryModeOpponentUnk0CObjectAndUnk26CGates (gates listed)",
    "0x800203B4": "fighter_ArmBoneLocalVectorEvent -> fighter_StoreBoneLocalVectorAndArmEventTrigger (it emits no event; store + arm)",
    "0x800204C0": "fighter_DispatchArmedBoneVectorEventAndClearTrigger -> fighter_DispatchArmedBoneVectorEventAndDisarm",
    "0x80020CDC": "motion_InvalidateAllLoadCachesAndSharedPack -> motion_InvalidateMarkedNdataPackAndResetLoadCaches",
    "0x80020D38": "resource_ForgetSharedFighterBufferContents -> motion_InvalidateMarkedNdataPackAndClearSlot1Id",
    "0x800213A0": "motion_SkipVariantMatchingTrackedNibblePosition -> motion_AdvanceVariantIfAtNibble4Or5Position (it advances; guard values literal)",
    "0x800219E4": "motion_GetScalarOffsetCommonScript -> motion_GetCommonScriptAtFighterBufferOffset",
    "0x80021A3C": "motion_GetIndexedOffsetCommonScript -> motion_GetCommonScriptAtFighterBufferIndexedOffset",
    "0x80021DB0": "fight_SearchRadialSupportPositionOrUseStartPair -> fight_SearchRadialFlatSurfacePointOrUseStartPositionPair (Support unproved)",
    "0x80022224": "fight_SelectRandomStartPointAfterDiscardingNearestThenFarthest -> fight_PickRandomStartPositionExcludingNearestAndFarthest",
    "0x800224E0": "motion_SelectOpponentNibbleVariantByPreferredValues -> motion_SelectVariantNibblePositionMatchingPreferredPairInOpponentValue",
    "0x80023DB8": "fighter_TestUpwardFacingContactWithExtendedDownwardProbe -> fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface (Extended belongs to the callee)",
}

DROPPED = {}
