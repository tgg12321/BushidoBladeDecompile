#!/usr/bin/env python3
"""Phase-1 function-NAMING CENSUS builder (read-only; emits docs/naming/function-names.csv).

Universe = splat's per-function split files asm/funcs/*.s (the linker-authoritative
glabel per function). For each function we determine name ORIGIN and a confidence TIER
per the owner directive of 2026-08-07 (names-require-evidence).
"""
import csv, os, re, subprocess, sys, json
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
J = lambda *p: os.path.join(ROOT, *p)

# ---------------------------------------------------------------- universe
funcs = {}   # name -> dict(addr, file, insns)
FUNCDIR = J("asm", "funcs")
glabel_re = re.compile(r"^glabel\s+(\S+)")
addr_re = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s")
for fn in sorted(os.listdir(FUNCDIR)):
    if not fn.endswith(".s"):
        continue
    path = os.path.join(FUNCDIR, fn)
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
    name = None
    addr = None
    insns = 0
    for ln in lines:
        m = glabel_re.match(ln)
        if m and name is None:
            name = m.group(1)
            continue
        m = addr_re.match(ln)
        if m:
            insns += 1
            if addr is None:
                addr = m.group(1).upper()
    if name is None:
        name = fn[:-2]
    funcs[name] = dict(addr=addr or "", stem=fn[:-2], insns=insns)

# Hand-written buildable asm (save_vc_ctrl) carries no splat address comments. Recover the
# address by decoding a `jal <name>` at a known site elsewhere in asm/.
_need = [n for n, i in funcs.items() if not i["addr"]]
if _need:
    JALLINE = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]{8}\s+([0-9A-Fa-f]{8})\s*\*/\s*jal\s+(\S+)")
    for fn in os.listdir(FUNCDIR):
        if not fn.endswith(".s") or not _need:
            continue
        for ln in open(os.path.join(FUNCDIR, fn), encoding="utf-8", errors="replace"):
            m = JALLINE.search(ln)
            if not m or m.group(2) not in _need:
                continue
            le = m.group(1)
            word = int("".join(reversed([le[i:i + 2] for i in range(0, 8, 2)])), 16)
            if word >> 26 == 3:                       # jal opcode
                va = 0x80000000 | ((word & 0x03FFFFFF) << 2)
                funcs[m.group(2)]["addr"] = f"{va:08X}"
                _need.remove(m.group(2))

# ---------------------------------------------------------------- src definitions
src_def = {}
INCASM = re.compile(r'INCLUDE_ASM\([^,]+,\s*([A-Za-z_]\w*)\s*\)')
for f in sorted(os.listdir(J("src"))):
    if not f.endswith(".c"):
        continue
    with open(J("src", f), encoding="utf-8", errors="replace") as fh:
        for i, ln in enumerate(fh, 1):
            m = INCASM.search(ln)
            if m:
                src_def.setdefault(m.group(1), f"src/{f}:{i}(INCLUDE_ASM)")
                continue
            m = re.match(r"^[A-Za-z_][\w \t\*]*?\**\s*([A-Za-z_]\w*)\s*\(", ln)
            if m and not ln.startswith(("if", "for", "while", "switch", "return", "else")):
                src_def.setdefault(m.group(1), f"src/{f}:{i}")

# ---------------------------------------------------------------- symbol files
sym_lines = {}   # symbol -> (file, lineno, addr, comment)
SYMLINE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;?(.*)$")
for sf in ("named_syms.txt", "symbol_addrs.txt"):
    p = J(sf)
    if not os.path.exists(p):
        continue
    with open(p, encoding="utf-8", errors="replace") as fh:
        for i, ln in enumerate(fh, 1):
            m = SYMLINE.match(ln)
            if m:
                sym_lines.setdefault(m.group(1), (sf, i, m.group(2).upper()[2:], m.group(3).strip()))

# addr -> list of symbol aliases (for misnomer flags recorded at the same address)
addr_syms = defaultdict(list)
for s, (sf, i, a, c) in sym_lines.items():
    addr_syms[a].append((s, sf, i, c))

MISFLAG = re.compile(r"MISNAMED|misnamed|likely misnamed|\bwrong\b", re.I)

# ---------------------------------------------------------------- kengo
kengo = {}
kp = J("kengo_matches.csv")
if os.path.exists(kp):
    with open(kp, newline="", encoding="utf-8", errors="replace") as fh:
        for row in csv.DictReader(fh):
            kengo[row["bb2_func"]] = row

kengo_names = set()
for row in kengo.values():
    if row.get("kengo_name"):
        kengo_names.add(row["kengo_name"])

kengo_decisions = {}
kd = J("kengo_name_decisions.csv")
if os.path.exists(kd):
    with open(kd, newline="", encoding="utf-8", errors="replace") as fh:
        for row in csv.DictReader(fh):
            kengo_decisions[row["renamed_to"]] = row

# ---------------------------------------------------------------- legacy renamer map
rename_map = {}
rename_band = {}    # new_name -> (band_label, evidence_class)
rp = J("tools", "rename_funcs.py")
if os.path.exists(rp):
    band = "unbanded"
    for ln in open(rp, encoding="utf-8", errors="replace"):
        s = ln.strip()
        if s.startswith("#"):
            c = s.lstrip("#").strip()
            if re.search(r"Kengo", c, re.I):
                band = "kengo:" + c
            elif re.search(r"Item 6: readability", c, re.I):
                band = "readability:" + c
            elif re.match(r"^(sound|ings2?|gpu|display|system|main|config)\.c", c):
                band = ("readability:" if band.startswith("readability") else "cluster:") + c
            elif re.match(r"^(HIGH|MED|LOW)\b", c):
                band = re.sub(r"(?<=:)[^:]*$", c + " @ " + band.split(":", 1)[1], band) if ":" in band else band
            continue
        m = re.match(r'"(func_[0-9A-Fa-f]{8})"\s*:\s*"([^"]+)"', s)
        if m:
            rename_map[m.group(1)] = m.group(2)
            rename_band[m.group(2)] = band
rename_targets = set(rename_map.values())

# ---------------------------------------------------------------- psyq stdlib scan
psyq = {}
pp = J("known_psyq_stdlib.txt")
if os.path.exists(pp):
    for ln in open(pp, encoding="utf-8", errors="replace"):
        if ln.startswith("#") or not ln.strip():
            continue
        parts = ln.split()
        if len(parts) >= 2:
            psyq[parts[0]] = (parts[1], " ".join(parts[2:]))

# ---------------------------------------------------------------- canonical-asm authorizations
canon = {}
cp = J("inline_asm_canonical.txt")
if os.path.exists(cp):
    for ln in open(cp, encoding="utf-8", errors="replace"):
        s = ln.strip()
        if not s or s.startswith("#"):
            continue
        nm = s.split()[0]
        canon[nm] = s[len(nm):].strip().lstrip("#").strip()[:400]

# ---------------------------------------------------------------- in-binary strings
# Extract printable strings from the shipped EXE and look for any that equal / contain
# a current function name -> VERIFIED in-binary evidence.
binstrings = set()
exe = J("disc", "SLUS_006.63")
if os.path.exists(exe):
    data = open(exe, "rb").read()
    for m in re.finditer(rb"[ -~]{4,}", data):
        binstrings.add(m.group(0).decode("ascii"))
# addr -> NUL-terminated string, for strings the code can point at
LOAD = 0x80010000
HDR = 0x800
str_at = {}
if os.path.exists(exe):
    # Every NUL-delimited chunk start (strings are packed back-to-back, so a
    # naive printable-run scan only finds the first of each cluster).
    OK = set(range(0x20, 0x7F)) | {0x09, 0x0A, 0x0D}
    n = len(data)
    i = HDR
    while i < n:
        if data[i] in OK and (i == HDR or data[i - 1] == 0):
            j = i
            while j < n and data[j] in OK:
                j += 1
            if j < n and data[j] == 0 and j - i >= 3:
                str_at[LOAD + i - HDR] = data[i:j].decode("ascii")
            i = j + 1
        else:
            i += 1

def norm(s):
    return re.sub(r"[^a-z0-9]", "", s.lower())

def lead_ident(s):
    """Leading C-identifier of a string: 'SetDispMask(%d)...' -> 'SetDispMask'."""
    m = re.match(r"[A-Za-z_]\w*", s)
    return m.group(0) if m else ""

# For each function, resolve rodata symbols it references and compare against its name.
FAMILY = re.compile(r"^(gpu|bios|cdrom|cd|sys|irq|pad|spu|snd|sound|file|psyq|mem|dma|gte|libgte|bb2)_", re.I)
SYMREF = re.compile(r"%(?:hi|lo)\(([A-Za-z_]\w*)\)")
name_in_binary = {}
for nm, info in funcs.items():
    if re.fullmatch(r"func_[0-9A-Fa-f]{8}", nm) or re.fullmatch(r"D_[0-9A-Fa-f]{8}", nm):
        continue
    base = re.sub(r"_[0-9A-Fa-f]{8}$", "", nm)
    stripped = FAMILY.sub("", base)
    if len(stripped) < 4:
        continue
    tgt = norm(stripped)
    path = os.path.join(FUNCDIR, info["stem"] + ".s")
    if not os.path.exists(path):
        continue
    body = open(path, encoding="utf-8", errors="replace").read()
    refs = set(SYMREF.findall(body))
    for r in refs:
        va = None
        m = re.fullmatch(r"D_([0-9A-Fa-f]{8})", r)
        if m:
            va = int(m.group(1), 16)
        elif r in sym_lines:
            va = int(sym_lines[r][2], 16)
        if va is None or va not in str_at:
            continue
        s = str_at[va]
        if norm(s) == tgt or (lead_ident(s) and norm(lead_ident(s)) == tgt):
            name_in_binary[nm] = (
                f'in-binary self-identifying string: this function loads {r} '
                f'(0x{va:08X}) = "{s}", which equals the name (PsyQ debug/assert '
                f'string embedded in the routine itself)')
            break

# ---------------------------------------------------------------- MISNOMERS.md
misnomer_doc = set()
mp = J("docs", "naming", "MISNOMERS.md")
if os.path.exists(mp):
    mt = open(mp, encoding="utf-8", errors="replace").read()
    for m in re.finditer(r"`([A-Za-z_]\w*)`", mt):
        misnomer_doc.add(m.group(1))

# ---------------------------------------------------------------- naming-analyzer provenance
# docs/naming/*.csv record WHERE each analyzer-proposed name came from and how strong the
# evidence was. Keyed by address; first source to claim an address wins (strongest first).
analyzer = {}   # ADDR(no 0x, upper) -> dict(src, name, confidence, method, evidence)
def _add(addr, src, name, conf, method, evi):
    a = addr.strip().upper().replace("0X", "")
    if not re.fullmatch(r"[0-9A-F]{8}", a):
        return
    analyzer.setdefault(a, dict(src=src, name=name, confidence=conf, method=method,
                                evidence=(evi or "")[:260]))

def _rd(fname):
    p = J("docs", "naming", fname)
    if not os.path.exists(p):
        return []
    with open(p, newline="", encoding="utf-8", errors="replace") as fh:
        return list(csv.DictReader(fh))

for r in _rd("residual_named.csv"):
    _add(r.get("address", ""), "residual_named.csv", r.get("proposed_name", ""),
         r.get("confidence", ""), r.get("method", ""), r.get("evidence", ""))
for r in _rd("proposals_resolved.csv"):
    _add(r.get("address", ""), "proposals_resolved.csv", r.get("applied_name") or r.get("proposed_name", ""),
         r.get("confidence", ""), r.get("resolution_category", ""), r.get("evidence_summary", ""))
for r in _rd("proposals.csv"):
    _add(r.get("address", ""), "proposals.csv", r.get("proposed_name", ""),
         r.get("confidence", ""), "analyzer-proposal", r.get("evidence_summary", ""))
for r in _rd("first_pass_applied.csv"):
    _add(r.get("address", ""), "first_pass_applied.csv", r.get("name", ""),
         "", "first-pass subsystem tag (" + (r.get("subsystem", "") or "?") + ")", r.get("evidence", ""))
for r in _rd("no_lead_first_pass.csv"):
    _add(r.get("address", ""), "no_lead_first_pass.csv", r.get("proposed_name", ""),
         r.get("confidence", ""), (r.get("signal", "") or "") + " votes=" + (r.get("votes", "") or ""),
         r.get("evidence", ""))
for r in _rd("dark_rescue.csv"):
    _add(r.get("address", ""), "dark_rescue.csv", r.get("proposed_name", ""),
         r.get("confidence", ""), r.get("technique", ""), r.get("evidence", ""))

# ---------------------------------------------------------------- libscan (verbatim PsyQ modules)
# The one DISPOSITIVE naming source in this tree. A PsyQ .LIB module whose entire .text is
# bit-identical to a span of the shipped EXE (reloc fields masked) carries Sony's own XDEF
# records, and module_placement_vaddr + xdef_offset is the exported symbol's BB2 address. That
# is byte evidence, not inference, so it outranks kengo-derived and naming-analyzer alike.
# Method + regeneration: tools/libscan/README.md.
libscan = {}   # ADDR(no 0x, upper) -> dict(name, lib, module, offset, classification, note)
lp = J("docs", "naming", "libscan", "rename_manifest.csv")
if os.path.exists(lp):
    with open(lp, newline="", encoding="utf-8", errors="replace") as fh:
        for r in csv.DictReader(fh):
            if not r.get("proposed_name"):
                continue          # MODULE_LOCAL_STATIC / AMBIGUOUS / IN_SPAN_NO_SYMBOL
            a = (r.get("addr") or "").strip().upper().replace("0X", "")
            if not re.fullmatch(r"[0-9A-F]{8}", a):
                continue
            # The XDEF's section-relative offset is what the placement was derived FROM;
            # recover it as vaddr - module base rather than re-parsing the .LIB set.
            try:
                off = int(a, 16) - int((r.get("mod_start") or "0"), 16)
            except ValueError:
                off = None
            libscan[a] = dict(name=r["proposed_name"].strip(),
                              lib=r.get("lib", ""), module=r.get("module", ""),
                              offset=off, classification=r.get("classification", ""),
                              note=(r.get("note") or "").strip())


def libscan_evidence(a):
    """`libscan-verbatim: <LIB>/<MODULE> XDEF <name> @ +0x<off>` — the fixed evidence form."""
    e = libscan[a]
    off = "+0x%X" % e["offset"] if isinstance(e["offset"], int) and e["offset"] >= 0 else "+0x?"
    return "libscan-verbatim: %s/%s XDEF %s @ %s" % (e["lib"], e["module"], e["name"], off)


# ---------------------------------------------------------------- queue
queue_funcs = set()
qp = J("engine", "queue.json")
if os.path.exists(qp):
    try:
        q = json.load(open(qp, encoding="utf-8", errors="replace"))
        items = q if isinstance(q, list) else q.get("items", q.get("functions", []))
        for it in items:
            if isinstance(it, dict):
                for k in ("func", "name", "function"):
                    if it.get(k):
                        queue_funcs.add(it[k]); break
    except Exception as e:
        print("queue parse:", e, file=sys.stderr)

# ---------------------------------------------------------------- classify
PSYQ_PREFIX = re.compile(r"^(bios_|gpu_|spu_|snd_dma|cdrom_|sys_|irq_|pad_|mem_|dma_|libgte_|gte_|tsl|su[A-Z]|Vu[01])")
PSYQ_KNOWN = re.compile(r"^(EnterCriticalSection|ExitCriticalSection|_start|main)$")
# Unsuffixed SDK-shaped names (the real library-entry-point claims) — these keep the
# PsyQ reading even when an analyzer CSV also touched the address.
PSYQ_STRICT = re.compile(r"^(bios_|cdrom_|spu_|gte_|libgte_)[A-Z]")
PS2ONLY = re.compile(r"^(tslSmd|tsl|su[A-Z]|Vu[01])")

AUTOPAT = re.compile(r"^(func|D)_[0-9A-Fa-f]{8}$")

_git_cache = {}
def git_intro(name):
    """Oldest commit that introduced this symbol name (provenance of last resort)."""
    if name in _git_cache:
        return _git_cache[name]
    out = ""
    try:
        r = subprocess.run(["git", "log", "--oneline", "-S", name, "--", "src", "include",
                            "named_syms.txt", "symbol_addrs.txt", "asm/funcs"],
                           cwd=ROOT, capture_output=True, text=True, timeout=120)
        lines = [l for l in r.stdout.splitlines() if l.strip()]
        if lines:
            out = "commit " + lines[-1].strip()[:110]   # oldest = introducing commit
    except Exception:
        out = ""
    _git_cache[name] = out
    return out

def strip_addr(s):
    return re.sub(r"_[0-9A-Fa-f]{8}$", "", s)

rows = []
for glabel in sorted(funcs, key=lambda n: funcs[n]["addr"] or "zzz"):
    info = funcs[glabel]
    addr = info["addr"]

    # ---- resolve the name a READER actually sees, across all three layers ----
    aliases = [s for (s, sf, i, c) in addr_syms.get(addr, [])]
    # An alias is AUTO if it is auto-shaped itself (`func_8007A5C4 = 0x8007A5C4;` — a
    # reverse-alias line the registry carries for its own convenience), or if stripping
    # its `_<ADDR>` suffix leaves an auto-shaped name. Testing only the stripped form was
    # a bug: strip_addr("func_8007A5C4") is "func", which AUTOPAT does not match, so the
    # reverse alias was read as a semantic claim and the row's current_name became the
    # bare word `func`. Nine such rows fed `func` into the 2026-08-07 libscan wave's
    # rename map as a renameable identifier.
    semantic_aliases = [s for s in aliases
                        if not AUTOPAT.match(s) and not AUTOPAT.match(strip_addr(s))]
    c_names = [s for s in {strip_addr(a) for a in semantic_aliases} if s in src_def]

    if not AUTOPAT.match(glabel):
        nm = glabel
        layer = "glabel"
    elif c_names:
        nm = sorted(c_names)[0]
        layer = "src/*.c definition + named_syms alias"
    elif semantic_aliases:
        nm = strip_addr(sorted(semantic_aliases, key=len)[0])
        layer = "named_syms.txt/symbol_addrs.txt alias only"
    else:
        nm = glabel
        layer = "glabel"

    info = dict(info)
    base = strip_addr(nm)
    ev = []
    origin = "unknown"
    tier = "INFERRED"
    action = "KEEP"
    proposed = ""

    common = dict(address="0x" + addr, glabel=glabel, current_name=nm,
                  name_layer=layer, aliases=";".join(sorted(set(semantic_aliases)))[:300],
                  insns=info["insns"], src_location=src_def.get(nm, ""),
                  queued="yes" if (nm in queue_funcs or glabel in queue_funcs) else "")

    # --- libscan: bit-verbatim PsyQ module identity.
    # Checked FIRST, and before the AUTO short-circuit — a FILL row is precisely a
    # function whose glabel is still `func_XXXXXXXX`. Nothing below can demote this:
    # every other tier reasons from behaviour or provenance, this one reads the name
    # off Sony's own OBJ record for bytes that are identical to Sony's own module.
    if addr in libscan:
        e = libscan[addr]
        ev = [libscan_evidence(addr)]
        WHY = {
            "CONFIRM": "already carries this name — no action",
            "FILL": "no naming claim existed at this address",
            "CONTRADICTED": "the glabel asserts a DIFFERENT name; this rename retires a misname",
            "CONTRADICTED_ALIAS": "the glabel is auto but a live alias asserts a DIFFERENT "
                                  "name; this rename fills the glabel and retires the alias",
        }
        ev.append(WHY.get(e["classification"], e["classification"]))
        if e["note"]:
            ev.append(e["note"][:200])
        if nm in name_in_binary:
            ev.append("cross-check: the in-binary self-identifying string agrees"
                      if norm(nm).endswith(norm(e["name"])) else
                      "CONFLICT: this address also carries an in-binary self-identifying "
                      "string that does NOT match the XDEF name — review before applying")
        same = (nm == e["name"]) or e["classification"] == "CONFIRM"
        rows.append(dict(common, origin="libscan-verbatim", tier="VERIFIED",
                         evidence="; ".join(ev)[:1000],
                         action="KEEP" if same else "RENAME",
                         proposed_name="" if same else e["name"]))
        continue

    # --- AUTO
    if AUTOPAT.match(nm):
        rows.append(dict(common, origin="splat-auto", tier="AUTO",
                         evidence="splat-generated placeholder; makes no semantic claim",
                         action="KEEP", proposed_name=""))
        continue

    # --- collect misnomer flags
    # A MISNAMED flag condemns the SYMBOL it is attached to. If it sits on a sibling
    # alias at the same address, the currently-displayed name is often the CORRECTION
    # that flag points to — so it must not be demoted by its own fix.
    flagged = ""
    sibling_flag = ""
    for (s, sf, i, c) in addr_syms.get(addr, []):
        if not MISFLAG.search(c):
            continue
        if strip_addr(s) == base:
            flagged = f"{sf}:{i} {c[:220]}"
            break
        if not sibling_flag:
            sibling_flag = f"{sf}:{i} [on sibling alias '{s}'] {c[:200]}"

    krow = kengo.get(nm)
    kengo_derived = False
    if krow and krow.get("kengo_name"):
        if base == krow["kengo_name"] or base.startswith(krow["kengo_name"]) or nm == krow["kengo_name"]:
            kengo_derived = True
    if base in kengo_names or nm in kengo_names:
        kengo_derived = True
    if nm in kengo_decisions or base in kengo_decisions:
        kengo_derived = True
    # Disambiguated variants of a Kengo name (foo_2, foo_3, fooB, DispX_A) inherit the
    # Kengo provenance — the semantic CLAIM is the same one, just re-used at another address.
    kvariant = ""
    if not kengo_derived:
        for cand in {re.sub(r"_\d+$", "", base), re.sub(r"_[A-Z]$", "", base),
                     re.sub(r"[A-Z]$", "", base)}:
            if cand and cand != base and cand in kengo_names:
                kengo_derived = True
                kvariant = cand
                break

    # --- legacy-renamer provenance band (evidence about WHERE the name came from,
    # so it outranks any prefix-shape heuristic below).
    rband = rename_band.get(nm) or rename_band.get(base) or ""
    if rband.startswith("kengo:"):
        kengo_derived = True
        ev.append(f"tools/rename_funcs.py RENAMES map, KENGO band — '{rband.split(':',1)[1][:120]}'")

    # --- VERIFIED sources
    if nm in name_in_binary:
        origin, tier = "in-binary-string", "VERIFIED"
        ev.append(name_in_binary[nm])
    elif nm == "main":
        origin, tier = "hardware-role", "VERIFIED"
        ev.append("the sole jal target of _start other than bios_InitHeap "
                  "(asm/funcs/_start.s) — a crt0's final call is main(). Applied by the "
                  "phase-2 naming wave; was cpu_set_move_command_and_dir_for_no_action_2. "
                  "Naming it literally `main` is codegen-neutral under this cc1 (measured: "
                  "no expand_main_function/__main injection)")
    elif nm == "_start":
        origin, tier = "hardware-role", "VERIFIED"
        ev.append("PS-EXE entry point 0x800836EC per AGENTS.md header; crt0 shape (BSS zero, $sp/$gp/$fp setup, jal main, break 0,1); canonical-asm authorized 2026-08-06")
    elif nm in psyq and psyq[nm][0] in ("syscall_kernel", "bios_table_call") :
        origin, tier = "psyq-signature", "VERIFIED"
        ev.append(f"known_psyq_stdlib.txt: {psyq[nm][0]} — {psyq[nm][1]}")

    # --- SUSPECT overrides (a recorded contradiction beats everything but a hard fact).
    # VERIFIED is in-binary/hardware fact: Kengo provenance cannot demote it, it only
    # means Kengo happened to reuse the same (correct) SDK name.
    if tier == "VERIFIED":
        if kengo_derived:
            ev.append("NOTE: name also appears in kengo_matches.csv, but in-binary/hardware evidence is dispositive — Kengo reused the same PsyQ SDK name")
        if flagged:
            ev.append("NOTE: a misname flag exists at this address for a DIFFERENT alias symbol; this glabel's own evidence is dispositive")
            tier = "SUSPECT"
            action = "RESET"
            ev.append("RECORDED CONTRADICTION: " + flagged)
    elif flagged:
        origin_note = "misname-flag"
        tier = "SUSPECT"
        origin = f"{origin}+{origin_note}" if origin not in ("unknown",) else origin_note
        ev.append("RECORDED CONTRADICTION: " + flagged)
        action = "RESET"
    elif kengo_derived:
        origin = "kengo-derived"
        tier = "SUSPECT"
        conf = krow.get("confidence", "?") if krow else "?"
        kn = krow.get("kengo_name", "?") if krow else "?"
        diff = krow.get("diff", "?") if krow else "?"
        cs = krow.get("combined_score", "") if krow else ""
        if kvariant:
            ev.append(f"disambiguated variant of Kengo name '{kvariant}' (same semantic claim re-used at a second address — the #1 false-positive shape per docs/naming/README.md)")
        ev.append(f"kengo_matches.csv: kengo_name={kn} confidence={conf} insn_diff={diff} combined_score={cs or '0'}")
        dec = kengo_decisions.get(nm) or kengo_decisions.get(base)
        if dec:
            ev.append(f"kengo_name_decisions.csv: {dec['decision']}/{dec['confidence']} — {dec['reason'][:140]}")
            if dec["decision"] == "keep":
                tier = "INFERRED"
                ev.append("reviewed-keep: downgraded SUSPECT->INFERRED by recorded review")
        action = "RESET" if tier == "SUSPECT" else "KEEP"
    elif PS2ONLY.match(nm):
        origin, tier, action = "kengo-derived(ps2-only-prefix)", "SUSPECT", "RESET"
        ev.append("PS2-only symbol family (tsl*/su*/Vu0/Vu1 are Kengo/PS2 engine names); no PS1 basis")
    elif tier != "VERIFIED":
        # remaining named functions
        if nm in psyq:
            origin, tier = "psyq-idiom-scan", "CORROBORATED"
            ev.append(f"known_psyq_stdlib.txt: {psyq[nm][0]} — {psyq[nm][1]}")
        elif "verified via" in rband:
            origin, tier = "legacy-renamer-map(verified band)", "CORROBORATED"
            ev.append(f"tools/rename_funcs.py RENAMES map — '{rband.split(':',1)[1][:140]}' "
                      "(opcode-level behavioral verification recorded in the band header)")
        elif addr in analyzer and not PSYQ_STRICT.match(base):
            a = analyzer[addr]
            conf = (a["confidence"] or "").lower()
            meth = a["method"] or ""
            origin = "naming-analyzer(" + a["src"].replace(".csv", "") + ")"
            if conf == "high" and "manual_re" in meth:
                tier = "CORROBORATED"
                ev.append(f"docs/naming/{a['src']}: manual reverse-engineering, confidence=high — {a['evidence']}")
            elif conf == "info" or "data_coaccess" in meth:
                tier = "INFERRED"
                ev.append(f"docs/naming/{a['src']}: WEAK — {meth}; {a['evidence']} (data/call affinity only, no body semantics)")
            else:
                tier = "INFERRED"
                ev.append(f"docs/naming/{a['src']}: confidence={conf or 'n/a'} method={meth}; {a['evidence']}")
            if PSYQ_PREFIX.match(base):
                ev.append("NOTE: name wears a PsyQ-style family prefix but its recorded origin is the "
                          "naming analyzer, NOT an SDK signature match — the prefix asserts more than the evidence")
        elif PSYQ_PREFIX.match(nm):
            origin = "psyq-family-prefix"
            if nm.startswith("cdrom_"):
                # The CD-ROM cluster had a recorded body spot-check.
                tier = "CORROBORATED"
                ev.append("PsyQ libcd family; bodies spot-checked against the g_cd_index_reg/"
                          "g_cd_irq_reg/g_cd_dma_ctrl register-pointer block "
                          "(docs/naming/kengo-rename-audit-2026-07-13.md, apply record 2026-07-13)")
            else:
                tier = "INFERRED"
                ev.append("PsyQ family prefix ONLY (linked libs per AGENTS.md) — the name asserts a "
                          "specific SDK entry point with no recorded body verification; REVIEW")
        elif nm in rename_targets or base in rename_targets:
            bd = rename_band.get(nm) or rename_band.get(base) or "unbanded"
            if bd.startswith("kengo:"):
                origin, tier, action = "legacy-renamer-map(kengo band)", "SUSPECT", "RESET"
                ev.append(f"tools/rename_funcs.py RENAMES map, KENGO band — '{bd.split(':',1)[1][:120]}'")
            elif "verified via" in bd:
                origin, tier = "legacy-renamer-map(verified band)", "CORROBORATED"
                ev.append(f"tools/rename_funcs.py RENAMES map — '{bd.split(':',1)[1][:140]}' (opcode-level behavioral verification recorded in the band header)")
            else:
                origin, tier = "legacy-renamer-map", "INFERRED"
                ev.append(f"tools/rename_funcs.py RENAMES map, band '{bd[:120]}'; see docs/naming/kengo-rename-audit-2026-07-13.md")
        elif addr in analyzer:
            a = analyzer[addr]
            conf = (a["confidence"] or "").lower()
            meth = a["method"] or ""
            origin = "naming-analyzer(" + a["src"].replace(".csv", "") + ")"
            if conf == "high" and "manual_re" in meth:
                tier = "CORROBORATED"
                ev.append(f"docs/naming/{a['src']}: manual reverse-engineering, confidence=high — {a['evidence']}")
            elif conf == "info" or "data_coaccess" in meth:
                tier = "INFERRED"
                ev.append(f"docs/naming/{a['src']}: WEAK — {meth}; {a['evidence']} (data/call affinity only, no body semantics)")
            else:
                tier = "INFERRED"
                ev.append(f"docs/naming/{a['src']}: confidence={conf or 'n/a'} method={meth}; {a['evidence']}")
            if a["name"] and strip_addr(a["name"]) != base:
                ev.append(f"NOTE: analyzer proposed '{a['name']}' but the applied name differs")
        else:
            origin, tier = "unattributed", "INFERRED"
            ev.append("named outside every recorded evidence path (Kengo / PsyQ / renamer map / "
                      "naming-analyzer CSVs); provenance UNKNOWN — cannot be defended, treat as review-required")
            intro = git_intro(nm)
            if intro:
                ev.append("introduced by " + intro)

    if sibling_flag and not flagged:
        ev.append("CONTEXT: a different alias at this address is flagged MISNAMED — "
                  "the displayed name may be that flag's correction: " + sibling_flag)

    if base in misnomer_doc and tier != "SUSPECT":
        ev.append("mentioned in docs/naming/MISNOMERS.md — review")

    if nm in canon:
        ev.append("canonical-asm authorized: " + canon[nm][:180])

    if src_def.get(nm):
        ev.append("body at " + src_def[nm])

    # descriptive-but-weak generic names
    if tier == "INFERRED" and re.match(r"^(get_global|set_global|stub|copy|empty_stub|helper|sub|local)_", nm):
        ev.append("generic auto-descriptive name; weak semantic claim")

    if tier == "SUSPECT":
        action = "RESET"
        proposed = ""

    rows.append(dict(common, origin=origin, tier=tier,
                     evidence="; ".join(ev)[:1000], action=action, proposed_name=proposed))

# ------------------------------------------------- libscan rows outside the asm/funcs universe
# The universe above is asm/funcs/*.s, so a function that reached COMPLETED-C — pure C in
# src/, no split file left — has no census row at all. 16 libscan placements land on exactly
# such functions, including some of the campaign's worst misnames (memcard_SetBusy is really
# SsSetMono). Skipping them would silently drop the strongest evidence in the set, so they get
# rows here. They carry no glabel, which is correct: there is no .s file and nothing for the
# wave to rename in asm/.
_covered = {r["address"].upper().replace("0X", "") for r in rows}
# These rows have no glabel to read a live name from, so the CURRENT name comes from the
# link map — `<addr> <name>` lines are object-provided definitions. Reading it (rather than
# trusting the manifest's current_name, which is a snapshot from whenever the scan last ran)
# is what lets the census self-heal after a wave: otherwise these 16 rows keep proposing a
# rename that already landed.
_mapdef = defaultdict(list)
_mp = J("build", "bb2.map")
if os.path.exists(_mp):
    MAPDEF = re.compile(r"^\s+0x0*([0-9a-f]{8})\s+([A-Za-z_]\w*)\s*$")
    for ln in open(_mp, encoding="utf-8", errors="replace"):
        m = MAPDEF.match(ln)
        if m:
            _mapdef[m.group(1).upper()].append(m.group(2))
_lsrows = {}
if os.path.exists(lp):
    with open(lp, newline="", encoding="utf-8", errors="replace") as fh:
        for r in csv.DictReader(fh):
            _lsrows[(r.get("addr") or "").strip().upper().replace("0X", "")] = r
for a in sorted(libscan):
    if a in _covered:
        continue
    e = libscan[a]
    mr = _lsrows.get(a, {})
    live = [n for n in _mapdef.get(a, []) if not AUTOPAT.match(n)] or _mapdef.get(a, [])
    nm = (live[0] if live else (mr.get("current_name") or "")).strip()
    if not nm:
        continue
    al = sorted({x.strip() for x in (mr.get("aliases") or "").split(";") if x.strip()}
                | {s for (s, sf, i, c) in addr_syms.get(a, [])
                   if not AUTOPAT.match(strip_addr(s))})
    same = (nm == e["name"]) or e["classification"] == "CONFIRM"
    rows.append(dict(
        address="0x" + a, glabel="", current_name=nm,
        name_layer=mr.get("name_source", "src/*.c definition (no asm/funcs split file)"),
        aliases=";".join(al)[:300], insns="", src_location=src_def.get(nm, ""),
        queued="yes" if nm in queue_funcs else "",
        origin="libscan-verbatim", tier="VERIFIED",
        evidence="; ".join([
            libscan_evidence(a),
            "outside the asm/funcs universe — decompiled to pure C in src/, so this row is "
            "seeded from the libscan manifest rather than a glabel",
            e["classification"]])[:1000],
        action="KEEP" if same else "RENAME",
        proposed_name="" if same else e["name"]))
rows.sort(key=lambda r: r["address"])

# ------------------------------------------------- BIOS jumptable index (second chain)
# Two addresses where the BIOS index decode and the libscan XDEF name DIFFER without
# contradicting each other: the library wrapper exports one name and trampolines to a
# BIOS entry that has another. Owner ruling 2026-08-07 takes the XDEF (the symbol the
# original linker actually placed) and requires the jumptable entry recorded so the
# second evidence chain is not lost. Hardcoded deliberately: the full index
# (tmp/bios_decode/bios_names.csv) is promoted in the addendum pass, not this wave.
BIOS_INDEX = {
    "80078948": "BIOS jumptable cross-check: A0:0x43 DoExecute — the library wrapper "
                "exported as Exec trampolines to it; the two names agree on the function, "
                "not on the spelling, and the XDEF is what the link placed here",
    "80078968": "BIOS jumptable cross-check: A0:0x9F SetMemSize — the library wrapper "
                "exported as SetMem trampolines to it; same relationship as Exec/DoExecute "
                "and as DelDrv/RemoveDevice at 0x8008D060",
}
for r in rows:
    a = r["address"].upper().replace("0X", "")
    if a in BIOS_INDEX:
        r["evidence"] = (r["evidence"] + "; " + BIOS_INDEX[a])[:1200]

# The full BIOS jumptable decode — the second census evidence tier, promoted
# 2026-08-07 from tmp/bios_decode (method: docs/naming/bios_decode/README.md).
# Evidence-append ONLY: it corroborates or cross-checks, it never changes an
# action or tier on its own. The two hand-written BIOS_INDEX entries above
# carry richer prose and take precedence.
_bd = J("docs", "naming", "bios_decode", "bios_names.csv")
if os.path.exists(_bd):
    with open(_bd, newline="", encoding="utf-8", errors="replace") as fh:
        _bios_rows = {(r2.get("addr") or "").upper().replace("0X", ""): r2
                      for r2 in csv.DictReader(fh)}
    for r in rows:
        a = r["address"].upper().replace("0X", "")
        br = _bios_rows.get(a)
        if br and a not in BIOS_INDEX:
            r["evidence"] = (r["evidence"] +
                             "; BIOS jumptable cross-check (%s): %s:%s -> %s"
                             % (br.get("status", ""), br.get("table", ""),
                                br.get("index", ""), br.get("spec_name", "")))[:1200]

# ---------------------------------------------------------------- targeted overrides
OVERRIDES = {
    "cpu_set_move_command_and_dir_for_no_action_2": dict(
        tier="SUSPECT", origin="kengo-derived", action="RENAME",
        proposed_name="main",
        extra="DECISIVE RENAME: this is the sole jal target of _start (asm/funcs/_start.s:42, jal 0x80017200) — a crt0's final call is main(). kengo_matches.csv gives kengo_name=gnd_land_hit_char_tsuba affinity-unique combined_score=0.00 (no basis). Current name is unrelated to both."),
    "ang_hosei": dict(
        tier="SUSPECT", origin="kengo-derived", action="RESET",
        extra="CONFIRMED MISLEAD (owner): break-0x107 Marionation engine file-IO trampoline, not angle correction. inline_asm_canonical.txt:353 + docs/grind/auth-packets-2026-08-06.md batch 2. Call sites src/ings.c:141,143,170 are file-descriptor reads."),
    "game_2d_CheckLifeGaugeNoDisp": dict(
        tier="SUSPECT", origin="kengo-derived", action="RESET",
        extra="CONFIRMED MISLEAD (owner): LIBGTE 3x3 matrix-vector multiply leaf (ctc2/mvmva/swc2), not a UI predicate. inline_asm_canonical.txt:346 + auth-packets-2026-08-06.md batch 2."),
    "_SpuCallback": dict(
        tier="SUSPECT", origin="libscan-rejected", action="RESET",
        extra="DEMOTED RENAME (addendum 2026-08-07): the fe40a52b wave applied _SpuCallback from the LIBSPU/S_CB island placement at 0x800469A0, but the placement fails reachability (zero j/jal callers, zero word-sized data refs to 0x800469A0 in the whole image) and the body calls func_80045510 (game text, arg 9), not InterruptCallback as S_CB's sole external ref requires. It is a dead game one-liner byte-identical to the Sony stub. docs/naming/libscan/ambiguous_resolutions.md; tools/libscan/manifest.py filter 1 reproduces the rejection mechanically. RESET to func_800469A0."),
}
by_name = {r["current_name"]: r for r in rows}
for nm, ov in OVERRIDES.items():
    r = by_name.get(nm)
    if not r:
        # Expected once the phase-2 wave has applied that row: the old name is gone.
        print(f"note: override target '{nm}' no longer present (wave applied?)", file=sys.stderr)
        continue
    extra = ov.pop("extra", "")
    r.update(ov)
    if extra:
        r["evidence"] = (extra + " | " + r["evidence"])[:1200]

# ---------------------------------------------------------------- write
outdir = J("docs", "naming")
os.makedirs(outdir, exist_ok=True)
outp = os.path.join(outdir, "function-names.csv")
cols = ["address", "current_name", "glabel", "name_layer", "aliases", "insns",
        "src_location", "origin", "tier", "evidence", "action", "proposed_name", "queued"]
with open(outp, "w", newline="", encoding="utf-8") as fh:
    w = csv.DictWriter(fh, fieldnames=cols, lineterminator="\n")
    w.writeheader()
    for r in rows:
        w.writerow(r)

tc = Counter(r["tier"] for r in rows)
oc = Counter(r["origin"] for r in rows)
ac = Counter(r["action"] for r in rows)
print("universe:", len(rows))
print("TIERS:", dict(tc))
print("ACTIONS:", dict(ac))
print("ORIGINS:")
for k, v in oc.most_common():
    print(f"  {v:5d}  {k}")
print("\nSUSPECT sample:")
for r in rows:
    if r["tier"] == "SUSPECT":
        print(" ", r["address"], r["current_name"], "|", r["origin"])
