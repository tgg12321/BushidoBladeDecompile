#!/usr/bin/env python3
"""Focused tests for migration replacement spans."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from engine import inlineasm
from tools.migrate_include_asm import find_body_span, splice_text


class TestBodySpan(unittest.TestCase):
    def test_shared_line_keeps_previous_closing_brace(self):
        for gap in ("", " \t"):
            with self.subTest(gap=gap):
                prefix = "s32 previous(void) { return 0;\n}" + gap
                body = "s32 f(void) { return 1; }"
                tail = "\ns32 next(void) { return 2; }\n"
                text = prefix + body + tail
                lo, hi = find_body_span(text, "f")
                self.assertEqual(text[lo:hi], body)
                new = splice_text(text, lo, hi, 'INCLUDE_ASM("asm/funcs", f);')
                self.assertEqual(new, "s32 previous(void) { return 0;\n}\n"
                                 'INCLUDE_ASM("asm/funcs", f);' + tail)
                self.assertEqual([s[0] for s in inlineasm.include_asm_spans(new)],
                                 ["f"])
                self.assertEqual(inlineasm.func_definition_candidates(new, "f"), [])

    def test_splice_split_head_replaces_entire_type_line(self):
        for indent in ("", " \t"):
            with self.subTest(indent=indent):
                prefix = "s32 previous(void) {}\n"
                body = indent + "static s32\nf(void) { return 1; }"
                tail = "\ns32 next(void) {}\n"
                text = prefix + body + tail
                lo, hi = find_body_span(text, "f")
                self.assertEqual(text[lo:hi], body)
                new = splice_text(text, lo, hi, 'INCLUDE_ASM("asm/funcs", f);')
                self.assertEqual(new, prefix + 'INCLUDE_ASM("asm/funcs", f);' + tail)
                self.assertEqual([s[0] for s in inlineasm.include_asm_spans(new)],
                                 ["f"])
                self.assertEqual(inlineasm.func_definition_candidates(new, "f"), [])

    def test_splice_rodata_lines_and_tail_newline(self):
        line = ('INCLUDE_RODATA("asm/rodata", jtbl_80010000);\n'
                'INCLUDE_ASM("asm/funcs", f);')
        text = "}s32 f(void) {}s32 g(void) {}\n"
        lo, hi = find_body_span(text, "f")
        self.assertEqual(splice_text(text, lo, hi, line),
                         "}\n" + line + "\ns32 g(void) {}\n")
        plain = "s32 f(void) {}\n"
        lo, hi = find_body_span(plain, "f")
        self.assertEqual(splice_text(plain, lo, hi, 'INCLUDE_ASM("asm/funcs", f);'),
                         'INCLUDE_ASM("asm/funcs", f);\n')

    def test_ordinary_and_split_heads_keep_exclusive_end(self):
        for body in ("s32 f(void) {}", "static s32\nf(void) {}"):
            with self.subTest(body=body):
                self.assertEqual(find_body_span(body + "\n", "f"), (0, len(body)))

    def test_missing_or_ambiguous_body(self):
        self.assertIsNone(find_body_span("int x;\n", "f"))
        self.assertIsNone(find_body_span("s32 f(void) {}\ns32 f(void) {}\n", "f"))


if __name__ == "__main__":
    unittest.main()
