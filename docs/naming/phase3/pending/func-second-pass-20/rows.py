"""wave112 rows (func-second-pass-20): the eff_ animated-quad Init pair for commands 0x10 / 0x11 (records 12+14 / 13+15).
Init handlers func_8006517C / func_800651F0 (src/main/51268.c:1862-1872 and :1874-1884; d_8BC50.c:240/:242 prototypes, handlers 34 / 36 at
:304 / :306; bases[16] = 0x22, [17] = 0x24 at :203). Research input tmp/codex/func-second-pass-20-research.md (untrusted; every claim
re-derived at HEAD d522e55e5, see notes.txt). Words: Init (record set-up: position copy, phase := 1, counters := 0), TwoAnimatedQuads
(the noun of the landed draws eff_DrawAndAdvanceTwoAnimatedQuadsUntil*CounterExpires: each of these Inits seeds TWO position records and TWO
counters, which no other animated-quad Init does; the one-record QuadPair Inits 6/7 are named otherwise), FirstSlot / SecondSlot (the issuer's
status byte: command 0x10 first, 0x11 second, eff_StartRecycledCommandWithOrangeLight 51268.c:479-500). No Until-clause (the Inits test no
counter, the no_name.tsv:53-54 objection). Verdicts PENDING (the reviewer fills them). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = 'naming-reviewer pre-apply 2026-10-31 (naming-b, HEAD d522e55e5): PASS 2/2 on code alone. TwoAnimatedQuads = the only Inits seeding two animated-quad records and two counters (12/14, 13/15; every other Init seeds one, QuadPair 6/7 included, 51268.c:1777-1900); partner draws call eff_DrawAnimatedQuads twice. FirstSlot/SecondSlot = issuer eff_StartRecycledCommandWithOrangeLight status[16]/[17] if/else-if (51268.c:487-492); 0x210010/0x210011 written nowhere else. Knockout 1b400bf88 and poke bc3f1927e corroboration only.'
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

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


# addr: (old, cmd, records, handler idx, d_8BC50 proto line, d_8BC50 table line, asm endlabel line, census csv line, coverage.tsv line)
FN = {
    "0x8006517C": ("func_8006517C", 0x10, (12, 14), 34, 240, 304, 31, 852, 860),
    "0x800651F0": ("func_800651F0", 0x11, (13, 15), 36, 242, 306, 31, 853, 861),
}
CMD = {
    0x10: dict(nth="FirstSlot", first=True, idx=16, base=0x22,
               il="status[16] when clear at 51268.c:487-489, word 0x210010"),
    0x11: dict(nth="SecondSlot", first=False, idx=17, base=0x24,
               il="status[17] only while status[16] is set and [17] clear at 51268.c:490-492, word 0x210011"),
}
NOUN = "TwoAnimatedQuads"
_PARTNER = {0x10: "eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires (0x80065680, 51268.c:2041-2054; draws modes 12 and 14, "
                  "returns 0 once counters[14] >= 11)",
            0x11: "eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires (0x800656EC, 51268.c:2056-2067; draws modes 13 and 15, "
                  "returns 0 once counters[13] >= 11)"}

_DISPATCH = ("dispatch: eff_StartCommand (51268.c:84-115) clears g_eff_command_phase[command_word.h] (:89) and calls "
             "g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase] (:105-109), storing the result in status_dest; "
             "eff_StepCommand (:117-144) repeats the call once per step with the stored phase; tables d_8BC50.c:201-204 (bases) and "
             ":269-330 (handlers, index 0 at :270). Lifecycle by the code: phase 0 = this Init (record set-up, phase := 1), then the "
             "phase 1 draw every step until its counter test returns 0, which zeroes the status and frees the issuer's slot")
_ISSUER = ("eff_StartRecycledCommandWithOrangeLight (51268.c:479-500): both statuses set -> both cleared (:483-486); status[16] clear "
           "-> 0x210010 into [16] (:487-489); else status[17] clear -> 0x210011 into [17] (:490-492); a clear second slot is only used "
           "while the first is live. Its callers: fight_RecordReplayEventAndDispatchSoundOrEffect event 4 (17AFC.c:5023-5031) when "
           "g_fight_mode_selector == 2 && D_800A389A == 0, or == 5; else eff_StartRecycledVectorCommandWithRedLight")
_TWO = ("it seeds TWO g_eff_animated_quad_positions records and TWO g_eff_animated_quad_counters (12+14 / 13+15, 51268.c:1862-1884) and the "
        "draw uses them as two separate quads (eff_DrawAnimatedQuads projects each mode's own record, :2136-2152, before `again:` "
        ":2155). Every other animated-quad Init seeds exactly one record (0,1,2,3,4,5,6,7,10,11,16,17; :1777-1900). The only other "
        "two-quad commands, 6/7, draw the second quad from the SAME record by `goto again` (mode 8/9 re-run), so the noun is true of "
        "exactly these two Inits")


def _evidence(addr):
    old, cmd, recs, hidx, proto, tbl, endl, csvl, cov = FN[addr]
    c = CMD[cmd]
    a, b = recs
    idx, base = c["idx"], c["base"]
    nm = "eff_Init" + NOUN + c["nth"]
    body = (f"g_eff_animated_quad_positions[{a}] = g_eff_command_position[0..2]; g_eff_animated_quad_counters[{a}] = 0; "
            f"g_eff_animated_quad_positions[{b}] = the same position; g_eff_command_phase[{idx}] = 1; "
            f"g_eff_animated_quad_counters[{b}] = 0; void, v0 left 1 by the phase store (asm/funcs/{old}.s), so status[{idx}] = 1; no calls")
    callers = (f"none by name: the one use is the address-taken table slot g_eff_command_handlers[{hidx}] = "
               f"g_eff_command_handler_bases[{idx}] (0x{base:02X}, d_8BC50.c:203) + phase 0 "
               f"(src/main/d_8BC50.c:{tbl}; prototype :{proto}); no jal and no other reference in src/, include/ or asm/ (git grep -w "
               f"{old}): the other occurrences are the C definition in src/main/51268.c, the asm glabel/endlabel asm/funcs/{old}.s:1,{endl}, "
               f"the census docs/naming/function-names.csv:{csvl}, the memory/grind ledger and _completed files, "
               + ("the rule .claude/rules/walking-pointer-serializes-parallel-loads.md:49 (a codegen example citing the name), " if cmd == 0x10 else "")
               + "docs/naming/apiscan/evidence.json, docs/naming/phase3/no_name.tsv:" + ("53" if cmd == 0x10 else "54")
               + " and recorded runtime data. "
               + f"Command 0x{cmd:X} is issued only by eff_StartRecycledCommandWithOrangeLight ({c['il']}). "
               + f"Partner draw (phase 1): {_PARTNER[cmd]}. " + _DISPATCH)
    why = (f"Init = the record set-up (position copy, phase := 1, counters := 0), as eff_InitCosineScaledQuad and the landed Inits; "
           f"TwoAnimatedQuads = the noun of the landed partner draw and true of this body: {_TWO}; {c['nth']} = "
           f"{'first' if c['first'] else 'second'} free status byte of the issuer ({_ISSUER}). The Until...CounterExpires clause of the "
           "draws is not borrowed: the Inits test no counter and both draws' lives are equal (counters start 0, +1 each, 11 steps). "
           "No colour or visual word is used")
    counter = ("(1) The Init draws nothing; its noun comes from its command's draw (cross-noun route of wave104/107/108/110 and "
               "eff_InitCosineScaledQuad); (2) the twins differ only in records, phase and slot (modes 12-15 share every arm of "
               "eff_DrawAnimatedQuads: colour/texture :2263-2266, size :2386-2389), so the slot word is the only separator and is "
               "required (without it both would share one name); (3) 'QuadPair' is taken by the 0x0C/0x0D family (one record, second "
               "quad by re-run); (4) 'Recycled' / 'Orange' / 'Light' belong to the issuer (point-light write 0xC06013, :499); "
               "'Burst' / 'Hit' / colour words are not in the code and only_x has 0 rows; (5) the w94 no_name objection (the Inits differ "
               "only by record index, test no counter) is answered by the two-record noun plus the issuer slot order")
    if cmd == 0x10:
        sub = (f"coverage.tsv:{cov}: reach in 4 captures (group-battle-hitmap-1, group-battle-shainto-wins-1, lib-group-battle-3-0-1 "
               "bouts 1-3, training-practice-ko-1 f12000/f14000); knockout training-practice-ko-orange-stub-1 (commit 1b400bf88) "
               "stubs only the partner draw (0x80065680): diffs are local boxes growing at frames 12320-12336 and 14598-14614, every "
               "other screenshot 0 px, which is the draw's output; for this Init it adds only reach")
    else:
        sub = (f"coverage.tsv:{cov}: 0 captures in unmodified runs, only_x 0 rows. Set-and-observe poke bc3f1927e "
               "(docs/naming/runtime/group-battle-orange-slot2-poke-1/slot17, a MODIFIED game) routes one start into status[17]: "
               "this Init hit once (from eff_StartCommand+0xBC) and the First-counter draw 11 times, 0 changed pixels against the 0x10 "
               "baseline; it shows the route and that the picture matches, not that retail reaches 0x11. The name rests on the code, "
               "which is live whenever a second event 4 arrives in mode 2/5 within the 11-step life of command 0x10")
    trace = f"TRACE (Q124, corroborating only; read, never the source of a word): {sub}"
    return nm, body, callers, why, counter, trace


_PRIOR = ("docs/naming/phase3/no_name.tsv rows 53-54, rounds w58..w94: NO_NAME 'records 12 and 14 copy ... twin makes the same writes on "
          "another record index' and 'the initializers test neither counter, so the partners' distinction does not carry over'. Answer: "
          "the Until-clause is dropped; the noun TwoAnimatedQuads is true of these two Inits alone (two records, two counters) and the "
          "slot word separates the twins (wave104 / 107 / 108 / 110 route)")

FUNCS = []
for _addr in FN:
    _nm, _body, _callers, _why, _counter, _trace = _evidence(_addr)
    _ev = (f"BODY: {B(FN[_addr][0], '51268.c')}: {_body}. CALLERS: {_callers}. WHY: {_why}. COUNTER-EVIDENCE: {_counter}. "
           f"Prior manifests: {_PRIOR}. {_trace}")
    FUNCS.append((_addr, _nm, "RENAME", _ev))

DATA = []
