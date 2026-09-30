p = "/tmp/claude-0/lbl/tools/maspsx/maspsx/__init__.py"
s = open(p).read()
old = """        next_instruction = self.get_next_instruction(
            skip=0, ignore_nop=True, ignore_set=True, ignore_label=True
        )
        next_next_instruction = self.get_next_instruction(
            skip=1, ignore_nop=True, ignore_set=True, ignore_label=True
        )
"""
assert s.count(old) == 1
new = old + """
        # An unconditional jump between mflo/mfhi and the mult/div ends the hazard: the mult/div
        # after the jump's target label is not reached from the mflo/mfhi on this path.
        if next_instruction.split("\\t")[0].strip() in ("j", "b", "jr"):
            return res
"""
s = s.replace(old, new)
open(p, "w").write(s)
print("patched")
