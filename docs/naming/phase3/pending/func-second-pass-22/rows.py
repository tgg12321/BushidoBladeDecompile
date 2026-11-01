"""wave114 rows (func-second-pass-22): the three choice-grid page handlers, entries 4/5/6 of g_entry_menu_handlers.
Research input tmp/codex/func-second-pass-22-research.md (untrusted); every claim re-derived at HEAD 23f14978d (see notes.txt).

Re-derived code facts (src/main/5ED34.c unless noted):
- Bodies: func_80072084 :1721-1723, func_800720AC :1725-1727, func_800720D4 :1729-1731; each is one call
  menu_UpdateChoiceGrid(draw_context, g_entry_menu_resources->unk_68 / unk_6C / unk_70, mode 0 / 1 / 2); void, no other statement.
- Table slots: src/main/d_8C41C.c:46-54 g_entry_menu_handlers[7], entries [4] :51, [5] :52, [6] :53; prototypes :40-42.
  Only dispatch: :275-278 g_entry_menu_handlers[g_entry_menu_handler_index](&draw_context) (the one reader of the table in the tree;
  the table is extern'd in include/bb2.h:78).
- How `mode` indexes the per-page tables in menu_UpdateChoiceGrid (:1733-2014): map cells block `cells + (mode * 6 + i)` :1795 and the
  selected cell `(row + mode * 3) * 2 + column` :1783-1784 (6 cells per mode); scroll offset g_entry_menu_choice_grid_draw_offsets[mode]
  :1816-1821 (table d_8C41C.c:109-110 = {0x00,0x17},{0x99,0x03},{0xA0,0x24}); the grid-code row is `(g_entry_menu_handler_index - 4) * 8`
  :1891 and :1997, which equals mode because handler 4+k always passes k. Bytes 6/7 of a row are the Left/Right link (:1891-1895:
  low nibble -> g_entry_menu_next_handler_index, 0xF = no switch, high nibble -> slide phase); :289 commits next to handler_index when
  the slide ends; :1822-1826 runs the transition lerp only while both indexes are in 4..6.
- Opening page: :1691-1696, after the darken reaches 0xFF: fight_mode < 2 exits; fight_mode == 4 -> next_handler_index = 5 (page 2);
  fight_mode == 6 -> next_handler_index = 6 (page 3), choice_grid_row = 1, choice_grid_column = 1; else exit. Nothing sets 4.
- Page 3 only, mode == 2 special cases: with fight_mode == 6, cell 3 (row 1, column 1) is skipped in the selected-cell draw (:1778-1781)
  and in the cell loop (:1791), and the third sheet read uses sheet_headers[12] instead of the row sheet (:1951-1955); on Cross a
  grid code 0xD becomes stage 0x25 when fight_mode == 6 (:2001-2005).
- Reached from nowhere else: g_entry_menu_handler_index / g_entry_menu_next_handler_index are file-static (:42-43); writes of handler_index
  are :115 (0), :289 (= next), :1981 and :1986 (0 / 1, Triangle-back); writes of next are :477 (3), :1146 (1), :1422 and :1532 (0),
  :1591 (2), :1692 (5), :1694 (6), :1893 (grid link code). So entries into 4..6 are :1692 / :1694 and the grid's own page links; no jal
  or other reference to the three names exists (git grep).
Words: menu_UpdateChoiceGrid (landed callee; menu_Update<What> scheme with menu_UpdateEntryGrid / menu_UpdateRow) + Page<mode + 1>:
the code fixes only the page ORDINAL (every per-page table above is keyed by mode). The name claims no stage, no arrows, no link target,
no fight mode and no opening role. Runtime capture 6002e8dd4 (stage-pages-versus-2p-castle-1 / -kimigaura-1; hit spans and screenshots as
in its commit body: [Map 1] after LEFT for mode 0, [Map 2] first for mode 1, [Map 3] after the second RIGHT for mode 2) corroborates the
1-based numbering only; it is not the source of any word (this draft did not re-hash the captures; the reviewer reads them under
.claude/rules/naming-runtime-evidence.md). Verdicts PENDING (the reviewer fills them). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = 'naming-reviewer pre-apply 2026-10-31 (naming-a, HEAD 23f14978d): PASS 3/3 on code alone. Page<n> = mode+1: mode 0/1/2 (= handler_index-4) indexes the cell block, selected cell, draw offset and choice_table row in menu_UpdateChoiceGrid (5ED34.c:1783-1795, :1816-1826, :1891, :1997); handlers 4..6 linked by the Left/Right link bytes. Names claim no stage, arrow, opening page or fight mode. Q124 captures 6002e8dd4 re-hashed (capture.jsonl + 3 screenshots match manifests): [Map 1]/[Map 2]/[Map 3] for modes 0/1/2, corroboration only.'
NOTES = {}
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


# addr: (old, new, mode, slot, d_8C41C proto line, d_8C41C initializer line, resources member, capture line)
FN = {
    "0x80072084": ("func_80072084", "menu_UpdateChoiceGridPage1", 0, 4, 40, 51, "unk_68",
                   "capture 6002e8dd4 (corroboration only): stage-pages-versus-2p-kimigaura-1, 135 hits at frames 7316-7450 after LEFT at "
                   "7300, all from ra 0x8006EBF8 (the :278 dispatch); screenshot [Map 1]; not hit in the castle run"),
    "0x800720AC": ("func_800720AC", "menu_UpdateChoiceGridPage2", 1, 5, 41, 52, "unk_6C",
                   "capture 6002e8dd4 (corroboration only): both runs hit it first (castle 7156-7495, kimigaura 7156-7315), ra "
                   "0x8006EBF8; screenshots [Map 2]"),
    "0x800720D4": ("func_800720D4", "menu_UpdateChoiceGridPage3", 2, 6, 42, 53, "unk_70",
                   "capture 6002e8dd4 (corroboration only): stage-pages-versus-2p-castle-1, 225 hits at frames 7496-7720 after the "
                   "second RIGHT at 7480, ra 0x8006EBF8; screenshot [Map 3]; not hit in the kimigaura run"),
}

_OPEN = {
    0: ("never an opening page: the only openings set next_handler_index to 5 (:1692, fight_mode == 4) or 6 (:1694, fight_mode == 6), "
        "so page 1 is reached only through a Left/Right link code (:1891-1895)"),
    1: ("the opening page when fight_mode == 4: :1691-1692 sets next_handler_index = 5 and :289 commits it; in fight_mode 6 the menu "
        "opens on page 3 instead (:1693-1696)"),
    2: ("the opening page when fight_mode == 6, at row 1 column 1 (:1693-1696). Its mode == 2 special cases: with fight_mode == 6 cell 3 "
        "is skipped (:1778-1781, :1791) and the sheet read uses sheet_headers[12] (:1951-1955); grid code 0xD becomes stage 0x25 on Cross "
        "(:2001-2005). No other mode has a mode-keyed branch"),
}

FUNCS = []
for _addr, (_old, _new, _mode, _slot, _pl, _il, _mem, _cap) in FN.items():
    _ev = (f"BODY: {B(_old, '5ED34.c')}: menu_UpdateChoiceGrid(draw_context, g_entry_menu_resources->{_mem}, {_mode}); void, nothing else. "
           f"CALLERS: none by name; the one use is the address-taken table slot g_entry_menu_handlers[{_slot}] "
           f"(src/main/d_8C41C.c:{_il}; prototype :{_pl}), dispatched only at src/main/5ED34.c:275-278 with g_entry_menu_handler_index "
           f"as the index; no jal. Entries into handler {_slot} come only from the opening branch (:1691-1696) and the grid's own page "
           f"links (:1891-1895, committed at :289); the handler index is file-static (:42) and no other screen reads the table. "
           f"WHY: mode {_mode} is the page ordinal for every per-page table in the callee: grid-code row (handler_index - 4) * 8 = "
           f"{_mode * 8} (:1891, :1997), g_entry_menu_choice_grid_draw_offsets[{_mode}] (:1816-1821, d_8C41C.c:109-110), map cells "
           f"{_mode * 6}..{_mode * 6 + 5} (:1783-1784, :1795). Page{_mode + 1} = that ordinal, 1-based like the on-screen [Map n]; "
           f"menu_UpdateChoiceGrid is the landed callee name, and the Page<n> suffix is the separator the twins need (their bodies differ "
           f"only by the sheet member and the mode constant; w56 / w89 / w94 NO_NAME, no_name.tsv:54-56). This page is {_OPEN[_mode]}. "
           f"COUNTER-EVIDENCE: the Left/Right link targets, which arrows draw (row-3 sheets, :1953) and which stages sit on a page are "
           f"data (SEL.BIN resource file 3, :86-89), not code, so the name encodes only the ordinal and claims no adjacency, stage, "
           f"arrow, fight mode or 'first/last'; the draw offsets are monotone (0 < 0x99 < 0xA0), consistent with the ordinal but not "
           f"proof of screen order; 'Stage' is code-true (:2002 writes stage_index) but is left out to stay keyed to the landed callee "
           f"name. TRACE: {_cap}")
    FUNCS.append((_addr, _new, "RENAME", _ev))

DATA = []
