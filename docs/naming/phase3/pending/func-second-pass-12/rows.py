"""wave102 rows (func-second-pass-12): Q132 stage-word upgrade of ONE name, node_InitSelector18FrameEntriesFromWorldTranslations
(0x8004777C) -> node_InitLumberyardFrameEntriesFromWorldTranslations. It is the func-second-pass-10 draft's name (dropped
only on the role word: the matrix then marked all 33 stage:lumberyard pair rows inconclusive by hit_in_untagged_capture);
trace-story-stage-tags (landed) stage-tagged the story capture, so at HEAD da94f6084 only_x.tsv has 34 pair rows for the
address, 22 conclusive, every one of the 17 other covered stages with >= 1 conclusive zero row, no hit outside
stage:lumberyard. Research input: tmp/codex/func-second-pass-12-research.md (1 RENAME, 3 KEEP; untrusted; every use
re-derived at HEAD, claims re-read in only_x.tsv / facts.tsv / screens.tsv / phase_tags.tsv). The Advance twin 0x8004779C
(0/34 pair rows conclusive) and the selector-14 pair (one inconclusive untagged row each) are KEEP, not rows.
(DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply 2026-10-31 (naming-b, HEAD da94f6084): PASS 1/1. Row 18 runs only while "
                 "ndata file 0x12 + 0x27 is the loaded block, by either loader (35000.c:238); hit map 5 hits all stage:lumberyard, "
                 "94 armed zeros, 0 unknown, every other stage excluded by a conclusive zero (only_x.tsv:2846-2879); stage card "
                 "capture.sel12.93500.png reads Lumberyard. Advance twin keeps Selector18 (its hit set not yet shown).")
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8004777C": ("node_InitLumberyardFrameEntriesFromWorldTranslations", "node_InitLumberyardFrameEntriesFromWorldTranslations", ""),
}
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


def R(addr, new, fn, body, callers, why, counter, prior, trace):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"BODY: {B(fn, '368E4.c')}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {trace}")
    return (addr, new, "RENAME", ev)


_TABLE = ("g_resource_selector_callbacks (src/main/d_80DF8.c:873, 38 entries, row 0 at :874; each row is {init, unk4}), "
          "indexed by resource_GetLoadedSelector(); the init slot is called only by resource_InvokeLoadedSelectorInitCallback "
          "(src/main/2B344.c:2292-2296), which runs on both the reuse path (src/main/368E4.c:70, selector == "
          "g_loaded_resource_selector) and the fresh-load path (:181, after g_loaded_resource_selector is set at :77) of "
          "resource_LoadOrReuseSelectedBlock; the unk4 slot is called only by "
          "gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards (src/main/2B344.c:2081-2082)")

_ONLYX = ("Q132 matrix, re-counted at HEAD (docs/naming/runtime/evidence/only_x.tsv:2846-2879): 34 pair rows for 0x8004777C, "
          "all tag stage:lumberyard / axis stage, 22 conclusive (inconclusive column 0, reason {}), 12 inconclusive "
          "(7 zero_side_shorter [stage_select]/[t_load] vs stage-map1-4; bamboo e7ab3e7ff61c inputs_differ_after_divergence + "
          "start_state_differs; railway b1a72c39296d start_state_differs; 3 kimigaura phase_lists_differ). All 17 other covered "
          "stages (abandoned_station, abandoned_tunnel, bamboo_thicket, building_parking_lot, building_rooftop, castle_rooftop, "
          "ferry_dock, future_dump_site, kimigaura_coast, meikyokan_dojo, purple_cavern, railway, shainto_cemetery, "
          "shainto_gate, snake_cliff, terraced_fields, under_railway) have at least one conclusive zero row (zero-set ids "
          "742261fc9f5d, f43900378140, 465939bbb687, 054eb7418a79, c3a025edfdc8, 852686345b20, 510c00cf7b72, 473a73204432, "
          "c02481d3a4bf, 8eeec82bcaed, 500edd2f4a13, f7fcc536b42a, 39bf9bdcb584, afe663c6de5a, 976d88e1c76d, 50e6e2835ff3, "
          "3c70919fe0f1); no row has hit_in_untagged_capture any more (matrix/refines.tsv:2 lib-story-final-fight-retail-1 -> "
          "story-final-fight-stages-1 ok; matrix/phase_tags.tsv:2 sel12 = stage:lumberyard). The hit side is 5 captures "
          "(matrix/facts.tsv:2071 reach.hit: lib-story-final-fight-retail-1 [story], story-final-fight-stages-1 [sel12], "
          "versus-2p-pair-lumberyard-1/-2 [t_load], versus-2p-stage-map1-4 [t_load]); the versus ones are stage:lumberyard "
          "scenarios (matrix/scenarios.tsv:81,82,108); evidence/screens.tsv:55637-55642 records the six hit rows")

FUNCS = [
    R("0x8004777C", "node_InitLumberyardFrameEntriesFromWorldTranslations", "node_InitSelector18FrameEntriesFromWorldTranslations",
      "node_InitFrameEntriesFromWorldTranslations(); nothing else. Instruction-identical to its selector-4 / selector-7 "
      "twins (asm/funcs/node_InitSelector18FrameEntriesFromWorldTranslations.s: frame, one jal, return); the code tells them "
      "apart only by the table row. Callee (src/main/32D04.c:1192-1219): paired_grid_nodes = gpu_GetGridNodeRecordTable(); "
      "for each of frame_entry_count entries a fresh root node, work.t = the paired grid node's world translation, "
      "unk58 = -1 (not started); the count is set from the same selector (32D04.c:1177-1186: 7 -> 0x11, 4 -> 2, 0x12 -> 0xA)",
      "none by name beyond the pair: the current name node_InitSelector18FrameEntriesFromWorldTranslations (wave101) occurs "
      "only at the C definition src/main/368E4.c:751, the declaration src/main/d_80DF8.c:40, the one address-taken use, the "
      ".init slot of row 18 = 0x12, src/main/d_80DF8.c:892 ({init, node_AdvanceSelector18...}), the asm glabel/endlabel "
      "asm/funcs/node_InitSelector18FrameEntriesFromWorldTranslations.s:1,10 (jal node_InitFrameEntriesFromWorldTranslations "
      "at :4) and the census docs/naming/function-names.csv:599 (CORROBORATED); no jal, no call, no comment in src/ or "
      "include/, no symbol_addrs.txt / undefined_syms_auto.txt line; named_syms.txt:281 is a RESET note naming only the "
      "retired game_EffStart_8004777C; the old func_ name survives only in history/provenance (tools/rename_funcs.py:60, "
      "docs/naming/apiscan/evidence.json:8801, no_name.tsv:40) and recorded runtime data (hazard in notes.txt). Table and "
      "call path: " + _TABLE + ". Every static site is within X: the only site is row 18, read only while selector 0x12 "
      "is the loaded selector, and the matrix shows that selector loaded only on the Lumberyard",
      "node_ and InitFrameEntriesFromWorldTranslations = unchanged from the landed name (callee wording, N1, node_ "
      "neighbour precedent d_80DF8.c:887); Selector18 -> Lumberyard = the Q132 role word: the stage that loads selector 0x12 "
      "(18). Evidence for the role: the only-X matrix above (a hit in Lumberyard captures, none outside, conclusive zero rows "
      "for all 17 other stages); the stage card 'Lumberyard' at the sel12 load (story-final-fight-stages-1 sel12.93500, "
      "frame 93500, 64 frames after the init hit at 93436); and the code (the one site is the selector-0x12 row). The name "
      "claims no posts: 10 entries are set up, only entries 8 and 9 were observed started",
      "(1) " + _ONLYX + "; (2) the name loses the code key 0x12 = 18 (368E4.c:56-58, :148, :272-279 case 18; 32D04.c:1184; "
      "35000.c:67): it is the first stage word among the landed selector names, spelling matching the stage:lumberyard tag; "
      "the table row stays discoverable from d_80DF8.c:892 and the Advance twin keeps Selector18; (3) PAIR SPLIT: the Advance "
      "twin node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws (d_80DF8.c:892-893) stays KEEP because its 34 pair rows "
      "are all inconclusive, so one table row now reads Lumberyard / Selector18; (4) 12 inconclusive rows remain for this "
      "address: the Q132 test is a hit in X and none outside, which every other stage's conclusive row settles, and "
      "diff.py treats pair rows as a whole-set judgment; (5) libscan docs/naming/libscan/matches.json:809 and "
      "psyq_versions.txt:99 list it among 8-word LIBAPI/SEND 'verbatim' shape matches (size-only, jal target masked; never "
      "evidence, N3); (6) the second loader caller motion_StartPlayback loads with its own selector "
      "((field & 0x3F) - 1), so the code alone does not confine selector 0x12 to a stage; the confinement is the matrix",
      "func-second-pass-10 (the same name; pre-apply mechanics PASS, FAIL only on the role word / matrix flag "
      "hit_in_untagged_capture); func-second-pass-11 / wave101 gave the current Selector18 name; no_name.tsv:40 (twin "
      "objection); retired RESET name game_EffStart (wave03)",
      "TRACE (Q124/Q132): docs/naming/runtime/frame-anim-story-lumberyard-cut-1 (e98ae53e5): one call at frame 93436 from "
      "resource_InvokeLoadedSelectorInitCallback (ra 0x8003F1B8; evidence/events.tsv:15, screens.tsv:55637, stage card "
      "story.93500); selector-loads-story-final-fight-retail-1 (events.tsv:482, same hit at 93436, no other init). Entries 8 "
      "and 9 are started later (97709 / 97953, per the research's events.tsv rows, not re-shown by this draft)."),
]

DATA = []
