"""wave101 rows (func-second-pass-11): code-keyed qualifier names (no stage word) for the two selector hook pairs of
g_resource_selector_callbacks that are filled by single-call forwarders: row 18 (0x12) = func_8004777C / func_8004779C
(the func-second-pass-10 draft's names with Lumberyard replaced by Selector18) and row 14 (0xE) = func_80047738 /
func_8004775C. Research input: tmp/codex/func-second-pass-11-research.md (4 RENAME, 0 NO_NAME; untrusted proposals).
Every use of the four functions was re-derived at 005f7d61c from the C definitions (368E4.c:747/749/751/753), the
declarations (d_80DF8.c:38-41), the table rows (d_80DF8.c:888 = row 14, :892 = row 18), the asm glabels, the census,
the registries and the comments. No name changed; 0 dropped. Selector18 / Selector14 are DECIMAL row indices (the code's
own switch cases spell them 18 / 13 / 11, 368E4.c:56-58 and :272-279); a bare hex "Selector12" would point at the empty
row 12. The selector-4/5/7 twins keep their func_ names. (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply 2026-10-31 (naming-a, HEAD 005f7d61c): PASS 4/4. Each wrapper has one "
                 "address-taken use, a row of g_resource_selector_callbacks (d_80DF8.c:888 = row 14, :892 = row 18), "
                 "and both callers index the table by resource_GetLoadedSelector(), so Selector<N> is a code fact; "
                 "decimal row index; node_/gpu_ from the callees; no stage or water word; twins stay neutral.")
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8004777C": ("node_InitSelector18FrameEntriesFromWorldTranslations", "node_InitSelector18FrameEntriesFromWorldTranslations", ""),
    "0x8004779C": ("node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws", "node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws", ""),
    "0x80047738": ("gpu_InitSelector14VramScroll", "gpu_InitSelector14VramScroll", ""),
    "0x8004775C": ("gpu_QueueSelector14VramScroll", "gpu_QueueSelector14VramScroll", ""),
}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

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

_SPELL = ("spelling: the qualifier is the DECIMAL row index, as the code's own selector switches spell it "
          "(368E4.c:56-58 case 18, :148 case 18, :272-279 case 18 / 13 / 11) and the value watches record it; a bare hex "
          "'Selector12' (the backlog's example) reads as row 12 = {0, 0} (d_80DF8.c:886), so it would mislead; "
          "Selector0x12 / Selector0xE is the acceptable hex alternative (precedent "
          "fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11, function-names.csv:79); mixing spellings across "
          "the pairs is not. 'selector values in names' is a research-prompt heuristic for var-locals/var-globals "
          "(pending/var-globals-d/prompt.txt:29), not naming-bar.md; here the number is the function's one static site "
          "(landed precedents for code-keyed numeric qualifiers: snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVab9 csv:86, "
          "gpu_DrawList1CEntry4SpriteCellsAtOrigin csv:914, menu_HandleSelPhase0CursorInput csv:980, "
          "eff_SpawnFighter1YawedSlotQuad csv:834)")

_ONLYX18 = ("Q132 matrix (docs/naming/runtime/evidence/only_x.tsv): 33 pair rows for this address, all stage:lumberyard, "
            "all inconclusive (hit_in_untagged_capture: lib-story-final-fight-retail-1), 0 conclusive, so NO stage word "
            "(the func-second-pass-10 FAIL reason; this row carries none)")

_ONLYX14 = ("no only_x row exists for this address; matrix/facts.tsv records never_hit and reach hit [] of 91 targeted "
            "captures, and its only hits (story-gengoro-stage-0e-1, the stub knockout) are not matrix sources, so no "
            "only-X claim and NO 'Water' / 'Courtyard' word: the preset is a bare VRAM rectangle copy and what it depicts "
            "is not visible in code")

FUNCS = [
    R("0x8004777C", "node_InitSelector18FrameEntriesFromWorldTranslations", "func_8004777C",
      "node_InitFrameEntriesFromWorldTranslations(); nothing else. Instruction-identical to its selector-4 / selector-7 "
      "twins (asm/funcs/func_8004777C.s: frame, one jal, return); the code tells them apart only by the table row. "
      "Callee (src/main/32D04.c:1192-1219): paired_grid_nodes = gpu_GetGridNodeRecordTable(); for each of "
      "frame_entry_count entries a fresh root node, work.t = the paired grid node's world translation, unk58 = -1 (not "
      "started); the count is set from the same selector (32D04.c:1177-1186: 7 -> 0x11, 4 -> 2, 0x12 -> 0xA)",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Reference sites: the C "
      "definition src/main/368E4.c:751; the declaration src/main/d_80DF8.c:40; the one address-taken use, the .init slot "
      "of row 18 = 0x12, src/main/d_80DF8.c:892 ({func_8004777C, func_8004779C}); the asm glabel/endlabel "
      "asm/funcs/func_8004777C.s:1,10 (jal node_InitFrameEntriesFromWorldTranslations at :4). Table and call path: "
      + _TABLE + ". Census docs/naming/function-names.csv:599 (AUTO placeholder, KEEP). Registries: no symbol_addrs.txt "
      "or undefined_syms_auto.txt line; named_syms.txt:281 is a RESET note naming only the retired game_EffStart_8004777C. "
      "No comment in src/ or include/ names it. History-only mentions (not applied): tools/rename_funcs.py:60 (provenance "
      "map), docs/naming apiscan/proposals/no_name.tsv:40/wave03 manifests. Old name in runtime data: see hazard in notes.txt",
      "node_ = the callee's subsystem, the whole effect is node records (selector-13 neighbour row "
      "node_InitViewOffsetNodeWithYXZRotation, d_80DF8.c:887, takes node_ the same way); InitFrameEntriesFromWorldTranslations "
      "= the accepted callee name and the whole effect (N1); Selector18 = the row of the function's only use, spelled as the "
      "code's resource selector (g_resource_selector_callbacks, resource_GetLoadedSelector, "
      "g_node_frame_animation_context.resource_selector); the callees branch on that same value (32D04.c:1184, "
      "35000.c:67), so 'selector-18 entries' is a code-level set, not a label (N4/Q132: wording that tells siblings apart)",
      "(1) " + _SPELL + "; (2) " + _ONLYX18 + "; (3) libscan docs/naming/libscan/matches.json:809 and psyq_versions.txt:99 list "
      "it among 8-word LIBAPI/SEND 'verbatim' shape matches (size-only, jal target masked; never evidence, N3); (4) no "
      "Post / Cut word: all 10 entries are set up, only entries 8 and 9 were observed; (5) prefix node_ (callee, neighbour "
      "precedent) not resource_ (the table owner; the body does nothing in the resource subsystem)",
      "no_name.tsv:40 (w49,w61,w70,w86,w93; twin objection); retired RESET name game_EffStart (wave03); "
      "pending/func-second-pass-10 (pre-apply mechanics PASS, FAIL only on the role word Lumberyard)",
      "TRACE (corroborates mechanics only, no role part taken): docs/naming/runtime/frame-anim-story-lumberyard-cut-1 "
      "(e98ae53e5): one call at frame 93436 from resource_InvokeLoadedSelectorInitCallback (ra 0x8003F1B8); entries 8 and 9 "
      "are started later (97709 / 97953, per the research's events.tsv rows, not re-shown by this draft)."),
    R("0x8004779C", "node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws", "func_8004779C",
      "node_AdvanceFrameAnimatedEntriesAndQueueDraws(); nothing else. Instruction-identical to its selector-4 / selector-7 "
      "twins (asm/funcs/func_8004779C.s). Callee (src/main/35000.c:11-109): per started entry (unk58 >= 0) set rotation and "
      "translation from frame unk58 rotated by anim_yaw and advance the frame; for resource_selector == 0x12 only also pose "
      "the paired grid node (:67-91); after the last frame fade unk60 by 0x40 and retire the entry at 0 unless selector 4 "
      "(:99-101); queue every started entry on g_draw_queue_cursor (:105-107)",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Reference sites: the C "
      "definition src/main/368E4.c:753; the declaration src/main/d_80DF8.c:41; the one address-taken use, the .unk4 slot of "
      "row 18 = 0x12, src/main/d_80DF8.c:892; the asm glabel/endlabel asm/funcs/func_8004779C.s:1,10 (jal "
      "node_AdvanceFrameAnimatedEntriesAndQueueDraws at :4). Table and call path: " + _TABLE + ". Census "
      "docs/naming/function-names.csv:600 (AUTO placeholder, KEEP). Registries: no symbol_addrs.txt or "
      "undefined_syms_auto.txt line; named_syms.txt:282 is a RESET note naming only the retired game_EffStop_8004779C. No "
      "comment in src/ or include/ names it. History-only: tools/rename_funcs.py:61, no_name.tsv:41/wave03 manifests. "
      "HAZARD: the old name is also a committed path and recorded data (tools/trace/scenarios/story-lumberyard-cut.json:7, "
      "docs/naming/runtime/story-lumberyard-node-stub-1/func_8004779C/, knockout.*, evidence/*.tsv, hitmap.tsv); see "
      "notes.txt and dry.txt",
      "node_ / AdvanceFrameAnimatedEntriesAndQueueDraws = the accepted callee name and the whole effect (N1), node_ as for "
      "the init and the selector-13 neighbour node_OrientAndQueueViewOffsetNode (d_80DF8.c:887); Selector18 = the row of "
      "the only use (argued in the func_8004777C row); the callee's own selector-0x12 branch (35000.c:67, :153) makes "
      "'selector-18 entries' a code distinction",
      "(1) " + _SPELL + "; (2) " + _ONLYX18 + "; (3) libscan matches.json:810, size-only; (4) no Post / Cut word (only "
      "entries 8 and 9 observed); (5) prefix as for func_8004777C",
      "no_name.tsv:41 (w49,w61,w70,w82,w93; twin objection); retired RESET name game_EffStop (wave03); "
      "pending/func-second-pass-10 (same)",
      "TRACE (corroborates mechanics only; the name takes no role part): stub-and-observe "
      "docs/naming/runtime/story-lumberyard-node-stub-1 (e98ae53e5; knockout.manifest.json lists baseline "
      "capture.manifest.json bc9b8da03e4b and func_8004779C bde9c4132a6d): stubbed from phase lumber (97000), the game ran "
      "to end_frame 98351 and the stub was called; 97700 / 97712 identical, from 97724 only a narrow region differs (the "
      "upper section of a cut wooden post is not drawn: the callee's queued entries, this body's own output)."),
    R("0x80047738", "gpu_InitSelector14VramScroll", "func_80047738",
      "gpu_InitVramScroll(1, 0); nothing else. Unlike the other pairs the body differs from its row-5 twin "
      "func_800474AC in code, only by the preset index (asm/funcs/func_80047738.s:4 addiu a0, zero, 0x1 vs a0 = 0 in "
      "func_800474AC.s:4). Callee (src/main/368E4.c:1685-1704, CORROBORATED csv:631): slot 0, so no "
      "sys_StoreZeroPastScratchpadOnFailedCheck; g_gpu_vram_scrolls[0].phase = 0, then the 7 control halfwords are copied "
      "from g_gpu_vram_scroll_presets[1] (src/main/d_89C78.c:212-214: {0x2F0, 0x140, 0x10, 0x40, 0x2A0, 0x180, 0x1}: "
      "source x/y 752/320, w 16, h 64, destination x/y 672/384, step 1)",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Reference sites: the C "
      "definition src/main/368E4.c:747; the declaration src/main/d_80DF8.c:38; the one address-taken use, the .init slot of "
      "row 14 = 0xE, src/main/d_80DF8.c:888 ({func_80047738, func_8004775C}); the asm glabel/endlabel "
      "asm/funcs/func_80047738.s:1,11 (jal gpu_InitVramScroll at :5). Table and call path: " + _TABLE + ". Census "
      "docs/naming/function-names.csv:597 (AUTO placeholder, KEEP). Registries: no symbol_addrs.txt or "
      "undefined_syms_auto.txt line; named_syms.txt:169 is a RESET note naming only the retired game_AnimStart_80047738. "
      "No comment in src/ or include/ names it. History-only: tools/rename_funcs.py:58, docs/naming/apiscan/evidence.json, "
      "no_name.tsv:38.",
      "gpu_ = the callee's subsystem and the whole effect (gpu_InitVramScroll / gpu_QueueVramScroll are CORROBORATED); "
      "InitVramScroll = the callee wording (N1); Selector14 = the row of its only use, decimal like the code's other "
      "selector cases (368E4.c:272-279). The preset index (1) is left out: the row already tells it from its row-5 twin "
      "and determines the preset at every use",
      "(1) " + _SPELL + "; (2) " + _ONLYX14 + "; (3) the ledger's 'Q132 forwarder exception' phrase is in no rule (the "
      "research found none in .claude/); N4 asks for wording that tells siblings apart, which the row does; (4) twin "
      "func_800474AC (row 5, preset 0) stays func_800474AC (never reached in any capture); (5) no libscan match is listed "
      "for 0x80047738",
      "no_name.tsv:38 ('preset-1 twin of func_800474AC; preset rows differ only numerically, d_89C78.c:210-212'); retired "
      "RESET name game_AnimStart (wave03)",
      "TRACE (corroborates the load-time init only): docs/naming/runtime/story-gengoro-stage-0e-1 (020a27765): hitmap.tsv "
      "shows func_80047738 and gpu_InitVramScroll hit only in phase stage_0e (first hit 85344); the phases.tsv value watch "
      "has g_loaded_resource_selector 13 at 85233 and 14 at the end (91000), consistent with row 14. No stub of the init "
      "was run."),
    R("0x8004775C", "gpu_QueueSelector14VramScroll", "func_8004775C",
      "gpu_QueueVramScroll(0); nothing else. Instruction-identical to its row-5 twin func_800474D0 (asm/funcs/"
      "func_8004775C.s vs func_800474D0.s: frame, jal gpu_QueueVramScroll with a0 = 0, return), so the row is the only code "
      "distinction. Callee (src/main/368E4.c:1706-1771, CORROBORATED csv:632): from slot 0's control halfwords builds two "
      "DR_MOVE strips per level for 3 levels (halving positions and sizes), linked at g_gpu_ot_offset_ptr[0xFFF], copying "
      "the h-tall source rectangle rotated by 'phase' rows, then advances phase by ctl[6] mod ctl[3]",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Reference sites: the C "
      "definition src/main/368E4.c:749; the declaration src/main/d_80DF8.c:39; the one address-taken use, the .unk4 slot of "
      "row 14, src/main/d_80DF8.c:888; the asm glabel/endlabel asm/funcs/func_8004775C.s:1,10 (jal gpu_QueueVramScroll at "
      ":4). Table and call path: " + _TABLE + ". Census docs/naming/function-names.csv:598 (AUTO placeholder, KEEP). "
      "Registries: no symbol_addrs.txt or undefined_syms_auto.txt line; named_syms.txt:280 is a RESET note naming only the "
      "retired game_AnimStop_8004775C. No comment in src/ or include/ names it. History-only: tools/rename_funcs.py:59, "
      "apiscan/evidence.json, no_name.tsv:39. HAZARD: the old name is also a committed path and recorded data "
      "(tools/trace/scenarios/story-gengoro-stage-0e.json:2,17, docs/naming/runtime/story-courtyard-vram-scroll-stub-1/"
      "func_8004775C/, knockout.*, hitmap.tsv); see notes.txt and dry.txt",
      "gpu_ / QueueVramScroll = the callee wording and subsystem, the whole effect (N1); Selector14 = the row of its only "
      "use and, because every load or reuse of a selector runs that row's .init first (368E4.c:77, :70, :181), also the "
      "preset slot 0 holds whenever this body queues it (func_80047738, same row); consistent with "
      "gpu_InitSelector14VramScroll",
      "(1) " + _SPELL + "; (2) " + _ONLYX14 + "; (3) the stub knockout (see TRACE) shows the changed pixels on the dark "
      "courtyard surface, but the knockout and the story-gengoro capture are not matrix sources, and slot 0 is shared with "
      "row 5's preset 0 (8x32 at 792,384 into 744,256), so the mechanism is not water-specific; (4) twin func_800474D0 (row "
      "5) stays func_800474D0; (5) libscan LIBSND/UT_ROFF and LIBSPU/S_I 8-word shape matches (matches.json:8894, :9358): "
      "size-only, never evidence; (6) runtime limit: from 90000 the runs diverge (cause not established), so only "
      "85300-89000 count",
      "no_name.tsv:39 (identical to func_800474D0, gpu_QueueVramScroll(0), .unk4 slot of selector 14); retired RESET name "
      "game_AnimStop (wave03)",
      "TRACE (corroborates the per-frame VRAM move; no role word taken): stub-and-observe "
      "docs/naming/runtime/story-courtyard-vram-scroll-stub-1 (5d03e3bfc; knockout.manifest.json lists baseline "
      "capture.manifest.json 51b96f11628b and func_8004775C f43d2b0b91a7): stubbed from phase stage_0e (85233), the stub was "
      "called at 85871 (ra 0x8003EB28, inside gpu_CollectGrid...AndDrawBillboards), the game ran to end_frame 91000; "
      "85300 / 85600 0 changed pixels, 86000 22749 px .. 89000 2035 px, all on the dark ground surface; a frozen scroll "
      "texture is this body's own output (its queued DR_MOVEs)."),
]

DATA = []
