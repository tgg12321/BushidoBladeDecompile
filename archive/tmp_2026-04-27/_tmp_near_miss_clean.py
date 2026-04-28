#!/usr/bin/env python3
"""
Find ±5% affinity single-match candidates that are NOT contradicted
by an exact match for the same Kengo function name.

A "contradiction" = the same Kengo function appears as an exact-size
match for a different BB2 stub.  These suggest the exact match is
the real assignment and the near-miss is coincidence.
"""
import subprocess, re, sys

def run_match(extra_args):
    result = subprocess.run(
        ["python3", "tools/kengo_match.py", "--use-affinity", "--no-header"] + extra_args,
        capture_output=True, text=True,
        cwd="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
    )
    return result.stdout

# --- build exact match set (kengo_name -> set of BB2 stubs) ---
exact_text = run_match(["--exact", "--top", "10"])
exact_map = {}   # kengo_name -> set of bb2 stub names
cur_bb2 = None
for line in exact_text.splitlines():
    m = re.match(r"(\w+)  \[(\S+)\]  (\d+) insns", line)
    if m:
        cur_bb2 = m.group(1)
        continue
    m = re.match(r"  → (\S+)\s+\d+ insns.*\[(\w+)\]", line)
    if m and cur_bb2:
        exact_map.setdefault(m.group(1), set()).add(cur_bb2)

# --- get ±5% single-match affinity candidates ---
near_text = run_match(["--tolerance", "0.05", "--single-match"])
cur_bb2, cur_file, cur_insns, has_aff = None, None, None, False
results = []
for line in near_text.splitlines():
    m = re.match(r"(\w+)  \[(\S+)\]  (\d+) insns(\s*\[affinity\])?", line)
    if m:
        cur_bb2 = m.group(1)
        cur_file = m.group(2)
        cur_insns = int(m.group(3))
        has_aff = bool(m.group(4))
        continue
    m = re.match(r"  → (\S+)\s+(\d+) insns \(\s*([+-]?\d+)\)  \[(\w+)\]", line)
    if m and cur_bb2:
        kengo_name = m.group(1)
        kengo_insns = int(m.group(2))
        diff = int(m.group(3))
        mod = m.group(4)

        # Check if this Kengo name has an exact match for a DIFFERENT stub
        contradicted = False
        if kengo_name in exact_map:
            exact_stubs = exact_map[kengo_name]
            if exact_stubs - {cur_bb2}:  # other stubs have exact match
                contradicted = True

        results.append({
            "bb2": cur_bb2, "file": cur_file, "bb2_insns": cur_insns,
            "kengo": kengo_name, "kengo_insns": kengo_insns, "diff": diff,
            "mod": mod, "aff": has_aff, "contradicted": contradicted,
        })

print("=== CLEAN near-miss candidates (not contradicted by exact match elsewhere) ===")
clean = [r for r in results if not r["contradicted"] and r["aff"]]
for r in sorted(clean, key=lambda x: abs(x["diff"])):
    tag = f"({r['diff']:+d})"
    pct = abs(r["diff"]) / r["kengo_insns"] * 100
    print(f"  {r['bb2']:<32} {r['bb2_insns']:4d}i → {r['kengo']:<48} {r['kengo_insns']:4d}i {tag:>5}  {pct:.1f}%  [{r['mod']}]  [{r['file']}]")

print(f"\n=== CONTRADICTED (exact match exists for same Kengo name, different stub) ===")
contr = [r for r in results if r["contradicted"] and r["aff"]]
for r in sorted(contr, key=lambda x: abs(x["diff"])):
    tag = f"({r['diff']:+d})"
    other = exact_map.get(r["kengo"], set()) - {r["bb2"]}
    print(f"  {r['bb2']:<32} → {r['kengo']:<44} {tag}  [exact: {', '.join(sorted(other))}]")

print(f"\nSummary: {len(clean)} clean candidates, {len(contr)} contradicted")
