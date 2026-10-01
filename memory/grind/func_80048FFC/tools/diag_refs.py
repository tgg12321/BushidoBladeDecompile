# DIAGNOSTIC ONLY (never a candidate): an empty asm use adds one reference to nx/ny without code.
C1 = "        SetDrawMove(p, &rect, nx, ny + phase);\n"
VARIANTS = [
    ("diag_use_both", [(C1, '        __asm__ volatile ("" : : "r"(nx), "r"(ny));\n' + C1)]),
    ("diag_use_after", [(C1, C1 + '        __asm__ volatile ("" : : "r"(nx), "r"(ny));\n')]),
]
