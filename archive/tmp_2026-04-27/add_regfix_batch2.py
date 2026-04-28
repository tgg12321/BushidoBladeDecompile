#!/usr/bin/env python3
"""Append batch 2 regfix rules: loop head, computation block, branch targets."""

rules = [
    # Loop head: reorder via subst + insert inner label
    # idx 27: la $3,D_800F7438 -> addiu a1 + .Linner: + lw v0 (2 instrs + label from 1 pseudo-op)
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "la\t\$3,D_800F7438" "addiu\t$5,$zero,4104\n.Linner:\n\tlw\t$2,%gp_rel(D_800A36AC)($gp)" @ 27',
    # idx 28: lw -> lui (part 1 of la expansion)
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "lw\t\$2,%gp_rel\(D_800A36AC\)\(\$gp\)" "lui\t$3,%hi(D_800F7438)" @ 28',
    # idx 29: addiu a1 -> addiu v1 (part 2 of la expansion)
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "addiu\t\$5,\$zero,4104" "addiu\t$3,$3,%lo(D_800F7438)" @ 29',

    # Computation block: register rename lui $2 -> lui $4
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "lui\t\$2" "lui\t$4" @ 104',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "lui\t\$2" "lui\t$4" @ 109',

    # Computation block: reorder via subst at .L129 (110-114)
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "lw\t\$3,%gp_rel\(D_800A38B4\)\(\$gp\)" "ori\t$4,$4,0xcc00" @ 110',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "ori\t\$2,\$2,0xcc00" "sll\t$2,$17,2" @ 111',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "addu\t\$3,\$3,\$2" "lw\t$3,%gp_rel(D_800A38B4)($gp)" @ 112',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "sll\t\$2,\$17,2" "addu\t$2,$2,$20" @ 113',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "addu\t\$2,\$2,\$20" "addu\t$3,$3,$4" @ 114',

    # Branch targets: .L105 -> .Linner for inner-label branches
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "\.L105" ".Linner" @ 146',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "\.L105" ".Linner" @ 148',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "\.L105" ".Linner" @ 161',
    r'cpu_set_move_command_and_dir_for_no_action_2: subst "\.L105" ".Linner" @ 169',
]

path = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/regfix.txt'
with open(path, 'a') as f:
    for r in rules:
        f.write(r + '\n')
print(f"Appended {len(rules)} rules")
