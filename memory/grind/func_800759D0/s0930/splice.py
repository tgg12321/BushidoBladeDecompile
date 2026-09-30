"""Splice memory/grind/func_800759D0/candidate.c over its INCLUDE_ASM line in src/text1b.c (LF)."""
from pathlib import Path

src = Path("src/text1b.c")
line = 'INCLUDE_ASM("asm/funcs", func_800759D0);\n'
text = src.read_bytes().decode("utf-8")
assert "\r" not in text
assert text.count(line) == 1
body = Path("memory/grind/func_800759D0/candidate.c").read_bytes().decode("utf-8")
assert "\r" not in body
if not body.endswith("\n"):
    body += "\n"
src.write_bytes(text.replace(line, body).encode("utf-8"))
print("spliced", len(body.splitlines()), "lines")
