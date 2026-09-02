#!/usr/bin/env python3
"""Unit tests for tools/sweep_variants.py span finder. Run: python tools/test_sweep_variants.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.sweep_variants import find_function_span


class TestSpan(unittest.TestCase):
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


if __name__ == "__main__":
    unittest.main()
