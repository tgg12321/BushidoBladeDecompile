"""wave92 rows (func-second-pass): Q132 mechanical names for 3 of the 24 neutral functions re-researched in the
func-second-pass batch 1: 0x8001EA04 (main/9F9C, the Init handler of the fighter-focus-view menu phase), 0x8002798C
(main/17AFC, the move-frame-window predicate) and 0x8003DDF8 (main/2B344, the one-store OT link writer).
Research input: tmp/codex/func-second-pass-research.md (3 RENAME, 21 NO_NAME over 24 targets; untrusted proposals).
Every row was re-derived from the C body and every caller (src/main/*.c, include/, asm/funcs/*.s, asm/data,
named_syms.txt, symbol_addrs.txt); names changed against the research say CHANGED FROM RESEARCH in the evidence and in the
row note (1 of the 3 differs: 8003DDF8; 2 are kept as researched).
Research NO_NAME (21; each stays neutral, not a row):
 80016D78 whole-game startup aggregate; its words fit sys_Init equally and no separating noun exists.
 80017F90 empty jr-ra hook identical to func_800168F0 / func_80017F98; a caller-role name describes the caller.
 8001B6F4 its distinctive callee func_8003F1E4 is itself nameless (D_800F6658 shift word), so no complete name.
 8001C51C eleven effects across both fighters, the pool, the camera and the replay; no covering verb.
 8001EEB4 gated common-script start plus entry->unk_26C = 1; an Init<phase> name would hide the unk_26C write.
 8002DE20 edge tests accept same-sign cross products, so an intersection name claims an unshown guarantee.
 8003B534 indistinguishable from twin 8003B56C without roles for the two record-pointer globals.
 8003B56C twin of 8003B534; same objection.
 8003E2AC uncalled clear of a bit of g_gpu_grid_and_fog_control that no code reads; no role to name.
 8003F1E4 writes the grid/fog word and D_800F6658 (unnamed srav shift count); a covering name invents a role.
 8003F218 switches D_800A322C and mirrors it into D_800F665C, both without a role.
 8003F268 getter of D_800A322C, whose shared role is unnamed.
 8003F5CC calls the empty func_80017F98 three times; no observable effect to name.
 80040D48 seven-part per-frame slot-object aggregate (also R6-failed in wave90); no single covering verb.
 800430E4 swaps the depth shift, halves a view-matrix row, picks a draw table and calls a nameless callee; omission.
 800444BC pure gpu_PrepareViewAndFlushDrawQueue(g_gpu_ot_offset_ptr) forwarder, identical to 800444E0; restates callee.
 800444E0 instruction-identical twin of 800444BC.
 80046780 uncalled getter of D_800A33B0, whose section role is unobserved.
 80046934 fixed-selector arena_OpenBlock(9) over an unidentified block.
 80046A80 arena move-callback adaptor forwarding a1 to motion_RebasePlayback; a name restates the callee.
 80046B44 broad startup aggregate (nine subsystem inits plus nine bare resets); no prefix spans it.
(DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (round 1, 3/3, 2026-10-28)"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8001EA04": ("fight_InitFighterFocusViewAndThreeItemMenu", "fight_InitFighterFocusViewAndThreeItemMenu", ""),
    "0x8002798C": ("fighter_TestMoveFrameWindowOrObjectVariantKind", "fighter_TestMoveFrameWindowOrObjectVariantKind", ""),
    "0x8003DDF8": ("gpu_SetLastOtEntryLinkToPacket", "gpu_SetLastDepthIndexOtEntryLinkToPacket",
                   "'Last OT entry' is false: the table is cleared with 0x1008 entries (6CF8.c:452) and 25C38.c:81 / 6CF8.c:369 "
                   "use entry 0x1007 as the real last; the entry written here is offset-table index 0xFFF, the highest index "
                   "of the depth range (the only caller guards gpu_DepthToOtIndex results with idx < 0x1000, 2B344.c:1550). "
                   "'LastDepthIndex' states that without a number"),
}
DROPPED = {}

for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new:
        NOTES[_a] = f"adjusted from research {_old}: {_why}. Pending naming-reviewer pre-apply (round 1)."

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")

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


def R(addr, new, fn, f, body, callers, why, counter, prior):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {T(addr)}")
    return (addr, new, "RENAME", ev)


FUNCS = [
    R("0x8001EA04", "fight_InitFighterFocusViewAndThreeItemMenu", "func_8001EA04", "9F9C.c",
      "node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0) and (1, 0); eff_StopCommands(); "
      "D_80101EC8[1].pose_blend_requests = D_80101EC8[0].pose_blend_requests = 0; g_handler_frame_counter = "
      "g_fight_choice_confirm_delay = 0; g_main_loop_handler_index = 0xD; g_fight_choice_menu_first_cursor = "
      "g_fight_choice_menu_cursor = (D_800A38D4 < 1)",
      "only the table word at d_7D870.c:42 (D_8008D090 index 0xC), reached when g_main_loop_handler_index is set to 0xC "
      "(28708.c:1550, :1567, :1575); no C call, no jal",
      "the Init half of the handler pair whose Update, at the next table word and the index this body installs (0xD), is "
      "fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange (9F9C.c:2188), which reads exactly the state "
      "reset here (frame counter, confirm delay, cursor, first cursor) and cycles the cursor over three items "
      "(9F9C.c:2223-2236); the sibling Init handlers in the table (fight_InitRotatingViewThreeOptionChoice, "
      "fight_InitRotatingViewDecimalPresentation, fight_InitNearestStartPairPlayback, fight_InitImageAndPlaybackPresentation) "
      "likewise absorb the node colour reset, eff_StopCommands and counter / cursor resets under fight_Init<phase>",
      "the body itself touches no view and no menu drawing: 'FocusView' and 'ThreeItemMenu' are the paired Update's words, "
      "so the name states which phase is initialised, not what the body draws; the pose_blend_requests clear is the one write "
      "no sibling Init has and is absorbed by 'Init'; D_800A38D4 (the first-cursor floor) is unnamed",
      "w84 N4/R4 FAIL (RestoreNodes guessed, Unk31A placeholder in the name) answered: neither is used"),
    R("0x8002798C", "fighter_TestMoveFrameWindowOrObjectVariantKind", "func_8002798C", "17AFC.c",
      "ret = 0; if move_kind is 6 / 0x25 / 0x33 / 4 / 0x14, or move_frame < frame_window_start, or frame_window_end < move_frame: "
      "ret = 1 only when object_variant_index != -1 and move_kind is one of 0xF, 0x1C..0x21, 0x2B; otherwise ret = 1; "
      "reads only the fighter record argument, no writes, no calls",
      "17AFC.c:331 (fighter_TryDropQueuedHeadAtBone, drops the queued object at bone 19 and clears object_variant_enabled), "
      "9F9C.c:5162 (fighter_Update, sets wielded_object_live_state and frame_flags bit 4); decl include/bb2.h:730; "
      "asm/funcs/fighter_TryDropQueuedHeadAtBone.s:15 and fighter_Update.s:2144 jal",
      "a predicate name for the tested condition: the move frame inside the per-move frame window (for kinds not excluded), or "
      "otherwise one of the move kinds accepted when an object variant exists; it states the test, not either caller's "
      "consequence, so it holds at both call sites that act differently on a true result; fighter_ is the record argument "
      "(Unk80101EC8Record) and both callers",
      "the name does not list the excluded kinds (the window result is suppressed for them) nor the object_variant_index != -1 "
      "precondition of the kind branch; a reader finds both in the body; the kind lists stay unnamed (numbers banned)",
      "w52 / w63 / w71 / w89 objection (callers differ in effect) answered: no consequence is claimed"),
    R("0x8003DDF8", "gpu_SetLastDepthIndexOtEntryLinkToPacket", "func_8003DDF8", "2B344.c",
      "u32 *ptr = g_gpu_ot_offset_ptr; arg0 &= 0xFFFFFF; ptr[0x3FFC / 4] = arg0 (one word store, offset-table index 0xFFF)",
      "2B344.c:1561 (gpu_QueueDepthSpacedMovesAndLinkTerminalPacket, passes its packet cursor (u32)pkt after the loop when "
      "arg4 != 0, else addPrim(&g_gpu_ot_offset_ptr[0xFFB], pkt)); decl include/bb2.h:825; "
      "asm/funcs/gpu_QueueDepthSpacedMovesAndLinkTerminalPacket.s:117 jal",
      "the body replaces the 24-bit link word of the offset-table entry at the highest depth index (the caller admits only "
      "gpu_DepthToOtIndex results idx < 0x1000) with a packet address, the setaddr link format; Set...Link (not Add) says it "
      "overwrites rather than inserts; the other writers of that index (368E4.c:1735 / :1740, 309CC.c:47) use addPrim, so "
      "this is the only overwrite",
      "the argument is u32 and only its low 24 bits are stored; the one caller passes a packet address, so 'Packet' holds at "
      "its only use; 'Last' of the research was dropped because the cleared table (0x1008 entries) extends past this index",
      "none (never researched as a ledger row)"),
]

DATA = []
