"""wave94 rows (func-second-pass-3): Q132 mechanical names for 5 of the 45 neutral functions re-researched in the
func-second-pass-3 batch: the four eff_DrawAnimatedQuads wrappers 0x80065540 / 0x80065590 / 0x800655E0 / 0x80065630
(main/51268) and 0x80071C20 (main/5ED34, the stage-index pick of the entry menu exit).
Research input: tmp/codex/func-second-pass-3-research.md (5 RENAME, 40 NO_NAME over 45 targets; untrusted proposals).
Every row was re-derived from the C bodies, eff_DrawAnimatedQuads (51268.c:2099-2520) and the callers/tables. The four
effect names are CHANGED FROM RESEARCH: the mirroring they distinguish by is conditional (view setting on and not replay),
so the research Mirroring/NonMirroring names are replaced by names that state the condition.
Research NO_NAME (40; each stays neutral, not a row):
 800650EC animated-quad record start (position copy + phase + counter reset); twins differ only by record index / phase slot.
 80065134 animated-quad record start (position copy + phase + counter reset); twins differ only by record index / phase slot.
 8006517C animated-quad record start (position copy + phase + counter reset); twins differ only by record index / phase slot.
 800651F0 animated-quad record start (position copy + phase + counter reset); twins differ only by record index / phase slot.
 80065264 animated-quad record start (position copy + phase + counter reset); twins differ only by record index / phase slot.
 800652AC animated-quad record start (position copy + phase + counter reset); twins differ only by record index / phase slot.
 80065344 eff_DrawAnimatedQuads(mode N) wrapper with +step / lifetime counter; twins differ only by mode index and constants.
 80065394 eff_DrawAnimatedQuads(mode N) wrapper with +step / lifetime counter; twins differ only by mode index and constants.
 800653E4 eff_DrawAnimatedQuads(mode N) wrapper with +step / lifetime counter; twins differ only by mode index and constants.
 80065434 eff_DrawAnimatedQuads(mode N) wrapper with +step / lifetime counter; twins differ only by mode index and constants.
 80065760 eff_DrawAnimatedQuads(mode N) wrapper with +step / lifetime counter; twins differ only by mode index and constants.
 800657B0 eff_DrawAnimatedQuads(mode N) wrapper with +step / lifetime counter; twins differ only by mode index and constants.
 80066EC0 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80066EF4 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80066F28 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80066F5C eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80066F90 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80066FC4 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80066FF8 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 8006702C eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80067060 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80067094 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 800670C8 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 800670FC eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80067130 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80067164 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 80067198 eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 800671CC eff_SpawnParticles(mode, kind, flag) + phase store; twin family separated only by argument indices.
 800676C8 eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 80067704 eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 80067740 eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 8006777C eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 800677B8 eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 800677F4 eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 80067830 eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 8006786C eff_PrepareParticles / UpdateParticles / LinkParticleQuads trio per particle mode; twin family separated only by indices.
 8006C1FC one-call forwarder of menu_UpdateMemCardChoices; callers pass fixed zeros, no role shown.
 80072084 menu_UpdateChoiceGrid(..., resource unk_68/6C/70, handler index 0/1/2) wrappers; only the index/resource slot differs.
 800720AC menu_UpdateChoiceGrid(..., resource unk_68/6C/70, handler index 0/1/2) wrappers; only the index/resource slot differs.
 800720D4 menu_UpdateChoiceGrid(..., resource unk_68/6C/70, handler index 0/1/2) wrappers; only the index/resource slot differs.
(DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 5/5 (2026-10-28, naming-a)"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x80065540": ("eff_DrawAndAdvanceMirroringViewSettingQuadPair", "eff_DrawAndAdvanceQuadPairWithFirstMirroredByViewSettingOutsideReplay",
                   "Mirroring is conditional (g_working_record.view_setting && g_main_loop_handler_index != 5), and only the first quad of the pair mirrors; the name states both"),
    "0x80065590": ("eff_DrawAndAdvanceNonMirroringViewSettingQuadPair", "eff_DrawAndAdvanceQuadPairNotMirroredByViewSetting",
                   "same contrast, stated as what the code shows: modes 7/9 never negate the width"),
    "0x800655E0": ("eff_DrawAndAdvanceMirroringViewSettingQuad", "eff_DrawAndAdvanceQuadMirroredByViewSettingOutsideReplay",
                   "Mirroring is conditional on view_setting and not replay; the name states the condition"),
    "0x80065630": ("eff_DrawAndAdvanceNonMirroringViewSettingQuad", "eff_DrawAndAdvanceQuadNotMirroredByViewSetting",
                   "same contrast, stated as what the code shows: mode 11 never negates the width"),
    "0x80071C20": ("menu_SelectStageIndexFromFirstSlotEntryFlag", "menu_SelectStageIndexFromFirstSlotEntryFlag", ""),
}
DROPPED = {}

for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new:
        NOTES[_a] = f"adjusted from research {_old}: {_why}. Pending naming-reviewer pre-apply (round 1)."

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


def R(addr, new, fn, f, body, callers, why, counter, prior):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {T(addr)}")
    return (addr, new, "RENAME", ev)


FUNCS = [
    R("0x80065540", "eff_DrawAndAdvanceQuadPairWithFirstMirroredByViewSettingOutsideReplay", "func_80065540", "51268.c",
      "draw_result = eff_DrawAnimatedQuads(6); g_eff_animated_quad_counters[6] += 0x32; returns draw_result while the "
      "s16 counter is below 0x100, else 0. Inside eff_DrawAnimatedQuads (51268.c:2099) mode 6 draws its quad and then "
      "re-runs the body as mode 8 (mode_index += 2, goto again, :2500-2506), so a second quad is drawn; modes 6/7 read "
      "g_working_record.view_setting for texture record, base size and tpage (:2218-2247); in the second switch "
      "case 6 (with case 10) negates the projected width p_w when g_working_record.view_setting && "
      "g_main_loop_handler_index != 5 (:2449-2453), then falls into case 7, whose view_setting corner code sets x0 = "
      "screen_x - w and x1 = w + screen_x (:2473-2475), so the negated width swaps the two edges (horizontal mirror)",
      "only the effect handler-table word d_8BC50.c:296 (declaration :232); no direct call",
      "the verb covers the whole effect: one drawer call, the counter advance by 0x32 and the 0x100 lifetime gate of the "
      "result. The twin 80065590 (mode 7, pair 9) has the same body with another index; the only code-shown difference is "
      "the case-6 negation, which acts only when the view setting is on and the main loop is not replay_PlayFrame "
      "(g_main_loop_handler_index 5: g_main_loop_handlers[5] = replay_PlayFrame, d_7D870.c:31-35; replay_Start sets 5, "
      "28708.c:1308). The name states that condition (ByViewSetting OutsideReplay) and that only the first quad of the "
      "pair mirrors (the mode-8 partner, case 8/9 :2398-2399, never negates); 'QuadPair' separates it from the single-quad "
      "80065630 / 800655E0 (mode 10 draws once: mode_index < 9 fails at :2500)",
      "with the view setting off, or during replay, modes 6 and 7 draw identical geometry, so 'mirrored' holds only under "
      "the stated condition (and the name says so); no visual identity of the quad is claimed, nor what the view setting "
      "is in play terms. The research Mirroring/NonMirroring ViewSetting names read as unconditional mirroring and as "
      "'a quad of the view setting', so they are replaced",
      "no_name.tsv w58/w72/w77/w83 rejected a universal mirrored-direction name; this one makes the conditional gate the claim"),
    R("0x80065590", "eff_DrawAndAdvanceQuadPairNotMirroredByViewSetting", "func_80065590", "51268.c",
      "draw_result = eff_DrawAnimatedQuads(7); g_eff_animated_quad_counters[7] += 0x32; returns draw_result while the "
      "s16 counter is below 0x100, else 0. Mode 7 re-runs as mode 9 (:2500-2506), so two quads are drawn; mode 7 enters "
      "the second switch at case 7 (:2454), after the case-6/10 negation, so p_w is never negated",
      "only the effect handler-table word d_8BC50.c:298 (declaration :234)",
      "same whole effect as 80065540 (one drawer call, counter += 0x32, 0x100 lifetime gate, two quads); the difference "
      "from it is the skipped negation, so under the view setting outside replay it draws the unmirrored version; "
      "'QuadPair' separates it from the single-quad 80065630",
      "without the view setting the geometry equals the twin's, so 'NotMirrored' is true everywhere but is only a contrast "
      "under the stated condition (ByViewSetting); modes 6/7 are the only pair that reads view_setting for the texture "
      "(10/11 are single)",
      "no_name.tsv w58/w72/w77/w81/w83 twin objection, answered by the case-6 negation that the code shows"),
    R("0x800655E0", "eff_DrawAndAdvanceQuadMirroredByViewSettingOutsideReplay", "func_800655E0", "51268.c",
      "draw_result = eff_DrawAnimatedQuads(0xA); g_eff_animated_quad_counters[10] += 0x32; returns draw_result while the "
      "s16 counter is below 0x100, else 0. Mode 10 draws once (the re-run needs mode_index < 9, :2500); case 10 negates "
      "p_w under g_working_record.view_setting && g_main_loop_handler_index != 5 (:2449-2453) and falls into case 11's "
      "corner code (:2454-2475), so the single quad is mirrored under that condition",
      "only the effect handler-table word d_8BC50.c:300 (declaration :236)",
      "whole effect: one drawer call, counter advance, lifetime gate; the contrast with 80065630 (mode 11, no negation) "
      "is the case-10 negation, stated with its condition; no Pair because nothing re-runs",
      "when the view setting is off or in replay it draws the same geometry as 80065630, so the mirroring is conditional "
      "(the name states it); the colour arm case 10/11 (:2200-2217) is shared with the twin",
      "no_name.tsv w58/w72/w77/w83 conditional-configuration objection: the gate globals are named since w83 and are in the name"),
    R("0x80065630", "eff_DrawAndAdvanceQuadNotMirroredByViewSetting", "func_80065630", "51268.c",
      "draw_result = eff_DrawAnimatedQuads(0xB); g_eff_animated_quad_counters[11] += 0x32; returns draw_result while the "
      "s16 counter is below 0x100, else 0. Mode 11 draws once (:2500) and enters the second switch at case 11 (:2455), "
      "after the case-10 negation, so p_w is never negated",
      "only the effect handler-table word d_8BC50.c:302 (declaration :238)",
      "same whole effect as 800655E0 with the case-10 negation skipped; single quad, so no Pair",
      "equal geometry to 800655E0 without the view setting / in replay, so 'NotMirrored' contrasts only under the "
      "stated condition",
      "no_name.tsv w58/w72/w77/w83 twin objection, answered by the case-10 negation that the code shows"),
    R("0x80071C20", "menu_SelectStageIndexFromFirstSlotEntryFlag", "func_80071C20", "5ED34.c",
      "stage_index = 3; if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[0].cursor_entry_value] & 2) "
      "stage_index = 9; return stage_index; no writes, no calls (asm/funcs/func_80071C20.s: one lbu of "
      "g_entry_menu_entry_flags[...], andi 2, li 9/3)",
      "5ED34.c:1685 only (g_entry_menu_working_record_ptr->stage_index = func_80071C20(), when g_entry_menu_fight_mode "
      "< 2, in DrawRotatedEntriesAndOnFullCounterFinishWithResultOrNextStateAndStoreSelections; asm jal :195); listed "
      "as a reader at d_8C41C.c:24",
      "the result is stored into the settings record's stage_index (game.h:553, the same member menu_UpdateChoiceGrid "
      "stores its chosen stage into, 5ED34.c:1997-2000); the choice is one of two constants driven by bit 2 of the "
      "entry flag (g_entry_menu_entry_flags is a static table, d_8C41C.c:91, never written with bit 2; bit 2 is the "
      "value-2 test other readers use to pick the sprite header, 5ED34.c:1555-1570, 1628-1645) of the first slot's "
      "cursor entry (rec[0].cursor_entry_value); 'Select' matches the two-way pick, 'FromFirstSlotEntryFlag' names the "
      "input exactly; no claim on what bit 2 or stages 3/9 are",
      "the bit's meaning and the stage identities are unknown (so none is claimed); rec[0] is slot 0 only, which the "
      "name says (FirstSlot)",
      "no_name.tsv w56/w74/w89 result-contract objection (field and table had no names); stage_index and "
      "g_entry_menu_entry_flags / g_entry_menu_slot_records are named now"),
]

DATA = []
