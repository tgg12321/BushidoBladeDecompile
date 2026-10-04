#!/usr/bin/env python3
"""Work item D / Q109: cut the renamed sound-library gap files at their xref/near-tier module starts
and rewrite every Q109 file's top comment with its evidence tier. Moves only: each part takes the old
file's lines verbatim from the comment block in front of its first function (or the old file's first
line after the includes) to the line before the next part; each part gets the old file's two include
lines. Usage (repo root): split_q109.py [--apply]"""
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if not os.path.exists(os.path.join(ROOT, "bb2.ld")):
    ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
os.chdir(ROOT)
D = "src/main/psxsdk/"
LS = "docs/naming/libscan/"
SND40 = "PsyQ 4.0 LIBSND.LIB"
NEWER = ("Not a verbatim LIBSCAN span: BB2 links an interim LIBSND build, between PsyQ 4.0 and 4.1, that no "
         "archived release holds (memory/closer/libsnd-hunt-report.md)")


def c(*lines):
    import textwrap
    text = " ".join(l.strip() for l in lines)
    out = textwrap.wrap(text, width=100, break_long_words=False, break_on_hyphens=False)
    out = ["/* " + out[0]] + [" * " + l for l in out[1:]]
    out[-1] += " */"
    return "\n".join(out) + "\n"


HDR = {
    "libsnd/cres": c(
        "PsyQ LIBSND CRES: _SsSndCrescendo. .text 0x800841E0..0x80084500. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan xref tier: the verbatim SSCALL module's REL26 at +0xF8 names",
        "_SsSndCrescendo -> EXE jal 0x800841E0 (" + LS + "near_manifest.csv), CRES's only XDEF (+0x0, " + SND40 + ")."),
    "libsnd/decre": c(
        "PsyQ LIBSND DECRES: _SsSndDecrescendo. .text 0x80084500..0x800848AC. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan xref tier: the verbatim SSCALL module's REL26 at +0x120 names",
        "_SsSndDecrescendo -> EXE jal 0x80084500 (" + LS + "near_manifest.csv), DECRES's only XDEF (+0x0, " + SND40 + ").",
        "File name: SOTN's (sotn-decomp src/main/psxsdk/libsnd/decre.c)."),
    "libsnd/midiread": c(
        "PsyQ LIBSND MIDIREAD: _SsSeqPlay, _SsSeqGetEof and func_80084CC0 (_SsGetSeqData's offset). .text",
        "0x80084974..0x80085064, the whole region between PLAY and MIDITIME. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan xref tier: PLAY, placed at 0x80084948 (" + LS + "ambiguous_resolutions.md),",
        "has one REL26, naming _SsSeqPlay, and the EXE word there is jal 0x80084974 (RELOC_CHAIN_ID); " + SND40 + " MIDIREAD XDEFs",
        "_SsSeqPlay +0x0, _SsSeqGetEof +0x108, _SsGetSeqData +0x34C, the offsets of the three functions here."),
    "libsnd/ut_keyv": c(
        "PsyQ LIBSND UT_KEYV: SsUtKeyOnV and SsUtKeyOffV. .text 0x80085A40..0x80085E4C, the whole region",
        "between UT_GVBA and UT_RDEP. Module start (owner ruling Q109), libscan near tier: UT_KEYV of the",
        SND40 + " matches 255/259 words at 0x80085A40, unique (the 4 differing words are the _svm_voice",
        "stride edit), XDEFs SsUtKeyOnV +0x0 and SsUtKeyOffV +0x394 (" + LS + "near_manifest.csv)."),
    "libsnd/ut_vvol": c(
        "PsyQ LIBSND UT_VVOL: func_80085FD8 (SsUtGetDetVVol), SsUtSetDetVVol, func_80086080 (SsUtGetVVol) and",
        "func_80086130 (at SsUtSetVVol's place in the module). .text 0x80085FD8..0x800861BC. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan near tier: the SsUtGetDetVVol sub-function of UT_VVOL",
        "(PsyQ 4.1-4.4) places 0/14 at 0x80085FD8, XDEF +0x0 (" + LS + "near_manifest.csv, CONFIRM; the",
        "SsUtSetDetVVol and SsUtGetVVol rows there are near too)."),
    "libsnd/vm_aloc2": c(
        "PsyQ LIBSND VM_ALOC2: _SsVmDoAllocate. .text 0x800861BC..0x800863CC (VM_DOFF follows). " + NEWER + ".",
        "Module start (owner ruling Q109), libscan near tier: the near-verbatim UT_KEYV's REL26 at +0x310 names",
        "_SsVmDoAllocate in all six builds -> EXE jal 0x800861BC (" + LS + "near_manifest.csv), VM_ALOC2's",
        "only XDEF (+0x0, " + SND40 + ")."),
    "libsnd/vm_f": c(
        "PsyQ LIBSND VM_F: _SsVmFlush. .text 0x800863DC..0x80086818. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan xref tier: the verbatim SSCALL module's REL26 at +0x44 names",
        "_SsVmFlush -> EXE jal 0x800863DC (" + LS + "near_manifest.csv), VM_F's only .text XDEF (+0x0,",
        SND40 + ")."),
    "libsnd/vm_init": c(
        "PsyQ LIBSND VM_INIT: _SsVmInit. .text 0x80086818..0x80086B38 (VM_N2P follows). " + NEWER + ".",
        "Module start (owner ruling Q109), libscan xref tier: the verbatim SSINIT module's REL26 at +0x80 names",
        "_SsVmInit -> EXE jal 0x80086818 (" + LS + "near_manifest.csv), VM_INIT's only XDEF (+0x0, " + SND40 + ")."),
    "libsnd/vm_no1": c(
        "PsyQ LIBSND VM_NO1: vmNoiseOn. .text 0x80086CF8..0x800871D4. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan near tier: the near-verbatim UT_KEYV's REL26 at +0x32C names",
        "vmNoiseOn in all six builds -> EXE jal 0x80086CF8 (" + LS + "near_manifest.csv), VM_NO1's only XDEF",
        "(+0x0, " + SND40 + ")."),
    "libsnd/vm_nowof": c(
        "PsyQ LIBSND VM_NOWOF: _SsVmKeyOffNow. .text 0x800871D4..0x800872A4. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan near tier: VM_NOWOF (PsyQ 4.1) matches 50/52 words at",
        "0x800871D4, unique, and the near-verbatim UT_KEYV names _SsVmKeyOffNow -> EXE jal 0x800871D4",
        "(" + LS + "near_manifest.csv); VM_NOWOF's only XDEF (+0x0, " + SND40 + ")."),
    "libsnd/vm_nowon": c(
        "PsyQ LIBSND VM_NOWON: _SsVmKeyOnNow. .text 0x800872A4..0x80087770. " + NEWER + ".",
        "Module start (owner ruling Q109), libscan near tier: the near-verbatim UT_KEYV's REL26 at +0x348 names",
        "_SsVmKeyOnNow in all six builds -> EXE jal 0x800872A4 (" + LS + "near_manifest.csv). VM_NOWON's",
        "only XDEF is _SsVmKeyOnNow (+0x0, " + SND40 + "), so the module ends where that function does."),
    "libsnd/77F70": c(
        "LIBSND code between VM_NOWON and VM_VSU: func_80087770, _SsVmGetSeqVol, func_80087D10, func_80087D58",
        "and _SsVmSeqKeyOff. .text 0x80087770..0x80087E3C. By layout this is LIBSND VM_SEQ (" + SND40 + " XDEFs",
        "_SsVmSetSeqVol +0x0, _SsVmGetSeqVol +0x538, _SsVmGetSeqLVol +0x59C, _SsVmGetSeqRVol +0x5E4,",
        "_SsVmSeqKeyOff +0x62C; here +0x53C, +0x5A0, +0x5E8, +0x630), but its first function func_80087770 is",
        "not identified at the libscan xref or near tier, so the region stays one gap file (owner ruling",
        "Q109), named by its ROM offset."),
    "libsnd/760D0": c(
        "LIBSND code between TEMPO and UT_GVBA: func_800858D0. .text",
        "0x800858D0..0x800859F0. By link order and size it is probably LIBSND UT_AKO (SsUtAllKeyOff;",
        "memory/closer/libsnd-hunt-report.md, PROBABLE), but no libscan xref or near-tier evidence identifies",
        "it, so the region stays one gap file (owner rulings Q106 D3, Q109), named by its ROM offset."),
    "libspu/s_sav": c(
        "PsyQ LIBSPU S_SAV: _SpuSetAnyVoice. .text 0x80089A48..0x80089D10, the whole region between S_SNV and",
        "S_SNC (a newer build than PsyQ 4.0's 0x208-byte S_SAV; memory/closer/libsnd-hunt-report.md).",
        "Module start (owner ruling Q109), libscan xref tier: the verbatim S_SNV and S_SRV modules' REL26 at",
        "+0xC name _SpuSetAnyVoice -> EXE jal 0x80089A48 (" + LS + "near_manifest.csv), S_SAV's only XDEF",
        "(+0x0, PsyQ 4.0 LIBSPU.LIB)."),
    "libspu/7BC88": c(
        "LIBSPU code between SR_GAKS and S_N2P: func_8008B488. .text 0x8008B488..0x8008BA94. By link order and",
        "size it is probably LIBSPU S_SVA (SpuSetVoiceAttr; memory/closer/libsnd-hunt-report.md, PROBABLE), but",
        "no libscan xref or near-tier evidence identifies it, so the region stays one gap file (owner rulings",
        "Q106 D3, Q109), named by its ROM offset."),
}

# old file (after the rename-first commits) -> [(part id, first line of the part)]; the first part
# starts right after the old includes. A part's first line is the first comment line in front of its
# first function (comments travel with the function they precede).
SPLITS = {
    "libsnd/cres": [("libsnd/cres", None), ("libsnd/decre", "void _SsSndDecrescendo(s16 a0, s16 a1) {")],
    "libsnd/ut_vvol": [("libsnd/ut_vvol", None),
                       ("libsnd/vm_aloc2", "/* Sony LIBSND `_SsVmDoAllocate` (psyz vm_aloc2.c analog): set up the")],
    "libsnd/vm_f": [("libsnd/vm_f", None),
                    ("libsnd/vm_init", "/* _SsVmInit - libsnd voice-manager init (SLUS-00663). */")],
    "libsnd/vm_no1": [("libsnd/vm_no1", None),
                      ("libsnd/vm_nowof", "/* Sony LIBSND `_SsVmKeyOffNow` (probable): mark the current voice's pending"),
                      ("libsnd/vm_nowon", "/* Sony LIBSND `_SsVmKeyOnNow` (VM_NOWON): compute the current voice's L/R"),
                      ("libsnd/77F70", "/* func_80087770: Sony LIBSND vmanager _SsVmSetSeqVol. The volume chain")],
}
RETITLE = ["libsnd/midiread", "libsnd/ut_keyv", "libsnd/760D0", "libspu/s_sav", "libspu/7BC88"]


def read(tid):
    s = open(D + tid + ".c", encoding="utf-8", newline="").read()
    assert "\r" not in s and s.endswith("\n")
    return s[:-1].split("\n")


def head_split(L):
    """(index after the top comment, index of the first line after the include block)."""
    e = next(i for i, l in enumerate(L) if l.rstrip().endswith("*/")) + 1
    j = e
    while j < len(L) and (L[j].startswith("#include") or L[j].startswith("#define")):
        j += 1
    return e, j


out = {}
for old, parts in SPLITS.items():
    L = read(old)
    e, j = head_split(L)
    incl = L[e:j]
    assert incl in (['#include "common.h"', '#include "libsnd_i.h"'],), (old, incl)
    starts = [j] + [L.index(first) for _, first in parts[1:]]
    assert starts == sorted(starts) and len(set(starts)) == len(starts)
    starts.append(len(L))
    for k, (tid, _) in enumerate(parts):
        body = L[starts[k]:starts[k + 1]]
        while body and body[0] == "":
            body = body[1:]
        while body and body[-1] == "":
            body = body[:-1]
        out[tid] = HDR[tid] + "\n".join(incl) + "\n\n" + "\n".join(body) + "\n"
for tid in RETITLE:
    L = read(tid)
    e, _ = head_split(L)
    out[tid] = HDR[tid] + "\n".join(L[e:]) + "\n"

# moves only: the parts' bodies, concatenated in order, are the old bodies
for old, parts in SPLITS.items():
    L = read(old)
    e, j = head_split(L)
    old_body = [l for l in L[j:] if l != ""]
    new_body = []
    for tid, _ in parts:
        P = out[tid].split("\n")
        pe, pj = head_split(P)
        new_body += [l for l in P[pj:] if l != ""]
    assert old_body == new_body, old

if "--apply" in sys.argv:
    for tid, s in out.items():
        open(D + tid + ".c", "w", encoding="utf-8", newline="\n").write(s)
for tid, s in out.items():
    print(f"== {tid}.c ({s.count(chr(10))} lines)")
    print(s[:s.index('#include')])
