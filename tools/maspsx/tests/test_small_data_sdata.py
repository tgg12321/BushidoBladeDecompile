"""cc1psx -G8 emits an initialized object of <= 8 bytes (global or static) in `.sdata`; our cc1 emits `.data`.
Under -G8 maspsx moves each such object as a unit (.globl/.align/.type/.size/label/data) to `.sdata`, where it is
the file's own small data (gp); larger objects stay in `.data`; without -G nothing moves (owner rulings Q65/Q68).
SRC is our cc1's real output for the calibration probe (docs/grind/gp-model-2026-09-30.md A.3, static probe);
the expected sections are cc1psx -G8's for the same source."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments

SRC = """	.file	1 "stdin"




	.version	"01.01"
gcc2_compiled.:
	.data
	.align	2
	.type	 si,@object
	.size	 si,4
si:
	.word	5
	.align	2
	.type	 sc,@object
	.size	 sc,3
sc:
	.byte	1
	.byte	2
	.byte	3
	.align	2
	.type	 sd,@object
	.size	 sd,3
sd:
	.byte	4
	.byte	5
	.byte	6
	.align	2
	.type	 sz,@object
	.size	 sz,4
sz:
	.word	0
	.globl	gi
	.align	2
	.type	 gi,@object
	.size	 gi,4
gi:
	.word	7
	.align	2
	.type	 big,@object
	.size	 big,16
big:
	.word	1
	.word	2
	.word	3
	.word	4
	.align	2
	.type	 s8,@object
	.size	 s8,8
s8:
	.string	"abcdefg"
	.align	2
	.type	 s4,@object
	.size	 s4,4
s4:
	.string	"abc"
	.align	2
	.type	 q2,@object
	.size	 q2,2
q2:
	.string	"\\001"
	.align	1
	.type	 hs,@object
	.size	 hs,2
hs:
	.half	3
.section	.rodata
	.align	2
.LC0:
	.string	"xy"
	.data
	.align	2
	.type	 sp8,@object
	.size	 sp8,4
sp8:
	.word	.LC0
	.text
	.align	2
	.globl	f
	.type	 f,@function
	.ent	f
f:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$4,sc+1
	lw	$2,si
	lbu	$3,sd+1
	addu	$2,$2,$4
	addu	$2,$2,$3
	lw	$3,sz
	lw	$4,gi
	addu	$2,$2,$3
	addu	$2,$2,$4
	lw	$3,big+8
	lbu	$4,s8+3
	addu	$2,$2,$3
	addu	$2,$2,$4
	lbu	$3,s4+1
	lbu	$4,q2
	addu	$2,$2,$3
	addu	$2,$2,$4
	lw	$3,sp8
	lh	$4,hs
	lbu	$3,0($3)
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	.end	f
.Lfe1:
	.size	 f,.Lfe1-f
	.ident	"GCC: (GNU) 2.7.2"
""".split("\n")
SMALL = ["si", "sc", "sd", "sz", "gi", "s8", "s4", "q2", "hs", "sp8"]   # cc1psx -G8: .sdata
SECTIONS = (".data", ".text", ".sdata", ".rdata", ".bss", ".sbss")


def run(g):
    return [l for l in strip_comments(MaspsxProcessor(SRC, sdata_limit=g).process_lines()) if l.strip()]


def section_of(res, idx):
    for l in reversed(res[:idx]):
        w = l.split()
        if w and w[0] == ".section":
            return w[1]
        if w and w[0] in SECTIONS:
            return w[0]
    return None


def label_index(res, name):
    return res.index(name + ":")


class TestSmallDataSdata(unittest.TestCase):
    def test_small_objects_are_sdata(self):
        res = run(8)
        for nm in SMALL:
            self.assertEqual(section_of(res, label_index(res, nm)), ".sdata", nm)
        self.assertEqual(section_of(res, label_index(res, "big")), ".data")

    def test_object_moves_as_a_unit(self):
        # sd's own .align/.type/.size come with it: it lands at +4 after the 3-byte sc, as cc1psx's
        res = run(8)
        k = label_index(res, "sd")
        self.assertEqual(res[k - 3:k], [".align\t2", ".type\t sd,@object", ".size\t sd,3"])
        self.assertEqual(section_of(res, k - 3), ".sdata")
        g = res.index(".globl\tgi")
        self.assertEqual(section_of(res, g), ".sdata")

    def test_string_array_and_static_are_gp(self):
        res = run(8)
        self.assertIn("lw\t$2,%gp_rel(si)($gp)", res)       # initialized static
        self.assertIn("lbu\t$4,%gp_rel(s8+3)($gp)", res)    # 8-byte `.string` array
        self.assertIn("lbu\t$3,%gp_rel(sd+1)($gp)", res)    # 3-byte static array, at an offset
        self.assertNotIn("lw\t$3,%gp_rel(big+8)($gp)", res)  # 16 bytes: stays .data

    def test_string_sizes(self):
        p = MaspsxProcessor([], sdata_limit=8)
        self.assertEqual(p._data_size('.string\t"abcdefg"'), 8)
        self.assertEqual(p._data_size('.asciz\t"abc"'), 4)
        self.assertEqual(p._data_size('.ascii\t"abc\\000"'), 4)
        self.assertEqual(p._data_size('.ascii\t"\\001\\n\\\\x"'), 4)

    def test_nothing_moves_without_g(self):
        res = run(0)
        self.assertNotIn(".section .sdata", res)
        self.assertNotIn("lw\t$2,%gp_rel(si)($gp)", res)


if __name__ == "__main__":
    unittest.main()
