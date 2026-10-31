"""wave99 rows (func-second-pass-8): Q132 mechanical names for the 4 functions revived in the
func-second-pass-8 batch (trace-unnamed cluster-1 captures, 179a0f92b): the yawed-slot-quad command
handlers of main/51268, start 0x80063AF0 / 0x80063B34 and draw 0x80063B78 / 0x80063BA4. They are TWO TWIN PAIRS
(lane 0 and lane 1) and the four rows stand or fall together: the twins are told apart only by the lane, and the
lane is fighter_record->index, shown by code (the only issuer, fighter_DispatchFrameEvent frame events 0/1 in
17AFC.c, passes fighter_record->index to eff_StartYawedQuadSlotCommand; fighter_Init, 9F9C.c, sets
D_80101EC8[k].index = k), corroborated (Q124, never the basis) by the 179a0f92b captures. A reviewer FAIL on any
one row sends all four back to blocked.
Research input: tmp/codex/func-second-pass-8-research.md (4 RENAME, 0 NO_NAME over 4 targets; untrusted proposals).
Every use of the four functions was re-derived at 7ad9f4409 from the C bodies, the handler table
(d_8BC50.c:200-203, :269-272), eff_StartCommand / eff_StepCommand, every command-word writer in 51268.c,
eff_StartYawedQuadSlotCommand and its two callers, the asm jal sites and fighter_Init with its four callers.
No name changed; 0 dropped.
(DATA is empty.)
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 4/4 (2026-10-30, naming-b); two twin pairs, lane = fighter_record->index shown by code"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x80063AF0": ("eff_SpawnFighter0YawedSlotQuad", "eff_SpawnFighter0YawedSlotQuad", ""),
    "0x80063B34": ("eff_SpawnFighter1YawedSlotQuad", "eff_SpawnFighter1YawedSlotQuad", ""),
    "0x80063B78": ("eff_DrawFighter0YawedSlotQuads", "eff_DrawFighter0YawedSlotQuads", ""),
    "0x80063BA4": ("eff_DrawFighter1YawedSlotQuads", "eff_DrawFighter1YawedSlotQuads", ""),
}
DROPPED = {}

for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new:
        NOTES[_a] = f"adjusted from research {_old}: {_why}."

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

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
    """Reach clause from coverage.tsv (verified at build time); the lane rests on code, the captures corroborate."""
    h = _COV[addr.lower()]
    return (f"TRACE: corroboration only (Q124); coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation "
            f"(captures docs/naming/runtime/eff-yawed-versus-2p-kimigaura-effects-1 and versus-2p-kimigaura-eff-stub-1, 179a0f92b).")


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


_ISSUER = ("eff_StartYawedQuadSlotCommand (51268.c:543-561) is the only writer of command index 0 / 1 "
           "(use_second_slot == 0: status_dest &g_eff_command_status[0], command_word bits + 0x10000; else "
           "&g_eff_command_status[1], bits + 0x10001); every other start function writes a literal word with index 2..0x1A "
           "and func_80061064 (51268.c:272-280) only stores the index of an already-live status for stepping. Its only "
           "callers are 17AFC.c:5210-5215 and :5239-5244 (fighter_DispatchFrameEvent frame events 0 and 1, "
           "g_fight_stage_index == 11; asm fighter_DispatchFrameEvent.s:153 / :222), both passing fighter_record->index as "
           "use_second_slot; fighter_Init (9F9C.c:3895-3909) sets D_80101EC8[k].index = k from fighter_index (callers "
           "9F9C.c:1255 / 1259 pass 1, 9F9C.c:1906 passes the loop index i, 2B344.c:169 passes fighter_index)")

FUNCS = [
    R("0x80063AF0", "eff_SpawnFighter0YawedSlotQuad", "func_80063AF0", "51268.c",
      "g_eff_command_phase[0] = 1 (arms the draw entry); g_eff_yawed_quad_tex_selectors[0] = command_word.w bits 17-18 of "
      "g_eff_active_command_block; return eff_AllocateOrRecycleYawedQuadSlot(0) (always 1: lowest free of 10 lane-0 slots or "
      "recycle, storing the yaw and the position). " + _ISSUER + ". So command 0 is started only for the record with "
      "index 0 (fighter 0), at both frame events",
      "one, address-taken: d_8BC50.c:269 g_eff_command_handlers[0] = command 0 phase-0 entry (g_eff_command_handler_bases[0] "
      "= 0x00, d_8BC50.c:200-203), prototype d_8BC50.c:205; called only through the jalr in eff_StartCommand (51268.c:105-109, "
      "asm eff_StartCommand.s:49), which zeroes g_eff_command_phase[idx] first (:89). The result goes to "
      "g_eff_command_status[0] via status_dest (:110). Comment mention include/game.h:994. Not in symbol_addrs.txt or "
      "named_syms.txt",
      "Spawn = the sibling start-handler verb (eff_SpawnBillboardPair, eff_SpawnJitteredBillboards, same handler table); "
      "Fighter0 = the lane is fighter 0's at the only issuer, by code (accepted convention fight_SaveFighter0..., "
      "fight_ConfigureFighter1...); YawedSlotQuad = the accepted nouns of eff_AllocateOrRecycleYawedQuadSlot / "
      "eff_DrawYawedSlotQuads; eff_ because callees and data are the eff_ command system",
      "Fighter0 is a claim about the single issuer, not the body (the body only uses lane 0); it holds because there is "
      "exactly one issuer (two arms) and no other path stores command index 0. Not claimed: footprint, sand, stage 11, "
      "P1 (fighter 0 is P1 only in the captured versus mode), the 10-slot recycling, the texture selector. Pair rule: "
      "stands or falls with 0x80063B34, 0x80063B78, 0x80063BA4",
      "no_name.tsv:62 (w58/w72/w77/w81/w83/w93: lane meaning not shown); revive evidence is code (the issuer's fighter_record->index), "
      "corroborated by 179a0f92b: 35 eff_StartYawedQuadSlotCommand hits with a3 0 each followed by func_80063AF0, and the "
      "knockout removes only left-side ground marks in frames 8730-10930 (later divergence is game state, not claimed)"),
    R("0x80063B34", "eff_SpawnFighter1YawedSlotQuad", "func_80063B34", "51268.c",
      "g_eff_command_phase[1] = 1; g_eff_yawed_quad_tex_selectors[1] = command_word.w bits 17-18; return "
      "eff_AllocateOrRecycleYawedQuadSlot(1) (always 1). " + _ISSUER + ". So command 1 is started only for the record with "
      "index 1 (fighter 1)",
      "one, address-taken: d_8BC50.c:271 g_eff_command_handlers[2] = command 1 phase-0 entry (g_eff_command_handler_bases[1] "
      "= 0x02, d_8BC50.c:201), prototype d_8BC50.c:207; called only via the eff_StartCommand jalr (51268.c:105-109, "
      "asm eff_StartCommand.s:49). No literal start word has index 1. Comment mention include/game.h:994. Not in "
      "symbol_addrs.txt or named_syms.txt",
      "As 0x80063AF0, lane 1 = fighter 1 by code at the only issuer",
      "As 0x80063AF0 (no footprint / stage / P2 claim). Pair rule: stands or falls with the other three",
      "no_name.tsv:63; revive evidence as 0x80063AF0: 14 hits with a3 1 each followed by func_80063B34, knockout marks on the "
      "right side in frames 8800-9827"),
    R("0x80063B78", "eff_DrawFighter0YawedSlotQuads", "func_80063B78", "51268.c",
      "*g_eff_yawed_quad_tex_index = g_eff_yawed_quad_tex_selectors[0] (the selector stored by func_80063AF0); return "
      "eff_DrawYawedSlotQuads(0) (one textured POLY_FT4 per live lane-0 slot at its stored position and yaw; returns 1, so "
      "status[0] stays set). Lane 0's slots are filled only by eff_AllocateOrRecycleYawedQuadSlot(0), called only from "
      "func_80063AF0, i.e. only on fighter 0's starts (issuer chain as in 0x80063AF0: 17AFC.c:5210-5215 / :5239-5244, "
      "use_second_slot = fighter_record->index, D_80101EC8[k].index = k at 9F9C.c:3909)",
      "one, address-taken: d_8BC50.c:270 g_eff_command_handlers[1] = command 0 phase-1 entry (base 0 + phase 1), prototype "
      "d_8BC50.c:206; reached only by eff_StepCommand (51268.c:137-141, asm eff_StepCommand.s:53) from func_80061064's "
      "per-frame loop (:272-280) while g_eff_command_status[0] != 0; eff_StartCommand zeroes the phase first (:89), so it never "
      "runs this entry. g_eff_command_phase[0] is set to 1 only by func_80063AF0 (:1343) and cleared only by "
      "eff_StartCommand / eff_ResetCommands. Reads the same g_eff_yawed_quad_tex_selectors the var-globals-f names fixed",
      "Draw = verb of the accepted callee name eff_DrawYawedSlotQuads narrowed to the lane; YawedSlotQuads as in "
      "0x80063AF0; Fighter0 because the only lane-0 slots are the ones func_80063AF0 filled for fighter 0",
      "the name claims no texture set and no footprint; not breakpointed itself (coverage.tsv:843 reach: fight / t_ko, 3 captures); "
      "the func_80063AF0 knockout (command never stepped, so this handler never runs) only corroborates the lane. Pair rule: "
      "stands or falls with the other three",
      "no_name.tsv:64 (lane twin of func_80063BA4); revive evidence is the handler-table position and the lane-0 fill chain"),
    R("0x80063BA4", "eff_DrawFighter1YawedSlotQuads", "func_80063BA4", "51268.c",
      "*g_eff_yawed_quad_tex_index = g_eff_yawed_quad_tex_selectors[1]; return eff_DrawYawedSlotQuads(1) (returns 1). Lane 1's "
      "slots are filled only by eff_AllocateOrRecycleYawedQuadSlot(1), called only from func_80063B34, i.e. only on fighter 1's "
      "starts (17AFC.c:5210-5215 / :5239-5244, use_second_slot = fighter_record->index, D_80101EC8[k].index = k at 9F9C.c:3909)",
      "one, address-taken: d_8BC50.c:272 g_eff_command_handlers[3] = command 1 phase-1 entry (base 2 + phase 1), prototype "
      "d_8BC50.c:208; reached only by eff_StepCommand (51268.c:137-141, asm .s:53) while g_eff_command_status[1] != 0; "
      "g_eff_command_phase[1] = 1 only from func_80063B34 (:1350)",
      "As 0x80063B78: lane 1 = fighter 1",
      "As 0x80063B78; coverage.tsv:844 reach: fight, 1 capture. Pair rule: stands or falls with the other three",
      "no_name.tsv:65; revive evidence as 0x80063B78"),
]

DATA = []
