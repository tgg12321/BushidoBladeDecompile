"""Register maspsx_comm_syms.txt everywhere a maspsx gate list must be registered
(.claude/rules/maspsx-gate-lists.md § maspsx_comm_syms.txt prong (c)).
usage: python3 tmp/func_80036140/register.py <tree-root>   (run once, after gate.py)"""
import sys
from pathlib import Path

R = Path(sys.argv[1])
H = Path('tmp/func_80036140')
L = 'maspsx_comm_syms.txt'


def rw(rel, *pairs):
    p = R / rel
    s = p.read_text(encoding='utf-8')
    for old, new, *cnt in pairs:
        n = cnt[0] if cnt else 1
        assert s.count(old) == n, (rel, old[:70], s.count(old))
        s = s.replace(old, new)
    p.write_bytes(s.encode('utf-8'))


# ---- engine ------------------------------------------------------------------------------------
rw('engine/buildconfig.py',
   ('    "--prefill-label-funcs=maspsx_prefill_label_funcs.txt"\n)',
    '    "--prefill-label-funcs=maspsx_prefill_label_funcs.txt "\n    "--comm-syms=maspsx_comm_syms.txt"\n)', 2))
rw('engine/cheats.py',
   ('    "maspsx_prefill_label_funcs.txt": "fidelity",  # ASPSX "retarget iff filled" label placement (owner ruling 2026-09-04)\n',
    '    "maspsx_prefill_label_funcs.txt": "fidelity",  # ASPSX "retarget iff filled" label placement (owner ruling 2026-09-04)\n'
    '    "maspsx_comm_syms.txt": "fidelity",            # ASPSX: a COMMON symbol is gp at its base, never at sym+N (owner ruling 2026-09-26)\n'),
   ('    toggle behind them. Names are the first whitespace token per non-comment\n'
    '    line (same format as the other one-name-per-line sidecars)."""',
    '    toggle behind them. Names are the first whitespace token per non-comment\n'
    '    line (same format as the other one-name-per-line sidecars); a `func: sym, sym`\n'
    '    row (maspsx_comm_syms.txt) counts by its func."""'),
   ('    """Function names in a one-name-per-line prologue list (delay_slot_ra /\n'
    '    frame_fix); first whitespace token, \'#\'/blank lines ignored."""',
    '    """Function names in a one-name-per-line prologue list (delay_slot_ra /\n'
    '    frame_fix); first whitespace token, \'#\'/blank lines ignored. A trailing\n'
    '    \':\' is dropped, so a `func: sym, sym` row (maspsx_comm_syms.txt) yields func."""'),
   ('            out.add(tok[0])\n    return out\n\n\ndef prologue_fix_funcs',
    '            out.add(tok[0].rstrip(":"))\n    return out\n\n\ndef prologue_fix_funcs'))
rw('engine/test_engine.py',
   ('        check("canonical_asm_funcs: missing file -> empty set",\n'
    '              cheats.canonical_asm_funcs("/nonexistent") == set())\n',
    '        check("canonical_asm_funcs: missing file -> empty set",\n'
    '              cheats.canonical_asm_funcs("/nonexistent") == set())\n'
    '\n'
    '    # maspsx_comm_syms.txt (owner ruling 2026-09-26): `func: sym, sym` rows count by func,\n'
    '    # so queue done / the integrity audit see a function that depends on the gate\n'
    '    with tempfile.TemporaryDirectory() as td:\n'
    '        cl = Path(td) / "comm.txt"\n'
    '        cl.write_text("# header\\ncdrom_SetMix: g_cd_atv\\nfunc_B: x, y\\n\\nfunc_C\\n")\n'
    '        eq("gate list: `func: syms` rows yield the func name",\n'
    '           cheats._prologue_txt_funcs(str(cl)), {"cdrom_SetMix", "func_B", "func_C"})\n'
    '    eq("gate list: maspsx_comm_syms.txt is a fidelity gate",\n'
    '       cheats.MASPSX_GATE_LISTS.get("maspsx_comm_syms.txt"), "fidelity")\n'))
rw('engine/oracle.py',
   ('    "bb2.ld", "tools/prologue_config.json", "maspsx_prefill_label_funcs.txt",\n]',
    '    "bb2.ld", "tools/prologue_config.json", "maspsx_prefill_label_funcs.txt",\n    "maspsx_comm_syms.txt",\n]'),
   ('                                       "maspsx_prefill_label_funcs.txt"}',
    '                                       "maspsx_prefill_label_funcs.txt",\n'
    '                                       "maspsx_comm_syms.txt"}'))
rw('engine/dossier.py',
   ('        if any(re.search(r"^" + re.escape(n) + r"\\s*$", t, re.M) for n in names):\n'
    '            out.append(f)\n',
    '        if any(re.search(r"^" + re.escape(n) + r"\\s*$", t, re.M) for n in names):\n'
    '            out.append(f)\n'
    '    # `func: sym, sym` rows (the sdata_exclude.txt format)\n'
    '    t = _read("maspsx_comm_syms.txt")\n'
    '    if any(re.search(r"^" + re.escape(n) + r"\\s*:", t, re.M) for n in names):\n'
    '        out.append("maspsx_comm_syms.txt")\n'))
rw('engine/buildstamp.py',
   ("    'multu_funcs.txt', 'multu_pad_funcs.txt', 'maspsx_prefill_label_funcs.txt',\n",
    "    'multu_funcs.txt', 'multu_pad_funcs.txt', 'maspsx_prefill_label_funcs.txt',\n"
    "    'maspsx_comm_syms.txt',\n"))
rw('engine/queue.py',
   ('    joined 2026-09-25 — before that an edit to them never moved the\n'
    '    fingerprint."""\n'
    '    return _maspsx_package_inputs() + tuple(cheats.PROLOGUE_CONFIGS)\n',
    '    joined 2026-09-25 — before that an edit to them never moved the\n'
    '    fingerprint. maspsx_comm_syms.txt (the COMMON gate list) joined\n'
    '    2026-09-26 with the gate itself."""\n'
    '    return _prologue_inputs() + ("maspsx_comm_syms.txt",)\n'
    '\n'
    '\n'
    'def _prologue_inputs() -> tuple[str, ...]:\n'
    '    """The 2026-09-25 input set: the maspsx package set + prologue_fix\'s configs."""\n'
    '    return _maspsx_package_inputs() + tuple(cheats.PROLOGUE_CONFIGS)\n'),
   ('    return [_LEGACY_FINGERPRINT_INPUTS, _maspsx_package_inputs()]\n',
    '    return [_LEGACY_FINGERPRINT_INPUTS, _maspsx_package_inputs(), _prologue_inputs()]\n'))

# ---- tools -------------------------------------------------------------------------------------
rw('tools/check_root_cleanliness.py',
   ('    "maspsx_prefill_label_funcs.txt",\n', '    "maspsx_prefill_label_funcs.txt",\n    "maspsx_comm_syms.txt",\n'))
rw('tools/desync_audit.py',
   ('    "maspsx_prefill_label_funcs.txt": r"^\\s*([A-Za-z_]\\w*)\\s*$",\n',
    '    "maspsx_prefill_label_funcs.txt": r"^\\s*([A-Za-z_]\\w*)\\s*$",\n'
    '    "maspsx_comm_syms.txt": r"^\\s*([A-Za-z_]\\w*)\\s*:",\n'))
rw('tools/naming_wave.py',
   ('RULE_FILES = ["regfix.txt", "regfix_stage2.txt", "asmfix.txt"]',
    'RULE_FILES = ["regfix.txt", "regfix_stage2.txt", "asmfix.txt",\n'
    '              "maspsx_comm_syms.txt"]  # `func: sym, sym` — keys AND body names are pipeline keys'))
rw('tools/libscan/manifest.py',
   ('            "multu_pad_funcs.txt", "maspsx_prefill_label_funcs.txt", "sdata.txt"]',
    '            "multu_pad_funcs.txt", "maspsx_prefill_label_funcs.txt", "maspsx_comm_syms.txt",\n'
    '            "sdata.txt"]'))
rw('tools/grinder/grindlib.py',
   ('    "maspsx_prefill_label_funcs.txt",    # assembler fidelity gates: substrate-adjacent,\n',
    '    "maspsx_prefill_label_funcs.txt",    # assembler fidelity gates: substrate-adjacent,\n'
    '    "maspsx_comm_syms.txt",              # (COMMON gate: owner ruling 2026-09-26)\n'),
   ('    "maspsx_prefill_label_funcs.txt",\n    "multu_funcs.txt",\n    "multu_pad_funcs.txt",\n    "sdata_funcs.txt",\n',
    '    "maspsx_prefill_label_funcs.txt",\n    "maspsx_comm_syms.txt",\n    "multu_funcs.txt",\n    "multu_pad_funcs.txt",\n    "sdata_funcs.txt",\n'))
for hook in ('tools/hooks/main_reintegration_lock.py', 'tools/hooks/worktree_contamination_guard.py'):
    rw(hook, (r'|maspsx_prefill_label_funcs\.txt|expand_dest_funcs\.txt"',
              r'|maspsx_prefill_label_funcs\.txt|maspsx_comm_syms\.txt|expand_dest_funcs\.txt"'))
rw('tools/hooks/park_src_guard.py',
   ('  multu_pad_funcs.txt, maspsx_prefill_label_funcs.txt\n',
    '  multu_pad_funcs.txt, maspsx_prefill_label_funcs.txt, maspsx_comm_syms.txt\n'),
   ('    re.compile(r"^maspsx_prefill_label_funcs\\.txt$"),\n',
    '    re.compile(r"^maspsx_prefill_label_funcs\\.txt$"),\n    re.compile(r"^maspsx_comm_syms\\.txt$"),\n'))
(R / 'tools/maspsx/tests/test_comm_syms.py').write_bytes((H / 'test_comm_syms.py').read_bytes())

# ---- rule docs (registration halves) -----------------------------------------------------------
rw('.claude/rules/integration-handoff-self-serve.md',
   ('- The maspsx fidelity-gate lists (`maspsx_prefill_label_funcs.txt`,\n'
    '  `expand_lb_funcs.txt`, `expand_dest_funcs.txt`, `multu_funcs.txt`,\n'
    '  `multu_pad_funcs.txt`) — assembler-behavior gates are substrate-adjacent.\n',
    '- The maspsx fidelity-gate lists (`maspsx_prefill_label_funcs.txt`,\n'
    '  `maspsx_comm_syms.txt`, `expand_lb_funcs.txt`, `expand_dest_funcs.txt`,\n'
    '  `multu_funcs.txt`, `multu_pad_funcs.txt`) — assembler-behavior gates are\n'
    '  substrate-adjacent. (`maspsx_comm_syms.txt` joined both halves of the pair\n'
    '  with the gate itself, 2026-09-26.)\n'))
rw('.claude/rules/maspsx-gate-lists.md',
   ('a fifth, maspsx_comm_syms.txt, authorized 2026-09-26 and not yet built)',
    'a fifth, maspsx_comm_syms.txt, authorized and built 2026-09-26)'),
   ('admission prongs (a)-(d) in § "`maspsx_comm_syms.txt`" below. Authorized; not yet built at the time of the ruling. Growth tag',
    'admission prongs (a)-(d) in § "`maspsx_comm_syms.txt`" below. Built 2026-09-26 (`--comm-syms`, first rows cdrom_SetMix / func_80035F78 / func_80036140; evidence memory/grind/func_80036140/evidence.md). Growth tag'))
