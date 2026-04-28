filepath = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/regfix.txt'

with open(filepath, 'r') as f:
    content = f.read()

# Add regfix entries for func_8003B10C
entries = """
# func_8003B10C: prologue scheduling - swap lui s2 and sw ra
func_8003B10C: reorder 5,4 @ 4-5
# func_8003B10C: insert ED6 load in if-branch before j merge (TEXT 34)
func_8003B10C: insert "lui $1,%hi(D_80101ED6)" @ 34
func_8003B10C: insert "addu $1,$1,$16" @ 35
func_8003B10C: insert "lh $3,%lo(D_80101ED6)($1)" @ 36
# func_8003B10C: reorder else-branch - E5CC first, then ED2, then ED6, then sll
func_8003B10C: reorder 42,39,40,41,44,45,46,43 @ 39-46
# func_8003B10C: merge operand order - addu v0,a0,v0 -> addu v0,v0,a0
func_8003B10C: subst "\\$4,\\$2" "$2,$4" @ 47
"""

content = content.rstrip('\n') + '\n' + entries

with open(filepath, 'w', newline='\n') as f:
    f.write(content)

print("Done - added regfix entries for func_8003B10C")
