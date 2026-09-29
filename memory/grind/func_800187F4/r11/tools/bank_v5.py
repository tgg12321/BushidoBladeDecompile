"""Bank the tmp-only reviewer probes and rewrite proof §3/§3A/§5 per the v4 layer-2 (run from repo root)."""
import re, shutil, pathlib

L = pathlib.Path("memory/grind/func_800187F4")
D = pathlib.Path("tmp/func_800187F4")
out = L / "r11" / "reviewer_probes"
out.mkdir(parents=True, exist_ok=True)
scores = {}
for l in open("tmp/f187/revprobes.log"):
    m = re.match(r"(\S+)\s+diff=(\d+)", l)
    if m:
        scores[m.group(1)] = int(m.group(2))
DS = "dead-store-fake-exception.md prerequisite 3: \"**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement.\""
DW = "do-while-zero-exception.md prerequisite 1: \"**Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site** (not file-header prose), naming the observed effect\""
fam = {
    "rv_wd_end": ("v1 layer-2", "work split + `sq1 = 0;` dead store at the end of the ellipsoid body", "SET ASIDE: dead store", DS),
    "rv_wd_after2": ("v1 layer-2", "work split + `sq1 = 0;` dead store before `tot`", "SET ASIDE: dead store", DS),
    "rv_both": ("v1 layer-2", "work and delta split + both dead stores", "SET ASIDE: dead stores", DS),
    "rv_tp1": ("v1 layer-2", "temp split + chain-extender `work + lut1 - lut1`", "SET ASIDE: chain-extender", DS),
    "rv_tp2": ("v1 layer-2", "temp split + dead stores", "SET ASIDE: dead store", DS),
    "rv_tp3": ("v1 layer-2", "temp split (bytes at loop-body scope) + chain-extender `work + lut1 - lut1`", "SET ASIDE: chain-extender", DS),
    "rv_tp4": ("v1 layer-2", "temp split, table-byte locals at loop-body scope", "COUNTS (FAKE-free)", ""),
    "rv_nf0": ("v2 layer-2", "nforce split", "COUNTS (FAKE-free)", ""),
    "rv_nfA1": ("v2 layer-2", "nforce split + do-while(0) wraps", "SET ASIDE: do-while(0)", DW),
    "rv_nfA1b": ("v2 layer-2", "nforce split + do-while(0) wraps", "SET ASIDE: do-while(0)", DW),
    "rv_nfA1c": ("v2 layer-2", "nforce split + do-while(0) wraps", "SET ASIDE: do-while(0)", DW),
    "rv_nfA2": ("v2 layer-2", "nforce split + do-while(0) wraps", "SET ASIDE: do-while(0)", DW),
    "rv4_base": ("v4 layer-2", "landing template control (the reuse spelling)", "control", ""),
}
v4 = {"rv4_nb_u32": "r11pv_nbits, u32 lzcount", "rv4_nb_reg": "register lzcount, shift", "rv4_nb_andvar": "lzcount = lz[0] & ~1; shift = 0x16 - lzcount",
      "rv4_nb_neg": "shift = -(lzcount & ~1) + 0x16", "rv4_nb_s16": "s16 shift", "rv4_dl_reg": "register dg", "rv4_dl_tern": "vy_new = vy - (dg > 0x3200 ? 0x400 : dg / 8)",
      "rv4_dl_inv": "if (dg <= 0x3200) {div} else {0x400}", "rv4_dl_split": "vy_new = vy; vy_new -= ... compound split", "rv4_dl_init": "s32 dg = ... initializer",
      "rv4_tq_u32": "u32 lzc_in (the Q28 copy as a fresh local)", "rv4_tq_reg": "register lzc_in", "rv4_nf_reg": "register nforce_add/nforce_sub",
      "rv4_ix_reg": "register idx_add/idx_sub", "rv4_wk_reg": "register sq1", "rv4_wk_u32": "u32 sq1"}
for k, v in v4.items():
    fam[k] = ("v4 layer-2", v, "COUNTS (FAKE-free)", "")
rows = ["# Reviewer-proposed spellings banked from tmp/ (measured 2026-09-28 with r11/tools/fast3.sh)", "",
        "lines = differing objdump lines vs the target on the real per-file recipe. Bodies: this directory.", "",
        "| tag | proposed by | spelling | lines | status under Q30/Q32 | annotation requirement (set-aside only) |", "|---|---|---|---|---|---|"]
for tag in sorted(fam):
    shutil.copy(D / f"{tag}.c", out / f"{tag}.c")
    by, what, st, q = fam[tag]
    rows.append(f"| {tag} | {by} | {what} | {scores[tag]} | {st} | {q} |")
rows += ["", "Counting spellings reaching 0: none (best 2). The four zeros are the reuse control and the dead-store closers, set aside."]
(out / "probes.md").write_text("\n".join(rows) + "\n", encoding="utf-8", newline="\n")
print("banked", len(fam))
