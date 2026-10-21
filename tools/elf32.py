"""Minimal ELF32 little-endian reader: sections, symbols, REL relocations.

Shared by tools/make_psexe.py and tools/shift_test.py (no pyelftools in the venv).
"""
import struct
import sys

SHT_SYMTAB, SHT_REL = 2, 9
SHN_ABS = 0xFFF1


class Elf:
    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            self.data = d = f.read()
        if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 1:
            sys.exit("elf32: %s is not an ELF32 LE file" % path)
        shoff, = struct.unpack_from("<I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from("<HHH", d, 0x2E)
        self.secs = []
        for i in range(shnum):
            f = struct.unpack_from("<10I", d, shoff + i * shentsize)
            self.secs.append(dict(name_off=f[0], type=f[1], flags=f[2], addr=f[3],
                                  offset=f[4], size=f[5], link=f[6], info=f[7]))
        strtab = self.secs[shstrndx]
        for s in self.secs:
            s["name"] = self.cstr(strtab["offset"] + s["name_off"])
        self.syms = []
        for s in self.secs:
            if s["type"] == SHT_SYMTAB:
                names = self.secs[s["link"]]["offset"]
                for off in range(s["offset"], s["offset"] + s["size"], 16):
                    nm, val, size, info, _, ndx = struct.unpack_from("<IIIBBH", d, off)
                    self.syms.append(dict(name=self.cstr(names + nm), value=val, size=size,
                                          bind=info >> 4, type=info & 0xF, shndx=ndx))

    def cstr(self, off):
        return self.data[off:self.data.index(b"\0", off)].decode("latin-1")

    def sec_bytes(self, i):
        """Contents of a section, by index or header dict."""
        s = self.secs[i] if isinstance(i, int) else i
        return self.data[s["offset"]:s["offset"] + s["size"]]

    def rels(self):
        """{target section index: [(offset, type, symbol index)]}"""
        out = {}
        for s in self.secs:
            if s["type"] != SHT_REL:
                continue
            lst = []
            for off in range(s["offset"], s["offset"] + s["size"], 8):
                r_off, info = struct.unpack_from("<II", self.data, off)
                lst.append((r_off, info & 0xFF, info >> 8))
            out[s["info"]] = lst
        return out

    def section(self, name):
        """Section header dict by name, or None."""
        return next((s for s in self.secs if s["name"] == name), None)

    def symbol(self, name):
        """Value of the global (or else any) defined symbol `name`, or None."""
        hits = [s for s in self.syms if s["name"] == name and s["shndx"] != 0]
        hits.sort(key=lambda s: s["bind"] not in (1, 2))
        return hits[0]["value"] if hits else None

