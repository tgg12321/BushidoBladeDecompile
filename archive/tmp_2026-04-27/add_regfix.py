import sys
with open("regfix.txt", "r") as f:
    lines = f.readlines()
lines = [l for l in lines if "motion_SetMotion" not in l and "anti-cross-jump" not in l]
while lines and lines[-1].strip() == "":
    lines.pop()
rule = "motion_SetMotion: subst \"addiu\t$16,$zero,12\" \"addiu\t6,,13\" @ 149\n"
lines.append("\n")
lines.append("# motion_SetMotion: anti-cross-jump fix\n")
lines.append(rule)
with open("regfix.txt", "w", newline="\n") as f:
    f.writelines(lines)
print("Done")
