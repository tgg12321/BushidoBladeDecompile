#!/usr/bin/env python3
"""Tests for tools/const_wave.py (no toolchain needed: cpp / cc1 are faked)."""
from __future__ import annotations

import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import const_wave as cw  # noqa: E402

HDR = "kind\twhere\tliteral\tname\trole\tevidence\n"

HEADER_H = """#ifndef PAD_H
#define PAD_H

void PadInit(int mode);

#endif
"""

GAME_C = """#include "pad.h"

int pad_buf;
int mode;

int f(void) {
    if (pad_buf & 0x10) { /* 0x10 in a comment */
        return 16;
    }
    __asm__("li $2, 0x20");
    puts("0x10");
    return mode == 3;
}
"""


def manifest(*rows: str) -> list[cw.Row]:
    return cw.parse_manifest(HDR + "".join(r + "\n" for r in rows))


def git(root, *args):
    return subprocess.run(["git", *args], cwd=root, check=True, capture_output=True,
                          text=True).stdout.strip()


def fake_cpp(tree: cw.Tree, tu: str) -> str:
    """Tiny cpp: #include "x.h" from include/, object-like #defines, enum text kept."""
    defs: dict[str, str] = {}
    out: list[str] = []

    def run(path: str):
        text = cw.blank_comments(tree.read(path) or "")
        out.append(f'# 1 "{path}"')
        for line in text.split("\n"):
            s = line.strip()
            m = re.match(r'#\s*include\s*[<"]([^>"]+)[>"]', s)
            if m:
                name = m.group(1)
                run(name[3:] if name.startswith("../") else "include/" + name)
                out.append(f'# 2 "{path}"')
                continue
            m = re.match(r"#\s*define\s+(\w+)\s+(.*)", s)
            if m:
                defs[m.group(1)] = m.group(2)
                out.append("")
                continue
            if s.startswith("#"):
                out.append("")
                continue
            out.append(re.sub(r"\b\w+\b", lambda k: defs.get(k.group(0), k.group(0)), line))

    run(tu)
    return "\n".join(out) + "\n"


def fake_cc1(tree, tu, source):
    """'Compiles' by folding enum members to values: equal output == same code."""
    vals = dict(re.findall(r"(\w+)\s*=\s*(\w+)\s*,", source))
    body = re.sub(r"enum\s+\w+\s*\{[^}]*\}\s*;", "", source)
    return " ".join(vals.get(t, t) for t in cw.cpp_tokens(body))


class Repo(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="const-wave-test-"))
        self.old_root = cw.ROOT
        cw.ROOT = self.tmp
        (self.tmp / "include").mkdir()
        (self.tmp / "src").mkdir()
        (self.tmp / "tmp").mkdir()
        self.write("include/pad.h", HEADER_H)
        self.write("src/game.c", GAME_C)
        self.write("src/other.c", "int g(void) { return 0x10; }\n")
        self.write("Makefile", "all:\n")
        (self.tmp / ".gitignore").write_text("tmp/\n")
        git(self.tmp, "init", "-q")
        git(self.tmp, "-c", "user.name=t", "-c", "user.email=t@t", "add", "-A")
        git(self.tmp, "-c", "user.name=t", "-c", "user.email=t@t", "commit", "-qm", "base")
        self.base = git(self.tmp, "rev-parse", "HEAD")

    def tearDown(self):
        cw.ROOT = self.old_root
        shutil.rmtree(self.tmp, ignore_errors=True)

    def write(self, path, text):
        (self.tmp / path).parent.mkdir(parents=True, exist_ok=True)
        (self.tmp / path).write_text(text, encoding="utf-8", newline="\n")

    def read(self, path):
        p = self.tmp / path
        return p.read_text(encoding="utf-8") if p.is_file() else None

    def apply(self, rows):
        return cw.apply(rows, self.read, self.write)

    def check(self, rows, loose=False, new=None, base=None):
        return cw.check(rows, base or self.base, new, loose, log=lambda *_: None,
                        cpp=fake_cpp, cc1=fake_cc1)

    def commit(self, msg="wave"):
        git(self.tmp, "-c", "user.name=t", "-c", "user.email=t@t", "commit", "-qam", msg)
        return git(self.tmp, "rev-parse", "HEAD")


PAD_ROWS = ("site\tsrc/game.c:7\t0x10\tPADLup\tpad bit\tlibetc.h",
            "define\tinclude/pad.h\t0x10\tPADLup\tpad bit\tlibetc.h")


class TestManifest(unittest.TestCase):
    def test_header_required(self):
        with self.assertRaises(cw.Refuse):
            cw.parse_manifest("site\tsrc/a.c:1\t1\tX\tr\te\n")

    def test_refuses_asm_and_inc_files(self):
        for path in ("asm/funcs/x.s", "src/a.inc"):
            with self.assertRaises(cw.Refuse):
                manifest(f"site\t{path}:1\t1\tX\tr\te", "define\tinclude/a.h\t1\tX\tr\te")

    def test_refuses_non_literal_site(self):
        for lit in ("FOO", "-1", "1+1", '"s"'):
            with self.assertRaises(cw.Refuse):
                manifest(f"site\tsrc/a.c:1\t{lit}\tX\tr\te", "define\tinclude/a.h\t1\tX\tr\te")

    def test_site_needs_definition_and_value(self):
        with self.assertRaises(cw.Refuse):
            manifest("site\tsrc/a.c:1\t1\tX\tr\te")
        with self.assertRaises(cw.Refuse):
            manifest("site\tsrc/a.c:1\t16\tX\tr\te", "define\tinclude/a.h\t0x20\tX\tr\te")
        manifest("site\tsrc/a.c:1\t16\tX\tr\te", "define\tinclude/a.h\t0x10\tX\tr\te")

    def test_role_and_evidence_required(self):
        with self.assertRaises(cw.Refuse):
            manifest("define\tinclude/a.h\t1\tX\t\te")

    def test_needs_cc1(self):
        rows = manifest("define\tinclude/a.h\t0x1000\tA\tr\te",
                        "define\tinclude/a.h\t(1 << 12)\tB\tr\te",
                        "enum\tinclude/a.h\t3\tMode.M_VS\tr\te")
        d = cw.definitions(rows)
        self.assertEqual([cw.needs_cc1(d[k]) for k in ("A", "B", "M_VS")], [False, True, True])


class TestApply(Repo):
    def test_apply_substitutes_and_defines(self):
        changed = self.apply(manifest(*PAD_ROWS))
        self.assertEqual(sorted(changed), ["include/pad.h", "src/game.c"])
        game = self.read("src/game.c")
        self.assertIn("if (pad_buf & PADLup) { /* 0x10 in a comment */", game)
        self.assertIn('puts("0x10");', game)
        self.assertIn("#define PADLup 0x10\n\n#endif", self.read("include/pad.h"))

    def test_refusals_change_nothing(self):
        cases = {
            "string": "site\tsrc/game.c:11\t0x10\tPADLup\tr\te",
            "asm": "site\tsrc/game.c:10\t0x20\tPADLup\tr\te",
            "absent": "site\tsrc/game.c:8\t0x10\tPADLup\tr\te",
            "directive": "site\tinclude/pad.h:1\t0x10\tPADLup\tr\te",
        }
        self.write("src/game.c", GAME_C.replace("li $2, 0x20", "li $2, %0\" :: \"i\"(0x20)"))
        for why, site in cases.items():
            before = self.read("src/game.c")
            defn = "define\tinclude/pad.h\t0x10\tPADLup\tr\te"
            if why == "asm":
                defn = "define\tinclude/pad.h\t0x20\tPADLup\tr\te"
            with self.subTest(why), self.assertRaises(cw.Refuse):
                self.apply(manifest(site, defn))
            self.assertEqual(self.read("src/game.c"), before)

    def test_refuses_site_in_define(self):
        self.write("include/pad.h", HEADER_H.replace("void", "#define OLD 0x10\nvoid"))
        with self.assertRaises(cw.Refuse):
            self.apply(manifest("site\tinclude/pad.h:4\t0x10\tPADLup\tr\te",
                                "define\tinclude/pad.h\t0x10\tPADLup\tr\te"))

    def test_refuses_non_unique_on_line(self):
        self.write("src/other.c", "int g(int x) { return x & 0x10 ? 0x10 : 0; }\n")
        with self.assertRaises(cw.Refuse):
            self.apply(manifest("site\tsrc/other.c:1\t0x10\tPADLup\tr\te",
                                "define\tinclude/pad.h\t0x10\tPADLup\tr\te"))

    def test_existing_define_reused_or_refused(self):
        self.write("include/pad.h", HEADER_H.replace("void", "#define PADLup 0x10\nvoid"))
        self.apply(manifest(*PAD_ROWS))
        self.assertEqual(self.read("include/pad.h").count("PADLup"), 1)
        self.write("include/pad.h", HEADER_H.replace("void", "#define PADLup 0x20\nvoid"))
        with self.assertRaises(cw.Refuse):
            self.apply(manifest("define\tinclude/pad.h\t0x10\tPADLup\tr\te"))

    def test_enum_block(self):
        self.apply(manifest("site\tsrc/game.c:12\t3\tGAME_MODE_VS\tmode\ttrace",
                            "enum\tsrc/game.c\t3\tGameMode.GAME_MODE_VS\tmode\ttrace"))
        game = self.read("src/game.c")
        self.assertIn('#include "pad.h"\n\nenum GameMode {\n    GAME_MODE_VS = 3,\n};\n\nint', game)
        self.assertIn("return mode == GAME_MODE_VS;", game)


class TestCheck(Repo):
    def test_define_wave_proves_by_cpp(self):
        self.apply(manifest(*PAD_ROWS))
        self.assertEqual(self.check(manifest(*PAD_ROWS)), [])
        new = self.commit()
        self.assertEqual(self.check(manifest(*PAD_ROWS), new=new), [])

    def test_unexplained_edit_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("src/game.c", self.read("src/game.c").replace("return 16;", "return 17;"))
        self.assertTrue(any("not explained" in p for p in self.check(rows)))

    def test_unlisted_replacement_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("src/other.c", "int g(void) { return PADLup; }\n")
        self.assertTrue(self.check(rows))

    def test_missing_site_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(manifest(PAD_ROWS[1]))
        self.assertTrue(any("matched 0" in p for p in self.check(rows)))

    def test_extra_inserted_code_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("include/pad.h", self.read("include/pad.h").replace(
            "#define PADLup 0x10", "#define PADLup 0x10\nint sneaky;"))
        self.assertTrue(any("inserted line" in p for p in self.check(rows)))

    def test_define_with_wrong_body_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("include/pad.h", self.read("include/pad.h").replace("PADLup 0x10", "PADLup 0x11"))
        self.assertTrue(self.check(rows))

    def test_cpp_change_without_cc1_reason_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        # a hidden macro collision: same tokens in the diff, different expansion
        self.write("src/other.c", "#define PADLup 0x99\n" + self.read("src/other.c"))
        problems = self.check(rows)
        self.assertTrue(problems)

    def test_enum_wave_proves_by_cc1(self):
        rows = manifest("site\tsrc/game.c:12\t3\tGAME_MODE_VS\tmode\ttrace",
                        "enum\tsrc/game.c\t3\tGameMode.GAME_MODE_VS\tmode\ttrace")
        self.apply(rows)
        self.assertEqual(self.check(rows), [])

    def test_enum_wave_wrong_value_fails_cc1(self):
        rows = manifest("site\tsrc/game.c:12\t3\tGAME_MODE_VS\tmode\ttrace",
                        "enum\tsrc/game.c\t3\tGameMode.GAME_MODE_VS\tmode\ttrace")
        self.apply(rows)
        self.write("src/game.c", self.read("src/game.c").replace("GAME_MODE_VS = 3", "GAME_MODE_VS = 4"))
        self.assertTrue(self.check(rows))

    def test_shifted_lines_need_loose(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        new = self.commit()
        # replay onto a main where another commit added lines above the site
        git(self.tmp, "checkout", "-q", self.base)
        self.write("src/game.c", "/* moved */\n\n" + self.read("src/game.c"))
        moved = self.commit("other")
        git(self.tmp, "cherry-pick", "-n", new)
        replayed = self.commit("replayed")
        self.assertTrue(self.check(rows, new=replayed, base=moved))
        self.assertEqual(self.check(rows, loose=True, new=replayed, base=moved), [])

    def test_loose_still_counts(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("src/game.c", self.read("src/game.c").replace("return 16;", "return PADLup;"))
        self.assertTrue(self.check(rows, loose=True))

    def test_non_source_change_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("Makefile", "all:\n\techo\n")
        self.assertTrue(any("Makefile" in p for p in self.check(rows)))

    def test_linker_script_or_tool_change_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        for path, stage in (("bb2.ld", False), ("gp_files.txt", False), ("src/new.c", False),
                            ("tools/x.py", True), ("Makefile", False)):
            with self.subTest(path):
                before = self.read(path)
                self.write(path, (before or "") + "x\n")
                if stage:
                    git(self.tmp, "add", path)
                self.assertTrue(any(path in p for p in self.check(rows)))
                if stage:
                    git(self.tmp, "rm", "-q", "--cached", path)
                if before is None:
                    (self.tmp / path).unlink()
                else:
                    self.write(path, before)
        # untracked junction-like directories outside the source trees are not wave edits
        self.write("tools/gcc/cc1", "bin\n")
        self.write(".venv/bin/python", "bin\n")
        self.write("docs/const.tsv", "x\n")
        self.assertEqual(self.check(rows), [])
        # the committed wave is checked whole: a committed tool file fails
        git(self.tmp, "add", "tools/gcc/cc1")
        self.assertTrue(any("tools/gcc/cc1" in p for p in self.check(rows, new=self.commit())))

    def test_added_include_must_be_a_tracked_header(self):
        rows = manifest("site\tsrc/game.c:12\t3\tGAME_MODE_VS\tmode\ttrace",
                        "enum\tinclude/empty.h\t3\tGameMode.GAME_MODE_VS\tmode\ttrace")
        self.write("include/empty.h", "/* no declarations: the fake cpp has no include guards */\n")
        self.write("include/other.h", "/* tracked, but no row of the wave lives here */\n")
        git(self.tmp, "add", "include/empty.h", "include/other.h")
        self.base = self.commit("empty header")
        self.apply(rows)
        applied = self.read("src/game.c")
        outside = self.tmp.parent / f"{self.tmp.name}-evil.h"
        outside.write_text("\n")
        try:
            for inc, ok in (('"empty.h"', True), ("<empty.h>", True), ('"nope.h"', False),
                            (f'"{outside.as_posix()}"', False), ('"../include/pad.h"', False),
                            ('"tmp/x.h"', False), ('"other.h"', False)):
                with self.subTest(inc):
                    self.write("src/game.c", applied.replace(
                        '#include "pad.h"\n', f'#include "pad.h"\n#include {inc}\n', 1))
                    problems = self.check(rows)
                    self.assertEqual(problems == [], ok, problems)
        finally:
            outside.unlink()

    def test_spliced_comment_cannot_hide_code(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied = self.read("src/game.c")
        for hidden in ('/* note *\\\n/\n#include "/abs/evil.h"\n/* end */\n',
                       '// note \\\nint sneaky;\n', "/* x *??/\n/\nint y;\n"):
            with self.subTest(hidden):
                self.write("src/game.c", applied.replace("int pad_buf;", hidden + "int pad_buf;"))
                self.assertTrue(any("splice" in p for p in self.check(rows)))

    def test_comment_cannot_extend_a_directive(self):
        rows = manifest(PAD_ROWS[1])
        base_h = self.read("include/pad.h")
        for edit in ('#include "pad.h" /* a note\n */ void PadInit(int mode);',
                     "#define PADLup 0x10 /* a note\n */ void PadInit(int mode);"):
            with self.subTest(edit):
                self.write("include/pad.h", base_h.replace("void PadInit(int mode);", edit))
                self.assertTrue(self.check(rows))
        # the bare definition is fine; any added comment is not (a wave never touches one)
        self.write("include/pad.h", base_h.replace("#endif", "#define PADLup 0x10\n\n#endif"))
        self.assertEqual(self.check(rows), [])
        self.write("include/pad.h", base_h.replace(
            "#endif", "/* pad\n * bits */\n#define PADLup 0x10 /* up */\n\n#endif"))
        self.assertTrue(any("comment" in p for p in self.check(rows)))

    def test_cpp_reading_an_untracked_file_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)

        def leaky_cpp(tree, tu):
            out = fake_cpp(tree, tu)
            return out + '# 1 "/abs/evil.h"\n' if tree.rev is None else out
        problems = cw.check(rows, self.base, None, False, log=lambda *_: None,
                            cpp=leaky_cpp, cc1=fake_cc1)
        self.assertTrue(any("outside the tracked tree" in p for p in problems), problems)

    def test_lone_cr_is_a_newline(self):
        # cpp ends a // comment at a lone CR, so the code after it is live
        self.assertIn("sneaky", [t for _, t, _ in cw.units(cw.cpp_text("// a\rint sneaky;\n"))])
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied = self.read("src/game.c")
        for hidden in ("// note\rint sneaky;\n", "/* a */\r\n"):
            with self.subTest(hidden):
                self.write("src/game.c", applied.replace("int pad_buf;", hidden + "int pad_buf;"))
                self.assertTrue(any("carriage return" in p for p in self.check(rows)))

    def test_lone_cr_in_base_read_as_cpp(self):
        # a base file already holding `// x<CR>code`: the code is code on both sides
        rows = self.wave_over_base_insert("// x\r")
        self.assertEqual(self.check(rows), [])
        raw = (self.tmp / "src/game.c").read_bytes().decode()
        self.write("src/game.c", raw.replace("\rreturn", "\rreturn 1 +"))
        problems = self.check(rows)
        # flagged as code on its own line 13 (after the CR), not hidden in the comment
        self.assertTrue(any("game.c:13:" in p and "return 1 +" in p for p in problems), problems)

    def wave_over_base_insert(self, insert):
        """Base: game.c with `insert` before its last return (after the site line); new: the
        PAD wave applied to it (apply refuses CR files, so the insert goes in afterwards)."""
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied_c, applied_h = self.read("src/game.c"), self.read("include/pad.h")
        self.write("src/game.c", GAME_C.replace("return mode", insert + "return mode"))
        self.write("include/pad.h", HEADER_H)
        self.base = self.commit("base insert")
        self.write("src/game.c", applied_c.replace("return mode", insert + "return mode"))
        self.write("include/pad.h", applied_h)
        return rows

    def test_backslash_blank_newline_is_a_splice(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied = self.read("src/game.c")
        for hidden in ("// note \\ \nint sneaky;\n", "// note \\\t\nint sneaky;\n"):
            with self.subTest(hidden):
                self.write("src/game.c", applied.replace("int pad_buf;", hidden + "int pad_buf;"))
                self.assertTrue(any("splice" in p for p in self.check(rows)))

    def test_misread_splice_in_base_refused(self):
        # an existing splice the reader cannot follow (comment, string, code line) refuses the
        # file; one inside a directive's code is read as cpp reads it
        for text, bad in (("// a \\ \nint x;\n", [1]), ('char *s = "a\\\nb";\n', [1]),
                          ("int x = 1\\\n0;\n", [1]), ("#define A 1 \\ \n  + 2\nint x;\n", []),
                          ("#define A 1 /* c \\\n */\n", [1])):
            with self.subTest(text):
                self.assertEqual(cw.misread_splices(text), bad)
        rows = self.wave_over_base_insert("// a \\\n")
        self.assertTrue(any("cannot read the file as cpp" in p for p in self.check(rows)))

    def test_directive_read_as_cpp_joins_it(self):
        # a splice joins with nothing (0x1\<NL>0 is 0x10), blanks between tokens count, and
        # string literals keep their inner whitespace: each edit below changes cpp's view
        base_h = HEADER_H.replace("#endif", '#define SCALE 0x1\\\n0\n#define MODE a b\n'
                                  '#define MSG "a  b"\n\n#endif')
        self.write("include/pad.h", base_h)
        self.base = self.commit("macros")
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied = self.read("include/pad.h")
        self.assertEqual(self.check(rows), [])
        for old, new in (("0x1\\\n0", "0x1\\\n 0"),
                         ("0x1\\\n0\n#define MODE a b", "0x1 0\n#define MODE a\\\nb"),
                         ('"a  b"', '"a b"')):
            with self.subTest(new):
                self.write("include/pad.h", applied.replace(old, new))
                self.assertTrue(self.check(rows))
        self.assertEqual(cw.directive_key("#  define X 0x1" + "0"), "#define X 0x10")
        self.assertNotEqual(cw.directive_key("#define X a+b"), cw.directive_key("#define X a + b"))

    def test_comment_change_fails(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied = self.read("src/game.c")
        for new in ("/* rewritten */", "", "/* 0x10 in a comment */ /* FAKE */"):
            with self.subTest(new):
                self.write("src/game.c", applied.replace("/* 0x10 in a comment */", new))
                self.assertTrue(any("comment" in p for p in self.check(rows)))
        # a moved comment too
        self.write("src/game.c", applied.replace(" /* 0x10 in a comment */", "").replace(
            "int mode;", "int mode; /* 0x10 in a comment */"))
        self.assertTrue(self.check(rows))

    def test_reflowed_line_comment_is_fine(self):
        # clang-format (ReflowComments, 80 columns) splits a trailing // comment that a longer
        # NAME pushes past the limit: the same words, so no false FAIL
        self.write("src/game.c", GAME_C.replace(
            "if (pad_buf & 0x10) { /* 0x10 in a comment */",
            "if (pad_buf & 0x10) { // the pad bit tested by the menu loop here ok"))
        self.base = self.commit("line comment")
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("src/game.c", self.read("src/game.c").replace(
            "// the pad bit tested by the menu loop here ok",
            "// the pad bit tested by the menu loop\n                         // here ok"))
        self.assertEqual(self.check(rows), [])
        self.write("src/game.c", self.read("src/game.c").replace("// here ok", "// here OK"))
        self.assertTrue(any("here OK" in p for p in self.check(rows)))

    def test_enum_block_must_be_contiguous(self):
        # `enum Tag {` and `};` elsewhere, a member slipped into a live enum: refused
        # (the reviewer's case: open and close in dead #if blocks around a live enum)
        base = GAME_C.replace("int mode;", "int mode;\n#if 0\n#endif\nenum Old {\n    OLD_A,\n"
                              "    OLD_B,\n};\n#if 0\n#endif")
        self.write("src/game.c", base)
        self.base = self.commit("old enum")
        rows = manifest("site\tsrc/game.c:20\t3\tGAME_MODE_VS\tmode\ttrace",
                        "enum\tsrc/game.c\t3\tGameMode.GAME_MODE_VS\tmode\ttrace")
        head, sep, tail = base.rpartition("#if 0\n")
        sneaky = (head.replace("#if 0\n", "#if 0\nenum GameMode {\n", 1)
                  .replace("enum Old {\n", "enum Old {\n    GAME_MODE_VS = 3,\n")
                  + sep + "};\n" + tail.replace("mode == 3", "mode == GAME_MODE_VS"))
        self.write("src/game.c", sneaky)
        self.assertTrue(any("not a manifest definition" in p for p in self.check(rows)))
        self.write("src/game.c", base)
        self.apply(rows)
        self.assertEqual(self.check(rows), [])

    def test_definition_must_be_in_its_named_file(self):
        # NAME already defined (same value) in game.c: the substitution is cpp-neutral, but
        # the row says include/pad.h, which the wave never defines it in
        base = "#define PADLup 0x10\n" + GAME_C
        self.write("src/game.c", base)
        self.base = self.commit("defined elsewhere")
        rows = manifest("site\tsrc/game.c:8\t0x10\tPADLup\tpad bit\tlibetc.h", PAD_ROWS[1])
        self.write("src/game.c", base.replace("pad_buf & 0x10", "pad_buf & PADLup"))
        self.assertTrue(any("does not define PADLup" in p for p in self.check(rows)))

    def test_strict_site_line_holding_the_literal_twice(self):
        # which of the two tokens the row means is not pinned: strict refuses either
        base = GAME_C.replace("int pad_buf;", "int pad_buf;\nint a[16];")
        base = base.replace("if (pad_buf & 0x10) {", "if (a[0x10] = pad_buf & 0x10) {")
        self.write("src/game.c", base)
        self.base = self.commit("twice")
        rows = manifest("site\tsrc/game.c:8\t0x10\tPADLup\tpad bit\tlibetc.h", PAD_ROWS[1])
        self.write("include/pad.h", HEADER_H.replace("#endif", "#define PADLup 0x10\n\n#endif"))
        for new in ("a[PADLup] = pad_buf & 0x10", "a[0x10] = pad_buf & PADLup"):
            with self.subTest(new):
                self.write("src/game.c", base.replace("a[0x10] = pad_buf & 0x10", new))
                self.assertTrue(any("matched 0" in p for p in self.check(rows)))

    def test_enum_block_at_a_declaration_boundary(self):
        base = GAME_C.replace("int mode;", "extern\nint mode;")
        self.write("src/game.c", base)
        self.base = self.commit("extern")
        rows = manifest("site\tsrc/game.c:13\t3\tGAME_MODE_VS\tmode\ttrace",
                        "enum\tsrc/game.c\t3\tGameMode.GAME_MODE_VS\tmode\ttrace")
        self.write("src/game.c", base.replace(
            "extern\nint mode;", "extern\nenum GameMode {\n    GAME_MODE_VS = 3,\n};\nint mode;")
            .replace("mode == 3", "mode == GAME_MODE_VS"))
        self.assertTrue(any("not a manifest definition" in p for p in self.check(rows)))

    def test_apply_refuses_a_site_format_would_hoist(self):
        # a trailing comment that the longer NAME pushes past 80 columns would be hoisted above
        # the statement by tools/format.py, which check reads as a moved comment
        line = "    pad_buf |= 0x10; /* FAKE: "
        line += "x" * (79 - len(line) - 3) + " */"
        self.assertEqual(len(line), 79)
        self.write("src/game.c", GAME_C.replace("int f(void) {\n", "int f(void) {\n" + line + "\n"))
        self.base = self.commit("long line")
        rows = manifest("site\tsrc/game.c:7\t0x10\tPADLup\tpad bit\tlibetc.h", PAD_ROWS[1])
        with self.assertRaises(cw.Refuse) as e:
            self.apply(rows)
        self.assertIn("hoist", str(e.exception))
        self.assertEqual(self.read("src/game.c").count("0x10"), 4)  # nothing written
        # one column shorter still fits: applied
        self.write("src/game.c", self.read("src/game.c").replace("x */", " */", 1))
        self.base = self.commit("shorter")
        self.assertIn("src/game.c", self.apply(manifest(
            "site\tsrc/game.c:7\t0x10\tPADLup\tpad bit\tlibetc.h", PAD_ROWS[1])))

    def test_c_definition_goes_above_its_first_use(self):
        # a merged TU with a second include block after code: the define goes after the
        # leading includes, not after the last #include
        late = GAME_C + '\n#include "pad.h"\nint late;\n'
        self.write("src/game.c", late)
        self.base = self.commit("late include")
        rows = manifest("site\tsrc/game.c:7\t0x10\tPADLup\tpad bit\tlibetc.h",
                        "define\tsrc/game.c\t0x10\tPADLup\tpad bit\tlibetc.h")
        self.apply(rows)
        text = self.read("src/game.c")
        self.assertLess(text.index("#define PADLup"), text.index("pad_buf & PADLup"))
        self.assertEqual(self.check(rows), [])
        # a header whose own code uses the name: the define goes after the guard, above it
        self.write("src/game.c", late)
        hdr = HEADER_H.replace("void PadInit(int mode);", "void PadInit(int mode[0x20]);")
        self.write("include/pad.h", hdr)
        self.base = self.commit("header site")
        rows = manifest("site\tinclude/pad.h:4\t0x20\tPAD_N\tn\tx",
                        "define\tinclude/pad.h\t0x20\tPAD_N\tn\tx")
        self.apply(rows)
        self.assertTrue(self.read("include/pad.h").startswith(
            "#ifndef PAD_H\n#define PAD_H\n\n#define PAD_N 0x20\n"))
        self.assertEqual(self.check(rows), [])

    def test_guarded_existing_define_is_reused(self):
        # `#ifndef TRUE / #define TRUE 1 / #endif` (include/common.h): the #ifndef line is not a
        # use, and an existing definition is not placement-checked
        self.write("include/pad.h", HEADER_H.replace(
            "#endif", "#ifndef TRUE\n#define TRUE 1\n#endif\n\n#endif"))
        self.write("src/game.c", GAME_C.replace("int mode;", "int mode = 1;"))
        self.base = self.commit("true")
        rows = manifest("site\tsrc/game.c:4\t1\tTRUE\tboolean\tcommon.h",
                        "define\tinclude/pad.h\t1\tTRUE\tboolean\tcommon.h")
        self.assertEqual(self.apply(rows), ["src/game.c"])
        self.assertEqual(self.check(rows), [])

    def test_leading_slot(self):
        for text, header, want in (
                ('/* top */\n#include "a.h"\n#include "b.h"\nint x;\n#include "c.h"\n', False, 3),
                ("int x;\n", False, 0),
                ('#ifdef X\n#include "a.h"\n#endif\nint x;\n', False, 0),
                ('#include "a.h"\n#define M(a) \\\n  (a)\nint x;\n', False, 1),
                ('#ifndef G\n#define G\n#include "a.h"\n#ifdef Y\n#define Z 1\n#endif\nint x;\n'
                 "#endif\n", True, 3)):
            with self.subTest(text):
                self.assertEqual(cw.leading_slot(text.split("\n"), header), want)

    def test_line_comment_run_stops_at_a_blank_line(self):
        self.assertEqual(len(cw.comments("int a; // one\n// two\n")), 1)
        self.assertEqual(len(cw.comments("int a; // one\n\n// two\n")), 2)

    def test_crash_is_no_verdict(self):
        from unittest.mock import patch
        self.write("docs/m.tsv", HDR + "".join(r + "\n" for r in PAD_ROWS))
        with patch.object(cw, "check", side_effect=RuntimeError("boom")):
            self.assertEqual(cw.main(["check", "docs/m.tsv", "--base", self.base]), 2)

    def test_refuses_added_has_include_nul_ff_vt(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        applied = self.read("src/game.c")
        for add, what in (("#if __has_include(\"pad.h\")\n#endif\n", "__has_include"),
                          ("int\0x;\n", "NUL"), ("int\fx;\n", "form feed"),
                          ("int\vx;\n", "vertical tab")):
            with self.subTest(what):
                self.write("src/game.c", applied.replace("int pad_buf;", add + "int pad_buf;"))
                self.assertTrue(any(what in p for p in self.check(rows)))

    def test_tree_reads_bytes_alike_in_both_modes(self):
        self.write("include/crlf.h", "int a;\r\nint b;\r")
        git(self.tmp, "add", "include/crlf.h")
        rev = self.commit("crlf")
        work, snap = cw.Tree(None), cw.Tree(rev)
        try:
            self.assertEqual(work.read("include/crlf.h"), "int a;\r\nint b;\r")
            self.assertEqual(snap.read("include/crlf.h"), work.read("include/crlf.h"))
            # the snapshot is the whole tracked tree but docs/, as cpp would find it
            self.assertTrue((snap.root() / ".gitignore").is_file())
        finally:
            snap.close()

    def test_every_tu_in_scope(self):
        # a changed .c reached only through another TU's #include is still proved
        self.write("include/frag.c", "int k = 0;\n")
        self.write("src/user.c", '#include "frag.c"\nint u;\n')
        git(self.tmp, "add", "-A")
        self.base = self.commit("frag")
        rows = manifest("site\tinclude/frag.c:1\t0\tK_ZERO\tzero\tx",
                        "define\tinclude/frag.c\t0\tK_ZERO\tzero\tx")
        self.apply(rows)
        logs = []
        problems = cw.check(rows, self.base, None, False, log=logs.append, cpp=fake_cpp,
                            cc1=fake_cc1)
        self.assertEqual(problems, [])
        self.assertIn("  src/user.c: cpp identical", logs)

    def test_tu_reading_changed_docs_fails(self):
        self.write("docs/x.h", "int d;\n")
        self.write("src/doc_user.c", '#include "../docs/x.h"\nint v;\n')
        git(self.tmp, "add", "-A")
        self.base = self.commit("docs include")
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("docs/x.h", "int d2;\n")
        problems = self.check(rows)
        self.assertTrue(any("changed docs/" in p for p in problems), problems)

    def test_paths_with_spaces(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.write("src/a.c docs/x", "int x;\n")
        self.assertTrue(any("src/a.c docs/x" in p for p in self.check(rows)))

    def test_snapshots_removed(self):
        rows = manifest(*PAD_ROWS)
        self.apply(rows)
        self.check(rows, new=self.commit())
        self.assertEqual(list((self.tmp / "tmp").iterdir()), [])


if __name__ == "__main__":
    unittest.main()
