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

from engine import canonical, score, inlineasm, cheats, metrics, volatile_cheats
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
    ruling Q39); tests that queue made-up functions give them one."""
    from engine import layer2
    real_index, real_of = layer2.addr_index, layer2.addr_of
    layer2.addr_index = lambda: _SynthAddrs(real_index())
    layer2.addr_of = lambda f: real_of(f) or _SynthAddrs().get(f)
    try:
        yield
    finally:
        layer2.addr_index, layer2.addr_of = real_index, real_of


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
    eq("gtemacro: DMPSX_WORDS is exactly the evidenced pairs (Q11 rtv0; Q29 rtv0tr/sqr0/gpf0/gpl12)",
       gtemacro.DMPSX_WORDS, {0x0000013F: 0x4A486012, 0x0000027F: 0x4A480012,
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
    # commit with layer-2; the func_8002EBDC / func_8002F2D0 / func_8002F770 cluster).
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
    # ONLY. Uses a real canonical member (func_8004A76C, custom $s0 ABI; was
    # save_vc_ctrl until its 2026-10-01 de-authorization)
    # so the check exercises the live list.
    check("canon-extract: wiring for canonical member IS exempt",
          cheats.is_canonical_extraction_rule(
              'func_8004A76C: replace_with_asmfile "asm/funcs/func_8004A76C.s"'))
    check("canon-extract: wiring for NON-canonical func is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'not_a_canonical_func_zz: replace_with_asmfile "asm/funcs/x.s"'))
    check("canon-extract: non-wiring rule for canonical member is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'func_8004A76C: insert_after "addu $8,$3,$zero"'))
    check("canon-extract: path outside asm/funcs is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'func_8004A76C: replace_with_asmfile "tmp/evil.s"'))
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
    with tempfile.TemporaryDirectory() as td:
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
                  not r.get("ok"))
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
    if (D_800A3580) {}
    other_thing();
}
"""
    body_lo = text.index("{")
    body_hi = text.rindex("}") + 1
    hits = volatile_cheats.find_empty_if_dead_reads(text, body_lo, body_hi)
    eq("empty-if: catches `if (D_800A3580) {}`", len(hits), 1)

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


# --------------------------------------------------------------------------
# metrics — the capture layer's non-negotiable: silent + swallow-on-failure
# --------------------------------------------------------------------------

@contextlib.contextmanager
def _env(**kv):
    """Temporarily set/clear env vars, restoring prior state on exit."""
    old = {k: os.environ.get(k) for k in kv}
    try:
        for k, v in kv.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
        yield
    finally:
        for k, v in old.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v


def test_metrics() -> None:
    # 1. normal append: one parseable line, normalized fields lifted out
    with tempfile.TemporaryDirectory() as td:
        logp = Path(td) / "events.jsonl"
        with _env(BB2_METRICS_LOG=str(logp), BB2_METRICS_DISABLE=None):
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
                metrics.record_event("sandbox", "func_X",
                                     {"score": 3, "file": "code6cac"},
                                     extra={"disable": "all"})
            eq("metrics: hot path prints nothing", buf.getvalue(), "")
            lines = logp.read_text().splitlines()
            eq("metrics: one event appended", len(lines), 1)
            rec = json.loads(lines[0])
            eq("metrics: command recorded", rec["command"], "sandbox")
            eq("metrics: func recorded", rec["func"], "func_X")
            eq("metrics: score normalized to top level", rec["score"], 3)
            eq("metrics: file normalized to top level", rec["file"], "code6cac")
            eq("metrics: full result preserved in payload", rec["payload"]["score"], 3)

    # 2. THE non-negotiable: a forced write failure is swallowed AND silent.
    #    A file where a directory is needed makes parent.mkdir()/open() fail.
    with tempfile.TemporaryDirectory() as td:
        blocker = Path(td) / "blocker"
        blocker.write_text("x")
        logp = blocker / "events.jsonl"
        with _env(BB2_METRICS_LOG=str(logp), BB2_METRICS_DISABLE=None):
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
                ret = metrics.record_event("retire", "func_Y", {"ok": True})
            eq("metrics: failure swallowed (returns None, no raise)", ret, None)
            eq("metrics: failure prints nothing", buf.getvalue(), "")

    # 3. BB2_METRICS_DISABLE is a hard no-op (writes nothing)
    with tempfile.TemporaryDirectory() as td:
        logp = Path(td) / "events.jsonl"
        with _env(BB2_METRICS_LOG=str(logp), BB2_METRICS_DISABLE="1"):
            metrics.record_event("sandbox", "func_Z", {"score": 0})
            check("metrics: DISABLE writes nothing", not logp.exists())


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
    # motion_Close lives inside motion_Open.s -> the old asm/funcs gate said
    # NO-TARGET; the ELF gate must resolve it.
    if "__do_global_dtors" in tbl:
        eq("classify(__do_global_dtors): resolves (not NO-TARGET)",
           canonical.classify("__do_global_dtors")["verdict"], "C")
    if "func_8002EBDC" in tbl:
        eq("classify(func_8002EBDC): GTE -> ASM-PARTIAL",
           canonical.classify("func_8002EBDC")["verdict"], "ASM-PARTIAL")
    if "func_8003F1C8" in tbl:
        eq("classify(func_8003F1C8): ordinary C -> C",
           canonical.classify("func_8003F1C8")["verdict"], "C")
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
    directory contains spaces (this repo's own path does) must give the same
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
    absolute = str(Path(o).resolve())
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
    eq("score: score_func(absolute, relative) == 0 (same object, either spelling)",
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
    knr = ('void func_8004A1FC(arg0) s16 *arg0; {\n'
           '    __asm__ volatile("addu $8,$3,$0");\n}\n')
    eq("body-span: K&R parameter declarations between `)` and `{`",
       inlineasm.func_cheat_asm_count(knr, "func_8004A1FC"), 1)
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
          volatile_cheats._func_body_span(knr, "func_8004A1FC") is not None)

    # 4. generate(): a negative (UNKNOWN) count must NOT drop an item, and a
    #    measured-zero count still must.
    def _regen_with(cheat_count: int, canon: set, src_text: str | None = None) -> list:
        with tempfile.TemporaryDirectory() as td:
            cwd = os.getcwd()
            os.chdir(td)
            Path("build/src").mkdir(parents=True)
            Path("build/src/faketu.o").write_text("")
            if src_text is not None:
                Path("src").mkdir()
                Path("src/faketu.c").write_text(src_text)
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
    # The ONE safe exception: the symbol is not a C-level function at all.
    eq("include_asm: regen drops an UNKNOWN symbol absent from the .c",
       _regen_with(-1, set(), src_text="void other(void) {}\n"), [])
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
        with tempfile.TemporaryDirectory() as td:
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
          r_attr.get("ok") is False)
    r_unk = _done_with(-1, set())
    check("include_asm: queue done REFUSES an UNKNOWN cheat count",
          r_unk.get("ok") is False)
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
            grants.write_text(json.dumps({"schema": 1,
                                          "functions": {"func_MIX": entry}}))
            eq("region grants: exact reviewed island is accepted",
               completion.source_issues("fake", "func_MIX", canonical=True), [])

            changed = text.replace("$13", "$14")
            inlineasm._read_src_cached = lambda stem: changed
            issues = completion.source_issues("fake", "func_MIX", canonical=True)
            check("region grants: an operand change invalidates completion",
                  any("changed since review" in i for i in issues))
        finally:
            inlineasm._read_src_cached, completion.REGIONS = old_read, old_regions


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

    with tempfile.TemporaryDirectory() as td:
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
                  r2.get("ok") is False)
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
    from engine import layer2

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
    eq("layer2: a C definition AND an asm body -> no key (fail closed)",
       layer2.body_key('int func_D(void) { return 1; }\nINCLUDE_ASM("asm/funcs", func_D);\n',
                       "func_D"), None)

    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td:
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
                # name no longer appears in the .c) is held too — both the
                # scored and the unscorable not-a-C drop go through _held. A
                # never-listed non-C symbol still drops silently.
                score._o_func_table = lambda o: {"func_NC": (0, 0), "func_NC2": (0, 0)}
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
    from engine import departures, layer2

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
        os.chdir(path)
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
    with tempfile.TemporaryDirectory() as td:
        try:
            os.chdir(td)
            Path(".git").write_text("gitdir: C:/Repo/.git/worktrees/w\n")
            eq("departures: WSL git reads a Windows gitdir",
               departures._git_cmd("posix"),
               ["git", "--no-replace-objects", "--git-dir=/mnt/c/Repo/.git/worktrees/w"])
            Path(".git").write_text("gitdir: /mnt/c/Repo/.git/worktrees/w\n")
            eq("departures: Windows git reads a /mnt gitdir",
               departures._git_cmd("nt"),
               ["git", "--no-replace-objects", "--git-dir=C:/Repo/.git/worktrees/w"])
            Path(".git").unlink()

            # ── repo a: record semantics, TU split, asm body, reopen ─────────
            fns = ["func_X", "func_Y", "func_Z", "func_V", "func_W", "func_K", "func_R",
                   "func_T", "func_CAL", "func_CL", "func_SK", "func_SX", "func_FA2",
                   "CdTest", "func_Q"]
            Path(td, "a").mkdir()
            os.chdir(Path(td, "a"))
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

            os.chdir(td)
            git("clone", "-q", "--depth", "1", f"file://{Path(td) / 'a'}", "shallow")
            os.chdir(Path(td) / "shallow")
            v = departures.unreviewed_departures()
            check("departures: a shallow clone fails closed",
                  len(v) == 1 and "cannot establish the anchor (shallow clone?)" in v[0])

            # ── repo w: naming waves never make a departure ────────────────
            new_repo(Path(td) / "w", ["func_80058580", "func_80055B60", "func_P1",
                                      "func_RVA", "func_DN", "func_RQ", "func_WL",
                                      "func_FX", "func_UL", "func_BX", "func_SX1",
                                      "func_SW1", "func_SX2", "func_SW2", "func_HX",
                                      "func_HW", "func_Q"])
            wave({"func_80055B60": "func_Caller", "func_80058580": "func_Callee"})
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
                os.chdir(repo_root)
                expected = audit()
                dirty = subprocess.run([*real_git, "status", "--porcelain", "--", "engine/queue.json",
                                        "memory/grind", "src", "include", "asm/funcs",
                                        "docs/naming", ":(glob)*.txt"],
                                       cwd=str(repo_root), capture_output=True, text=True).stdout
                os.chdir(td)
                git("clone", "-q", "--shared", "--no-checkout", common, "real")
                os.chdir(Path(td) / "real")
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

    # naming_wave no longer writes renamed_from on queue items (FAIL-4), and
    # its residual audit is clean after a wave over a queued function
    import naming_wave as nw
    data = {"items": [{"func": "func_A", "addr": "80010000"}]}
    nw.retarget_queue_items(data, {"func_A": "func_A2"})
    eq("departures: naming_wave renames a queue item's func only",
       data["items"], [{"func": "func_A2", "addr": "80010000"}])
    with tempfile.TemporaryDirectory() as td:
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


def test_layer2_addresses() -> None:
    """Round-6 review FAIL-2..4: every function's address from TRACKED files
    (a fresh clone has no build/), the link map's assignment lines, refusals
    for an item or record without one, the gate refusing a pre-address record,
    and record() never stamping a date earlier than its file's last."""
    from engine import layer2
    cwd = os.getcwd()
    with tempfile.TemporaryDirectory() as td:
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

    with tempfile.TemporaryDirectory() as td:
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
       len(spans_for("func_8001E404", pad)), 0)
    eq("pad-allowlist: second sanctioned function exempt",
       len(spans_for("func_8001E6E4", pad)), 0)
    eq("pad-allowlist: unsanctioned function still flagged",
       len(spans_for("func_80012345", pad)), 1)
    eq("pad-allowlist: non-volatile spelling still flagged",
       len(spans_for("func_8001E404",
                     "    s32 pre_pad[2];" + NL + "    other();" + NL)), 1)
    eq("pad-allowlist: wrong count still flagged",
       len(spans_for("func_8001E404",
                     "    volatile u32 pre_pad[8];" + NL + "    other();" + NL)), 1)
    eq("pad-allowlist: wrong name still flagged",
       len(spans_for("func_8001E404",
                     "    volatile u32 spill[2];" + NL + "    other();" + NL)), 1)
    text = "void func_8001E404(void) {" + NL + pad + "}" + NL
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
            "g_leaf_slot_state = 0x800A3918;  /* 6-byte slot state table (per-leaf counter byte) */\n"
            "g_leaf_position_table = 0x80107850;  /* 12-byte stride per leaf, 6 entries */\n"
            "g_leaf_position_table_plus_4 = 0x80107854;  /* +4 from g_leaf_position_table (0x80107850) */\n"
            "g_plain = 0x80107900;  /* just a flag */\n", encoding="utf-8")
        Path("undefined_syms_auto.txt").write_text(
            "D_800A3918 = 0x800A3918;\nD_80107850 = 0x80107850;\nD_80107854 = 0x80107854;\n"
            "D_80107900 = 0x80107900;\n", encoding="utf-8")
        Path("include/x.h").write_text(
            "extern u8 D_800A3918;\nextern s32 D_80107850;\nextern s32 D_80107854;\n"
            "extern u8 D_80107900;\n", encoding="utf-8")
        Path("asm/funcs/func_A.s").write_text(
            "glabel func_A\n"
            "    lui $at, %hi(D_800A3918)\n    addu $at, $at, $v1\n"
            "    lbu $v0, %lo(D_800A3918)($at)\n"
            "    lui $at, %hi(D_80107850)\n    sw $v1, %lo(D_80107850)($at)\n"
            "    lui $at, %hi(D_80107854)\n    sw $a0, %lo(D_80107854)($at)\n"
            "    lui $v0, %hi(D_80107900)\n    lbu $v0, %lo(D_80107900)($v0)\n"
            "    jal func_B\n", encoding="utf-8")
        Path("asm/funcs/func_B.s").write_text(
            "glabel func_B\n    lui $at, %hi(D_80107854)\n    lw $v0, %lo(D_80107854)($at)\n",
            encoding="utf-8")
        Path("src/a.c").write_text('INCLUDE_ASM("asm/funcs", func_A);\n'
                                   'INCLUDE_ASM("asm/funcs", func_B);\n', encoding="utf-8")
        DM.reset_cache()
        rows, flags = DM.data_model("func_A")
        eq("one row per data symbol, callee excluded", len(rows), 4)
        check("rows are address-ordered", rows[0].startswith("  D_800A3918"))
        check("sub-symbol row marked", any("SUB-SYMBOL +4 of g_leaf_position_table" in r for r in rows))
        check("census comment surfaced", any('"12-byte stride per leaf, 6 entries"' in r for r in rows))
        check("header decl surfaced", any("decl `extern u8 D_800A3918;` (x.h)" in r for r in rows))
        check("asm xref lists the still-INCLUDE_ASM sibling", any("xref asm:func_B" in r for r in rows))
        kinds = [f.split(":")[0] for f in flags]
        # D_800A3918: indexed + census ("table"); D_80107850: census ("stride");
        # the split flag; D_80107850's plain lui/sw must NOT read as indexed.
        eq("signals: 2x census-vs-decl + indexed + split-aggregate", sorted(kinds),
           sorted(["!! INDEXED-ACCESS", "!! CENSUS-VS-DECL", "!! CENSUS-VS-DECL",
                   "!! SPLIT-AGGREGATE"]))
        check("indexed flag names the indexed symbol only",
              any("INDEXED-ACCESS: the target indexes D_800A3918" in f for f in flags)
              and not any("INDEXED-ACCESS: the target indexes D_80107850" in f for f in flags))
        split = next(f for f in flags if f.startswith("!! SPLIT-AGGREGATE"))
        check("split flag names the sibling that keeps the config row alive",
              "still-INCLUDE_ASM func_B" in split)
        check("plain flag global raises no signal", not any("D_80107900" in f for f in flags))
        # a declaration merge silences the signals it answers
        Path("include/x.h").write_text(
            "extern u8 D_800A3918[6];\nextern LeafPos D_80107850[6];\nextern u8 D_80107900;\n",
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
            "Makefile": ("GP_FILES := gpu\nPSYQ_LIBRARY_FILES := comb gpu\nEXPAND_LB_FILES :=\n"
                         "EXPAND_LH_FILES :=\nNO_SR_FILES :=\n"),
            "engine/buildconfig.py": ('GP_FILES = {"gpu"}\nEXPAND_LB_FILES = set()\nEXPAND_LH_FILES = set()\n'
                                      'PSYQ_LIBRARY_FILES = {"comb", "gpu"}\nNO_SR_FILES = set()\n'),
            "oracle/manifest.json": json.dumps({"corpus": {"src/comb.c": "h1", "src/gpu.c": "h2"},
                                                "golden_fixtures": [{"name": "f", "file": "src/comb.c"}]},
                                               indent=2) + "\n",
            "tools/cc1_tu_expectation.txt": "# src-digest x\n" + "a" * 40 + "  comb\n" + "b" * 40 + "  gpu\n",
            "tools/canonical_asm_regions.json": json.dumps({"schema": 1, "functions": {}}, indent=2) + "\n",
            "engine/queue.json": json.dumps({"items": [{"func": "f", "file": "comb"}]}, indent=2) + "\n",
            ".claude/rules/r.md": '---\nname: r\npaths: ["src/comb.c", "src/**/*.c"]\n---\nbody src/comb.c\n',
            "tools/other.sh": "case $stem in comb) ;; esac\n",
        }
        for rel, t in files.items():
            Path(R, rel).parent.mkdir(parents=True, exist_ok=True)
            Path(R, rel).write_bytes(t.encode())
        git("init", "-q")
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
            Path(R, "src/gpu.c").write_text("int gpu2;\n")
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                eq("move_tu: a dirty tree refuses", mt.main(["comb", new, "--root", td, "--apply"]), 1)
            git("checkout", "--", "src/gpu.c")
            with contextlib.redirect_stdout(io.StringIO()):
                eq("move_tu: apply exits 0", mt.main(["comb", new, "--root", td, "--apply"]), 0)
            check("move_tu: the source moved", Path(R, f"src/{new}.c").read_text() == "int comb;\n"
                  and not Path(R, "src/comb.c").exists())
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
            check("move_tu: the round trip restores every keyed file byte-for-byte",
                  all(Path(R, rel).read_bytes() == b for rel, b in snapshot.items()))
            check("move_tu: the round trip removes the emptied directories",
                  not Path(R, "src/main").exists())
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                git("commit", "-qm", "back")
                eq("move_tu: an existing target refuses", mt.main(["comb", "gpu", "--root", td]), 1)
                eq("move_tu: a missing source refuses", mt.main(["nope", "x", "--root", td]), 1)
                eq("move_tu: a non-id target refuses", mt.main(["comb", "../x", "--root", td]), 1)
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
                     "dlabel NoData\n"
                     "\n"
                     "enddlabel NoData\n"
                     "    /* 0 80000008 */ .byte 0x00\n")
        with open(os.path.join(d, "syms.txt"), "w") as fh:
            fh.write("Overridden = 0x80001000;\n")
        score._SYMTAB_CACHE = None
        score._DATA_ASM_GLOB = os.path.join(d, "*.s")
        score.cfg.LD_SYM_FILES = [os.path.join(d, "syms.txt")]
        t = score._symtab()
        eq("dlabel: splat-named data symbol resolves", t.get("D_800A12FC"), 0x800A12FC)
        eq("dlabel: Sony-named data symbol resolves", t.get("CD_debug"), 0x800A11C0)
        eq("dlabel: linker symbol file wins on conflict", t.get("Overridden"), 0x80001000)
        eq("dlabel: only the first data line after the label counts",
           t.get("NoData"), None)
    finally:
        score._SYMTAB_CACHE, score._DATA_ASM_GLOB = saved_tab, saved_glob
        score.cfg.LD_SYM_FILES = saved_files


def main() -> int:
    test_datamodel()
    test_canonical()
    test_score()
    test_insn_diff()
    test_score_section_addend_mask()
    test_symtab_data_dlabels()
    test_inlineasm()
    test_gte_macro_units()
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
    with _synth_addrs():
        test_include_asm_whole_body()
    test_completion_region_grants()
    test_buildstamp()
    with _synth_addrs():
        test_canonical_completion_is_the_drop()
    with _synth_addrs():
        test_layer2_gate()
    test_departures()
    test_layer2_addresses()
    test_naming_wave_renames()
    test_volatile_unused_locals()
    test_always_true_if_scaffolds()
    test_empty_do_while_zero()
    test_empty_if_dead_reads()
    test_void_discard_unused_locals()
    test_orphaned_local_decls()
    test_dead_conditional_stores()
    test_fake_annotated_lever_d_bypass()
    test_metrics()
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
    test_tus()
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
