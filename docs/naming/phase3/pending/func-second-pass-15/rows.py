"""wave106 rows (func-second-pass-15): the row-5 hook pair of g_resource_selector_callbacks, func_800474AC (.init) and
func_800474D0 (.unk4), named like the wave101 selector-14 pair they mirror: the callee's accepted wording plus the DECIMAL
row index (gpu_InitSelector14VramScroll / gpu_QueueSelector14VramScroll, landed). Research input:
tmp/codex/func-second-pass-15-research.md (2 RENAME, 0 NO_NAME; untrusted proposals). Every use of the two functions was
re-derived at 92609d0dc from the C definitions (368E4.c:683/685), the declarations (d_80DF8.c:28-29), the table row
(d_80DF8.c:879 = row 5), the asm glabels, the census, the registries and the comments. No name changed; 0 dropped.
No stage word (only_x inconclusive for both). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = ('naming-reviewer pre-apply 2026-10-31 (naming-a, HEAD 92609d0dc): PASS 2/2. Each wrapper has one address-taken use, row 5 of g_resource_selector_callbacks (d_80DF8.c:879), dispatched by resource_GetLoadedSelector() with no offset, so Selector5 is a code fact and the only distinction from the identical selector-14 twins (wave101); the 0-pixel knockout (547c9673e) does not undercut the callee wording VramScroll; no stage word (only_x inconclusive); fresh.')
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x800474AC": ("gpu_InitSelector5VramScroll", "gpu_InitSelector5VramScroll", ""),
    "0x800474D0": ("gpu_QueueSelector5VramScroll", "gpu_QueueSelector5VramScroll", ""),
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
          "(src/main/2B344.c:2292-2296), which runs on both the reuse path (src/main/368E4.c:70) and the fresh-load path "
          "(:182, after g_loaded_resource_selector is set at :77) of resource_LoadOrReuseSelectedBlock; the unk4 slot is "
          "called only by gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards (src/main/2B344.c:2081-2082)")

_SPELL = ("spelling: the qualifier is the row index of the function's only use; 5 is the same digit in decimal and hex, so the "
          "wave101 decimal convention (Selector14 / Selector18; the code's own switch cases spell rows in decimal, "
          "368E4.c:56-58, :270-281) needs no choice here; Selector5 is the matching sibling of the landed "
          "gpu_InitSelector14VramScroll / gpu_QueueSelector14VramScroll (function-names.csv:597-598). Reader hazard: "
          "g_fight_mode_selector == 5 is a different global (not the resource selector); the gpu_..VramScroll context and "
          "the Selector14 / Selector18 family disambiguate it; fallback ResourceSelector5 only together with renaming the "
          "wave101 pair, never mixed")

_ONLYX = ("Q132 matrix (docs/naming/runtime/evidence/only_x.tsv:277 / :278): inconclusive (capture_level_only, tag '-'), the "
          "pair rows (:2570 / :2571, event:vs_cpu_clear) inconclusive too, and versus-cpu-clear-retail-1 has no phase_tags "
          "rows, so no stage axis: NO 'Pier' (or any stage / surface) word; the slot is shared with row 14's preset 1 "
          "so the mechanism is not specific to any surface")

FUNCS = [
    R("0x800474AC", "gpu_InitSelector5VramScroll", "func_800474AC",
      "gpu_InitVramScroll(0, 0); nothing else (asm/funcs/func_800474AC.s:4-5: a0 = 0, jal gpu_InitVramScroll with a1 = 0 in "
      "the delay slot). Differs from its row-14 twin (gpu_InitSelector14VramScroll, 368E4.c:748, a0 = 1) only by the preset "
      "index. Callee (src/main/368E4.c:1690-1709, CORROBORATED csv:631): slot 0, so no "
      "sys_StoreZeroPastScratchpadOnFailedCheck; g_gpu_vram_scrolls[0].phase = 0, then the 7 control halfwords are copied "
      "from g_gpu_vram_scroll_presets[0] (src/main/d_89C78.c:213: {0x318, 0x180, 0x8, 0x20, 0x2E8, 0x100, 0x1}: source x/y "
      "792/384, w 8, h 32, destination x/y 744/256, step 1)",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Reference sites: the C "
      "definition src/main/368E4.c:683; the declaration src/main/d_80DF8.c:28; the one address-taken use, the .init slot of "
      "row 5, src/main/d_80DF8.c:879 ({func_800474AC, func_800474D0}); the asm glabel/endlabel "
      "asm/funcs/func_800474AC.s:1,11 (jal gpu_InitVramScroll at :5). Table and call path: " + _TABLE + ". Census "
      "docs/naming/function-names.csv:589 (AUTO placeholder, KEEP). Registries: no symbol_addrs.txt or "
      "undefined_syms_auto.txt line; named_syms.txt:167 is a RESET note naming only the retired game_AnimInit_800474AC (:1393 "
      "is a retired-name back-reference in gpu_InitVramScroll's note). No comment in src/ or include/ names it. "
      "History-only: tools/rename_funcs.py:51, docs/naming/apiscan/evidence.json:8707, no_name.tsv:34. "
      "HAZARD: the old name is also in committed trace data (tools/trace/scenarios/versus-cpu-clear-retail.json, "
      "hitmap.tsv / evidence / matrix tsv files); see notes.txt and dry.txt",
      "gpu_ = the callee's subsystem and the whole effect (gpu_InitVramScroll is CORROBORATED); InitVramScroll = the callee "
      "wording (N1); Selector5 = the row of its only use, decimal like the landed Selector14 / Selector18. The preset index "
      "(0) is left out, as wave101 left out preset 1: the row fixes the preset at every use. Uniqueness: row 5's only other "
      "occupant is func_800474D0; the only other VRAM-scroll init is gpu_InitSelector14VramScroll; the row-4 / row-7 twins "
      "(func_8004746C / func_800474F0) call node_ functions and stay func_",
      "(1) " + _SPELL + "; (2) " + _ONLYX + "; (3) the revived NO_NAME objection (preset rows differ only numerically) was "
      "answered by wave101 for the twin: the row is the code distinction (N4 wording that tells siblings apart)",
      "no_name.tsv:34 (w49,w61,w86,w95; 'twin func_80047738 is the same call with preset 1'); retired RESET name "
      "game_AnimInit (wave03); wave101 (landed) named the twin",
      "TRACE (corroborates reach and args only, no role word; per the research, not re-shown by this draft): "
      "docs/naming/runtime/versus-cpu-clear-retail-1 hit map: func_800474AC only in opp4, first hit frame 19428 with ra "
      "0x8003F1B8 (inside resource_InvokeLoadedSelectorInitCallback), gpu_InitVramScroll args (0, 0). The pier stub "
      "capture (versus-cpu-pier-vram-scroll-stub-1, 547c9673e) arms no hit on this function."),
    R("0x800474D0", "gpu_QueueSelector5VramScroll", "func_800474D0",
      "gpu_QueueVramScroll(0); nothing else. Instruction-identical to gpu_QueueSelector14VramScroll (368E4.c:750; "
      "asm/funcs/func_800474D0.s: frame, jal gpu_QueueVramScroll with a0 = 0 in the delay slot, return), so the row is the "
      "only code distinction. Callee (src/main/368E4.c:1711-1776, CORROBORATED csv:632): from slot 0's control halfwords "
      "builds two DR_MOVE strips per level for 3 levels (halving positions and sizes), linked at g_gpu_ot_offset_ptr[0xFFF], "
      "copying the h-tall source rectangle rotated by 'phase' rows, then advances phase by ctl[6] mod ctl[3]",
      "none by name: no jal, no call, no other reference in src/, include/ or asm/ (git grep). Reference sites: the C "
      "definition src/main/368E4.c:685; the declaration src/main/d_80DF8.c:29; the one address-taken use, the .unk4 slot of "
      "row 5, src/main/d_80DF8.c:879; the asm glabel/endlabel asm/funcs/func_800474D0.s:1,10 (jal gpu_QueueVramScroll at "
      ":4). Table and call path: " + _TABLE + ". Census docs/naming/function-names.csv:590 (AUTO placeholder, KEEP). "
      "Registries: no symbol_addrs.txt or undefined_syms_auto.txt line; named_syms.txt:168 is a RESET note naming only the "
      "retired game_AnimCleanup_800474D0 (:1394 back-reference). No comment in src/ or include/ names it. History-only: "
      "tools/rename_funcs.py:52, apiscan/evidence.json:8716, no_name.tsv:35, libscan matches.json:8893 / :9357 (size-only "
      "shape matches, never evidence). HAZARD: the old name is also a committed path and recorded data "
      "(docs/naming/runtime/versus-cpu-pier-vram-scroll-stub-1/func_800474D0/, knockout.*, hitmap.tsv, tools/trace/scenarios); "
      "see notes.txt and dry.txt",
      "gpu_ / QueueVramScroll = the callee wording and subsystem, the whole effect (N1); Selector5 = the row of its only use "
      "and, because every load or reuse of a selector runs that row's .init first (368E4.c:77, :70, :182), also the preset "
      "slot 0 holds whenever this body queues it (func_800474AC, same row); consistent with gpu_InitSelector5VramScroll "
      "and the selector-14 counterparts. Uniqueness: the only other gpu_QueueVramScroll wrapper is gpu_QueueSelector14VramScroll",
      "(1) " + _SPELL + "; (2) " + _ONLYX + "; (3) the stub knockout shows 0 changed pixels at every compare frame, so it "
      "neither supports nor contradicts the callee wording; 'nothing visible in these views' is not 'does nothing' and no "
      "role word is taken from it; (4) the 'Q132 forwarder exception' phrase in no_name.tsv:35 is in no rule (git grep -i "
      "forwarder .claude/ = 0 hits); N4 asks for wording that tells siblings apart, which the row does",
      "no_name.tsv:35 (w49,w61,w86,w95; identical to func_8004775C); retired RESET name game_AnimCleanup (wave03); wave101 "
      "named the twin gpu_QueueSelector14VramScroll",
      "TRACE (corroborates reach and the per-frame call site only; per the research, not re-shown by this draft): "
      "stub-and-observe docs/naming/runtime/versus-cpu-pier-vram-scroll-stub-1 (547c9673e): stubbed from frame 19220 (opp4), "
      "first stubbed call 19664 with ra 0x8003EB28 (inside gpu_CollectGrid...AndDrawBillboards), game ran to end_frame "
      "25000; 0 changed pixels at compare frames 19300-20700. Hit map versus-cpu-clear-retail-1: func_800474D0 only in opp4, "
      "first hit 19664, gpu_QueueVramScroll a0 = 0."),
]

DATA = []
