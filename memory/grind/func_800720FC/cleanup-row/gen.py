"""Generate grid-loop respellings of func_800720FC (drop the `row` named intermediate)."""
base = open("tmp/cleanup/720fc/base.c", encoding="utf-8").read()

OLD_HEAD = """        s32 row;

        for (j = 0, row = i * 2; j < 2; j++) {"""
OLD_COND = "if (row + j != 3 || D_800A35BC != 6 || mode != 2) {"
OLD_LOAD = "s.header = ((s32 *)arg1 + i * 2)[j + 3];"
assert OLD_HEAD in base and OLD_COND in base and OLD_LOAD in base

def v(head, cond, load):
    b = base.replace(OLD_HEAD, head).replace(OLD_COND, cond).replace(OLD_LOAD, load)
    return b

PLAIN = "        for (j = 0; j < 2; j++) {"
variants = {
    "base": base,
    # inline i * 2 everywhere
    "inl_i2": v(PLAIN, "if (i * 2 + j != 3 || D_800A35BC != 6 || mode != 2) {", OLD_LOAD),
    "inl_j_i2": v(PLAIN, "if (j + i * 2 != 3 || D_800A35BC != 6 || mode != 2) {", OLD_LOAD),
    # i + i (ledger :59)
    "ipi_cond": v(PLAIN, "if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {", OLD_LOAD),
    "ipi_both": v(PLAIN, "if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {",
                  "s.header = ((s32 *)arg1 + i + i)[j + 3];"),
    "ipi_both2": v(PLAIN, "if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {",
                   "s.header = ((s32 *)arg1 + (i + i))[j + 3];"),
    # the struct view of the header grid
    "hdrC_i2": v(PLAIN, "if (i * 2 + j != 3 || D_800A35BC != 6 || mode != 2) {",
                 "s.header = ((Sheets720FC *)arg1)->hdrC[i][j];"),
    "hdrC_ipi": v(PLAIN, "if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {",
                  "s.header = ((Sheets720FC *)arg1)->hdrC[i][j];"),
    "hdrC_row": v(OLD_HEAD, OLD_COND, "s.header = ((Sheets720FC *)arg1)->hdrC[i][j];"),
    # row kept but not in the for-init
    "row_before": v("""        s32 row = i * 2;

        for (j = 0; j < 2; j++) {""", OLD_COND, OLD_LOAD),
    # i * 2 inline, cell address via row pointer
    "inl_i2_ptr": v(PLAIN, "if (i * 2 + j != 3 || D_800A35BC != 6 || mode != 2) {",
                    "s.header = ((s32 *)arg1)[i * 2 + j + 3];"),
    # j == 1 && i == 1 form (cell 3 of the 4x2 grid)
    "ij11": v(PLAIN, "if (i != 1 || j != 1 || D_800A35BC != 6 || mode != 2) {", OLD_LOAD),
}
import os
os.makedirs("tmp/cleanup/720fc/v", exist_ok=True)
for k, t in variants.items():
    open(f"tmp/cleanup/720fc/v/{k}.c", "w", newline="\n").write(t)
print(",".join(f"tmp/cleanup/720fc/v/{k}.c" for k in variants))
