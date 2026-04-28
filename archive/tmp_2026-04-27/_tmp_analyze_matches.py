import csv
from collections import defaultdict, Counter

with open("kengo_matches.csv") as f:
    rows = list(csv.DictReader(f))

# Per-BB2-file stats
by_file = defaultdict(list)
for r in rows:
    by_file[r["bb2_file"]].append(r)

print("=== Per-file match stats ===")
for fname in sorted(by_file):
    file_rows = by_file[fname]
    stubs = set(r["bb2_func"] for r in file_rows)
    exact_stubs = set(r["bb2_func"] for r in file_rows if r["diff"] == "0")
    no_match_stubs = set(r["bb2_func"] for r in file_rows if not r["kengo_name"])
    print(f"  {fname:<20} {len(stubs):3d} stubs | {len(exact_stubs):3d} with exact hit | {len(no_match_stubs)} no match")

# High-confidence: stubs with exactly ONE exact-size match (unique name)
print("\n=== High-confidence candidates (single exact-size Kengo match) ===")
exact_rows = [r for r in rows if r["diff"] == "0"]
by_stub = defaultdict(list)
for r in exact_rows:
    by_stub[r["bb2_func"]].append(r)

single_exact = [(stub, matches[0]) for stub, matches in by_stub.items() if len(matches) == 1]
# sort by bb2_file then stub
single_exact.sort(key=lambda x: (x[1]["bb2_file"], x[0]))

print(f"Total: {len(single_exact)} stubs")
print()
for stub, r in single_exact:
    print(f"  {stub}  →  {r['kengo_name']:<44}  [{r['kengo_module']}]  ({r['bb2_insns']} insns)  [{r['bb2_file']}]")
