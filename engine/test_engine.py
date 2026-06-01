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
    eq("verdict: d600 -> ASM-STRUCTURAL",
       canonical._verdict("f", [], 80, distance=600)["verdict"], "ASM-STRUCTURAL")
    eq("verdict: distance=None stays C",
       canonical._verdict("f", [], 8, distance=None)["verdict"], "C")

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

    # register-pin qualifier neutralized, variable kept
    srcp = 'void k(void){ register int v asm("$16"); v = 0; }'
    outp, np = inlineasm.strip_cheat_asm_file(srcp)
    check("strip: pin asm(\"$16\") qualifier removed", 'asm("$16")' not in outp)
    check("strip: pinned variable declaration kept", "register int v" in outp)
    check("strip: pin counted", np >= 1)

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
    # a pure-C function reports 0 (locatable, no cheat asm)
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

    # Wired-into-count check — counts the cheat
    cnt = volatile_cheats.func_volatile_cheat_count(text5, "j")
    eq("addr-coerced: func_volatile_cheat_count includes the coercion", cnt, 1)


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
    if "DispSchoolBG" in tbl:
        eq("classify(DispSchoolBG): GTE -> ASM-PARTIAL",
           canonical.classify("DispSchoolBG")["verdict"], "ASM-PARTIAL")
    if "game_GetMode" in tbl:
        eq("classify(game_GetMode): ordinary C -> C",
           canonical.classify("game_GetMode")["verdict"], "C")
    # no function should be NO-TARGET (the motion_Close-class fix)
    no_target = [r["func"] for r in canonical.scan_all() if r["verdict"] == "NO-TARGET"]
    eq("scan_all: zero NO-TARGET", len(no_target), 0)


def main() -> int:
    test_canonical()
    test_score()
    test_inlineasm()
    test_cheats()
    test_addr_coerced_locals()
    test_dead_conditional_stores()
    test_metrics()
    test_canonical_build()
    print(f"\n{_passed} passed, {_failed} failed, {_skipped} skipped")
    return 1 if _failed else 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
