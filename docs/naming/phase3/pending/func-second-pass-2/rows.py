"""wave93 rows (func-second-pass-2): Q132 mechanical name for 1 of the 20 neutral functions re-researched in the
func-second-pass-2 batch: 0x8004A4E0 (main/3AB48, the draw-queue walk called by gpu_PrepareViewAndFlushDrawQueue).
Research input: tmp/codex/func-second-pass-2-research.md (1 RENAME, 19 NO_NAME over 20 targets; untrusted proposals).
The row was re-derived from asm/funcs/func_8004A4E0.s, jtbl_8001541C / jtbl_8001545C (d_5BF0.c), the kind bodies in
asm/funcs/func_8004A940.s and the sole caller (32D04.c:1161); the research name is kept (not CHANGED FROM RESEARCH).
Research NO_NAME (19; each stays neutral, not a row):
 8004746C twin of 800474F0 / 8004777C: one call node_InitFrameEntriesFromWorldTranslations; only the unexplained selector row differs.
 8004748C twin of 80047510 / 8004779C: one call node_AdvanceFrameAnimatedEntriesAndQueueDraws; stage identity of the selector row unshown.
 800474F0 selector-7 twin of 8004746C; a size (0x11), not a role, is all the code shows.
 80047510 selector-7 twin of 8004748C.
 8004777C selector-0x12 twin of 8004746C; same objection.
 8004779C selector-0x12 twin of 8004748C; same objection.
 8004A940 symbol extent also holds the node-kind case bodies of 8004A4E0 and a primitive handler; no one verb covers it.
 8004C404 kind-12 case: scratch matrix fill, section-stream decode, optional tail jump, consumes following kind-12 entries; roles of D_800F6658 / D_80103624 undecoded.
 8004DDB4 one gte call inside a section/repeat-block decoder; the repeat-block contract is unbounded.
 80063AF0 effect command 0 yawed-quad start; lane twin of 80063B34, no code-shown distinction from command 1.
 80063B34 lane-1 twin of 80063AF0.
 80063B78 lane-0 phase-1 draw twin of 80063BA4.
 80063BA4 lane-1 twin of 80063B78.
 80064ED8 animated-quad record 1 start; twin of 80064F20, only the mode index separates them.
 80064F20 mode-2 twin of 80064ED8.
 80064F68 animated-quad record 3 start; twin of 80064FB4 (same 0x40 seed), only the mode index differs.
 80064FB4 mode-4 twin of 80064F68.
 8006505C animated-quad record 6 start; twin of 800650A4.
 800650A4 mode-7 twin of 8006505C.
(DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 1/1 (2026-10-28, naming-a)"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8004A4E0": ("gpu_DrawQueuedNodes", "gpu_DrawQueuedNodes", ""),
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
    R("0x8004A4E0", "gpu_DrawQueuedNodes", "func_8004A4E0", "3AB48.c",
      "asm/funcs/func_8004A4E0.s: loops s7 over g_draw_queue until g_draw_queue_cursor; per entry loads the node pointer, "
      "dispatches on its node_kind byte through jtbl_8001541C (d_5BF0.c:58-62; kinds 2, 4-9 and 11 fall to the next entry); "
      "for the model kinds looks up g_reloc_block_entry_tables[reloc_block_id][reloc_entry_index] (:39), walks the section "
      "stream and per section runs gte_ProjectSVectorTriples / gte_ProjectPackedVertices (jtbl_8001545C, d_5BF0.c:65) with "
      "the 0x1F800008 shift, then func_8004A940, whose handler table at 0x1F80001C emits the primitives; the other kinds' "
      "bodies in func_8004A940.s reach gte_MulMatrix0ClearTrans, math_ApplyLightPlaneProjection, MulMatrix2, "
      "gpu_ProjectStridedNodeVerticesAndDispatchPackets (:597), wave_Draw (:879), func_8004C404 (:493) and func_800430E4 (:485); "
      "no write outside scratchpad and the primitive buffer, no change to g_draw_queue or the cursor",
      "32D04.c:1161 only (gpu_PrepareViewAndFlushDrawQueue, after setting the view and OT scratch; it resets "
      "g_draw_queue_cursor = g_draw_queue at :1163); extern 32D04.c:1133; INCLUDE_ASM 3AB48.c:14; "
      "asm/funcs/gpu_PrepareViewAndFlushDrawQueue.s jal",
      "the verb covers the whole effect: every reachable path of the queue walk ends in projection and primitive emission "
      "(or wave_Draw / strided-vertex dispatch), and the walk consumes the queue the name says; the flush's reset of the "
      "cursor is the caller's, not this body's; gpu_ matches the callee tables (gpu_Emit* handlers, "
      "gpu_ProjectStridedNodeVerticesAndDispatchPackets) and the caller; Draw is distinct from siblings gpu_Draw<Ui layout> "
      "(screen sprite drawers, none takes the queue) and from gpu_QueueDepthSpacedMovesAndLinkTerminalPacket / "
      "gpu_PrepareViewAndFlushDrawQueue (the Queue*/Flush names)",
      "the case bodies sit inside func_8004A940's symbol extent (a split boundary on that side, not a contradiction); the "
      "kind-10 body flips the draw-state shift via func_800430E4 and kind 12 consumes following kind-12 entries, both still "
      "queued-node drawing; kinds 2, 4-9, 11 do nothing, which 'Draw' tolerates (skip, not contradict); the node kinds stay "
      "unnamed so 'Nodes' makes no kind claim",
      "ledger objection w59 / w82 (interpreter contract unbounded) answered: the name claims the walk and the draw, not the contract"),
]

DATA = []
