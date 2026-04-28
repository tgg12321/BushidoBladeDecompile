#!/usr/bin/env python3
"""Add regfix rules for func_80023648."""

with open("regfix.txt", "r") as f:
    lines = f.readlines()

# Remove trailing blank lines
while lines and lines[-1].strip() == "":
    lines.pop()

D = "\x24"  # dollar sign

rules = f"""
# func_80023648: register allocation fixes (36 diffs, score 180->0)
# Region 1: table value v1->a2 ({D}3->{D}6)
func_80023648: subst "\\{D}3,0\\(\\{D}3\\)" "{D}6,0({D}3)" @ 34
func_80023648: subst "\\{D}3," "{D}6," @ 41
func_80023648: subst ",\\{D}3,\\{D}zero" ",{D}6,{D}zero" @ 42
func_80023648: subst ",\\{D}3,3" ",{D}6,3" @ 43
func_80023648: subst ",\\{D}3" ",{D}6" @ 53
# Region 2: abs_val v1->a2 ({D}3->{D}6)
func_80023648: subst "\\{D}3,336" "{D}6,336" @ 63
func_80023648: subst "\\{D}3," "{D}6," @ 64
func_80023648: subst ",\\{D}3," ",{D}6," @ 65
func_80023648: subst "\\{D}3,\\{D}0,\\{D}3" "{D}6,{D}0,{D}6" @ 66
func_80023648: subst ",\\{D}3," ",{D}6," @ 67
func_80023648: subst "\\{D}3,\\{D}zero" "{D}6,{D}zero" @ 70
# Region 3: sub_result+abs in subu: a1,v0,v1 -> a0,v0,a2
func_80023648: subst "\\{D}5,\\{D}2,\\{D}3" "{D}4,{D}2,{D}6" @ 73
# Region 4: div16 a0->v1 ({D}4->{D}3)
func_80023648: subst "\\{D}4,26" "{D}3,26" @ 72
func_80023648: subst "\\{D}4," "{D}3," @ 74
func_80023648: subst "\\{D}5,334" "{D}4,334" @ 75
func_80023648: subst "\\{D}4,\\{D}4,15" "{D}3,{D}3,15" @ 76
func_80023648: subst "\\{D}4,\\{D}4,4" "{D}3,{D}3,4" @ 77
# Region 5: addu operand swap: v1,a1,a0 -> v1,a0,v1
func_80023648: subst "\\{D}3,\\{D}5,\\{D}4" "{D}3,{D}4,{D}3" @ 79
# Region 6: 0x1A reload a1->a0 ({D}5->{D}4)
func_80023648: subst "\\{D}5,26" "{D}4,26" @ 84
func_80023648: subst "\\{D}5,\\{D}2" "{D}4,{D}2" @ 89
# Region 7: mflo a2->a3 ({D}6->{D}7) and sll/sra sources
func_80023648: subst "\\{D}6" "{D}7" @ 92
func_80023648: subst "\\{D}6," "{D}7," @ 93
func_80023648: subst "\\{D}6" "{D}7" @ 109
# Region 8: speed+mflo: sra a1,a2,12 -> sra a2,a3,12
func_80023648: subst "\\{D}5,\\{D}6," "{D}6,{D}7," @ 113
# mult v0,a1 -> mult v0,a2
func_80023648: subst ",\\{D}5" ",{D}6" @ 114
# mflo a2->a3
func_80023648: subst "\\{D}6" "{D}7" @ 120
func_80023648: subst ",\\{D}6," ",{D}7," @ 121
# mult v0,a1 -> mult v0,a2
func_80023648: subst ",\\{D}5" ",{D}6" @ 127
# mflo a2->a3
func_80023648: subst "\\{D}6" "{D}7" @ 129
func_80023648: subst ",\\{D}6," ",{D}7," @ 130
"""

lines.append(rules)

with open("regfix.txt", "w", newline="\n") as f:
    f.writelines(lines)

print("Added regfix rules for func_80023648")
