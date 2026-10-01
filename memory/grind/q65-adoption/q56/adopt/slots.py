"""slots.py: from /tmp/q56/defs_plan.json - per file, static-region (0x800A3308..0x800A3618) C names grouped by
4-byte slot; print every slot holding a name at a non-4-aligned address (A8: one .lcomm static per slot start)."""
import json
P = json.load(open("/tmp/q56/defs_plan.json"))
for f, uses in sorted(P.items()):
    slots = {}
    for nm, u in uses.items():
        a = u["addr"]
        if 0x800A3308 <= a < 0x800A3618:
            slots.setdefault(a & ~3, []).append((a, nm, u["gp"], u["ref"], (u["decls"] or u["defs"] or ["-"])[0], u.get("gp_offsets")))
    for s, L in sorted(slots.items()):
        if any(a % 4 for a, *_ in L):
            print(f"{f} slot {s:#x}:")
            for a, nm, gp, ref, d, offs in sorted(L):
                print(f"    {a:#x} {nm:14s} gp={gp} ref={ref} offs={offs} {d}")
