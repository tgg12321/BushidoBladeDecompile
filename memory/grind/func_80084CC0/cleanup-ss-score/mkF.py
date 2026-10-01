"""Final candidate: T9 (_SsSndTempo) + func_80084CC0 naming/comment cleanup -> tmp/laneF/F.c"""
import re

s = open("tmp/laneF/T9.c", encoding="utf-8", newline="").read()
start = s.index("s32 func_80084CC0(s16 a0, s16 a1)")
end = s.index("  end:\n  return ret;\n\n}", start) + len("  end:\n  return ret;\n\n}")
f = s[start:end]

OLD_CMT = f[f.index("  /* 100% pure C"):f.index("  struct SeqStruct *state;")]
NEW_CMT = """  /* Each status-byte arm reads its operand bytes through its own block-local
   * pointer. `velocity` is the Note On velocity (the second data byte) read
   * in both 0x90 arms: after a status byte and under running status. It is
   * one value with one meaning on two exclusive paths. Splitting it into one
   * local per arm changes the global allocator's order and scores 40
   * (memory/grind/func_80084CC0/cleanup-ss-score/evidence.md). */
"""
f = f.replace(OLD_CMT, NEW_CMT)
f = re.sub(r"\bnext\b", "velocity", f)
f = re.sub(r"\bcp\b", "cmd_ptr", f)
f = f.replace("        u32 data;\n", "        u32 note;\n")
f = re.sub(r"\bdata\b", "note", f)
s = s[:start] + f + s[end:]
open("tmp/laneF/F.c", "w", encoding="utf-8", newline="").write(s)
print(f)
