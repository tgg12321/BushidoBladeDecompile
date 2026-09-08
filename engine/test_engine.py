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
from engine import queue as Q
from engine import pipeline as P
from engine import buildconfig as cfg

_passed = _failed = _skipped = 0


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
    score._SYMTAB_CACHE = {"D_800F19B8": 0x800F19B8, "D_800F19BC": 0x800F19BC,
                           "g_known": 0x80100000, "g_alias": 0x80100002,
                           "g_edge": 0x80107FFC}
    try:
        with _stub_objdump(obj("4", sym="D_800F19B8")):
            a = score.normalized_insns("a.o", "f")
        with _stub_objdump(obj("0", sym="D_800F19BC")):
            b = score.normalized_insns("b.o", "f")
        eq("named-addend: D_800F19B8+4 vs D_800F19BC scores 0 (the CD_datasync s58 artifact)",
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
    # ONLY. Uses a real canonical member (save_vc_ctrl, authorized 2026-06-07)
    # so the check exercises the live list.
    check("canon-extract: wiring for canonical member IS exempt",
          cheats.is_canonical_extraction_rule(
              'save_vc_ctrl: replace_with_asmfile "asm/funcs/save_vc_ctrl.s"'))
    check("canon-extract: wiring for NON-canonical func is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'not_a_canonical_func_zz: replace_with_asmfile "asm/funcs/x.s"'))
    check("canon-extract: non-wiring rule for canonical member is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'save_vc_ctrl: insert_after "addu $8,$3,$zero"'))
    check("canon-extract: path outside asm/funcs is NOT exempt",
          not cheats.is_canonical_extraction_rule(
              'save_vc_ctrl: replace_with_asmfile "tmp/evil.s"'))
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
            gone = next((i for i in q2["items"] if i["func"] == "func_PARKED_GONE"), None)
            check("regen: parked item the scan no longer produces DROPS (unchanged old behavior)",
                  gone is None)
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
    if "motion_Close" in tbl:
        eq("classify(motion_Close): resolves (not NO-TARGET)",
           canonical.classify("motion_Close")["verdict"], "C")
    if "func_8002EBDC" in tbl:
        eq("classify(func_8002EBDC): GTE -> ASM-PARTIAL",
           canonical.classify("func_8002EBDC")["verdict"], "ASM-PARTIAL")
    if "game_GetMode" in tbl:
        eq("classify(game_GetMode): ordinary C -> C",
           canonical.classify("game_GetMode")["verdict"], "C")
    # no function should be NO-TARGET (the motion_Close-class fix)
    no_target = [r["func"] for r in canonical.scan_all() if r["verdict"] == "NO-TARGET"]
    eq("scan_all: zero NO-TARGET", len(no_target), 0)


def test_score_object_paths() -> None:
    """score reads objects through an argv list, so an ABSOLUTE path whose
    directory contains spaces (this repo's own path does) must give the same
    answer as the relative path the engine normally passes. Under the old
    shell-interpolated command the path word-split into an empty symbol table
    and surfaced as a bogus '<func> not found in <obj>'."""
    import shutil
    objs = sorted(Path("build/src").glob("*.o"))
    if not objs:
        skip("score: absolute object path with spaces", "no build/src/*.o — run build first")
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


def test_include_asm_whole_body() -> None:
    """A whole-body INCLUDE_ASM must never read as clean, pure-C, or complete.

    Regression for the 2026-08-06 Campaign-4 pilot finding: the stripper runs on
    UNEXPANDED source, so `INCLUDE_ASM(...)` never matched cia.ASM_KEYWORD_RE
    (which only knows `__asm__`/`__asm`) and was not even a candidate for
    stripping. A function with zero lines of decompiled C therefore scored an
    honest pure-C distance of 0 and counted as clean (-1 read as <= 0), so
    queue.generate dropped it and mark_done would have recorded it COMPLETED-C.
    """
    inc = 'INCLUDE_ASM("asm/funcs", func_800836C8);\n'

    # 1. Recognition + attribution.
    eq("include_asm: macro invocation recognised",
       [f for f, _s, _e in inlineasm.include_asm_spans(inc)], ["func_800836C8"])
    check("include_asm: named in whole_body_asm_funcs",
          "func_800836C8" in inlineasm.whole_body_asm_funcs(inc))
    check("include_asm: a #define of the macro is not an invocation",
          inlineasm.include_asm_spans('#define INCLUDE_ASM(F, N) __asm__()\n') == [])

    # 2. Stripped for scoring — otherwise the sandbox assembles target bytes
    #    straight from asm/funcs/<name>.s and reports distance 0.
    out, n = inlineasm.strip_cheat_asm_file(inc)
    check("include_asm: stripped from the sandbox source", "INCLUDE_ASM" not in out)
    check("include_asm: counted as a stripped cheat construct", n >= 1)

    # 3. Counted > 0 despite there being no C body to attribute it to.
    eq("include_asm: attributed to the named function",
       inlineasm.func_cheat_asm_count(inc, "func_800836C8"), 1)
    # The hand-expanded `.include` spelling is the same fact.
    exp = ('__asm__(\n    ".section .text\\n"\n'
           '    "    .include \\"asm/funcs/func_800836C8.s\\"\\n"\n);\n')
    check("include_asm: hand-expanded .include attributed too",
          "func_800836C8" in inlineasm.whole_body_asm_funcs(exp))
    # Genuinely unexplained symbols still report UNKNOWN rather than a fake 1.
    eq("include_asm: unrelated symbol still UNKNOWN",
       inlineasm.func_cheat_asm_count(inc, "some_other_func"), -1)
    # Attribution is spelling-INDEPENDENT: a `glabel` whole-body block is the
    # same fact and must attribute too, or the defect just moves one spelling
    # over. (It is still never STRIPPED — canonical_body behaviour is unchanged.)
    gl = ('__asm__(\n    ".section .text\\n"\n    "glabel func_800836B8\\n"\n'
          '    "    jr $ra\\n"\n    "    nop\\n"\n    "endlabel func_800836B8\\n"\n);\n')
    eq("include_asm: glabel whole-body block attributed",
       inlineasm.func_cheat_asm_count(gl, "func_800836B8"), 1)
    check("include_asm: glabel whole-body block still NOT stripped",
          "glabel func_800836B8" in inlineasm.strip_cheat_asm_file(gl)[0])
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
    for stem in ("ings", "ings2", "system", "main", "text1b", "display"):
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
                    inlineasm.file_func_cheat_asm_count, Q.O.verify)
            Q.QUEUE_PATH = str(qp)
            Q._rule_count = lambda f: 0
            cheats.func_prologue_count = lambda f: 0
            cheats.canonical_asm_funcs = lambda: canon
            cheats.maspsx_gate_entries = lambda f: []
            inlineasm.file_func_cheat_asm_count = lambda s, f: cheat_count
            Q.O.verify = lambda rebuild=False: {"build_matches": True,
                                                "build_sha1": "deadbeef"}
            try:
                return Q.mark_done("func_WB")
            finally:
                (Q.QUEUE_PATH, Q._rule_count, cheats.func_prologue_count,
                 cheats.canonical_asm_funcs, cheats.maspsx_gate_entries,
                 inlineasm.file_func_cheat_asm_count, Q.O.verify) = orig

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
                Q.sandbox.build_stripped_object, score._o_func_table,
                score.score_func, Q.O.verify, cheats.is_jtbl_infra,
                cheats.is_canonical_extraction_only)
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
            Q.sandbox.build_stripped_object = lambda *a, **k: None
            score._o_func_table = lambda o: ["func_CANON"]
            score.score_func = lambda a, b, f: {"score": 300}
            Q.O.verify = lambda rebuild=False: {"build_matches": True,
                                                "build_sha1": "deadbeef"}
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
             Q.sandbox.build_stripped_object, score._o_func_table,
             score.score_func, Q.O.verify, cheats.is_jtbl_infra,
             cheats.is_canonical_extraction_only) = orig


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
                     Q.inlineasm.file_func_cheat_asm_count, Q.O.verify)
            Q._rule_count = lambda f: 0
            Q.cheats.func_prologue_count = lambda f: 0
            Q.cheats.maspsx_gate_entries = lambda f: []
            Q.cheats.canonical_asm_funcs = lambda: set()
            Q.inlineasm.file_func_cheat_asm_count = lambda s, f: 0
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
                 Q.inlineasm.file_func_cheat_asm_count, Q.O.verify) = saved

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


def main() -> int:
    test_datamodel()
    test_canonical()
    test_score()
    test_score_section_addend_mask()
    test_inlineasm()
    test_cheats()
    test_prologue_cheat()
    test_addr_coerced_locals()
    test_sanctioned_unwritten_pads()
    test_volatile_extern_allowlist()
    test_memo_and_rule_index_caches()
    test_lowercase_asm_cheats()
    test_macro_asm_strip_round_trip()
    test_include_asm_whole_body()
    test_canonical_completion_is_the_drop()
    test_volatile_unused_locals()
    test_always_true_if_scaffolds()
    test_empty_do_while_zero()
    test_empty_if_dead_reads()
    test_void_discard_unused_locals()
    test_orphaned_local_decls()
    test_dead_conditional_stores()
    test_fake_annotated_lever_d_bypass()
    test_metrics()
    test_queue_reopen()
    test_queue_hand_coded_tier()
    test_queue_write_serialization()
    test_queue_rotation()
    test_canonical_build()
    test_score_object_paths()
    print(f"\n{_passed} passed, {_failed} failed, {_skipped} skipped")
    return 1 if _failed else 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
