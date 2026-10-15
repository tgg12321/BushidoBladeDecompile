#!/usr/bin/env python3
"""Unit tests for tools/scan_hand_coded.py's word-level gate signals (S9-S12),
the tiering and the asm parser. Synthetic asm only; no build, no EXE.
Run: python tools/test_scan_hand_coded.py -v"""
import os
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import scan_hand_coded as s  # noqa: E402

R = {n: i for i, n in enumerate(s.REG_NAMES)}


def i_type(op, rs, rt, imm):
    return (op << 26) | (R[rs] << 21) | (R[rt] << 16) | (imm & 0xFFFF)


def r_type(fn, rs="zero", rt="zero", rd="zero", sa=0):
    return (R[rs] << 21) | (R[rt] << 16) | (R[rd] << 11) | (sa << 6) | fn


ADDIU = lambda rt, rs, imm: i_type(0x09, rs, rt, imm)    # noqa: E731
ADDI = lambda rt, rs, imm: i_type(0x08, rs, rt, imm)     # noqa: E731
LW = lambda rt, off, base: i_type(0x23, base, rt, off)   # noqa: E731
SW = lambda rt, off, base: i_type(0x2B, base, rt, off)   # noqa: E731
LHU = lambda rt, off, base: i_type(0x25, base, rt, off)  # noqa: E731
LWL = lambda rt, off, base: i_type(0x22, base, rt, off)  # noqa: E731
LWR = lambda rt, off, base: i_type(0x26, base, rt, off)  # noqa: E731
ADDU = lambda rd, rs, rt: r_type(0x21, rs, rt, rd)       # noqa: E731
SUB = lambda rd, rs, rt: r_type(0x22, rs, rt, rd)        # noqa: E731
MULT = lambda rs, rt: r_type(0x18, rs, rt)               # noqa: E731
MFLO = lambda rd: r_type(0x12, rd=rd)                    # noqa: E731
JR_RA = r_type(0x08, "ra")
JAL = (0x03 << 26) | 0x40000
NOP = 0
RTPS = (0x12 << 26) | (1 << 25) | 0x0180001             # GTE command


def insns(*words):
    return [s.Insn(mnemonic="?", operands="", addr=0x80010000 + 4 * i, word=w)
            for i, w in enumerate(words)]


def asm_file(name, words, labels=None, start=0x80010000, ops=None):
    """Render a splat-style asm/funcs file; labels = {index: label line},
    ops = {index: mnemonic + operands text}."""
    out = [f"glabel {name}"]
    for i, w in enumerate(words):
        if labels and i in labels:
            out.append(labels[i])
        a = start + 4 * i
        out.append(f"    /* {a - 0x80010000 + 0x800:X} {a:08X} "
                   f"{struct.pack('<I', w).hex().upper()} */  {(ops or {}).get(i, 'op $x')}")
    out.append(f"endlabel {name}")
    return "\n".join(out) + "\n"


class TestTrappingArith(unittest.TestCase):
    def test_addi_and_sub_hit(self):
        hit, d = s.signal_trapping_arith(insns(ADDI("s0", "s0", 4), NOP))
        self.assertTrue(hit)
        self.assertIn("addi", d)
        self.assertTrue(s.signal_trapping_arith(insns(SUB("s5", "s5", "t5")))[0])

    def test_non_trapping_forms_clean(self):
        hit, _ = s.signal_trapping_arith(insns(ADDIU("sp", "sp", -8), ADDU("v0", "a0", "a1"),
                                               r_type(0x23, "a0", "a1", "v0"), JR_RA, NOP))
        self.assertFalse(hit)


class TestHazardFreeNop(unittest.TestCase):
    def test_nop_between_addiu_and_jal_hits(self):
        hit, _ = s.signal_hazard_free_nop(insns(ADDIU("s4", "s4", 8), NOP, JAL, NOP, JR_RA, NOP))
        self.assertTrue(hit)

    def test_required_nops_clean(self):
        cases = [
            insns(LW("v0", 0, "a0"), NOP, ADDU("v0", "v0", "v0"), JR_RA, NOP),   # load delay
            insns(ADDIU("a0", "a0", 1), JAL, NOP, JR_RA, NOP),                   # delay slot
            insns(MFLO("a0"), ADDU("s5", "v1", "v0"), NOP, MULT("s5", "s5"),     # HI/LO
                  JR_RA, NOP),
            insns(ADDIU("a0", "a0", 1), NOP, NOP, RTPS, JR_RA, NOP),             # GTE pad
        ]
        for c in cases:
            self.assertFalse(s.signal_hazard_free_nop(c)[0])

    def test_unoptimised_gcc_shapes_exempt(self):
        labelled = insns(ADDIU("a0", "a0", 1), NOP, JAL, NOP, JR_RA, NOP)
        labelled[1].label = ".L80010004"                       # emit_nop at a label
        self.assertFalse(s.signal_hazard_free_nop(labelled)[0])
        fp_frame = insns(ADDU("fp", "sp", "zero"), ADDIU("a0", "a0", 1), NOP, JAL, NOP,
                         JR_RA, NOP)
        self.assertFalse(s.signal_hazard_free_nop(fp_frame)[0])


class TestCustomCalleeSavedAbi(unittest.TestCase):
    def test_live_in_input_register_hits(self):
        hit, d = s.signal_custom_cs_abi(insns(LHU("a0", 0, "s0"), ADDIU("s0", "s0", 2),
                                              JR_RA, NOP))
        self.assertTrue(hit)
        self.assertIn("live-in $s0", d)

    def test_written_never_read_hits(self):
        hit, d = s.signal_custom_cs_abi(insns(
            ADDIU("sp", "sp", -24), SW("s6", 16, "sp"), SW("ra", 20, "sp"),
            ADDU("s6", "fp", "zero"), JAL, NOP,
            LW("ra", 20, "sp"), LW("s6", 16, "sp"), JR_RA, ADDIU("sp", "sp", 24)))
        self.assertTrue(hit)
        self.assertIn("written-never-read $s6", d)

    def test_standard_prologue_clean(self):
        # save; write; outgoing-arg store of the same reg (a use, not a save);
        # restore from the save slot.
        hit, d = s.signal_custom_cs_abi(insns(
            ADDIU("sp", "sp", -32), SW("s1", 24, "sp"), ADDU("s1", "a3", "zero"),
            SW("ra", 28, "sp"), JAL, SW("s1", 16, "sp"),
            LW("ra", 28, "sp"), LW("s1", 24, "sp"), JR_RA, ADDIU("sp", "sp", 32)))
        self.assertFalse(hit, d)

    def test_large_frame_temp_base_save_clean(self):
        # GCC saves through a $sp-derived temp for frames over 32 KB.
        hit, d = s.signal_custom_cs_abi(insns(
            i_type(0x0F, "zero", "t4", 1), r_type(0x23, "sp", "t4", "sp"),
            ADDU("t5", "t4", "sp"), SW("s2", -32, "t5"), ADDU("s2", "a0", "zero"),
            ADDU("v0", "s2", "zero"), JR_RA, NOP))
        self.assertFalse(hit, d)

    def test_lwl_lwr_pair_is_a_write(self):
        hit, d = s.signal_custom_cs_abi(insns(
            ADDIU("sp", "sp", -8), SW("s0", 0, "sp"), LWL("s0", 3, "a0"), LWR("s0", 0, "a0"),
            SW("s0", 0, "a1"), LW("s0", 0, "sp"), JR_RA, ADDIU("sp", "sp", 8)))
        self.assertFalse(hit, d)


class TestParserAndCrossFunctionJtbl(unittest.TestCase):
    def test_jlabel_alabel_are_not_instructions(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "f.s"
            p.write_text(asm_file("f", [NOP, NOP, JR_RA, NOP],
                                  {1: "  jlabel .L80010004", 2: "  alabel D_80010008"}),
                         encoding="utf-8")
            name, ins = s.parse_func(p)
            self.assertEqual(name, "f")
            self.assertEqual(len(ins), 4)
            self.assertTrue(ins[1].jtarget)
            self.assertEqual(ins[1].addr, 0x80010004)
            self.assertEqual(ins[2].word, JR_RA)

    def test_table_entry_in_other_function_hits(self):
        with tempfile.TemporaryDirectory() as td:
            d = Path(td)
            a = asm_file("dispatch", [NOP] * 4 + [JR_RA, NOP], {1: "  jlabel .L80010004"},
                         ops={0: "lw $t0, %lo(jtbl_80020000)($at)"})
            b = asm_file("other", [NOP] * 4 + [JR_RA, NOP], {2: "  jlabel .L80010108"},
                         start=0x80010100)
            (d / "dispatch.s").write_text(a, encoding="utf-8")
            (d / "other.s").write_text(b, encoding="utf-8")
            parsed = [(p, *s.parse_func(p)) for p in sorted(d.glob("*.s"))]
            exe = bytearray(0x800 + 0x10010 + 16)
            struct.pack_into("<III", exe, 0x800 + 0x10000, 0x80010004, 0x80010108, 0x1234)
            (d / "exe").write_bytes(bytes(exe))
            targets, owner = s.load_jtbl_targets(parsed, d / "exe")
            self.assertEqual(targets[0x80020000], [0x80010004, 0x80010108])
            fn = {name: ins for _p, name, ins in parsed}
            hit, det = s.signal_cross_function_jtbl("dispatch", fn["dispatch"], targets, owner)
            self.assertTrue(hit, det)
            self.assertIn("1/2 entries inside other", det)
            local = {0x80020000: [0x80010004]}
            hit, det = s.signal_cross_function_jtbl("dispatch", fn["dispatch"], local, owner)
            self.assertFalse(hit, det)
            hit, det = s.signal_cross_function_jtbl("dispatch", fn["dispatch"], {}, owner)
            self.assertFalse(hit)
            self.assertIn("EXE unavailable", det)


class TestTier(unittest.TestCase):
    def test_new_signals_are_decisive_gate_class(self):
        for sig in ("S9", "S10", "S11"):
            self.assertIn(sig, s.GATE_CLASS)
            tier, reason = s.classify_tier({sig: True}, 1)
            self.assertEqual(tier, "STRONG", sig)
            self.assertIn(sig, reason)

    def test_s12_is_informational(self):
        self.assertNotIn("S12", s.GATE_CLASS)
        self.assertNotIn("S12", s.DECISIVE)
        self.assertEqual(s.classify_tier({"S12": True}, 1)[0], "LOW")

    def test_existing_tiering_unchanged(self):
        self.assertEqual(s.classify_tier({"S1": True}, 1)[0], "POSSIBLE")
        self.assertEqual(s.classify_tier({"S1": True, "S2": True}, 4)[0], "STRONG")
        self.assertEqual(s.classify_tier({}, 3)[0], "TIGHT_C")
        self.assertEqual(s.classify_tier({}, 0)[0], "LOW")
        self.assertEqual(s.classify_tier({"S7": True}, 1)[0], "STRONG")


if __name__ == "__main__":
    unittest.main()
