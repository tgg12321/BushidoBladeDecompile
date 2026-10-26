#!/usr/bin/env python3
"""Unit tests for tools/sweep_variants.py span finder. Run: python tools/test_sweep_variants.py -v"""
import os, sys, unittest
import contextlib
import io
import json
import tempfile
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.sweep_variants import find_function_span, write_retry
from tools import reviewer_precheck as precheck


class TestWriteRetry(unittest.TestCase):
    """The restore write must ride out DrvFS's transient EINVAL (2026-09-25:
    one refused restore left candidates spliced into src/)."""

    class Flaky:
        def __init__(self, fails):
            self.fails, self.data = fails, None

        def write_bytes(self, data):
            if self.fails:
                self.fails -= 1
                raise OSError(22, "Invalid argument")
            self.data = data

    def test_transient_failure_retried(self):
        p = self.Flaky(3)
        write_retry(p, b"orig", delay=0)
        self.assertEqual(p.data, b"orig")

    def test_persistent_failure_raises(self):
        with self.assertRaises(OSError):
            write_retry(self.Flaky(99), b"orig", attempts=3, delay=0)


class TestSpan(unittest.TestCase):
    def test_split_head(self):
        t = "static s32\nf(void) { return 1; }\n"
        self.assertEqual(find_function_span(t, "f"), (0, len(t)))

    def test_ambiguous_definition_refuses_asm_fallback(self):
        t = 's32\nf(void) {}\ns32 f(void) {}\nINCLUDE_ASM("asm/funcs", f);\n'
        with self.assertRaisesRegex(ValueError, "ambiguous"):
            find_function_span(t, "f")

    def test_c_body(self):
        t = "s32 f(void);\n\ns32 f(void)\n{\n    return 1;\n}\n\nvoid g(void) { f(); }\n"
        s, e = find_function_span(t, "f")
        self.assertEqual(t[s:e], "s32 f(void)\n{\n    return 1;\n}\n")

    def test_include_asm_line(self):
        t = 'void a(void) {}\n\nINCLUDE_ASM("asm/funcs", func_80027640);\n\nvoid b(void) {}\n'
        s, e = find_function_span(t, "func_80027640")
        self.assertEqual(t[s:e], 'INCLUDE_ASM("asm/funcs", func_80027640);\n')

    def test_missing_raises(self):
        with self.assertRaises(ValueError):
            find_function_span("int x;\n", "nope")


class TestPrecheckAmbiguity(unittest.TestCase):
    ambiguous = 'int f(void) {}\nint f(void) {}\nINCLUDE_ASM("asm/funcs", f);\n'

    def test_unscanned_body_flags_json_and_text(self):
        for stem in ("missing", "/outside/f", r"C:\outside\f", r"\outside\f"):
            for json_mode in (False, True):
                with self.subTest(stem=stem, json_mode=json_mode):
                    output = io.StringIO()
                    argv = ["precheck", "--func", "f", "--file", stem]
                    if json_mode:
                        argv.append("--json")
                    with patch.object(sys, "argv", argv), \
                            patch.object(precheck, "function_body", return_value=None), \
                            patch.object(precheck, "rule_lines", return_value={}), \
                            patch.object(precheck, "diff_files", return_value=[]), \
                            patch.object(precheck, "diff_text", return_value=""), \
                            contextlib.redirect_stdout(output):
                        self.assertEqual(precheck.main(), 0)
                    text = output.getvalue()
                    if json_mode:
                        report = json.loads(text)
                        self.assertEqual(report["mechanical_flags"], [
                            "function body not scanned: " + report["checks"]["body"]["note"]])
                    else:
                        self.assertIn("MECHANICAL FLAGS", text)
                        self.assertIn("function body not scanned:", text)
                        self.assertNotIn("mechanical checks clean", text)

    def test_absolute_and_drive_stems_use_missing_body_contract(self):
        reports = []
        for stem in ("missing", "/outside/f", r"C:\outside\f", r"C:outside\f",
                     r"\outside\f", r"\\server\share\f"):
            with self.subTest(stem=stem):
                output = io.StringIO()
                with patch.object(sys, "argv", ["precheck", "--func", "f",
                                                 "--file", stem, "--json"]), \
                        patch.object(Path, "exists", lambda p: p.suffix == ".c"
                                     and p.stem != "missing"), \
                        patch.object(Path, "read_text", return_value="int f(void) {}\n"), \
                        patch.object(precheck, "rule_lines", return_value={}), \
                        patch.object(precheck, "diff_files", return_value=[]), \
                        patch.object(precheck, "diff_text", return_value=""), \
                        contextlib.redirect_stdout(output):
                    self.assertEqual(precheck.main(), 0)
                report = json.loads(output.getvalue())
                self.assertIsNone(report["checks"]["body"]["file"])
                if stem == "missing":
                    self.assertIn("function body not found", report["checks"]["body"]["note"])
                else:
                    self.assertEqual(report["checks"]["body"]["note"],
                                     "absolute --file not accepted; pass a src-relative stem")
                reports.append(report)
        self.assertTrue(all(report == reports[1] for report in reports[1:]))

    def test_real_outside_file_refuses_absolute_and_rooted_stems(self):
        (precheck.ROOT / "tmp").mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=precheck.ROOT / "tmp") as td:
            outside = Path(td) / "f.c"
            outside.write_text("int f(void) {}\n", encoding="utf-8")
            root = Path(td) / "repo"
            (root / "src").mkdir(parents=True)
            stem = str(outside.with_suffix(""))
            stems = [stem]
            if os.name == "nt":
                stems.append(os.path.splitdrive(stem)[1])
            with patch.object(precheck, "ROOT", root):
                for stem in stems:
                    with self.subTest(stem=stem):
                        self.assertTrue(Path(stem + ".c").is_file())
                        self.assertIsNone(precheck.function_body("f", stem))

    def test_commit_rejects_absolute_and_drive_stems_before_read(self):
        for stem in ("/outside/f", r"C:\outside\f", r"C:outside\f",
                     r"\outside\f", r"\\server\share\f"):
            with self.subTest(stem=stem), patch.object(
                    precheck, "read_text_at_commit", return_value="int f(void) {}\n") as read:
                self.assertIsNone(precheck.function_body("f", stem, "candidate"))
                read.assert_not_called()

    def test_commit_ambiguity_does_not_fall_through(self):
        with patch.object(precheck, "list_src_files_at_commit", return_value=["src/a.c", "src/b.c"]), \
                patch.object(precheck, "read_text_at_commit",
                             side_effect=[self.ambiguous, "int f(void) {}\n"]):
            with self.assertRaisesRegex(precheck.AmbiguousBody, "src/a.c: ambiguous definitions of f"):
                precheck.function_body("f", None, "candidate")

    def test_working_tree_ambiguity(self):
        with patch.object(Path, "exists", return_value=True), \
                patch.object(Path, "read_text", return_value=self.ambiguous):
            with self.assertRaisesRegex(precheck.AmbiguousBody, "ambiguous definitions of f"):
                precheck.function_body("f", "main/a")

    def test_commit_reports_all_ambiguous_files_in_json(self):
        for sources in (
                [self.ambiguous, self.ambiguous],
                ["int f(void) {}\n", self.ambiguous, self.ambiguous],
                [self.ambiguous, "int f(void) {}\n", self.ambiguous]):
            with self.subTest(sources=sources):
                paths = [f"src/{i}.c" for i in range(len(sources))]
                expected = [f"{path}: ambiguous definitions of f"
                            for path, text in zip(paths, sources)
                            if text == self.ambiguous]
                output = io.StringIO()
                with patch.object(sys, "argv", ["precheck", "--func", "f",
                                                 "--commit", "candidate", "--json"]), \
                        patch.object(precheck, "list_src_files_at_commit", return_value=paths), \
                        patch.object(precheck, "read_text_at_commit", side_effect=sources), \
                        patch.object(precheck, "rule_lines", return_value={}), \
                        patch.object(precheck, "diff_files", return_value=[]), \
                        patch.object(precheck, "diff_text", return_value=""), \
                        contextlib.redirect_stdout(output):
                    self.assertEqual(precheck.main(), 0)
                report = json.loads(output.getvalue())
                self.assertIsNone(report["checks"]["body"]["file"])
                self.assertEqual(report["checks"]["body"]["note"], "\n".join(expected))
                self.assertEqual(report["mechanical_flags"], expected)

    def test_working_tree_reports_all_ambiguous_files_in_json(self):
        for sources in (
                [self.ambiguous, self.ambiguous],
                ["int f(void) {}\n", self.ambiguous, self.ambiguous],
                [self.ambiguous, "int f(void) {}\n", self.ambiguous],
                [self.ambiguous, self.ambiguous, "int f(void) {}\n"]):
            with self.subTest(sources=sources):
                paths = [precheck.ROOT / "src" / f"{i}.c"
                         for i in range(len(sources))]
                expected = [f"{path.relative_to(precheck.ROOT)}: ambiguous definitions of f"
                            for path, text in zip(paths, sources)
                            if text == self.ambiguous]
                output = io.StringIO()
                with patch.object(sys, "argv", ["precheck", "--func", "f", "--json"]), \
                        patch.object(Path, "rglob", return_value=paths), \
                        patch.object(Path, "exists", lambda p: p in paths), \
                        patch.object(Path, "read_text", side_effect=sources), \
                        patch.object(precheck, "rule_lines", return_value={}), \
                        patch.object(precheck, "diff_files", return_value=[]), \
                        patch.object(precheck, "diff_text", return_value=""), \
                        contextlib.redirect_stdout(output):
                    self.assertEqual(precheck.main(), 0)
                report = json.loads(output.getvalue())
                self.assertIsNone(report["checks"]["body"]["file"])
                self.assertEqual(report["checks"]["body"]["note"], "\n".join(expected))
                self.assertEqual(report["mechanical_flags"], expected)

    def test_main_reports_ambiguity_distinctly(self):
        output = io.StringIO()
        with patch.object(sys, "argv", ["precheck", "--func", "f", "--json"]), \
                patch.object(precheck, "function_body", side_effect=precheck.AmbiguousBody(
                    "src/a.c: ambiguous definitions of f")), \
                patch.object(precheck, "rule_lines", return_value={}), \
                patch.object(precheck, "diff_files", return_value=[]), \
                patch.object(precheck, "diff_text", return_value=""), \
                contextlib.redirect_stdout(output):
            self.assertEqual(precheck.main(), 0)
        report = json.loads(output.getvalue())
        self.assertEqual(report["checks"]["body"]["note"], "src/a.c: ambiguous definitions of f")
        self.assertEqual(report["mechanical_flags"], ["src/a.c: ambiguous definitions of f"])

    def test_missing_body_still_returns_none(self):
        with patch.object(precheck, "read_text_at_commit", return_value="int x;\n"):
            self.assertIsNone(precheck.function_body("f", "main/a", "candidate"))


if __name__ == "__main__":
    unittest.main()
