# func_800187F4 — v8 landing, eighth layer-2 FAIL (records only), 2026-09-29

Body: the same bytes (sandbox 0 on 644/644; the reviewer's own verify-oracle --rebuild --allow-dirty gave the
oracle SHA1; fast3 control 0/662). The reviewer passed:
- the 85 islands (its own strict checker, tmp/func_800187F4/rv8_islands.py) and the seven Q29 words;
- the 85 hashes, the gate regions, and `source_issues`;
- all seven Ruling 11 locals, with prongs A–H and Q28 (a)–(e);
- the lz[6] arithmetic, the struct offsets, the sibling citations, and every §3A count it tested.

Its 7 new FAKE-free probes missed (best 4).

FAIL grounds:
1. The frame-census bodies (r11/variants_v2/fr_*.c) were misclassified. They are v2-chassis spellings with
   work and delta split, and they declare lz[2] or scalars instead of the landing's lz[6]. Under Q30 they
   are therefore not set aside; they count. Proof §5 called them "the reuse body, or its lz[2] form", §3A
   said every c6 body is set aside, and the counting table omitted them. All miss (24–185 lines).
2. The w2 delta-ablation bodies with one value per variable (e.g. abl_d_1111111, 4 lines) were not in the
   counting record. Their chassis (tmp/func_800187F4/w1.c, w2.c) was not banked, although the v9 note said
   the tmp-only evidence was.
3. The Match message's set-aside sentence and "14 ordinary spellings" inherited error 1.

Fixed in v10:
- §3A is rebuilt on r11/manifest.md (r11/tools/manifest.py). One uniform sweep re-measured all 210 banked
  bodies, and the manifest adds the 15 result-file sets. It lists every body that reaches 0 with its reason
  (REUSE or SET ASIDE), and the generator fails on an unexplained zero.
- §5 classifies fr_* as counting spellings.
- w1/w2 and the rv8_* probes are banked.
- frame_census.md and Match message v10 are reworded.
