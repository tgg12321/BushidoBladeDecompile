"""modules4f.py: the module / gap table of the old main.c (.text 0x80083BE4..0x8008BE04), link order.
(start, kind, lib, module, file id, what). kind: 'v' = verbatim LIBSCAN module (docs/naming/libscan/
matches.json); 'v40u' = bit-verbatim in the Jun-06-1997 PsyQ 4.0 build (memory/closer/libsnd-hunt-report.md);
'gap' = an unidentified region between placed modules (one file per gap, Q106 D3, named by ROM offset)."""

P = "main/psxsdk/"


def rom(a):
    return f"{a - 0x80010000 + 0x800:X}"


MODS = [
    (0x80083BE4, "v", "LIBSND", "SSSMV", "libsnd/scsmvol"),
    (0x80083C34, "v", "LIBSND", "SSSTART", "libsnd/ssstart"),
    (0x80083F6C, "v", "LIBSND", "SSCALL", "libsnd/sscall"),
    (0x800841E0, "gap", "LIBSND", None, None),
    (0x800848AC, "v", "LIBSND", "PAUSE", "libsnd/pause"),
    (0x80084948, "v", "LIBSND", "PLAY", "libsnd/play"),
    (0x80084974, "gap", "LIBSND", None, None),
    (0x80085064, "v", "LIBSND", "MIDITIME", "libsnd/miditime"),
    (0x80085114, "v", "LIBSND", "NEXT", "libsnd/next"),
    (0x80085210, "v", "LIBSND", "REPLAY", "libsnd/replay"),
    (0x80085270, "v40u", "LIBSND", "SSSTOP", "libsnd/stop"),
    (0x80085448, "v", "LIBSND", "SSSV", "libsnd/scssvol"),
    (0x80085544, "v", "LIBSND", "SSTICK", "libsnd/sstick"),
    (0x800856B0, "v", "LIBSND", "TEMPO", "libsnd/tempo"),
    (0x800858D0, "gap", "LIBSND", None, None),
    (0x800859F0, "v", "LIBSND", "UT_GVBA", "libsnd/ut_gvba"),
    (0x80085A40, "gap", "LIBSND", None, None),
    (0x80085E4C, "v", "LIBSND", "UT_RDEP", "libsnd/ut_rdep"),
    (0x80085EE4, "v", "LIBSND", "UT_REV", "libsnd/ut_rev"),
    (0x80085F98, "v", "LIBSND", "UT_ROFF", "libsnd/ut_roff"),
    (0x80085FB8, "v", "LIBSND", "UT_RON", "libsnd/ut_ron"),
    (0x80085FD8, "gap", "LIBSND", None, None),
    (0x800863CC, "v", "LIBSND", "VM_DOFF", "libsnd/vm_doff"),
    (0x800863DC, "gap", "LIBSND", None, None),
    (0x80086B38, "v", "LIBSND", "VM_N2P", "libsnd/vm_n2p"),
    (0x80086CF8, "gap", "LIBSND", None, None),
    (0x80087E3C, "v", "LIBSND", "VM_VSU", "libsnd/vm_vsu"),
    (0x80087F00, "v", "LIBSND", "VS_AUTO", "libsnd/vs_auto"),
    (0x80087F10, "v", "LIBSND", "VS_MONO", "libsnd/vs_mono"),
    (0x80087F34, "v", "LIBSND", "VS_SRV", "libsnd/vs_srv"),
    (0x80087F64, "v", "LIBSND", "VS_VAB", "libsnd/vs_vab"),
    (0x80087FE8, "v", "LIBSND", "VS_VFB", "libsnd/vs_vfb"),
    (0x80088058, "v", "LIBSND", "VS_VH", "libsnd/vs_vh"),
    (0x800884C4, "v", "LIBSND", "VS_VTB", "libsnd/vs_vtb"),
    (0x80088584, "v", "LIBSND", "VS_VTC", "libsnd/vs_vtc"),
    (0x800885AC, "v", "LIBSPU", "S_I", "libspu/s_i"),
    (0x800885CC, "v", "LIBSPU", "S_INI", "libspu/s_ini"),
    (0x80088740, "v", "LIBSPU", "SPU", "libspu/spu"),
    (0x800892D4, "v", "LIBSPU", "S_DCB", "libspu/s_dcb"),
    (0x800892F8, "v", "LIBSPU", "S_Q", "libspu/s_q"),
    (0x80089374, "v", "LIBAPI", "A13", "libapi/a13"),
    (0x80089384, "v", "LIBSPU", "S_M_INIT", "libspu/s_m_init"),
    (0x800893D8, "v", "LIBSPU", "S_M_M", "libspu/s_m_m"),
    (0x800896A0, "v", "LIBSPU", "S_M_INT", "libspu/s_m_int"),
    (0x800899A8, "v", "LIBSPU", "S_M_F", "libspu/s_m_f"),
    (0x80089A24, "v", "LIBSPU", "S_SNV", "libspu/s_snv"),
    (0x80089A48, "gap", "LIBSPU", None, None),
    (0x80089D10, "v", "LIBSPU", "S_SNC", "libspu/s_snc"),
    (0x80089D60, "v", "LIBSPU", "S_SR", "libspu/s_sr"),
    (0x80089E30, "v", "LIBSPU", "S_M_UTIL", "libspu/s_m_util"),
    (0x80089F3C, "v", "LIBSPU", "S_SRMP", "libspu/s_srmp"),
    (0x8008A434, "v", "LIBSPU", "S_SRA", "libspu/s_sra"),
    (0x8008A904, "v", "LIBSPU", "S_SRV", "libspu/s_srv"),
    (0x8008A928, "v", "LIBSPU", "S_CRWA", "libspu/s_crwa"),
    (0x8008AAC4, "v", "LIBAPI", "A10", "libapi/a10"),
    (0x8008AAD4, "v", "LIBSPU", "S_SK", "libspu/s_sk"),
    (0x8008ACD0, "v", "LIBSPU", "S_GKS", "libspu/s_gks"),
    (0x8008AD64, "v", "LIBSPU", "S_R", "libspu/s_r"),
    (0x8008ADC4, "v", "LIBSPU", "S_W", "libspu/s_w"),
    (0x8008AE24, "v", "LIBSPU", "S_STSA", "libspu/s_stsa"),
    (0x8008AE7C, "v", "LIBSPU", "S_STM", "libspu/s_stm"),
    (0x8008AEB0, "v", "LIBSPU", "S_ITC", "libspu/s_itc"),
    (0x8008AF58, "v", "LIBSPU", "S_IT", "libspu/s_it"),
    (0x8008AF9C, "v", "LIBSPU", "S_SCA", "libspu/s_sca"),
    (0x8008B330, "v", "LIBSPU", "SR_GAKS", "libspu/sr_gaks"),
    (0x8008B488, "gap", "LIBSPU", None, None),
    (0x8008BA94, "v", "LIBSPU", "S_N2P", "libspu/s_n2p"),
    (0x8008BD88, "v", "LIBSPU", "S_GVV", "libspu/s_gvv"),
    (0x8008BDE8, "v", "LIBSPU", "S_GVEX", "libspu/s_gvex"),
]
END = 0x8008BE04
# gaps take their library directory and ROM-offset name
MODS = [(a, k, lib, mod, fid if fid else f"{lib.lower()}/{rom(a)}") for a, k, lib, mod, fid in MODS]
MODS = [(a, k, lib, mod, P + fid) for a, k, lib, mod, fid in MODS]


def span(i):
    return MODS[i][0], (MODS[i + 1][0] if i + 1 < len(MODS) else END)
