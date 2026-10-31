"""wave110 rows (func-second-pass-18): the eff_ animated-quad command pairs 5/6 (records 1/2, eff_DrawAnimatedQuads modes 1/2) and
9/0xA (records 3/4, modes 3/4). Init handlers func_80064ED8 / func_80064F20 (commands 5 / 6) and func_80064F68 / func_80064FB4
(commands 9 / 0xA); draw handlers func_80065344 / func_80065394 (commands 5 / 6) and func_800653E4 / func_80065434 (commands 9 / 0xA)
(src/main/51268.c:1786-1818 and :1917-1963; d_8BC50.c:217-228 prototypes, handlers 12..15 and 20..23 at :281-284 and :289-292;
bases[5] = 0x0C, [6] = 0x0E, [9] = 0x14, [10] = 0x16 at :200-203). Research input tmp/codex/func-second-pass-18-research.md (untrusted;
every claim re-derived at HEAD 9955335a3, see notes.txt). Words: SpinningQuad (mode 1/2 geometry arm 51268.c:2342-2385: RotMatrix
about Z by the counter, corners rotated and added to the projected centre, shared only with modes 16/17), WithDepthSelectedTexture
(mode 1/2 texture arm :2163-2172, the only texture arm reading the projected depth; modes 16/17 use the fixed D_8009B8C8 :2289-2291),
Preseeded (the Init seeds the counter at 0x40, the only animated-quad Init that stores a nonzero counter: :1807 / :1816 against :1783,
:1791, :1799, :1825-1900 all 0), GrowingQuad (mode 3/4 size arm :2411-2416: w, h *= rsin(counter) >> 11 on a counter that runs
0x40..0xBD, all below the 0x400 sine peak, so the factor rises at every draw), FirstSlot / SecondSlot (the issuer's status byte: command 5 / 9
first, 6 / 0xA second, eff_StartRecycledCommandWithPaleRedLight :404-425 and ...RoseLight :325-346). Verdicts PENDING (the reviewer fills them).
(DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply 2026-10-31 (naming-c, HEAD 9955335a3): PASS 8/8 on code alone. Spinning = RotMatrix about Z on the counter (51268.c:2356-2377); DepthSelectedTexture = texture chosen by SZ3 > 4000 (:2165), sets modes 1/2 apart from spin modes 16/17; Preseeded = 0x40 counter seed unique to records 3/4 (:1807/:1816); Growing = rsin(counter)>>11 size factor rising every draw (0x40..0xBD < 0x400, :2411-2416). FirstSlot/SecondSlot = issuer status order (PaleRed :412-417, Rose :333-338). Knockout ef77b68a8 corroboration only."
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


# addr: (old, cmd, kind ("init"/"draw"), record, handler idx, d_8BC50 proto line, d_8BC50 table line, asm endlabel line,
#        census csv line, named_syms RESET line or None, coverage.tsv line)
FN = {
    "0x80064ED8": ("func_80064ED8", 5, "init", 1, 12, 217, 281, 20, 843, 691, 851),
    "0x80064F20": ("func_80064F20", 6, "init", 2, 14, 219, 283, 20, 844, 692, 852),
    "0x80064F68": ("func_80064F68", 9, "init", 3, 20, 225, 289, 21, 845, 693, 853),
    "0x80064FB4": ("func_80064FB4", 0xA, "init", 4, 22, 227, 291, 21, 846, 694, 854),
    "0x80065344": ("func_80065344", 5, "draw", 1, 13, 218, 282, 23, 857, None, 865),
    "0x80065394": ("func_80065394", 6, "draw", 2, 15, 220, 284, 23, 858, None, 866),
    "0x800653E4": ("func_800653E4", 9, "draw", 3, 21, 226, 290, 23, 859, 1744, 867),
    "0x80065434": ("func_80065434", 0xA, "draw", 4, 23, 228, 292, 23, 860, None, 868),
}
CMD = {
    5: dict(noun="SpinningQuadWithDepthSelectedTexture", nth="FirstSlot", first=True, pair="5/6", mode=1,
            il="status[5] when clear at 51268.c:412-414, word 0x210005; PaleRed issuer :404-425; effect event 1 at 17AFC.c:5044-5045",
            idx=5),
    6: dict(noun="SpinningQuadWithDepthSelectedTexture", nth="SecondSlot", first=False, pair="5/6", mode=2,
            il="status[6] only while status[5] is set and [6] clear at 51268.c:415-417, word 0x210006; PaleRed issuer :404-425; "
               "effect event 1 at 17AFC.c:5044-5045", idx=6),
    9: dict(noun="PreseededGrowingQuad", nth="FirstSlot", first=True, pair="9/0xA", mode=3,
            il="status[9] when clear at 51268.c:333-335, word 0x210009; Rose issuer :325-346; effect event 3 at 17AFC.c:5035-5036",
            idx=9),
    0xA: dict(noun="PreseededGrowingQuad", nth="SecondSlot", first=False, pair="9/0xA", mode=4,
              il="status[10] only while status[9] is set and [10] clear at 51268.c:336-338, word 0x21000A; Rose issuer :325-346; "
                 "effect event 3 at 17AFC.c:5035-5036", idx=10),
}

_DISPATCH = ("dispatch: eff_StartCommand (51268.c:84-115) clears g_eff_command_phase[command_word.h] and calls "
             "g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase], storing the result in status_dest; eff_StepCommand "
             "(:117-144) repeats the call once per step with the stored phase; tables d_8BC50.c:200-203 (bases) and :268-330 "
             "(handlers, index 0 at :269). Lifecycle by the code: phase 0 = the Init (record set-up, phase := 1), then the phase 1 draw "
             "every frame until its counter test returns 0, which zeroes the status and frees the issuer's slot")

_SPIN = ("mode 1/2 geometry arm (51268.c:2342-2385, `case 1: case 2: case 16: case 17:`): p_v = (0, 0, counter) feeds RotMatrix (:2359), "
         "a rotation about Z by the counter (4096 per turn); the four corners (+-w, +-h) go through ApplyRotMatrix (:2375) and are "
         "added to the projected centre (:2378-2385), so the quad turns in the screen plane; the counter runs 0, 0x1C6, ... 0x11BC = "
         "4540 (11 draws), about 1.1 turns")
_TEX = ("mode 1/2 texture arm (51268.c:2163-2172): `*g_eff_depth_work > 4000` picks D_8009B8D0 else D_8009B8D8, the depth being the "
        "projected z of gte_RotTransPers (:2153-2154, shifted left 2). No other mode's texture arm reads the depth: mode 0 fixed "
        "D_8009B8C8 (:2198), 3/4 test their counter (:2174-2179), 5 tests counters[4] (:2188), 6/7 and 10/11 test view_setting, "
        "8/9 and 12-15 their counters, 16/17 use the fixed D_8009B8C8 (:2289-2291)")
_SEED = ("every other animated-quad Init stores 0: eff_InitCosineScaledQuad :1783, func_80064ED8 :1791, func_80064F20 :1799, then "
         ":1825, 1836, 1844, 1852, 1860, 1867, 1872, 1879, 1884, 1892, 1900 (grep `g_eff_animated_quad_counters[N] = `); only "
         "records 3/4 store 0x40 (:1807 / :1816)")
_GROW = ("mode 3/4 size arm (51268.c:2411-2416): w and h are multiplied by rsin(counter & 0xFFF) >> 11 on a 0xC0 base (:2182-2183); "
         "the counter is drawn at 0x40, 0x59, 0x72, 0x8B, 0xA4, 0xBD (6 draws; the draw at 0xBD takes it to 0xD6 >= 0xC9 and returns 0, "
         ":1941-1951), all below the sine's 0x400 peak, so the factor rises at every draw; the quad is centred (x +-w, y +-h/2, "
         ":2417-2426)")
_ISSUER = ("both issuers: both statuses set -> both cleared; first status clear -> first command; else second status clear -> second "
           "command (a clear second slot is only used while the first is live)")


def _evidence(addr):
    old, cmd, kind, rec, hidx, proto, tbl, endl, csvl, nsl, cov = FN[addr]
    c = CMD[cmd]
    init = kind == "init"
    pre = "eff_Init" if init else "eff_DrawAndAdvance"
    nm = pre + c["noun"] + c["nth"]
    idx, mode = c["idx"], c["mode"]
    spin = c["noun"].startswith("Spinning")
    base = 0x0C if cmd == 5 else 0x0E if cmd == 6 else 0x14 if cmd == 9 else 0x16
    if init:
        body = (f"g_eff_animated_quad_positions[{rec}] = g_eff_command_position[0..2]; g_eff_command_phase[{idx}] = 1; "
                f"g_eff_animated_quad_counters[{rec}] = {'0' if spin else '0x40'}"
                + ("; void, v0 left 1 by the phase store (asm/funcs/%s.s)" % old if spin else "; return 1") + "; no calls")
    else:
        step = "0x1C6" if spin else "0x19"
        lim = "0x11C8" if spin else "0xC9"
        body = (f"draw_result = eff_DrawAnimatedQuads({mode}) (always 1, 51268.c:2518); g_eff_animated_quad_counters[{rec}] += {step}; "
                f"returns draw_result while (s16)counter < {lim}, else 0 (a 0 return is stored as status and ends the command)")
    partner_idx = hidx + 1 if init else hidx - 1
    callers = (f"none by name: the one use is the address-taken table slot g_eff_command_handlers[{hidx}] = "
               f"g_eff_command_handler_bases[{idx}] (0x{base:02X}, d_8BC50.c:200-203) + phase {0 if init else 1} "
               f"(src/main/d_8BC50.c:{tbl}; prototype :{proto}); no jal and no other reference in src/, include/ or asm/ (git grep -w "
               f"{old}): the other occurrences are the C definition in src/main/51268.c, the asm glabel/endlabel asm/funcs/{old}.s:1,{endl}, "
               f"the census docs/naming/function-names.csv:{csvl}, "
               + (f"the named_syms.txt:{nsl} RESET note (retired motion_ex_ name, address-keyed), " if nsl else "")
               + "the memory/grind ledger, docs/naming/apiscan/evidence.json, docs/naming/phase3/no_name.tsv, tools/trace/scenarios/"
               "versus-2p-kimigaura-effects.json and recorded runtime data. Command %d is issued only by %s (%s), whose only caller is "
               "fight_RecordReplayEventAndDispatchSoundOrEffect (17AFC.c, asm jal). Partner handler: g_eff_command_handlers[%d]. "
               % (cmd, "eff_StartRecycledCommandWithPaleRedLight" if cmd in (5, 6) else "eff_StartRecycledCommandWithRoseLight",
                  c["il"], partner_idx) + _DISPATCH)
    if init:
        if spin:
            word = (f"Init = the record set-up (position copy, phase := 1, counter := 0), as eff_InitCosineScaledQuad (record 0) and the "
                    f"landed Inits; SpinningQuad = the partner draw's {_SPIN}; WithDepthSelectedTexture = the partner draw's {_TEX}")
        else:
            word = (f"Init = the record set-up, as the landed Inits; Preseeded = this body's own counter seed 0x40, unique to records 3/4 "
                    f"({_SEED}); GrowingQuad = the partner draw's {_GROW}. Without the seed the first draw at counter 0 would have zero size")
    else:
        if spin:
            word = (f"DrawAndAdvance = it draws then steps the counter, as eff_DrawAndAdvanceCosineScaledQuad; SpinningQuad = {_SPIN}; "
                    f"WithDepthSelectedTexture = {_TEX}. The spinning arm is shared only with modes 16/17 (commands 18/19, still unnamed), "
                    "which use the fixed texture, so the texture word is what separates commands 5/6 from every other command")
        else:
            word = (f"DrawAndAdvance as the siblings; GrowingQuad = {_GROW}; Preseeded = the 0x40 start its Init gives (only records 3/4, "
                    f"{_SEED}); modes 8/9 and 12-15 also scale by rsin but their counters start at 0")
    why = (f"{word}. {c['nth']} = {'first' if c['first'] else 'second'} free status byte of the issuer ({_ISSUER}). The noun of "
           "an Init is taken from its draw (and the other way round), the cross-noun route of wave104 / 107 / 108 and "
           "eff_InitCosineScaledQuad; no colour or visual word is used")
    counter = ("(1) The colour (FF,80,80 for modes 1/2, r FF / g = counter / b 60 for 3/4) modulates a CLUT texture (SetShadeTex(prim, 0) "
               ":2322) and is drawn additive (tpage 0x2E :2161, SetSemiTrans :2323), so it is not a seen colour and stays out; PaleRed / "
               "Rose are the issuers' light writes (:424 / :345), not acts of these handlers; ")
    if spin:
        counter += ("(2) 'Spinning' is about 1.1 turns per life, not an endless spin; the size envelope (width 1-cos, height sin over half a "
                    "turn, :2346-2353) is shared with 16/17 and left out; (3) the twins 5 and 6 (modes 1 and 2 share one `case 1: case 2:` "
                    "in both arms) differ only in record, phase and slot, so the slot word is the only separator; ")
    else:
        counter += ("(2) 'Growing' is the counter factor only: the base size also scales with perspective (ReadGeomScreen * 1000 / depth, "
                    ":2300-2308), so the on-screen size can shrink if the camera retreats; modes 1/2, 8/9 and 12-15 also scale up, so "
                    "'Preseeded' is what separates; fallback if the reviewer refuses 'Growing': NO_NAME these four with the reason "
                    "'the only unique code fact is the 0x40 seed'; (3) not 'SineScaled' (a substring of the landed CosineScaled names, "
                    "shared by many modes); mode 5 reads counters[4] for its own texture test (:2188), a read by another command, not a "
                    "use of this function; the twins 9 and 0xA differ only in record, phase and slot; ")
    counter += "(4) no role word: the matrix shows hits across versus, story and group battle"
    if cmd in (5, 9) and init:
        sub = ("docs/naming/runtime/versus-2p-kimigaura-anim-quad-stub-1/knockout.md (corroborates the partner draw only: stubbing it "
               "removes a local quad whose growth/shape agrees with the code), versus-2p-kimigaura-rose-slot2-poke-1 baseline report.md "
               "(Rose: 12 draw hits for 2 starts = 6 per life, the seeded life)" if cmd == 9 else
               "docs/naming/runtime/lib-versus-2p-kimigaura-effects-1/hitmap.tsv (reach)")
    elif cmd in (6, 0xA):
        sub = (f"coverage.tsv:{cov}: reach only" + ("; versus-2p-kimigaura-rose-slot2-poke-1/poked (issuer poked to the second slot: "
               "these handlers hit, output pixel-identical to the first slot; modified game, route only)" if cmd == 0xA else
               "; no knockout or poke covers command 6"))
    else:
        sub = ("versus-2p-kimigaura-anim-quad-stub-1 knockout.md (stubbing the draw removes only a local quad; both runs end)" if cmd == 5
               else "versus-2p-kimigaura-anim-quad-stub-1 knockout.md (func_800653E4 stub: a quad growing 480 to 5278 px over 6 samples) and "
                    "rose-slot2-poke-1 baseline (12 hits for 2 starts)")
    trace = (f"TRACE (Q124, corroborating only; read, never the source of a word): {sub}; docs/naming/runtime/matrix/coverage.tsv:{cov}")
    return nm, body, callers, why, counter, trace


_PRIOR = ("docs/naming/phase3/no_name.tsv rows 51-54 (Inits) and 61-64 (draws), rounds w58..w94: NO_NAME 'bodies differ only by "
          "record' / 'gates the draw on the counter; shared globals'. Answer: the record selects a draw mode, and the mode's own arms in "
          "eff_DrawAnimatedQuads (depth-selected texture, in-screen spin; 0x40 seed, rising sine size) separate each pair from every other "
          "command; the slot word separates the twins (wave104 / 107 / 108 route)")

FUNCS = []
for _addr in FN:
    _nm, _body, _callers, _why, _counter, _trace = _evidence(_addr)
    _ev = (f"BODY: {B(FN[_addr][0], '51268.c')}: {_body}. CALLERS: {_callers}. WHY: {_why}. COUNTER-EVIDENCE: {_counter}. "
           f"Prior manifests: {_PRIOR}. {_trace}")
    FUNCS.append((_addr, _nm, "RENAME", _ev))

DATA = []
