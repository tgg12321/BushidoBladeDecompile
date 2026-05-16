#!/usr/bin/env python3
"""render_naming_docs.py -- build markdown views of docs/naming/proposals.csv.

Outputs:
  docs/naming/proposals_high_confidence.md     -- high tier review table
  docs/naming/proposals_medium_confidence.md   -- medium tier review table
  docs/naming/subsystem_clusters.md            -- call-graph cluster analysis
  docs/naming/psyq_library_matches.md          -- PsyQ idiom matches
  docs/naming/data_symbols_quick_wins.md       -- data-symbol naming hints

Run after `tools/propose_function_names.py`.
"""
from __future__ import annotations

import csv
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs" / "naming"
PROPOSALS_CSV = DOCS / "proposals.csv"
UNDEF_SYMS = ROOT / "undefined_syms_auto.txt"
ASM_FUNCS = ROOT / "asm" / "funcs"
ASM_DATA_DIR = ROOT / "asm" / "data"


def load_proposals() -> list[dict]:
    if not PROPOSALS_CSV.exists():
        print(f"ERROR: {PROPOSALS_CSV} missing -- run propose_function_names.py first",
              file=sys.stderr)
        return []
    return list(csv.DictReader(PROPOSALS_CSV.open()))


def by_conf(rows: list[dict]) -> dict[str, list[dict]]:
    out: dict[str, list[dict]] = defaultdict(list)
    for r in rows:
        out[r.get("confidence", "none")].append(r)
    return out


def render_tier_md(rows: list[dict], tier_label: str, intro: str) -> str:
    out = []
    out.append(f"# Naming Proposals -- {tier_label} confidence")
    out.append("")
    out.append(intro)
    out.append("")
    out.append(f"Total {tier_label}: **{len(rows)}**")
    out.append("")
    # Group by leading evidence type
    by_kind: dict[str, list[dict]] = defaultdict(list)
    for r in rows:
        summary = r.get("evidence_summary", "")
        kinds = []
        for tok in summary.split(";"):
            tok = tok.strip()
            if "=" in tok:
                kinds.append(tok.split("=", 1)[0].strip())
        primary = kinds[0] if kinds else "unknown"
        by_kind[primary].append(r)
    for kind in sorted(by_kind, key=lambda k: -len(by_kind[k])):
        out.append(f"## Primary evidence: `{kind}` ({len(by_kind[kind])})")
        out.append("")
        out.append("| address | current | proposed | evidence_summary | evidence_file |")
        out.append("|---|---|---|---|---|")
        for r in sorted(by_kind[kind], key=lambda r: r["address"]):
            ev_file = r.get("evidence_detail_file", "")
            ev_link = f"[md]({ev_file.replace('docs/naming/', '')})" if ev_file else ""
            out.append(
                f"| `{r['address']}` "
                f"| `{r['current_name']}` "
                f"| `{r['proposed_name']}` "
                f"| {r['evidence_summary']} "
                f"| {ev_link} |"
            )
        out.append("")
    return "\n".join(out) + "\n"


def render_high(rows: list[dict]) -> str:
    intro = (
        "**High confidence**: deterministic pattern (BIOS jumptable, raw syscall, "
        "data-as-code, recognizable PsyQ stdlib idiom) OR a Kengo `name-unique` "
        "match with diff <=1 instruction. These are the lowest-risk renames; "
        "review and apply in small batches with SHA1 verification.\n\n"
        "**Risk**: BIOS A0/B0/C0 lookups are deterministic from the trampoline "
        "shape; PsyQ memcpy/memset proposals are address-suffixed to avoid name "
        "collisions if multiple wrappers exist for the same primitive.\n\n"
        "**Workflow**: copy the proposed name into `named_syms.txt`, run "
        "`make setup && make`, verify SHA1 unchanged, commit."
    )
    return render_tier_md(rows, "High", intro)


def render_medium(rows: list[dict]) -> str:
    intro = (
        "**Medium confidence**: weak Kengo match (size-diff > 1 but same name), "
        "single named caller (`sole_caller_path` proposal: `<caller>_helper_<addr>`), "
        "or call-graph subsystem cluster. These need callsite/body inspection "
        "before applying."
    )
    return render_tier_md(rows, "Medium", intro)


def parse_named_for_cluster() -> list[tuple[str, int]]:
    """Existing named functions: (name, addr) tuples."""
    sym_files = [ROOT / "symbol_addrs.txt", ROOT / "named_syms.txt"]
    pat = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
    seen: set[str] = set()
    out: list[tuple[str, int]] = []
    for sf in sym_files:
        if not sf.exists():
            continue
        for line in sf.read_text(errors="replace").splitlines():
            m = pat.match(line)
            if m:
                name, addr = m.group(1), int(m.group(2), 16)
                if name not in seen and not name.startswith("D_") and not name.startswith("g_str_"):
                    seen.add(name)
                    out.append((name, addr))
    return out


def render_subsystem_clusters(rows: list[dict]) -> str:
    """Identify call-graph clusters of unnamed functions surrounded by a
    named subsystem."""
    # Re-parse the call graph briefly
    sys.path.insert(0, str(ROOT / "tools"))
    from propose_function_names import (parse_all_funcs, build_call_graph,
                                          subsystem_prefix, _is_trustworthy_anchor)  # type: ignore
    funcs = parse_all_funcs()
    callers, callees = build_call_graph(funcs)
    unnamed = [f["name"] for f in funcs if f["name"].startswith("func_")]
    # For each unnamed, find dominant subsystem prefix in callers.
    # Same rule as caller_cluster() in the analyzer: trustworthy callers only,
    # dominant prefix needs >=2 callers AND >=50% share to count as a cluster.
    cluster: dict[str, list[str]] = defaultdict(list)
    no_pfx: list[str] = []
    sole_caller_cluster: dict[str, list[str]] = defaultdict(list)
    for fn in unnamed:
        all_cs = list(callers.get(fn, set()))
        cs = [c for c in all_cs if _is_trustworthy_anchor(c)]
        # Sole-caller case promotes that caller's subsystem (matches sole_caller_path)
        if len(cs) == 1:
            sp = subsystem_prefix(cs[0])
            if sp:
                sole_caller_cluster[sp].append(fn)
                continue
            no_pfx.append(fn)
            continue
        pfx_count = Counter()
        for c in cs:
            p = subsystem_prefix(c)
            if p:
                pfx_count[p] += 1
        if pfx_count:
            top, n = pfx_count.most_common(1)[0]
            if n >= 2 and n / len(cs) >= 0.5:
                cluster[top].append(fn)
            else:
                no_pfx.append(fn)
        else:
            no_pfx.append(fn)
    # Merge sole-caller cluster into main cluster (they both indicate a "this
    # function belongs to subsystem X" relationship).
    for k, v in sole_caller_cluster.items():
        cluster[k].extend(v)
    out = []
    out.append("# Subsystem Clusters (unnamed functions)")
    out.append("")
    out.append(
        "For each unnamed function, look at its named callers and "
        "find the dominant subsystem prefix (e.g., callers all start "
        "with `cpu_*` -> the function is likely a `cpu` helper)."
    )
    out.append("")
    out.append(f"Total unnamed: {len(unnamed)}")
    out.append(f"In a cluster: {sum(len(v) for v in cluster.values())}")
    out.append(f"No cluster signal: {len(no_pfx)}")
    out.append("")
    out.append("## Cluster sizes")
    out.append("")
    out.append("| subsystem | unnamed functions | rep names |")
    out.append("|---|---:|---|")
    for pfx in sorted(cluster, key=lambda k: -len(cluster[k])):
        if len(cluster[pfx]) < 2:
            continue
        reps = sorted(cluster[pfx])[:5]
        out.append(f"| `{pfx}_*` | {len(cluster[pfx])} | {', '.join(reps)}... |")
    out.append("")
    # Detail per top cluster
    out.append("## Per-subsystem detail")
    out.append("")
    for pfx in sorted(cluster, key=lambda k: -len(cluster[k])):
        if len(cluster[pfx]) < 3:
            continue
        out.append(f"### `{pfx}` ({len(cluster[pfx])} unnamed)")
        out.append("")
        for fn in sorted(cluster[pfx]):
            cs = sorted(callers.get(fn, set()))[:5]
            out.append(f"- `{fn}` called by {cs}")
        out.append("")
    return "\n".join(out) + "\n"


def render_psyq_library_matches(rows: list[dict]) -> str:
    out = []
    out.append("# PsyQ Library Match Proposals")
    out.append("")
    out.append(
        "High-confidence proposals that match PsyQ stdlib/BIOS idioms. "
        "These are the lowest-risk renames in the corpus. The detection "
        "heuristics in `tools/propose_function_names.py` are conservative "
        "and require both shape signals AND tight loop body pairing.\n"
    )
    out.append("")
    psyq_rows = [r for r in rows if r.get("proposed_name", "").startswith("psyq_")]
    bios_rows = [r for r in rows if r.get("proposed_name", "").startswith("bios_")]
    sys_rows = [r for r in rows if r.get("proposed_name", "").startswith("syscall_")]

    out.append(f"- PsyQ memcpy/memset: **{len(psyq_rows)}**")
    out.append(f"- BIOS A0/B0/C0 jumptable wrappers: **{len(bios_rows)}**")
    out.append(f"- Raw syscall/break wrappers: **{len(sys_rows)}**")
    out.append("")

    out.append("## BIOS jumptable wrappers")
    out.append("")
    out.append("| address | current | proposed BIOS name | evidence |")
    out.append("|---|---|---|---|")
    for r in sorted(bios_rows, key=lambda r: r["address"]):
        out.append(
            f"| `{r['address']}` | `{r['current_name']}` "
            f"| `{r['proposed_name']}` | {r['evidence_summary']} |"
        )
    out.append("")

    out.append("## PsyQ memcpy/memset")
    out.append("")
    out.append("| address | current | proposed | evidence |")
    out.append("|---|---|---|---|")
    for r in sorted(psyq_rows, key=lambda r: r["address"]):
        out.append(
            f"| `{r['address']}` | `{r['current_name']}` "
            f"| `{r['proposed_name']}` | {r['evidence_summary']} |"
        )
    out.append("")

    out.append("## Syscall / break wrappers")
    out.append("")
    out.append("| address | current | proposed | evidence |")
    out.append("|---|---|---|---|")
    for r in sorted(sys_rows, key=lambda r: r["address"]):
        out.append(
            f"| `{r['address']}` | `{r['current_name']}` "
            f"| `{r['proposed_name']}` | {r['evidence_summary']} |"
        )
    out.append("")
    return "\n".join(out) + "\n"


def scan_data_symbols() -> str:
    """Scan undefined_syms_auto.txt + .rodata.s + .data.s for naming hints."""
    out: list[str] = []
    out.append("# Data Symbol Naming Quick Wins")
    out.append("")
    out.append(
        "Scan of `undefined_syms_auto.txt` (1735 D_* labels) for cases "
        "where the access pattern in code gives away the underlying type "
        "or purpose. These are PROPOSALS, not renames. Apply with care.\n"
    )
    out.append("")
    if not UNDEF_SYMS.exists():
        out.append("(undefined_syms_auto.txt missing)")
        return "\n".join(out) + "\n"
    # Parse undef syms
    pat = re.compile(r"^\s*(D_[0-9A-Fa-f]+)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
    undef: dict[str, int] = {}
    for line in UNDEF_SYMS.read_text(errors="replace").splitlines():
        m = pat.match(line)
        if m:
            undef[m.group(1)] = int(m.group(2), 16)

    # Per-line scan: for any line that references a D_<addr> symbol, note
    # the mnemonic of that line as the kind of access. Faster and more
    # accurate than the prior O(N*M) approach. Also tracks function-pointer
    # patterns where `lw $vN, ..(D_xxx)` is followed within 3 lines by a
    # `jalr $vN` -- a common load-and-jumpvia-table idiom.
    use_patterns: dict[str, dict[str, int]] = defaultdict(Counter)
    sym_pat = re.compile(r"\b(D_[0-9A-Fa-f]{8})\b")
    mnem_pat = re.compile(r"/\*[^*]+\*/\s+(\S+)\s*(.*)")
    for f in ASM_FUNCS.glob("*.s"):
        lines = f.read_text(errors="replace").splitlines()
        for i, line in enumerate(lines):
            sm = sym_pat.search(line)
            if not sm:
                continue
            sym = sm.group(1)
            if sym not in undef:
                continue
            mm = mnem_pat.search(line)
            if not mm:
                continue
            mnem = mm.group(1).lower()
            ops = mm.group(2)
            use_patterns[sym][mnem] += 1
            # Function-pointer pattern: lw $vN, ... followed by jalr $vN soon.
            if mnem == "lw":
                reg_m = re.search(r"\$(v[01]|s[0-9]|t[0-9])", ops)
                if reg_m:
                    reg = reg_m.group(0)
                    for j in range(i + 1, min(i + 4, len(lines))):
                        if f"jalr {reg}" in lines[j] or re.search(
                                rf"jalr\s+{re.escape(reg)}\b", lines[j]):
                            use_patterns[sym]["fp_call"] += 1
                            break

    # Categorize. Mnemonics we care about:
    #   lwc2 / swc2 / mtc2 / mfc2  -> GTE matrix/vector candidate
    #   jalr / jal                  -> function pointer (rare for D_, usually a jumptable)
    #   lh / lhu                    -> short table (s16/u16)
    #   lb / lbu                    -> byte table (s8/u8)
    #   lw                          -> word table (s32 / pointer / packed)
    matrix_cands = []
    function_ptr_cands = []
    word_table_cands = []
    byte_table_cands = []
    short_table_cands = []
    for sym, addr in undef.items():
        p = use_patterns.get(sym, {})
        if not p:
            continue
        gte_uses = p.get("lwc2", 0) + p.get("swc2", 0) + p.get("mtc2", 0) + p.get("mfc2", 0)
        if gte_uses >= 1:
            matrix_cands.append((sym, addr, dict(p)))
            continue
        if p.get("fp_call", 0) >= 1 or p.get("jalr", 0) + p.get("jal", 0) >= 1:
            function_ptr_cands.append((sym, addr, dict(p)))
            continue
        b_uses = p.get("lb", 0) + p.get("lbu", 0) + p.get("sb", 0)
        h_uses = p.get("lh", 0) + p.get("lhu", 0) + p.get("sh", 0)
        w_uses = p.get("lw", 0) + p.get("sw", 0)
        if w_uses >= 3 and h_uses == 0 and b_uses == 0:
            word_table_cands.append((sym, addr, dict(p)))
        elif h_uses >= 3 and w_uses == 0:
            short_table_cands.append((sym, addr, dict(p)))
        elif b_uses >= 3 and w_uses == 0:
            byte_table_cands.append((sym, addr, dict(p)))

    def render_section(title: str, items: list, propose_prefix: str) -> list[str]:
        x = [f"## {title} ({len(items)})", ""]
        if not items:
            x.append("(none)")
            x.append("")
            return x
        x.append("| current | address | proposed | usage |")
        x.append("|---|---|---|---|")
        for sym, addr, usage in sorted(items, key=lambda r: r[1])[:50]:
            x.append(f"| `{sym}` | `0x{addr:08X}` | `{propose_prefix}_{sym[2:]}` | {usage} |")
        if len(items) > 50:
            x.append(f"| ... {len(items) - 50} more | | | |")
        x.append("")
        return x

    out.append(f"Total D_* symbols analyzed: {len(undef)}; with usage patterns: "
               f"{sum(1 for s in undef if s in use_patterns)}")
    out.append("")
    out.extend(render_section("GTE matrix/vector candidates (direct lwc2/swc2 references)",
                               matrix_cands, "MATRIX"))
    out.append("> NOTE: GTE access usually goes through a pointer-loaded base "
               "address rather than a direct D_<addr> reference. So this section "
               "is sparse; most GTE matrices live behind indirect pointer chains "
               "and require body inspection to identify.\n")
    out.extend(render_section("Function pointer / callback candidates",
                               function_ptr_cands, "fp"))
    out.append("> NOTE: Same caveat as above -- function-pointer tables are "
               "typically loaded into a register first and then `jalr`'d, so "
               "the direct-reference pattern misses most of them.\n")
    out.extend(render_section("Word-table candidates (lw/sw only access)",
                               word_table_cands, "wtbl"))
    out.extend(render_section("Short-table candidates (lh/sh only access)",
                               short_table_cands, "stbl"))
    out.extend(render_section("Byte-table candidates (lb/sb only access)",
                               byte_table_cands, "btbl"))

    out.append("---")
    out.append("")
    out.append("These are NAMING HINTS based on access width. Width tells you "
               "the element type; the actual semantic name still needs caller "
               "study. For example, an `lw`-only table could be `int[]`, a "
               "function-pointer table, or a packed RGB array.")
    out.append("")
    out.append("Do NOT bulk-apply these names. The width classifier has many "
               "false positives (e.g., a struct field accessed with `lw` but "
               "actually a `char[4]`). Use them as suggestions for "
               "decomp-time naming, not as a rename batch.")
    return "\n".join(out) + "\n"


def render_readme() -> str:
    return (
        "# Naming Proposal Database\n"
        "\n"
        "Evidence-backed function-name proposals for the 1217 unnamed "
        "`func_XXXXXXXX` functions in Bushido Blade 2.\n"
        "\n"
        "## Files\n"
        "\n"
        "- `proposals.csv` -- canonical machine-readable proposal table\n"
        "  - one row per still-unnamed function\n"
        "  - schema: address, current_name, proposed_name, confidence, evidence_summary, evidence_detail_file\n"
        "- `proposals_high_confidence.md` -- review table for `high`-tier proposals\n"
        "- `proposals_medium_confidence.md` -- review table for `medium`-tier proposals\n"
        "- `psyq_library_matches.md` -- focused view on PsyQ stdlib / BIOS jumptable / syscall wrapper proposals\n"
        "- `subsystem_clusters.md` -- call-graph cluster analysis (which subsystem do unnamed funcs belong to)\n"
        "- `data_symbols_quick_wins.md` -- naming hints for `D_*` undefined symbols based on access pattern\n"
        "- `methodology.md` -- analyzer design + evidence kinds + scoring + known caveats\n"
        "- `evidence/<func_name>.md` -- per-function evidence detail file\n"
        "\n"
        "## How to use\n"
        "\n"
        "Open `proposals_high_confidence.md` for the lowest-risk batch. "
        "For each row you want to apply:\n"
        "\n"
        "1. Read `evidence/<func>.md` to verify the evidence stands up.\n"
        "2. Spot-check the disassembly snippet matches the proposed semantics.\n"
        "3. Add `<proposed_name> = 0x<addr>;` to `named_syms.txt`.\n"
        "4. Run `make setup && make` (in WSL); verify SHA1 unchanged.\n"
        "5. Commit one batch (e.g., all BIOS wrappers, all PsyQ memcpy/memset) at a time.\n"
        "\n"
        "The existing `tools/apply_kengo_names.py` knows how to apply Kengo-named "
        "proposals in lockstep (named_syms.txt + src/*.c). For BIOS / PsyQ / "
        "syscall proposals the asm rename happens via `make setup` after editing "
        "`named_syms.txt`. Do NOT touch `symbol_addrs.txt` or `undefined_syms_auto.txt` "
        "manually unless you know the splat tooling.\n"
        "\n"
        "## Regenerating\n"
        "\n"
        "```\n"
        "python3 tools/propose_function_names.py            # rebuild proposals.csv + evidence/\n"
        "python3 tools/render_naming_docs.py                # rebuild markdown views\n"
        "python3 tools/diff_naming_proposals.py OLD.csv NEW.csv  # show diff vs prior run\n"
        "```\n"
        "\n"
        "Inputs the analyzer reads:\n"
        "\n"
        "- `asm/funcs/*.s` -- raw function disassembly\n"
        "- `named_syms.txt`, `symbol_addrs.txt` -- already-named functions and globals\n"
        "- `kengo_matches.csv` -- Kengo PS2 cross-reference (from `tools/kengo_match.py`)\n"
        "- `kengo_name_decisions.csv` -- reviewed decisions to suppress demoted names\n"
        "- `known_psyq_stdlib.txt` -- existing PsyQ idiom scan\n"
        "- `undefined_syms_auto.txt` -- splat-auto-detected data symbols\n"
        "- `asm/data/*.rodata*.s` -- string constants for `string_adjacent` evidence\n"
        "\n"
        "## Important caveats (from NAMING_TRIAGE_2026-05-12.md)\n"
        "\n"
        "- **Size-only Kengo matches were the #1 source of false-positive renames** in "
        "  the prior pass (`katinuki_game_get_katinuki_max_num_*`, 5 functions). The "
        "  analyzer downgrades these to `low` and address-suffixes them.\n"
        "- **Address-suffixed names indicate uncertainty.** A proposed `foo_8001ABCD` "
        "  is the analyzer saying \"this is the best candidate but I can't promise "
        "  it's a true match.\" When applying, prefer dropping the suffix only AFTER "
        "  body inspection confirms semantics.\n"
        "- **Per-function evidence files** are the single source of truth for why "
        "  the proposal exists. Read them.\n"
    )


def render_methodology() -> str:
    return (
        "# Methodology\n"
        "\n"
        "How `tools/propose_function_names.py` works.\n"
        "\n"
        "## Inputs\n"
        "\n"
        "- `asm/funcs/*.s` -- per-function disassembly (one file each)\n"
        "- `named_syms.txt`, `symbol_addrs.txt` -- existing name -> addr table; "
        "anchors for call-graph and address-neighborhood evidence\n"
        "- `kengo_matches.csv` -- output of `tools/kengo_match.py` (Kengo cross-reference)\n"
        "- `kengo_name_decisions.csv` -- reviewed decisions (rename / demote / keep)\n"
        "- `known_psyq_stdlib.txt` -- auto-detected PsyQ stdlib functions\n"
        "- `undefined_syms_auto.txt` -- splat data symbols\n"
        "- `asm/data/*.rodata*.s` -- string constants\n"
        "\n"
        "## Evidence kinds\n"
        "\n"
        "For each unnamed function the analyzer accumulates evidence rows; "
        "each row has a `kind`, a proposed `name`, and a `rank` "
        "(`high`/`medium`/`low`/`info`):\n"
        "\n"
        "| kind | rank | what it detects |\n"
        "|---|---|---|\n"
        "| `bios_jumptable` | high | 3-4 insn body matching `addiu $tN,$zero,0xA0|0xB0|0xC0; jr $tN; addiu $t1,$zero,<idx>`. The `<idx>` is looked up in standard PsyQ/nocash A0/B0/C0 tables to propose the canonical BIOS name (e.g., `bios_FileRead`). |\n"
        "| `syscall_wrapper` | high | Body contains a raw `syscall` or `break`. Address-suffixed because multiple wrappers can exist. |\n"
        "| `psyq_idiom` | high | Tight `lbu`-`sb` pair-loop with stride+1, no `jal`, no jumptable -> `psyq_memcpy`. Store-only loop with no load and stride+1 -> `psyq_memset`. Both address-suffixed. |\n"
        "| `data_as_code` | high | Function body is >=70% `lb $t0, neg_offset($zero)` with no actual logic -- a data table that splat misclassified as code. |\n"
        "| `kengo_unique` | high | Kengo `name-unique` match with `|diff| <= 1` insns. Demoted to `medium` if multi-claimant. |\n"
        "| `kengo_unique` | medium | Kengo `name-unique` / `name-callgraph` match with larger but still-small size diff. |\n"
        "| `kengo_pattern` | medium | Kengo `callgraph` / `caller-unique` / `affinity-unique` match, or `seq-similarity` with `opseq_ratio >= 0.5`. |\n"
        "| `sole_caller_path` | medium | Function has exactly one caller and that caller is trustworthy-named (not address-suffixed, not in known-bad cluster). Proposes `<caller>_helper_<addr>`. |\n"
        "| `call_graph_cluster` | medium | Function has >=2 named callers and one subsystem prefix dominates (e.g., `cpu_*`). Proposes `<subsystem>_helper_<addr>`. |\n"
        "| `kengo_pattern` | low | `seq-similarity` with `opseq_ratio` in 0.30-0.50 range. Always address-suffixed. |\n"
        "| `kengo_size_only` | low | Kengo `size-only-unique` -- match was driven only by instruction count. Always address-suffixed. |\n"
        "| `gte_op` | low | Body contains GTE atomic ops (`rtps`, `nclip`, `mvmva`, etc.). Proposes `gte_<top_op>_wrapper_<addr>`. |\n"
        "| `string_adjacent` | low | Function loads a known string with a recognizable substring (e.g., `CHANBARA` -> `chanbara_*`). |\n"
        "| `address_neighborhood` | low | >=2 functions at +-0x200 bytes share a trustworthy subsystem prefix. |\n"
        "| `string_adjacent_info` | info | Function loads strings but no clear naming slug derives from them. Informational only -- doesn't count toward confidence. |\n"
        "\n"
        "## Rolled-up confidence\n"
        "\n"
        "- `high` if any one `high`-rank evidence fires OR >=2 `medium`-rank evidence rows agree\n"
        "- `medium` if 1 `medium`-rank evidence row OR >=2 `low`-rank evidence rows\n"
        "- `low` if 1 `low`-rank evidence row\n"
        "- `none` if no evidence found\n"
        "\n"
        "## Best-proposed-name selection\n"
        "\n"
        "When multiple evidence rows propose names, the highest-rank evidence "
        "wins. Ties resolve by source-order in `analyze_one()`. The "
        "`evidence_detail_file` markdown lists ALL evidence rows so a reviewer "
        "can override the analyzer's pick.\n"
        "\n"
        "## Known caveats (from prior triage)\n"
        "\n"
        "Per `NAMING_TRIAGE_2026-05-12.md`:\n"
        "\n"
        "- **Size-only Kengo false positives**: a Kengo `size-only-*` match is "
        "  weak. Five `katinuki_game_get_katinuki_max_num_*` renames were "
        "  applied in a prior pass and four of them turned out to be unrelated "
        "  GPU wrappers; one was `gpu_EnableDisplay` and one was "
        "  `gpu_DisableDisplay`. The analyzer **skips** this Kengo name entirely "
        "  and downgrades `size-only` to `low` with mandatory address-suffix.\n"
        "- **Address-suffix indicates uncertainty**: a Kengo proposal with "
        "  `_<address>` suffix means even the reference is uncertain. The "
        "  analyzer reflects this by using suffixes for `seq-similarity` / "
        "  `size-only` / multi-claimant rows.\n"
        "- **Trustworthy anchors**: the cluster-driving heuristics "
        "  (`call_graph_cluster`, `address_neighborhood`, `sole_caller_path`) "
        "  refuse to use address-suffixed names as anchors. Otherwise a stale "
        "  `katinuki_game_get_katinuki_max_num_80016868` could propagate naming "
        "  errors. Anchors also exclude the demoted clusters listed in "
        "  `KNOWN_BAD_ANCHOR_PREFIXES`.\n"
        "- **Permanent out-of-scope categories** (`bios_or_syscall`, "
        "  `not_code_symbol`, `data_as_code`) are still in scope for NAMING "
        "  even though they're out of scope for decompilation. A BIOS wrapper "
        "  is a perfectly nameable function -- it just shouldn't be "
        "  attempted as pure C.\n"
        "\n"
        "## Things the analyzer doesn't do\n"
        "\n"
        "- It does not modify `named_syms.txt`, `symbol_addrs.txt`, or any "
        "  source files. Output is proposals only.\n"
        "- It does not infer parameter types or return type. Those come from "
        "  caller-audit work, not naming.\n"
        "- It does not propose names for `D_*` data symbols (see "
        "  `data_symbols_quick_wins.md` for a separate analysis).\n"
        "- It does not run the build. SHA1 verification is the human's job.\n"
    )


def main() -> int:
    rows = load_proposals()
    if not rows:
        return 1
    buckets = by_conf(rows)
    DOCS.mkdir(parents=True, exist_ok=True)

    high_md = DOCS / "proposals_high_confidence.md"
    med_md = DOCS / "proposals_medium_confidence.md"
    sub_md = DOCS / "subsystem_clusters.md"
    psyq_md = DOCS / "psyq_library_matches.md"
    data_md = DOCS / "data_symbols_quick_wins.md"
    readme_md = DOCS / "README.md"
    method_md = DOCS / "methodology.md"

    high_md.write_text(render_high(buckets["high"]), encoding="utf-8", newline="\n")
    med_md.write_text(render_medium(buckets["medium"]), encoding="utf-8", newline="\n")
    sub_md.write_text(render_subsystem_clusters(rows), encoding="utf-8", newline="\n")
    psyq_md.write_text(render_psyq_library_matches(rows), encoding="utf-8", newline="\n")
    data_md.write_text(scan_data_symbols(), encoding="utf-8", newline="\n")
    readme_md.write_text(render_readme(), encoding="utf-8", newline="\n")
    method_md.write_text(render_methodology(), encoding="utf-8", newline="\n")

    for f in (high_md, med_md, sub_md, psyq_md, data_md, readme_md, method_md):
        print(f"wrote {f.relative_to(ROOT)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
