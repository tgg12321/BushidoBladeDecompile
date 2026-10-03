"""mk_G3.py REPO : run in a tree root holding the G2 state (game.h/bb2.h split + duplicate drop +
G1 comment fixes). Hoists the identical multi-file game declarations and types:
  - bb2.h gets the hoist.json candidates minus EXCLUDE (one sorted section, data then functions);
  - game.h gets the identical multi-file local types (with their comment blocks);
  - every game TU loses its now-redundant copies (dedupe.py, with the original comment texts);
  - follow-ups: 87A0.c's forward GameObj typedef, 63D2C.c's S_80074488 copy (a K&R definition
    hides it from the item parser), 64FD8.c's detached GameObj comment, empty section banners;
  - comment edits: Unk8009BD38Flags ("this file" -> its C readers), GameObj.
REPO is the main repo root (for tmp/s5/hoist.json and the scripts)."""
import glob, json, os, re, subprocess, sys

REPO = sys.argv[1]
S5 = os.path.join(REPO, "tmp", "s5")
sys.path.insert(0, S5)
from citems import items, strip_cs
from dedupe import proto_key, ws

EXCLUDE = {
    "func_8005C2A8",   # main/309CC.c calls it implicitly; the s16 prototype changes its bytes
    "D_800A3899",      # Q21 per-file view: main/9F9C.c declares these bytes as D_800A3898[2]
    "D_800A38AB",      # Q21 per-file view: main/9F9C.c declares these bytes as D_800A38AA[2]
    "D_800963EE",      # second name for D_800963EC[0].length_sectors (main/35000.c)
}
# Vec3 is not hoisted (main/9F9C.c has a different block-scope Vec3, Phase 2); Vec3s16 / Vec3s32 are
# used nowhere and stay where they are.
TYPES = [("Vec2s16", "src/main/3AB48.c"), ("Copy16", "src/main/309CC.c"), ("S46C", "src/main/3AB48.c"),
         ("S_80074488", "src/main/64FD8.c"), ("Unk8009BD24Record", "src/main/3AB48.c"),
         ("Unk8009BD38Flags", "src/main/3AB48.c"), ("GameObj", "src/main/3AB48.c")]


def rd(p):
    return open(p, encoding="utf-8", newline="").read()


def wr(p, t):
    open(p, "w", encoding="utf-8", newline="\n").write(t)


def sub(t, old, new, what):
    assert t.count(old) == 1, (what, old[:70])
    return t.replace(old, new)


# 1. declarations into bb2.h
cands = [c for c in json.load(open(os.path.join(S5, "hoist.json"))) if c["name"] not in EXCLUDE]
objs, funcs = [], []
for d in sorted(cands, key=lambda d: d["name"]):
    if d["kind"] == "O":
        objs.append(ws(strip_cs(d["text"])))
    else:
        _, name, ret, ps = proto_key(d["text"])
        sep = "" if ret.endswith("*") else " "
        line = f"extern {ret}{sep}{name}({', '.join(ps)});"
        line = re.sub(r"(\w)\*", r"\1 *", line)
        funcs.append(line)
block = ("/* Declared by more than one translation unit: data, then functions, by name. */\n"
         + "\n".join(objs) + "\n\n" + "\n".join(funcs) + "\n\n")
b = rd("include/bb2.h")
b = sub(b, "/* Game-side declarations of Sony library functions", block + "/* Game-side declarations of Sony library functions", "bb2.h")
wr("include/bb2.h", b)

# 2. types into game.h (verbatim, with the comment block directly above)
out = []
for name, f in TYPES:
    t = rd(f)
    its, lines = items(t)
    hit = next(it for it in its if " ".join(strip_cs("\n".join(lines[it["start"] - 1:it["end"]])).split()).startswith("typedef")
               and re.search(r"\b" + name + r"\s*;$", " ".join(strip_cs("\n".join(lines[it["start"] - 1:it["end"]])).split()))
               and "{" in "\n".join(lines[it["start"] - 1:it["end"]]))
    j = hit["start"] - 1
    while j >= hit["lead"] and lines[j - 1].strip() != "":
        j -= 1
    out.append("\n".join(lines[j:hit["end"]]))
g = rd("include/game.h")
g = sub(g, "\n#endif /* GAME_H */", "\n" + "\n\n".join(out) + "\n\n#endif /* GAME_H */", "game.h")
wr("include/game.h", g)

# 3. drop the local copies (dedupe sees the original comment texts)
game_files = sorted(glob.glob("src/main/*.c"))
subprocess.run([sys.executable, os.path.join(S5, "dedupe.py"), "--apply", "include/game.h,include/bb2.h"] + game_files,
               check=True, env=dict(os.environ, PYTHONIOENCODING="utf-8"))

# 4. follow-ups
s = rd("src/main/87A0.c")
wr("src/main/87A0.c", sub(s, "typedef struct GameObj GameObj;\n\n", "", "87A0"))
s = rd("src/main/63D2C.c")
m = re.search(r"typedef struct \{\n    s32 sp18;.*?\} S_80074488;\n\n", s, re.S)
assert m
wr("src/main/63D2C.c", s.replace(m.group(0), ""))
s = rd("src/main/64FD8.c")
old = ("/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The\n * field layout is the union of all"
       " observed accesses; m2c picks the type\n * that best fits each access site. Mirroring smart_match.py's layout. */\n")
i = s.index(old)
k = i + len(old)
while s[k] == "\n":
    k += 1
wr("src/main/64FD8.c", s[:i] + s[k:])
subprocess.run([sys.executable, os.path.join(S5, "banners2.py")] + game_files, check=True)

# 5. comment edits in game.h
g = rd("include/game.h")
g = sub(g, """/* 0x8009BD38: the match-settings flag word, bit fields named by bit offset.
   Every reader in this file extracts it by field: unk0 (`& 0xF`), unk10 (the
   round count - 3; also picks the results-screen layout), unk12 (`== 2`
   tests), unk14 (1 bit), unk15 (one bit per player); func_80077894 stores
   unk0. Byte 3 is not named here (text1b_b.c reads it as D_8009BD3B). */""",
        """/* 0x8009BD38: the match-settings flag word, bit fields named by bit offset.
   Its C readers extract it by field: unk0 (`& 0xF`), unk10 (the round count
   - 3; also picks the results-screen layout), unk12 (`== 2` tests), unk14
   (1 bit), unk15 (one bit per player); func_80077894 stores unk0. Byte 3 is
   not named here (main/64FD8.c reads it as D_8009BD3B). */""", "flags comment")
g = sub(g, """/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The
 * field layout is the union of all observed accesses; m2c picks the type
 * that best fits each access site. Mirroring smart_match.py's layout. */""",
        """/* GameObj: a generic 0x100-byte layout, fields named by offset (field_XX), from the early m2c
 * context tooling (feaa560b2) rather than from the game's objects. In C the only member used by
 * name is field_18, a GPU primitive write cursor (func_80069898 and func_80070C70 build primitives
 * at it and advance it); func_80072BC4 / func_80072CD4 take a GameObj * but write a POLY_G4
 * through byte casts. Retyping these users is Phase 2 work. */""",
        "GameObj comment")
wr("include/game.h", g)
print("G3 built:", len(objs), "data +", len(funcs), "function declarations;", len(TYPES), "types")
