"""Tempo-step spelling variants of _SsSndTempo on top of v6c.c -> tmp/laneF/T*.c"""
s = open("tmp/laneF/v6c.c", encoding="utf-8", newline="").read()
OLD = """        u32 new_val;
        if ((score->unkA8 % score->unk4E) != 0) {
            return;
        }
        if (score->unk94 > score->unkAC) {
            new_val = score->unk94 - 1;
            goto tempo_store;
        }
        if (score->unk94 < score->unkAC) {
            new_val = score->unk94 + 1;
        tempo_store:
            score->unk94 = new_val;
        }
"""
assert s.count(OLD) == 1
HEAD = """        if ((score->unkA8 % score->unk4E) != 0) {
            return;
        }
"""
V = {
 "T1": HEAD + """        if (score->unk94 > score->unkAC) {
            score->unk94--;
        } else if (score->unk94 < score->unkAC) {
            score->unk94++;
        }
""",
 "T3": HEAD + """        if (score->unk94 != score->unkAC) {
            score->unk94 = (score->unk94 > score->unkAC) ? score->unk94 - 1 : score->unk94 + 1;
        }
""",
 "T5": HEAD + """        {
            u32 tempo = score->unk94;
            u32 target = score->unkAC;
            if (tempo > target) {
                score->unk94 = tempo - 1;
            } else if (tempo < target) {
                score->unk94 = tempo + 1;
            }
        }
""",
 "T8": HEAD + """        if (score->unk94 < score->unkAC) {
            score->unk94++;
        } else if (score->unk94 > score->unkAC) {
            score->unk94--;
        }
""",
 "T9": HEAD + """        if (score->unk94 > score->unkAC || score->unk94 < score->unkAC) {
            score->unk94 = (score->unk94 > score->unkAC) ? score->unk94 - 1 : score->unk94 + 1;
        }
""",
 "T12": """        s32 step;
""" + HEAD + """        if (score->unk94 > score->unkAC) {
            step = -1;
        } else if (score->unk94 < score->unkAC) {
            step = 1;
        } else {
            step = 0;
        }
        if (step != 0) {
            score->unk94 += step;
        }
""",
}
for k, body in V.items():
    t = s.replace(OLD, body)
    if k == "T11":
        t = t.replace("""    score->unk54 = (score->unk50 * score->unk94 * 10) / (VBLANK_MINUS * 60);""",
                      """recalc:
    score->unk54 = (score->unk50 * score->unk94 * 10) / (VBLANK_MINUS * 60);""")
    open(f"tmp/laneF/{k}.c", "w", encoding="utf-8", newline="").write(t)
print(" ".join(f"tmp/laneF/{k}.c" for k in V))
