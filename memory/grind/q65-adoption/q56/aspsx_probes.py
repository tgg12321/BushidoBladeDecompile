#!/usr/bin/env python3
"""aspsx_probes.py — the rule probes behind the per-file model, run on Sony's own Psy-Q ASPSX 2.34
(psyq3.5 archive, dosemu2; tmp/research36140/aspsx.sh, the same harness research-common-gp.md used).
usage (WSL, repo root): python3 tmp/q56/aspsx_probes.py  -> tmp/q56/aspsx_probe_results.txt"""
import subprocess, sys
from pathlib import Path
sys.path.insert(0, "tmp/q56")
import psyqobj

HERE = Path("tmp/q56/probes"); HERE.mkdir(exist_ok=True)
OPS = {0x20: "lb", 0x24: "lbu", 0x23: "lw", 0x28: "sb", 0x2B: "sw", 0x21: "lh", 0x25: "lhu", 0x29: "sh"}

def dec(w):
    op = w >> 26
    if w == 0: return "nop"
    if op == 0x0F: return f"lui r{(w >> 16) & 31}"
    if op == 0 and (w & 0x3F) == 0x21: return f"addu r{(w >> 11) & 31},r{(w >> 21) & 31},r{(w >> 16) & 31}"
    if op == 0x09 and ((w >> 21) & 31) == 0 and ((w >> 16) & 31) == 0: return f"MARK{w & 0xFFFF}"
    if op in OPS: return f"{OPS[op]} base=r{(w >> 21) & 31}" + (" (GP)" if (w >> 21) & 31 == 28 else "")
    return f"op{op:x}"

P = {
    # the storage-class table of research-common-gp.md s0, re-run
    "comm4":   ["\t.comm\tcv,4", "\t.text", "\tlb\t$4,cv", "\tlb\t$5,cv+1"],
    "lcomm4":  ["\t.lcomm\tlv,4", "\t.text", "\tlb\t$4,lv", "\tlb\t$5,lv+1"],
    "sdata4":  ["\t.sdata", "sv:", "\t.word\t0", "\t.text", "\tlb\t$4,sv", "\tlb\t$5,sv+1"],
    "extern4": ["\t.extern\tev,4", "\t.text", "\tlb\t$4,ev", "\tlb\t$5,ev+1"],
    "undecl":  ["\t.text", "\tlb\t$4,uv"],
    # new: one file, two functions -> the decision is per file, identical in both
    "two_funcs": ["\t.comm\ts2,4", "\t.text", "\t.ent\tfa", "fa:", "\tlw\t$2,s2", "\tjr\t$31", "\tnop", "\t.end\tfa",
                  "\t.ent\tfb", "fb:", "\tsw\t$3,s2", "\tjr\t$31", "\tnop", "\t.end\tfb"],
    # new: -G8 threshold: a 12-byte COMMON object is not small data
    "comm12":  ["\t.comm\tbig,12", "\t.text", "\tlw\t$2,big"],
    "comm8":   ["\t.comm\teight,8", "\t.text", "\tlw\t$2,eight"],
    # new: an INDEXED store of a small-data symbol right after a load of the stored register
    # (func_8003047C's shape): expanded via $at, and does ASPSX add a load-delay nop?
    "indexed_after_load_comm":   ["\t.comm\tix,4", "\t.text", "\tlbu\t$2,0($4)", "\tsb\t$2,ix($3)"],
    "indexed_after_load_extern": ["\t.text", "\tlbu\t$2,0($4)", "\tsb\t$2,ix2($3)"],
    "direct_after_load_comm":    ["\t.comm\tdx,4", "\t.text", "\tlbu\t$2,0($4)", "\tsb\t$2,dx"],
    # explicit relocation operators on a small symbol the file defines (hand-written asm form): does ASPSX
    # leave %hi/%lo as written, or rewrite it to gp? (the per-file-gp-model.md explicit-relocation clause)
    "explicit_hilo_comm":  ["\t.comm\tex,4", "\t.text", "\tlui\t$4,%hi(ex)", "\tlw\t$4,%lo(ex)($4)"],
    "explicit_hilo_lcomm": ["\t.lcomm\tel,4", "\t.text", "\tlui\t$4,%hi(el)", "\tlw\t$4,%lo(el)($4)"],
    "explicit_hilo_sdata": ["\t.sdata", "es:", "\t.word\t0", "\t.text", "\tlui\t$4,%hi(es)", "\tlw\t$4,%lo(es)($4)"],
    "macro_same_symbol":   ["\t.comm\tem,4", "\t.text", "\tlw\t$4,em"],
}


def main():
    out = []
    for name, lines in P.items():
        s = HERE / f"{name}.s"; o = HERE / f"{name}.obj"
        s.write_text("\n".join(lines) + "\n")
        o.unlink(missing_ok=True)
        r = subprocess.run(["bash", "tmp/research36140/aspsx.sh", str(s), str(o), "-G8"], capture_output=True, text=True)
        err = [l.strip() for l in r.stderr.splitlines() if "Error" in l]
        if r.returncode or err or not o.exists():
            out.append(f"{name}: REJECTED by ASPSX {err[:1]}"); continue
        t = psyqobj.read_text(o.read_bytes())
        ws = [int.from_bytes(t[i:i + 4], "little") for i in range(0, len(t), 4)]
        out.append(f"{name}: " + " | ".join(dec(w) for w in ws))
    open("tmp/q56/aspsx_probe_results.txt", "w", newline="\n").write("\n".join(out) + "\n")
    print("\n".join(out))


if __name__ == "__main__":
    main()
