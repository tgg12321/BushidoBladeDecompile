NL = "\n"
s = open("tmp/rev-ssscore/v5.c", encoding="utf-8", newline="").read()


def mk(name, reps):
    a = s
    for o, n in reps:
        assert a.count(o) == 1, (name, a.count(o), o[:70])
        a = a.replace(o, n)
    open(f"tmp/laneF/{name}.c", "w", encoding="utf-8", newline="").write(a)


DATA_TOP = (NL + "  u32 data;" + NL, NL)
NEXT_TOP = (NL + "  u8 next;" + NL, NL)
ARM1 = "      case 0x90:" + NL + "      {" + NL + "        u8 *cmd_ptr;" + NL
CP = "        u8 *cp = state->read_pos;" + NL
# only data per-arm
mk("v6c", [DATA_TOP, (ARM1, ARM1 + "        u32 data;" + NL)])
# next per-arm in the second switch only (first arm keeps the function-level next)
mk("v6d", [(CP, CP + "        u8 next;" + NL)])
# next per-arm in the first switch only
mk("v6e", [(ARM1, ARM1 + "        u8 next;" + NL)])
# second-switch arm: no named next at all, read through cp[0] at the call
mk("v6f", [(CP + "        state->read_pos = cp + 1;" + NL + "        next = cp[0];" + NL,
            CP + "        state->read_pos = cp + 1;" + NL)])
print("ok")
