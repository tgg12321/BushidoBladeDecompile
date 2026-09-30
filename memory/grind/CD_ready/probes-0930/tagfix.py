"""Move the SOTN tag off the signature line (the layer2 body hasher cannot parse it there)."""
import re
from pathlib import Path

PAT = re.compile(r"^(s32 CD_\w+\([^)]*\)) (/\* SOTN: [^*]*\*/)\n", re.M)
for p in ("src/system.c", "memory/grind/CD_ready/candidate.c",
          "memory/grind/CD_ready/probes-0930/CD_sync_sotn.c",
          "memory/grind/CD_ready/probes-0930/CD_datasync_sotn.c"):
    t = Path(p).read_bytes().decode("utf-8")
    t2, n = PAT.subn(r"\2\n\1\n", t)
    Path(p).write_bytes(t2.encode("utf-8"))
    print(p, n)
