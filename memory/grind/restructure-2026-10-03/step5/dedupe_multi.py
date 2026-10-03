"""dedupe_multi.py FILE... : for each psxsdk public header H, apply dedupe.py to FILE with
--include <psxsdk/H> when FILE has a duplicate of something in H, or FILE is in H's own library dir."""
import subprocess, sys, os
HDRS = {"libapi": "libapi", "libetc": "libetc", "libc": "libc2", "libcard": "libcard", "libcomb": "libcomb",
        "libsn": "libsn", "kernel": None}
env = dict(os.environ, PYTHONIOENCODING="utf-8")
for f in sys.argv[1:]:
    for h, libdir in HDRS.items():
        hp = f"include/psxsdk/{h}.h"
        dry = subprocess.run([sys.executable, "tmp/s5/dedupe.py", hp, f], capture_output=True, text=True, env=env).stdout
        own = libdir and f"/psxsdk/{libdir}/" in f.replace("\\", "/")
        if dry.strip() or own:
            subprocess.run([sys.executable, "tmp/s5/dedupe.py", "--apply", "--include", f"#include <psxsdk/{h}.h>", hp, f], env=env, check=True)
            print(f, "<-", h, "(own lib)" if own and not dry.strip() else "")
