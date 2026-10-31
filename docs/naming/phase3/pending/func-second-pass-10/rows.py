"""wave101 rows (func-second-pass-10): Q132 role-word names for the 2 selector-0x12 stage-callback pair members
revived by trace-unnamed's cluster-3 capture e98ae53e5 (Lumberyard cuttable posts: frame-anim-story-lumberyard-cut-1
breakpoints + story-lumberyard-node-stub-1 stub-and-observe). The role word is Lumberyard, the stage whose id (0x12)
indexes the g_resource_selector_callbacks row these two fill; the rest of each name is the callee's accepted wording.
Research input: tmp/codex/func-second-pass-10-research.md (2 RENAME, 4 NO_NAME over 6 targets; untrusted proposals).
Every use of both functions was re-derived at 03842e204 from the C definitions (368E4.c:751 / :753), the declarations
(d_80DF8.c:40-41), the table row (d_80DF8.c:892 = index 0x12), the asm glabels, the census, the registries and the
comments. No name changed; 0 dropped. Line numbers cite HEAD (the research's ledger lines 744/746/890 are stale).
The 4 selector-4 / selector-7 twins (func_8004746C, func_8004748C, func_800474F0, func_80047510) stay NO_NAME.
(DATA is empty.)
"""
import csv
import glob
import os
import re

VERDICT = "FAIL"
VERIFIER_NOTE = "naming-reviewer pre-apply FAIL 0/2 (2026-10-31, naming-a): role word Lumberyard inadmissible under Q132 while every only_x stage:lumberyard row is inconclusive (hit_in_untagged_capture: lib-story-final-fight-retail-1, first hit per phase only); mechanics pass. Resubmit unchanged once the matrix is conclusive."
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8004777C": ("node_InitLumberyardFrameEntriesFromWorldTranslations", "node_InitLumberyardFrameEntriesFromWorldTranslations", ""),
    "0x8004779C": ("node_AdvanceLumberyardFrameAnimatedEntriesAndQueueDraws", "node_AdvanceLumberyardFrameAnimatedEntriesAndQueueDraws", ""),
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


def ch(addr):
    if addr not in ADJUSTED:
        return ""
    old, new, why = ADJUSTED[addr]
    if old == new:
        return ""
    return f"CHANGED FROM RESEARCH: {old}; {why}. "


def R(addr, new, fn, f, body, callers, why, counter, prior, trace):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {trace}")
    return (addr, new, "RENAME", ev)


_TWIN = ("instruction-identical to its selector-4 and selector-7 twins (asm/funcs/func_8004777C.s:2-9: frame, one jal, "
         "return); the code tells them apart only by the g_resource_selector_callbacks row index, which is 0x12 here "
         "(src/main/d_80DF8.c:892; the table starts at :874 = index 0, :873 declares it with 38 entries, indexed by "
         "resource_GetLoadedSelector(), 2B344.c:2081-2082 / :2293-2294), and the callees branch on that same selector value")

_MATRIX = ("the Q132 matrix (docs/naming/runtime/evidence/only_x.tsv) has 33 pair rows for stage:lumberyard against 17 other "
           "covered stages with zero hits in each, but all 33 are flagged inconclusive by hit_in_untagged_capture "
           "(lib-story-final-fight-retail-1 carries no stage tag); the capture's hit sits on the Lumberyard by its own frames "
           "(story.93500 stage card follows the 93436 load), so the untagged hit is inside X, but a reviewer applying the "
           "matrix flag literally may FAIL the role word until that story capture is stage-tagged")

_LIBSCAN = ("docs/naming/libscan/matches.json:809-810 and psyq_versions.txt:99 list both addresses among 8-word LIBAPI/SEND "
            "'verbatim' shape matches (size-only, the jal target masked; never evidence, N3): the jal target is game code "
            "and the only use is a game table")

FUNCS = [
    R("0x8004777C", "node_InitLumberyardFrameEntriesFromWorldTranslations", "func_8004777C", "368E4.c",
      "node_InitFrameEntriesFromWorldTranslations(); nothing else, " + _TWIN + ". Callee (src/main/32D04.c:1192-1219): "
      "paired_grid_nodes = gpu_GetGridNodeRecordTable(); for each of frame_entry_count entries a fresh root node (kind 0, "
      "select 0, reloc entry 0, the bound reloc block, rotation order 0, light setup 4, zero rotation), work.t = the paired "
      "grid node's world translation, matrix_state 0, unk58 = -1 (not started). The callee's selector cases "
      "(32D04.c:1177-1187): 7 -> 0x11 entries, 4 -> 2, 0x12 -> 0xA (:1184-1185), so row 0x12 is the 10-entry setup",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Four reference sites: the "
      "C definition src/main/368E4.c:751 (void func_8004777C(void) { node_InitFrameEntriesFromWorldTranslations(); }); the "
      "declaration src/main/d_80DF8.c:40; the one address-taken use, the .init slot of g_resource_selector_callbacks[0x12], "
      "src/main/d_80DF8.c:892 ({func_8004777C, func_8004779C}); the asm glabel/endlabel asm/funcs/func_8004777C.s:1,10 "
      "(leftover per-function file; the file name is renamed by naming_wave). The slot is read only by "
      "resource_InvokeLoadedSelectorInitCallback (src/main/2B344.c:2292-2296) while resource_GetLoadedSelector() == 0x12. "
      "Census docs/naming/function-names.csv:599 (AUTO placeholder, KEEP). Registries: no symbol_addrs.txt or "
      "undefined_*_auto.txt line; named_syms.txt:281 is a RESET note naming only the retired game_EffStart_8004777C. "
      "No comment in src/ or include/ names it. History-only mentions (not applied): docs/naming/apiscan/evidence.json:8801, "
      "proposals*.csv, no_name.tsv:40, phase3 wave03 / sweep-2026-09-25 manifests, tools/rename_funcs.py:60 (provenance map, "
      "excluded by naming_wave PY_EXCLUDE_FILES). Runtime-data mentions of the old name: see the hazard in notes.txt",
      "node_ = the callee's subsystem and the body's whole effect is node records (selector-13 neighbour row "
      "node_InitViewOffsetNodeWithYXZRotation, d_80DF8.c:887, takes node_ the same way); InitFrameEntriesFromWorldTranslations "
      "= the accepted callee name (wave88), which is the whole effect (N1); Lumberyard = the Q132 role word: the stage whose "
      "id 0x12 selects this row, the word that tells the body from its twins (the code shows the row, the capture shows the "
      "row's stage). The stage name is the game's own on-screen text: the stage card 'Next Battle Stage / Lumberyard' "
      "(capture store frame-anim-story-lumberyard-cut-1 capture.story.93500.png, read by this draft), 64 frames after the "
      "init's hit at 93436. The name claims no posts (10 entries are set up; only entries 8 and 9 were observed started)",
      "(1) the body is generic and the name claims nothing the callee does not do; (2) " + _MATRIX + "; (3) " + _LIBSCAN +
      "; (4) every static site is within X: the only site is row 0x12, read only while selector 0x12 is loaded; the "
      "captured loads of 0x12 are versus-2p-pair-lumberyard-1/-2, versus-2p-stage-map1-*, and Story Mode, all on the "
      "Lumberyard (screens.tsv rows 53461-53464); no capture shows 0x12 on another stage; (5) prefix: node_ (callee, "
      "neighbour precedent) vs a role prefix lumberyard_; (6) length: node_InitLumberyardFrameEntries would drop "
      "FromWorldTranslations, which is part of the effect",
      "no_name.tsv:40 (w49,w61,w70,w86,w93; twin objection); retired RESET name game_EffStart (wave03, sweep-2026-09-25)",
      "TRACE (Q124, corroborating): docs/naming/runtime/frame-anim-story-lumberyard-cut-1 (spec.json, capture.manifest.json): "
      "one call at frame 93436 from resource_InvokeLoadedSelectorInitCallback (ra 0x8003F1B8; evidence/events.tsv:15, "
      "screens.tsv:53461); later node_StartFrameAnimatedEntryAndInitPairedNode(8, 0x721) at 97709 and (9, 0x786) at 97953 "
      "from fight_ProcessPositionRecordsInQuad (per the research's events.tsv rows), i.e. two of the 10 entries this init "
      "set up are started by blade sweeps. Not stubbed (the knockout stubs only func_8004779C). The role rests on the "
      "table row index and the stage card."),
    R("0x8004779C", "node_AdvanceLumberyardFrameAnimatedEntriesAndQueueDraws", "func_8004779C", "368E4.c",
      "node_AdvanceFrameAnimatedEntriesAndQueueDraws(); nothing else, " + _TWIN + ". Callee (src/main/35000.c:11-109): "
      "for each of frame_entry_count entries: skip if not started (unk58 < 0); while unk58 < anim_frame_count set the "
      "entry's rotation and translation from frame unk58 rotated by its anim_yaw, rebuild its world matrix, advance unk58 "
      "(unk60 = 0x1000 at the last frame) and, for selector 0x12 only (:67-91), pose the paired grid node from the frame data "
      "too; after the last frame fade unk60 by 0x40 and (selector != 4) retire the entry at 0 (unk58 = -2, :99-101); queue "
      "every started entry on g_draw_queue_cursor (:105-107)",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Four reference sites: the "
      "C definition src/main/368E4.c:753; the declaration src/main/d_80DF8.c:41; the one address-taken use, the .unk4 slot "
      "of g_resource_selector_callbacks[0x12], src/main/d_80DF8.c:892; the asm glabel/endlabel "
      "asm/funcs/func_8004779C.s:1,10 (leftover per-function file). The slot is called only through "
      "gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards (src/main/2B344.c:2081-2082) while "
      "resource_GetLoadedSelector() == 0x12. Census docs/naming/function-names.csv:600 (AUTO placeholder, KEEP). "
      "Registries: no symbol_addrs.txt or undefined_*_auto.txt line; named_syms.txt:282 is a RESET note naming only the "
      "retired game_EffStop_8004779C. No comment in src/ or include/ names it. History-only mentions (not applied): "
      "docs/naming/apiscan/evidence.json:8810, proposals*.csv, no_lead_first_pass.csv:193, no_name.tsv:41, wave03 / "
      "sweep-2026-09-25 manifests, tools/rename_funcs.py:61 (excluded). HAZARD: the old name is also a committed path and "
      "recorded data (tools/trace/scenarios/story-lumberyard-cut.json:7, docs/naming/runtime/story-lumberyard-node-stub-1/"
      "func_8004779C/, knockout.* and evidence/*.tsv); see notes.txt and dry.txt",
      "node_ / AdvanceFrameAnimatedEntriesAndQueueDraws = the accepted callee name (wave89), the whole effect (N1), node_ "
      "as for the init and the selector-13 neighbour node_OrientAndQueueViewOffsetNode (d_80DF8.c:887); Lumberyard = the "
      "Q132 role word, the stage of row 0x12 (stage card at 93500), and the stub's lost effect is on that stage (the "
      "Lumberyard's own cut posts, below)",
      "(1) generic body; no Post / Cut word: only entries 8 and 9 were observed (as cut post sections), entries 0-7 never "
      "started in any capture; (2) " + _MATRIX + " (the matrix hits of this address: story phase 93910 and katze phase "
      "126600 of that capture, the latter the same lumber-shed scene per the research's screenshot reading); (3) " + _LIBSCAN +
      "; (4) prefix and length as for func_8004777C",
      "no_name.tsv:41 (w49,w61,w70,w82,w93; twin objection); retired RESET name game_EffStop (wave03, sweep-2026-09-25)",
      "TRACE (Q124, stub-and-observe, naming-runtime-evidence): docs/naming/runtime/story-lumberyard-node-stub-1 "
      "(knockout.md, knockout.tsv; baseline manifest SHA1 bc9b8da03e4b, stubbed bde9c4132a6d; knockout.manifest.json lists "
      "every file's SHA1): this function stubbed from phase lumber (97000), game kept running to end_frame 98351, called "
      "while stubbed; 97700 / 97712 identical, 97724 onward differ in a narrow region (1.7-3.3% of pixels through 98030, "
      "10.7% at 98100) matching the research's reading that the baseline draws a toppling cut post section where the stub "
      "draws only the stump (the paired grid node), the second post (entry 9, started 97953) likewise; 98290 (rematch "
      "round, 77%) and live excluded as changed game state. The lost pixels are the callee's queued entries, this body's "
      "own output. frame-anim-story-lumberyard-cut-1 also hits it per frame after the load."),
]

DATA = []
