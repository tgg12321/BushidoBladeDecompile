#!/usr/bin/env python3
"""Score remaining INCLUDE_ASM stubs by estimated matchability.

Ranks functions by how likely they are to produce a byte-identical match
when decompiled to C with GCC 2.7.2. Higher score = more likely to match.

Scoring factors (from agent feedback):
- Functions with many jal calls and few branches match best
- Leaf functions (no jal) with simple arithmetic match well
- Functions with GTE/COP2 instructions need .word encoding (special)
- Functions with jr $t2 are BIOS trampolines (need __asm__)
- GP-rel functions are now decompilable (--sdata-funcs)

Usage: python3 tools/find_candidates.py [--file src/foo.c] [--top N] [--min-score N]
"""
import os, re, argparse

def analyze_stub(func_name):
    """Analyze an asm stub and return a score + metadata."""
    asm_path = f"asm/funcs/{func_name}.s"
    if not os.path.exists(asm_path):
        return None

    content = open(asm_path).read()
    lines = [l.strip() for l in content.split("\n") if "/*" in l]
    instr_count = len(lines)

    # Count key patterns
    jal_count = sum(1 for l in lines if "\tjal\t" in l or " jal " in l)
    branch_count = sum(1 for l in lines if any(b in l for b in
        ["beqz", "bnez", "beq ", "bne ", "bgez", "bltz", "bgtz", "blez"]))
    has_gp = "gp_rel" in content
    has_gte = any(w in content for w in ["mtc2", "mfc2", "ctc2", "cfc2", "lwc2", "swc2", "cop2"])
    has_jr_t2 = any("jr" in l and "$t2" in l for l in lines)
    has_break = "break" in content
    has_div = any("\tdiv\t" in l or " div " in l for l in lines)
    is_leaf = jal_count == 0
    has_lui = sum(1 for l in lines if "\tlui\t" in l or " lui " in l)

    # Classify
    if has_jr_t2 and instr_count <= 6:
        return {"name": func_name, "type": "BIOS_TRAMPOLINE", "score": 100,
                "instr": instr_count, "reason": "Mechanical __asm__ conversion"}

    if has_gte:
        return {"name": func_name, "type": "GTE", "score": 60,
                "instr": instr_count, "reason": f"GTE/COP2 — use register asm() + .word"}

    # Score calculation
    score = 50  # base

    # jal-heavy functions match best (constrained register allocation)
    if jal_count >= 5:
        score += 30
    elif jal_count >= 3:
        score += 20
    elif jal_count >= 1:
        score += 10

    # Few branches = simpler control flow
    if branch_count == 0:
        score += 15
    elif branch_count <= 2:
        score += 5
    elif branch_count >= 6:
        score -= 15

    # Size penalty (larger = harder)
    if instr_count <= 15:
        score += 10
    elif instr_count <= 30:
        score += 5
    elif instr_count >= 100:
        score -= 20
    elif instr_count >= 50:
        score -= 10

    # Leaf functions with simple patterns
    if is_leaf and instr_count <= 20 and branch_count <= 1:
        score += 10

    # Penalties
    if has_break:
        score -= 10  # break encoding mismatch
    if has_div:
        score -= 5   # division expansion sensitivity

    # GP-rel is fine now but note it
    gp_tag = " [GP]" if has_gp else ""

    reason_parts = []
    if jal_count > 0:
        reason_parts.append(f"{jal_count} calls")
    if branch_count > 0:
        reason_parts.append(f"{branch_count} branches")
    if is_leaf:
        reason_parts.append("leaf")
    reason = ", ".join(reason_parts) if reason_parts else "simple"

    return {"name": func_name, "type": "C", "score": score,
            "instr": instr_count, "jals": jal_count, "branches": branch_count,
            "gp": has_gp, "reason": reason + gp_tag}


def main():
    parser = argparse.ArgumentParser(description="Find best decompilation candidates")
    parser.add_argument("--file", type=str, help="Only analyze stubs in this file")
    parser.add_argument("--top", type=int, default=40, help="Show top N candidates")
    parser.add_argument("--min-score", type=int, default=0, help="Minimum score to show")
    parser.add_argument("--type", type=str, help="Filter by type: C, GTE, BIOS_TRAMPOLINE")
    args = parser.parse_args()

    # Collect all INCLUDE_ASM stubs
    candidates = []
    src_files = [args.file] if args.file else sorted(
        f"src/{f}" for f in os.listdir("src") if f.endswith(".c"))

    for src in src_files:
        if not os.path.exists(src):
            continue
        content = open(src).read()
        stubs = re.findall(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', content)
        for func in stubs:
            result = analyze_stub(func)
            if result:
                result["file"] = os.path.basename(src)
                candidates.append(result)

    # Filter
    if args.type:
        candidates = [c for c in candidates if c["type"] == args.type]
    if args.min_score:
        candidates = [c for c in candidates if c["score"] >= args.min_score]

    # Sort by score descending
    candidates.sort(key=lambda c: (-c["score"], c["instr"]))

    # Print
    print(f"{'Score':>5} {'Instr':>5} {'File':>18} {'Function':>20} {'Type':>16} Reason")
    print("-" * 95)
    for c in candidates[:args.top]:
        print(f"{c['score']:>5} {c['instr']:>5} {c['file']:>18} {c['name']:>20} {c['type']:>16} {c['reason']}")

    # Summary
    by_type = {}
    for c in candidates:
        by_type.setdefault(c["type"], 0)
        by_type[c["type"]] += 1
    print(f"\nTotal: {len(candidates)} stubs")
    for t, n in sorted(by_type.items()):
        print(f"  {t}: {n}")


if __name__ == "__main__":
    main()
