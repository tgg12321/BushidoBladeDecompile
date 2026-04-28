#!/usr/bin/env python3
"""Append third regfix batch for func_8008AAD4.

Three regions to fix:
  A. case1 inactive block reorder (idx 49-55) — substs only
  B. case0 active block prefix rewrite (idx 61-72) — 12 deletes + 1 multi-line insert
  C. case0 inner-cond + inactive nor reg fixes (idx 86-87, 96-97) — substs

The case0 active block keeps mine's idx 73-79 (just changes register references).
"""

TAB = "\t"
NL = "\\n"  # literal \n in the regfix.txt file (parser converts to newline)
TT = "\\t"

# Build the multi-line insert text. Each instruction separated by \\n\\t to
# get \n + tab in the parsed asm_text. (regfix wraps the whole thing in
# leading tab + trailing newline already.)
target_lines = [
    "sh" + TT + "$6,D_800F7424",
    "sh" + TT + "$7,D_800F7424+2",
    "lw" + TT + "$2,D_800A28A0",
    "nor" + TT + "$4,$0,$5",
    "ori" + TT + "$2,$2,0x1",
    "sw" + TT + "$2,D_800A28A0",
    "lw" + TT + "$2,D_800A289C",
    "la" + TT + "$3,D_800F7420",
    "and" + TT + "$2,$2,$4",
    "sw" + TT + "$2,D_800A289C",
    "lhu" + TT + "$2,0($3)",
    "nop",
]
multi_line = NL.join(target_lines)

lines = [
    "",
    "# func_8008AAD4 batch 3:",
    "# A. case1 inactive (idx 49-55): reorder via subst — load D_800A2874 first,",
    "#    cache D_800A2CDC in $v1, fill j-delay with nop instead of or.",
    'func_8008AAD4: subst "lw' + TAB + '\\$2,D_800A2CDC" "lw' + TAB + '$2,D_800A2874" @ 49',
    'func_8008AAD4: subst "nop" "lw' + TAB + '$3,D_800A2CDC" @ 50',
    'func_8008AAD4: subst "sh' + TAB + '\\$6,392\\(\\$2\\)" "or' + TAB + '$2,$2,$5" @ 51',
    'func_8008AAD4: subst "sh' + TAB + '\\$7,394\\(\\$2\\)" "sh' + TAB + '$6,0x188($3)" @ 52',
    'func_8008AAD4: subst "lw' + TAB + '\\$2,D_800A2874" "sh' + TAB + '$7,0x18A($3)" @ 53',
    'func_8008AAD4: subst "or' + TAB + '\\$2,\\$2,\\$5" "nop" @ 55',
    "",
    "# B. case0 active prefix (idx 61-72): full rewrite via 12 deletes + multi-line insert.",
    "#    Mine cached la $4,D_800F7420 + scattered lui+sh; target uses fresh sh + cached la $3.",
    "#    Also flip nor reg from $a2 to $a0 (inside the insert).",
    "func_8008AAD4: delete @ 61",
    "func_8008AAD4: delete @ 62",
    "func_8008AAD4: delete @ 63",
    "func_8008AAD4: delete @ 64",
    "func_8008AAD4: delete @ 65",
    "func_8008AAD4: delete @ 66",
    "func_8008AAD4: delete @ 67",
    "func_8008AAD4: delete @ 68",
    "func_8008AAD4: delete @ 69",
    "func_8008AAD4: delete @ 70",
    "func_8008AAD4: delete @ 71",
    "func_8008AAD4: delete @ 72",
    'func_8008AAD4: insert "' + multi_line + '" @ 61',
    "",
    "# B-cont. After insert, original idx 73-79 remain. Fix register references:",
    "#   $3 -> $2 (load went into $2 not $3), $4 -> $3 (cache in $v1 not $a0), $6 -> $4 (notmask in $a0).",
    'func_8008AAD4: subst "and' + TAB + '\\$3,\\$3,\\$5" "and' + TAB + '$2,$2,$5" @ 73',
    'func_8008AAD4: subst "beq' + TAB + '\\$3,\\$0," "beq' + TAB + '$2,$0," @ 74',
    'func_8008AAD4: subst "lhu' + TAB + '\\$2,0\\(\\$4\\)" "lhu' + TAB + '$2,0($3)" @ 76',
    'func_8008AAD4: subst "and' + TAB + '\\$2,\\$2,\\$6" "and' + TAB + '$2,$2,$4" @ 78',
    'func_8008AAD4: subst "sh' + TAB + '\\$2,0\\(\\$4\\)" "sh' + TAB + '$2,0($3)" @ 79',
    "",
    "# C. case0 active inner cond nor (idx 86-87) — same shape as @ 44-45 but for case0.",
    'func_8008AAD4: subst "nor' + TAB + '\\$8,\\$0,\\$8" "nor' + TAB + '$3,$0,$8" @ 86',
    'func_8008AAD4: subst "and' + TAB + '\\$2,\\$2,\\$8" "and' + TAB + '$2,$2,$3" @ 87',
    "",
    "# C-cont. case0 inactive nor (idx 96-97).",
    'func_8008AAD4: subst "nor' + TAB + '\\$5,\\$0,\\$5" "nor' + TAB + '$3,$0,$5" @ 96',
    'func_8008AAD4: subst "and' + TAB + '\\$2,\\$2,\\$5" "and' + TAB + '$2,$2,$3" @ 97',
]

with open("regfix.txt", "a") as f:
    f.write("\n".join(lines) + "\n")

print("appended", len(lines), "lines")
