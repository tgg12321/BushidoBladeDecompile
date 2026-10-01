# func_8002D518 s8 cse evidence, regenerated 2026-09-30 (laneH)

func_8002D518's comment cited `tmp/grind/func_8002D518/s8/dumps_*` (gitignored, lost). These
files regenerate that evidence on today's tree. The comment now cites this directory.

- A = the landed body, with `ud = disc;` written twice. B = the s8 control with a single write,
  control-single-ud.c (= rejected/s8-single-ud-assign-cse-deletes-copy-score3.c from
  0e949509f^). Its `(&D_8008D118)[i]` is spelled `g_sqrt_table_u8[i]` for today's declaration.
- Command: commands.txt. Trees: tmp/laneH/d518dump.py; script: tmp/laneH/d518dump.sh (both
  copied here).
- A.copies.txt: `insn 245` and `insn 255` are `(set (reg 131) (reg 117))`. Both copies survive
  cse, as the comment states.
- B.copies.txt: no 131 <- 117 copy. cse's make_regs_eqv merged ud and disc and deleted it.
- A.s.txt vs B.s.txt: A has `move $4,$6` (target: `addu $a0,$a2,$zero` at 0x8002D680), and the
  LZCS island and slow-path `srl` read `$4`. B has no copy and reads `$6`.
