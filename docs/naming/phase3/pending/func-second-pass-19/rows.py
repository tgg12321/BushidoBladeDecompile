"""wave111 rows (func-second-pass-19): the eff_ animated-quad command pair 0x12/0x13 (records 16/17, eff_DrawAnimatedQuads modes 16/17).
Init handlers func_80065264 / func_800652AC (commands 0x12 / 0x13) and draw handlers func_80065760 / func_800657B0 (commands 0x12 / 0x13)
(src/main/51268.c:1886-1899 and :2069-2091; d_8BC50.c:244-247 prototypes, handlers 38..41 at :308-311; bases[18] = 0x26, [19] = 0x28 at
:203). Research input tmp/codex/func-second-pass-19-research.md (untrusted; every claim re-derived at HEAD f43a8a0ea, see notes.txt).
Words: SpinningQuad (mode 16/17 geometry arm 51268.c:2341-2385, `case 1: case 2: case 16: case 17:`: RotMatrix about Z by the counter,
corners rotated and added to the projected centre, shared only with modes 1/2), WithFixedTexture (mode 16/17 texture arm :2288-2294:
g_eff_current_tex_rec = D_8009B8C8 unconditionally; the other spinners, modes 1/2, select D_8009B8D0 / D_8009B8D8 by depth :2162-2171;
the word alone is shared with mode 0 :2196-2197, but mode 0 does not spin, so only the whole noun is unique to 16/17), FirstSlot /
SecondSlot (the issuer's status byte: command 0x12 first, 0x13 second, eff_StartRecycledCommandWithMagentaLight :686-707). Verdicts
PENDING (the reviewer fills them). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = 'naming-reviewer pre-apply 2026-10-31 (naming-a, HEAD f43a8a0ea): PASS 4/4 on code alone. SpinningQuad = RotMatrix about Z on the counter, arm `case 1/2/16/17` (51268.c:2341-2385), shared only with modes 1/2; WithFixedTexture = modes 16/17 set D_8009B8C8 unconditionally (:2288-2294), false of 1/2 (depth-selected :2162-2171); modes 0/5/12-15 that use D_8009B8C8 do not rotate. FirstSlot/SecondSlot = issuer status[18]/[19] order (:694-699). Knockout 1ff099887 corroboration only; 0x13 pair in no capture, names rest on code.'
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

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
    "0x80065264": ("func_80065264", 0x12, "init", 16, 38, 244, 308, 20, 854, 363, 862),
    "0x800652AC": ("func_800652AC", 0x13, "init", 17, 40, 246, 310, 20, 855, None, 863),
    "0x80065760": ("func_80065760", 0x12, "draw", 16, 39, 245, 309, 23, 868, None, 876),
    "0x800657B0": ("func_800657B0", 0x13, "draw", 17, 41, 247, 311, 23, 869, 1747, 877),
}
CMD = {
    0x12: dict(nth="FirstSlot", first=True, mode=16, idx=18, base=0x26,
               il="status[18] when clear at 51268.c:694-696, word 0x210012"),
    0x13: dict(nth="SecondSlot", first=False, mode=17, idx=19, base=0x28,
               il="status[19] only while status[18] is set and [19] clear at 51268.c:697-699, word 0x210013"),
}
NOUN = "SpinningQuadWithFixedTexture"

_DISPATCH = ("dispatch: eff_StartCommand (51268.c:84-115) clears g_eff_command_phase[command_word.h] and calls "
             "g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase], storing the result in status_dest; eff_StepCommand "
             "(:117-144) repeats the call once per step with the stored phase; tables d_8BC50.c:201-204 (bases) and :269-330 "
             "(handlers, index 0 at :270). Lifecycle by the code: phase 0 = the Init (record set-up, phase := 1), then the phase 1 draw "
             "every frame until its counter test returns 0, which zeroes the status and frees the issuer's slot")

_SPIN = ("mode 1/2/16/17 geometry arm (51268.c:2341-2385, `case 1: case 2: case 16: case 17:`): p_v = (0, 0, counter) feeds RotMatrix "
         "(:2358), a rotation about Z by the counter (4096 per turn); the four corners (+-w, +-h) go through ApplyRotMatrix (:2374) and "
         "are added to the projected centre (:2377-2384), so the quad turns in the screen plane; the counter runs 0, 0x1C6, ... 0x11BC = "
         "4540 (11 draws), about 1.1 turns. The arm is shared only with modes 1/2 (commands 5/6, wave110)")
_TEX = ("mode 16/17 texture arm (51268.c:2288-2294): g_eff_current_tex_rec = D_8009B8C8 unconditionally, no counter, depth or view test. "
        "The only other spinners, modes 1/2, pick D_8009B8D0 / D_8009B8D8 by `*g_eff_depth_work > 4000` (:2162-2171), so the texture "
        "word is what separates 0x12/0x13 from 5/6. The word alone is not unique: mode 0 takes D_8009B8C8 unconditionally (:2196-2197), "
        "mode 5 while counters[4] <= 0x800 (:2186-2191), modes 12-15 once their counter >= 8 (:2273); none of those spins, so only the "
        "whole noun SpinningQuadWithFixedTexture is true of modes 16/17 alone (the func-second-pass-13 lesson, phase3/backlog.tsv:98)")
_ISSUER = ("eff_StartRecycledCommandWithMagentaLight (51268.c:686-707): both statuses set -> both cleared (:690-693); first status "
           "clear -> first command; else second status clear -> second command (a clear second slot is only used while the first is live)")


def _evidence(addr):
    old, cmd, kind, rec, hidx, proto, tbl, endl, csvl, nsl, cov = FN[addr]
    c = CMD[cmd]
    init = kind == "init"
    pre = "eff_Init" if init else "eff_DrawAndAdvance"
    nm = pre + NOUN + c["nth"]
    idx, mode, base = c["idx"], c["mode"], c["base"]
    if init:
        body = (f"g_eff_animated_quad_positions[{rec}] = g_eff_command_position[0..2]; g_eff_command_phase[{idx}] = 1; "
                f"g_eff_animated_quad_counters[{rec}] = 0; void, v0 left 1 by the phase store (asm/funcs/{old}.s); no calls")
    else:
        body = (f"draw_result = eff_DrawAnimatedQuads(0x{mode:X}) (always 1, 51268.c:2517); g_eff_animated_quad_counters[{rec}] += "
                f"0x1C6; returns draw_result while (s16)counter < 0x11C8, else 0 (a 0 return is stored as status and ends the command)")
    partner_idx = hidx + 1 if init else hidx - 1
    callers = (f"none by name: the one use is the address-taken table slot g_eff_command_handlers[{hidx}] = "
               f"g_eff_command_handler_bases[{idx}] (0x{base:02X}, d_8BC50.c:203) + phase {0 if init else 1} "
               f"(src/main/d_8BC50.c:{tbl}; prototype :{proto}); no jal and no other reference in src/, include/ or asm/ (git grep -w "
               f"{old}): the other occurrences are the C definition in src/main/51268.c, the asm glabel/endlabel asm/funcs/{old}.s:1,{endl}, "
               f"the census docs/naming/function-names.csv:{csvl}, "
               + (f"the named_syms.txt:{nsl} RESET note (retired motion_ name, address-keyed), " if nsl else "")
               + "the memory/grind ledger, docs/naming/apiscan/evidence.json, docs/naming/phase3/no_name.tsv and recorded runtime data. "
               + (f"Command 0x{cmd:X} is issued only by eff_StartRecycledCommandWithMagentaLight ({c['il']}); its one caller is "
                  "fight_RecordReplayEventAndDispatchSoundOrEffect (17AFC.c:5099-5100 effect event 18, asm jal). ")
               + f"Partner handler: g_eff_command_handlers[{partner_idx}]. " + _DISPATCH)
    if init:
        word = (f"Init = the record set-up (position copy, phase := 1, counter := 0), as eff_InitCosineScaledQuad (record 0) and the "
                f"landed Inits; SpinningQuad = the partner draw's {_SPIN}; WithFixedTexture = the partner draw's {_TEX}")
    else:
        word = (f"DrawAndAdvance = it draws then steps the counter, as eff_DrawAndAdvanceCosineScaledQuad; SpinningQuad = {_SPIN}; "
                f"WithFixedTexture = {_TEX}")
    why = (f"{word}. {c['nth']} = {'first' if c['first'] else 'second'} free status byte of the issuer ({_ISSUER}). The noun of "
           "an Init is taken from its draw (and the other way round), the cross-noun route of wave104 / 107 / 108 / 110 and "
           "eff_InitCosineScaledQuad; no colour or visual word is used")
    counter = ("(1) The vertex colour (FF,60,FF for modes 16/17, FF,80,80 for 1/2) modulates a CLUT texture (SetShadeTex(prim, 0) "
               ":2321) and is drawn additive (tpage 0x2E :2160, SetSemiTrans :2322), so it is not a seen colour and stays out; Magenta "
               "is the issuer's light write (0xFF00FF, :705), not an act of these handlers; (2) 'Spinning' is about 1.1 turns per life, "
               "not an endless spin; the size envelope (width 1-cos, height sin, :2345-2352) and the counter step/limit are identical to "
               "modes 1/2 and left out; (3) 'FixedTexture' alone is shared with mode 0 (and conditionally modes 5, 12-15), which is why "
               "it is only used inside the whole noun; (4) the twins 0x12 and 0x13 (modes 16 and 17 share one `case 16: case 17:` in "
               "both arms) differ only in record, phase and slot, so the slot word is the only separator; (5) no role word: the "
               "only_x rows for the first-slot pair are menu:stage_card and mode:story, both inconclusive = 1")
    if cmd == 0x12 and init:
        sub = (f"coverage.tsv:{cov}: reach in 2 final-fight captures (lib-story-final-fight-retail-1, story-final-fight-stages-1); "
               "knockout story-final-fight-magenta-stub-1 corroborates the partner draw only")
    elif cmd == 0x12:
        sub = (f"coverage.tsv:{cov}: reach in the same 2 captures; docs/naming/runtime/story-final-fight-magenta-stub-1/knockout.md + "
               "knockout.tsv (commit 1ff099887; stubbing this draw from final_fight frame 153700 removes a local quad: "
               "knockout.154568-154580 growing 15 -> 2095 px, 154584 a thin wide box 258,82-449,99, 154590 0 px; repeats at "
               "154694-154706 and 155008-155020; every other screenshot 0 px; the lost picture is this function's output, and the "
               "window and shape agree with the code's envelope; corroboration only, Q124)")
    else:
        sub = (f"coverage.tsv:{cov}: 0 captures, only_x 0 rows; no poke covers command 0x13 (the magenta knockout stubs only the first "
               "slot, and a stubbed 0x12 returns status 0 at once so every later event 18 also takes slot 0x12). The name rests on the "
               "code, which is reachable (whenever a second event 18 arrives within the 11-draw life of command 0x12)")
    trace = f"TRACE (Q124, corroborating only; read, never the source of a word): {sub}"
    return nm, body, callers, why, counter, trace


_PRIOR = ("docs/naming/phase3/no_name.tsv rows 55-58, rounds w58..w94: NO_NAME 'bodies differ only by record / mode index' and "
          "'gates the draw on the counter; shared globals'. Answer: the record selects a draw mode, and the mode's own arms in "
          "eff_DrawAnimatedQuads (in-screen spin shared only with modes 1/2, fixed texture absent from 1/2) separate the pair from every "
          "other command; the slot word separates the twins (wave104 / 107 / 108 / 110 route)")

FUNCS = []
for _addr in FN:
    _nm, _body, _callers, _why, _counter, _trace = _evidence(_addr)
    _ev = (f"BODY: {B(FN[_addr][0], '51268.c')}: {_body}. CALLERS: {_callers}. WHY: {_why}. COUNTER-EVIDENCE: {_counter}. "
           f"Prior manifests: {_PRIOR}. {_trace}")
    FUNCS.append((_addr, _nm, "RENAME", _ev))

DATA = []
