"""Reason for every body that reaches 0 (tmp/f187/manifest.py fails on any zero not listed here).
REUSE = the landing's reused locals are kept (not a one-variable-per-value spelling);
SET ASIDE = needs a FAKE-family construct the landing body does not carry (Q30, family-based)."""

R = "REUSE: "
S = "SET ASIDE (Q30): "
ZERO_REASONS = {
    "candidate.c": R + "the landing body",
    "template.c": R + "the landing template (= r11/variants_v3/c7.c)",
    "r11/variants_v3/c7.c": R + "the landing template c7",
    "r11/variants/c5.c": R + "v1 chassis of the landing code (c7 = c5 with comment and type-name changes only)",
    "r11/variants/c5a.c": R + "c5 with its comments stripped (code identical; the generator base of the v1 per-value spellings)",
    "r11/reviewer_probes/rv4_base.c": R + "the v4 reviewer's control (the landing template)",
    "r11/reviewer_probes/rv5_base.c": R + "the v5 reviewer's control (the landing template)",
    "r11/reviewer_probes/rv6_base.c": R + "the v6 reviewer's control (the landing template)",
    "r11/reviewer_probes/rv8_base.c": R + "the v8 reviewer's control (the landing template)",
    "rejected/v3_review/rv3_c7.c": R + "the v3 reviewer's base (the landing template c7; comments differ)",
    "r11/reviewer_probes/rv6_lz5.c": R + "frame probe: the landing body with lz[5] (Q32 twin)",
    "r11/reviewer_probes/rv6_lz6.c": R + "frame probe: the landing body with lz[6]",
    "r11/variants/lzc5_5.c": R + "frame probe: the v1 chassis with lz[5] (Q32 twin)",
    "r11/variants/r11pv_f.c": R + "the v1 chassis with `f_add`/`f_sub` moved to node-loop scope; `f` is not a Ruling 11 "
                              "local and all seven Ruling 11 locals stay shared",
    "r11/banked/pc_joined.c": R + "the v1-era landing template; in gen.py's default mode its markers expand to "
                              "header-exact statements, i.e. the landing code (pure-C attempt [2] is its --joined "
                              "expansion: 180 lines, below)",
    "rejected/r11-work-delta-reuse-layer2-fail-0.c": R + "the first landing (v1 code)",
    "rejected/r11-work-delta-reuse-layer2-fail-0.template.c": R + "the first landing's template (v1 code)",
    "superseded/candidate_g2.c": R + "an earlier spelling of the landing code under generic names (`j` for the three "
                                 "loop counters, `n` for both force counts, `d` for seven values)",
    "superseded/template_g2.c": R + "the template of superseded/candidate_g2.c (generic names; reuses kept)",
    "r11/banked/w1.c": R + "the delta-ablation chassis: the landing code under generic names (`j` x3 loops, `n` x2 "
                       "counts, `d` x7 values)",
    "r11/banked/w2.c": R + "w1 with the byte-neutral `dist2` split (mkabl.py); reuses kept",
    "r11/reviewer_probes/rv_both.c": S + "work and delta per-value + FAKE dead stores (dead-store family)",
    "r11/reviewer_probes/rv_wd_after2.c": S + "work per-value + FAKE dead store `sq1 = 0;` (dead-store family)",
    "r11/reviewer_probes/rv_wd_end.c": S + "work per-value + FAKE dead store `sq1 = 0;` (dead-store family)",
    "r11/variants/fam_delta_deadstore_end.c": S + "delta per-value + dead store after the ellipsoid loop (dead-store "
                                              "family, which requires the FAKE annotation; the probe omits it)",
    "r11/banked/rv3_nfdw_lz5.c": S + "lz[5] twin (Q32) of the six-wrap nforce closer: do-while(0) wraps + c6's two FAKE dead stores",
    "r11/banked/rv3_rv_both_lz5.c": S + "lz[5] twin (Q32) of rv_both: FAKE dead stores",
    "r11/banked/rv3_rv_wd_end_lz5.c": S + "lz[5] twin (Q32) of rv_wd_end: FAKE dead store",
    "rejected/v3_review/bodies/rv3_nfdw_lz5.c": S + "copy of r11/banked/rv3_nfdw_lz5.c",
    "rejected/v3_review/bodies/rv3_rv_both_lz5.c": S + "copy of r11/banked/rv3_rv_both_lz5.c",
    "rejected/v3_review/bodies/rv3_rv_wd_end_lz5.c": S + "copy of r11/banked/rv3_rv_wd_end_lz5.c",
    "r11/variants_v2/c6.c": S + "v2 chassis: work and delta per-value + two FAKE dead stores",
    "rejected/r11-nforce-dowhile-layer2-fail-0.c": S + "the v2 landing (c6: two FAKE dead stores)",
    "rejected/nforce-split-six-dowhile-wraps-closes-0.template.c": S + "nforce per-value on c6 + six FAKE do-while(0) "
                                                                   "wraps (and c6's two FAKE dead stores)",
}

RESULT_ZERO_REASONS = [
    ("rejected/v2_review/res_g.txt", r"nf4_[0-9_]+", S + "nforce per-value on the v2 chassis c6 (two FAKE dead stores) "
                                                   "+ a do-while(0) wrap combination"),
    ("rejected/v3_review/loop_res.txt", r"rv3L_c7_\w+", R + "the landing template c7 with loop forms respelled"),
    ("rejected/v3_review/order_res.txt", r"rv3O_c7_\w+", R + "the landing template c7 with statement order respelled"),
    ("r11/abl_d.log", r"w2", R + "the ablation chassis (see r11/banked/w2.c)"),
    ("r11/abl_d.log", r"abl_d_(?!1111110)[01]{6}0", R + "delta still reused: the ground depth (the mask's last digit, "
                                                     "0 = shared) shares `d` with at least one sphere delta"),
]
