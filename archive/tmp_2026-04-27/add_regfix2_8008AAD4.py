#!/usr/bin/env python3
"""Append second regfix batch for func_8008AAD4 — remove cached la for D_800A28A0/289C."""

TAB = "\t"
lines = [
    "",
    "# Second batch: volatile casts on D_800A28A0/289C introduce cached la $3,...",
    "# Target uses fresh lui+lw and lui+sw per access. Delete the la pseudos and",
    "# expand 0($3) lw/sw uses to symbolic form. Binary count preserved (la=2 -> 0,",
    "# lw=1 -> lw symbolic=2, sw=1 -> sw symbolic=2; net per pair: 0).",
    "func_8008AAD4: delete @ 16",
    'func_8008AAD4: subst "lw' + TAB + '\\$2,0\\(\\$3\\)" "lw' + TAB + '$2,D_800A28A0" @ 19',
    'func_8008AAD4: subst "sw' + TAB + '\\$2,0\\(\\$3\\)" "sw' + TAB + '$2,D_800A28A0" @ 22',
    "func_8008AAD4: delete @ 23",
    'func_8008AAD4: subst "lw' + TAB + '\\$2,0\\(\\$3\\)" "lw' + TAB + '$2,D_800A289C" @ 24',
    'func_8008AAD4: subst "sw' + TAB + '\\$2,0\\(\\$3\\)" "sw' + TAB + '$2,D_800A289C" @ 27',
]

with open("regfix.txt", "a") as f:
    f.write("\n".join(lines) + "\n")

print("appended", len(lines), "lines")
