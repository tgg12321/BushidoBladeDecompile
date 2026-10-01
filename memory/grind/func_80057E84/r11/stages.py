"""stages.py: print the build's text1b stages after cpp (cc1 | prologue_fix | maspsx | multu_pad | as),
one per line, for perm_setup.sh's compile.sh. Run from the repo root."""
import sys

sys.path.insert(0, ".")
from engine import pipeline  # noqa: E402

cmd = pipeline.c_pipeline_cmd("text1b", "OUTFILE").split(" && ")[0]
for st in cmd.split(" | ")[1:]:
    print(st)
