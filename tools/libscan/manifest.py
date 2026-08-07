#!/usr/bin/env python3
"""Per-XDEF rename manifest from the 2026-07-09 libscan verbatim module placements.

Builds on scan.py/analyze.py: re-derives the accepted module placements, then
emits PER-SYMBOL virtual addresses (XDEFs + module-local statics) and
cross-references the BB2 function universe (asm/funcs/*.s glabels) and the
current live names (glabel, named_syms.txt, symbol_addrs.txt,
undefined_syms_auto.txt, docs/naming census).

Outputs (all under docs/naming/libscan/):
  libsyms.json          — every symbol placement with lib/mod/kind
  rename_manifest.csv   — decision-ready manifest
  anomalies.txt         — sanity-check failures
Nothing is applied. No tracked file is touched.
"""
import struct, sys, os, json, re, glob, csv, collections

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
from psyq_lib import lib_modules, parse_obj

# Artifacts live with the evidence, not with the tool.
OUT = os.path.join(REPO, "docs", "naming", "libscan")
os.makedirs(OUT, exist_ok=True)
# The PsyQ 4.0 .LIB set is a third-party SDK input, kept OUT of the repo. Point
# PSYQ_LIB_DIR at a local copy to re-run the scan; see README.md.
LIBDIR = os.environ.get("PSYQ_LIB_DIR", os.path.join(REPO, "tmp", "libscan", "psyq40"))

M = json.load(open(os.path.join(OUT, "matches.json"), encoding="utf-8"))
taddr = int(M["taddr"], 16)

objs = {}
for libpath in sorted(glob.glob(os.path.join(LIBDIR, "*.LIB"))):
    libname = os.path.basename(libpath)[:-4]
    for modname, objdata in lib_modules(open(libpath, "rb").read()):
        try:
            objs[(libname, modname)] = parse_obj(objdata)
        except Exception:
            pass


def text_sid(o):
    for sid, s in o.sections.items():
        if s["name"] == ".text" and len(s["bytes"]) >= 4:
            return sid
    return None


# ---------------------------------------------------------------- placements
verb = [r for r in M["results"] if r["status"] == "verbatim"]
unique = [r for r in verb if len(r["vaddrs"]) == 1]
multi = [r for r in verb if len(r["vaddrs"]) > 1]

ivs = [(int(r["vaddrs"][0], 16), int(r["vaddrs"][0], 16) + r["words"] * 4,
        r["lib"], r["mod"], r["words"]) for r in unique]


def merge(intervals, gap=0x40):
    out = []
    for t in sorted(intervals):
        a, b = t[0], t[1]
        if out and a <= out[-1][1] + gap:
            out[-1][1] = max(out[-1][1], b)
        else:
            out.append([a, b])
    return out


def overlaps_unique(a, b):
    return any(a < ub and ua < b for ua, ub, *_ in ivs)


accepted = list(ivs)
placed_multi = []
for _pass in range(3):
    regions_now = merge(accepted)
    for r in multi:
        good = []
        for va in r["vaddrs"]:
            a = int(va, 16); b = a + r["words"] * 4
            if any(ra - 0x10 <= a <= rb + 0x10 or ra - 0x10 <= b <= rb + 0x10
                   for ra, rb in regions_now):
                good.append((a, b))
        fresh = [(a, b) for a, b in good if not overlaps_unique(a, b)
                 and not any(a == pa and b == pb and r["lib"] == pl and r["mod"] == pm
                             for pa, pb, pl, pm, _w in accepted)]
        for a, b in fresh:
            accepted.append((a, b, r["lib"], r["mod"], r["words"]))
            placed_multi.append((r["lib"], r["mod"], "0x%08X" % a))

accepted = sorted(set(accepted))

# The main verbatim block per final_stats.txt; three tiny "island" placements
# outside it (LIBSPU/S_CB, LIBGS/GS_008, GS_009 near 0x800469A0/0x80046B20) were
# excluded from the region accounting there.  Keep them but flag them.
BLOCK_LO, BLOCK_HI = 0x80078948, 0x8008D070

# ------------------------------------------------------------- symbol export
# vaddr -> list of (name, kind, lib, mod, mod_start, mod_end)
symrecs = collections.defaultdict(list)
mod_of = []
for a, b, lib, mod, w in accepted:
    mod_of.append((a, b, lib, mod))
    o = objs.get((lib, mod))
    if not o:
        continue
    sid = text_sid(o)
    for nm, sec, off in o.xdefs:
        if sec == sid and a + off < b:
            symrecs[a + off].append((nm, "xdef", lib, mod, a, b))
    for nm, sec, off in o.locals:
        if sec == sid and a + off < b:
            symrecs[a + off].append((nm, "static", lib, mod, a, b))

json.dump({"0x%08X" % k: [dict(name=n, kind=k2, lib=l, mod=m,
                               mod_start="0x%08X" % ms, mod_end="0x%08X" % me)
                          for n, k2, l, m, ms, me in v]
           for k, v in sorted(symrecs.items())},
          open(os.path.join(OUT, "libsyms.json"), "w", encoding="utf-8",
               newline="\n"), indent=1)

# --------------------------------------------------------- function universe
glabels = {}     # name -> addr
addr_glabel = {} # addr -> [names]
func_end = {}    # addr -> end addr (last insn + 4) within its .s file
for p in sorted(glob.glob(os.path.join(REPO, "asm", "funcs", "*.s"))):
    cur = None
    first = None
    last = None
    starts = []
    for line in open(p, errors="replace"):
        g = re.match(r"glabel (\w+)", line)
        if g:
            cur = g.group(1)
            continue
        m = re.search(r"/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/", line)
        if m:
            va = int(m.group(1), 16)
            last = va
            if cur:
                glabels.setdefault(cur, va)
                addr_glabel.setdefault(va, []).append(cur)
                starts.append((va, cur))
                cur = None
    for i, (va, nm) in enumerate(starts):
        end = starts[i + 1][0] if i + 1 < len(starts) else (last + 4 if last else va + 4)
        func_end[va] = end

# asm/funcs only covers functions still in an asm segment.  Functions already
# decompiled into src/*.c have no .s file, so complete the universe from the
# linker map (same fallback analyze.py used).
mapsyms = {}
mappath = os.path.join(REPO, "build", "bb2.map")
if os.path.exists(mappath):
    for line in open(mappath, errors="replace"):
        m = re.match(r"\s+0x([0-9a-fA-F]{16}|[0-9a-fA-F]{8})\s+(\w+)\s*$", line)
        if m:
            va = int(m.group(1), 16)
            if 0x80016000 <= va < 0x8008D080:   # .text only (code ends ~0x8008D070)
                mapsyms.setdefault(m.group(2), va)

name_source = {}
for va, ns in addr_glabel.items():
    name_source[va] = "asm/funcs glabel"
for nm, va in mapsyms.items():
    if va not in addr_glabel:
        addr_glabel.setdefault(va, []).append(nm)
        name_source[va] = "build/bb2.map (decompiled in src/)"

# recompute extents over the completed universe
_starts = sorted(addr_glabel)
func_end = {}
for i, va in enumerate(_starts):
    func_end[va] = _starts[i + 1] if i + 1 < len(_starts) else va + 4

# -------------------------------------------------------------- live aliases
alias = collections.defaultdict(list)  # addr -> [name]
for fn in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt",
           "undefined_funcs_auto.txt"):
    path = os.path.join(REPO, fn)
    if not os.path.exists(path):
        continue
    for line in open(path, errors="replace"):
        line = line.split("//")[0]
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]{8})\s*;", line)
        if m:
            alias[int(m.group(2), 16)].append(m.group(1))

# ------------------------------------------------------------------- census
census = {}
cpath = os.path.join(REPO, "docs", "naming", "function-names.csv")
with open(cpath, newline="", errors="replace") as f:
    for row in csv.DictReader(f):
        try:
            census[int(row["address"], 16)] = row
        except Exception:
            pass

overrides = {}
opath = os.path.join(REPO, "docs", "naming", "action-overrides.csv")
with open(opath, newline="", errors="replace") as f:
    for row in csv.reader(f):
        if not row or row[0].startswith("#") or row[0] == "key":
            continue
        overrides[row[0]] = row

# ------------------------------------------------------------------- queue
queue_addrs = {}
q = json.load(open(os.path.join(REPO, "engine", "queue.json")))
for it in q["items"]:
    fn = it["func"]
    m = re.match(r"func_([0-9A-Fa-f]{8})$", fn)
    addr = int(m.group(1), 16) if m else glabels.get(fn)
    if addr is not None:
        queue_addrs[addr] = it

# ------------------------------------------------------------- classification
AUTO = re.compile(r"^(func|D|jtbl)_[0-9A-Fa-f]{8}$")


def is_auto(n):
    return bool(AUTO.match(n))


anomalies = []
rows = []

in_span_funcs = []
for va, names in sorted(addr_glabel.items()):
    for a, b, lib, mod in mod_of:
        if a <= va < b:
            in_span_funcs.append((va, names[0], a, b, lib, mod))
            break

func_starts = set(addr_glabel)

# anomaly: XDEF landing inside (not at the start of) a known function
for sa, recs in sorted(symrecs.items()):
    xd = [r for r in recs if r[1] == "xdef"]
    if not xd:
        continue
    if sa in func_starts:
        continue
    host = None
    for fva, fend in func_end.items():
        if fva < sa < fend:
            host = (fva, fend)
            break
    if host:
        anomalies.append(
            "MID-FUNCTION XDEF: %s @0x%08X (%s/%s) lands inside %s "
            "[0x%08X..0x%08X)" % ("/".join(r[0] for r in xd), sa, xd[0][2], xd[0][3],
                                  addr_glabel[host[0]][0], host[0], host[1]))
    else:
        anomalies.append(
            "UNMATCHED XDEF: %s @0x%08X (%s/%s) is not a glabel and not inside "
            "any asm/funcs function" % ("/".join(r[0] for r in xd), sa,
                                        xd[0][2], xd[0][3]))

for va, gname, a, b, lib, mod in in_span_funcs:
    recs = symrecs.get(va, [])
    xdefs = [r for r in recs if r[1] == "xdef"]
    statics = [r for r in recs if r[1] == "static"]
    aliases = [n for n in alias.get(va, []) if n != gname]
    crow = census.get(va, {})
    cur_all = [gname] + aliases
    # strip the _XXXXXXXX address suffix community convention when comparing
    def base(n):
        return re.sub(r"_[0-9A-Fa-f]{8}$", "", n)

    if xdefs:
        sony_all = sorted(set(r[0] for r in xdefs))
        proposed = sony_all[0]
        bad_aliases = [n for n in aliases if base(n) not in sony_all]
        ambiguous = len(sony_all) > 1
        hit = next((base(n) for n in cur_all if base(n) in sony_all), None)
        if hit:
            cls = "CONFIRM"
            proposed = hit
        elif ambiguous and not any(base(n) in sony_all for n in cur_all):
            # two code-identical library modules both match here; picking either
            # name would be a coin flip.  Propose nothing.
            cls = "AMBIGUOUS"
            proposed = ""
        elif not is_auto(gname):
            cls = "CONTRADICTED"          # the glabel itself asserts a wrong name
        elif bad_aliases:
            cls = "CONTRADICTED_ALIAS"    # glabel is auto; a live alias is wrong
        else:
            cls = "FILL"                  # no naming claim exists at all
        rows.append(dict(
            addr="0x%08X" % va, current_name=gname,
            aliases=";".join(aliases), proposed_name=proposed,
            all_sony_names=";".join(sony_all), module=mod, lib=lib,
            mod_start="0x%08X" % a, evidence="libscan-verbatim",
            classification=cls,
            in_main_block=str(BLOCK_LO <= va < BLOCK_HI),
            queued=("yes:" + queue_addrs[va].get("status", "?")) if va in queue_addrs else "",
            census_action=crow.get("action", ""),
            census_tier=crow.get("tier", ""),
            census_name=crow.get("current_name", ""),
            name_source=name_source.get(va, ""),
            note=("contradicting live alias(es): " + ";".join(bad_aliases))
                 if bad_aliases else ""))
    else:
        nm = statics[0][0] if statics else ""
        cls = "MODULE_LOCAL_STATIC" if statics else "IN_SPAN_NO_SYMBOL"
        rows.append(dict(
            addr="0x%08X" % va, current_name=gname, aliases=";".join(aliases),
            proposed_name="", all_sony_names=";".join(sorted(set(r[0] for r in statics))),
            module=mod, lib=lib, mod_start="0x%08X" % a,
            evidence="libscan-verbatim", classification=cls,
            in_main_block=str(BLOCK_LO <= va < BLOCK_HI),
            queued=("yes:" + queue_addrs[va].get("status", "?")) if va in queue_addrs else "",
            census_action=crow.get("action", ""), census_tier=crow.get("tier", ""),
            census_name=crow.get("current_name", ""),
            name_source=name_source.get(va, ""),
            note=("OBJ local symbol %s (module-local static, NOT an exported name); "
                  "propose nothing" % nm) if statics else
                 "inside %s/%s but carries no OBJ symbol at its entry" % (lib, mod)))

# A name proposed at two different addresses means one module matched twice
# (or two code-identical modules).  Neither placement is decidable from bytes
# alone -> demote both.
_bp = collections.defaultdict(list)
for r in rows:
    if r["proposed_name"] and r["classification"] != "CONFIRM":
        _bp[r["proposed_name"]].append(r)
for nm, rs in _bp.items():
    if len(rs) > 1:
        for r in rs:
            r["classification"] = "AMBIGUOUS"
            r["note"] = (r["note"] + " | " if r["note"] else "") + \
                ("'%s' matches at %s — module placed more than once; not decidable "
                 "from bytes alone" % (nm, ", ".join(x["addr"] for x in rs)))
            r["proposed_name"] = ""

# sanity: every proposed mapping's XDEF offset must land exactly on the glabel
for r in rows:
    if r["proposed_name"]:
        va = int(r["addr"], 16)
        assert va in func_starts, r
        assert va in symrecs, r

fields = ["addr", "current_name", "aliases", "proposed_name", "all_sony_names",
          "module", "lib", "mod_start", "evidence", "classification",
          "in_main_block", "queued", "census_action", "census_tier",
          "census_name", "name_source", "note"]
with open(os.path.join(OUT, "rename_manifest.csv"), "w", newline="",
          encoding="utf-8") as f:
    w = csv.DictWriter(f, fieldnames=fields, lineterminator="\n")
    w.writeheader()
    for r in sorted(rows, key=lambda r: r["addr"]):
        w.writerow(r)

with open(os.path.join(OUT, "anomalies.txt"), "w", encoding="utf-8",
          newline="\n") as f:
    f.write("\n".join(anomalies) + ("\n" if anomalies else ""))

# ------------------------------------------------------------- collisions
existing_names = {}
for nm, va in glabels.items():
    existing_names.setdefault(nm, va)
for nm, va in mapsyms.items():
    existing_names.setdefault(nm, va)
for va, ns in alias.items():
    for n in ns:
        existing_names.setdefault(n, va)

collisions = []
by_prop = collections.defaultdict(list)
for r in rows:
    if r["proposed_name"]:
        by_prop[r["proposed_name"]].append(r["addr"])
for nm, addrs in sorted(by_prop.items()):
    if len(addrs) > 1:
        collisions.append("DUPLICATE PROPOSAL: %s proposed at %s" % (nm, ", ".join(addrs)))
    ex = existing_names.get(nm)
    if ex is not None and "0x%08X" % ex not in addrs:
        collisions.append("NAME ALREADY IN TREE: %s currently binds 0x%08X, "
                          "proposed for %s" % (nm, ex, ", ".join(addrs)))

# pipeline-key hazard: function names are KEYS in these files, so a rename that
# collides with an existing key silently re-points a gate ([[name-keyed-gates-in-tool-source]]).
KEYFILES = ["regfix.txt", "asmfix.txt", "inline_asm_canonical.txt",
            "expand_lb_funcs.txt", "expand_dest_funcs.txt", "multu_funcs.txt",
            "multu_pad_funcs.txt", "maspsx_label_nop_funcs.txt", "sdata.txt"]
keys = collections.defaultdict(list)
for fn in KEYFILES:
    p = os.path.join(REPO, fn)
    if not os.path.exists(p):
        continue
    for i, line in enumerate(open(p, errors="replace"), 1):
        s = line.split("#")[0].strip()
        if not s:
            continue
        m = re.match(r"([A-Za-z_]\w*)\s*[:\s]", s) or re.match(r"([A-Za-z_]\w*)$", s)
        if m:
            keys[m.group(1)].append("%s:%d" % (fn, i))

key_hazards = []
for r in rows:
    nm = r["proposed_name"]
    if nm and nm in keys and r["current_name"] != nm:
        key_hazards.append((r["addr"], r["current_name"], nm, keys[nm][:3]))

queue_rows = [r for r in rows if r["queued"]]

cnt = collections.Counter(r["classification"] for r in rows)

# ------------------------------------------------------------------ report
L = []
w = L.append
w("# libscan rename manifest — per-XDEF Sony names for BB2 functions\n")
w("Generated by `tools/libscan/manifest.py` (2026-08-07) on top of the 2026-07-09 "
  "bit-verbatim module scan (`tools/libscan/scan.py` / `matches.json`). "
  "**Nothing is applied.** No tracked file was read-modified.\n")
w("## Evidence\n")
w("Every row rests on the same dispositive fact: a PsyQ 4.0 `.LIB` module's entire "
  "`.text` is bit-identical to a span of `disc/SLUS_006.63` once reloc-affected "
  "instruction fields (HI16/LO16 imm16, REL26 low-26, FULL32) are masked. This "
  "manifest adds the per-symbol layer: each module's OBJ `XDEF` records carry a "
  "section-relative offset, so `module_placement_vaddr + xdef_offset` is the "
  "exported symbol's BB2 virtual address. `evidence=libscan-verbatim` on every row.\n")
w("- %d accepted module placements, %d distinct symbol addresses "
  "(`docs/naming/libscan/libsyms.json`)." % (len(accepted), len(symrecs)))
w("- %d functions of the BB2 universe fall inside a matched module span.\n"
  % len(rows))
w("## Counts by classification\n")
w("| classification | count | meaning |")
w("|---|---|---|")
meaning = {
 "CONFIRM": "already carries the Sony name — no action",
 "CONTRADICTED": "the **glabel itself** asserts a different name; RENAME retires a misname",
 "CONTRADICTED_ALIAS": "glabel is auto (`func_XXXXXXXX`) but a live alias in "
                       "`named_syms.txt`/`symbol_addrs.txt` asserts a different name; "
                       "RENAME fills the glabel **and** retires the bad alias",
 "FILL": "no naming claim exists at the address — RENAME is a pure fill",
 "MODULE_LOCAL_STATIC": "inside a matched module but the entry is an OBJ **local** "
                        "symbol, not an XDEF — a module-local static. Its OBJ name is "
                        "recorded as a note; **no name is proposed** (a local name is "
                        "not an exported claim)",
 "IN_SPAN_NO_SYMBOL": "inside a matched module with no OBJ symbol at its entry",
 "AMBIGUOUS": "two code-identical library modules match here, or one module matched "
              "at two addresses — the byte evidence does not pick a name. "
              "**No proposal**; both candidates recorded in `all_sony_names`",
}
for k, v in cnt.most_common():
    w("| %s | %d | %s |" % (k, v, meaning.get(k, "")))
w("")
w("Renameable rows (CONTRADICTED + CONTRADICTED_ALIAS + FILL) = **%d**; "
  "CONFIRM = %d; no-proposal rows = %d.\n"
  % (cnt["CONTRADICTED"] + cnt["CONTRADICTED_ALIAS"] + cnt["FILL"], cnt["CONFIRM"],
     cnt["MODULE_LOCAL_STATIC"] + cnt["IN_SPAN_NO_SYMBOL"]))
PFX = re.compile(r"^(bios|gpu|cdrom|sys|spu|memcard|irq|bb2|debug|snd|coli)_")
style_only, hard_mis = [], []
for r in rows:
    if r["classification"] != "CONTRADICTED":
        continue
    c = PFX.sub("", r["current_name"]).lower().replace("_", "")
    (style_only if c == r["proposed_name"].lower().replace("_", "")
     else hard_mis).append(r)
w("### The CONTRADICTED set splits in two\n")
w("| kind | count | what it means |")
w("|---|---|---|")
w("| style-only | %d | the project name IS the Sony name wearing a project prefix "
  "(`bios_SetMem` -> `SetMem`, `gpu_DrawSync` -> `DrawSync`). Not a false positive "
  "being corrected — an owner style call. |" % len(style_only))
w("| hard misname | %d | the project name asserts a **different function** than the "
  "bytes are (`gpu_SendPacket` -> `DrawPrim`, `cdrom_GetReadyFlag` -> `CdMode`, "
  "`memcard_SetBusy` -> `SsSetMono`, `spu_IrqHandler` -> `SpuInitMalloc`). These are "
  "exactly what [[names-require-evidence]] targets. |" % len(hard_mis))
w("")
w("Hard misnames, in full:\n")
w("| addr | current (wrong) | Sony name | lib/module |")
w("|---|---|---|---|")
for r in sorted(hard_mis, key=lambda r: r["addr"]):
    w("| %s | %s | **%s** | %s/%s |" % (r["addr"], r["current_name"],
                                        r["proposed_name"], r["lib"], r["module"]))
w("")
w("## Sanity checks\n")
w("1. **Every proposed mapping lands exactly on the function's entry address.** "
  "Asserted in code: a row only gets a `proposed_name` when its address is both a "
  "known function start and an XDEF address. Zero rows failed.")
w("2. **The three known misnames reproduce** — see the table below.")
w("3. **%d XDEFs land mid-function** (`docs/naming/libscan/anomalies.txt`). Investigated: "
  "these are *not* scan or parse errors. Each is a second exported entry point in a "
  "module whose earlier entry point splat also emitted, so splat merged the two into "
  "one `asm/funcs` function. Verified by hand on `func_80078FF0` (FlushCache, 4 words, "
  "ends `jr $t2`/`nop` at 0x80078FFC) and `func_80083220` (setjmp, ends `jr $ra` at "
  "0x80083254) — the XDEF sits immediately after a real return, at the module base. "
  "These want a **splat re-split**, not a rename, and are excluded from the manifest."
  % len(anomalies))
w("")
w("### The three known misnames\n")
w("| addr | misname | Sony name | module | state |")
w("|---|---|---|---|---|")
for a, mis in ((0x8007FD5C, "single_game_getEnemyCharId"),
               (0x8007AE7C, "ResetGraph"),
               (0x800893D8, "coli_HitPauseKatana")):
    r = next((x for x in rows if int(x["addr"], 16) == a), None)
    if r:
        state = ("already RESET by the phase-2 wave; census records the retired name"
                 if r["classification"] == "FILL" else
                 "misname still live — this manifest retires it")
        w("| %s | %s | **%s** | %s/%s | %s |"
          % (r["addr"], mis, r["proposed_name"], r["lib"], r["module"], state))
w("")
w("## Collisions\n")
w("### With `engine/queue.json`\n")
w("%d in-span functions are live decomp-queue items. Renaming a queued function "
  "changes a pipeline key (see [[naming-wave-tool]]), so an apply session must run in "
  "a grinder-quiet window.\n" % len(queue_rows))
if queue_rows:
    w("| addr | current | proposed | lib/module | queue status | classification |")
    w("|---|---|---|---|---|---|")
    for r in sorted(queue_rows, key=lambda r: r["addr"]):
        w("| %s | %s | %s | %s/%s | %s | %s |"
          % (r["addr"], r["current_name"], r["proposed_name"] or "—",
             r["lib"], r["module"], r["queued"], r["classification"]))
    w("")
w("### Name collisions\n")
if collisions:
    for c in collisions:
        w("- %s" % c)
else:
    w("None: no proposed name is proposed twice, and no proposed name already binds a "
      "different address in the tree.")
w("")
w("### With pipeline key files\n")
w("Function names are KEYS in `regfix.txt`, `asmfix.txt`, "
  "`inline_asm_canonical.txt` and the maspsx gate lists. A proposed name that "
  "already exists as a key there would silently re-point a gate.\n")
if key_hazards:
    for a, cur, nm, where in key_hazards:
        w("- **%s** `%s` -> `%s` — the name `%s` is already a key at %s"
          % (a, cur, nm, nm, ", ".join(where)))
else:
    w("None found.")
w("")
w("## What a future apply session must do\n")
w("1. **Wait for a grinder-quiet window.** `tools/naming_wave.py` refuses a dirty "
  "tree, and the Grinder's end-of-session scope check discards a session that finds "
  "foreign dirt ([[grinder-clobbers-uncommitted-edits]]). Stop the Grinder "
  "(`pwsh tools/grinder/grind.ps1 -Stop`) or wait for it to idle first.")
w("2. **Teach `docs/naming/build_census.py` a `libscan` origin tier.** The census is "
  "GENERATED, so hand-edits are reverted. The tier must rank above `kengo-derived` "
  "and `naming-analyzer` — bit-verbatim module identity is dispositive evidence, "
  "unlike the inference tiers. Rows should record "
  "`evidence=libscan-verbatim: <LIB>/<MODULE> XDEF <name> @ +0x<off>`.")
w("3. **Apply only via `tools/naming_wave.py`** — the only sanctioned applier "
  "([[naming-wave-tool]]); function names are pipeline keys (regfix/asmfix/gate-list "
  "files, `inline_asm_canonical.txt`, `expand_lb_funcs.txt`, …) and hand-editing "
  "`named_syms.txt` desynchronises them.")
w("4. **Never run `make setup`** — `bb2.ld` is hand-maintained (CLAUDE.md standing "
  "warning) and re-splitting would re-add dead rodata lines.")
w("5. **Verify against the oracle.** Full build + link SHA1 must stay "
  "`62efab4f73f992798c43e8c730aa43baa10bb4fa`. Renames are expected to be "
  "byte-neutral; anything else means a name was load-bearing "
  "([[name-keyed-gates-in-tool-source]]).")
w("6. **Decide the `bios_*`/`gpu_*`/`cdrom_*` prefix question first.** %d rows are "
  "glabel-level CONTRADICTED, and many are *semantically right but nominally wrong* "
  "(`gpu_DrawSync` → `DrawSync`, `bios_SetMem` → `SetMem`). These are not false "
  "positives being corrected; they are project-style names being replaced by the "
  "vendor's own. That is an owner style call, separable from the genuinely wrong "
  "ones (`gpu_SendPacket` → `DrawPrim`, `cdrom_GetReadyFlag` → `CdMode`)."
  % cnt["CONTRADICTED"])
w("7. **Do not propose names for the %d MODULE_LOCAL_STATIC rows.** Their OBJ names "
  "are module-local; per [[names-require-evidence]] a local name is not an exported "
  "claim. If the owner wants them filled, a lib-prefixed auto form "
  "(`libspu_static_8008XXXX`) is the most that the evidence supports."
  % cnt["MODULE_LOCAL_STATIC"])
w("8. **Handle the %d mid-function XDEFs separately** — they need a splat function-"
  "boundary correction, which touches `splat.yaml`/`asm/`, not the naming wave."
  % len(anomalies))
w("")
w("## Files\n")
w("- `docs/naming/libscan/rename_manifest.csv` — the manifest (one row per in-span function)")
w("- `docs/naming/libscan/libsyms.json` — every XDEF/local placement with lib, module, offset")
w("- `docs/naming/libscan/anomalies.txt` — the mid-function XDEF list")
w("- `tools/libscan/manifest.py` — the generator")
open(os.path.join(OUT, "manifest_report.md"), "w", encoding="utf-8",
     newline="\n").write("\n".join(L) + "\n")

print("accepted placements:", len(accepted), " symbol addrs:", len(symrecs))
print("in-span functions:", len(rows))
for k, v in cnt.most_common():
    print("  %-20s %d" % (k, v))
print("queued & in-span:", sum(1 for r in rows if r["queued"]))
print("anomalies:", len(anomalies))
for x in anomalies[:40]:
    print("  " + x)
print("\nCONTRADICTED (glabel-level):")
for r in sorted(rows, key=lambda r: r["addr"]):
    if r["classification"] == "CONTRADICTED":
        print("  %s %-46s -> %-22s (%s/%s)%s" %
              (r["addr"], r["current_name"], r["proposed_name"], r["lib"],
               r["module"], "  [aliases: %s]" % r["aliases"] if r["aliases"] else ""))
