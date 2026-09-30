"""Replace the CD_cw section in src/system.c (helpers + definition) with memory/grind/CD_cw/candidate.c."""
from pathlib import Path
s = Path("src/system.c").read_bytes().decode("utf-8")
body = Path("memory/grind/CD_cw/candidate.c").read_bytes().decode("utf-8")
start = s.index("/* bios.c's alarm helpers, as in Sony's source")
end = s.index("    return -(Intr.sync == 5);\n}\n", start) + len("    return -(Intr.sync == 5);\n}\n")
assert s.count("/* bios.c's alarm helpers, as in Sony's source") == 1
s = s[:start] + body + s[end:]
assert "\r" not in s
Path("src/system.c").write_bytes(s.encode("utf-8"))
print("respliced")
