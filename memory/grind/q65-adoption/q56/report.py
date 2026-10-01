#!/usr/bin/env python3
"""report.py: build tmp/q56/RESULTS.md and tmp/q56/dead_rows.txt from results.json + census.json."""
import json, re
Q = "/tmp/q56"
OUT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56"
R = json.load(open(Q + "/results.json"))
C = json.load(open(Q + "/census.json"))
commit = open(Q + "/commit.txt").read().strip()
dead_lines = [int(x) for x in open(Q + "/dead_linenos.txt").read().split()]
queue = json.load(open("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/engine/queue.json"))
items = queue if isinstance(queue, list) else queue.get("items", queue)
qstat = {}
if isinstance(items, dict):
    qstat = {k: v.get("status") for k, v in items.items()}
else:
    qstat = {(it.get("func") or it.get("name")): it.get("status") for it in items}
canon = set(l.split()[0] for l in open("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/inline_asm_canonical.txt")
            if l.strip() and not l.startswith("#"))
DUP = {73: 56, 104: 80}

def status(func, info):
    if info["include_asm"]:
        if func in canon:
            return "INCLUDE_ASM (canonical)"
        return f"INCLUDE_ASM (queue: {qstat.get(func, '?')})"
    return "C" + (f" (queue: {qstat[func]})" if func in qstat else "")

def hi_removed(diff, sym):
    return sum(1 for l in diff.splitlines() if re.fullmatch(r"-R_MIPS_HI16\s+" + re.escape(sym), l.strip()))

def flag(c):
    if c["same_tu_gp_users"]:
        return ("LEAN-FAIL (TU-inconsistent)",
                "same-file gp user(s) " + ", ".join(c["same_tu_gp_users"][:4]) + ("..." if len(c["same_tu_gp_users"]) > 4 else "")
                + ": per-file ASPSX cannot give the base access gp there and not here in one file; row stands only if a file boundary is proven between them")
    return ("NEEDS JUDGMENT (TU-consistent)",
            "no gp user of the address in this TU: equals 'S not defined in F's file' — ASPSX-reproducible in principle; needs the (a)(2) cc1psx+ASPSX run")

rows = R["rows"]; rt = R["row_tests"]; st = R["sym_tests"]
live_rows, dead_rows = [], []
for n, info in rows.items():
    t = rt[n]
    n_i = int(n)
    is_live = not t["obj_identical"]
    if n_i in (56, 80):
        is_live = True  # live once its exact duplicate (73 / 104) is deleted (func-level test)
    (live_rows if is_live else dead_rows).append(n_i)

L = []
L.append("# Q56 step 1 — sdata_exclude.txt liveness audit (+ step-2 evidence prep)\n")
L.append(f"Scratch commit: `{commit}` (main HEAD at start). Scratch tree `/tmp/q56/tree` (WSL-native; "
         "`git archive` + symlinked tools/gcc-2.7.2, .venv, disc). No worktree, no main-tree tracked edits, no commit.\n")
L.append("## Baseline and method\n")
L.append("- **Scratch baseline:** clean `make build/bb2.exe` → exe SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` "
         "(== oracle), bin SHA1 `42fce5aff1490a579e919b68af56ebc5b0dc657f` (`/tmp/q56/baseline.sha1`).")
L.append("- **Per-row test (incremental):** for each row, a copy of sdata_exclude.txt without that line; the owning TU "
         "(the C object defining the function, via `nm` of the reference objects) is rebuilt with the Makefile's own recipe "
         "(`make BUILD_DIR=<scratch> MASPSX_FLAGS=… MASPSX_FLAGS_GP=…` with only `--sdata-exclude=` repointed) and the .o "
         "compared byte-for-byte with the reference .o. Differing objects were relinked (reference build dir, object "
         "swapped, the Makefile's ld/objcopy) and the .bin compared: every differing object also changed the linked bin.")
L.append("- **Method controls:** (1) all 20 affected TUs rebuilt with the UNMODIFIED list → 20/20 objects identical; "
         "(2) full clean build without row 5 (live) → exe `637575f6…`, bin identical to the incremental relink bin; "
         "(3) full clean build without row 18 (dead) → oracle SHA1.")
L.append("- **Sanity bound (all 105 rows removed, full clean build):** exe `c1819c29…` (≠ oracle); 18 objects differ; "
         "exactly **74 functions** change (branch-target shifts normalised) — identical to the set of live-row functions "
         "(72 single-row live + func_8006EACC + func_80070C70). No function outside the list changes.")
L.append("- **Proposed deletion verified by FULL clean build:** the 31 lines in `dead_rows.txt` deleted → exe SHA1 "
         "`62efab4f73f992798c43e8c730aa43baa10bb4fa` (oracle). Additionally deleting the 11 dead symbol entries inside "
         "live rows (section 'Dead symbols inside live rows') → also oracle SHA1.")
L.append("- **Duplicates:** lines 56/73 (`func_8006EACC: D_800A36AC`) and 80/104 (`func_80070C70: g_gpu_ot_ptr`) are "
         "exact duplicates: each alone tests dead, removing both is live (bin changes). Keep 56 and 80, delete 73 and 104.")
L.append("- **Step-2 census (`census.json`):** shipped bytes = `asm/funcs/*.s`; symbols compared by resolved address. "
         "ASPSX model used for the flag (memory/grind/func_80036140/research-common-gp.md §0, Sony ASPSX 2.34 run under dosemu2): "
         "a file gets gp for a symbol only if it DEFINES it (`.comm`/`.lcomm`/`.sdata`); an `extern` symbol is never gp. "
         "So within one original file, the base access of a symbol is gp in every function or in none.\n")
L.append(f"## Counts\n\n- Rows: **105** (103 functions; 160 symbol entries).\n- **LIVE: {len(live_rows)}** "
         f"(72 live individually + lines 56 and 80, live once their duplicate is gone).\n- **DEAD (delete): {len(dead_rows)}** — "
         "exact lines in `dead_rows.txt`.\n- Dead symbol entries inside live rows: 11 (optional trim, oracle-verified).\n"
         "- Rows whose removal breaks the build: **none** (every variant compiled and linked).\n")

# DEAD table
L.append("## DEAD rows (31)\n")
L.append("| line | row | TU | function status | why inert |")
L.append("|---|---|---|---|---|")
for n_i in sorted(dead_rows):
    n = str(n_i); info = rows[n]; f = info["func"]
    why = []
    if n_i in DUP:
        why.append(f"exact duplicate of line {DUP[n_i]}")
    elif info["include_asm"]:
        why.append("body is INCLUDE_ASM — maspsx never rewrites it")
        # would it be live on landing?
        lv = []
        for s in info["syms"]:
            c = C[f"{n}:{s}"]
            if info["in_sdata_funcs"] and info["syms_in_sdata_syms"][s] and c.get("own_lo_access", 0) > 0 and c.get("own_gp", 0) == 0:
                lv.append(s)
        if lv and f in canon:
            why.append("canonical-asm (COMPLETED-INLINE-ASM-CANONICAL): the body stays asm, the row can never apply")
        elif lv:
            why.append("**shipped bytes access " + ", ".join(lv) + " non-gp while F is in sdata_funcs: the row is likely "
                       "needed again if this function lands as C (it would then have to meet (a)-(d))**")
    elif not info["in_sdata_funcs"]:
        why.append("function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members)")
    else:
        nos = [s for s, v in info["syms_in_sdata_syms"].items() if not v]
        if nos and len(nos) == len(info["syms"]):
            why.append("symbol(s) not in sdata_syms.txt, never gp-eligible")
        else:
            parts = []
            for s in info["syms"]:
                c = C[f"{n}:{s}"]
                if not info["syms_in_sdata_syms"][s]:
                    parts.append(f"{s}: not in sdata_syms")
                else:
                    parts.append(f"{s}: shipped F refs lo-access={c.get('own_lo_access')} la={c.get('own_la')} gp={c.get('own_gp')}")
            why.append("inert for our C: without the row maspsx gp-converts nothing here (no direct access under that "
                       "name — la/indexed only, or a merged/aliased spelling). Shipped refs: " + "; ".join(parts))
    L.append(f"| {n} | `{info['text'].strip()}` | {', '.join(info['tus'])} | {status(f, info)} | {'; '.join(why)} |")

# live table
L.append("\n## LIVE rows (74) — bytes that change + step-2 evidence\n")
L.append("Removing a live symbol turns each listed direct access `lui rX,%hi(S); l/s rY,%lo(S)(rX)` into one "
         "`l/s rY,%gp_rel(S)($gp)` (function shrinks one word per access; later branches shift). Columns: **chg** = "
         "accesses that flip (HI16 relocs removed when that symbol alone is dropped); **shipped F** = F's own refs to S's "
         "address in the original bytes (lo-access / la / gp); **gp users** = functions anywhere that access S's address "
         "gp-relative (count, their TUs); **same-TU gp** = gp users built into F's own TU.\n")
L.append("| line | function | TU | status | symbol | sym live? | chg | shipped F (lo/la/gp) | F gp refs (any sym) | gp users (n; TUs) | same-TU gp | flag |")
L.append("|---|---|---|---|---|---|---|---|---|---|---|---|")
flags_count = {}
for n_i in sorted(live_rows):
    n = str(n_i); info = rows[n]; f = info["func"]
    t = rt[n] if n_i not in (56, 80) else R["func_tests"][f]
    for s in info["syms"]:
        c = C[f"{n}:{s}"]
        if len(info["syms"]) > 1:
            sr = st.get(n, {}).get(s, {})
            live = not sr.get("obj_identical", True)
            chg = hi_removed(sr.get("funcdiff", {}).get("diff", ""), s) if live else 0
        else:
            live = True
            chg = hi_removed(t["funcdiff"]["diff"], s)
        if live:
            fl, why = flag(c)
            flags_count[fl] = flags_count.get(fl, 0) + 1
        else:
            fl, why = "— (dead symbol; trim)", ""
        L.append(f"| {n} | {f} | {info['tus'][0]} | {status(f, info)} | {s} | {'LIVE' if live else 'dead'} | {chg} | "
                 f"{c.get('own_lo_access')}/{c.get('own_la')}/{c.get('own_gp')} | {c['func_gp_refs_any_sym']} | "
                 f"{c.get('gp_users_total')}; {', '.join(c.get('gp_user_tus', []))} | "
                 f"{', '.join(c.get('same_tu_gp_users', [])) or '—'} | {fl} |")

L.append("\n## Per-symbol view (distinct live symbols)\n")
L.append("gp users = every function whose SHIPPED bytes reach the symbol's address gp-relative (under the ASPSX model its "
         "original file defined the symbol). The last column lists functions in those same TUs whose shipped bytes make a "
         "direct non-gp (lui/%lo load/store) access — a per-file model cannot put them in the defining file unless the "
         "access is indexed.\n")
L.append("| symbol | addr | live exclusions | gp users (function (TU)) | non-gp direct refs in the gp users' TUs |")
L.append("|---|---|---|---|---|")
seen = {}
for n_i in sorted(live_rows):
    n = str(n_i); info = rows[n]
    for s_ in info["syms"]:
        if len(info["syms"]) > 1 and st.get(n, {}).get(s_, {}).get("obj_identical", False):
            continue
        seen.setdefault(s_, (C[f"{n}:{s_}"], []))[1].append(info["func"])
for s_, (c, fs) in sorted(seen.items()):
    L.append(f"| {s_} | {c['addr']} | {len(fs)} | {', '.join(c.get('gp_users', []))} | "
             f"{', '.join(c.get('gp_tu_nongp_direct', [])) or '—'} |")
L.append("\nNote D_800A3820: func_80044800 (text1a_c, not in sdata_funcs) accesses it directly non-gp "
         "(0x80044AC4/0x80044AD4, `lw`/`sw` via lui) while func_80044504 in the same object uses gp (0x80044634) — "
         "under the per-file model these two cannot share an original file; that is a text1a_c TU-boundary fact, "
         "independent of the exclusion rows (whose functions are in other TUs).\n")
L.append("\n## Dead symbols inside live rows (11 entries; optional trim, oracle-verified jointly)\n")
L.append("line 5 func_80016E60: D_800A3770 · line 6 main: D_800A3770 · line 24 func_8003D39C: D_800A3930 · "
         "line 41 func_80063E10: D_800A344C, D_800A3454 · lines 55/61/64/67/71 (func_8006B898, func_8006C168, "
         "func_8006D338, func_8006D74C, func_8006E068): D_800A3518 · line 78 func_8006F97C: D_800A3588, D_800A358C. "
         "Variant file: `/tmp/q56/variants/cleaned_trim.txt`.\n")

L.append("## Step-2 flags — summary and reading\n")
for k, v in sorted(flags_count.items()):
    L.append(f"- {k}: **{v}** live (function, symbol) pairs")
L.append("""
- **(a) evidence status:** for NO live row is Sony-ASPSX evidence banked (no ledger runs cc1psx+ASPSX with the
  row's behaviour; the only ASPSX runs, research-common-gp.md, concern the COMMON `sym+N` rule). So as of today
  no row *clearly meets* (a). (a)(1)'s shipped signature holds for every live pair (F's own shipped refs to S are
  all non-gp — `gp` column is 0 throughout — while other functions reach S gp-relative).
- **What ASPSX would do:** per file, gp iff the file defines S. A live row therefore asserts "F's original file
  declared S `extern`". *TU-consistent* pairs (no gp user of S in F's current TU) are reproducible in principle
  by a per-file model and need the (a)(2)/(a)(3) cc1psx+ASPSX runs on the TU. *TU-inconsistent* pairs (a
  same-TU function reaches S gp-relative in the shipped bytes) cannot be reproduced by one ASPSX run over our
  current file; they hold only if the original file boundary lies between F and those gp users — a TU-split
  question, not an assembler-fidelity one. Neither is decided here.
- **(b)** the gate's single transform is structural: `_sdata_allowed_for_current_func` returns False for listed
  (F,S) — only removes gp, base and offset alike, keyed on `.ent` (INCLUDE_ASM bodies never touched). It is
  broader than the COMMON gate's (b) (which spared the base access).
- **(c)/(d) registration gap:** sdata_exclude.txt is in the Makefile `PIPELINE_DEPS` and mirrored in
  `engine/buildconfig.py`, but it is NOT in `engine/cheats.py MASPSX_GATE_LISTS`, not in grindlib `GATE_FILES`,
  and has no layer-2 PASS on record — (c)'s registration and (d)'s review are unmet for every row.
- **Aggregate pattern (all 88 live pairs are TU-consistent):** every gp user of every live symbol sits in a
  DIFFERENT TU from all the functions excluded for it — mostly `ings` (g_gpu_ot_ptr, D_800A36AC, D_800A3834,
  D_800A38DC, D_800A3690/36F1/36F9, g_disp_fade), `text1a_c` (D_800A3708/378C/3790/3820), `code6cac_c`
  (g_memcard_fd) and the 0x80035480-0x80035828 TUs (D_800A31DA). In aggregate the list is exactly what the
  ASPSX per-file rule ("gp only in the file that defines the symbol") predicts, i.e. it patches the global
  sdata_syms x sdata_funcs over-grant. That suggests a global per-file model could replace the list (an owner
  question under the Q62 pattern; not evaluated here). The only per-file contradiction found is outside the
  rows: D_800A3820 in text1a_c (func_80044504 gp vs func_80044800 non-gp).
- **Surprises:** (1) two exact duplicate rows; (2) six dead rows name INCLUDE_ASM functions — func_8006BD28 is
  canonical-asm (safe to drop); func_80065800, func_800693CC, func_80074E08, func_800759D0, func_800770B8 are
  queued, and their shipped bytes access the listed symbols non-gp, so the row is likely needed again on
  landing (func_80065800 has a ready landing package whose memory/grind/func_80065800/candidate.c names
  g_gpu_ot_ptr directly — deleting line 43 before it lands would very likely make that package miss the
  oracle; hold line 43, or the landing must re-add it under (a)-(d)); (3) 7 rows name functions not in sdata_funcs.txt and are structurally inert; (4) no row's removal
  breaks the build.
""")
CT = json.load(open(Q + "/candtest.json")) if __import__("os").path.exists(Q + "/candtest.json") else {}
def ct(f, k):
    r = CT.get(f, {}).get(k, {})
    return ("oracle" if r.get("oracle") else f"NOT oracle ({r.get('exe_sha1', 'no exe')[:8]})") if r else "—"
L.append("""
## Follow-up (B): the 5 dead rows naming still-queued INCLUDE_ASM functions — do their banked candidates need the row?

Measured by applying each banked landing package to fresh scratch copies of the pinned commit and building the
WHOLE EXE (`tmp/q56/candtest.py` -> `/tmp/q56/candtest.json`): (1) with the row, (2) with the row deleted,
(3) on the per-file-model POC tree (`MODEL.md`: no sdata lists at all).

| line | row | function | banked candidate | with row | row deleted | per-file model (no lists) | needs the row today? |
|---|---|---|---|---|---|---|---|""")
L.append(f"| 43 | `func_80065800: g_gpu_ot_ptr, D_800A3834` | func_80065800 (queue: active) | `candidate.c` + ready landing package "
         f"`tools/land.py` (body `tmp/f65800/final.c`) | {ct('func_80065800','list_row_kept')} | {ct('func_80065800','list_row_deleted')} | "
         f"{ct('func_80065800','perfile_model')} | **YES** — deleting line 43 breaks the package |")
L.append(f"| 92 | `func_800759D0: g_gpu_ot_ptr` | func_800759D0 (queue: active) | `candidate.c` + `landing.patch` (its 2 text1b_tu2.c "
         f"hunks respelling D_8009BCE4 are already on main) | {ct('func_800759D0','list_row_kept')} | {ct('func_800759D0','list_row_deleted')} | "
         f"{ct('func_800759D0','perfile_model')} | **YES** — deleting line 92 breaks the package |")
L.append("| 48 | `func_800693CC: D_800A350C, D_800A3518, D_800A36AC` | func_800693CC (queue: rotated) | none (evidence.md, "
         "hypotheses.md, pre-include-asm-body.c only) | — | — | — | no candidate; the shipped bytes access D_800A36AC non-gp, "
         "so a future C body naming it directly would need the D_800A36AC part (the other two symbols: la only) |")
L.append("| 90 | `func_80074E08: g_gpu_ot_ptr` | func_80074E08 (queue: active) | none at `candidate.c` (spellings under "
         "`ff-c-2026-09-30/`, not tested) | — | — | — | no banked candidate.c; shipped bytes access g_gpu_ot_ptr non-gp -> a C "
         "body naming it directly would need the row |")
L.append("| 95 | `func_800770B8: g_gpu_ot_ptr` | func_800770B8 (queue: active) | none (`ff-c-2026-09-30/f0_control.diff` only) | — | — "
         "| — | as line 90 |")
L.append("""
**Consequence for step 1's deletion list:** lines 43 and 92 are dead on today's tree but are LIVE for the ready
landing packages (both packages reach the oracle only with their row). Deleting them would force those landings to
re-add the rows under (a)-(d). Lines 48/90/95 are likely in the same position once their functions become C. Under the
per-file model (MODEL.md) none of the five rows is needed: both packages land at the oracle SHA1 with no list at
all, because their files (text1b_tu1c, text1b_tu2) do not define g_gpu_ot_ptr / D_800A36AC / D_800A3834 (those are
defined, i.e. gp-accessed, only in `ings`).
""")
open(OUT + "/RESULTS.md", "w", newline="\n", encoding="utf-8").write("\n".join(L) + "\n")

lines = open("/tmp/q56/tree/sdata_exclude.txt").read().split("\n")
dl = ["# Q56 step 1: exact sdata_exclude.txt lines to delete (line number in " + commit[:9] + "'s file, then the text).",
      "# Full clean build with all of them deleted: exe SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa (oracle)."]
for n_i in sorted(dead_rows):
    dl.append(f"{n_i}\t{lines[n_i-1]}")
open(OUT + "/dead_rows.txt", "w", newline="\n", encoding="utf-8").write("\n".join(dl) + "\n")
print("live", len(live_rows), "dead", len(dead_rows), flags_count)
