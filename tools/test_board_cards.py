#!/usr/bin/env python3
"""Unit tests for tools/board_cards.py.

Run: python tools/test_board_cards.py   (exit 0 = pass)

Covers the PURE logic — budgeting/truncation, template assembly, the cheat/wip
brief variants, and extract_c_body on real repo funcs. Network/objdump-dependent
paths are stubbed or guarded (skip if build/bb2.elf absent).
"""
from __future__ import annotations

import sys
from pathlib import Path

_REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(_REPO / "tools"))
import board_cards as bc  # noqa: E402

_passed = _failed = 0
_SENTINEL = object()


def check(desc, cond, expected=_SENTINEL):
    global _passed, _failed
    ok = (cond == expected) if expected is not _SENTINEL else bool(cond)
    if ok:
        _passed += 1
    else:
        _failed += 1
        print(f"  FAIL: {desc}", file=sys.stderr)


def eq(desc, got, want):
    check(f"{desc} (got {got!r}, want {want!r})", got == want)


# --- shared fixtures -------------------------------------------------------

def _card(status="active", **kw):
    base = {"func": "func_80012345", "file": "code6cac", "distance": 9,
            "verdict": "C", "rules": 3, "status": status, "park_reason": None,
            "addr": "80012345"}
    base.update(kw)
    return base


def _build(card, **kw):
    args = {"cheats": [], "wip": None, "history": [], "c_body": None,
            "disasm": None, "tech_index": {}, "rule_slugs": set(),
            "head_short": "abcd123"}
    args.update(kw)
    return bc.build_body(card, **args)


# --- budgeting / truncation ------------------------------------------------

def test_small_body_not_truncated():
    body, trunc = _build(_card(), disasm="80012345 <func_80012345>:\n  nop\n  jr $ra")
    eq("small body not truncated", trunc, False)
    check("small body under limit", len(body) <= bc.BODY_LIMIT)
    check("has metadata header", "**File:**" in body)


def test_huge_asm_truncated_and_under_limit():
    huge = "80012345 <func_80012345>:\n" + "\n".join(
        f"{0x80012345 + i:08x}:\t00000000 \tnop  # padding instruction line {i}"
        for i in range(40000))
    body, trunc = _build(_card(), disasm=huge)
    check("huge asm marked truncated", trunc)
    check("huge asm body <= BODY_LIMIT", len(body) <= bc.BODY_LIMIT)
    check("truncation note present", "[truncated" in body)
    check("asm header kept", "80012345 <func_80012345>:" in body)


def test_huge_c_and_asm_both_truncate():
    huge_asm = "80012345 <f>:\n" + "\n".join(f"line{i} nop" for i in range(30000))
    huge_c = "void f(void) {\n" + "\n".join(f"    do_thing_{i}();" for i in range(20000)) + "\n}"
    body, trunc = _build(_card(), disasm=huge_asm, c_body=huge_c)
    check("both-huge marked truncated", trunc)
    check("both-huge under hard limit", len(body) <= bc.BODY_LIMIT)


def test_truncate_asm_keeps_header_and_note():
    disasm = "HDR\n" + "\n".join(f"i{i}" for i in range(100))
    out = bc._truncate_asm(disasm, 10)
    lines = out.splitlines()
    eq("keeps first 10 + note", len(lines), 11)
    eq("header kept", lines[0], "HDR")
    check("note mentions truncated", "[truncated 91 of 101" in lines[-1])


def test_truncate_asm_noop_when_small():
    disasm = "HDR\ni0\ni1"
    eq("no truncation when keep>=total", bc._truncate_asm(disasm, 99), disasm)


# --- template assembly: brief variants -------------------------------------

def test_brief_active_c():
    b = bc._brief(_card(status="active", verdict="C", distance=9, rules=3))
    check("active+C brief", "INCOMPLETE — honest pure-C distance 9, 3 cheat" in b)


def test_brief_active_asm():
    b = bc._brief(_card(status="active", verdict="ASM-STRUCTURAL"))
    check("active+ASM brief", "routed ASM-STRUCTURAL" in b)


def test_brief_parked():
    b = bc._brief(_card(status="parked", park_reason="needs canonical decision"))
    check("parked brief", b.startswith("PARKED — needs canonical decision"))


def test_brief_completed():
    b = bc._brief(_card(status="completed"))
    eq("completed brief", b, "COMPLETED — matched, byte-identical, zero cheats.")


def test_body_completed_label_and_sections():
    body, _ = _build(_card(status="completed", verdict=None, distance=None, rules=0),
                     c_body="void f(void) { return; }")
    check("completed label", "COMPLETED" in body.splitlines()[0])
    check("completed cheats says pure C", "None — pure C." in body)
    check("has C details", "Current C source" in body)
    check("no WIP section", "## WIP checkpoint" not in body)


def test_body_active_with_cheats():
    cheats = [{"type": "regfix-subst", "rule_text": "func_X: subst a -> b @ 3",
               "description": "desc", "source_file": "regfix.txt"}]
    body, _ = _build(_card(), cheats=cheats)
    check("cheat count in header", "## Identified cheats / rules (1)" in body)
    check("cheat rule text shown", "func_X: subst a -> b @ 3" in body)
    check("cheat source file shown", "_(regfix.txt)_" in body)


def test_body_active_no_cheats_note():
    body, _ = _build(_card(rules=0), cheats=[])
    check("no-cheats active note", "No regfix/asmfix rules; INCOMPLETE via honest" in body)


def test_section_wip_renders_floor_and_resume():
    wip = {"candidate_floor": 4, "head_floor": 9, "target_insns": 131,
           "reviewer": "PASS", "sessions": 1, "rejected_forms": 7,
           "latest_lever": "consolidated GTE block", "gap_summary": "two diffs in /100",
           "lever_slugs": [], "memory_refs": []}
    body, _ = _build(_card(), wip=wip)
    check("wip section present", "## WIP checkpoint" in body)
    check("floor rendered", "candidate **4** vs head 9" in body)
    check("target insns rendered", "target insns: 131" in body)
    check("reviewer rendered", "Reviewer: PASS" in body)
    check("latest lever rendered", "consolidated GTE block" in body)
    check("resume hint rendered", "apply `memory/wip/func_80012345/candidate.c`" in body)
    check("header WIP yes", "**WIP:** yes" in body)


def test_section_techniques_validated_slug():
    wip = {"candidate_floor": 4, "head_floor": 9, "target_insns": 100,
           "reviewer": "PASS", "sessions": 1, "rejected_forms": 0,
           "latest_lever": None, "gap_summary": None,
           "lever_slugs": ["shared-end-label", "no-new-park-categories", "bogus-slug"],
           "memory_refs": []}
    tech_index = {"shared-end-label": "restructure to goto end"}
    body, _ = _build(_card(), wip=wip, tech_index=tech_index,
                     rule_slugs={"shared-end-label", "no-new-park-categories"})
    check("techniques section present", "## Techniques" in body)
    check("validated slug shown", "`shared-end-label` — restructure to goto end" in body)
    check("policy slug filtered out", "no-new-park-categories" not in body)
    check("unknown slug filtered out", "bogus-slug" not in body)


def test_section_history_caps_at_15():
    hist = [{"sha": f"sha{i:05d}", "date": "2026-06-13", "subject": f"subj {i}",
             "prefix": "match", "techniques": [], "verdict": None, "scores": []}
            for i in range(20)]
    body, _ = _build(_card(), history=hist)
    check("history header counts all", "## History (20 commits, newest first)" in body)
    check("caps and notes the rest", "and 5 more" in body)
    eq("only 15 bullets rendered", body.count("\n- `sha"), 15)


def test_section_memrefs():
    wip = {"candidate_floor": 1, "head_floor": 2, "target_insns": 10,
           "reviewer": "PASS", "sessions": 1, "rejected_forms": 0,
           "latest_lever": None, "gap_summary": None, "lever_slugs": [],
           "memory_refs": ["memory/project/func-x-lever.md"]}
    body, _ = _build(_card(), wip=wip)
    check("memrefs section present", "## Memory refs" in body)
    check("memref path shown", "`memory/project/func-x-lever.md`" in body)


def test_details_block_gfm_blank_line():
    block = bc._details_block("Sum", "c", "int x;")
    check("blank line after summary", "</summary>\n\n```c" in block)
    check("closing details", block.rstrip().endswith("</details>"))


def test_c_unavailable_placeholder():
    body, _ = _build(_card(file="code6cac"), c_body=None)
    check("C placeholder", "// C source not available in src/code6cac.c" in body)


def test_asm_unavailable_placeholder():
    body, _ = _build(_card(), disasm=None)
    check("asm placeholder", "disassembly not available" in body)


def test_next_step_parked_includes_reason():
    body, _ = _build(_card(status="parked", park_reason="needs user auth"))
    check("next step parked reason", "needs user auth" in body)


def test_next_step_active_wip_resume():
    wip = {"candidate_floor": 4, "head_floor": 9, "target_insns": 1,
           "reviewer": "PASS", "sessions": 1, "rejected_forms": 0,
           "latest_lever": None, "gap_summary": None, "lever_slugs": [],
           "memory_refs": []}
    body, _ = _build(_card(status="active"), wip=wip)
    check("active+wip next step resume", "Resume from the WIP checkpoint" in body)


# --- board set construction ------------------------------------------------

def test_build_board_set_merges_queue_and_completed():
    items = [{"func": "func_a", "file": "x", "distance": 9, "verdict": "C",
              "rules": 3, "status": "active"}]
    inv = {"func_a": "x", "func_done": "display"}  # func_a is in queue -> stays active
    board = bc.build_board_set(items, inv)
    eq("two board funcs", sorted(board), ["func_a", "func_done"])
    eq("queue func active", board["func_a"]["status"], "active")
    eq("completed func status", board["func_done"]["status"], "completed")
    eq("completed file from inventory", board["func_done"]["file"], "display")


def test_build_board_set_park_reason_carried():
    items = [{"func": "func_p", "file": "x", "distance": 0, "verdict": "C",
              "rules": 0, "status": "parked", "park_reason": "blocked: foo"}]
    board = bc.build_board_set(items, {})
    eq("park reason carried", board["func_p"]["park_reason"], "blocked: foo")


# --- WIP reader (real func_8002BEA0) ---------------------------------------

def test_read_wip_real_func_8002bea0():
    wip = bc.read_wip("func_8002BEA0")
    if wip is None:
        check("SKIP read_wip (func_8002BEA0 WIP absent)", True)
        return
    eq("candidate floor", wip["candidate_floor"], 4)
    eq("head floor", wip["head_floor"], 9)
    eq("target insns", wip["target_insns"], 131)
    eq("reviewer verdict from dict", wip["reviewer"], "PASS")
    check("rejected forms counted", wip["rejected_forms"] >= 1)
    check("latest lever present", bool(wip["latest_lever"]))


def test_read_wip_string_reviewer_guard():
    # initDrawMode's meta has reviewer as a bare string — must not crash.
    wip = bc.read_wip("initDrawMode")
    if wip is None:
        check("SKIP read_wip (initDrawMode WIP absent)", True)
        return
    check("string reviewer handled", isinstance(wip["reviewer"], (str, type(None))))


def test_read_wip_absent_returns_none():
    eq("absent WIP -> None", bc.read_wip("func_does_not_exist_zz"), None)


# --- extract_c_body on real repo funcs -------------------------------------

def test_extract_c_body_real_func():
    # Pick a real func from the queue and try to extract it.
    items = bc.load_queue_items()
    found = False
    for it in items:
        stem = it.get("file")
        if not stem:
            continue
        src = _REPO / "src" / f"{stem}.c"
        if not src.exists():
            continue
        body = bc.extract_c_body(src, it["func"])
        if body is not None:
            check("extracted body has a brace", "{" in body and body.rstrip().endswith("}"))
            check("extracted body names the func", it["func"] in body)
            found = True
            break
    if not found:
        check("SKIP extract_c_body (no extractable queue func found)", True)


def test_extract_c_body_missing_file():
    eq("missing file -> None", bc.extract_c_body(_REPO / "src" / "nope_zz.c", "func_x"), None)


# --- technique index parse -------------------------------------------------

def test_load_technique_index_real():
    idx = bc.load_technique_index()
    if not idx:
        check("SKIP technique index (file absent)", True)
        return
    check("parsed several slugs", len(idx) > 20)
    check("known slug present", "shared-end-label" in idx)
    check("slug has a description", bool(idx.get("shared-end-label")))


# --- guarded real-objdump smoke test ---------------------------------------

def test_disasm_by_address_slices_flat_stream():
    """Address slicing must recover a function even when objdump omits its
    `<addr> <name>:` separator (the header-less case)."""
    text = (
        "80044370:\t03e00008 \tjr\tra\n"
        "80044374:\t00000000 \tnop\n"
        "80044378:\t27bdffc8 \taddiu\tsp,sp,-56\n"   # calc_fc_frame: NO header line
        "8004437c:\tafb40020 \tsw\ts4,32(sp)\n"
        "80044380:\t00a0a021 \tmove\ts4,a1\n"
    )
    sym = {0x80044360: "prev_func", 0x80044378: "calc_fc_frame"}
    d = bc._disasm_by_address(text, sym)
    check("header-less func recovered", "calc_fc_frame" in d)
    check("synthesized header", d["calc_fc_frame"].splitlines()[0] == "80044378 <calc_fc_frame>:")
    check("owns its insns", "addiu\tsp,sp,-56" in d["calc_fc_frame"])
    check("stops at next boundary not present here", "8004437c" in d["calc_fc_frame"])
    check("prev func owns earlier insns", "80044370" in d.get("prev_func", ""))


def test_disasm_by_func_real():
    if not bc.ELF.exists():
        check("SKIP disasm_by_func (build/bb2.elf absent)", True)
        return
    d = bc.disasm_by_func()
    if not d:
        check("SKIP disasm_by_func (objdump unavailable)", True)
        return
    check("objdump returns >1000 funcs", len(d) > 1000)
    check("known func has asm", "func_80016A8C" in d and bool(d["func_80016A8C"]))
    check("addr parsed from header", bc.addr_from_disasm(d["func_80016A8C"]) is not None)
    # address-sliced path recovers MORE STT_FUNC names than header parsing
    # (header parsing drops functions objdump emits without a `<name>:` separator).
    import board_sync
    sym = board_sync._elf_func_symbols(bc.ELF)
    func_names = set(sym.values())
    d2 = bc.disasm_by_func(sym_by_addr=sym)
    header_funcs = func_names & set(d)
    addr_funcs = func_names & set(d2)
    check("address-sliced recovers >= header STT_FUNC names", len(addr_funcs) >= len(header_funcs))
    check("address-sliced has known func", "func_80016A8C" in d2)


def test_collect_cheats_real_cache():
    cache = bc.load_rule_cache()
    # Find a queue func that actually has rules and assert we collect >=1.
    items = bc.load_queue_items()
    target = next((it for it in items if it.get("rules", 0) > 0), None)
    if target is None:
        check("SKIP collect_cheats (no func with rules)", True)
        return
    cheats = bc.collect_func_cheats(target["func"], cache)
    check("collected at least one cheat", len(cheats) >= 1)
    check("cheat has required keys",
          all(set(c) >= {"type", "rule_text", "description", "source_file"} for c in cheats))


def main():
    test_small_body_not_truncated()
    test_huge_asm_truncated_and_under_limit()
    test_huge_c_and_asm_both_truncate()
    test_truncate_asm_keeps_header_and_note()
    test_truncate_asm_noop_when_small()
    test_brief_active_c()
    test_brief_active_asm()
    test_brief_parked()
    test_brief_completed()
    test_body_completed_label_and_sections()
    test_body_active_with_cheats()
    test_body_active_no_cheats_note()
    test_section_wip_renders_floor_and_resume()
    test_section_techniques_validated_slug()
    test_section_history_caps_at_15()
    test_section_memrefs()
    test_details_block_gfm_blank_line()
    test_c_unavailable_placeholder()
    test_asm_unavailable_placeholder()
    test_next_step_parked_includes_reason()
    test_next_step_active_wip_resume()
    test_build_board_set_merges_queue_and_completed()
    test_build_board_set_park_reason_carried()
    test_read_wip_real_func_8002bea0()
    test_read_wip_string_reviewer_guard()
    test_read_wip_absent_returns_none()
    test_extract_c_body_real_func()
    test_extract_c_body_missing_file()
    test_load_technique_index_real()
    test_disasm_by_address_slices_flat_stream()
    test_disasm_by_func_real()
    test_collect_cheats_real_cache()
    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0


if __name__ == "__main__":
    sys.exit(main())
