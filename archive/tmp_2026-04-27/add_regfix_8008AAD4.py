#!/usr/bin/env python3
"""Append regfix block for func_8008AAD4."""

# Each line uses literal tab character between mnemonic and operands.
# Pattern strings need regex escapes: \$ for literal $, \( \) for literal parens.
# Replacement strings are literal (regfix passes them through lambda to avoid backref).

TAB = "\t"
lines = [
    "",
    "# func_8008AAD4: GCC 2.7.2 caches D_800F7424 base in v1 ($3) for 3 accesses;",
    "# target emits 3 fresh lui/lhu/sh pseudos. Plus the conditional store nor uses",
    "# v1 ($3) in target but mine clobbers a1 ($5). Same nor issue in [1] section.",
    "# Delete cached la, expand 3 uses to symbolic form, fix nor in both [0] and [1].",
    "func_8008AAD4: delete @ 28",
    'func_8008AAD4: subst "lhu' + TAB + '\\$2,0\\(\\$3\\)" "lhu' + TAB + '$2,D_800F7424" @ 29',
    'func_8008AAD4: subst "lhu' + TAB + '\\$2,0\\(\\$3\\)" "lhu' + TAB + '$2,D_800F7424" @ 34',
    'func_8008AAD4: subst "nor' + TAB + '\\$5,\\$0,\\$5" "nor' + TAB + '$3,$0,$5" @ 35',
    'func_8008AAD4: subst "and' + TAB + '\\$2,\\$2,\\$5" "and' + TAB + '$2,$2,$3" @ 36',
    'func_8008AAD4: subst "sh' + TAB + '\\$2,0\\(\\$3\\)" "sh' + TAB + '$2,D_800F7424" @ 37',
    'func_8008AAD4: subst "nor' + TAB + '\\$8,\\$0,\\$8" "nor' + TAB + '$3,$0,$8" @ 44',
    'func_8008AAD4: subst "and' + TAB + '\\$2,\\$2,\\$8" "and' + TAB + '$2,$2,$3" @ 45',
]

with open("regfix.txt", "a") as f:
    f.write("\n".join(lines) + "\n")

print("appended", len(lines), "lines")
