import unittest

from maspsx import MaspsxProcessor
from maspsx import parse_load_or_store


from .util import strip_comments


class TestEmptyOffset(unittest.TestCase):
    def test_parse_empty_offset(self):
        """
        `($12)` (no offset) parses exactly like `0($12)`; PsyQ inline_o.h
        writes GTE loads/stores that way (e.g. gte_ldv0 `lwc2 $0,($12)`).
        """
        self.assertEqual(
            parse_load_or_store("$0,0($12)"), parse_load_or_store("$0,($12)")
        )
        self.assertEqual(
            parse_load_or_store("$25,0($12)"), parse_load_or_store("$25,($12)")
        )

    def test_empty_offset_passed_through(self):
        """
        The line is emitted unchanged (GNU as assembles `($12)` as `0($12)`).
        """
        lines = [
            "	lwc2	$0,($12)",
            "	lwc2	$1,4($12)",
            "	swc2	$25,($12)",
        ]
        mp = MaspsxProcessor(lines)
        res = mp.process_lines()

        clean_lines = [l for l in strip_comments(res) if l]
        self.assertEqual(
            ["lwc2\t$0,($12)", "lwc2\t$1,4($12)", "swc2\t$25,($12)"], clean_lines
        )

    def test_empty_offset_load_delay_nop(self):
        """
        A load with an empty offset still gets its load-delay nop when the
        next instruction reads the loaded register.
        """
        lines = [
            "	lw	$13,($12)",
            "	addu	$2,$13,$4",
        ]
        mp = MaspsxProcessor(lines)
        res = mp.process_lines()

        clean_lines = [l for l in strip_comments(res) if l]
        self.assertEqual(["lw\t$13,($12)", "nop", "addu\t$2,$13,$4"], clean_lines)
