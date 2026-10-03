"""regions.py FUNC=NEWID ... : relocate canonical_asm_regions grants after a split, refusing unless the
islands in the new file hash to the reviewed grant (engine.completion.region_hashes)."""
import json, sys
from pathlib import Path
sys.path.insert(0, ".")
from engine import completion
p = Path("tools/canonical_asm_regions.json")
data = json.loads(p.read_text(encoding="utf-8"))
for arg in sys.argv[1:]:
    f, new = arg.split("=")
    text = Path(f"src/{new}.c").read_text(encoding="utf-8")
    got = completion.region_hashes(text, f)
    assert got == data["functions"][f]["sha256"], (f, got)
    print(f, data["functions"][f]["file"], "->", new, "hashes OK")
    data["functions"][f]["file"] = new
p.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8", newline="\n")
