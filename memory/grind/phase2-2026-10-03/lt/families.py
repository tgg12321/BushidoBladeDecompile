#!/usr/bin/env python3
# Group tmp/p2/lt/casts.tsv sites into object families (one real object per family).
import collections, re
rows = []
with open("tmp/p2/lt/casts.tsv", encoding="utf-8") as f:
    hdr = f.readline().lstrip("# ").rstrip("\n").split("\t"); hdr[-1] = "cast_note"
    for line in f:
        rows.append(dict(zip(hdr, line.rstrip("\n").split("\t"))))

WS51 = re.compile(r"^D_800A34[6-9A-E][0-9A-F]$")
FAM = collections.OrderedDict([
 ("F01", ("51268 scratchpad workspace 0x1F800000..0x1F8000BC (reached through the D_800A3468..D_800A34EC pointer globals)",
          "no (the globals are s32 / u8 *; no struct)", "func_80060E38 seeds each global with a 0x1F8000xx address (51268.c:268-299): the field map is in that code; widths from the asm per site",
          "none recorded; the globals are all TU-static in 51268")),
 ("F02", ("17AFC collision scratchpad workspace (scr / obj on 0x1F800xxx)", "partly: game.h:636-650 ScrPad / SPAD (0x1F800000 point tables); 51268 overlays the same scratchpad with F01 at another time", "scratchpad literals in the locals / params (casts.py how=scratchpad-*)", "none recorded")),
 ("F03", ("51268 layout-A draw context (s32 word array, func_8006E390's) and its resource root (*(arg0 + 4))",
          "no: kept a word array by ruling (P3 round 2)", "func_8006E390 fills it; frames s32[15] / s32[22]",
          "RULED a word array (func_8006A880 needs non-struct access; callers' frame sizes). Only the resource-root reads (*(s32 *)(arg0 + 4) + off) are open")),
 ("F04", ("5ED34 resource roots D_800A35C4 / D_800A35A8 / D_800A3568 (+ 64FD8 D_800A35F8)", "no (void * / s32 TU globals)", "the files they point into (MOD.BIN sections); asm widths per site",
          "EnvA descriptor unification and the sheet-header tables (D_8009B708-style) touch the same bodies")),
 ("F05", ("368E4 transform nodes (0x68-byte Unk80101DF0Record family) + vehicle / part", "yes: game.h Unk80101DF0Record / Unk800A6690Rec",
          "func_800417D0 takes the node as Unk80101DF0Record *; vehicle +0x7E4 parts of 0x68",
          "368E4 transform-nodes long-tail row (ot -> list, prim name, comment 'ordering table'); P7c's g_gpu_ot256_ptr rides on it; P7b's 368E4 bodies")),
 ("F06", ("SPU register block _spu_RXX (libspu)", "partly: header-global s32 base", "SPU MMIO 0x1F801C00..; PsyQ libspu's union SPU_RXX", "hardware: mmio-volatile-type-level applies")),
 ("F07", ("3AB48 move-to-target context (D_800A33F4 / p in func_80053304 family)", "no (s32 TU global)", "named_syms init_move_to_target_80053304: ctx D_800EF9F8 / D_800A33F4", "none recorded")),
 ("F08", ("309CC records (a0 s16 * vectors in func_800404A0 / 40594 / 408F8; arg0 large object +0x18F4.. in func_80040A78 / B44 / CB8)",
          "check: arg0's +0x10D4 / +0x18F4 offsets look like the character object (Unk80101EC8Record?)", "asm widths", "none recorded")),
 ("F09", ("28708 records (D_800A36EC 0x1C records in func_8003993C; slot / dest records in func_800393C8 / 395B4 / 39680)",
          "partly: game.h has the 0x1C-byte record comment (func_8003993C)", "asm/funcs/func_8003993C.s:57-89 stride 0x1C", "none recorded")),
 ("F10", ("31548 func_80040D48 animation locals s1..s4", "check", "asm widths", "func_80040D48 is called with &vec.vx (3AB48:915): first-member-pass row")),
 ("F11", ("368E4 other bases (p u32 * lists in func_80047EE8 family; s0 in func_800460E4 family; arg0 in func_800482C8 family)",
          "check", "asm", "368E4 transform-nodes batch touches the same TU")),
 ("F12", ("libgpu internals (sys.c rect / g_gpu_ctx, ext.c SetDefDrawEnv / SetDefDispEnv a0)", "yes: RECT / DRAWENV / DISPENV in libgpu.h",
          "PsyQ LIBGPU.H", "Sony library code: verbatim-module spelling")),
 ("F13", ("51268 POLY_FT4 holder views (prim in func_8006295C / 80063084 / 80063E10)", "yes: POLY_FT4", "libgpu.h", "P1's setlen / OTag len rows")),
 ("F14", ("51268 p (void *) in the func_80064E90..func_800650A4 accessor family (+0 / +4 / +8)", "check", "asm", "none recorded")),
 ("F15", ("64FD8 Unk8006EACCRec residual views", "yes: game.h Unk8006EACCRec", "game.h", "EnvA descriptor unification")),
 ("F16", ("3AB48 func_80060768 chunk (tile_off)", "no: D7 ruled no record", "D7 RESULT: member forms 192 -> 191", "RULED: keeps int arithmetic (non-struct schedule)")),
 ("F17", ("3AB48 data pun (func_80053754 / 80053E9C)", "check", "asm", "none recorded")),
 ("F18", ("17AFC func_8002EECC arg0 record", "check", "asm", "none recorded")),
 ("F19", ("51268 work block at D_800A34FC (the pool end func_8006E49C returns; +0x34 header)", "check: game.h SelWork is the 64FD8 work area at the pool end",
          "func_8006E49C returns base4 + 0x1FB0; D_800A3500 = it + 0x34", "51268 bodies with FAKE fences (func_8006919C neighbourhood)")),
 ("F20", ("51268 D_800A3524 record (func_8006D74C / 693CC: the arg1 the game passes)", "check", "asm", "none recorded")),
 ("FZZ", ("misc (bases with fewer sites, not yet grouped)", "-", "-", "-")),
])

def fam(r):
    fl = r["file"].split("/")[-1][:-2]; root = r["root"]; fn = r["func"]; how = r["how"]
    if fl == "51268" and root == "D_800A34FC": return "F19"
    if fl == "51268" and root == "D_800A3524": return "F20"
    if fl == "51268":
        if WS51.match(root) or root in ("outer", "base"): return "F01"
        if root in ("arg0", "arg1", "") or root.startswith("arg"): return "F03"
        if root == "prim": return "F13"
        if root == "p" and fn.startswith("func_80064") or root == "p" and fn.startswith("func_800650"): return "F14"
    if fl == "17AFC":
        if how.startswith("scratchpad"): return "F02"
        if root == "arg0" and fn == "func_8002EECC": return "F18"
    if fl == "5ED34" and root in ("D_800A35C4", "D_800A35A8", "D_800A3568"): return "F04"
    if fl == "64FD8" and root == "D_800A35F8": return "F04"
    if fl == "64FD8" and root == "arg0" and r["decl"].startswith("Unk8006EACCRec"): return "F15"
    if fl == "368E4":
        if root in ("obj", "part", "vehicle", "prim") or fn in ("func_80048BA4", "func_80049718", "func_80049A2C"): return "F05"
        if root in ("p", "s0", "arg0"): return "F11"
    if root == "_spu_RXX": return "F06"
    if r["file"].endswith("s_sra.c") and root == "arg0": return "F06"
    if fl == "28708" and root == "D_800A36EC": return "F09"
    if fl == "3AB48":
        if root == "D_800A33F4" or (root == "p" and fn in ("func_80053304", "func_8005344C", "func_80053694", "func_80054604")): return "F07"
        if root == "tile_off": return "F16"
        if root == "data": return "F17"
    if fl == "309CC" and root in ("a0", "arg0", "p"): return "F08"
    if fl == "28708" and root in ("slot", "p", "e", "dest"): return "F09"
    if fl == "31548" and fn == "func_80040D48": return "F10"
    if fl in ("sys", "ext") and r["file"].find("libgpu") >= 0: return "F12"
    return "FZZ"

out = collections.defaultdict(list)
for r in rows:
    out[fam(r)].append(r)
with open("tmp/p2/lt/families.tsv", "w", encoding="utf-8", newline="\n") as f:
    f.write("# family\tsites\tfiles\tfuncs\tfake_sites\tname\theader_declared\tevidence\tblockers\tfunc_list\n")
    order = sorted(FAM, key=lambda k: (k == "FZZ", -len(out[k])))
    for k in order:
        rs = out[k]
        files = sorted(set(r["file"].split("/")[-1] for r in rs))
        funcs = sorted(set(r["func"] for r in rs))
        fk = sum(1 for r in rs if r["fake"])
        name, hd, ev, bl = FAM[k]
        f.write("\t".join([k, str(len(rs)), ",".join(files), str(len(funcs)), str(fk), name, hd, ev, bl, " ".join(funcs)]) + "\n")
        print("%s %5d sites %3d funcs %-30s %s" % (k, len(rs), len(funcs), ",".join(files)[:30], name[:70]))
print("total", sum(len(v) for v in out.values()))
# the misc bucket's biggest roots, for a second pass
mz = collections.Counter((r["file"].split("/")[-1], r["root"]) for r in out["FZZ"])
print("FZZ top:", mz.most_common(25))
