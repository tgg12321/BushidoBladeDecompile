#!/usr/bin/env python3
"""Append regfix rules for single_game_SetWazaData Phase 1 (LICM unhoist)."""
with open("regfix.txt", "rb") as f:
    content = f.read().decode()

assert "single_game_SetWazaData" not in content, "already has rules"

new_rules = """
# single_game_SetWazaData: LICM unhoist of /1800 magic constant (0x91a2b3c5).
# Target materializes lui+ori inline each iteration (ori fills lw delay slot).
# Mine hoists it to $10 in preheader and inserts nop delay filler.
# Recipe: delete preheader lui+ori, delete nop delay filler, insert lui $2 before lw,
# insert_after ori $2 as delay slot. Cascade rename $11->$10 for mfhi/addu uses.
single_game_SetWazaData: $10 <-> $11 @ 47-92
single_game_SetWazaData: subst "\\$10" "$2" @ 46
single_game_SetWazaData: delete @ 39
single_game_SetWazaData: delete @ 40
single_game_SetWazaData: delete @ 45
single_game_SetWazaData: insert "lui	$2,(0x91a2b3c5 >> 16) & 0xFFFF" @ 42
single_game_SetWazaData: insert_after "ori	$2,$2,0xb3c5" @ 43
"""

content = content.rstrip() + "\n" + new_rules

with open("regfix.txt", "wb") as f:
    f.write(content.encode())
print("OK")
