"""Generate the proof bodies from ONE base (v/cand.c = the landing body)."""
import os
D = os.path.dirname(os.path.abspath(__file__)) + '/v/'
cand = open(D + 'cand.c').read()
COMMENT_START = "    /* tbl, tbl_arg:"
DECL_T = "    Tbl8008E194 *tbl;     /* the record, read field by field */\n"
DECL_A = "    Tbl8008E194 *tbl_arg; /* the record, passed on to func_800278C0 */\n"
INIT_T = "    tbl = rec;\n"
INIT_A = "    tbl_arg = rec;\n"
CALL_A = "limb, (s32)tbl_arg, scr"
for s in (DECL_T, DECL_A, INIT_T, INIT_A, COMMENT_START):
    assert cand.count(s) == 1, s
assert cand.count(CALL_A) == 3
COMMENT = cand[cand.index(COMMENT_START):cand.index(DECL_T)]
out = {}
# A. no field-read copy: the field reads and the NULL test read the parameter
out['no_tbl'] = cand.replace(DECL_T, '').replace(INIT_T, '').replace('tbl->', 'rec->').replace('tbl == NULL', 'rec == NULL')
# B. no argument copy: the calls pass tbl
out['no_arg'] = cand.replace(DECL_A, '').replace(INIT_A, '').replace(CALL_A, 'limb, (s32)tbl, scr')
# B'. no argument copy: the calls pass the parameter
out['no_arg_rec'] = cand.replace(DECL_A, '').replace(INIT_A, '').replace(CALL_A, 'limb, (s32)rec, scr')
# neither copy (== memory/grind/func_80027AD8/candidate.c modulo the comment)
out['neither'] = out['no_tbl'].replace(COMMENT, '').replace(DECL_A, '').replace(INIT_A, '').replace(CALL_A, 'limb, (s32)rec, scr')
# sanctioned FAKE families applied to the copy-free spellings
SA = "    rec = rec; /* FAKE: dead param self-assign */\n"
out['no_tbl_selfassign'] = out['no_tbl'].replace(INIT_A, INIT_A + SA)
out['no_arg_rec_selfassign'] = out['no_arg_rec'].replace(INIT_T, INIT_T + SA)
out['neither_selfassign'] = out['neither'].replace("    scr = ", SA + "    scr = ", 1)
DEAD = "            same = ch[0xAF] == opp[0xAF];\n"
assert cand.count(DEAD) == 1
DS = "            rec = NULL; /* FAKE: dead param store (R6a returns) */\n"
out['no_tbl_deadstore'] = out['no_tbl'].replace(DEAD, DS + DEAD)
out['no_arg_rec_deadstore'] = out['no_arg_rec'].replace(DEAD, DS + DEAD)
out['neither_deadstore'] = out['neither'].replace(DEAD, DS + DEAD)
# chain-extender detour on a parameter read (tree fold removes it before RTL)
out['no_tbl_chain'] = out['no_tbl'].replace("rec->unkC == 0", "((Tbl8008E194 *)((u8 *)rec + 1 - 1))->unkC == 0")
# do { } while (0) around the remaining entry copy
out['no_tbl_dowhile'] = out['no_tbl'].replace(INIT_A, "    do {\n    " + INIT_A + "    } while (0);\n")
# the argument copy sunk to each call site (hoist/sink + duplicated-into-arms family)
CALL_STMT_I = "        func_800278C0(pass, (s32 *)ch, limb, (s32)tbl_arg, scr, arg6);\n"
CALL_STMT_O = "    func_800278C0(pass, (s32 *)ch, limb, (s32)tbl_arg, scr, arg6);\n"
out['arg_sink'] = cand.replace(INIT_A, '').replace(CALL_STMT_I, "        tbl_arg = tbl;\n" + CALL_STMT_I).replace(
    "\n" + CALL_STMT_O, "\n    tbl_arg = tbl;\n" + CALL_STMT_O)
for k, v in out.items():
    open(D + k + '.c', 'w', newline='\n').write(v)
    print(k, len(v))
