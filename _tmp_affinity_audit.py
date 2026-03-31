"""Summarize the affinity-filtered results, excluding known-reverted PS2 UI stubs."""
import subprocess, re

REVERTED = {
    "func_8001B748", "func_80020E74", "func_800290B8",
    "func_80083E9C", "func_8002EBDC", "func_8003D52C",
    "func_800344B4", "func_800889D4", "func_8003C714",
}

result = subprocess.run(
    ["python3", "tools/kengo_match.py", "--exact", "--use-affinity", "--single-match", "--no-header"],
    capture_output=True, text=True,
    cwd="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
)

# Parse output into blocks
blocks = re.split(r'\n(?=func_|[a-z])', result.stdout.strip())

high, med, collision, reverted_ps2, no_affinity = [], [], [], [], []

current_func = None
current_name = None
current_module = None
current_file = None
current_insns = None
has_affinity = False

for line in result.stdout.splitlines():
    m = re.match(r'(func_\w+|\w+)\s+\[(\S+)\]\s+(\d+) insns(.*)', line)
    if m:
        current_func = m.group(1)
        current_file = m.group(2)
        current_insns = int(m.group(3))
        has_affinity = "[affinity]" in m.group(4)
        continue
    m = re.match(r'\s+→\s+(\S+)\s+\d+ insns.*\[(\w+)\]', line)
    if m and current_func:
        current_name = m.group(1)
        current_module = m.group(2)

        if current_func in REVERTED:
            reverted_ps2.append((current_func, current_name, current_module, current_insns, current_file))
        elif not has_affinity:
            no_affinity.append((current_func, current_name, current_module, current_insns, current_file))
        else:
            # Classify by module
            core = {"nm_camera","nm_single_game","md_game","is_coli","nm_cpu",
                    "is_pad","nm_replay_cam","is_damage_calc","is_action","nm_special_cam"}
            med_mods = {"sa_tan0","sa_tan1","sa_tan2","sa_tan3","sa_tan4","sa_tan5",
                        "sa_eft","am_rmd","my_rob","my_eff","my_hirahira","se_fc","tsl_pkt"}
            if current_module in core:
                high.append((current_func, current_name, current_module, current_insns, current_file))
            else:
                med.append((current_func, current_name, current_module, current_insns, current_file))

print("=== HIGH confidence — core engine, affinity match ===")
for func, name, mod, insns, f in high:
    print(f"  {func:<32} → {name:<52} [{mod}] {insns}i  [{f}]")

print(f"\n=== MED confidence — affinity match ===")
for func, name, mod, insns, f in med:
    print(f"  {func:<32} → {name:<52} [{mod}] {insns}i  [{f}]")

print(f"\n=== Ignored — reverted PS2 UI (no affinity, known bad) ===")
for func, name, mod, insns, f in reverted_ps2:
    print(f"  {func}  [{f}]")

print(f"\nSummary: {len(high)} HIGH + {len(med)} MED new candidates")
print(f"         {len(reverted_ps2)} reverted PS2 UI (ignored)")
