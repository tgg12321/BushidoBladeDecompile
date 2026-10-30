"""Engine regression suite — the spine the whole workflow trusts.

Run:  python3 -m engine.test_engine     (or: python3 -m engine.cli test)

Two layers:
  * FAST (pure logic, no build) — always runs. Covers the distance metric, the
    canonical C-vs-asm detection (incl. the c2 GTE-command + region regressions),
    cheat-asm stripping, and the regfix masking. The last two together PROVE the
    cheat-invisibility guarantee: any cheat an agent adds (a keyed regfix rule OR
    an injected __asm__) is stripped before scoring, so it can't move the score.
  * BUILD-READ — runs only when build/bb2.elf is present (else skipped). Pins the
    canonical gate's verdicts on real functions against the linked ELF.

Self-contained runner (no pytest, matching tools/hooks/test_tooling_error_guard.py).
"""
from __future__ import annotations

import contextlib
import io
import json
import os
import tempfile
from pathlib import Path

from engine import canonical, score, inlineasm, cheats, volatile_cheats
from engine import diagnose
from engine import queue as Q
from engine import pipeline as P
from engine import buildconfig as cfg
from engine import completion, buildstamp

_passed = _failed = _skipped = 0


class _SynthAddrs(dict):
    """Every name has an address: the tree's own if it gives one, else a
    stable synthetic one — for tests whose functions are made up."""

    def get(self, k, d=None):
        import zlib
        return super().get(k) or "8%07X" % (zlib.crc32(k.encode()) & 0x0FFFFFFF)


@contextlib.contextmanager
def _synth_addrs():
    """queue.generate()/reopen() refuse an item without an address (owner
    ruling Q39); tests that queue made-up functions give them one.

    This globally patches layer2.addr_index, also consumed by the departures
    audit. Running that audit here is forbidden: synthetic addresses would
    hide missing-address failures.
    """
    from engine import departures, layer2
    from unittest.mock import patch
    real_index, real_of = layer2.addr_index, layer2.addr_of
    layer2.addr_index = lambda: _SynthAddrs(real_index())
    layer2.addr_of = lambda f: real_of(f) or _SynthAddrs().get(f)
    try:
        with patch.object(departures, "unreviewed_departures", side_effect=
                          AssertionError("departures audit inside _synth_addrs")):
            yield
    finally:
        layer2.addr_index, layer2.addr_of = real_index, real_of


def test_synth_addrs_guard() -> None:
    from engine import departures, layer2
    real_index, real_audit = layer2.addr_index, departures.unreviewed_departures
    with _synth_addrs():
        try:
            departures.unreviewed_departures()
        except AssertionError as exc:
            eq("synthetic addresses forbid departures audit", str(exc),
               "departures audit inside _synth_addrs")
        else:
            check("synthetic addresses forbid departures audit", False)
    check("synthetic addresses restore the index", layer2.addr_index is real_index)
    check("synthetic addresses restore the audit", departures.unreviewed_departures is real_audit)


@contextlib.contextmanager
def _stub_tus(root, sources):
    """Give synthetic queue items real TUs without bypassing the live walk."""
    from engine import gtemacro
    from unittest.mock import patch
    root = Path(root).resolve()
    for stem, text in sources.items():
        path = root / f"src/{stem}.c"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    with patch.object(gtemacro, "REPO_ROOT", root):
        yield


def check(desc: str, cond: bool) -> None:
    global _passed, _failed
    if cond:
        _passed += 1
    else:
        _failed += 1
        print(f"  FAIL: {desc}")


def eq(desc: str, got, want) -> None:
    check(f"{desc} (got {got!r}, want {want!r})", got == want)


def skip(desc: str, why: str) -> None:
    global _skipped
    _skipped += 1
    print(f"  SKIP: {desc} ({why})")


def _dline(addr: str, byts: str, mn: str, ops: str = "") -> str:
    """Synthesize an `objdump -d` instruction line (addr:\\tbytes \\tmn\\tops)."""
    return f"{addr}:\t{byts} \t{mn}\t{ops}".rstrip()


# --------------------------------------------------------------------------
# canonical — C-vs-asm detection (the gate I reworked; 2 bugs lived here)
# --------------------------------------------------------------------------

def test_canonical() -> None:
    # An explicit whole-function grant outranks opcode/distance heuristics.
    # Read the real list format in an isolated cwd, including trailing comments.
    with tempfile.TemporaryDirectory() as td:
        cwd = os.getcwd()
        try:
            os.chdir(td)
            Path("inline_asm_canonical.txt").write_text(
                "# grants\n\nlisted # owner evidence\nmixed # region grant\n", encoding="utf-8")
            Path("tools").mkdir()
            Path("tools/canonical_asm_regions.json").write_text(
                json.dumps({"schema": 2, "functions": {"mixed": {"file": "fake", "sha256": []}}}),
                encoding="utf-8")
            for hits, distance in (([], None), ([], 600), ([(0, "c2", "GTE")], 0)):
                r = canonical._verdict("listed", hits, 8, distance=distance)
                eq("canonical grant: overrides opcode/distance heuristics",
                   r["verdict"], "ASM-WHOLE")
                check("canonical grant: reports its authority",
                      "inline_asm_canonical.txt" in r["reason"])
                eq("canonical grant: preserves detected instruction count",
                   r["asm_insns"], len(hits))
                eq("canonical grant: reviewed mixed body stays partial",
                   canonical._verdict("mixed", hits, 8, distance=distance)["verdict"],
                   "ASM-PARTIAL")
            eq("canonical grant: an unlisted ordinary function stays C",
               canonical._verdict("unlisted", [], 8)["verdict"], "C")
            Path("inline_asm_canonical.txt").unlink()
            eq("canonical grant: removing the list removes the grant",
               canonical._verdict("listed", [], 8)["verdict"], "C")
        finally:
            os.chdir(cwd)

    lines = [
        _dline("8002ec84", "c9890000", "lwc2", "$9,0(t4)"),   # GTE transfer
        _dline("80018500", "4aa00428", "c2", "0xa00428"),      # GTE command (regression)
        _dline("80018504", "27180010", "addiu", "t8,t8,16"),   # ordinary C
        _dline("800831d4", "0040809c", ".word", "0x40809c"),   # raw .word
        _dline("80010000", "0000000c", "syscall"),             # BIOS trampoline
        "80018500 <foo>:",                                     # header — ignored
        "",                                                    # blank — ignored
    ]
    hits, total, structural = canonical._detect(lines)
    eq("detect: total instruction count", total, 5)
    eq("detect: structural (nop/jr) count", structural, 0)
    reasons = {mn for _, mn, _ in hits}
    eq("detect: 4 definitive-asm hits", len(hits), 4)
    check("detect: c2 GTE-command detected (the regression)", "c2" in reasons)
    check("detect: lwc2 transfer detected", "lwc2" in reasons)
    check("detect: .word detected", ".word" in reasons)
    check("detect: syscall detected", "syscall" in reasons)
    check("detect: ordinary addiu NOT flagged", "addiu" not in reasons)

    # Whole-body signals (inline-asm audit A8, 2026-10-01).
    tramp = [_dline("80078948", "240a00a0", "li", "t2,160"),
             _dline("8007894c", "01400008", "jr", "t2"),
             _dline("80078950", "24090043", "li", "t1,67"),
             _dline("80078954", "00000000", "nop")]
    h, t, st = canonical._detect(tramp)
    eq("detect: BIOS vector tail-jump -> ASM-WHOLE",
       canonical._verdict("Exec", h, t, structural=st)["verdict"], "ASM-WHOLE")
    fptr = [_dline("80010000", "24020a00", "li", "v0,160"),
            _dline("80010004", "0040f809", "jalr", "v0"),
            _dline("80010008", "00000000", "nop")]
    eq("detect: a C call through a constant pointer is NOT flagged",
       canonical._detect(fptr)[0], [])
    eq("detect: break 7 (GCC div-by-zero trap) NOT flagged",
       canonical._detect([_dline("80010000", "0007000d", "break", "0x7")])[0], [])
    check("detect: break with a non-GCC code flagged",
          len(canonical._detect([_dline("80010000", "0001000d", "break", "0x1")])[0]) == 1)
    slot = [_dline("80010000", "14400003", "bnez", "v0,80010010"),
            _dline("80010004", "c8800000", "lwc2", "$0,0(a0)"),
            _dline("80010008", "27bdfff0", "addiu", "sp,sp,-16"),
            _dline("8001000c", "27bd0010", "addiu", "sp,sp,16"),
            _dline("80010010", "27bd0010", "addiu", "sp,sp,16")]
    h, t, st = canonical._detect(slot)
    eq("detect: cop2 op in a branch delay slot -> ASM-WHOLE",
       canonical._verdict("f", h, t, structural=st)["verdict"], "ASM-WHOLE")
    s7 = [_dline("80010000", "26100018", "addiu", "s0,s0,24"),
          _dline("80010004", "03e00008", "jr", "ra"),
          _dline("80010008", "00000000", "nop")]
    h, t, st = canonical._detect(s7)
    eq("detect: unsaved callee-saved use -> ASM-WHOLE",
       canonical._verdict("f", h, t, structural=st)["verdict"], "ASM-WHOLE")
    saved = [_dline("80010000", "afb00010", "sw", "s0,16(sp)"),
             _dline("80010004", "26100018", "addiu", "s0,s0,24"),
             _dline("80010008", "8fb00010", "lw", "s0,16(sp)")]
    eq("detect: saved callee-saved use NOT flagged", canonical._detect(saved)[0], [])
    check("verdict: whole-mark stripped from the reported reasons",
          not any(r.startswith(canonical.WHOLE_MARK) for r in
                  canonical._verdict("Exec", *canonical._detect(tramp)[:2])["reasons"]))

    # _verdict: opcode fraction
    eq("verdict: all-asm -> ASM-WHOLE",
       canonical._verdict("f", [(0, "c2", "x")], 1)["verdict"], "ASM-WHOLE")
    eq("verdict: sparse asm -> ASM-PARTIAL",
       canonical._verdict("f", [(0, "c2", "x")], 5)["verdict"], "ASM-PARTIAL")
    # GTE leaf wrapper: 5 GTE ops + 2 nop + 1 jr = 8 total, 3 structural. frac
    # 5/8=0.625 < 0.8, but every NON-structural insn is canonical -> ASM-WHOLE.
    eq("verdict: pure GTE leaf (struct-excluded) -> ASM-WHOLE",
       canonical._verdict("f", [(i, "c2", "x") for i in range(5)], 8, structural=3)["verdict"],
       "ASM-WHOLE")
    eq("verdict: GTE + 1 real C insn stays ASM-PARTIAL",
       canonical._verdict("f", [(i, "c2", "x") for i in range(4)], 7, structural=2)["verdict"],
       "ASM-PARTIAL")
    # _verdict: distance tiers (no opcode signal)
    eq("verdict: d0 -> C",
       canonical._verdict("f", [], 8, distance=0)["verdict"], "C")
    eq("verdict: d60 -> ASM-SUSPECT",
       canonical._verdict("f", [], 80, distance=60)["verdict"], "ASM-SUSPECT")
    # 2026-06-09: distance > 500 alone no longer routes to ASM-STRUCTURAL.
    # The synthetic name "f" has no asm/funcs/f.s, so _hand_coded_tier
    # returns "UNAVAILABLE" (treated as no signal) -> demote to ASM-SUSPECT.
    # See canonical-gate-distance-not-evidence.md.
    eq("verdict: d600 + no hand-coded signal -> ASM-SUSPECT (demoted)",
       canonical._verdict("f", [], 80, distance=600)["verdict"], "ASM-SUSPECT")
    # With a corroborating STRONG/POSSIBLE tier, distance > 500 routes to
    # ASM-STRUCTURAL. Stub _hand_coded_tier to simulate the signal.
    _saved_tier = canonical._hand_coded_tier
    try:
        canonical._hand_coded_tier = lambda f: "STRONG"
        eq("verdict: d600 + tier=STRONG -> ASM-STRUCTURAL",
           canonical._verdict("g", [], 80, distance=600)["verdict"], "ASM-STRUCTURAL")
        canonical._hand_coded_tier = lambda f: "POSSIBLE"
        eq("verdict: d600 + tier=POSSIBLE -> ASM-STRUCTURAL",
           canonical._verdict("h", [], 80, distance=600)["verdict"], "ASM-STRUCTURAL")
        canonical._hand_coded_tier = lambda f: "TIGHT_C"
        eq("verdict: d600 + tier=TIGHT_C -> ASM-SUSPECT (not enough signal)",
           canonical._verdict("i", [], 80, distance=600)["verdict"], "ASM-SUSPECT")
        # 2026-08-18 re-gate: a scanner-confirmed LOW tier is affirmative
        # counter-evidence (the triage of all 47 suspects found 0 candidates),
        # so the suspicion label is dropped and the item is an ordinary C
        # target — in BOTH distance bands.
        canonical._hand_coded_tier = lambda f: "LOW"
        eq("verdict: d600 + tier=LOW -> C (re-verdict, distance is size)",
           canonical._verdict("j", [], 80, distance=600)["verdict"], "C")
        eq("verdict: d60 + tier=LOW -> C (re-verdict)",
           canonical._verdict("k", [], 80, distance=60)["verdict"], "C")
        canonical._hand_coded_tier = lambda f: "POSSIBLE"
        eq("verdict: d60 + tier=POSSIBLE stays ASM-SUSPECT (below near-certain)",
           canonical._verdict("l", [], 80, distance=60)["verdict"], "ASM-SUSPECT")
        # The tier is RECORDED on every verdict the scanner was consulted for —
        # the evidence must travel with the routing (queue.json carried
        # `hand_coded_tier: null` on all 47 suspects before this).
        canonical._hand_coded_tier = lambda f: "TIGHT_C"
        eq("verdict: tier recorded on the >SUSPECT band too",
           canonical._verdict("m", [], 80, distance=60).get("hand_coded_tier"),
           "TIGHT_C")
    finally:
        canonical._hand_coded_tier = _saved_tier
    eq("verdict: no tier field when the scanner was never consulted",
       "hand_coded_tier" in canonical._verdict("f", [], 8, distance=10), False)
    eq("verdict: distance=None stays C",
       canonical._verdict("f", [], 8, distance=None)["verdict"], "C")

    # Bulk tier prefill: `--all --json` carries only the NON-LOW candidates, so
    # "has an asm file but is absent from the dump" MUST read as LOW, while "no
    # asm file at all" must NOT (it falls through to the single-function path,
    # which reports UNAVAILABLE). This inference is what keeps `queue regen`
    # from re-running the whole-tree scan once per item.
    _saved = (canonical._hand_coded_bulk_tiers, dict(canonical._hand_coded_cache))
    with tempfile.TemporaryDirectory() as td:
        cwd = os.getcwd()
        os.chdir(td)
        try:
            Path("asm/funcs").mkdir(parents=True)
            Path("asm/funcs/known.s").write_text("")
            Path("asm/funcs/plain.s").write_text("")
            canonical._hand_coded_cache.clear()
            canonical._hand_coded_bulk_tiers = lambda: {"known": "STRONG"}
            eq("bulk tier: candidate in the dump keeps its tier",
               canonical._hand_coded_tier("known"), "STRONG")
            eq("bulk tier: asm file present but absent from the dump -> LOW",
               canonical._hand_coded_tier("plain"), "LOW")
            eq("bulk tier: no asm file is UNAVAILABLE, never LOW",
               canonical._hand_coded_tier("nosuchfunc"), "UNAVAILABLE")
        finally:
            os.chdir(cwd)
            canonical._hand_coded_bulk_tiers = _saved[0]
            canonical._hand_coded_cache.clear()
            canonical._hand_coded_cache.update(_saved[1])

    # _regions: contiguous-span collapse
    eq("regions: collapse contiguous spans",
       canonical._regions([0, 1, 2, 5, 6, 9]), [(0, 2), (5, 6), (9, 9)])
    eq("regions: empty", canonical._regions([]), [])


# --------------------------------------------------------------------------
# score — the distance metric the loop optimizes
# --------------------------------------------------------------------------

def test_score() -> None:
    eq("levenshtein: identical -> 0",
       score._levenshtein(["a", "b", "c"], ["a", "b", "c"]), 0)
    eq("levenshtein: one substitution -> 1",
       score._levenshtein(["a", "b", "c"], ["a", "x", "c"]), 1)
    eq("levenshtein: one insertion -> 1",
       score._levenshtein(["a", "b"], ["a", "b", "c"]), 1)
    # the SequenceMatcher-overcount regression: a leading delete is 1, not 2
    eq("levenshtein: leading delete -> 1 (not over-counted)",
       score._levenshtein(["a", "b", "c"], ["b", "c"]), 1)
    eq("levenshtein: empty vs n -> n",
       score._levenshtein([], ["a", "b", "c"]), 3)

    # branch/jump recognition drives control-flow-target masking
    for mn in ("b", "beq", "bne", "bnez", "j", "jal", "jr", "jalr"):
        check(f"_BRANCH matches {mn}", bool(score._BRANCH.match(mn)))
    for mn in ("addiu", "lw", "sw", "lui", "mtc2"):
        check(f"_BRANCH does NOT match {mn}", not score._BRANCH.match(mn))


def _fake_objdump(body: str, func: str = "f", size: int = 0x100):
    """Stand-in for score._objdump serving a one-function object.

    `body` is the `objdump -dr` instruction/relocation block. Lets the masking
    tests pin behaviour on exact objdump text without needing a built object.
    """
    def _run(*args: str) -> str:
        if args[0] == "-t":
            return f"00000000 g     F .text\t{size:08x} {func}\n"
        if args[0] == "-h":
            return f"  0 .text         {size:08x}  00000000  00000000  00000034  2**2\n"
        return body
    return _run


@contextlib.contextmanager
def _stub_objdump(body: str):
    saved = score._objdump
    score._objdump = _fake_objdump(body)
    try:
        yield
    finally:
        score._objdump = saved


def _reloc(addr: str, typ: str, sym: str) -> str:
    """Synthesize an `objdump -dr` relocation line (tab-indented, follows its
    instruction)."""
    return f"\t\t\t{addr}: {typ}\t{sym}"


@contextlib.contextmanager
def _stub_objdump_by_path(bodies: dict):
    """Path-keyed objdump stub — insn_diff reads TWO objects per call, so the
    single-body _stub_objdump cannot express a difference between them."""
    saved = score._objdump
    runners = {p: _fake_objdump(b) for p, b in bodies.items()}

    def _run(*args: str) -> str:
        return runners[args[-1]](*args)
    score._objdump = _run
    try:
        yield
    finally:
        score._objdump = saved


def test_insn_diff() -> None:
    """`sandbox --diff` must answer the question the score cannot: is the
    residual a register-seat difference or a source-level divergence?

    Motivated by _SsSndCrescendo — four sessions of register-allocation work
    against a floor whose surplus instructions were 12/13 source-level. The
    class tally is the part that must not silently invert."""
    def obj(*insns: tuple[str, str]) -> str:
        lines = ["00000000 <f>:"]
        for i, (mn, ops) in enumerate(insns):
            lines.append(_dline(f"{i * 4:4x}", "00000000", mn, ops))
        return "\n".join(lines)

    # 1. Same opcodes, different register seats -> operand-only.
    with _stub_objdump_by_path({
            "ours.o": obj(("lui", "v0,0x8010"), ("lw", "v0,0(v0)"), ("jr", "ra")),
            "ref.o": obj(("lui", "s1,0x8010"), ("lw", "s1,0(s1)"), ("jr", "ra"))}):
        d = score.insn_diff("ours.o", "ref.o", "f")
    eq("insn_diff: reg-seat difference is operand-only", d["operand_only_hunks"], 1)
    eq("insn_diff: reg-seat difference is NOT source-level", d["source_level_hunks"], 0)
    eq("insn_diff: both instruction counts reported",
       (d["target_insns"], d["build_insns"]), (3, 3))

    # 2. A different opcode is a source-level divergence even at equal length —
    #    the case that must never read as "just reallocate a register".
    with _stub_objdump_by_path({
            "ours.o": obj(("addu", "v0,v0,v1"), ("jr", "ra")),
            "ref.o": obj(("sll", "v0,v0,2"), ("jr", "ra"))}):
        d = score.insn_diff("ours.o", "ref.o", "f")
    eq("insn_diff: opcode change is source-level", d["source_level_hunks"], 1)
    eq("insn_diff: opcode change is NOT operand-only", d["operand_only_hunks"], 0)

    # 3. A surplus instruction on our side: unequal runs are source-level, and
    #    the hunk must carry the position a reader needs to find it.
    with _stub_objdump_by_path({
            "ours.o": obj(("lw", "v0,0(a0)"), ("nop", ""), ("jr", "ra")),
            "ref.o": obj(("lw", "v0,0(a0)"), ("jr", "ra"))}):
        d = score.insn_diff("ours.o", "ref.o", "f")
    eq("insn_diff: surplus instruction is source-level", d["source_level_hunks"], 1)
    eq("insn_diff: surplus instruction is located", d["hunks"][0]["build_at"], 1)
    eq("insn_diff: surplus instruction counted on our side only",
       (d["target_insns"], d["build_insns"]), (2, 3))

    # 3b. THE CASCADE TRAP: a branch whose displacement moved because earlier
    #     code changed size. The score masks control-flow targets precisely so
    #     this does not count, so the diff must not dress it up as a
    #     register-allocation difference and send a session chasing it.
    #     (Caught by running --diff on the real _SsVmInit candidate: 7 of 12
    #     hunks were this, all originally mislabelled operand-only.)
    with _stub_objdump_by_path({
            "ours.o": obj(("bnez", "v0,20fc"), ("jr", "ra")),
            "ref.o": obj(("bnez", "v0,2c78"), ("jr", "ra"))}):
        d = score.insn_diff("ours.o", "ref.o", "f")
    eq("insn_diff: moved branch displacement is not-scored", d["not_scored_hunks"], 1)
    eq("insn_diff: moved branch displacement is NOT operand-only",
       d["operand_only_hunks"], 0)
    eq("insn_diff: moved branch displacement is NOT source-level",
       d["source_level_hunks"], 0)
    eq("insn_diff: the not-scored hunk still shows real displacements",
       (d["hunks"][0]["target"], d["hunks"][0]["built"]),
       (["bnez v0,2c78"], ["bnez v0,20fc"]))

    # 3c. A branch difference that SURVIVES masking is a real one — the bound
    #     that stops `not-scored` from swallowing genuine control-flow changes.
    with _stub_objdump_by_path({
            "ours.o": obj(("bnez", "v0,20fc"), ("jr", "ra")),
            "ref.o": obj(("beqz", "v0,2c78"), ("jr", "ra"))}):
        d = score.insn_diff("ours.o", "ref.o", "f")
    eq("insn_diff: changed branch OPCODE is still source-level",
       (d["source_level_hunks"], d["not_scored_hunks"]), (1, 0))

    # 4. Identical objects produce no hunks at all (the score-0 case).
    same = obj(("lw", "v0,0(a0)"), ("jr", "ra"))
    with _stub_objdump_by_path({"ours.o": same, "ref.o": same}):
        d = score.insn_diff("ours.o", "ref.o", "f")
    eq("insn_diff: identical objects have no hunks", d["hunks"], [])

    # 5. diagnose.diff_pairs delegates here — the two views must not drift.
    with _stub_objdump_by_path({
            "ours.o": obj(("addu", "v0,v0,v1"), ("jr", "ra")),
            "ref.o": obj(("sll", "v0,v0,2"), ("jr", "ra"))}):
        pairs, nt, nb = diagnose.diff_pairs("ours.o", "ref.o", "f")
    eq("diff_pairs: still returns (tag, target, built) triples",
       (pairs[0][0], pairs[0][1], pairs[0][2]),
       ("replace", ["sll v0,v0,2"], ["addu v0,v0,v1"]))
    eq("diff_pairs: still returns both lengths", (nt, nb), (2, 2))


def test_score_section_addend_mask() -> None:
    """Section-relative HI16/LO16 addends are link-resolved offsets into this
    object's OWN .text/.rodata, so cheat-stripping earlier code in the TU moves
    the referenced static and changes the addend with nothing about this
    function's codegen having changed. Masking them removes the false +1 that
    mis-ranked saEft00Add at the queue top and made `queue regen` resurrect its
    completion (memory/sandbox-lo16-text-addend-false-distance.md).

    Pinned BOTH ways: the artifact shape scores 0, and every genuinely
    different immediate still scores nonzero."""
    def obj(addend: str, sym: str = ".text", typ: str = "R_MIPS_LO16") -> str:
        return "\n".join([
            "00000000 <f>:",
            _dline("   0", "3c040000", "lui", "a0,0x0"),
            _reloc("0", "R_MIPS_HI16", sym),
            _dline("   4", "24840000", "addiu", f"a0,a0,{addend}"),
            _reloc("4", typ, sym),
            _dline("   8", "03e00008", "jr", "ra"),
        ])

    # 1. THE ARTIFACT: identical instruction stream, addend shifted by the
    #    0x9B0 layout delta cheat-stripping introduced. Must score 0.
    with _stub_objdump(obj("5652")):
        honest = score.normalized_insns("honest.o", "f")
    with _stub_objdump(obj("8132")):
        target = score.normalized_insns("target.o", "f")
    eq("section-addend: la-form addend shift scores 0 (the saEft00Add artifact)",
       score._levenshtein(target, honest), 0)
    check("section-addend: the masked operand names its section",
          any(op.endswith("@.text") for op in honest))

    # 2. FALSE-NEGATIVE BOUND, part 1 — a plain immediate with NO relocation is
    #    untouched. A wrong constant still counts.
    def plain(imm: str) -> str:
        return "\n".join([
            "00000000 <f>:",
            _dline("   0", "24840000", "addiu", f"a0,a0,{imm}"),
            _dline("   4", "03e00008", "jr", "ra"),
        ])
    with _stub_objdump(plain("16")):
        a = score.normalized_insns("a.o", "f")
    with _stub_objdump(plain("24")):
        b = score.normalized_insns("b.o", "f")
    eq("section-addend: unrelocated immediate difference still scores 1",
       score._levenshtein(a, b), 1)

    # 3. FALSE-NEGATIVE BOUND, part 2 — a NAMED-symbol reloc carries a
    #    source-level addend (`&sym + 2`), not a layout artifact. Not masked.
    with _stub_objdump(obj("2", sym="g_thing")):
        a = score.normalized_insns("a.o", "f")
    with _stub_objdump(obj("4", sym="g_thing")):
        b = score.normalized_insns("b.o", "f")
    eq("section-addend: named-symbol addend difference still scores 1",
       score._levenshtein(a, b), 1)

    # 3b. NAMED-SYMBOL RESOLUTION (owner ruling 2026-09-04, Ruling B.4): when the
    #     symbol resolves through the linker symbol files, both immediates are
    #     rewritten to the words ld writes, so two spellings of ONE address score
    #     0 and two different addresses still score. Pinned both ways, including
    #     the %hi carry and the unresolvable fallback.
    saved_tab = score._SYMTAB_CACHE
    score._SYMTAB_CACHE = {"D_800F19B8": 0x800F19B8, "Alarm_plus_0x4": 0x800F19BC,
                           "g_known": 0x80100000, "g_alias": 0x80100002,
                           "g_edge": 0x80107FFC}
    try:
        with _stub_objdump(obj("4", sym="D_800F19B8")):
            a = score.normalized_insns("a.o", "f")
        with _stub_objdump(obj("0", sym="Alarm_plus_0x4")):
            b = score.normalized_insns("b.o", "f")
        eq("named-addend: D_800F19B8+4 vs Alarm_plus_0x4 scores 0 (the CD_datasync s58 artifact)",
           score._levenshtein(a, b), 0)
        eq("named-addend: lui token is the linked %hi", a[0], "lui a0,@hi(0x800f)")
        eq("named-addend: lo token is the linked %lo", a[1], "addiu a0,a0,@lo(0x19bc)")
        with _stub_objdump(obj("0", sym="D_800F19B8")):
            c = score.normalized_insns("c.o", "f")
        eq("named-addend: D_800F19B8+4 vs D_800F19B8+0 still scores 1",
           score._levenshtein(a, c), 1)
        with _stub_objdump(obj("2", sym="g_known")):
            a = score.normalized_insns("a.o", "f")
        with _stub_objdump(obj("4", sym="g_known")):
            b = score.normalized_insns("b.o", "f")
        eq("named-addend: resolvable &sym+2 vs &sym+4 still scores 1",
           score._levenshtein(a, b), 1)
        with _stub_objdump(obj("0", sym="g_alias")):
            b = score.normalized_insns("b.o", "f")
        eq("named-addend: g_known+2 vs g_alias+0 (same address) scores 0",
           score._levenshtein(a, b), 0)
        # %hi carry: 0x80107FFC + 8 = 0x80108004 -> lui 0x8011, lo 0x8004
        with _stub_objdump(obj("8", sym="g_edge")):
            a = score.normalized_insns("a.o", "f")
        eq("named-addend: %hi carry follows ld's (value+0x8000)>>16",
           a[0], "lui a0,@hi(0x8011)")
        eq("named-addend: %lo of a carried address", a[1], "addiu a0,a0,@lo(0x8004)")
        # unresolvable symbol: literal immediates, previous behaviour
        with _stub_objdump(obj("2", sym="g_unknown")):
            a = score.normalized_insns("a.o", "f")
        with _stub_objdump(obj("4", sym="g_unknown")):
            b = score.normalized_insns("b.o", "f")
        eq("named-addend: unresolvable symbol keeps literal (2 vs 4 scores 1)",
           score._levenshtein(a, b), 1)
        eq("named-addend: unresolvable symbol is not tokenised", a[1], "addiu a0,a0,2")
        # a LO16 with no pending HI16 for its symbol is left literal
        def lo_only(addend: str) -> str:
            return "\n".join([
                "00000000 <f>:",
                _dline("   0", "8c820000", "lw", f"v0,{addend}(a0)"),
                _reloc("0", "R_MIPS_LO16", "D_800F19B8"),
                _dline("   4", "03e00008", "jr", "ra"),
            ])
        with _stub_objdump(lo_only("4")):
            a = score.normalized_insns("a.o", "f")
        eq("named-addend: unpaired LO16 stays literal", a[0], "lw v0,4(a0)")
        # one lui feeding several loads of the same symbol: every load resolves
        def shared_lui(a1: str, a2: str) -> str:
            return "\n".join([
                "00000000 <f>:",
                _dline("   0", "3c010000", "lui", "at,0x0"),
                _reloc("0", "R_MIPS_HI16", "D_800F19B8"),
                _dline("   4", "8c220000", "lw", f"v0,{a1}(at)"),
                _reloc("4", "R_MIPS_LO16", "D_800F19B8"),
                _dline("   8", "8c230000", "lw", f"v1,{a2}(at)"),
                _reloc("8", "R_MIPS_LO16", "D_800F19B8"),
            ])
        with _stub_objdump(shared_lui("4", "8")):
            a = score.normalized_insns("a.o", "f")
        eq("named-addend: shared lui, first load resolved", a[1], "lw v0,@lo(0x19bc)(at)")
        eq("named-addend: shared lui, second load resolved too", a[2], "lw v1,@lo(0x19c0)(at)")
        # mask=False (diagnosis view) is untouched
        with _stub_objdump(obj("4", sym="D_800F19B8")):
            a = score.normalized_insns("a.o", "f", mask=False)
        eq("named-addend: mask=False keeps the raw immediate", a[1], "addiu a0,a0,4")
    finally:
        score._SYMTAB_CACHE = saved_tab

    # 4. The masked token keeps the SECTION, so a reference that moves between
    #    sections is still a difference.
    with _stub_objdump(obj("100", sym=".text")):
        a = score.normalized_insns("a.o", "f")
    with _stub_objdump(obj("100", sym=".rodata")):
        b = score.normalized_insns("b.o", "f")
    eq("section-addend: .text vs .rodata reference still scores 2 (hi+lo)",
       score._levenshtein(a, b), 2)

    # 5. Reloc types whose field is NOT an addend are left to the existing
    #    control-flow masking — R_MIPS_26 against .text is a jump target.
    def jump(tgt: str) -> str:
        return "\n".join([
            "00000000 <f>:",
            _dline("   0", "08000000", "j", f"{tgt} <f+0x{tgt}>"),
            _reloc("0", "R_MIPS_26", ".text"),
            _dline("   4", "00000000", "nop"),
        ])
    with _stub_objdump(jump("a8")):
        a = score.normalized_insns("a.o", "f")
    with _stub_objdump(jump("2c")):
        b = score.normalized_insns("b.o", "f")
    eq("section-addend: branch masking unchanged (j target still scores 0)",
       score._levenshtein(a, b), 0)
    eq("section-addend: j target masked with the control-flow token, not @.text",
       a[0], "j @")

    # 6. THE COST, pinned rather than merely described (layer-2 review
    #    2026-08-07): the offset identifies WHICH object in the section, so two
    #    different .rodata literal-pool loads compare equal. Inert for the
    #    regen / COMPLETED-C decision (both objects come from the same C text —
    #    only layout can differ), LIVE when scoring an edited .c against a
    #    stale reference .o. func_byte_signature and the SHA1 oracle are
    #    unmasked, so this costs a wasted cycle, never a false completion.
    def pool_load(off: str) -> str:
        return "\n".join([
            "00000000 <f>:",
            _dline("   0", "3c010000", "lui", "at,0x0"),
            _reloc("0", "R_MIPS_HI16", ".rodata"),
            _dline("   4", "8c220000", "lw", f"v0,{off}(at)"),
            _reloc("4", "R_MIPS_LO16", ".rodata"),
        ])
    with _stub_objdump(pool_load("32")):
        a = score.normalized_insns("a.o", "f")
    with _stub_objdump(pool_load("124")):
        b = score.normalized_insns("b.o", "f")
    eq("section-addend: KNOWN COST — a different same-section referent "
       "scores 0 (bounded to 74 sites; SHA1 oracle is the real gate)",
       score._levenshtein(a, b), 0)

    # 7. mask=False is the DIAGNOSIS path — it must still show the real addend,
    #    or `diagnose` would report an empty diff for the artifact case.
    with _stub_objdump(obj("5652")):
        raw = score.normalized_insns("honest.o", "f", mask=False)
    check("section-addend: mask=False keeps the literal addend for diagnosis",
          any("5652" in op for op in raw))
    check("section-addend: mask=False emits no @-token at all",
          not any("@" in op for op in raw))


# --------------------------------------------------------------------------
# inlineasm — cheat-asm stripping (half of cheat-invisibility)
# --------------------------------------------------------------------------

def test_inlineasm() -> None:
    eq("_match_paren: simple nesting",
       inlineasm._match_paren("foo(a(b)c)d", 3), 10)
    eq("_match_paren: string-aware (ignores ) in a literal)",
       inlineasm._match_paren('f("a)b")c', 1), 8)

    # cheat-asm GPR injection is stripped; surrounding C is kept
    src3 = 'void f(void){ int x; __asm__ volatile("addu $8,$3,$0"); x = 1; }'
    out3, n3 = inlineasm.strip_cheat_asm_file(src3)
    check("strip: cheat-asm GPR __asm__ removed", "addu $8,$3,$0" not in out3)
    check("strip: cheat-asm count == 1", n3 == 1)
    check("strip: surrounding C kept", "x = 1;" in out3)

    # canonical GTE op (authentic, no C form) -> KEPT
    src2 = 'void g(void){ __asm__ volatile("mtc2 $2,$0"); }'
    out2, n2 = inlineasm.strip_cheat_asm_file(src2)
    check("strip: canonical GTE __asm__ kept", "mtc2 $2,$0" in out2)
    eq("strip: canonical strip count == 0", n2, 0)

    # a #define body must NOT be stripped (would break every use site)
    srcm = '#define BARRIER __asm__ volatile("addu $8,$3,$0")\nvoid h(void){ BARRIER; }'
    outm, _ = inlineasm.strip_cheat_asm_file(srcm)
    check("strip: macro definition left intact", "#define BARRIER __asm__" in outm)

    # register-pin qualifier neutralized, storage-class hint removed
    srcp = 'void k(void){ register int v asm("$16"); v = 0; }'
    outp, np = inlineasm.strip_cheat_asm_file(srcp)
    check("strip: pin asm(\"$16\") qualifier removed", 'asm("$16")' not in outp)
    check("strip: register storage hint removed from pin", "register int v" not in outp)
    check("strip: pinned variable declaration kept as ordinary C", "int v" in outp)
    check("strip: pin + register hint counted", np >= 2)

    # plain register hints are allocator steering and must not affect score
    srcr = (
        "s32 plain(void){\n"
        "    register int v;\n"
        "    const char *s = \"register stays in string\";\n"
        "    // register stays in comment\n"
        "    v = 1;\n"
        "    return v;\n"
        "}\n"
    )
    outr, nr = inlineasm.strip_cheat_asm_file(srcr)
    check("strip: plain register hint removed", "register int v" not in outr)
    check("strip: string register text preserved", "register stays in string" in outr)
    check("strip: comment register text preserved", "register stays in comment" in outr)
    eq("strip: plain register hint counted", nr, 1)
    eq("func_cheat_asm_count: plain register hint counted",
       inlineasm.func_cheat_asm_count(srcr, "plain"), 1)

    # _match_brace: string/comment-aware
    eq("_match_brace: simple", inlineasm._match_brace("{a{b}c}", 0), 7)
    eq("_match_brace: brace in string ignored",
       inlineasm._match_brace('{ x = "}"; }', 0), 12)
    eq("_match_brace: brace in // comment ignored",
       inlineasm._match_brace("{ // }\n}", 0), 8)

    # func_cheat_asm_count: cheats counted only inside the named function's body
    fsrc = (
        "s32 target(u8 *a){\n"
        '    __asm__ volatile("addu $8,$3,$0");\n'   # cheat-asm, inside target
        "    return 0;\n"
        "}\n"
        "void other(void){\n"
        '    __asm__ volatile("addu $9,$4,$0");\n'   # cheat-asm, but a different func
        "}\n"
    )
    eq("func_cheat_asm_count: counts only target's cheats", inlineasm.func_cheat_asm_count(fsrc, "target"), 1)
    eq("func_cheat_asm_count: counts only other's cheats", inlineasm.func_cheat_asm_count(fsrc, "other"), 1)
    eq("func_cheat_asm_count: unknown func -> -1", inlineasm.func_cheat_asm_count(fsrc, "nope"), -1)
    # a pure-C function reports 0 (locatable, no cheat constructs)
    psrc = "s32 pure(void){\n    return 1;\n}\n"
    eq("func_cheat_asm_count: pure-C func -> 0", inlineasm.func_cheat_asm_count(psrc, "pure"), 0)


# --------------------------------------------------------------------------
# gtemacro — header-exact PsyQ GTE macro units (owner scorer ruling 2026-09-25)
# --------------------------------------------------------------------------

_CLOB = '"$12", "$13", "$14", "$15", "memory"'
_MOVE = '__asm__ volatile("move  $12,%0" : : "r"({op}) : ' + _CLOB + ');'
_MTC2 = '__asm__ volatile("mtc2  $12,$30" : : : ' + _CLOB + ');'
_NOP = '__asm__ volatile("nop   " : : : ' + _CLOB + ');'
_SWC2 = '__asm__ volatile("swc2  $31,($12)" : : : ' + _CLOB + ');'


def _gte_src(*stmts: str) -> str:
    return ("void f(s32 n) {\n    s32 lz;\n    lz = 0;\n    "
            + "\n    ".join(stmts) + "\n    lz += n;\n}\n")


def test_gte_macro_units() -> None:
    from engine import gtemacro

    # The pinned header text is byte-exact (excerpt hashes re-checked here).
    for hdr in gtemacro.PINNED:
        for name, _lines, sha, text in hdr["macros"]:
            eq(f"gtemacro: pinned excerpt {name} hash", gtemacro.excerpt_sha256(text), sha)
    macros = gtemacro.pinned_macros()
    eq("gtemacro: gte_Lzc expands to six statements", len(macros["gte_Lzc"][1]), 6)
    eq("gtemacro: gte_ldlzc expands to two statements", len(macros["gte_ldlzc"][1]), 2)

    def kept(src):
        return [m for _s, _e, m in gtemacro.unit_spans(src)]

    def as_today(desc, src):
        """Negative: the sandbox strip is byte-identical to the pre-ruling strip."""
        eq(f"gte unit NEGATIVE ({desc}): nothing recognized", kept(src), [])
        eq(f"gte unit NEGATIVE ({desc}): stripped exactly as today",
           inlineasm.strip_cheat_asm_file(src, keep_gte_macro_units=True),
           inlineasm.strip_cheat_asm_file(src))

    lzc = [_MOVE.format(op="n"), _MTC2, _NOP, _NOP, _MOVE.format(op="&lz"), _SWC2]

    # POSITIVE: verbatim six-statement gte_Lzc — whitespace/separator spelling may
    # differ, comments may sit between statements; the unit is kept whole.
    six = _gte_src(
        '__asm__ volatile("move   $12, %0\\n" : : "r"(n) : ' + _CLOB + ');',
        "/* gte_ldlzc -> mtc2 */", _MTC2,
        '__asm__ volatile ("nop" :::' + _CLOB + ');', _NOP,
        _MOVE.format(op="& lz"), _SWC2)
    eq("gte unit POSITIVE: six-statement gte_Lzc recognized", kept(six), ["gte_Lzc"] * 6)
    out, n = inlineasm.strip_cheat_asm_file(six, keep_gte_macro_units=True)
    eq("gte unit POSITIVE: gte_Lzc kept whole (sandbox strips nothing)", (out, n), (six, 0))
    out0, n0 = inlineasm.strip_cheat_asm_file(six)
    eq("gte unit: default mode still strips the 4 GPR-only statements", n0, 4)
    eq("gte unit: scoring is not admission — completion gate still counts 4",
       inlineasm.func_cheat_asm_count(six, "f"), 4)

    # POSITIVE: two-statement inline_o.h gte_ldlzc followed by ordinary C.
    ld = _gte_src(_MOVE.format(op="n"), _MTC2)
    eq("gte unit POSITIVE: gte_ldlzc recognized", kept(ld), ["gte_ldlzc"] * 2)
    eq("gte unit POSITIVE: gte_ldlzc kept whole",
       inlineasm.strip_cheat_asm_file(ld, keep_gte_macro_units=True), (ld, 0))

    # A unit does not shelter anything else in the file.
    mixed = _gte_src(*lzc) + ("void g(s32 n) {\n    "
                              + _MOVE.format(op="n") + "\n}\n")
    outm, nm = inlineasm.strip_cheat_asm_file(mixed, keep_gte_macro_units=True)
    eq("gte unit: unit in f kept, stray move in g stripped", nm, 1)
    eq("gte unit: exactly two `move` statements survive", outm.count("move  $12"), 2)

    # POSITIVE (owner amendment 2026-09-25, second batch): `0($12)` for the
    # header's `($12)` — maspsx cannot parse the bare form — is equal.
    swc2_0 = _SWC2.replace("($12)", "0($12)")
    six0 = _gte_src(*lzc[:5], swc2_0)
    eq("gte unit POSITIVE: gte_Lzc with 0($12) recognized", kept(six0), ["gte_Lzc"] * 6)
    eq("gte unit POSITIVE: gte_Lzc with 0($12) kept whole",
       inlineasm.strip_cheat_asm_file(six0, keep_gte_macro_units=True), (six0, 0))
    st0 = _gte_src(_MOVE.format(op="&lz"), swc2_0)
    eq("gte unit POSITIVE: gte_stlzc with 0($12) recognized", kept(st0), ["gte_stlzc"] * 2)
    eq("gte unit POSITIVE: gte_stlzc with 0($12) kept whole",
       inlineasm.strip_cheat_asm_file(st0, keep_gte_macro_units=True), (st0, 0))
    ik = gtemacro._instr_key
    eq("0(reg) key: swc2 0($12) == ($12)", ik("swc2 $31,0($12)"), ik("swc2 $31,($12)"))
    check("0(reg) key: header 4($12) != 0($12)", ik("lwc2 $10,4($12)") != ik("lwc2 $10,0($12)"))
    check("0(reg) key: header 4($12) != ($12)", ik("lwc2 $10,4($12)") != ik("lwc2 $10,($12)"))
    check("0(reg) key: not outside a memory operand (mtc2 0($30))",
          ik("mtc2 $12,0($30)") != ik("mtc2 $12,($30)"))
    check("0(reg) key: not in a non-memory position of a load/store",
          ik("swc2 0($31),($12)") != ik("swc2 ($31),($12)"))

    # NEGATIVES — each stripped exactly as today.
    as_today("lone byte-identical move $12,%0", _gte_src(_MOVE.format(op="n")))
    for bad in ("4($12)", "0x0($12)", "00($12)", "-0($12)", "+0($12)", "0($13)"):
        as_today(f"{bad} for the header's ($12)",
                 _gte_src(*lzc[:5], _SWC2.replace("($12)", bad)))
    as_today("0(...) rewrite outside a memory operand (mtc2)",
             _gte_src(_MOVE.format(op="n"), _MTC2.replace("$12,$30", "$12,0($30)")))
    as_today("0(...) rewrite outside a memory operand (move)",
             _gte_src(_MOVE.format(op="n").replace("%0", "0(%0)"), _MTC2))
    as_today("dropped clobber",
             _gte_src(_MOVE.format(op="n").replace(', "memory"', ""), _MTC2))
    as_today("changed constraint",
             _gte_src(_MOVE.format(op="n").replace('"r"(', '"g"('), _MTC2))
    as_today("changed register in the template",
             _gte_src(_MOVE.format(op="n").replace("$12,%0", "$13,%0"), _MTC2))
    as_today("extra nop appended to gte_Lzc", _gte_src(*lzc, _NOP))
    as_today("extra nop appended to gte_ldlzc", _gte_src(_MOVE.format(op="n"), _MTC2, _NOP))
    as_today("extra nop inserted", _gte_src(_MOVE.format(op="n"), _NOP, _MTC2))
    as_today("reordered unit", _gte_src(_MTC2, _MOVE.format(op="n")))
    as_today("partial gte_Lzc (no final swc2)", _gte_src(*lzc[:5]))
    as_today("unit split by an intervening C statement",
             _gte_src(_MOVE.format(op="n"), "lz = 1;", _MTC2))
    as_today("gte_Lzc split between its nops", _gte_src(*lzc[:3], "lz = 1;", *lzc[3:]))
    as_today("standalone gte_nop()", _gte_src(_NOP))
    as_today("hardcoded-$N statement injected between macro statements",
             _gte_src(_MOVE.format(op="n"), '__asm__ volatile("addu $4,$2,$0");', _MTC2))
    as_today("hardcoded-$N injected inside gte_Lzc",
             _gte_src(*lzc[:4], '__asm__ volatile("move $4,$2");', *lzc[4:]))
    as_today("first statement guarded by if",
             _gte_src("if (n) " + _MOVE.format(op="n"), _MTC2))
    as_today("second statement commented out",
             _gte_src(_MOVE.format(op="n"), "/* " + _MTC2 + " */"))
    as_today("non-volatile statement",
             _gte_src(_MOVE.format(op="n").replace(" volatile", ""), _MTC2))
    as_today("unit reached through a #define in the source",
             "#define LZC(x) \\\n    " + _MOVE.format(op="x") + " \\\n    " + _MTC2
             + "\n" + _gte_src("LZC(n);"))

    # --- owner ruling 2026-09-26 (func_8002DE20, Q11): gte_ldv0 / gte_rtv0 /
    # gte_stlvnl / gte_ApplyRotMatrix pinned; back-to-back macros; the DMPSX
    # post-pass word equals the header placeholder (DMPSX_WORDS only).
    eq("gtemacro: gte_ApplyRotMatrix expands to ten statements",
       len(macros["gte_ApplyRotMatrix"][1]), 10)
    eq("gtemacro: UNIT_DMPSX_WORDS is exactly the evidenced pairs (Q11 rtv0; Q29 rtv0tr/sqr0/gpf0/gpl12)",
       gtemacro.UNIT_DMPSX_WORDS, {0x0000013F: 0x4A486012, 0x0000027F: 0x4A480012,
                              0x00000F3F: 0x4AA00428, 0x000012FF: 0x4B90003D,
                              0x0000133F: 0x4BA8003E})

    def stmt(t, op=None):
        ins = f'"r"({op})' if op else ""
        return f'__asm__ volatile ("{t}": :{ins}:' + _CLOB.replace(" ", "") + ');'
    ldv0 = [stmt("move  $12,%0", "&v"), stmt("lwc2  $0,($12)"), stmt("lwc2  $1,4($12)")]
    rtv0_ph = [stmt("nop   "), stmt("nop   "), stmt(".word 0x0000013f")]
    rtv0 = [stmt("nop   "), stmt("nop   "), stmt(".word 0x4A486012")]
    stl = [stmt("move  $12,%0", "&out"), stmt("swc2  $25,($12)"),
           stmt("swc2  $26,4($12)"), stmt("swc2  $27,8($12)")]

    def vsrc(*stmts: str) -> str:
        return ("void f(s32 n) {\n    s32 v[2];\n    s32 out[3];\n    v[0] = n;\n    "
                + "\n    ".join(stmts) + "\n    out[0] += n;\n}\n")

    eq("gte unit POSITIVE: gte_ldv0 (header ($12)) recognized", kept(vsrc(*ldv0)), ["gte_ldv0"] * 3)
    eq("gte unit POSITIVE: gte_stlvnl recognized", kept(vsrc(*stl)), ["gte_stlvnl"] * 4)
    eq("gte unit POSITIVE: gte_rtv0 with the header placeholder recognized",
       kept(vsrc(*rtv0_ph)), ["gte_rtv0"] * 3)
    eq("gte unit POSITIVE: gte_rtv0 with the post-DMPSX word recognized",
       kept(vsrc(*rtv0)), ["gte_rtv0"] * 3)
    pair = vsrc(*ldv0, *rtv0)
    eq("gte unit POSITIVE: back-to-back gte_ldv0 + gte_rtv0 split into two units",
       kept(pair), ["gte_ldv0"] * 3 + ["gte_rtv0"] * 3)
    eq("gte unit POSITIVE: back-to-back pair kept whole",
       inlineasm.strip_cheat_asm_file(pair, keep_gte_macro_units=True), (pair, 0))
    triple = vsrc(*ldv0, *rtv0, *stl)
    eq("gte unit POSITIVE: ldv0+rtv0+stlvnl is one gte_ApplyRotMatrix",
       kept(triple), ["gte_ApplyRotMatrix"] * 10)
    split = vsrc(*ldv0, *rtv0, "out[1] = n;", *stl)
    eq("gte unit POSITIVE: ldv0+rtv0, C, stlvnl -> three units",
       kept(split), ["gte_ldv0"] * 3 + ["gte_rtv0"] * 3 + ["gte_stlvnl"] * 4)
    eq("gte unit: scoring is not admission — gate still counts the 4 GPR-only statements",
       inlineasm.func_cheat_asm_count(split, "f"), 4)
    ik = gtemacro._instr_key
    eq("dmpsx key: post word == placeholder", ik(".word 0x4A486012"), ik(".word 0x0000013f"))
    check("dmpsx key: another GTE word is not the rtv0 placeholder",
          ik(".word 0x4A486013") != ik(".word 0x0000013f"))
    check("dmpsx key: a non-8-digit spelling of the word is not equal",
          ik(".word 0x04A486012") != ik(".word 0x0000013f"))

    # NEGATIVES (Q11 additions) — each stripped exactly as today.
    as_today("rtv0 with a different GTE word", vsrc(*rtv0[:2], stmt(".word 0x4A486013")))
    as_today("rtv0 missing a nop", vsrc(rtv0[0], rtv0[2]))
    as_today("standalone gte_nop left between two units",
             vsrc(*ldv0, stmt("nop   "), *rtv0))
    as_today("hardcoded-$N statement between back-to-back units",
             vsrc(*ldv0, '__asm__ volatile("move $4,$2");', *rtv0))
    as_today("back-to-back run with a partial trailing macro",
             vsrc(*ldv0, *rtv0, *stl[:3]))
    as_today("DMPSX word outside a placeholder position (appended to ldv0)",
             vsrc(*ldv0, stmt(".word 0x4A486012")))

    # --- owner ruling 2026-09-28 (func_800187F4, Q29): gte_ldlvl / gte_lddp /
    # gte_rtv0tr / gte_sqr0 / gte_gpf0 / gte_gpl12 / gte_stlvl pinned; four more
    # post-DMPSX words equal their header placeholders.
    for name, n in (("gte_ldlvl", 4), ("gte_lddp", 2), ("gte_rtv0tr", 3), ("gte_sqr0", 3),
                    ("gte_gpf0", 3), ("gte_gpl12", 3), ("gte_stlvl", 4)):
        eq(f"gtemacro: {name} expands to {n} statements", len(macros[name][1]), n)
    ldlvl = [stmt("move  $12,%0", "&v"), stmt("lwc2  $9,($12)"),
             stmt("lwc2  $10,4($12)"), stmt("lwc2  $11,8($12)")]
    stlvl = [stmt("move  $12,%0", "&out"), stmt("swc2  $9,($12)"),
             stmt("swc2  $10,4($12)"), stmt("swc2  $11,8($12)")]
    lddp = [stmt("move  $12,%0", "n"), stmt("mtc2  $12,$8")]

    def cmd(word):
        return [stmt("nop   "), stmt("nop   "), stmt(f".word {word}")]
    q29 = (("gte_rtv0tr", "0x0000027f", "0x4A480012"), ("gte_sqr0", "0x00000f3f", "0x4AA00428"),
           ("gte_gpf0", "0x000012ff", "0x4B90003D"), ("gte_gpl12", "0x0000133f", "0x4BA8003E"))
    eq("gte unit POSITIVE: gte_ldlvl recognized", kept(vsrc(*ldlvl)), ["gte_ldlvl"] * 4)
    eq("gte unit POSITIVE: gte_stlvl recognized", kept(vsrc(*stlvl)), ["gte_stlvl"] * 4)
    eq("gte unit POSITIVE: gte_lddp recognized", kept(vsrc(*lddp)), ["gte_lddp"] * 2)
    eq("gte unit POSITIVE: gte_lddp with a constant operand recognized",
       kept(vsrc(stmt("move  $12,%0", "1"), lddp[1])), ["gte_lddp"] * 2)
    for name, ph, post in q29:
        eq(f"gte unit POSITIVE: {name} with the header placeholder recognized",
           kept(vsrc(*cmd(ph))), [name] * 3)
        eq(f"gte unit POSITIVE: {name} with the post-DMPSX word recognized",
           kept(vsrc(*cmd(post))), [name] * 3)
        eq(f"dmpsx key: {name} post word == placeholder", ik(f".word {post}"), ik(f".word {ph}"))
    sq = vsrc(*ldlvl, *cmd("0x4AA00428"), *stl)
    eq("gte unit POSITIVE: ldlvl + sqr0 + stlvnl back to back -> three units",
       kept(sq), ["gte_ldlvl"] * 4 + ["gte_sqr0"] * 3 + ["gte_stlvnl"] * 4)
    eq("gte unit POSITIVE: ldlvl + sqr0 + stlvnl kept whole",
       inlineasm.strip_cheat_asm_file(sq, keep_gte_macro_units=True), (sq, 0))
    gp = vsrc(*lddp, *ldlvl, *cmd("0x4B90003D"), "out[1] = n;", *ldlvl, *lddp,
              *cmd("0x4BA8003E"), *stlvl)
    eq("gte unit POSITIVE: lddp+ldlvl+gpf0, C, ldlvl+lddp+gpl12+stlvl -> seven units",
       kept(gp), ["gte_lddp"] * 2 + ["gte_ldlvl"] * 4 + ["gte_gpf0"] * 3
       + ["gte_ldlvl"] * 4 + ["gte_lddp"] * 2 + ["gte_gpl12"] * 3 + ["gte_stlvl"] * 4)
    eq("gte unit: scoring is not admission — gate still counts the GPR-only statements (Q29 set)",
       inlineasm.func_cheat_asm_count(gp, "f"), 9)
    rt = vsrc(*ldv0, *cmd("0x4A480012"), *stl)
    eq("gte unit POSITIVE: ldv0 + rtv0tr + stlvnl is three units (not gte_ApplyRotMatrix)",
       kept(rt), ["gte_ldv0"] * 3 + ["gte_rtv0tr"] * 3 + ["gte_stlvnl"] * 4)
    check("dmpsx key: the rtv0tr word is not the rtv0 placeholder",
          ik(".word 0x4A480012") != ik(".word 0x0000013f"))
    check("dmpsx key: gte_gpl0's word (sf=0) is not gte_gpl12's placeholder",
          ik(".word 0x4BA0003E") != ik(".word 0x0000133f"))

    # NEGATIVES (Q29 additions) — each stripped exactly as today.
    as_today("sqr0 with a word in no DMPSX pair", vsrc(*cmd("0x4AA00429")))
    as_today("gpl12 spelled with gte_gpl0's word (sf=0)", vsrc(*cmd("0x4BA0003E")))
    as_today("lddp with an edited transfer register", vsrc(lddp[0], stmt("mtc2  $12,$9")))
    as_today("ldlvl missing its third lwc2", vsrc(*ldlvl[:3]))
    as_today("stlvl with two stores reordered", vsrc(stlvl[0], stlvl[2], stlvl[1], stlvl[3]))
    as_today("ldlvl with 4($12) for the header's ($12)",
             vsrc(ldlvl[0], stmt("lwc2  $9,4($12)"), *ldlvl[2:]))
    as_today("sqr0 missing a nop", vsrc(stmt("nop   "), stmt(".word 0x4AA00428")))
    as_today("sqr0 with an extra nop", vsrc(stmt("nop   "), *cmd("0x4AA00428")))
    as_today("gpf0 word appended to ldlvl", vsrc(*ldlvl, stmt(".word 0x4B90003D")))
    as_today("rtv0tr word with a 9-digit spelling", vsrc(*cmd("0x04A480012")))

    # --- gte_ldlv0 / gte_SetRotMatrix pinned (2026-09-26 class grant (A): an engine:
    # commit with layer-2; the func_8002EBDC / func_8002F2D0 / math_ComposeAxisRotationsAndRewriteAngles cluster).
    eq("gtemacro: gte_ldlv0 expands to 7 statements", len(macros["gte_ldlv0"][1]), 7)
    eq("gtemacro: gte_SetRotMatrix expands to 11 statements", len(macros["gte_SetRotMatrix"][1]), 11)
    ldlv0 = [stmt("move  $12,%0", "&v"), stmt("lhu   $14,4($12)"), stmt("lhu   $13,($12)"),
             stmt("sll   $14,$14,16"), stmt("or    $13,$13,$14"), stmt("mtc2  $13,$0"),
             stmt("lwc2  $1,8($12)")]
    srm = [stmt("move  $12,%0", "&m"), stmt("lw    $13,($12)"), stmt("lw    $14,4($12)"),
           stmt("ctc2  $13,$0"), stmt("ctc2  $14,$1"), stmt("lw    $13,8($12)"),
           stmt("lw    $14,12($12)"), stmt("lw    $15,16($12)"), stmt("ctc2  $13,$2"),
           stmt("ctc2  $14,$3"), stmt("ctc2  $15,$4")]
    eq("gte unit POSITIVE: gte_ldlv0 recognized", kept(vsrc(*ldlv0)), ["gte_ldlv0"] * 7)
    eq("gte unit POSITIVE: gte_SetRotMatrix recognized", kept(vsrc(*srm)), ["gte_SetRotMatrix"] * 11)
    eq("gte unit POSITIVE: gte_ldlv0 with 0($12) for the header's ($12) recognized",
       kept(vsrc(ldlv0[0], ldlv0[1], stmt("lhu   $13,0($12)"), *ldlv0[3:])), ["gte_ldlv0"] * 7)
    cl = vsrc(*srm, *ldlv0, *rtv0, "out[1] = n;", *stl)
    eq("gte unit POSITIVE: SetRotMatrix+ldlv0+rtv0, C, stlvnl -> four units",
       kept(cl), ["gte_SetRotMatrix"] * 11 + ["gte_ldlv0"] * 7 + ["gte_rtv0"] * 3 + ["gte_stlvnl"] * 4)
    eq("gte unit POSITIVE: SetRotMatrix+ldlv0+rtv0, C, stlvnl kept whole",
       inlineasm.strip_cheat_asm_file(cl, keep_gte_macro_units=True), (cl, 0))
    eq("gte unit: scoring is not admission — gate still counts the GPR-only statements (cluster set)",
       inlineasm.func_cheat_asm_count(cl, "f"), 14)

    # NEGATIVES (cluster additions) — each stripped exactly as today.
    as_today("ldlv0 with its two lhu reordered", vsrc(ldlv0[0], ldlv0[2], ldlv0[1], *ldlv0[3:]))
    as_today("ldlv0 missing its lwc2", vsrc(*ldlv0[:6]))
    as_today("ldlv0 with an edited or register", vsrc(*ldlv0[:4], stmt("or    $13,$13,$15"), *ldlv0[5:]))
    as_today("SetRotMatrix missing its last ctc2", vsrc(*srm[:10]))
    as_today("SetRotMatrix with 4($12) for the header's ($12)",
             vsrc(srm[0], stmt("lw    $13,4($12)"), *srm[2:]))
    as_today("SetRotMatrix with an extra nop appended", vsrc(*srm, stmt("nop   ")))


# --------------------------------------------------------------------------
# gtemacro — vendored PsyQ headers (owner decisions 2026-10-15, R1-R3)
# --------------------------------------------------------------------------

_VCLOB = '"$12","$13","$14","$15","memory"'
# Fixture headers in the three vendored shapes: inline_o.h (brace blocks of
# asm statements), inline_c.h (one brace-less asm statement) and gtemac.h
# (composites calling the inline header's macros).
_FIX_O = (
    "/*\n * $PSLibId: fixture$\n */\n"
    "#define gte_ldv0(r1) { \\\n"
    f'  __asm__ volatile ("move  $12,%0": :"r"(r1):{_VCLOB}); \\\n'
    f'  __asm__ volatile ("lwc2  $0,($12)": : :{_VCLOB}); \\\n'
    f'  __asm__ volatile ("lwc2  $1,4($12)": : :{_VCLOB}); \\\n'
    "}\n"
    "#define gte_rt() { \\\n"
    f'  __asm__ volatile ("nop   ": : :{_VCLOB}); \\\n'
    f'  __asm__ volatile (".word 0x000000ff": : :{_VCLOB}); \\\n'
    "}\n"
    "#define gte_rtv0tr() { \\\n"
    f'  __asm__ volatile ("nop   ": : :{_VCLOB}); \\\n'
    f'  __asm__ volatile (".word 0x0000027f": : :{_VCLOB}); \\\n'
    "}\n"
    "#define gte_gpl0() { \\\n"
    f'  __asm__ volatile (".word 0x0000137f": : :{_VCLOB}); \\\n'
    "}\n"
    "#define gte_nop() { \\\n"
    f'  __asm__ volatile ("nop   ": : :{_VCLOB}); \\\n'
    "}\n")
_FIX_C = (
    "#define gte_ldv0( r0 ) __asm__ volatile (\t\t\t\\\n"
    '\t"lwc2\t$0, 0( %0 );"\t\t\t\t\t\\\n'
    '\t"lwc2\t$1, 4( %0 )"\t\t\t\t\t\\\n'
    "\t:\t\t\t\t\t\t\t\\\n"
    '\t: "r"( r0 ) )\n\n'
    "#define gte_rt() __asm__ volatile (\t\t\t\t\\\n"
    '\t"nop;"\t\t\t\t\t\t\t\\\n'
    '\t".word 0x000000ff" )\n'
    "#define gte_rt_b() __asm__ volatile ( \".word 0x000000ff\" )\n")
_FIX_MAC = ("#define gte_RotTrans0(r1)\t\t\t\t\t\\\n"
            "\t\t\t\t{\tgte_ldv0(r1);\t\t\\\n"
            "\t\t\t\t\tgte_rt();\t}\n"
            "#define gte_OnlyInC(r1)\t\t\t\t\t\\\n"
            "\t\t\t\t{\tgte_ldv0(r1);\t\t\\\n"
            "\t\t\t\t\tgte_rt_b();\t}\n")


def _vendored_fixture_root(o: str = _FIX_O, c: str = _FIX_C, mac: str = _FIX_MAC) -> str:
    from engine import gtemacro
    root = tempfile.mkdtemp(prefix="bb2_vendored_")
    (Path(root) / "include/psxsdk").mkdir(parents=True)
    for base, text in (("inline_o.h", o), ("inline_c.h", c), ("gtemac.h", mac)):
        if text is not None:
            if base != "gtemac.h":
                text = gtemacro.dmpsx(text)
            (Path(root) / "include/psxsdk" / base).write_bytes(text.encode("ascii"))
    return root


def test_gte_vendored_model() -> None:
    from engine import gtemacro as G

    # --- VENDORED pins: the three files, each with a Sony sha256 and sources.
    eq("vendored: exactly the four PsyQ files",
       sorted(G.VENDORED), ["include/psxsdk/gtemac.h", "include/psxsdk/inline_c.h",
                            "include/psxsdk/inline_o.h", "include/psxsdk/libsn.h"])
    eq("vendored: every VENDORED file has a basename the gates know",
       sorted(Path(r).name for r in G.VENDORED), sorted(G.VENDORED_BASENAMES))
    for rel, meta in G.VENDORED.items():
        check(f"vendored: {rel} pins a sha256", len(meta["sony_sha256"]) == 64
              and int(meta["sony_sha256"], 16) >= 0)
        check(f"vendored: {rel} names its source and a second copy",
              "psyq400.tar.gz" in meta["source"] and meta["second_copy"])
        eq(f"vendored: {rel} is Release 4.0", meta["pslibid"],
           "$PSLibId: Run-time Library Release 4.0$")
    eq("vendored: only the inline headers carry DMPSX words",
       sorted(r for r, m in G.VENDORED.items() if m["dmpsx"]),
       ["include/psxsdk/inline_c.h", "include/psxsdk/inline_o.h"])

    # --- DMPSX_WORDS: every pair is a GTE command, both sources cited.
    eq("dmpsx table: exactly the eleven owner-approved placeholders",
       sorted(G.DMPSX_WORDS), [0x7F, 0xBF, 0xFF, 0x13F, 0x1FF, 0x27F, 0xF3F,
                               0x127F, 0x12BF, 0x12FF, 0x133F])
    check("dmpsx table: gte_gpl0 stays a placeholder",
          0x0000137F not in G.DMPSX_WORDS)
    for ph, (word, macro, nug, psn) in G.DMPSX_WORDS.items():
        check(f"dmpsx table: 0x{ph:08x} is a DMPSX placeholder (n<<6 | 0x3f)",
              ph & 0x3F == 0x3F and ph < 0x2000)
        check(f"dmpsx table: {macro} word 0x{word:08X} is a GTE command (cop2 imm25)",
              word >> 25 == 0x25)
        check(f"dmpsx table: {macro} cites two lines in each source",
              len(nug) == 2 and len(psn) == 2 and all(isinstance(n, int) for n in nug + psn))
    eq("dmpsx table: one macro per placeholder",
       len({m for _w, m, _n, _p in G.DMPSX_WORDS.values()}), 11)
    for ph, word in G.UNIT_DMPSX_WORDS.items():
        eq(f"dmpsx table agrees with the owner-ruled unit word 0x{ph:08x}",
           G.DMPSX_WORDS[ph][0], word)
    eq("dmpsx table: the design's eleven words",
       {w for w, _m, _n, _p in G.DMPSX_WORDS.values()},
       {0x4A180001, 0x4A280030, 0x4A480012, 0x4A486012, 0x4A49E012, 0x4AA00428,
        0x4B70000C, 0x4B98003D, 0x4B90003D, 0x4BA8003E})

    # --- dmpsx / undmpsx: per macro, so the shared word reverses uniquely.
    ven = G.dmpsx(_FIX_O)
    check("dmpsx: gte_rt placeholder -> 0x4A480012", ".word 0x4A480012\"" in ven.split("gte_rtv0tr")[0])
    eq("dmpsx: both rt and rtv0tr carry 0x4A480012", ven.count(".word 0x4A480012"), 2)
    check("dmpsx: gte_gpl0 keeps its placeholder", ".word 0x0000137f" in ven)
    eq("undmpsx: reverses to the Sony text", G.undmpsx(ven), _FIX_O)
    eq("undmpsx: inline_c brace-less + _b variant reverse", G.undmpsx(G.dmpsx(_FIX_C)), _FIX_C)
    eq("dmpsx: _b variant substituted", G.dmpsx(_FIX_C).count(".word 0x4A480012"), 2)
    eq("dmpsx: a word outside a covered macro is untouched",
       G.undmpsx('#define gte_nop() { __asm__ volatile (".word 0x4A480012"); }\n'),
       '#define gte_nop() { __asm__ volatile (".word 0x4A480012"); }\n')

    # --- check_vendored on fixture roots (Sony sha cannot match a fixture).
    root = _vendored_fixture_root()
    issues = G.check_vendored(root)
    eq("check_vendored: a fixture is not Sony's text (three sha issues)",
       sum("not Sony's" in i for i in issues), 3)
    root = _vendored_fixture_root(c=None)
    check("check_vendored: missing file reported",
          any("inline_c.h: vendored PsyQ header missing" in i for i in G.check_vendored(root)))
    root = _vendored_fixture_root(o=_FIX_O.replace("\n", "\r\n"))
    check("check_vendored: CR reported",
          any("inline_o.h: carriage return" in i for i in G.check_vendored(root)))

    # --- header_macros: both forms, composites resolved per inline header.
    root = _vendored_fixture_root()
    mo, mc = G.header_macros("inline_o.h", root), G.header_macros("inline_c.h", root)
    eq("header_macros(inline_o): fixture names",
       sorted(mo), ["gte_RotTrans0", "gte_gpl0", "gte_ldv0", "gte_nop", "gte_rt", "gte_rtv0tr"])
    eq("header_macros: gte_OnlyInC resolves against inline_c only",
       ("gte_OnlyInC" in mo, "gte_OnlyInC" in mc), (False, True))
    eq("header_macros: inline_o gte_ldv0 has three statements", len(mo["gte_ldv0"].stmts), 3)
    eq("header_macros: inline_c gte_ldv0 is one statement, two instructions",
       [len(s.instrs) for s in mc["gte_ldv0"].stmts], [2])
    eq("header_macros: brace-less param binds", mc["gte_ldv0"].stmts[0].inputs,
       (("r", ("param", "r0")),))
    eq("header_macros: composite statements = ldv0 + rt (inline_o)",
       len(mo["gte_RotTrans0"].stmts), 5)
    eq("header_macros: composite header tag", mo["gte_RotTrans0"].header, "gtemac.h")
    eq("header_macros: cop2 flags", (mo["gte_ldv0"].has_cop2, mo["gte_rt"].has_cop2,
                                     mo["gte_nop"].has_cop2, mo["gte_gpl0"].has_cop2),
       (True, True, False, False))
    eq("header_macros: placeholder flag", (mo["gte_gpl0"].placeholder, mo["gte_rt"].placeholder),
       (True, False))
    check("header_macros: a placeholder is not a GTE command", not mo["gte_gpl0"].has_cop2)
    eq("vendored_names: union of the three files",
       sorted(G.vendored_names(root)),
       ["gte_OnlyInC", "gte_RotTrans0", "gte_gpl0", "gte_ldv0", "gte_nop", "gte_rt",
        "gte_rt_b", "gte_rtv0tr"])
    try:
        G.header_macros("gtemac.h", root)
        check("header_macros: refuses a non-inline header", False)
    except ValueError:
        check("header_macros: refuses a non-inline header", True)

    # --- tu_includes / tu_header.
    eq("tu_includes: angle and quoted spellings, comments ignored",
       sorted(G.tu_includes('#include <psxsdk/inline_o.h>\n#include "psxsdk/gtemac.h"\n'
                            '/* #include <psxsdk/inline_c.h> */\n')), ["gtemac.h", "inline_o.h"])
    eq("tu_header: inline_o", G.tu_header("#include <psxsdk/inline_o.h>\n"), "inline_o.h")
    eq("tu_header: none", G.tu_header("#include <psxsdk/libgte.h>\n"), None)
    try:
        G.tu_header("#include <psxsdk/inline_o.h>\n#include <psxsdk/inline_c.h>\n")
        check("tu_header: both inline headers refused", False)
    except ValueError:
        check("tu_header: both inline headers refused", True)
    eq("tu_macros: no gtemac.h include -> no composites",
       "gte_RotTrans0" in G.tu_macros("#include <psxsdk/inline_o.h>\n", root), False)
    eq("tu_macros: gtemac.h included -> composites",
       "gte_RotTrans0" in G.tu_macros("#include <psxsdk/inline_o.h>\n"
                                      "#include <psxsdk/gtemac.h>\n", root), True)

    # --- a standalone header (libsn.h): bare `asm`, a trailing comment, a trap.
    sn = ('#define\tpollhost()\tasm("break 1024")\t/* inline to keep variable scope */\n'
          '#define\tgte_fake()\tasm("nop")\n')
    sn_root = _vendored_fixture_root()
    (Path(sn_root) / "include/psxsdk/libsn.h").write_text(sn, newline="\n")
    ms = G.standalone_macros("libsn.h", sn_root)
    eq("standalone: pollhost parses to one break, a trap, admitted",
       ([s.instrs for s in ms["pollhost"].stmts], ms["pollhost"].has_cop2,
        ms["pollhost"].has_trap, ms["pollhost"].admitted), ([("break 1024",)], False, True, True))
    check("standalone: a cop2- and trap-free macro is not admitted", not ms["gte_fake"].admitted)
    check("standalone: a lone nop is not admitted (gte_nop)",
          not G.header_macros("inline_o.h", sn_root)["gte_nop"].admitted)
    try:
        G.standalone_macros("inline_o.h", sn_root)
        check("standalone_macros: refuses a non-standalone header", False)
    except ValueError:
        check("standalone_macros: refuses a non-standalone header", True)
    check("vendored_names: libsn.h names included", "pollhost" in G.vendored_names(sn_root))
    sn_inc = "#include <psxsdk/libsn.h>\n"
    eq("tu_includes: libsn.h", G.tu_includes(sn_inc), frozenset({"libsn.h"}))
    eq("tu_macros: libsn.h alone", sorted(G.tu_macros(sn_inc, sn_root)), ["gte_fake", "pollhost"])
    eq("tu_macros: libsn.h with inline_o.h -> both",
       ("pollhost" in G.tu_macros(sn_inc + "#include <psxsdk/inline_o.h>\n", sn_root),
        "gte_rt" in G.tu_macros(sn_inc + "#include <psxsdk/inline_o.h>\n", sn_root)),
       (True, True))
    eq("call_header: a libsn.h macro is keyed by libsn.h, whatever the inline header",
       (G.call_header(sn_inc, "pollhost", sn_root),
        G.call_header(sn_inc + "#include <psxsdk/inline_o.h>\n", "pollhost", sn_root),
        G.call_header(sn_inc + "#include <psxsdk/inline_o.h>\n", "gte_rt", sn_root)),
       ("libsn.h", "libsn.h", "inline_o.h"))

    # --- call_sites: statement-position calls, compiler view only.
    inc = "#include <psxsdk/inline_o.h>\n#include <psxsdk/gtemac.h>\n"

    def calls(body: str, text_inc: str = inc):
        src = text_inc + "void f(s32 *v) {\n" + body + "\n}\n"
        return [(src[s:e], n) for s, e, n in G.call_sites(src, "f", root)]

    def refused(desc: str, body: str) -> None:
        try:
            calls(body)
            check(f"call_sites refuses {desc}", False)
        except ValueError:
            check(f"call_sites refuses {desc}", True)

    eq("call_sites: two calls", calls("    gte_ldv0(v);\n    gte_rt();"),
       [("gte_ldv0(v);", "gte_ldv0"), ("gte_rt();", "gte_rt")])
    eq("call_sites: span runs through the `;`", calls("    gte_ldv0( v ) ;"),
       [("gte_ldv0( v ) ;", "gte_ldv0")])
    eq("call_sites: comment and string mentions ignored",
       calls('    /* gte_rt(); */ // gte_rt();\n    func_x("gte_rt();");'), [])
    eq("call_sites: a #define body in the function is not a call",
       calls("#define X gte_rt();\n    func_x(1);"), [])
    eq("call_sites: after if / else / label",
       [n for _t, n in calls("    if (v) gte_rt(); else gte_nop();\n    l: gte_ldv0(v);")],
       ["gte_rt", "gte_nop", "gte_ldv0"])
    eq("call_sites: composite", calls("    gte_RotTrans0(v);"), [("gte_RotTrans0(v);", "gte_RotTrans0")])
    eq("call_sites: no direct vendored include -> still found (call_issues refuses)",
       calls("    gte_rt();", "#include <psxsdk/libgte.h>\n"), [("gte_rt();", "gte_rt")])
    eq("call_sites: a backslash-newline inside the name does not hide it",
       calls("    gte_r\\\nt();"), [("gte_r\\\nt();", "gte_rt")])
    eq("call_sites: a call after a directive line is at statement position",
       calls("    func_x(v);\n#ifdef X\n    gte_rt();\n#endif"), [("gte_rt();", "gte_rt")])
    refused("a parenthesized name", "    (gte_nop)();")
    refused("a name passed to a macro", "    CALL(gte_nop);")
    refused("a name taken as a value", "    func_x(gte_rt);")
    eq("call_sites: other functions' names ignored", calls("    func_x(v);\n    gte_rt();"),
       [("gte_rt();", "gte_rt")])
    refused("expression position", "    s32 a = 1 + gte_rt();")
    refused("a call without `;`", "    gte_rt() , func_x(1);")
    refused("a call nested in another's arguments", "    func_x(gte_rt());")
    refused("a call as a for-loop clause", "    for (gte_rt(); ; ) ;")
    eq("call_sites: other function", G.call_sites(inc + "void g(void) {\n    gte_rt();\n}\n",
                                                  "f", root), [])

    # --- the real vendored files (from commit 2 on).
    if not all(Path(r).exists() for r in G.VENDORED):
        skip("vendored: real header pins", "include/psxsdk vendored headers absent")
        return
    eq("vendored: check_vendored() clean on the tree", G.check_vendored(), [])
    here = os.getcwd()
    os.chdir(tempfile.mkdtemp(prefix="bb2_vendored_cwd_"))
    try:
        eq("vendored: reads resolve from the repo root, not the cwd",
           (G.check_vendored(), "gte_rtv0" in G.vendored_names(),
            "gte_rtv0" in G.header_macros("inline_o.h")), ([], True, True))
    finally:
        os.chdir(here)
    import importlib
    import sys
    sys.path.insert(0, str(Path("tools").resolve()))
    fmt = importlib.import_module("format")
    for rel in G.VENDORED:
        check(f"format: {rel} is vendored (never formatted)", fmt.is_vendored(rel))
        text = Path(rel).read_text(encoding="ascii")
        eq(f"format: {rel} returned as it is (clang-format never runs)",
           fmt._format_checked("/nonexistent/clang-format", rel, text), ("ok", text))
    check("format: libgte.h is not vendored", not fmt.is_vendored("include/psxsdk/libgte.h"))
    eq("vendored: define counts inline_o / inline_c / gtemac",
       tuple(len(G._macro_defines(G._read_vendored(str(G.REPO_ROOT), b)))
             for b in ("inline_o.h", "inline_c.h", "gtemac.h")), (174, 241, 47))
    for b in G.INLINE_HEADERS:
        _m, failed = G.parse_header(G._read_vendored(str(G.REPO_ROOT), b), b)
        eq(f"vendored: every {b} #define parses", failed, [])
    own, _f = G.parse_header(G._read_vendored(str(G.REPO_ROOT), "inline_o.h"), "inline_o.h")
    _m, failed = G.parse_header(G._read_vendored(str(G.REPO_ROOT), "gtemac.h"), "gtemac.h", own)
    eq("vendored: gtemac.h composites inline_o.h lacks (inline_c.h-only callees)",
       sorted(n for n, _r in failed), ["gte_OuterProduct0SV", "gte_OuterProduct0SVL",
                                       "gte_OuterProduct12SV", "gte_OuterProduct12SVL"])
    own, _f = G.parse_header(G._read_vendored(str(G.REPO_ROOT), "inline_c.h"), "inline_c.h")
    _m, failed = G.parse_header(G._read_vendored(str(G.REPO_ROOT), "gtemac.h"), "gtemac.h", own)
    eq("vendored: every gtemac.h composite parses against inline_c.h", failed, [])
    for b in G.INLINE_HEADERS:
        ms = G.header_macros(b)
        own = {n: m for n, m in ms.items() if m.header == b and not n.endswith("_b")}
        eq(f"vendored: {b} macros without a GTE command or a placeholder: gte_nop",
           sorted(n for n, m in own.items() if not m.has_cop2 and not m.placeholder),
           ["gte_nop"])
        eq(f"vendored: {b} placeholder-free command macros are the eleven approved",
           sorted(n for n, m in own.items() if not m.placeholder
                  and any(G._DOTWORD_RE.fullmatch(x) for st in m.stmts for x in st.instrs)),
           sorted(m for _w, m, _n, _p in G.DMPSX_WORDS.values()))
        check(f"vendored: {b} unapproved placeholders stay (gte_gpl0, gte_rtv1, gte_ncs)",
              all(own[n].placeholder for n in ("gte_gpl0", "gte_rtv1", "gte_ncs")))
        eq(f"vendored: {b} rtv0 carries the command word",
           ms["gte_rtv0"].stmts[-1].instrs[-1], ".word 0x4A486012")
    sn = G.standalone_macros("libsn.h")
    eq("vendored: libsn.h macros are pollhost / PSYQpause, one admitted break each",
       {n: ([s.instrs for s in m.stmts], m.admitted) for n, m in sn.items()},
       {"pollhost": ([("break 1024",)], True), "PSYQpause": ([("break 1031",)], True)})
    real = G._read_vendored(str(G.REPO_ROOT), "inline_o.h")
    for word in {w for w, _m, _n, _p in G.DMPSX_WORDS.values()}:
        check(f"vendored: command word 0x{word:08X} is not in Sony's text",
              f"0x{word:08X}".lower() not in G.undmpsx(real).lower())
    for rel in G.VENDORED:
        text = Path(rel).read_text(encoding="ascii")
        tmp = tempfile.mkdtemp(prefix="bb2_vendored_edit_")
        for r2 in G.VENDORED:
            (Path(tmp) / r2).parent.mkdir(parents=True, exist_ok=True)
            (Path(tmp) / r2).write_bytes(Path(r2).read_bytes())
        (Path(tmp) / rel).write_bytes(text.replace('"$15","memory"', '"$15"', 1)
                                      .replace("gte_", "gtE_", 1)
                                      .replace("break 1024", "break 1025").encode("ascii"))
        check(f"check_vendored: an edit to {rel} is caught",
              any(rel in i and "not Sony's" in i for i in G.check_vendored(tmp)))
    for rel in ("include/psxsdk/inline_o.h", "include/psxsdk/inline_c.h"):
        tmp = tempfile.mkdtemp(prefix="bb2_vendored_ph_")
        for r2 in G.VENDORED:
            (Path(tmp) / r2).parent.mkdir(parents=True, exist_ok=True)
            (Path(tmp) / r2).write_bytes(Path(r2).read_bytes())
        (Path(tmp) / rel).write_bytes(G.undmpsx(Path(rel).read_text(encoding="ascii"))
                                      .encode("ascii"))
        check(f"check_vendored: {rel} with placeholders left is caught",
              any("not substituted" in i for i in G.check_vendored(tmp)))


def test_gte_call_gates() -> None:
    """The completion gates see vendored macro calls (design section 2)."""
    from engine import gtemacro as G
    if not all(Path(r).exists() for r in G.VENDORED):
        skip("gte call gates", "include/psxsdk vendored headers absent")
        return
    inc_o = "#include <psxsdk/inline_o.h>\n#include <psxsdk/gtemac.h>\n"
    inc_c = "#include <psxsdk/inline_c.h>\n#include <psxsdk/gtemac.h>\n"

    def tu(body: str, inc: str = inc_o, pre: str = "") -> str:
        return (inc + pre + "s32 func_GTE(s32 *v, s32 *out) {\n" + body
                + "\n    return out[0] + 1;\n}\n")

    calls3 = "    gte_ldv0(v);\n    gte_rtv0();\n    gte_stlvnl(out);"
    text = tu(calls3)
    eq("call gate: each call is an island",
       [text[s:e] for s, e in completion.blocks(text, "func_GTE")],
       ["gte_ldv0(v);", "gte_rtv0();", "gte_stlvnl(out);"])
    eq("call gate: asm statements and calls sorted together",
       len(completion.blocks(tu(calls3 + '\n    __asm__ volatile ("ctc2 %0,$13" :: "r"(v));'),
                             "func_GTE")), 4)

    with tempfile.TemporaryDirectory() as td:
        grants = Path(td) / "regions.json"
        old_read, old_regions = inlineasm._read_src_cached, completion.REGIONS
        old_foreign, old_flag = G.foreign_asm_macros, completion.REFUSE_WRITTEN_OUT_UNITS

        def issues(src: str, canonical: bool = True) -> list[str]:
            inlineasm._read_src_cached = lambda stem: src
            return completion.source_issues("fake", "func_GTE", canonical=canonical)

        def grant(src: str) -> None:
            grants.write_text(json.dumps({"schema": 2, "functions": {"func_GTE": {
                "file": "fake", "sha256": completion.region_hashes(src, "func_GTE")}}}))
        try:
            completion.REGIONS = grants
            check("call gate: a non-canonical body with a call is refused",
                  any("requires canonical" in i for i in issues(text, canonical=False)))
            check("call gate: a canonical body without a grant is refused",
                  any("no reviewed region grant" in i for i in issues(text)))
            grant(text)
            eq("call gate: a canonical body of calls passes with its grant", issues(text), [])
            check("call gate: an argument edit voids the grant", any(
                "changed since review" in i for i in issues(text.replace("gte_ldv0(v)",
                                                                         "gte_ldv0(v + 1)"))))
            check("call gate: a macro change voids the grant", any(
                "changed since review" in i for i in issues(text.replace("gte_rtv0()",
                                                                         "gte_rtv0tr()"))))
            switched = tu(calls3, inc_c)
            eq("call gate: inline_c.h defines the same calls", G.call_issues(switched, "func_GTE"), [])
            check("call gate: switching inline_o.h -> inline_c.h voids the grant",
                  any("changed since review" in i for i in issues(switched)))
            check("call gate: region hash differs by header only",
                  completion.region_hashes(text, "func_GTE")
                  != completion.region_hashes(switched, "func_GTE"))
            relaid = text.replace("gte_ldv0(v);", "gte_ldv0( v ); /* load */")
            eq("call gate: a layout/comment change keeps the grant", issues(relaid), [])

            nop = tu(calls3 + "\n    gte_nop();")
            grant(nop)
            check("call gate: standalone gte_nop() refused (no cop2)",
                  any("gte_nop: its expansion has no cop2 or trap" in i for i in issues(nop)))
            src_nop = tu("    out[0] = 1;\n    gte_nop();")
            eq("call gate: gte_nop() counted as cheat asm",
               inlineasm.func_cheat_asm_count(src_nop, "func_GTE"), 1)
            stripped, n = inlineasm.strip_cheat_asm_file(src_nop)
            eq("call gate: gte_nop() stripped for the sandbox", ("gte_nop" in stripped, n), (False, 1))
            kept, n = inlineasm.strip_cheat_asm_file(text)
            eq("call gate: calls with a GTE command are kept and not counted", (kept, n), (text, 0))
            eq("call gate: calls with a GTE command are not cheat asm",
               inlineasm.func_cheat_asm_count(text, "func_GTE"), 0)

            # LIBSN.H pollhost(): a trap, admitted alone or beside an inline header.
            inc_sn = "#include <psxsdk/libsn.h>\n"
            poll = tu("    for (;;) {\n        pollhost();\n    }", inc_sn)
            eq("call gate: pollhost() is an island", [poll[s:e] for s, e in
                                                      completion.blocks(poll, "func_GTE")],
               ["pollhost();"])
            grants.write_text(json.dumps({"schema": 2, "functions": {}}))
            check("call gate: pollhost() without a grant is refused",
                  any("no reviewed region grant" in i for i in issues(poll)))
            grant(poll)
            eq("call gate: pollhost() passes with its grant", issues(poll), [])
            eq("call gate: pollhost() kept for the sandbox, not counted",
               (inlineasm.strip_cheat_asm_file(poll), inlineasm.func_cheat_asm_count(poll, "func_GTE")),
               ((poll, 0), 0))
            check("call gate: PSYQpause() for pollhost() voids the grant", any(
                "changed since review" in i for i in issues(poll.replace("pollhost", "PSYQpause"))))
            mixed = tu(calls3 + "\n    pollhost();", inc_o + inc_sn)
            eq("call gate: pollhost() beside GTE calls", G.call_issues(mixed, "func_GTE"), [])
            check("call gate: pollhost() keyed by libsn.h, GTE calls by the inline header",
                  completion.region_hashes(mixed, "func_GTE")[-1]
                  == completion.region_hashes(tu(calls3 + "\n    pollhost();", inc_c + inc_sn),
                                              "func_GTE")[-1])
            check("call gate: pollhost() without libsn.h is refused",
                  any("pollhost: not defined" in i for i in issues(tu(calls3 + "\n    pollhost();"))))
            check("call gate: pollhost() with no vendored include is refused",
                  any("does not include its header" in i
                      for i in issues(tu("    pollhost();", "#include <psxsdk/libgte.h>\n"))))

            gpl0 = tu(calls3 + "\n    gte_gpl0();")
            check("call gate: a DMPSX placeholder macro refused",
                  any("gte_gpl0: its expansion keeps a DMPSX placeholder" in i for i in issues(gpl0)))

            wrong = tu(calls3 + "\n    gte_rtps_b();")
            check("call gate: a macro the TU's header lacks is refused (inline_c.h-only name)",
                  any("gte_rtps_b: not defined" in i for i in issues(wrong)))
            nocomp = tu("    gte_ApplyRotMatrix(v, out);", "#include <psxsdk/inline_o.h>\n")
            check("call gate: a gtemac.h composite without gtemac.h is refused",
                  any("gte_ApplyRotMatrix: not defined" in i for i in issues(nocomp)))
            onlymac = tu("    gte_ApplyRotMatrix(v, out);", "#include <psxsdk/gtemac.h>\n")
            check("call gate: gtemac.h without an inline header is refused",
                  any("no inline header" in i for i in issues(onlymac)))
            both = tu(calls3, inc_o + "#include <psxsdk/inline_c.h>\n")
            check("call gate: both inline headers in a TU refused",
                  any("both inline_o.h and inline_c.h" in i for i in issues(both)))
            check("call gate: a call in expression position refused",
                  any("not at statement position" in i
                      for i in issues(tu("    out[0] = 1 + gte_rtv0();"))))

            shadow = tu(calls3, pre="#define gte_rtv0() func_x()\n")
            grant(shadow)
            check("call gate: a same-file #define of a vendored name refused",
                  any("redefines vendored macro gte_rtv0" in i for i in issues(shadow)))
            undef = tu(calls3, pre="#undef gte_rtv0\n")
            check("call gate: a same-file #undef of a vendored name refused",
                  any("redefines vendored macro gte_rtv0" in i for i in issues(undef)))
            in_body = tu("#undef gte_rtv0\n    out[0] = 1;", inc="#include <psxsdk/inline_o.h>\n")
            eq("call gate: a shadowing directive in a body counted",
               inlineasm.func_cheat_asm_count(in_body, "func_GTE"), 1)
            stripped, _n = inlineasm.strip_cheat_asm_file(shadow)
            check("call gate: a shadowing #define stripped for the sandbox",
                  "#define gte_rtv0" not in stripped)

            # A vendored macro reached through another name: never an island.
            for desc, pre, use in (
                    ("a same-file wrapper of gte_nop", "#define BAR() gte_nop()\n", "BAR();"),
                    ("a same-file wrapper of gte_rtv0", "#define R() gte_rtv0()\n", "R();"),
                    ("an object-like alias", "#define BAR gte_nop\n", "BAR();"),
                    ("a name passed to a macro", "#define CALL(m) m()\n", "CALL(gte_nop);")):
                src = tu("    out[0] = 1;\n    " + use + "\n    out[1] = 2;", pre=pre)
                eq(f"call gate: {desc} counted", inlineasm.func_cheat_asm_count(src, "func_GTE"), 1)
                full, _n = inlineasm.strip_cheat_asm_file(src)
                body = full[full.index("func_GTE"):]
                check(f"call gate: {desc} stripped for the sandbox",
                      use not in body and "out[1] = 2;" in body)
                check(f"call gate: {desc} refused (non-canonical)",
                      issues(src, canonical=False) != [])
                check(f"call gate: {desc} refused (canonical)", issues(src) != [])
            spliced = tu("    out[0] = 1;\n    gte_n\\\nop();")
            eq("call gate: a spliced gte_nop() counted",
               inlineasm.func_cheat_asm_count(spliced, "func_GTE"), 1)
            check("call gate: a spliced gte_nop() stripped",
                  "gte_n" not in inlineasm.strip_cheat_asm_file(spliced)[0])
            # A token paste can build asm or a vendored name no gate sees.
            for desc, pre, use in (
                    ("an asm keyword", "#define J(a, b) a##b\n", 'J(__as, m__)("nop");'),
                    ("a vendored name", "#define J(a, b) a ## b\n", "J(gte_, nop)();"),
                    ("a digraph paste", "%:define J(a, b) a %:%: b\n", "J(gte_, nop)();")):
                src = tu("    out[0] = 1;\n    " + use, pre=pre)
                check(f"paste gate: a paste building {desc} refused",
                      any("token-paste" in i for i in issues(src)))
                full, _n = inlineasm.strip_cheat_asm_file(src)
                check(f"paste gate: a paste building {desc} stripped for the sandbox",
                      "##" not in full and "%:%:" not in full)
            eq("paste gate: a paste in a body counted",
               inlineasm.func_cheat_asm_count(
                   tu("#define J(a, b) a##b\n    out[0] = J(1, 2);"), "func_GTE"), 1)
            splice = tu("    out[0] = J(gte_, nop)();", pre="#define J(a, b) a #\\\n# b\n")
            check("paste gate: a paste split by a line splice refused",
                  G.paste_spans(splice) != [] and any("token-paste" in i for i in issues(splice)))
            quiet = tu('    /* a ## b */\n    func_x("##", \'#\');\n    // c ## d')
            eq("paste gate: `##` in comments and literals ignored",
               (G.paste_spans(quiet), inlineasm.func_cheat_asm_count(quiet, "func_GTE")), ([], 0))
            hidden = tu(calls3, inc="#define H <psxsdk/inline_o.h>\n#include H\n")
            check("call gate: a call without a direct vendored #include refused",
                  any("does not include its header" in i for i in issues(hidden)))
            for desc, arg in (("an asm keyword", '({ __asm__ volatile("nop"); v; })'),
                              ("a statement-expression", "({ s32 *w = v; w; })")):
                bad = tu(f"    gte_ldv0({arg});\n    gte_rtv0();")
                grant(bad)
                check(f"call gate: {desc} in a call's arguments refused",
                      any("in its arguments" in i for i in issues(bad)))

            G.foreign_asm_macros = lambda root=".": {"MY_LDV": True, "MY_BARRIER": False}
            foreign = tu("    MY_LDV(v);\n    out[0] = 2;\n    MY_BARRIER;", inc="")
            eq("call gate: foreign header asm macros counted",
               inlineasm.func_cheat_asm_count(foreign, "func_GTE"), 2)
            stripped, n = inlineasm.strip_cheat_asm_file(foreign)
            eq("call gate: foreign header asm macros stripped",
               ("MY_LDV" in stripped, "MY_BARRIER" in stripped, n), (False, False, 2))
            G.foreign_asm_macros = old_foreign

            lzc = ('__asm__ volatile("move  $12,%0" : : "r"(v) : ' + _CLOB + ');\n    '
                   '__asm__ volatile("mtc2  $12,$30" : : : ' + _CLOB + ');')
            written = tu("    " + lzc)
            grant(written)
            completion.REFUSE_WRITTEN_OUT_UNITS = False
            eq("call gate: written-out unit allowed while the flag is off", issues(written), [])
            completion.REFUSE_WRITTEN_OUT_UNITS = True
            check("call gate: written-out unit refused with the flag on",
                  any("written-out PsyQ macro gte_ldlzc" in i for i in issues(written)))
        finally:
            inlineasm._read_src_cached, completion.REGIONS = old_read, old_regions
            G.foreign_asm_macros, completion.REFUSE_WRITTEN_OUT_UNITS = old_foreign, old_flag


def test_gte_macro_rewrite_tool() -> None:
    """tools/gte_macro_rewrite.py on fixture TUs (no scoring)."""
    from engine import gtemacro as G
    if not all(Path(r).exists() for r in G.VENDORED):
        skip("gte_macro_rewrite", "include/psxsdk vendored headers absent")
        return
    import importlib
    import sys
    sys.path.insert(0, str(Path("tools").resolve()))
    R = importlib.import_module("gte_macro_rewrite")
    clob = '"$12","$13","$14","$15","memory"'

    def st(t, op=None):
        ins = f'"r"({op})' if op else ""
        return f'    __asm__ volatile ("{t}": :{ins}:{clob});\n'

    ldv0 = st("move  $12,%0", "v") + st("lwc2  $0,($12)") + st("lwc2  $1,4($12)")
    rtv0tr = st("nop   ") + st("nop   ") + st(".word 0x0000027f")

    def fn(name, body, pre=""):
        return pre + f"void {name}(s32 *v, s32 n) {{\n    s32 sp_tmp;\n{body}    func_x(sp_tmp);\n}}\n"

    # statement-exact, ambiguous rt / rtv0tr, stale comment dropped
    t1 = fn("func_T1", "    /* gte_ldv0(v): inline_o.h :16-20, clobbers as the header */\n"
            + ldv0 + rtv0tr)
    r = R.rewrite_function(t1, "func_T1")
    eq("rewrite: statement-exact header", (r.error, r.header), (None, "inline_o.h"))
    eq("rewrite: statement-exact calls",
       [(u.macro, u.args) for _l, t, us in r.units for u in us], [("gte_ldv0", ["v"]), ("gte_rt", [])])
    eq("rewrite: statement-exact tier", [t for _l, t, _u in r.units], ["statement-exact"])
    eq("rewrite: identical expansions listed (rt / rtv0tr)",
       [u.alternatives for _l, _t, us in r.units for u in us], [[], ["gte_rtv0tr"]])
    check("rewrite: the island's stale comment is dropped and reported",
          len(r.dropped_comments) == 1 and "/* gte_ldv0(v)" not in r.body)
    check("rewrite: no asm left in the body", "__asm__" not in r.body)
    eq("rewrite: includes the TU needs", r.includes, ["#include <psxsdk/inline_o.h>"])
    check("rewrite: candidate = includes + body",
          R.candidate_text(r).startswith("#include <psxsdk/inline_o.h>\nvoid func_T1("))
    t1b = t1.replace("clobbers as the header */", "then gte_rtv0tr */")
    r = R.rewrite_function(t1b, "func_T1")
    eq("rewrite: a macro the comments name wins the tie",
       [u.macro for _l, _t, us in r.units for u in us], ["gte_ldv0", "gte_rtv0tr"])

    # instruction-exact: the hand LZC with a stack "=m" output
    lzc = ('    __asm__ volatile(\n        "addu   $t4, %1, $zero\\n"\n        "mtc2   $t4, $30\\n"\n'
           '        "nop\\n"\n        "nop\\n"\n        "addu   $t4, $sp, $zero\\n"\n'
           '        "swc2   $31, 0($t4)\\n"\n'
           '        : "=m"(sp_tmp) : "r"(n) : "$12", "$13", "$14", "$15");\n')
    r = R.rewrite_function(fn("func_T2", lzc), "func_T2")
    eq("rewrite: instruction-exact LZC -> gte_Lzc",
       (r.error, [(u.macro, u.args) for _l, _t, us in r.units for u in us],
        [t for _l, t, _u in r.units]),
       (None, [("gte_Lzc", ["n", "&sp_tmp"])], ["instruction-exact"]))
    eq("rewrite: a composite needs gtemac.h too", r.includes,
       ["#include <psxsdk/inline_o.h>", "#include <psxsdk/gtemac.h>"])
    check("rewrite: composite lists its parts as the alternative",
          any("parts" in a for _l, _t, us in r.units for u in us for a in u.alternatives))
    r = R.rewrite_function(fn("func_T2", lzc.replace("$sp, $zero", "$sp, $zero\\n\"\n"
                                                    '        "nop')), "func_T2")
    check("rewrite: an extra instruction is not a macro sequence",
          r.error is not None and "not a sequence" in r.error)

    # inline_c form, header choice
    cform = '    __asm__ volatile ("lwc2\\t$0, 0( %0 );" "lwc2\\t$1, 4( %0 )" : : "r"( v ) );\n'
    r = R.rewrite_function(fn("func_T3", cform), "func_T3")
    eq("rewrite: inline_c.h form -> inline_c.h", (r.error, r.header), (None, "inline_c.h"))
    r = R.rewrite_function(fn("func_T3", cform), "func_T3", header="inline_o.h")
    check("rewrite: forced inline_o.h refuses the inline_c.h form", r.error is not None)
    r = R.rewrite_function(fn("func_T3", cform, "#include <psxsdk/inline_c.h>\n"), "func_T3")
    eq("rewrite: the TU's own header is used and not re-included",
       (r.error, r.header, r.includes), (None, "inline_c.h", []))

    # refusals
    r = R.rewrite_function(fn("func_T4", '    __asm__ volatile ("addiu $2,$0,1");\n'), "func_T4")
    check("rewrite: refuses a non-GTE island", r.error is not None and r.error.startswith("not GTE"))
    r = R.rewrite_function(fn("func_T5", '    __asm__ volatile ("mtc2 %0, $29" :: "r"(n));\n'),
                           "func_T5")
    check("rewrite: refuses a cop2 island that is no macro", r.error is not None
          and "not a sequence" in r.error)
    r = R.rewrite_function(fn("func_T6", "    n = 1;\n"), "func_T6")
    eq("rewrite: nothing to do without islands", r.error, "no assembly islands")

    # the doc comment above the function is listed for review
    doc = "/* Loads v. GTE: gte_ldv0 with the clobbers spelled out ($12). */\n"
    r = R.rewrite_function(doc + fn("func_T7", ldv0 + rtv0tr), "func_T7")
    check("rewrite: a stale doc comment is listed for review",
          any(ln == 1 and "clobbers spelled out" in c for ln, c in r.review_comments))
    r = R.rewrite_function("/* Loads v. */\n" + fn("func_T7", ldv0 + rtv0tr), "func_T7")
    eq("rewrite: a plain doc comment is not listed", r.review_comments, [])

    # --write guards: the Grinder lock
    with tempfile.TemporaryDirectory() as td:
        lock = Path(td) / "grind.lock"
        eq("rewrite: no Grinder lock -> not running", R.grinder_running(lock), None)
        lock.write_text("not a pid\n")
        eq("rewrite: an unreadable Grinder lock counts as live", R.grinder_running(lock), -1)
        lock.write_text("4000000000\r\n")
        old = R._windows_pid_alive
        try:
            R._windows_pid_alive = lambda pid: False
            eq("rewrite: a stale Grinder lock -> not running", R.grinder_running(lock), None)
            R._windows_pid_alive = lambda pid: True
            eq("rewrite: a live Grinder lock -> its pid", R.grinder_running(lock), 4000000000)
        finally:
            R._windows_pid_alive = old
        r = R.rewrite_function(fn("func_T1", ldv0 + rtv0tr), "func_T1")
        R.write_candidate(r, grind=Path(td))
        dest = Path(td) / "func_T1" / "candidate.c"
        check("rewrite: --write writes a new candidate", r.candidate and dest.exists())
        dest.write_text("keep\n")
        r = R.rewrite_function(fn("func_T1", ldv0 + rtv0tr), "func_T1")
        R.write_candidate(r, grind=Path(td))
        check("rewrite: --write keeps an existing candidate.c",
              dest.read_text() == "keep\n" and r.candidate is None and "--force" in r.error)
        r = R.rewrite_function(fn("func_T1", ldv0 + rtv0tr), "func_T1")
        R.write_candidate(r, force=True, grind=Path(td))
        check("rewrite: --write --force overwrites it", dest.read_text() != "keep\n")


def test_gte_header_registry() -> None:
    """Only the vendored headers, bios.h and include_asm.h define asm macros."""
    from engine import gtemacro as G
    root = tempfile.mkdtemp(prefix="bb2_registry_")
    files = {
        "include/bios.h": "#define BIOS_A_FUNCTION(n, id) __asm__(\".text\")\n",
        "include/include_asm.h": "#define INCLUDE_ASM(f, n) __asm__(\".include\")\n",
        "include/psxsdk/inline_o.h": _FIX_O,
        "include/psxsdk/gtemac.h": _FIX_MAC,
        "include/foo.h": "#define BAR() __asm__ volatile (\"nop\")\n#define PLAIN(x) (x + 1)\n",
        "include/baz.h": "#define QUX(x) BAR()\n#define VIA_GTE(x) gte_ldv0(x)\n",
        "include/obj.h": "#define FENCE \\\n    __asm__(\"\" ::: \"memory\")\n",
        "src/main/local.h": "#define VIA_BIOS BIOS_A_FUNCTION(a, 1)\n",
        "include/incl.h": "#include <psxsdk/inline_o.h>\n",
        "include/incl_sn.h": "#include <psxsdk/libsn.h>\n",
        "include/paste.h": "#define J(a, b) a ## b\n",
        "src/main/paste.h": "%:define K(a, b) a #\\\n# b\n",
    }
    for rel, text in files.items():
        (Path(root) / rel).parent.mkdir(parents=True, exist_ok=True)
        (Path(root) / rel).write_text(text, newline="\n")
    reg = {(n, r) for n, r, _f in G.asm_macro_registry(root)}
    check("registry: the vendored and allowed macros are asm macros",
          {("gte_ldv0", "include/psxsdk/inline_o.h"), ("BIOS_A_FUNCTION", "include/bios.h"),
           ("INCLUDE_ASM", "include/include_asm.h")} <= reg)
    issues = G.header_registry_issues(root)
    for rel, name in (("include/foo.h", "BAR"), ("include/baz.h", "QUX"),
                      ("include/baz.h", "VIA_GTE"), ("include/obj.h", "FENCE"),
                      ("src/main/local.h", "VIA_BIOS")):
        check(f"registry: {rel} {name} flagged", any(i.startswith(f"{rel}: #define {name} ")
                                                     for i in issues))
    check("registry: a plain macro is not flagged", not any("PLAIN" in i for i in issues))
    check("registry: the allowed headers are not flagged", not any(
        i.startswith(("include/bios.h", "include/include_asm.h", "include/psxsdk/"))
        for i in issues))
    check("registry: a header including a vendored header is flagged",
          any(i.startswith("include/incl.h: includes a vendored") for i in issues))
    check("registry: a header including libsn.h is flagged",
          any(i.startswith("include/incl_sn.h: includes a vendored") for i in issues))
    for rel in ("include/paste.h", "src/main/paste.h"):
        check(f"registry: a token paste in {rel} is flagged",
              any(i.startswith(f"{rel}: uses the `##` token-paste") for i in issues))
    eq("registry: foreign asm macros (function-like flag)",
       G.foreign_asm_macros(root),
       {"BAR": True, "QUX": True, "VIA_GTE": True, "FENCE": False, "VIA_BIOS": False})
    eq("registry: the tree is clean", G.header_registry_issues(), [])


# --------------------------------------------------------------------------
# cheats — regfix masking (other half of cheat-invisibility)
# --------------------------------------------------------------------------

def test_cheats() -> None:
    check("is_lost_codegen: addu ...,$zero",
          cheats.is_lost_codegen('funcA: insert_after "addu $8,$3,$zero"'))
    check("is_lost_codegen: addu ...,$0",
          cheats.is_lost_codegen('funcA: insert "addu $9,$0,$0"'))
    check("is_lost_codegen: plain lw is NOT lost-codegen",
          not cheats.is_lost_codegen('funcA: insert "lw $1,0($2)"'))
    check("is_lost_codegen: replace_with_asmfile is NOT",
          not cheats.is_lost_codegen('funcA: replace_with_asmfile foo'))

    # canonical-extraction wiring recognition ([infra-rule: canonical-asm-
    # extraction]): replace_with_asmfile + inline_asm_canonical.txt member
    # ONLY. Uses a real canonical member (gte_ProjectSVectorTriples, custom $s0 ABI; was
    # save_vc_ctrl until its 2026-10-01 de-authorization)
    # so the check exercises the live list.
    check("canon-extract: wiring for canonical member IS exempt",
          cheats.is_canonical_extraction_rule(
              'gte_ProjectSVectorTriples: replace_with_asmfile "asm/funcs/gte_ProjectSVectorTriples.s"'))
    check("canon-extract: wiring for NON-canonical func is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'not_a_canonical_func_zz: replace_with_asmfile "asm/funcs/x.s"'))
    check("canon-extract: non-wiring rule for canonical member is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'gte_ProjectSVectorTriples: insert_after "addu $8,$3,$zero"'))
    check("canon-extract: path outside asm/funcs is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'gte_ProjectSVectorTriples: replace_with_asmfile "tmp/evil.s"'))
    # The wiring still COUNTS as a rule for completion purposes (reviewer
    # verdict 2026-08-06: zero-rules bar unchanged; the recognizer only
    # routes wiring-only functions to the authorize bucket, never to done).
    #
    # SYNTHETIC fixture since Campaign 4 Wave 7 (2026-08-06). save_vc_ctrl was
    # the LAST live canonical-extraction wiring, and its TU re-split retired it,
    # so no function in the tree exhibits this state any more. Re-keying to a
    # fabricated function keeps the routing pinned: the recognizer must still
    # refuse a wiring-only function if one is ever introduced again.
    from engine import queue as _q
    with tempfile.TemporaryDirectory() as td, _stub_tus(td, {
            "text1a_post": 'INCLUDE_ASM("asm/funcs", canon_zz);\n'}):
        rf = Path(td) / "regfix.txt";        rf.write_text("")
        rf2 = Path(td) / "regfix_stage2.txt"; rf2.write_text("")
        af = Path(td) / "asmfix.txt"
        af.write_text('canon_zz: replace_with_asmfile "asm/funcs/canon_zz.s"\n')
        qp = Path(td) / "queue.json"
        qp.write_text(json.dumps({"items": [{
            "func": "canon_zz", "file": "text1a_post", "distance": 2,
            "verdict": "CANON-EXTRACT", "rules": 1, "status": "authorize"}],
            "counts": {}}))
        saved = (cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX,
                 cheats.canonical_asm_funcs, _q.QUEUE_PATH)
        cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX = str(rf), str(rf2), str(af)
        cheats.canonical_asm_funcs = lambda *a, **k: {"canon_zz"}
        _q.QUEUE_PATH = str(qp)
        try:
            check("canon-extract: wiring-only func still has rule_count > 0",
                  _q._rule_count("canon_zz") > 0)
            check("canon-extract: is_canonical_extraction_only(wiring-only func)",
                  cheats.is_canonical_extraction_only(func="canon_zz"))
            r = _q.mark_done("canon_zz")
            check("canon-extract: mark_done still REFUSES wiring-only func",
                  not r.get("ok")
                  and "replace_with_asmfile" in r.get("reason", "")
                  and "missing; cannot check" not in r.get("reason", ""))
            check("canon-extract: the refusal names the wiring, not a bare rule count",
                  "replace_with_asmfile" in r.get("reason", ""))
        finally:
            (cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX,
             cheats.canonical_asm_funcs, _q.QUEUE_PATH) = saved

    # Rules-to-zero end state (2026-08-25; files deleted 2026-08-30): the tree
    # carries NO rule files at all. Their absence IS the invariant now.
    check("rules-to-zero: no live regfix/asmfix rule files exist",
          not any(Path(f).exists()
                  for f in (cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX)))

    with tempfile.TemporaryDirectory() as td:
        cfg = Path(td) / "regfix.txt"
        cfg.write_text(
            'funcA: insert_after "addu $8,$3,$zero"\n'   # lost-codegen
            'funcA: subst "x" "y"\n'                       # other rule
            'funcB: insert "lw $1,0($2)"\n'                # different func
        )
        p = str(cfg)
        _, drop_all = cheats._filter_text("funcA", "all", p)
        eq("filter: disable=all drops ALL of funcA's rules", drop_all, 2)
        _, drop_lcg = cheats._filter_text("funcA", "lost-codegen", p)
        eq("filter: disable=lost-codegen drops only the LCG rule", drop_lcg, 1)
        txtB, dropB = cheats._filter_text("funcB", "all", p)
        eq("filter: funcB filter leaves funcA's rules", txtB.count("funcA:"), 2)
        eq("filter: funcB's own rule dropped", dropB, 1)

        # cheat-invisibility plumbing: a NEWLY-added cheat keyed to the function
        # is also stripped under disable=all -> it can never move the score.
        with cfg.open("a") as f:
            f.write('funcA: insert "addu $12,$0,$zero"\n')  # an agent sneaking a cheat
        _, drop_after = cheats._filter_text("funcA", "all", p)
        eq("filter: a freshly-injected funcA cheat is ALSO stripped", drop_after, 3)

    # is_jtbl_infra: jump-table rodata-split infra (asmfix-only) vs real cheats
    with tempfile.TemporaryDirectory() as td:
        rf = Path(td) / "regfix.txt"
        af = Path(td) / "asmfix.txt"
        rf.write_text('funcR: subst "$2" "$3" @ 5\n')        # a real register cheat
        af.write_text(
            'funcJ: rename ".L28" "jtbl_800108CC"\n'          # jtbl infra (asmfix only)
            'funcJ: replace_first "^.L80035644:$" "jlabel .L80035644"\n'
            'funcJ: delete_between "^\\.section\\s+\\.rodata" "^\\.text$"\n'
            'funcN: rename ".L10" ".L20"\n'                   # rename but NOT a jtbl symbol
            'funcR: rename ".L5" "jtbl_80001234"\n'           # has jtbl asmfix BUT also a regfix
        )
        rfp, afp = str(rf), str(af)
        check("jtbl-infra: asmfix-only jtbl rules -> True",
              cheats.is_jtbl_infra("funcJ", regfix=rfp, regfix2=rfp, asmfix=afp) is True)
        check("jtbl-infra: rename without a jtbl_ symbol -> False",
              cheats.is_jtbl_infra("funcN", regfix=rfp, regfix2="/nonexistent", asmfix=afp) is False)
        check("jtbl-infra: any regfix rule disqualifies -> False",
              cheats.is_jtbl_infra("funcR", regfix=rfp, regfix2="/nonexistent", asmfix=afp) is False)
        check("jtbl-infra: no rules -> False",
              cheats.is_jtbl_infra("funcZ", regfix="/nonexistent", regfix2="/nonexistent", asmfix=afp) is False)

    # canonical_asm_funcs: the COMPLETED gate's cheat-asm exemption set
    with tempfile.TemporaryDirectory() as td:
        caf = Path(td) / "canon.txt"
        caf.write_text("# header comment\nfunc_AAA  # GTE wrapper\nfunc_BBB\n\nfunc_CCC\n")
        eq("canonical_asm_funcs: first-token names, # ignored",
           cheats.canonical_asm_funcs(str(caf)), {"func_AAA", "func_BBB", "func_CCC"})
        check("canonical_asm_funcs: missing file -> empty set",
              cheats.canonical_asm_funcs("/nonexistent") == set())

    # maspsx_comm_syms.txt (owner ruling 2026-09-26): `func: sym, sym` rows count by func,
    # so queue done / the integrity audit see a function that depends on the gate
    with tempfile.TemporaryDirectory() as td:
        cl = Path(td) / "comm.txt"
        cl.write_text("# header\ncdrom_SetMix: g_cd_atv\nfunc_B: x, y\n\nfunc_C\n")
        eq("gate list: `func: syms` rows yield the func name",
           cheats._prologue_txt_funcs(str(cl)), {"cdrom_SetMix", "func_B", "func_C"})
    eq("gate list: maspsx_comm_syms.txt is a cheat-pathway gate (owner ruling 2026-09-30)",
       cheats.MASPSX_GATE_LISTS.get("maspsx_comm_syms.txt"), "cheat-pathway")


def test_lowercase_asm_cheats() -> None:
    """find_lowercase_asm_cheats — closes the bare `asm(...)` loophole that
    bypassed ASM_KEYWORD_RE (which excludes bare `asm` for register-pin
    safety). 5 COMPLETED-C functions affected per the 2026-06-02 thorough
    audit."""
    text = """\
void f(void) {
    asm volatile("" ::: "memory");
    do_thing();
}
"""
    hits = volatile_cheats.find_lowercase_asm_cheats(text, text.index("{"),
                                                     text.rindex("}") + 1)
    eq("lowercase-asm: catches `asm volatile(...)`", len(hits), 1)

    text2 = """\
void g(void) {
    asm("" : "=r"(x) : "0"(x));
}
"""
    hits = volatile_cheats.find_lowercase_asm_cheats(text2, text2.index("{"),
                                                     text2.rindex("}") + 1)
    eq("lowercase-asm: catches `asm(...)` (no volatile)", len(hits), 1)

    # Register-pin pattern should NOT match (different shape — `asm` is
    # part of a declaration starting with `register`)
    text3 = """\
void h(int x) {
    register int t asm("$8") = x;
    do_thing(t);
}
"""
    hits = volatile_cheats.find_lowercase_asm_cheats(text3, text3.index("{"),
                                                     text3.rindex("}") + 1)
    eq("lowercase-asm: register pin NOT flagged", len(hits), 0)


def test_asm_keyword_recognition() -> None:
    """cia.find_asm_keywords — inline asm is recognized the way cc1 tokenizes
    it. Layer-2 finding 2026-09-25: `__asm__ /**/ ("move $5,%0" : : "r"(n));`
    was counted 0 by the completion gate AND stripped 0 by the sandbox, because
    the raw-text ASM_KEYWORD_RE allowed only whitespace before the `(`. Every
    evasion below must be counted by the gate and stripped by the sandbox; every
    form handled before must still be; a mere MENTION must not count."""
    import classify_inline_asm as cia
    import audit_asm_cheats as AAC

    def one(stmt, pre=""):
        return pre + "void f(s32 n) {\n    " + stmt + "\n    n = 3;\n}\n"

    def caught(desc, src, count=1):
        eq(f"asm-kw ({desc}): gate counts it", inlineasm.func_cheat_asm_count(src, "f"), count)
        out, n = inlineasm.strip_cheat_asm_file(src)
        check(f"asm-kw ({desc}): sandbox strip count >= {count}", n >= count)
        if "Q(" in src:  # unparseable shape: the bare keyword is stripped (fail-safe)
            check(f"asm-kw ({desc}): sandbox strips the keyword", "__asm__" not in out)
        else:
            check(f"asm-kw ({desc}): sandbox strips the injected insn",
                  "$5" not in out and "move" not in out)
        check(f"asm-kw ({desc}): surrounding C survives", "n = 3;" in out)
        out_k, _ = inlineasm.strip_cheat_asm_file(src, keep_gte_macro_units=True)
        eq(f"asm-kw ({desc}): sandbox (GTE-unit mode) strips it too", out_k, out)

    mv = '("move $5,%0" : : "r"(n));'
    evasions = {
        "comment before (": "__asm__ /**/ " + mv,
        "comment before qualifier": "__asm__ /* x */ volatile " + mv,
        "comment between qualifier and (": "__asm__ volatile /* x */ " + mv,
        "// comment then newline": "__asm__ // hi\n    " + mv,
        "newlines around qualifier": "__asm__\n    volatile\n    " + mv,
        "backslash-newline before (": "__asm__ \\\n    " + mv,
        "backslash-newline inside keyword": "__as\\\nm__ volatile " + mv,
        "backslash-newline inside qualifier": "__asm__ vola\\\ntile " + mv,
        "__volatile qualifier": "__asm__ __volatile " + mv,
        "const qualifier": "__asm const " + mv,
        "__const__ qualifier": "__asm__ __const__ " + mv,
        "macro standing for the qualifier": "__asm__ VOL " + mv,
        "decoy cop2 opcode in a comment in the body":
            '__asm__("move $5,%0" /* "mtc2 $2,$0" */ : : "r"(n));',
        "bare asm, comment before qualifier": "asm /**/ volatile " + mv,
        "bare asm after a statement on the same line": "n = 1; asm" + mv,
        "bare asm after a leading comment": "/* x */ asm" + mv,
        "bare asm, backslash-newline in keyword": "as\\\nm volatile " + mv,
        "keyword with no operand list (macro argument)": 'Q(__asm__)("move $5,%0");',
    }
    for desc, stmt in evasions.items():
        caught(desc, one(stmt))
    caught("two statements on one line",
           one('__asm__("move $5,$4"); __asm__ /**/ ("move $5,$6");'), count=2)
    caught("stray quote in an #if 0 block hides nothing",
           one("__asm__ " + mv, pre='#if 0\n"\n#endif\n') + '#if 0\n"\n#endif\n')

    # The completion-region scanner and the manual audit see the same blocks.
    for desc in ("comment before (", "backslash-newline inside keyword", "__volatile qualifier"):
        eq(f"asm-kw ({desc}): completion.blocks finds it",
           len(completion.blocks(one(evasions[desc]), "f")), 1)
        eq(f"asm-kw ({desc}): audit_asm_cheats finds it",
           len(list(AAC._extract_balanced_asm_blocks(one(evasions[desc])))), 1)

    # Forms handled before are still handled.
    caught("plain __asm__ volatile", one("__asm__ volatile " + mv))
    caught("__asm spelling", one("__asm" + mv))
    caught("__asm__ __volatile__", one("__asm__ __volatile__ " + mv))
    caught("multi-line operand list", one('__asm__ volatile(\n        "move $5,%0\\n"\n'
                                          '        : : "r"(n));'))
    gte = one('__asm__ volatile("mtc2 %0, $0" : : "r"(n));')
    eq("asm-kw: canonical GTE op not counted", inlineasm.func_cheat_asm_count(gte, "f"), 0)
    eq("asm-kw: canonical GTE op kept", inlineasm.strip_cheat_asm_file(gte), (gte, 0))
    gte_c = one('__asm__ /* x */ volatile("mtc2 %0, $0" : : "r"(n));')
    eq("asm-kw: canonical GTE op behind a comment still canonical (kept)",
       inlineasm.strip_cheat_asm_file(gte_c), (gte_c, 0))
    # cc1 2.7.2 accepts a string literal spanning lines (code6cac_c_ab.c had
    # one until 2026-10-03); its body must come out whole, not desync the parser.
    ml = '__asm__(".section .rodata\n\t.word 0\n\t.text");\nvoid g(void) { h(1); }\n'
    kws = cia.find_asm_keywords(ml)
    eq("asm-kw: multi-line string literal body intact",
       [k.body for k in kws], ['".section .rodata\n\t.word 0\n\t.text"'])
    check("asm-kw: code after a multi-line string survives the strip",
          "void g(void) { h(1); }" in inlineasm.strip_cheat_asm_file(ml)[0])
    # A whole-body glabel block is still attributed, also behind a comment.
    wb = '__asm__ /* body */ (\n    "glabel func_X\\n"\n    "    jr $ra\\n"\n);\n'
    eq("asm-kw: glabel whole body attributed", inlineasm.whole_body_asm_funcs(wb), {"func_X"})
    eq("asm-kw: glabel whole body not stripped", inlineasm.strip_cheat_asm_file(wb)[0], wb)

    # A MENTION is not the keyword.
    for desc, stmt in {
        "block comment": '/* was: __asm__ ("move $5,$4"); */',
        "line comment": '// __asm__ volatile("move $5,$4");',
        "string literal": 'puts("__asm__(\\"move $5,$4\\")");',
        "char-literal neighbour": "c = '\"'; puts(\"__asm__(x)\");",
        "identifier containing it": "my__asm__(n); asm_helper(n);",
    }.items():
        src = one(stmt)
        eq(f"asm-kw: {desc} mentioning __asm__ is not counted",
           inlineasm.func_cheat_asm_count(src, "f"), 0)
        eq(f"asm-kw: {desc} mentioning __asm__ is not stripped",
           inlineasm.strip_cheat_asm_file(src), (src, 0))

    # Macro-hidden asm (volatile_cheats pattern 5): every keyword spelling, a
    # backslash-continued #define, and not a keyword mentioned in a comment/string.
    for desc, pre in {
        "__asm__": "#define A __asm__\n",
        "__asm": "#define A __asm\n",
        "bare asm": "#define A asm\n",
        "continued #define": "#define A \\\n    __asm__\n",
        "comment in the #define": "#define A /**/ __asm__ /**/\n",
    }.items():
        src = pre + 'void f(s32 n) {\n    A("move $5,$4");\n}\n'
        eq(f"macro-asm ({desc}): definition found",
           [d[2] for d in volatile_cheats.find_macro_asm_defs(src)], ["A"])
        check(f"macro-asm ({desc}): use counted by the gate",
              inlineasm.func_cheat_asm_count(src, "f") >= 1)
        check(f"macro-asm ({desc}): sandbox neutralizes the use",
              "move" not in inlineasm.strip_cheat_asm_file(src)[0].split("*/", 1)[-1])
    # Layer-2 round 2: the digraph `%:` IS `#` to the build's cpp, macro chains,
    # and nested parens in a use.
    for desc, pre, use in (
        ("%:define", "%:define A __asm__\n", 'A("move $5,%0" : : "r"(n));'),
        ("%: define, function-like", "%: define A(x) __asm__ x\n",
         'A(("move $5,%0" : : "r"(n)));'),
        ("chain V -> W", "#define V __asm__\n#define W V\n", 'W("move $5,%0" : : "r"(n));'),
        ("chain of three, digraph link", "#define V __asm__\n%:define W V\n#define X W\n",
         'X volatile ("move $5,%0" : : "r"(n));'),
    ):
        src = pre + "void f(s32 n) {\n    " + use + "\n}\n"
        check(f"macro-asm ({desc}): use counted by the gate",
              inlineasm.func_cheat_asm_count(src, "f") >= 1)
        body = inlineasm.strip_cheat_asm_file(src)[0].rsplit("*/", 1)[-1]
        check(f"macro-asm ({desc}): sandbox neutralizes the use", "move" not in body)
    eq("macro-asm: a same-named parameter shadows (not a chain)",
       [d[2] for d in volatile_cheats.find_macro_asm_defs(
           "#define V __asm__\n#define W(V) V\nint x;\n")], ["V"])
    eq("asm-kw: `%:` line is a directive (define body not a statement)",
       [k.directive for k in cia.find_asm_keywords('%:define B __asm__("nop")\n')], [True])

    # A bare `asm` statement behind a same-file empty macro; a pin is still not one.
    for desc, stmt in {
        "empty macro before asm": 'E asm("move $5,%0" : : "r"(n));',
        "empty macro before asm volatile": 'n = 1; E asm volatile ("move $5,%0" : : "r"(n));',
        "empty macro, single-string template": 'E asm("move $5,$4");',
    }.items():
        caught(desc, one(stmt, pre="#define E\n"))
    eq("asm-kw: register pin still not a lowercase-asm statement",
       volatile_cheats.find_lowercase_asm_cheats(
           one('register s32 t asm("$8") = n;', pre="#define E\n"), 0, 999), [])

    # A `glabel` template inside a C function body is not a whole-function body.
    caught("glabel decoy inside a function body",
           one('__asm__("glabel f\\n move $5,%0" : : "r"(n));'))

    for desc, pre in {"string": '#define A "__asm__"\n',
                      "comment": "#define A 1 /* not __asm__ */\n"}.items():
        eq(f"macro-asm: keyword only in a {desc} of the #define is not a definition",
           volatile_cheats.find_macro_asm_defs(pre + "void f(void) {}\n"), [])
    eq("macro-asm: continued #define spans to its last line",
       volatile_cheats.find_macro_asm_defs("#define A \\\n  __asm__\nint x;\n"),
       [(0, 21, "A")])

    # Alias-rename declarations (volatile_cheats patterns 1 and 4) carry the
    # same keyword: a comment inside the declaration, or the __asm__
    # spelling, does not hide them.
    for desc, decl in {
        "nonvol, comment before (": 'extern u16 D_x_h asm /**/ ("D_80001234");',
        "nonvol, comment after extern": 'extern /* t */ u16 D_x_h asm("D_80001234");',
        "nonvol, __asm__ spelling": 'extern u16 D_x_h __asm__("D_80001234");',
        "nonvol, backslash-newline in keyword": 'extern u16 D_x_h as\\\nm("D_80001234");',
        "vol, comment before (": 'extern volatile s32 D_x_h asm /**/ ("D_80001234");',
        "no extern": 's32 D_x_h asm("D_80001234");',
        "label split into literals": 'extern s32 D_x_h asm("D_8000" "1234");',
        "trailing __attribute__": 'extern s32 D_x_h asm("D_80001234") __attribute__((unused));',
        "second declarator": 'extern s32 D_y, D_x_h asm("D_80001234");',
        "array declarator": 'extern u8 D_x_h[4] asm("D_80001234");',
    }.items():
        src = decl + "\nvoid f(void) {\n    g(D_x_h);\n}\n"
        eq(f"alias ({desc}): found", len(volatile_cheats.find_alias_renames(src))
           + len(volatile_cheats.find_nonvolatile_alias_renames(src)), 1)
        eq(f"alias ({desc}): use counted by the gate",
           inlineasm.func_cheat_asm_count(src, "f"), 1)
        out = inlineasm.strip_cheat_asm_file(src)[0]
        check(f"alias ({desc}): sandbox neutralizes it",
              ("volatile" not in out) if "volatile" in decl else ('"D_80001234"' not in out))

    for desc, src in {
        "same-name label": 'extern s32 g_x asm("g_x");\nvoid f(void) { g(g_x); }\n',
        "function declarator label": 'void h(void) asm("g");\nvoid f(void) { h(); }\n',
        "register pin": 'void f(s32 n) { register s32 t asm("$8") = n; g(t); }\n',
    }.items():
        eq(f"alias: {desc} is not an alias rename",
           volatile_cheats.find_alias_renames(src)
           + volatile_cheats.find_nonvolatile_alias_renames(src), [])

    # Documented limit: a block mixing a cop2 op with a GPR op is kept whole by
    # the sandbox (authorized GTE islands carry such feeder instructions); the
    # completion gate refuses it through completion.blocks' island check.
    mixed = one('__asm__("mtc2 $2,$0\\n move $5,%0" : : "r"(n));')
    eq("asm-kw: mixed cop2+GPR block kept by the sandbox (island-gated)",
       inlineasm.strip_cheat_asm_file(mixed), (mixed, 0))
    eq("asm-kw: ... and seen by completion.blocks", len(completion.blocks(mixed, "f")), 1)

    # GTE-unit recognition stays strict: a comment or splice inside a
    # statement is not header-exact, so the run is not a unit (stripped).
    from engine import gtemacro
    ld_c = _gte_src(_MOVE.format(op="n").replace("__asm__ volatile", "__asm__ /**/ volatile"),
                    _MTC2)
    eq("asm-kw: GTE statement with a comment inside is not a unit",
       gtemacro.unit_spans(ld_c), ())
    eq("asm-kw: ... and is stripped in both sandbox modes",
       inlineasm.strip_cheat_asm_file(ld_c, keep_gte_macro_units=True),
       inlineasm.strip_cheat_asm_file(ld_c))

    # The classifier report sees the same blocks (one record per statement).
    with tempfile.TemporaryDirectory(dir=str(Path(cia.ROOT) / "tmp")) as td:
        p = Path(td) / "t.c"
        p.write_text(one('__asm__("nop"); __asm__ /**/ ("move $5,$4");'))
        recs = [r for r in cia.scan_file(p) if r["kind"] == "asm_block"]
        eq("asm-kw: classify_inline_asm records both statements on the line",
           [(r["line"], r["func"], r["category"]) for r in recs],
           [(2, "f", "cheat"), (2, "f", "cheat")])


def test_macro_asm_strip_round_trip() -> None:
    """find_macro_asm_defs + strip_volatile_cheats_file — the PAD_NOPS
    macro family pattern that appears in display.c, code6cac*.c. The
    macro definition declares an `__asm__` body; downstream uses appear
    as bare `NAME;` statements. Both must be stripped together — leaving
    use sites with the macro definition removed causes maspsx to treat
    `NAME;` as a tentative-global (`.comm NAME,4,4`) which crashes the
    pipeline ('too many values to unpack'). Regression test wired
    2026-06-03 after a round-2 worker report that surfaced the symptom
    (turned out to be a stale-worktree env, but the regression test pins
    the contract: macro definitions and call sites strip together)."""
    src = '''\
#define PAD_NOPS_1 __asm__(".section .text\\n    nop\\n")
#define PAD_NOPS_2 __asm__(".section .text\\n    nop\\n    nop\\n")

void f1(int a) {
    do_thing(a);
}
PAD_NOPS_1; /* 1 NOP after f1 */

void f2(int b) {
    do_thing(b);
}
PAD_NOPS_2; /* 2 NOPs after f2 */
'''
    stripped, n_stripped = volatile_cheats.strip_volatile_cheats_file(src)

    # Definitions: replaced with a /* STRIPPED CHEAT MACRO: ... */ comment so
    # line numbers are preserved (downstream tools may anchor by line).
    check("macro-strip: PAD_NOPS_1 def replaced with comment",
          "/* STRIPPED CHEAT MACRO: #define PAD_NOPS_1" in stripped)
    check("macro-strip: PAD_NOPS_2 def replaced with comment",
          "/* STRIPPED CHEAT MACRO: #define PAD_NOPS_2" in stripped)

    # Use sites: replaced with whitespace (NOT left as `PAD_NOPS_1;` — that
    # would leak to maspsx). The contract is that no `PAD_NOPS_<n>;` token
    # survives the strip, whether followed by a comment or end-of-line.
    eq("macro-strip: PAD_NOPS_1; use sites stripped",
       stripped.count("PAD_NOPS_1;"), 0)
    eq("macro-strip: PAD_NOPS_2; use sites stripped",
       stripped.count("PAD_NOPS_2;"), 0)

    # Real function bodies and surrounding text untouched.
    check("macro-strip: f1 body retained", "void f1(int a) {" in stripped)
    check("macro-strip: f2 body retained", "void f2(int b) {" in stripped)
    check("macro-strip: do_thing(a) retained", "do_thing(a)" in stripped)

    # Strip count should be >=1 per macro family (2 defs minimum); use-site
    # replacements may or may not count depending on implementation, but at
    # least the macro-def strips must be accounted for.
    check("macro-strip: nonzero strip count reported", n_stripped >= 2)


def test_volatile_unused_locals() -> None:
    """find_volatile_unused_locals — closes the `volatile T pad;` scalar
    frame-coercion loophole. 4 COMPLETED-C functions affected per the
    2026-06-02 thorough audit."""
    text = """\
void f(void) {
    volatile s32 pad;
    do_real_work();
}
"""
    hits = volatile_cheats.find_volatile_unused_locals(text, text.index("{"),
                                                       text.rindex("}") + 1)
    eq("volatile-unused: catches `volatile s32 pad;`", len(hits), 1)

    # Calibration loop usage NOT flagged (i is referenced)
    text2 = """\
void g(void) {
    volatile s32 i;
    for (i = 0; i < 100; i++) {
        delay();
    }
}
"""
    hits = volatile_cheats.find_volatile_unused_locals(text2, text2.index("{"),
                                                       text2.rindex("}") + 1)
    eq("volatile-unused: calibration counter NOT flagged", len(hits), 0)

    # Pointer-to-volatile NOT matched by regex
    text3 = """\
void h(void) {
    volatile s32 *ptr;
    ptr = (volatile s32 *)0x1F800000;
}
"""
    hits = volatile_cheats.find_volatile_unused_locals(text3, text3.index("{"),
                                                       text3.rindex("}") + 1)
    eq("volatile-unused: pointer-to-volatile NOT flagged", len(hits), 0)


def test_always_true_if_scaffolds() -> None:
    """find_always_true_if_scaffolds — closes `if (1) { ... }` wrapping.
    3 COMPLETED-C functions affected per 2026-06-02 thorough audit."""
    text = """\
void f(void) {
    if (1) {
        real_work();
        more_work();
    }
}
"""
    hits = volatile_cheats.find_always_true_if_scaffolds(text, text.index("{"),
                                                          text.rindex("}") + 1)
    eq("if(1): catches `if (1) { body }`", len(hits), 1)

    # Genuine non-trivial condition NOT flagged
    text2 = """\
void g(int x) {
    if (x) { real_work(); }
}
"""
    hits = volatile_cheats.find_always_true_if_scaffolds(text2, text2.index("{"),
                                                          text2.rindex("}") + 1)
    eq("if(1): genuine condition NOT flagged", len(hits), 0)


def test_empty_do_while_zero() -> None:
    """find_empty_do_while_zero — RETIRED 2026-06-04 per
    memory/project/sotn-do-while-zero-research-2026-06-04.md. SOTN ships
    `do { } while (0);` (both empty and non-empty body) as a matching
    technique. The detector now returns [] always. Test verifies the
    no-op behavior so a future agent doesn't accidentally re-enable it
    without also unwinding the SOTN-allowed status."""
    # Empty body — previously flagged, now NO-OP
    text = """\
void f(void) {
    real_work();
    do { } while (0);
    more_work();
}
"""
    hits = volatile_cheats.find_empty_do_while_zero(text, text.index("{"),
                                                     text.rindex("}") + 1)
    eq("empty-do-while-0 RETIRED: returns []", len(hits), 0)

    # Non-empty body — also no-op
    text2 = """\
void g(void) {
    do { real_work(); } while (0);
}
"""
    hits = volatile_cheats.find_empty_do_while_zero(text2, text2.index("{"),
                                                     text2.rindex("}") + 1)
    eq("empty-do-while-0 RETIRED: non-empty also returns []", len(hits), 0)

    # Non-zero while — also no-op (was previously not flagged, still isn't)
    text3 = """\
void h(int x) {
    do { } while (x);
}
"""
    hits = volatile_cheats.find_empty_do_while_zero(text3, text3.index("{"),
                                                     text3.rindex("}") + 1)
    eq("empty-do-while-0 RETIRED: while(non-zero) returns []", len(hits), 0)


def test_empty_if_dead_reads() -> None:
    """find_empty_if_dead_reads — closes the empty-body `if (cond) { }`
    dead-read loophole identified by the 2026-06-02 cheat-by-spelling
    audit (3 COMPLETED-C functions affected). Same family as
    dead-conditional-store / Lever D — a code construct with no
    semantic purpose, written purely to influence GCC's analysis."""

    # Positive — the exact audit pattern (`if (D_GLOBAL) {}`)
    text = """\
void f(void) {
    do_thing();
    if (g_entry_menu_handler_index) {}
    other_thing();
}
"""
    body_lo = text.index("{")
    body_hi = text.rindex("}") + 1
    hits = volatile_cheats.find_empty_if_dead_reads(text, body_lo, body_hi)
    eq("empty-if: catches `if (g_entry_menu_handler_index) {}`", len(hits), 1)

    # Positive — multiple instances
    text2 = """\
void g(void) {
    if (a) {}
    if (b) {}
    if (c) {}
}
"""
    hits = volatile_cheats.find_empty_if_dead_reads(text2, text2.index("{"),
                                                    text2.rindex("}") + 1)
    eq("empty-if: catches 3 separate empty-ifs", len(hits), 3)

    # Negative — if/else (legitimate sense-flip)
    text3 = """\
void h(int x) {
    if (cond) { } else { do_thing(); }
}
"""
    hits = volatile_cheats.find_empty_if_dead_reads(text3, text3.index("{"),
                                                    text3.rindex("}") + 1)
    eq("empty-if: if/else with empty if-body NOT flagged", len(hits), 0)

    # Negative — non-empty body
    text4 = """\
void i(void) {
    if (cond) { do_thing(); }
}
"""
    hits = volatile_cheats.find_empty_if_dead_reads(text4, text4.index("{"),
                                                    text4.rindex("}") + 1)
    eq("empty-if: non-empty body NOT flagged", len(hits), 0)

    # Strip integration
    text5 = """\
void j(void) {
    use_a();
    if (D_xxx) {}
    use_b();
}
"""
    stripped, _ = volatile_cheats.strip_volatile_cheats_file(text5)
    check("empty-if: strip removes the empty-if",
          "if (D_xxx) {}" not in stripped)

    # Count integration
    cnt = volatile_cheats.func_volatile_cheat_count(text5, "j")
    check("empty-if: func_volatile_cheat_count includes the cheat", cnt >= 1)


def test_void_discard_unused_locals() -> None:
    """find_void_discard_unused_locals — closes the `(void) name;` discard
    loophole (without `&`) identified by the 2026-06-02 audit. Same intent
    as `(void) &name;` frame-coercion. Parameters with the standard
    warning-suppression pattern are NOT flagged (legitimate)."""

    # Positive — local with discard
    text = """\
void f(void) {
    s32 dummy[2];
    (void) dummy;
    do_real_work();
}
"""
    body_lo = text.index("{")
    body_hi = text.rindex("}") + 1
    hits = volatile_cheats.find_void_discard_unused_locals(text, body_lo, body_hi, [])
    eq("void-discard: catches `(void) dummy;` array local",
       len(hits), 1)

    # Positive — scalar local
    text2 = """\
void g(void) {
    char new_var4;
    (void) new_var4;
    real_work();
}
"""
    hits = volatile_cheats.find_void_discard_unused_locals(
        text2, text2.index("{"), text2.rindex("}") + 1, [])
    eq("void-discard: catches `(void) new_var4;` scalar local",
       len(hits), 1)

    # Negative — parameter (legitimate warning suppression)
    text3 = """\
void h(int unused_param) {
    (void) unused_param;
    real_work();
}
"""
    params = ["unused_param"]
    hits = volatile_cheats.find_void_discard_unused_locals(
        text3, text3.index("{"), text3.rindex("}") + 1, params)
    eq("void-discard: `(void) param;` warning-suppression NOT flagged",
       len(hits), 0)

    # Negative — local with real use elsewhere
    text4 = """\
void i(void) {
    int x;
    x = 5;
    (void) x;
    return;
}
"""
    hits = volatile_cheats.find_void_discard_unused_locals(
        text4, text4.index("{"), text4.rindex("}") + 1, [])
    eq("void-discard: local with real use NOT flagged", len(hits), 0)

    # Strip integration
    text5 = """\
void j(void) {
    s32 _pad;
    (void) _pad;
}
"""
    stripped, _ = volatile_cheats.strip_volatile_cheats_file(text5)
    check("void-discard: strip removes the discard line",
          "(void) _pad;" not in stripped)


def test_dead_conditional_stores() -> None:
    """find_dead_conditional_stores — closes the dead-conditional-store
    loophole identified during the func_8007B844 directed-permuter session
    (2026-06-01). Sibling to find_addr_coerced_locals + the literal Lever D
    detector. User policy: cheats by any spelling are forbidden, full stop —
    permuter finds matching cheat-pattern families must be rejected, not
    surfaced for sanctioning."""

    # Positive case — agent's exact pattern
    text = """\
s32 f(u8 *ot, int debug) {
    u32 *p;
    if (debug) {
        do_thing();
        p = ot;
    }
    other_code();
    p = ot;
    *p = 5;
    return 0;
}
"""
    body_lo = text.index("{")
    body_hi = text.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text, body_lo, body_hi)
    eq("dead-cond-store: catches `if {...; p = ot;} ...; p = ot;`", len(hits), 1)
    if hits:
        eq("dead-cond-store: identifies the variable name", hits[0][2], "p")

    # Negative — outer comes BEFORE inner (legitimate init + re-init)
    text2 = """\
void g(void) {
    int i;
    i = 0;
    while (cond) {
        if (special) {
            i = 0;
        }
        do_work();
    }
}
"""
    body_lo = text2.index("{")
    body_hi = text2.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text2, body_lo, body_hi)
    eq("dead-cond-store: outer-before-inner (init + reset) NOT flagged",
       len(hits), 0)

    # Negative — block has early exit (goto), so inner is alive along exit path
    text3 = """\
void h(int x, int *p, int *ot) {
    if (x) {
        p = ot;
        goto done;
    }
    p = ot;
    *p = 1;
done:
    return;
}
"""
    body_lo = text3.index("{")
    body_hi = text3.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text3, body_lo, body_hi)
    eq("dead-cond-store: goto-bearing block NOT flagged (inner alive via exit)",
       len(hits), 0)

    # Negative — block has return
    text4 = """\
int i(int x, int *p, int *ot) {
    if (x) {
        p = ot;
        return 1;
    }
    p = ot;
    return 0;
}
"""
    body_lo = text4.index("{")
    body_hi = text4.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text4, body_lo, body_hi)
    eq("dead-cond-store: return-bearing block NOT flagged", len(hits), 0)

    # Negative — var used in gap between inner and outer
    text5 = """\
void j(int x, int *p, int *ot) {
    if (x) {
        p = ot;
    }
    use_ptr(p);
    p = ot;
}
"""
    body_lo = text5.index("{")
    body_hi = text5.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text5, body_lo, body_hi)
    eq("dead-cond-store: var used in gap NOT flagged (inner read before outer)",
       len(hits), 0)

    # Negative — break in loop block
    text6 = """\
void k(int *p, int *ot) {
    while (1) {
        if (cond) {
            p = ot;
            break;
        }
    }
    p = ot;
}
"""
    body_lo = text6.index("{")
    body_hi = text6.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text6, body_lo, body_hi)
    eq("dead-cond-store: break-bearing block NOT flagged", len(hits), 0)

    # Strip integration
    text7 = """\
void l(int debug, int *p, int *ot) {
    if (debug) {
        p = ot;
    }
    p = ot;
}
"""
    stripped, _ = volatile_cheats.strip_volatile_cheats_file(text7)
    check("dead-cond-store: strip removes the inner store",
          stripped.count("p = ot;") < text7.count("p = ot;"))

    # Count integration
    cnt = volatile_cheats.func_volatile_cheat_count(text7, "l")
    check("dead-cond-store: func_volatile_cheat_count includes the cheat",
          cnt >= 1)


def test_fake_annotated_lever_d_bypass() -> None:
    """dead-store-fake-exception.md (owner ruling 2026-07-01): a dead
    conditional store / dead param assign carrying a `/* FAKE */` or
    `// FAKE` annotation (same line or the line above) is a sanctioned
    last-resort lever — the Lever-D detectors skip it, so it also
    survives the cheat-strip and contributes to the honest distance.
    Un-annotated instances stay flagged (the annotation requirement is
    mechanically enforced)."""

    # Annotated dead conditional store (trailing comment) -> bypassed.
    text = """\
s32 f(u8 *ot, int debug) {
    u32 *p;
    if (debug) {
        do_thing();
        p = ot; /* FAKE: keeps p's allocno priority above q */
    }
    other_code();
    p = ot;
    *p = 5;
    return 0;
}
"""
    body_lo = text.index("{")
    body_hi = text.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text, body_lo, body_hi)
    eq("fake-annot: trailing /* FAKE */ dead-cond-store bypassed", len(hits), 0)

    # Annotated on the line ABOVE -> bypassed.
    text2 = """\
s32 g(u8 *ot, int debug) {
    u32 *p;
    if (debug) {
        do_thing();
        // FAKE: RA lever, see dead-store-fake-exception.md
        p = ot;
    }
    other_code();
    p = ot;
    *p = 5;
    return 0;
}
"""
    body_lo = text2.index("{")
    body_hi = text2.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text2, body_lo, body_hi)
    eq("fake-annot: line-above // FAKE dead-cond-store bypassed", len(hits), 0)

    # Un-annotated (plain trailing comment, no FAKE) -> still flagged.
    text3 = """\
s32 h(u8 *ot, int debug) {
    u32 *p;
    if (debug) {
        do_thing();
        p = ot; // reset pointer
    }
    other_code();
    p = ot;
    *p = 5;
    return 0;
}
"""
    body_lo = text3.index("{")
    body_hi = text3.rindex("}") + 1
    hits = volatile_cheats.find_dead_conditional_stores(text3, body_lo, body_hi)
    eq("fake-annot: non-FAKE comment still flagged", len(hits), 1)

    # Dead param assign: annotated -> bypassed; un-annotated -> flagged.
    text4 = """\
void k(int arg0) {
    do_thing();
    arg0 = 0; /* FAKE: breaks the a0 value association */
}
"""
    body_lo = text4.index("{")
    body_hi = text4.rindex("}") + 1
    hits = volatile_cheats.find_dead_param_assigns(text4, body_lo, body_hi, ["arg0"])
    eq("fake-annot: annotated dead param assign bypassed", len(hits), 0)

    text5 = """\
void m(int arg0) {
    do_thing();
    arg0 = 0;
}
"""
    body_lo = text5.index("{")
    body_hi = text5.rindex("}") + 1
    hits = volatile_cheats.find_dead_param_assigns(text5, body_lo, body_hi, ["arg0"])
    eq("fake-annot: un-annotated dead param assign still flagged", len(hits), 1)

    # Wrapped forms of the comment above (tools/format.py hoists long ones):
    # a `/* ... */` block and a `//` run ending on the line above count; a
    # FAKE comment trailing the previous statement, or one separated by a
    # blank line, does not.
    def dead_param(above: str) -> int:
        t = "void w(int arg0) {\n    do_thing();\n" + above + "    arg0 = 0;\n}\n"
        return len(volatile_cheats.find_dead_param_assigns(
            t, t.index("{"), t.rindex("}") + 1, ["arg0"]))
    eq("fake-annot: wrapped /* FAKE */ block above bypassed",
       dead_param("    /* FAKE: breaks the a0 value\n     * association */\n"), 0)
    eq("fake-annot: // FAKE run above bypassed",
       dead_param("    // FAKE: breaks the a0 value\n    // association\n"), 0)
    eq("fake-annot: FAKE trailing the previous statement still flagged",
       dead_param("    x = 1; /* FAKE: about x,\n              * not arg0 */\n"), 1)
    eq("fake-annot: FAKE block then a blank line still flagged",
       dead_param("    /* FAKE: breaks the a0 value\n     * association */\n\n"), 1)
    eq("fake-annot: non-FAKE block above still flagged",
       dead_param("    /* breaks the a0 value\n     * association */\n"), 1)
    eq("fake-annot: wrapped block that only mentions FAKE still flagged",
       dead_param("    /* not a fake store: it breaks\n     * the a0 association */\n"), 1)
    eq("fake-annot: // run whose label is not first still flagged",
       dead_param("    // breaks the a0 value\n    // FAKE: association\n"), 1)

    # Strip integration: the annotated store survives the cheat-strip.
    stripped, _ = volatile_cheats.strip_volatile_cheats_file(text)
    check("fake-annot: annotated store survives strip",
          stripped.count("p = ot;") == text.count("p = ot;"))


def test_volatile_extern_allowlist() -> None:
    """Narrow IRQ-touched-global carve-out (user policy 2026-06-08): the
    `extern volatile T D_xxxxxxxx;` plain-extern detector honors a
    per-symbol allowlist in `volatile_extern_allowlist.txt`. Listed
    symbols bypass the detector; non-listed symbols still trigger.

    The allowlist applies ONLY to pattern 3 (plain scalar extern). Pattern
    1 (alias renames), pattern 2 (inline `*(volatile T *)&G` casts),
    pattern 4 (non-volatile alias renames), and pattern 5 (macro `__asm__`)
    are unchanged — they stay forbidden regardless of allowlist membership."""

    # Snapshot the cache so we can restore it after the test.
    original_cache = volatile_cheats._volatile_extern_allowlist_cache
    original_cwd = os.getcwd()

    try:
        with tempfile.TemporaryDirectory() as td:
            os.chdir(td)
            volatile_cheats._volatile_extern_allowlist_cache = None  # force reload

            # 1. No allowlist file -> default ban applies.
            text = "extern volatile s32 D_80098894;\n"
            hits = volatile_cheats.find_plain_volatile_externs(text)
            eq("vol-allowlist: missing file -> default ban (1 hit)",
               len(hits), 1)

            # 2. Allowlist with the symbol -> bypassed.
            Path("volatile_extern_allowlist.txt").write_text(
                "# header comment\n"
                "D_80098894    # consumer — IRQ writer: StCdInterrupt:libcd/c_009.c:42\n"
                "\n"
                "D_8009ABCD\n"
            )
            volatile_cheats._volatile_extern_allowlist_cache = None  # force reload
            hits = volatile_cheats.find_plain_volatile_externs(text)
            eq("vol-allowlist: listed symbol bypassed (0 hits)", len(hits), 0)

            # 3. Non-listed symbol still triggers.
            text2 = "extern volatile s32 D_BADBADBA;\n"
            hits = volatile_cheats.find_plain_volatile_externs(text2)
            eq("vol-allowlist: non-listed symbol still flagged (1 hit)",
               len(hits), 1)

            # 4. The allowlist does NOT bypass OTHER families. An alias-rename
            # (pattern 1) on an allowlisted symbol is still a cheat.
            text3 = 'extern volatile s32 D_80098894_v asm("D_80098894");\n'
            hits = volatile_cheats.find_alias_renames(text3)
            eq("vol-allowlist: alias-rename on listed symbol still flagged",
               len(hits), 1)

            # 5. Inline volatile cast on an allowlisted symbol is still a cheat
            # (pattern 2 — different syntactic family).
            text4 = "void f(void) { *(volatile s32 *)&D_80098894 = 0; }\n"
            hits = volatile_cheats.find_global_volatile_casts(text4)
            eq("vol-allowlist: inline cast on listed symbol still flagged",
               len(hits), 1)

            # 6. Empty / comment-only allowlist file behaves like missing file.
            Path("volatile_extern_allowlist.txt").write_text(
                "# just a header\n# no symbols here\n\n"
            )
            volatile_cheats._volatile_extern_allowlist_cache = None
            hits = volatile_cheats.find_plain_volatile_externs(text)
            eq("vol-allowlist: comment-only file -> default ban",
               len(hits), 1)

            # 7. Cache invalidation: changing the file mtime + content updates
            # the result without an explicit cache-reset call.
            Path("volatile_extern_allowlist.txt").write_text(
                "D_80098894\n"
            )
            # Bump mtime explicitly in case the file-system resolution is coarse.
            import time as _t
            future = _t.time() + 2
            os.utime("volatile_extern_allowlist.txt", (future, future))
            hits = volatile_cheats.find_plain_volatile_externs(text)
            eq("vol-allowlist: cache picks up file changes (now bypassed)",
               len(hits), 0)
    finally:
        os.chdir(original_cwd)
        volatile_cheats._volatile_extern_allowlist_cache = original_cache


def test_addr_coerced_locals() -> None:
    """find_addr_coerced_locals — closes the (void)&local frame-coercion
    loophole identified during the func_8007CE0C de-cheat investigation
    (2026-06-01). Sibling to find_unused_local_arrays."""

    # Positive case — the exact agent-proposed pattern
    text = """\
void f(int arg0) {
    s32 stk_a, stk_b, stk_c, stk_d;
    (void)&stk_a;
    return;
}
"""
    body_lo = text.index("{")
    body_hi = text.rindex("}") + 1
    hits = volatile_cheats.find_addr_coerced_locals(text, body_lo, body_hi)
    eq("addr-coerced: catches `(void)&stk_a;`", len(hits), 1)
    if hits:
        eq("addr-coerced: identifies the variable name", hits[0][2], "stk_a")

    # Multi-coercion case
    text2 = """\
void g(void) {
    s32 a, b, c, d;
    (void)&a;
    (void)&b;
    (void)&c;
    (void)&d;
}
"""
    body_lo = text2.index("{")
    body_hi = text2.rindex("}") + 1
    hits = volatile_cheats.find_addr_coerced_locals(text2, body_lo, body_hi)
    eq("addr-coerced: catches all 4 separate coercions", len(hits), 4)

    # Negative — `&local` passed to a callee is real usage, NOT coercion
    text3 = """\
void h(void) {
    s32 ptr;
    callee(&ptr);
    use_ptr_value();
}
"""
    body_lo = text3.index("{")
    body_hi = text3.rindex("}") + 1
    hits = volatile_cheats.find_addr_coerced_locals(text3, body_lo, body_hi)
    eq("addr-coerced: `callee(&ptr)` is NOT flagged (real use)", len(hits), 0)

    # Negative — plain unused variable WITHOUT (void)& is just DCE'd, no frame impact
    text4 = """\
void i(void) {
    s32 unused;
    s32 used = 5;
    return used;
}
"""
    body_lo = text4.index("{")
    body_hi = text4.rindex("}") + 1
    hits = volatile_cheats.find_addr_coerced_locals(text4, body_lo, body_hi)
    eq("addr-coerced: declared-but-unused local without coercion is NOT flagged", len(hits), 0)

    # Wired-into-strip check — strip removes the coercion line
    text5 = """\
void j(void) {
    s32 stk_a;
    (void)&stk_a;
    return;
}
"""
    stripped, _ = volatile_cheats.strip_volatile_cheats_file(text5)
    check("addr-coerced: strip removes the `(void)&stk_a;` line",
          "(void)&stk_a;" not in stripped)

    # Wired-into-count check — counts the cheat. TWO since 2026-08-06: the
    # coercion statement AND the declaration it orphans (`s32 stk_a;` has no
    # remaining reference once `(void)&stk_a;` is stripped, and leaving it
    # standing left the frame reservation — the coercion's entire effect — in
    # the "honest" build). See find_orphaned_local_decls.
    cnt = volatile_cheats.func_volatile_cheat_count(text5, "j")
    eq("addr-coerced: func_volatile_cheat_count includes the coercion", cnt, 2)
    check("addr-coerced: strip also removes the orphaned declaration",
          "stk_a" not in volatile_cheats.strip_volatile_cheats_file(text5)[0])


def test_queue_reopen() -> None:
    """queue reopen: re-open a semantic-audit REGRESSION (owner ruling
    2026-07-06) as an ordinary INCOMPLETE queue item. The flagged construct is
    plain C -- the committed body is 0 rules / 0 honest distance / 0
    cheat-asm, i.e. exactly generate()'s COMPLETED-C `continue` case -- so the
    mechanical scan can never re-derive it as outstanding on its own.
    `origin: "regression"` items must survive `generate()`'s preserve step the
    same way `parked` items do."""
    with tempfile.TemporaryDirectory() as td:
        qp = Path(td) / "queue.json"
        qp.write_text(json.dumps({"items": [], "counts": {}}))
        orig_path = Q.QUEUE_PATH
        Q.QUEUE_PATH = str(qp)
        try:
            r = Q.reopen("func_REOPEN", "text1a_c", reason="regression: dead constant-holder")
            check("reopen: ok", r.get("ok") is True)
            q = Q.load()
            it = next((i for i in q["items"] if i["func"] == "func_REOPEN"), None)
            check("reopen: item landed in queue", it is not None)
            eq("reopen: status active", it["status"], "active")
            eq("reopen: origin regression", it["origin"], "regression")
            eq("reopen: file recorded", it["file"], "text1a_c")

            # duplicate rejected
            r2 = Q.reopen("func_REOPEN", "text1a_c", reason="dup attempt")
            check("reopen: duplicate rejected", r2.get("ok") is False)
            eq("reopen: duplicate leaves queue at 1 item", len(Q.load()["items"]), 1)

            # seed a PARKED item too -- the pre-existing preserve contract:
            # if the scan no longer produces a parked func, it's mechanically
            # COMPLETED and must drop (not zombie-re-added like regression
            # items, whose cheat the scan can never see in the first place).
            q_seed = Q.load()
            q_seed["items"].append({"func": "func_PARKED_GONE", "file": "text1a_c",
                                    "distance": 12, "verdict": "C", "rules": 1,
                                    "status": "parked", "park_reason": "test seed"})
            Q.save(q_seed)

            # regen preservation: an origin=="regression" item survives a scan
            # that (correctly) finds nothing mechanically outstanding for it.
            # Stub the scan's real-build dependencies so this stays a
            # pure-logic test (no build/ required).
            orig_stems = P.c_stems
            orig_scan = canonical.scan_all
            orig_canon_funcs = cheats.canonical_asm_funcs
            P.c_stems = lambda: []
            canonical.scan_all = lambda: []
            cheats.canonical_asm_funcs = lambda: set()
            try:
                q2 = Q.generate(workdir=str(Path(td) / "scan"), preserve=True)
            finally:
                P.c_stems = orig_stems
                canonical.scan_all = orig_scan
                cheats.canonical_asm_funcs = orig_canon_funcs
            it2 = next((i for i in q2["items"] if i["func"] == "func_REOPEN"), None)
            check("regen: regression-origin item NOT lost by preserve", it2 is not None)
            eq("regen: preserved item keeps origin", it2.get("origin") if it2 else None, "regression")
            eq("regen: preserved item stays active", it2["status"] if it2 else None, "active")
            # Owner ruling Q39 changed this: a LISTED item the scan no longer
            # produces drops only on a layer-2 PASS for its current body
            # (engine/queue.py generate's choke point); without one it is held.
            held = next((i for i in q2["items"] if i["func"] == "func_PARKED_GONE"), None)
            check("regen: listed item the scan no longer produces is HELD without a layer-2 PASS",
                  held is not None and "layer-2 gate" in held.get("layer2_pending", ""))
            P.c_stems, canonical.scan_all = (lambda: []), (lambda: [])
            cheats.canonical_asm_funcs = lambda: set()
            orig_gate = Q.layer2.gate
            Q.layer2.gate = lambda f, s: None   # a matching PASS on record
            try:
                q3 = Q.generate(workdir=str(Path(td) / "scan"), preserve=True)
            finally:
                P.c_stems, canonical.scan_all = orig_stems, orig_scan
                cheats.canonical_asm_funcs = orig_canon_funcs
                Q.layer2.gate = orig_gate
            gone = next((i for i in q3["items"] if i["func"] == "func_PARKED_GONE"), None)
            check("regen: ...and DROPS once its layer-2 PASS matches", gone is None)
        finally:
            Q.QUEUE_PATH = orig_path


# --------------------------------------------------------------------------
# BUILD-READ tier — canonical verdicts against the real linked ELF
# --------------------------------------------------------------------------

def test_canonical_build() -> None:
    if not Path("build/bb2.elf").exists():
        skip("canonical verdicts vs build/bb2.elf", "no build/bb2.elf — run verify-oracle --rebuild")
        return
    try:
        tbl = canonical._func_table()
    except Exception as e:  # objdump missing / unreadable ELF
        skip("canonical verdicts vs build/bb2.elf", f"_func_table failed: {e}")
        return
    if not tbl:
        skip("canonical verdicts vs build/bb2.elf", "empty symbol table")
        return
    check("func_table: nontrivial function count (>1000)", len(tbl) > 1000)
    scanned = {r["func"]: r for r in canonical.scan_all()}
    for func in ("_SendPAD", "SetGeomOffset"):
        check(f"canonical grant: {func} exists in the linked ELF", func in tbl)
        eq(f"canonical grant: classify({func})",
           canonical.classify(func)["verdict"], "ASM-WHOLE")
        eq(f"canonical grant: classify_full({func})",
           canonical.classify_full(func)["verdict"], "ASM-WHOLE")
        eq(f"canonical grant: scan_all({func})",
           scanned[func]["verdict"], "ASM-WHOLE")
    # motion_Close lives inside motion_Open.s -> the old asm/funcs gate said
    # NO-TARGET; the ELF gate must resolve it.
    if "__do_global_dtors" in tbl:
        eq("classify(__do_global_dtors): resolves its canonical grant",
           canonical.classify("__do_global_dtors")["verdict"], "ASM-WHOLE")
    if "math_ScaleVectorInDirectionFrame" in tbl:
        eq("classify(math_ScaleVectorInDirectionFrame): reviewed mixed C/GTE -> ASM-PARTIAL",
           canonical.classify("math_ScaleVectorInDirectionFrame")["verdict"], "ASM-PARTIAL")
    if "gpu_GetLastGridPassViewYaw" in tbl:
        eq("classify(gpu_GetLastGridPassViewYaw): ordinary C -> C",
           canonical.classify("gpu_GetLastGridPassViewYaw")["verdict"], "C")
    # no function should be NO-TARGET (the motion_Close-class fix)
    no_target = [r["func"] for r in canonical.scan_all() if r["verdict"] == "NO-TARGET"]
    eq("scan_all: zero NO-TARGET", len(no_target), 0)


def test_rodata_object_alignment() -> None:
    """Owner ruling 2026-09-30 (.claude/rules/rodata-object-alignment.md): ONE uniform rule for
    every C object -- .rodata alignment set to 4 after `as` -- and the retired per-file
    `.align 3 -> .align 2` sed must not come back in the engine or the Makefile."""
    from engine import pipeline, buildconfig as bcfg
    eq("rodata rule: the uniform objcopy flag", bcfg.RODATA_OBJ_ALIGN, "--set-section-alignment .rodata=4")
    check("rodata rule: RODATA_ALIGN2_FILES retired from buildconfig", not hasattr(bcfg, "RODATA_ALIGN2_FILES"))
    stems = pipeline.c_stems()
    check("rodata rule: C stems found", len(stems) > 10)
    bad_sed, bad_tail = [], []
    for stem in stems:
        cmd = pipeline.c_pipeline_cmd(stem, "tmp/x.o")
        if ".align" in cmd:
            bad_sed.append(stem)
        if not cmd.endswith(f" && {bcfg.OBJCOPY} {bcfg.RODATA_OBJ_ALIGN} tmp/x.o"):
            bad_tail.append(stem)
    eq("rodata rule: no stem rewrites .align", bad_sed, [])
    eq("rodata rule: every stem ends with the uniform objcopy", bad_tail, [])
    mk = Path("Makefile").read_text()
    check("rodata rule: Makefile has no per-file align sed", "RODATA_ALIGN2_FILES :=" not in mk and "rodata_align_fix" not in mk)
    check("rodata rule: Makefile applies the uniform objcopy after as",
          "$(AS) $(AS_FLAGS) -o $@\n\t$(OBJCOPY) $(RODATA_OBJ_ALIGN) $@\n" in mk)


def test_score_object_paths() -> None:
    """score reads objects through an argv list, so an ABSOLUTE path whose
    directory contains spaces must give the same
    answer as the relative path the engine normally passes. Under the old
    shell-interpolated command the path word-split into an empty symbol table
    and surfaced as a bogus '<func> not found in <obj>'."""
    import shutil
    objs = sorted(Path("build/src").rglob("*.o"))
    if not objs:
        skip("score: absolute object path with spaces", "no build/src/**/*.o — run build first")
        return
    if shutil.which(cfg.OBJDUMP) is None:
        skip("score: absolute object path with spaces", f"{cfg.OBJDUMP} not on PATH")
        return
    # objdump IS available, so any failure past here is a real regression —
    # let it surface rather than degrading to a skip.
    rel = None
    for o in objs:
        tbl = score._o_func_table(str(o))
        if tbl:
            rel = (o, tbl)
            break
    if rel is None:
        skip("score: absolute object path with spaces", "no object with functions")
        return
    o, tbl_rel = rel
    Path("tmp").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="score path with spaces ", dir="tmp") as td:
        copied = Path(td) / o.name
        shutil.copyfile(o, copied)
        absolute = str(copied.resolve())
        check("score: test path really contains a space (else this proves nothing)",
              " " in absolute)
        tbl_abs = score._o_func_table(absolute)
        eq("score: _o_func_table(absolute with spaces) == relative", tbl_abs, tbl_rel)
        func = sorted(tbl_rel)[0]
        eq("score: normalized_insns(absolute) == normalized_insns(relative)",
           score.normalized_insns(absolute, func), score.normalized_insns(str(o), func))
        eq("score: func_byte_signature(absolute) == relative",
           score.func_byte_signature(absolute, func), score.func_byte_signature(str(o), func))
        check("score: signature is non-empty (an empty read would fake equality)",
              len(score.func_byte_signature(absolute, func)) > 0)
        eq("score: score_func(absolute, relative) == 0 (identical objects)",
           score.score_func(absolute, str(o), func)["score"], 0)


def test_prologue_cheat() -> None:
    """prologue_fix is a TRACKED cheat (audit 2026-06-15): the cheat-invisible
    sandbox STRIPS it (empty / per-function-filtered configs) so the honest
    distance is real, and func_prologue_count flags entry-carrying functions so
    a prologue-reordered function cannot pass `queue done` as COMPLETED-C."""
    with tempfile.TemporaryDirectory() as d:
        dp = Path(d)
        pc = dp / "prologue_config.json"
        pc.write_text(json.dumps({"funcP": ["sw\t$s0,0x30($sp)"], "funcQ": ["x"]}))
        dl = dp / "delay_slot_ra_funcs.txt"; dl.write_text("# hdr\nfuncP\nfuncR\n")
        ff = dp / "frame_fix_funcs.txt"; ff.write_text("funcS 56\n")
        orig = (cheats.PROLOGUE_CONFIG, cheats.DELAY_SLOT_RA, cheats.FRAME_FIX)
        cheats.PROLOGUE_CONFIG, cheats.DELAY_SLOT_RA, cheats.FRAME_FIX = (str(pc), str(dl), str(ff))
        try:
            eq("prologue_fix_funcs: union of the 3 lists", cheats.prologue_fix_funcs(),
               {"funcP", "funcQ", "funcR", "funcS"})
            eq("func_prologue_count: config+delay", cheats.func_prologue_count("funcP"), 2)
            eq("func_prologue_count: config-only", cheats.func_prologue_count("funcQ"), 1)
            eq("func_prologue_count: frame-only", cheats.func_prologue_count("funcS"), 1)
            eq("func_prologue_count: none", cheats.func_prologue_count("funcZ"), 0)
            # strip ALL (sandbox file-wide / build_stripped_object): prologue_fix off everywhere
            ov_all = cheats._write_prologue_overrides(dp / "ovA", None)
            eq("strip-all: prologue_config emptied",
               json.loads(Path(ov_all["prologue_config_path"]).read_text()), {})
            check("strip-all: delay list emptied of funcs",
                  "funcP" not in Path(ov_all["delay_slot_ra_path"]).read_text()
                  and "funcR" not in Path(ov_all["delay_slot_ra_path"]).read_text())
            check("strip-all: dropped count >= 4", ov_all["dropped_prologue"] >= 4)
            # strip ONE func: only its entries removed
            ov_p = cheats._write_prologue_overrides(dp / "ovP", "funcP")
            cfg_p = json.loads(Path(ov_p["prologue_config_path"]).read_text())
            check("strip-funcP: funcP removed", "funcP" not in cfg_p)
            check("strip-funcP: funcQ kept", "funcQ" in cfg_p)
            check("strip-funcP: delay funcP removed, funcR kept",
                  "funcP" not in Path(ov_p["delay_slot_ra_path"]).read_text()
                  and "funcR" in Path(ov_p["delay_slot_ra_path"]).read_text())
            # make_overrides('all') wires the prologue strip; 'lost-codegen' does NOT
            mo = cheats.make_overrides("funcP", "all", str(dp / "mo"))
            check("make_overrides(all): prologue path present", "prologue_config_path" in mo)
            check("make_overrides(all): funcP stripped",
                  "funcP" not in json.loads(Path(mo["prologue_config_path"]).read_text()))
            mol = cheats.make_overrides("funcP", "lost-codegen", str(dp / "mol"))
            check("make_overrides(lost-codegen): prologue UNtouched",
                  "prologue_config_path" not in mol)
            # empty_overrides (build_stripped_object) strips prologue too
            eo = cheats.empty_overrides(str(dp / "eo"))
            eq("empty_overrides: prologue emptied",
               json.loads(Path(eo["prologue_config_path"]).read_text()), {})
        finally:
            cheats.PROLOGUE_CONFIG, cheats.DELAY_SLOT_RA, cheats.FRAME_FIX = orig


def test_substitute_body() -> None:
    """Swapping a candidate body into a copy of the source — the mechanism the
    chassis check needs to re-measure a banked floor.

    Motivation: the driver's chassis check shipped 2026-08-18 grepping for a
    `"distance"` key sandbox never emitted, AND its premise expired the next day
    when asm-until-matched made main carry INCLUDE_ASM for every incomplete
    function. Scoring main answers 'how big is this function'; only scoring the
    candidate answers 'is the ledger floor still real'."""
    body = "s32 func_X(void)\n{\n    return 1;\n}\n"

    # 1. The asm-until-matched representation: the INCLUDE_ASM line AND its
    #    trailing semicolon are replaced, leaving no stray `;`.
    src = 'extern int g;\nINCLUDE_ASM("asm/funcs", func_X);\nvoid after(void) {}\n'
    out = inlineasm.substitute_body(src, "func_X", body)
    check("substitute_body: INCLUDE_ASM line is gone", "INCLUDE_ASM" not in out)
    check("substitute_body: candidate body is present", "return 1;" in out)
    check("substitute_body: no orphaned semicolon", ";\nvoid after" not in out)
    check("substitute_body: surrounding text survives",
          out.startswith("extern int g;") and out.rstrip().endswith("void after(void) {}"))

    # 2. Only the NAMED function is replaced when several are present.
    two = ('INCLUDE_ASM("asm/funcs", func_A);\n'
           'INCLUDE_ASM("asm/funcs", func_X);\n')
    out = inlineasm.substitute_body(two, "func_X", body)
    check("substitute_body: sibling INCLUDE_ASM untouched",
          'INCLUDE_ASM("asm/funcs", func_A);' in out and "func_X);" not in out)

    # 3. An existing C definition is replaced whole, with nothing eaten after
    #    the closing brace (the _match_brace span end is EXCLUSIVE — an
    #    off-by-one here silently deletes the next character).
    csrc = "s32 func_X(void)\n{\n    return 0;\n}\nint tail = 7;\n"
    out = inlineasm.substitute_body(csrc, "func_X", body)
    check("substitute_body: old C body replaced", "return 0;" not in out)
    check("substitute_body: new body present", "return 1;" in out)
    eq("substitute_body: trailing declaration intact (no off-by-one)",
       out.count("int tail = 7;"), 1)
    check("substitute_body: no truncated tail", out.rstrip().endswith("int tail = 7;"))

    # 4. A function that is in NEITHER form must raise, not silently no-op —
    #    a silent no-op would score main and report it as the candidate's floor.
    try:
        inlineasm.substitute_body("int unrelated;\n", "func_X", body)
        check("substitute_body: absent function raises", False)
    except KeyError:
        check("substitute_body: absent function raises", True)


def test_brace_prefixed_definition_head() -> None:
    previous = "void prev(void) { return;\n}"
    replacement = "s32 func_X(void) { return 2; }"
    for gap in ("", " ", "\t", " \t "):
        body = "s32 func_X(void) { return 1; }"
        prefix = previous + gap
        src = prefix + body + "\n"
        span = (len(prefix), len(prefix) + len(body))
        eq(f"brace head {gap!r}: starts at type",
           inlineasm.func_definition_candidates(src, "func_X"), [span])
        eq(f"brace head {gap!r}: substitution preserves previous brace and gap",
           inlineasm.substitute_body(src, "func_X", replacement),
           prefix + replacement + "\n")
        eq(f"brace head {gap!r}: attribution uses corrected span",
           inlineasm.attributed_spans(src, "func_X"), [("func_X", span)])
        eq(f"brace head {gap!r}: TU brace walk agrees",
           [(f.start, f.end) for f in inlineasm.tu_functions(src)
            if f.name == "func_X"], [span])
        eq(f"brace head {gap!r}: previous span unchanged",
           inlineasm._func_body_span(src, "prev"), (0, len(previous)))
    for desc, body in (("ordinary", "s32 func_X(void) { return 1; }"),
                       ("split", "static s32\nfunc_X(void) { return 1; }")):
        src = previous + "\n" + body + "\n"
        start = len(previous) + 1
        eq(f"brace head: {desc} span unchanged",
           inlineasm.func_definition_candidates(src, "func_X"),
           [(start, start + len(body))])
    src = previous + "s32 func_X(void) {}\ns32 func_Y(void) {}\n"
    eq("brace head: head order unchanged", inlineasm.definition_head_names(src),
       ["prev", "func_X", "func_Y"])


def test_additional_file_scope_heads() -> None:
    from engine import volatile_cheats

    cheat = 'register int x; __asm__ volatile("addu $8,$3,$0");'
    for gap in ("", " ", "\t"):
        prefix = "void prev(void) {\n}" + gap
        body = "int\nf(void) { " + cheat + " return 1; }"
        src = prefix + body + "\n"
        span = (len(prefix), len(prefix) + len(body))
        eq(f"additional head: brace split {gap!r}",
           inlineasm.func_definition_candidates(src, "f"), [span])
        eq(f"additional head: brace split replacement {gap!r}",
           inlineasm.substitute_body(src, "f", "int f(void) {}"),
           prefix + "int f(void) {}\n")
    for indent in (" ", "  ", "\t", " \t "):
        body = "int f(void) { " + cheat + " return 1; }"
        src = indent + body + "\n"
        span = (len(indent), len(indent) + len(body))
        eq(f"additional head: indented {indent!r}",
           inlineasm.func_definition_candidates(src, "f"), [span])
        eq(f"additional head: indented roster {indent!r}",
           [(f, lo, hi) for f, lo, hi, _ in
            volatile_cheats._iter_func_bodies(src)], [("f", *span)])
        check(f"additional head: cheats visible {indent!r}",
              inlineasm.func_cheat_asm_count(src, "f") > 0)
    lookalikes = (
        "  int f(void) { return 1; }",
        "  int f(void);",
        "  f(x);",
        "  (int)f(x);",
        "  if (x) (int)y;",
        "  MACRO(x) { x++; }",
        "  int f(x) int x; { return x; }",
        "  {\n}int\nf(void) { return 1; }",
    )
    for statement in lookalikes:
        src = "void outer(void) {\n" + statement + "\n}\n"
        eq(f"additional head: body lookalike {statement!r}",
           inlineasm.definition_head_names(src), ["outer"])
    for body in ("  int f(void) {}", "void p(void) {\n}int\nf(void) {}"):
        for prefix, suffix in (("/*\n", "\n*/"),
                               ('const char *s = "\\\n', '\\n";'),
                               ("#if 0\n", "\n#endif"),
                               ("// spliced \\\n", "\n")):
            src = prefix + body + suffix
            eq(f"additional head: inactive {prefix!r} {body!r}",
               inlineasm.func_definition_candidates(src, "f"), [])
    # Braces in literals, comments and macro replacements must not hide a head.
    prefix = 'const char *s = "{"; /* { */\n#define OPEN {\n'
    src = prefix + "  int f(void) {}"
    eq("additional head: masked braces do not alter scope",
       inlineasm.func_definition_candidates(src, "f"),
       [(len(prefix) + 2, len(src))])
    prefix = '#if 0\n"\n#endif\n'
    for body in ("  int f(void) {}", "void p(void) {\n}int\nf(void) {}"):
        src = prefix + body
        eq("additional head: disabled stray quote does not hide live head",
           inlineasm.func_definition_candidates(src, "f"),
           [(src.index("int f") if "int f" in body else src.index("int\nf"),
             len(src))])


def test_file_scope_head_review() -> None:
    from engine import volatile_cheats

    for alternative in ("#else", "#elif OTHER\nvoid g(char a) {\n#else"):
        src = ("#ifdef OPTION\nvoid g(int a) {\n" + alternative
               + "\nvoid g(void) {\n#endif\n"
               "    if (a) {}\n    else if (a) { a++; }\n"
               "    int nested(void) { return 1; }\n}\n"
               "  int tail(void) {}\n")
        g_span = (src.index("void g(void)"), src.index("\n}\n") + 2)
        eq("scope review: alternate-head g span unchanged",
           inlineasm.func_definition_candidates(src, "g"), [g_span])
        eq("scope review: alternate heads do not expose body definitions",
           inlineasm.definition_names(src), ["g", "tail"])
        eq("scope review: roster has no if/nested body",
           [(name, lo, hi) for name, lo, hi, _ in
            volatile_cheats._iter_func_bodies(src)],
           [("g", *g_span), ("tail", src.index("int tail"),
                                src.index("int tail") + len("int tail(void) {}"))])
        check("scope review: joined depth is first arm exit",
              src.index("int nested") not in inlineasm._file_scope_tokens(src))
    for region in ("#ifdef X\n{\n#else\n#endif\n",
                   "#ifdef X\n#else\n{\n#endif\n",
                   "#ifdef X\n{\n#elif Y\n{\n#else\n#endif\n",
                   "#ifdef X\n{\n#endif\n"):
        src = "  int before(void) {}\n" + region + "  int after(void) {}\n"
        eq("scope review: unequal exits stop new heads",
           inlineasm.definition_names(src), ["before"])
    # A #ifdef with no #else has an implicit empty arm: a `}`-only arm
    # exits one level below its entry, so the exits differ and fail closed.
    src = ("void g(void) {\n#ifdef X\n}\n#endif\n"
           "  if (a) { }\n  int z(void) {}\n}\n")
    eq("scope review: implicit empty arm keeps an in-body head hidden",
       inlineasm.definition_names(src), ["g"])
    # `%:` is the `#` digraph: its directives are skipped and its #if 0
    # arms removed like `#` ones; an #elif 0 arm is removed too.
    for src in ("void f(void) {\n%:define CLOSE }\n"
                "  int z(void) { return 0; }\n}\n",
                "%:if 0\n  int z(void) { return 0; }\n%:endif\n",
                "#if A\n#elif 0\n  int z(void) { return 0; }\n#endif\n"):
        eq("scope review: %: directives and #elif 0 arms hide z",
           inlineasm.func_definition_candidates(src, "z"), [])
    src = ("void g(void) <%\n  int hidden(void) {}\n%>\n"
           "  int tail(void) {}\n")
    eq("scope review: digraph body is not file scope",
       inlineasm.func_definition_candidates(src, "hidden"), [])
    eq("scope review: digraph close restores file scope",
       inlineasm.func_definition_candidates(src, "tail"),
       [(src.index("int tail"), len(src) - 1)])
    for keyword in ("if", "else", "while", "for", "switch", "return", "sizeof",
                    "do", "case", "goto", "volatile", "__asm__", "asm",
                    "typeof", "__typeof__", "int", "static", "const"):
        for src in (f"  int {keyword}(void) {{}}",
                    f"void p(void) {{\n}}int\n{keyword}(void) {{}}"):
            eq(f"scope review: keyword name {keyword}",
               inlineasm.func_definition_candidates(src, keyword), [])
    for keyword in ("if", "else", "while", "for", "switch", "return", "sizeof",
                    "do", "case", "goto", "volatile", "__asm__", "asm",
                    "typeof", "__typeof__", "static", "const"):
        for src in (f"  {keyword} f(void) {{}}",
                    f"void p(void) {{\n}}{keyword}\nf(void) {{}}"):
            eq(f"scope review: keyword type tail {keyword}",
               inlineasm.func_definition_candidates(src, "f"), [])
    for src in ('  __asm__ volatile("nop");',
                'void p(void) {\n}__asm__\nvolatile("nop");'):
        eq("scope review: file-scope asm is not a head named volatile",
           inlineasm._definition_heads(src, "volatile"), [])
        eq("scope review: file-scope asm has no definition body",
           inlineasm.func_definition_candidates(src, "volatile"), [])
    # A second splice pass would remove the remaining backslash + newline,
    # shifting scope offsets relative to compiler_view(text).pos.
    prefix = "\\" * 2 + "\n\n"
    src = prefix + "  int f(void) {}"
    eq("scope review: scope token positions survive one splice only",
       inlineasm._file_scope_tokens(src),
       frozenset(i for i, c in enumerate(src) if c.isalpha()))
    eq("scope review: already-spliced indexes stay aligned",
       inlineasm.func_definition_candidates(src, "f"),
       [(len(prefix) + 2, len(src))])


def test_shared_brace_head_review() -> None:
    prefixes = ("int g(void) {\n  }", "int g(void) { return 0; }")
    for prefix in prefixes:
        for separator in (" ", "\n"):
            src = prefix + "int" + separator + "f(void) {}"
            eq("shared brace: plain int head span",
               inlineasm.func_definition_candidates(src, "f"),
               [(len(prefix), len(src))])
    for prefix in prefixes:
        for separator in (" ", "\n"):
            for gap in ("", " \t"):
                head = "static int *" + separator + "f(void) {}"
                src = prefix + gap + head
                expected = [(len(prefix + gap), len(src))]
                eq("shared brace: span starts at return type",
                   inlineasm.func_definition_candidates(src, "f"), expected)
                eq("shared brace: census sees both bodies",
                   inlineasm.definition_names(src), ["g", "f"])
                eq("shared brace: head census sees both heads",
                   inlineasm.definition_head_names(src), ["g", "f"])
                eq("shared brace: substitution preserves preceding brace",
                   inlineasm.substitute_body(src, "f", "int f(void) {}"),
                   prefix + gap + "int f(void) {}")
                ambiguous = src + "\nint f(void) {}"
                eq("shared brace: duplicate stays ambiguous",
                   len(inlineasm.func_definition_candidates(ambiguous, "f")), 2)
                for keyword in ("if", "else", "int", "return", "volatile",
                                "__asm__", "typeof", "const"):
                    bad = prefix + gap + "int" + separator + keyword + "(void) {}"
                    eq("shared brace: keyword name rejected",
                       inlineasm.func_definition_candidates(bad, keyword), [])
                    if keyword != "int":
                        bad = prefix + gap + keyword + separator + "f(void) {}"
                        eq("shared brace: keyword type tail rejected",
                           inlineasm.func_definition_candidates(bad, "f"), [])
    for separator in (" ", "\n"):
        for prefix in ("int g(void) {\n  if (x) { }",
                       "int g(void) { if (x) { }"):
            for tail in ("int" + separator + "f(void) {}",
                         "else if (x) {}", "int y;"):
                src = prefix + tail + "\n}"
                eq("shared brace: body look-alikes hidden",
                   inlineasm.definition_names(src), ["g"])
                eq("shared brace: body head names hidden",
                   inlineasm.definition_head_names(src), ["g"])
        for prefix in prefixes:
            head = prefix + "int" + separator + "f(void) {}"
            for src in ("/*\n" + head + "\n*/",
                        "#if 0\n" + head + "\n#endif",
                        "#ifdef X\n{\n#endif\n" + head):
                eq("shared brace: masked or unknown scope hides head",
                   inlineasm.func_definition_candidates(src, "f"), [])
    # A cached roster is immutable; callers get independent lists and edits
    # select a new cache entry even when only the function name changes.
    src = prefixes[0] + "int f(void) {}"
    inlineasm._definition_heads_cached.cache_clear()
    heads = inlineasm._definition_heads(src)
    heads.clear()
    inlineasm.func_definition_candidates(src, "f")
    eq("head cache: one scan across names",
       inlineasm._definition_heads_cached.cache_info().misses, 1)
    eq("head cache: caller mutation cannot erase heads",
       inlineasm.definition_head_names(src), ["g", "f"])
    eq("head cache: changed text has new names",
       inlineasm.definition_names(src.replace("f(void)", "h(void)")), ["g", "h"])


def test_delimited_definition_heads() -> None:
    prefix = "int g(void) { if (a) { b(); } }"
    src = prefix + "int\nf(void) {}"
    eq("shared brace: last of two braces bounds split head",
       inlineasm.func_definition_candidates(src, "f"), [(len(prefix), len(src))])
    src = "int ff(void) {}\nint f(void) {}"
    eq("head lookup: f never includes ff span",
       inlineasm.func_definition_candidates(src, "f"),
       [(src.index("int f("), len(src))])
    eq("head lookup: ff keeps its own span",
       inlineasm.func_definition_candidates(src, "ff"), [(0, src.index("}") + 1)])
    prefixes = ("int g(void) {} /*c*/ ",
                "int g(void) {} __attribute__((unused)) ",
                "int x; ", "struct S { int a; } s; ")
    for prefix in prefixes:
        for separator in (" ", "\n"):
            head = "int" + separator + "f(void) {}"
            src = prefix + head
            span_start = len(prefix)
            expected = [(span_start, len(src))]
            eq("delimited head: span begins at return type",
               inlineasm.func_definition_candidates(src, "f"), expected)
            check("delimited head: definition census includes f",
                  "f" in inlineasm.definition_names(src))
            check("delimited head: head census includes f",
                  "f" in inlineasm.definition_head_names(src))
            eq("delimited head: substitution preserves delimiter and gap",
               inlineasm.substitute_body(src, "f", "int f(void) {}"),
               src[:span_start] + "int f(void) {}")
            eq("delimited head: duplicate remains ambiguous",
               len(inlineasm.func_definition_candidates(src + "\nint f(void) {}", "f")), 2)
            for keyword in ("if", "else", "int", "return", "volatile", "__asm__"):
                bad = prefix + "int" + separator + keyword + "(void) {}"
                eq("delimited head: keyword name rejected",
                   inlineasm.func_definition_candidates(bad, keyword), [])
                if keyword != "int":
                    bad = prefix + keyword + separator + "f(void) {}"
                    eq("delimited head: keyword type tail rejected",
                       inlineasm.func_definition_candidates(bad, "f"), [])
            for wrapper in ("int outer(void) {\n%s\n}", "/*\n%s\n*/",
                            '#if 0\n%s\n#endif',
                            '#ifdef X\n{\n#endif\n%s'):
                eq("delimited head: nested, masked or unknown scope excluded",
                   inlineasm.func_definition_candidates(
                       wrapper % (src.replace("/*c*/", "")
                                  if wrapper.startswith("/*") else src), "f"), [])
            # Unknown but balanced arms remain visible: no clean fallback
            # when one arm supplies another spelling of the same definition.
            conditional = "#ifdef X\n" + src + "\n#else\nint f(void) {}\n#endif"
            eq("delimited head: unknown alternatives stay ambiguous",
               len(inlineasm.func_definition_candidates(conditional, "f")), 2)
    # A comment or attribute inside the return type: after `}` the raw path
    # cannot read the head, so the masked path must (as after `;`).
    for prefix in ("int g(void) {} ", "int x; "):
        for head in ("static __attribute__((unused)) int f(void) {}",
                     "int /*c*/ f(void) {}",
                     "static int __attribute__((x)) f(void) {}"):
            src = prefix + head
            eq("delimited head: in-type comment or attribute, span from type",
               inlineasm.func_definition_candidates(src, "f"), [(len(prefix), len(src))])


def test_definition_head_debt() -> None:
    # Every added spelling must be found with its WHOLE type/attribute span.
    cases = []
    for prefix in ("", "\n", "int g(void) {}\n", "int g(void) {}\n\n",
                   "int x; ", "int g(void) {} "):
        for lead in ("__attribute__((unused)) static int ", "/*c*/ int "):
            head = lead + "f(void) {}"
            start = len(prefix) + (len("/*c*/ ") if lead.startswith("/*")
                                   else len("__attribute__((unused)) "))
            cases.append((prefix + head, start))
    for prefix in ("int x; ", "int g(void) {} "):
        cases.append((prefix + "int\n    f(void) {}", len(prefix)))
    for kind in ("struct", "union", "enum"):
        for tag in (" S", ""):
            body = " A, B " if kind == "enum" else " int a; struct N { int b; } n; "
            for prefix in ("", "int g(void) {} ", "int x; "):
                cases.append((prefix + kind + tag + " {" + body + "} f(void) {}",
                              len(prefix)))
    cases += [("struct S {\n int a;\n union { int b; } n;\n} f(void) {}", 0),
              ("__attribute__((\nunused\n)) static int f(void) {}",
               len("__attribute__((\nunused\n)) ")),
              ("__attribute__((aligned(4))) __attribute__((unused)) int f(void) {}",
               len("__attribute__((aligned(4))) __attribute__((unused)) "))]
    for src, start in cases:
        eq("head debt: exact span", inlineasm.func_definition_candidates(src, "f"),
           [(start, len(src))])
        check("head debt: definition census", "f" in inlineasm.definition_names(src))
        check("head debt: head census", "f" in inlineasm.definition_head_names(src))
        eq("head debt: substitution keeps delimiter",
           inlineasm.substitute_body(src, "f", "int f(void) {}"),
           src[:start] + "int f(void) {}")
        eq("head debt: duplicate ambiguous",
           len(inlineasm.func_definition_candidates(src + "\nint f(void) {}", "f")), 2)
        try:
            inlineasm.substitute_body(src + "\nint f(void) {}", "f", "int f(void) {}")
        except KeyError:
            check("head debt: duplicate substitution refuses", True)
        else:
            check("head debt: duplicate substitution refuses", False)
        # Avoid invalid nested C comments in the comment look-alike.
        plain = src.replace("/*c*/", "")
        for wrapped in ("int outer(void) {\n" + src + "\n}",
                        "/*\n" + plain + "\n*/", '#if 0\n' + src + '\n#endif',
                        'const char *s = "' + src.replace('\n', ' ') + '";',
                        '#ifdef X\n{\n#endif\n' + src):
            eq("head debt: lexical look-alike excluded",
               inlineasm.func_definition_candidates(wrapped, "f"), [])
        for keyword in ("if", "return", "int", "__asm__", "__attribute__"):
            eq("head debt: keyword name rejected",
               inlineasm.func_definition_candidates(src.replace("f(void)", keyword + "(void)"),
                                                    keyword), [])
        conditional = '#if X\n' + src + '\n#else\n' + src + '\n#endif'
        eq("head debt: unknown alternatives ambiguous",
           len(inlineasm.func_definition_candidates(conditional, "f")), 2)
    # Preprocessor prose must be removed BEFORE multiline quote masking.
    for prefix in ("#error don't\n", '#warning "unclosed\n',
                   "#if 0\ndon't\n#endif\n", '#if 0\n"\n#else\n#endif\n',
                   "%:warning don't\n", "#error don't \\\ncontinued\n"):
        head = "/*c*/ __attribute__((unused)) int\n    f(void) {}"
        src = prefix + head
        start = len(prefix) + len("/*c*/ __attribute__((unused)) ")
        eq("head debt: directive quote cannot mask live head",
           inlineasm.func_definition_candidates(src, "f"), [(start, len(src))])
        view, masked, _ = inlineasm._definition_view(src)
        eq("head debt: view mapping length", len(view.pos), len(masked) + 1)
        eq("head debt: view mapping end", view.pos[-1], len(src))
        eq("head debt: substitution keeps directive",
           inlineasm.substitute_body(src, "f", "int f(void) {}"),
           src[:start] + "int f(void) {}")
    src = 'int g(void) {} __attribute__((noinline)) int f(void) {}'
    eq("head debt: leading attribute outside span", inlineasm.func_definition_candidates(src, "f"),
       [(src.index("int f("), len(src))])


def test_definition_head_consumers() -> None:
    from engine import gtemacro

    src = "__attribute__((unused)) static int h(int a) { return a; }"
    eq("head params: unnamed fallback skips leading attribute",
       volatile_cheats._extract_func_params(src, (0, len(src))), ["a"])
    src = "__attribute__((unused)) static int h(void) { return 0; }"
    eq("head params: attribute is not a parameter in void fallback",
       volatile_cheats._extract_func_params(src, (0, len(src))), [])
    for noise in ("/* ) */", '/* "(" */', '__attribute__((annotate(")")))'):
        src = "int f(int " + noise + " a, int b) { return a; }"
        eq("head params: masked parentheses preserve both parameters",
           volatile_cheats._extract_func_params(src, (0, len(src)), "f"),
           ["a", "b"])
    src = "int f(int a /* ) */, int b { return a; }"
    eq("head params: comment cannot close an unbalanced parameter list",
       volatile_cheats._extract_func_params(src, (0, len(src)), "f"), [])
    src = "int x;\nstatic int\n    f(void) {}"
    eq("head debt: prior-line semicolon admits split head",
       inlineasm.func_definition_candidates(src, "f"),
       [(src.index("static int"), len(src))])
    src = "int x;\nstatic int\n    f(void);\nint g(void) {}"
    eq("head debt: split declaration cannot borrow next body",
       inlineasm.func_definition_candidates(src, "f"), [])

    plain = "void f(int a, int b) {\n a = 0;\n g();\n}"
    eq("head consumers: plain cheat baseline", inlineasm.func_cheat_asm_count(plain, "f"), 1)
    for prefix in ("", "int g(void) {} ", "int g(void) {}\n"):
        for ret in ("__attribute__((unused)) void",
                    "static __attribute__((unused)) void",
                    "struct S { int (*fp)(void); }",
                    "union S { int (*fp)(void); }",
                    "enum E { A = sizeof(int) }"):
            src = prefix + plain.replace("void f(", ret + " f(")
            span = inlineasm.func_definition_candidates(src, "f")[0]
            eq("head consumers: actual parameter names",
               volatile_cheats._extract_func_params(src, span), ["a", "b"])
            eq("head consumers: cheat count equals plain head",
               inlineasm.func_cheat_asm_count(src, "f"), 1)
            eq("head consumers: body brace excludes tag brace",
               gtemacro._body_open(src, span), src.index("{\n a ="))
    for conditional in ("#ifdef X\n__attribute__((unused))\n#endif\n",
                        "#ifdef X\n__attribute__((unused))\n#else\n"
                        "__attribute__((noinline))\n#endif\n"):
        prefix = "int a;\n" + conditional
        src = prefix + "static int f(void) { return 1; }"
        eq("head consumers: conditional attributes outside span",
           inlineasm.func_definition_candidates(src, "f"), [(len(prefix), len(src))])
        eq("head consumers: substitution keeps all directives",
           inlineasm.substitute_body(src, "f", "int f(void) {}"),
           prefix + "int f(void) {}")
        # These split names need a leading attribute to admit them. Neither
        # a directive nor a blank line may carry that attribute into the head.
        src = prefix + "int\n    f(void) {}"
        eq("head consumers: directive breaks leading attribute",
           inlineasm.func_definition_candidates(src, "f"), [])
    src = "__attribute__((unused))\n\nint\n    f(void) {}"
    eq("head consumers: blank line clears leading attribute",
       inlineasm.func_definition_candidates(src, "f"), [])
    src = "__attribute__((unused))\nint\n    f(void) {}"
    eq("head consumers: adjacent attribute admits split head",
       inlineasm.func_definition_candidates(src, "f"), [(src.index("int\n"), len(src))])
    # TU-index helpers the head grammar does not read keep their parameters
    # (Lever D still counts the helper's dead parameter store in the caller).
    for helper in ("static inline int\n    helper(int a) {\n  a = 0;\n  g();\n  return 0;\n}",
                   "static inline int (*helper(int a))(void) {\n  a = 0;\n  g();\n  return 0;\n}",
                   "static inline int (*helper(int a))[4] {\n  a = 0;\n  g();\n  return 0;\n}"):
        src = "void g(void);\n" + helper + "\nvoid f(int b) { helper(b); }\n"
        eq("head consumers: helper dead param store counted",
           inlineasm.func_cheat_asm_count(src, "f"), 1)
    # Head-shaped text inside a helper's body (or a comment in its head) never
    # supplies the parameters.
    for helper in ("static inline int\n    helper(int a) {\n/*\nint helper(int q) */\n  a = 0;\n  g();\n  return 0;\n}",
                   "static inline int (*helper(int a))(void) {\n/*\nint helper(int q) */\n  a = 0;\n  g();\n  return 0;\n}",
                   "static inline int (*helper(int a))(void) {\n#if 0\nint helper(int q)\n#endif\n  a = 0;\n  g();\n  return 0;\n}",
                   "static inline int (* /* helper(int q) */ helper(int a))(void) {\n  a = 0;\n  g();\n  return 0;\n}",
                   "static inline int (*helper(int a))[4] {\n/* see:\nint helper(int unused) */\n  a = 0;\n  g();\n  return 0;\n}"):
        src = "void g(void);\n" + helper + "\nvoid f(int b) { helper(b); }\n"
        eq("head consumers: look-alike head never supplies helper params",
           inlineasm.func_cheat_asm_count(src, "f"), 1)
    src = ("void g(void);\nstatic inline int (*helper(int a, int b))(void) {\n  b = 0;\n"
           "  return a;\n}\nvoid f(int c) { helper(c, c); }\n")
    eq("head consumers: helper second param store counted",
       inlineasm.func_cheat_asm_count(src, "f"), 1)
    src = "__attribute__((\n__attribute__((unused)) int f(void) {}"
    eq("head consumers: unbalanced attribute cannot hide later attribute",
       inlineasm.func_definition_candidates(src, "f"), [(src.index("int f("), len(src))])


def test_split_definition_head() -> None:
    split = "static s32\n\nfunc_X(s32 a) { return a; }"
    src = "extern int g;\n" + split + "\nint tail;\n"
    expected = [(src.index("static"), src.index("}") + 1)]
    eq("split head: span includes type line, exclusive end",
       inlineasm.func_definition_candidates(src, "func_X"), expected)
    eq("split head: definition census", inlineasm.definition_names(src), ["func_X"])
    replacement = "s32 func_X(s32 a) { return 1; }"
    eq("split head: substitution replaces both lines",
       inlineasm.substitute_body(src, "func_X", replacement),
       "extern int g;\n" + replacement + "\nint tail;\n")
    decoy = "#if 0\ns32 func_X(s32 a) { return 0; }\n#endif\n"
    ambiguous = decoy + split
    eq("split head: disabled copy is ambiguous",
       len(inlineasm.func_definition_candidates(ambiguous, "func_X")), 2)
    try:
        inlineasm.substitute_body(ambiguous, "func_X", replacement)
    except KeyError:
        refused = True
    else:
        refused = False
    check("split head: ambiguous substitution refuses", refused)
    declared = "extern s32 func_X(s32 a);\n" + split
    eq("split head: extern declaration adds no candidate",
       inlineasm.func_definition_candidates(declared, "func_X"),
       [(declared.index("static"), len(declared))])
    for desc, text in (
            ("indented name", "static s32\n    func_X(s32 a) { return a; }"),
            ("semicolon", "static s32;\nfunc_X(s32 a) { return a; }"),
            ("closing paren", "other()\nfunc_X(s32 a) { return a; }")):
        eq(f"split head: reject {desc}",
           inlineasm.func_definition_candidates(text, "func_X"), [])
    knr = "static s32 *\nfunc_X(a) s32 a; { return 0; }"
    eq("split head: pointer return and K&R declarations",
       inlineasm.func_definition_candidates(knr, "func_X"), [(0, len(knr))])
    from engine import volatile_cheats
    eq("split head: the cheat roster's body walk sees it",
       [(f, lo, hi) for f, lo, hi, _ in volatile_cheats._iter_func_bodies(src)],
       [("func_X", *expected[0])])
    eq("split head: decoy and split body both walked",
       len(list(volatile_cheats._iter_func_bodies(ambiguous))), 2)
    eq("split head: head names list it", inlineasm.definition_head_names(src),
       ["func_X"])


def test_body_span_ambiguous() -> None:
    from engine import layer2

    fake = 's32 func_X(s32 a) { return a; }\n'
    real = ('s32 func_X(s32 a) {\n'
            '    __asm__ volatile("addu $8,$3,$0");\n'
            '    return a + 1;\n}\n')
    for desc, prefix, suffix in (
        ("block comment", "/* old version:\n" + fake + "*/\n", ""),
        # The splice makes the column-0 head part of the // comment to cpp.
        ("line comment", "// old version: \\\n" + fake, ""),
        ("if 0", "#if 0\n" + fake + "#endif\n", ""),
        ("if 0 else", "#if 0\n" + fake + "#else\n", "#endif\n"),
        ("if 1 else", "#if 1\n", "#else\n" + fake + "#endif\n"),
        ("string literal", 'char *s = "old version:\n' + fake + '";\n', ""),
        ("unclosed quote directive", 'char *s = "x\n#if 0\n";\n'
         + fake + '#else\n";\n', "#endif\n"),
        ("digraph directives", '%:if 0\n' + fake + '%:else\n', '%:endif\n'),
        ("ifdef branches", "#ifdef X\n" + fake + "#else\n", "#endif\n"),
    ):
        src = prefix + real + suffix
        eq(f"body-span ambiguous: {desc}",
           inlineasm._func_body_span(src, "func_X"), None)
        eq(f"body-span ambiguous: cheat count unknown for {desc}",
           inlineasm.func_cheat_asm_count(src, "func_X"), -1)
        try:
            inlineasm.substitute_body(src, "func_X", fake)
        except KeyError:
            refused = True
        else:
            refused = False
        check(f"body-span ambiguous: substitute refuses {desc}", refused)
        eq(f"body-span ambiguous: no layer2 key for {desc}",
           layer2.body_key(src, "func_X"), None)
    for desc, prefix, body in (
        ("brace prefix", "void prev(void) { return;\n",
         "}s32 func_X(s32 a) { return a; }"),
        ("K&R parameters", "", "s32 func_X(a) s32 a; { return a; }"),
        ("extern declaration", "extern s32 func_X(s32 a);\n", real.rstrip("\n")),
    ):
        eq(f"body-span unique: exact original span for {desc}",
           inlineasm._func_body_span(prefix + body + "\n", "func_X"),
           (len(prefix) + (1 if body.startswith("}") else 0),
            len(prefix) + len(body)))


def test_ambiguous_c_asm_fallbacks() -> None:
    from engine import gtemacro, layer2, sandbox, tus
    from unittest.mock import patch

    fake = 's32 func_X(s32 a) { return a; }\n'
    real = 's32 func_X(s32 a) { __asm__("addu $8,$3,$0"); return a; }\n'
    asm = 'INCLUDE_ASM("asm/funcs", func_X);\n'
    for desc, src, count in (
        ("reviewer one-line comment", "/* " + fake.rstrip("\n") + " */\n"
         + "#if 0\n" + asm + "#endif\n" + real, 1),
        ("ambiguous comment and asm", "/* old version:\n" + fake + "*/\n"
         + "#if 0\n" + asm + "#endif\n" + real, 2),
        ("GNUC C or asm", "#ifdef __GNUC__\n" + real + "#else\n"
         + asm + "#endif\n", 1),
    ):
        eq(f"C/asm fallback: candidate count for {desc}",
           len(inlineasm.func_definition_candidates(src, "func_X")), count)
        with patch.object(inlineasm, "_read_src_cached", return_value=src):
            issues = completion.source_issues("fake", "func_X", canonical=True)
        check(f"C/asm fallback: completion refuses {desc}", bool(issues))
        if count > 1:
            eq("C/asm fallback: explicit ambiguity issue", issues,
               ["ambiguous C definition: 2 definition-shaped heads"])
            eq("C/asm fallback: ambiguous cheat count remains unknown",
               inlineasm.func_cheat_asm_count(src, "func_X"), -1)
            try:
                gtemacro.call_sites(src, "func_X")
            except ValueError:
                refused = True
            else:
                refused = False
            check("C/asm fallback: macro call scan refuses ambiguity", refused)
        eq(f"C/asm fallback: no layer2 key for {desc}",
           layer2.body_key(src, "func_X", read=lambda p: "asm body"), None)
        with patch.object(Path, "read_text", return_value=src):
            eq(f"C/asm fallback: queue sees C for {desc}",
               Q._no_c_body("fake", "func_X"), False)

    # Exercise sandbox_score's missing-symbol fallback without building/writing.
    for desc, src, no_c in (("ambiguous C", "/*\n" + fake + "*/\n" + real + asm, False),
                            ("GNUC C", "#ifdef __GNUC__\n" + real + "#else\n"
                             + asm + "#endif\n", False),
                            ("reviewer one-line comment", "/* " + fake.rstrip("\n")
                             + " */\n#if 0\n" + asm + "#endif\n" + real, False),
                            ("assembly only", asm, True)):
        with patch.object(sandbox, "func_file", return_value="fake"), \
             patch.object(sandbox.cheats, "empty_overrides", return_value={}), \
             patch.object(sandbox.cheats, "func_rule_lines", return_value=[]), \
             patch.object(inlineasm, "write_stripped", return_value=0), \
             patch.object(sandbox.pipeline, "build_c_object"), \
             patch.object(Path, "exists", return_value=True), \
             patch.object(Path, "read_text", return_value=src), \
             patch.object(sandbox.score, "score_func", side_effect=KeyError("missing")), \
             patch.object(sandbox.score, "normalized_insns", return_value=["target"]):
            result = sandbox.sandbox_score("func_X", disable="all", strip_cheat_asm=True)
            eq(f"C/asm fallback: sandbox no_c_body for {desc}",
               result.get("no_c_body", False), no_c)
            eq(f"C/asm fallback: sandbox scorable for {desc}", result["scorable"], no_c)
            eq(f"C/asm fallback: queue agrees for {desc}", Q._no_c_body("fake", "func_X"), no_c)

    ambiguous = "/*\n" + fake + "*/\n" + real
    with patch.object(tus, "src_tus", return_value=["fake"]), \
         patch.object(layer2, "_read_text", return_value=ambiguous):
        eq("C/asm fallback: ambiguous TU still found by locate_stems",
           layer2.locate_stems("func_X"), ["fake"])


def test_ambiguous_body_discovery_and_stripping() -> None:
    from engine import layer2, tus
    from unittest.mock import patch

    cheat = ('s32 func_DECOY(s32 a) {\n'
             '    volatile s32 pad_a;\n    a = 0;\n    return 1;\n}\n')
    decoy = 's32 func_DECOY(s32 b) { return b; }\n'
    sources = {"src/a.c": cheat + "/* old version:\n" + decoy + "*/\n",
               "src/b.c": "#if 0\n" + decoy + "#endif\n"}
    with patch.object(tus, "src_tus", return_value=["a", "b"]), \
         patch.object(layer2, "_read_text", side_effect=lambda p: sources[p.as_posix()]), \
         patch.object(Path, "read_text", return_value='{"items": []}'):
        eq("body discovery: real+comment TU and disabled-only TU both count",
           layer2.locate_stems("func_DECOY"), ["a", "b"])
        try:
            layer2.locate_stem("func_DECOY")
        except layer2.AmbiguousStem as exc:
            stems = exc.stems
        else:
            stems = []
        eq("body discovery: decoys cannot redirect stem selection", stems, ["a", "b"])
        sources["src/b.c"] = cheat + "#if 0\n" + decoy + "#endif\n"
        eq("layer2: a TU with an #if 0 second copy still counts as a candidate",
           layer2.locate_stems("func_DECOY"), ["a", "b"])

    other = cheat.replace("pad_a", "pad_b").replace("s32 a", "s32 b").replace("a =", "b =")
    for desc, src in (("commented copy", "/* old version:\n" + decoy + "*/\n" + cheat),
                      ("disabled copy", "#if 0\n" + other + "#endif\n" + cheat)):
        candidates = inlineasm.func_definition_candidates(src, "func_DECOY")
        bodies = list(volatile_cheats._iter_func_bodies(src))
        eq(f"body discovery: every {desc} span yielded once",
           [(lo, hi) for _name, lo, hi, _params in bodies], candidates)
        eq(f"body discovery: each {desc} has its own parameters",
           [params for _name, _lo, _hi, params in bodies], [["b"], ["a"]])
        hits = volatile_cheats.find_all_cheats(src)
        stripped, count = volatile_cheats.strip_volatile_cheats_file(src)
        for snippet in (["volatile s32 pad_a;", "a = 0;"] if desc == "commented copy"
                        else ["volatile s32 pad_a;", "a = 0;", "volatile s32 pad_b;", "b = 0;"]):
            start = src.index(snippet)
            check(f"body discovery: roster finds {snippet} after {desc}",
                  any(s <= start and e >= start + len(snippet) for s, e, _anchor in hits))
            check(f"body discovery: stripper removes {snippet} after {desc}", snippet not in stripped)
        check(f"body discovery: nonzero strip count for {desc}", count >= 2)
        check(f"body discovery: real return survives {desc}", "return 1;" in stripped)

    prefixed = ('void prev(void) { return;\n}s32 func_Y(s32 a) {\n'
                '    volatile s32 pad_a;\n    a = 0;\n    return 1;\n}\n')
    bodies = list(volatile_cheats._iter_func_bodies(prefixed))
    eq("body discovery: brace-prefixed func_Y is yielded",
       [(name, params) for name, _lo, _hi, params in bodies],
       [("prev", []), ("func_Y", ["a"])])
    hits = volatile_cheats.find_all_cheats(prefixed)
    stripped, count = volatile_cheats.strip_volatile_cheats_file(prefixed)
    for snippet in ("volatile s32 pad_a;", "a = 0;"):
        start = prefixed.index(snippet)
        check(f"body discovery: brace-prefixed roster finds {snippet}",
              any(s <= start and e >= start + len(snippet) for s, e, _anchor in hits))
        check(f"body discovery: brace-prefixed stripper removes {snippet}", snippet not in stripped)
    eq("body discovery: brace-prefixed strip count", count, 2)


def test_tool_ambiguous_cheat_readers() -> None:
    from engine import integrate
    from tools.spotcheck import spot_check_completed as spot
    import audit_asm_cheats as audit
    import headless_review as headless
    from unittest.mock import patch

    cheat = ('s32 func_X(s32 a) {\n'
             '    __asm__("addu $8,$3,$0");\n    return a;\n}\n')
    copy = 's32 func_X(s32 a) { return a; }\n'
    rec = {"func": "func_X", "file": "fake", "advanced": True,
           "head_before": "same", "head_after": "same"}

    def review(src, count=None):
        with patch.object(headless, "queue_item", return_value=None), \
             patch.object(headless.ha, "find_projects_dir", return_value=None), \
             patch.object(headless, "uncommitted_leftover", return_value=[]), \
             patch.object(headless, "classify_outcome", return_value="MATCHED"), \
             patch.object(cheats, "canonical_asm_funcs", return_value=set()), \
             patch.object(inlineasm, "_read_src_cached", return_value=src), \
             patch.object(inlineasm, "file_func_cheat_asm_count",
                          side_effect=count or (lambda stem, f: inlineasm.func_cheat_asm_count(src, f))):
            return headless.review(rec)

    for desc, src in (("comment copy", cheat + "/* old version:\n" + copy + "*/\n"),
                      ("disabled copy", cheat + "#if 0\n" + copy + "#endif\n")):
        for unknown in (True, False):
            completed, violations, no_body = spot.classify_pool(
                {"fake": ["func_X"]}, set(), set(), {}, set(), {"fake": src},
                lambda p: None, unknown_is_completed=unknown)
            eq(f"tool ambiguity: spot excludes {desc}, unknown={unknown}", completed, [])
            check(f"tool ambiguity: spot reports {desc}, unknown={unknown}",
                  len(violations) == 1 and "ambiguous C definition: 2" in violations[0])
            eq(f"tool ambiguity: spot does not call {desc} absent", no_body, [])
        packet = review(src)
        eq(f"tool ambiguity: headless escalates {desc}", packet["decision"], "ESCALATE")
        check(f"tool ambiguity: headless names {desc}", "ambiguous C definition: 2" in packet["why"])
        for authorized in (set(), {"func_X"}):
            err = io.StringIO()
            with patch.object(Path, "rglob", return_value=[Path("src/fake.c")]), \
                 patch.object(Path, "read_text", return_value=src), \
                 patch.object(audit, "load_authorized", return_value=authorized), \
                 contextlib.redirect_stderr(err):
                rc = audit.cmd_func("func_X")
            eq(f"tool ambiguity: audit refuses {desc}, authorized={bool(authorized)}", rc, 1)
            check(f"tool ambiguity: audit names {desc}",
                  "UNAUTHORIZED" in err.getvalue() and "ambiguous C definition: 2" in err.getvalue())

    def fail_count(stem, f):
        raise RuntimeError("test failure")

    for desc, counter in (("UNKNOWN", lambda stem, f: -1),
                          ("audit exception", fail_count)):
        packet = review(copy, counter)
        eq(f"tool ambiguity: headless escalates {desc}", packet["decision"], "ESCALATE")
        check(f"tool ambiguity: headless explains {desc}",
              ("UNKNOWN" if desc == "UNKNOWN" else "RuntimeError: test failure") in packet["why"])
        eq(f"tool ambiguity: headless does not clamp {desc}", packet["match_cheat_asm"], -1)
    eq("tool ambiguity: headless still accepts uniquely clean C", review(copy)["decision"], "ACCEPT")

    # Unlocated spot-check symbols keep their existing pool policy; this task
    # distinguishes ambiguous definitions from that separate absence policy.
    with patch.object(Q, "not_a_c_function_text", return_value=False):
        completed, violations, _no_body = spot.classify_pool(
            {"fake": ["absent"]}, set(), set(), {}, set(), {"fake": cheat}, lambda p: None)
    eq("tool ambiguity: spot absent-body policy unchanged", (completed, violations), ([("absent", "fake")], []))

    src = cheat + "#if 0\n" + copy + "#endif\n"
    with patch.object(integrate.sandbox, "func_file", return_value="fake"), \
         patch.object(integrate, "_backup", return_value={}), \
         patch.object(integrate, "_restore"), \
         patch.object(integrate.cheats, "_filter_text", return_value=("", 1)), \
         patch.object(integrate.cheats, "drop_prologue_fix_entries", return_value=0), \
         patch.object(Path, "write_bytes"), \
         patch.object(integrate, "_rebuild_file_and_sha1", return_value="mismatch"), \
         patch.object(inlineasm, "file_func_cheat_asm_count", return_value=-1), \
         patch.object(inlineasm, "_read_src_cached", return_value=src):
        result = integrate.retire_function("func_X")
    check("tool ambiguity: failed retirement hint names ambiguous C",
          not result["ok"] and "ambiguous C definition: 2" in result["hint"])


def test_data_as_code_conflicts() -> None:
    import check_completion_integrity as CCI
    from tools.spotcheck import spot_check_completed as spot
    from unittest.mock import patch

    base = Path("src/main/d_7D870.c").read_text(encoding="utf-8")
    cases = []
    for func in ("g_data_start", "g_main_loop_handlers"):
        decoy = f"void {func}(void) {{ }}\n"
        cases.extend(((f"{func} plain data", base, func, False),
                      (f"{func} disabled decoy", base + "#if 0\n" + decoy + "#endif\n", func, True),
                      (f"{func} commented decoy", base + "/* old version:\n" + decoy + "*/\n", func, True)))
    cheat = ('s32 func_X(s32 a) {\n'
             '    volatile s32 pad;\n    a = 0;\n    return 1;\n}\n')
    copy = 's32 func_X(s32 a) { return a; }\n'
    ambiguous = cheat + "#if 0\n" + copy + "#endif\n// .aent func_X\n"
    cases.extend((("real C and comment signal", cheat + "// .aent func_X\n", "func_X", True),
                  ("ambiguous C and comment signal", ambiguous, "func_X", True)))
    for text in (ambiguous, ambiguous + 'INCLUDE_ASM("asm/funcs", func_X);\n'):
        try:
            inlineasm.substitute_body(text, "func_X", copy)
        except KeyError as exc:
            reason = str(exc)
        else:
            reason = ""
        check("data conflict: substitution names ambiguity even with INCLUDE_ASM",
              "ambiguous C definition: 2" in reason and "neither" not in reason)

    with contextlib.ExitStack() as stack:
        for obj, attr, value in (
            (P, "c_stems", lambda: ["fake"]),
            (Path, "exists", lambda p: p.as_posix() == "build/src/fake.o"),
            (inlineasm, "_read_src_cached", lambda stem: src),
            (canonical, "scan_all", lambda: []),
            (cheats, "canonical_asm_funcs", lambda: set()),
            (cheats, "func_prologue_count", lambda f: 0),
            (cheats, "func_rule_lines", lambda *a: []),
            (cheats, "all_keyed_functions", lambda: set()),
            (cheats, "maspsx_gate_entries", lambda f: []),
            (cheats, "is_jtbl_infra", lambda f: False),
            (cheats, "is_canonical_extraction_only", lambda f: False),
            (Q, "_rule_count", lambda f: 0),
            (Q, "_fingerprint", lambda: "dry"),
            (Q, "_locked", contextlib.nullcontext),
            (Q, "_route_with_evidence", lambda *a: ("C", None)),
            (Q.sandbox, "build_stripped_object", lambda *a, **k: {}),
            (score, "_o_func_table", lambda p: {func: (0, 4)}),
            (score, "score_func", lambda *a: {"score": 0}),
            (Q.layer2, "addr_index", lambda: {func: "80010000"}),
            (CCI.buildstamp, "check", lambda: {"fresh": True}),
            (inlineasm, "project_header_texts", lambda *a: []),
            (inlineasm, "project_header_macro_facts", lambda *a, **k: frozenset()),
            (CCI, "_helper_issues", lambda *a: []),
            (CCI, "_island_funcs", lambda *a: {}),
            (CCI.buildconfig, "LINKED_ASM_FUNCS", set()),
            (CCI.departures, "unreviewed_departures", lambda: []),
            (completion, "grant_issues", lambda *a: []),
            (CCI.gtemacro, "check_vendored", lambda: []),
            (CCI.gtemacro, "header_registry_issues", lambda: []),
            (CCI, "_header_function_issues", lambda **k: []),
            (CCI, "_header_safety_issues", lambda **k: []),
            (CCI.CRN, "scan", lambda *a: ([], [], [])),
        ):
            stack.enter_context(patch.object(obj, attr, value))
        stack.enter_context(patch.object(Q, "save"))
        for desc, src, func, conflict in cases:
            eq(f"data conflict: structural signal preserved for {desc}",
               Q.not_a_c_function_text(src, func, lambda p: None), True)
            eq(f"data conflict: shared helper for {desc}", Q.data_as_code_conflict(src, func), conflict)
            for missing in (False, True):
                with patch.object(score, "score_func", side_effect=KeyError("missing") if missing else None,
                                  return_value={"score": 0}):
                    q = Q.generate(workdir="tmp/codex/mock-queue")
                eq(f"data conflict: queue membership for {desc}, missing={missing}",
                   [it["func"] for it in q["items"]], [func] if conflict else [])
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                rc = CCI.main()
            eq(f"data conflict: integrity result for {desc}", rc, 1 if conflict else 0)
            eq(f"data conflict: integrity diagnostic for {desc}",
               Q.DATA_AS_CODE_CONFLICT in out.getvalue(), conflict)
            eq(f"data conflict: integrity data-as-code bucket for {desc}",
               "data-as-code symbols" in out.getvalue(), not conflict)
            check(f"data conflict: never counted as completed for {desc}",
                  "COMPLETED-C:                    0 functions" in out.getvalue())
            done, violations, no_body = spot.classify_pool(
                {"fake": [func]}, set(), set(), {}, set(), {"fake": src}, lambda p: None)
            eq(f"data conflict: spot never completes {desc}", done, [])
            eq(f"data conflict: spot diagnostic for {desc}",
               violations, [f"{func} (fake.c): {Q.DATA_AS_CODE_CONFLICT}"] if conflict else [])
            eq(f"data conflict: spot no-body bucket for {desc}", no_body,
               [] if conflict else [(func, "fake")])


def test_include_asm_whole_body() -> None:
    """A whole-body INCLUDE_ASM must never read as clean, pure-C, or complete.

    Regression for the 2026-08-06 Campaign-4 pilot finding: the stripper runs on
    UNEXPANDED source, so `INCLUDE_ASM(...)` never matched cia.ASM_KEYWORD_RE
    (which only knows `__asm__`/`__asm`) and was not even a candidate for
    stripping. A function with zero lines of decompiled C therefore scored an
    honest pure-C distance of 0 and counted as clean (-1 read as <= 0), so
    queue.generate dropped it and mark_done would have recorded it COMPLETED-C.
    """
    inc = 'INCLUDE_ASM("asm/funcs", PClseek);\n'

    # 1. Recognition + attribution.
    eq("include_asm: macro invocation recognised",
       [f for f, _s, _e in inlineasm.include_asm_spans(inc)], ["PClseek"])
    check("include_asm: named in whole_body_asm_funcs",
          "PClseek" in inlineasm.whole_body_asm_funcs(inc))
    bios = 'BIOS_B_FUNCTION(DeliverEvent, 0x7);\n'
    eq("include_asm: BIOS trampoline macro recognised",
       [f for f, _s, _e in inlineasm.include_asm_spans(bios)], ["DeliverEvent"])
    check("include_asm: BIOS trampoline named in whole_body_asm_funcs",
          "DeliverEvent" in inlineasm.whole_body_asm_funcs(bios))
    eq("include_asm: direct BIOS_FUNCTION(name, vector, id) recognised",
       [f for f, _s, _e in inlineasm.include_asm_spans('BIOS_FUNCTION(Exec, 0xA0, 0x43);\n')], ["Exec"])
    eq("include_asm: a non-numeric BIOS id is NOT recognised (could inject asm)",
       inlineasm.include_asm_spans('BIOS_B_FUNCTION(f, 0x1; jr $ra);\n'), [])
    eq("include_asm: BIOS_FUNCTION with a non-vector address is NOT recognised",
       inlineasm.include_asm_spans('BIOS_FUNCTION(f, 0x80, 0x1);\n'), [])
    for inj in ('BIOS_B_FUNCTION(bar, 0x1; .word 0x1);',
                'BIOS_FUNCTION(bar, 0x80, 0x1);',
                'BIOS_FUNCTION(bar, 0xB0, 0x1; nop);',
                'BIOS_B_FUNCTION(bar, 0x7);'):
        body = "void foo(void) {\n    " + inj + "\n}\n"
        check(f"bios: in-body invocation counted as a cheat ({inj})",
              inlineasm.func_cheat_asm_count(body, "foo") >= 1)
        out, _n = inlineasm.strip_cheat_asm_file(body)
        check(f"bios: in-body invocation stripped from the sandbox ({inj})",
              "FUNCTION" not in out)
    ind = "void foo(void) {\n    BIOS_B_FUNCTION(bar, 0x7);\n    x = 1;\n}\nvoid baz(void) {\n    y = 2;\n}\n"
    out, _n = inlineasm.strip_cheat_asm_file(ind)
    check("bios: stripping an indented in-body invocation keeps the following code intact",
          "    x = 1;\n}\nvoid baz(void) {\n    y = 2;\n}\n" in out and "BIOS" not in out)
    trail = "BIOS_B_FUNCTION(bar, 0x7);   \nint baz(void) {\n    return 2;\n}\n"
    out, _n = inlineasm.strip_cheat_asm_file(trail)
    check("bios: stripping a file-scope invocation with trailing blanks keeps the next function",
          "int baz(void) {\n    return 2;\n}\n" in out and "BIOS" not in out)
    evil = ("#define EVIL(x) BIOS_B_FUNCTION(x, 0x1; .word 1)\n"
            "void foo(void) {\n    EVIL(bar);\n}\n")
    check("bios: a local #define wrapping a BIOS macro counts at its use site",
          inlineasm.func_cheat_asm_count(evil, "foo") >= 1)
    loose = 'BIOS_B_FUNCTION(bar, 0x1; .word 0x1);\n'
    eq("bios: a loose file-scope invocation attributes nothing (stays UNKNOWN)",
       inlineasm.func_cheat_asm_count(loose, "bar"), -1)
    check("include_asm: a #define of the BIOS macro is not an invocation",
          inlineasm.include_asm_spans('#define BIOS_B_FUNCTION(name, id) BIOS_FUNCTION(name, 0xB0, id)\n') == [])
    check("include_asm: a #define of the macro is not an invocation",
          inlineasm.include_asm_spans('#define INCLUDE_ASM(F, N) __asm__()\n') == [])

    # 2. Stripped for scoring — otherwise the sandbox assembles target bytes
    #    straight from asm/funcs/<name>.s and reports distance 0.
    out, n = inlineasm.strip_cheat_asm_file(inc)
    check("include_asm: stripped from the sandbox source", "INCLUDE_ASM" not in out)
    check("include_asm: counted as a stripped cheat construct", n >= 1)

    # 3. Counted > 0 despite there being no C body to attribute it to.
    eq("include_asm: attributed to the named function",
       inlineasm.func_cheat_asm_count(inc, "PClseek"), 1)
    # The hand-expanded `.include` spelling is the same fact.
    exp = ('__asm__(\n    ".section .text\\n"\n'
           '    "    .include \\"asm/funcs/PClseek.s\\"\\n"\n);\n')
    check("include_asm: hand-expanded .include attributed too",
          "PClseek" in inlineasm.whole_body_asm_funcs(exp))
    # Genuinely unexplained symbols still report UNKNOWN rather than a fake 1.
    eq("include_asm: unrelated symbol still UNKNOWN",
       inlineasm.func_cheat_asm_count(inc, "some_other_func"), -1)
    # Attribution is spelling-INDEPENDENT: a `glabel` whole-body block is the
    # same fact and must attribute too, or the defect just moves one spelling
    # over. (It is still never STRIPPED — canonical_body behaviour is unchanged.)
    gl = ('__asm__(\n    ".section .text\\n"\n    "glabel PCclose\\n"\n'
          '    "    jr $ra\\n"\n    "    nop\\n"\n    "endlabel PCclose\\n"\n);\n')
    eq("include_asm: glabel whole-body block attributed",
       inlineasm.func_cheat_asm_count(gl, "PCclose"), 1)
    check("include_asm: glabel whole-body block still NOT stripped",
          "glabel PCclose" in inlineasm.strip_cheat_asm_file(gl)[0])
    # ...but a glabel block with NO instructions is a bare SYMBOL marker, not a
    # body (system.c emits one so `&D_80081F1C` resolves). Not decomp work.
    marker = '__asm__(\n    ".set noreorder\\n"\n    "glabel D_80081F1C\\n"\n);\n'
    eq("include_asm: instruction-less glabel marker NOT attributed",
       inlineasm.func_cheat_asm_count(marker, "D_80081F1C"), -1)
    # A body encoded entirely as byte-emitting `.word`s IS a body, not a marker
    # (ings.c ships func_800164F8 that way: 0x03E00008 is `jr ra`).
    words = ('__asm__(\n    ".set noreorder\\n"\n    "glabel func_800164F8\\n"\n'
             '    ".word 0x2402270F\\n"\n    ".word 0x03E00008\\n"\n);\n')
    eq("include_asm: .word-encoded body attributed as a body",
       inlineasm.func_cheat_asm_count(words, "func_800164F8"), 1)
    # A block carrying BOTH a glabel and an asm/funcs `.include` is a body, and
    # must not also be reported as a marker — the two sets must stay disjoint,
    # or _not_a_c_function could call an attributed function "not a function".
    hybrid = ('__asm__(\n    "glabel foo\\n"\n'
              '    "    .include \\"asm/funcs/foo.s\\"\\n"\n);\n')
    check("include_asm: glabel+.include block is a body",
          "foo" in inlineasm.whole_body_asm_funcs(hybrid))
    check("include_asm: glabel+.include block is NOT a marker",
          "foo" not in inlineasm.symbol_marker_funcs(hybrid))
    # Unknown directives fail SAFE toward "this is a body", so a future
    # byte-emitting directive can never make a real function droppable.
    unknown = '__asm__(\n    "glabel bar\\n"\n    "    .quad 0x1\\n"\n);\n'
    check("include_asm: unrecognised directive counts as a body",
          "bar" not in inlineasm.symbol_marker_funcs(unknown))
    for stem in ("main/6CF8", "main/psxsdk/libetc/intr", "main/psxsdk/libcd/bios", "main/psxsdk/libspu/spu", "main/368E4",
                 "main/psxsdk/libgte/msc00", "main/psxsdk/libgte/patchgte"):
        t = Path(f"src/{stem}.c").read_text(encoding="utf-8")
        eq(f"include_asm: body/marker sets disjoint in {stem}.c",
           sorted(inlineasm.whole_body_asm_funcs(t)
                  & inlineasm.symbol_marker_funcs(t)), [])

    # 3b. Body-span shapes that used to be misparsed as "no C body", which left
    #     a real function's cheat count UNKNOWN and its cheats unpoliced.
    brace_prefixed = ('void prev(void) {\n    int a;\n'
                      '}s32 _version(s32 arg0) {\n'
                      '    __asm__ volatile("addu $8,$3,$0");\n    return arg0;\n}\n')
    eq("body-span: definition sharing a line with the previous `}`",
       inlineasm.func_cheat_asm_count(brace_prefixed, "_version"), 1)
    knr = ('void calc_LightMatrix(arg0) s16 *arg0; {\n'
           '    __asm__ volatile("addu $8,$3,$0");\n}\n')
    eq("body-span: K&R parameter declarations between `)` and `{`",
       inlineasm.func_cheat_asm_count(knr, "calc_LightMatrix"), 1)
    # ...without letting an indented CALL be mistaken for a definition.
    call_only = ('extern int foo(int);\nvoid bar(void) {\n    foo(1);\n}\n')
    eq("body-span: an indented call is still not a definition",
       inlineasm.func_cheat_asm_count(call_only, "foo"), -1)
    # engine/volatile_cheats.py used to carry a verbatim COPY of the body span,
    # which went stale the moment inlineasm's learned these shapes — leaving the
    # same two functions policed by one detector and invisible to the other.
    # One implementation, so the two can never diverge again.
    check("body-span: volatile_cheats delegates to the single implementation",
          volatile_cheats._func_body_span(brace_prefixed, "_version")
          == inlineasm._func_body_span(brace_prefixed, "_version")
          is not None)
    check("body-span: volatile_cheats sees the K&R shape too",
          volatile_cheats._func_body_span(knr, "calc_LightMatrix") is not None)

    # 4. generate(): a negative (UNKNOWN) count must NOT drop an item, and a
    #    measured-zero count still must.
    def _regen_with(cheat_count: int, canon: set, src_text: str | None = None,
                    asm_text: str | None = None) -> list:
        with tempfile.TemporaryDirectory() as td:
            cwd = os.getcwd()
            os.chdir(td)
            Path("build/src").mkdir(parents=True)
            Path("build/src/faketu.o").write_text("")
            if src_text is not None:
                Path("src").mkdir()
                Path("src/faketu.c").write_text(src_text)
            if asm_text is not None:
                Path("asm/funcs").mkdir(parents=True)
                Path("asm/funcs/other.s").write_text(asm_text)
            orig = (Q.QUEUE_PATH, P.c_stems, canonical.scan_all,
                    cheats.canonical_asm_funcs, Q._rule_count,
                    cheats.func_prologue_count, inlineasm.file_func_cheat_asm_count,
                    completion.source_issues,
                    cheats.is_jtbl_infra, cheats.is_canonical_extraction_only,
                    Q.sandbox.build_stripped_object, score._o_func_table,
                    score.score_func)
            Q.QUEUE_PATH = str(Path(td) / "queue.json")
            Path(Q.QUEUE_PATH).write_text(json.dumps({"items": [], "counts": {}}))
            P.c_stems = lambda: ["faketu"]
            canonical.scan_all = lambda: []
            cheats.canonical_asm_funcs = lambda: canon
            Q._rule_count = lambda f: 0
            cheats.func_prologue_count = lambda f: 0
            inlineasm.file_func_cheat_asm_count = lambda s, f: cheat_count
            completion.source_issues = lambda *a, **k: []
            cheats.is_jtbl_infra = lambda f: False
            cheats.is_canonical_extraction_only = lambda f: False
            Q.sandbox.build_stripped_object = lambda *a, **k: {}
            score._o_func_table = lambda o: {"func_WB": (0, 0)}
            score.score_func = lambda a, b, f: {"score": 0}
            try:
                return [it["func"] for it in Q.generate(workdir=td)["items"]]
            finally:
                os.chdir(cwd)
                (Q.QUEUE_PATH, P.c_stems, canonical.scan_all,
                 cheats.canonical_asm_funcs, Q._rule_count,
                 cheats.func_prologue_count, inlineasm.file_func_cheat_asm_count,
                 completion.source_issues,
                 cheats.is_jtbl_infra, cheats.is_canonical_extraction_only,
                 Q.sandbox.build_stripped_object, score._o_func_table,
                 score.score_func) = orig

    # An asm-supplied function reaches generate() with an ATTRIBUTED count of 1,
    # never -1, so this is the case that keeps ang_hosei in the queue.
    eq("include_asm: regen KEEPS an item with attributed cheat-asm",
       _regen_with(1, set()), ["func_WB"])
    eq("include_asm: regen still drops a measured-clean COMPLETED-C item",
       _regen_with(0, set()), [])
    eq("include_asm: regen still drops a canonical-authorized item",
       _regen_with(1, {"func_WB"}), [])
    # THE INVARIANT: unknown never reads as clean. A -1 count is retained, so a
    # function whose body-span parse failed can never silently leave the queue.
    eq("include_asm: regen KEEPS an item whose cheat count is UNKNOWN",
       _regen_with(-1, set()), ["func_WB"])
    # The ONE safe exception: the symbol is not a C-level function at all. Absence
    # from the .c is not that evidence by itself: the included asm must declare it
    # as an `alabel` / `.aent` (a bare second `glabel` is a function of its own).
    eq("include_asm: regen KEEPS an UNKNOWN symbol absent from the .c (no asm evidence)",
       _regen_with(-1, set(), src_text="void other(void) {}\n"), ["func_WB"])
    included = 'INCLUDE_ASM("asm/funcs", other);\n'
    eq("include_asm: regen KEEPS a bare second glabel in included asm",
       _regen_with(-1, set(), src_text=included,
                   asm_text="glabel other\n  jr $ra\n  nop\n"
                            "glabel func_WB\n  jr $ra\n  nop\n"), ["func_WB"])
    eq("include_asm: regen KEEPS an absent symbol whose included asm is missing",
       _regen_with(-1, set(), src_text=included), ["func_WB"])
    for entry in ("alabel", ".aent"):
        eq(f"include_asm: regen drops an absent {entry} of included asm",
           _regen_with(-1, set(), src_text=included,
                       asm_text=f"glabel other\n  jr $ra\n  nop\n{entry} func_WB\n"), [])
    eq("include_asm: regen drops an UNKNOWN `.aent` alternate entry",
       _regen_with(-1, set(),
                   src_text='__asm__("    .aent func_WB\\n");\n'), [])
    eq("include_asm: regen drops an instruction-less glabel symbol marker",
       _regen_with(-1, set(),
                   src_text='__asm__(\n    ".set noreorder\\n"\n'
                            '    "glabel func_WB\\n"\n);\n'), [])

    # 5. mark_done(): refuse a non-canonical function on attributed cheat-asm
    #    AND on UNKNOWN; still ACCEPT a canonical one (the campaign's payoff
    #    path — the 66 authorized functions must reach zero-rule canonical
    #    completion once their asmfix wiring is retired).
    def _done_with(cheat_count: int, canon: set) -> dict:
        with tempfile.TemporaryDirectory() as td, _stub_tus(td, {
                "faketu": ('INCLUDE_ASM("asm/funcs", func_WB);\n' if canon
                           else "void other(void) {}\n")}):
            qp = Path(td) / "queue.json"
            qp.write_text(json.dumps(
                {"items": [{"func": "func_WB", "file": "faketu", "distance": 0,
                            "verdict": "C", "rules": 0, "status": "active"}],
                 "counts": {}}))
            orig = (Q.QUEUE_PATH, Q._rule_count, cheats.func_prologue_count,
                    cheats.canonical_asm_funcs, cheats.maspsx_gate_entries,
                    inlineasm.file_func_cheat_asm_count, completion.source_issues,
                    Q.O.verify, Q.layer2.gate)
            Q.QUEUE_PATH = str(qp)
            Q.layer2.gate = lambda f, s: None  # the Q39 gate has its own test
            Q._rule_count = lambda f: 0
            cheats.func_prologue_count = lambda f: 0
            cheats.canonical_asm_funcs = lambda: canon
            cheats.maspsx_gate_entries = lambda f: []
            inlineasm.file_func_cheat_asm_count = lambda s, f: cheat_count
            completion.source_issues = lambda *a, **k: []
            Q.O.verify = lambda rebuild=False: {"build_matches": True,
                                                "build_sha1": "deadbeef"}
            try:
                return Q.mark_done("func_WB")
            finally:
                (Q.QUEUE_PATH, Q._rule_count, cheats.func_prologue_count,
                 cheats.canonical_asm_funcs, cheats.maspsx_gate_entries,
                 inlineasm.file_func_cheat_asm_count, completion.source_issues,
                 Q.O.verify, Q.layer2.gate) = orig

    r_attr = _done_with(1, set())
    check("include_asm: queue done REFUSES attributed cheat-asm",
          r_attr.get("ok") is False
          and "cheat construct(s)" in r_attr.get("reason", "")
          and "missing; cannot check" not in r_attr.get("reason", ""))
    r_unk = _done_with(-1, set())
    check("include_asm: queue done REFUSES an UNKNOWN cheat count",
          r_unk.get("ok") is False
          and "UNKNOWN" in r_unk.get("reason", "")
          and "missing; cannot check" not in r_unk.get("reason", ""))
    check("include_asm: UNKNOWN refusal names the missing C body",
          "UNKNOWN" in r_unk.get("reason", ""))
    r_canon = _done_with(1, {"func_WB"})
    check("include_asm: canonical-authorized function still completes",
          r_canon.get("ok") is True)
    eq("include_asm: canonical completion state",
       r_canon.get("completion"), "COMPLETED-INLINE-ASM-CANONICAL")


def test_completion_region_grants() -> None:
    """Mixed C/asm completion authorizes exact reviewed islands, not a whole C body."""
    text = '''\
s32 func_MIX(s32 value) {
    __asm__ volatile ("ctc2 %0,$13" :: "r"(value));
    return value + 1;
}
'''
    with tempfile.TemporaryDirectory() as td:
        grants = Path(td) / "regions.json"
        old_read, old_regions = inlineasm._read_src_cached, completion.REGIONS
        try:
            inlineasm._read_src_cached = lambda stem: text
            completion.REGIONS = grants
            issues = completion.source_issues("fake", "func_MIX", canonical=False)
            check("region grants: an unclassified inline-asm island is rejected",
                  any("requires canonical" in i for i in issues))

            entry = {"file": "fake", "sha256": completion.region_hashes(text, "func_MIX")}
            grants.write_text(json.dumps({"schema": 2,
                                          "functions": {"func_MIX": entry}}))
            eq("region grants: exact reviewed island is accepted",
               completion.source_issues("fake", "func_MIX", canonical=True), [])

            relaid = text.replace('__asm__ volatile ("ctc2 %0,$13" :: "r"(value));',
                                  '__asm__ volatile("ctc2 %0,$13"\n'
                                  '                     /* re-laid out */ :: "r"(value));')
            assert relaid != text
            inlineasm._read_src_cached = lambda stem: relaid
            eq("region grants: a layout / comment-only change keeps the grant",
               completion.source_issues("fake", "func_MIX", canonical=True), [])

            respaced = text.replace('"ctc2 %0,$13"', '"ctc2 %0, $13"')
            inlineasm._read_src_cached = lambda stem: respaced
            issues = completion.source_issues("fake", "func_MIX", canonical=True)
            check("region grants: whitespace inside the template string still invalidates",
                  any("changed since review" in i for i in issues))

            changed = text.replace("$13", "$14")
            inlineasm._read_src_cached = lambda stem: changed
            issues = completion.source_issues("fake", "func_MIX", canonical=True)
            check("region grants: an operand change invalidates completion",
                  any("changed since review" in i for i in issues))

            # grant_issues: the whole registry, queue state aside
            old_canon = cheats.canonical_asm_funcs
            try:
                cheats.canonical_asm_funcs = lambda: {"func_MIX"}
                inlineasm._read_src_cached = lambda stem: text
                eq("grant audit: a matching grant is clean",
                   completion.grant_issues({"fake"}), [])
                inlineasm._read_src_cached = lambda stem: relaid
                eq("grant audit: a layout-only change stays clean",
                   completion.grant_issues({"fake"}), [])
                renamed = text.replace("value", "var_0")
                inlineasm._read_src_cached = lambda stem: renamed
                issues = completion.grant_issues({"fake"})
                check("grant audit: an operand rename makes the grant stale",
                      len(issues) == 1 and "stale grant" in issues[0])
                inlineasm._read_src_cached = lambda stem: "s32 other(void) { return 0; }\n"
                check("grant audit: a vanished body makes the grant stale",
                      any("stale grant" in i for i in completion.grant_issues({"fake"})))
                inlineasm._read_src_cached = lambda stem: None
                check("grant audit: an unreadable source makes the grant stale",
                      any("stale grant" in i for i in completion.grant_issues({"fake"})))
                eq("grant audit: a grant file that is not a C TU, and nothing more",
                   completion.grant_issues({"other"}),
                   [f"{grants}: func_MIX: grant file 'fake' is not a C TU"])
                inlineasm._read_src_cached = lambda stem: text
                cheats.canonical_asm_funcs = lambda: set()
                check("grant audit: a grant for a non-canonical function",
                      any("not listed in inline_asm_canonical" in i
                          for i in completion.grant_issues({"fake"})))
                cheats.canonical_asm_funcs = lambda: {"func_MIX"}
                for bad in ({"file": "fake"}, {"file": "fake", "sha256": []},
                            {"file": "fake", "sha256": "x"},
                            dict(entry, extra=1), dict(entry, file=["fake"]),
                            dict(entry, file=None), "fake"):
                    grants.write_text(json.dumps({"schema": 2, "functions": {"func_MIX": bad}}))
                    check(f"grant audit: malformed grant {bad!r}",
                          any("malformed" in i for i in completion.grant_issues({"fake"})))
                for reg in ('{"schema": 1, "functions": {}}', '{"schema": 2}', "[]", "{"):
                    grants.write_text(reg)
                    check(f"grant audit: bad registry {reg!r}",
                          len(completion.grant_issues({"fake"})) == 1)
                grants.unlink()
                check("grant audit: a missing registry",
                      any("unreadable" in i for i in completion.grant_issues({"fake"})))
            finally:
                cheats.canonical_asm_funcs = old_canon
        finally:
            inlineasm._read_src_cached, completion.REGIONS = old_read, old_regions

    # the integrity wiring: check_completion_integrity.main() extends its
    # violations with the grant audit before it decides the exit code
    import ast
    repo_root = Path(__file__).resolve().parent.parent
    src = (repo_root / "tools/check_completion_integrity.py").read_text(encoding="utf-8")
    main_fn = next(n for n in ast.parse(src).body
                   if isinstance(n, ast.FunctionDef) and n.name == "main")
    wired = verdict_at = None
    for i, st in enumerate(main_fn.body):
        seg = ast.get_source_segment(src, st) or ""
        if seg.strip() == "violations.extend(completion.grant_issues(set(P.c_stems())))":
            wired = i
        if isinstance(st, ast.If) and (ast.get_source_segment(src, st.test) or "") == "violations":
            verdict_at = i if verdict_at is None else verdict_at
    check("grant audit: check_completion_integrity.main() runs it before its verdict",
          wired is not None and verdict_at is not None and wired < verdict_at)


def test_q126_inline_helpers() -> None:
    """Owner ruling Q126: a same-TU `static inline` helper's islands, calls and
    tokens are attributed to each caller (region hashes, layer-2 key, cheat
    audits, integrity); helper-free bodies key exactly as before."""
    from engine import gtemacro as G
    from engine import layer2 as L2
    for head, name, words in (
            ("void (*getp(int a))(void) ", "getp", {"void"}),
            ("s32 (*f(void))[3]", "f", {"s32"}),
            ("u8 (*getp(int a))(void)", "getp", {"u8"}),
            ("struct S (*f(void))", "f", {"struct", "S"}),
            ("static inline MyFn (*getp(void))", "getp", {"static", "inline", "MyFn"}),
            ("static inline u8 (*getp(int a))(void)", "getp", {"static", "inline", "u8"}),
            ("int (f(int x))", "f", {"int"}),
            ("void f(T(int))", "f", {"void"}),
            ("inline void f(T(int))", "f", {"inline", "void"}),
            ("void f(u8 (x))", "f", {"void"}),
            ("static void f(UNUSED(x))", "f", {"static", "void"}),
            ('void f(M(a)) asm("x")', "f", {"void"}),
            ("static inline s32 f(DECL(int) x)", "f", {"static", "inline", "s32"}),
            ("static inline int f(g(h))", "f", {"static", "inline", "int"}),
            ("static inline s32 f(s32 (x), s32 y)", "f", {"static", "inline", "s32"}),
            ("int f(a, cb) char *a; int (*cb)(int); ", "f", {"int"}),
            ("static T (*f(void))", "f", {"static", "T"}),
            ("static const T (*f(void))", "f", {"static", "const", "T"}),
            ("static inline s32 *f(void)", "f", {"static", "inline", "s32"}),
            ("static inline int helper(int (*p)[1 == 1])",
             "helper", {"static", "inline", "int"}),
            ("s32 *(*f(void))[3]", "f", {"s32"}),
            ("struct S (f(void))", "f", {"struct", "S"}),
            ("int (* const h(void))(void)", "h", {"int", "const"}),
            ("void (^b(void))(void)", "b", {"void"}),
            ("u8 (*(*f(int))(int))(void)", "f", {"u8"}),
            ("void (*getp(int (*cb)(void)))(void)", "getp", {"void"}),
            ("static inline s32 *(*f(void))[3]", "f", {"static", "inline", "s32"}),
            ("static inline s32 sq(s32 x)", "sq", {"static", "inline", "s32"}),
            ("static inline s32 __attribute__((unused)) sq(s32 x)",
             "sq", {"static", "inline", "s32"}),
            ("void f(void)", "f", {"void"}),
            ("int f(a)", "f", {"int"}),
            ("int f(cb) int (*cb)();", "f", {"int"}),
            ('void f(void) asm("y")', "f", {"void"}),
            ('void f(void) __asm__("y")', "f", {"void"})):
        eq(f"q126: declarator head {head!r}", inlineasm._head_function(head),
           (name, frozenset(words)))
    for head in ("void *p", "void (*p)[3]", "sizeof(int)", "typeof(int)",
                 "__typeof__(int)", "int a = getp(1)",
                 "u8 (getp)(int a)", "int (f)(int x)", "T (h)(int)",
                 "int (*h)(int)", "int (h)(int)(void)",
                 "static inline int (h)(int x)",
                 "void ((f))(void)", "int (*(h)(void))(void)",
                 "__typeof__(g(1)) f(void)", "typeof(g(1)) f(void)",
                 "typeof(int) f(void)", "DECL(int) f(void)",
                 "FOO(x) void f(void)", "_Pragma(   ) void f(void)",
                 "f(void)", "static inline u8 (sq)(u8 x)", "static u8 (f)(int x)",
                 "extern u8 (g)(int)", "static u8 (const f(void))",
                 "u8 (f(int x))", "MyFn (getp(void))", "__inline__ static u8 (f(void))",
                 "extern T (f(int x))", "static X(g(int a))", "f (T(int))",
                 "static inline u8 (sq(u8 x))",
                 "static inline s32 f(s32 (x))", "static inline s32 f(MATRIX(m))",
                 "static inline T f(U(x))",
                 "int ATTR(x) f(a) int a; ", "T ATTR(x) f(a) int a; ",
                 "T *ATTR(x) f(a) int a; ", "int (*ATTR(x))(void) f(a) int a; ",
                 "int f(a) g(b) int b; ",
                 "int " + "(" * 5000 + "f(void)" + ")" * 5000):
        eq(f"q126: head without a function name {head!r}",
           inlineasm._head_function(head), None)
    fnptr_tu = ("static inline void (*getp(int a))(void) { return 0; }\n"
                "void caller(void) { getp(1)(); }\n")
    eq("q126: function-pointer-returning helper has its real name",
       list(inlineasm.static_inline_defs(fnptr_tu)), ["getp"])
    eq("q126: function-pointer-returning helper is attributed to its caller",
       inlineasm.inline_helpers(fnptr_tu, "caller"), ["getp"])
    eq("q126: function-pointer-returning head leaves a balanced TU",
       inlineasm.tu_unbalanced(fnptr_tu), False)
    typedef_fnptr_tu = fnptr_tu.replace("inline void", "inline u8")
    eq("q126: typedef-returning helper has its real name",
       list(inlineasm.static_inline_defs(typedef_fnptr_tu)), ["getp"])
    eq("q126: typedef-returning helper is attributed to its caller",
       inlineasm.inline_helpers(typedef_fnptr_tu, "caller"), ["getp"])
    eq("q126: typedef-returning head leaves a balanced TU",
       inlineasm.tu_unbalanced(typedef_fnptr_tu), False)
    grouped_tu = ("static inline int (sq(int x)) { return x; }\n"
                  "void caller(void) { sq(1); }\n")
    eq("q126: grouped helper has its real name",
       list(inlineasm.static_inline_defs(grouped_tu)), ["sq"])
    eq("q126: grouped helper is attributed to its caller",
       inlineasm.inline_helpers(grouped_tu, "caller"), ["sq"])
    eq("q126: grouped helper leaves a balanced TU",
       inlineasm.tu_unbalanced(grouped_tu), False)
    unused_tu = ("static inline void h(UNUSED(x)) { }\n"
                 "void caller(void) { h(1); }\n")
    eq("q126: a macro-wrapped parameter keeps the helper's name",
       list(inlineasm.static_inline_defs(unused_tu)), ["h"])
    eq("q126: ... and its attribution",
       inlineasm.inline_helpers(unused_tu, "caller"), ["h"])
    typedef_grouped_tu = ("static inline u8 (sq(u8 x)) { return x; }\n"
                          "void caller(void) { sq(1); }\n")
    eq("q126: an ambiguous typedef-grouped helper head fails closed",
       inlineasm.tu_unbalanced(typedef_grouped_tu), True)
    macro_type_tu = ("static inline DECL(int) sq(int x) { return x; }\n"
                     "void caller(void) { sq(1); }\n")
    eq("q126: uncertain macro type fails closed",
       inlineasm.tu_unbalanced(macro_type_tu), True)
    kr_tu = "int f(cb) int (*cb)(); { return cb(); }\n"
    eq("q126: K&R declaration segments retain the function name",
       [fn.name for fn in inlineasm.tu_functions(kr_tu)], ["f"])
    eq("q126: K&R declaration segments leave a balanced TU",
       inlineasm.tu_unbalanced(kr_tu), False)
    if not all(Path(r).exists() for r in G.VENDORED):
        skip("q126 inline helpers", "include/psxsdk vendored headers absent")
        return
    inc = "#include <psxsdk/inline_o.h>\n#include <psxsdk/gtemac.h>\n"
    sq = ("static inline s32 sq(s32 x) {\n"
          "    s32 lz[4]; /* FAKE: frame layout */\n"
          "    gte_Lzc(x, &lz[0]);\n"
          "    return lz[0];\n"
          "}\n")
    rest = ("static inline s32 outer(s32 x) { return sq(x) * 2; }\n"
            "static inline s32 ping(s32 x);\n"
            "static inline s32 pong(s32 x) { return ping(x); }\n"
            "static inline s32 ping(s32 x) { return x ? pong(x - 1) : 0; }\n"
            "static s32 plain(s32 x) {\n    s32 lz;\n    gte_Lzc(x, &lz);\n    return lz;\n}\n"
            "s32 func_A(s32 v) {\n    return sq(v) + 1;\n}\n"
            "s32 func_B(s32 v) {\n    return outer(v);\n}\n"
            "s32 func_C(s32 v) {\n    return ping(v);\n}\n"
            "s32 func_D(s32 v) {\n    /* sq(v) in a comment */\n    return plain(v);\n}\n"
            "s32 func_E(s32 v) {\n    return v + 2;\n}\n")
    text = inc + sq + rest
    split_head = ("#ifdef A\nint f(int a) {\n#else\nint f(int a, int b) {\n"
                  "#endif\n    return a;\n}\n"
                  "static inline int h(int x) { return x; }\n")
    normal_tu = "int f(int a) { return a; }\n"
    lexical_cases = [
        ("stray quote pair", '#if 0\n"\n#endif\n'
         'static inline int helper(int x) { return x; }\n'
         '#if 0\n"\n#endif\nint f(int a) { return helper(a); }\n'),
        ("dollar macro", "#define $H static inline int helper(int x) { return x; }\n$H\n"
         + normal_tu),
        ("form feed before directive", "\f#define H int x\n" + normal_tu),
        ("form feed inside directive", "#\fdefine H int x\n" + normal_tu),
        ("vertical tab", "#\vdefine H int x\n" + normal_tu),
        ("unparsed define", "#define\n" + normal_tu),
        ("unparsed conditional-looking directive", "#iffoo A\n" + normal_tu)]
    for desc, src in lexical_cases:
        eq(f"q126: {desc} fails closed", inlineasm.tu_unbalanced(src), True)
    evasion_cases = [
        (f"alternate token {token}", normal_tu + token + "\n")
        for token in ("%:", "<%", "%>", "??=", "??<", "??>")]
    evasion_cases += [
        ("file-scope token pasting",
         "#define CAT(a,b) a##b\n"
         "#define HX static inline int k(int x) { return x; }\nCAT(H,X);\n"
         + normal_tu),
        ("function-like head macro",
         "#define f(x) static inline int helper(int x)\n"
         'f(y) { __asm__("nop"); return y; }\n#undef f\n'
         "int f(int a) { return helper(a); }\n"),
        ("head word macro", "#define INL inline\n"
         "static INL int helper(int x) { return x; }\n" + normal_tu),
        ("duplicate ordinary definition", normal_tu + "#if 0\n" + normal_tu + "#endif\n")]
    for desc, src in evasion_cases:
        eq(f"q126: {desc} fails closed", inlineasm.tu_unbalanced(src), True)
    eq("q126: alternate tokens in comments/literals are masked",
       inlineasm.tu_unbalanced('/* %: <% %> ??= ??< ??> */\n'
                              'char *s = "%: <% %> ??= ??< ??>";\n' + normal_tu), False)
    macro_head = ('#define HEAD static inline int helper(int x)\n'
                  'HEAD { __asm__("nop"); return x; }\n'
                  'int f(int a) { return helper(a); }\n')
    eq("q126: a macro-only function head fails closed",
       inlineasm.tu_unbalanced(macro_head), True)
    comparison_heads = []
    for op in ("==", "<=", ">=", "!="):
        src = (f'static inline int helper(int (*p)[1 {op} 1]) {{ '
               '__asm__("nop"); return 0; }\n'
               'int f(int a) { return helper(0); }\n')
        comparison_heads.append(src)
        eq(f"q126: {op} in parameters does not hide the helper",
           inlineasm.inline_helpers(src, "f"), ["helper"])
        eq(f"q126: {op} in parameters leaves a recognized balanced head",
           inlineasm.tu_unbalanced(src), False)
    nonfunction_heads = ("int a[] = {1, 2};\n", "struct S { int x; };\n",
                         "typedef struct { int x; } T;\n", "enum E { A, B };\n")
    for src in nonfunction_heads:
        eq(f"q126: clear nonfunction head is allowed: {src.strip()}",
           inlineasm.tu_unbalanced(src), False)
    dead_tu = "#if 0\n" + normal_tu + "#endif\n"
    compensated = split_head + "#if 0\n}\n#endif\n"
    complete_branches = ("#ifdef A\n" + normal_tu + "#else\n"
                         + normal_tu.replace("int f(", "int g(") + "#endif\n")
    brace_macro = "#define OPEN {\n" + normal_tu
    close_macro = "#define CLOSE }\n" + normal_tu
    balanced_macro = "#define BLK(x) { x; }\n" + normal_tu
    reversed_macro = "#define REVERSE } {\n" + normal_tu
    reopened_branch = ('static inline int h(int x) {\n#if 0\n}\n{\n#endif\n'
                       '    __asm__("nop"); return x;\n}\n'
                       'int f(int a) { return h(a); }\n')
    replaced_branch = reopened_branch.replace("#if 0\n}\n{", "#ifdef A\n} int g(void) {")
    hidden_macro = ('#define H static inline int h(int x) { '
                    '__asm__("nop"); return x; }\nH\n' + normal_tu)
    body_macro = "#define BLK(x) { x; }\nint f(int a) { BLK(a); return a; }\n"
    hidden_alias = hidden_macro.replace("\nH\n", "\n#define G H\nG\n")
    body_alias = body_macro.replace("\nint f", "\n#define G BLK\nint f").replace(
        "{ BLK(a);", "{ G(a);")
    eq("q126: a file-scope alias reaches the brace macro",
       inlineasm.tu_unbalanced(hidden_alias), True)
    eq("q126: an alias to a balanced brace macro inside a body is allowed",
       inlineasm.tu_unbalanced(body_alias), False)
    for desc, src in (("branch closes before reopening", reopened_branch),
                      ("branch replaces a function head", replaced_branch),
                      ("brace macro defines a file-scope helper", hidden_macro)):
        eq(f"q126: {desc} is an unsafe TU walk", inlineasm.tu_unbalanced(src), True)
    eq("q126: self-balanced brace macro use inside a body stays allowed",
       inlineasm.tu_unbalanced(body_macro), False)
    eq("q126: compensating EOF depth does not hide branch imbalance",
       inlineasm.tu_unbalanced(compensated), True)
    eq("q126: complete definitions in each conditional branch balance",
       inlineasm.tu_unbalanced(complete_branches), False)
    eq("q126: a brace macro makes the TU walk unreliable",
       inlineasm.tu_unbalanced(brace_macro), True)
    eq("q126: a closing brace macro makes the TU walk unreliable",
       inlineasm.tu_unbalanced(close_macro), True)
    eq("q126: a self-balanced brace macro does not shift the TU walk",
       inlineasm.tu_unbalanced(balanced_macro), False)
    eq("q126: zero net macro delta still refuses a negative running depth",
       inlineasm.tu_unbalanced(reversed_macro), True)
    eq("q126: alternative function heads leave the TU brace walk unbalanced",
       inlineasm.tu_unbalanced(split_head), True)
    eq("q126: a stray closing brace marks the TU unbalanced",
       inlineasm.tu_unbalanced("}" + normal_tu), True)
    eq("q126: a normal TU has balanced braces",
       inlineasm.tu_unbalanced(normal_tu), False)
    eq("q126: a complete function inside #if 0 has balanced braces",
       inlineasm.tu_unbalanced(dead_tu), False)
    header_facts = (inlineasm.tu_macro_facts(
        "#define OPEN {\n#define CLOSE }\n#define BLK { x(); }\n"
        "#define PASTE(a,b) a ## b\n#define TYPE int\n")
        | inlineasm.tu_macro_facts("#define H_ALIAS OPEN\n#define BACK TU_ALIAS\n"))
    header_cases = (
        ("header brace at file scope", "OPEN\nvoid f(void) {}", True),
        ("unbalanced header brace in body", "void f(void) { OPEN }", True),
        ("balanced header brace in body", "void f(void) { BLK }", False),
        ("header brace alias in body", "void f(void) { H_ALIAS }", True),
        ("TU brace alias in body",
         "#define LOCAL OPEN\nvoid f(void) { LOCAL }", True),
        ("header macro splits function bodies",
         "void f(void) { CLOSE static inline int h(int x) { x = 0; return 1; } "
         "static int d[] = OPEN 0 };\nint k(void) { return h(2); }", True),
        ("header alias at file scope", "H_ALIAS\nvoid f(void) {}", True),
        ("TU alias to header brace", "#define LOCAL H_ALIAS\nLOCAL\nvoid f(void) {}", True),
        ("header alias through TU", "#define TU_ALIAS OPEN\nBACK\nvoid f(void) {}", True),
        ("header paste at file scope", "PASTE(a,b)\nvoid f(void) {}", True),
        ("header paste rebuilds brace macros in body",
         "void f(void) { PASTE(CL,OSE) static inline int h(int x) "
         "{ x = 0; return 1; } static int d[] = PASTE(OP,EN) 0 };\n"
         "int k(void) { return h(2); }", True),
        ("header macro in head", "TYPE f(void) {}", True),
        ("header macro as function name", "void TYPE(void) {}", True),
        ("header macro in parameters", "int f(TYPE x) { return x; }", True),
        ("header macro in K&R tail", "int f(x) TYPE x; { return x; }", True),
        ("header macro in attributes",
         "int f(void) __attribute__((TYPE)) { return 0; }", True),
        ("unused header macros", "void f(void) {}", False))
    for desc, src, unsafe in header_cases:
        eq(f"q126: {desc}", inlineasm.tu_unbalanced(src, header_facts), unsafe)
        eq(f"q126: {desc} leaves default unchanged",
           inlineasm.tu_unbalanced(src), inlineasm.tu_unbalanced(src, frozenset()))
    eq("q126: absent header facts preserve unseen file-scope macro behavior",
       inlineasm.tu_unbalanced("OPEN\nvoid f(void) {}"), False)
    paren_helper = ("static inline int (h)(int x) { x = 0; return 1; }\n"
                    "int f(void) { return h(1); }")
    eq("q126: parenthesised helper fails closed for all body readers",
       inlineasm.tu_unbalanced(paren_helper), True)
    walk_cases = (
        ("TU macro rewrites parameter list",
         "#define P void); static inline int realh(int x\n"
         "static inline int fakeh(P) { return x; }", True),
        ("TU macro in K&R parameter declaration",
         "#define PARAM int\nint f(x) PARAM x; { return x; }", True),
        ("TU macro in attributes",
         "#define ATTR unused\nint f(void) __attribute__((ATTR)) { return 0; }", True),
        ("ordinary typedef parameters", "s32 f(s32 a, u8 *b) { return a; }", False),
        ("semicolon in macro arguments hides helper",
         "#define EAT(...)\nEAT(;=0) static inline int h(int x) { x = 0; return 1; }\n"
         "int f(void) { return h(2); }", True),
        ("directive in macro arguments hides helper",
         "#define EAT(...)\nEAT(\n#if 1\n#endif\n=0) "
         "static inline int h(int x) { x = 0; return 1; }\n"
         "int f(void) { return h(2); }", True),
        ("macro in initializer head", "#define EAT(...)\nEAT(0) int a[] = {1};", True),
        ("ordinary initializer", "int a[2] = {1, 2};", False),
        ("body-only initializer macro",
         "#define F(x) (x)\nstatic s32 t[] = { F(1), F(2) };", False),
        ("unclosed file-scope paren", "int f(", True),
        ("negative file-scope paren", ") void f(void) {}", True),
        ("paren open at brace", "int a[2] = ( {1, 2};", True),
        ("C include", '#include "part.c"\nvoid f(void) {}', True),
        ("C import", '#import "h.c"\nvoid f(void) {}', True),
        ("header import", '#import "h.h"\nvoid f(void) {}', False),
        ("parent header path", '#include "../outside.h"\nvoid f(void) {}', True),
        ("angle parent header path", '#include <dir/../x.h>\nvoid f(void) {}', True),
        ("comment after include", '#include "foo.h" // c\nvoid f(void) {}', False),
        ("spaced include", '#  include <x.h>\nvoid f(void) {}', False),
        ("macro include", '#include MACRO\nvoid f(void) {}', True),
        ("include_next", '#include_next "x.h"\nvoid f(void) {}', True),
        ("include inside body",
         'void f(void) {\n#include "close.h"\n'
         'static inline int h(int x) { x = 0; return 1; }\n'
         '#include "open.h"\n0 };\nint k(void) { return h(2); }', True),
        ("import inside body", 'void f(void) {\n#import "x.h"\n}', True),
        ("include inside parentheses", 'int f(\n#include "x.h"\nvoid) {}', True),
        ("absolute POSIX include", '#include "/x.h"\n', True),
        ("absolute drive include", '#include "C:/x.h"\n', True),
        ("absolute backslash include", '#include <\\x.h>\n', True),
        ("inc include", '#include "x.inc"\nvoid f(void) {}', True),
        ("angle non-header include", '#include <part.c>\nvoid f(void) {}', True),
        ("quoted header include", '#include "foo.h"\nvoid f(void) {}', False),
        ("angle header include", '#include <libgte.h>\nvoid f(void) {}', False))
    for desc, src, unsafe in walk_cases:
        eq(f"q126: {desc}", inlineasm.tu_unbalanced(src), unsafe)
    eq("q126: header macro in initializer head is refused",
       inlineasm.tu_unbalanced("TYPE a[] = {1};", header_facts), True)
    eq("q126: variadic macro parameters are excluded from aliases",
       inlineasm._tu_index("#define F(a, ...) a + __VA_ARGS__ + real\n").macro_aliases["F"],
       frozenset({"real"}))
    eq("q126: object-like parenthesised replacement keeps alias",
       inlineasm._tu_index("#define F (x)\n").macro_aliases["F"], frozenset({"x"}))
    unsafe_headers = (
        "%:define P void); static inline int realh(int x\n",
        "#define\fP void); static inline int realh(int x\n",
        "#define\vP 0\n", "#define $P 0\n", "??=define P 0\n",
        "#define\n", "}\n", "static int pad[] = {\n", "int f(\n",
        "static inline", "static inline int h(int x)")
    for header in unsafe_headers:
        eq(f"q126: unsafe header refused {header!r}", inlineasm.header_unsafe(header), True)
    eq("q126: unbalanced define alone is not an unsafe header",
       inlineasm.header_unsafe("#define OPEN {\n#define CLOSE }\n"), False)
    eq("q126: balanced define is a safe header",
       inlineasm.header_unsafe("#define BLK { x(); }\n"), False)
    linkage_header = ('#if defined(__cplusplus)\nextern "C" {\n#endif\n'
                      'int f(void);\n#if defined(__cplusplus)\n}\n#endif\n')
    eq("q126: balanced conditional SDK C++ linkage wrapper is safe",
       inlineasm.header_unsafe(linkage_header), False)
    eq("q126: unclosed SDK C++ linkage wrapper is refused",
       inlineasm.header_unsafe(linkage_header.replace("}\n", "")), True)
    hidden_linkage_helper = ('#ifdef __cplusplus\nextern "C" {\n#endif\n'
                             'static inline int h(int x) { return x + 1; }\n'
                             '#ifdef __cplusplus\n}\n#endif\n')
    eq("q126: helper body inside C++ linkage wrapper is refused",
       inlineasm.header_unsafe(hidden_linkage_helper), True)
    eq("q126: real libsn linkage wrapper remains safe",
       inlineasm.header_unsafe(Path("include/psxsdk/libsn.h").read_text(encoding="utf-8")),
       False)
    paste_facts = inlineasm.tu_macro_facts(
        "#define PASTE(a,b) a##b\n#define ALIAS PASTE\n#define STR(a) #a\n")
    eq("q126: paste macros and aliases are unsafe but stringizing stays balanced",
       {fact[0]: fact[3] for fact in paste_facts},
       {"PASTE": False, "ALIAS": False, "STR": True})
    eq("q126: header paste alias in body is refused",
       inlineasm.tu_unbalanced("void f(void) { ALIAS(CL,OSE) }", paste_facts), True)
    eq("q126: header facts carry per-macro balance",
       {fact[0]: fact[3] for fact in inlineasm.tu_macro_facts(
           "#define OPEN {\n#define ALIAS OPEN\n#define BLK { x(); }\n")},
       {"OPEN": False, "ALIAS": False, "BLK": True})
    wrap_facts = inlineasm.tu_macro_facts("#define WRAP(NAME) NAME\n#define REAL NAME\n")
    for seed in ("{", "{}"):
        wrapped = f"#define NAME {seed}\nWRAP(x);\nvoid f(void) {{}}"
        eq("q126: own macro parameter is not a brace alias",
           "WRAP" in inlineasm._tu_index(wrapped, wrap_facts).brace_macros, False)
        eq("q126: genuine header alias still propagates",
           "REAL" in inlineasm._tu_index(wrapped, wrap_facts).brace_macros, True)
    eq("q126: macro parameter does not refuse file-scope WRAP use",
       inlineasm.tu_unbalanced("#define NAME {}\nWRAP(x);\nvoid f(void) {}", wrap_facts), False)
    eq("q126: TU macro parameters are excluded from alias closure too",
       inlineasm.tu_unbalanced("#define NAME {}\n#define WRAP(NAME) NAME\n"
                               "WRAP(x);\nvoid f(void) {}"), False)
    eq("q126: genuine alias use remains refused",
       inlineasm.tu_unbalanced("#define NAME {}\nREAL\nvoid f(void) {}", wrap_facts), True)
    H = inlineasm.inline_helpers
    eq("q126: a called helper is attributed", H(text, "func_A"), ["sq"])
    eq("q126: helpers are reached transitively, in file order",
       H(text, "func_B"), ["sq", "outer"])
    eq("q126: a helper cycle terminates (and a prototype is not a definition)",
       H(text, "func_C"), ["pong", "ping"])
    eq("q126: a helper reaching itself back excludes the function", H(text, "ping"), ["pong"])
    eq("q126: a non-inline static is not attributed, nor a comment mention",
       H(text, "func_D"), [])
    eq("q126: a helper-free body has no helpers", H(text, "func_E"), [])
    spliced = text.replace("return sq(v) + 1;", "return s\\\nq(v) + 1;")
    eq("q126: a backslash-newline inside the helper name does not hide it",
       H(spliced, "func_A"), ["sq"])
    inl2 = text.replace("static inline s32 outer", "inline static s32 outer")
    eq("q126: `inline static` spelling is a helper too", H(inl2, "func_B"), ["sq", "outer"])

    # helpers reached through a macro (expanded transitively)
    mac = text.replace("s32 func_A(s32 v) {\n    return sq(v) + 1;\n}\n",
                       "#define CALLSQ(x) sq(x)\n#define SQ2 CALLSQ\n"
                       "s32 func_A(s32 v) {\n    return SQ2(v) + 1;\n}\n")
    eq("q126: a helper reached through #define bodies is attributed",
       H(mac, "func_A"), ["sq"])
    eq("q126: ... and its island is the caller's",
       completion.region_hashes(mac, "func_A"), completion.region_hashes(text, "sq"))
    alias = text.replace("s32 func_A(s32 v) {\n    return sq(v) + 1;\n}\n",
                         "#define SQ sq\ns32 func_A(s32 v) {\n    return SQ(v) + 1;\n}\n")
    eq("q126: an object-like alias macro reaches the helper", H(alias, "func_A"), ["sq"])
    # head layouts the line regex missed
    for desc, head in (("type and name on two lines", "static inline s32\nsq(s32 x) {"),
                       ("a comment inside the head", "static /*x*/ inline s32 sq(s32 x) {"),
                       ("an __attribute__ group",
                        "static inline s32 __attribute__((unused)) sq(s32 x) {"),
                       ("a K&R head", "static inline s32 sq(x) s32 x; {")):
        lay = text.replace("static inline s32 sq(s32 x) {", head)
        eq(f"q126: layout ({desc}) is a helper", H(lay, "func_A"), ["sq"])
        eq(f"q126: layout ({desc}): its island is the caller's",
           [lay[s:e] for s, e in completion.blocks(lay, "func_A")], ["gte_Lzc(x, &lz[0]);"])
    # false positives: member access and a shadowing local
    member = text.replace("return v + 2;", "return p->sq + q.sq + v;")
    eq("q126: `->sq` / `.sq` member names are not the helper", H(member, "func_E"), [])
    local = text.replace("return v + 2;", "int sq = v;\n    return sq + 2;")
    eq("q126: a local re-declaring the name (never called) is not the helper",
       H(local, "func_E"), [])
    called = text.replace("return v + 2;", "s32 sq = v;\n    return sq(v);")
    eq("q126: ... but a call of the name still is", H(called, "func_E"), ["sq"])
    helper = "static inline int h(int a) { return a + 1; }\n"
    for call in ("(h)(a)", "(h) (a)", "( h )(a)", "((h))(a)", "(*h)(a)",
                 "(**h)(a)", "(&h)(a)", "(0, h)(a)", "(c ? h : h)(a)"):
        paren_call = (helper + "int f(int a) {\n"
                      "    { int h = 2; a += h; }\n"
                      f"    return {call};\n}}\n")
        eq(f"q126: {call} reaches the helper despite an inner-block local",
           H(paren_call, "f"), ["h"])
    indirect = (helper + "int f(int a) {\n"
                "    int (*fp)(int);\n"
                "    { int h = 2; a += h; }\n"
                "    fp = h; return fp(a);\n}\n")
    eq("q126: an address taken outside an inner-block local reaches the helper",
       H(indirect, "f"), ["h"])
    parameter = helper + "int f(int h) { return h; }\n"
    eq("q126: a parameter shadows the helper for the whole body",
       H(parameter, "f"), [])
    local_only = helper + "int f(void) { int h = 2; return h; }\n"
    eq("q126: only using a same-name local does not reach the helper",
       H(local_only, "f"), [])
    for call in ("(*h)(a)", "((h))(a)"):
        brace_head = (helper + "}int f(int a) {\n"
                      "    { int h = 2; a += h; }\n"
                      f"    return {call};\n}}\n")
        eq(f"q126: brace-prefixed head preserves nested shadowing for {call}",
           H(brace_head, "f"), ["h"])
    brace_local = helper + "}int f(void) { int h = 2; return h; }\n"
    eq("q126: brace-prefixed head still recognizes an outermost local",
       H(brace_local, "f"), [])
    for initializer, result in (
            ("int (*fp)(int) = h;", "fp(a)"),
            ("int r = (*h)(a);", "r"),
            ("int (*t[1])(int) = {h};", "t[0](a)")):
        early_use = (helper + f"int f(int a) {{ {initializer} int h = 2; "
                     f"return {result} + h; }}\n")
        eq(f"q126: {initializer} reaches the helper before a later local",
           H(early_use, "f"), ["h"])
    for member_expr in ("s.h", "p->h"):
        member_only = helper + f"int f(void) {{ return {member_expr}; }}\n"
        eq(f"q126: {member_expr} is not a helper reference",
           H(member_only, "f"), [])
    for tag_decl in ("struct h;", "union h *p;"):
        tag_use = helper + f"int f(int a) {{ {tag_decl} return (*h)(a); }}\n"
        eq(f"q126: {tag_decl} declares a tag, not a shadowing local",
           H(tag_use, "f"), ["h"])
    for body in (
            "extern int other(int h); return (*h)(a) + other(a);",
            "int (*g)(int h); g = h; return g(a);",
            "int (*g)(int); g = (int (*)(int h)) h; return g(a);",
            "\n#if 0\n    int x; int h;\n#endif\n    return (*h)(a);\n"):
        hidden_use = helper + f"int f(int a) {{ {body} }}\n"
        eq(f"q126: prototype/cast names or directives retain the helper: {body}",
           H(hidden_use, "f"), ["h"])
    function_type = "typedef int FN(int);\n"
    for body in ("extern FN h; return (*h)(a);",
                 "int (*fp)(int); FN h; fp = h; return fp(a);"):
        redeclared = function_type + helper + f"int f(int a) {{ {body} }}\n"
        eq(f"q126: a block-scope function declaration retains the helper: {body}",
           H(redeclared, "f"), ["h"])
    for body in ("int h = 2; return h;", "int *h = 0; return h != 0;",
                 "struct S h; return 0;"):
        object_local = (function_type + "struct S { int x; };\n" + helper
                        + f"int f(void) {{ {body} }}\n")
        eq(f"q126: a certain object declaration shadows the helper: {body}",
           H(object_local, "f"), [])
    typedef_pointer = function_type + helper + (
        "int f(void) { FN *h = 0; return h != 0; }\n")
    eq("q126: an unknown typedef prefix keeps the helper even with a pointer",
       H(typedef_pointer, "f"), ["h"])
    for typedef, body in (
            ("", "typedef int s32(int); s32 h; return (*h)(a);"),
            ("", "typedef int u8(int); u8 h; int (*fp)(int) = h; return fp(a);"),
            ("typedef int u8(int);\n", "u8 h; return (*h)(a);")):
        typedef_use = typedef + helper + f"int f(int a) {{ {body} }}\n"
        eq(f"q126: a project typedef name may denote a function: {typedef}{body}",
           H(typedef_use, "f"), ["h"])
    for macros, body in (
            ("#define V (void)\n", "V *h; return (*h)(a);"),
            ("#define WHEN if (a)\n", "WHEN *h; return (*h)(a);"),
            ("#define Z 0,\n", "Z *h; return (*h)(a);"),
            ("#define SET fp =\n", "int (*fp)(int); SET *h; return fp(a);"),
            ("#define OPEN {\n#define CLOSE }\n",
             "OPEN int h = 2; a += h; CLOSE return (*h)(a);")):
        macro_use = macros + helper + f"int f(int a) {{ {body} }}\n"
        eq(f"q126: a same-TU macro disables shadow discards: {body}",
           H(macro_use, "f"), ["h"])
    for definition in (
            "int f(struct S { int h; } *p) { return (*h)(p->h); }",
            "int f(union U { int h; } *p) { int (*fp)(int) = h; return fp(1); }",
            "int f(int a, struct S { int x; int h; } *p) { return (&h)(a); }",
            "int f(a) int a; { int h = 2; return (*h)(a); }"):
        uncertain_head = helper + definition + "\n"
        eq(f"q126: an uncertain function head retains the helper: {definition}",
           H(uncertain_head, "f"), ["h"])
    addr = text.replace("return v + 2;", "return sq;")
    eq("q126: `return sq;` (the helper's address) is not a local declaration",
       H(addr, "func_E"), ["sq"])
    # an ambiguous helper (an #if 0 copy before the live one) fails closed
    amb = text.replace(sq, "#if 0\nstatic inline s32 sq(s32 x) { return x; }\n#endif\n" + sq)
    eq("q126: an ambiguous helper makes the cheat count unknown",
       inlineasm.func_cheat_asm_count(amb, "func_A"), -1)
    eq("q126: ... and the volatile count unknown",
       volatile_cheats.func_volatile_cheat_count(amb, "func_A"), -1)
    old_read2 = inlineasm._read_src_cached
    try:
        inlineasm._read_src_cached = lambda stem: amb
        check("q126: ... and source_issues refuses it", any(
            "definitions (ambiguous)" in i
            for i in completion.source_issues("fake", "func_A", canonical=True)))
    finally:
        inlineasm._read_src_cached = old_read2
    eq("q126: ... and the caller has no layer-2 key", L2.body_key(amb, "func_A"), None)
    eq("q126: own_blocks excludes helper islands (rewrite tools)",
       completion.own_blocks(text, "func_A"), [])
    eq("q126: own_blocks of the helper is its own island",
       [text[s:e] for s, e in completion.own_blocks(text, "sq")], ["gte_Lzc(x, &lz[0]);"])
    import importlib
    import sys as _sys
    _sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
    GMR = importlib.import_module("gte_macro_rewrite")
    eq("q126 rewrite tool: a caller's runs exclude its helper's islands",
       GMR._runs(text, "func_A"), [])
    eq("q126 rewrite tool: the helper is rewritten as its own target",
       [[text[s:e] for s, e, _st in run] for run in GMR._runs(text, "sq")],
       [["gte_Lzc(x, &lz[0]);"]])

    # islands / region hashes
    eq("q126: the helper's macro call is the caller's island",
       [text[s:e] for s, e in completion.blocks(text, "func_A")], ["gte_Lzc(x, &lz[0]);"])
    eq("q126: caller's region hashes are the helper's",
       completion.region_hashes(text, "func_A"), completion.region_hashes(text, "sq"))
    eq("q126: transitive caller gets the inner helper's island",
       completion.region_hashes(text, "func_B"), completion.region_hashes(text, "sq"))
    eq("q126: non-inline static's island stays its own", completion.blocks(text, "func_D"), [])
    eq("q126: helper-free body has no islands", completion.blocks(text, "func_E"), [])

    with tempfile.TemporaryDirectory() as td:
        grants = Path(td) / "regions.json"
        old_read, old_regions = inlineasm._read_src_cached, completion.REGIONS

        def issues(src: str, func: str = "func_A", canonical: bool = True) -> list[str]:
            inlineasm._read_src_cached = lambda stem: src
            return completion.source_issues("fake", func, canonical=canonical)
        try:
            completion.REGIONS = grants
            check("q126: a non-canonical caller of an island helper is refused",
                  any("requires canonical" in i for i in issues(text, canonical=False)))
            grants.write_text(json.dumps({"schema": 2, "functions": {"func_A": {
                "file": "fake", "sha256": completion.region_hashes(text, "func_A")}}}))
            eq("q126: a granted caller of an island helper passes", issues(text), [])
            check("q126: a helper island edit voids the caller's grant", any(
                "changed since review" in i
                for i in issues(text.replace("gte_Lzc(x, &lz[0]);", "gte_Lzc(x + 1, &lz[0]);"))))
            pinned = text.replace("    return lz[0];\n}\n",
                                  "    register s32 r asm(\"$8\") = lz[0];\n    return r;\n}\n", 1)
            eq("q126: a register pin inside the helper is the caller's cheat (qualifier + hint)",
               inlineasm.func_cheat_asm_count(pinned, "func_A"), 2)
            check("q126: ... and the caller's granted islands no longer match", any(
                "changed since review" in i for i in issues(pinned)))
            unwritten = text.replace("    gte_Lzc(x, &lz[0]);\n",
                                     "    s32 pad[4];\n    gte_Lzc(x, &lz[0]);\n", 1)
            eq("q126: an unwritten array in the helper fails the caller's source check",
               issues(unwritten), ["1 cheat construct(s) outside approved assembly"])
            nop = text.replace("    return lz[0];\n}\n", "    gte_nop();\n    return lz[0];\n}\n", 1)
            check("q126: gte_nop() inside the helper is refused for the caller", any(
                "gte_nop" in i for i in issues(nop)))
        finally:
            inlineasm._read_src_cached, completion.REGIONS = old_read, old_regions

    # cheat counts: every detector reads the helper as part of the caller
    eq("q126: the 87A0 shape (lz[4], lz[0] live) counts as in the caller: 0",
       inlineasm.func_cheat_asm_count(text, "func_A"), 0)
    in_caller = (inc + "s32 func_A(s32 v) {\n    s32 lz[4]; /* FAKE: frame layout */\n"
                 "    gte_Lzc(v, &lz[0]);\n    return lz[0] + 1;\n}\n")
    eq("q126: ... the same count as the pad written in the caller",
       inlineasm.func_cheat_asm_count(in_caller, "func_A"),
       inlineasm.func_cheat_asm_count(text, "func_A"))
    dead = text.replace("    s32 lz[4]; /* FAKE: frame layout */\n",
                        "    s32 lz[4]; /* FAKE: frame layout */\n    s32 pad[4];\n", 1)
    eq("q126: an unwritten local array in the helper counts against the caller",
       inlineasm.func_cheat_asm_count(dead, "func_A"), 1)
    eq("q126: ... and against a transitive caller",
       inlineasm.func_cheat_asm_count(dead, "func_B"), 1)
    eq("q126: ... but not against a caller of a non-inline static",
       inlineasm.func_cheat_asm_count(dead, "func_D"), 0)
    vpad = text.replace("    s32 lz[4]; /* FAKE: frame layout */\n",
                        "    s32 lz[4]; /* FAKE: frame layout */\n"
                        "    volatile u32 pre_pad[2]; /* FAKE: frame layout */\n", 1)
    rows = volatile_cheats._SANCTIONED_UNWRITTEN_PADS
    try:
        volatile_cheats._SANCTIONED_UNWRITTEN_PADS = {"func_A": frozenset({("pre_pad", 2)})}
        eq("q126: a pad in the helper is admitted by the CALLER's pad row",
           volatile_cheats.func_volatile_cheat_count(vpad, "func_A"), 0)
        eq("q126: ... not for another caller without the row",
           volatile_cheats.func_volatile_cheat_count(vpad, "func_B"), 1)
        volatile_cheats._SANCTIONED_UNWRITTEN_PADS = {"sq": frozenset({("pre_pad", 2)})}
        eq("q126: a row keyed by the helper's name admits nothing",
           volatile_cheats.func_volatile_cheat_count(vpad, "func_A"), 1)
    finally:
        volatile_cheats._SANCTIONED_UNWRITTEN_PADS = rows
    vext = ("extern volatile s32 D_80001234;\n" + inc
            + "static inline s32 rd(void) { return D_80001234; }\n"
            "s32 func_V(void) {\n    return rd();\n}\n")
    check("q126: a volatile extern read only in the helper counts for the caller",
          volatile_cheats.func_volatile_cheat_count(vext, "func_V") >= 1)

    # layer-2 key
    k = lambda src, f: L2.body_key(src, f)[1]  # noqa: E731
    own = lambda src, f: L2._key(src[inlineasm._func_body_span(src, f)[0]:  # noqa: E731
                                     inlineasm._func_body_span(src, f)[1]])
    eq("q126: a helper-free body keys exactly as its own definition",
       k(text, "func_E"), own(text, "func_E"))
    eq("q126: ... whatever helpers the TU defines",
       k(text, "func_E"), k(inc + "s32 func_E(s32 v) {\n    return v + 2;\n}\n", "func_E"))
    eq("q126: a non-inline static's caller keys as its own definition",
       k(text, "func_D"), own(text, "func_D"))
    check("q126: a helper caller's key covers the helper", k(text, "func_A") != own(text, "func_A"))
    check("q126: a helper token edit moves the caller's key",
          k(text, "func_A") != k(text.replace("return lz[0];", "return lz[1];"), "func_A"))
    check("q126: ... and a transitive caller's key",
          k(text, "func_B") != k(text.replace("return lz[0];", "return lz[1];"), "func_B"))
    eq("q126: a helper comment edit keeps the caller's key",
       k(text, "func_A"), k(text.replace("/* FAKE: frame layout */", "/* FAKE: other */"),
                            "func_A"))
    eq("q126: a non-inline static's edit does not move its caller's key",
       k(text, "func_D"), k(text.replace("return lz;", "return lz + 0;"), "func_D"))
    eq("q126: a cyclic helper graph keys (no recursion)",
       L2.body_key(text, "func_C")[0], "c")
    dup = text + "#if 0\nstatic inline s32 sq(s32 x) { return x; }\n#endif\n"
    eq("q126: a helper with two definitions leaves the caller unkeyed (fail closed)",
       L2.body_key(dup, "func_A"), None)
    eq("q126: helpers=False is the pre-Q126 definition-only key",
       L2.body_key(text, "func_A", helpers=False)[1], own(text, "func_A"))
    from engine import departures as DEP
    eq("q126 departures: cutoff is the committer instant of b832f3d48",
       DEP.Q126_CUTOFF, "2026-10-23T18:12:59Z")
    both = DEP._keys(text, "func_A")
    eq("q126 departures: both keys of a helper caller",
       both, (k(text, "func_A"), own(text, "func_A")))
    before, after = "2026-10-01T00:00:00Z", DEP.Q126_CUTOFF
    for pre in (False, True):
        for fields in ({}, {"date": ""}, {"date": "garbage"}, {"date": after}):
            check("q126 departures: a helper-inclusive key always matches",
                  DEP._on_body({"body_hash": both[0], **fields}, both, pre))
    check("q126 departures: a pre-Q126 record on the definition-only key matches",
          DEP._on_body({"body_hash": both[1], "date": before}, both, True))
    for fields in ({}, {"date": ""}, {"date": "garbage"}, {"date": None},
                   {"date": "2026-10-01"}, {"date": "2026-10-01T00:00:00"},
                   {"date": after}, {"date": "2026-10-25T00:00:00Z"},
                   {"date": "2026-10-23T18:12:59Z"},
                   {"date": "2026-10-23T18:13:00Z"},
                   {"date": "2026-10-23T23:30:00Z"},
                   {"date": "2026-10-24T00:30:00+01:00"},
                   {"date": "2026-10-23T23:30:00-01:00"}):
        check(f"q126 departures: invalid or non-pre-cutoff date {fields} cannot fall back",
              not DEP._on_body({"body_hash": both[1], **fields}, both, True))
    for date in (before, "2026-10-23T13:12:58-05:00"):
        check("q126 departures: record offsets compare as UTC instants",
              DEP._on_body({"body_hash": both[1], "date": date}, both, True))
        check("q126 departures: a new departure never uses an old definition-only PASS",
              not DEP._on_body({"body_hash": both[1], "date": date}, both, False))
    check("q126 departures: an unrelated key never matches",
          not DEP._on_body({"body_hash": "0" * 16, "date": before}, both, True))
    check("q126 departures: no body, no match",
          not DEP._on_body({"body_hash": both[0], "date": before}, None, True))

    class CommitBatch:
        def __init__(self, data):
            self.data = data

        def must(self, obj, what):
            eq("q126 departures: reads the departure commit", obj, "commit-id")
            return self.data

    from datetime import datetime
    cutoff_epoch = int(datetime.fromisoformat(after.replace("Z", "+00:00")).timestamp())
    for delta in (-1, 0, 1):
        for offset in ("+1400", "-1200"):
            data = (f"author A <a@b> 0 +0000\n"
                    f"committer C <c@d> {cutoff_epoch + delta} {offset}\n\nmessage\n").encode()
            eq("q126 departures: uses committer epoch, independent of displayed offset",
               DEP._pre_q126(CommitBatch(data), "commit-id"), delta < 0)
    for c in (None, "WORKTREE"):
        eq("q126 departures: unknown / working-tree departure never reads a commit",
           DEP._pre_q126(None, c), False)
    for data in (b"author A <a@b> 0 +0000\n\ncommitter fake 0 +0000\n",
                 b"committer C <c@d> garbage +0000\n\nmessage\n"):
        try:
            DEP._pre_q126(CommitBatch(data), "commit-id")
        except DEP._AuditError:
            check("q126 departures: malformed committer date fails closed", True)
        else:
            check("q126 departures: malformed committer date fails closed", False)

    # integrity: an out-of-line helper is refused; an unrecognized helper too
    import importlib
    cwd = os.getcwd()
    try:
        CCI = importlib.import_module("tools.check_completion_integrity")
    except ImportError:
        import sys as _sys
        _sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
        CCI = importlib.import_module("check_completion_integrity")
    finally:
        os.chdir(cwd)
    ftab = {"func_A": 0, "func_B": 0, "func_C": 0, "func_D": 0, "func_E": 0, "plain": 0}
    eq("q126 integrity: all helpers inlined, nothing refused",
       CCI._helper_issues(text, "fake", ftab), [])
    balance_issue = (
        "fake.c: file-scope braces do not balance or cannot be indexed "
        "safely (lexical view/identifier/directive escapes; brace/branch "
        "imbalance; unsafe macros/heads; duplicate definitions) -- walk "
        "reads one masked TU view with supplied header macro facts "
        "(included bodies unseen); "
        "use explicit, complete definitions")
    for desc, src, unsafe in header_cases:
        eq(f"q126 integrity: {desc}",
           balance_issue in CCI._helper_issues(src, "fake", {"f": 0}, header_facts),
           unsafe)
    for desc, src, unsafe in walk_cases:
        eq(f"q126 integrity: {desc}",
           balance_issue in CCI._helper_issues(src, "fake", {"f": 0}), unsafe)
    check("q126 integrity: parenthesised helper with dead assignment is refused",
          balance_issue in CCI._helper_issues(paren_helper, "fake", {"f": 0}))
    eq("q126 integrity: alternative function heads hiding a helper are refused",
       CCI._helper_issues(split_head, "fake", {"f": 0}), [balance_issue])
    for desc, src in evasion_cases:
        check(f"q126 integrity: {desc} is refused",
              balance_issue in CCI._helper_issues(src, "fake", {"f": 0}))
    for desc, src in lexical_cases:
        check(f"q126 integrity: {desc} is refused",
              balance_issue in CCI._helper_issues(src, "fake", {"f": 0}))
    eq("q126 integrity: a macro-only function head is refused",
       CCI._helper_issues(macro_head, "fake", {"f": 0}), [balance_issue])
    for src in comparison_heads:
        eq("q126 integrity: a comparison parameter helper is attributed, not refused",
           CCI._helper_issues(src, "fake", {"f": 0}), [])
    for src in nonfunction_heads:
        eq("q126 integrity: clear nonfunction brace head passes",
           CCI._helper_issues(src, "fake", {}), [])
    eq("q126 integrity: compensated branch imbalance is refused",
       CCI._helper_issues(compensated, "fake", {"f": 0}), [balance_issue])
    eq("q126 integrity: complete conditional definitions pass",
       CCI._helper_issues(complete_branches, "fake", {"f": 0, "g": 0}), [])
    eq("q126 integrity: a brace macro is refused",
       CCI._helper_issues(brace_macro, "fake", {"f": 0}), [balance_issue])
    eq("q126 integrity: a closing brace macro is refused",
       CCI._helper_issues(close_macro, "fake", {"f": 0}), [balance_issue])
    eq("q126 integrity: a self-balanced brace macro passes",
       CCI._helper_issues(balanced_macro, "fake", {"f": 0}), [])
    eq("q126 integrity: a macro closing before opening is refused",
       CCI._helper_issues(reversed_macro, "fake", {"f": 0}), [balance_issue])
    for desc, src in (("branch closes before reopening", reopened_branch),
                      ("branch replaces a function head", replaced_branch),
                      ("brace macro defines a file-scope helper", hidden_macro)):
        check(f"q126 integrity: {desc} is refused",
              balance_issue in CCI._helper_issues(src, "fake", {"f": 0}))
    eq("q126 integrity: self-balanced brace macro use in a body passes",
       CCI._helper_issues(body_macro, "fake", {"f": 0}), [])
    eq("q126 integrity: a file-scope alias to a brace macro is refused",
       CCI._helper_issues(hidden_alias, "fake", {"f": 0}), [balance_issue])
    eq("q126 integrity: a body-only alias to a balanced brace macro passes",
       CCI._helper_issues(body_alias, "fake", {"f": 0}), [])
    eq("q126 integrity: a stray brace is refused even without helpers",
       CCI._helper_issues("}" + normal_tu, "fake", {"f": 0}), [balance_issue])
    eq("q126 integrity: a normal TU without helpers passes",
       CCI._helper_issues(normal_tu, "fake", {"f": 0}), [])
    eq("q126 integrity: a complete function inside #if 0 passes brace checking",
       CCI._helper_issues(dead_tu, "fake", {"f": 0}), [])
    check("q126 integrity: a helper also emitted out of line is refused", any(
        "sq (fake.c): static inline helper emitted out of line" in i
        for i in CCI._helper_issues(text, "fake", dict(ftab, sq=0))))
    hidden = text.replace("static s32 plain", "static INL s32 plain")
    del ftab["plain"]
    check("q126 integrity: a symbol-less definition that is no recognized helper is refused",
          any("plain (fake.c): function body with no symbol" in i
              for i in CCI._helper_issues(hidden, "fake", ftab)))
    ftab["plain"] = 0
    check("q126 integrity: a duplicated helper is refused", any(
        "sq (fake.c): static inline helper defined 2 times" in i
        for i in CCI._helper_issues(dup, "fake", ftab)))
    viamac = text + "#define CALLSQ(x) sq(x)\n"
    check("q126 integrity: a helper named in a #define body is refused", any(
        "sq (fake.c): static inline helper named in a #define" in i
        for i in CCI._helper_issues(viamac, "fake", ftab)))
    orphan = text + "static inline s32 unused_h(s32 x) { return x; }\n"
    check("q126 integrity: a helper no function reaches is refused", any(
        "unused_h (fake.c): static inline helper that no function in the object reaches" in i
        for i in CCI._helper_issues(orphan, "fake", ftab)))
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        (root / "include/psxsdk").mkdir(parents=True)
        (root / "src/main").mkdir(parents=True)
        (root / "include/game.h").write_text("extern s32 g;\n#define SQ(x) ((x) * (x))\n")
        (root / "include/bad.h").write_text("static inline s32 h(s32 x) {\n    return x;\n}\n")
        (root / "src/main/local.h").write_text("s32 f(void) { return 0; }\n")
        (root / "include/psxsdk/inline_o.h").write_text("static inline s32 v(void) { return 0; }\n")
        eq("q126 integrity: function bodies in project headers are refused (vendored excepted)",
           [i.split(":")[0] + ":" + i.split("definition ")[1].split(" ")[0]
            for i in CCI._header_function_issues(root)],
           ["include/bad.h:h", "src/main/local.h:f"])
        (root / "include/psxsdk/inline_o.h").write_text("#define OPEN {\n")
        (root / "src/main/local.h").write_text("#define LOCAL OPEN\n")
        facts = CCI._header_macro_facts(root)
        eq("q126 integrity: gathers vendored brace macro and src header alias",
           inlineasm.tu_unbalanced("LOCAL\nvoid f(void) {}", facts), True)
        eq("q126 integrity: gathered unbalanced header macros refuse body uses",
           CCI._helper_issues("void f(void) { LOCAL }", "fake", {"f": 0}, facts),
           [balance_issue])
        (root / "movovl/include").mkdir(parents=True)
        (root / "movovl/include/overlay.h").write_text(
            "#define OVERLAY_OPEN {\n", encoding="utf-8")
        eq("q126 integrity: movovl header macros are collected",
           inlineasm.tu_unbalanced("OVERLAY_OPEN\nvoid f(void) {}",
                                   CCI._header_macro_facts(root)), True)
        eq("q126 integrity: unbalanced defines alone do not refuse headers",
           CCI._header_safety_issues(root), [])
        for rel in ("include/psxsdk/unsafe.h", "src/main/unsafe.h",
                    "movovl/include/unsafe.h"):
            p = root / rel
            for header in unsafe_headers:
                p.write_text(header, encoding="utf-8")
                eq(f"q126 integrity: unsafe header {rel}: {header!r}",
                   [i.split(":")[0] for i in CCI._header_safety_issues(root)], [rel])
            p.unlink()
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        (root / "include/psxsdk").mkdir(parents=True)
        (root / "src/main").mkdir(parents=True)
        (root / "src/main/a.c").write_text(
            "static inline s32 sq(s32 x) { return x * x; }\n"
            "s32 f(s32 x) { return sq(x); }\n")
        issue = ("sq (main/a.c): static inline helper named in a #define body in "
                 "include/game.h -- call it directly, so its callers are visible")
        for desc, header, expected in (
                ("replacement list", "#define SQ2(x) sq(x)\n", [issue]),
                ("line splice", "#define SQ2(x) s\\\nq(x)\n", [issue]),
                ("parameter list", "#define SQ2(sq) 0\n", [issue]),
                ("multiple macros report once",
                 "#define SQ2(x) sq(x)\n#define SQ3(x) sq(x)\n", [issue]),
                ("comments", "/* sq */\n#define SQ2(x) (x) /* sq */\n", []),
                ("string literal", '#define SQ2 "sq"\n', []),
                ("non-define line", "extern s32 sq(s32 x);\n", []),
                ("unrelated macro", "#define SQ2(x) other(x)\n", []),
                ("identifier boundary", "#define SQ2(x) sq_more(x)\n", []),
                ("macro named like the helper", "#define sq(x) ((x) * (x))\n",
                 [issue])):
            (root / "include/game.h").write_text(header)
            eq(f"q126 integrity: header macro helper check: {desc}",
               CCI._header_function_issues(root), expected)
        (root / "include/game.h").write_text("#define SQ2(x) sq(x)\n")
        (root / "include/psxsdk/inline_o.h").write_text("#define SQ3(x) sq(x)\n")
        (root / "src/main/local.h").write_text("#define SQ4(x) sq(x)\n")
        eq("q126 integrity: helper macros in vendored and src headers are refused too",
           CCI._header_function_issues(root),
           [issue, issue.replace("include/game.h", "include/psxsdk/inline_o.h"),
            issue.replace("include/game.h", "src/main/local.h")])
    eq("q126 integrity: helper island blocks count for the caller",
       CCI._attributed_islands({"sq": 2, "func_A": 1}, text, "func_B"), 2)
    eq("q126 integrity: ... and add to the caller's own",
       CCI._attributed_islands({"sq": 2, "func_A": 1}, text, "func_A"), 3)


def test_buildstamp() -> None:
    """A matching old artifact stops being evidence as soon as inputs drift."""
    with tempfile.TemporaryDirectory() as td:
        state = {"inputs": {"src/a.c": "one"}, "artifacts": {"build/a.o": "obj"}}
        old = (buildstamp.STAMP, buildstamp.inputs, buildstamp.artifacts)
        try:
            buildstamp.STAMP = Path(td) / "verified-inputs.json"
            buildstamp.inputs = lambda: dict(state["inputs"])
            buildstamp.artifacts = lambda: dict(state["artifacts"])
            buildstamp.record(buildstamp.inputs())
            eq("build stamp: freshly recorded inputs are accepted",
               buildstamp.check(), {"fresh": True})
            state["inputs"]["src/a.c"] = "two"
            check("build stamp: source drift makes the artifact stale",
                  buildstamp.check().get("reason") == "build inputs changed")
            buildstamp.record(buildstamp.inputs())
            state["artifacts"]["build/a.o"] = "tampered"
            check("build stamp: artifact drift is detected",
                  buildstamp.check().get("reason") == "build artifacts changed")
        finally:
            buildstamp.STAMP, buildstamp.inputs, buildstamp.artifacts = old


def test_queue_non_function_labels() -> None:
    """No-body source errors must not turn object-table labels into work."""
    from unittest.mock import patch
    repo_root = Path(Q.__file__).resolve().parent.parent

    labels = {
        "main/3AB48": ("D_800521AC", "D_800521FC", "D_80052344",
                        "D_80052394"),
        "main/43500": ("D_800545F8", "D_800545FC", "D_80054600"),
        "main/d_7D870": ("g_data_start", "g_main_loop_handlers"),
    }
    texts = {stem: Path(f"src/{stem}.c").read_text(encoding="utf-8")
             for stem in labels}
    included_asm = {name: Path(f"asm/funcs/{name}.s").read_text(encoding="utf-8")
                    for text in texts.values()
                    for name, _s, _e in inlineasm.include_asm_spans(text)
                    if Path(f"asm/funcs/{name}.s").is_file()}
    label_names = {name for names in labels.values() for name in names}
    texts["unknown"] = "extern void mystery(void);\n"
    texts["body"] = ('void real_body(void) { __asm__("nop"); }\n'
                     '__asm__(".aent real_body\\n");\n'
                     'INCLUDE_ASM("asm/funcs", asm_body);\n')
    tables = {stem: {name: (0, 4) for name in names}
              for stem, names in labels.items()}
    tables.update(unknown={"mystery": (0, 4)},
                  body={"real_body": (0, 4), "asm_body": (0, 4)})
    scanned, gated = [], []
    rules, prologues = {}, {}
    gate_reason = None
    missing = False

    def measure(a, b, func):
        scanned.append(func)
        if missing:
            raise KeyError(func)
        return {"score": 23}

    def gate(func, stem):
        gated.append(func)
        return gate_reason

    with tempfile.TemporaryDirectory() as td, contextlib.ExitStack() as stack:
        cwd = os.getcwd()
        try:
            os.chdir(td)
            Path("asm/funcs").mkdir(parents=True)
            for name, asm in included_asm.items():
                Path(f"asm/funcs/{name}.s").write_text(asm, encoding="utf-8")
            for stem, text in texts.items():
                p = Path(f"src/{stem}.c")
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text(text, encoding="utf-8")
                obj = Path(f"build/src/{stem}.o")
                obj.parent.mkdir(parents=True, exist_ok=True)
                obj.touch()
            stack.enter_context(patch.dict(inlineasm._FILE_TEXT_CACHE, {}, clear=True))
            for obj, attr, value in (
                (Q.gtemacro, "REPO_ROOT", Path(td).resolve()),
                (Q, "QUEUE_PATH", "queue.json"),
                (P, "c_stems", lambda: list(texts)),
                (canonical, "scan_all", lambda: []),
                (cheats, "canonical_asm_funcs", lambda: set()),
                (Q, "_rule_count", lambda f: rules.get(f, 0)),
                (cheats, "func_prologue_count", lambda f: prologues.get(f, 0)),
                (cheats, "is_jtbl_infra", lambda f: False),
                (cheats, "is_canonical_extraction_only", lambda f: False),
                (Q, "_route_with_evidence", lambda *a: ("C", None)),
                (Q.sandbox, "build_stripped_object", lambda *a, **k: {}),
                (score, "_o_func_table", lambda p: tables[str(Path(p).relative_to("build/src").with_suffix(""))]),
                (score, "score_func", measure),
                (score, "normalized_insns", lambda *a: ["nop"]),
                (Q.layer2, "gate", gate),
            ):
                stack.enter_context(patch.object(obj, attr, value))

            for stem, names in labels.items():
                for name in names:
                    check(f"queue labels: real source classifies {name}",
                          Q._not_a_c_function(stem, name))
                    check(f"queue labels: {name} has the no-body issue being bypassed",
                          bool(completion.source_issues(stem, name)))

            # The tools' classification (check_completion_integrity.py hands the
            # working-tree reader to not_a_c_function_text; spotcheck's roster mode
            # a git-ref reader): the alabels of the included asm are data-as-code,
            # and without the asm evidence the same absent names stay outstanding.
            alabel_stems = ("main/3AB48", "main/43500")
            for stem in alabel_stems:
                for name in labels[stem]:
                    check(f"integrity labels: {name} is data-as-code with its included asm",
                          Q.not_a_c_function_text(texts[stem], name, Q.read_asm_file))
                    check(f"integrity labels: {name} is outstanding without the asm",
                          not Q.not_a_c_function_text(texts[stem], name, lambda n: None))
            import importlib.util
            spec = importlib.util.spec_from_file_location(
                "spot_check_completed_t", repo_root / "tools/spotcheck/spot_check_completed.py")
            spot = importlib.util.module_from_spec(spec)
            here = os.getcwd()
            spec.loader.exec_module(spot)   # the script chdirs to the repo on import
            os.chdir(here)
            pool_syms = {stem: list(labels[stem]) for stem in alabel_stems}
            pool_syms["main/3AB48"].append("real_c_fn")
            pool_src = {stem: texts[stem] for stem in alabel_stems}
            pool_src["main/3AB48"] += "\nvoid real_c_fn(void) {}\n"
            done, _viol, no_body = spot.classify_pool(pool_syms, set(), set(), {}, set(),
                                                      pool_src, Q.read_asm_file)
            eq("spotcheck labels: the seven alabels are no-C-body, not completions",
               sorted(f for f, _ in no_body),
               sorted(n for stem in alabel_stems for n in labels[stem]))
            eq("spotcheck labels: a real C function stays in the pool",
               [f for f, _ in done], ["real_c_fn"])

            for missing in (False, True):
                Path(Q.QUEUE_PATH).write_text('{"items": []}')
                scanned.clear()
                q = Q.generate(workdir="scan")
                eq("queue labels: ten labels excluded even with nonzero/missing score",
                   {it["func"] for it in q["items"]},
                   {"mystery", "real_body", "asm_body"})
                check("queue labels: excluded labels never reach scoring",
                      label_names.isdisjoint(scanned))

            rules["D_800521AC"] = 1
            prologues["g_data_start"] = 1
            Path(Q.QUEUE_PATH).write_text('{"items": []}')
            q = Q.generate(workdir="scan")
            check("queue labels: rule/prologue debt still retained",
                  {"D_800521AC", "g_data_start"} <= {it["func"] for it in q["items"]})
            rules.clear()
            prologues.clear()

            seed = {"func": "g_data_start", "file": "main/d_7D870",
                    "distance": 0, "rules": 0, "verdict": "C", "status": "active"}
            for gate_reason in ("needs review", None):
                Path(Q.QUEUE_PATH).write_text(json.dumps({"items": [seed]}))
                gated.clear()
                q = Q.generate(workdir="scan")
                held = [it for it in q["items"] if it["func"] == "g_data_start"]
                eq("queue labels: previously listed label uses Q39 gate", gated, ["g_data_start"])
                eq("queue labels: no departure bypass for previously listed label",
                   bool(held), gate_reason is not None)
        finally:
            os.chdir(cwd)


def test_canonical_completion_is_the_drop() -> None:
    """THE DROP is the canonical completion — pinned end-to-end, not just at
    mark_done.

    Campaign 4 Wave 6 converted 65 canonical-authorized functions to zero-rule
    INCLUDE_ASM form, and the payoff checker written for it first reported
    0/19 FAILED: it called mark_done on the POST-regen queue and got "not in
    queue" every time. Nothing was broken. generate() DROPS a canonical
    zero-rule function before mark_done can ever see it, so the drop IS the
    completion.

    test_include_asm_whole_body could not have surfaced that: it hand-builds a
    queue CONTAINING the function and calls mark_done, which is precisely the
    precondition regen removes. That gap is the reason this test exists —
    a unit test that constructs the state the real path deletes will pass
    forever while the real path says something else.

    Pinned here:
      1. generate() drops a zero-rule canonical member.
      2. mark_done on a queue that still LISTS it returns
         COMPLETED-INLINE-ASM-CANONICAL (the gate itself).
      3. mark_done AFTER the drop refuses, and the refusal says the function is
         already complete rather than the bare "not in queue" that reads as an
         error to anyone typing `queue done <canonical-func>`.
    """
    import pathlib

    class _RefObjExists:
        """Path shim — a `.o` always 'exists', everything else is a real Path.

        Keeps this in the FAST tier: driving generate() against real build/
        objects would both need a build and race any agent mid-rebuild.
        """

        def __init__(self, p):
            self._p = pathlib.Path(p)

        def exists(self):
            return True if str(self._p).endswith(".o") else self._p.exists()

        def __getattr__(self, n):
            return getattr(self._p, n)

        def __fspath__(self):
            return str(self._p)

        def __str__(self):
            return str(self._p)

    seed = {"items": [{"func": "func_CANON", "file": "faketu", "distance": 300,
                       "verdict": "ASM-WHOLE", "rules": 0, "status": "authorize"}],
            "counts": {}}

    with tempfile.TemporaryDirectory() as td, _stub_tus(td, {
            "faketu": 'INCLUDE_ASM("asm/funcs", func_CANON);\n'}):
        qp = Path(td) / "queue.json"
        orig = (Q.QUEUE_PATH, Q.Path, P.c_stems, canonical.scan_all,
                cheats.canonical_asm_funcs, Q._rule_count,
                cheats.func_prologue_count, cheats.maspsx_gate_entries,
                inlineasm.file_func_cheat_asm_count,
                completion.source_issues,
                Q.sandbox.build_stripped_object, score._o_func_table,
                score.score_func, Q.O.verify, cheats.is_jtbl_infra,
                cheats.is_canonical_extraction_only, Q.layer2.gate)
        try:
            # Patch INSIDE the try: a raise part-way through the assignments
            # would otherwise leak monkeypatches into every later test in the
            # process.
            Q.QUEUE_PATH = str(qp)
            Q.Path = _RefObjExists
            P.c_stems = lambda: ["faketu"]
            canonical.scan_all = lambda: [{"func": "func_CANON", "verdict": "ASM-WHOLE"}]
            cheats.canonical_asm_funcs = lambda: {"func_CANON"}
            Q._rule_count = lambda f: 0
            cheats.func_prologue_count = lambda f: 0
            cheats.maspsx_gate_entries = lambda f: []
            # Stub these two as well, or the rule-count-1 regen below reads the
            # REAL regfix.txt/asmfix.txt (matching test_include_asm_whole_body).
            cheats.is_jtbl_infra = lambda f: False
            cheats.is_canonical_extraction_only = lambda f: False
            # >0 == whole-body asm attributed to it, which is what a converted
            # canonical function looks like to the detectors (9e68966c).
            inlineasm.file_func_cheat_asm_count = lambda s, f: 1
            completion.source_issues = lambda *a, **k: []
            Q.sandbox.build_stripped_object = lambda *a, **k: None
            score._o_func_table = lambda o: ["func_CANON"]
            score.score_func = lambda a, b, f: {"score": 300}
            Q.O.verify = lambda rebuild=False: {"build_matches": True,
                                                "build_sha1": "deadbeef"}
            Q.layer2.gate = lambda f, s: None  # the Q39 gate has its own test
            # 2. the gate, against a queue that still lists it
            qp.write_text(json.dumps(seed))
            r = Q.mark_done("func_CANON")
            check("canon-drop: mark_done accepts a listed zero-rule canonical",
                  r.get("ok") is True)
            eq("canon-drop: completion state",
               r.get("completion"), "COMPLETED-INLINE-ASM-CANONICAL")

            # 1. regen drops it
            qp.write_text(json.dumps(seed))
            regen = Q.generate(workdir=str(Path(td) / "wd"), preserve=False)
            names = [it["func"] for it in regen["items"]]
            check("canon-drop: generate() DROPS a zero-rule canonical member",
                  "func_CANON" not in names)

            # ...and does NOT drop it once a rule reappears — the drop must be
            # earned by the zero-rule state, not by canonical membership alone.
            Q._rule_count = lambda f: 1
            regen2 = Q.generate(workdir=str(Path(td) / "wd2"), preserve=False)
            check("canon-drop: a canonical member WITH a rule is retained",
                  "func_CANON" in [it["func"] for it in regen2["items"]])
            Q._rule_count = lambda f: 0

            # 3. post-drop mark_done: refused, and the reason says COMPLETE
            qp.write_text(json.dumps(regen))
            r2 = Q.mark_done("func_CANON")
            check("canon-drop: mark_done after the drop is REFUSED",
                  r2.get("ok") is False
                  and "COMPLETED-INLINE-ASM-CANONICAL" in r2.get("reason", "")
                  and "missing; cannot check" not in r2.get("reason", ""))
            check("canon-drop: refusal names the completion, not 'not in queue'",
                  "COMPLETED-INLINE-ASM-CANONICAL" in r2.get("reason", ""))
            check("canon-drop: refusal is not the bare not-in-queue string",
                  r2.get("reason") != "not in queue")

            # The refusal EXPLAINS the absence; it must not CERTIFY completion.
            # No oracle runs on this path, so a machine-readable completion key
            # here would be more assertive on less evidence than any other
            # `completion` in mark_done (all of which are O.verify-gated), and
            # it cannot mirror generate()'s output anyway — generate() re-adds
            # origin=="regression" items after the loop, a state that lives only
            # in queue.json and is unrecoverable once the item is absent.
            check("canon-drop: refusal mints NO machine-readable completion key",
                  r2.get("already_complete") is None
                  and r2.get("completion") is None
                  and r2.get("completion_state") is None)
            check("canon-drop: refusal says no oracle check was run",
                  "no oracle check" in r2.get("reason", ""))

            # The refusal's predicate must still match the IN-LOOP drop conjunct
            # for conjunct: absence has causes other than completion (stale
            # queue.json, missing build/src/<stem>.o, a failed stripped build),
            # so a predicate weaker than the drop's would describe a function
            # the drop would have RETAINED. An earlier version omitted
            # `prologue == 0` and so covered a prologue_fix-carrying function.
            cheats.func_prologue_count = lambda f: 1
            r4 = Q.mark_done("func_CANON")
            check("canon-drop: prologue_fix entry blocks the completion wording",
                  r4.get("ok") is False
                  and "COMPLETED-INLINE-ASM-CANONICAL" not in r4.get("reason", ""))
            eq("canon-drop: prologue_fix absentee gets the plain refusal",
               r4.get("reason"), "not in queue")
            # ...and generate() likewise RETAINS it, which is the property the
            # refusal is mirroring.
            qp.write_text(json.dumps(seed))
            regen3 = Q.generate(workdir=str(Path(td) / "wd3"), preserve=False)
            check("canon-drop: canonical member WITH a prologue entry is retained",
                  "func_CANON" in [it["func"] for it in regen3["items"]])
            cheats.func_prologue_count = lambda f: 0

            # A NON-canonical absentee still gets the plain message — the
            # friendlier reason must not leak onto typos or unknown symbols.
            cheats.canonical_asm_funcs = lambda: set()
            r3 = Q.mark_done("func_TYPO")
            eq("canon-drop: unknown function keeps the plain refusal",
               r3.get("reason"), "not in queue")
        finally:
            (Q.QUEUE_PATH, Q.Path, P.c_stems, canonical.scan_all,
             cheats.canonical_asm_funcs, Q._rule_count,
             cheats.func_prologue_count, cheats.maspsx_gate_entries,
             inlineasm.file_func_cheat_asm_count,
             completion.source_issues,
             Q.sandbox.build_stripped_object, score._o_func_table,
             score.score_func, Q.O.verify, cheats.is_jtbl_infra,
             cheats.is_canonical_extraction_only, Q.layer2.gate) = orig


def test_header_facts_gates() -> None:
    """Live per-function gates see headers; historical body keys do not."""
    from unittest.mock import patch
    from engine import gtemacro, layer2
    import check_completion_integrity as CCI

    cwd = Path.cwd()
    with tempfile.TemporaryDirectory() as td, contextlib.ExitStack() as stack:
        root = Path(td).resolve()
        stack.enter_context(patch.object(gtemacro, "REPO_ROOT", root))
        os.chdir(root)
        stack.callback(os.chdir, cwd)
        Path("src").mkdir()
        Path("include").mkdir()
        src = ("CLOSE;\nstatic inline int helper(int x) { return x + 1; }\n"
               "int func_80011000(int x) { return helper(x); }\n")
        Path("src/gates.c").write_text(src, encoding="utf-8")
        func = "func_80011000"
        h = layer2.body_key(src, func)[1]
        item = {"func": func, "file": "gates", "status": "active"}
        for obj, name, value in (
                (Q, "load", lambda: {"items": [item]}),
                (Q, "_rule_count", lambda f: 0),
                (Q, "_locked", contextlib.nullcontext),
                (Q, "_fingerprint", lambda: "fixture"),
                (Q, "save", lambda *a, **kw: None),
                (cheats, "func_prologue_count", lambda f: 0),
                (cheats, "canonical_asm_funcs", lambda: set()),
                (cheats, "maspsx_gate_entries", lambda f: []),
                (completion, "source_issues", lambda *a: []),
                (Q.O, "verify", lambda **kw: {"build_matches": True}),
                (layer2, "addr_of", lambda f: "80011000"),
                (layer2, "addr_dates", lambda a: []),
                (layer2, "_head", lambda: "fixture")):
            stack.enter_context(patch.object(obj, name, value))

        def record(verdict="PASS"):
            return layer2.record(func, verdict, "fixture-reviewer", "match",
                                 stem="gates", expect_hash=h)

        eq("header gates: TU alone balances", inlineasm.tu_unbalanced(src), False)
        check("header gates: record without header accepted", record()["ok"])
        eq("header gates: check without header accepted", layer2.gate(func, "gates"), None)
        check("header gates: queue done without header accepted", Q.mark_done(func)["ok"])
        before = layer2.record_path(func).read_bytes()
        header = Path("include/brace.h")
        header.write_text("#define CLOSE }\n", encoding="utf-8")
        facts = inlineasm.project_header_macro_facts(root)
        eq("header gates: project facts refuse TU", inlineasm.tu_unbalanced(src, facts), True)
        for label, result in (("queue done", Q.mark_done(func)),
                              ("record PASS", record())):
            check(f"header gates: {label} refused with TU and cause",
                  not result["ok"] and "src/gates.c" in result["reason"]
                  and "project header macro facts" in result["reason"])
        why = layer2.gate(func, "gates") or ""
        check("header gates: check refused with TU and cause",
              "src/gates.c" in why and "project header macro facts" in why)
        import sys
        from engine import cli
        for args in (("check", func, "--file", "gates"),
                     ("record", func, "--file", "gates", "--verdict", "PASS",
                      "--reviewer", "fixture-reviewer", "--scope", "match",
                      "--expect-hash", h)):
            out = io.StringIO()
            with patch.object(sys, "argv", ["engine.cli", "layer2", *args]), \
                    contextlib.redirect_stdout(out):
                rc = cli.main()
            check(f"header gates: CLI {args[0]} refuses",
                  rc == 1 and "src/gates.c" in out.getvalue()
                  and "project header macro facts" in out.getvalue())
        eq("header gates: refused records write nothing", layer2.record_path(func).read_bytes(), before)
        eq("header gates: body key stays TU-only", layer2.body_key(src, func)[1], h)
        eq("header gates: integrity fixture same refusal with shared facts",
           CCI._helper_issues(src, "gates", {func: 0}, facts),
           CCI._helper_issues(src, "gates", {func: 0}, inlineasm.tu_macro_facts(header.read_text())))
        for verdict in ("NEEDS_USER", "FAIL"):
            lines = len(layer2.read_records(func))
            check(f"header gates: unsafe TU accepts {verdict}", record(verdict)["ok"])
            recs = layer2.read_records(func)
            check(f"header gates: unsafe TU appends {verdict}",
                  len(recs) == lines + 1 and recs[-1]["verdict"] == verdict)
            check(f"header gates: unsafe TU gate stays closed after {verdict}",
                  "project header macro facts" in (layer2.gate(func, "gates") or ""))
        header.unlink()
        check("header gates: reverted header cannot revive PASS after FAIL",
              "latest layer-2 verdict for func_80011000 is FAIL"
              in (layer2.gate(func, "gates") or ""))
        check("header gates: queue stays closed after header reversion",
              not Q.mark_done(func)["ok"])
        check("header gates: record after header removal accepted", record()["ok"])
        eq("header gates: check after header removal accepted", layer2.gate(func, "gates"), None)
        check("header gates: queue after header removal accepted", Q.mark_done(func)["ok"])


def test_live_tu_refusal_root() -> None:
    """Live checks anchor both TU and headers independently of the cwd."""
    from unittest.mock import patch
    from engine import gtemacro

    cwd = Path.cwd()
    with tempfile.TemporaryDirectory() as td, contextlib.ExitStack() as stack:
        root = Path(td).resolve()
        stack.enter_context(patch.object(gtemacro, "REPO_ROOT", root))
        (root / "src/main").mkdir(parents=True)
        (root / "include").mkdir()
        other = root / "elsewhere"
        (other / "src/main").mkdir(parents=True)
        # A cwd-relative implementation would read this clean decoy instead.
        (other / "src/main/rooted.c").write_text("int decoy;\n", encoding="utf-8")
        tu = root / "src/main/rooted.c"
        tu.write_text("CLOSE;\nint rooted(void) { return 0; }\n", encoding="utf-8")
        os.chdir(other)
        stack.callback(os.chdir, cwd)
        eq("live root: safe TU outside cwd accepted",
           inlineasm.live_tu_refusal("main/rooted"), None)
        header = root / "include/brace.h"
        header.write_text("#define CLOSE }\n", encoding="utf-8")
        check("live root: header under root refuses TU outside cwd",
              "project header macro facts" in
              (inlineasm.live_tu_refusal("main/rooted") or ""))
        header.unlink()
        tu.write_text("int rooted(void) {\n", encoding="utf-8")
        check("live root: unsafe TU under root seen outside cwd",
              "unsafe file-scope brace walk" in
              (inlineasm.live_tu_refusal("main/rooted") or ""))
        tu.unlink()
        eq("live root: missing TU refuses despite cwd decoy",
           inlineasm.live_tu_refusal("main/rooted"),
           "src/main/rooted.c: missing; cannot check the header-aware brace walk")
        eq("live root: empty stem remains unchecked", inlineasm.live_tu_refusal(""), None)


def test_project_header_facts_cache() -> None:
    from unittest.mock import patch
    import check_completion_integrity as CCI

    with tempfile.TemporaryDirectory() as td:
        root = Path(td).resolve()
        header = root / "include/brace.h"
        header.parent.mkdir()
        header.write_text("#define CLOSE }\n", encoding="utf-8")
        reads = []
        original = Path.read_bytes

        def read(p, *args, **kwargs):
            reads.append(p)
            return original(p, *args, **kwargs)

        with patch.object(Path, "read_bytes", read):
            a = inlineasm.project_header_macro_facts(root)
            b = inlineasm.project_header_macro_facts(root)
            eq("header cache: facts same object for same content", a is b, True)
            eq("header cache: audit shares gathering", CCI._header_macro_facts(root), a)
            eq("header cache: safety fixture unchanged", CCI._header_safety_issues(root), [])
            eq("header cache: each gathering validates bytes", reads, [header] * 4)
            snapshot = inlineasm.project_header_texts(root)
            with patch.object(inlineasm, "project_header_texts",
                              side_effect=AssertionError("snapshot must be reused")):
                eq("header cache: public snapshot API reuses gathering",
                   inlineasm.project_header_macro_facts(root, headers=snapshot), a)
            eq("header cache: snapshot consumers do not read again", reads, [header] * 5)
            stamp = header.stat()
            header.write_text("#define CLOSE 0\n", encoding="utf-8")
            original_stat = Path.stat

            def same_stamp(p, *args, **kwargs):
                return stamp if p == header else original_stat(p, *args, **kwargs)

            with patch.object(Path, "stat", same_stamp):
                changed = inlineasm.project_header_texts(root)
                eq("header cache: identical stamp and size sees new text",
                   changed, ((header, "#define CLOSE 0\n"),))
                c = inlineasm.project_header_macro_facts(root, headers=changed)
                check("header cache: identical stamp and size invalidates facts", c != a)
            eq("header cache: changed header read once", reads, [header] * 6)
            eq("header cache: changed content cached", inlineasm.project_header_macro_facts(root), c)
            eq("header cache: cached content still validated", reads, [header] * 7)
            header.write_bytes(b"// bad \xff\r\n// CR\r// LF\n")
            eq("header cache: replacement decoding and universal newlines",
               inlineasm.project_header_texts(root),
               ((header, "// bad \ufffd\n// CR\n// LF\n"),))


def test_layer2_gate() -> None:
    """Owner ruling Q39 (2026-09-29): `queue done` refuses unless the latest
    layer-2 verdict recorded in memory/grind/<func>/layer2.jsonl is a PASS on
    the EXACT body being landed — and regen's drop of an item it previously
    listed is the same completion, so it is gated the same way.

    The retro-audit behind the ruling found landings with no recorded PASS and
    PASSes given on an earlier body than the one landed. Pinned: a record
    binds only to the hash the reviewer reported (--expect-hash, required);
    a comment/layout edit keeps the key and every token change moves it; no
    record, a later FAIL, or a changed body all refuse; the refusal names the
    fix; and neither mark_done nor any regen drop point has a way around it.
    """
    from engine import gtemacro, layer2
    from unittest.mock import patch

    src = ("/* header */\nint other(void) { return 1; }\n"
           "int func_L2(int a) {\n    /* why */\n    return a + 1;\n}\n")
    commented = src.replace("/* why */", "/* a different, longer comment */")
    changed = src.replace("a + 1", "a + 2")

    # 1. The key: the full definition as C tokens (comments/layout ignored).
    eq("layer2: C body keys as c", (layer2.body_key(src, "func_L2") or ("",))[0], "c")
    eq("layer2: comment-only edit keeps the hash",
       layer2.body_key(commented, "func_L2"), layer2.body_key(src, "func_L2"))
    eq("layer2: whitespace-only edit keeps the hash",
       layer2.body_key(src.replace("a + 1", "a+1"), "func_L2"),
       layer2.body_key(src, "func_L2"))
    check("layer2: a code change moves the hash",
          layer2.body_key(changed, "func_L2") != layer2.body_key(src, "func_L2"))
    check("layer2: a sibling function's edit does not move the hash",
          layer2.body_key(src.replace("return 1;", "return 7;"), "func_L2")
          == layer2.body_key(src, "func_L2"))
    eq("layer2: no body -> no key (fail closed)",
       layer2.body_key("int x;\n", "func_L2"), None)
    knr = "void func_KR(a) s16 *a; {\n    *a = 1;\n}\nint other(void) { return 1; }\n"
    check("layer2: a K&R definition is keyed by itself, not the whole file",
          layer2.body_key(knr, "func_KR")
          == layer2.body_key(knr.replace("return 1;", "return 7;"), "func_KR"))
    check("layer2: a K&R definition's parameter declaration is in the key",
          layer2.body_key(knr, "func_KR")
          != layer2.body_key(knr.replace("s16 *a;", "s32 *a;"), "func_KR"))
    # Collisions the first key (grindlib.body_hash) had — layer-2 review of
    # 5d46a66e1. Each pair differs in bytes-relevant C and must key apart.
    for desc, a_src, b_src in (
            ("return type", "s16 func_C(void) { return 0; }\n",
             "s32 func_C(void) { return 0; }\n"),
            ("storage class", "static int func_C(void) { return 0; }\n",
             "int func_C(void) { return 0; }\n"),
            ("whitespace inside a string",
             'void func_C(void) { g("a  b"); }\n', 'void func_C(void) { g("a b"); }\n'),
            ("`//` inside a string",
             'void func_C(void) { g("x//y"); h(1); }\n',
             'void func_C(void) { g("x//y"); h(2); }\n'),
            ("`/*` inside a char/string",
             "void func_C(void) { g('/'); g(\"/*\"); h(1); }\n",
             "void func_C(void) { g('/'); g(\"/*\"); h(2); }\n"),
            ("token boundary `a - --b` vs `a-- - b`",
             "int func_C(int a, int b) { return a - --b; }\n",
             "int func_C(int a, int b) { return a-- - b; }\n")):
        ka, kb = layer2.body_key(a_src, "func_C"), layer2.body_key(b_src, "func_C")
        check(f"layer2: no collision on {desc}", ka is not None and ka != kb)
    eq("layer2: layout-only edit (incl. a brace-shared line) keeps the key",
       layer2.body_key("}int func_C(int a){return a+1;}\n", "func_C"),
       layer2.body_key("}\nint func_C(int a)\n{\n    return a + 1; // x\n}\n", "func_C"))
    # Preprocessor lines are newline-terminated (round-2 review C5): the line
    # break after a directive is a token; a backslash-newline splice is not.
    check("layer2: a directive's terminating newline is a token",
          layer2.tokens("#define A 1\nx") != layer2.tokens("#define A 1 x"))
    eq("layer2: a spliced directive continues the same line",
       layer2.tokens("#define A \\\n 1\nx"), layer2.tokens("#define A 1\nx"))
    check("layer2: an in-body directive's line break moves the key",
          layer2.body_key("int func_C(void) {\n#if X\n    a();\n#endif\n}\n", "func_C")
          != layer2.body_key("int func_C(void) {\n#if X a();\n#endif\n}\n", "func_C"))
    eq("layer2: two definitions (an `#if 0` copy) -> no key (fail closed)",
       layer2.body_key("#if 0\nint func_D(void) { return 1; }\n#endif\n"
                       "int func_D(void) { return 2; }\n", "func_D"), None)
    eq("layer2: two live definitions -> no key (fail closed)",
       layer2.body_key("int func_D(void) { return 1; }\n"
                       "int func_D(void) { return 2; }\n", "func_D"), None)
    eq("layer2: a C definition AND an asm body -> no key (fail closed)",
       layer2.body_key('int func_D(void) { return 1; }\nINCLUDE_ASM("asm/funcs", func_D);\n',
                       "func_D"), None)

    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td, \
            patch.object(gtemacro, "REPO_ROOT", Path(td).resolve()):
        os.chdir(td)
        saved = (Q.QUEUE_PATH, Q._rule_count, cheats.func_prologue_count,
                 cheats.canonical_asm_funcs, cheats.maspsx_gate_entries,
                 inlineasm.file_func_cheat_asm_count, completion.source_issues,
                 Q.O.verify)
        try:
            Path("src").mkdir()
            Path("src/l2tu.c").write_text(src)
            rec_p = Path("memory/grind/func_L2/layer2.jsonl")
            # a record carries the function's address, from its glabel file
            Path("asm/funcs").mkdir(parents=True)
            Path("asm/funcs/func_L2.s").write_text(
                "glabel func_L2\n    /* 1000 80011000 27BDFFE8 */  addiu $sp, $sp, -0x18\n")
            eq("layer2: a function's address comes from its glabel file",
               layer2.addr_of("func_L2"), "80011000")

            # 2. whole-body asm is keyed by its block AND the included .s
            Path("asm/funcs").mkdir(parents=True, exist_ok=True)
            Path("asm/funcs/func_AS.s").write_text("glabel func_AS\n  jr $ra\n  nop\n")
            asm_src = 'INCLUDE_ASM("asm/funcs", func_AS);\n'
            k1 = layer2.body_key(asm_src, "func_AS")
            eq("layer2: INCLUDE_ASM body keys as asm", (k1 or ("", ""))[0], "asm")
            Path("asm/funcs/func_AS.s").write_text("glabel func_AS\n  jr $ra\n  li $v0, 1\n")
            check("layer2: editing the included .s moves the asm key",
                  layer2.body_key(asm_src, "func_AS") != k1)

            # 3. the gate itself
            why = layer2.gate("func_L2", "l2tu")
            check("layer2: no record -> refused", bool(why))
            check("layer2: refusal names the fix",
                  "layer2 record func_L2" in (why or "") and "Q39" in (why or ""))

            def _rec(verdict, reviewer="rev-A", scope="match"):
                """Record `verdict` for the body now in src/ — the hash the
                reviewer would have reported via `layer2 hash`."""
                return layer2.record("func_L2", verdict, reviewer, scope, stem="l2tu",
                                     expect_hash=layer2.current_key("func_L2", "l2tu")[1])

            r = layer2.record("func_L2", "PASS", "rev-A", "match", "ok", stem="l2tu",
                              expect_hash="0" * 16)
            check("layer2: record refuses a body other than the reviewed one",
                  r["ok"] is False and not rec_p.exists())
            for scope in layer2.SCOPES:
                r = layer2.record("func_L2", "PASS", "rev-A", scope, stem="l2tu")
                check(f"layer2: record refuses without --expect-hash (scope {scope})",
                      r["ok"] is False and "--expect-hash is required" in r.get("reason", "")
                      and not rec_p.exists())
            r = layer2.record("func_L2", "PASS", "rev-A", "bogus", stem="l2tu",
                              expect_hash=layer2.current_key("func_L2", "l2tu")[1])
            check("layer2: record refuses an unknown scope", r["ok"] is False)

            h = layer2.body_key(src, "func_L2")[1]
            r = layer2.record("func_L2", "PASS", "rev-A", "match", "ok", stem="l2tu",
                              expect_hash=h)
            check("layer2: PASS recorded", r["ok"] is True and rec_p.exists())
            rec = json.loads(rec_p.read_text().splitlines()[-1])
            eq("layer2: record fields",
               sorted(rec), sorted(["func", "addr", "verdict", "body_hash", "body_kind", "file",
                                    "reviewer", "scope", "date", "head", "notes"]))
            eq("layer2: the record carries the function's address", rec["addr"], "80011000")
            eq("layer2: PASS on the same body -> gate open",
               layer2.gate("func_L2", "l2tu"), None)

            Path("src/l2tu.c").write_text(commented)
            eq("layer2: comment-only edit after review keeps the PASS",
               layer2.gate("func_L2", "l2tu"), None)

            Path("src/l2tu.c").write_text(changed)
            why = layer2.gate("func_L2", "l2tu") or ""
            check("layer2: PASS on a different body hash -> refused",
                  "changed after review" in why and h in why)

            # The F1 scenario: PASS given on body A, body B edited in, then a
            # PASS "recorded" with A's hash — refused, nothing written.
            n_before = len(rec_p.read_text().splitlines())
            r = layer2.record("func_L2", "PASS", "rev-A", "match", stem="l2tu", expect_hash=h)
            check("layer2: a PASS on body A cannot be recorded against body B",
                  r["ok"] is False and len(rec_p.read_text().splitlines()) == n_before)

            # C1: a FAIL binds to the reviewer's hash even though src/ moved on
            # (it can only close the gate) — so the stale PASS on body A cannot
            # come back to life when src/ returns to A.
            r = layer2.record("func_L2", "FAIL", "rev-A2", "match", stem="l2tu", expect_hash=h)
            check("layer2: a FAIL on body A records while src/ holds body B",
                  r["ok"] is True and r["body_hash"] == h and r["body_kind"] == "")
            Path("src/l2tu.c").write_text(src)
            check("layer2: src/ back at body A -> the stale PASS stays revoked",
                  "latest layer-2 verdict for func_L2 is FAIL"
                  in (layer2.gate("func_L2", "l2tu") or ""))
            r = layer2.record("func_L2", "NEEDS_USER", "rev-A3", "match", stem="l2tu",
                              expect_hash="f" * 16)
            check("layer2: a NEEDS_USER binds to a hash src/ does not hold", r["ok"] is True)
            r = layer2.record("func_L2", "FAIL", "rev-A4", "match", stem="l2tu",
                              expect_hash="not-a-hash")
            check("layer2: a malformed hash is refused for every verdict", r["ok"] is False)

            _rec("FAIL", "rev-B")
            why = layer2.gate("func_L2", "l2tu") or ""
            check("layer2: a later FAIL on the same body revokes the PASS",
                  "latest layer-2 verdict for func_L2 is FAIL" in why)
            _rec("NEEDS_USER", "rev-C")
            check("layer2: NEEDS_USER is not a PASS",
                  "is NEEDS_USER" in (layer2.gate("func_L2", "l2tu") or ""))

            _rec("PASS", "rev-D")
            eq("layer2: a fresh PASS on the current body reopens the gate",
               layer2.gate("func_L2", "l2tu"), None)
            with open(rec_p, "a") as fh:
                fh.write("not json\n")
            check("layer2: a malformed record fails closed",
                  "not JSON" in (layer2.gate("func_L2", "l2tu") or ""))
            rec_p.unlink()

            # R2: the CLI surface — `layer2 record` via engine.cli.main(argv).
            import sys
            from engine import cli as _cli

            def _cli_run(*argv):
                old_argv, buf = sys.argv, io.StringIO()
                sys.argv = ["engine.cli", "layer2", *argv]
                try:
                    with contextlib.redirect_stdout(buf):
                        rc = _cli.main()
                finally:
                    sys.argv = old_argv
                return rc, buf.getvalue()

            cur = layer2.current_key("func_L2", "l2tu")[1]
            base = ("record", "func_L2", "--reviewer", "rev-cli", "--scope", "match",
                    "--file", "l2tu")
            rc, _out = _cli_run(*base, "--verdict", "PASS")
            check("layer2 cli: record without --expect-hash -> exit 1, nothing written",
                  rc == 1 and not rec_p.exists())
            rc, _out = _cli_run(*base, "--verdict", "PASS", "--expect-hash", "0" * 16)
            check("layer2 cli: record with a wrong hash -> exit 1, nothing written",
                  rc == 1 and not rec_p.exists())
            rc, _out = _cli_run(*base, "--verdict", "PASS", "--expect-hash", cur)
            check("layer2 cli: record with the reviewed hash -> exit 0, written",
                  rc == 0 and rec_p.exists()
                  and json.loads(rec_p.read_text().splitlines()[-1])["body_hash"] == cur)
            rc, out = _cli_run("hash", "func_L2", "--file", "l2tu")
            check("layer2 cli: hash prints the key", rc == 0 and cur in out)
            rc, out = _cli_run("show", "func_L2", "--file", "l2tu")
            check("layer2 cli: show prints the keyed definition",
                  rc == 0 and "int func_L2(int a)" in out and "return a + 1;" in out)
            # A duplicate `static` name in two TUs (get_alarm: libcd/bios.c and
            # libgpu/sys.c) must not resolve to whichever file the scan meets first.
            Path("src/l2dup_a.c").write_text("static int func_DUP(int a) { return a; }\n")
            Path("src/l2dup_b.c").write_text("static int func_DUP(int a) { return a * 3; }\n")
            try:
                layer2.locate_stem("func_DUP")
                amb = None
            except layer2.AmbiguousStem as e:
                amb = e.stems
            eq("layer2: a name defined in two TUs -> AmbiguousStem with both stems",
               amb, ["l2dup_a", "l2dup_b"])
            rc, out = _cli_run("hash", "func_DUP")
            check("layer2 cli: hash on an ambiguous name -> exit 1, candidates listed",
                  rc == 1 and "l2dup_a" in out and "l2dup_b" in out and "body_hash" not in out)
            rc, out = _cli_run("hash", "func_DUP", "--file", "l2dup_b")
            check("layer2 cli: --file disambiguates",
                  rc == 0 and json.loads(out)["file"] == "l2dup_b"
                  and json.loads(out)["body_hash"] == layer2.current_key("func_DUP", "l2dup_b")[1])
            r = layer2.record("func_DUP", "FAIL", "rev-dup", "match", expect_hash="0" * 16)
            check("layer2: record on an ambiguous name without a stem -> refused",
                  r["ok"] is False and r.get("candidates") == ["l2dup_a", "l2dup_b"]
                  and not layer2.record_path("func_DUP").exists())
            # A TU whose own body is ambiguous (a real definition plus an `#if 0`
            # copy) is still a candidate, not silently dropped.
            Path("src/l2dup_b.c").write_text(
                "static int func_DUP(int a) { return a * 3; }\n"
                "#if 0\nstatic int func_DUP(int a) { return a * 4; }\n#endif\n")
            eq("layer2: a TU with an #if 0 second copy still counts as a candidate",
               layer2.locate_stems("func_DUP"), ["l2dup_a", "l2dup_b"])
            Path("src/l2dup_a.c").unlink()
            Path("src/l2dup_b.c").unlink()
            rec_p.unlink()
            vf = Path("verdict.json")
            vf.write_text("reviewer chatter\n" + json.dumps(
                {"decision": "PASS", "function": "func_L2", "body_hash": cur,
                 "summary": "clean", "evidence": [], "next_action": ""}) + "\n")
            rc, _out = _cli_run(*base, "--verdict-file", str(vf), "--expect-hash", "0" * 16)
            check("layer2 cli: --expect-hash disagreeing with the verdict file -> exit 1",
                  rc == 1 and not rec_p.exists())
            rc, _out = _cli_run("record", "func_OTHER", "--reviewer", "r", "--scope", "match",
                                "--file", "l2tu", "--verdict-file", str(vf))
            check("layer2 cli: a verdict file for another function -> exit 1", rc == 1)
            rc, _out = _cli_run(*base, "--verdict-file", str(vf))
            rec = json.loads(rec_p.read_text().splitlines()[-1]) if rec_p.exists() else {}
            check("layer2 cli: --verdict-file records verdict + hash + its sha1",
                  rc == 0 and rec.get("verdict") == "PASS" and rec.get("body_hash") == cur
                  and rec.get("notes") == "clean" and rec.get("verdict_source") == "verdict.json"
                  and len(rec.get("verdict_sha1", "")) == 40)
            # K5: the verdict JSON is kept, byte-for-byte, where the record points
            import hashlib as _hl
            kept = Path(rec.get("verdict_file", "") or "/nonexistent")
            check("layer2 cli: --verdict-file keeps a byte copy under layer2_verdicts/",
                  kept.as_posix() == f"memory/grind/func_L2/layer2_verdicts/{rec.get('verdict_sha1')}.json"
                  and kept.is_file()
                  and _hl.sha1(kept.read_bytes()).hexdigest() == rec.get("verdict_sha1"))
            vf.unlink()
            check("layer2 cli: the kept copy outlives the reviewer's tmp file",
                  kept.is_file() and not vf.exists())
            vf.write_text("{}")
            vf.write_text(json.dumps({"decision": "PASS", "function": "func_L2"}))
            rc, _out = _cli_run(*base, "--verdict-file", str(vf))
            check("layer2 cli: a verdict file without body_hash -> exit 1", rc == 1)
            rec_p.unlink()

            # 4. mark_done: refuses on the gate BEFORE the (minutes-long) oracle
            #    check, and completes once the PASS matches.
            qp = Path("queue.json")
            seed = {"items": [{"func": "func_L2", "file": "l2tu", "distance": 0,
                               "verdict": "C", "rules": 0, "status": "active"}],
                    "counts": {}}
            qp.write_text(json.dumps(seed))
            verified = []
            Q.QUEUE_PATH = str(qp)
            Q._rule_count = lambda f: 0
            cheats.func_prologue_count = lambda f: 0
            cheats.canonical_asm_funcs = lambda: set()
            cheats.maspsx_gate_entries = lambda f: []
            inlineasm.file_func_cheat_asm_count = lambda s, f: 0
            completion.source_issues = lambda *a, **k: []
            Q.O.verify = lambda rebuild=False: (verified.append(1) or
                                                {"build_matches": True,
                                                 "build_sha1": "deadbeef"})
            r = Q.mark_done("func_L2")
            check("layer2: queue done REFUSES with no record",
                  r["ok"] is False and "layer-2 gate" in r.get("reason", ""))
            eq("layer2: refusal comes before the oracle check", verified, [])
            eq("layer2: refused item stays queued",
               [it["func"] for it in Q.load()["items"]], ["func_L2"])
            _rec("PASS")
            Path("src/l2tu.c").write_text(changed)
            r = Q.mark_done("func_L2")
            check("layer2: queue done REFUSES a PASS on an earlier body",
                  r["ok"] is False and "changed after review" in r.get("reason", ""))
            Path("src/l2tu.c").write_text(commented)
            r = Q.mark_done("func_L2")
            check("layer2: queue done ACCEPTS a PASS on the same body (comments differ)",
                  r["ok"] is True and r.get("completion") == "COMPLETED-C")
            rec_p.unlink()

            # 5. regen: dropping an item it LISTED is a completion — gated at
            #    each of the three drop points (COMPLETED-C, canonical, and the
            #    unscorable branch), with or without --no-preserve. `other` is a
            #    never-listed completion and always drops.
            Path("build/src").mkdir(parents=True)
            Path("build/src/l2tu.o").write_text("")
            saved_regen = (P.c_stems, canonical.scan_all, cheats.is_jtbl_infra,
                           cheats.is_canonical_extraction_only,
                           Q.sandbox.build_stripped_object, score._o_func_table,
                           score.score_func)
            P.c_stems = lambda: ["l2tu"]
            canonical.scan_all = lambda: []
            cheats.is_jtbl_infra = lambda f: False
            cheats.is_canonical_extraction_only = lambda f: False
            Q.sandbox.build_stripped_object = lambda *a, **k: {}
            score._o_func_table = lambda o: {"func_L2": (0, 0), "other": (0, 0)}

            def _unscorable(a, b, f):
                raise KeyError(f)

            try:
                for point, canon, scorer in (
                        ("COMPLETED-C", set(), lambda a, b, f: {"score": 0}),
                        # distance 300: only the canonical branch can drop it
                        ("canonical", {"func_L2"},
                         lambda a, b, f: {"score": 300 if f == "func_L2" else 0}),
                        ("unscorable", set(), _unscorable)):
                    cheats.canonical_asm_funcs = lambda canon=canon: canon
                    score.score_func = scorer
                    for preserve in (True, False):
                        qp.write_text(json.dumps(seed))
                        items = Q.generate(workdir=str(Path(td) / "wd"),
                                           preserve=preserve)["items"]
                        eq(f"layer2: regen [{point}, preserve={preserve}] HOLDS a listed "
                           f"item without a PASS", [it["func"] for it in items], ["func_L2"])
                        check(f"layer2: held item says why [{point}, preserve={preserve}]",
                              bool(items) and "layer-2 gate"
                              in items[0].get("layer2_pending", ""))
                    _rec("PASS")
                    qp.write_text(json.dumps(seed))
                    eq(f"layer2: regen [{point}] drops a listed item once its PASS matches",
                       Q.generate(workdir=str(Path(td) / "wd"))["items"], [])
                    rec_p.unlink()

                # R1: the scan can skip a whole stem — no reference .o, or its
                # stripped build raises — and never reach any drop point. The
                # choke point after the scan must hold the listed item, and the
                # SECOND regen (whose `listed` is the first one's output) too.
                cheats.canonical_asm_funcs = lambda: set()
                score.score_func = lambda a, b, f: {"score": 0}

                def _build_raises(*a, **k):
                    raise RuntimeError("stripped build broke")

                ref_o = Path("build/src/l2tu.o")
                for point in ("missing ref .o", "stripped-build exception"):
                    if point == "missing ref .o":
                        ref_o.unlink()
                    else:
                        Q.sandbox.build_stripped_object = _build_raises
                    qp.write_text(json.dumps(seed))
                    for n in (1, 2):
                        items = Q.generate(workdir=str(Path(td) / "wd"))["items"]
                        eq(f"layer2: regen #{n} [{point}] HOLDS a listed item without a PASS",
                           [it["func"] for it in items], ["func_L2"])
                        check(f"layer2: held item says why [{point}, regen #{n}]",
                              bool(items) and "layer-2 gate"
                              in items[0].get("layer2_pending", ""))
                    _rec("PASS")
                    eq(f"layer2: [{point}] a matching PASS lets the item go",
                       Q.generate(workdir=str(Path(td) / "wd"))["items"], [])
                    rec_p.unlink()
                    ref_o.write_text("")
                    Q.sandbox.build_stripped_object = lambda *a, **k: {}

                # K1: a LISTED item that turns out not to be a C function (its
                # name is an `alabel` of an asm body the .c includes) is held
                # too — both the scored and the unscorable not-a-C drop go
                # through _held. A never-listed non-C symbol still drops silently.
                score._o_func_table = lambda o: {"func_NC": (0, 0), "func_NC2": (0, 0)}
                l2_src = Path("src/l2tu.c").read_text()
                Path("src/l2tu.c").write_text(l2_src + '\nINCLUDE_ASM("asm/funcs", nc_body);\n')
                inlineasm._FILE_TEXT_CACHE.pop("l2tu", None)
                Path("asm/funcs").mkdir(parents=True, exist_ok=True)
                Path("asm/funcs/nc_body.s").write_text(
                    "glabel nc_body\n  jr $ra\n  nop\nalabel func_NC\nalabel func_NC2\n")
                inlineasm.file_func_cheat_asm_count = (
                    lambda s, f: -1 if f.startswith("func_NC") else 0)
                nc_seed = {"items": [{"func": "func_NC", "file": "l2tu", "distance": 0,
                                      "verdict": "C", "rules": 0, "status": "active"}],
                           "counts": {}}
                for point, scorer in (("scored", lambda a, b, f: {"score": 0}),
                                      ("unscorable", _unscorable)):
                    score.score_func = scorer
                    qp.write_text(json.dumps(nc_seed))
                    items = Q.generate(workdir=str(Path(td) / "wd"))["items"]
                    eq(f"layer2: regen [not-a-C, {point}] HOLDS a listed item, drops the "
                       f"never-listed one", [it["func"] for it in items], ["func_NC"])
                    check(f"layer2: held not-a-C item says why [{point}]",
                          bool(items) and "layer-2 gate" in items[0].get("layer2_pending", ""))
                inlineasm.file_func_cheat_asm_count = lambda s, f: 0
                Path("src/l2tu.c").write_text(l2_src)
                inlineasm._FILE_TEXT_CACHE.pop("l2tu", None)

                # every item regen writes carries its function's address (the
                # departures audit's key) — here from asm/funcs/func_L2.s
                score._o_func_table = lambda o: {"func_L2": (0, 0)}
                score.score_func = lambda a, b, f: {"score": 5}
                qp.write_text(json.dumps(seed))
                items = Q.generate(workdir=str(Path(td) / "wd"))["items"]
                eq("layer2: regen writes each item's addr",
                   [(it["func"], it.get("addr")) for it in items], [("func_L2", "80011000")])
                qp.write_text(json.dumps({"items": [dict(seed["items"][0], addr="8001100")]}))
                bad = None
                try:
                    Q.load()
                except ValueError as e:
                    bad = str(e)
                check("queue: load() refuses an addr that is not 8 hex digits",
                      bad is not None and "8001100" in bad)
            finally:
                (P.c_stems, canonical.scan_all, cheats.is_jtbl_infra,
                 cheats.is_canonical_extraction_only,
                 Q.sandbox.build_stripped_object, score._o_func_table,
                 score.score_func) = saved_regen

            # N1: a naming wave moves the ledger (and the archived one) and must
            # retarget layer2.jsonl's `func`, or the renamed function could
            # never be recorded again. The old PASS must not open the gate.
            # Round 4: the rename chain is kept as a LIST, the layer2_verdicts/
            # pointer follows the move, and a rename that cannot run is
            # refused BEFORE anything is written.
            import naming_wave as nw

            def _wave(code_map):
                saved_root = nw.ROOT
                nw.ROOT = Path.cwd()
                try:
                    plan = nw.Plan()
                    nw.plan_ledger_renames(plan, code_map)
                    nw.apply_plan(plan)
                finally:
                    nw.ROOT = saved_root

            Path("src/nw.c").write_text("int func_OLD(int a) { return a + 1; }\n")
            Path("asm/funcs/func_OLD.s").write_text(
                "glabel func_OLD\n    /* 2000 80012000 27BDFFE8 */  addiu $sp, $sp, -0x18\n")
            old_h = layer2.current_key("func_OLD", "nw")[1]
            vf = Path("nw_verdict.json")
            vf.write_text(json.dumps({"decision": "PASS", "function": "func_OLD",
                                      "body_hash": old_h, "summary": "ok"}))
            r = layer2.record_from_verdict_file("func_OLD", str(vf), "rev", "match", stem="nw")
            done_p = layer2.completed_record_path("func_OLD")
            done_p.parent.mkdir(parents=True)
            done_p.write_text(layer2.record_path("func_OLD").read_text())
            _wave({"func_OLD": "func_NEW"})
            Path("src/nw.c").write_text("int func_NEW(int a) { return a + 1; }\n")
            Path("asm/funcs/func_OLD.s").rename("asm/funcs/func_NEW.s")
            Path("asm/funcs/func_NEW.s").write_text(
                Path("asm/funcs/func_NEW.s").read_text().replace("func_OLD", "func_NEW"))
            try:
                recs = layer2.read_records("func_NEW")
                moved = layer2.read_records("func_NEW", layer2.completed_record_path("func_NEW"))
            except ValueError as e:
                recs, moved = [], [{"error": str(e)}]
            check("layer2: naming wave retargets the moved record's func",
                  r["ok"] and len(recs) == 1 and recs[0]["func"] == "func_NEW"
                  and recs[0].get("renamed_from") == ["func_OLD"]
                  and not layer2.record_path("func_OLD").exists())
            check("layer2: naming wave retargets the archived (_completed) record too",
                  len(moved) == 1 and moved[0].get("func") == "func_NEW")
            vptr = recs[0].get("verdict_file", "") if recs else ""
            check("layer2: naming wave re-points verdict_file at the moved copy",
                  vptr.startswith("memory/grind/func_NEW/layer2_verdicts/")
                  and Path(vptr).is_file())
            check("layer2: the pre-rename PASS does not open the gate for the new name",
                  "changed after review" in (layer2.gate("func_NEW", "nw") or ""))
            new_h = layer2.current_key("func_NEW", "nw")[1]
            r = layer2.record("func_NEW", "PASS", "rev", "match", stem="nw", expect_hash=new_h)
            check("layer2: the renamed function is recordable after the wave",
                  r["ok"] is True and layer2.gate("func_NEW", "nw") is None)
            _wave({"func_NEW": "func_NEWER"})
            try:
                chain = [x.get("renamed_from") for x in layer2.read_records("func_NEWER")]
            except ValueError:
                chain = []
            eq("layer2: a second wave appends to the renamed_from chain",
               chain, [["func_OLD", "func_NEW"], ["func_NEW"]])

            # Preflight: func_P's ledger cannot move onto an existing func_Q/,
            # so the wave dies — and its layer2.jsonl retarget was NOT written.
            p_rec = Path("memory/grind/func_P/layer2.jsonl")
            p_rec.parent.mkdir(parents=True)
            p_line = json.dumps({"func": "func_P", "verdict": "PASS",
                                 "body_hash": "0" * 16}) + "\n"
            p_rec.write_text(p_line)
            Path("memory/grind/func_Q").mkdir(parents=True)
            died = False
            with contextlib.redirect_stderr(io.StringIO()):
                try:
                    _wave({"func_P": "func_Q"})
                except SystemExit:
                    died = True
            check("naming_wave: a rename onto an existing ledger dies in preflight",
                  died and p_rec.read_text() == p_line
                  and not Path("memory/grind/func_Q/layer2.jsonl").exists())
        finally:
            os.chdir(cwd)
            (Q.QUEUE_PATH, Q._rule_count, cheats.func_prologue_count,
             cheats.canonical_asm_funcs, cheats.maspsx_gate_entries,
             inlineasm.file_func_cheat_asm_count, completion.source_issues,
             Q.O.verify) = saved


def test_naming_keycheck_movovl_q130() -> None:
    """Owner ruling Q130: a movovl/ freshness hit is excused only for the same verbatim routine."""
    import sys
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
    import naming_keycheck as nk

    def tree(**over):
        t = {
            nk.LIBSCAN_SYMS: json.dumps({
                "0x80001010": [{"name": "zap", "kind": "static", "lib": "LIBX", "mod": "M",
                                "mod_start": "0x80001000", "mod_end": "0x80001020"}],
                "0x80002008": [{"name": "zap", "kind": "static", "lib": "LIBX", "mod": "M_VB",
                                "mod_start": "0x80002000", "mod_end": "0x80002010"}]}),
            nk.LIBSCAN_MATCHES: json.dumps({"results": [
                {"lib": "LIBX", "mod": "M", "words": 8, "status": "verbatim", "vaddrs": ["0x80001000"]},
                {"lib": "LIBX", "mod": "M_VB", "words": 4, "status": "verbatim", "vaddrs": ["0x80002000"]}]}),
            nk.MOVOVL_SCAN + "matches.json": json.dumps({"results": [
                {"lib": "LIBX", "mod": "M", "words": 8, "status": "verbatim", "vaddrs": ["0x801E0000"]},
                {"lib": "LIBX", "mod": "M_VB", "words": 4, "status": "verbatim", "vaddrs": ["0x801E0100"]}]}),
            nk.MOVOVL_SCAN + "placements.json": json.dumps({"modules": [
                ["0x801E0000", "0x801E0020", "LIBX", "M"], ["0x801E0100", "0x801E0110", "LIBX", "M_VB"]]}),
            nk.MOVOVL_SCAN + "symbols.txt": "# vaddr lib/module symbol(s)\n"
                "0x801E0010 LIBX/M                zap (static)\n"
                "0x801E0108 LIBX/M_VB             zap (static)\n",
            nk.MOVOVL_SYMADDRS: "// header: names collide across modules\n"
                "zap = 0x801E0010; // type:func\nzap_M_VB = 0x801E0108; // type:func\n",
            "movovl/asm/libx_m.s": "glabel other\n"
                "    /* 0 801E0000 0800E003 */  jal        zap\n"
                "nonmatching zap, 0x8\n\nglabel zap\n"
                "    /* 10 801E0010 0800E003 */  jr         $ra\n"
                "    /* 14 801E0014 00000000 */   nop\n"
                "endlabel zap\n",
            "movovl/asm/libx_other.s": "    /* 40 801E0040 0480070C */  jal        zap\n",
        }
        t.update(over)
        return lambda p: t.get(p)

    psx = ["src/main/psxsdk/libx/m.c"]
    ok = nk.movovl_same_routine(tree(), "zap", "80001010", psx)
    check("keycheck Q130: same verbatim routine admits its movovl lines",
          ok == {nk.MOVOVL_SYMADDRS: {2}, "movovl/asm/libx_m.s": {2, 3, 5, 8}})
    for path in (nk.MOVOVL_SYMADDRS, "movovl/asm/libx_m.s"):
        check("keycheck Q130: same-routine hit excused in " + path,
              nk.movovl_hit_excused(tree()(path), path, ["zap"], {"zap": ok}) is None)
    check("keycheck Q130: a comment naming it still blocks",
          nk.movovl_hit_excused("// zap here\n", nk.MOVOVL_SYMADDRS, ["zap"], {"zap": ok}) is not None)
    check("keycheck Q130: a reference outside the module's own asm still blocks",
          nk.movovl_hit_excused(tree()("movovl/asm/libx_other.s"), "movovl/asm/libx_other.s",
                                ["zap"], {"zap": ok}) is not None)
    # git grep -w words are ASCII: "ézap" is a hit there, so it must not be excused here.
    asm_m = tree()("movovl/asm/libx_m.s")
    check("keycheck Q130: a non-ASCII-prefixed name is still a hit and blocks",
          nk.movovl_hit_excused(asm_m + "    ézap\n", "movovl/asm/libx_m.s", ["zap"], {"zap": ok}) is not None)
    check("keycheck Q130: a grep hit file with no matching line blocks",
          nk.movovl_hit_excused("nothing here\n", nk.MOVOVL_SYMADDRS, ["zap"], {"zap": ok}) is not None)
    bad_sym = tree(**{nk.MOVOVL_SCAN + "symbols.txt": "0xZZZZ LIBX/M zap (static)\n"})
    try:
        why = nk.movovl_same_routine(bad_sym, "zap", "80001010", psx)
    except Exception as e:  # noqa: BLE001
        why = e
    check("keycheck Q130: a malformed symbols.txt line is a reason, not a crash (" + repr(why) + ")",
          isinstance(why, str))
    check("keycheck Q130: a movovl hit for an unevaluated name blocks",
          nk.movovl_hit_excused("zap = 0x801E0010;\n", nk.MOVOVL_SYMADDRS, ["zap"], {}) is not None)
    # A different routine with the same name: M_VB's zap is movovl's zap_M_VB, not its `zap`.
    why = nk.movovl_same_routine(tree(), "zap", "80002008", ["src/main/psxsdk/libx/m_vb.c"])
    check("keycheck Q130: different routine with the same name blocked (" + str(why) + ")",
          isinstance(why, str) and nk.movovl_hit_excused(
              tree()(nk.MOVOVL_SYMADDRS), nk.MOVOVL_SYMADDRS, ["zap"], {"zap": why}) is not None)
    shifted = tree(**{nk.MOVOVL_SCAN + "symbols.txt": "0x801E0014 LIBX/M zap (static)\n"})
    check("keycheck Q130: same module, different offset blocked",
          isinstance(nk.movovl_same_routine(shifted, "zap", "80001010", psx), str))
    resized = tree(**{nk.MOVOVL_SCAN + "matches.json": json.dumps({"results": [
        {"lib": "LIBX", "mod": "M", "words": 9, "status": "verbatim", "vaddrs": ["0x801E0000"]}]})})
    check("keycheck Q130: a differently sized movovl module blocked",
          isinstance(nk.movovl_same_routine(resized, "zap", "80001010", psx), str))
    # Not a libscan-verbatim src/main/psxsdk/ module.
    check("keycheck Q130: an old name outside src/main/psxsdk/ blocked",
          isinstance(nk.movovl_same_routine(tree(), "zap", "80001010", ["src/main/368E4.c"]), str))
    check("keycheck Q130: an address libscan does not name blocked",
          isinstance(nk.movovl_same_routine(tree(), "zap", "80001014", psx), str))
    near = tree(**{nk.LIBSCAN_MATCHES: json.dumps({"results": [
        {"lib": "LIBX", "mod": "M", "words": 8, "status": "near", "vaddrs": ["0x80001000"]}]})})
    check("keycheck Q130: a near-tier (not verbatim) main module blocked",
          isinstance(nk.movovl_same_routine(near, "zap", "80001010", psx), str))
    check("keycheck Q130: no base address blocked",
          isinstance(nk.movovl_same_routine(tree(), "zap", None, psx), str))


def test_naming_keycheck_scoped_pairs() -> None:
    """Pure-logic scoped proof regressions; no build/toolchain or git writes."""
    import sys
    import contextlib
    import io
    import re
    from unittest.mock import call, patch
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
    import naming_keycheck as nk

    def cpp_preamble(tu):
        # GCC 12.4 mipsel cpp's real implicit system-header preamble.
        return ('# 0 ' + json.dumps(tu) + '\n# 0 "<built-in>"\n# 0 "<command-line>"\n'
                '# 1 "/usr/mipsel-linux-gnu/include/stdc-predef.h" 1 3 4\n'
                '# 0 "<command-line>" 2\n# 1 ' + json.dumps(tu) + '\n')

    def fake_cpp(tree, tu):
        # Fixture-only marker output; real compilation eligibility is tested
        # separately with explicit emitted/omitted header markers below.
        active, output = set(), []
        def emit(path):
            path = nk.normpath(path)
            if path in active or path not in tree:
                return
            active.add(path)
            text = tree[path]
            for line, content in enumerate(text.splitlines(), 1):
                include = re.match(r'^\s*#\s*include\s*([<"])([^>"\n]+)[>"]', content)
                output.append((content if re.match(r'^\s*#\s*(?:define|undef)\b', content) else
                               '' if re.match(r'^\s*(?:#|%:)', content) else content) + '\n')
                if include:
                    quote, target = include.groups()
                    choices = ([Path(path).parent / target] if quote == '"' else []) + [Path("include") / target]
                    candidate = next((nk.normpath(p.as_posix()) for p in choices if nk.normpath(p.as_posix()) in tree), None)
                    if candidate and candidate not in active:
                        output.append('# 1 ' + json.dumps(candidate) + ' 1\n')
                        emit(candidate)
                        output.append('# ' + str(line + 1) + ' ' + json.dumps(path) + ' 2\n')
            active.remove(path)
        preamble = cpp_preamble(tu)
        definitions = nk.command_line_macros(tree.get("Makefile"))
        preamble = preamble.replace('# 0 "<command-line>"\n',
                                    '# 0 "<command-line>"\n' + (definitions + "\n" if definitions else ""), 1)
        output.append(preamble)
        emit(tu)
        return "".join(output)

    dependency_calls = []
    def fake_dependencies(tree, tu):
        dependency_calls.append((tree.label, tu))
        # This is an injected compiler fixture, not production include discovery.
        return nk.emitted_lines(fake_cpp(tree, tu), tree, tu).sources | {tu}

    def fake_cc1(tree, tu):
        return nk.subprocess.CompletedProcess([], 0, "\taddiu $2,$0,1\n", "")

    def proof(spec, base, new, compile_headers=True, cpp=fake_cpp, cc1=fake_cc1, deps=fake_dependencies):
        if compile_headers:
            base = dict(base)
            headers = [p for p in base if p.endswith(".h")]
            if headers:
                base["src/keycheck_header_probe.c"] = "".join('#include "../' + p + '"\n' for p in headers)
        pairs = nk.parse_pairs(spec)
        scoped = {o: n for o, n in pairs.items() if "." in o or ":" in o}
        globals = {o: n for o, n in pairs.items() if o not in scoped}
        try:
            edits = nk.scoped_plan(base, scoped, globals)
            nk.scoped_applied(base, edits, lambda p: new.get(p, base.get(p)))
            with patch.object(nk, "preprocess", cpp), patch.object(nk, "compile_cc1", cc1), patch.object(nk, "cpp_dependencies", deps):
                nk.verify_compiled_edits(base, {**base, **new}, edits, scoped=scoped, global_pairs=globals)
        except ValueError as e:
            return False, str(e)
        for path, text in new.items():
            if any("." in o for o in scoped) and not nk.aggregate_unchanged(
                    base[path], text, edits.get(path, {}), globals):
                return False, "aggregate definition changed"
            want = nk.scoped_text(base[path], edits.get(path, {}), text)
            if globals:
                pat = re.compile(r"\b(?:" + "|".join(map(re.escape, globals)) + r")\b")
                want = nk.nw.sub_c(pat, lambda m: globals[m[0]], want)[0]
            if not nk.scoped_comments(want, text, scoped)[0]:
                return False, "unexplained token change"
        return True, ""

    member = ("struct T { int m; }; struct U { int m; };\n"
              "int f(struct T *t, struct U *u) { return t->m + u->m; }\n"
              "struct T a = { .m = 1 }; /* m */\n")
    renamed = member.replace("struct T { int m;", "struct T { int fresh;")
    renamed = renamed.replace("t->m", "t->fresh").replace(".m =", ".fresh =")
    renamed = renamed.replace("/* m */", "/* fresh */")
    check("keycheck: member shared by another aggregate, optional accesses and comments",
          proof("T.m=fresh", {"src/a.c": member}, {"src/a.c": renamed})[0])
    check("keycheck: member access after dot", proof("T.m=fresh",
          {"src/a.c": member + "int h(struct T t) { return t.m; }"},
          {"src/a.c": renamed + "int h(struct T t) { return t.fresh; }"})[0])
    for label, extra in [("C identifier", "int fresh;"),
                         ("header identifier", "int fresh;"),
                         ("asm identifier", "glabel fresh")]:
        path = {"C identifier": "src/b.c", "header identifier": "include/b.h",
                "asm identifier": "asm/b.s"}[label]
        ok, why = proof("T.m=fresh", {"src/a.c": member, path: extra},
                        {"src/a.c": renamed})
        check("keycheck: unrelated member destination token allowed in " + label, ok)
    check("keycheck: member bare-name change rejected", not proof("T.m=fresh",
          {"src/a.c": member + "int m;"}, {"src/a.c": renamed + "int fresh;"})[0])

    # Nested members (T.u.m): m of the anonymous aggregate declared as member u of T.
    nested = ("typedef struct { int d; union { int pair; struct { short b; short a; } node; } ends; } E;\n"
              "struct N { int node; };\n"
              "int f(E *e, struct N *n) { return e->ends.node.a + e[1].ends.node.b + n->node; } /* node */\n")
    nested_new = (nested.replace("} node; } ends;", "} pt; } ends;")
                  .replace("ends.node.a", "ends.pt.a").replace("ends.node.b", "ends.pt.b")
                  .replace("/* node */", "/* pt */"))
    check("keycheck: nested union member T.u.m renamed (declaration, accesses, comment)",
          proof("E.ends.node=pt", {"src/a.c": nested}, {"src/a.c": nested_new})[0])
    check("keycheck: nested member accesses may keep the old name",
          proof("E.ends.node=pt", {"src/a.c": nested},
                {"src/a.c": nested.replace("} node; } ends;", "} pt; } ends;")
                 .replace("ends.node.a", "ends.pt.a")})[0])
    ok, why = proof("E.node=pt", {"src/a.c": nested}, {"src/a.c": nested_new})
    check("keycheck: flat T.m for a nested member rejected (" + why + ")",
          not ok and "nested" in why)
    ok, why = proof("E.missing.node=pt", {"src/a.c": nested},
                    {"src/a.c": nested_new})
    check("keycheck: nested path naming no inline aggregate rejected (" + why + ")",
          not ok and "nested path" in why)
    tagged = nested.replace("struct { short b;", "struct NS { short b;")
    ok, why = proof("E.ends.node=pt", {"src/a.c": tagged},
                    {"src/a.c": tagged.replace("} node; } ends;", "} pt; } ends;")})
    check("keycheck: nested member whose own type is a tagged inline struct (" + why + ")", ok)
    tagged_path = nested.replace("union { int pair;", "union U { int pair;")
    ok, why = proof("E.ends.node=pt", {"src/a.c": tagged_path},
                    {"src/a.c": tagged_path.replace("} node; } ends;", "} pt; } ends;")})
    check("keycheck: a tagged path step is not followed (use U.node) (" + why + ")",
          not ok and "nested path" in why)
    twice = nested.replace("int d;", "int node;")
    ok, why = proof("E.ends.node=pt", {"src/a.c": twice},
                    {"src/a.c": twice.replace("} node; } ends;", "} pt; } ends;")})
    check("keycheck: nested member also named at T's top level rejected (" + why + ")",
          not ok and "exactly once" in why)
    ok, why = proof("E.ends.node=pair", {"src/a.c": nested},
                    {"src/a.c": nested.replace("} node; } ends;", "} pair; } ends;")})
    check("keycheck: nested member destination already a member rejected", not ok)
    ok, why = proof("E.ends.node=pt", {"src/a.c": nested},
                    {"src/a.c": nested_new.replace("int pair;", "int pairs;")})
    check("keycheck: other edits in the nested aggregate rejected", not ok)
    check("keycheck: nested member pair parses", nk.parse_pairs("E.ends.node=pt") == {"E.ends.node": "pt"})
    # bit-field and function-pointer declarators in the nested container
    for label, decl, new_decl, use, new_use in [
            ("bit-field", "unsigned node : 4;", "unsigned pt : 4;", "e->ends.node", "e->ends.pt"),
            ("function pointer", "int (*node)(int);", "int (*pt)(int);", "e->ends.node(1)", "e->ends.pt(1)")]:
        b = ("typedef struct { int d; union { int pair; " + decl + " } ends; } E;\n"
             "int f(E *e) { return " + use + "; }\n")
        n_ = b.replace(decl, new_decl).replace(use, new_use)
        ok, why = proof("E.ends.node=pt", {"src/a.c": b}, {"src/a.c": n_})
        check("keycheck: nested " + label + " member renamed (" + why + ")", ok)
    ts = ["{", "unsigned", "node", ":", "W", ";", "int", "x", ":", "3", ";", "}"]
    check("keycheck: an identifier bit-field width is not a declarator",
          nk.container_declarators(ts, 0, len(ts) - 1) == {2: "node", 7: "x"})
    # depth-2 path T.u.v.m
    deep = ("typedef struct { int d; union { int pair; struct { short b; union { int raw; short node; } w; } s; } ends; } E;\n"
            "int f(E *e) { return e->ends.s.w.node; }\n")
    deep_new = deep.replace("short node; } w;", "short pt; } w;").replace("w.node", "w.pt")
    ok, why = proof("E.ends.s.w.node=pt", {"src/a.c": deep}, {"src/a.c": deep_new})
    check("keycheck: depth-2 nested path T.u.v.m (" + why + ")", ok)
    ok, why = proof("E.ends.node=pt", {"src/a.c": deep}, {"src/a.c": deep_new})
    check("keycheck: depth-2 member is not reachable with a depth-1 path (" + why + ")", not ok)
    # a nested pair and a flat pair on the same T
    both_new = nested_new.replace("int d;", "int dd;")
    ok, why = proof("E.ends.node=pt,E.d=dd", {"src/a.c": nested}, {"src/a.c": both_new})
    check("keycheck: nested and flat pairs on one T (" + why + ")", ok)

    # naming-bar §5: member pairs naming distinct members may share one n in a wave.
    faces = ("typedef struct { int pad; unsigned char col[4]; } FaceTri;\n"
             "typedef struct { int pad; unsigned char col[4]; short uv; } FaceQuad;\n"
             "int f(FaceTri *t, FaceQuad *q) { return t->col[0] + q->col[1]; }\n"
             "int g(FaceQuad *q) { return q->col[2]; }\n")
    faces_new = faces.replace("col", "vtx")
    ok, why = proof("FaceTri.col=vtx,FaceQuad.col=vtx", {"src/a.c": faces}, {"src/a.c": faces_new})
    check("keycheck: the same n across distinct types in one wave accepted (" + why + ")", ok)
    ok, why = proof("FaceTri.col=vtx,FaceQuad.col=vtx", {"src/a.c": faces},
                    {"src/a.c": faces_new.replace("unsigned char vtx[4]; short uv;",
                                                  "unsigned char col[4]; short uv;")})
    check("keycheck: a shared-n wave still renames every pair's declaration (" + why + ")", not ok)
    mixed = faces.replace("FaceQuad", "FaceQ").replace("FaceQ *q) { return q->col[2]", "FaceQ *q) { return q->idx[2]")
    mixed = mixed.replace("unsigned char col[4]; short uv;", "unsigned char idx[4]; short uv;").replace("q->col[1]", "q->idx[1]")
    ok, why = proof("FaceTri.col=vtx,FaceQ.idx=vtx", {"src/a.c": mixed},
                    {"src/a.c": mixed.replace("col", "vtx").replace("idx", "vtx")})
    check("keycheck: distinct old names sharing n accepted (" + why + ")", ok)
    for label, extra in [
            ("another member", "struct NavPoly { int vtx; };\n"),
            ("a type", "typedef int vtx;\n"),
            ("a macro", "#define vtx 3\n")]:
        ok, why = proof("FaceTri.col=vtx,FaceQuad.col=vtx", {"src/a.c": extra + faces},
                        {"src/a.c": extra + faces_new})
        check("keycheck: shared n colliding with " + label + " outside the pairs refused (" + why + ")", not ok)
    same_t = "struct T { int a; int b; };\nint f(struct T *t) { return t->a + t->b; }\n"
    ok, why = proof("T.a=n,T.b=n", {"src/a.c": same_t}, {"src/a.c": same_t.replace("a", "n").replace("b", "n")})
    check("keycheck: two members of one T sharing n refused (" + why + ")", not ok and "distinct member" in why)
    local_new = faces.replace("FaceTri *t, ", "FaceTri *vtx, ").replace("t->col[0]", "vtx->vtx[0]")
    local_new = local_new.replace("unsigned char col[4]; } FaceTri", "unsigned char vtx[4]; } FaceTri")
    ok, why = proof("FaceTri.col=vtx,f:t=vtx", {"src/a.c": faces}, {"src/a.c": local_new})
    check("keycheck: a member pair sharing n with a local pair refused (" + why + ")",
          not ok and "distinct member" in why)
    # The aggregate is the resolved definition: a tag and its typedef are one T, and
    # a nested member (T.u.m) is T's.
    tagged = ("typedef struct Tag { int a; int b; int m; } T;\n"
              "int f(T *t) { return t->a + t->b + t->m; }\n")
    for spec, new_text in [("Tag.a=n,T.b=n", tagged.replace("int a; int b;", "int n; int n;")
                            .replace("t->a + t->b", "t->n + t->n")),
                           ("Tag.m=n,T.m=n", tagged.replace("int m;", "int n;").replace("t->m", "t->n"))]:
        ok, why = proof(spec, {"src/a.c": tagged}, {"src/a.c": new_text})
        check("keycheck: tag and typedef spellings of one T cannot share n: " + spec + " (" + why + ")",
              not ok and "same aggregate" in why)
    nest = ("typedef struct { int d; union { int pair; int m; } u; } T;\n"
            "typedef struct { int m; } V;\n"
            "int f(T *t, V *v) { return t->u.m + v->m + t->d; }\n")
    ok, why = proof("T.u.m=n,V.m=n", {"src/a.c": nest},
                    {"src/a.c": nest.replace("int m; } u;", "int n; } u;").replace("int m; } V;", "int n; } V;")
                     .replace("t->u.m", "t->u.n").replace("v->m", "v->n")})
    check("keycheck: a nested pair sharing n with another type's flat pair accepted (" + why + ")", ok)
    ok, why = proof("T.d=n,T.u.m=n", {"src/a.c": nest},
                    {"src/a.c": nest.replace("int d;", "int n;").replace("int m; } u;", "int n; } u;")
                     .replace("t->u.m", "t->u.n").replace("t->d", "t->n")})
    check("keycheck: T.a=n with T.u.b=n refused: one T (" + why + ")", not ok and "same aggregate" in why)
    # A third type U keeps col. Keeping u->col passes. A hand edit u->col -> u->vtx is
    # token-explained (the token proof never resolves a receiver); n is fresh, so it
    # names no member of U and the build refuses it: the member-pair contract is
    # keycheck + oracle build (real cc1 below, skipped without the toolchain).
    third = faces + "typedef struct { int col; } U;\nint h(U *u) { return u->col; }\n"
    third_new = faces_new + "typedef struct { int col; } U;\nint h(U *u) { return u->vtx; }\n"
    ok, why = proof("FaceTri.col=vtx,FaceQuad.col=vtx", {"src/a.c": third},
                    {"src/a.c": third_new.replace("u->vtx", "u->col")})
    check("keycheck: a third type keeping col and its access passes (" + why + ")", ok)
    make_text = (Path(nk.CODE) / "Makefile").read_text(encoding="utf-8")
    if not nk.shutil.which(nk.cpp_command(make_text)[0]) or not (nk.ROOT / nk.cc1_command(make_text, "src/a.c")[0]).is_file():
        skip("keycheck: shared-n wrong receiver", "project cpp / cc1 unavailable")
    else:
        tree = nk.CppTree({"Makefile": make_text, "src/a.c": third_new}, "shared-n-third")
        try:
            result = nk.compile_cc1(tree, "src/a.c")
        finally:
            nk.close_cpp_trees([tree])
        check("keycheck: the build refuses a third type's access edited to the shared n",
              result.returncode != 0 and "has no member named `vtx'" in result.stderr)

    # Member pairs and macro arguments: an unchanged argument naming m is allowed when
    # the macro expands it plainly; an edited argument, or one the macro may drop,
    # stringize or paste, is refused.
    macro_base = ("#define SQ(x) ((x) * (x))\n#define STR(x) #x\n#define DROP(x) 0\n"
                  "struct T { int m; }; struct U { int m; };\n"
                  "int f(struct T *t, struct U *u) { return t->m + SQ(u->m); }\n")
    macro_new = macro_base.replace("struct T { int m;", "struct T { int n;").replace("t->m +", "t->n +")
    ok, why = proof("T.m=n", {"src/a.c": macro_base}, {"src/a.c": macro_new})
    check("keycheck: unchanged plain macro argument naming m accepted (" + why + ")", ok)
    edited_base = macro_base.replace("SQ(u->m)", "SQ(t->m)")
    edited_new = macro_new.replace("SQ(u->m)", "SQ(t->n)")
    ok, why = proof("T.m=n", {"src/a.c": edited_base}, {"src/a.c": edited_new})
    check("keycheck: edited plain macro argument accepted (" + why + ")", ok)
    for macro in ("STR", "DROP"):
        b = macro_base.replace("SQ(u->m)", macro + "(u->m)")
        n_ = macro_new.replace("SQ(u->m)", macro + "(u->m)")
        ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_})
        check("keycheck: unchanged argument of a stringizing / dropping macro refused " + macro + " (" + why + ")",
              not ok and "macro " + macro in why)
    # A small expanding cpp for the macro-argument sentinel proof (fake_cpp echoes
    # source lines): object / function-like #define, # stringize, rescan with a
    # hide set, `#if 0` blocks; one TU, no includes.
    def mini_cpp(tree, tu):
        macros, out, skip = {}, [cpp_preamble(tu)], 0
        tok_re = re.compile(r'"[^"]*"|\w+|->|##|#|\S')

        def expand(toks, hide=frozenset()):
            res, i = [], 0
            while i < len(toks):
                t = toks[i]
                if t in macros and t not in hide:
                    params, body = macros[t]
                    if params is None:
                        res += expand(body, hide | {t})
                        i += 1
                        continue
                    if toks[i + 1:i + 2] == ["("]:
                        depth, args, j = 0, [[]], i + 1
                        while True:
                            tok = toks[j]
                            if tok == "(":
                                depth += 1
                                if depth > 1:
                                    args[-1].append(tok)
                            elif tok == ")":
                                depth -= 1
                                if depth == 0:
                                    break
                                args[-1].append(tok)
                            elif tok == "," and depth == 1:
                                args.append([])
                            else:
                                args[-1].append(tok)
                            j += 1
                        sub, k = [], 0
                        while k < len(body):
                            bt = body[k]
                            if bt == "#" and k + 1 < len(body) and body[k + 1] in params:
                                sub.append('"' + " ".join(args[params.index(body[k + 1])]) + '"')
                                k += 2
                                continue
                            sub += expand(args[params.index(bt)], hide) if bt in params else [bt]
                            k += 1
                        res += expand(sub, hide | {t})
                        i = j + 1
                        continue
                res.append(t)
                i += 1
            return res
        lines, joined = tree[tu].splitlines(), 0
        for n, line in enumerate(lines):
            if joined:
                joined -= 1
                out.append("\n")
                continue
            # A line with an open parenthesis takes the following lines (an
            # invocation's expansion comes out on its first line, as with cpp).
            while not line.lstrip().startswith("#") and line.count("(") > line.count(")") \
                    and n + joined + 1 < len(lines):
                joined += 1
                line += " " + lines[n + joined]
            m = re.match(r"\s*#\s*(\w+)(.*)", line)
            if m:
                if m[1] == "if":
                    skip += 1 if skip or m[2].strip() == "0" else 0
                elif m[1] == "endif" and skip:
                    skip -= 1
                elif m[1] == "define" and not skip:
                    d = re.match(r"\s*(\w+)(\(([^)]*)\))?\s*(.*)", m[2])
                    params = [p.strip() for p in d[3].split(",")] if d[2] else None
                    macros[d[1]] = (params, tok_re.findall(d[4]))
                    out.append(line + "\n")
                    continue
                out.append("\n")
                continue
            out.append("\n" if skip else " ".join(expand(tok_re.findall(line))) + "\n")
        return "".join(out)

    # The unchanged m inside a macro argument must reach cpp output as a member access.
    sq = "#define SQ(x) ((x) * (x))\n"
    plain_base = (sq + "struct T { int m; }; struct U { int m; };\n"
                  "int f(struct T *t, struct U *u) { return t->m + SQ(u->m); }\n")
    plain_new = plain_base.replace("struct T { int m;", "struct T { int n;").replace("t->m +", "t->n +")
    ok, why = proof("T.m=n", {"src/a.c": plain_base}, {"src/a.c": plain_new}, cpp=mini_cpp)
    check("keycheck: kept macro argument expanded as a member access passes (" + why + ")", ok)
    for label, defs, invocation in [
            ("nested forwarding into a dropping macro", "#define DROP(y) 0\n#define A(x) DROP(x)\n", "A(u->m)"),
            ("macro passed as an argument", "#define DROP(y) 0\n#define CALL(f, a) f(a)\n", "CALL(DROP, u->m)"),
            ("stringized through forwarding", "#define STR(y) #y\n#define A(x) STR(x)\n", "A(u->m)")]:
        b = defs + plain_base.replace("SQ(u->m)", invocation)
        n_ = defs + plain_new.replace("SQ(u->m)", invocation)
        ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
        check("keycheck: kept macro argument refused: " + label + " (" + why + ")",
              not ok and ("discarded" in why or "stringified" in why))
    getm = "#define GETM(p, f) ((p)->f)\n"
    b = getm + plain_base.replace("SQ(u->m)", "GETM(u, m)")
    n_ = getm + plain_new.replace("SQ(u->m)", "GETM(u, m)")
    ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
    check("keycheck: member name passed as a macro argument and expanded after -> passes (" + why + ")", ok)
    b = "#define ID(x) (x)\n" + plain_base.replace("SQ(u->m)", "ID(m)") .replace("int f(", "int m;\nint f(")
    n_ = "#define ID(x) (x)\n" + plain_new.replace("SQ(u->m)", "ID(m)").replace("int f(", "int m;\nint f(")
    ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
    check("keycheck: kept m in a macro argument that is not a member access refused (" + why + ")",
          not ok and "member access" in why)
    # item 2: conditional code and headers no TU includes
    b = plain_base.replace("int f(", "#if 0\nint g(struct U *u) { return SQ(u->m); }\n#endif\nint f(")
    n_ = plain_new.replace("int f(", "#if 0\nint g(struct U *u) { return SQ(u->m); }\n#endif\nint f(")
    ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
    check("keycheck: kept macro argument in #if 0 code refused (" + why + ")",
          not ok and ("never compiles" in why or "conditional code" in why))
    hdr = sq + "struct U; static int h(struct U *u) { return SQ(u->m); }\n"
    ok, why = proof("T.m=n", {"src/a.c": plain_base.replace(sq, ""), "include/unused.h": hdr},
                    {"src/a.c": plain_new.replace(sq, "")}, compile_headers=False)
    check("keycheck: kept macro argument in a header no TU includes refused (" + why + ")",
          not ok and "never compiles" in why)
    # A retained plain .m / ->m (no macro) needs a compiled TU too.
    access_base = plain_base.replace("SQ(u->m)", "u->m")
    access_new = plain_new.replace("SQ(u->m)", "u->m")
    ok, why = proof("T.m=n", {"src/a.c": access_base}, {"src/a.c": access_new}, cpp=mini_cpp)
    check("keycheck: kept plain member access in compiled code passes (" + why + ")", ok)
    dead = "#if 0\nint g(struct U *u) { return u->m; }\n#endif\nint f("
    ok, why = proof("T.m=n", {"src/a.c": access_base.replace("int f(", dead)},
                    {"src/a.c": access_new.replace("int f(", dead)}, cpp=mini_cpp)
    check("keycheck: kept plain member access in #if 0 code refused (" + why + ")",
          not ok and ("member access in code the build never compiles" in why or "conditional code" in why))
    hdr = "struct U; static int h(struct U *u) { return u->m; }\n"
    ok, why = proof("T.m=n", {"src/a.c": access_base, "include/unused.h": hdr},
                    {"src/a.c": access_new}, compile_headers=False)
    check("keycheck: kept plain member access in a header no TU includes refused (" + why + ")",
          not ok and "never compiles" in why)
    # An entire-file #ifndef guard is not conditional code to the static plan; a guard
    # the TU pre-defines drops the header body, so the kept access never compiles.
    guarded = "#ifndef G_H\n#define G_H\nstruct U; static int h(struct U *u) { return u->m; }\n#endif\n"
    predefined = '#define G_H\n#include "../include/g.h"\n'

    def guard_cpp(tree, tu):
        out, inside = [], False
        for line in fake_cpp(tree, tu).splitlines(keepends=True):
            if line.startswith("# "):
                inside = '"include/g.h"' in line and line.rstrip().endswith(" 1")
                out.append(line)
            else:
                out.append("\n" if inside else line)
        return "".join(out)
    ok, why = proof("T.m=n", {"src/a.c": predefined + access_base, "include/g.h": guarded},
                    {"src/a.c": predefined + access_new}, compile_headers=False, cpp=guard_cpp)
    check("keycheck: kept plain member access in a pre-guarded header refused (" + why + ")",
          not ok and "member access in code the build never compiles" in why)
    # A file the census cannot map, or one missing from the new tree, refuses instead
    # of skipping its kept sites.
    ok, why = proof("T.m=n", {"src/a.c": access_base, "src/b.c": "int x;\n"},
                    {"src/a.c": access_new, "src/b.c": "int x ;;\n"}, cpp=mini_cpp)
    check("keycheck: token-count mismatch in the keep census refused (" + why + ")",
          not ok and "cannot map" in why)
    edits = nk.scoped_plan({"src/a.c": access_base, "src/b.c": "int x;\n"}, {"T.m": "n"}, {})
    try:
        with patch.object(nk, "preprocess", mini_cpp), patch.object(nk, "compile_cc1", fake_cc1), \
                patch.object(nk, "cpp_dependencies", fake_dependencies):
            nk.verify_compiled_edits({"src/a.c": access_base, "src/b.c": "int x;\n"},
                                     {"src/a.c": access_new}, edits, scoped={"T.m": "n"}, global_pairs={})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: file missing from the new tree refused (" + why + ")", "missing from the new tree" in why)
    # An edited member token inside a macro argument: its edit sentinel must be emitted
    # by a compiled TU, survive cpp and follow . or -> (4b).
    eb = plain_base.replace("SQ(u->m)", "SQ(t->m)")
    en = plain_new.replace("SQ(u->m)", "SQ(t->n)")
    ok, why = proof("T.m=n", {"src/a.c": eb}, {"src/a.c": en}, cpp=mini_cpp)
    check("keycheck: edited macro argument expanded as a member access passes (" + why + ")", ok)
    multi_b = eb.replace("SQ(t->m)", "SQ(t->m +\n        1)")
    multi_n = en.replace("SQ(t->n)", "SQ(t->n +\n        1)")
    ok, why = proof("T.m=n", {"src/a.c": multi_b}, {"src/a.c": multi_n}, cpp=mini_cpp)
    check("keycheck: edited argument on a multi-line invocation passes (" + why + ")", ok)
    multi_b = eb.replace("SQ(t->m)", "SQ(1 +\n        t->m)")
    multi_n = en.replace("SQ(t->n)", "SQ(1 +\n        t->n)")
    ok, why = proof("T.m=n", {"src/a.c": multi_b}, {"src/a.c": multi_n}, cpp=mini_cpp)
    check("keycheck: edited argument on an invocation's second line passes (" + why + ")", ok)
    # ... also when it is a TU's only edit (that line emits nothing itself)
    other = sq + "struct T;\nint g(struct T *t) { return SQ(1 +\n        t->m); }\n"
    ok, why = proof("T.m=n", {"src/a.c": eb, "src/b.c": other},
                    {"src/a.c": en, "src/b.c": other.replace("t->m", "t->n")}, cpp=mini_cpp)
    check("keycheck: a TU whose only edit is on an invocation's second line passes (" + why + ")", ok)
    for label, defs, invocation in [
            ("nested forwarding into a dropping macro", "#define DROP(y) 0\n#define A(x) DROP(x)\n", "A(t->%s)"),
            ("macro passed as an argument", "#define DROP(y) 0\n#define CALL(f, a) f(a)\n", "CALL(DROP, t->%s)"),
            ("stringized through forwarding", "#define STR(y) #y\n#define A(x) STR(x)\n", "A(t->%s)")]:
        b = defs + plain_base.replace("SQ(u->m)", invocation % "m")
        n_ = defs + plain_new.replace("SQ(u->m)", invocation % "n")
        ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
        check("keycheck: edited macro argument refused: " + label + " (" + why + ")",
              not ok and ("discarded" in why or "stringified" in why))
    for macro in ("STR", "DROP"):
        b = macro_base.replace("SQ(u->m)", macro + "(t->m)")
        n_ = macro_new.replace("SQ(u->m)", macro + "(t->n)")
        ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_})
        check("keycheck: edited argument of a stringizing / dropping macro refused " + macro + " (" + why + ")",
              not ok and "macro " + macro in why)
    # GETM(p, f) pastes nothing but puts its member after ->; the bare argument m is
    # no member access in the source, so editing it stays an unexplained change.
    b = getm + plain_base.replace("SQ(u->m)", "GETM(t, m)")
    n_ = getm + plain_new.replace("SQ(u->m)", "GETM(t, n)")
    ok, why = proof("T.m=n", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
    check("keycheck: edited bare macro argument (not after . / ->) refused (" + why + ")", not ok)
    hdr = sq + "struct T; static int h(struct T *t) { return SQ(t->m); }\n"
    ok, why = proof("T.m=n", {"src/a.c": plain_base.replace(sq, ""), "include/unused.h": hdr},
                    {"src/a.c": plain_new.replace(sq, ""), "include/unused.h": hdr.replace("t->m", "t->n")},
                    compile_headers=False)
    check("keycheck: edited macro argument in a header no TU includes refused (" + why + ")",
          not ok and "never compiles" in why)
    # Shared n: an edit owned by two pairs (same m and n) inside a macro argument.
    sq_faces = sq + faces.replace("t->col[0] + q->col[1]", "SQ(t->col[0]) + SQ(q->col[1])")
    ok, why = proof("FaceTri.col=vtx,FaceQuad.col=vtx", {"src/a.c": sq_faces},
                    {"src/a.c": sq_faces.replace("col", "vtx")}, cpp=mini_cpp)
    check("keycheck: shared-n edits inside macro arguments carry the combined namespace (" + why + ")", ok)
    # item 3: n as an unrelated local inside a macro argument
    b = plain_base.replace("return t->m + SQ(u->m);", "int pt = 2; return t->m + SQ(pt);")
    n_ = (plain_base.replace("struct T { int m;", "struct T { int pt;")
          .replace("return t->m + SQ(u->m);", "int pt = 2; return t->pt + SQ(pt);"))
    ok, why = proof("T.m=pt", {"src/a.c": b}, {"src/a.c": n_}, cpp=mini_cpp)
    check("keycheck: destination as an unrelated local in a macro argument is not an edit (" + why + ")", ok)
    ok, why = proof("T.m=pt", {"src/a.c": b}, {"src/a.c": n_.replace("SQ(pt)", "SQ(u->pt)")}, cpp=mini_cpp)
    check("keycheck: destination after -> in a macro argument is an edit, refused (" + why + ")", not ok)

    nested_macro = nested.replace("+ n->node;", "+ SQ(n->node);")
    nested_macro = "#define SQ(x) ((x) * (x))\n" + nested_macro
    check("keycheck: nested member with an unchanged plain macro argument naming m (" +
          proof("E.ends.node=pt", {"src/a.c": nested_macro},
                {"src/a.c": "#define SQ(x) ((x) * (x))\n" + nested_new.replace("+ n->node;", "+ SQ(n->node);")})[1] + ")",
          proof("E.ends.node=pt", {"src/a.c": nested_macro},
                {"src/a.c": "#define SQ(x) ((x) * (x))\n" + nested_new.replace("+ n->node;", "+ SQ(n->node);")})[0])
    with contextlib.redirect_stderr(io.StringIO()):
        try:
            nk.parse_pairs("f:a.b=c")
            bad = False
        except SystemExit:
            bad = True
    check("keycheck: a local pair cannot carry a member path", bad)
    for text in ["struct T { int x; };", "struct T { int m; int m; };",
                 "struct T { int m; }; struct T { int m; };",
                 "struct T { struct { int m; } x; };",
                 "typedef struct { int m; } *T;"]:
        check("keycheck: ambiguous/missing aggregate fails " + text, not proof("T.m=fresh",
              {"src/a.c": text}, {"src/a.c": text.replace("int m;", "int fresh;")})[0])
    for decl in ["typedef struct { int m; } T;", "union T { int m; };",
                 "typedef struct Tag { int m; } T;",
                 "typedef struct Tag T; struct Tag { int m; };"]:
        check("keycheck: supported aggregate spelling " + decl, proof("T.m=fresh",
              {"include/t.h": decl}, {"include/t.h": decl.replace("int m;", "int fresh;")})[0])
    for label, extra in [("macro body", "#define GET(x) x->m\n"),
                         ("macro splice", "#define GET(x) x->\\\nm\n"),
                         ("token paste", "#define GET(x) x ## m\n")]:
        check("keycheck: rejects member " + label, not proof("T.m=fresh",
              {"src/a.c": member + extra}, {"src/a.c": renamed + extra})[0])
    local = "int f(int l) { return l + 1; } int g(int l) { return l; } /* l */"
    local_new = "int f(int fresh) { return fresh + 1; } int g(int l) { return l; } /* fresh */"
    check("keycheck: local parameter bounded to function", proof("f:l=fresh",
          {"src/a.c": local}, {"src/a.c": local_new})[0])
    check("keycheck: local outside function fails", not proof("f:l=fresh",
          {"src/a.c": local}, {"src/a.c": local_new.replace("g(int l)", "g(int fresh)")})[0])
    for label, text in [
            ("shadowing", "int f(int l) { { int l; l = 1; } return l; }"),
            ("parenthesized shadowing", "int f(int l) { { int (l); } return l; }"),
            ("parenthesized array shadowing", "int f(int l) { { int (l[2]); } return l; }"),
            ("function pointer shadowing", "int f(int l) { { int (*l)(void); } return l; }"),
            ("comma shadowing after initializer", "int f(int l) { { int x = 0, l; } return l; }"),
            ("enum shadowing", "int f(int l) { { enum E { l }; } return l; }"),
            ("duplicate definitions", "int f(int l) { return l; } int f(int l) { return l; }"),
            ("macro", "#define X l\nint f(int l) { return l; }"),
            ("macro parameter", "#define X(l) 1\nint f(int l) { return l; }"),
            ("nonfresh destination", "int f(int l) { int fresh; return l; }"),
            ("use before declaration", "int f(void) { use(l); int l; return l; }")]:
        ok, why = proof("f:l=fresh", {"src/a.c": text}, {"src/a.c": text.replace("l", "fresh")})
        check("keycheck: local rejects " + label, not ok and bool(why))
    text = "int f(void) { int l; l = 1; return l; }"
    check("keycheck: simple outer local", proof("f:l=fresh", {"src/a.c": text},
          {"src/a.c": text.replace("l", "fresh")})[0])
    for label, text in [
            ("nested local", "int f(void) { { int l; l = 1; } return 0; }"),
            ("doubly nested local", "int f(int x) { if (x) { while (x) { int l = x; x -= l; } } return x; }"),
            ("comma declarator", "int f(void) { int a, l; l = 1; a = l; return a; }"),
            ("comma pointer declarator", "int f(int *p) { int a, *l; l = p; a = *l; return a; }"),
            ("comma declarator after initializer", "int f(void) { int a = 1, l; l = a; return l; }"),
            ("comma array declarator", "int f(void) { int a, l[2]; l[0] = 1; a = l[0]; return a; }"),
            ("nested comma declarator", "int f(int x) { if (x) { short a, l; l = x; a = l; x = a; } return x; }"),
            ("comma declarator after initializer braces", "int f(void) { int b[1] = {0}, l; l = b[0]; return l; }"),
            ("comma declarator after a compound literal", "int f(void) { int a = (int){1}, l; l = a; return l; }"),
            ("comma expression statement after a block", "int f(int x) { int l; { x++; } x = 1, l = x; return l; }"),
            # Block openers the brace scan must keep recognising.
            ("use in an else block", "int f(int x) { int l = x; if (x) { x++; } else { x += l; } return l; }"),
            ("use in a do block", "int f(int x) { int l = x; do { x += l; } while (x < 10); return l; }"),
            ("use in a case block", "int f(int x) { int l = x; switch (x) { case 1: { x += l; } break; } return l; }"),
            ("use in a labelled block", "int f(int x) { int l = x; again: { x += l; } if (x < 9) goto again; return l; }"),
            ("use after a statement expression", "int f(int x) { int l = x; x = ({ x + 1; }) + l; return x; }"),
            ("use beside a pointer-returning nested function",
             "int f(int x) { int l = x; int *h(int *p) { return p; } return *h(&l); }"),
            ("use in an old-style designator brace",
             "typedef struct { int m[1]; } S;\nint f(void) { int l = 1; S s = { m: { l } }; return s.m[0]; }"),
            ("uses led by unary operators", "int f(void) { int l = 1; -l; !l; ~l; ++l; --l; +l; &l; *&l; return l; }"),
            ("use beside a GNU nested function","int f(int x) { int l = x; int h(int y) { return y + 1; } return h(l); }"),
            ("use after a nested initializer","int f(void) { { int l = 1; int a[2][2] = {{0, 1}, {l, 2}}; return a[1][0]; } }"),
            ("comma declarator after const pointer", "int f(int *p) { int * const a = p, l; l = *a; return l; }"),
            ("third comma declarator", "int f(void) { int a, b, l; l = 1; a = l; b = a; return b; }"),
            ("pointer comma declarator after a function typedef",
             "typedef int FN(void);\nint g(void);\nint f(void) { FN b, *l; l = g; return l == g; }"),
            ("parameter in a switch and case", "int f(int l) { switch (l) { case 1: l = 2; } return l; }"),
            ("local assigned under a case", "int f(int x) { int l = x; switch (x) { case 1: l = 2; } return l; }"),
            ("sizeof statement", "int f(int l) { sizeof l; return l; }"),
            ("use in a later declarator's parenthesized initializer",
             "int f(int x) { int l = x; int a = 0, b = (l + 1); return a + b; }"),
            ("uses beside casts and an array-bound type name",
             "int f(int x) { int l = x; x = (int)l + (short)(l) + sizeof(int[l]); return ((int)(l + 1)) + x; }"),
            ("use under a default label", "int f(int x) { int l = x; switch (x) { default: l = 1; } return l; }"),
            ("while statement", "int f(int l) { while (l) l--; return l; }"),
            ("builtin call statement", "int f(int *p) { int l = 1; __builtin_memcpy(p, &l, 4); return l; }"),
            ("computed goto", "int f(void) { void *l = &&out; goto *l; out: return 0; }"),
            ("nested local after initializer braces",
             "int f(int x) { { int a[1] = {0}; int l = x; x += l + a[0]; } return x; }"),
            # A `;` inside a for header does not start a statement.
            ("use in a for header after a number", "int f(int x) { int l = x; for (x = 0; 9 > l; l++) ; return l; }")]:
        ok, why = proof("f:l=fresh", {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "fresh", text)})
        check("keycheck: local accepts " + label, ok)
    text = "struct l { int m; };\nint f(int l, struct l *p) { return l + p->m; }"
    check("keycheck: local accepts parameter beside a struct tag of its name",
          proof("f:l=fresh", {"src/a.c": text},
                {"src/a.c": text.replace("int l,", "int fresh,").replace("return l", "return fresh")})[0])
    for label, tokens in [("closer mismatch", ["{", "(", "]", "}"]), ("stray closer", ["{", ")", "}"])]:
        try:
            nk.statement_starts(tokens, 0, len(tokens) - 1, set())
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: statement scan rejects " + label, "unbalanced" in why)
    for text, names in [("int typedef T;", {"T"}), ("struct S typedef T, *P;", {"T", "P"}),
                        ("unsigned typedef int T;", {"T"}), ("int typedef *T;", {"T"}),
                        ("typedef Ext Y;", {"Ext", "Y"}),
                        ("typedef int (__attribute__((aligned(8))) T);", {"T"}),
                        ("typedef int A, __attribute ((mode(SI))) B;", {"A", "B"}),
                        ("int typedef (*T)(void);", {"T"}), ("int typedef (T);", {"T"})]:
        check("keycheck: type census reads " + text,
              nk.declaration_types({"src/a.c": text}) - set(nk.DECL_WORDS) == names)
    # Even with a census that misses every typedef, a typedef's name before `(`
    # or `*` is no proven value.
    with patch.object(nk, "declaration_types", lambda cfiles: set(nk.DECL_WORDS)):
        for label, text in [
                ("paren", "typedef int T;\nint f(int x) { int l = x; { T (l); l = 3; x += l; } return l + x; }"),
                ("star", "typedef int T;\nint f(int x) { int l = x; { int T = 2; l += T; } { T *l = &x; *l = 5; } "
                 "return l + x; }"),
                ("type-name", "typedef int T;\nint f(int x) { int l = x; x = (T)(T (*)(T (*l)))0; return l + x; }")]:
            try:
                nk.scoped_plan({"src/a.c": text}, {"f:l": "nm"}, {})
                why = ""
            except ValueError as e:
                why = str(e)
            check("keycheck: typedef words refuse a missed type's " + label + " shadow",
                  ("inside a type name" if label == "type-name" else "unrecognised type") in why)
    text = "int f(int l) { int a = l, b = l + 1; return a + b + l; }"
    check("keycheck: local use in comma initializers", proof("f:l=fresh", {"src/a.c": text},
          {"src/a.c": re.sub(r"\bl\b", "fresh", text)})[0])
    for label, text, why_part in [
            ("use after nested block", "int l; int f(void) { { int l; l = 1; } return l; }", "after the nested"),
            ("use after inner of two blocks", "int l; int f(int x) { { { int l; l = x; } x = l; } return x; }", "after the nested"),
            ("sibling block rebinding", "int f(void) { { int l; l = 1; } { int l; l = 2; } return 0; }", "found 2"),
            ("comma parenthesized declarator", "int f(void) { int a = 0, (l); l = a; return l; }", "comma declarator"),
            ("comma struct declarator", "struct S { int m; }; int f(void) { struct S a, l; l = a; return l.m; }", "comma declarator"),
            ("comma array-bound use", "int f(int l) { int a, b[l]; a = l; return a; }", "comma declarator"),
            # A second binding after an initializer's braces is a binding, not a use.
            ("comma shadow after initializer braces",
             "int g(int);\nint f(void) { int a, l; l = 1; a = 0; { int b[1] = {0}, l; l = 2; g(l); } return l + a; }", "found 2"),
            ("nested shadow after initializer braces",
             "int f(int x) { { int l = x; { int b[1] = {0}, l = 3; x += l; } x += l; } return x; }", "found 2"),
            ("outer local shadowed after initializer braces",
             "int f(int x) { int l = 1; { int b[1] = {0}, l = 2; x += l; } return x + l; }", "found 2"),
            ("parameter shadowed after initializer braces",
             "int f(int l) { { int b[1] = {0}, l = 2; l++; } return l; }", "found 2"),
            # A compound literal's braces belong to the declaration: the later
            # declarator is a second binding.
            ("shadow after a compound literal",
             "int f(int x) { int l = x; { int a = (int){1}, l; l = a; x += l; } return x + l; }", "found 2"),
            ("shadow after a compound literal in a subscript",
             "int c[2];\nint f(int x) { { int l = x; { int a = c[(int){0}], l; l = 3; x += l + a; } x += l; } return x; }",
             "found 2"),
            ("shadow after a compound literal at depth 1",
             "int c[2];\nint f(int x) { int l = x; { int a = c[(int){0}], l; l = 3; x += l + a; } return x + l; }", "found 2"),
            ("comma-bound shadow after a compound literal",
             "int c[2];\nint f(int x) { int b = x, l = x; { int a = c[(int){0}], l; l = 3; x += l + a + b; } return x + l; }",
             "found 2"),
            ("pointer shadow after an array compound literal",
             "int c[2];\nint f(int x) { { int l = x; { int a = c[*(int[1]){0}], *l = &a; x += *l; } x += l; } return x; }",
             "found 2"),
            ("shadow after a sizeof compound literal",
             "int f(int x) { { int l = x; { int a[sizeof (int){0}], l; l = 3; a[0] = l; x += a[0]; } x += l; } return x; }",
             "parenthesized declarator"),
            ("shadow after a compound literal sum",
             "int c[2];\nint f(int x) { { int l = x; { int a = c[(int){0} + 0], l; l = 4; x += l + a; } x += l; } return x; }",
             "found 2"),
            # A nested function the scan cannot recognise is never a compound literal.
            ("shadow in a struct-returning nested function",
             "struct S { int m; };\nint f(int x) { int l = x; struct S g(void) { struct S s; int z = 0, l = 2; "
             "s.m = z + l; return s; } return g().m + l; }", "classify the brace"),
            ("shadow in a parenthesized nested function",
             "int f(int x) { int l = x; int (g)(void) { int z = 0, l = 2; return z + l; } return g() + l; }",
             "classify the brace"),
            ("shadow in an enum-returning nested function",
             "enum E { EA, EB };\nint f(int x) { int l = x; enum E g(void) { int z = 0, l = 2; return z + l; } "
             "return g() + l; }", "classify the brace"),
            ("shadow in a recognised nested function",
             "int f(int x) { int l = x; int g(void) { int z = 0, l = 2; return z + l; } return g() + l; }", "found 2"),
            # A statement led by a word the parser does not know as a type must
            # prove it is an expression; an unlisted keyword type cannot.
            ("GNU complex pointer shadow", "int f(int x) { int l = x; { __complex__ int *l = 0; x += l != 0; } return x + l; }",
             "found 2"),
            ("GNU complex pointer shadow, deeper",
             "int f(int x) { { int l = x; { __complex__ int *l = 0; x += l != 0; } x += l; } return x; }", "found 2"),
            ("GNU __complex pointer shadow", "int f(int x) { int l = x; { __complex int *l = 0; x += l != 0; } return x + l; }",
             "found 2"),
            ("GNU complex comma shadow", "int f(int x) { int l = x; { __complex__ int a, *l = 0; x += l != 0; } return x + l; }",
             "found 2"),
            ("unknown GNU keyword shadow", "int f(int x) { int l = x; { __bogus__ int *l; x += 1; } return x + l; }",
             "unrecognised type"),
            ("unknown word before a qualifier", "int f(int x) { int l = x; { __foo__ const *l; x += 1; } return x + l; }",
             "unrecognised type"),
            ("unknown word alone before the name", "int f(int x) { int l = x; { __bogus__ l = 3; x += l; } return x + l; }",
             "unrecognised type"),
            ("unknown word before a pointer", "int f(int x) { int l = x; { unknown_t *l; x += 1; } return x + l; }",
             "unrecognised type"),
            ("statement led by neither a name nor a keyword", "int f(int x) { int l = x; 0, l = 2; return l; }",
             "no declaration"),
            ("statement start after a stray comma", "int f(int x) { int l = x; { x++; } , l = 1; return l; }",
             "statement start"),
            ("unbalanced body", "int f(int x) { int l = x; x = (l]; return l; }", "unbalanced"),
            ("brace after a name in a block", "int f(int x) { int l = x; x { l++; } return l; }", "classify the brace"),
            ("brace after a name in an initializer", "int f(void) { int l = 1; int a[2] = { l {0} }; return a[0]; }",
             "classify the brace"),
            ("shadow in a pointer-returning nested function",
             "int f(int x) { int l = x; int *h(void) { int z = 0, l = 2; return 0; } return l; }", "found 2"),
            ("brace after a non-cast parenthesis", "int f(int x) { int l = x; g(x) { l++; } return l; }", "classify the brace"),
            # Specifiers before `typedef` still declare a type name.
            ("paren shadow of a typedef after its type", "int typedef T;\nint f(int x) { int l = x; { T (l); l = 3; x += l; } "
             "return l + x; }", "parenthesized declarator"),
            ("pointer paren shadow of a typedef after its type",
             "int typedef T;\nint f(int x) { int l = x; { T (*l); l = &x; *l = 4; } return l + x; }", "parenthesized declarator"),
            ("paren shadow of a typedef after its struct type",
             "struct S { int a; };\nstruct S typedef T;\nint f(int x) { int l = x; { T (l); l.a = 3; x += l.a; } "
             "return l + x; }", "parenthesized declarator"),
            ("star shadow of a typedef after its type, beside a value T",
             "int typedef T;\nint f(int x) { int l = x; { int T = 2; l += T; } { T *l = &x; *l = 5; } return l + x; }",
             "uncertain typedef"),
            # An attribute inside a typedef's declarator; cc1's own keywords.
            ("paren shadow of an attributed pointer typedef",
             "typedef int * __attribute__((aligned(8))) T;\nint f(int x) { int l = x; { T (l); l = &x; *l = 3; } "
             "return l + x; }", "parenthesized declarator"),
            ("paren shadow of a typedef attributed in its declarator",
             "typedef int (__attribute__((aligned(8))) T);\nint f(int x) { int l = x; { T (l); l = 3; x += l; } "
             "return l + x; }", "parenthesized declarator"),
            ("star shadow of an attributed typedef beside a value T",
             "typedef int (__attribute__((aligned(8))) T);\nint f(int x) { int l = x; { int T = 2; l += T; } "
             "{ T *l = &x; *l = 5; } return l + x; }", "uncertain typedef"),
            ("paren shadow of a parameter by an attributed typedef",
             "typedef int * __attribute__((aligned(8))) T;\nint f(int l) { { T (l) = &l; *l = 3; } return l; }",
             "parenthesized declarator"),
            ("paren shadow of a function-pointer typedef after its type",
             "int typedef (*T)(void);\nint g(void) { return 0; }\nint f(int x) { int l = x; { T (l); l = g; "
             "x += l != 0; } return x + l; }", "parenthesized declarator"),
            ("name repeated in a parameter's identifier list", "int f(int l, int (*cb)(l)) { return cb(l); }",
             "parameter list"),
            ("identifier list after an assignment inside the declarator",
             "int f(int x) { int l = x; int (*fp[sizeof(x = 1)])(l); return l; }", "parenthesized declarator"),
            ("identifier list in a later declarator",
             "int f(int x) { int l = x; int a = 0, (*fp[sizeof(x = 1)])(l); return l + a; }", "comma declarator"),
            ("second identifier-list name in a nested later declarator",
             "int f(int x) { int l = x; { int a = 0, (*fp[sizeof(a = 1)])(x, l); return l + a; } }", "comma declarator"),
            ("prototype name after a struct tag in sizeof",
             "struct S;\nint f(void) { int l = 0; return l + sizeof(int (*)(struct S (*l))); }", "inside a type name"),
            ("prototype name after a union tag in a cast initializer",
             "union U;\nint f(void) { int l = 0; int *x = (int *)(int (*)(union U (*l)))0; return l + (x != 0); }",
             "inside a type name"),
            ("qualifier-only prototype parameter in sizeof",
             "int f(void) { int l = 0; return l + sizeof(int (*)(const (*l))); }", "inside a type name"),
            ("bare qualifier prototype parameter in a cast",
             "int f(int x) { int l = 0; x = (int)(int (*)(volatile (l)))0; return l + x; }", "inside a type name"),
            ("C99 restrict as a typedef name", "typedef int restrict;\nint f(int x) { int l = x; { restrict *l = &x; "
             "x += *l; } return l + x; }", "found 2"),
            ("GNU __real__ statement", "int f(int x) { int l = x; __real__ l = 1; return l; }", "cannot prove the statement"),
            ("GNU __alignof__ statement", "int f(int x) { int l = x; __alignof__(l); return l; }", "cannot prove the statement"),
            ("struct-tag parenthesized parameter", "struct S;\nint f(struct S (*l)) { return l != 0; }",
             "parenthesized declarator"),
            ("C99 word as a function typedef", "typedef int _Bool(int);\nint f(void) { _Bool l; return 0; }",
             "not a local object binding"),
            ("GNU iterator shadow", "int f(int x) { int l = x; { __iterator__ (l) = 3; } return l + x; }",
             "cannot prove the statement"),
            ("GNU iterator shadow of a parameter",
             "int f(char *l, int x) { { __iterator__ (l) = 3; x += l * 2; } return *l + x; }", "cannot prove the statement"),
            ("unknown double-underscore word before a parenthesis",
             "int f(int x) { int l = x; { __foo__ (l) = 3; } return l + x; }", "unrecognised type"),
            ("brace after a call in an initializer",
             "int f(void) { int l = 1; int a[2] = { g(l) {0} }; return a[0]; }", "classify the brace"),
            ("use inside a statement expression", "int f(int x) { int l = x; x = ({ int t = l; t; }); return x; }",
             "statement expression"),
            # The shared declaration checks must see a comma declarator's own type.
            ("comma block-scope extern", "int f(void) { extern int a, l; l = 1; return l; }", "extern"),
            ("comma function-typedef declarator", "typedef int FN(void); int f(void) { FN a, l; return 0; }",
             "function declarator"),
            ("comma function after a pointer declarator", "typedef int FN(void); int f(void) { FN *a, l; return 0; }",
             "function declarator"),
            # A type the parser does not know (a macro, an unparsed typedef) can hide a shadow.
            ("macro-type comma shadow, nested",
             "#define MT int\nint f(int x) { { int l = x; { MT a, l; l = 3; x += l; } x += l; } return x; }",
             "unrecognised type"),
            ("macro-type comma shadow, outer local",
             "#define MT int\nint f(int x) { int l = x; { MT a, l; l = 3; x += l; } x += l; return x; }",
             "unrecognised type"),
            ("macro-type comma pointer shadow",
             "#define MT int\nint f(int x) { { int l = x; { MT a, *l; l = &a; x += *l; } x += l; } return x; }",
             "unrecognised type"),
            ("macro-type plain shadow",
             "#define MT int\nint f(int x) { int l = x; { MT l; l = 3; x += l; } return x + l; }",
             "object-like macro"),
            ("macro-type initialized comma shadow",
             "#define MT int\nint f(int x) { { int l = x; { MT a = 0, l = 2; x += l; } x += l; } return x; }",
             "unrecognised type"),
            ("macro-made typedef comma shadow",
             "#define DT(n) typedef int n\nDT(U);\nint f(int x) { { int l = x; { U a, *l = &x; x += *l; } x += l; } return x; }",
             "unrecognised type"),
            ("unknown-type comma shadow",
             "int f(int x) { { int l = x; { unknown_t a, *l; x += *l; } x += l; } return x; }",
             "unrecognised type"),
            ("qualified macro-type comma shadow",
             "#define MT int\nint f(int x) { { int l = x; { const MT a, *l = &x; x += *l; } x += l; } return x; }",
             "comma declarator"),
            # Shadows the source plan refuses before the cpp rerun.
            ("plan: qualified macro-type comma",
             "#define MT int\nint f(int x) { { int l = x; { MT const a = 1, *l; l = &a; x += *l; } x += l; } return x; }",
             "unrecognised type"),
            ("plan: macro-type volatile pointer comma",
             "#define MT int\nint f(int x) { { int l = x; { MT volatile *a = 0, *l; l = 0; x += a == l; } x += l; } "
             "return x; }", "unrecognised type"),
            ("plan: macro-type const comma",
             "#define MT int\nint f(int x) { { int l = x; { MT const a, l; l = 3; x += l; } x += l; } return x; }",
             "unrecognised type"),
            ("plan: macro type after a local of it", "#define MT int\nint f(int x) { unsigned MT u = 0; { int l = x; "
             "{ MT a, l; l = 3; x += l; } x += l; } return x + u; }", "unrecognised type"),
            ("plan: macro typedef const pointer comma", "#define DT(n) typedef int n\nDT(U);\nint f(int x) { { int l = x; "
             "{ U const *a, l; l = 3; x += l; } x += l; } return x; }", "unrecognised type"),
            ("plan: macro typedef shadowed by a sibling local",
             "#define TD(n) typedef int n;\nTD(X)\nint f(int x) { { int l = x; { int X = 1; x += X; } "
             "{ X a, *l; a = 1; l = &a; x += *l; } x += l; } return x; }", "unrecognised type"),
            # Each guard of the comma head.
            ("comma after a macro type and a qualifier", "#define MT int\nint f(void) { const MT a, l; l = 1; return l; }",
             "comma declarator"),
            ("comma after a struct pointer",
             "struct S { int m; };\nint f(void) { struct S *a, l; l.m = 0; a = &l; return a->m; }", "comma declarator"),
            ("comma after a parenthesized first declarator", "int f(int *p) { int (*a) = p, l; l = *a; return l; }",
             "comma declarator")]:
        new_text = re.sub(r"\bl\b", "fresh", text)
        ok, why = proof("f:l=fresh", {"src/a.c": text}, {"src/a.c": new_text})
        check("keycheck: local rejects " + label, not ok and why_part in why)
    for expression in ["call(l)", "(l + 1)", "x = (s32)l", "int x = call(l)",
                       "int x = call(1, l)", "call(1, l)", "*l", "l ? l : 0"]:
        text = "int f(int l) { " + expression + "; return l; }"
        check("keycheck: local expression " + expression, proof("f:l=fresh",
              {"src/a.c": text}, {"src/a.c": text.replace("int l", "int fresh")
               .replace(expression, re.sub(r"\bl\b", "fresh", expression))
               .replace("return l", "return fresh")})[0])
    for text, after in [
            ("int f(int l) { return p->l + l; }", "int f(int fresh) { return p->l + fresh; }"),
            ("int f(int l) { return p.l + l; }", "int f(int fresh) { return p.l + fresh; }"),
            ("int f(int l) { l: l++; goto l; return l; }",
             "int f(int fresh) { l: fresh++; goto l; return fresh; }"),
            ("int f(int l) { struct l *p; union l *q; enum l x; return l; }",
             "int f(int fresh) { struct l *p; union l *q; enum l x; return fresh; }")]:
        check("keycheck: local preserves other namespaces " + text,
              proof("f:l=fresh", {"src/a.c": text}, {"src/a.c": after})[0])
        check("keycheck: local forbids changing other namespaces " + text,
              not proof("f:l=fresh", {"src/a.c": text},
                        {"src/a.c": after.replace("->l", "->fresh").replace(".l", ".fresh")
                         .replace("l:", "fresh:").replace("goto l", "goto fresh")
                         .replace("struct l", "struct fresh").replace("union l", "union fresh")
                         .replace("enum l", "enum fresh")})[0])
    for macro_path in ["include/macros.h", "src/macros.h", "src/macros.c"]:
        ok, why = proof("f:l=fresh", {"src/a.c": local,
                        macro_path: "#define GET(p) ((p)->l)\n"}, {"src/a.c": local_new})
        # A .c nothing includes is not visible to f's TU (scoped-plan-bindings).
        check("keycheck: local rejects visible macro " + macro_path, ok if macro_path.endswith(".c")
              else not ok and macro_path in why and "#define body" in why)
    text = 'int f(int l) { __asm__("add %[l], %[l]" : [l] "+r"(l)); return l; }'
    check("keycheck: asm string substitution forbidden", not proof("f:l=fresh", {"src/a.c": text},
          {"src/a.c": text.replace("[l]", "[fresh]").replace("int l", "int fresh")
           .replace("(l)", "(fresh)").replace("return l", "return fresh")})[0])
    check("keycheck: asm literal text remains exact", proof("f:l=fresh", {"src/a.c": text},
          {"src/a.c": text.replace("int l", "int fresh").replace("[l] \"+r\"", "[fresh] \"+r\"")
           .replace("(l)", "(fresh)").replace("return l", "return fresh")})[0])
    shared_base = ("struct T { int m; }; union U { int k; int m; }; "
                   "int f(union U *u) { return u->m; }")
    shared_new = ("struct T { int n; }; union U { int n; int m; }; "
                  "int f(union U *u) { return u->n; }")
    ok, why = proof("T.m=n,U.k=n", {"src/a.c": shared_base}, {"src/a.c": shared_new})
    check("keycheck: shared scoped destination union rebinding fails (" + why + ")",
          not ok and "not fresh in cpp aggregates" in why)
    ok, why = proof("T.m=n,k=n", {"src/a.c": shared_base}, {"src/a.c": shared_new})
    check("keycheck: scoped/global destination collision fails", not ok and "global pair" in why)
    # An unrelated nested declarator contains a member-selection token. It must
    # not receive the optional access substitution allowed in executable code.
    text = "struct T { int m; }; struct U { int k[sizeof(((struct T *)0)->m)]; };"
    after = text.replace("int m;", "int n;").replace("->m", "->n")
    check("keycheck: aggregate interior is protected", not proof("T.m=n",
          {"src/a.c": text}, {"src/a.c": after})[0])
    text = "int n;\n#define GET() n\nint f(int l) { return l + GET(); }"
    ok, why = proof("f:l=n", {"src/a.c": text},
                    {"src/a.c": text.replace("int l", "int n").replace("return l", "return n")})
    check("keycheck: destination macro capture fails", not ok and "#define body" in why)
    for path in ["src/b.c", "include/b.h", "asm/b.s"]:
        ok, why = proof("f:l=n", {"src/a.c": "int f(int l) { return l; }", path: "int n;"},
                        {"src/a.c": "int f(int n) { return n; }"})
        check("keycheck: local destination visible declarations only " + path,
              ok if path.startswith("asm/") else not ok and "not fresh" in why)
    text = "int motion_InitStream(int bits_left) { return bits_left; }"
    after = "int motion_InitStream(int nbits) { return nbits; }"
    for other in ["int g(void) { int nbits; return nbits; }",
                  "int g(int nbits) { return nbits; }"]:
        check("keycheck: another function's local destination is safe " + other,
              proof("motion_InitStream:bits_left=nbits", {"src/a.c": text, "src/b.c": other},
                    {"src/a.c": after})[0])
        check("keycheck: same-file other-function local destination is safe " + other,
              proof("motion_InitStream:bits_left=nbits", {"src/a.c": text + "\n" + other},
                    {"src/a.c": after + "\n" + other})[0])
    for label, other in [
            ("global", "int nbits;"), ("function", "int nbits(void) { return 0; }"),
            ("typedef", "typedef int nbits;"),
            ("enum", "enum E { nbits };"),
            ("nested enum", "struct S { enum E { nbits } value; };"),
            ("function-local enum", "int g(void) { enum E { nbits }; return 0; }"),
            ("macro name", "#define nbits 1\n"),
            ("macro parameter", "#define X(nbits) 1\n"),
            ("macro body", "#define X nbits\n"),
            ("prototype parameter", "int g(int nbits);"),
            ("token paste", "#define X(x) x ## nbits\n")]:
        ok, why = proof("motion_InitStream:bits_left=nbits",
                        {"src/a.c": text, "include/b.h": other}, {"src/a.c": after})
        check("keycheck: local destination visible via " + label,
              ok if label in ("macro parameter", "prototype parameter") else not ok and "nbits" in why)
    for symbol in ["glabel nbits", "dlabel nbits", ".global nbits", "jal nbits"]:
        check("keycheck: unreferenced asm identifier permits local destination " + symbol, proof(
              "motion_InitStream:bits_left=nbits", {"src/a.c": text, "asm/b.s": symbol},
              {"src/a.c": after})[0])
    text = "int l; int f(int c, int l) { return c ? (int) l : 0; }"
    after = "int l; int f(int c, int n) { return c ? (int) n : 0; }"
    check("keycheck: ternary cast operand renames", proof("f:l=n",
          {"src/a.c": text.removeprefix("int l; ")}, {"src/a.c": after.removeprefix("int l; ")})[0])
    check("keycheck: round-11 ordinary old name outside f fails", not proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": after})[0])
    check("keycheck: ternary cast global rebinding fails", not proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": after.replace("(int) n", "(int) l")})[0])
    for expression in ["c ? (int) l : 0", "c ? 1 : l ? l : 0", "a[(l)]"]:
        text = "int f(int l) { return " + expression + "; }"
        check("keycheck: nested expression operand renames " + expression, proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    for spec, text, after in [
            ("f:l=b,a=b", "int f(int l){return l+a;}", "int f(int b){return b+b;}"),
            ("f:l=a,a=b", "int f(int l){return l;}", "int f(int a){return a;}"),
            ("f:l=g", "int l[4]; int g[4]; int f(void){return h(sizeof l);}",
             "int l[4]; int g[4]; int f(void){return h(sizeof g);}"),
            ("f:a=x,f:b=x", "int f(int a, int b){return a+b;}",
             "int f(int x, int x){return x+x;}")]:
        check("keycheck: follow-up capture probe " + spec, not proof(spec,
              {"src/a.c": text}, {"src/a.c": after})[0])
    for use in ["return h(sizeof l);", "if(c) return 0; else return l;",
                "if(c) return 0; else l = 1;", "if(c) return 0; else *l = 1;",
                "do l++; while(c);"]:
        text = "int l; int f(void){" + use + "}"
        after = text.replace(use, re.sub(r"\bl\b", "fresh", use))
        ok, why = proof("f:l=fresh", {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: non-declaration never supplies binding " + use,
              not ok and "simple binding" in why)
    text = "#define GET(p,f) ((p)->f)\nint f(int l){return GET(s, l) + l;}"
    ok, why = proof("f:l=n", {"src/a.c": text},
                    {"src/a.c": text.replace("int l", "int n").replace("s, l", "s, n")
                     .replace("+ l", "+ n")})
    check("keycheck: macro argument is not a local use", not ok and "macro GET" in why)
    for conditional in ["#if 0", "#if FEATURE", "#ifdef FEATURE", "#ifndef FEATURE"]:
        text = "struct T { int m; };\n" + conditional + "\nint f(struct T *p){return p->m;}\n#endif\n"
        ok, why = proof("T.m=n", {"src/a.c": text},
                        {"src/a.c": text.replace("int m", "int n").replace("->m", "->n")})
        check("keycheck: rejects conditional member access " + conditional,
              not ok and "preprocessor-conditional" in why)
    text = "#if FEATURE\nint f(int l){return l;}\n#endif\n"
    check("keycheck: rejects conditional local definition", not proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    text = "int f(int l){ int a[2] = { 0, l: 5 }; return l; }"
    after = "int f(int n){ int a[2] = { 0, l: 5 }; return n; }"
    check("keycheck: GNU designator remains exact", proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": after})[0])
    check("keycheck: GNU designator rename fails", not proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": after.replace("l:", "n:")})[0])
    for branch in ["case 1:", "default:", "case (1 ? 2 : 3):"]:
        text = "int f(int l){ switch(x){ " + branch + " l: x++; } return l; }"
        after = text.replace("int l", "int n").replace("return l", "return n")
        check("keycheck: preserves label after " + branch,
              proof("f:l=n", {"src/a.c": text}, {"src/a.c": after})[0])
        check("keycheck: rejects label change after " + branch,
              not proof("f:l=n", {"src/a.c": text}, {"src/a.c": after.replace("l:", "n:")})[0])
    for macro in ["#define l 1\n", "#define l(x) (x)\n"]:
        check("keycheck: rejects cross-file old macro name " + macro.strip(), not proof("f:l=n",
              {"src/a.c": "int f(int l){return l;}", "include/b.h": macro},
              {"src/a.c": "int f(int n){return n;}"})[0])
    for qualifier in ["__volatile__", "__const__", "__signed__", "__inline__", "auto",
                      "__attribute__((unused))", "__extension__"]:
        text = "int f(int l){ { " + qualifier + " extern int l; use(l); } return l; }"
        check("keycheck: GNU shadow declaration " + qualifier, not proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    text = "int l; int f(void){extern int l; return l;}"
    check("keycheck: extern is not local", not proof("f:l=n", {"src/a.c": text},
          {"src/a.c": text.replace("extern int l", "extern int n").replace("return l", "return n")})[0])
    for keyword in ["int", "auto", "__volatile__", "__extension__", "__attribute__"]:
        check("keycheck: refuses keyword destination " + keyword, not proof("f:l=" + keyword,
              {"src/a.c": "int f(int l){return l;}"}, {})[0])
    for path in ["bb2.ld", *nk.REGISTRIES]:
        check("keycheck: undeclared linker symbol permits local destination " + path, proof("f:l=n",
              {"src/a.c": "int f(int l){return l;}", path: "n = 0x80010000;"},
              {"src/a.c": "int f(int n){return n;}"})[0])
    for decl in ["int m[0x10];", "int m[0x1E6 - 0x1DE];", "u_char u3, m;", "u_char m, v3;"]:
        text = "/* game.h */\n#ifndef GAME_H\n#define GAME_H\nstruct T { " + decl + " };\n#endif\n"
        check("keycheck: guarded aggregate declaration " + decl, proof("T.m=n",
              {"include/game.h": text}, {"include/game.h": re.sub(r"\bm\b", "n", text)})[0])
    text = "#ifndef GAME_H\n#define GAME_H\n#if FEATURE\nstruct T { int m; };\n#endif\n#endif\n"
    check("keycheck: guard exemption keeps nested conditionals", not proof("T.m=n",
          {"include/game.h": text}, {"include/game.h": text.replace("int m", "int n")})[0])
    text = "#ifndef GAME_H\n#define GAME_H\n#ifndef FEATURE\n#define FEATURE\nstruct T { int m; };\n#endif\n#endif\n"
    check("keycheck: nested guard-shaped condition is still conditional", not proof("T.m=n",
          {"include/game.h": text}, {"include/game.h": text.replace("int m", "int n")})[0])
    text = "int f(int l){ return c ? 0, l : 1; }"
    check("keycheck: comma ternary use is not a GNU designator", proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    text = "struct T { int h; };\n#include <foo.h>\n"
    after = text.replace("int h", "int q")
    check("keycheck: member directive unchanged", proof("T.h=q",
          {"src/a.c": text}, {"src/a.c": after})[0])
    check("keycheck: member cannot rename directive token", not proof("T.h=q",
          {"src/a.c": text}, {"src/a.c": after.replace("foo.h", "foo.q")})[0])
    typedefs = ("typedef struct { int x; } S; typedef enum { E } Enum; "
                "typedef int (*Callback)(int p); typedef int A, B; "
                "typedef int __attribute__((aligned(4))) Aligned;")
    types = nk.declaration_types({"include/t.h": typedefs})
    check("keycheck: typedef names exclude members/constants/parameters",
          {"S", "Enum", "Callback", "A", "B", "Aligned"} <= types
          and not {"x", "E", "p", "aligned"} & types)
    text = "int f(int l){ int x; x * l; return l; }"
    check("keycheck: anonymous-struct member is not a type", proof("f:l=n",
          {"src/a.c": text, "include/t.h": typedefs},
          {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    text = "int f(int l){ { ExternalType *l; } return l; }"
    check("keycheck: unknown pointer declaration fails closed", not proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    for local_name in ["p", "v", "s", "dst", "out", "val"]:
        text = f"int f(int {local_name}){{return {local_name};}}"
        macro = f"#define M({local_name}) ({local_name})\n"
        check("keycheck: other-file macro parameter does not block " + local_name,
              proof("f:" + local_name + "=n", {"src/a.c": text, "include/m.h": macro},
                    {"src/a.c": re.sub(r"\b" + local_name + r"\b", "n", text)})[0])
    text = "struct T { int m; }; int f(struct T *p){return p->m;}"
    check("keycheck: unchanged member declaration is not applied", not proof("T.m=n",
          {"src/a.c": text}, {"src/a.c": text.replace("->m", "->n")})[0])
    check("keycheck: untouched local definition is not applied", not proof("f:l=n",
          {"src/a.c": local}, {})[0])
    for label, text, after, reason in [
            ("type destination", "typedef int n; struct T { int m; };", "typedef int n; struct T { int n; };", "typedef/type"),
            ("qualifier-only", "struct T { int x; const m; };", "struct T { int x; const n; };", "qualifier-only"),
            ("missing trailing semicolon", "struct T{int m;}; union U{int m;int n};",
             "struct T{int n;}; union U{int m;int n};", "final semicolon"),
            ("existing member clash", "struct T{int m;}; struct U{int m;int n;}; int f(struct U*u){return u->m;}",
             "struct T{int n;}; struct U{int m;int n;}; int f(struct U*u){return u->m;}", "aggregate declarations")]:
        ok, why = proof("T.m=n", {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: isolated member guard " + label, not ok and reason in why)
    for text in ["int l; int f(void){int l[sizeof l]; return 0;}",
                 "int l; int f(int a[sizeof l], int l){return l;}"]:
        after = text.replace("int l[", "int n[").replace("sizeof l", "sizeof n")
        after = after.replace(", int l", ", int n").replace("return l", "return n")
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: outer reference before binding/declarator " + text,
              not ok and ("own declarator" in why or "before" in why))
    for other in ["#define DECL(x) int x\nDECL(n);", "typedef int s32; s32 (n);"]:
        ok, why = proof("f:l=n", {"src/a.c": "int f(int l){return l;}", "src/b.c": other},
                        {"src/a.c": "int f(int n){return n;}"})
        check("keycheck: file-scope call-like declaration is visible " + other,
              not ok and "file scope" in why)
    header = "#ifndef T_H\n#define T_H\nstruct T{int m;};\n#endif\n"
    ok, why = proof("T.m=n", {"include/unused.h": header, "src/a.c": "int f(void){return 0;}"},
                    {"include/unused.h": header.replace("int m", "int n")}, compile_headers=False)
    check("keycheck: uncompiled header cannot certify rename", not ok and "never compiles" in why)
    ok, why = proof("T.m=n", {"include/t.h": header, "include/bridge.h": '#include "t.h"\n',
                    "src/a.c": '#include "bridge.h"\n'},
                    {"include/t.h": header.replace("int m", "int n")}, compile_headers=False)
    check("keycheck: include graph follows transitive headers", ok)
    ok, why = proof("T.m=n", {"include/t.h": header, "src/a.c": '#include "t.h"\n',
                    "src/b.c": "#define T_H\n"},
                    {"include/t.h": header.replace("int m", "int n")}, compile_headers=False)
    check("keycheck: unrelated TU guard definition does not replace cpp eligibility", ok)
    for label, base, target in [
            ("inactive include", {"src/a.c": '#if 0\n#include "evil.h"\n#endif\n',
                                  "include/evil.h": "struct T {int m;};\n"}, "include/evil.h"),
            ("command-line guard", {"src/a.c": '#include "evil.h"\n',
                                    "include/evil.h": "#ifndef mips\n#define mips\nstruct T {int m;};\n#endif\n"}, "include/evil.h"),
            ("angle-include decoy", {"src/dir/a.c": '#include <ev.h>\n',
                                     "src/dir/ev.h": "struct T {int m;};\n",
                                     "include/ev.h": "struct Actual {int x;};\n"}, "src/dir/ev.h")]:
        def omitted(tree, tu):
            out = '# 1 ' + json.dumps(tu) + '\nint fixture;\n'
            if label == "angle-include decoy":
                out += '# 1 "include/ev.h" 1\nstruct Actual {int x;};\n# 2 ' + json.dumps(tu) + ' 2\n'
            return out
        ok, why = proof("T.m=n", base, {target: base[target].replace("int m", "int n")},
                        compile_headers=False, cpp=omitted)
        check("keycheck: cpp rejects round-5 " + label, not ok and "never compiles" in why)
    header = "/* fixture */\n\nstruct T {int m;};\n"
    base = {"src/a.c": '#include "h.h"\n', "include/h.h": header}
    def emitted(tree, tu):
        return ('# 1 ' + json.dumps(tu) + '\n# 3 "include/h.h" 1\n'
                + tree["include/h.h"].splitlines()[2] + '\n# 2 ' + json.dumps(tu) + ' 2\n')
    check("keycheck: emitted header line is authoritative", proof("T.m=n", base,
          {"include/h.h": header.replace("int m", "int n")}, compile_headers=False, cpp=emitted)[0])
    for omitted_tree in ("base", "new"):
        def one_tree(tree, tu):
            return '' if tree.label == omitted_tree else emitted(tree, tu)
        ok, why = proof("T.m=n", base, {"include/h.h": header.replace("int m", "int n")},
                        compile_headers=False, cpp=one_tree)
        check("keycheck: cpp must emit both trees " + omitted_tree, not ok and "never compiles" in why)
    # Round 6: a surviving line is not evidence that its edited token survived.
    for label, definitions, expression, cpp_defs in [
            ("ID(DROP)", "#define ID(x) x\n#define DROP(x) 0\n", "ID(DROP)(s->m)", ""),
            ("digraph DROP", "%:define DROP(x) 0\n", "DROP(s->m)", ""),
            ("command-line DROP", "", "DROP(s->m)", " -DDROP(x)=0")]:
        text = ("struct T{int m;};\nstruct S{int m;};\n" + definitions
                + "int f(struct S*s){return " + expression + ";}\n")
        fixture_make = ("CPP := missing-keycheck-cpp\nCPP_FLAGS := -Iinclude\n"
                        "CPP_DEFS := -Dmips" + cpp_defs + "\n"
                        "\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | cc1\n")
        base = {"src/a.c": text, "Makefile": fixture_make}
        after = text.replace("struct T{int m", "struct T{int n").replace("s->m", "s->n")
        def dropped(tree, tu):
            output = ['# 1 ' + json.dumps(tu) + '\n']
            for content in tree[tu].splitlines():
                if content.startswith(("#", "%:")):
                    output.append('\n')
                elif content.startswith("int f"):
                    output.append("int f(struct S*s){return 0;}\n")
                else:
                    output.append(content + '\n')
            return "".join(output)
        ok, why = proof("T.m=n", base, {"src/a.c": after}, cpp=dropped)
        check("keycheck: cpp rejects discarded round-6 " + label,
              not ok and any(reason in why for reason in ("sentinel discarded", "macro DROP", "digraph", "chained function-like macro")))
        # Even an identical spelling supplied by expansion is not this edit.
        def produced(tree, tu):
            spelling = "m" if tree.label == "base" else "n"
            return dropped(tree, tu).replace("return 0", "return s->" + spelling)
        ok, why = proof("T.m=n", base, {"src/a.c": after}, cpp=produced)
        check("keycheck: cpp rejects expansion-origin spelling " + label,
              not ok and any(reason in why for reason in ("sentinel discarded", "macro DROP", "digraph", "chained function-like macro")))

    unused = "/* not included */\nstruct T{int m;};\n"
    for label, directive, reason in [
            ("line", '#line 2 "include/evil.h"\n', "line-control"),
            ("GNU line", '# 2 "include/evil.h"\n', "line-control"),
            ("macro marker", '#define HASH #\nHASH 2 "include/evil.h"\n', "include-stack")]:
        base = {"src/a.c": directive + "int x;\n", "include/evil.h": unused}
        def spoof(tree, tu):
            return ('# 1 ' + json.dumps(tu) + '\n# 2 "include/evil.h"\n'
                    + tree["include/evil.h"].splitlines()[1] + '\n')
        ok, why = proof("T.m=n", base, {"include/evil.h": unused.replace("int m", "int n")},
                        compile_headers=False, cpp=spoof)
        check("keycheck: cpp rejects round-6 spoof " + label, not ok and reason in why)
    for label, output in [
            ("first marker", '# 1 "include/evil.h"\n'),
            ("wrong pop", '# 1 "src/a.c"\n# 1 "include/h.h" 1\n# 2 "src/wrong.c" 2\n'),
            ("unflagged switch", '# 1 "src/a.c"\n# 1 "include/h.h"\n'),
            ("unfinished include", '# 1 "src/a.c"\n# 1 "include/h.h" 1\n'),
            ("malformed marker", '# 1 "src/a.c"\n# 1 "include/h.h" 9\n')]:
        try:
            nk.emitted_lines(output, nk.CppTree({"src/a.c": "", "include/h.h": ""}, "test"), "src/a.c")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: strict marker " + label, bool(why))
    tree = nk.CppTree({"src/a.c": "int x;\n"}, "test")
    startup = '# 0 "src/a.c"\n# 0 "<built-in>"\n# 0 "<command-line>"\n# 1 "src/a.c"\nint x;\n'
    eq("keycheck: legitimate cpp startup prelude", nk.emitted_lines(startup, tree, "src/a.c"),
       {("src/a.c", 1): ["int", "x", ";"]})
    eq("keycheck: exact real mipsel cpp 12.4 system preamble", nk.emitted_lines(
       cpp_preamble("src/a.c") + "int x;\n", tree, "src/a.c"), {("src/a.c", 1): ["int", "x", ";"]})
    system = cpp_preamble("src/a.c").replace('# 0 "<command-line>" 2', 'int system_decl;\n# 0 "<command-line>" 2')
    eq("keycheck: flagged system push may emit text", nk.emitted_lines(system + "int x;\n", tree, "src/a.c"),
       {("src/a.c", 1): ["int", "x", ";"],
        ("/usr/mipsel-linux-gnu/include/stdc-predef.h", 1): ["int", "system_decl", ";"]})
    empty_external = cpp_preamble("src/a.c").replace(' 1 3 4', ' 1')
    eq("keycheck: empty external push accepted", nk.emitted_lines(empty_external + "int x;\n", tree, "src/a.c"),
       {("src/a.c", 1): ["int", "x", ";"]})
    try:
        nk.emitted_lines(empty_external.replace('# 0 "<command-line>" 2', 'int unexpected;\n# 0 "<command-line>" 2'), tree, "src/a.c")
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: unflagged external push must emit no text", "outside a source file" in why)
    for header in ("/usr/mipsel-linux-gnu/include/other.h", "include/fake-system.h"):
        try:
            nk.emitted_lines(cpp_preamble("src/a.c").replace("/usr/mipsel-linux-gnu/include/stdc-predef.h", header), tree, "src/a.c")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: flag-3 push only accepts compiler predef " + header, "unsupported cpp system header" in why)
    for hazard in ("__has_include(x)", "a ## b", "%:define X 1"):
        output = cpp_preamble("src/a.c").replace('# 0 "<command-line>" 2', hazard + '\n# 0 "<command-line>" 2')
        try:
            nk.emitted_lines(output, tree, "src/a.c")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: system output hazard scan " + hazard, bool(why))

    simple = {"src/a.c": "struct T{int m;};\nint f(struct T*p){return p->m;}\n"}
    changed_simple = {"src/a.c": simple["src/a.c"].replace("int m", "int n").replace("->m", "->n")}
    for condition, reason in [("discarded", "sentinel discarded"), ("string", "stringified or pasted"),
                              ("char", "stringified or pasted"), ("paste", "stringified or pasted"),
                              ("wrong-line", "wrong source line")]:
        def sentinel_bad(tree, tu):
            out = fake_cpp(tree, tu)
            if tree.label.endswith("-sentinel"):
                name = re.findall(r'__kc_s\w+__', tree[tu])[-1]
                if condition == "discarded":
                    out = out.replace(name, "unrelated")
                elif condition in ("string", "char", "paste"):
                    extra = {'string': '"' + name + '"', 'char': "'" + name + "'", 'paste': name + "suffix"}[condition]
                    out += '# 2 ' + json.dumps(tu) + '\n' + extra + ';\n'
                else:
                    out += '# 3 ' + json.dumps(tu) + '\n' + name + ';\n'
            return out
        ok, why = proof("T.m=n", simple, changed_simple, cpp=sentinel_bad)
        check("keycheck: sentinel condition " + condition, not ok and reason in why)
    # An included header must prove its sentinel in every TU that emits its line.
    header_base = {"src/a.c": '#include "h.h"\n', "src/b.c": '#include "h.h"\n',
                   "include/h.h": "struct T{int m;};\n"}
    def one_header_tu_drops(tree, tu):
        out = fake_cpp(tree, tu)
        if tree.label.endswith("-sentinel") and tu.endswith("b.c"):
            for name in re.findall(r'__kc_s\w+__', tree["include/h.h"]):
                out = out.replace(name, "discarded")
        return out
    ok, why = proof("T.m=n", header_base, {"include/h.h": "struct T{int n;};\n"},
                    compile_headers=False, cpp=one_header_tu_drops)
    check("keycheck: sentinel must survive every header TU", not ok and "sentinel discarded" in why)

    for side in ("base", "new"):
        local_text = "int f(int l){\nreturn l;\nSYSTEM_CALL(p->l);\n}\n"
        local_after = local_text.replace("int l", "int fresh").replace("return l", "return fresh")
        def local_survival_drop(tree, tu):
            out = fake_cpp(tree, tu)
            if tree.label.startswith(side):
                out = re.sub(r'SYSTEM_CALL\(p->\w+\)', '0', out)
            return out
        ok, why = proof("f:l=fresh", {"src/a.c": local_text}, {"src/a.c": local_after}, cpp=local_survival_drop)
        check("keycheck: function-wide local survival " + side,
              not ok and "local survival, " + side in why)
        # A separate untouched line produces a name, without source-name removal.
        producing = "int f(int l){\nreturn l;\nSYSTEM_NAME;\n}\n"
        def produced_on_side(tree, tu):
            return fake_cpp(tree, tu).replace("SYSTEM_NAME", "fresh") if tree.label.startswith(side) else fake_cpp(tree, tu)
        ok, why = proof("f:l=fresh", {"src/a.c": producing},
                        {"src/a.c": producing.replace("int l", "int fresh").replace("return l", "return fresh")}, cpp=produced_on_side)
        check("keycheck: function-wide local capture " + side,
              not ok and "macro expansion produces or captures" in why and "(" + side + ")" in why)

    for label, macro, struct_type in [("cheat r4", "DROP", "T"), ("code P1", "STR", "T"),
                                      ("code P3", "DROP", "T"), ("code P5", "DROP", "S")]:
        text = ("struct T{int m;int q;}; struct S{int m;};\n#define DROP(x) 0\n"
                "#define ID(f) f\n#define STR(x) #x\n#define KEEP(x) (s->q + x)\n"
                "int m,n;\nint f(struct " + struct_type + "*s){return ID(" + macro + ")\n"
                "(s->m) + ID(KEEP)(0\n+ m + n); }\n")
        after = text.replace("struct T{int m", "struct T{int n").replace("(s->m)", "(s->n)")
        def multiline_cpp(tree, tu):
            # Real shape: dropped/stringified call starts on the preceding line;
            # KEEP's argument expansion supplies same-spelling twins on line 8.
            lines = tree[tu].splitlines()
            argument = lines[7].split(" + ID(KEEP)")[0]
            value = json.dumps(argument) if macro == "STR" else "0"
            return (cpp_preamble(tu) + lines[0] + '\n# 6 ' + json.dumps(tu) + '\nint m,n;\n'
                    '# 7 ' + json.dumps(tu) + '\nint f(struct ' + struct_type + '*s){return ' + value + '\n'
                    '# 8 ' + json.dumps(tu) + '\n+(s->q + 0 + m + n); }\n')
        ok, why = proof("T.m=n", {"src/a.c": text}, {"src/a.c": after}, cpp=multiline_cpp)
        check("keycheck: round-8 chained macro guard " + label, not ok and "chained function-like macro" in why)
        with patch.object(nk, "reject_macro_arguments", lambda *args, **kwargs: None), \
                patch.object(nk, "reject_unexpanded_macro_arguments", lambda *args, **kwargs: None), \
                patch.object(nk, "macro_argument_indices", lambda *args, **kwargs: set()):
            ok, why = proof("T.m=n", {"src/a.c": text}, {"src/a.c": after}, cpp=multiline_cpp)
        check("keycheck: round-8 sentinel proof independent of guard " + label,
              not ok and ("stringified or pasted" in why if macro == "STR" else "sentinel discarded" in why))
    for aggregate, callee in [("union", "MEMBER"), ("struct", "MEMBER"), ("union", "DROP")]:
        text = ("#define MEMBER(p,x) ((p)->x)\n#define DROP(x) 0\n#define OPEN " + callee + " (\n"
                "#define RP )\n" + aggregate + " S{int l;int n;};\n"
                "int f(int l," + aggregate + " S*s){return (0 RP + OPEN s,l+0)+l;}\n")
        after = text.replace("f(int l", "f(int n").replace("OPEN s,l+0", "OPEN s,n+0").replace(")+l;", ")+n;")
        def opener_cpp(tree, tu):
            lines = tree[tu].splitlines()
            body = lines[-1]
            binding = re.search(r'f\(int (\w+)', body)[1]
            argument = re.search(r'OPEN s,(\w+)\+0', body)[1]
            final = re.search(r'\)\+(\w+);', body)[1]
            expansion = '((s)->' + argument + '+0)' if callee == "MEMBER" else '0'
            return (cpp_preamble(tu) + '\n'.join(lines[:4]) + '\n# 5 ' + json.dumps(tu) + '\n' + lines[4] + '\n'
                    + 'int f(int ' + binding + ',' + aggregate + ' S*s){return (0)+' + expansion + '+' + final + ';}\n')
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after}, cpp=opener_cpp)
        check("keycheck: round-9 object-like opener " + aggregate + " " + callee,
              not ok and "object-like macro call opener" in why)
    for body in ("MEMBER", "ALIAS", "MEMBER ("):
        records = nk.macro_analysis('#define MEMBER(p,x) ((p)->x)\n#define ALIAS MEMBER\n#define OPEN ' + body)[0]
        try:
            nk.closure_macro_checks(records, {"f:l": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: object-like opener chains " + body, "call opener" in why)
    b2 = "union T{int m;int k;};\n#define m k\nint f(union T*t){return t->m;}\n"
    for label, base in [("f2", {"src/a.c": b2}),
                        ("f2h", {"src/a.c": b2.replace('#define m k', '#include "m.h"'), "include/m.h": "#define m k\n"}),
                        ("f2f", {"src/a.c": b2.replace('#define m k', '#define m(x) (x)')})]:
        after = base["src/a.c"].replace("int m;", "int n;").replace("->m", "->n")
        def rewritten_cpp(tree, tu):
            return fake_cpp(tree, tu).replace("->m", "->k") if label != "f2f" else fake_cpp(tree, tu)
        ok, why = proof("T.m=n", base, {"src/a.c": after}, cpp=rewritten_cpp)
        check("keycheck: round-9 old member macro name " + label, not ok and "#define name" in why)
        if label != "f2f":
            actual_macro_uses = nk.macro_uses
            with patch.object(nk, "macro_uses", lambda text, name, body_only=False, name_only=False:
                              False if name_only else actual_macro_uses(text, name, body_only, name_only)), \
                    patch.object(nk, "closure_macro_checks", lambda *args: None):
                ok, why = proof("T.m=n", base, {"src/a.c": after}, cpp=rewritten_cpp)
            check("keycheck: original spelling check independently rejects " + label,
                  not ok and "original spelling/count" in why)
    # Round 10: fixtures retain real -dD definitions and the code cpp emits.
    round10 = []
    for probe, spelling, punctuation in [("j14", "ARROW", "->"), ("j14d", "DOT", ".")]:
        text = ("union S { int l; int n; };\n#define " + spelling + "() " + punctuation +
                "\nint f(int l, union S *s) { return s " + spelling + "() l + l; }\n")
        round10.append((probe, {"src/a.c": text}, "s " + spelling + "() l", "s " + spelling + "() n"))
    for probe, aggregate, callee in [("j1", "union", "MEMBER"), ("j1b", "struct", "MEMBER"),
                                     ("OPENF-DROP", "union", "DROP"), ("k2", "union", "MEMBER")]:
        text = (aggregate + " S { int l; int n; };\n#define MEMBER(p,x) ((p)->x)\n"
                "#define DROP(x) 0\n#define RP )\n")
        if probe != "k2":
            text += "#define OPENF() " + callee + "(\n"
        text += "int f(int l, " + aggregate + " S *s) { return (0 RP + OPENF() s, l + 0) + l; }\n"
        files = {"src/a.c": text}
        if probe == "k2":
            files["Makefile"] = ("CPP := fake-cpp\nCPP_FLAGS := -E -Iinclude\n"
                                 "CPP_DEFS := '-DOPENF()=MEMBER('\n"
                                 "\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | cc1\n")
        round10.append((probe, files, "s, l + 0", "s, n + 0"))
    for probe, directory, macro in [("j2", "include", "MEMBER"), ("j2b", "src", "MEMBER"),
                                     ("j13", "include", "ARROW")]:
        macro_use = "MEMBER(s, l + 0)" if macro == "MEMBER" else "s ARROW l"
        text = ('#include "m.inc"\nunion S { int l; int n; };\n'
                'int f(int l, union S *s) { return ' + macro_use + ' + l; }\n')
        files = {"src/a.c": text, directory + "/m.inc":
                 "#define MEMBER(p,x) ((p)->x)\n" if macro == "MEMBER" else "#define ARROW ->\n"}
        round10.append((probe, files, macro_use, macro_use.replace(" l", " n")))

    def namespace_cpp(tree, tu):
        out = fake_cpp(tree, tu)
        expanded = []
        for line in out.splitlines():
            if not line.lstrip().startswith("#"):
                line = re.sub(r'\bARROW\(\)', '->', line)
                line = re.sub(r'\bDOT\(\)', '.', line)
                line = re.sub(r'\bARROW\b', '->', line)
                line = re.sub(r'\bRP\b', ')', line)
                definitions = nk.command_line_macros(tree.get("Makefile")) + "\n" + tree[tu]
                callee = "DROP" if "#define OPENF() DROP(" in definitions else "MEMBER"
                line = re.sub(r'\bOPENF\(\)', callee + '(', line)
                line = re.sub(r'\bMEMBER\(\s*s,\s*(\w+)\s*\+\s*0\)', r'((s)->\1 + 0)', line)
                line = re.sub(r'\bDROP\([^)]*\)', '0', line)
            expanded.append(line)
        return "\n".join(expanded) + "\n"

    for probe, files, old_use, new_use in round10:
        after = files["src/a.c"].replace("f(int l,", "f(int n,").replace(old_use, new_use).replace(" + l;", " + n;")
        ok, why = proof("f:l=n", files, {"src/a.c": after}, cpp=namespace_cpp)
        reason = ("call opener" if probe in ("j1", "j1b", "OPENF-DROP", "k2") else
                  "argument of function-like macro" if probe in ("j2", "j2b") else "adjacent to cpp macro")
        check("keycheck: round-10 macro namespace " + probe, not ok and reason in why)
        # Namespace proof independently kills keeping macros; discarders are
        # rejected by the sentinel proof even with all static guards disabled.
        with patch.object(nk, "closure_macro_checks", lambda *a: None), \
                patch.object(nk, "cpp_source_guards", lambda *a: None), \
                patch.object(nk, "reject_macro_arguments", lambda *a, **kw: None), \
                patch.object(nk, "reject_unexpanded_macro_arguments", lambda *a, **kw: None):
            ok, why = proof("f:l=n", files, {"src/a.c": after}, cpp=namespace_cpp)
        check("keycheck: round-10 primary cpp namespace " + probe,
              not ok and ("namespace" in why if probe != "OPENF-DROP" else "sentinel discarded" in why))

    for side in ("base", "new"):
        files = round10[0][1]
        text = files["src/a.c"]
        after = text.replace("f(int l,", "f(int n,").replace("ARROW() l", "ARROW() n").replace(" + l;", " + n;")
        def one_side_namespace(tree, tu):
            return namespace_cpp(tree, tu) if tree.label.startswith(side) else fake_cpp(tree, tu)
        with patch.object(nk, "closure_macro_checks", lambda *a: None), \
                patch.object(nk, "cpp_source_guards", lambda *a: None):
            ok, why = proof("f:l=n", files, {"src/a.c": after}, cpp=one_side_namespace)
        check("keycheck: cpp namespace proof independently checks " + side, not ok and "namespace" in why)

    # Namespace checks follow the ordered output stream across source lines.
    for kind, prefix, suffix, scope in [
            ("local", "s->", ";", "f"), ("local", "s.", ";", "f"),
            ("local", "struct ", " *p;", "f"), ("local", "union ", " *p;", "f"),
            ("local", "enum ", " *p;", "f"), ("local", "", ": ;", "f"),
            ("member", "return ", ";", "T"), ("member", "struct U {int ", ";};", "T")]:
        text = prefix + "\n__kc_test__" + suffix + "\n"
        if kind == "local":
            text = "int f(void){" + text + "}"
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
        try:
            nk.sentinel_namespaces(output, {"__kc_test__": (kind, scope)})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: namespace guard independently rejects " + kind + " " + prefix,
              "namespace" in why)
    for prefix, suffix, kind, scope in [("s->", ";", "member", "T"),
            ("struct T {int ", ";};", "member", "T"),
            ("typedef struct {int ", ";} T;", "member", "T"),
            ("c ? (int) ", " : 0;", "local", "f")]:
        text = prefix + "\n__kc_test__" + suffix + "\n"
        if kind == "local":
            text = "int f(void){" + text + "}"
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
        nk.sentinel_namespaces(output, {"__kc_test__": (kind, scope)})
        check("keycheck: namespace positive " + prefix, True)
    # Macro records must come from emitted -dD records, not extensions or
    # inactive raw definitions. Both trees' names and bodies are guarded.
    output = nk.emitted_lines(cpp_preamble("src/a.c") + "int f(void){return 0;}\n",
                             {"src/a.c": "#define HIDDEN l\nint f(void){return 0;}\n"}, "src/a.c")
    check("keycheck: macro authority excludes un-emitted raw definitions", not output.macros)
    for params in ("", "()"):
        records = nk.macro_analysis("#define MEMBER(p,x) ((p)->x)\n#define OPEN" + params + " MEMBER(\n")[0]
        try:
            nk.closure_macro_checks(records, {"f:l": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: cpp call-opener guard " + (params or "object"), "call opener" in why)

    # Old/new name and body checks use emitted records from arbitrary suffixes,
    # including system predefinitions, rather than a .c/.h file inventory.
    for side in ("base", "new"):
        for definition, reason in [("#define m k", "#define name"),
                ("#define m(x) (x)", "#define name"), ("#define n k", "#define name"),
                ("#define OTHER m", "macro body"), ("#define OTHER n", "macro body")]:
            text = '#include "m.inc"\nunion T {int m;int k;};\nint f(union T*t){return t->m;}\n'
            files = {"src/a.c": text, "include/m.inc": "/* definitions supplied by cpp */\n"}
            after = text.replace("int m;", "int n;").replace("->m", "->n")
            def records_cpp(tree, tu):
                out = fake_cpp(tree, tu)
                if tree.label == side:
                    out = out.replace('# 1 "include/m.inc" 1\n', '# 1 "include/m.inc" 1\n' + definition + "\n")
                return out
            ok, why = proof("T.m=n", files, {"src/a.c": after}, cpp=records_cpp)
            check("keycheck: dD-only closure records " + side + " " + definition, not ok and reason in why)
    for side in ("base", "new"):
        def system_macro_cpp(tree, tu):
            out = fake_cpp(tree, tu)
            if tree.label == side:
                out = out.replace('# 0 "<command-line>" 2', '#define m k\n# 0 "<command-line>" 2')
            return out
        text = 'union T{int m;int k;};\nint f(union T*t){return t->m;}\n'
        ok, why = proof("T.m=n", {"src/a.c": text},
                        {"src/a.c": text.replace("int m;", "int n;").replace("->m", "->n")}, cpp=system_macro_cpp)
        check("keycheck: dD system macro old name " + side, not ok and "#define name" in why)

    # Round 11: all analyses must see the token stream actually compiled.
    for declaration in ["struct Unk80101EC8Record {int unk_4;};",
                        "typedef struct {int unk_4;} Unk80101EC8Record;",
                        "typedef struct Tag {int unk_4;} Unk80101EC8Record;",
                        "typedef struct Tag Unk80101EC8Record; struct Tag {int unk_4;};"]:
        for scope in ("Unk80101EC8Record", "GoodRec"):
            after = declaration.replace("Unk80101EC8Record", "GoodRec").replace("unk_4", "hp")
            ok, why = proof("Unk80101EC8Record=GoodRec," + scope + ".unk_4=hp",
                            {"src/a.c": declaration}, {"src/a.c": after})
            check("keycheck: K1 global type/member " + scope + " " + declaration, ok)

    def scope_cpp(tree, tu):
        # Emitted -dD definitions are retained; only compiled code is expanded.
        expanded_tree = dict(tree)
        expanded_tree[tu] = tree[tu].replace('#include UH', '#include "u.inc"')
        out = fake_cpp(expanded_tree, tu)
        replacements = {"UNION": "union", "AGG": "struct {", "EOPEN": "enum {",
                        "END": "}", "CLOSE": "}", "OPEN": "{"}
        return "\n".join(line if line.lstrip().startswith("#") else
                         re.sub(r'\b(?:UNION|AGG|EOPEN|END|CLOSE|OPEN)\b',
                                lambda m: replacements[m[0]], line)
                         for line in out.splitlines()) + "\n"

    for probe in ("m1", "m1b", "m1c", "d5", "d6"):
        other = "union U2 {int m;int n;};\n"
        prefix = ""
        if probe in ("m1", "m1c"):
            prefix = "#define UNION union\n"
            other = "UNION U2 {int m;int n;};\n" if probe == "m1" else "typedef UNION {int m;int n;} U2;\n"
        files = {}
        if probe in ("m1b", "d5"):
            prefix = '#include "u.inc"\n' if probe == "m1b" else '#define UH "u.inc"\n#include UH\n'
            files["include/u.inc"] = other
            other = ""
        if probe == "d6":
            other = "UNION U2 {int m;int n;};\n"
            files["Makefile"] = ("CPP := fake-cpp\nCPP_FLAGS := -E -Iinclude\nCPP_DEFS := -DUNION=union\n"
                                 "\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | cc1\n")
        text = prefix + "struct T {int m;};\n" + other + "int f(struct T*t,union U2*u){return t->m + u->m;}\n"
        files["src/a.c"] = text
        after = text.replace("struct T {int m;}", "struct T {int n;}").replace("->m", "->n")
        ok, why = proof("T.m=n", files, {"src/a.c": after}, cpp=scope_cpp)
        check("keycheck: round-11 competing aggregate " + probe, not ok)
        with patch.object(nk, "closure_macro_checks", lambda *a: None), \
                patch.object(nk, "cpp_source_guards", lambda *a: None):
            ok, why = proof("T.m=n", files, {"src/a.c": after}, cpp=scope_cpp)
        check("keycheck: cpp member freshness independently rejects " + probe,
              not ok and "fresh in cpp aggregates" in why)
    text = "struct T{int m;}; union U2{int m;int q;}; int f(struct T*t,union U2*u){return t->m+u->m;}"
    check("keycheck: m1z other aggregate without destination control", proof("T.m=n",
          {"src/a.c": text}, {"src/a.c": text.replace("T{int m", "T{int n").replace("t->m", "t->n")})[0])
    # Census every TU, even when the edit is only in one .c file.
    text = "struct T{int m;}; int f(struct T*t){return t->m;}"
    ok, why = proof("T.m=n", {"src/a.c": text, "src/b.c": '#include "u.inc"\n',
                   "include/u.inc": "union U2{int m;int n;};"},
                   {"src/a.c": text.replace("int m", "int n").replace("->m", "->n")}, cpp=scope_cpp)
    check("keycheck: member cpp freshness includes untouched TU", not ok and "cpp aggregates" in why)

    for probe, macro, code in [
            ("a1", "#define AGG struct {\n#define END }\n", "AGG struct T *l; END x;"),
            ("a1c", "#define AGG struct {\n#define END }\n", "AGG struct T *l; END x;"),
            ("a2", "#define EOPEN enum {\n#define END }\n", "EOPEN l END x;")]:
        text = macro + "struct T{int x;};\nint f(void){" + code + "return 0;}\n"
        after = text if probe == "a1c" else text.replace("*l;", "*n;").replace("EOPEN l", "EOPEN n")
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after}, cpp=scope_cpp)
        check("keycheck: brace-macro aggregate/enum " + probe, not ok)
    for keyword in ("struct", "union", "enum"):
        text = "int f(void){" + keyword + " {int __kc_test__; } x;}" if keyword != "enum" else "int f(void){enum {__kc_test__} x;}"
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
        try:
            nk.sentinel_namespaces(output, {"__kc_test__": ("local", "f")})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: local namespace independently excludes " + keyword + " body", "namespace" in why)
    for word in ("{", "}", "struct", "union", "enum"):
        try:
            nk.closure_macro_checks(nk.macro_analysis("#define SCOPE " + word)[0], {"f:l": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: closure macro scope syntax " + word, "brace/aggregate scope" in why)

    for probe, text, extra in [
            ("m2", "typedef int T; int l; int g(int a), n; int f(int T){T * l; return sizeof l;}", {}),
            ("m2b", 'typedef int T; int l; #include "u.inc"\nint f(int T){T * l; return sizeof l;}',
             {"include/u.inc": "enum {n=5};\n"}),
            ("m2c", "#define CLOSE }\n#define OPEN {\nint l; int g(int a), n; int f(int l){int x=0; CLOSE int h(void) OPEN return l;}", {}),
            ("m2e", "typedef int T; int l; int f(int T){T * l; return sizeof l;}", {}),
            ("m2f", "int g(int a), n; int f(int l){return l;}", {}),
            ("m5b", "int g(int a), n; int h(void){return 0;} int f(int l){return l;}", {})]:
        after = text.replace("int f(int l)", "int f(int n)").replace("T * l", "T * n").replace("sizeof l", "sizeof n").replace("return l", "return n")
        check("keycheck: cpp local binding/freshness " + probe, not proof("f:l=n",
              {"src/a.c": text, **extra}, {"src/a.c": after}, cpp=scope_cpp)[0])
    # Isolate shadowed typedefs from the outside-global check (invalid code is
    # deliberately sufficient here: it must not become a false binding proof).
    text = "typedef int T; int f(int T){T * l; return sizeof l;}"
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
    try:
        nk.cpp_scope_checks(output, {"f:l": "n"}, {}, "base")
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: shadowed typedef cpp guard independently rejects", "shadowed typedef" in why)
    text = '#include "u.inc"\nint f(int l){return l;}\n'
    with patch.object(nk, "local_freshness", lambda *a: None):
        ok, why = proof("f:l=n", {"src/a.c": text, "include/u.inc": "enum {n=5};\n"},
                        {"src/a.c": text.replace("int l", "int n").replace("return l", "return n")}, cpp=scope_cpp)
    check("keycheck: local freshness sees inc enum independently", not ok and "outside f in cpp" in why)
    for text in ("int g(int a), n; int f(int l){return l;}",
                 "int g(int a), n; int h(void){return 0;} int f(int l){return l;}"):
        visible, _ = nk.file_scope_tokens(text)
        check("keycheck: comma prototype cannot hide later global", "n" in visible)
    text = "int legacy(n) int n; {return n;} int f(int l){return l;}"
    visible, _ = nk.file_scope_tokens(text)
    check("keycheck: genuine K&R parameter still excluded", "n" not in visible)
    # Mid-file -dD lines consume their original line, even without a new marker.
    text = "int x;\n#define MID 1\nint y;\n"
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
    check("keycheck: mid-file dD define advances source line",
          output.get(("src/a.c", 3)) == ["int", "y", ";"] and ("src/a.c", 2) not in output)
    for definition in ("#define OPEN MEMBER x", "#define OPEN ("):
        records = nk.macro_analysis("#define MEMBER(p,x) ((p)->x)\n" + definition)[0]
        try:
            nk.closure_macro_checks(records, {"f:l": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: separate macro opener " + definition, "call opener" in why)
    for definition in ("#define m k", "#define m(x) (x)"):
        try:
            nk._scoped_plan({"src/a.c": "struct T{int m;};\n" + definition}, {"T.m": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: static macro-name check independent " + definition, "#define name" in why)
    try:
        nk.scoped_plan({"Makefile": "CPP := fake\nCPP_FLAGS := -E\nCPP_DEFS :=\n\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | cc1\n",
                        "include/.keycheck-cpp-defs.h": "", "src/a.c": "int f(int l){return l;}"}, {"f:l": "n"})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: reserved macro proof path independently fails", "reserved cpp macro proof path" in why)
    for side in ("base", "new"):
        text = "int f(int l){\nreturn l; LAST_MACRO;}\n"
        after = text.replace("int l", "int n").replace("return l", "return n")
        def last_line_cpp(tree, tu):
            out = fake_cpp(tree, tu)
            return out.replace("LAST_MACRO", "n" if side == "base" else "l") if tree.label.startswith(side) else out.replace("LAST_MACRO", "0")
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after}, cpp=last_line_cpp)
        check("keycheck: function-wide last-line capture " + side,
              not ok and "macro expansion produces or captures" in why)

    text = "int f(int l){return l;}\n#define l 1\n"
    ok, why = proof("f:l=n", {"src/a.c": text},
                    {"src/a.c": text.replace("int l", "int n").replace("return l", "return n")})
    check("keycheck: d1 macro after f deliberately fails closed", not ok and "#define name" in why)

    # File-scope freshness cannot depend on the old K&R exclusion heuristic.
    for side in ("base", "new"):
        text = "int g(int a), n; int f(int " + ("l" if side == "base" else "n") + "){return " + ("l" if side == "base" else "n") + ";}"
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
        with patch.object(nk, "file_scope_tokens", lambda *a: (set(), set())):
            try:
                nk.cpp_scope_checks(output, {"f:l": "n"}, {}, side)
                why = ""
            except ValueError as e:
                why = str(e)
        check("keycheck: cpp ordinary outside f independently checks " + side, "outside f in cpp" in why)
    text = "struct l; int f(int l){return l;}"
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
    nk.cpp_scope_checks(output, {"f:l": "n"}, {}, "base")
    check("keycheck: tag outside f is not an ordinary binding", True)

    text = '#include "u.inc"\nstruct T{int m;}; int f(struct T*t){return t->m;}'
    check("keycheck: cpp competing freshness requires both member spellings", proof("T.m=n",
          {"src/a.c": text, "include/u.inc": "union U2{int n;int q;};"},
          {"src/a.c": text.replace("int m", "int n").replace("->m", "->n")})[0])

    text = "struct S{int l;}; struct S s={l:1}; int f(int l){return l;}"
    check("keycheck: outside-f member designator is not an ordinary name", proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": text.replace("f(int l)", "f(int n)").replace("return l", "return n")})[0])

    # Follow-up 16: harmless balanced blocks and unrelated local names pass.
    for body in ("{ x++; }", "do { x++; } while (0)", "({ x; })"):
        nk.closure_macro_checks(nk.macro_analysis("#define BLOCK(x) " + body)[0], {"f:l": "n"})
        check("keycheck: balanced macro blocks allowed " + body, True)
    text = "int f(int l){return l;} int g(int n){return n;}"
    check("keycheck: another function's local destination is harmless", proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": text.replace("f(int l)", "f(int n)").replace("return l", "return n")})[0])
    # Declaration census ignores tags, members, prototype parameters and uses.
    for text, names in [
            ("int obj; int g(int n); int f(int l){return l;}", {"obj", "g", "f"}),
            ("struct n{int l;}; struct n obj;", {"obj"}),
            ("int (*n)(int l);", {"n"}), ("int g(int a),n;", {"g", "n"}),
            ("typedef int n;", {"n"}), ("enum E{n=5,q=7};", {"n", "q"}),
            ("int a=helper(n);", {"a"}), ("int g(void){int n; return n;}", {"g"})]:
        ts = [m[0] for m in nk.code_spans(text)]
        types = nk.declaration_types({"src/a.c": text})
        actual = nk.cpp_file_declarations(ts, types, nk.cpp_functions(ts, types), {"l", "n"})
        check("keycheck: file ordinary declaration census " + text, actual == names)

    # K3: exercise the absolute-path branch even on Windows with a dict fixture.
    text = "int f(void){return 0;}"
    tree = nk.CppTree({"src/a.c": text}, "fixture")
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, tree, "src/a.c")
    check("keycheck: CppTree real preamble fixture", ("src/a.c", 1) in output)
    with patch.object(nk.Path, "is_absolute", return_value=True):
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, {"src/a.c": text}, "src/a.c")
    check("keycheck: plain dict absolute marker path cannot crash", ("src/a.c", 1) in output)

    for body, reason in [
            ("goto __kc_test__; l: return l;", "goto"),
            ("(c ? x); (__kc_test__: 0);", "closed sibling frame"),
            ("c ? (__kc_test__: 0) : 1;", "nesting blind colon"),
            ('_Pragma("kc ?") __kc_test__: return l;', "pragma operator"),
            ("\n#pragma kc ?\n__kc_test__: return l;", "pragma directive")]:
        text = "int f(int l){" + body + "}"
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
        try:
            nk.sentinel_namespaces(output, {"__kc_test__": ("local", "f")})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: local namespace independent " + reason, "namespace" in why)
        if "pragma" in reason:
            check("keycheck: pragma metadata absent " + reason, "?" not in [tok for _, tok in output.stream])
    # Fake output shape for DROP(?) _Pragma: cpp emits a pragma directive,
    # then resumes the same source line at the label.
    text = '#define DROP(x)\nint f(int l){DROP(?) _Pragma("kc ?") l: return l;}\n'
    after = text.replace("int l", "int n").replace("l:", "n:").replace("return l", "return n")
    def pragma_cpp(tree, tu):
        line = tree[tu].splitlines()[1]
        line = line.replace('DROP(?) _Pragma("kc ?")', '\n#pragma kc ?\n# 2 "' + tu + '"\n')
        return cpp_preamble(tu) + '#define DROP(x)\n# 2 ' + json.dumps(tu) + '\n' + line + '\n'
    ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after}, cpp=pragma_cpp)
    check("keycheck: DROP pragma label rename fails", not ok and "namespace" in why)

    text = "struct T{int __kc_t__;}; struct U{int __kc_u__;};"
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
    nk.sentinel_namespaces(output, {"__kc_t__": ("member", "T"), "__kc_u__": ("member", "U")})
    check("keycheck: multi-pair member owners positive", True)
    try:
        nk.sentinel_namespaces(output, {"__kc_t__": ("member", "U"), "__kc_u__": ("member", "T")})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: multi-pair ownership cannot pool aggregate names", "namespace" in why)
    for body in ("return l NEXT;", "return n NEXT;", "return FN l;"):
        text = "#define NEXT +0\n#define FN(x) (x)\nint f(int " + ("n" if "n NEXT" in body else "l") + "){" + body + "}"
        tree = nk.CppTree({"src/a.c": text}, "fixture")
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, tree, "src/a.c")
        try:
            nk.cpp_source_guards(output, tree, {"f:l": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: independent macro adjacency " + body, "adjacent to cpp macro" in why)
    for text, fails in [
            ("#define FN(x) (x)\nstruct T{int m;}; int f(struct T*t){return FN(t->n);}", False),
            ("#define FN(x) (x)\nstruct T{int m;}; int f(struct T*t){return FN(t->m);}", False),
            ("#define FN(x) (0)\nstruct T{int m;}; int f(struct T*t){return FN(t->m);}", True)]:
        tree = nk.CppTree({"src/a.c": text}, "fixture")
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, tree, "src/a.c")
        try:
            nk.cpp_source_guards(output, tree, {"T.m": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: cpp member argument scan: dropped argument fails, plain kept / edited one passes ("
              + text.split("return ")[1] + text.split(")")[1][:4] + ")",
              ("argument of function-like macro" in why) == fails)
    for param in ("l", "n"):
        text = '#include "params.h"\nint f(int l){return l;}\n'
        check("keycheck: cpp guards exclude macro own parameter " + param, proof("f:l=n",
              {"src/a.c": text, "include/params.h": "#define COPY(" + param + ") (" + param + ")\n"},
              {"src/a.c": text.replace("int l", "int n").replace("return l", "return n")})[0])
    # scoped-plan-bindings: a #define body in a .c nothing includes cannot reach f;
    # the same body in an included header still fails.
    text = "int f(int l){return l;}\n"
    other = "#define GET(x) { int l = 32 - (x); x = l; }\n"
    check("keycheck: local pair ignores a #define body in an unincluded .c", proof("f:l=n",
          {"src/a.c": text, "src/b.c": other}, {"src/a.c": text.replace("l", "n")})[0])
    ok, why = proof("f:l=n", {"src/a.c": '#include "b.h"\n' + text, "include/b.h": other},
                    {"src/a.c": '#include "b.h"\n' + text.replace("l", "n")})
    check("keycheck: local pair still refuses a #define body in an included header",
          not ok and "#define body" in why)
    ok, why = proof("f:l=n", {"src/a.c": '#include "b.c"\n' + text, "src/b.c": other},
                    {"src/a.c": '#include "b.c"\n' + text.replace("l", "n")})
    check("keycheck: local pair still refuses a #define body in an included .c",
          not ok and "#define body" in why)
    # A local passed to a plainly expanding macro is renamed in place; a macro
    # that stringizes, drops or moves it into a member/tag/label namespace fails.
    for body, reason in [("(((x) >> 2) << 2)", None), ("((s)->x)", "member/tag/label"),
                         ("#x", "member/tag/label"), ("(0)", "drops a parameter"),
                         ("(s.x)", "member/tag/label"), ("(s x)", "outside an operand position")]:
        text = "#define A4(x) " + body + "\nint s;\nint f(int l){return A4(l) + l;}\n"
        ok, why = proof("f:l=n", {"src/a.c": text},
                        {"src/a.c": text.replace("int l", "int n").replace("A4(l) + l", "A4(n) + n")})
        check("keycheck: local macro argument " + body,
              ok if reason is None else not ok and reason in why)
    # Review probes: a parameter forwarded into another macro, or an identity
    # macro invoked where its expansion is a member, tag or label, fails; so does
    # a #define reached through a ../ or #import include.
    struct = "struct S{int l;int n;}; struct S s; int l_tab, n_tab;\n"
    for label, text in [
            ("forwarded to a member macro", "#define B(y) (s.y)\n#define A(x) B(x)\n" + struct
             + "int f(int l){ return A(l) + l; }\n"),
            ("forwarded to a paste macro", "#define B(y) y##_tab\n#define A(x) B(x)\n" + struct
             + "int f(int l){ return A(l) + l; }\n"),
            ("forwarded to a stringizing macro", "#define B(y) #y\n#define A(x) B(x)\nint g(char *);\n"
             "int f(int l){ g(A(l)); return l; }\n"),
            ("identity after ->", "#define ID(x) x\n" + struct + "int f(struct S *p, int l){ return p->ID(l) + l; }\n"),
            ("identity as a label", "#define ID(x) x\nint f(int l){ if (l) goto l; ID(l): return l; }\n")]:
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": text})
        check("keycheck: local macro argument review probe: " + label, not ok and "macro" in why)
    struct = "struct S{int l;int a;} s, *p;\n"
    for label, text in [
            ("adjacent parameters", "#define PAIR(a, b) a b\n" + struct + "int f(int l){ return PAIR(s., l) + l; }\n"),
            ("macro name as an argument", "#define MEMBER(x) s.x\n#define APPLY(m, x) m(x)\n" + struct
             + "int f(int l){ return APPLY(MEMBER, l) + l; }\n"),
            ("parameter declares a member", "#define SD(x) struct { int x; } q\n"
             "int f(int l){ SD(l); q.l = l; return q.l; }\n"),
            ("object macro before the call", "#define DOT .\n#define ID(x) x\nint g(int);\n" + struct
             + "int f(int l){ return g(s DOT ID(l)) + l; }\n"),
            ("label address after a cast", "#define ID(x) x\n"
             "void *f(int l){ void *q = (void *)&&ID(l); l: return l ? q : 0; }\n")]:
        try:
            nk.scoped_plan({"src/a.c": text}, {"f:l": "n"}, {})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: local macro argument second review probe: " + label, why != "")
    files = {"src/a.c": "#ifndef M\n#define M 1\n#endif\nint f(int l){ return M + l; }\n",
             "src/b.c": "#define M (l + 1)\n#include \"a.c\"\n"}
    try:
        nk.scoped_plan(files, {"f:l": "n"}, {})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: a .c that includes f's file keeps its #define bodies visible", "#define body" in why)
    # Third review: the reverse include may be spelled through a macro, a line
    # splice or a comment; each still keeps the including file's macros visible.
    a_c = "#ifndef G\n#define G(x) (x)\n#endif\nint l;\nint f(int l){ return G(1) + l; }\n"
    for include in ('#define A_C "a.c"\n#include A_C\n', '#include \\\n"a.c"\n',
                    '#include "a.\\\nc"\n', '#include /*\n*/ "a.c"\n'):
        try:
            nk.scoped_plan({"src/a.c": a_c, "src/b.c": "#define G(x) ((x) + l)\n" + include},
                           {"f:l": "n"}, {})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: reverse include spelled " + repr(include), "#define body" in why)
    for include in ('#include "../b.c"\n', '#import "b.c"\n'):
        files = {"src/main/a.c": include + "int f(int l){ return G + l; }\n",
                 "src/b.c": "int l;\n#define G (l + 1)\n"}
        try:
            nk.scoped_plan(files, {"f:l": "n"}, {})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: include closure follows " + include.strip(), "#define body" in why)
    # Fourth review: cpp on /mnt/c matches include paths case-insensitively, and an
    # absolute or unmatched target may name any file; each keeps b.c's macros visible.
    for include in ('#include "B.c"\n', '#include "b.C"\n', '#include "../Main/b.c"\n',
                    '#include <MAIN/B.C>\n', '#include "/mnt/c/repo/src/main/b.c"\n',
                    '#include "C:/repo/src/main/b.c"\n', '#include "gone/q.inc"\n',
                    '\ufeff#include "b.c"\n', '#inc\\ \nlude "b.c"\n'):
        files = {"src/main/a.c": include + "int f(int l){ return G + l; }\n",
                 "src/main/b.c": "int l;\n#define G (l + 1)\n"}
        try:
            nk.scoped_plan(files, {"f:l": "n"}, {})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: include closure casefolds / falls back " + include.strip(), "#define body" in why)
    for include in ('#include "A.C"\n', '#include "/abs/a.c"\n', '#include "gone/q.inc"\n',
                    '﻿#include "a.c"\n', '#inc\\ \nlude "a.c"\n'):
        try:
            nk.scoped_plan({"src/a.c": a_c, "src/b.c": include + "#define G(x) ((x) + l)\n"},
                           {"f:l": "n"}, {})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: reverse include casefolds / falls back " + include.strip(), "#define body" in why)
    # A function-local struct/union that mentions neither old nor new leaves the
    # binding exact; one naming either (member, declarator, bound) fails.
    for label, body, good in [
            ("unrelated aggregate", "struct { int a; int b[2]; } s; s.a = l; return s.a + l;", True),
            ("unrelated tagged union", "union U { int a; short h; } s; s.a = l; return s.h + l;", True),
            ("member named old", "struct { int l; } s; s.l = l; return s.l;", False),
            ("declarator named old", "{ struct { int a; } l; l.a = 1; } return l;", False),
            ("bound naming old", "struct { int a[sizeof l]; } s; s.a[0] = l; return l;", False)]:
        text = "int f(int l){ " + body + " }\n"
        after = re.sub(r"(?<![.])\bl\b", "n", text)
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: local pair with in-body aggregate: " + label,
              ok if good else not ok and ("aggregate" in why or "found" in why))
    records = nk.macro_analysis("#define FN(x) (x)\n#define COPY(FN) FN\n")[0]
    nk.closure_macro_checks(records, {"f:l": "n"})
    check("keycheck: opener guard excludes macro own function-name parameter", True)
    text = "int f(int l){return l;}\n"
    files = {"src/a.c": '#include "inline.h"\n', "include/inline.h": text}
    check("keycheck: local function source in header", proof("f:l=n", files,
          {"include/inline.h": text.replace("int l", "int n").replace("return l", "return n")}, compile_headers=False)[0])
    def shifted_header_end(tree, tu):
        out = fake_cpp(tree, tu)
        return re.sub(r'(return \w+;)\}', r'\1\n# 2 "include/inline.h"\n}', out)
    ok, why = proof("f:l=n", files, {"include/inline.h": text.replace("int l", "int n").replace("return l", "return n")},
                    compile_headers=False, cpp=shifted_header_end)
    check("keycheck: header function span agreement independently enforced", not ok and "span disagrees" in why)
    # j1b's actual RPF() closer, not the earlier object-like RP approximation.
    text = ("struct S{int l;int n;};\n#define MEMBER(p,x) ((p)->x)\n"
            "#define OPENF() MEMBER(\n#define RPF() )\n"
            "int f(int l,struct S*s){return (0 RPF() + OPENF() s,l+0)+l;}\n")
    after = text.replace("f(int l", "f(int n").replace("s,l+0", "s,n+0").replace(")+l;", ")+n;")
    def rpf_cpp(tree, tu):
        out = fake_cpp(tree, tu)
        return "\n".join(line if line.lstrip().startswith("#") else
            re.sub(r'MEMBER\( s,(\w+)\+0\)', r'((s)->\1+0)',
                   line.replace("RPF()", ")").replace("OPENF()", "MEMBER(")) for line in out.splitlines()) + "\n"
    ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after}, cpp=rpf_cpp)
    check("keycheck: real j1b RPF shape rejects opener", not ok and "call opener" in why)
    with patch.object(nk, "closure_macro_checks", lambda *a: None), \
            patch.object(nk, "cpp_source_guards", lambda *a: None):
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after}, cpp=rpf_cpp)
    check("keycheck: real j1b RPF shape namespace proof independently rejects", not ok and "namespace" in why)

    # Round 12 attacks: hider definitions must end their declaration rows.
    hide_l = "int (*hide_l(void))(void){return 0;}\nint l;\n"
    hide_n = "int (*hide_n(void))(void){return 0;}\nint n;\n"
    inactive_n = "#if 0\n{\n#endif\n" + hide_n + "#if 0\n}\n#endif\n"
    for probe, prefix, body, globals_l, globals_n, header in [
            ("n1", "typedef int T;\n", "int a,T; T*l; return sizeof l;", hide_l, hide_n, ".inc"),
            ("n2", "typedef int T;\n", "int (T); T*l; return sizeof l;", hide_l, hide_n, ".inc"),
            ("n3", "int T; int g(void){typedef int T; T x=1; return x;}\n", "T*l; return sizeof l;", hide_l, hide_n, ".inc"),
            ("n4", "int T; int g(void){typedef int T; T x=1; return x;}\n", "T*l; return sizeof l;",
             "struct Y{int y;}; struct Y hide_l(void){struct Y r;r.y=0;return r;}\nint l;\n",
             "struct Y hide_n(void){struct Y r;r.y=0;return r;}\nint n;\n", ".inc"),
            ("n6", "typedef int T;\n", "int a,T; T*l; return sizeof l;", hide_l, inactive_n, None),
            ("n7", "typedef int T;\n", "int a,T; T*l; return sizeof l;", hide_l, inactive_n, ".h")]:
        if header is None:
            text = prefix + globals_l + globals_n + "int f(void){" + body + "}\n"
            files = {"src/a.c": text}
        else:
            path = "include/x" + header
            text = prefix + ('#include "x' + header + '"\n') + (globals_l if probe != "n7" else "") + "int f(void){" + body + "}\n"
            files = {"src/a.c": text, path: (globals_l if probe == "n7" else "") + globals_n}
        after = text.replace("T*l", "T*n").replace("sizeof l", "sizeof n")
        def inactive_cpp(tree, tu):
            clean = dict(tree)
            for path, value in clean.items():
                clean[path] = re.sub(r'#if 0\n[{}]\n#endif\n', lambda m: '\n' * m[0].count('\n'), value)
            return fake_cpp(clean, tu)
        ok, why = proof("f:l=n", files, {"src/a.c": after}, cpp=inactive_cpp)
        check("keycheck: round-12 attack " + probe, not ok)
        # Isolate cpp proof from source preflight and its conditional filter.
        clean = {p: re.sub(r'#if 0\n[{}]\n#endif\n', lambda m: '\n' * m[0].count('\n'), value) for p, value in files.items()}
        output = nk.emitted_lines(inactive_cpp(nk.CppTree(clean, "fixture"), "src/a.c"),
                                  nk.CppTree(clean, "fixture"), "src/a.c")
        try:
            nk.cpp_scope_checks(output, {"f:l": "n"}, {}, "base")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: independent compiled attack " + probe, bool(why))
    for declaration in ("int a,T;", "int (T);", "int (*T)(void);", "int T;"):
        text = "typedef int T; int f(void){" + declaration + "T*l; return sizeof l;}"
        output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
        try:
            nk.cpp_scope_checks(output, {"f:l": "n"}, {}, "base")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: every typedef-shadow declarator " + declaration, "shadowed typedef" in why)
    text = "int g(void){typedef int T; T x=1;return x;} int f(void){T*l;return sizeof l;}"
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, nk.CppTree({"src/a.c": text}, "fixture"), "src/a.c")
    try:
        nk.cpp_scope_checks(output, {"f:l": "n"}, {}, "base")
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: typedef inside other function is never visible", bool(why))
    for hider in (hide_l, "struct Y{int y;}; struct Y hide_l(void){struct Y r;return r;} int l;",
                  '__attribute__((unused)) int hide_l(void){return 0;} int l;'):
        ts = [m[0] for m in nk.code_spans(hider)]
        types = nk.declaration_types({"src/a.c": hider})
        functions = nk.cpp_functions(ts, types)
        names = nk.cpp_file_declarations(ts, types, functions, {"l"})
        check("keycheck: returning/attributed hider recognized " + hider, "l" in names and any(f[0] == "hide_l" for f in functions))
    text = "UNKNOWN_RET hide(void){return 0;} int l;"
    ts = [m[0] for m in nk.code_spans(text)]
    try:
        nk.cpp_file_declarations(ts, set(nk.DECL_WORDS), [], {"l"})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: unknown body cannot merge away following global", "unrecognized cpp file-scope body" in why)
    for spelling in (".inc", ".h"):
        try:
            nk.local_freshness({"include/x" + spelling: hide_n}, "n")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: source freshness includes hider file " + spelling, bool(why))
    try:
        nk.local_freshness({"src/a.c": inactive_n}, "n")
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: source inactive braces fail closed", "conditional" in why)

    # L1: all K&R parameter declarations belong to that function's span.
    for knr in ("int legacy(arg1,arg5) s16 arg1; s16 arg5; {return arg1+arg5;}",
                "int legacy(a,n) int a,n; {return a+n;}",
                "int legacy(mode,y) Env *mode; s16 y; {return y;}"):
        header = "typedef short s16; typedef struct {int x;} Env;\n"
        text = '#include "types.h"\n' + knr + "\nint f(int arg1){return arg1;}"
        for dest in ("arg5", "n", "y"):
            after = text.replace("f(int arg1)", "f(int " + dest + ")").replace("return arg1;", "return " + dest + ";")
            check("keycheck: header-typedef K&R old/destination " + dest + " " + knr,
                  proof("f:arg1=" + dest, {"src/a.c": text, "include/types.h": header}, {"src/a.c": after})[0])
    for old, dest, knr in [("mode", "rvq_mode", "int legacy(mode) Env *mode; {return 0;}"),
                           ("y", "rvq_y", "int legacy(y) s16 y; {return y;}")]:
        text = '#include "types.h"\n' + knr + " int f(int " + old + "){return " + old + ";}"
        after = text.replace("f(int " + old + ")", "f(int " + dest + ")").replace("return " + old + ";}", "return " + dest + ";}", 1 if old == "mode" else 0)
        if old == "y":
            after = text.replace("f(int y){return y;}", "f(int rvq_y){return rvq_y;}")
        check("keycheck: K&R shared old name " + old, proof("f:" + old + "=" + dest,
              {"src/a.c": text, "include/types.h": "typedef short s16; typedef struct {int x;} Env;"}, {"src/a.c": after})[0])
    for scope in ("func_80010000", "good_fn"):
        text = "int func_80010000(int arg0){return arg0;}"
        after = "int good_fn(int count){return count;}"
        check("keycheck: globally renamed local function " + scope, proof("func_80010000=good_fn," + scope + ":arg0=count",
              {"src/a.c": text}, {"src/a.c": after})[0])
    for expr in ("(const T) l", "sizeof(volatile T) + l"):
        text = "typedef int T; int f(int l){return " + expr + ";}"
        after = text.replace("int l", "int n").replace(") l", ") n").replace("+ l", "+ n")
        check("keycheck: qualified type context is not a shadow " + expr, proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": after})[0])
    text = "#define ARROW() ->\nunion S{int l;int n;}; int f(int l,union S*s){return l+s ARROW() l+l;}"
    tree = nk.CppTree({"src/a.c": text}, "fixture")
    output = nk.emitted_lines(cpp_preamble("src/a.c") + text, tree, "src/a.c")
    try:
        nk.cpp_source_guards(output, tree, {"f:l": "n"})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: k5 repeated local around macro member", "local l adjacent to cpp macro invocation" in why)

    for label, globals_l, globals_n, header in [
            ("n1 plain n", hide_l, "int n;", ".inc"),
            ("n1 plain l", "int l;", hide_n, ".inc"),
            ("n7 no conditional braces", hide_l, hide_n, ".h")]:
        text = 'typedef int T;\n#include "x' + header + '"\n' + globals_l + "int f(void){int a,T; T*l; return sizeof l;}"
        after = text.replace("T*l", "T*n").replace("sizeof l", "sizeof n")
        check("keycheck: round-12 retained control " + label, not proof("f:l=n",
              {"src/a.c": text, "include/x" + header: globals_n}, {"src/a.c": after})[0])
    for text in ("int f(int l){\n#pragma kc\nreturn l;}\n", "int f(l) int l; {return l;}"):
        check("keycheck: pragma/target K&R remains fail-closed", not proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": text.replace("int l", "int n").replace("return l", "return n")})[0])

    text = "struct T{int m;}; int f(int l){return l;}"
    check("keycheck: mixed member/local census excludes unrelated TU function proofs",
          proof("T.m=fresh,f:l=n", {"src/a.c": text, "src/b.c": "int g(int x){return x;}"},
                {"src/a.c": text.replace("int m", "int fresh").replace("int l", "int n").replace("return l", "return n")})[0])

    try:
        nk.scoped_plan({"src/a.c": "int f(int a,int b){return a+b;}"}, {"f:a": "n", "good_fn:b": "n"}, {"f": "good_fn"})
        why = ""
    except ValueError as e:
        why = str(e)
    check("keycheck: function aliases cannot share a scoped destination", "shared by another pair" in why)

    # A count mismatch fails even when the correct spelling appears once.
    def duplicate_normal(tree, tu):
        out = fake_cpp(tree, tu)
        if not tree.label.endswith("-sentinel"):
            field = "m" if tree.label == "base" else "n"
            out = out.replace("return p->" + field, "return p->" + field + "+p->" + field)
        return out
    ok, why = proof("T.m=n", simple, changed_simple, cpp=duplicate_normal)
    check("keycheck: original spelling counts must equal sentinel counts", not ok and "original spelling/count" in why)
    # Definitions absent from source files must still be obtained from real cpp.
    for defined in ("m", "n"):
        fake_make = 'CPP := cpp\nCPP_FLAGS := -Iinclude\nCPP_DEFS := -undef\n\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | cc1\n'
        def cpp_definitions(args, **kwargs):
            out = cpp_preamble("src/a.c") + "int x;\n"
            if "-dD" in args:
                out = out.replace('# 0 "<command-line>" 2', '#define ' + defined + '(x) (x)\n# 0 "<command-line>" 2')
            return nk.subprocess.CompletedProcess(args, 0, out, "")
        with patch.object(nk.shutil, "which", return_value=sys.executable), \
                patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
                patch.object(nk.subprocess, "run", side_effect=cpp_definitions):
            cpp_tree = nk.CppTree({"Makefile": fake_make, "src/a.c": "int x;"}, "test")
            output = nk.emitted_lines(nk.preprocess(cpp_tree, "src/a.c"), cpp_tree, "src/a.c")
        try:
            nk.closure_macro_checks(output.macros, {"T.m": "n"})
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: system-defined scoped macro name requires -dD " + defined, "#define name" in why)
    for side, name in (("base", "m"), ("new", "n")):
        def defined_on_side(tree, tu):
            out = fake_cpp(tree, tu)
            if tree.label.startswith(side):
                out = out.replace('# 0 "<command-line>" 2', '#define ' + name + '(x) (x)\n# 0 "<command-line>" 2')
            return out
        ok, why = proof("T.m=n", simple, changed_simple, cpp=defined_on_side)
        check("keycheck: scoped macro-name proof checks " + side + " tree", not ok and "#define name" in why)
    for probe in ("__has_include", "__has_include_next"):
        text = 'struct T{int m;};\n#if ' + probe + '("not-in-snapshot.txt")\n#define VALUE 1\n#endif\n'
        ok, why = proof("T.m=n", {"src/a.c": text, "notes/not-in-snapshot.txt": "exists"},
                        {"src/a.c": text.replace("int m", "int n")})
        check("keycheck: round-7 snapshot presence probe " + probe, not ok and "__has_include" in why)
    for label, text, spec in [
            ("hidden member", '#define JOIN(a,b) a ## b\nstruct T{int m;}; struct U{int JOIN(fr,esh);};\n', "T.m=fresh"),
            ("local paste capture", '#define JOIN(a,b) a ## b\nextern int JOIN(fr,esh);\nint f(int l){return l+JOIN(fr,esh);}\n', "f:l=fresh"),
            ("digraph define", '%:define UNUSED 0\nstruct T{int m;};\n', "T.m=fresh"),
            ("digraph paste", '%:define JOIN(a,b) a %:%: b\nstruct T{int m;};\n', "T.m=fresh")]:
        after = text.replace("int m;", "int fresh;").replace("int l", "int fresh").replace("return l", "return fresh")
        ok, why = proof(spec, {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: round-7 closure paste/digraph " + label, not ok and "token paste or digraph" in why)
    fixture_make = ('CPP := missing-keycheck-cpp\nCPP_FLAGS := -Iinclude\nCPP_DEFS := -Dmips\n'
                    '\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | cc1\n')
    eq("keycheck: separated -D form exposes macro body", nk.command_line_macros(
       fixture_make.replace("-Dmips", "-D GET()=fresh")), "#define GET() fresh")
    for spec, text, definition in [("T.m=fresh", "struct T{int m;};", "-D MEMBER=fresh"),
                                   ("f:l=fresh", "int f(int l){return l;}", "-D GET()=l")]:
        ok, why = proof(spec, {"src/a.c": text, "Makefile": fixture_make.replace("-Dmips", definition)},
                        {"src/a.c": text.replace("int m;", "int fresh;").replace("int l", "int fresh").replace("return l", "return fresh")})
        check("keycheck: separated -D macro freshness " + spec, not ok and ("macros" in why or "#define" in why))
    for label, define, spec, text, reason in [
            ("member destination body", '-DMEMBER=fresh', 'T.m=fresh', 'struct T{int m;}; struct U{int MEMBER;};', 'macros'),
            ("member destination name", '-Dfresh=1', 'T.m=fresh', 'struct T{int m;};', 'macros'),
            ("local destination body", '-DGET()=fresh', 'f:l=fresh', 'int f(int l){return l;}', '#define'),
            ("local old body", '-DGET()=l', 'f:l=fresh', 'int f(int l){return l;}', '#define'),
            ("local old name", '-Dl=outer', 'f:l=fresh', 'int f(int l){return l;}', '#define'),
            ("command-line hidden member", '-DMEMBER=fr\\#\\#esh', 'T.m=fresh', 'struct T{int m;}; struct U{int MEMBER;};', 'token paste')]:
        make = fixture_make.replace('-Dmips', '-Dmips ' + define)
        after = text.replace("int m;", "int fresh;").replace("int l", "int fresh").replace("return l", "return fresh")
        ok, why = proof(spec, {"src/a.c": text, "Makefile": make}, {"src/a.c": after})
        check("keycheck: round-7 command-line macro " + label, not ok and reason in why)
    for name in ("l", "fresh"):
        text = "int f(int l){\nreturn l;\nSYSTEM_NAME;\n}\n"
        after = text.replace("int l", "int fresh").replace("return l", "return fresh")
        def captured(tree, tu):
            return fake_cpp(tree, tu).replace("SYSTEM_NAME", name)
        ok, why = proof("f:l=fresh", {"src/a.c": text}, {"src/a.c": after}, cpp=captured)
        check("keycheck: round-7 expansion on untouched function line produces " + name,
              not ok and "macro expansion produces or captures" in why)
    # Real-tree failures were unrelated macro expansion on the edited line.
    for func, body, macro, expanded in [
            ("func_8007636C", "s.y = SELWORK->f3C[arg3] * 34;", "SELWORK", "((SelWork *)g_sel_screen_work)"),
            ("math_MatrixToAngles", "m = &SPAD->unk2B8.rec.unkD8;", "SPAD", "((ScrPad *)0x1F800000)"),
            ("menu_StepSelScreen", "s.unk_00.v80076FF8 = SELWORK->f04;", "SELWORK", "((SelWork *)g_sel_screen_work)")]:
        old = "m" if func == "math_MatrixToAngles" else "s"
        text = f"typedef int Matrix;\n#define {macro} {expanded}\nint {func}(int arg3){{\nMatrix {old};\n{body}\nreturn 0;\n}}\n"
        after = re.sub(r"\b" + old + r"\b", "renamed", text)
        def real_shape(tree, tu):
            return fake_cpp(tree, tu).replace(macro, expanded)
        check("keycheck: real macro-line token survives " + func,
              proof(func + ":" + old + "=renamed", {"src/a.c": text}, {"src/a.c": after}, cpp=real_shape)[0])
    member_line = "func_80032854(pid, 4, &SPAD->unkA8[pid][rec->unk_352].x, out);"
    text = ("#define SPAD ((ScrPad *)0x1F800000)\nstruct Unk80101EC8Record{int unk_352;};\n"
            "int f(struct Unk80101EC8Record *rec){\n" + member_line + "\nreturn 0;\n}\n")
    after = text.replace("unk_352", "renamed")
    check("keycheck: real member macro-line token survives", proof("Unk80101EC8Record.unk_352=renamed",
          {"src/a.c": text}, {"src/a.c": after}, cpp=lambda tree, tu: fake_cpp(tree, tu).replace("SPAD", "((ScrPad *)0x1F800000)"))[0])
    # Source inspection follows the entered closure, including unchanged files.
    for control in ['/**/ #line 2 "other.h"', '%:line 2 "other.h"', '# 2 "other.h"']:
        tree = nk.CppTree({"src/a.c": "", "include/h.h": control}, "test")
        try:
            nk.emitted_lines('# 1 "src/a.c"\n# 1 "include/h.h" 1\n# 2 "src/a.c" 2\n', tree, "src/a.c")
            why = ""
        except ValueError as e:
            why = str(e)
        check("keycheck: included source line control fails " + control, "line-control" in why)
    stale, recent = Path(nk.ROOT) / "tmp/keycheck-cpp-old", Path(nk.ROOT) / "tmp/keycheck-cpp-new"
    with patch.object(Path, "glob", return_value=[stale, recent]), \
            patch.object(Path, "is_dir", return_value=True), \
            patch.object(Path, "is_symlink", return_value=False), \
            patch.object(Path, "stat", side_effect=[type("Stat", (), {"st_mtime": 0})(), type("Stat", (), {"st_mtime": 7200})()]), \
            patch.object(Path, "resolve", autospec=True, side_effect=lambda p: p), \
            patch.object(nk.time, "time", return_value=7200), \
            patch.object(nk.shutil, "rmtree") as remove:
        nk.cleanup_cpp_snapshots(Path(nk.ROOT) / "tmp")
        eq("keycheck: only abandoned hour-old snapshots removed", remove.call_args_list, [call(stale)])
    for text, after in [
            ("int l; int f(int (*cb)(int l)){return 0;}", "int l; int f(int (*cb)(int n)){return 0;}"),
            ("int l; int f(void){use((int (*)(int l))cb);return 0;}",
             "int l; int f(void){use((int (*)(int n))cb);return 0;}")]:
        ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: isolated nesting guard preserves global",
              not ok and ("prototype/cast binding" in why or "inside a type name" in why))
    for other in ["#define DECL(x) int x\nDECL(n);", "typedef int s32; s32 (n);"]:
        ok, why = proof("f:l=n", {"src/a.c": "int f(int l){return l;}", "src/b.c": other},
                        {"src/a.c": "int f(int n){return n;}"})
        check("keycheck: isolated real-prototype skip guard", not ok and "file scope" in why)
    for label, other in [("macro declarator", "#define DECL(x) x\nint DECL(n);"),
                         ("macro type prefix", "#define TYPE int\nTYPE proto(n);")]:
        ok, why = proof("f:l=n", {"src/a.c": "int f(int l){return l;}", "src/b.c": other},
                        {"src/a.c": "int f(int n){return n;}"})
        check("keycheck: isolated prototype macro gate " + label, not ok and "file scope" in why)
    for text, reason in [("int f(int l){return l;} int f(int l){return l;}", "exactly one"),
                         ("struct T{struct {int m;} a;};", "nested")]:
        spec = "f:l=n" if text.startswith("int") else "T.m=n"
        ok, why = proof(spec, {"src/a.c": text}, {"src/a.c": re.sub(r"\b[ml]\b", "n", text)})
        check("keycheck: specific definition/aggregate failure reason " + reason, not ok and reason in why)
    makefile = ("CPP := missing-keycheck-cpp\nCPP_FLAGS := -Iinclude -undef\n"
                "CPP_DEFS := -Dmips \\\n -DLANGUAGE_C\n"
                "\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | other\n")
    eq("keycheck: Makefile cpp command preserves flags/defines", nk.cpp_command(makefile),
       ["missing-keycheck-cpp", "-Iinclude", "-undef", "-Dmips", "-DLANGUAGE_C"])
    appended = makefile + "CPP_FLAGS += -I extra # fixture\nCPP_DEFS += -DADDED=1\nCPP_FLAGS += -Ivendor\n"
    eq("keycheck: Makefile append flags and definitions honoured", nk.cpp_command(appended),
       ["missing-keycheck-cpp", "-Iinclude", "-undef", "-I", "extra", "-Ivendor",
        "-Dmips", "-DLANGUAGE_C", "-DADDED=1"])
    eq("keycheck: snapshot configured include roots", nk.cpp_snapshot_roots(appended),
       ("extra", "include", "src", "vendor"))
    snapshot = nk.CppTree({"Makefile": appended, "src/a.c": "a", "include/h.h": "h",
                           "extra/h.inc": "extra", "vendor/v.h": "vendor", "asm/s.s": "asm",
                           "unrelated/u.h": "unused"}, "test")
    with patch.object(nk, "cleanup_cpp_snapshots"), patch.object(Path, "mkdir"), \
            patch.object(Path, "write_bytes", autospec=True) as write:
        snapshot.materialize()
        copied = sorted(args[0][0].relative_to(snapshot.directory).as_posix() for args in write.call_args_list)
    eq("keycheck: snapshot excludes asm and unrelated roots", copied,
       ["Makefile", "extra/h.inc", "include/h.h", "src/a.c", "vendor/v.h"])
    with patch.object(nk.shutil, "which", return_value=sys.executable), \
            patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
            patch.object(nk.subprocess, "run", return_value=nk.subprocess.CompletedProcess([], 1, "", "fixture cpp error")):
        try:
            nk.preprocess(nk.CppTree({"Makefile": makefile}, "test"), "src/a.c")
            why = ""
        except ValueError as e:
            why = str(e)
    check("keycheck: cpp nonzero status fails clearly", "cpp failed for src/a.c: fixture cpp error" in why)
    shifted_base = {"src/a.c": "int f(int l){\nreturn l;\n}\n"}
    shifted_new = {"src/a.c": "\nint f(int n){\n\nreturn n;\n}\n"}
    check("keycheck: new-tree shifted source line mapping passes", proof("f:l=n", shifted_base, shifted_new)[0])
    runs = []
    def counted_cpp(tree, tu):
        runs.append((tree.label, tu))
        return fake_cpp(tree, tu)
    pair_text = "int f(int l){return l;}\nint g(int k){return k;}\n"
    check("keycheck: multiple pairs reuse one cpp run per tree/TU", proof("f:l=n,g:k=q",
          {"src/a.c": pair_text}, {"src/a.c": pair_text.replace("l", "n").replace("k", "q")}, cpp=counted_cpp)[0]
          and runs == [(label, "src/a.c") for label in ("base", "new", "base-sentinel", "new-sentinel")])
    with patch.object(nk.shutil, "which", return_value=None):
        try:
            nk.preprocess(nk.CppTree({"Makefile": makefile}, "test"), "src/a.c")
            missing = ""
        except ValueError as e:
            missing = str(e)
    check("keycheck: missing cpp toolchain fails clearly", "toolchain missing" in missing)
    for bad in [makefile.replace("-Iinclude", "$(INCLUDE_FLAGS)"), makefile.replace("$< |", "-Dextra $< |")]:
        try:
            nk.cpp_command(bad)
            rejected = False
        except ValueError:
            rejected = True
        check("keycheck: ambiguous Makefile cpp setup fails closed", rejected)
    # Real-toolchain coverage is optional on hosts without the exact cross cpp.
    real_command = nk.cpp_command((Path(nk.CODE) / "Makefile").read_text(encoding="utf-8"))
    if not nk.shutil.which(real_command[0]):
        global _skipped
        _skipped += 1
        print("  SKIP: real cpp/real CLI timing: project cross preprocessor unavailable")
    else:
        files = {"Makefile": (Path(nk.CODE) / "Makefile").read_text(encoding="utf-8"),
                 "src/cpp_probe.c": '#include "h.h"\n', "include/h.h": "struct T {int m;};\n"}
        edits = nk.scoped_plan(files, {"T.m": "n"})
        nk.verify_compiled_edits(files, {**files, "include/h.h": "struct T {int n;};\n"}, edits, scoped={"T.m": "n"})
        check("keycheck: real project cpp emits edited header", True)
        files = {"Makefile": files["Makefile"],
                 "src/cpp_probe.c": ("#define SQ(x) ((x) * (x))\nstruct T {int m;};\n"
                                     "int f(struct T *t) { return SQ(1 +\n        t->m); }\n")}
        tree = nk.CppTree(files, "real-cpp-probe")
        try:
            out = nk.emitted_lines(nk.preprocess(tree, "src/cpp_probe.c"), tree, "src/cpp_probe.c")
        finally:
            nk.close_cpp_trees([tree])
        check("keycheck: real cpp emits a multi-line invocation's expansion on its first line",
              out.get(("src/cpp_probe.c", 3), []).count("m") == 2 and "m" not in out.get(("src/cpp_probe.c", 4), []))
        new_files = {**files, "src/cpp_probe.c": files["src/cpp_probe.c"].replace("m;", "n;").replace("->m", "->n")}
        edits = nk.scoped_plan(files, {"T.m": "n"})
        nk.verify_compiled_edits(files, new_files, edits, scoped={"T.m": "n"})
        check("keycheck: real cpp proves an edit on an invocation's second line", True)
    text = "int f(int arg0){return arg0;} int g(int arg0){return arg0;}"
    check("keycheck: distinct functions share local destination", proof("f:arg0=player,g:arg0=player",
          {"src/a.c": text}, {"src/a.c": text.replace("arg0", "player")})[0])
    text = "struct T{int m;}; struct U{int m;}; int f(struct T*t,struct U*u){return t->m+u->m;}"
    after = "struct T{int x;}; struct U{int y;}; int f(struct T*t,struct U*u){return t->x+u->y;}"
    check("keycheck: same member spelling in distinct aggregates", proof("T.m=x,U.m=y",
          {"src/a.c": text}, {"src/a.c": after})[0])
    text = "int f(int l){ return &&l; l: return l; }"
    check("keycheck: GNU label address is never a local use", not proof("f:l=n",
          {"src/a.c": text}, {"src/a.c": text.replace("int l", "int n").replace("return l", "return n")})[0])
    # Each of these must fail specifically in freshness, with all unrelated
    # declarations preserved in the proposed output.
    for other in ["struct U {int n;};", "#define n 1\n", "#define X n\n"]:
        text = "struct T {int m;};\n" + other
        ok, why = proof("T.m=n", {"src/a.c": text}, {"src/a.c": text.replace("int m", "int n")})
        check("keycheck: non-vacuous member freshness " + other, not ok and "fresh" in why)
    text = "typedef int Unk1F800000Unk00; struct Rec1C { Unk1F800000Unk00 b18; };"
    after = "typedef int X; struct Rec1C { X field; };"
    check("keycheck: member/global aggregate interior", proof("Rec1C.b18=field,Unk1F800000Unk00=X",
          {"src/a.c": text}, {"src/a.c": after})[0])
    import time
    many = "\n".join(f"int fn{i}(int arg0){{return arg0 + {i};}}" for i in range(600))
    pairs = {f"fn{i}:arg0": "player" for i in range(30)}
    start = time.perf_counter()
    edits = nk.scoped_plan({"src/many.c": many}, pairs)
    elapsed = time.perf_counter() - start
    check("keycheck: many-function 30-pair proof is bounded", elapsed < 10
          and sum(len(e) for e in edits.values()) == 60)
    print(f"  scoped timing: 600 functions, 30 pairs: {elapsed:.3f}s")
    for changed in [False, True]:
        text = "#define TRAP_IF(c) (0)\nstruct T { int m; }; int f(struct T *u){return TRAP_IF(u->m);}"
        after = text.replace("int m", "int n")
        if changed:
            after = after.replace("->m", "->n")
        ok, why = proof("T.m=n", {"src/a.c": text}, {"src/a.c": after})
        check("keycheck: member macro argument rejects stale/changed " + str(changed),
              not ok and "macro TRAP_IF" in why)
    text = "struct T { int m; };\n/**/ #if 0\nint f(struct T *u){return u->m;}\n/**/ #endif\n"
    check("keycheck: comments cannot hide conditionals", not proof("T.m=n",
          {"src/a.c": text}, {"src/a.c": text.replace("int m", "int n").replace("->m", "->n")})[0])
    for macro, use in [("#define S struct\n", "S l *p;"),
                       ("#define GP gp->\n", "return GP l;"),
                       ("#define TAIL + 1\n", "return l TAIL;")]:
        text = macro + "int f(int l){ " + use + " return l; }"
        check("keycheck: object macro adjacency " + use, not proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    text = "#define GET(p,f) ((p)->f)\n#define ALIAS GET\nint f(int l){return ALIAS(s,l)+l;}"
    ok, why = proof("f:l=n", {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})
    check("keycheck: object alias cannot hide macro argument", not ok and "macro alias ALIAS" in why)
    for label in ["if(c) l:", "else l:", "do l:"]:
        text = "int f(int l){ " + label + " x++; return l; }"
        check("keycheck: ambiguous non-start label " + label, not proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    for text in ["int l; int f(void){int (*cb)(int l); return 0;}",
                 "int l; int f(int (*cb)(int l)){return 0;}",
                 "int l; int f(void){ use((int (*)(int l))p); return 0;}",
                 "int f(int l){ __label__ l; return l; }"]:
        check("keycheck: hygiene nested binding/label " + text, not proof("f:l=n",
              {"src/a.c": text}, {"src/a.c": re.sub(r"\bl\b", "n", text)})[0])
    text = "typedef int SPU_VOICE_REG; int SpuGetKeyStatus(void){volatile SPU_VOICE_REG *voices; return 0;}"
    check("keycheck: a type occurrence cannot be the local", not proof("SpuGetKeyStatus:SPU_VOICE_REG=n",
          {"src/a.c": text}, {"src/a.c": text.replace("volatile SPU_VOICE_REG", "volatile n")})[0])
    for dest in ["color", "count", "flags", "next", "state", "mode", "timer"]:
        text = "int " + dest + "; struct Unk8007352CEnv { int col_b; }; int f(struct Unk8007352CEnv *p){return p->col_b;}"
        after = text.replace("int col_b", "int " + dest).replace("->col_b", "->" + dest)
        check("keycheck: meaningful member destination " + dest, proof("Unk8007352CEnv.col_b=" + dest,
              {"src/a.c": text, "asm/s.s": "glabel " + dest}, {"src/a.c": after})[0])
        other = " struct Other { int " + dest + "; };"
        ok, why = proof("Unk8007352CEnv.col_b=" + dest,
                        {"src/a.c": text + other}, {"src/a.c": after + other})
        check("keycheck: existing member destination still forbidden " + dest,
              not ok and "fresh" in why)
    for literal in ['"l"', '"%[l]"']:
        text = f'int f(int l) {{ use({literal}); return l; }}'
        check("keycheck: ordinary string unchanged " + literal, not proof("f:l=fresh",
              {"src/a.c": text}, {"src/a.c": text.replace("l", "fresh")})[0])
    eq("keycheck: global grammar unchanged", nk.parse_pairs("old=new"), {"old": "new"})
    eq("keycheck: mixed grammar and pairs-file lines",
       nk.parse_pairs("# heading\nT.m=fresh\nf:l=other,old=new\n"),
       {"T.m": "fresh", "f:l": "other", "old": "new"})
    check("keycheck: scoped comment words have boundaries", not nk.scoped_comments(
          "/* 1m */", "/* 1fresh */", {"T.m": "fresh"})[0])

    # Follow-up 18: cc1 is the authoritative local proof. Reject-only parsers
    # are deliberately bypassed here so each compiler guard is independently live.
    cc_source = "int f(int l) { return l + 1; }\n"
    def cc_check(runner, source=cc_source, spec=None):
        output = nk.emitted_lines(fake_cpp({"src/a.c": source}, "src/a.c"), {"src/a.c": source}, "src/a.c")
        try:
            with patch.object(nk, "compile_cc1", runner):
                nk.verify_local_cc1({"src/a.c": source}, {"src/a.c": output}, spec or {"f:l": "n"}, {})
            return True, ""
        except ValueError as e:
            return False, str(e)
    for side in ("fresh", "destination"):
        for mode in ("rc", "undeclared", "symbol", "instruction", "error"):
            def bad_cc1(tree, tu, side=side, mode=mode):
                if tree.label != "cc1-" + side:
                    return fake_cc1(tree, tu)
                return nk.subprocess.CompletedProcess([], 1 if mode == "rc" else 0,
                    "lw $2,__kc_fresh_0\n" if mode == "symbol" else
                    "\taddiu $2,$0,2\n" if mode == "instruction" else fake_cc1(tree, tu).stdout,
                    "undeclared identifier" if mode == "undeclared" else "error: failed" if mode == "error" else "")
            ok, why = cc_check(bad_cc1)
            check(f"keycheck cc1: {side} {mode} rejects", not ok and "cc1" in why)
    check("keycheck cc1: genuine local accepts", cc_check(fake_cc1)[0])
    def static_cc1(tree, tu):
        name = {"cc1-base": "l", "cc1-fresh": "__kc_fresh_0", "cc1-destination": "n"}[tree.label]
        return nk.subprocess.CompletedProcess([], 0, f"{name}.17:\n.word {name}.17\n", "")
    check("keycheck cc1: only static-local numeric labels normalize", cc_check(static_cc1)[0])
    def plain_symbol(tree, tu):
        name = {"cc1-base": "l", "cc1-fresh": "__kc_fresh_0", "cc1-destination": "n"}[tree.label]
        return nk.subprocess.CompletedProcess([], 0, f"lw $2,{name}\n", "")
    check("keycheck cc1: plain globals never normalize", not cc_check(plain_symbol)[0])
    calls = []
    def batch_cc1(tree, tu):
        calls.append((tree.label, tu, tree[tu]))
        return fake_cc1(tree, tu)
    cc_check(batch_cc1, "int f(int l,int a){return l+a;}\n", {"f:l": "n", "f:a": "z"})
    check("keycheck cc1: pairs batch in three compiles", len(calls) == 3 and
          "__kc_fresh_0" in calls[1][2] and "__kc_fresh_1" in calls[1][2])
    def preserved_cc1(tree, tu):
        if tree.label == "cc1-fresh":
            text = tree[tu]
            check("keycheck cc1: namespaces literals comments retained",
                  "p->l" in text and '"l"' in text and "/* l */" in text and
                  "goto l; l:" in text and "int __kc_fresh_0" in text)
        return fake_cc1(tree, tu)
    cc_check(preserved_cc1, 'int f(int l){goto l; l: return p->l + l; /* l */ "l";}\n')

    # Every round-13 source shape reaches either a rejecting prefilter or cc1.
    enum_hide = "enum EL { AL=1 } hide_l(void){return AL;}\nint l;\n"
    hidden_n = "#if 0\n{\n#elif 1\nint n;\n#else\n}\n#endif\n"
    cc_probes = {
        "s1": ("int T;\n"+enum_hide+hidden_n+"int f(void){T*l;return sizeof l;}\n", "n"),
        "s1c": ("int T; struct Y {int y;} g(void){typedef int T;return r;}\n"+enum_hide+
                "int n; int f(void){T*l;return sizeof l;}\n", "n"),
        "s2": (enum_hide+hidden_n+"int f(void){return l;}\n", "n"),
        "s3a": ("int l; int n\\\nn; int f(void){return l;}\n", "nn"),
        "s3b": (enum_hide+"#if 0\n{\n#error }\n#endif\nint n;int f(void){return l;}\n", "n"),
        "s4a": ("typedef int U; int l;static U (n); int f(void){return l;}\n", "n"),
        "M1-w1": ("typedef int T; int x; __typeof__(x) l; extern __typeof__(x) n __asm__(\"l\"); "
                   "int f(void){__typeof__(x) T;T*l;return l;}\n", "n"),
        "M1-u1": ("typedef int T; int x; __typeof__(x) l; extern __typeof__(x) n __asm__(\"l\"); "
                   "int f(__typeof__(x) T){T*l;return l;}\n", "n"),
        "M1-u5": ("typedef int T; int x; __typeof__(x) l; extern __typeof__(x) n __asm__(\"l\"); "
                   "int f(void){__typeof__(x) T;T*l;return sizeof l;}\n", "n"),
        "M2": ("int T;enum E{A=1}g(void){typedef int T;return A;}int l;int n;"
               "int f(void){T*l;return l;}\n", "n"),
    }
    def reject_rebinding(tree, tu):
        return plain_symbol(tree, tu)
    for label, (source, dest) in cc_probes.items():
        # Isolate the authoritative proof: a permissive/removed prefilter cannot accept.
        ok, why = cc_check(reject_rebinding, source, {"f:l": dest})
        check("keycheck cc1 round13 " + label, not ok)
    for label in ("M1-w1", "M1-u1", "M1-u5"):
        source, dest = cc_probes[label]
        source = source.replace('__typeof__(x) l; extern __typeof__(x) n __asm__("l"); ', '\n#include "types.def"\n')
        output = nk.emitted_lines(fake_cpp({"src/a.c": source,
            "include/types.def": '__typeof__(x) l; extern __typeof__(x) n __asm__("l");\n'}, "src/a.c"), {"src/a.c": source, "include/types.def": '__typeof__(x) l; extern __typeof__(x) n __asm__("l");\n'}, "src/a.c")
        check("keycheck round13 .def closure " + label, "include/types.def" in output.sources)
        with patch.object(nk, "compile_cc1", reject_rebinding):
            try:
                nk.verify_local_cc1({"src/a.c": source, "include/types.def": '__typeof__(x) l; extern __typeof__(x) n __asm__("l");\n'},
                                    {"src/a.c": output}, {"f:l": dest}, {})
                rejected = False
            except ValueError:
                rejected = True
        check("keycheck cc1 round13 .def " + label, rejected)
    try:
        nk.file_scope_tokens("int n\\\nn;\n")
        splice_ok = "nn" in nk.file_scope_tokens("int n\\\nn;\n")[0]
    except ValueError:
        splice_ok = False
    check("keycheck: freshness joins line splices", splice_ok)
    check("keycheck: directive brace ignored", "n" in nk.file_scope_tokens("#define OPEN {\nint n;\n")[0])
    try:
        nk.file_scope_tokens(hidden_n)
        branches_fail = False
    except ValueError:
        branches_fail = True
    check("keycheck: conditional branches balance separately", branches_fail)
    # C++-only branches (PsyQ's extern "C" guards, include/psxsdk/libsn.h) are inactive for cc1.
    guard = "#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)\n"
    try:
        visible, _ = nk.file_scope_tokens(guard + 'extern "C" {\n#endif\nint n;\n' + guard + "}\n#endif\n")
        cxx_ok = "n" in visible
    except ValueError:
        cxx_ok = False
    check("keycheck: C++-only extern C guard is inactive", cxx_ok)
    visible, _ = nk.file_scope_tokens("#ifdef __cplusplus\nint a;\n#else\nint b;\n#endif\n")
    check("keycheck: C++-only branch inactive, #else active", "b" in visible and "a" not in visible)
    for mixed in ("#if defined(__cplusplus) || FOO\n{\n#endif\nint n;\n",
                  "#ifndef __cplusplus\n{\n#endif\nint n;\n",
                  "#if defined(__cplusplus)\nint a;\n#elif FOO\n{\n#endif\nint n;\n"):
        try:
            nk.file_scope_tokens(mixed)
            mixed_fail = False
        except ValueError:
            mixed_fail = True
        check("keycheck: non-C++-only conditional still balances", mixed_fail)
    visible, _ = nk.file_scope_tokens("#define c_plusplus 1\n#if c_plusplus\nint g;\n#endif\n")
    check("keycheck: defined C++ macro keeps its branch active", "g" in visible)
    visible, _ = nk.file_scope_tokens("#ifdef __cplusplus\nint g;\n#endif\n", frozenset({"__cplusplus"}))
    check("keycheck: C++ macro among all_macros keeps its branch active", "g" in visible)
    for label, text in (("CRLF", guard.replace("\n", "\r\n") + 'extern "C" {\r\n#endif\r\nint n;\r\n'
                                 + guard.replace("\n", "\r\n") + "}\r\n#endif\r\n"),
                        ("continuation", "#if defined(__cplusplus) \\\n || defined(c_plusplus)\n"
                                         'extern "C" {\n#endif\nint n;\n#ifdef __cplusplus\n}\n#endif\n'),
                        ("nested #if", "#ifdef __cplusplus\n#if 1\n{\n#endif\nint a;\n#else\nint n;\n#endif\n")):
        try:
            visible, _ = nk.file_scope_tokens(text)
            ok = "n" in visible and "a" not in visible
        except ValueError:
            ok = False
        check("keycheck: C++-only guard, " + label, ok)
    libsn = (nk.ROOT / "include/psxsdk/libsn.h").read_text(encoding="utf-8")
    try:
        libsn_ok = "PCclose" in nk.file_scope_tokens(libsn)[0]
    except ValueError:
        libsn_ok = False
    check("keycheck: vendored libsn.h establishes file scope", libsn_ok)
    ok, why = proof("f:l=n", {"src/a.c": "int (*g(void))(int){return 0;}\n"+cc_source},
                    {"src/a.c": "int (*g(void))(int){return 0;}\n"+cc_source.replace(" l", " n")})
    check("keycheck: function-pointer-returning neighbor no delimiter false reject", ok)
    # Real runner's literal Makefile flags, no silent debug or per-TU flag drift.
    make_text = (nk.ROOT / "Makefile").read_text(encoding="utf-8")
    check("keycheck cc1: exact project flags", "-G8" in nk.cc1_command(make_text, "src/main/368E4.c") and
          "-G0" in nk.cc1_command(make_text, "src/main/32D04.c"))
    try:
        nk.cc1_command(make_text.replace("-O2 -G0", "-g -O2 -G0"), "src/a.c")
        debug_failed = False
    except ValueError:
        debug_failed = True
    check("keycheck cc1: debug flags forbidden", debug_failed)

    with patch.object(nk.shutil, "which", return_value=None):
        try:
            nk.compile_cc1(nk.CppTree({"Makefile": make_text}, "missing"), "src/a.c")
            missing_cc1 = False
        except ValueError as e:
            missing_cc1 = "toolchain missing" in str(e)
    check("keycheck cc1: missing real toolchain fails closed", missing_cc1)
    check("keycheck cc1: Makefile flag appends honored", "-fno-common" in
          nk.cc1_command(make_text + "\nCC_FLAGS += -fno-common\n", "src/a.c"))
    with patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
            patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
            patch.object(nk.subprocess, "run", side_effect=[
                nk.subprocess.CompletedProcess([], 0, "int f(void){return 0;}\n", ""),
                nk.subprocess.CompletedProcess([], 1, "", "undeclared l")]) as mocked:
        result = nk.compile_cc1(nk.CppTree({"Makefile": make_text.replace(
            "tools/gcc-2.7.2/build/cc1", "Makefile")}, "test"), "src/a.c")
    check("keycheck cc1: runner preserves nonzero status and errors", result.returncode == 1 and
          "undeclared" in result.stderr and "-dD" not in mocked.call_args_list[0].args[0])
    with patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
            patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
            patch.object(nk.subprocess, "run", return_value=nk.subprocess.CompletedProcess([], 1, "", "bad include")):
        try:
            nk.compile_cc1(nk.CppTree({"Makefile": make_text.replace(
                "tools/gcc-2.7.2/build/cc1", "Makefile")}, "test"), "src/a.c")
            cpp_compile_failed = False
        except ValueError as e:
            cpp_compile_failed = "cpp failed" in str(e)
    check("keycheck cc1: plain cpp nonzero fails", cpp_compile_failed)
    seen_fresh = []
    def unique_cc1(tree, tu):
        if tree.label == "cc1-fresh":
            seen_fresh.append(tree[tu])
        return fake_cc1(tree, tu)
    cc_check(unique_cc1, "int __kc_fresh_0;int f(int l){return l;}\n")
    check("keycheck cc1: fresh name cannot already occur", seen_fresh and
          "int __kc_fresh_0_;" in seen_fresh[0].replace("(", " ").replace(")", ";"))

    try:
        nk.verify_compiled_edits({"src/a.c": cc_source}, {"src/a.c": cc_source}, {}, scoped={"f:l": "n"})
        no_edit_failed = False
    except ValueError as e:
        no_edit_failed = "applied local edit" in str(e)
    check("keycheck cc1: unchanged local cannot bypass proof", no_edit_failed)

    initializer_trap = "enum E {A=1} h(void){return A;} int l;"
    tokens = [m[0] for m in nk.code_spans(initializer_trap)]
    try:
        nk.cpp_file_declarations(tokens, nk.DECL_WORDS, [], {"l"})
        initializer_failed = False
    except ValueError as e:
        initializer_failed = "unrecognized" in str(e)
    check("keycheck: aggregate enum equals cannot disguise following function body initializer", initializer_failed)

    # Follow-up 19: every include context must reach cc1, including edited .c files.
    included = "int f(int l){return l;}\n"
    for label, caller, extra in (
            ("a1", '#include "b.c"\n', {}),
            ("a1b", '#include "b.c"\n', {}),
            ("a2", '#include "b.c"\n', {}),
            ("a3", '#include "x.h"\n', {"include/x.h": '#include "../src/b.c"\n'}),
            ("a4", '#include "x.h"\n', {"include/x.h": included})):
        owner = "include/x.h" if label == "a4" else "src/b.c"
        files = {"src/a.c": caller, owner: included, **extra}
        compiled = []
        def context_cc1(tree, tu, label=label):
            compiled.append((tree.label, tu))
            if tu == "src/a.c" and tree.label != "cc1-base":
                if label == "a2":
                    return nk.subprocess.CompletedProcess([], 1, "", "undeclared n")
                return nk.subprocess.CompletedProcess([], 0, "sw $0,n\n", "")
            return fake_cc1(tree, tu)
        ok, why = proof("f:l=n", files, {owner: included.replace(" l", " n")}, cc1=context_cc1)
        check("keycheck cc1 include context " + label, not ok and "cc1" in why and
              any(tu == "src/a.c" for _, tu in compiled))
    # Supplied global/asm-alias shapes, with independent prefilter rejection allowed.
    for label, prefix in (("a1-global", 'int l; extern int n __asm__("l");\n'),
                          ("a1b-global", 'extern int n;\n')):
        files = {"src/a.c": prefix + '#include "b.c"\n', "src/b.c": included}
        check("keycheck included globals " + label, not proof("f:l=n", files,
              {"src/b.c": included.replace(" l", " n")})[0])
    cc_calls = []
    def shared_cc1(tree, tu):
        cc_calls.append((tree.label, tu, id(tree), tree["src/b.c"]))
        return fake_cc1(tree, tu)
    files = {"src/a.c": '#include "b.c"\n', "src/b.c": included,
             "src/c.c": '#include "../include/x.h"\n', "include/x.h": '#include "../src/b.c"\n'}
    ok, why = proof("f:l=n", files, {"src/b.c": included.replace(" l", " n")}, compile_headers=False, cc1=shared_cc1)
    check("keycheck cc1: all three include contexts pass", ok and len(cc_calls) == 9)
    check("keycheck cc1: exactly three variant objects across TUs", len({row[2] for row in cc_calls}) == 3)
    check("keycheck cc1: fresh identifier shared across TUs", len({row[3] for row in cc_calls if row[0] == "cc1-fresh"}) == 1)
    # Snapshot materialization itself, not just variant object identity, is reused.
    outputs = {tu: nk.emitted_lines(fake_cpp(files, tu), files, tu) for tu in files if tu.endswith(".c")}
    materialized = []
    def snapshot_cc1(tree, tu):
        tree.materialize()
        return fake_cc1(tree, tu)
    def fake_materialize(tree):
        if tree.directory is None:
            materialized.append(tree.label)
            tree.directory = nk.ROOT / "tmp" / ("fake-" + tree.label)
        return tree.directory
    with patch.object(nk.CppTree, "materialize", fake_materialize), \
            patch.object(nk.CppTree, "close", lambda tree: None), \
            patch.object(nk, "compile_cc1", snapshot_cc1):
        nk.verify_local_cc1(files, outputs, {"f:l": "n"}, {})
    check("keycheck cc1: snapshot builds once per variant", sorted(materialized) ==
          ["cc1-base", "cc1-destination", "cc1-fresh"])
    borrowed_tree = nk.CppTree(files, "base")
    borrowed_tree.directory = nk.ROOT / "tmp" / "fake-borrowed"
    materialized.clear()
    closed = []
    with patch.object(nk.CppTree, "materialize", fake_materialize), \
            patch.object(nk.CppTree, "close", lambda tree: closed.append(tree.label)), \
            patch.object(nk, "compile_cc1", snapshot_cc1):
        nk.verify_local_cc1(files, outputs, {"f:l": "n"}, {}, borrowed_tree)
    check("keycheck cc1: borrows existing base snapshot without closing it",
          sorted(materialized) == ["cc1-destination", "cc1-fresh"] and "cc1-base" not in closed)

    function_cases = {
        "b1": "int l(void);return sizeof(l());",
        "b2": "FN l;return sizeof(l());",
        "b3": "int l(void);return 0;",
        "b4": "int l(void);return l();",
        "b5": "extern int l(void);return 0;",
        "b6": "int l(void);if(0)l();return 0;",
        "b7": "int *l();return 0;",
    }
    for label, body in function_cases.items():
        source = "typedef int FN(void);\nint f(void){" + body + "}\n"
        ok, why = proof("f:l=n", {"src/a.c": source}, {"src/a.c": source.replace(" l", " n").replace("(l", "(n")})
        check("keycheck block function binding " + label, not ok and
              ("function declarator/type" in why or "extern" in why))
    check("keycheck: function typedef aliases rejected", not proof("f:l=n",
          {"src/a.c": "typedef int FN(void);typedef FN FN2;int f(void){FN2 l;return 0;}\n"},
          {"src/a.c": "typedef int FN(void);typedef FN FN2;int f(void){FN2 n;return 0;}\n"})[0])
    macro_extern = '#define EXT extern\nint f(void){EXT int l;return 0;}\n'
    check("keycheck: macro extern cannot certify object binding", not proof("f:l=n",
          {"src/a.c": macro_extern}, {"src/a.c": macro_extern.replace("int l", "int n")},
          cpp=lambda tree, tu: fake_cpp(tree, tu).replace("EXT int", "extern int"))[0])
    # Fresh symbols reject even when identical BASE assembly would mask the comparison.
    def hidden_fresh(tree, tu):
        return nk.subprocess.CompletedProcess([], 0, "__kc_fresh_0:\n", "")
    ok, why = cc_check(hidden_fresh)
    check("keycheck cc1: fresh symbol guard independent of asm equality", not ok and "fresh symbol" in why)
    duplicate_static = "static int f(int l){return l;}\n"
    check("keycheck: duplicate static function scope unsupported", not proof("f:l=n",
          {"src/a.c": duplicate_static, "src/b.c": duplicate_static},
          {"src/a.c": duplicate_static.replace(" l", " n")})[0])
    with patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
            patch.object(nk.CppTree, "materialize", side_effect=OSError(12, "ENOMEM snapshot write")):
        try:
            nk.compile_cc1(nk.CppTree({"Makefile": make_text.replace(
                "tools/gcc-2.7.2/build/cc1", "Makefile")}, "test"), "src/a.c")
            snapshot_failed = False
        except ValueError as e:
            snapshot_failed = "snapshot materialization failed" in str(e) and "ENOMEM" in str(e) and "cannot run toolchain" not in str(e)
    check("keycheck cc1: snapshot failure message accurate", snapshot_failed)
    # Real runner coverage is explicitly skipped if either exact compiler is absent.
    real_cc1 = nk.ROOT / nk.cc1_command(make_text, "src/a.c")[0]
    if not nk.shutil.which(nk.cpp_command(make_text)[0]) or not real_cc1.is_file():
        _skipped += 1
        print("  SKIP: real compile_cc1: project cpp/cc1 toolchain unavailable")
    else:
        tree = nk.CppTree({"Makefile": make_text, "src/a.c": "int f(int l){return l;}\n"}, "real-cc1")
        try:
            result = nk.compile_cc1(tree, "src/a.c")
            check("keycheck: real compile_cc1 emits assembly", result.returncode == 0 and
                  bool(result.stdout.strip()) and "undeclared" not in result.stderr)
        finally:
            tree.close()
        # A shadow whose type a macro spells passes the source-text plan; the
        # binding parser's rerun on real cpp output must refuse it (cc1 cannot:
        # merging two bindings keeps the asm).
        real = {"cpp": nk.preprocess, "cc1": nk.compile_cc1, "deps": nk.cpp_dependencies}
        mt = "#define MT int\nint f(int x) { { int l = x; { "
        for label, text, why_part in [
                ("macro-type parenthesized", mt + "MT (l); l = 3; x += l; } x += l; } return x; }\n",
                 "parenthesized declarator"),
                ("macro-type parenthesized first declarator",
                 mt + "MT (a), *l; a = 1; l = &a; x += *l; } x += l; } return x; }\n", "parenthesized declarator"),
                ("macro-type parenthesized pointer", mt + "MT (*l) = 0; x += l == 0; } x += l; } return x; }\n",
                 "parenthesized declarator"),
                ("function-like macro type", "#define PT(t) t\nint f(int x) { { int l = x; { PT(int) a, l; "
                 "l = 3; x += l; } x += l; } return x; }\n", "found 2"),
                ("macro-type pointer beside a value of it", "#define MT int\nint f(int x) { unsigned MT u = 0; "
                 "{ int l = x; { MT *l = &x; x += *l; } x += l; } return x + u; }\n", "found 2"),
                ("macro typedef pointer after a sibling shadow", "#define TD(n) typedef int n;\nTD(X)\nint f(int x) "
                 "{ { int l = x; { int X = 1; x += X; } { X *l = &x; x += *l; } x += l; } return x; }\n",
                 "shadowed typedef")]:
            # Each case must pass the source plan, so the rerun is what refuses it.
            try:
                planned = bool(nk.scoped_plan({"src/a.c": text}, {"f:l": "nm"}, {}))
            except ValueError:
                planned = False
            ok, why = proof("f:l=nm", {"Makefile": make_text, "src/a.c": text},
                            {"src/a.c": re.sub(r"\bl\b", "nm", text)}, compile_headers=False, **real)
            check("keycheck real gate: rejects " + label, planned and not ok and why_part in why)
        text = "#define MT int\nint f(int x) { { MT a = x; { int b = 0, *l; l = &b; x += *l + a; } } return x; }\n"
        check("keycheck real gate: accepts a nested comma local beside a macro type",
              proof("f:l=nm", {"Makefile": make_text, "src/a.c": text},
                    {"src/a.c": re.sub(r"\bl\b", "nm", text)}, compile_headers=False, **real)[0])

    prefix_text = "".join(f"int pf_{i}(int arg0){{return arg0;}}\n" for i in range(100))
    prefix_output = nk.emitted_lines(fake_cpp({"src/a.c": prefix_text}, "src/a.c"),
                                     {"src/a.c": prefix_text}, "src/a.c")
    with patch.object(nk, "cpp_definition_ranges", wraps=nk.cpp_definition_ranges) as offsets:
        nk.cpp_scope_checks(prefix_output, {f"pf_{i}:arg0": f"fresh_{i}" for i in range(30)}, {}, "base")
    check("keycheck: definition starts computed once per TU across pairs", offsets.call_count == 1)
    with patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
            patch.object(nk.CppTree, "materialize", side_effect=OSError(12, "ENOMEM source snapshot")):
        try:
            nk.preprocess(nk.CppTree({"Makefile": make_text}, "test"), "src/a.c")
            cpp_snapshot_failed = False
        except ValueError as e:
            cpp_snapshot_failed = "snapshot materialization failed" in str(e) and "cannot run" not in str(e)
    check("keycheck cpp: snapshot error identifies write failure", cpp_snapshot_failed)
    # Type-of declarations may hide a global from the reject-only prefilter;
    # genuine locals still accept when cc1 proves their binding independently.
    typeof_global = '#include "globals.def"\nint f(int l){return l;}\n'
    files = {"src/a.c": typeof_global, "include/globals.def": "extern __typeof__(0) n;\n"}
    # Inspect the proof directly: this control does not promise the parser accepts
    # every GNU source form, only that file-scope n alone is not an authority.
    output = nk.emitted_lines(fake_cpp(files, "src/a.c"), files, "src/a.c")
    with patch.object(nk, "compile_cc1", fake_cc1):
        try:
            nk.verify_local_cc1(files, {"src/a.c": output}, {"f:l": "n"}, {})
            typeof_ok = True
        except ValueError:
            typeof_ok = False
    check("keycheck cc1: genuine local may shadow typeof global destination", typeof_ok)

    failed_tree = nk.CppTree({}, "failed-create")
    failed_tree.directory = nk.ROOT / "tmp" / ("keycheck-cpp-uncreated-" + nk.uuid.uuid4().hex)
    try:
        failed_tree.close()
        cleanup_ok = True
    except OSError:
        cleanup_ok = False
    check("keycheck: failed snapshot creation cleanup cannot mask original error", cleanup_ok)

    # Follow-up 20: name-outward declarator classification, not adjacent-star guesses.
    function_shapes = {
        "f1": ("typedef int *FN(void);", "FN l;return sizeof(l());"),
        "f1b": ("typedef int *FN(void);", "FN l;return 0;"),
        "f1c": ("typedef int *FN(void);", "FN l;if(0)return *l();return 0;"),
        "f2": ("int g(void);typedef __typeof__(g) FN;", "FN l;return 0;"),
        "f3": ("typedef __typeof__(int(void)) FN;", "FN l;return 0;"),
        "f4": ("typedef int X,*FN(void);", "FN l;return 0;"),
        "f7": ("typedef int *FN(void);", "static FN l;return 0;"),
    }
    for label, (prefix, body) in function_shapes.items():
        source = prefix + "\nint f(void){" + body + "}\n"
        check("keycheck function typedef " + label, not proof("f:l=n",
              {"src/a.c": source, "src/other.c": "int *l(void){return 0;}\n"},
              {"src/a.c": re.sub(r"\bl\b", "n", source)})[0])
    for decl in ("typedef int *FN(void);", "typedef int (FN)(void);", "typedef int **FN(int);"):
        classes = nk._typedef_classes((decl,))
        check("keycheck: function typedef structurally classified " + decl, classes.get("FN") == "function")
    for decl in ("typedef int (*FN)(void);", "typedef int *(*FN)(void);", "typedef int FN[4];"):
        check("keycheck: object typedef structurally classified " + decl,
              nk._typedef_classes((decl,)).get("FN") == "object")
    for decl in ("typedef int X,*FN(void);", "typedef __typeof__(int(void)) FN;",
                 "#define DECL typedef int FN(void);\n"):
        check("keycheck: uncertain typedef fails closed " + decl,
              nk._typedef_classes((decl,)).get("FN") == "unknown")
    for source in ("typedef int FN(void);int f(void){FN *l=0;return l==0;}\n",
                   "typedef int *FN(void);int f(void){FN *l=0;return l==0;}\n",
                   "typedef int (*FN)(void);int f(void){FN l=0;return l==0;}\n"):
        check("keycheck: known function pointer objects accepted", proof("f:l=n",
              {"src/a.c": source}, {"src/a.c": re.sub(r"\bl\b", "n", source)})[0])
    macro_type = '#define TYPE int\ntypedef TYPE FN;int f(void){FN l=0;return l;}\n'
    check("keycheck: macro-produced typedef object not guessed", not proof("f:l=n",
          {"src/a.c": macro_type}, {"src/a.c": re.sub(r"\bl\b", "n", macro_type)},
          cpp=lambda tree, tu: fake_cpp(tree, tu).replace("typedef TYPE", "typedef int"))[0])
    # Compiled typedef whose raw declaration came from a macro remains uncertain.
    macro_source = '#define DECL typedef int FN;\nDECL\nint f(void){FN l=0;return l;}\n'
    macro_output = nk.emitted_lines(fake_cpp({"src/a.c": macro_source}, "src/a.c").replace("\nDECL\n", "\ntypedef int FN;\n"),
                                   {"src/a.c": macro_source}, "src/a.c")
    try:
        nk.cpp_scope_checks(macro_output, {"f:l": "n"}, {}, "base", {"src/a.c": macro_source})
        macro_unknown = False
    except ValueError as e:
        macro_unknown = "uncertain typedef" in str(e)
    check("keycheck: cpp macro-produced typedef provenance rejects", macro_unknown)

    graph_make = make_text
    graph_files = {"Makefile": graph_make, "src/a.c": '#include "b.c"\n',
                   "src/b.c": included, "src/unrelated.c": "int z;\n",
                   "src/c.c": '#include "x.h"\n', "include/x.h": '#include "../src/b.c"\n'}
    dependency_calls.clear()
    selected_runs = []
    def selection_cpp(tree, tu):
        selected_runs.append((tree.label, tu))
        return fake_cpp(tree, tu)
    ok, why = proof("f:l=n", graph_files, {"src/b.c": included.replace(" l", " n")},
                    compile_headers=False, cpp=selection_cpp)
    check("keycheck: real dependency seam queries every TU in both trees", ok and
          len(dependency_calls) == 2 * len({p for p in graph_files if p.startswith("src/") and p.endswith(".c")}) and
          set(dependency_calls) == {(label, tu) for label in ("base", "new")
              for tu in graph_files if tu.startswith("src/") and tu.endswith(".c")})
    check("keycheck: full preprocessing excludes cpp-unreachable TU", ok and
          not any(tu == "src/unrelated.c" for _, tu in selected_runs))
    with patch.object(nk, "cpp_dependencies", lambda tree, tu: {tu, "src/b.c"}):
        candidates = nk.dependency_census(nk.CppTree(graph_files, "base"), {"src/b.c"})
    check("keycheck: cpp dependency result is sole selection authority",
          candidates == {p for p in graph_files if p.startswith("src/") and p.endswith(".c")})

    # Literal Makefile parsing must reject hidden configuration overrides.
    for line in ("src/a.o: CPP_FLAGS += -Dhidden", "override CC_FLAGS += -g", "export CPP_DEFS := -Dhidden",
                 "override GP_FILES := main/a", "export NO_SR_FILES := main/a", "include extra.mk", "-include extra.mk",
                 "export override CPP_FLAGS += -Dhidden", "override define CC_FLAGS\n-g\nendef"):
        for command in (lambda text: nk.cpp_command(text), lambda text: nk.cc1_command(text, "src/a.c")):
            try:
                command(make_text + "\n" + line + "\n")
                hidden_make = False
            except ValueError as e:
                hidden_make = "unsupported Makefile" in str(e)
            check("keycheck: hidden Make toolchain override " + line, hidden_make)

    # A transient ENOMEM cleanup retries; persistent cleanup never hides proof errors.
    cleanup_tree = nk.CppTree({}, "cleanup")
    cleanup_tree.directory = nk.ROOT / "tmp" / "keycheck-cpp-fake-cleanup"
    with patch.object(Path, "exists", return_value=True), patch.object(nk.shutil, "rmtree",
            side_effect=[OSError(12, "ENOMEM"), None]) as removed, contextlib.redirect_stderr(io.StringIO()):
        cleanup_tree.close()
    check("keycheck: cleanup ENOMEM retries and removes snapshot", removed.call_count == 2)
    closed = []
    def broken_close(tree):
        closed.append(tree.label)
        if tree.label == "first":
            raise OSError(12, "ENOMEM cleanup")
    try:
        try:
            raise ValueError("primary compiler failure")
        finally:
            with patch.object(nk.CppTree, "close", broken_close), contextlib.redirect_stderr(io.StringIO()) as cleanup_notes:
                nk.close_cpp_trees([nk.CppTree({}, "first"), nk.CppTree({}, "second")])
    except ValueError as e:
        kept_primary = str(e) == "primary compiler failure"
    check("keycheck: cleanup visits every tree and preserves proof outcome", kept_primary and closed == ["first", "second"]
          and "snapshot cleanup failed" in cleanup_notes.getvalue())
    with patch.object(nk.CppTree, "close", side_effect=OSError(12, "ENOMEM cleanup")):
        try:
            nk.close_cpp_trees([nk.CppTree({}, "first")])
            cleanup_reject = False
        except ValueError as e:
            cleanup_reject = "snapshot cleanup failed" in str(e)
    check("keycheck: cleanup failure cannot silently accept successful proof", cleanup_reject)
    with patch.object(Path, "glob", return_value=[cleanup_tree.directory]), \
            patch.object(Path, "is_dir", return_value=True), \
            patch.object(Path, "stat", side_effect=FileNotFoundError("concurrent removal")):
        nk.cleanup_cpp_snapshots(nk.ROOT / "tmp")
    check("keycheck: stale cleanup tolerates concurrent disappearance", True)
    # Real differential pair coverage, beyond merely invoking compile_cc1.
    if not nk.shutil.which(nk.cpp_command(make_text)[0]) or not real_cc1.is_file():
        _skipped += 1
        print("  SKIP: real cc1 local/global differential pairs: project toolchain unavailable")
    else:
        for source, expected in (("int f(int l){return l+1;}\n", True),
                                 ("int l;int f(void){return l;}\n", False)):
            files = {"Makefile": make_text, "src/a.c": source}
            tree = nk.CppTree(files, "real-pair")
            try:
                output = nk.emitted_lines(nk.preprocess(tree, "src/a.c"), tree, "src/a.c")
                try:
                    nk.verify_local_cc1(files, {"src/a.c": output}, {"f:l": "n"}, {}, tree)
                    actual = True
                except ValueError:
                    actual = False
                check("keycheck: real cc1 differential local/global pair", actual == expected)
            finally:
                tree.close()

    nested = "int CdIntToPos(void){int ENCODE_BCD(int l){return l;}return ENCODE_BCD(0);}\n"
    ok, why = proof("ENCODE_BCD:l=n", {"src/a.c": nested},
                    {"src/a.c": re.sub(r"\bl\b", "n", nested)})
    check("keycheck: GNU nested function target explicitly unsupported", not ok and "function definition" in why)

    # Follow-up 21: real dependency records defeat the textual-lexer/path vectors.
    include_vectors = {
        "h_apos": '#if 0\nit\'s /*\n#endif\n#include "b.c"\n#if 0\n*/\n#endif\n',
        "h_dquote": '#if 0\n" /*\n#endif\n#include "b.c"\n#if 0\n*/\n#endif\n',
        "h_bsspace": '/* x *\\   \n/\n#include "b.c"\n',
        "h_bom": '\ufeff#include "b2.h"\n',
        "h_nul": '\x00#include "b.c"\n',
        "h_dotdot": '#include "q/../b.h"\n',
        "h_case": '#include "B.c"\n',
        "h_ctl_plain": '#include "b.c"\n',
    }
    for label, directive in include_vectors.items():
        owner = "include/b.h" if label == "h_dotdot" else "src/b.c"
        fn = '#ifndef IN_A\ntypedef int T;\n#endif\nstatic int f(void){T *l;l=0;return 1;}\n'
        files = {"src/a.c": 'int T;int l;\n#include "n.def"\n#define IN_A\n' + directive,
                 owner: fn, "include/n.def": 'extern __typeof__(l) n __asm__("l");\n',
                 "include/b2.h": '#include "../src/b.c"\n'}
        if label == "h_dotdot":
            files["src/b.c"] = '#include "../include/b.h"\n'
            files["src/b.h"] = "int decoy;\n"
        if label == "h_case":
            files["include/B.c"] = "int decoy;\n"
        queries = []
        def vector_deps(tree, tu, owner=owner):
            queries.append((tree.label, tu))
            return {tu, owner}
        def vector_cpp(tree, tu, owner=owner):
            view = dict(tree)
            if tu == "src/a.c":
                # Real cpp has entered the actual owner, despite each lexical/path
                # trap; markers here reproduce that compiler-selected closure.
                view[tu] = 'int T;int l;\n#include "n.def"\n#define IN_A\n#include "../' + owner + '"\n'
            return fake_cpp(view, tu)
        ok, why = proof("f:l=n", files, {owner: re.sub(r"\bl\b", "n", fn)},
                        compile_headers=False, cpp=vector_cpp, deps=vector_deps)
        check("keycheck cpp dependency vector " + label, not ok and
              (("base", "src/a.c") in queries or label == "h_case" and "case-alias" in why))
    # Physical identity handles case-insensitive aliases even on a sensitive host.
    tree = nk.CppTree({"src/b.c": ""}, "identity")
    from types import SimpleNamespace
    with patch.object(nk.os, "stat", return_value=SimpleNamespace(st_dev=7, st_ino=42)):
        same_case = nk.dependency_identity(tree, "src/b.c") == nk.dependency_identity(tree, "src/B.c")
    check("keycheck: dependency identity uses real inode for case alias", same_case)
    for failing in ("src/a.c", "src/unrelated.c"):
        def failed_deps(tree, tu, failing=failing):
            if tu == failing:
                raise ValueError("dependency cpp failed: " + tu)
            return {tu, "src/b.c"}
        try:
            with patch.object(nk, "cpp_dependencies", failed_deps):
                nk.dependency_census(nk.CppTree(graph_files, "base"), {"src/b.c"})
            census_failed = False
        except ValueError as e:
            census_failed = "dependency cpp failed" in str(e)
        check("keycheck: any dependency TU failure rejects " + failing, census_failed)
    with patch.object(nk, "cpp_dependencies", lambda tree, tu: {"src/b.c"}):
        try:
            nk.dependency_census(nk.CppTree(graph_files, "base"), {"src/b.c"})
            omitted_tu = False
        except ValueError as e:
            omitted_tu = "omitted its TU" in str(e)
    check("keycheck: dependency records cannot omit their TU", omitted_tu)
    counts = {"active": 0, "peak": 0}
    lock = nk.threading.Lock()
    def parallel_deps(tree, tu):
        with lock:
            counts["active"] += 1
            counts["peak"] = max(counts["peak"], counts["active"])
        time.sleep(0.002)
        with lock:
            counts["active"] -= 1
        return {tu}
    with patch.object(nk, "cpp_dependencies", parallel_deps), patch.object(nk.os, "cpu_count", return_value=3):
        nk.dependency_census(nk.CppTree({f"src/dep_{i}.c": "" for i in range(12)}, "base"), {"src/dep_0.c"})
    check("keycheck: dependency census bounded parallel pool", 1 < counts["peak"] <= 3)
    # The actual subprocess boundary uses -M and parses escaped dependency paths.
    with patch.object(nk, "validate_cpp_dependencies", side_effect=lambda tree, paths, *args: paths), patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
            patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
            patch.object(nk.subprocess, "run", return_value=nk.subprocess.CompletedProcess(
                [], 0, '__kc__: src/a.c include/with\\ space.h \\\n include/h.h\n', '')) as dep_run:
        paths = nk.cpp_dependencies(nk.CppTree({"Makefile": make_text}, "base"), "src/a.c")
    check("keycheck: real dependency runner uses flags and escaped Make paths",
          paths == {"src/a.c", "include/with space.h", "include/h.h"} and
          "-M" in dep_run.call_args.args[0] and "-Iinclude" in dep_run.call_args.args[0])
    with patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
            patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
            patch.object(nk.subprocess, "run", return_value=nk.subprocess.CompletedProcess([], 1, '', 'bad include')):
        try:
            nk.cpp_dependencies(nk.CppTree({"Makefile": make_text}, "base"), "src/a.c")
            dependency_nonzero = False
        except ValueError as e:
            dependency_nonzero = "dependency cpp failed" in str(e)
    check("keycheck: dependency cpp nonzero fails closed", dependency_nonzero)

    # Follow-up 23: one reject-only shape gate surrounds both census paths.
    from types import SimpleNamespace
    def shape_fails(files, reason):
        try:
            nk.plain_tree(files)
            return False
        except ValueError as e:
            return "scoped tree shape" in str(e) and reason in str(e)
    for label in ("t_bslash", "c_bslash"):
        check("keycheck plain shape " + label,
              shape_fails({"src/q\\": "", "src/b.c": ""}, "non-plain path"))
    for label, text in (
            ("t_hasinc_splice", '#if __has_incl\\\nude("b.c")\n#endif\n'),
            ("c_hasinc_splice", '#if __has_incl\\\nude_next("b.c")\n#endif\n'),
            ("t_hasinc_paste", '#define JOIN(a,b) a ## b\n#if JOIN(__has_,include)("b.c")\n#endif\n'),
            ("c_hasinc_paste", '#define JOIN(a,b) a ## b\n#if JOIN(__has_,include_next)("b.c")\n#endif\n')):
        check("keycheck plain shape " + label,
              shape_fails({"src/a.c": text.replace("\\n", "\n")}, "hazard"))
    for mode, path in (("120000", "include"), ("120000", "docs/link"),
                       ("160000", "tools/submodule")):
        with patch.object(nk, "git", return_value=mode + " blob " + "1"*40 + "\t" + path + "\0"):
            try:
                nk.read_base_tree("HEAD")
                rootlink = False
            except ValueError as e:
                rootlink = "scoped tree shape" in str(e) and "symlink/submodule" in str(e)
        check("keycheck c_rootlink/all-tree modes " + mode + " " + path, rootlink)
    for path in ("src/a b.c", "include/a#b.h", "asm/a$b.s", "q'.h", "src/./b.c", "src/../b.c"):
        check("keycheck plain path catalog " + path, shape_fails({path: ""}, "non-plain"))
    for text, reason in (('#include HEADER\n', "non-literal"),
                         ('#include_next "b.c"\n', "unsupported directive"),
                         ('#include "../b.c"\n', "non-plain"),
                         ('#define MAKE include #x\n', "stringizing"),
                         ('#in\\\nclude HEADER\n', "non-literal"),
                         ('#include "b.c"\\  \n', "line splice")):
        check("keycheck plain include gate " + text,
              shape_fails({"include/any.h": text}, reason))
    check("keycheck plain spliced literal include allowed", not shape_fails(
          {"src/a.c": '#in\\\nclude "b.c"\n', "src/b.c": ""}, ""))
    for flags in ("-isystem include", "-isysteminclude", "-idirafter include",
                  "-include include/a.h", "-imacros include/a.h", "--sysroot=/x",
                  "-B/x", "-iprefix include", "-IInclude", "-iquoteinclude",
                  "-Iother", "-I include/../include"):
        check("keycheck m_isystem_alias/flag shapes " + flags, shape_fails(
              {"Makefile": make_text + "\nCPP_FLAGS += " + flags + "\n", "src/a.c": "",
               "include/a.h": ""}, "scoped tree shape"))
    for flags in ("-I include", "-iquote include"):
        check("keycheck plain tracked include flag allowed " + flags, not shape_fails(
              {"Makefile": make_text + "\nCPP_FLAGS += " + flags + "\n",
               "include/a.h": ""}, ""))
    for path in nk.ROOT_METADATA:
        check("keycheck root metadata catalog exemption " + path,
              not shape_fails({path: ""}, ""))
        check("keycheck metadata still not include target " + path,
              shape_fails({"src/a.c": '#include "' + path + '"\n'}, "non-plain"))
    gas = "## GTE instruction macros\n## Instructions which take an argument\n"
    check("keycheck exact GAS comments excluded", not shape_fails(
          {"include/gte_macros.inc": gas}, ""))
    check("keycheck unused assembler directives not cpp-checked", not shape_fails(
          {"include/gte_macros.inc": gas + "#include HEADER\n"}, ""))
    check("keycheck tracked assembler bytes remain LF-only", shape_fails(
          {"include/macro.inc": "\r\n"}, "control byte"))
    check("keycheck configured cpp roots also gated", shape_fails(
          {"Makefile": make_text + "\nCPP_FLAGS += -I headers\n",
           "include/a.h": "", "headers/a.h": "#include HEADER\n"}, "non-literal"))
    # No disk links/directories needed: emulate lstat/working-tree inventory.
    normal = SimpleNamespace(st_mode=nk.stat_mode.S_IFDIR, st_file_attributes=0)
    root = nk.ROOT
    for dirs, names, suffix in (([], [], ""), ([], ["scratch.h"], "/zz")):
        with patch.object(nk.os, "walk", return_value=iter([(str(root / ("src" + suffix)), dirs, names)])), \
                patch.object(nk.os, "lstat", return_value=normal):
            try:
                nk.working_shape([("100644", "src/b.c")], [])
                empty_failed = False
            except ValueError as e:
                empty_failed = "empty/untracked directory" in str(e)
        check("keycheck t_emptydir/untracked directory " + suffix, empty_failed)
    for mode, attrs in ((nk.stat_mode.S_IFLNK, 0), (nk.stat_mode.S_IFDIR, 0x400)):
        with patch.object(nk.os, "walk", return_value=iter([(str(root / "src"), [], ["b.c"])])), \
                patch.object(nk.os, "lstat", return_value=SimpleNamespace(st_mode=mode, st_file_attributes=attrs)):
            try:
                nk.working_shape([("100644", "src/b.c")], [])
                link_failed = False
            except ValueError as e:
                link_failed = "symlink/junction" in str(e)
        check("keycheck working link/junction shape " + str(attrs), link_failed)
    try:
        nk.working_shape([], ["include/not plain.h"])
        untracked_failed = False
    except ValueError as e:
        untracked_failed = "non-plain path" in str(e)
    check("keycheck nonignored untracked path shape", untracked_failed)
    # Read-only real-tree gate (not cpp or cc1): all contents, modes and dirs.
    try:
        actual_plain = nk.read_base_tree("HEAD")
        nk.plain_tree(actual_plain)
        nk.read_working_tree()
        real_shape_passed = True
    except (ValueError, OSError, nk.subprocess.CalledProcessError) as e:
        print("  real-tree shape gate: " + str(e))
        real_shape_passed = False
    check("keycheck real base/working trees pass shape gate", real_shape_passed)

    @contextlib.contextmanager
    def real_shape_fixture():
        fixture_parent = nk.ROOT / "tmp" / "codex"
        directory = fixture_parent / ("fu25-fixture-" + nk.uuid.uuid4().hex)
        directory.mkdir(parents=True)
        try:
            (directory / "src").mkdir()
            (directory / "include").mkdir()
            (directory / "src/a.c").write_bytes(b"int x;\n")
            (directory / "include/a.h").write_bytes(b"int y;\n")
            def inventory(*args, **kwargs):
                if "--stage" in args:
                    return "100644 " + "1"*40 + " 0\tsrc/a.c\0" + "100644 " + "2"*40 + " 0\tinclude/a.h\0"
                if "--others" in args:
                    return ".venv\0include/new.h\0" if (directory / "include/new.h").exists() else ".venv\0"
                return ""
            with patch.object(nk, "ROOT", directory), patch.object(nk, "git", inventory):
                yield directory
        finally:
            # Confined UUID fixture, no links are followed by rmtree.
            assert directory.parent == fixture_parent
            assert directory.resolve().is_relative_to(fixture_parent.resolve())
            nk.shutil.rmtree(directory)

    # Follow-up 24: byte/directive allowlists precede all comment interpretation.
    with real_shape_fixture() as directory:
        for control in ("\x00", "\x0c", "\x0b", "\r", "\x1b", "\x7f"):
            for mode in ("commit", "working"):
                text = '#in\\' + control + '\nclude "b.c"\n'
                try:
                    if mode == "working":
                        (directory / "src/a.c").write_bytes(text.encode("utf-8"))
                        nk.read_working_tree()
                    else:
                        raw = text.encode("utf-8")
                        oid = "1"*40
                        result = f"{oid} blob {len(raw)}\n".encode() + raw + b"\n"
                        with patch.object(nk, "git", return_value=f"100644 blob {oid}\tsrc/a.c\0"), \
                                patch.object(nk.subprocess, "run", return_value=SimpleNamespace(stdout=result)):
                            nk.read_base_tree("HEAD")
                    refused = False
                except ValueError as e:
                    refused = "control byte" in str(e)
                check("keycheck S byte splice " + mode + " " + repr(control), refused)
    check("keycheck S lone CR comment", shape_fails(
          {"src/a.c": '// x\r#include "b.c"\n'}, "control byte"))
    check("keycheck S NUL directive", shape_fails(
          {"src/a.c": '#\x00include "b.c"\n'}, "control byte"))
    check("keycheck S UTF-8 comments accepted", not shape_fails(
          {"src/a.c": "/* café — valid UTF-8 */\nint x;\n"}, ""))
    check("keycheck S invalid UTF-8 offset", shape_fails(
          {"src/a.c": "int x;" + chr(0xdc80)}, "invalid UTF-8"))
    for directive in ("import", "include_next", "ident", "sccs", "assert", "unassert", "unknown"):
        check("keycheck S directive name " + directive, shape_fails(
              {"src/a.c": '#' + directive + ' "b.c"\n'}, "unsupported directive"))
    check("keycheck S spliced import", shape_fails(
          {"src/a.c": '#im\\\nport "b.c"\n'}, "unsupported directive"))
    for text in ("#define Q '/*\n#include HEADER\n*/\n", "#warning it's /*\n#include HEADER\n*/\n",
                 '#error "oops\n', "#define Q \\\n'/*\n",
                 "/* prefix\n*/ #warning it's /*\n#include HEADER\n*/\n"):
        check("keycheck S unterminated directive quote " + repr(text),
              shape_fails({"src/a.c": text}, "unterminated directive quote"))
    check("keycheck S valid directive strings", not shape_fails(
          {"src/a.c": '#define Q "/* and ## are literal"\n#\n'}, ""))
    check("keycheck nonblocking README directives ignored", not shape_fails(
          {"include/README.md": "# arbitrary heading\n## subheading\n", "src/a.c": "int x;"}, ""))
    check("keycheck nonblocking literal header text scanned", shape_fails(
          {"src/a.c": '/*prefix*/ #include "settings.h"\n',
           "include/settings.h": '#import "b.c"\n'}, "unsupported directive"))
    check("keycheck nonblocking non-cpp README bytes excluded", not shape_fails(
          {"include/README.md": "# header\x00"}, ""))
    cached_shape = nk.ShapeTree({"src/a.c": "int x;\n"})
    with patch.object(nk, "plain_bytes", wraps=nk.plain_bytes) as checked:
        nk.plain_tree(cached_shape)
        nk.plain_tree(cached_shape)
    check("keycheck shape success cache", checked.call_count == 1)
    cached_shape["src/a.c"] = "int x;\r\n"
    check("keycheck shape cache invalidates on mutation", shape_fails(cached_shape, "control byte"))
    with patch.object(Path, "open", return_value=io.StringIO("int x;\r\n")) as raw_open:
        raw_working = nk.shape_read(Path("src/a.c"))
    check("keycheck S working reader preserves raw CR", raw_working == "int x;\r\n"
          and raw_open.call_args.kwargs.get("newline") == ""
          and shape_fails({"src/a.c": raw_working}, "control byte"))
    check("keycheck Make parser does not claim complete export refusal",
          nk.cpp_command(make_text + "\nexport UNRELATED := value\n") == nk.cpp_command(make_text))
    # Real root entries, not labels over identical mock inventories.
    for venv_kind in ("file", "symlink"):
        with real_shape_fixture() as directory:
            if venv_kind == "file":
                (directory / ".venv").write_bytes(b"root fixture")
            else:
                (directory / "venv-target").mkdir()
                try:
                    (directory / ".venv").symlink_to(directory / "venv-target", target_is_directory=True)
                except OSError as e:
                    skip("keycheck real .venv symlink", str(e))
                    continue
            nk.read_working_tree()
            check("keycheck V real root venv ignored " + venv_kind,
                  (directory / ".venv").is_symlink() if venv_kind == "symlink" else (directory / ".venv").is_file())
    for phase in ("shape", "read"):
        def unreadable_walk(*args, **kwargs):
            kwargs["onerror"](PermissionError("fixture unreadable"))
            return iter([])
        with patch.object(nk.os, "walk", unreadable_walk):
            try:
                if phase == "shape":
                    nk.working_shape([], [])
                else:
                    nk.read_working_tree()
                unreadable_failed = False
            except ValueError as e:
                unreadable_failed = "unreadable directory" in str(e)
        check("keycheck unreadable directory rejects " + phase, unreadable_failed)
    for path in ("../plugin/../include/iso646.h", "include/../include/iso646.h"):
        dep_tree = nk.CppTree({}, "dotdot-fixture")
        dep_tree.directory = nk.ROOT / "tmp" / "cpp-dependency-fixture"
        with patch.object(nk.os, "stat", side_effect=AssertionError("dotdot must reject before stat")):
            try:
                nk.validate_cpp_dependencies(dep_tree, {path}, "cpp", [])
                dotdot_failed = False
            except ValueError as e:
                dotdot_failed = "dotdot component" in str(e)
        check("keycheck S dependency dotdot " + path, dotdot_failed)

    # Follow-up 25 othern shapes: refusal replaces modelling once/header metadata.
    for label, text in (
            ("t_once_mtime_othern", "#pragma once\n"),
            ("t_pragmaop_once_othern", '_Pragma("once")\n'),
            ("t_skiphide_hasinc_othern", "#if 0\nit's /*\n#endif\n#if __has_incl\\\nude(\"b.c\")\n#endif\n*/\n"),
            ("t_sep_u2028_hasinc_othern", "// hidden\u2028#if __has_include(\"b.c\")\n"),
            ("t_sep_u2029_hasinc_othern", "// hidden\u2029#if __has_include(\"b.c\")\n"),
            ("t_sep_nel_hasinc_othern", "// hidden\u0085#if __has_include(\"b.c\")\n"),
            ("t_bom_define_hasinc_othern", "\ufeff#define Q __has_include(\"b.c\")\n"),
            ("t_bomimport_dedup_othern", "\ufeff#import \"b.c\"\n")):
        check("keycheck C othern " + label, shape_fails({"src/a.c": text}, "scoped tree shape"))
    for separator in ("\ufeff", "\u2028", "\u2029", "\u0085"):
        check("keycheck C Makefile separator rejected " + repr(separator), shape_fails(
              {"Makefile": make_text + separator, "include/a.h": ""}, "BOM/Unicode"))
    _, ordinary_code = nk.source_quotes("src/a.c", "#if 0\nit's /*\n#endif\n__has_include(x)\n")
    check("keycheck ordinary quotes end at LF", "__has_include" in ordinary_code and "#endif" in ordinary_code)
    with patch.object(nk, "cpp_dependencies", side_effect=AssertionError("scan must precede cpp")):
        try:
            nk.dependency_census(nk.CppTree({"src/a.c": '__has_incl\\\nude("x")\n'}, "splice"), {"src/a.c"})
            joined_scan = False
        except ValueError as e:
            joined_scan = "__has_include" in str(e)
    check("keycheck census joins probe splices", joined_scan)
    check("keycheck M asm bytes not checked", not shape_fails(
          {"src/a.c": "int x;", "asm/header.s": "\r" + chr(0xdc80)}, ""))
    check("keycheck M case-alias target refused", shape_fails(
          {"src/a.c": '#include "SETTINGS.h"\n', "include/settings.h": '#import "x"\n'}, "case-alias"))
    check("keycheck M exact target checked", shape_fails(
          {"src/a.c": '#include "settings.h"\n', "include/settings.h": '#import "x"\n'}, "unsupported directive"))
    with real_shape_fixture() as directory:
        # Ignored cache exists on disk but Git's non-ignored inventory omits it.
        (directory / "include/m2c_context.h.m2c").write_bytes(b"\x80\x00binary cache")
        result = nk.read_working_tree()
        check("keycheck I unused ignored binary mirrored unchecked",
              result["include/m2c_context.h.m2c"].encode("utf-8", "surrogateescape")
              == b"\x80\x00binary cache")
        snapshot = nk.CppTree(result, "ignored-binary")
        try:
            check("keycheck I snapshot preserves binary bytes",
                  (snapshot.materialize() / "include/m2c_context.h.m2c").read_bytes()
                  == b"\x80\x00binary cache")
        finally:
            snapshot.close()
        (directory / "include/m2c_context.h").write_bytes(b"#pragma once\n")
        check("keycheck I unused ignored text header unchecked",
              "include/m2c_context.h" in nk.read_working_tree())
        (directory / "include/new.h").write_bytes(b"\x80bad nonignored file")
        try:
            nk.read_working_tree()
            bad_new = False
        except ValueError as e:
            bad_new = "invalid UTF-8" in str(e) and "include/new.h" in str(e)
        check("keycheck M nonignored untracked bad header rejects", bad_new)

    # Follow-up 26: actual readers and dependency census, not inventory labels.
    with real_shape_fixture() as directory:
        (directory / "src/a.c").write_bytes(b'#include "a.h"\nint x;\n')
        (directory / "src/a.h").write_bytes(b'#include "../include/a.h"\n')
        working = nk.read_working_tree()
        check("keycheck I ignored same-directory header mirrored", "src/a.h" in working)
        tree = nk.CppTree(working, "ignored-shadow")
        with patch.object(nk, "cpp_dependencies", fake_dependencies):
            try:
                nk.dependency_census(tree, {"include/a.h"})
                shadow_ok = False
            except ValueError as e:
                shadow_ok = "src/a.h" in str(e) and "ignored" in str(e)
        check("keycheck I ignored shadow dependency rejected", shadow_ok)
        for text, reason in ((b"\x80bad", "invalid UTF-8"),
                             (b"#pragma once\n", "unsupported directive"),
                             (b'R"(a"/*)"\n', "raw string")):
            (directory / "src/a.h").write_bytes(text)
            tree = nk.CppTree(nk.read_working_tree(), "ignored-opened")
            tree._working_nonignored = None  # Commit-like fixture isolates dependency text gate.
            with patch.object(nk, "cpp_dependencies", fake_dependencies):
                try:
                    nk.dependency_census(tree, {"src/a.c"})
                    opened_failed = False
                except ValueError as e:
                    opened_failed = reason in str(e)
            check("keycheck I ignored opened header text-checked " + reason, opened_failed)
        (directory / "src/a.h").write_bytes(b"int y;\n")
        tree = nk.CppTree(nk.read_working_tree(), "ignored-shadow-positive")
        with patch.object(nk, "cpp_dependencies", fake_dependencies):
            try:
                nk.dependency_census(tree, {"src/a.h"})
                shadow_rejected = False
            except ValueError as e:
                shadow_rejected = "ignored: src/a.h" in str(e)
        check("keycheck B1 included ignored header rejects", shadow_rejected)
        # A dependency of an unrelated TU is checked before census filtering.
        tree._working_nonignored = None
        tree["src/b.c"] = "int z;\n"
        tree["include/unused.inc"] = "#import \"x\"\n"
        with patch.object(nk, "cpp_dependencies", lambda _, tu:
                          {tu, "include/unused.inc"} if tu == "src/b.c" else {tu}):
            try:
                nk.dependency_census(tree, {"src/a.c"})
                unrelated_failed = False
            except ValueError as e:
                unrelated_failed = "unsupported directive" in str(e)
        check("keycheck I all TU dependencies checked before selection", unrelated_failed)

    for prefix in ("R", "LR", "uR", "UR", "u8R"):
        check("keycheck R raw prefix " + prefix, shape_fails(
              {"src/a.c": prefix + '\"(a\"/*)\"\n#pragma once\n'}, "raw string"))
    check("keycheck R longer identifier is not raw prefix", not shape_fails(
          {"src/a.c": '#define Q FOOR"x"\n'}, ""))
    with real_shape_fixture() as directory:
        for hazard in ('#pragma once', '#import "b.c"', '#include MACRO', '#define X a ## b'):
            text = 'R"(a"/*)"\n' + hazard + "\n"
            for mode in ("working", "commit"):
                try:
                    if mode == "working":
                        (directory / "src/a.c").write_bytes(text.encode("utf-8"))
                        nk.read_working_tree()
                    else:
                        raw, oid = text.encode("utf-8"), "1"*40
                        batch = f"{oid} blob {len(raw)}\n".encode() + raw + b"\n"
                        with patch.object(nk, "git", return_value=f"100644 blob {oid}\tsrc/a.c\0"), \
                                patch.object(nk.subprocess, "run", return_value=SimpleNamespace(stdout=batch)):
                            nk.read_base_tree("HEAD")
                    raw_failed = False
                except ValueError as e:
                    raw_failed = "raw string" in str(e)
                check("keycheck R real reader " + mode + " " + hazard, raw_failed)

    def create_fifo_fixture(path):
        try:
            nk.os.mkfifo(path)
        except OSError as e:
            skip("keycheck actual FIFO rejected", f"mkfifo failed on fixture filesystem: {e}")
            return False
        return True

    with patch.object(nk.os, "mkfifo", create=True,
                      side_effect=OSError(95, "Operation not supported")), \
            patch.object(sys.modules[__name__], "skip") as skipped_fifo:
        check("keycheck FIFO filesystem refusal skips without crashing",
              not create_fifo_fixture(nk.ROOT / "tmp" / "codex" / "unused-fifo")
              and skipped_fifo.call_count == 1 and "95" in skipped_fifo.call_args.args[1])

    # Follow-up 27: ignored contexts are forbidden; unused caches still pass.
    with real_shape_fixture() as directory:
        for name in (".368E4.c.swp", "game.h~", ".DS_Store"):
            (directory / "include" / name).write_bytes(b"\x80\x00ignored junk")
        with patch.object(nk, "shape_read", wraps=nk.shape_read) as reads:
            working = nk.read_working_tree()
        check("keycheck ignored nonplain editor junk skipped",
              all("include/" + name not in working for name in
                  (".368E4.c.swp", "game.h~", ".DS_Store"))
              and all(call.args[0].name not in (".368E4.c.swp", "game.h~", ".DS_Store")
                      for call in reads.call_args_list))
        tree = nk.CppTree(working, "unused-ignored")
        with patch.object(nk, "cpp_dependencies", fake_dependencies):
            check("keycheck unused ignored inputs do not reject census",
                  nk.dependency_census(tree, {"src/a.c"}) == {"src/a.c"})
        for kind in (nk.stat_mode.S_IFIFO, nk.stat_mode.S_IFCHR,
                     nk.stat_mode.S_IFBLK, nk.stat_mode.S_IFSOCK):
            with patch.object(nk.os, "lstat", return_value=SimpleNamespace(st_mode=kind)), \
                    patch.object(nk, "shape_read", side_effect=AssertionError("must not open")):
                try:
                    nk.read_working_tree()
                    special_failed = False
                except ValueError as e:
                    special_failed = "non-regular working input" in str(e)
            check("keycheck nonregular input rejected before read " + str(kind), special_failed)
        with patch.object(nk, "working_shape", return_value=None), \
                patch.object(nk.os, "lstat",
                             return_value=SimpleNamespace(st_mode=nk.stat_mode.S_IFIFO)), \
                patch.object(nk, "shape_read",
                             side_effect=AssertionError("file loop must not open")):
            try:
                nk.read_working_tree()
                loop_failed = False
            except ValueError as e:
                loop_failed = "non-regular working input" in str(e)
        check("keycheck file-read loop directly rejects FIFO before read", loop_failed)
        if not hasattr(nk.os, "mkfifo"):
            skip("keycheck actual FIFO rejected", "os.mkfifo unavailable on this platform")
        else:
            fifo = directory / "include" / "pipe"
            if create_fifo_fixture(fifo):
                try:
                    with patch.object(nk, "shape_read",
                                      side_effect=AssertionError("must not open FIFO")):
                        try:
                            nk.read_working_tree()
                            fifo_failed = False
                        except ValueError as e:
                            fifo_failed = "non-regular working input" in str(e)
                    check("keycheck actual FIFO rejected before read", fifo_failed)
                finally:
                    fifo.unlink()
    for flag in ("h", "S", "s"):
        with real_shape_fixture() as directory:
            original_git = nk.git
            def flagged_inventory(*args, **kwargs):
                return flag + " src/a.c\0" if "-v" in args else original_git(*args, **kwargs)
            with patch.object(nk, "git", flagged_inventory):
                try:
                    nk.read_working_tree()
                    flag_failed = False
                except ValueError as e:
                    flag_failed = "hidden working index flag" in str(e) and "src/a.c" in str(e)
            check("keycheck working hidden index flag " + flag, flag_failed)
    hidden = nk.ShapeTree({"src/a.c": "int n;\n", "Makefile": make_text})
    hidden._tracked = set(hidden)
    for path in hidden:
        head = dict(hidden)
        head[path] = "different HEAD bytes\n"
        try:
            nk.verify_working_changes(hidden, head, [])
            hidden_failed = False
        except ValueError as e:
            hidden_failed = "outside change set: " + path in str(e)
        check("keycheck raw hidden working change names " + path, hidden_failed)
        nk.verify_working_changes(hidden, head, [("M", path)])
        check("keycheck reported working change allowed " + path, True)

    for comment in ('/* see "gte_macros.inc" */', '/* see <macro.inc> */'):
        check("keycheck assembler mention not cpp input " + comment, not shape_fails(
              {"src/a.c": comment, "include/gte_macros.inc": "## GAS comment\n",
               "include/macro.inc": "# arbitrary GAS comment\n"}, ""))
    # System-only extra dependencies must not trigger a full-tree secondary gate.
    system_only = nk.CppTree({"src/a.c": "int x;"}, "system-only")
    with patch.object(nk, "cpp_dependencies", lambda *_:
                      {"src/a.c", "/usr/mipsel-linux-gnu/include/stdc-predef.h"}), \
            patch.object(nk, "plain_tree", side_effect=AssertionError("no extra local input")):
        check("keycheck external predef skips secondary tree gate",
              nk.dependency_census(system_only, {"src/a.c"}) == {"src/a.c"})
    for spelling in ("typeof", "__typeof", "__typeof__"):
        tokens = nk.layer2.tokens(spelling + "(int) l; " + spelling + "(int) n;")
        declared = nk.cpp_file_declarations(tokens, set(nk.DECL_WORDS), [], {"l", "n"})
        check("keycheck typeof globals recorded " + spelling, {"l", "n"} <= declared)
    parenthesised = "int (g)(void){ typedef int T; return 0; } int f(int l){return l;}"
    tokens = nk.layer2.tokens(parenthesised)
    functions = nk.cpp_functions(tokens, nk.declaration_types({"src/a.c": parenthesised}))
    check("keycheck parenthesised function recognized in cpp",
          {name for name, *_ in functions} == {"g", "f"})
    check("keycheck parenthesised function typedef stays local",
          "T" not in nk.file_typedef_types(tokens, functions))
    check("keycheck parenthesised source function recognized",
          nk.function_names(parenthesised) == {"g", "f"})

    # Follow-up 22: Make filenames are not shell words. Both apostrophe routes
    # must preserve b.c as a distinct dependency, so its includer is selected.
    for label in ("d_apos", "c_apos"):
        deps = nk.parse_cpp_dependencies("__kc__: src/a.c src/q'.h src/b.c src/r'.h\n", "src/a.c")
        with patch.object(nk, "cpp_dependencies", lambda tree, tu: deps if tu == "src/a.c" else {tu}):
            selected = nk.dependency_census(nk.CppTree({"src/a.c": "", "src/b.c": ""}, "base"), {"src/b.c"})
        check("keycheck Make apostrophe census " + label, selected == {"src/a.c", "src/b.c"})
    escaped = nk.parse_cpp_dependencies(r"__kc__: src/a.c include/with\ space\#$$.h", "src/a.c")
    check("keycheck d_ctl_space Make escaping", escaped == {"src/a.c", "include/with space#$.h"})
    for bad in (r"__kc__: src/a.c include/a\q.h", "__kc__: src/a.c $x.h", "__kc__: src/a.c #x.h"):
        try:
            nk.parse_cpp_dependencies(bad, "src/a.c")
            bad_make = False
        except ValueError:
            bad_make = True
        check("keycheck ambiguous Make dependency rejects " + bad, bad_make)
    root = nk.ROOT / "tmp" / "cpp-dependency-fixture"
    fixture_stat = SimpleNamespace(st_dev=3, st_ino=77, st_mode=nk.stat_mode.S_IFREG)
    with patch.object(nk.os, "stat", return_value=fixture_stat), patch.object(nk.os.path, "isfile", return_value=True):
        tree = nk.CppTree({}, "path-fixture")
        tree.directory = root
        good = nk.validate_cpp_dependencies(tree, escaped, "cpp", [])
        check("keycheck d_ctl_space dependencies exist at cpp cwd", good == escaped)
        for label, path in (("d_proc", "/proc/self/cwd/src/b.c"),
                                                        ("d_abs", str(nk.ROOT / "src/b.c")),
                            ("e_esc", "../../../../outside/src/b.c"),
                            ("missing", "src/missing.c")):
            with patch.object(nk, "cpp_system_directories", return_value=()), patch.object(nk.os, "stat", side_effect=FileNotFoundError if label == "missing" else None, return_value=fixture_stat):
                try:
                    nk.validate_cpp_dependencies(tree, {path}, "cpp", [])
                    refused = False
                except ValueError:
                    refused = True
            check("keycheck dependency confinement " + label, refused)
        # Identity follows stat without any per-dependency resolve traversal.
        with patch.object(Path, "resolve", side_effect=AssertionError("resolve must not run")):
            identity = nk.dependency_identity(tree, "src/b.c")
        check("keycheck dependency identity cheap stat", identity == (3, 77))
        stderr = '#include <...> search starts here:\n ' + str(root / "include") + '\nEnd of search list.\n'
        with patch.object(nk.subprocess, "run", return_value=nk.subprocess.CompletedProcess([],0,'',stderr)) as verbose:
            first = nk.cpp_system_directories(tree, "cpp", ["-Iinclude"])
            second = nk.cpp_system_directories(tree, "cpp", ["-Iinclude"])
        check("keycheck cpp -v include discovery cached", first == second == (str(root / "include"),) and verbose.call_count == 1)
    system_tree = nk.CppTree({}, "system-dependency-fixture")
    system_tree.directory = root
    system_root = nk.ROOT.parent / "compiler-owned"
    with patch.object(nk.os, "stat", return_value=fixture_stat), patch.object(nk, "cpp_system_directories", return_value=(str(system_root),)):
        system_path = str(system_root / "stdc-predef.h")
        allowed_system = nk.validate_cpp_dependencies(system_tree, {system_path}, "cpp", [])
    check("keycheck dependency compiler include root allowed", allowed_system == {system_path})
    for result in (nk.subprocess.CompletedProcess([],1,'','bad'), nk.subprocess.CompletedProcess([],0,'','no search list')):
        unknown_system = nk.CppTree({}, "unknown-system")
        unknown_system.directory = root
        with patch.object(nk.subprocess, "run", return_value=result):
            try:
                nk.cpp_system_directories(unknown_system, "cpp", [])
                system_failed = False
            except ValueError:
                system_failed = True
        check("keycheck dependency system roots cannot be guessed", system_failed)
    checked_tree = nk.CppTree({}, "cache-fixture")
    checked_tree.directory = root
    with patch.object(nk.os, "stat", return_value=fixture_stat) as cached_stat:
        nk.validate_cpp_dependencies(checked_tree, {"src/b.c"}, "cpp", [])
        nk.validate_cpp_dependencies(checked_tree, {"src/b.c"}, "cpp", [])
    check("keycheck dependency existence cached across TUs", cached_stat.call_count == 1)
    with patch.object(nk, "cpp_dependencies", side_effect=AssertionError("existence probes must fail before selection")):
        try:
            nk.dependency_census(nk.CppTree({"src/a.c": '#if __has_include("../asm/x.s")\n#include "b.c"\n#endif\n', "src/b.c": "int f(int l){return l;}"}, "base"), {"src/b.c"})
            hasinc_failed = False
        except ValueError as e:
            hasinc_failed = "__has_include" in str(e)
    check("keycheck c_hasinc census preflight", hasinc_failed)
    for mode in ("120000",):
        with patch.object(nk, "git", return_value=mode + " blob " + "1"*40 + "\tsrc/lnk.h\0"):
            try:
                nk.read_base_tree("HEAD")
                linked = False
            except ValueError as e:
                linked = "symlink" in str(e)
        check("keycheck e_linkc git symlink rejects", linked)
    with patch.object(nk, "working_shape", return_value=None), \
            patch.object(nk.os, "lstat", return_value=SimpleNamespace(st_mode=nk.stat_mode.S_IFLNK)):
        try:
            nk.read_working_tree()
            linked_working = False
        except ValueError as e:
            linked_working = "symlink" in str(e)
    check("keycheck e_linkc working symlink rejects", linked_working)
    for line in ('build/src/a.o: src/a.c\n\tcpp src/a.c | cc1', 'export PATH := /x', 'SHELL := /x',
                 '.RECIPEPREFIX := >', 'export CPATH := /x', 'export GCC_EXEC_PREFIX := /x',
                 'export DEPENDENCIES_OUTPUT := /x', 'vpath %.h /x', 'VPATH := /x', 'export PATH', '$(BUILD_DIR)/$(SRC_DIR)/a.o: src/a.c', 'X := $(shell cat config)'):
        for command in (lambda text: nk.cpp_command(text), lambda text: nk.cc1_command(text, "src/a.c")):
            try:
                command(make_text + "\n" + line + "\n")
                rejected = False
            except ValueError as e:
                rejected = "unsupported Makefile" in str(e)
            check("keycheck N4doubleprime strict Make guard " + line, rejected)

    # Provenance is irrelevant for members and indexed once for each local file.
    member_files = {"src/a.c": "typedef int I;struct T{int m;};int f(struct T*t){return t->m;}\n"}
    member_output = nk.emitted_lines(fake_cpp(member_files, "src/a.c"), member_files, "src/a.c")
    with patch.object(nk, "typedef_source_index", side_effect=AssertionError("member provenance must not run")):
        nk.cpp_scope_checks(member_output, {"T.m": "n"}, {}, "base", member_files)
    check("keycheck: member census skips typedef provenance entirely", True)
    indexed = "typedef int I;\ntypedef I J;\ntypedef J K;\nint f(int l){return l;}\n"
    index_files = {"src/a.c": indexed}
    index_output = nk.emitted_lines(fake_cpp(index_files, "src/a.c"), index_files, "src/a.c")
    with nk.analysis_cache(), patch.object(nk, "typedef_rows", wraps=nk.typedef_rows) as rows_seen:
        nk.cpp_scope_checks(index_output, {"f:l": "n"}, {}, "base", index_files)
    source_scans = sum(call.args[0] == indexed for call in rows_seen.call_args_list)
    check("keycheck: local typedef provenance indexes source once", source_scans == 1)
    check("keycheck: provenance line index uses actual source starts",
          sorted(nk.typedef_source_index(indexed)) == [1,2,3])
    for name, prefix in (("k2", '#define SIZE 4\ntypedef struct {int a[SIZE];} T;\n'),
                         ("k3", '#define TYPE int\ntypedef struct {TYPE a;} T;\n'),
                         ("k5", 'typedef int __attribute__((aligned(4))) T;\n')):
        source = prefix + "int f(void){T *l=0;return l==0;}\n"
        def expanded_cpp(tree, tu):
            return fake_cpp(tree, tu).replace("[SIZE]", "[4]").replace("{TYPE a;}", "{int a;}")
        check("keycheck conservative typedef limit " + name, not proof("f:l=n",
              {"src/a.c": source}, {"src/a.c": re.sub(r"\bl\b", "n", source)}, cpp=expanded_cpp)[0])
    for line in (' CPP_DEFS += -DIN_A', '$(eval CPP_DEFS += -DIN_A)',
                 'KCP := CPP\n$(KCP)_DEFS += -DIN_A', '$(eval CC_FLAGS += -g)',
                 'MAKEFLAGS += -e', 'X := $(call mystery)', '$(KCP)_DEFS += -DIN_A'):
        for get_command in (lambda text: nk.cpp_command(text), lambda text: nk.cc1_command(text, "src/a.c")):
            try:
                get_command(make_text + "\n" + line + "\n")
                refused = False
            except ValueError as e:
                refused = "unsupported Makefile" in str(e)
            check("keycheck N4prime literal Make guard " + line, refused)

    for malformed in ('not a dependency record\n', '__kc__:\n'):
        with patch.object(nk.shutil, "which", return_value=str(nk.ROOT / "Makefile")), \
                patch.object(nk.CppTree, "materialize", return_value=nk.ROOT), \
                patch.object(nk.subprocess, "run", return_value=nk.subprocess.CompletedProcess([], 0, malformed, '')):
            try:
                nk.cpp_dependencies(nk.CppTree({"Makefile": make_text}, "base"), "src/a.c")
                bad_records = False
            except ValueError:
                bad_records = True
        check("keycheck: malformed/empty dependency records reject", bad_records)
    locked_tree = nk.CppTree({}, "lock-fixture")
    lock_state = {"active": 0, "peak": 0, "writes": 0}
    def materialization_fixture():
        lock_state["active"] += 1
        lock_state["peak"] = max(lock_state["peak"], lock_state["active"])
        if locked_tree.directory is None:
            time.sleep(0.002)
            lock_state["writes"] += 1
            locked_tree.directory = nk.ROOT / "tmp" / "lock-fixture-unused"
        lock_state["active"] -= 1
        return locked_tree.directory
    with patch.object(locked_tree, "_materialize", materialization_fixture):
        with nk.ThreadPoolExecutor(max_workers=8) as pool:
            list(pool.map(lambda _: locked_tree.materialize(), range(8)))
    check("keycheck: parallel census materializes snapshot safely once", lock_state["writes"] == 1 and lock_state["peak"] == 1)

    with patch.object(nk, "cpp_dependencies", lambda tree, tu: {tu, "include/shared.h", "src/b.c"}), \
            patch.object(nk, "dependency_identity", wraps=nk.dependency_identity) as identities:
        nk.dependency_census(nk.CppTree(graph_files, "base"), {"src/b.c"})
    check("keycheck: dependency filesystem identity cached across TUs",
          sum(call.args[1] == "include/shared.h" for call in identities.call_args_list) == 1)

    # Exercise main's separation of global and scoped pairs, survival scan and
    # complete layer-2 report with a read-only synthetic tree adapter.
    def run_main(spec, before, after, extra=None, cpp=fake_cpp, deps=fake_dependencies,
                 rows=None, storage=lambda *_: None, head_oids=None, base_revision="HEAD",
                 base_oids=None, status_output=""):
        base = before if isinstance(before, dict) else {"src/a.c": before}
        new = after if isinstance(after, dict) else {"src/a.c": after, **(extra or {})}
        if not isinstance(new, nk.ShapeTree):
            new = nk.ShapeTree(new)
        if not hasattr(new, "_tracked"):
            new._tracked = set()
        def git(*args, **kwargs):
            if args[:2] == ("--no-optional-locks", "status"):
                return status_output
            if args[0] == "ls-files" and "--stage" in args:
                return "".join(getattr(new, "_index_modes", {}).get(p, "100644")
                               + " " + oid + " 0\t" + p + "\0"
                               for p, oid in getattr(new, "_index", {}).items())
            if args[0] == "ls-tree":
                if "-z" in args:
                    return "".join("100644 blob " + oid + "\t" + p + "\0"
                                   for p, oid in ((head_oids if args[-1] == "HEAD" else base_oids)
                                                 or {}).items())
                return "src/a.c\n"
            return ""  # no global freshness/survival hits
        out = io.StringIO()
        with patch.object(nk, "git", git), patch.object(nk, "show",
                lambda rev, path: (base if rev else new).get(path)), \
                patch.object(nk, "read_base_tree", lambda *_: base), \
                patch.object(nk, "read_working_tree", lambda: new), \
                patch.object(nk, "preprocess", cpp), \
                patch.object(nk, "cpp_dependencies", deps), \
                patch.object(nk, "compile_cc1", fake_cc1), \
                patch.object(nk, "verify_working_storage", storage), \
                patch.object(nk, "changed_files", lambda *_: rows if rows is not None else
                             [("M", p) for p in new if base.get(p) != new[p]]), \
                patch.object(sys, "argv", ["naming_keycheck", "--pairs", spec,
                                          "--base", base_revision]), \
                contextlib.redirect_stdout(out):
            result = nk.main()
        return result, out.getvalue()
    for label in ("t_f1_ignored_shadow_typeof", "t_f1x_exclude_shadow_typeof",
                  "t_g3_ignored_hidden_includer"):
        before = {"src/a.c": '#include "s.h"\nint f(int l){return l;}\n',
                  "include/s.h": "typedef int T;\n"}
        after = nk.ShapeTree({**before, "src/a.c": before["src/a.c"].replace("int l", "int n").replace("return l", "return n"),
                             "src/s.h": "__typeof__(int) l; __typeof__(int) n;\n"})
        if label == "t_g3_ignored_hidden_includer":
            after["src/s.h"] += '#include "../include/s.h"\n'
        after._nonignored = set(before)
        after._working_nonignored = set(before)
        rc, message = run_main("f:l=n", before, after)
        check("keycheck B1 CLI ignored context " + label,
              rc != 0 and "working-tree cpp dependency is ignored: src/s.h" in message)
    before = {"src/a.c": "int f(int l){return l;}\n", "include/s.h": "int harmless;\n"}
    after = nk.ShapeTree({"src/a.c": "int f(int n){return n;}\n",
                          "include/s.h": "int hidden;\n"})
    after._tracked = set(before)
    rc, message = run_main("f:l=n", before, after, rows=[("M", "src/a.c")])
    check("keycheck CLI hidden clean-filter raw change rejects",
          rc != 0 and "outside change set: include/s.h" in message)
    original_storage = nk.verify_working_storage
    real_run = nk.subprocess.run
    original_is_file = Path.is_file
    def storage_proof(rows, files=None, base="HEAD"):
        def fixture_file(path):
            return (path == nk.ROOT / "src/a.c" or path == nk.ROOT / "src/s.h"
                    or path == nk.ROOT / "asm/probe.s" or path == nk.ROOT / "bb2.ld"
                    or path == nk.ROOT / "named_syms.txt"
                    or path == nk.ROOT / "asm/funcs/gg.s"
                    or original_is_file(path))
        with patch.object(Path, "is_file", fixture_file):
            return original_storage(rows, files, base)
    def attr_response(cmd, kwargs, active=None):
        names = kwargs["input"].decode().rstrip("\0").split("\0")
        data = "".join(p + "\0filter\0" + ((active or {}).get(p, "unspecified")) + "\0"
                       for p in names).encode()
        return nk.subprocess.CompletedProcess(cmd, 0, data, b"")
    for mode, spec in (("scoped", "f:l=n"), ("global", "l=n")):
        before = {"src/a.c": "int f(int l){return l;}\n", "src/s.h": "int K;\n"}
        for label, path, converted in (
                ("cs_ident_hdr", "src/s.h", True),
                ("cs_filter_hdr", "src/s.h", True),
                ("cs_filter_wavefile", "src/a.c", True),
                ("cs_utf7_hdr", "src/s.h", True),
                ("idx_staged_hdr", "src/s.h", False)):
            after = nk.ShapeTree({**before, "src/a.c": "int f(int n){return n;}\n"})
            after._tracked = set(before)
            after._index = {p: "1"*40 for p in before}
            raw = {p: "1"*40 for p in before}
            raw["src/a.c"] = "2"*40
            after._index = dict(raw)
            clean = dict(raw)
            if converted:
                clean[path] = "3"*40
            else:
                after._index[path] = "4"*40
            def hash_response(cmd, **kwargs):
                if cmd[:2] not in (["git", "hash-object"], ["git", "check-attr"]):
                    return real_run(cmd, **kwargs)
                if "check-attr" in cmd:
                    return attr_response(cmd, kwargs)
                names = [json.loads(p) for p in kwargs["input"].decode().splitlines()]
                values = raw if "--no-filters" in cmd else clean
                return nk.subprocess.CompletedProcess(cmd, 0,
                       "".join(values[p] + "\n" for p in names).encode(), b"")
            with patch.object(nk.subprocess, "run", hash_response) as hashes:
                rc, message = run_main(spec, before, after, storage=storage_proof,
                                       rows=[("M", "src/a.c"), ("M", "src/s.h")],
                                       head_oids={p: "1"*40 for p in before})
            reason = "clean conversion" if converted else "index blob differs"
            check("keycheck storage rejects " + mode + " " + label,
                  rc != 0 and reason in message and path in message)
        for label in ("staged", "staged-format", "staged-plus-unstaged", "unstaged"):
            before = {"src/a.c": "int f(int l){return l;} /* l */\n",
                      "src/s.h": "int K;\n"}
            wave = "int f(int n){return n;} /* l */\n"
            working = wave.replace("/* l */", "/* n */")
            after = nk.ShapeTree({**before, "src/a.c": working})
            if label in ("staged-format", "staged-plus-unstaged"):
                after["src/a.c"] = after["src/a.c"].replace("return n", "return  n")
            after._tracked = set(before)
            raw = {"src/a.c": "2"*40, "src/s.h": "1"*40}
            heads = {p: "1"*40 for p in before}
            after._index = dict(heads)
            if label in ("staged", "staged-format"):
                after._index["src/a.c"] = raw["src/a.c"]
            elif label == "staged-plus-unstaged":
                # Exact reset --soft state: HEAD=base, index=wave, working=wave+comments.
                after._index["src/a.c"] = "3"*40
            def identity_hash(cmd, **kwargs):
                if cmd[:2] not in (["git", "hash-object"], ["git", "check-attr"]):
                    return real_run(cmd, **kwargs)
                if "check-attr" in cmd:
                    return attr_response(cmd, kwargs)
                names = [json.loads(p) for p in kwargs["input"].decode().splitlines()]
                return nk.subprocess.CompletedProcess(cmd, 0,
                       "".join(raw[p] + "\n" for p in names).encode(), b"")
            with patch.object(nk.subprocess, "run", side_effect=identity_hash) as hashes:
                rc, message = run_main(spec, before, after, storage=storage_proof,
                                       rows=[("M", "src/a.c")], head_oids=heads)
            if label in ("staged", "staged-format"):
                check("keycheck storage accepts " + mode + " " + label, rc == 0)
            else:
                check("keycheck storage refuses partial staging " + mode + " " + label,
                      rc != 0 and "src/a.c" in message
                      and "stage the wave (git add -A) before the keycheck" in message)
            check("keycheck storage exactly two hash batches " + mode + " " + label,
                  sum("hash-object" in call.args[0] for call in hashes.call_args_list) == 2)
    for label, spec, target, failure in (
            ("a_sneaky_filter_hdr_scoped", "f:l=n", "src/s.h", "filter"),
            ("a_sneaky_filter_hdr_global", "l=n", "src/s.h", "filter"),
            ("a_sneaky_filter_wavefile_scoped", "f:l=n", "src/a.c", "filter"),
            ("a_partial_global_ifdef", "l=n", "src/s.h", "staging"),
            ("a_partial_mixed_scoped", "f:l=n", "src/s.h", "staging"),
            ("a_asm_filter_global", "l=n", "asm/probe.s", "filter"),
            ("a_asm_staged_global", "l=n", "asm/probe.s", "staging")):
        before = {"src/a.c": "int f(int l){return l;}\n", "src/s.h": "#define K 1\n"}
        if target.startswith("asm/"):
            before[target] = ".word l\n"
        after = nk.ShapeTree({**before, "src/a.c": "int f(int n){return n;}\n"})
        after[target] = ".word n\n" if target.startswith("asm/") else after[target]
        after._tracked = set(before)
        raw = {p: "1"*40 for p in before}
        raw["src/a.c"] = "2"*40
        raw[target] = "3"*40
        after._index = dict(raw)
        if failure == "staging":
            after._index[target] = "1"*40
        calls = []
        def storage_response(cmd, **kwargs):
            if cmd[:2] not in (["git", "hash-object"], ["git", "check-attr"]):
                return real_run(cmd, **kwargs)
            calls.append(cmd)
            if "check-attr" in cmd:
                return attr_response(cmd, kwargs, {target: "caller-aware"} if failure == "filter" else {})
            names = [json.loads(p) for p in kwargs["input"].decode().splitlines()]
            # Caller-aware filters return identity under both hash invocations.
            return nk.subprocess.CompletedProcess(cmd, 0,
                   "".join(raw[p] + "\n" for p in names).encode(), b"")
        with patch.object(nk.subprocess, "run", storage_response):
            rc, message = run_main(spec, before, after, storage=storage_proof,
                                   rows=[("M", p) for p in after],
                                   head_oids={p: "1"*40 for p in before})
        reason = "filter attribute" if failure == "filter" else "stage the wave (git add -A)"
        check("keycheck fully-staged invariant " + label,
              rc != 0 and target in message and reason in message)
        if failure == "filter":
            check("keycheck filter refusal precedes any filter invocation " + label,
                  len([cmd for cmd in calls if cmd[:2] in
                       (["git", "hash-object"], ["git", "check-attr"])]) == 1
                  and any(cmd[:2] == ["git", "check-attr"] for cmd in calls))
    for label, spec, target, new_file in (
            ("s_stale_asm_global", "l=n", "asm/probe.s", False),
            ("s_stale_asm_scoped", "f:l=n", "asm/probe.s", False),
            ("s_stale_ld_global", "l=n", "bb2.ld", False),
            ("s_stale_newfile_asm_global", "l=n", "asm/funcs/extra.s", True),
            ("s_stale_registry_scoped", "f:l=n", "named_syms.txt", False)):
        before = {"src/a.c": "int f(int l){return l;}\n"}
        if not new_file:
            before[target] = "unchanged working bytes\n"
        after = nk.ShapeTree({**before, "src/a.c": "int f(int n){return n;}\n"})
        after._tracked = set(after)
        after._index = {"src/a.c": "2"*40, target: "3"*40}
        heads = {p: "1"*40 for p in before}
        raw = {p: "1"*40 for p in before}
        raw["src/a.c"] = "2"*40
        def stale_response(cmd, **kwargs):
            if cmd[:2] == ["git", "check-attr"]:
                return attr_response(cmd, kwargs)
            if cmd[:2] == ["git", "hash-object"]:
                names = [json.loads(p) for p in kwargs["input"].decode().splitlines()]
                return nk.subprocess.CompletedProcess(cmd, 0,
                       "".join(raw[p] + "\n" for p in names).encode(), b"")
            return real_run(cmd, **kwargs)
        with patch.object(nk.subprocess, "run", stale_response):
            rc, message = run_main(spec, before, after, storage=storage_proof,
                                   rows=[("M", "src/a.c")], head_oids=heads)
        check("keycheck index-only stale edit rejected " + label,
              rc != 0 and target in message and "stage the wave (git add -A)" in message)
    # Mode changes and staged removals also belong to the checked set, even when
    # no working-tree row reports them. A filter refusal isolates that census.
    for kind in ("mode", "removed"):
        files = nk.ShapeTree({"src/a.c": "int x;"})
        files._index = {"src/a.c": "1"*40}
        files._index_modes = {"src/a.c": "100644"}
        if kind == "mode":
            files._index["asm/probe.s"] = "1"*40
            files._index_modes["asm/probe.s"] = "100755"
        def index_inventory(*args, **kwargs):
            if args[0] == "ls-tree":
                return "100644 blob " + "1"*40 + "\tasm/probe.s\0"
            return ""
        with patch.object(nk, "git", index_inventory), \
                patch.object(nk.subprocess, "run", side_effect=lambda cmd, **kwargs:
                             attr_response(cmd, kwargs, {"asm/probe.s": "guard-probe"})):
            try:
                original_storage([], files)
                index_census_failed = False
            except ValueError as e:
                index_census_failed = "filter attribute" in str(e) and "asm/probe.s" in str(e)
        check("keycheck index-vs-HEAD census includes " + kind, index_census_failed)
    for label, spec, target, new_file in (
            ("b_reverted_asm_unstaged_global", "l=n", "asm/probe.s", False),
            ("b_reverted_asm_unstaged_scoped", "f:l=n", "asm/probe.s", False),
            ("b_reverted_ld_unstaged_global", "l=n", "bb2.ld", False),
            ("b_reverted_newasm_unstaged_global", "l=n", "asm/funcs/extra.s", True)):
        before = {"src/a.c": "int f(int l){return l;}\n"}
        if not new_file:
            before[target] = "base bytes\n"
        for staged_revert in (False, True):
            after = nk.ShapeTree({**before, "src/a.c": "int f(int n){return n;}\n"})
            after._tracked = set(before)
            bases = {p: "1"*40 for p in before}
            heads = {**bases, target: "3"*40}
            raw = {**bases, "src/a.c": "2"*40}
            after._index = {**heads, "src/a.c": "2"*40}
            if staged_revert:
                if new_file:
                    del after._index[target]
                else:
                    after._index[target] = raw[target]
            def history_response(cmd, **kwargs):
                if cmd[:2] == ["git", "check-attr"]:
                    return attr_response(cmd, kwargs)
                if cmd[:2] == ["git", "hash-object"]:
                    names = [json.loads(p) for p in kwargs["input"].decode().splitlines()]
                    return nk.subprocess.CompletedProcess(cmd, 0,
                           "".join(raw[p] + "\n" for p in names).encode(), b"")
                return real_run(cmd, **kwargs)
            with patch.object(nk.subprocess, "run", history_response):
                rc, message = run_main(spec, before, after, storage=storage_proof,
                                       rows=[("M", "src/a.c")], head_oids=heads,
                                       base_revision="HEAD~1", base_oids=bases)
            check("keycheck non-HEAD reverted path " + label + " staged=" + str(staged_revert),
                  rc == 0 if staged_revert else rc != 0 and target in message
                  and "stage the wave (git add -A)" in message)
    for spec in ("f:l=n", "l=n"):
        before = {"src/a.c": "int f(int l){return l;}\n", "src/s.h": "/* l */\n"}
        after = nk.ShapeTree({"src/a.c": "int f(int n){return n;}\n", "src/s.h": "/* n */\n"})
        after._tracked = set(before)
        after._index = {"src/a.c": "2"*40, "src/s.h": "3"*40}
        def committed_response(cmd, **kwargs):
            if cmd[:2] == ["git", "check-attr"]:
                return attr_response(cmd, kwargs)
            if cmd[:2] == ["git", "hash-object"]:
                names = [json.loads(p) for p in kwargs["input"].decode().splitlines()]
                return nk.subprocess.CompletedProcess(cmd, 0,
                       "".join(after._index[p] + "\n" for p in names).encode(), b"")
            return real_run(cmd, **kwargs)
        with patch.object(nk.subprocess, "run", committed_response):
            rc, message = run_main(spec, before, after, storage=storage_proof,
                                   rows=[("M", p) for p in before], head_oids=dict(after._index),
                                   base_revision="HEAD~2", base_oids={p: "1"*40 for p in before})
        check("keycheck committed two-part wave with older base " + spec, rc == 0)
    for mode_spec in ("f:l=n", "l=n"):
        before = {"src/a.c": "int f(int l){return l;}\n", "asm/probe.s": "unchanged bytes\n"}
        after = nk.ShapeTree({**before, "src/a.c": "int f(int n){return n;}\n"})
        after._tracked = set(before)
        after._index = {"src/a.c": "2"*40, "asm/probe.s": "1"*40}
        after._index_modes = {"src/a.c": "100644", "asm/probe.s": "100755"}
        with patch.object(nk.subprocess, "run", side_effect=lambda cmd, **kwargs:
                         attr_response(cmd, kwargs) if cmd[:2] == ["git", "check-attr"]
                         else real_run(cmd, **kwargs)):
            rc, message = run_main(mode_spec, before, after, storage=storage_proof,
                                   rows=[("M", "src/a.c")], head_oids={p: "1"*40 for p in before})
        check("keycheck staged mode change rejected " + mode_spec,
              rc != 0 and "index mode differs from base mode: asm/probe.s" in message)
    empty_oid = "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391"
    for content, intent in (("", True), ("nonempty\n", True), ("", False)):
        before = {"asm/funcs/g.s": content}
        after = nk.ShapeTree({"asm/funcs/gg.s": content})
        after._tracked = set(after)
        oid = empty_oid if not content else "1"*40
        after._index = {"asm/funcs/gg.s": empty_oid if intent else oid}
        status = ("1 .A N... 000000 000000 100644 " + "0"*40 + " " + "0"*40
                  + " asm/funcs/gg.s\0") if intent else ""
        def ita_response(cmd, **kwargs):
            if cmd[:2] == ["git", "check-attr"]:
                return attr_response(cmd, kwargs)
            if cmd[:2] == ["git", "hash-object"]:
                return nk.subprocess.CompletedProcess(cmd, 0, (oid + "\n").encode(), b"")
            return real_run(cmd, **kwargs)
        with patch.object(nk.subprocess, "run", ita_response):
            rc, message = run_main("g=gg", before, after, storage=storage_proof,
                                   rows=[("D", "asm/funcs/g.s"), ("A", "asm/funcs/gg.s")],
                                   head_oids={"asm/funcs/g.s": oid}, status_output=status)
        check("keycheck empty-file rename intent=" + str(intent) + " content=" + repr(content),
              rc != 0 and "intent-to-add" in message and "asm/funcs/gg.s" in message
              and "stage the wave (git add -A)" in message if intent else rc == 0)
    # A rename source record that resembles status syntax is a filename, not an ITA row.
    with patch.object(nk, "git", side_effect=lambda *args, **kwargs:
                      ("2 R. N... 100644 100644 100644 " + "1"*40 + " " + "1"*40
                       + " R100 ordinary\0" + "1 .A fake filename\0")
                      if args[:2] == ("--no-optional-locks", "status") else ""):
        files = nk.ShapeTree({})
        files._index = {}
        try:
            original_storage([], files)
            rename_record_ok = True
        except ValueError as e:
            rename_record_ok = False
    check("keycheck porcelain rename source is not status", rename_record_ok)
    # Round 30 B1: `git rm --cached P; git add -N P` on a HEAD-tracked empty file is DA.
    # Every unstaged status letter (Y) fails; unmerged rows fail; status takes no lock.
    hexes = " ".join(("1"*40, "0"*40))
    for label, status_row, expect in (
            ("DA intent-to-add", "1 DA N... 100644 000000 100644 " + hexes + " include/e.h",
             "intent-to-add file is not staged: include/e.h"),
            (".A intent-to-add", "1 .A N... 000000 000000 100644 " + hexes + " asm/new.s",
             "intent-to-add file is not staged: asm/new.s"),
            ("unstaged modify", "1 .M N... 100644 100644 100644 " + hexes + " asm/probe.s",
             "unstaged change is not staged: asm/probe.s"),
            ("unstaged delete", "1 .D N... 100644 100644 000000 " + hexes + " bb2.ld",
             "unstaged change is not staged: bb2.ld"),
            ("staged plus unstaged", "1 MM N... 100644 100644 100644 " + hexes + " src/a.c",
             "unstaged change is not staged: src/a.c"),
            ("worktree rename", "2 .R N... 100644 100644 100644 " + hexes + " R100 new.s\0old.s",
             "unstaged change is not staged: new.s"),
            ("worktree copy", "2 .C N... 100644 100644 100644 " + hexes + " C100 cp.s\0old.s",
             "unstaged change is not staged: cp.s"),
            ("unmerged", "u UU N... 100644 100644 100644 100644 " + "1"*40 + " " + hexes
             + " src/a.c", "unmerged working index: src/a.c")):
        calls = []
        def status_git(*args, row=status_row, **kwargs):
            calls.append(args)
            return row + "\0" if args[:2] == ("--no-optional-locks", "status") else ""
        with patch.object(nk, "git", side_effect=status_git):
            files = nk.ShapeTree({})
            files._index = {}
            try:
                original_storage([], files)
                status_message = ""
            except ValueError as e:
                status_message = str(e)
        check("keycheck status row rejects " + label, expect in status_message
              and ("stage the wave (git add -A)" in status_message or label == "unmerged"))
        check("keycheck status is read-only " + label,
              calls[:1] == [("--no-optional-locks", "status", "--porcelain=v2", "-z",
                             "--no-renames", "--untracked-files=no")])
    for label, status_row in (
            ("staged add", "1 A. N... 000000 100644 100644 " + hexes + " asm/new.s"),
            ("staged delete", "1 D. N... 100644 000000 000000 " + hexes + " asm/old.s"),
            ("staged modify", "1 M. N... 100644 100644 100644 " + hexes + " src/a.c")):
        with patch.object(nk, "git", side_effect=lambda *args, row=status_row, **kwargs:
                          row + "\0" if args[:2] == ("--no-optional-locks", "status") else ""):
            files = nk.ShapeTree({})
            files._index = {}
            try:
                original_storage([], files)
                staged_ok = True
            except ValueError:
                staged_ok = False
        check("keycheck fully staged status row passes " + label, staged_ok)
    # The field-count guard: a status row with too few fields, or an XY field that is not two
    # letters, is refused rather than read.
    for label, status_row in (
            ("short ordinary row", "1 M. N... 100644 100644 100644 " + "1"*40 + " src/a.c"),
            ("short rename row", "2 R. N... 100644 100644 100644 " + hexes + " src/a.c"),
            ("three-letter XY", "1 M.. N... 100644 100644 100644 " + hexes + " src/a.c")):
        with patch.object(nk, "git", side_effect=lambda *args, row=status_row, **kwargs:
                          row + "\0" if args[:2] == ("--no-optional-locks", "status") else ""):
            files = nk.ShapeTree({})
            files._index = {}
            try:
                original_storage([], files)
                field_message = ""
            except ValueError as e:
                field_message = str(e)
        check("keycheck status field-count guard refuses " + label,
              field_message == "invalid porcelain-v2 status entry")
    # Hidden index flags are read only for checked paths and cpp inputs: a flag on an
    # unchanged path outside both is not inspected (the docstring's scope).
    with patch.object(nk, "git", side_effect=lambda *args, **kwargs:
                      "h docs/unrelated.md\0" if "-v" in args else ""):
        files = nk.ShapeTree({})
        files._index = {}
        try:
            original_storage([], files)
            unchecked_flag_ok = True
        except ValueError:
            unchecked_flag_ok = False
    check("keycheck hidden flag outside the checked set is not inspected", unchecked_flag_ok)
    for path in ("asm/funcs/caf\u00e9.s", "docs/with\ttab.txt", "docs/with\nnewline.txt"):
        def nul_diff(*args, **kwargs):
            return "M\0" + path + "\0" if args[0] == "diff" else "new/\u00e9\ttab\nname\0"
        with patch.object(nk, "git", side_effect=nul_diff) as diff_git:
            records = nk.changed_files("HEAD", None)
        check("keycheck NUL diff preserves path " + repr(path),
              records == [("M", path), ("A", "new/\u00e9\ttab\nname")])
        check("keycheck NUL diff/untracked commands " + repr(path),
              all("-z" in call.args for call in diff_git.call_args_list))
    # Hidden flags on changed non-cpp inputs must be included too.
    with patch.object(nk, "git", side_effect=lambda *args, **kwargs:
                      "h asm/probe.s\0" if "-v" in args else ""):
        files = nk.ShapeTree({"src/a.c": "int x;"})
        files._index = {"src/a.c": "1"*40}
        try:
            original_storage([("M", "asm/probe.s")], files)
            asm_flag_failed = False
        except ValueError as e:
            asm_flag_failed = "hidden working index flag: asm/probe.s" in str(e)
    check("keycheck changed asm hidden flag refused", asm_flag_failed)
    for staged in (False, True):
        files = nk.ShapeTree({})
        files._index = {} if staged else {"asm/probe.s": "1"*40}
        def deletion_git(*args, **kwargs):
            return ""
        with patch.object(nk, "git", deletion_git), \
                patch.object(nk.subprocess, "run", side_effect=lambda cmd, **kwargs:
                             attr_response(cmd, kwargs)):
            try:
                original_storage([("D", "asm/probe.s")], files)
                deleted_ok = True
            except ValueError as e:
                deleted_ok = False
                deletion_hint = "asm/probe.s" in str(e) and "stage the wave (git add -A)" in str(e)
        check("keycheck deletion staging " + str(staged),
              deleted_ok if staged else not deleted_ok and deletion_hint)
    files = nk.ShapeTree({"src/a.c": "int x;"})
    files._index = {}
    with patch.object(nk, "git", return_value=""), \
            patch.object(nk.subprocess, "run", side_effect=lambda cmd, **kwargs:
                         attr_response(cmd, kwargs)):
        try:
            original_storage([("A", "src/a.c")], files)
            unstaged_added = False
        except ValueError as e:
            unstaged_added = "src/a.c" in str(e) and "stage the wave (git add -A)" in str(e)
    check("keycheck added working file must be staged", unstaged_added)
    try:
        nk.verify_working_changes({}, {}, [])
        missing_tracked = False
    except AttributeError:
        missing_tracked = True
    check("keycheck missing tracked metadata cannot silently bypass guard", missing_tracked)
    # run_main injects its own diff function; count the query inside main directly.
    diff_count = []
    original_main = nk.main
    def counted_main():
        original_diff = nk.changed_files
        def count_diff(*args):
            diff_count.append(args)
            return original_diff(*args)
        with patch.object(nk, "changed_files", count_diff):
            return original_main()
    with patch.object(nk, "main", counted_main):
        run_main("f:l=n", local, local_new)
    check("keycheck scoped main reuses first diff", len(diff_count) == 1)
    # The policy must propagate to the base-side census too.
    before = {"src/a.c": '#include "s.h"\nint f(int l){return l;}\n',
              "src/s.h": "int hidden;\n"}
    after = nk.ShapeTree({"src/a.c": '#include "s.h"\nint f(int n){return n;}\n',
                          "src/s.h": "int hidden;\n"})
    after._nonignored = {"src/a.c"}
    after._working_nonignored = {"src/a.c"}
    rc, message = run_main("f:l=n", before, after, deps=lambda tree, tu:
                           {tu, "src/s.h"} if tree.label == "base" else {tu})
    check("keycheck B1 CLI base-only ignored dependency rejects",
          rc != 0 and "working-tree cpp dependency is ignored: src/s.h" in message)
    with patch.object(nk, "plain_tree", side_effect=ValueError("scoped tree shape: fixture")):
        result, output = run_main("f:l=fresh", local, local_new)
    check("keycheck main cannot bypass plain-tree gate", result == 1 and "tree shape" in output)
    result, output = run_main("f:l=fresh", local, local_new, extra={"include/new.h": '#include HEADER\n'})
    check("keycheck main gates new-tree includes", result == 1 and "non-literal include" in output)
    with patch.object(nk, "verify_local_cc1", side_effect=ValueError("local cc1 differential asm differs")):
        rc, message = run_main("f:l=fresh", local, local_new)
    check("keycheck cc1: main cannot bypass mandatory proof", rc != 0 and "cc1 differential" in message)
    result, output = run_main("f:l=fresh", local, local_new)
    check("keycheck: scoped main has no globals and reports moved key",
          result == 0 and "pairs: 0 " in output and "moved layer-2 keys: 1" in output)
    result, output = run_main("f:l=fresh", local, local_new,
                              cpp=lambda tree, tu: '# 1 ' + json.dumps(tu) + '\n\n')
    check("keycheck: main must invoke cpp verification on negative case",
          result == 1 and "never compiles" in output)
    source = "int f(int l){\nreturn l;\nSYSTEM_NAME;\n}\n"
    result, output = run_main("f:l=fresh", source, source.replace("int l", "int fresh").replace("return l", "return fresh"),
                              cpp=lambda tree, tu: fake_cpp(tree, tu).replace("SYSTEM_NAME", "fresh"))
    check("keycheck: main must pass scoped pairs for function-wide proof",
          result == 1 and "macro expansion produces or captures" in output)
    cli_before = "".join(f"int timed_{i}(int arg0){{return arg0;}}\n" for i in range(30))
    cli_after = "".join(f"int timed_{i}(int value_{i}){{return value_{i};}}\n" for i in range(30))
    cli_pairs = ",".join(f"timed_{i}:arg0=value_{i}" for i in range(30))
    runs.clear()
    start = time.perf_counter()
    result, output = run_main(cli_pairs, cli_before, cli_after, cpp=counted_cpp)
    elapsed = time.perf_counter() - start
    check("keycheck: 30-pair fake-cpp main reuses TU outputs", result == 0
          and runs == [(label, "src/a.c") for label in ("base", "new", "base-sentinel", "new-sentinel")])
    print(f"  fake-cpp main timing: 30 functions, 30 pairs: {elapsed:.3f}s")
    result, output = run_main("f:l=fresh", local.replace("int f", "int\nf"),
                             local_new.replace("int f", "int\nf"))
    # A split head (return type on its own line) is a definition candidate
    # now, so layer-2 keys it like the one-line form above.
    check("keycheck: split-head function is keyed like a one-line head",
          result == 0 and "pairs: 0 " in output
          and "moved layer-2 keys: 1" in output)
    result, output = run_main("old=new", "int old(void) { return 1; }",
                              "int new(void) { return 1; }")
    check("keycheck: global substitution path unchanged", result == 0 and "pairs: 1 " in output)
    result, output = run_main("f:l=fresh,old=new",
                              'int f(int l) { return old(l + 1); }',
                              'int f(int fresh) { return new(fresh + 1); }')
    check("keycheck: mixed scoped/global substitutions", result == 0)
    result, output = run_main("f:l=fresh,old=new",
                              'int f(int l) { return l + old; }',
                              'int f(int fresh) { return fresh + new; }')
    check("keycheck: mixed scoped/global simple substitutions", result == 0 and "pairs: 1 " in output)
    for declaration in ("struct Unk80101EC8Record {int unk_4;};",
                        "typedef struct {int unk_4;} Unk80101EC8Record;"):
        for scope in ("Unk80101EC8Record", "GoodRec"):
            result, output = run_main("Unk80101EC8Record=GoodRec," + scope + ".unk_4=hp",
                                     declaration, declaration.replace("Unk80101EC8Record", "GoodRec").replace("unk_4", "hp"))
            check("keycheck: main passes global type resolution to cpp " + scope + " " + declaration, result == 0)
    for scope in ("func_80010000", "good_fn"):
        result, output = run_main("func_80010000=good_fn," + scope + ":arg0=count",
                                 "int func_80010000(int arg0){return arg0;}", "int good_fn(int count){return count;}")
        check("keycheck: CLI globally renamed local function " + scope, result == 0)
    result, output = run_main("T.m=n,U.k=n", shared_base, shared_new)
    check("keycheck: CLI rejects shared scoped destination (" + output[-300:] + ")",
          result == 1 and "not fresh in cpp aggregates" in output)
    result, output = run_main("T.m=n,k=x", shared_base,
                              shared_base.replace("int m; }; union", "int n; }; union")
                              .replace("int k;", "int x;"))
    check("keycheck: explicit global pair can rename another member declaration",
          result == 0)
    macro = "#define GETBITS(x) if (x) { x--; }\n"
    text = macro + "int f(int l) { if (l) { return l; } return 0; }"
    result, output = run_main("f:l=n", text, text.replace("int l", "int n")
                             .replace("if (l)", "if (n)").replace("return l", "return n"))
    check("keycheck: macro if is not a phantom function", result == 0
          and nk.function_names(text) == {"f"} and nk.function_tokens(text, "if") == [])
    knr = "int legacy(x)\nint x;\n{ return x; }\n"
    text = "struct T { int m; };\n" + knr.replace("return x", "return p->m + x")
    after = text.replace("int m", "int n").replace("->m", "->n")
    result, output = run_main("T.m=n", text, after)
    check("keycheck: K&R moved key remains reported", result == 0 and "legacy" in output
          and "moved layer-2 keys: 1" in output)
    with patch.object(nk.layer2, "body_key", return_value=None):
        result, output = run_main("T.m=n", text, after)
    check("keycheck: unkeyable changed K&R fails", result == 1 and "layer-2 cannot key" in output)
    result, output = run_main("f:l=n", local, local)
    check("keycheck: no changes cannot certify scoped pairs", result == 1 and "not applied" in output)
    result, output = run_main("f:l=n", "int f(int l){return l;}", "int f(int n){return n;")
    check("keycheck: unbalanced changed file fails cleanly", result == 1 and "unbalanced" in output)
    result, output = run_main("f:l=b,a=b", "int f(int l){return l+a;}", "int f(int b){return b+b;}")
    check("keycheck: CLI mixed capture fails", result == 1 and "global pair" in output)
    result, output = run_main("Rec1C.b18=field,Unk1F800000Unk00=X",
                              "typedef int Unk1F800000Unk00; struct Rec1C { Unk1F800000Unk00 b18; };",
                              "typedef int X; struct Rec1C { X field; };")
    check("keycheck: mixed aggregate CLI succeeds", result == 0)
    for fn, old_name, dest in [("gpu_BuildFogRowLevels", "buf", "dst"),
                                ("func_800167EC", "rec", "obj"),
                                ("func_80049C24", "temp_v0", "next"),
                                ("gpu_UploadAndDrawSelectedEntryQuads", "i", "j"),
                                ("func_80040CB8", "kind", "mode")]:
        text = (f"#define M({dest}) ({dest})\nextern int proto(int {dest});\n"
                f"int other({dest})\nint {dest};\n{{return {dest};}}\n"
                f"int {fn}(int {old_name}){{return call({old_name});}}\n")
        after = re.sub(r"\b" + old_name + r"\b", dest, text)
        result, output = run_main(fn + ":" + old_name + "=" + dest, text, after)
        check("keycheck: usable local pair end-to-end " + fn, result == 0)
    # Binary batch framing (including embedded newlines/UTF-8) and one process.
    from types import SimpleNamespace
    oid1, oid2 = "1" * 40, "2" * 40
    data1, data2 = b"int x;\n", "/* café */\n".encode("utf-8")
    response = (f"{oid1} blob {len(data1)}\n".encode() + data1 + b"\n"
                + f"{oid2} blob {len(data2)}\n".encode() + data2 + b"\n")
    cr_bytes = b"int x;\r\n"
    cr_response = f"{oid1} blob {len(cr_bytes)}\n".encode() + cr_bytes + b"\n"
    with patch.object(nk, "git", return_value=f"100644 blob {oid1}\tsrc/a.c\0"), \
            patch.object(nk.subprocess, "run", return_value=SimpleNamespace(stdout=cr_response)):
        try:
            nk.read_base_tree("HEAD")
            raw_cr_failed = False
        except ValueError as e:
            raw_cr_failed = "control byte" in str(e) and "offset 6" in str(e)
    check("keycheck S commit reader preserves raw CR", raw_cr_failed)
    listing = f"100644 blob {oid1}\tsrc/a.c\0" + f"100644 blob {oid2}\tinclude/b.h\0"
    listing += f"100644 blob {oid2}\tbb2.ld\0"
    response += f"{oid2} blob {len(data2)}\n".encode() + data2 + b"\n"
    with patch.object(nk, "git", return_value=listing), patch.object(nk.subprocess, "run",
            return_value=SimpleNamespace(stdout=response)) as batch:
        eq("keycheck: batch reads base bytes like show", nk.read_base_tree("HEAD"),
           {"src/a.c": "int x;\n", "include/b.h": "/* café */\n", "bb2.ld": "/* café */\n"})
        check("keycheck: base read uses one cat-file process", batch.call_count == 1
              and batch.call_args.args[0] == ["git", "cat-file", "--batch"]
              and batch.call_args.kwargs["input"] == (oid1 + "\n" + oid2 + "\n" + oid2 + "\n").encode())
    for path in ["named_syms.txt", "asm/a.s", "notes.txt", "memory/grind/f/layer2.jsonl"]:
        result, output = run_main("f:l=fresh", local, local_new, {path: "fresh"})
        check("keycheck: scoped pairs cannot explain " + path, result == 1)


def test_queue_repo_root() -> None:
    """Queue storage follows the call-time root, including lock and CAS."""
    from unittest.mock import patch
    from engine import gtemacro, queue as Q

    cwd = Path.cwd()
    with tempfile.TemporaryDirectory() as td:
        root = Path(td).resolve()
        (root / "engine").mkdir()
        other = root / "elsewhere"
        (other / "engine").mkdir(parents=True)
        decoy = other / "engine/queue.json"
        decoy.write_text('{"items": [{"func": "decoy"}]}\n')
        before = decoy.read_bytes()
        q = {"items": [{"func": "rooted", "addr": "80010000"}]}
        qp = root / "engine/queue.json"
        qp.write_text(json.dumps(q, indent=2) + "\n")
        with patch.object(gtemacro, "REPO_ROOT", root), \
                patch.object(Q, "QUEUE_PATH", "engine/queue.json"):
            try:
                os.chdir(other)
                eq("queue root: foreign cwd reads rooted queue", Q.load(), q)
                token = Q._fingerprint()
                original_replace = os.replace
                replaced = []

                def replace(src, dst):
                    replaced.append((src, dst))
                    return original_replace(src, dst)

                with Q._locked(), patch.object(Q.os, "replace", replace):
                    check("queue root: sidecar lock sits beside rooted queue",
                          qp.with_name("queue.json.lock").is_file())
                    Q.save(q, expect=token)
                eq("queue root: saved bytes retain the root format", qp.read_text(),
                   json.dumps(q, indent=2) + "\n")
                check("queue root: atomic replace stays in queue directory",
                      len(replaced) == 1 and replaced[0][0].parent == qp.parent
                      and replaced[0][1] == qp)
                qp.write_text('{"items": []}\n')
                try:
                    Q.save(q, expect=token)
                    check("queue root: fingerprint rejects intervening write", False)
                except Q.QueueConflict as e:
                    check("queue root: conflict keeps relative queue name",
                          str(e).startswith("engine/queue.json changed on disk"))
                eq("queue root: foreign queue untouched", decoy.read_bytes(), before)
                check("queue root: no lock or temporary files in cwd",
                      sorted(p.name for p in decoy.parent.iterdir()) == ["queue.json"])
                with patch.object(gtemacro, "REPO_ROOT", other):
                    eq("queue root: root changes take effect at call time",
                       Q.load()["items"][0]["func"], "decoy")
            finally:
                os.chdir(cwd)


def test_queue_regen_diff_repo_root() -> None:
    """Scratch regen seeds and writes the call-time root, even from another cwd."""
    from unittest.mock import patch
    from engine import gtemacro
    from tools.spotcheck import queue_regen_diff as regen

    cwd = Path.cwd()
    with tempfile.TemporaryDirectory() as td:
        root = Path(td).resolve()
        other = root / "elsewhere"
        (root / "engine").mkdir()
        (other / "engine").mkdir(parents=True)
        queue = root / "engine/queue.json"
        seed = '{"items": [{"func": "rooted", "parked": true}]}\n'
        queue.write_text(seed, encoding="utf-8")
        decoy = other / "engine/queue.json"
        decoy.write_text('{"items": []}\n', encoding="utf-8")
        scratch = root / regen.SCRATCH

        def generate(**kwargs):
            eq("regen root: scratch preserves real queue input",
               Q._queue_path().read_text(encoding="utf-8"), seed)
            eq("regen root: scratch resolves from repo root", Q._queue_path(), scratch)
            eq("regen root: generation options", kwargs,
               {"workdir": "unused", "preserve": True})
            scratch.with_name(scratch.name + ".lock").write_text("", encoding="utf-8")
            return {"items": []}

        try:
            os.chdir(other)
            with patch.object(gtemacro, "REPO_ROOT", root):
                for raises in (False, True):
                    def attempt(**kwargs):
                        result = generate(**kwargs)
                        if raises:
                            raise RuntimeError("generation failed")
                        return result

                    for path in ("engine/queue.json", str(queue)):
                        with patch.object(Q, "QUEUE_PATH", path), \
                                patch.object(Q, "generate", side_effect=attempt):
                            try:
                                eq("regen root: mocked generation result",
                                   regen.regen_to_scratch("unused"), {"items": []})
                            except RuntimeError as exc:
                                check("regen root: generation error propagated",
                                      raises and str(exc) == "generation failed")
                            else:
                                check("regen root: expected outcome", not raises)
                            eq("regen root: queue path restored", Q.QUEUE_PATH, path)
                        check("regen root: scratch lock removed",
                              not scratch.with_name(scratch.name + ".lock").exists())
            eq("regen root: real queue unchanged", queue.read_text(encoding="utf-8"), seed)
            eq("regen root: cwd queue unchanged",
               decoy.read_text(encoding="utf-8"), '{"items": []}\n')
            check("regen root: no scratch tree in cwd", not (other / "tmp").exists())
        finally:
            os.chdir(cwd)


def test_departures() -> None:
    """Q39 departures audit (engine/departures.py), keyed by function ADDRESS,
    on throwaway git repos: every departure shape the name-keyed rounds had to
    special-case — naming waves (caller+callee, committed, uncommitted,
    reverted, re-reverted, renamed back and forth, double rename after
    completion), asm bodies with a later callee rename, TU splits, reopen and
    re-land, hand-drops, revocation, a FAIL on the current body — plus every
    fail-closed path, the three-process budget, the integrity wiring and a
    synthetic hand-drop on a clone of the real history. Runs under WSL (git
    writes loose objects read-only; the tests that delete one chmod first)."""
    import re
    import subprocess
    from datetime import datetime, timedelta
    from engine import departures, gtemacro, layer2
    from unittest.mock import patch

    original_root = gtemacro.REPO_ROOT

    def enter_repo(path):
        os.chdir(path)
        gtemacro.REPO_ROOT = Path.cwd()

    def git(*a, date=None):
        env = dict(os.environ)
        if date:
            env["GIT_COMMITTER_DATE"] = env["GIT_AUTHOR_DATE"] = date
        return subprocess.run(["git", "-c", "user.email=t@e", "-c", "user.name=t",
                               "-c", "core.hooksPath=/dev/null", "-c", "init.defaultBranch=main",
                               *a], check=True, capture_output=True, text=True, env=env).stdout

    addrs = {}
    stamp = [0]

    def glabel_file(name, addr, calls=()):
        base = int(addr, 16)
        lines = [f"glabel {name}"] + [
            f"    /* {base - 0x80010000 + 4 * i:X} {base + 4 * i:08X} {(base * 7 + i) & 0xFFFFFFFF:08X} */"
            f"  addiu      $v{i % 2}, $a0, {i}" for i in range(8)]
        lines += [f"    /* {base - 0x80010000 + 32 + 4 * i:X} {base + 32 + 4 * i:08X} 0C00{i:04X} */"
                  f"  jal        {callee}" for i, callee in enumerate(calls)]
        return "\n".join(lines) + "\n"

    def fn(name, src="src/k.c", include_asm=False, calls=()):
        """A function: a C body (or an INCLUDE_ASM line) and its glabel file."""
        addrs.setdefault(name, f"8001{len(addrs) * 0x100:04X}")
        Path(src).parent.mkdir(parents=True, exist_ok=True)
        body = " ".join(f"{c}(a);" for c in calls)
        with open(src, "a") as fh:
            fh.write(f'INCLUDE_ASM("asm/funcs", {name});\n' if include_asm else
                     f"int {name}(int a) {{ {body} return a + {len(addrs)}; }}\n")
        Path("asm/funcs").mkdir(parents=True, exist_ok=True)
        Path(f"asm/funcs/{name}.s").write_text(glabel_file(name, addrs[name], calls))

    def wave(code_map):
        """tools/naming_wave.py's effect: every source and .s rewritten with
        the whole map (word-bounded); each renamed glabel file moves."""
        pat = re.compile(r"\b(" + "|".join(map(re.escape, code_map)) + r")\b")
        for p in [*Path(".").glob("src/**/*.[ch]"), *Path(".").glob("include/**/*.h"),
                  *Path(".").glob("asm/funcs/*.s")]:
            t = p.read_text()
            if pat.search(t):
                p.write_text(pat.sub(lambda m: code_map[m.group(1)], t))
        for old, new in code_map.items():
            if Path(f"asm/funcs/{old}.s").exists():
                Path(f"asm/funcs/{old}.s").rename(f"asm/funcs/{new}.s")
            addrs[new] = addrs[old]
        q = json.loads(Path("engine/queue.json").read_text())
        for it in q["items"]:
            it["func"] = code_map.get(it["func"], it["func"])
        Path("engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")

    def edit(name, src="src/k.c"):
        lines = Path(src).read_text().splitlines(True)
        Path(src).write_text("".join(
            l.replace("return ", "return 1 + ", 1) if l.startswith(f"int {name}(") else l
            for l in lines))

    def item(f, addr=True):
        it = {"func": f, "file": "k", "distance": 3, "verdict": "C", "rules": 0,
              "status": "active"}
        if addr:
            it["addr"] = addrs[f]
        return it

    def write_queue(names, addr=True):
        Path("engine/queue.json").write_text(json.dumps(
            {"items": [item(f, addr) for f in names]}, indent=2) + "\n")

    def queued():
        return [it["func"] for it in json.loads(Path("engine/queue.json").read_text())["items"]]

    def drop(*names):
        write_queue([f for f in queued() if f not in names])

    def key(f, src="src/k.c"):
        return layer2.body_key(Path(src).read_text(), f)[1]

    def rec(func, h, verdict="PASS"):
        stamp[0] += 1
        p = layer2.record_path(func)
        p.parent.mkdir(parents=True, exist_ok=True)
        with open(p, "a") as fh:
            fh.write(json.dumps({"func": func, "addr": addrs[func], "verdict": verdict,
                                 "body_hash": h,
                                 "date": f"2026-10-01T00:{stamp[0] // 60:02d}:{stamp[0] % 60:02d}Z"})
                     + "\n")

    def commit(msg, date=None):
        git("add", "-A")
        git("commit", "-qm", msg, date=date)
        return git("rev-parse", "HEAD").strip()

    def audit():
        v = departures.unreviewed_departures()
        return v, sorted(s.split(" (addr")[0] for s in v if not s.startswith("Q39"))

    def new_repo(path, funcs):
        """A repo with a pre-address queue commit, then the anchor (every item
        addressed)."""
        path.mkdir()
        enter_repo(path)
        addrs.clear()
        git("init", "-q")
        Path("engine").mkdir()
        for f in funcs:
            fn(f)
        write_queue(funcs, addr=False)
        commit("pre-address queue")
        write_queue(funcs)
        commit("anchor: every queue item addressed")

    repo_root = Path(departures.__file__).resolve().parent.parent
    eq("departures: Windows gitdir -> /mnt path",
       departures.wsl_gitdir("gitdir: C:/Users/T/My Repo/.git/worktrees/w\n"),
       "/mnt/c/Users/T/My Repo/.git/worktrees/w")
    eq("departures: /mnt gitdir -> Windows path",
       departures.win_gitdir("gitdir: /mnt/c/Repo/.git/worktrees/w"), "C:/Repo/.git/worktrees/w")

    cwd = os.getcwd()
    real_git = departures._git_cmd()          # this tree's git (a worktree, maybe)
    with tempfile.TemporaryDirectory() as td, patch.object(gtemacro, "REPO_ROOT", gtemacro.REPO_ROOT):
        try:
            enter_repo(td)
            Path(".git").write_text("gitdir: C:/Repo/.git/worktrees/w\n")
            eq("departures: WSL git reads a Windows gitdir",
               departures._git_cmd("posix"),
               ["git", "--no-replace-objects", "--git-dir=/mnt/c/Repo/.git/worktrees/w"])
            Path(".git").write_text("gitdir: /mnt/c/Repo/.git/worktrees/w\n")
            eq("departures: Windows git reads a /mnt gitdir",
               departures._git_cmd("nt"),
               ["git", "--no-replace-objects", "--git-dir=C:/Repo/.git/worktrees/w"])
            Path(".git").unlink()

            new_repo(Path(td) / "rooted", ["func_ROOT", "func_KEEP"])
            Path("src/k.c").write_text(
                'INCLUDE_ASM("asm/funcs", func_ROOT);\n'
                'int func_KEEP(void) { return 0; }\n')
            drop("func_ROOT")
            h = key("func_ROOT")
            rec("func_ROOT", h)
            root = Path.cwd()
            other = Path(td) / "elsewhere"
            other.mkdir()
            try:
                os.chdir(other)  # Keep REPO_ROOT on the synthetic repository.
                eq("departures root: foreign cwd reads git, source, asm and ledger",
                   audit(), ([], []))
                eq("departures root: current name returns a relative TU",
                   departures._current_name(addrs["func_ROOT"], layer2.addr_index(),
                                            None, None), ("func_ROOT", "k"))
                ledger = root / "memory/grind/func_ROOT/layer2.jsonl"
                ledger.write_text("{broken\n")
                v = departures.unreviewed_departures()
                check("departures root: record errors stay repo-relative",
                      len(v) == 1 and "memory/grind/func_ROOT/layer2.jsonl:1 is not JSON" in v[0]
                      and str(root) not in v[0])
                ledger.unlink()
                eq("departures root: foreign cwd sees an unreviewed hand-drop",
                   audit()[1], ["func_ROOT"])
                # The addr-less current item must resolve through this index;
                # at_d must reject ROOT's PASS when the index moves its name.
                (root / "engine/queue.json").write_text(
                    json.dumps({"items": [item("func_KEEP", False)]}), encoding="utf-8")
                rec("func_ROOT", h)
                shared_index = {"shared_name": addrs["func_ROOT"],
                                "func_ROOT": addrs["func_KEEP"],
                                "func_KEEP": addrs["func_KEEP"]}
                with patch.object(layer2, "addr_index", return_value=shared_index), \
                        patch.object(layer2, "lookup", wraps=layer2.lookup) as lookup, \
                        patch.object(departures, "_current_name",
                                     wraps=departures._current_name) as current_name:
                    eq("departures root: at_d uses shared index over a recorded PASS",
                       audit()[1], ["func_ROOT"])
                    eq("departures root: audit uses layer2's address index",
                       current_name.call_args.args[1], shared_index)
                    for name in ("func_KEEP", "func_ROOT"):
                        check("departures root: shared lookup for " + name,
                              any(call.args[0] is shared_index and call.args[1] == name
                                  for call in lookup.call_args_list))
                (root / "engine/queue.json").write_text(
                    json.dumps({"items": [item("func_KEEP")]}), encoding="utf-8")
                ledger.unlink()
                graft = root / ".git/info/grafts"
                graft.parent.mkdir(exist_ok=True)
                graft.write_text("\n")
                check("departures root: relative git graft path uses repo root",
                      "grafts file" in departures.unreviewed_departures()[0])
                graft.unlink()
                # A worktree gitdir file is read from the root even outside it.
                fake = other / "worktree"
                fake.mkdir()
                (fake / ".git").write_text("gitdir: C:/Repo/.git/worktrees/w\n")
                with patch.object(gtemacro, "REPO_ROOT", fake):
                    eq("departures root: translated gitdir command",
                       departures._git_cmd("posix"),
                       ["git", "--no-replace-objects", "--git-dir=/mnt/c/Repo/.git/worktrees/w"])
            finally:
                enter_repo(root)

            # Q126: thread the departure's committer date into the legacy-key
            # fallback; an old record alone cannot clear a new departure.
            cutoff = datetime.fromisoformat(departures.Q126_CUTOFF.replace("Z", "+00:00"))
            before = (cutoff - timedelta(seconds=1)).isoformat()
            after = (cutoff + timedelta(seconds=1)).isoformat()
            for case, date, full_key, rec_date, expected in (
                    ("pre-cutoff", before, False, None, []),
                    ("at-cutoff", cutoff.isoformat(), False, None, ["func_Q126"]),
                    ("worktree", None, False, None, ["func_Q126"]),
                    ("post-cutoff-record", before, False, after, ["func_Q126"]),
                    ("current-key", after, True, None, [])):
                new_repo(Path(td) / ("q126-" + case), ["func_Q126"])
                text = ("static inline int helper(int a) { return a + 1; }\n"
                        "int func_Q126(int a) { return helper(a); }\n")
                Path("src/k.c").write_text(text)
                helper_key = layer2.body_key(text, "func_Q126")
                own_key = layer2.body_key(text, "func_Q126", helpers=False)
                check("departures (q126): helper changes key " + case,
                      helper_key is not None and own_key is not None
                      and helper_key != own_key)
                rec("func_Q126", helper_key[1] if full_key else own_key[1])
                record = json.loads(layer2.record_path("func_Q126").read_text())
                if rec_date is not None:
                    record["date"] = rec_date
                    layer2.record_path("func_Q126").write_text(json.dumps(record) + "\n")
                record_date = datetime.fromisoformat(record["date"].replace("Z", "+00:00"))
                check("departures (q126): record date as intended " + case,
                      record_date.tzinfo is not None
                      and (record_date < cutoff) == (rec_date is None))
                commit("queued helper body and pre-cutoff PASS")
                drop("func_Q126")
                if date is not None:
                    commit("departure " + case, date=date)
                v, fl = audit()
                eq("departures (q126): " + case, fl, expected)
                if not expected:
                    eq("departures (q126): the audit itself is clean " + case, v, [])
            addrs.clear()                  # repo a builds its repo without new_repo

            # ── repo a: record semantics, TU split, asm body, reopen ─────────
            fns = ["func_X", "func_Y", "func_Z", "func_V", "func_W", "func_K", "func_R",
                   "func_T", "func_CAL", "func_CL", "func_SK", "func_SX", "func_FA2",
                   "CdTest", "func_Q"]
            Path(td, "a").mkdir()
            enter_repo(Path(td, "a"))
            git("init", "-q")
            Path("engine").mkdir()
            for f in fns:
                fn(f)
            fn("func_A1", include_asm=True, calls=["func_CAL"])  # asm body, jal CAL
            write_queue(fns + ["func_A1"], addr=False)
            commit("pre-address queue")
            eq("departures: no addressed queue -> nothing audited", audit()[1], [])
            write_queue(fns + ["func_A1"])
            commit("anchor")
            rec("func_Y", key("func_Y"))
            rec("func_Z", "f" * 16)                       # a PASS on another body
            rec("func_V", key("func_V"))
            rec("func_V", key("func_V"), verdict="FAIL")  # revoked
            rec("func_W", key("func_W"))
            rec("func_R", key("func_R"))
            rec("func_A1", key("func_A1"))                # asm body: INCLUDE_ASM + .s
            rec("func_CL", key("func_CL"))                # CL lands; cleaned up later
            # FA2: an ARCHIVED future-dated PASS; a later live FAIL via record()
            # must still come after it
            pth = Path("memory/grind/_completed/func_FA2/layer2.jsonl")
            pth.parent.mkdir(parents=True, exist_ok=True)
            pth.write_text(json.dumps({"func": "func_FA2", "addr": addrs["func_FA2"],
                                       "verdict": "PASS", "body_hash": key("func_FA2"),
                                       "date": "2099-01-01T00:00:00Z"}) + "\n")
            # SK: a PASS, then a FAIL on the same body whose clock went BACK —
            # line order decides within a file
            rec("func_SK", key("func_SK"))
            with open(layer2.record_path("func_SK"), "a") as fh:
                fh.write(json.dumps({"func": "func_SK", "addr": addrs["func_SK"],
                                     "verdict": "FAIL", "body_hash": key("func_SK"),
                                     "date": "2026-01-01T00:00:00Z"}) + "\n")
            # SX: a FAIL in the live ledger, a LATER-dated PASS in the archived
            # one (which sorts first by path) — files merge by date
            for where, verdict, date in (("func_SX", "FAIL", "2026-10-01T09:00:00Z"),
                                         ("_completed/func_SX", "PASS", "2026-10-02T09:00:00Z")):
                pth = Path("memory/grind", where, "layer2.jsonl")
                pth.parent.mkdir(parents=True, exist_ok=True)
                pth.write_text(json.dumps({"func": "func_SX", "addr": addrs["func_SX"],
                                           "verdict": verdict, "body_hash": key("func_SX"),
                                           "date": date}) + "\n")
            # CdTest: the same, for a ledger that sorts BEFORE _completed/
            # ('C' < '_'): a tie must not be broken by path order
            pth = Path("memory/grind/_completed/CdTest/layer2.jsonl")
            pth.parent.mkdir(parents=True, exist_ok=True)
            pth.write_text(json.dumps({"func": "CdTest", "addr": addrs["CdTest"],
                                       "verdict": "PASS", "body_hash": key("CdTest"),
                                       "date": "2099-01-01T00:00:00Z"}) + "\n")
            t = Path("src/k.c").read_text()               # TU split: T moves to k2.c,
            t_line = next(l for l in t.splitlines(True) if l.startswith("int func_T("))
            Path("src/k.c").write_text(t.replace(t_line, ""))
            Path("src/k2.c").write_text(t_line)           # its item still says k
            rec("func_T", key("func_T", "src/k2.c"))
            drop("func_X", "func_Y", "func_Z", "func_V", "func_W", "func_R", "func_T",
                 "func_A1", "func_CL", "func_SK", "func_SX", "func_FA2", "CdTest")
            d1 = commit("X hand-dropped; Y Z V W R T A1 leave")
            edit("func_Y")                                # a later callee rename
            edit("func_W")
            rec("func_W", key("func_W"), verdict="FAIL")  # FAIL on a LATER body
            edit("func_CL")                               # a reviewed cheat-cleanup
            r = layer2.record("CdTest", "FAIL", "rev", "match", stem="k",
                              expect_hash=key("CdTest"))
            check("departures: record() dates a live FAIL strictly after a future PASS "
                  "(no tie)", r.get("ok") is True and r.get("date") > "2099-01-01T00:00:00Z")
            r = layer2.record("func_FA2", "FAIL", "rev", "match", stem="k",
                              expect_hash=key("func_FA2"))
            check("departures: record() dates a live FAIL after an archived future PASS",
                  r.get("ok") is True and r.get("date") >= "2099-01-01T00:00:00Z")
            rec("func_CL", key("func_CL"))
            write_queue(queued() + ["func_R"])
            commit("R reopened")
            edit("func_R")
            drop("func_R")
            commit("R hand-dropped again, body changed, no new PASS")
            wave({"func_CAL": "func_Callee"})             # rewrites A1's .s jal operand
            commit("callee rename")
            drop("func_K")                                # hand edit, not committed
            v, fl = audit()
            eq("departures: record semantics", fl,
               ["CdTest", "func_FA2", "func_K", "func_R", "func_SK", "func_V", "func_X",
                "func_Z"])
            check("departures: a reviewed cleanup after landing keeps the landing clear",
                  "func_CL" not in fl)
            check("departures: a FAIL on a later body does not revoke the landed body's PASS",
                  "func_W" not in fl)
            check("departures: within a file, line order beats a clock that went back",
                  "func_SK" in fl)
            check("departures: across files, the later effective date wins",
                  "func_SX" not in fl)
            check("departures: a PASS on the body that left clears it despite a later edit",
                  "func_Y" not in fl)
            check("departures: a TU split finds the body at D in any source file",
                  "func_T" not in fl)
            check("departures: an asm body is read with its .s AT D (callee renamed "
                  "since)", "func_A1" not in fl)
            check("departures: every finding names the fix commands",
                  all("queue reopen" in s and "layer2 record" in s for s in v))
            rec("func_K", key("func_K"))
            rec("func_R", key("func_R"))
            _v, fl = audit()
            check("departures: a working-tree hand-drop clears on a PASS on the current body",
                  "func_K" not in fl)
            check("departures: reopen then re-land clears on a PASS on the re-landed body",
                  "func_R" not in fl)

            # the budget: three git processes however long the history
            real_popen, calls = subprocess.Popen, []

            class CountingPopen(real_popen):
                def __init__(self, args, *a, **kw):
                    if args and args[0] == "git":
                        calls.append(args)
                    super().__init__(args, *a, **kw)
            subprocess.Popen = CountingPopen
            try:
                departures.unreviewed_departures()
            finally:
                subprocess.Popen = real_popen
            eq("departures: three git processes", len(calls), 3)
            check("departures: every git call ignores replace refs",
                  all("--no-replace-objects" in c for c in calls))
            check("departures: the history walk is --topo-order --full-history",
                  any("--topo-order" in c and "--full-history" in c for c in calls))

            # unparseable versions: a conflict gives its addresses by pattern
            q = json.loads(Path("engine/queue.json").read_text())
            fn("func_M1")
            Path("engine/queue.json").write_text(
                '{"items": [\n<<<<<<< HEAD\n' + json.dumps(q["items"][0]) + ',\n=======\n'
                + json.dumps(item("func_M1")) + ',\n>>>>>>> side\n'
                + ",\n".join(json.dumps(i) for i in q["items"][1:]) + "\n]}\n")
            commit("conflict committed")
            write_queue([i["func"] for i in q["items"]])
            commit("resolved")
            Path("engine/queue.json").write_text("{ not json")
            commit("garbage")
            write_queue([i["func"] for i in q["items"]])
            commit("repaired")
            v, fl = audit()
            check("departures: a conflicted version's addresses are audited",
                  any(s.startswith(f"? (addr {addrs['func_M1']})") for s in v))
            check("departures: a version giving no address is a violation",
                  any(s.startswith("Q39") and "gives no address" in s for s in v))

            os.environ["GIT_DIR"] = str(Path(td) / "no-such-gitdir")
            try:
                v = departures.unreviewed_departures()
            finally:
                del os.environ["GIT_DIR"]
            check("departures: a git failure fails the audit closed",
                  len(v) == 1 and "could not read git history" in v[0])

            enter_repo(td)
            git("clone", "-q", "--depth", "1", f"file://{Path(td) / 'a'}", "shallow")
            enter_repo(Path(td) / "shallow")
            v = departures.unreviewed_departures()
            check("departures: a shallow clone fails closed",
                  len(v) == 1 and "cannot establish the anchor (shallow clone?)" in v[0])

            # ── repo w: naming waves never make a departure ────────────────
            new_repo(Path(td) / "w", ["func_80058580", "fighter_BuildCpuPad", "func_P1",
                                      "func_RVA", "func_DN", "func_RQ", "func_WL",
                                      "func_FX", "func_UL", "func_BX", "func_SX1",
                                      "func_SW1", "func_SX2", "func_SW2", "func_HX",
                                      "func_HW", "func_Q"])
            wave({"fighter_BuildCpuPad": "func_Caller", "func_80058580": "func_Callee"})
            commit("caller + callee renamed in one wave")
            wave({"func_RQ": "func_RQ2"})                 # renamed while queued,
            commit("RQ -> RQ2")
            edit("func_RQ2")                              # worked on,
            rec("func_RQ2", key("func_RQ2"))              # completed properly
            drop("func_RQ2")
            commit("RQ2 done with a PASS")
            wave({"func_RVA": "func_RVB"})
            w1 = commit("wave RVA -> RVB")
            git("revert", "--no-edit", w1)                # revert
            git("revert", "--no-edit", "HEAD")            # revert of the revert
            wave({"func_RVB": "func_RVA"})                # and back again
            commit("rename back")
            rec("func_DN", key("func_DN"))
            drop("func_DN")
            commit("DN done with a PASS")
            wave({"func_DN": "func_DN1"})
            commit("DN -> DN1")
            wave({"func_DN1": "func_DN2"})
            edit("func_DN2")
            commit("DN1 -> DN2, then a callee change")
            wave({"func_WL": "func_WL2"})                 # renamed AND landed in one commit
            rec("func_WL2", key("func_WL2"))
            drop("func_WL2")
            commit("WL -> WL2 and WL2 done, one commit")
            # BX: renamed and dropped in one commit with NO genuine PASS; a record
            # carries BX's address but names ANOTHER function (Q) with Q's body
            wave({"func_BX": "func_BX2"})
            layer2.record_path("func_Q").parent.mkdir(parents=True, exist_ok=True)
            with open(layer2.record_path("func_Q"), "a") as fh:
                fh.write(json.dumps({"func": "func_Q", "addr": addrs["func_BX"],
                                     "verdict": "PASS", "body_hash": key("func_Q"),
                                     "date": "2026-10-01T01:00:00Z"}) + "\n")
            drop("func_BX2")
            commit("BX -> BX2 and dropped, one commit, no genuine PASS")
            # S1: a CHAIN rename in one wave (SX1 -> SY1, SW1 -> SX1), SY1 landed
            # in the same commit: name@D (SX1) is another function at D
            wave({"func_SX1": "func_SY1", "func_SW1": "func_SX1"})
            rec("func_SY1", key("func_SY1"))
            drop("func_SY1")
            commit("chain wave + SY1 done, one commit")
            # R2: HX hand-dropped, then a chain wave HX -> HY, HW -> HX
            drop("func_HX")
            commit("HX hand-dropped")
            wave({"func_HX": "func_HY", "func_HW": "func_HX"})
            commit("chain wave HX -> HY, HW -> HX")
            drop("func_FX")                               # hand-dropped, no PASS,
            commit("FX hand-dropped")
            wave({"func_FX": "func_FX2"})                 # then renamed
            commit("FX -> FX2")
            wave({"func_P1": "func_P1N"})                 # uncommitted wave
            wave({"func_UL": "func_UL2"})                 # uncommitted wave + landing
            rec("func_UL2", key("func_UL2"))
            drop("func_UL2")
            wave({"func_SX2": "func_SY2", "func_SW2": "func_SX2"})   # S2: uncommitted
            rec("func_SY2", key("func_SY2"))                         # chain wave +
            drop("func_SY2")                                         # landing
            v, fl = audit()
            eq("departures: waves (caller+callee, revert, revert-of-revert, back and "
               "forth, double rename after completion, rename+land in one commit and "
               "uncommitted, uncommitted) -> clear; only the hand-drop is flagged",
               fl, ["func_BX", "func_FX", "func_HX"])
            check("departures (S1/S2): a chain rename + landing in one commit, committed "
                  "or not, is clear", "func_SX1" not in fl and "func_SX2" not in fl)
            check("departures (R2): after a chain wave the fix names the function that "
                  "left (func_HY), not the one now called func_HX",
                  any(s.startswith("func_HX ") and "queue reopen func_HY " in s for s in v))
            check("departures: a stand-in name must have the departed address at D",
                  "func_BX" in fl)
            check("departures: the fix names the function as it is NOW",
                  any(s.startswith("func_FX ") and "queue reopen func_FX2 " in s for s in v))

            # ── repo mp: a merge's parent-2 version is addressed from parent
            #    2's own files (Z's glabel file exists only there) ────────────
            new_repo(Path(td) / "mp", ["func_A"])
            git("checkout", "-qb", "side")
            fn("func_Z")
            q = json.loads(Path("engine/queue.json").read_text())
            q["items"].append(item("func_Z", addr=False))  # a hand-added, addr-less item
            Path("engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")
            commit("side: Z queued without addr")
            git("checkout", "-q", "main")
            Path("other").write_text("x\n")
            commit("main moves on")
            git("merge", "-q", "--no-commit", "side")
            rec("func_Z", key("func_Z"))
            Path("asm/funcs/func_Z.s").unlink()           # only parent 2 has Z's glabel file
            write_queue(["func_A"])                       # Z leaves IN the merge, reviewed
            commit("merge: Z done with a PASS")
            eq("departures (merge): the parent-2 version is addressed from parent 2's files",
               audit()[1], [])

            # ── fail closed: grafts, replace refs, missing objects, records ─
            new_repo(Path(td) / "k", ["func_A", "func_X"])
            drop("func_X")
            d = commit("X leaves unreviewed")
            anchor = git("rev-parse", "HEAD~1").strip()
            eq("departures: baseline sees X", audit()[1], ["func_X"])
            git("replace", "--graft", d, git("rev-parse", "HEAD~2").strip())
            eq("departures: replace refs are ignored", audit()[1], ["func_X"])
            git("replace", "-d", d)
            Path(".git/info").mkdir(exist_ok=True)
            Path(".git/info/grafts").write_text(f"{d} {anchor}\n")
            v = departures.unreviewed_departures()
            check("departures: a grafts file fails closed",
                  len(v) == 1 and "grafts file" in v[0])
            Path(".git/info/grafts").unlink()
            layer2.record_path("func_X").parent.mkdir(parents=True, exist_ok=True)
            layer2.record_path("func_X").write_text("{broken\n")
            v = departures.unreviewed_departures()
            check("departures: an unreadable record fails closed",
                  len(v) == 1 and "is not JSON" in v[0])
            layer2.record_path("func_X").unlink()
            layer2.record_path("func_X").mkdir()          # a record "file" that cannot be read
            v = departures.unreviewed_departures()
            check("departures: an unreadable record file fails closed",
                  len(v) == 1 and "is unreadable" in v[0])
            layer2.record_path("func_X").rmdir()
            # an item without addr: addressed from the tracked files if they can,
            # a violation only if nothing does
            q = json.loads(Path("engine/queue.json").read_text())
            del q["items"][0]["addr"]                     # func_A: has a glabel file
            q["items"].append({"func": "func_NOSRC", "file": "k"})
            Path("engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")
            v = departures.unreviewed_departures()
            check("departures: an addr-less item its tracked files address is audited normally",
                  not any("func_A " in s and "no valid addr" in s for s in v))
            check("departures: an addr-less item nothing addresses is a violation",
                  any("func_NOSRC" in s and "no valid addr" in s for s in v))
            write_queue(["func_A"])                       # back to HEAD's queue
            q = json.loads(Path("engine/queue.json").read_text())
            del q["items"][0]["addr"]
            Path("engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")
            commit("a HISTORICAL version without addr on func_A")
            write_queue(["func_A"])
            commit("addr restored")
            v = departures.unreviewed_departures()
            check("departures: a historical addr-less item is addressed from its commit's "
                  "own files", not any("no valid addr" in s for s in v))

            for what, spec in (("queue version", "HEAD~1:engine/queue.json"),
                               ("source blob", "HEAD:src/k.c"),
                               ("source tree", "HEAD:src")):
                new_repo(Path(td) / f"m-{what.replace(' ', '-')}", ["func_A", "func_X"])
                drop("func_X")
                commit("X leaves")
                lost = git("rev-parse", spec).strip()
                obj = Path(".git/objects", lost[:2], lost[2:])
                os.chmod(obj, 0o644)
                obj.unlink()
                v = departures.unreviewed_departures()
                check(f"departures: a missing {what} fails closed",
                      any("missing from the object store" in s for s in v))

            # ── the real history, walked on a clone ──────────────────────────
            real = subprocess.run([*real_git, "rev-parse", "--git-common-dir", "HEAD"],
                                  cwd=str(repo_root), capture_output=True, text=True)
            if real.returncode != 0:
                skip("departures: real-history smoke on a clone",
                     f"git cannot read this tree here: {real.stderr.strip()[:120]}")
            else:
                common, head = real.stdout.splitlines()[:2]
                if not os.path.isabs(common):
                    common = str((repo_root / common).resolve())
                # the real audit on this tree — the clone of its HEAD must agree
                # unless the working tree has uncommitted edits the audit reads
                enter_repo(repo_root)
                expected = audit()
                dirty = subprocess.run([*real_git, "status", "--porcelain", "--", "engine/queue.json",
                                        "memory/grind", "src", "include", "asm/funcs",
                                        "docs/naming", ":(glob)*.txt"],
                                       cwd=str(repo_root), capture_output=True, text=True).stdout
                enter_repo(td)
                git("clone", "-q", "--shared", "--no-checkout", common, "real")
                enter_repo(Path(td) / "real")
                # every path the audit reads: the queue, the source, the asm
                # bodies and glabel files, every ledger (memory/grind/**/
                # layer2.jsonl, _completed/ too), the census; cone mode keeps
                # the top-level symbol files
                git("sparse-checkout", "set", "engine", "src", "include", "asm/funcs",
                    "memory/grind", "docs/naming")
                git("checkout", "-q", "--detach", head)
                check("departures (real history): the clone holds every ledger the tree has",
                      sorted(p.as_posix() for p in Path("memory/grind").glob("**/layer2.jsonl"))
                      == sorted(l for l in git("ls-files", "memory/grind").splitlines()
                                if l.endswith("/layer2.jsonl")))
                q = json.loads(Path("engine/queue.json").read_text())
                if not all("addr" in it for it in q["items"]):
                    idx = layer2.addr_index()     # anchor the clone as the backfill does
                    for it in q["items"]:
                        it["addr"] = idx[it["func"]]
                    Path("engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")
                    git("commit", "-qam", "synthetic anchor")
                got = audit()
                if dirty.strip():
                    skip("departures (real history): the clone audits as the tree does",
                         "the tree has uncommitted edits the audit reads: "
                         + " ".join(dirty.split())[:120])
                else:
                    eq("departures (real history): the clone audits as the tree does",
                       got[0], expected[0])
                # an item with a body and no record at all: dropping it by hand
                # can't be cleared by an existing PASS
                gone = next((it for it in q["items"]
                             if layer2.current_key(it["func"], it.get("file", ""))
                             and not layer2.addr_dates(it["addr"])), None)
                if gone is None and not q["items"]:
                    # a drained queue has nothing real to drop; the hand-drop
                    # shape itself is proven on the throwaway repos above
                    skip("departures (real history): a synthetic hand-drop is caught",
                         "the queue is empty: no real item to drop")
                elif gone is None:
                    check("departures (real history): a queued item with no record to drop",
                          False)
                else:
                    q["items"] = [it for it in q["items"] if it is not gone]
                    Path("engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")
                    git("commit", "-qam", "synthetic unreviewed departure")
                    eq("departures (real history): a synthetic hand-drop is caught",
                       audit()[1], sorted(got[1] + [gone["func"]]))
        finally:
            os.chdir(cwd)
            gtemacro.REPO_ROOT = original_root

    # naming_wave no longer writes renamed_from on queue items (FAIL-4), and
    # its residual audit is clean after a wave over a queued function
    import naming_wave as nw
    data = {"items": [{"func": "func_A", "addr": "80010000"}]}
    nw.retarget_queue_items(data, {"func_A": "func_A2"})
    eq("departures: naming_wave renames a queue item's func only",
       data["items"], [{"func": "func_A2", "addr": "80010000"}])
    with tempfile.TemporaryDirectory() as td, patch.object(gtemacro, "REPO_ROOT", gtemacro.REPO_ROOT):
        saved = nw.ROOT
        nw.ROOT = Path(td)
        try:
            Path(td, "engine").mkdir()
            q = {"items": [{"func": "func_OLDNAME", "addr": "80010000"}]}
            op = nw.Op("0x80010000", "RENAME", "func_NEWNAME", {})
            op.olds = {"func_OLDNAME"}
            wv = nw.Wave([op])
            nw.retarget_queue_items(q, wv.code_map)
            Path(td, "engine/queue.json").write_text(json.dumps(q, indent=2) + "\n")
            eq("naming_wave: residual_audit is clean after a wave over a queued function",
               dict(nw.residual_audit(wv)), {})
        finally:
            nw.ROOT = saved

    # the integrity wiring: check_completion_integrity.main() extends its
    # violations with the audit before it decides the exit code
    import ast
    src = (repo_root / "tools/check_completion_integrity.py").read_text(encoding="utf-8")
    main_fn = next(n for n in ast.parse(src).body
                   if isinstance(n, ast.FunctionDef) and n.name == "main")
    wired = verdict_at = None
    for i, st in enumerate(main_fn.body):
        seg = ast.get_source_segment(src, st) or ""
        if seg.strip() == "violations.extend(departures.unreviewed_departures())":
            wired = i
        if isinstance(st, ast.If) and (ast.get_source_segment(src, st.test) or "") == "violations":
            verdict_at = i if verdict_at is None else verdict_at
    check("departures: check_completion_integrity.main() runs the audit before its verdict",
          wired is not None and verdict_at is not None and wired < verdict_at)


def test_layer2_repo_paths() -> None:
    """Layer-2 disk access follows the patched repo, independent of cwd."""
    from engine import gtemacro, layer2
    from unittest.mock import patch
    cwd = Path.cwd()
    real_source = cwd / "src/main/368E4.c"
    if real_source.exists():
        text = real_source.read_text(encoding="utf-8", errors="replace")
        names = inlineasm.definition_names(text)
        for name in names:
            eq("layer2 root: current key unchanged",
               layer2.current_key(name, "main/368E4"), layer2.body_key(text, name))
    (cwd / "tmp").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=cwd / "tmp") as td:
        root = Path(td) / "repo"
        elsewhere = Path(td) / "elsewhere"
        elsewhere.mkdir()
        def write(rel, text):
            p = root / rel
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(text, encoding="utf-8")
        source = 'INCLUDE_ASM("asm/funcs", fixture);\n'
        assembly = "glabel fixture\n/* 0 80012340 00000000 */ nop\n"
        write("src/main/fixture.c", source)
        write("asm/funcs/fixture.s", assembly)
        write("src/main/scanned.c", "int scanned(void) { return 1; }\n")
        write("symbol_addrs.txt", "registered = 0x80012344;\n")
        write("build/bb2.map", "  0x80012348 mapped\n")
        write("engine/queue.json", json.dumps({"items": [
            {"func": "fixture", "file": "main/fixture"}]}))
        rec = {"func": "fixture", "addr": "80012340", "verdict": "PASS",
               "body_hash": "0123456789abcdef", "date": "2026-10-01T00:00:00Z"}
        write("memory/grind/fixture/layer2.jsonl", json.dumps(rec) + "\n")
        write("memory/grind/_completed/old/layer2.jsonl", json.dumps(rec) + "\n")
        write("tmp/verdict.json", json.dumps({"function": "fixture", "decision": "FAIL",
              "body_hash": rec["body_hash"], "summary": "fixture verdict"}))
        with patch.object(gtemacro, "REPO_ROOT", root):
            try:
                os.chdir(elsewhere)
                eq("layer2 away: current key reads TU and included assembly",
                   layer2.current_key("fixture", "main/fixture"),
                   layer2.body_key(source, "fixture", lambda p: assembly))
                eq("layer2 away: queue locates TU", layer2.locate_stem("fixture"), "main/fixture")
                eq("layer2 away: fallback scan", layer2.locate_stem("scanned"), "main/scanned")
                eq("layer2 away: active ledger", layer2.read_records("fixture"), [rec])
                eq("layer2 away: archived ledger",
                   layer2.read_records("fixture", layer2.completed_record_path("old")), [rec])
                eq("layer2 away: recursive ledger dates", layer2.addr_dates("80012340"),
                   [rec["date"], rec["date"]])
                eq("layer2 away: address sources", layer2.addr_index(),
                   {"fixture": "80012340", "registered": "80012344", "mapped": "80012348"})
                eq("layer2 away: record display remains relative",
                   layer2._display_path(layer2.record_path("fixture")),
                   "memory/grind/fixture/layer2.jsonl")
                with patch.object(layer2, "_head", return_value="fixture"):
                    result = layer2.record_from_verdict_file(
                        "fixture", "tmp/verdict.json", "reviewer", "match")
                eq("layer2 away: recorded path remains relative", result.get("path"),
                   "memory/grind/fixture/layer2.jsonl")
                eq("layer2 away: verdict source remains relative", result.get("verdict_source"),
                   "tmp/verdict.json")
                check("layer2 away: copied verdict path remains relative",
                      result.get("verdict_file", "").startswith(
                          "memory/grind/fixture/layer2_verdicts/"))
                check("layer2 away: copied verdict exists under root",
                      (root / result["verdict_file"]).is_file())
            finally:
                os.chdir(cwd)


def test_layer2_addresses() -> None:
    """Round-6 review FAIL-2..4: every function's address from TRACKED files
    (a fresh clone has no build/), the link map's assignment lines, refusals
    for an item or record without one, the gate refusing a pre-address record,
    and record() never stamping a date earlier than its file's last."""
    from engine import gtemacro, layer2
    from unittest.mock import patch
    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td, \
            patch.object(gtemacro, "REPO_ROOT", Path(td).resolve()):
        os.chdir(td)
        try:
            Path("engine").mkdir()
            Path("src").mkdir()
            Path("engine/queue.json").write_text(json.dumps({"items": []}) + "\n")
            # a static library function: no glabel file, only the TRACKED
            # linker symbol file names it (the case the map regex missed)
            Path("undefined_syms_auto.txt").write_text("sys_Panic = 0x80016C3C;\n")
            eq("addresses: a tracked symbol file addresses a static function",
               layer2.addr_of("sys_Panic"), "80016C3C")
            eq("addresses: ...through the tracked-files-only lookup the audit uses on history",
               layer2.addr_at("sys_Panic", layer2._disk), "80016C3C")
            saved = Q.QUEUE_PATH
            Q.QUEUE_PATH = "engine/queue.json"
            try:
                r = Q.reopen("sys_Panic", "system", reason="test")
                check("addresses: reopen of sys_Panic carries its address",
                      r.get("ok") is True and r["item"].get("addr") == "80016C3C")
                r = Q.reopen("func_nowhere", "system", reason="test")
                check("addresses: reopen REFUSES a function with no address",
                      r.get("ok") is False and "no address for func_nowhere" in r.get("reason", ""))
                eq("addresses: the refused item was not written",
                   [it["func"] for it in Q.load()["items"]], ["sys_Panic"])
            finally:
                Q.QUEUE_PATH = saved
            Path("docs/naming").mkdir(parents=True)
            Path("docs/naming/function-names.csv").write_text(
                "address,current_name,glabel,aliases\n0x80012340,cen_Named,func_80012340,"
                "cen_alias\n")
            eq("addresses: the census addresses a function (current name and alias)",
               (layer2.addr_of("cen_Named"), layer2.addr_of("cen_alias")),
               ("80012340", "80012340"))
            eq("addresses: a splat auto-name is its own address",
               layer2.addr_of("func_8001ABCD"), "8001ABCD")
            Path("build").mkdir()
            Path("build/bb2.map").write_text(
                "                0x800171b8                        rng_Next = 0x800171b8\n"
                "                0x8001c624                func_mapdef\n")
            idx = layer2.addr_index()
            eq("addresses: the link map's assignment AND definition lines",
               (idx.get("rng_Next"), idx.get("func_mapdef")), ("800171B8", "8001C624"))

            # records: an address is required; the gate refuses a record
            # written before records carried one; dates never go backwards
            Path("src/r.c").write_text("int func_80013000(int a) { return a; }\n")
            h = layer2.current_key("func_80013000", "r")[1]
            p = layer2.record_path("func_80013000")
            p.parent.mkdir(parents=True)
            p.write_text(json.dumps({"func": "func_80013000", "verdict": "PASS",
                                     "body_hash": h, "date": "2026-09-30T07:20:22Z"}) + "\n")
            why = layer2.gate("func_80013000", "r") or ""
            check("addresses: gate refuses a latest record without addr",
                  "carries no `addr`" in why)
            p.write_text(json.dumps({"func": "func_80013000", "addr": "80013000",
                                     "verdict": "FAIL", "body_hash": h,
                                     "date": "2099-01-01T00:00:00Z"}) + "\n")
            r = layer2.record("func_80013000", "PASS", "rev", "match", stem="r", expect_hash=h)
            check("addresses: record() never stamps a date earlier than its file's last",
                  r.get("ok") is True and r.get("date") >= "2099-01-01T00:00:00Z"
                  and r.get("addr") == "80013000")
            eq("addresses: ...and the gate then opens on it",
               layer2.gate("func_80013000", "r"), None)
            with open(p, "a") as fh:
                fh.write(json.dumps({"func": "func_80013000", "addr": "80019999",
                                     "verdict": "PASS", "body_hash": h,
                                     "date": "2099-01-02T00:00:00Z"}) + "\n")
            check("addresses: gate refuses a record whose addr is not the function's",
                  "is at 80013000 now" in (layer2.gate("func_80013000", "r") or ""))
            # one precedence for addr_of and addr_index: the glabel file wins
            # over a symbol file that disagrees
            Path("asm/funcs").mkdir(parents=True, exist_ok=True)
            Path("asm/funcs/func_PR.s").write_text(
                "glabel func_PR\n    /* 5000 80015000 27BDFFE8 */  addiu $sp, $sp, -0x18\n")
            with open("undefined_syms_auto.txt", "a") as fh:
                fh.write("func_PR = 0x80099999;\n")
            eq("addresses: addr_of and addr_index agree on precedence",
               (layer2.addr_of("func_PR"), layer2.addr_index().get("func_PR")),
               ("80015000", "80015000"))
            with open("build/bb2.map", "a") as fh:            # a map that disagrees with
                fh.write("                0x8001c630                func_8001C624\n")
            eq("addresses: ...an auto-name's own address beats the link map in both",
               (layer2.addr_of("func_8001C624"), layer2.addr_index().get("func_8001C624")),
               ("8001C624", "8001C624"))

            # round-9 hardening: the dates record() steps past are the audit's
            # own (running max per ledger), and a date it can't step past is
            # a refusal, never an equal stamp or a traceback
            Path("src/r2.c").write_text("int func_80014000(int a) { return a + 1; }\n")
            h2 = layer2.current_key("func_80014000", "r2")[1]
            arch = Path("memory/grind/_completed/func_80014000/layer2.jsonl")
            arch.parent.mkdir(parents=True)

            def arch_lines(*recs):
                arch.write_text("".join(json.dumps({"func": "func_80014000", "addr": a,
                                                    "verdict": "PASS", "body_hash": h2,
                                                    "date": d}) + "\n" for a, d in recs))

            def rec2():
                try:
                    return layer2.record("func_80014000", "FAIL", "rev", "match", stem="r2",
                                         expect_hash=h2)
                except Exception as e:                   # a refusal must not raise
                    return {"ok": None, "reason": f"raised {type(e).__name__}: {e}"}

            # another address's line dated 2099, then this one's with a clock
            # that went back: the audit orders the second at 2099-06-01
            arch_lines(("80019999", "2099-06-01T00:00:00Z"), ("80014000", "2026-01-01T00:00:00Z"))
            eq("dates: addr_dates returns the running-max effective dates the audit uses",
               layer2.addr_dates("80014000"), ["2099-06-01T00:00:00Z"])
            r = rec2()
            check("dates: ...so record() stamps strictly after that effective date",
                  r.get("ok") is True and r.get("date") > "2099-06-01T00:00:00Z")
            layer2.record_path("func_80014000").unlink()
            arch_lines(("80014000", "2099-01-01 00:00:00"))
            r = rec2()
            check("dates: record() REFUSES when a record carrying the address has a "
                  "non-canonical date", r.get("ok") is False
                  and "not a canonical" in r.get("reason", ""))
            check("dates: ...and writes nothing", not layer2.record_path("func_80014000").exists())
            arch_lines(("80014000", "9999-12-31T23:59:59Z"))
            r = rec2()
            check("dates: record() REFUSES (no traceback) when no later date exists",
                  r.get("ok") is False and "no later date" in r.get("reason", ""))
            check("dates: ...and writes nothing", not layer2.record_path("func_80014000").exists())

            # regen refuses to write an item nothing addresses
            Path("build/src").mkdir(parents=True)
            Path("build/src/faketu.o").write_text("")
            orig = (P.c_stems, canonical.scan_all, cheats.canonical_asm_funcs, Q._rule_count,
                    cheats.func_prologue_count, inlineasm.file_func_cheat_asm_count,
                    completion.source_issues, cheats.is_jtbl_infra,
                    cheats.is_canonical_extraction_only, Q.sandbox.build_stripped_object,
                    score._o_func_table, score.score_func, Q.QUEUE_PATH)
            try:
                Q.QUEUE_PATH = "engine/queue.json"
                P.c_stems = lambda: ["faketu"]
                canonical.scan_all = lambda: []
                cheats.canonical_asm_funcs = lambda: set()
                Q._rule_count = lambda f: 0
                cheats.func_prologue_count = lambda f: 0
                inlineasm.file_func_cheat_asm_count = lambda s, f: 1
                completion.source_issues = lambda *a, **k: []
                cheats.is_jtbl_infra = lambda f: False
                cheats.is_canonical_extraction_only = lambda f: False
                Q.sandbox.build_stripped_object = lambda *a, **k: {}
                score._o_func_table = lambda o: {"func_nowhere2": (0, 0)}
                score.score_func = lambda a, b, f: {"score": 7}
                refused = None
                try:
                    Q.generate(workdir=str(Path(td) / "wd"))
                except ValueError as e:
                    refused = str(e)
                check("addresses: regen REFUSES to write an item with no address",
                      refused is not None and "no address for func_nowhere2" in refused)
            finally:
                (P.c_stems, canonical.scan_all, cheats.canonical_asm_funcs, Q._rule_count,
                 cheats.func_prologue_count, inlineasm.file_func_cheat_asm_count,
                 completion.source_issues, cheats.is_jtbl_infra,
                 cheats.is_canonical_extraction_only, Q.sandbox.build_stripped_object,
                 score._o_func_table, score.score_func, Q.QUEUE_PATH) = orig
        finally:
            os.chdir(cwd)


def test_note_kind_function_reset() -> None:
    """data_wave.note_kind: a retired alias whose surviving name is the address's auto name is a
    RESET for a function (func_) as for data (D_), so naming_keycheck accepts the RESET note
    naming_wave writes when a wave resets a function's registry alias (wave57: 0x80069898
    gpu_draw_layered_box_80069898 -> func_80069898)."""
    import sys
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
    import data_wave as dw
    import naming_keycheck as nk

    eq("note_kind: function auto name is a RESET", dw.note_kind("0x80069898", "func_80069898"),
       "RESET")
    eq("note_kind: data auto name is a RESET", dw.note_kind("0x800A3928", "D_800A3928"), "RESET")
    eq("note_kind: a semantic survivor is a RENAME", dw.note_kind("0x80069898", "gpu_Box"),
       "RENAME")
    line = "gpu_draw_layered_box_80069898  = 0x80069898;  /* 3x SetTile */"
    outs = nk.registry_outputs(line, "named_syms.txt",
                               {"gpu_draw_layered_box_80069898": "func_80069898"},
                               lambda n, a: True, lambda n, a: False)
    check("keycheck: the function alias RESET note is a tool output",
          ["/* RESET 0x80069898: retired name 'gpu_draw_layered_box_80069898' — "
           "preserved note: 3x SetTile */"] in outs)


def test_naming_wave_py_escape_boundary() -> None:
    r"""naming_wave.sub_py splits a non-raw string literal on its escape sequences, so a name
    glued to `\t` / `\n` in the source is renamed with the rest of the file (wave52 half-renamed
    tools/test_label_census.py's cc1 dump: '\t.globl\tfunc_800272FC' has no \b before `func`).
    A raw literal keeps its backslashes as characters, so `r"\tfunc_A"` holds no name."""
    import re

    import naming_wave as nw

    pat = re.compile(r"\b(func_80001000)\b")
    lines = ['    "\\t.globl\\tfunc_80001000\\n"',
             '    "func_80001000:\\n"',
             '    "\\tjal\\tfunc_80001000\\n\\tnop\\n\\t.end\\tfunc_80001000\\n  # a long dump line"',
             '    "\\\\func_80001000"',
             '    r"\\tfunc_80001000"']
    new, n, _ = nw.sub_py(pat, lambda m: "sem_EscProbe", "X = (\n" + "\n".join(lines) + ")\n")
    got = new.split("\n")[1:6]
    eq("naming_wave: sub_py renames a name glued to an escape (short literal)",
       got[0], '    "\\t.globl\\tsem_EscProbe\\n"')
    eq("naming_wave: sub_py renames the plain key in the same file", got[1],
       '    "sem_EscProbe:\\n"')
    eq("naming_wave: sub_py renames names glued to escapes in a long literal", got[2],
       '    "\\tjal\\tsem_EscProbe\\n\\tnop\\n\\t.end\\tsem_EscProbe\\n  # a long dump line"')
    eq("naming_wave: sub_py renames after an escaped backslash", got[3],
       '    "\\\\sem_EscProbe"')
    eq("naming_wave: sub_py leaves a raw literal's glued name alone", got[4],
       '    r"\\tfunc_80001000")')
    eq("naming_wave: sub_py counts every rename", n, 5)


def test_census_manifest_precedence() -> None:
    """docs/naming/census_manifests.read_manifests: for one address the latest CONFIRMed
    manifest row wins -- a later reset-contradicted RESET retires an earlier sotn-review KEEP
    (wave10's gpu_draw_layered_box, reset by wave57), and a later KEEP supersedes an earlier
    RESET. Rows of other addresses and unCONFIRMed rows are unaffected."""
    import csv
    import sys

    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "docs" / "naming"))
    from census_manifests import read_manifests

    cols = ["addr", "glabel", "current_name", "proposed_name", "action", "evidence_class",
            "evidence", "verdict", "verifier_note"]
    tier = {"sotn-review": ("sotn-review", "CORROBORATED")}
    scratch = Path(__file__).resolve().parent.parent / "tmp"
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch) as td:
        def manifest(name, rows):
            p = Path(td, name, "func_manifest.csv")
            p.parent.mkdir()
            with open(p, "w", newline="", encoding="utf-8") as fh:
                w = csv.writer(fh, lineterminator="\n")
                w.writerow(cols)
                for r in rows:
                    w.writerow(r)
            return (str(p), None, None)

        keep = ["0x80069898", "gpu_draw_layered_box", "gpu_draw_layered_box",
                "gpu_draw_layered_box", "KEEP", "sotn-review", "e", "CONFIRM", "n"]
        reset = ["0x80069898", "gpu_draw_layered_box", "gpu_draw_layered_box",
                 "func_80069898", "RESET", "reset-contradicted", "h=1", "CONFIRM", "r"]
        other = ["0x80010000", "x_Other", "x_Other", "x_Other", "KEEP", "sotn-review", "e",
                 "CONFIRM", "n"]
        unconfirmed = list(reset)
        unconfirmed[0] = "0x80010000"
        unconfirmed[7] = "FAIL"
        w10 = manifest("wave10", [keep, other])
        w57 = manifest("wave57", [reset, unconfirmed])
        api, resets = read_manifests([w10, w57], tier)
        check("census: a later RESET retires an earlier KEEP", "80069898" not in api)
        eq("census: the later RESET is recorded", resets.get("80069898", {}).get("cls"),
           "reset-contradicted")
        eq("census: another address's KEEP is unaffected", api.get("80010000", {}).get("name"),
           "x_Other")
        check("census: an unCONFIRMed RESET is ignored", "80010000" not in resets)
        api, resets = read_manifests([w57, w10], tier)
        eq("census: a later KEEP supersedes an earlier RESET",
           (api.get("80069898", {}).get("name"), "80069898" in resets),
           ("gpu_draw_layered_box", False))


def test_naming_wave_keyed_registries() -> None:
    """A RENAME reaches every function-keyed grant / registry / fixture file:
    canonical_asm_regions.json and deferred.json keys, the golden fixtures'
    `name`, and the `<func> <rest>` lines of scope_allow / owner_cluster_grants /
    known_psyq_stdlib, leaving every other key, value and comment alone."""
    import naming_wave as nw
    from unittest.mock import patch

    scratch = Path(__file__).resolve().parent.parent / "tmp"
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch) as td:
        root = Path(td)
        (root / "asm/funcs").mkdir(parents=True)
        (root / "asm/funcs/func_80001000.s").write_text(
            "glabel func_80001000\n/* 000000 80001000 0800E003 */ jr $ra\n"
            "endlabel func_80001000\n", newline="\n")
        (root / "src/main").mkdir(parents=True)
        (root / "src/main/t.c").write_text("void func_80001000(void) {}\n", newline="\n")
        for d in ("tools/grinder", "engine", "oracle"):
            (root / d).mkdir(parents=True, exist_ok=True)

        def dump(rel, obj):
            (root / rel).write_text(json.dumps(obj, indent=2) + "\n", newline="\n")

        dump("tools/canonical_asm_regions.json", {"schema": 2, "functions": {
            "func_80001000": {"file": "main/t", "sha256": ["aa"]},
            "func_80002000": {"file": "main/u", "sha256": ["bb"]}}})
        dump("engine/deferred.json", {"_doc": "func_80001000 in prose stays",
                                      "members": {"func_80001000": "REFUSED: x"}})
        dump("oracle/manifest.json", {"schema": 1, "golden_fixtures": [
            {"name": "func_80001000", "file": "src/main/t.c"},
            {"name": "func_80002000", "file": "src/main/u.c"}]})
        lines = {
            "tools/grinder/scope_allow.txt": "# func_80001000 comment\n"
                                             "func_80001000 include/game.h\n",
            "tools/grinder/owner_cluster_grants.txt": "func_80001000 ruling.md\n",
            "known_psyq_stdlib.txt": "func_80001000  syscall_kernel  4 insns\n",
        }
        for rel, text in lines.items():
            (root / rel).write_text(text, newline="\n")

        op = nw.Op("0x80001000", "RENAME", "sem_KeyedProbe", {"glabel": "func_80001000"})
        op.olds = {"func_80001000"}
        with patch.object(nw, "ROOT", root), \
             patch.object(nw, "load_census", return_value=[]), \
             patch.object(nw, "_SRC_DEF_CACHE", {}), \
             patch.object(nw, "git_ignored", return_value=False), \
             patch.object(nw, "asm_file_referenced", return_value=True):
            wave = nw.Wave([op])
            before = dict(nw.residual_audit(wave)).get("func_80001000", [])
            for rel in ("tools/canonical_asm_regions.json", "engine/deferred.json",
                        "oracle/manifest.json", *lines):
                check(f"naming_wave: residual_audit sees func_80001000 in {rel} before apply",
                      rel in before)
            plan = nw.plan_wave(wave)
            for rel in ("tools/canonical_asm_regions.json", "engine/deferred.json",
                        "oracle/manifest.json"):
                check(f"naming_wave: {rel} is a planned JSON edit", rel in plan.json_edits)
            nw.apply_plan(plan)
            for rel in ("tools/canonical_asm_regions.json", "engine/deferred.json",
                        "oracle/manifest.json"):
                raw = (root / rel).read_bytes()
                eq(f"naming_wave: {rel} written as indent-2 JSON with LF",
                   raw, json.dumps(json.loads(raw), indent=2).encode() + b"\n")
            got = json.loads((root / "tools/canonical_asm_regions.json").read_text())
            eq("naming_wave: canonical_asm_regions key renamed, order and others kept",
               list(got["functions"]), ["sem_KeyedProbe", "func_80002000"])
            eq("naming_wave: canonical_asm_regions grant body unchanged",
               got["functions"].get("sem_KeyedProbe"), {"file": "main/t", "sha256": ["aa"]})
            got = json.loads((root / "engine/deferred.json").read_text())
            eq("naming_wave: deferred.json member key renamed",
               got["members"], {"sem_KeyedProbe": "REFUSED: x"})
            eq("naming_wave: deferred.json prose untouched",
               got["_doc"], "func_80001000 in prose stays")
            got = json.loads((root / "oracle/manifest.json").read_text())
            eq("naming_wave: golden fixture name renamed, other fixture kept",
               [f["name"] for f in got["golden_fixtures"]],
               ["sem_KeyedProbe", "func_80002000"])
            for rel, text in lines.items():
                want = text.replace("func_80001000 include", "sem_KeyedProbe include") \
                    if rel.endswith("scope_allow.txt") else \
                    text.replace("func_80001000", "sem_KeyedProbe")
                eq(f"naming_wave: {rel} key line renamed (comments kept)",
                   (root / rel).read_text(), want)
            eq("naming_wave: residual_audit clean over the keyed registries",
               dict(nw.residual_audit(wave)), {})


def test_naming_wave_own_target() -> None:
    """An address-named reference file can already belong to the live glabel."""
    import naming_wave as nw
    from unittest.mock import patch

    # Keep the fixture inside the workspace; no git writes or build needed.
    scratch = Path(__file__).resolve().parent.parent / "tmp"
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch) as td:
        root = Path(td)
        funcs = root / "asm/funcs"
        funcs.mkdir(parents=True)
        src = root / "src/main/test.c"
        src.parent.mkdir(parents=True)
        src.write_text("void sem_OwnTargetProbe(void) {}\n", newline="\n")
        target = funcs / "func_80001000.s"
        body = ("glabel sem_OwnTargetProbe\n"
                "/* 000000 80001000 0800E003 */ jr $ra\n"
                "endlabel sem_OwnTargetProbe\n")
        target.write_text(body, newline="\n")
        caller = funcs / "caller.s"
        caller.write_text("jal sem_OwnTargetProbe\n", newline="\n")

        def op():
            result = nw.Op("0x80001000", "RESET", "func_80001000",
                           {"glabel": "sem_OwnTargetProbe"})
            result.olds = {result.glabel}
            return result

        with patch.object(nw, "ROOT", root), \
             patch.object(nw, "load_census", return_value=[]), \
             patch.object(nw, "_SRC_DEF_CACHE", {}), \
             patch.object(nw, "git_ignored", return_value=False), \
             patch.object(nw, "asm_file_referenced", return_value=True):
            own = op()
            eq("naming_wave: own target passes preflight", nw.preflight([own]), {})
            plan = nw.plan_wave(nw.Wave([own]))
            eq("naming_wave: own target has no file moves", plan.file_renames, [])
            eq("naming_wave: own target has no file deletes", plan.file_deletes, [])
            nw.apply_plan(plan)
            eq("naming_wave: own target labels rewritten in place", target.read_text(),
               body.replace("sem_OwnTargetProbe", "func_80001000"))
            eq("naming_wave: own target caller rewritten", caller.read_text(),
               "jal func_80001000\n")
            eq("naming_wave: own target C definition rewritten", src.read_text(),
               "void func_80001000(void) {}\n")
            check("naming_wave: own target never creates semantic-stem file",
                  not (funcs / "sem_OwnTargetProbe.s").exists())

            for desc, invalid in (
                ("wrong opcode address", body.replace("80001000", "80001004")),
                ("matching later glabel", "glabel other_func\n" + body),
                ("missing opcode address", "glabel sem_OwnTargetProbe\n"),
            ):
                target.write_text(invalid, newline="\n")
                rejected = op()
                check(f"naming_wave: own target with {desc} refuses",
                      bool(nw.preflight([rejected]).get(rejected.address)))

            # Both filenames carrying the live glabel are still stale twins:
            # remove the unreferenced target, then move the semantic-stem file.
            target.write_text(body, newline="\n")
            semantic = funcs / "sem_OwnTargetProbe.s"
            semantic.write_text(body, newline="\n")
            with patch.object(nw, "asm_file_referenced", return_value=False):
                stale = op()
                eq("naming_wave: same-glabel stale twin passes preflight",
                   nw.preflight([stale]), {})
                eq("naming_wave: same-glabel stale twin marked for deletion",
                   stale.delete_stale, "asm/funcs/func_80001000.s")
                plan = nw.plan_wave(nw.Wave([stale]))
                eq("naming_wave: same-glabel stale twin plans semantic file move",
                   plan.file_renames,
                   [("asm/funcs/sem_OwnTargetProbe.s", "asm/funcs/func_80001000.s")])
                # Exercise apply's filesystem fallback without git writes.
                import subprocess
                with patch.object(nw.subprocess, "run", return_value=
                                  subprocess.CompletedProcess([], 1)):
                    nw.apply_plan(plan)
                check("naming_wave: same-glabel stale twin applies cleanly",
                      not semantic.exists() and target.read_text() ==
                      body.replace("sem_OwnTargetProbe", "func_80001000"))

            # Same bytes do not make another glabel this function's file.
            (funcs / "sem_OwnTargetProbe.s").write_text(body, newline="\n")
            target.write_text(body.replace("sem_OwnTargetProbe", "other_func"),
                              newline="\n")
            other = op()
            errors = nw.preflight([other])
            check("naming_wave: referenced twin with different glabel still refuses",
                  "it IS referenced by a build input" in
                  "\n".join(errors.get(other.address, [])))
            eq("naming_wave: refused twin is not marked for deletion",
               other.delete_stale, None)
            (funcs / "sem_OwnTargetProbe.s").unlink()
            with patch.object(nw, "asm_file_referenced", return_value=False):
                other = op()
                errors = nw.preflight([other])
                check("naming_wave: different function target still refuses",
                      "its opcode bytes DIFFER" in
                      "\n".join(errors.get(other.address, [])))


def test_naming_wave_renames() -> None:
    """naming_wave rename mechanics the Q39 record depends on (round-5 review
    C1-C4): the stale-duplicate .s delete-then-rename still applies under the
    preflight; a missing source is refused with NOTHING written; a legacy
    single-string renamed_from is promoted to a list; any other shape dies
    with a message naming the record."""
    import naming_wave as nw

    def run(plan):
        """(died, stderr) — apply `plan` rooted at the cwd."""
        saved, err = nw.ROOT, io.StringIO()
        nw.ROOT = Path.cwd()
        try:
            with contextlib.redirect_stderr(err):
                nw.apply_plan(plan)
            return False, err.getvalue()
        except SystemExit:
            return True, err.getvalue()
        finally:
            nw.ROOT = saved

    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td:
        os.chdir(td)
        try:
            # C1: the stale duplicate under the NEW name is deleted, then the
            # old .s moves onto that path — the preflight must count the delete
            Path("asm/funcs").mkdir(parents=True)
            Path("asm/funcs/func_OLD.s").write_text("glabel func_OLD\n")
            Path("asm/funcs/func_NEW.s").write_text("stale duplicate\n")
            plan = nw.Plan()
            plan.file_deletes.append("asm/funcs/func_NEW.s")
            plan.file_renames.append(("asm/funcs/func_OLD.s", "asm/funcs/func_NEW.s"))
            died, err = run(plan)
            check("naming_wave: stale-duplicate .s delete-then-rename applies",
                  not died and not Path("asm/funcs/func_OLD.s").exists()
                  and Path("asm/funcs/func_NEW.s").read_text() == "glabel func_OLD\n")

            # C2: a rename whose source is missing is refused before ANY write
            Path("engine").mkdir()
            Path("engine/queue.json").write_text("before\n")
            plan = nw.Plan()
            plan.json_edits["engine/queue.json"] = ("after\n", ["x"])
            plan.dir_renames.append(("memory/grind/func_GONE", "memory/grind/func_G2"))
            died, err = run(plan)
            check("naming_wave: a missing rename source dies in preflight, nothing written",
                  died and "rename preflight failed" in err
                  and "cannot move missing path memory/grind/func_GONE" in err
                  and Path("engine/queue.json").read_text() == "before\n")
        finally:
            os.chdir(cwd)

    # C3: a legacy single-string renamed_from becomes a list, on both writers
    line = json.dumps({"func": "func_B", "verdict": "PASS", "body_hash": "0" * 16,
                       "renamed_from": "func_A"})
    text, _ = nw.retarget_layer2_record(line, "func_B", "func_C")
    eq("naming_wave: legacy string renamed_from promoted (layer2.jsonl)",
       json.loads(text)["renamed_from"], ["func_A", "func_B"])
    data = {"items": [{"func": "func_B", "addr": "80010000"}]}
    nw.retarget_queue_items(data, {"func_B": "func_C"})
    eq("naming_wave: a queue item is renamed in place — no chain (the audit keys on addr)",
       data["items"], [{"func": "func_C", "addr": "80010000"}])

    # C4: any other shape is a clear, named refusal — never a guess
    for bad in ({"x": 1}, 7, ["func_A", 3]):
        err = io.StringIO()
        try:
            with contextlib.redirect_stderr(err):
                nw.extend_rename_chain(bad, "func_B", "memory/grind/func_B/layer2.jsonl line 1")
            died = False
        except SystemExit:
            died = True
        check(f"naming_wave: renamed_from {bad!r} is refused by name",
              died and "memory/grind/func_B/layer2.jsonl line 1" in err.getvalue()
              and "renamed_from must be a list" in err.getvalue())


def test_orphaned_local_decls() -> None:
    """find_orphaned_local_decls — the strip-completeness closure (2026-08-06).

    A detected construct whose stripped span held a local's only references
    used to leave the DECLARATION standing, so GCC still reserved the frame
    bytes and the cheat-invisible sandbox under-reported the honest pure-C
    distance. Measured on gnd_init_80041688: stripped distance 2 vs true
    cheat-free 8, the whole gap being a 32-byte volatile array (fd1497f7).
    """

    # Positive — the gnd_init shape: volatile array whose only use is a
    # `(void)` discard the stripper already removes.
    text = """\
void f(void) {
    s32 live;
    volatile s32 sp10[8];

    live = 1;
    use(live);
    (void)sp10;
}
"""
    stripped, _n = volatile_cheats.strip_volatile_cheats_file(text)
    check("orphan-decl: strips the declaration whose only use was a discard",
          "sp10" not in stripped)
    check("orphan-decl: leaves an unrelated live local alone",
          "s32 live;" in stripped)
    cnt = volatile_cheats.func_volatile_cheat_count(text, "f")
    eq("orphan-decl: count covers discard + orphaned declaration", cnt, 2)

    # Negative — a volatile local WITH real uses is not stripped. `hw` is read
    # into a live value, so nothing about it is a frame-coercion cheat.
    text2 = """\
void g(void) {
    volatile s32 hw;
    s32 dead;

    hw = read_reg();
    sink(hw);
    (void)dead;
}
"""
    stripped2, _n2 = volatile_cheats.strip_volatile_cheats_file(text2)
    check("orphan-decl: volatile local with real uses is KEPT",
          "volatile s32 hw;" in stripped2)
    check("orphan-decl: its real uses survive too", "sink(hw);" in stripped2)
    check("orphan-decl: the genuinely orphaned sibling is still stripped",
          "dead" not in stripped2)

    # Negative — zero-reference locals are out of scope: the closure requires a
    # reference INSIDE a stripped span, so a plain unused local stays.
    text3 = """\
void h(void) {
    s32 never_used;
    s32 dead;

    (void)dead;
}
"""
    stripped3, _n3 = volatile_cheats.strip_volatile_cheats_file(text3)
    check("orphan-decl: zero-reference local is NOT claimed by the closure",
          "s32 never_used;" in stripped3)

    # Conservative keeps: an initializer may carry a side effect, and a
    # multi-declarator statement may have live siblings. Both stay, and the
    # audit reports them so the under-strip is visible.
    text4 = """\
void i(void) {
    s32 sized = compute();
    s32 a, b;

    (void)sized;
    if (1) {
        a = b;
    }
}
"""
    stripped4, _n4 = volatile_cheats.strip_volatile_cheats_file(text4)
    check("orphan-decl: initialized declaration is KEPT (side-effect safety)",
          "s32 sized = compute();" in stripped4)
    check("orphan-decl: multi-declarator statement is KEPT",
          "s32 a, b;" in stripped4)
    reasons = {(d["name"], d["reason"]) for d in
               volatile_cheats.orphaned_decl_audit(text4)["kept"]}
    check("orphan-decl: audit documents the initializer keep",
          any(n == "sized" and "initializer" in r for n, r in reasons))
    check("orphan-decl: audit documents the multi-declarator keep",
          any(n == "a" and "declarator" in r for n, r in reasons))

    # Struct members are not locals — a body-local struct definition must not
    # have its members stripped.
    text5 = """\
void j(void) {
    struct { s32 field; } s;
    s32 dead;

    s.field = 1;
    sink(s.field);
    (void)dead;
}
"""
    stripped5, _n5 = volatile_cheats.strip_volatile_cheats_file(text5)
    check("orphan-decl: struct member declaration is not treated as a local",
          "s32 field;" in stripped5)


def test_queue_write_serialization() -> None:
    """A queue mutation must never SILENTLY lose a concurrent one.

    Measured 2026-08-07 (docs/grind/decisions.md): a `queue done` reported
    ok:true and was then clobbered, so the completion vanished with no error
    anywhere. Contract now: mutators run their load-modify-save under an
    advisory lock and save through an atomic write-then-rename guarded by a
    content fingerprint. Interleaved writers are either serialized (both
    survive) or fail LOUDLY with QueueConflict — never silently dropped."""
    def seed(qp: Path, funcs) -> None:
        items = [{"func": f, "file": "text1a_c", "distance": 1, "verdict": "C",
                  "rules": 0, "status": "active"} for f in funcs]
        qp.write_text(json.dumps({"items": items, "counts": {}}, indent=2) + "\n")

    with tempfile.TemporaryDirectory() as td, _stub_tus(td, {
            "text1a_c": "void func_A(void) {}\nvoid func_LAST(void) {}\n",
            "x": "void func_Z(void) {}\n"}):
        qp = Path(td) / "queue.json"
        orig_path = Q.QUEUE_PATH
        Q.QUEUE_PATH = str(qp)
        try:
            # --- 1. SERIALIZED: two sequential mutations both persist. This is
            #        the shape the race destroyed (B's park ate A's completion).
            seed(qp, ["func_A", "func_B", "func_C"])
            Q.mark_parked("func_A", reason="lane A")
            Q.mark_parked("func_B", reason="lane B")
            by_func = {it["func"]: it for it in Q.load()["items"]}
            # mark_parked is a legacy alias for mark_rotated since the
            # 2026-09-08 ruling (rotation-not-foreclosure) — the persisted
            # status is "rotated"; the lock semantics under test are the same.
            eq("queue lock: lane A's park survived lane B's write",
               by_func["func_A"]["status"], "rotated")
            eq("queue lock: lane B's park landed too",
               by_func["func_B"]["status"], "rotated")
            eq("queue lock: untouched item intact",
               by_func["func_C"]["status"], "active")

            # --- 2. INTERLEAVED: writer A reads, writer B commits, then A
            #        writes its STALE snapshot. A must be refused, not win.
            seed(qp, ["func_A", "func_B"])
            a_snapshot = Q.load()                    # writer A reads
            a_token = Q._fingerprint()
            Q.mark_parked("func_B", reason="writer B wins the race")   # B commits
            a_snapshot["items"] = [it for it in a_snapshot["items"]
                                   if it["func"] != "func_A"]          # A mutates
            conflicted = False
            try:
                Q.save(a_snapshot, expect=a_token)                     # A writes
            except Q.QueueConflict:
                conflicted = True
            check("queue lock: stale interleaved write raises QueueConflict",
                  conflicted)
            after = {it["func"]: it for it in Q.load()["items"]}
            eq("queue lock: writer B's park was NOT silently clobbered",
               after.get("func_B", {}).get("status"), "rotated")
            check("queue lock: writer A's stale drop did not take effect",
                  "func_A" in after)

            # --- 3. The fingerprint tracks CONTENT, so an unchanged file lets
            #        the same cycle commit normally (no spurious conflicts).
            seed(qp, ["func_A"])
            tok = Q._fingerprint()
            q = Q.load()
            q["items"] = []
            Q.save(q, expect=tok)
            eq("queue lock: uncontended write commits", len(Q.load()["items"]), 0)

            # --- 4. Back-compat: save() with no `expect` is unchecked, as
            #        before. Callers that legitimately overwrite (test seeds,
            #        `regen --no-preserve`) keep working.
            Q.save({"items": [{"func": "func_Z", "file": "x", "distance": 0,
                               "verdict": "C", "rules": 0, "status": "active"}]})
            eq("queue lock: unchecked save still overwrites",
               Q.load()["items"][0]["func"], "func_Z")

            # --- 5. mark_done re-reads under the lock AFTER a minutes-long
            #        gate (O.verify). If queue.json vanished meanwhile, load()
            #        answers {"items": []} and _fingerprint() answers "" — they
            #        AGREE, so the save would go through and write an empty
            #        worklist with ok:true. Refuse loudly instead (layer-2
            #        review 2026-08-07). Stub the gates so this stays pure
            #        logic: the point is the write, not the completion bar.
            seed(qp, ["func_A", "func_B"])
            saved = (Q._rule_count, Q.cheats.func_prologue_count,
                     Q.cheats.maspsx_gate_entries, Q.cheats.canonical_asm_funcs,
                     Q.inlineasm.file_func_cheat_asm_count,
                     completion.source_issues, Q.O.verify, Q.layer2.gate)
            Q._rule_count = lambda f: 0
            Q.layer2.gate = lambda f, s: None  # the Q39 gate has its own test
            Q.cheats.func_prologue_count = lambda f: 0
            Q.cheats.maspsx_gate_entries = lambda f: []
            Q.cheats.canonical_asm_funcs = lambda: set()
            Q.inlineasm.file_func_cheat_asm_count = lambda s, f: 0
            completion.source_issues = lambda *a, **k: []
            # the "gate" deletes queue.json, standing in for a concurrent
            # git checkout / stash landing during O.verify
            def _verify_that_loses_the_file(rebuild=False):
                qp.unlink()
                return {"build_matches": True, "build_sha1": "deadbeef"}
            Q.O.verify = _verify_that_loses_the_file
            try:
                vanished = False
                try:
                    Q.mark_done("func_A")
                except Q.QueueConflict:
                    vanished = True
                check("queue lock: mark_done refuses to write an empty queue "
                      "when queue.json vanishes mid-gate", vanished)
                check("queue lock: no empty queue.json was written",
                      not qp.exists())

                # ...and the guard must NOT fire on the legitimate case it
                # most resembles: completing the LAST item, which correctly
                # leaves an empty queue. The check is on the RE-READ being
                # empty, not on the post-drop result.
                seed(qp, ["func_LAST"])
                Q.O.verify = lambda rebuild=False: {"build_matches": True,
                                                    "build_sha1": "deadbeef"}
                r = Q.mark_done("func_LAST")
                eq("queue lock: completing the LAST item still succeeds",
                   r.get("ok"), True)
                eq("queue lock: last completion empties the queue", Q.load()["items"], [])
            finally:
                (Q._rule_count, Q.cheats.func_prologue_count,
                 Q.cheats.maspsx_gate_entries, Q.cheats.canonical_asm_funcs,
                 Q.inlineasm.file_func_cheat_asm_count,
                 completion.source_issues, Q.O.verify, Q.layer2.gate) = saved

            # --- 6. Atomicity hygiene: the write-then-rename staging file is
            #        never left behind (a reader must never find a partial
            #        queue, and the tree must stay clean).
            leftovers = sorted(p.name for p in Path(td).iterdir()
                               if p.name.startswith("queue.json.tmp"))
            eq("queue lock: no staging file left behind", leftovers, [])
        finally:
            Q.QUEUE_PATH = orig_path


def test_memo_and_rule_index_caches() -> None:
    """Pin the 2026-08-07 memoization contracts (promoted from
    tmp/spotcheck/verify_memo.py per owner approval 2026-08-07).

    Load-bearing property: find_plain_volatile_externs / find_all_cheats depend
    on volatile_extern_allowlist.txt, not on `text` alone. A text-only memo key
    would let a stale answer survive an allowlist edit — the exact D_800F7420
    failure class (a silently dropped grant) reintroduced INSIDE the detector.
    Also pins: use_allowlist() overrides are observed through the memo and fully
    restored on exit; returned lists are fresh copies; and the func_rule_lines
    one-pass index invalidates on config edit."""
    import time as _t

    def _clear_memos():
        for f in (inlineasm._strip_spans_cached,
                  inlineasm._register_hint_spans_cached,
                  volatile_cheats._find_alias_renames_cached,
                  volatile_cheats._find_plain_volatile_externs_cached,
                  volatile_cheats._find_all_cheats_cached):
            f.cache_clear()

    text = (
        'extern volatile s32 D_80098894;\n'
        'extern volatile s32 D_8009AAAA;\n'
        'extern volatile s32 D_80098894_v asm("D_80098894");\n'
        'void f(void) {\n'
        '    register int x;\n'
        '    __asm__ volatile ("addu $8, $3, $zero");\n'
        '    x = 1;\n'
        '}\n'
    )

    original_cache = volatile_cheats._volatile_extern_allowlist_cache
    original_cwd = os.getcwd()
    try:
        with tempfile.TemporaryDirectory() as td:
            os.chdir(td)
            volatile_cheats._volatile_extern_allowlist_cache = None
            Path("volatile_extern_allowlist.txt").write_text(
                "D_80098894    # granted\nD_8009AAAA    # granted\n")

            # 1. Memoized results identical to from-scratch, cold/warm/re-cold.
            _clear_memos()
            cold_all = volatile_cheats.find_all_cheats(text)
            cold_strip = inlineasm._strip_spans(text)
            warm_all = volatile_cheats.find_all_cheats(text)
            warm_strip = inlineasm._strip_spans(text)
            _clear_memos()
            recold_all = volatile_cheats.find_all_cheats(text)
            eq("memo: find_all_cheats warm == cold", warm_all, cold_all)
            eq("memo: find_all_cheats recold == cold", recold_all, cold_all)
            eq("memo: _strip_spans warm == cold", warm_strip, cold_strip)

            # 2. An allowlist edit is OBSERVED through the memo (invalidation).
            before = len(volatile_cheats.find_plain_volatile_externs(text))
            eq("memo: both plain externs granted before the edit", before, 0)
            Path("volatile_extern_allowlist.txt").write_text("# emptied\n")
            future = _t.time() + 2
            os.utime("volatile_extern_allowlist.txt", (future, future))
            after = len(volatile_cheats.find_plain_volatile_externs(text))
            eq("memo: allowlist emptied -> plain externs now flagged THROUGH "
               "the memo (no stale answer)", after, 2)
            after_all = volatile_cheats.find_all_cheats(text)
            eq("memo: find_all_cheats grew with the dropped grants",
               len(after_all), len(cold_all) + 2)

            # 3. use_allowlist(): override observed through the memo, restored
            # exactly on exit; path=None is a no-op.
            Path("granted.txt").write_text("D_80098894\nD_8009AAAA\n")
            with volatile_cheats.use_allowlist(str(Path("granted.txt").resolve())):
                eq("use_allowlist: override observed through the memo",
                   len(volatile_cheats.find_plain_volatile_externs(text)), 0)
                with volatile_cheats.use_allowlist(None):
                    eq("use_allowlist: None nested inside is a no-op "
                       "(keeps the outer override)",
                       len(volatile_cheats.find_plain_volatile_externs(text)), 0)
            eq("use_allowlist: working-tree allowlist restored on exit",
               len(volatile_cheats.find_plain_volatile_externs(text)), 2)

            # 4. Returned lists are fresh copies — a mutating caller cannot
            # poison the cache.
            a = volatile_cheats.find_all_cheats(text)
            n = len(a)
            a.clear()
            eq("memo: mutating find_all_cheats result does not corrupt cache",
               len(volatile_cheats.find_all_cheats(text)), n)
            b = inlineasm._strip_spans(text)
            m = len(b)
            b.clear()
            eq("memo: mutating _strip_spans result does not corrupt cache",
               len(inlineasm._strip_spans(text)), m)

            # 5. func_rule_lines one-pass index: correct, invalidates on edit,
            # returns fresh copies.
            cfgp = str(Path("rules.txt").resolve())
            Path(cfgp).write_text(
                'foo: insert_after "addu\\t$18,$0,$zero" @ 52\n'
                '# comment line\n'
                'bar: rename $t0 $t1\n'
                'foo: reorder 3 5\n')
            eq("rule-index: two lines keyed foo, correct indices",
               cheats.func_rule_lines("foo", cfgp),
               [(0, 'foo: insert_after "addu\\t$18,$0,$zero" @ 52'),
                (3, 'foo: reorder 3 5')])
            eq("rule-index: one line keyed bar",
               len(cheats.func_rule_lines("bar", cfgp)), 1)
            eq("rule-index: unknown key -> empty",
               cheats.func_rule_lines("baz", cfgp), [])
            got = cheats.func_rule_lines("foo", cfgp)
            got.clear()
            eq("rule-index: mutating the result does not corrupt the cache",
               len(cheats.func_rule_lines("foo", cfgp)), 2)
            Path(cfgp).write_text('bar: rename $t0 $t1\n')
            os.utime(cfgp, (future + 2, future + 2))
            eq("rule-index: config edit observed (foo's rules gone)",
               cheats.func_rule_lines("foo", cfgp), [])
            eq("rule-index: missing file -> empty",
               cheats.func_rule_lines("foo", str(Path("absent.txt").resolve())), [])
    finally:
        os.chdir(original_cwd)
        volatile_cheats._volatile_extern_allowlist_cache = original_cache
        _clear_memos()


def test_sanctioned_unwritten_pads() -> None:
    """The owner-ruled 2026-08-17 unwritten-pad allowlist: EXACTLY the three
    (function, name, count) triples, volatile-qualified, are exempt from the
    unused-local-array roster; every deviation stays a cheat."""
    NL = chr(10)

    def spans_for(fname: str, body: str):
        text = "void " + fname + "(void) {" + NL + body + "}" + NL
        lo = text.index("{")
        hi = text.rindex("}") + 1
        params: list = []
        return volatile_cheats.body_cheat_spans(text, lo, hi, params, fname)

    pad = "    volatile u32 pre_pad[2];" + NL + "    other();" + NL
    eq("pad-allowlist: sanctioned func+name+count+volatile is exempt",
       len(spans_for("camera_ApplyFightView", pad)), 0)
    eq("pad-allowlist: second sanctioned function exempt",
       len(spans_for("camera_ApplyReplayView", pad)), 0)
    eq("pad-allowlist: unsanctioned function still flagged",
       len(spans_for("func_80012345", pad)), 1)
    eq("pad-allowlist: non-volatile spelling still flagged",
       len(spans_for("camera_ApplyFightView",
                     "    s32 pre_pad[2];" + NL + "    other();" + NL)), 1)
    eq("pad-allowlist: wrong count still flagged",
       len(spans_for("camera_ApplyFightView",
                     "    volatile u32 pre_pad[8];" + NL + "    other();" + NL)), 1)
    eq("pad-allowlist: wrong name still flagged",
       len(spans_for("camera_ApplyFightView",
                     "    volatile u32 spill[2];" + NL + "    other();" + NL)), 1)
    text = "void camera_ApplyFightView(void) {" + NL + pad + "}" + NL
    eq("pad-allowlist: no-fname call (default) still flagged",
       len(volatile_cheats.body_cheat_spans(
           text, text.index("{"), text.rindex("}") + 1, [])), 1)


def test_queue_hand_coded_tier() -> None:
    """The 2026-08-18 gate fix: regen must PERSIST the scan tier on every item
    the gate consulted the scanner for, and a LOW tier must re-verdict a
    distance-only suspect as an ordinary C target.

    Root cause it pins: queue._route re-implemented canonical._verdict's tier
    logic, computed the tier for corroboration and then THREW IT AWAY — so
    every ASM-SUSPECT entry in queue.json carried no evidence field, and the
    50 < distance <= 500 band never consulted the scanner at all.
    """
    def _regen(distance: int, tier: str, prev: dict | None = None) -> dict:
        with tempfile.TemporaryDirectory() as td:
            cwd = os.getcwd()
            os.chdir(td)
            Path("build/src").mkdir(parents=True)
            Path("build/src/faketu.o").write_text("")
            Path("src").mkdir()
            Path("src/faketu.c").write_text("void func_WB(void) { body(); }" + chr(10))
            orig = (Q.QUEUE_PATH, P.c_stems, canonical.scan_all,
                    cheats.canonical_asm_funcs, Q._rule_count,
                    cheats.func_prologue_count, inlineasm.file_func_cheat_asm_count,
                    cheats.is_jtbl_infra, cheats.is_canonical_extraction_only,
                    Q.sandbox.build_stripped_object, score._o_func_table,
                    score.score_func, canonical._hand_coded_tier)
            Q.QUEUE_PATH = str(Path(td) / "queue.json")
            Path(Q.QUEUE_PATH).write_text(json.dumps(
                {"items": [prev] if prev else [], "counts": {}}))
            P.c_stems = lambda: ["faketu"]
            canonical.scan_all = lambda: []
            cheats.canonical_asm_funcs = lambda: set()
            Q._rule_count = lambda f: 0
            cheats.func_prologue_count = lambda f: 0
            inlineasm.file_func_cheat_asm_count = lambda s, f: 0
            cheats.is_jtbl_infra = lambda f: False
            cheats.is_canonical_extraction_only = lambda f: False
            Q.sandbox.build_stripped_object = lambda *a, **k: {}
            score._o_func_table = lambda o: {"func_WB": (0, 0)}
            score.score_func = lambda a, b, f: {"score": distance}
            canonical._hand_coded_tier = lambda f: tier
            try:
                return Q.generate(workdir=td)["items"][0]
            finally:
                os.chdir(cwd)
                (Q.QUEUE_PATH, P.c_stems, canonical.scan_all,
                 cheats.canonical_asm_funcs, Q._rule_count,
                 cheats.func_prologue_count, inlineasm.file_func_cheat_asm_count,
                 cheats.is_jtbl_infra, cheats.is_canonical_extraction_only,
                 Q.sandbox.build_stripped_object, score._o_func_table,
                 score.score_func, canonical._hand_coded_tier) = orig

    # (a) tier PERSISTED on every scanned item, in both distance bands
    eq("queue tier: persisted on a >SUSPECT item",
       _regen(60, "TIGHT_C").get("hand_coded_tier"), "TIGHT_C")
    eq("queue tier: persisted on a >NEAR_CERTAIN item",
       _regen(600, "POSSIBLE").get("hand_coded_tier"), "POSSIBLE")
    eq("queue tier: absent when the scanner was never consulted (distance<=50)",
       "hand_coded_tier" in _regen(10, "LOW"), False)

    # (b) LOW tier re-verdicts a suspect to an ordinary C target (both bands),
    #     and it stays in the ACTIVE lane — this is pure-C work, not authorize.
    low_hi = _regen(600, "LOW")
    eq("queue tier: LOW at distance>500 re-verdicts to C", low_hi["verdict"], "C")
    eq("queue tier: LOW re-verdict stays active", low_hi["status"], "active")
    eq("queue tier: LOW re-verdict records the evidence",
       low_hi["hand_coded_tier"], "LOW")
    eq("queue tier: LOW at distance>50 re-verdicts to C",
       _regen(60, "LOW")["verdict"], "C")

    # (c) a genuinely-signalled function still routes to ASM-STRUCTURAL /
    #     authorize at >500, and an unknown tier keeps the SUSPECT label.
    strong = _regen(600, "STRONG")
    eq("queue tier: STRONG at distance>500 still routes ASM-STRUCTURAL",
       strong["verdict"], "ASM-STRUCTURAL")
    eq("queue tier: ASM-STRUCTURAL still lands in authorize",
       strong["status"], "authorize")
    eq("queue tier: POSSIBLE at distance>500 still routes ASM-STRUCTURAL",
       _regen(600, "POSSIBLE")["verdict"], "ASM-STRUCTURAL")
    eq("queue tier: UNAVAILABLE is not counter-evidence — stays ASM-SUSPECT",
       _regen(600, "UNAVAILABLE")["verdict"], "ASM-SUSPECT")
    eq("queue tier: POSSIBLE below near-certain stays ASM-SUSPECT",
       _regen(60, "POSSIBLE")["verdict"], "ASM-SUSPECT")

    # (d) STICKY items (owner_override / parked) are carried verbatim by regen,
    #     so they never reach _route at all — the second half of the tier gap.
    #     They get the evidence recorded; an ACTIVE LOW suspect also sheds the
    #     label, but STATUS is never moved by the gate.
    def _sticky(status: str, tier: str, verdict: str = "ASM-SUSPECT") -> dict:
        return _regen(600, tier, prev={
            "func": "func_WB", "file": "faketu", "distance": 600,
            "verdict": verdict, "rules": 0, "status": status,
            "owner_override": True, "regate": "prior note"})

    ov = _sticky("active", "LOW")
    eq("queue tier: sticky owner_override suspect records the tier",
       ov["hand_coded_tier"], "LOW")
    eq("queue tier: sticky active LOW suspect re-verdicts to C", ov["verdict"], "C")
    eq("queue tier: sticky re-verdict keeps the owner override", ov["owner_override"], True)
    check("queue tier: sticky re-verdict appends to the prior regate note",
          ov["regate"].startswith("prior note | 2026-08-18 re-gate"))
    parked = _sticky("parked", "LOW")
    eq("queue tier: a PARKED sticky item keeps its verdict (status is sticky)",
       parked["verdict"], "ASM-SUSPECT")
    eq("queue tier: a parked sticky item still records the tier",
       parked["hand_coded_tier"], "LOW")
    eq("queue tier: sticky TIGHT_C keeps the ASM-SUSPECT label",
       _sticky("active", "TIGHT_C")["verdict"], "ASM-SUSPECT")
    eq("queue tier: sticky ASM-STRUCTURAL is never demoted by the gate",
       _sticky("authorize", "LOW", verdict="ASM-STRUCTURAL")["verdict"],
       "ASM-STRUCTURAL")


def test_datamodel() -> None:
    """engine/datamodel.py (2026-09-03): declared shape vs evidence signals,
    evaluated against a synthetic repo tree."""
    from engine import datamodel as DM
    print("test_datamodel")
    cwd = os.getcwd()
    try:
        _test_datamodel_body()
    finally:
        os.chdir(cwd)


def _test_datamodel_body() -> None:
    from engine import datamodel as DM
    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td:
        os.chdir(td)
        for d in ("asm/funcs", "include", "src"):
            Path(d).mkdir(parents=True)
        Path("named_syms.txt").write_text(
            "g_fixture_slot_state = 0x80F00018;  /* 6-byte slot table (one counter byte per slot) */\n"
            "g_fixture_pos_table = 0x80F00050;  /* 12-byte stride per slot, 6 entries */\n"
            "g_fixture_pos_table_plus_4 = 0x80F00054;  /* +4 from g_fixture_pos_table (0x80F00050) */\n"
            "g_fixture_plain = 0x80F00100;  /* just a flag */\n", encoding="utf-8")
        Path("undefined_syms_auto.txt").write_text(
            "D_80F00018 = 0x80F00018;\nD_80F00050 = 0x80F00050;\nD_80F00054 = 0x80F00054;\n"
            "D_80F00100 = 0x80F00100;\n", encoding="utf-8")
        Path("include/x.h").write_text(
            "extern u8 D_80F00018;\nextern s32 D_80F00050;\nextern s32 D_80F00054;\n"
            "extern u8 D_80F00100;\n", encoding="utf-8")
        Path("asm/funcs/func_A.s").write_text(
            "glabel func_A\n"
            "    lui $at, %hi(D_80F00018)\n    addu $at, $at, $v1\n"
            "    lbu $v0, %lo(D_80F00018)($at)\n"
            "    lui $at, %hi(D_80F00050)\n    sw $v1, %lo(D_80F00050)($at)\n"
            "    lui $at, %hi(D_80F00054)\n    sw $a0, %lo(D_80F00054)($at)\n"
            "    lui $v0, %hi(D_80F00100)\n    lbu $v0, %lo(D_80F00100)($v0)\n"
            "    jal func_B\n", encoding="utf-8")
        Path("asm/funcs/func_B.s").write_text(
            "glabel func_B\n    lui $at, %hi(D_80F00054)\n    lw $v0, %lo(D_80F00054)($at)\n",
            encoding="utf-8")
        Path("src/a.c").write_text('INCLUDE_ASM("asm/funcs", func_A);\n'
                                   'INCLUDE_ASM("asm/funcs", func_B);\n', encoding="utf-8")
        DM.reset_cache()
        rows, flags = DM.data_model("func_A")
        eq("one row per data symbol, callee excluded", len(rows), 4)
        check("rows are address-ordered", rows[0].startswith("  D_80F00018"))
        check("sub-symbol row marked", any("SUB-SYMBOL +4 of g_fixture_pos_table" in r for r in rows))
        check("census comment surfaced", any('"12-byte stride per slot, 6 entries"' in r for r in rows))
        check("header decl surfaced", any("decl `extern u8 D_80F00018;` (x.h)" in r for r in rows))
        check("asm xref lists the still-INCLUDE_ASM sibling", any("xref asm:func_B" in r for r in rows))
        kinds = [f.split(":")[0] for f in flags]
        # D_80F00018: indexed + census ("table"); D_80F00050: census ("stride");
        # the split flag; D_80F00050's plain lui/sw must NOT read as indexed.
        eq("signals: 2x census-vs-decl + indexed + split-aggregate", sorted(kinds),
           sorted(["!! INDEXED-ACCESS", "!! CENSUS-VS-DECL", "!! CENSUS-VS-DECL",
                   "!! SPLIT-AGGREGATE"]))
        check("indexed flag names the indexed symbol only",
              any("INDEXED-ACCESS: the target indexes D_80F00018" in f for f in flags)
              and not any("INDEXED-ACCESS: the target indexes D_80F00050" in f for f in flags))
        split = next(f for f in flags if f.startswith("!! SPLIT-AGGREGATE"))
        check("split flag names the sibling that keeps the config row alive",
              "still-INCLUDE_ASM func_B" in split)
        check("plain flag global raises no signal", not any("D_80F00100" in f for f in flags))
        # a declaration merge silences the signals it answers
        Path("include/x.h").write_text(
            "extern u8 D_80F00018[6];\nextern FixturePos D_80F00050[6];\nextern u8 D_80F00100;\n",
            encoding="utf-8")
        DM.reset_cache()
        _, flags2 = DM.data_model("func_A")
        check("array decl clears INDEXED-ACCESS + CENSUS-VS-DECL",
              not any(f.startswith(("!! INDEXED", "!! CENSUS")) for f in flags2))
        eq("no asm file -> empty", DM.data_model("func_missing"), ([], []))
        # dossier + render wiring
        from engine import dossier as DS
        DM.reset_cache()
        check("dossier carries the data-model block", "data model (4 globals" in DS.dossier("func_A"))
        os.chdir(cwd)  # leave the temp dir before it is removed (Windows)


def test_queue_rotation() -> None:
    """Owner ruling 2026-09-08 (rotation-not-foreclosure): `rotated` replaces
    `foreclosed`; legacy statuses normalise to rotated; the oldest rotation
    returns automatically when the active list drains; a coupled sibling
    notice stamped after `rotated_at` returns the item; an unchanged
    toolchain fingerprint re-measures nothing."""
    def seed(qp: Path, items) -> None:
        qp.write_text(json.dumps({"items": items, "counts": {}}, indent=2) + "\n")

    with tempfile.TemporaryDirectory() as td:
        qp = Path(td) / "queue.json"
        orig_path, orig_cwd = Q.QUEUE_PATH, os.getcwd()
        Q.QUEUE_PATH = str(qp)
        os.chdir(td)          # ledger lookups + fingerprint inputs resolve here (none exist)
        try:
            base = {"file": "text1a_c", "distance": 1, "verdict": "C", "rules": 0}
            # 1. rotate: status + reason + stamp; legacy alias still works
            seed(qp, [dict(base, func="f_A", status="active"),
                      dict(base, func="f_B", status="active")])
            r = Q.mark_rotated("f_A", "ROTATED: test record")
            eq("rotation: ok", r["ok"], True)
            it = {i["func"]: i for i in Q.load()["items"]}
            eq("rotation: status rotated", it["f_A"]["status"], "rotated")
            check("rotation: rotated_at stamped", bool(it["f_A"].get("rotated_at")))
            eq("rotation: legacy foreclose alias", Q.mark_foreclosed("f_B", "x")["ok"], True)
            # 2. queue drain -> oldest rotation returns with an unpark_reason
            it = {i["func"]: i for i in Q.load()["items"]}
            it["f_A"]["rotated_at"] = "2026-01-01T00:00:00+00:00"   # make f_A the oldest
            seed(qp, list(it.values()))
            r = Q.auto_return(rescan=False)
            eq("auto-return: one item returned on drain", len(r["returned"]), 1)
            eq("auto-return: the oldest rotation returned", r["returned"][0]["func"], "f_A")
            it = {i["func"]: i for i in Q.load()["items"]}
            eq("auto-return: status active", it["f_A"]["status"], "active")
            check("auto-return: unpark_reason set (resets the exhaustion window)",
                  "drained" in it["f_A"].get("unpark_reason", ""))
            eq("auto-return: the other stays rotated", it["f_B"]["status"], "rotated")
            check("auto-return: next_item sees the returned item",
                  (Q.next_item() or {}).get("func") == "f_A")
            # 3. legacy statuses normalise to rotated
            seed(qp, [dict(base, func="f_C", status="foreclosed", foreclosure="old"),
                      dict(base, func="f_D", status="active")])
            Q.auto_return(rescan=False)
            it = {i["func"]: i for i in Q.load()["items"]}
            eq("legacy: foreclosed reads as rotated", it["f_C"]["status"], "rotated")
            eq("legacy: reason carried", it["f_C"]["rotation"], "old")
            # 4. sibling movement after rotation returns the item
            led = Path(td) / "memory" / "grind" / "f_C"
            led.mkdir(parents=True)
            (led / "state.json").write_text(json.dumps({
                "sibling_progress": [{"from": "f_X", "floor": 0,
                                      "at": "2026-12-31T00:00:00+00:00", "consumed": None}]}))
            r = Q.auto_return(rescan=False)
            eq("sibling: returned", [x["func"] for x in r["returned"]], ["f_C"])
            check("sibling: reason names the sibling",
                  "f_X" in Q.load()["items"][0].get("unpark_reason", "")
                  or any("f_X" in i.get("unpark_reason", "") for i in Q.load()["items"]))
            # 5. fingerprint: first run records it, second run sees no movement
            seed(qp, [dict(base, func="f_E", status="rotated", rotated_at="2026-01-01T00:00:00+00:00"),
                      dict(base, func="f_F", status="active")])
            r1 = Q.auto_return(rescan=True)
            r2 = Q.auto_return(rescan=True)
            eq("fingerprint: recorded", bool(Q.load().get("toolchain_fingerprint")), True)
            eq("fingerprint: unchanged -> no re-measure", r2["toolchain_moved"], False)
            eq("fingerprint: unchanged -> nothing returned", r2["returned"], [])
            # 6. unpark clears the rotation fields
            r = Q.mark_unparked("f_E", "owner: early return")
            eq("unpark: ok on rotated", r["ok"], True)
            it = {i["func"]: i for i in Q.load()["items"]}
            check("unpark: rotation field cleared", "rotation" not in it["f_E"])
            eq("status counts: rotated key", Q._counts(Q.load()["items"])["by_status"].get("rotated", 0), 0)
        finally:
            os.chdir(orig_cwd)
            Q.QUEUE_PATH = orig_path


def test_queue_remeasure_source_integrity() -> None:
    """2026-09-25 (queue commit c144a8556): the toolchain re-measure spliced
    candidates into src/ through tools/sweep_variants.py, whose single restore
    write DrvFS transiently refuses — candidates stayed spliced over
    INCLUDE_ASM and later items measured against that dirt as null floors.
    The re-measure now scores OUT OF TREE (sandbox candidate= copy) and runs
    under a src/+include/ snapshot guard. Pinned: src/ + include/ come back
    byte-identical on the normal path, when the scorer raises, and when an
    exception escapes auto_return mid-loop; the scorer is handed the ledger
    candidate (never a src/ splice); a scorer failure is a null floor WITH
    its reason; --force-rescan re-measures on an unchanged fingerprint."""
    import errno

    def seed(qp: Path, items, fp) -> None:
        qp.write_text(json.dumps({"items": items, "counts": {},
                                  "toolchain_fingerprint": fp}, indent=2) + "\n")

    def tree() -> dict:
        return {p.as_posix(): p.read_bytes() for d in ("src", "include")
                for p in sorted(Path(d).rglob("*")) if p.is_file()}

    with tempfile.TemporaryDirectory() as td:
        qp = Path(td) / "queue.json"
        orig_path, orig_cwd = Q.QUEUE_PATH, os.getcwd()
        orig_score = Q.sandbox.sandbox_score
        Q.QUEUE_PATH = str(qp)
        os.chdir(td)
        try:
            Path("src").mkdir()
            Path("include").mkdir()
            Path("src/a.c").write_bytes(b'#include "x.h"\nINCLUDE_ASM("asm/funcs", f_R);\n')
            Path("include/x.h").write_bytes(b"typedef int s32;\n")
            for f in ("f_R", "f_S"):
                led = Path("memory/grind") / f
                led.mkdir(parents=True)
                (led / "candidate.c").write_text(f"s32 {f}(void) {{ return 0; }}\n")
            before = tree()
            base = {"file": "a", "distance": 9, "verdict": "C", "rules": 0,
                    "status": "rotated", "rotated_at": "2026-01-01T00:00:00+00:00"}
            items = [dict(base, func="f_R"), dict(base, func="f_S"),
                     dict(base, func="f_N"),                       # no candidate.c
                     {"func": "f_A", "file": "a", "distance": 1, "verdict": "C",
                      "rules": 0, "status": "active"}]
            calls = []

            def dirty_scorer(func, disable="all", strip_cheat_asm=False, candidate=""):
                # Simulates a scorer that splices in place and never restores.
                calls.append((func, disable, strip_cheat_asm, candidate, tree() == before))
                Path("src/a.c").write_bytes(b"s32 f_R(void) { return 0; }\n")
                if func == "f_R":
                    # files CREATED during the re-measure must go too
                    Path("src/new.h").write_bytes(b"#define X 1\n")
                    Path("include/sub").mkdir(exist_ok=True)
                    Path("include/sub/new2.h").write_bytes(b"x\n")
                if func == "f_S":
                    Path("include/x.h").unlink()
                    raise RuntimeError("C build failed for a\nSTDERR:\nsrc/a.c:1: parse error")
                return {"score": 4, "scorable": True}

            # 1. normal + scorer-error paths, fingerprint moved
            seed(qp, items, "stale-fingerprint")
            Q.sandbox.sandbox_score = dirty_scorer
            r = Q.auto_return(rescan=True)
            eq("remeasure: src/ + include/ byte-identical after a dirtying scorer",
               tree(), before)
            eq("remeasure: restored + removed paths reported",
               sorted(set(r["sources_restored"])),
               ["include/sub/new2.h", "include/x.h", "src/a.c", "src/new.h"])
            check("remeasure: files created during a re-measure are removed",
                  not Path("src/new.h").exists() and not Path("include/sub/new2.h").exists())
            eq("remeasure: scorer handed the ledger candidate out of tree",
               [(c[0], c[1], c[2], Path(c[3]).as_posix()) for c in calls],
               [("f_R", "all", True, "memory/grind/f_R/candidate.c"),
                ("f_S", "all", True, "memory/grind/f_S/candidate.c")])
            rows = {x["func"]: x for x in r["remeasured"]}
            eq("remeasure: measured floor", rows["f_R"]["new"], 4)
            eq("remeasure: scorer failure is a null floor", rows["f_S"]["new"], None)
            check("remeasure: scorer failure carries its reason",
                  "parse error" in rows["f_S"].get("error", ""))
            eq("remeasure: missing candidate reason", rows["f_N"].get("error"), "no candidate.c")
            eq("remeasure: only the measured mover returns",
               [x["func"] for x in r["returned"]], ["f_R"])

            check("remeasure: each item scored against a clean tree (no cascade)",
                  all(c[4] for c in calls))

            # 2. an interrupt ESCAPING the scorer and auto_return mid-loop
            #    (BaseException: _remeasure_candidate's `except Exception`
            #    does not catch it) still restores
            class Abort(BaseException):
                pass

            def aborting_scorer(func, disable="all", strip_cheat_asm=False, candidate=""):
                Path("src/a.c").write_bytes(b"half-spliced")
                raise Abort()
            seed(qp, items, "stale-fingerprint")
            Q.sandbox.sandbox_score = aborting_scorer
            raised = False
            try:
                Q.auto_return(rescan=True)
            except Abort:
                raised = True
            check("remeasure: escaping interrupt propagates", raised)
            eq("remeasure: src/ + include/ byte-identical after an escaping interrupt",
               tree(), before)

            # 2b. a file the guard cannot READ back is rewritten, not given up on
            import pathlib
            real_read = pathlib.Path.read_bytes
            state = {"armed": False}

            def flaky_read(self):
                if state["armed"] and self.as_posix() == "src/a.c":
                    state["armed"] = False
                    raise OSError(errno.EINVAL, "Invalid argument")
                return real_read(self)

            def unreadable_scorer(func, disable="all", strip_cheat_asm=False, candidate=""):
                Path("src/a.c").write_bytes(b"spliced")
                state["armed"] = True          # the guard's comparison read fails once
                return {"score": 4, "scorable": True}
            seed(qp, items, "stale-fingerprint")
            Q.sandbox.sandbox_score = unreadable_scorer
            pathlib.Path.read_bytes = flaky_read
            try:
                r = Q.auto_return(rescan=True)
            finally:
                pathlib.Path.read_bytes = real_read
            check("remeasure: unreadable file is rewritten and reported",
                  "src/a.c" in r["sources_restored"])
            eq("remeasure: src/ + include/ byte-identical after an unreadable read-back",
               tree(), before)
            Q.sandbox.sandbox_score = dirty_scorer

            # 3. unchanged fingerprint: no re-measure unless forced
            seed(qp, items, Q.toolchain_fingerprint())
            calls.clear()
            r = Q.auto_return(rescan=True)
            eq("remeasure: unchanged fingerprint re-measures nothing", calls, [])
            r = Q.auto_return(rescan=True, force_rescan=True)
            eq("remeasure: --force-rescan re-measures", [c[0] for c in calls], ["f_R", "f_S"])
            check("remeasure: forced return names itself",
                  "forced re-measure" in r["returned"][0]["reason"])
            eq("remeasure: src/ + include/ byte-identical after a forced re-measure",
               tree(), before)

            # 3b. no state.json floor_history (a manual-lane ledger): the
            #     queue's recorded distance is the old floor, so an unchanged
            #     floor does not return the item
            seed(qp, [dict(base, func="f_R", distance=4), items[3]], Q.toolchain_fingerprint())
            calls.clear()
            r = Q.auto_return(rescan=True, force_rescan=True)
            eq("remeasure: no ledger floor, unchanged distance -> stays rotated",
               r["returned"], [])
            eq("remeasure: no ledger floor falls back to queue distance",
               r["remeasured"][0]["old"], 4)

            # 4. the restore write rides out DrvFS's transient EINVAL
            class Flaky:
                def __init__(self, fails):
                    self.fails, self.data = fails, None
                def write_bytes(self, data):
                    if self.fails:
                        self.fails -= 1
                        raise OSError(errno.EINVAL, "Invalid argument")
                    self.data = data
            fl = Flaky(2)
            Q._write_bytes_retry(fl, b"x", delay=0)
            eq("remeasure: restore retries transient EINVAL", fl.data, b"x")
            raised = False
            try:
                Q._write_bytes_retry(Flaky(99), b"x", attempts=3, delay=0)
            except OSError:
                raised = True
            check("remeasure: a persistent write failure is loud", raised)
        finally:
            Q.sandbox.sandbox_score = orig_score
            os.chdir(orig_cwd)
            Q.QUEUE_PATH = orig_path


def test_maspsx_indexed_operand_not_gp() -> None:
    """Owner ruling Q65 (2026-09-30), step 1: maspsx's load-delay helper `_uses_gp`
    treated an INDEXED operand `sym($reg)` of a gp-eligible symbol as gp-relative, so
    `lbu $2,..; sb $2,sym($3)` got a spurious nop (the expansion itself goes through
    $at). Sony ASPSX 2.34 emits no nop there; a direct `sb $2,sym` stays gp with the nop.
    Byte-neutral under the sdata lists (verify-oracle); load-bearing under Q65's
    per-file model (func_8003047C)."""
    import sys
    sys.path.insert(0, str(Path("tools/maspsx").resolve()))
    try:
        from maspsx import MaspsxProcessor
    finally:
        sys.path.pop(0)

    def run(store):
        mp = MaspsxProcessor([".ent\tf", "lbu\t$2,0($4)", store, ".end\tf"], sdata_sym_list=["sym"])
        return [l.split("#")[0].strip() for l in mp.process_lines()
                if l.split("#")[0].strip() and not l.lstrip().startswith(("#", "."))]

    eq("maspsx: indexed store of a gp-eligible symbol takes no load-delay nop",
       "nop" in run("sb\t$2,sym($3)"), False)
    eq("maspsx: direct store stays gp-relative with its nop",
       run("sb\t$2,sym"), ["lbu\t$2,0($4)", "nop", "sb\t$2,%gp_rel(sym)($gp)"])


def test_maspsx_static_lcomm() -> None:
    """Owner ruling Q65: an uninitialized static (`.local`+`.comm`, cc1psx `.lcomm`) is file-own small data
    under -G8, gp at every offset, never COMMON; the Q62-era fail-closed error is gone."""
    import sys
    sys.path.insert(0, str(Path("tools/maspsx").resolve()))
    try:
        from maspsx import MaspsxProcessor
    finally:
        sys.path.pop(0)
    lines = [".text", ".ent\tf", "lb\t$4,st", "lb\t$5,st+1", ".end\tf", ".local\tst", ".comm\tst,4,4"]
    res = [l.split("#")[0].strip() for l in MaspsxProcessor(lines, sdata_limit=8).process_lines()]
    eq("maspsx -G8: a static is gp at base and offset",
       [l for l in res if l.startswith("lb")], ["lb\t$4,%gp_rel(st)($gp)", "lb\t$5,%gp_rel(st+1)($gp)"])
    # owner ruling Q79 (A8): Sony ASPSX + PSYLINK place every `.lcomm` static 4-aligned (lcomm_align_probe)
    lines2 = [".text", ".ent\tf", "lb\t$4,a1", "lw\t$5,b8", ".end\tf",
              ".local\ta1", ".comm\ta1,1,1", ".local\tb8", ".comm\tb8,8,4"]
    res2 = [l.split("#")[0].strip() for l in MaspsxProcessor(lines2, sdata_limit=8).process_lines()]
    eq("maspsx: a 1-byte and an 8-byte static are each 4-aligned (Sony probe)",
       [res2[res2.index("a1:") - 1], res2[res2.index("b8:") - 1]], [".align 2", ".align 2"])


def test_maspsx_small_data_sdata() -> None:
    """Owner rulings Q65/Q68 (amendment A3): under -G8 a <= 8-byte initialized object (global or static) our cc1
    put in `.data` moves to `.sdata` as a unit (its .align/.type/.size/label/data; cc1psx -G8's choice) and is gp;
    two 3-byte arrays keep their own alignment; without -G nothing moves (byte-neutral today). Input: our cc1's
    real output shape."""
    import sys
    sys.path.insert(0, str(Path("tools/maspsx").resolve()))
    try:
        from maspsx import MaspsxProcessor
    finally:
        sys.path.pop(0)
    sd = [".data", ".align\t2", ".type\t a,@object", ".size\t a,3", "a:", ".byte\t1", ".byte\t2", ".byte\t3",
          ".align\t2", ".type\t b,@object", ".size\t b,3", "b:", ".byte\t4", ".byte\t5", ".byte\t6",
          ".align\t2", ".type\t s8,@object", ".size\t s8,8", "s8:", '.string\t"abcdefg"',
          ".text", ".ent\tf", "lbu\t$4,b+1", "lbu\t$5,s8+3", ".end\tf"]

    def run(g):
        return [l.split("#")[0].strip() for l in MaspsxProcessor(sd, sdata_limit=g).process_lines() if l.split("#")[0].strip()]
    r8 = run(8)
    kb = r8.index("b:")
    check("maspsx -G8: static 3-byte arrays and an 8-byte .string move to .sdata as units and are gp",
          ".section .sdata" in r8 and r8[kb - 3] == ".align\t2" and "lbu\t$4,%gp_rel(b+1)($gp)" in r8
          and "lbu\t$5,%gp_rel(s8+3)($gp)" in r8)
    check("maspsx without -G: no .sdata move, no gp",
          ".section .sdata" not in run(0) and "lbu\t$4,%gp_rel(b+1)($gp)" not in run(0))


def test_psyq_library_files() -> None:
    """Owner ruling Q69: maspsx runs -G8 for every file except Sony PsyQ library code (compiled -G0). The
    Makefile and buildconfig carry the same set; with a linked build it equals the evidence-derived set
    (tools/psyq_library_files.py: the census library span over the link map), with no gp access inside it."""
    import importlib.util
    spec = importlib.util.spec_from_file_location("plf", "tools/psyq_library_files.py")
    plf = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(plf)
    mk, bc = plf.declared(".")
    check("PSYQ_LIBRARY_FILES: Makefile == engine/buildconfig.py", mk == bc and len(mk) > 0)
    if Path("build/bb2.map").exists():
        lib, gp = plf.library_files(".")
        check("PSYQ_LIBRARY_FILES == the census library span over the link map", lib == mk)
        check("no gp-relative access in Sony library code", not gp)
    # owner ruling Q122: data-only files whose small objects the original keeps in .data skip maspsx -G8
    dmk, dbc = plf.declared_data_only(".")
    check("DATA_ONLY_NO_SDATA_FILES: Makefile == engine/buildconfig.py", dmk is not None and dmk == dbc)
    check("DATA_ONLY_NO_SDATA_FILES is disjoint from PSYQ_LIBRARY_FILES", not set(dmk or []) & set(mk))
    if Path("build/bb2.map").exists() and plf.build_is_oracle("."):
        check("DATA_ONLY_NO_SDATA_FILES == the data-only <= 8-byte .data files over the link map",
              plf.data_only_files(".") == dmk)
    for stem in dmk or []:
        cmd = P.c_pipeline_cmd(stem, "o/d.o")
        check(f"Q122: {stem} builds without maspsx -G8", not cmd.split("|")[3].rstrip().endswith("-G8"))


def test_data_only_files_fixture() -> None:
    """Q122: derive the exclusions without a real build or a cross-toolchain."""
    import importlib.util
    from subprocess import CompletedProcess
    from unittest.mock import patch

    spec = importlib.util.spec_from_file_location("plf_fixture", "tools/psyq_library_files.py")
    plf = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(plf)
    small = "00000000 g     O .data 00000004 small\n"
    # (input sections, objdump -t output, expected inclusion)
    cases = {
        "main/at_lower_bound": ([('.data', 0x800A3000, 4)], small, True),
        "main/at_upper_bound": ([('.data', 0x800A30D4, 8)],
                                "00000000 l     O .data 00000008 small\n", True),
        "main/one_byte": ([('.data', 0x800A3004, 1), ('.text', 0, 0)],
                          "00000000 g     O .data 00000001 small\n", True),
        "main/code": ([('.data', 0x800A3010, 4), ('.text', 0x80010000, 4)], small, False),
        "main/rodata_only": ([('.rodata', 0x80090000, 4)], small, False),
        "main/above_sdata": ([('.data', 0x800A30E0, 4)], small, False),
        "main/starts_at_sdata": ([('.data', 0x800A30DC, 4)], small, False),
        "main/crosses_sdata": ([('.data', 0x800A30D8, 8)], small, False),
        "main/zero_address": ([('.data', 0, 4)], small, False),
        "main/empty_data": ([('.data', 0x800A3020, 0)], small, False),
        "main/no_data": ([('.text', 0, 0)], small, False),
        "main/function_symbol": ([('.data', 0x800A3020, 4)],
                                 small + "00000000 g     F .text 00000004 function\n", False),
        "main/zero_size": ([('.data', 0x800A3024, 4)],
                           "00000000 g     O .data 00000000 empty\n", False),
        "main/large_object": ([('.data', 0x800A3028, 9)],
                              "00000000 g     O .data 00000009 large\n", False),
        "main/wrong_section": ([('.data', 0x800A3034, 4)],
                               "00000000 g     O .rodata 00000004 constant\n", False),
        # six fields, so it passes the length guard and only the "O" flag rejects it
        "main/not_object": ([('.data', 0x800A3038, 4)],
                            "00000000 l     d .data 00000004 label\n", False),
        "main/no_symbols": ([('.data', 0x800A303C, 4)], "", False),
        "main/malformed_symbols": ([('.data', 0x800A3040, 4)],
                                   "header g O .data 00000004 ignored\n00000000 g O\n", False),
    }
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        (root / "build").mkdir()
        # The original .data range starts at the lowest non-empty .data of any object
        # (here an asm one); only address 0 falls below it.
        lines = [" .data 0x800a3000 0x4 build/asm/data/start.o\n"]
        symbols = {}
        for tu, (sections, stdout, _) in cases.items():
            lines.extend(f" {section} 0x{address:08x} 0x{size:x} build/src/{tu}.o\n"
                         for section, address, size in sections)
            symbols[root / f"build/src/{tu}.o"] = stdout
        (root / "build/bb2.map").write_text("".join(lines), encoding="utf-8")

        def objdump(cmd, **kwargs):
            eq("Q122 fixture: only objdump's symbol table is requested",
               cmd[:2], ["mipsel-linux-gnu-objdump", "-t"])
            eq("Q122 fixture: objdump captures text", kwargs,
               {"capture_output": True, "text": True})
            # The keyed lookup also rejects any request outside the fixture root.
            return CompletedProcess(cmd, 0, stdout=symbols[Path(cmd[2])])

        with patch.object(plf.subprocess, "run", side_effect=objdump):
            got = plf.data_only_files(root)
        for tu, (_, _, included) in cases.items():
            eq(f"Q122 fixture: {tu}", tu in got, included)
        eq("Q122 fixture: exact sorted TU set", got,
           sorted(tu for tu, (_, _, included) in cases.items() if included))


def test_tus() -> None:
    """Restructure step 0 (Q106 D1/D8): TU ids are paths under src/ without .c;
    the linked set comes from bb2.ld; tus.check is the bb2.ld consistency audit."""
    import re
    import subprocess
    from engine import tus, fixtures

    # the live tree: consistent, and every flag list of the Makefile mirrors buildconfig
    eq("tus: the live bb2.ld / flag lists are consistent", tus.check(), [])
    mk = Path("Makefile").read_text()
    for name in tus.FLAG_LISTS:
        m = re.search(r"^" + name + r" :=(.*)$", mk, re.M)
        eq(f"tus: Makefile {name} == engine/buildconfig.py", sorted(m.group(1).split()) if m else None,
           sorted(getattr(cfg, name)))

    # a nested id builds from its nested path with its own flags; a same-basename
    # sibling in another directory gets none of them
    saved = (cfg.GP_FILES, cfg.PSYQ_LIBRARY_FILES)
    try:
        cfg.GP_FILES, cfg.PSYQ_LIBRARY_FILES = {"main/libgpu/sys"}, {"main/libcd/sys"}
        gpu = P.c_pipeline_cmd("main/libgpu/sys", "o/a.o")
        cd = P.c_pipeline_cmd("main/libcd/sys", "o/b.o")
        check("tus: a nested id compiles src/<id>.c", " src/main/libgpu/sys.c " in gpu)
        check("tus: GP_FILES applies by id, not basename", "-G8 " in gpu.split("|")[1]
              and "-G8 " not in cd.split("|")[1])
        check("tus: PSYQ_LIBRARY_FILES applies by id, not basename",
              cd.split("|")[3].rstrip().endswith("--use-comm-section")
              and gpu.split("|")[3].rstrip().endswith("-G8"))
    finally:
        cfg.GP_FILES, cfg.PSYQ_LIBRARY_FILES = saved

    # hygiene-w: the faithful build gates warnings per TU like make; a sandbox
    # (override) build never does
    saved_nw = cfg.NOWARN_FILES
    try:
        cfg.NOWARN_FILES = {"main/libcd/sys"}
        cc1 = lambda stem, ov=None: P.c_pipeline_cmd(stem, "o/a.o", ov).split("|")[1]
        check("tus: a gated TU's faithful build gets WARN_FLAGS",
              cc1("main/libgpu/sys").rstrip().endswith(cfg.WARN_FLAGS) and " -w" not in cc1("main/libgpu/sys"))
        check("tus: a NOWARN_FILES TU keeps -w", cc1("main/libcd/sys").rstrip().endswith(" -w"))
        check("tus: an override (sandbox) build always gets -w",
              cc1("main/libgpu/sys", {"src_override": "tmp/x.c"}).rstrip().endswith(" -w"))
    finally:
        cfg.NOWARN_FILES = saved_nw

    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td:
        os.chdir(td)
        try:
            for rel in ("src/a.c", "src/main/libgpu/sys.c", "src/main/libcd/sys.c", "src/z.c"):
                Path(rel).parent.mkdir(parents=True, exist_ok=True)
                Path(rel).write_text("int x;\n")
            eq("tus: src_tus is recursive, ids keep their directories", tus.src_tus(),
               ["a", "main/libcd/sys", "main/libgpu/sys", "z"])
            lines = lambda sec, ids: "".join(f"        build/src/{i}.o({sec});\n" for i in ids)
            good = (lines(".rodata", ["a", "main/libcd/sys", "z"])
                    + lines(".text", ["a", "main/libgpu/sys", "main/libcd/sys", "z"])
                    + lines(".data", ["main/libgpu/sys", "z"]))
            nolists = {n: set() for n in tus.FLAG_LISTS}
            eq("tus: a consistent layout passes", tus.check(text=good, lists=nolists), [])
            eq("tus: linked_tus keeps first-appearance order", tus.linked_tus(text=good),
               ["a", "main/libcd/sys", "z", "main/libgpu/sys"])
            probs = tus.check(text=good.replace("build/src/z.o(.text)", "build/src/y.o(.text)"),
                              lists=nolists)
            check("tus: a linked TU with no source is caught",
                  any("build/src/y.o but src/y.c does not exist" in p for p in probs))
            check("tus: a source bb2.ld does not link is caught",
                  any("src/z.c is not linked" in p for p in tus.check(
                      text=good.replace("build/src/z.o", "build/src/a.o").replace(
                          lines(".text", ["a"]), ""), lists=nolists)))
            swapped = good.replace(lines(".rodata", ["a", "main/libcd/sys", "z"]),
                                   lines(".rodata", ["main/libcd/sys", "a", "z"]))
            check("tus: one object order across sections",
                  any("order differs" in p and ".rodata: main/libcd/sys before a" in p
                      for p in tus.check(text=swapped, lists=nolists)))
            check("tus: a TU twice in one section is caught",
                  any("more than once" in p for p in tus.check(
                      text=good + lines(".data", ["z"]), lists=nolists)))
            check("tus: an unparseable build/src line is caught",
                  any("not of the form" in p for p in tus.check(
                      text=good + "        KEEP(build/src/a.o(.text))\n", lists=nolists)))
            eq("tus: a flag-list entry that is no TU is caught",
               tus.flag_list_problems(lists={"GP_FILES": {"main/libgpu/sys", "sys"}}),
               ["engine/buildconfig.py GP_FILES names 'sys', which is not a TU (no src/sys.c)"])

            # _file_index reads only LINKED objects: a stale build/src object that
            # sorts last can no longer shadow the live owner (plan R5)
            Path("bb2.ld").write_text(good)
            for i in ("a", "main/libgpu/sys", "main/libcd/sys", "z", "zz_stale"):
                Path(f"build/src/{i}.o").parent.mkdir(parents=True, exist_ok=True)
                Path(f"build/src/{i}.o").write_text("")
            owner = {"build/src/a.o": "f_a", "build/src/main/libgpu/sys.o": "f_gpu",
                     "build/src/main/libcd/sys.o": "f_cd", "build/src/z.o": "f_z",
                     "build/src/zz_stale.o": "f_a"}
            real_run = fixtures.subprocess.run

            def fake_nm(cmd, **kw):
                return subprocess.CompletedProcess(cmd, 0, f"00000000 T {owner[cmd[-1]]}\n", "")
            fixtures.subprocess.run = fake_nm
            try:
                idx = fixtures._file_index()
            finally:
                fixtures.subprocess.run = real_run
            eq("tus: _file_index ignores a stale unlinked object", idx.get("f_a"), "a")
            eq("tus: _file_index maps same-basename TUs to distinct ids",
               (idx.get("f_gpu"), idx.get("f_cd")), ("main/libgpu/sys", "main/libcd/sys"))

            import argparse
            eq("tus: arg_id accepts src/<id>.c", tus.arg_id("src/main/libcd/sys.c"), "main/libcd/sys")
            try:
                tus.arg_id("sys")
                check("tus: arg_id refuses a bare basename", False)
            except argparse.ArgumentTypeError as e:
                check("tus: arg_id refuses a bare basename and names the candidates",
                      "main/libcd/sys" in str(e) and "main/libgpu/sys" in str(e))
            Path("tools").mkdir()
            Path("tools/tu_renames.tsv").write_text("# header\nold\tmid\tafter:1\nmid\tmain/new\tafter:2\n")
            eq("tus: resolve follows the rename chain", tus.resolve("old"), "main/new")
        finally:
            os.chdir(cwd)

    # psyq_library_files parses nested ids from both declarations
    import importlib.util
    spec = importlib.util.spec_from_file_location("plf", "tools/psyq_library_files.py")
    plf = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(plf)
    with tempfile.TemporaryDirectory() as td:
        Path(td, "engine").mkdir()
        Path(td, "Makefile").write_text("PSYQ_LIBRARY_FILES := main/psxsdk/libcomb/comb gpu\n")
        Path(td, "engine/buildconfig.py").write_text(
            'PSYQ_LIBRARY_FILES = {"gpu", "main/psxsdk/libcomb/comb"}\n')
        mk_ids, bc_ids = plf.declared(td)
        eq("psyq_library_files: nested ids parse from Makefile and buildconfig",
           (mk_ids, bc_ids), (["gpu", "main/psxsdk/libcomb/comb"], ["gpu", "main/psxsdk/libcomb/comb"]))


def test_naming_member_apply() -> None:
    """tools/naming_member_apply.py on a scratch tree: a member named in a
    comment of the struct body is not a declarator; two bodies or no declarator
    still fail closed (exit 2); --dry-run reports the planned edits and writes
    nothing (the compiler is stubbed by a member-error oracle)."""
    import importlib.util
    import re
    spec = importlib.util.spec_from_file_location("nma", "tools/naming_member_apply.py")
    nma = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(nma)
    hdr = ("typedef struct {\n"
           "    s32 unk_198[4]; /* the counts; unk_198[i] is read by f() */\n"
           "    /* unk_198[i] again, with a } brace and a } Rec; */\n"
           "    s32 other; // unk_198: also in a line comment\n"
           "} Rec;\n")
    src = "s32 f(Rec *r) {\n    return r->unk_198[0];\n}\n"

    def exit_code(fn, *a):
        try:
            with contextlib.redirect_stdout(io.StringIO()), \
                    contextlib.redirect_stderr(io.StringIO()):
                fn(*a)
        except SystemExit as e:
            return e.code
        return 0

    with tempfile.TemporaryDirectory() as td:
        def put(rel, text):
            Path(td, rel).parent.mkdir(parents=True, exist_ok=True)
            Path(td, rel).write_bytes(text.encode())

        nma.ROOT = nma.WORK = td
        put("include/t.h", hdr)
        eq("naming_member_apply: a comment's unk_198[i] is not a second declarator",
           exit_code(nma.rename_declaration, "Rec", "unk_198", "counts", ["include/t.h"]), 0)
        eq("naming_member_apply: only the declarator is renamed, the comments are kept",
           Path(td, "include/t.h").read_text(), hdr.replace("s32 unk_198[4];", "s32 counts[4];", 1))
        put("include/t.h", hdr)
        put("include/u.h", hdr)
        eq("naming_member_apply: two `} Rec;` bodies still exit 2",
           exit_code(nma.rename_declaration, "Rec", "unk_198", "counts",
                     ["include/t.h", "include/u.h"]), 2)
        eq("naming_member_apply: a name only in comments is no declarator (exit 2)",
           exit_code(nma.rename_declaration, "Rec", "missing", "x", ["include/t.h"]), 2)

        # T.u.m: the member of the inline anonymous aggregate declared as member u
        edge = ("typedef struct {\n    s32 dist;\n    union {\n        s32 pair; /* a << 16 | b */\n"
                "        struct {\n            u16 b;\n            s16 a;\n        } node;\n"
                "    } ends; /* ends.node: the two points */\n    s32 node_count;\n} Edge;\n")
        put("include/e.h", edge)
        eq("naming_member_apply: nested T.u.m pair parses",
           nma.MEMBER_PAIR.match("Edge.ends.node=pt").groups(), ("Edge.ends", "node", "pt"))
        eq("naming_member_apply: nested member renamed in its union",
           exit_code(nma.rename_declaration, "Edge.ends", "node", "pt", ["include/e.h"]), 0)
        eq("naming_member_apply: only the nested declarator changes",
           Path(td, "include/e.h").read_text(), edge.replace("} node;", "} pt;"))
        put("include/e.h", edge)
        eq("naming_member_apply: a flat T.m does not reach the nested member (exit 2)",
           exit_code(nma.rename_declaration, "Edge", "node", "pt", ["include/e.h"]), 2)
        eq("naming_member_apply: a path naming no inline aggregate exits 2",
           exit_code(nma.rename_declaration, "Edge.missing", "node", "pt", ["include/e.h"]), 2)
        put("include/e.h", edge.replace("union {", "union U {"))
        eq("naming_member_apply: a tagged path step is not followed (exit 2)",
           exit_code(nma.rename_declaration, "Edge.ends", "node", "pt", ["include/e.h"]), 2)
        Path(td, "include/e.h").unlink()

        # --dry-run: the whole run on a scratch copy, nothing written to the tree
        put("src/a.c", src)
        Path(td, "pairs.txt").write_text("Rec.unk_198=counts\n")
        seen = []

        def fake_errors(mv, srcs):
            seen.append(nma.WORK)
            if "counts[4]" not in nma.rd("include/t.h"):
                return []
            return [("src/a.c", i, "unk_198")
                    for i, line in enumerate(nma.rd("src/a.c").split("\n"), 1)
                    if "->unk_198" in line]

        nma.tracked = lambda *roots: ["include/t.h", "src/a.c"]
        nma.make_vars = lambda: {}
        nma.compile_errors = fake_errors
        before = {f: Path(td, f).read_bytes() for f in ("include/t.h", "src/a.c")}
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nma.main(["--dry-run", str(Path(td, "pairs.txt"))])
        text = out.getvalue()
        eq("naming_member_apply --dry-run: exit 0", rc, 0)
        eq("naming_member_apply --dry-run: the tree is untouched",
           {f: Path(td, f).read_bytes() for f in before}, before)
        check("naming_member_apply --dry-run: compiled in a scratch copy, not the tree",
              bool(seen) and all(w != td for w in seen))
        check("naming_member_apply --dry-run: the scratch copy is removed",
              not any(os.path.exists(w) for w in seen))
        eq("naming_member_apply --dry-run: WORK is the tree again", nma.WORK, td)
        check("naming_member_apply --dry-run: the plan shows the declaration and the access",
              "+    s32 counts[4];" in text and "+    return r->counts[0];" in text
              and "DRY RUN: 2 file(s) would change; nothing written" in text)

        # nested pair, a line that also reaches another type's .m: the revert path
        put("include/e.h", "typedef struct { s32 d; union { s32 pair; struct { u16 b; s16 a; } node; } ends; } Edge;\n"
                           "struct N { s32 node; };\n")
        mixed = "s32 g(Edge *e, struct N *n) {\n    return e->ends.node.a + n->node;\n}\n"
        put("src/b.c", mixed)
        Path(td, "pairs2.txt").write_text("Edge.ends.node=pt\n")

        def mixed_errors(mv, srcs):
            errs = []
            for i, line in enumerate(nma.rd("src/b.c").split("\n"), 1):
                if "ends.node" in line:
                    errs.append(("src/b.c", i, "node"))
                elif "n->pt" in line:
                    errs.append(("src/b.c", i, "pt"))
            return errs
        nma.tracked = lambda *roots: ["include/e.h", "src/b.c"]
        nma.compile_errors = mixed_errors
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nma.main([str(Path(td, "pairs2.txt"))])
        text = out.getvalue()
        eq("naming_member_apply: a line reaching T.u.m and another type's .m is unresolved (exit 1)", rc, 1)
        eq("naming_member_apply: that line is restored for a hand edit", Path(td, "src/b.c").read_text(), mixed)
        check("naming_member_apply: the nested declaration is renamed and the line reported",
              "} pt; } ends;" in Path(td, "include/e.h").read_text()
              and "src/b.c:2: also reaches `node` through another type" in text)
        # bit-field and function-pointer declarators (nested)
        for decl, new_decl in [("unsigned node : 4;", "unsigned pt : 4;"),
                               ("s32 (*node)(s32);", "s32 (*pt)(s32);"),
                               ("s32 ( * node ) (s32);", "s32 ( * pt ) (s32);")]:
            put("include/e.h", "typedef struct { s32 d; union { s32 pair; " + decl + " } ends; } Edge;\n")
            eq("naming_member_apply: nested declarator " + decl,
               exit_code(nma.rename_declaration, "Edge.ends", "node", "pt", ["include/e.h"]), 0)
            check("naming_member_apply: renamed " + decl, new_decl in Path(td, "include/e.h").read_text())
        put("include/e.h", "typedef struct { s32 d; } Edge;\n")
        err = io.StringIO()
        with contextlib.redirect_stderr(err), contextlib.redirect_stdout(io.StringIO()):
            try:
                nma.rename_declaration("Edge.ends", "node", "pt", ["include/e.h"])
            except SystemExit:
                pass
        check("naming_member_apply: the failure message keeps the nested path",
              "Edge.ends.node" in err.getvalue())
        put("include/f.h", "typedef struct { s32 pad; u8 col[4]; } FaceTri;\n"
                           "typedef struct { s32 pad; u8 col[4]; s16 uv; } FaceQuad;\n")
        put("src/f.c", "s32 f(FaceTri *t, FaceQuad *q) {\n    return t->col[0] +\n        q->col[1];\n}\n")
        Path(td, "pairs5.txt").write_text("FaceTri.col=vtx_ids\nFaceQuad.col=vtx_ids\n")

        def face_errors(mv, srcs):
            h = nma.rd("include/f.h")
            members = {"t": "vtx_ids" if "col[4]; } FaceTri" not in h else "col",
                       "q": "vtx_ids" if "col[4]; s16 uv; } FaceQuad" not in h else "col"}
            return sorted(("src/f.c", i, field)
                          for i, line in enumerate(nma.rd("src/f.c").split("\n"), 1)
                          for var, field in re.findall(r"(\w)->(\w+)", line) if field != members[var])
        nma.tracked = lambda *roots: ["include/f.h", "src/f.c"]
        nma.compile_errors = face_errors
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nma.main([str(Path(td, "pairs5.txt"))])
        eq("naming_member_apply: two declarations sharing n in one pass (exit 0)", rc, 0)
        check("naming_member_apply: one pass renames both declarations and every access",
              out.getvalue().count("PASS ") == 1
              and Path(td, "include/f.h").read_text().count("vtx_ids[4]") == 2
              and Path(td, "src/f.c").read_text().count("->vtx_ids[") == 2)
        put("include/f.h", "typedef struct { s32 col; } A;\ntypedef struct { s32 idx; } B;\nstruct N { s32 col; };\n")
        put("src/f.c", "s32 f(A *a, struct N *n) {\n    return a->col + n->col;\n}\n")
        Path(td, "pairs6.txt").write_text("A.col=v\nB.idx=v\n")

        def reach_errors(mv, srcs):
            line = nma.rd("src/f.c").split("\n")[1]
            if "s32 v; } A" in nma.rd("include/f.h") and "a->col" in line:
                return [("src/f.c", 2, "col")]
            return [("src/f.c", 2, "v")] if "n->v" in line else []
        nma.compile_errors = reach_errors
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nma.main([str(Path(td, "pairs6.txt"))])
        check("naming_member_apply: the report names every old name the shared n replaces",
              rc == 1 and "also reaches `col` / `idx` through another type" in out.getvalue())
        eq("naming_member_apply: pairs sharing m and n run in one pass, a different n in another",
           nma.split_passes([("A", "col", "vtx"), ("B", "col", "vtx"), ("C", "col", "rgb"), ("D", "x", "y")]),
           [[("A", "col", "vtx"), ("B", "col", "vtx"), ("D", "x", "y")], [("C", "col", "rgb")]])

        # cc1 reports a multi-line macro invocation's error on its first line: the
        # access on a following line of the invocation is renamed; when that edit
        # then reaches another type's member, the whole span is restored.
        put("include/t.h", "typedef struct { s32 pos; } Rec;\nstruct N { s32 pos; };\n")
        sq = ("s32 f(Rec *r, struct N *n) {\n    if (SQ(1 -\n           r->pos) +\n"
              "        SQ(2 -\n           r->pos) > 0) {\n        return 1;\n    }\n    return 0;\n}\n")
        put("src/c.c", sq)
        Path(td, "pairs3.txt").write_text("Rec.pos=world_pos\n")
        first_lines = [i for i, line in enumerate(sq.split("\n"), 1) if "SQ(" in line]

        def macro_errors(mv, srcs):
            L = nma.rd("src/c.c").split("\n")
            if "world_pos;" not in nma.rd("include/t.h"):
                return []
            errs = []
            for i in first_lines:
                tail = L[i - 1] + L[i]
                if "->pos" in tail:
                    errs.append(("src/c.c", i, "pos"))
                elif "n->world_pos" in tail:
                    errs.append(("src/c.c", i, "world_pos"))
            return errs
        nma.tracked = lambda *roots: ["include/t.h", "src/c.c"]
        nma.compile_errors = macro_errors
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nma.main([str(Path(td, "pairs3.txt"))])
        eq("naming_member_apply: accesses on an invocation's later lines renamed (exit 0)", rc, 0)
        eq("naming_member_apply: both invocations' accesses renamed",
           Path(td, "src/c.c").read_text(), sq.replace("r->pos", "r->world_pos"))
        mixed_sq = sq.replace("SQ(2 -\n           r->pos)", "SQ(2 -\n           n->pos)")
        put("include/t.h", "typedef struct { s32 pos; } Rec;\nstruct N { s32 pos; };\n")
        put("src/c.c", mixed_sq)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nma.main([str(Path(td, "pairs3.txt"))])
        eq("naming_member_apply: an invocation span reaching another type's .m is unresolved (exit 1)", rc, 1)
        eq("naming_member_apply: only that span is restored",
           Path(td, "src/c.c").read_text(),
           mixed_sq.replace("SQ(1 -\n           r->pos)", "SQ(1 -\n           r->world_pos)"))
        check("naming_member_apply: the invocation's first line is reported",
              "src/c.c:" + str(first_lines[1]) + ": also reaches `pos` through another type" in out.getvalue())

        # A small cc1 model: an access's member error is reported on the first line of
        # the outermost macro invocation (SQ, VEL, AT) holding it, else on its own
        # line; r and g are Rec *, n is struct N *; VEL(x) is x->vel, AT(x) is g x.
        rec_hdr = "typedef struct { s32 pos; s32 vel; } Rec;\nstruct N { s32 pos; s32 vel; };\n"

        def model_errors(mv, srcs):
            rec = set(re.findall(r"s32 (\w+);", nma.rd("include/t.h").split("} Rec;")[0]))
            types = {"r": rec, "g": rec, "n": {"pos", "vel"}}
            text = nma.rd("src/c.c")
            masked = "\n".join("" if line.lstrip().startswith("#") else line
                               for line in nma.mask_comments(text).split("\n"))
            calls = []
            for m in re.finditer(r"\b(SQ|VEL|AT)\(", masked):
                depth, i = 0, m.end() - 1
                while True:
                    depth += {"(": 1, ")": -1}.get(masked[i], 0)
                    if depth == 0:
                        break
                    i += 1
                calls.append((m.start(), i, m[1]))
            accesses = [(m.start(), m[1], m[2]) for m in re.finditer(r"\b(\w+)\s*->\s*(\w+)", masked)]
            for start, end, macro in calls:
                inner = masked[start:end]
                if macro == "VEL":
                    accesses.append((start, re.match(r"VEL\(\s*(\w+)", inner)[1], "vel"))
                if macro == "AT":
                    accesses.append((start, "g", re.match(r"AT\(\s*->\s*(\w+)", inner)[1]))
            errs = set()
            for pos, var, field in accesses:
                if var in types and field not in types[var]:
                    outer = [c for c in calls if c[0] <= pos <= c[1]]
                    at = min(c[0] for c in outer) if outer else pos
                    errs.add(("src/c.c", masked.count("\n", 0, at) + 1, field))
            return sorted(errs)
        nma.tracked = lambda *roots: ["include/t.h", "src/c.c"]
        nma.compile_errors = model_errors
        Path(td, "pairs4.txt").write_text("Rec.pos=world_pos\nRec.vel=speed\n")

        def apply_model(body):
            put("include/t.h", rec_hdr)
            put("src/c.c", "#define VEL(x) x->vel\n#define AT(x) g x\nextern Rec *g;\n"
                           "s32 f(Rec *r, struct N *n) {\n" + body + "    return 0;\n}\n")
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                rc = nma.main([str(Path(td, "pairs4.txt"))])
            return rc, Path(td, "src/c.c").read_text().split("s32 f(Rec *r, struct N *n) {\n")[1][:-len("    return 0;\n}\n")], out.getvalue()
        # 1: an error on a line an owner's span already covers belongs to that owner
        body = "    s32 a = SQ(1 -\n        r->pos) + VEL(r);\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: an unsettled line inside an owner's span restores the owner (exit 1)", rc, 1)
        eq("naming_member_apply: the owner's lines are restored", got, body)
        check("naming_member_apply: the owner is reported", "src/c.c:5: no `.vel` / `->vel` token" in text)
        # 2: restore by line, to the line before this tool first edited it
        body = "    s32 a = SQ(1 -\n        r->pos) + r->vel + n->vel;\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: a line edited by two errors and then reaching another type (exit 1)", rc, 1)
        eq("naming_member_apply: it is restored to its true original", got, body)
        check("naming_member_apply: an error filtered on a frozen line is reported with its owner (" + text + ")",
              "also `vel` at :6" in text and "src/c.c:6:" not in text)
        # a line ending above its lowest depth leaves a parenthesis open
        body = "    s32 a = (1 +\n        2) + SQ(3 -\n        r->pos);\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: a span opens on `2) + SQ(3 -` (net 0, lowest -1)",
           (rc, got), (0, body.replace("r->pos", "r->world_pos")))
        # 3: parentheses in comments and literals are not counted
        body = "    s32 a = SQ(1 - /* ) */\n        r->pos) + f(\")\", ')');\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: comment / literal parentheses do not cut the span (exit 0)", rc, 0)
        eq("naming_member_apply: the access after a commented ')' is renamed", got, body.replace("r->pos", "r->world_pos"))
        # two invocations sharing a line, the edit on it
        body = "    s32 a = SQ(r->pos) + SQ(r->pos);\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: two invocations on one line both renamed", (rc, got), (0, body.replace("pos", "world_pos")))
        body = "    s32 a = SQ(r->pos) + SQ(n->pos);\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: two invocations on one line, one reaching another type: restored",
           (rc, got), (1, body))
        # #if inside an invocation's arguments
        body = "    s32 a = SQ(1 -\n#if 1\n        r->pos\n#else\n        r->pos + 1\n#endif\n        );\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: #if inside an invocation's arguments: the span runs to its ')'",
           (rc, got), (0, body.replace("r->pos", "r->world_pos")))
        # the receiver from the macro body: AT(->m) is spelled ->m in the argument
        body = "    s32 b = AT(->pos);\n"
        rc, got, text = apply_model(body)
        eq("naming_member_apply: a receiver from the macro body (AT(->m)) renamed",
           (rc, got), (0, body.replace("pos", "world_pos")))
        raw = 'a("/*", \'(\') /* ( */ b( // (\n"x\\"(" c'
        masked = nma.mask_comments(raw)
        eq("naming_member_apply: mask_comments blanks comments and literals, keeps offsets",
           (len(masked), masked.split(), masked.index("\n")), (len(raw), ["a(", ",", ")", "b(", "c"], raw.index("\n")))


def test_definition_type_line() -> None:
    """Fast equivalence sample; BB2_EXHAUSTIVE=1 checks all short strings."""
    import itertools
    import random
    import re
    import time

    old = re.compile(r"(?:[A-Za-z_]\w*|[ \t*])+")
    exhaustive = os.environ.get("BB2_EXHAUSTIVE") == "1"
    cases = ("".join(chars) for length in range(8 if exhaustive else 4)
             for chars in itertools.product("a1_ *;(", repeat=length))
    if not exhaustive:
        rng = random.Random(4027)
        sample = ("".join(rng.choices("a1_ *;(", k=rng.randrange(4, 17)))
                  for _ in range(4096))
        cases = itertools.chain(cases, sample)
    mismatch = next((text for text in cases
                     if bool(old.fullmatch(text)) != inlineasm._definition_type_line(text)), None)
    eq("type line: old/new equivalence (" +
       ("exhaustive through length 7" if exhaustive else "short exhaustive + fixed sample") + ")",
       mismatch, None)
    for text in ("", "1", "a1", "_1", "*1", "a 1", "a*", "a;", "a(",
                 "\t*", "a\u00e91", "\u00e9", "a\u0661", "\u0661", "a\n", "a\r"):
        eq("type line: boundary equivalence " + repr(text),
           inlineasm._definition_type_line(text), bool(old.fullmatch(text)))
    text = "a" * 5000 + ";"
    start = time.perf_counter()
    valid = inlineasm._definition_type_line(text)
    elapsed = time.perf_counter() - start
    check("type line: long invalid statement rejected", not valid)
    check("type line: 5000-character rejection under 50 ms", elapsed < 0.05)


def test_definition_names() -> None:
    text = ("extern int declared(void);\n"
            "int first(void) { return 1;\n"
            "}int joined(void) { return 2; }\n"
            "/*\nint copied(void) { return 0; }\n*/\n"
            "int copied(void) { return 3; }\n"
            "void old_style(arg) int arg; { }\n")
    eq("definition names: ordered, unique, definitions only",
       inlineasm.definition_names(text), ["first", "joined", "copied", "old_style"])
    eq("definition names: ambiguous name remains visible",
       len(inlineasm.func_definition_candidates(text, "copied")), 2)
    eq("definition names: empty", inlineasm.definition_names(""), [])


def test_tool_definition_heads() -> None:
    from unittest.mock import patch
    import gte_macro_rewrite as rewrite
    from engine import completion
    unique = "}int func_HEAD(void) { return 1; }\n"
    ambiguous = "/*\nint func_HEAD(void) { return 0; }\n*/\n" + unique
    paths = [rewrite.ROOT / "src/a.c", rewrite.ROOT / "src/z.c"]
    for texts, queued in [(ts, q) for ts in ([ambiguous, unique], [unique, ambiguous])
                          for q in (False, True)]:
        mapping = dict(zip(paths, texts))
        with patch.object(Path, "is_file", return_value=False), \
             patch.object(Path, "rglob", return_value=paths), \
             patch.object(Path, "read_text", autospec=True,
                          side_effect=lambda p, **kw: json.dumps({"items": [{"func": "func_HEAD", "file": "z"}] if queued else []})
                          if p.name == "queue.json" else mapping[p]):
            try:
                rewrite._functions("func_HEAD")
            except SystemExit as exc:
                check("GTE lookup refuses ambiguity in either TU order", "ambiguous C definition" in str(exc))
            else:
                check("GTE lookup must refuse ambiguity", False)
    for text, want_error in [(unique, False), (ambiguous, True)]:
        with patch.object(Path, "is_file", return_value=True), \
             patch.object(Path, "read_text", return_value=text), \
             patch.object(completion, "own_blocks", return_value=[object()]):
            try:
                result = rewrite._functions("a")
            except SystemExit as exc:
                check("GTE TU census refuses ambiguity", want_error and "ambiguous" in str(exc))
            else:
                check("GTE TU census accepts brace-prefixed definition", not want_error)
                eq("GTE TU census names", result, [("func_HEAD", "a")])


def test_queue_ambiguous_definition_message() -> None:
    from unittest.mock import patch
    source = "/*\nint func_HEAD(void) { }\n*/\nint func_HEAD(void) { }\n"
    with patch.object(Q, "load", return_value={"items": [{"func": "func_HEAD", "file": "fake"}]}), \
         patch.object(inlineasm, "live_tu_refusal", return_value=None), \
         patch.object(Q, "_rule_count", return_value=0), \
         patch.object(cheats, "func_prologue_count", return_value=0), \
         patch.object(cheats, "maspsx_gate_entries", return_value=[]), \
         patch.object(cheats, "canonical_asm_funcs", return_value=set()), \
         patch.object(inlineasm, "file_func_cheat_asm_count", return_value=-1), \
         patch.object(Path, "read_text", return_value=source):
        result = Q.mark_done("func_HEAD")
    check("queue refusal names ambiguity", not result["ok"] and
          "ambiguous C definition: 2 definition-shaped heads" in result["reason"])


def test_ambiguous_tool_diagnostics() -> None:
    import move_tu as mt
    import migrate_include_asm as migrate
    import resplit_convert as resplit
    import gte_macro_rewrite as rewrite
    from engine import tus
    from unittest.mock import patch

    source = ('s32 func_MOVE(void) { return 1; }\n#if 0\n'
              's32 func_MOVE(void) { return 2; }\n#endif\n')
    root = Path("tmp/codex/mock-move")
    files = {"bb2.ld": "build/src/old.o(.text);\n", "Makefile": "",
             "engine/buildconfig.py": "", "src/old.c": source,
             "tools/canonical_asm_regions.json": json.dumps({"functions": {
                 "func_MOVE": {"file": "old", "sha256": []}}})}
    with patch.object(Path, "is_file", return_value=True), \
         patch.object(Path, "exists", autospec=True,
                      side_effect=lambda p: p.name == "canonical_asm_regions.json"), \
         patch.object(Path, "read_text", autospec=True,
                      side_effect=lambda p, **kw: files[p.relative_to(root).as_posix()]), \
         patch.object(mt, "_buildconfig_lists", return_value={}), \
         patch.object(tus, "check", return_value=[]), \
         patch.object(tus, "ld_sections", return_value={"text": ["old"]}):
        try:
            mt.plan(root, "old", "new")
        except mt.MoveError as exc:
            reason, cause = str(exc), exc.__cause__
        else:
            reason, cause = "", None
    check("move_tu: ambiguous granted function raises MoveError with context",
          "func_MOVE in src/old.c" in reason and "ambiguous C definition: 2" in reason)
    check("move_tu: grant error preserves the region-hash ValueError cause", isinstance(cause, ValueError))

    for desc, text, want in (("ambiguous", source, "ambiguous C definition"),
                             ("absent", "int unrelated;\n", "no C")):
        with patch.object(migrate, "rd", side_effect=lambda p: text if p.startswith("src/") else ""), \
             patch.object(migrate.os.path, "isfile", return_value=True):
            status, details = migrate.migrate_one("func_MOVE", "fake", dry=True)
        check(f"migration diagnostic: {desc} reason", want in status)
        eq(f"migration diagnostic: {desc} has no applied edits", details, [])
        try:
            resplit.convert_source(text, "func_MOVE")
        except SystemExit as exc:
            reason = str(exc)
        else:
            reason = ""
        check(f"resplit diagnostic: {desc} reason", want in reason)
        check(f"macro rewrite diagnostic: {desc} reason",
              want in rewrite.rewrite_function(text, "func_MOVE").error)

    with patch.object(Path, "is_file", return_value=False), \
         patch.object(Path, "read_text", autospec=True, side_effect=lambda p, **kw: '{"items": []}'
                      if p.name == "queue.json" else source), \
         patch.object(Path, "rglob", return_value=[rewrite.ROOT / "src/fake.c"]):
        try:
            rewrite._functions("func_MOVE")
        except SystemExit as exc:
            reason = str(exc)
        else:
            reason = ""
    check("macro rewrite diagnostic: ambiguous lookup names the TU",
          "ambiguous C definition in src/fake.c" in reason)


def test_move_tu() -> None:
    """tools/move_tu.py on a scratch git repo: dry run writes nothing, a dirty
    tree refuses, --apply moves every keyed surface, and moving back restores
    every file byte-for-byte (the rename map keeps both rows)."""
    import importlib.util
    import shutil
    import subprocess
    spec = importlib.util.spec_from_file_location("move_tu", "tools/move_tu.py")
    mt = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mt)
    repo_root = Path.cwd()

    def git(*a):
        return subprocess.run(["git", "-c", "user.email=t@e", "-c", "user.name=t",
                               "-c", "core.hooksPath=/dev/null", *a], cwd=td,
                              check=True, capture_output=True, text=True).stdout

    with tempfile.TemporaryDirectory() as td:
        R = Path(td)
        files = {
            "src/comb.c": "int comb;\n",
            "src/gpu.c": "int gpu;\n",
            "bb2.ld": ("SECTIONS {\n        build/src/gpu.o(.rodata);\n        build/src/comb.o(.rodata);\n"
                       "        build/src/gpu.o(.text);\n        build/src/comb.o(.text);\n"
                       "        build/src/comb.o(.bss);\n}\n"),
            "Makefile": ("GP_FILES := gpu\nPSYQ_LIBRARY_FILES := comb gpu\nDATA_ONLY_NO_SDATA_FILES :=\n"
                         "EXPAND_LH_FILES :=\nNO_SR_FILES :=\nNOWARN_FILES :=\n"),
            "engine/buildconfig.py": ('GP_FILES = {"gpu"}\nEXPAND_LH_FILES = set()\n'
                                      'PSYQ_LIBRARY_FILES = {"comb", "gpu"}\nDATA_ONLY_NO_SDATA_FILES = set()\n'
                                      'NO_SR_FILES = set()\n'
                                      'NOWARN_FILES = set()\n'),
            "oracle/manifest.json": json.dumps({"corpus": {"src/comb.c": "h1", "src/gpu.c": "h2"},
                                                "golden_fixtures": [{"name": "f", "file": "src/comb.c"}]},
                                               indent=2) + "\n",
            "tools/cc1_tu_expectation.txt": "# src-digest x\n" + "a" * 40 + "  comb\n" + "b" * 40 + "  gpu\n",
            "tools/canonical_asm_regions.json": json.dumps({"schema": 2, "functions": {}}, indent=2) + "\n",
            "engine/queue.json": json.dumps({"items": [{"func": "f", "file": "comb"}]}, indent=2) + "\n",
            ".claude/rules/r.md": '---\nname: r\npaths: ["src/comb.c", "src/**/*.c"]\n---\nbody src/comb.c\n',
            "tools/other.sh": "case $stem in comb) ;; esac\n",
            "tools/gpu_refs.sh": "gpu\n" * 45,
            "tools/lanes/lanes.json": (
                '{\n  "regenerate": [\n'
                '    {"path": "docs/naming/runtime/evidence/comb.tsv",\n'
                '     "cmd": "python3 notes.py build --tu comb --verbose",\n'
                '     "subject": "docs: notes (comb)",\n'
                '     "inputs": ["src/comb.c", "src/comb_extra.c"]},\n'
                '    {"path": "docs/naming/runtime/evidence/comb_extra.tsv",\n'
                '     "cmd": "python3 notes.py build --tu=comb_extra",\n'
                '     "subject": "docs: notes (comb_extra)",\n'
                '     "inputs": ["src/comb.c", "bb2.ld"]},\n'
                '    {"path": "unrelated.tsv", "cmd": "python3 notes.py build --tu=comb",\n'
                '     "subject": "comb", "inputs": ["src/gpu.c"]}\n'
                '  ],\n  "other": {"path": "docs/naming/runtime/evidence/comb.tsv",\n'
                '    "cmd": "python3 notes.py build --tu comb",\n'
                '    "subject": "docs: notes (comb)", "inputs": ["src/comb.c"]}\n}\n'),
            "docs/naming/runtime/evidence/comb.tsv": "generated note\n",
            "docs/naming/runtime/evidence/gpu.tsv": "orphaned generated note\n",
        }
        quoted_commands = [f"python3 notes.py build --tu{sep}{quote}{tu}{quote} --verbose"
                           for sep in (" ", "=") for quote in ("'", '"')
                           for tu in ("comb", "comb_extra")]
        quoted_entries = "".join(
            '    ' + json.dumps({"cmd": cmd}) + ',\n' for cmd in quoted_commands)
        files["tools/lanes/lanes.json"] = files["tools/lanes/lanes.json"].replace(
            '  "regenerate": [\n', '  "regenerate": [\n' + quoted_entries)
        for rel, t in files.items():
            Path(R, rel).parent.mkdir(parents=True, exist_ok=True)
            Path(R, rel).write_bytes(t.encode())
        git("init", "-q")
        git("config", "core.autocrlf", "false")
        git("add", "-A")
        git("commit", "-qm", "base")
        snapshot = {rel: Path(R, rel).read_bytes() for rel in files}
        new = "main/psxsdk/libcomb/comb"
        cwd = os.getcwd()
        os.chdir(repo_root)          # move_tu imports engine.* from the real tree
        try:
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                rc = mt.main(["comb", new, "--root", td])
            eq("move_tu: dry run exits 0", rc, 0)
            check("move_tu: dry run changes nothing",
                  all(Path(R, rel).read_bytes() == b for rel, b in snapshot.items())
                  and not Path(R, "tools/tu_renames.tsv").exists())
            check("move_tu: dry run lists a hand-review leftover", "tools/other.sh" in out.getvalue())
            old_note = "docs/naming/runtime/evidence/comb.tsv"
            new_note = f"docs/naming/runtime/evidence/{new.replace('/', '__')}.tsv"
            check("move_tu: dry run lists lanes edits and the generated note move",
                  "lanes.json regenerate: 1 path(s), 6 command(s), 1 subject(s), 2 input(s)"
                  in out.getvalue() and f"generated evidence: {old_note} -> {new_note}" in out.getvalue())
            check("move_tu: dry run reports references outside rewritten lane spans",
                  '"subject": "comb"' in out.getvalue()
                  and '"cmd": "python3 notes.py build --tu comb"' in out.getvalue())
            edits, moves, _, leftovers = mt.plan(R, "comb", new)
            lane_text = edits.get("tools/lanes/lanes.json", b"").decode()
            entries = json.loads(lane_text or '{"regenerate": []}')["regenerate"]
            eq("move_tu: quoted commands rewrite only whole ids",
               [entry["cmd"] for entry in entries[:len(quoted_commands)]],
               [cmd.replace("'comb'", f"'{new}'").replace('"comb"', f'"{new}"')
                for cmd in quoted_commands])
            entries = entries[len(quoted_commands):]
            eq("move_tu: lane evidence path encodes nested TU id",
               entries[0]["path"] if entries else None,
               f"docs/naming/runtime/evidence/{new.replace('/', '__')}.tsv")
            eq("move_tu: lane command rewrites exact --tu argument",
               entries[0]["cmd"] if entries else None,
               f"python3 notes.py build --tu {new} --verbose")
            eq("move_tu: lane command rewrites --tu=id at end of command",
               entries[2]["cmd"] if len(entries) > 2 else None,
               f"python3 notes.py build --tu={new}")
            eq("move_tu: lane subject rewrites parenthesized id",
               entries[0]["subject"] if entries else None, f"docs: notes ({new})")
            check("move_tu: lane inputs follow in every regenerate entry",
                  len(entries) == 3 and all(f"src/{new}.c" in e["inputs"] for e in entries[:2]))
            before, after = files["tools/lanes/lanes.json"].split('  "other":', 1)
            expected = (before.replace('evidence/comb.tsv',
                                      f"evidence/{new.replace('/', '__')}.tsv")
                        .replace('--tu comb ', f'--tu {new} ')
                        .replace('--tu=comb"', f'--tu={new}"')
                        .replace('(comb)', f'({new})')
                        .replace('"src/comb.c"', f'"src/{new}.c"') + '  "other":' + after)
            for cmd in quoted_commands:
                expected = expected.replace(
                    json.dumps(cmd), json.dumps(cmd.replace("'comb'", f"'{new}'")
                                               .replace('"comb"', f'"{new}"')))
            eq("move_tu: lane formatting and unrelated strings are byte-stable",
               lane_text, expected)
            eq("move_tu: generated evidence is a planned move", moves, {old_note: new_note})
            check("move_tu: generated evidence is not a leftover",
                  old_note in moves
                  and not any(line.startswith("generated evidence with no regenerate entry:")
                          for line in leftovers))
            check("move_tu: lanes leftovers include unchanged fields on rewritten lines",
                  any('"subject": "comb"' in line for line in leftovers))
            lane_path = Path(R, "tools/lanes/lanes.json")
            original_command = '"python3 notes.py build --tu comb --verbose"'
            for command in ("python3 notes.py build --tu 'comb\" --verbose",
                            "python3 notes.py build x--tu comb --verbose"):
                literal = json.dumps(command)
                variant = files["tools/lanes/lanes.json"].replace(original_command, literal)
                lane_path.write_bytes(variant.encode())
                variant_edits, _, _, _ = mt.plan(R, "comb", new)
                eq("move_tu: mismatched quotes and embedded flags stay unchanged: " + command,
                   variant_edits["tools/lanes/lanes.json"],
                   expected.replace(original_command.replace("--tu comb", f"--tu {new}"),
                                    literal).encode())
            literal = '"python3 notes.py build --tu comb --label é --path a\\/b"'
            variant = files["tools/lanes/lanes.json"].replace(original_command, literal)
            lane_path.write_bytes(variant.encode())
            variant_edits, _, _, _ = mt.plan(R, "comb", new)
            eq("move_tu: cmd preserves non-ASCII and escaped slash bytes",
               variant_edits["tools/lanes/lanes.json"],
               expected.replace(original_command.replace("--tu comb", f"--tu {new}"),
                                literal.replace("--tu comb", f"--tu {new}")).encode())
            lane_path.write_bytes(files["tools/lanes/lanes.json"].replace("\n", "\r\n").encode())
            variant_edits, _, _, _ = mt.plan(R, "comb", new)
            eq("move_tu: CRLF lanes remain byte-stable outside replacements",
               variant_edits["tools/lanes/lanes.json"], expected.replace("\n", "\r\n").encode())
            for escaped in (r"\u0063omb", r"c\u006fmb"):
                variant = files["tools/lanes/lanes.json"].replace(
                    original_command, original_command.replace("comb", escaped))
                lane_path.write_bytes(variant.encode())
                try:
                    mt.plan(R, "comb", new)
                except mt.MoveError as exc:
                    check("move_tu: escaped TU argument refuses", "escaped --tu argument" in str(exc))
                else:
                    check("move_tu: escaped TU argument refuses", False)
            lane_path.write_bytes(snapshot["tools/lanes/lanes.json"])
            Path(R, new_note).write_bytes(b"existing target\n")
            try:
                mt.plan(R, "comb", new)
            except mt.MoveError as exc:
                eq("move_tu: an existing evidence target refuses", str(exc), f"{new_note} already exists")
            else:
                check("move_tu: an existing evidence target refuses", False)
            eq("move_tu: refusal preserves the evidence target", Path(R, new_note).read_bytes(), b"existing target\n")
            Path(R, new_note).unlink()
            Path(R, "src/gpu.c").write_text("int gpu2;\n")
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                eq("move_tu: a dirty tree refuses", mt.main(["comb", new, "--root", td, "--apply"]), 1)
            git("checkout", "--", "src/gpu.c")
            applied = io.StringIO()
            with contextlib.redirect_stdout(applied):
                eq("move_tu: apply exits 0", mt.main(["comb", new, "--root", td, "--apply"]), 0)
            check("move_tu: apply reports unchanged lane references",
                  '"subject": "comb"' in applied.getvalue()
                  and '"cmd": "python3 notes.py build --tu comb"' in applied.getvalue())
            check("move_tu: the source moved", Path(R, f"src/{new}.c").read_text() == "int comb;\n"
                  and not Path(R, "src/comb.c").exists())
            eq("move_tu: apply writes the planned lane configuration",
               Path(R, "tools/lanes/lanes.json").read_bytes(), expected.encode())
            check("move_tu: apply moves generated evidence byte-identical",
                  not Path(R, old_note).exists() and Path(R, new_note).is_file()
                  and Path(R, new_note).read_bytes() == snapshot[old_note])
            ld = Path(R, "bb2.ld").read_text()
            check("move_tu: every bb2.ld section line moved in place",
                  ld == files["bb2.ld"].replace("build/src/comb.o", f"build/src/{new}.o"))
            check("move_tu: the flag list follows the file",
                  f"PSYQ_LIBRARY_FILES := {new} gpu" in Path(R, "Makefile").read_text()
                  and f'"{new}"' in Path(R, "engine/buildconfig.py").read_text())
            man = json.loads(Path(R, "oracle/manifest.json").read_text())
            eq("move_tu: oracle corpus key + fixture file follow (hash kept)",
               (man["corpus"].get(f"src/{new}.c"), man["golden_fixtures"][0]["file"]),
               ("h1", f"src/{new}.c"))
            check("move_tu: cc1 expectation, queue item and rule paths follow",
                  f"  {new}\n" in Path(R, "tools/cc1_tu_expectation.txt").read_text()
                  and json.loads(Path(R, "engine/queue.json").read_text())["items"][0]["file"] == new
                  and f'"src/{new}.c"' in Path(R, ".claude/rules/r.md").read_text()
                  and "body src/comb.c" in Path(R, ".claude/rules/r.md").read_text())
            check("move_tu: the rename map gains a row",
                  f"comb\t{new}\tafter:" in Path(R, "tools/tu_renames.tsv").read_text())
            check("move_tu: everything is staged", git("status", "--porcelain").count("\n")
                  == len(git("diff", "--cached", "--name-only").splitlines()))
            git("commit", "-qm", "move")
            with contextlib.redirect_stdout(io.StringIO()):
                eq("move_tu: moving back exits 0", mt.main([new, "comb", "--root", td, "--apply"]), 0)
            for rel, b in snapshot.items():
                eq(f"move_tu: round trip restores {rel} byte-for-byte", Path(R, rel).read_bytes(), b)
            check("move_tu: the round trip removes the emptied directories",
                  not Path(R, "src/main").exists())
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                git("commit", "-qm", "back")
                eq("move_tu: an existing target refuses", mt.main(["comb", "gpu", "--root", td]), 1)
                eq("move_tu: a missing source refuses", mt.main(["nope", "x", "--root", td]), 1)
                eq("move_tu: a non-id target refuses", mt.main(["comb", "../x", "--root", td]), 1)
            orphan = "docs/naming/runtime/evidence/gpu.tsv"
            warning = f"generated evidence with no regenerate entry: {orphan}"
            _, _, _, gpu_leftovers = mt.plan(R, "gpu", "main/gpu")
            for mode in ([], ["--apply"]):
                out = io.StringIO()
                with contextlib.redirect_stdout(out):
                    eq(f"move_tu: orphan evidence rename {mode} exits 0",
                       mt.main(["gpu", "main/gpu", "--root", td, *mode]), 0)
                check(f"move_tu: orphan evidence is reported {mode}", warning in out.getvalue())
                printed = out.getvalue().split(
                    "  other tracked references to the old id (review by hand; not rewritten):\n", 1)[-1]
                eq(f"move_tu: orphan warning prints first {mode}",
                   printed.splitlines()[0], f"    {warning}")
                check(f"move_tu: truncated leftovers report omitted count {mode}",
                      len(gpu_leftovers) > 40
                      and f"    ... and {len(gpu_leftovers) - 40} more\n" in printed)
                eq(f"move_tu: orphan evidence stays byte-identical {mode}",
                   Path(R, orphan).read_bytes(), snapshot[orphan])
                check(f"move_tu: orphan evidence is not moved {mode}",
                      not Path(R, "docs/naming/runtime/evidence/main__gpu.tsv").exists())
        finally:
            os.chdir(cwd)
        shutil.rmtree(Path(R, ".git"), ignore_errors=True)


def test_maspsx_fingerprint() -> None:
    """2026-09-25: the oracle's `maspsx_rev` ran `git -C tools/maspsx rev-parse
    HEAD`, but tools/maspsx is vendored (no .git), so it read the PARENT repo's
    HEAD and every commit reported maspsx toolchain drift. The queue's
    toolchain fingerprint hashed only the maspsx.py wrapper, missing the
    `maspsx` package where the assembler logic lives. Pinned: both now hash the
    executed maspsx sources; a package edit moves both; tests/ does not; the
    oracle identity carries no git-derived maspsx key; a queue fingerprint
    stored under the old input set re-records without a spurious re-measure."""
    from engine import oracle as O
    with tempfile.TemporaryDirectory() as td:
        orig_path, orig_cwd = Q.QUEUE_PATH, os.getcwd()
        qp = Path(td) / "queue.json"
        Q.QUEUE_PATH = str(qp)
        os.chdir(td)
        try:
            pkg = Path("tools/maspsx/maspsx")
            pkg.mkdir(parents=True)
            Path("tools/maspsx/tests").mkdir()
            Path("tools/maspsx/maspsx.py").write_text("from maspsx import main\n")
            (pkg / "__init__.py").write_text("def main(): pass\n")
            Path("tools/maspsx/tests/test_x.py").write_text("assert 1\n")
            eq("maspsx: executed sources = entry + package",
               [p.as_posix() for p in O.maspsx_source_files()],
               ["tools/maspsx/maspsx.py", "tools/maspsx/maspsx/__init__.py"])
            h0, fp0 = O.maspsx_sources_sha1(), Q.toolchain_fingerprint()
            legacy0 = Q.toolchain_fingerprint(Q._LEGACY_FINGERPRINT_INPUTS)
            Path("tools/maspsx/tests/test_x.py").write_text("assert 2\n")
            eq("maspsx: tests/ edit is not toolchain drift", O.maspsx_sources_sha1(), h0)
            (pkg / "__init__.py").write_text("def main(): return 1\n")
            check("maspsx: package edit moves the oracle hash", O.maspsx_sources_sha1() != h0)
            check("maspsx: package edit moves the queue fingerprint",
                  Q.toolchain_fingerprint() != fp0)
            eq("maspsx: the old input set was blind to the package",
               Q.toolchain_fingerprint(Q._LEGACY_FINGERPRINT_INPUTS), legacy0)
            tc = O._toolchain_identity()
            check("maspsx: oracle identity has no git-derived maspsx_rev", "maspsx_rev" not in tc)
            eq("maspsx: oracle identity carries the source hash",
               tc.get("maspsx_sources_sha1"), O.maspsx_sources_sha1())
            # a queue that recorded the OLD-definition fingerprint re-records
            base = {"file": "a", "distance": 1, "verdict": "C", "rules": 0}
            qp.write_text(json.dumps({"items": [
                dict(base, func="f_R", status="rotated", rotated_at="2026-01-01T00:00:00+00:00"),
                dict(base, func="f_A", status="active")], "counts": {},
                "toolchain_fingerprint": Q.toolchain_fingerprint(Q._LEGACY_FINGERPRINT_INPUTS)}))
            r = Q.auto_return(rescan=True)
            eq("maspsx: definition change is not a toolchain move", r["toolchain_moved"], False)
            eq("maspsx: definition change flagged", r["fingerprint_migrated"], True)
            eq("maspsx: new fingerprint recorded", Q.load()["toolchain_fingerprint"],
               Q.toolchain_fingerprint())
            (pkg / "__init__.py").write_text("def main(): return 2\n")
            r = Q.auto_return(rescan=True)
            eq("maspsx: a later package edit IS a toolchain move", r["toolchain_moved"], True)
            eq("maspsx: ...and not a migration", r["fingerprint_migrated"], False)
        finally:
            os.chdir(orig_cwd)
            Q.QUEUE_PATH = orig_path


def test_prologue_config_fingerprint() -> None:
    """2026-09-25 (layer-2 note on 5ab08e1ce): prologue_fix reads
    tools/prologue_config.json, delay_slot_ra_funcs.txt and frame_fix_funcs.txt
    in every faithful build, and the oracle manifest watches the first, but
    none of the three fed the queue's toolchain fingerprint. Pinned: an edit to
    each moves it; a fingerprint stored under either retired input set
    re-records without a re-measure; a later config edit IS a move."""
    with tempfile.TemporaryDirectory() as td:
        orig_path, orig_cwd = Q.QUEUE_PATH, os.getcwd()
        qp = Path(td) / "queue.json"
        Q.QUEUE_PATH = str(qp)
        os.chdir(td)
        try:
            Path("tools").mkdir()
            for rel in cheats.PROLOGUE_CONFIGS:
                Path(rel).write_text("{}\n" if rel.endswith(".json") else "# none\n")
            check("prologue: all three configs are fingerprint inputs",
                  all(rel in Q.toolchain_fingerprint_inputs() for rel in cheats.PROLOGUE_CONFIGS))
            for rel in cheats.PROLOGUE_CONFIGS:
                before = Q.toolchain_fingerprint()
                Path(rel).write_text(Path(rel).read_text() + "# edit\n")
                check(f"prologue: editing {rel} moves the fingerprint",
                      Q.toolchain_fingerprint() != before)
            base = {"file": "a", "distance": 1, "verdict": "C", "rules": 0}
            items = [dict(base, func="f_R", status="rotated",
                          rotated_at="2026-01-01T00:00:00+00:00"),
                     dict(base, func="f_A", status="active")]
            for i, prior in enumerate(Q._prior_fingerprint_input_sets()):
                qp.write_text(json.dumps({"items": items, "counts": {},
                                          "toolchain_fingerprint": Q.toolchain_fingerprint(prior)}))
                r = Q.auto_return(rescan=True)
                eq(f"prologue: retired input set {i} is not a toolchain move",
                   r["toolchain_moved"], False)
                eq(f"prologue: retired input set {i} flagged as migration",
                   r["fingerprint_migrated"], True)
            Path(cheats.PROLOGUE_CONFIG).write_text('{"f": []}\n')
            r = Q.auto_return(rescan=True)
            eq("prologue: a later config edit IS a toolchain move", r["toolchain_moved"], True)
        finally:
            os.chdir(orig_cwd)
            Q.QUEUE_PATH = orig_path


def test_objdump_failure_is_loud() -> None:
    """2026-09-25: score._objdump returned stdout whatever objdump's exit
    status, so a transient failure read as an empty symbol table ("<func> not
    found") or an empty function. It now retries briefly and then RAISES on a
    non-zero exit or empty output — never a silent "absent"."""
    import subprocess as sp
    real_run, real_delay = score.subprocess.run, score._OBJDUMP_RETRY_DELAY
    calls = []

    def fake(results):
        seq = list(results)

        def run(argv, capture_output=True, text=True):
            calls.append(argv)
            rc, out, err = seq.pop(0) if len(seq) > 1 else seq[0]
            return sp.CompletedProcess(argv, rc, out, err)
        return run
    score._OBJDUMP_RETRY_DELAY = 0
    try:
        for label, results in (("non-zero exit", [(1, "", "bfd: file truncated")]),
                               ("empty stdout, exit 0", [(0, "  \n", "")])):
            calls.clear()
            score.subprocess.run = fake(results)
            raised = ""
            try:
                score._objdump("-t", "x.o")
            except RuntimeError as e:
                raised = str(e)
            check(f"objdump: {label} raises", bool(raised))
            eq(f"objdump: {label} retried before raising", len(calls), score._OBJDUMP_ATTEMPTS)
            if results[0][2]:
                check(f"objdump: {label} error carries stderr", results[0][2] in raised)
        calls.clear()
        score.subprocess.run = fake([(1, "", "busy"), (0, "x.o: file format elf32\n", "")])
        eq("objdump: transient failure recovers on retry",
           score._objdump("-t", "x.o"), "x.o: file format elf32\n")
        eq("objdump: one retry used", len(calls), 2)
        score.subprocess.run = fake([(1, "", "nope")])
        raised = False
        try:
            score.normalized_insns("missing.o", "f")
        except RuntimeError:
            raised = True
        except KeyError:
            raised = False
        check("objdump: a failed read is not reported as 'function not found'", raised)
    finally:
        score.subprocess.run, score._OBJDUMP_RETRY_DELAY = real_run, real_delay


def test_symtab_data_dlabels() -> None:
    """_symtab() resolves data symbols defined only as `dlabel`s in the data
    asm (2026-09-22): before, `D_800A13FC` vs `D_800A12FC+0x100` (one address)
    scored a false +2 per lui/load pair. Linker symbol files still win."""
    import os
    import tempfile
    from . import score
    saved_tab, saved_glob = score._SYMTAB_CACHE, score._DATA_ASM_GLOB
    saved_files = score.cfg.LD_SYM_FILES
    saved_reg = score.cfg.SYM_REGISTRY_FILES
    d = tempfile.mkdtemp()
    try:
        with open(os.path.join(d, "x.data.s"), "w") as fh:
            fh.write("nonmatching D_800A12FC\n"
                     "dlabel D_800A12FC\n"
                     "    /* 91AFC 800A12FC 00000000 */ .word 0x00000000\n"
                     "enddlabel D_800A12FC\n"
                     "dlabel CD_debug\n"
                     "    /* 919C0 800A11C0 */ .byte 0x00\n"
                     "dlabel Overridden\n"
                     "    /* 0 80000004 */ .byte 0x00\n"
                     "dlabel Judge\n"
                     "dlabel g_sin_table\n"
                     "    /* 0 800973FC */ .word 0x00000000\n"
                     "dlabel NoData\n"
                     "\n"
                     "enddlabel NoData\n"
                     "    /* 0 80000008 */ .byte 0x00\n")
        with open(os.path.join(d, "syms.txt"), "w") as fh:
            fh.write("Overridden = 0x80001000;\n")
        with open(os.path.join(d, "reg.txt"), "w") as fh:
            fh.write("Overridden = 0x80002000;\nCD_debug = 0x80002004;\n"
                     "g_obj_defined = 0x800A1300; /* note */\n")
        score._SYMTAB_CACHE = None
        score._DATA_ASM_GLOB = os.path.join(d, "*.s")
        score.cfg.LD_SYM_FILES = [os.path.join(d, "syms.txt")]
        score.cfg.SYM_REGISTRY_FILES = [os.path.join(d, "reg.txt")]
        t = score._symtab()
        eq("dlabel: splat-named data symbol resolves", t.get("D_800A12FC"), 0x800A12FC)
        eq("dlabel: Sony-named data symbol resolves", t.get("CD_debug"), 0x800A11C0)
        eq("dlabel: linker symbol file wins on conflict", t.get("Overridden"), 0x80001000)
        eq("dlabel: only the first data line after the label counts",
           t.get("NoData"), None)
        eq("dlabel: stacked labels share the next data line",
           (t.get("Judge"), t.get("g_sin_table")), (0x800973FC, 0x800973FC))
        eq("registry: an object-defined name with no pin resolves",
           t.get("g_obj_defined"), 0x800A1300)
        eq("registry: linker files and dlabels win over it",
           (t.get("Overridden"), t.get("CD_debug")), (0x80001000, 0x800A11C0))
    finally:
        score._SYMTAB_CACHE, score._DATA_ASM_GLOB = saved_tab, saved_glob
        score.cfg.LD_SYM_FILES = saved_files
        score.cfg.SYM_REGISTRY_FILES = saved_reg


def test_layer2_asm_addr_after_glabel() -> None:
    """A .s may carry its module's leading data words ahead of the glabel (InitGeom.s holds MSC00's
    D_8007E08C); the function's address is the first machine column after its glabel."""
    from engine import layer2
    text = ("dlabel D_8007E08C\n"
            "    /* 6E88C 8007E08C 50730915 */ .word 0x15097350\n"
            "    /* 6E890 8007E090 9C9F4000 */ .word 0x00409F9C\n"
            "enddlabel D_8007E08C\n"
            "glabel InitGeom\n"
            "    /* 6E894 8007E094 0A80013C */  lui        $at, %hi(D_8009C798)\n")
    eq("addresses: the first machine column after the glabel, not the leading data words",
       layer2._asm_addr(text, "InitGeom"), "8007E094")
    eq("addresses: a plain glabel file reads as before",
       layer2._asm_addr("glabel f\n    /* 1 80010000 00000000 */  nop\n", "f"), "80010000")
    eq("addresses: another function's file gives nothing", layer2._asm_addr(text, "D_8007E08C"), None)


@contextlib.contextmanager
def _data_wave_tree(files, rows):
    import sys
    from unittest.mock import patch
    import data_wave as dw
    (dw.ROOT / "tmp").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=dw.ROOT / "tmp") as td:
        root = Path(td)
        for rel, text in files.items():
            p = root / rel
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(text, encoding="utf-8", newline="\n")
        manifest = root / "rows.csv"
        manifest.write_text("addr,proposed_name,verdict,kind,current_names\n" + rows, encoding="utf-8")
        with patch.object(dw, "ROOT", root), patch.object(sys, "argv", [
                "data_wave", "--manifest-csv", str(manifest), "--apply"]), \
                patch.object(dw.subprocess, "run") as run:
            run.return_value.returncode = 0
            run.return_value.stdout = ""
            yield dw, root


def _data_wave_fixture_keycheck(files, rows):
    """Apply a wave and check its manifest with in-memory Git reads (no Git writes)."""
    import re
    import sys
    import naming_keycheck as nk
    from unittest.mock import patch
    with _data_wave_tree(files, rows) as (dw, root):
        manifest = root / "wave.json"
        sys.argv += ["--manifest", str(manifest)]
        with contextlib.redirect_stdout(io.StringIO()):
            eq("fixture wave applies", dw.main(), 0)
        after = {rel: (root / rel).read_text(encoding="utf-8") for rel in files}
        pairs = {old: op["new"] for op in json.loads(manifest.read_text())["ops"]
                 for old in op["olds"]}
        # Mirror the rebuilt map, which is ignored build output rather than a wave edit.
        if "build/bb2.map" in after:
            pat = re.compile(r"\b(" + "|".join(map(re.escape, pairs)) + r")\b")
            after["build/bb2.map"] = pat.sub(lambda m: pairs[m.group()], after["build/bb2.map"])

        def git_read(*args, **kwargs):
            if args[0] == "ls-tree":
                return "\n".join(files)
            if args[0] == "ls-files":
                return "\n".join(files)
            if args[0] == "grep":
                tree = files if "HEAD" in args else after
                pat = re.compile(r"\b(?:" + args[args.index("-E") + 1] + r")\b")
                return "\n".join(("HEAD:" if tree is files else "") + p
                                 for p, text in tree.items() if pat.search(text))
            raise AssertionError(args)

        output = io.StringIO()
        with patch.object(nk, "ROOT", root), \
                patch.object(nk, "show", lambda rev, p: (files if rev else after).get(p)), \
                patch.object(nk, "load_tree_texts", lambda rev, names=(): files if rev else after), \
                patch.object(nk, "git", git_read), patch.object(nk, "_is_macro", return_value=False), \
                patch.object(nk, "verify_working_storage", lambda *a: None), \
                patch.object(nk, "changed_files", return_value=[("M", p) for p in files
                             if files[p] != after[p] and not p.startswith("build/")]), \
                patch.object(sys, "argv", ["keycheck", "--from-manifests", str(manifest)]), \
                contextlib.redirect_stdout(output):
            result = nk.main()
        if result:
            print(output.getvalue())
        eq("fixture manifest accepted by keycheck", result, 0)


def test_data_wave_fixture_p3w6() -> None:
    offsets = ("3AC", "3B0", "3B4", "3B8", "3BC", "3C0")
    files = {"named_syms.txt": "", "symbol_addrs.txt": "", "undefined_syms_auto.txt": ""}
    rows = ""
    for offset in offsets:
        addr, old = "1F800" + offset, "fixture_scratchpad_ptr_" + offset
        for reg in files:
            files[reg] += f"{old} = 0x{addr};\n"
        rows += f"0x{addr},D_{addr},CONFIRM\n"
    _data_wave_fixture_keycheck(files, rows)


def test_data_wave_fixture_p3w17() -> None:
    files = {"named_syms.txt": "fixture_char_setup_tbl = 0x80011048;\n",
             "undefined_syms_auto.txt": "",
             "asm/data/7D920.data.s": "dlabel fixture_module_lamp\n/* 1 80011048 00000000 */ .word 0\n",
             "src/main/31D3C.c": "extern int fixture_module_lamp;\nint *p = &fixture_module_lamp;\n"}
    rows = "0x80011048,D_80011048,CONFIRM\n"
    for offset in ("3AC", "3B0", "3B4", "3B8", "3BC", "3C0", "3C4", "3C8"):
        addr = "1F800" + offset
        files["undefined_syms_auto.txt"] += f"fixture_spad_{offset} = 0x{addr};\n"
        rows += f"0x{addr},D_{addr},CONFIRM\n"
    _data_wave_fixture_keycheck(files, rows)


def test_data_wave_fixture_p3w14() -> None:
    files = {"named_syms.txt": "", "symbol_addrs.txt": "", "undefined_syms_auto.txt": "",
             "asm/data/module.s": ""}
    rows = ""
    for addr, old, new in (("80011050", "fixture_gpu_stat_reg", "fixture_gpu_port"),
                           ("80011054", "fixture_cd_reg0", "fixture_cd_port"),
                           ("80011058", "fixture_ings2_u16_buf_a1578", "fixture_interrupt_blob")):
        for reg in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt"):
            files[reg] += f"{old} = 0x{addr};\nD_{addr} = 0x{addr};\n"
        files["asm/data/module.s"] += f"dlabel D_{addr}\n/* 1 {addr} 00000000 */ .word 0\n"
        rows += f"0x{addr},{new},CONFIRM\n"
    _data_wave_fixture_keycheck(files, rows)


def test_data_wave_fixture_p3w9() -> None:
    files = {"named_syms.txt": "fixture_str_build_date = 0x80011100;\n",
             "symbol_addrs.txt": "fixture_str_build_date = 0x80011100;\n",
             "src/main/6CF8.c": 'const char fixture_str_build_date[28] = "Fri Aug  7 22:26:32 1998\\n";\n',
             "build/bb2.map": "    0x80011100 fixture_str_build_date\n",
             "asm/data/93950.data.s": ""}
    rows = "0x80011100,fixture_timestamp,CONFIRM\n"
    for addr, word in (("80011104", "network"), ("80011108", "team"), ("8001110C", "vs"),
                       ("80011110", "vscpu"), ("80011114", "story")):
        files["named_syms.txt"] += f"fixture_str_mode_{word}_{addr} = 0x{addr};\n"
        files["asm/data/93950.data.s"] += f"dlabel D_{addr}\n/* 1 {addr} 00000000 */ .word 0\n"
        rows += f"0x{addr},fixture_str_{word}_{addr},CONFIRM\n"
    _data_wave_fixture_keycheck(files, rows)
    # The fixture's INCLUDE_RODATA row intentionally refuses, as permitted by C9.
    test_data_wave_held_rename()


def test_keycheck_registry_address_proof() -> None:
    import naming_keycheck as nk
    old = "alias = 0x80010000;\n"
    note = "/* data-wave 2026-10-09: alias = 0x80010000 retired; target is now object-defined */\n"
    for files in ({}, {"named_syms.txt": "target = 0x80010004;\n"},
                  {"asm/data/a.s": "dlabel target\n/* 1 80010004 00000000 */ .word 0\n"},
                  {"src/main/a.c": "int target = 0;\n"},
                  {"src/main/a.c": "extern int target;\n", "build/bb2.map": "  0x80010000 target\n"},
                  {"src/main/a.c": "int target = 0;\n", "build/bb2.map": "  0x80010004 target\n"},
                  {"src/main/a.c": "int data[target];\n", "build/bb2.map": "  0x80010000 target\n"},
                  {"src/main/a.c": "#define target 1\nint data;\n", "build/bb2.map": "  0x80010000 target\n"},
                  {"named_syms.txt": "target = 0x80010000;\n",
                   "symbol_addrs.txt": "target = 0x80010004;\n"}):
        for output in ("", note):
            check("unknown/different address rejects registry retirement",
                  nk.registry_ok(old, output, "undefined_syms_auto.txt", {"alias": "target"},
                                 files.get, list(files)) is not None)
    for files in ({"named_syms.txt": "target = 0x80010000;\n"},
                  {"symbol_addrs.txt": "target = 0x80010000;\n"},
                  {"asm/data/a.s": "dlabel target\n/* 1 80010000 00000000 */ .word 0\n"},
                  {"asm/funcs/a.s": "glabel target\n/* 1 80010000 00000000 */ nop\n"},
                  {"src/main/a.c": "int target = 0;\n", "build/bb2.map": "  0x80010000 target\n"},
                  {"src/main/a.c": "int target = 0;\n",
                   "asm/data/a.s": "dlabel target\n/* 1 80010000 00000000 */ .word 0\n"}):
        for output in ("", note):
            eq("same address accepts registry retirement",
               nk.registry_ok(old, output, "undefined_syms_auto.txt", {"alias": "target"},
                              files.get, list(files)), None)
    files = {"src/main/a.c": "volatile int fixture_gpu_port = 0;\n",
             "build/bb2.map": "  0x80011050 fixture_gpu_port\n"}
    note = "/* data-wave 2026-10-09: fixture_gpu_port = 0x80011050 retired; fixture_gpu_port is now object-defined */\n"
    eq("non-auto destination's own object alias accepted",
       nk.registry_ok("fixture_gpu_port = 0x80011050;\n", note, "named_syms.txt",
                      {"old": "fixture_gpu_port"}, files.get, list(files)), None)


def test_data_wave_scratchpad() -> None:
    import data_wave as dw
    for addr in ("0x1F800000", "0x1F8003AC", "0x1F8003FF", "0x80000000", "0x8FFFFFFF"):
        check("data wave accepts " + addr, bool(dw.ADDR.fullmatch(addr)))
    for addr in ("0x1F7FFFFF", "0x1F800400", "0x90000000", "0x00000000"):
        check("data wave rejects " + addr, not dw.ADDR.fullmatch(addr))
    with _data_wave_tree({"undefined_syms_auto.txt": "fixture_spad = 0x1F8003AC;\n",
                          "src/main/a.c": "extern int fixture_spad;\n"},
                         "0x1F8003AC,D_1F8003AC,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            eq("scratchpad wave applies", dw.main(), 0)
        check("scratchpad auto name", "D_1F8003AC =" in (root / "undefined_syms_auto.txt").read_text())


def test_data_wave_dlabel_only() -> None:
    files = {"named_syms.txt": "fixture_setup = 0x80011048;\n",
             "asm/data/module.s": "dlabel fixture_earlier\n/* 1 80011040 00000000 */ .word 0\n"
                                  "dlabel fixture_module_lamp\n/* 5 80011048 00000000 */ .word 0\n",
             "src/main/a.c": "extern int fixture_module_lamp;\n"}
    with _data_wave_tree(files, "0x80011048,D_80011048,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            eq("dlabel-only wave applies", dw.main(), 0)
        check("all dlabel aliases retired", "dlabel D_80011048" in (root / "asm/data/module.s").read_text())
        check("dlabel reference retired", "fixture_module_lamp" not in (root / "src/main/a.c").read_text())
        import naming_keycheck as nk
        from unittest.mock import patch
        with patch.object(nk, "show", lambda rev, path: files.get(path)), \
                patch.object(nk, "load_tree_texts", return_value=files), \
                patch.object(nk, "git", return_value="asm/data/module.s\n"), \
                patch.object(nk, "_is_macro", return_value=False):
            nk._ADDRESS_INDEXES.clear()
            eq("keycheck finds module data label", nk.base_addr("HEAD", "fixture_module_lamp"), "80011048")


def test_data_wave_object_target() -> None:
    for definition in ({"src/main/a.c": "int D_80011044 = 0;\n"},
                       {"asm/data/a.s": "dlabel D_80011044\n/* 1 80011044 00000000 */ .word 0\n"}):
        files = {"named_syms.txt": "old = 0x80011044;\nD_80011044 = 0x80011044;\n", **definition}
        with _data_wave_tree(files, "0x80011044,D_80011044,CONFIRM\n") as (dw, root):
            with contextlib.redirect_stdout(io.StringIO()):
                eq("object target applies", dw.main(), 0)
            check("object target has no registry assignment",
                  not any(dw.SYM_LINE.match(line) for line in (root / "named_syms.txt").read_text().splitlines()))


def test_data_wave_cross_registry() -> None:
    files = {"undefined_syms_auto.txt": "fixture_poll = 0x80011118;\n",
             "named_syms.txt": "fixture_tick = 0x80011118;\nnew_counter = 0x80011118;\n"}
    with _data_wave_tree(files, "0x80011118,new_counter,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            eq("cross-registry wave applies", dw.main(), 0)
        lines = [line for rel in dw.REGISTRIES if (root / rel).exists()
                 for line in (root / rel).read_text().splitlines() if dw.SYM_LINE.match(line)]
        eq("one cross-registry definition", len(lines), 1)
    _data_wave_fixture_keycheck({"undefined_syms_auto.txt": "fixture_poll = 0x80011118;\n",
                                 "named_syms.txt": "fixture_tick = 0x80011118;\n"},
                                "0x80011118,new_counter,CONFIRM\n")


def test_data_wave_held_rename() -> None:
    files = {"named_syms.txt": "fixture_str_build_date = 0x8001111C;\n",
             "src/main/a.c": 'const char fixture_str_build_date[4] = "abc";\n',
             "asm/data/a.s": "dlabel D_80011120\n/* 1 80011120 00000000 */ .word 0\n"}
    with _data_wave_tree(files, "0x8001111C,build_date,CONFIRM\n0x80011120,data_word,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            eq("C-defined and auto-dlabel rename applies", dw.main(), 0)
        check("C definition renamed", "char build_date" in (root / "src/main/a.c").read_text())
        check("auto dlabel renamed", "dlabel data_word" in (root / "asm/data/a.s").read_text())
    files = {"named_syms.txt": "old = 0x8001111C;\n",
             "src/main/a.c": 'INCLUDE_RODATA("asm/rodata", old);\n',
             "asm/rodata/old.s": "dlabel old\n/* 1 8001111C 00000000 */ .word 0\n"}
    with _data_wave_tree(files, "0x8001111C,new,CONFIRM\n") as (dw, root):
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            try:
                dw.main()
            except SystemExit as e:
                eq("INCLUDE_RODATA fails closed", e.code, 2)
            else:
                check("INCLUDE_RODATA refuses", False)
        check("refusal names required file move", "asm/rodata/old.s" in err.getvalue())
        for rel, text in files.items():
            eq("refusal leaves " + rel + " untouched", (root / rel).read_text(), text)


def test_data_wave_standalone() -> None:
    import shutil
    import subprocess
    import sys
    import data_wave as dw
    scratch = Path(__file__).resolve().parent.parent / "tmp"
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch) as td:
        root = Path(td)
        (root / "tools").mkdir()
        (root / "engine").mkdir()
        for name in ("data_wave.py", "naming_wave.py", "classify_inline_asm.py"):
            shutil.copyfile(dw.ROOT / "tools" / name, root / "tools" / name)
        for name in ("__init__.py", "layer2.py", "inlineasm.py", "gtemacro.py"):
            shutil.copyfile(dw.ROOT / "engine" / name, root / "engine" / name)
        (root / "src").mkdir()
        (root / "src/a.c").write_text("int fixture_standalone = 1;\n", encoding="utf-8")
        (root / "named_syms.txt").write_text("fixture_standalone = 0x80011000;\n", encoding="utf-8")
        manifest = root / "rows.csv"
        manifest.write_text("addr,proposed_name\n0x80011000,fixture_standalone_new\n", encoding="utf-8")
        env = dict(os.environ)
        env.pop("PYTHONPATH", None)
        result = subprocess.run([sys.executable, "tools/data_wave.py", "--manifest-csv", str(manifest)],
                                cwd=root, env=env, capture_output=True, text=True)
        eq("standalone data_wave imports engine", result.returncode, 0)
        if result.returncode:
            print(result.stderr)


def test_data_wave_tentative_definitions() -> None:
    import data_wave as dw
    import naming_keycheck as nk
    eq("tentative definition is discarded COMMON", dw.c_data_names("int fixture_common;\n"), set())
    eq("disabled initialized definitions ignored", dw.c_data_names(
        "#if 0\nint fixture_disabled = 1;\n#endif\n"
        "#ifdef NON_MATCHING\nint fixture_nonmatching = 1;\n#endif\n"), set())
    files = {"named_syms.txt": "fixture_common_alias = 0x80011000;\n",
             "src/main/a.c": "int fixture_common_alias;\n",
             "build/bb2.map": "Discarded input sections\n  0x80011000 fixture_common_alias\n"
                               "Linker script and memory map\n"}
    _data_wave_fixture_keycheck(files, "0x80011000,fixture_common_new,CONFIRM\n")
    with _data_wave_tree(files, "0x80011000,fixture_common_new,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            dw.main()
        check("COMMON keeps its linker assignment", "fixture_common_new =" in (root / "named_syms.txt").read_text())
    proof = {"src/main/a.c": "int fixture_common_new;\n",
             "build/bb2.map": "  0x80011000 fixture_common_new\n"}
    check("COMMON plus map is not C object proof", nk.registry_ok(
        "fixture_common_alias = 0x80011000;\n", "", "named_syms.txt",
        {"fixture_common_alias": "fixture_common_new"}, proof.get, list(proof)) is not None)


def test_data_wave_curated_dedupe() -> None:
    files = {"named_syms.txt": "fixture_curated = 0x80011000; /* curated evidence */\n",
             "symbol_addrs.txt": "fixture_splat = 0x80011000; // splat evidence\n",
             "undefined_syms_auto.txt": "fixture_auto = 0x80011000;\n"}
    rows = "0x80011000,fixture_curated_new,CONFIRM\n"
    _data_wave_fixture_keycheck(files, rows)
    with _data_wave_tree(files, rows) as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            dw.main()
        for rel, note in (("named_syms.txt", "curated evidence"), ("symbol_addrs.txt", "splat evidence")):
            text = (root / rel).read_text()
            check("curated line and note retained in " + rel, "fixture_curated_new =" in text and note in text)
        check("auto duplicate removed", "=" not in (root / "undefined_syms_auto.txt").read_text())


def test_data_wave_fictional_symbols() -> None:
    import re
    import data_wave as dw
    text = Path(__file__).read_text(encoding="utf-8")
    start = text.index("def _data_wave_tree")
    text = text[start:text.index("\ndef main()", start)]
    check("fictional-symbol guard scans the actual test region", "def test_data_wave_fixture_p3w6" in text)
    live = {name for names in dw.registry_defs().values() for name in names}
    eq("C9 fixtures have no live named symbols", sorted(live & set(re.findall(r"\b[A-Za-z_]\w*\b", text))), [])


def test_keycheck_bitfield_member_declarators() -> None:
    import naming_keycheck as nk
    eq("bit-field declarators", nk.member_declarators(nk.ctokens("u32 unk14_0 : 4, grayscale : 1")),
       ["unk14_0", "grayscale"])
    eq("plain array declarator", nk.member_declarators(nk.ctokens("u8 picks[2]")), ["picks"])


def test_keycheck_index_once() -> None:
    import naming_keycheck as nk
    from unittest.mock import patch
    files = {"named_syms.txt": "fixture_index_a = 0x80011000;\nfixture_index_b = 0x80011004;\n",
             "src/main/a.c": "int fixture_index_a = 1;\nint fixture_index_b = 2;\n"}
    nk._ADDRESS_INDEXES.clear()
    with patch.object(nk, "load_tree_texts", return_value=files) as load:
        for _ in range(50):
            eq("indexed address a", nk.base_addr("fixture-rev", "fixture_index_a"), "80011000")
            eq("indexed address b", nk.base_addr("fixture-rev", "fixture_index_b"), "80011004")
        eq("each requested name indexed once", load.call_count, 2)
    reads = []
    def read_target(path):
        reads.append(path)
        return files.get(path)
    pairs = {f"fixture_index_old_{i}": "fixture_index_a" for i in range(50)}
    old = "".join(f"{name} = 0x80011000;\n" for name in pairs)
    eq("many retired aliases share one target index", nk.registry_ok(
        old, "", "undefined_syms_auto.txt", pairs, read_target, list(files)), None)
    eq("target surfaces read once per index", len(reads), len(set(reads)))
    nk._ADDRESS_INDEXES.clear()
    bodies = {"src/a.c": "int fixture_batch = 1;\n", "named_syms.txt": "fixture_batch = 0x80011000;\n"}
    nk._FILE_TEXTS.clear()
    nk._INDEX_QUERIES.clear()
    with patch.object(nk, "git", return_value="fixture-rev:src/a.c") as listing, \
            patch.object(nk.subprocess, "run") as run:
        def batch_read(*args, **kwargs):
            from types import SimpleNamespace
            output = b""
            for query in kwargs["input"].decode().splitlines():
                path = query.split(":", 1)[1]
                if path in bodies:
                    body = bodies[path].encode()
                    output += b"deadbeef blob " + str(len(body)).encode() + b"\n" + body + b"\n"
                else:
                    output += query.encode() + b" missing\n"
            return SimpleNamespace(stdout=output)
        run.side_effect = batch_read
        loaded = nk.load_tree_texts("fixture-rev", {"fixture_batch"})
        eq("revision gets one candidate search", listing.call_count, 1)
        eq("revision gets one batch read", run.call_count, 1)
        for path, body in bodies.items():
            eq("batch decodes " + path, loaded[path], body)


def test_data_wave_retire_alias() -> None:
    files = {"named_syms.txt": "fixture_false = 0x80011000; /* false evidence */\n"
                               "fixture_true = 0x80011000; /* true evidence */\n",
             "undefined_syms_auto.txt": "D_80011000 = 0x80011000;\n",
             "src/main/a.c": "extern int fixture_false, fixture_true, D_80011000;\n"
                             "int *fixture_use = &fixture_false;\n"}
    rows = "0x80011000,fixture_true,CONFIRM,retire-alias,fixture_false\n"
    _data_wave_fixture_keycheck(files, rows)
    with _data_wave_tree(files, rows) as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            dw.main()
        check("retire-alias keeps true line intact", "fixture_true = 0x80011000; /* true evidence */" in
              (root / "named_syms.txt").read_text())
        eq("retire-alias keeps auto alias intact", (root / "undefined_syms_auto.txt").read_text(),
           files["undefined_syms_auto.txt"])
        check("retired code spelling rewritten", "&fixture_true" in (root / "src/main/a.c").read_text())
    bad = {**files, "named_syms.txt": "fixture_false = 0x80011000;\nfixture_true = 0x80011004;\n"}
    with _data_wave_tree(bad, rows) as (dw, root), contextlib.redirect_stdout(io.StringIO()), \
            contextlib.redirect_stderr(io.StringIO()):
        try:
            dw.main()
        except SystemExit as e:
            eq("retire-alias different-address survivor rejected", e.code, 2)
        else:
            check("retire-alias different-address refuses", False)
    for head in ("int fixture_false(void) { return 1; }",
                 "int\nfixture_false(void) { return 1; }",
                 "int fixture_false(x)\nint x;\n{ return x; }"):
        nested = "int outer(void) {\n" + "\n".join(
            "    " + line for line in head.splitlines()) + "\n    return fixture_false();\n}\n"
        nested_files = {**files, "src/main/a.c": nested}
        output = io.StringIO()
        with _data_wave_tree(nested_files, rows) as (dw, root), \
                contextlib.redirect_stdout(output), contextlib.redirect_stderr(io.StringIO()):
            try:
                dw.main()
            except SystemExit as exc:
                eq("retire-alias rejects nested definition", exc.code, 2)
            else:
                check("retire-alias rejects nested definition", False)
            check("retire-alias names nested function refusal",
                  "cannot retire a function" in output.getvalue())
            eq("retire-alias leaves nested source intact",
               (root / "src/main/a.c").read_text(encoding="utf-8"), nested)


def test_data_wave_nested_census() -> None:
    import data_wave as dw
    text = ("int outer(int value) {\n"
            "    if (value) { return 1; }\n"
            "    else\n"
            "        if (value + 1) { return 2; }\n"
            "    return 0;\n}\n")
    eq("nested census excludes deindented control statement", dw.c_function_names(text), {"outer"})
    eq("nested census keeps genuine nested helper",
       dw.c_function_names("int outer(void) {\n    inline int ENCODE_BCD(n) { return n; }\n}\n"),
       {"outer", "ENCODE_BCD"})


def test_data_wave_commented_dedupe() -> None:
    for survivor in ("D_80011200", "fixture_reset_rename_target"):
        own = "D_80011200 = 0x80011200; /* survivor detail */\n" if survivor.startswith("D_") else ""
        files = {"named_syms.txt": "fixture_home = 0x80011200; /* home detail */\n",
                 "undefined_syms_auto.txt": "fixture_extra = 0x80011200; /* alias detail */\n" + own}
        rows = f"0x80011200,{survivor},CONFIRM\n"
        # Calls both actual main functions; Git tree reads/status alone are mocked.
        _data_wave_fixture_keycheck(files, rows)
        with _data_wave_tree(files, rows) as (dw, root):
            with contextlib.redirect_stdout(io.StringIO()):
                dw.main()
            text = (root / "undefined_syms_auto.txt").read_text(encoding="utf-8")
            check("dedupe kind matches survivor", dw.note_kind("0x80011200", survivor) + " 0x80011200:" in text)
            check("dedupe names only disappearing alias", "retired name 'fixture_extra'" in text)
            if own:
                check("survivor duplicate cites its definition", "duplicate of named_syms.txt:1" in text)
                check("survivor itself is not retired", "retired name 'D_80011200'" not in text)
    with _data_wave_tree({"named_syms.txt": "fixture_empty = 0x80011200;\n",
                          "undefined_syms_auto.txt": "fixture_empty_other = 0x80011200;\n"},
                         "0x80011200,D_80011200,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            dw.main()
        check("bare duplicate has no empty provenance", "preserved note:" not in
              (root / "undefined_syms_auto.txt").read_text(encoding="utf-8"))
    import naming_keycheck as nk
    note = "/* duplicate of named_syms.txt:2 — preserved note: survivor detail */\n"
    proof = {"named_syms.txt": "D_80011200 = 0x80011200;\n"}
    check("duplicate note cannot cite a nonexistent definition", nk.registry_ok(
        "D_80011200 = 0x80011200; /* survivor detail */\n", note, "undefined_syms_auto.txt",
        {"fixture_before": "D_80011200"}, proof.get, list(proof)) is not None)


def test_keycheck_lazy_candidates() -> None:
    import naming_keycheck as nk
    from unittest.mock import patch
    files = {"named_syms.txt": "fixture_lazy = 0x80011200;\n",
             "src/main/relevant.c": "int fixture_lazy = 1;\n",
             "src/main/unrelated.c": "int fixture_unrelated = 1;\n"}
    with _data_wave_tree(files, "0x80011200,D_80011200,CONFIRM\n") as (_, root), \
            patch.object(nk, "ROOT", root), patch.object(nk, "git", return_value="src/main/relevant.c") as grep, \
            patch.object(nk.nw, "read", wraps=nk.nw.read) as read:
        nk._ADDRESS_INDEXES.clear()
        nk._FILE_TEXTS.clear()
        nk._INDEX_QUERIES.clear()
        for _ in range(20):
            nk.tree_address_index(None, {"fixture_lazy"})
        eq("one candidate grep for repeated name", grep.call_count, 1)
        paths = [call.args[0].relative_to(root).as_posix() for call in read.call_args_list]
        check("unrelated source is never read", "src/main/unrelated.c" not in paths)
        eq("relevant source is read once", paths.count("src/main/relevant.c"), 1)


def test_data_wave_preflight_forms() -> None:
    base = {"named_syms.txt": "fixture_bad = 0x80011200;\nfixture_kept = 0x80011200;\n"}
    cases = [(base, "0x80011200,fixture_kept,CONFIRM,retire_alias,fixture_bad\n"),
             (base, "0x80011200,fixture_kept,CONFIRM,retire-alias,fixture_bad\n"
                    "0x80011200,fixture_bad,CONFIRM,retire-alias,fixture_kept\n"),
             ({**base, "src/main/a.c": "int fixture_bad(void) { return 1; }\n"},
              "0x80011200,fixture_kept,CONFIRM,retire-alias,fixture_bad\n"),
             ({**base, "asm/data/a.s": "dlabel fixture_bad\n/* 1 80011200 00000000 */ .word 0\n"},
              "0x80011200,fixture_kept,CONFIRM,retire-alias,fixture_bad\n"),
             ({**base, "build/bb2.map": ".text 0x80011200 0x20\n  0x80011200 fixture_text_func\n"},
              "0x80011200,fixture_kept,CONFIRM,retire-alias,fixture_bad\n")]
    for files, rows in cases:
        with _data_wave_tree(files, rows) as (dw, root), contextlib.redirect_stdout(io.StringIO()), \
                contextlib.redirect_stderr(io.StringIO()):
            try:
                dw.main()
            except SystemExit as e:
                eq("invalid row form rejects before writes", e.code, 2)
            else:
                check("invalid row form must refuse", False)
            for path, text in files.items():
                eq("preflight refusal preserves " + path, (root / path).read_text(encoding="utf-8"), text)


def test_keycheck_alias_auto_and_macro() -> None:
    import sys
    import naming_keycheck as nk
    from unittest.mock import patch
    for old, new, files in (("D_80011200", "func_80011200", {}),
                            ("fixture_alias", "fixture_macro", {
                             "named_syms.txt": "fixture_alias = 0x80011200;\nfixture_macro = 0x80011200;\n",
                             "include/a.h": "#define fixture_macro 1\n"})):
        with _data_wave_tree(files, "0x80011200,D_80011200,CONFIRM\n") as (_, root):
            manifest = root / "forged.json"
            manifest.write_text(json.dumps({"ops": [{"new": new, "olds": [old], "kind": "retire-alias"}]}),
                                encoding="utf-8")
            with patch.object(nk, "ROOT", root), \
                    patch.object(nk, "load_tree_texts", lambda rev, names=(): files), \
                    patch.object(nk, "changed_files", return_value=[]), patch.object(nk, "git", return_value=""), \
                    patch.object(sys, "argv", ["keycheck", "--from-manifests", str(manifest)]), \
                    contextlib.redirect_stdout(io.StringIO()):
                eq("forged auto/macro alias manifest fails closed", nk.main(), 1)


def test_keycheck_pre_read_candidates() -> None:
    import naming_keycheck as nk
    from unittest.mock import patch
    files = {"named_syms.txt": "fixture_cache_label = 0x80011300;\n",
             "asm/data/module.s": "dlabel fixture_cache_label\n/* 1 80011300 00000000 */ .word 0\n",
             "include/cache.h": "#define fixture_cache_macro 1\n"}
    with _data_wave_tree(files, "0x80011300,D_80011300,CONFIRM\n") as (_, root), \
            patch.object(nk, "ROOT", root), \
            patch.object(nk, "git", return_value="asm/data/module.s\ninclude/cache.h"):
        nk._ADDRESS_INDEXES.clear()
        nk._FILE_TEXTS.clear()
        nk._READ_TEXTS.clear()
        nk._INDEX_QUERIES.clear()
        for path in files:
            nk.read_index_file(None, path)
        index, macros = nk.tree_address_index(None, {"fixture_cache_label", "fixture_cache_macro"})
        eq("pre-read label still enters proof index", index.get("fixture_cache_label"), {"80011300"})
        check("pre-read macro still enters proof index", "fixture_cache_macro" in macros)


def test_keycheck_real_git_round4() -> None:
    """Unmocked Git in isolated temporary repositories; never changes the project Git tree."""
    import subprocess
    import sys
    import data_wave as dw
    import naming_keycheck as nk
    from unittest.mock import patch
    scratch = Path(__file__).resolve().parent.parent / "tmp"
    scratch.mkdir(exist_ok=True)
    for leave_old in (False, True):
        with tempfile.TemporaryDirectory(dir=scratch) as td:
            root = Path(td)
            def run_git(*args):
                return subprocess.run(["git", "-c", "user.name=Fixture", "-c",
                        "user.email=fixture@example.invalid", "-c", "core.hooksPath=" + str(root / "no-hooks"), *args], cwd=root,
                        check=True, capture_output=True, text=True).stdout.strip()
            run_git("init", "--quiet")
            files = {".gitignore": "tmp/\n",
                     "named_syms.txt": "fixture_git_alias = 0x80011300; /* alias detail */\n",
                     "undefined_syms_auto.txt": "D_80011300 = 0x80011300; /* auto detail */\n",
                     "asm/data/module.s": "dlabel fixture_git_label\n/* 1 80011300 00000000 */ .word 0\n"}
            if leave_old:
                files["include/untouched.h"] = "extern int fixture_git_alias;\n"
            for path, text in files.items():
                p = root / path
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text(text, encoding="utf-8", newline="\n")
            run_git("add", ".")
            run_git("commit", "--quiet", "-m", "fixture base")
            base = run_git("rev-parse", "HEAD")
            (root / "tmp").mkdir()
            rows, manifest = root / "tmp/rows.csv", root / "tmp/wave.json"
            rows.write_text("addr,proposed_name\n0x80011300,fixture_git_target\n", encoding="utf-8")
            with patch.object(dw, "ROOT", root), patch.object(sys, "argv", [
                    "data_wave", "--manifest-csv", str(rows), "--apply", "--manifest", str(manifest)]), \
                    contextlib.redirect_stdout(io.StringIO()):
                eq("real-git fixture wave applies", dw.main(), 0)
            if leave_old:
                (root / "include/untouched.h").write_text(files["include/untouched.h"], encoding="utf-8")
            def check_tree(rev):
                argv = ["keycheck", "--base", base, "--from-manifests", str(manifest)]
                if rev:
                    argv += ["--new", rev]
                output = io.StringIO()
                with patch.object(nk, "ROOT", root), patch.object(sys, "argv", argv), \
                        contextlib.redirect_stdout(output):
                    result = nk.main()
                eq("real-git " + ("revision" if rev else "working") + " wave result", result, int(leave_old))
                if leave_old:
                    check("untouched build spelling detected", "old name survives" in output.getvalue())
            run_git("add", "-u")  # Stage the completed wave before working keycheck.
            check_tree(None)
            run_git("add", "-u")
            run_git("commit", "--quiet", "-m", "fixture wave")
            revision = run_git("rev-parse", "HEAD")
            check_tree(revision)
            for rev in (None, revision):
                with patch.object(nk, "ROOT", root):
                    try:
                        options = ["--untracked"] if rev is None else []
                        nk.git("grep", "-l", "-E", *options, "[", *([rev] if rev else []),
                               "--", "asm", check=False)
                    except subprocess.CalledProcessError as e:
                        check("real grep errors reject instead of returning no hits", e.returncode > 1)
                    else:
                        check("real grep error must refuse", False)


def test_keycheck_probe_suite() -> None:
    """The phase-3 keycheck probes (scratch tmp/p3s0/kc_test.sh, 2026-10-07) on a fixture repo:
    a complete global-pair wave passes; the raw wave and each single off-wave edit fail, each on
    the edited path. Unmocked Git in an isolated temporary repository."""
    import subprocess
    import sys
    import naming_keycheck as nk
    import naming_wave as nw
    from unittest.mock import patch
    files = {
        ".gitignore": "tmp/\nbuild/\n",
        "Makefile": "CC_FLAGS := -O2 -G0\nMASPSX_FLAGS := --aspsx-version=2.34 --expand-lb\n",
        "include/macro.inc": ".macro glabel label\n    .global \\label\n\\label:\n.endm\n",
        "include/bios.h": "#define BIOS_A_FUNCTION(name, id) void name(void)\n",
        "src/main/a.c": ('#include "bios.h"\n\n'
                         "/* fixture_old: the probe target */\n"
                         "void fixture_old(void) {\n"
                         "    const char *s = \"probe string\";\n"
                         "    /* FAKE: probe reason */\n"
                         "    (void)s;\n"
                         "}\n\n"
                         "void caller(void) { fixture_old(); }\n"),
        "src/main/d_1068.c": "int fixture_data;\n",
        "tools/naming_wave.py": "# fixture tool\n",
        ".claude/rules/completion-bar.md": "fixture rule\n",
        "named_syms.txt": "fixture_old = 0x80011000;\nfixture_ctlbuf = 0x800F189C;\n",
        "memory/grind/_completed/fixture_old/layer2.jsonl":
            json.dumps({"func": "fixture_old", "verdict": "PASS"}) + "\n",
        "memory/grind/_completed/fixture_old/notes.txt": "fixture notes\n",
        "memory/grind/other/plan.txt": "other ledger\n",
    }
    scratch = Path(__file__).resolve().parent.parent / "tmp"
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch) as td:
        root = Path(td)
        def run_git(*args):
            return subprocess.run(["git", "-c", "user.name=Fixture", "-c",
                    "user.email=fixture@example.invalid", "-c", "core.hooksPath=" + str(root / "no-hooks"),
                    "-c", "commit.gpgsign=false", "-c", "core.autocrlf=false", *args], cwd=root,
                    check=True, capture_output=True, text=True).stdout.strip()
        def write(path, text):
            p = root / path
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(text, encoding="utf-8", newline="\n")
        def read(path):
            return (root / path).read_text(encoding="utf-8")
        def commit(message):
            run_git("add", "-A")
            run_git("commit", "--quiet", "-m", message)
            return run_git("rev-parse", "HEAD")
        run_git("init", "--quiet")
        for path, text in files.items():
            write(path, text)
        base = commit("fixture base")
        # The raw wave renames code, registry and ledger but leaves the comment naming fixture_old.
        write("src/main/a.c", read("src/main/a.c").replace("void fixture_old", "void fixture_new")
              .replace("{ fixture_old()", "{ fixture_new()"))
        write("named_syms.txt", read("named_syms.txt").replace("fixture_old =", "fixture_new ="))
        ledger = "memory/grind/_completed/fixture_new"
        run_git("mv", "memory/grind/_completed/fixture_old", ledger)
        record, _ = nw.retarget_layer2_record(read(ledger + "/layer2.jsonl"), "fixture_old", "fixture_new")
        write(ledger + "/layer2.jsonl", record)
        raw = commit("fixture raw wave")
        write("src/main/a.c", read("src/main/a.c").replace("/* fixture_old:", "/* fixture_new:"))
        good = commit("fixture complete wave")
        def keycheck(new, pairs="fixture_old=fixture_new"):
            output = io.StringIO()
            with patch.object(nk, "ROOT", root), patch.object(sys, "argv", [
                    "naming_keycheck", "--base", base, "--new", new, "--pairs", pairs]), \
                    contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
                return nk.main(), output.getvalue()
        rc, out = keycheck(good)
        eq("keycheck probes: the complete wave passes", rc, 0)
        rc, out = keycheck(raw)
        check("keycheck probes: the raw wave (comment names the old name) fails",
              rc == 1 and "FAIL src/main/a.c: an old name survives" in out)
        rc, out = keycheck(good, "fixture_old=fixture_neX")
        check("keycheck probes: a pair naming another new name fails", rc == 1)
        swap = ("    /* FAKE: probe reason */\n    (void)s;\n", "    (void)s;\n    /* FAKE: probe reason */\n")
        def edit(path, old, new):
            return lambda: write(path, read(path).replace(old, new))
        def append(path, text):
            return lambda: write(path, read(path) + text)
        probes = (
            ("layer2 duplicate line", ledger + "/layer2.jsonl",
             lambda: write(ledger + "/layer2.jsonl", read(ledger + "/layer2.jsonl") * 2)),
            ("Makefile edit", "Makefile", edit("Makefile", " --expand-lb", "")),
            ("macro.inc edit", "include/macro.inc", append("include/macro.inc", "# x\n")),
            ("FAKE comment edit", "src/main/a.c", edit("src/main/a.c", "FAKE", "FAKX")),
            ("FAKE comment moved", "src/main/a.c", edit("src/main/a.c", *swap)),
            ("function-like macro made object-like", "include/bios.h",
             edit("include/bios.h", "BIOS_A_FUNCTION(", "BIOS_A_FUNCTION (")),
            ("file rename not in the pairs", "src/main/d_1068.c",
             lambda: run_git("mv", "src/main/d_1068.c", "src/main/zz.c")),
            ("string literal change", "src/main/a.c", edit("src/main/a.c", "probe string", "probe strinG")),
            ("tool edit", "tools/naming_wave.py", append("tools/naming_wave.py", "# x\n")),
            ("rule edit", ".claude/rules/completion-bar.md", append(".claude/rules/completion-bar.md", "x\n")),
            ("moved ledger file edited", ledger + "/notes.txt", append(ledger + "/notes.txt", "x\n")),
            ("other ledger names the new name", "memory/grind/other/plan.txt",
             append("memory/grind/other/plan.txt", "fixture_new\n")),
            ("stray new file", "src/main/new.c", lambda: write("src/main/new.c", "int x;\n")),
            ("partial rename (old name left)", "src/main/a.c",
             edit("src/main/a.c", "{ fixture_new()", "{ fixture_old()")),
            ("registry line edited", "named_syms.txt", edit("named_syms.txt", "0x800F189C", "0x800F18A0")),
            ("registry definition deleted", "named_syms.txt",
             edit("named_syms.txt", "fixture_ctlbuf = 0x800F189C;\n", "")),
            ("registry fake provenance suffix", "named_syms.txt",
             edit("named_syms.txt", "fixture_ctlbuf = 0x800F189C;",
                  "fixture_ctlbuf = 0x800F189C;  /* data-wave 2026-10-07: was fixture_ctlbuf */")),
            # Registry shapes from the kcrev / kcrev2 attack rounds (tmp/kcrev*/attack*.sh).
            ("registry lines reordered", "named_syms.txt",
             lambda: write("named_syms.txt", "".join(reversed(read("named_syms.txt").splitlines(True))))),
            ("registry line added", "named_syms.txt",
             append("named_syms.txt", "fixture_extra = 0x800F18A0;\n")),
            ("provenance suffix with a trailer", "named_syms.txt",
             edit("named_syms.txt", "fixture_new = 0x80011000;",
                  "fixture_new = 0x80011000;  /* data-wave 2026-10-07: was fixture_old */ x")),
            ("provenance suffix naming another old name", "named_syms.txt",
             edit("named_syms.txt", "fixture_new = 0x80011000;",
                  "fixture_new = 0x80011000;  /* data-wave 2026-10-07: was fixture_other */")),
        )
        for label, path, probe in probes:
            run_git("checkout", "--quiet", "-B", "probe", good)
            probe()
            rc, out = keycheck(commit("probe: " + label))
            check("keycheck probes: " + label + " fails on " + path,
                  rc == 1 and ("FAIL " + path + ":") in out)
        # Control: data_wave's own provenance rewrite of the pair's line is a wave output.
        run_git("checkout", "--quiet", "-B", "probe", good)
        edit("named_syms.txt", "fixture_new = 0x80011000;",
             "fixture_new = 0x80011000;  /* data-wave 2026-10-07: was fixture_old */")()
        rc, out = keycheck(commit("probe: provenance control"))
        eq("keycheck probes: data_wave's provenance rewrite passes", rc, 0)


def test_keycheck_formatted_belt() -> None:
    """naming_keycheck.formatted offers a formatter candidate only from the pinned clang-format
    and only when format.py's token belt holds."""
    import types
    import naming_keycheck as nk
    from unittest.mock import patch
    for status, want in (("ok", "int a;\n"), ("tokens", None), ("error", None)):
        fake = types.SimpleNamespace(_format_checked=lambda exe, path, text, s=status:
                                     (s, "int a;\n" if s == "ok" else ""))
        with patch.object(nk, "_FMT", (fake, "clang-format")):
            eq("keycheck formatted candidate on format status " + status,
               nk.formatted("src/a.c", "int  a;\n"), want)
    with patch.object(nk, "_FMT", None), patch.object(nk.subprocess, "run",
            return_value=nk.subprocess.CompletedProcess([], 0, "clang-format version 99.0.0\n", "")), \
            patch.object(Path, "exists", return_value=True):
        eq("keycheck formatted refuses another clang-format version",
           nk.formatted("src/a.c", "int  a;\n"), None)
        eq("keycheck formatted caches the refusal", nk._FMT, False)
    # The real pinned clang-format is accepted and formats (skipped when not installed).
    exe = nk.CODE / ".venv/bin/clang-format"
    if not exe.exists():
        skip("keycheck formatted accepts the pinned clang-format", "no .venv/bin/clang-format")
        return
    with patch.object(nk, "_FMT", None):
        out = nk.formatted("src/main/a.c", "int  fixture_a ;\n")
        check("keycheck formatted accepts the pinned clang-format",
              nk._FMT is not False and out == "int fixture_a;\n")


def test_data_wave_round4_edge_cases() -> None:
    files = {"named_syms.txt": "fixture_array_alias = 0x80011300;\nD_80011300 = 0x80011300; /* array detail */\n",
             "src/main/array.c": "int D_80011300[2][3] = {{0}};\n"}
    with _data_wave_tree(files, "0x80011300,D_80011300,CONFIRM\n") as (dw, root):
        with contextlib.redirect_stdout(io.StringIO()):
            dw.main()
        text = (root / "named_syms.txt").read_text(encoding="utf-8")
        check("multi-bound array cites source", "duplicate of src/main/array.c:1" in text)
        check("duplicate never cites map", "build/bb2.map" not in text)
    with _data_wave_tree({}, "0x80011300,D_80011300,CONFIRM\n") as (dw, root), \
            contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
        try:
            dw.main()
        except SystemExit as e:
            eq("empty identifier map refuses before edits", e.code, 2)
        else:
            check("empty identifier map must refuse", False)
    import naming_keycheck as nk
    old = "D_80011300 = 0x80011300; /* array detail */\n"
    for home in ("../src/array.c", "build/bb2.map"):
        proof = {home: "int D_80011300[2][3] = {{0}};\n"}
        note = f"/* duplicate of {home}:1 — preserved note: array detail */\n"
        check("duplicate home requires a build path", nk.registry_ok(old, note, "named_syms.txt",
              {"fixture_old_array": "D_80011300"}, proof.get, list(proof)) is not None)


def test_retired_names_audit() -> None:
    """tools/check_retired_names.py passes its fixture self-test, and
    check_completion_integrity.main() adds its hits to the violations before
    its verdict."""
    import ast
    import contextlib
    import io
    import sys
    repo_root = Path(__file__).resolve().parent.parent
    sys.path.insert(0, str(repo_root / "tools"))
    import check_retired_names as crn
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        status = crn.self_test()
    eq("retired names: fixture self-test passes", (status, "FAIL" in out.getvalue()), (0, False))
    src = (repo_root / "tools/check_completion_integrity.py").read_text(encoding="utf-8")
    main_fn = next(n for n in ast.parse(src).body
                   if isinstance(n, ast.FunctionDef) and n.name == "main")
    wired = verdict_at = None
    for i, st in enumerate(main_fn.body):
        seg = ast.get_source_segment(src, st) or ""
        if seg.startswith("violations.extend(") and "CRN.scan(REPO)" in seg:
            wired = i
        if isinstance(st, ast.If) and (ast.get_source_segment(src, st.test) or "") == "violations":
            verdict_at = i if verdict_at is None else verdict_at
    check("retired names: check_completion_integrity.main() runs the scan before its verdict",
          wired is not None and verdict_at is not None and wired < verdict_at)


def test_private_head_sites() -> None:
    """Source consumers share the engine's split definition/declaration heads."""
    import sys
    from unittest.mock import patch
    from engine import dossier
    import naming_wave as nw
    import naming_keycheck as nk
    import resplit_convert as rc

    definition = "static s32\nfixture_old(void) { return 1; }\n"
    declaration = "static s32\nfixture_old(void);\n"
    check("resplit sees split declaration", rc.has_declaration(declaration, "fixture_old"))
    check("resplit excludes split definition", not rc.has_declaration(definition, "fixture_old"))
    root = Path("tmp/head-sites-fixture").resolve()
    for text, expected in ((definition, True), (declaration, False),
                           (definition * 2, True)):
        with patch.object(nw, "ROOT", root), patch.object(nw, "_SRC_DEF_CACHE", {}), \
                patch.object(nw, "read", return_value=text), \
                patch.object(Path, "rglob", return_value=[Path("src/a.c")]):
            eq("naming wave split head / declaration / duplicate", nw._defined_in_src(
                "fixture_old"), expected and text != definition * 2)
        with patch.object(dossier, "_read", return_value=text), \
                patch.object(Path, "rglob", return_value=[Path("src/a.c")]):
            eq("dossier split head / declaration / duplicate", dossier._src_representation(
                "fixture_old", "a")[0], "c_body" if expected else "absent")

    manifest = json.dumps({"ops": [{"kind": "retire-alias",
        "olds": ["fixture_old"], "new": "fixture_kept"}]})
    for text in (definition, definition * 2):
        def index(*args, **kwargs):
            key = (str(root), "HEAD")
            nk._DEFINITION_INDEXES[key] = {"fixture_kept": {"80011000"}}
            nk._FILE_TEXTS[key] = {"src/a.c": text}
            return ({}, set())

        output = io.StringIO()
        with patch.object(nk, "ROOT", root), patch.object(nk, "tree_address_index", index), \
                patch.object(nk, "base_addr", return_value="80011000"), \
                patch.object(nk, "changed_files", return_value=[]), \
                patch.object(nk, "git", return_value=""), \
                patch.object(nk, "show", return_value=""), \
                patch.object(nk, "verify_working_storage", return_value=None), \
                patch.object(Path, "read_text", return_value=manifest), \
                patch.object(sys, "argv", ["keycheck", "--from-manifests", "fixture.json"]), \
                contextlib.redirect_stdout(output):
            eq("retire-alias refuses split and ambiguous definitions", nk.main(), 1)
        check("retire-alias reports function label refusal",
              "cannot retire a function or assembly definition label" in output.getvalue())


def main() -> int:
    test_private_head_sites()
    test_live_tu_refusal_root()
    test_header_facts_gates()
    test_project_header_facts_cache()
    test_retired_names_audit()
    test_data_wave_round4_edge_cases()
    test_keycheck_pre_read_candidates()
    test_keycheck_real_git_round4()
    test_keycheck_probe_suite()
    test_keycheck_formatted_belt()
    test_data_wave_commented_dedupe()
    test_keycheck_lazy_candidates()
    test_data_wave_preflight_forms()
    test_keycheck_alias_auto_and_macro()
    test_data_wave_standalone()
    test_data_wave_tentative_definitions()
    test_data_wave_curated_dedupe()
    test_data_wave_fictional_symbols()
    test_keycheck_bitfield_member_declarators()
    test_keycheck_index_once()
    test_data_wave_nested_census()
    test_data_wave_retire_alias()
    test_data_wave_scratchpad()
    test_data_wave_dlabel_only()
    test_data_wave_object_target()
    test_data_wave_cross_registry()
    test_data_wave_held_rename()
    test_data_wave_fixture_p3w6()
    test_data_wave_fixture_p3w17()
    test_data_wave_fixture_p3w14()
    test_data_wave_fixture_p3w9()
    test_keycheck_registry_address_proof()
    test_datamodel()
    test_canonical()
    test_score()
    test_insn_diff()
    test_score_section_addend_mask()
    test_symtab_data_dlabels()
    test_inlineasm()
    test_gte_macro_units()
    test_gte_vendored_model()
    test_gte_call_gates()
    test_gte_header_registry()
    test_gte_macro_rewrite_tool()
    test_cheats()
    test_prologue_cheat()
    test_addr_coerced_locals()
    test_sanctioned_unwritten_pads()
    test_volatile_extern_allowlist()
    test_memo_and_rule_index_caches()
    test_lowercase_asm_cheats()
    test_asm_keyword_recognition()
    test_macro_asm_strip_round_trip()
    test_substitute_body()
    test_brace_prefixed_definition_head()
    test_additional_file_scope_heads()
    test_file_scope_head_review()
    test_shared_brace_head_review()
    test_delimited_definition_heads()
    test_definition_head_debt()
    test_definition_head_consumers()
    test_split_definition_head()
    test_definition_type_line()
    test_definition_names()
    test_tool_definition_heads()
    test_queue_ambiguous_definition_message()
    test_body_span_ambiguous()
    test_ambiguous_c_asm_fallbacks()
    test_ambiguous_body_discovery_and_stripping()
    test_tool_ambiguous_cheat_readers()
    test_data_as_code_conflicts()
    test_synth_addrs_guard()
    with _synth_addrs():
        test_include_asm_whole_body()
    test_completion_region_grants()
    test_q126_inline_helpers()
    test_buildstamp()
    with _synth_addrs():
        test_canonical_completion_is_the_drop()
        test_queue_non_function_labels()
    with _synth_addrs():
        test_layer2_gate()
    test_queue_repo_root()
    test_queue_regen_diff_repo_root()
    test_departures()
    test_layer2_repo_paths()
    test_layer2_addresses()
    test_layer2_asm_addr_after_glabel()
    test_naming_wave_own_target()
    test_naming_wave_keyed_registries()
    test_census_manifest_precedence()
    test_naming_wave_py_escape_boundary()
    test_note_kind_function_reset()
    test_naming_wave_renames()
    test_naming_keycheck_scoped_pairs()
    test_naming_keycheck_movovl_q130()
    test_volatile_unused_locals()
    test_always_true_if_scaffolds()
    test_empty_do_while_zero()
    test_empty_if_dead_reads()
    test_void_discard_unused_locals()
    test_orphaned_local_decls()
    test_dead_conditional_stores()
    test_fake_annotated_lever_d_bypass()
    with _synth_addrs():
        test_queue_reopen()
    with _synth_addrs():
        test_queue_hand_coded_tier()
    test_queue_write_serialization()
    test_queue_rotation()
    test_queue_remeasure_source_integrity()
    test_maspsx_indexed_operand_not_gp()
    test_maspsx_static_lcomm()
    test_maspsx_small_data_sdata()
    test_psyq_library_files()
    test_data_only_files_fixture()
    test_tus()
    test_naming_member_apply()
    test_ambiguous_tool_diagnostics()
    test_move_tu()
    test_maspsx_fingerprint()
    test_objdump_failure_is_loud()
    test_prologue_config_fingerprint()
    test_canonical_build()
    test_rodata_object_alignment()
    test_score_object_paths()
    print(f"\n{_passed} passed, {_failed} failed, {_skipped} skipped")
    return 1 if _failed else 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
