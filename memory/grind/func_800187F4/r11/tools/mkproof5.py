P = "memory/grind/func_800187F4/r11/proof.md"
s = open(P, encoding="utf-8").read()


def rep(a, b):
    global s
    assert s.count(a) == 1, (a[:90], s.count(a))
    s = s.replace(a, b)


rep("# func_800187F4 — Ruling 11 (D) proof and landing record, v4 (manual, 2026-09-28)",
    "# func_800187F4 — Ruling 11 (D) proof and landing record, v5 (manual, 2026-09-28)")
i = s.index("v4 (same body): the third fresh layer-2")
j = s.index("\n\n", i)
s = s[:j] + """

v5 (same body): the fourth fresh layer-2 FAILed the ledger only (the landing spelling's dumps for work and
delta were no longer banked at HEAD; §3 still stated universals; tmp-only reviewer probes; three figures).
v5 banks the landing-chassis dumps (r11/dumps_table_landing.txt, r11/lreg_excerpts_landing.txt), rewrites
§3 as mechanism + record only, banks every reviewer probe (r11/reviewer_probes/), and corrects the figures.
r11/dumps_table.txt remains the v2 (c6) chassis table; the landing citations below use the landing files.""" + s[j:]

a = s.index("## 3. Mechanism and necessity, per variable")
b = s.index("## 3A.")
s = s[:a] + """## 3. Mechanism per Ruling 11 local (Q31 (a)), with the measured record (Q31 (b)-(c) in §3A)

Dumps of the LANDING spelling (c7 = v1 code) and of each one-variable-per-value spelling on that chassis:
r11/dumps_table_landing.txt (r11table rows: pseudo, allocator, register, ALLOCDBG ord/nrefs/livelen/pri) and
r11/lreg_excerpts_landing.txt (.lreg insns of the two cse decisions); commands in the files' headers.
Mechanism sources: global.c:635-656 `allocno_compare` (priority = floor_log2(n_refs) * n_refs /
live_length * 10000, seated in that order, find_reg taking the lowest free register); flow.c:2081
(reg_n_refs += loop depth); local-alloc.c:472 (a pseudo in one block with one death is a local quantity,
seated before global); local-alloc.c combine_regs (:1784, called :1295-1346); cse.c make_regs_eqv (:840-857,
class head = the register whose last reference, reads and sets per regclass.c:1763-1764, is later).

### idx — global.c priority, with the guard fold
- Landing (c7): pseudo 89 (all three counters) nrefs 45 livelen 245 pri 9183, ord 36 -> $t2, seated after
  bits ($a0), vx/vy/vz, the node+8 giv and nforce (pseudo 90, pri 10000, ord 34, $t1); each init in its
  for-init, so the guard `0 < count` folds into `blez` with the init in the delay slot. Target: `addiu
  $t2,$t2,1; slt $v0,$t2,$t1` / `... $v0,$t2,$v0`.
- Per-value (v1pv_idx): the force counters nrefs 16, livelen 25 / 22, pri 25600 / 29090, ord 21 / 20 ->
  $a0; 67 lines. With the init hoisted (v3 reviewer, rv3I_1000000_ff_loop) the priority falls below the
  count's (9552 < 9767, $t2) but the guard no longer folds and a phantom slot is lost (27; 3 with lz[8]).
- Record: every banked counting spelling for idx misses (§3A).

### nforce — global.c priority, the floor_log2 step
- Landing: pseudo 90 (both counts) nrefs 14 over 42 -> 3*14/42 = pri 10000, seated before idx (9183) ->
  $t1; idx -> $t2.
- Per-value (v1pv_nforce): each count nrefs 7 over 21 -> 2*7/21 = 6666, seated after idx; the seats swap
  (21 lines). Loop-form respellings (goto/while/do-while, v3 reviewer) change the depth weighting and are
  measured in §3A.
- Record: every banked counting spelling for nforce misses (§3A).

### temp — local-alloc of single-block values vs global priority
- Landing: pseudo 300 (copy + both table bytes) nrefs 18 livelen 8 -> pri 90000, ord 0 -> $a0; the shift
  pseudos (359 / 423, pri 64285) take $v1. Target: `addu $a0,$a1,$zero` (0x80018E18), `lbu $a0`
  (0x80018E6C / 0x80018FE4), shift in $v1.
- Per-value (v1pv_temp): each table byte is one write and one read in its LZC arm, a local-alloc quantity
  (local-alloc.c:472) seated before global into $v1; the shifts then take $a0 and the copy $v1 (42 lines).
  Ablations: copy alone 2, byte 1 alone 9, byte 2 alone 31.
- Q28 clause for V1 (the copy `temp = work;`): (a) one write, whole RHS the named local `work`, no cast;
  `work` is read again (`work < 0x400`, `[work]`, `work >> nbits`). (b) its only read is
  `gte_Lzc(temp, &lz[0])`, the bare variable as the whole `"r"(r1)` operand of gte_ldlzc's `move $12,%0`,
  an admitted class unit. (c) later on that path the variable holds V2, the table byte (a load; `lbu` in the
  target). (d) the copy is the target's `addu $a0,$a1,$zero` at 0x80018E18, emitted at the same position
  (0 lines). (e) Q31 governs its (D)(3): fresh copy locals, every placement (function scope, in the arm, as
  an initializer, u32, register, no copy at all) are banked: 2 each, none reaching the target.
- Record: every banked counting spelling for temp misses (§3A).

### work — cse.c make_regs_eqv class head (lreg_excerpts_landing.txt)
- Landing (c7): insn 883 sets pseudo 301 (work) to the sum, insn 886 copies it to 300 (temp), insn 889
  compares 301 with 1024: work's last reference (the scale, the lddp) is later than temp's, so work stays
  the class head and the copy stays its own move (target `addu $a1,$v0,$a0; slti $v0,$a1,0x400; beqz;
  addu $a0,$a1,$zero`).
- Per-value (v1pv_work): insn 883 sets pseudo 300 (temp's) to the sum directly, there is no copy insn, and
  insn 889 compares 300: sq1's last reference (the LZC-1 index) is before temp's, temp became the head and
  took sq1's reads (`addu $a0,$v0,$a0 ... nop`); 26 lines. With the Q28 copy also its own local, sq1 stays
  the head but the spelling misses through other seats (v3 reviewer rv3x_0011000 27, rv3x_0041001 13).
- Record: every banked counting spelling for work misses (§3A); the FAKE dead-store closers are set aside
  (§5).

### delta — cse.c make_regs_eqv class head, with the `/ 8` copy (lreg_excerpts_landing.txt)
- Landing (c7): insn 645 copies 269 (delta) into 276, jump 647 tests `(ge (reg/v:SI 269) 0)` and insn 649
  adds 7 to `(reg/v:SI 269)`: delta's last reference (the focus-0 Y delta) is later than 276's, so delta
  stays the head: `bgez $v1` with the copy in the delay slot = target.
- Per-value (v1pv_delta): insn 645 carries REG_DEAD for 269 (the ground depth's last reference), 276
  becomes the head, and 647 / 649 read `(reg:SI 276)`: `bgez $v0`; 4 lines.
- Record: every banked counting spelling for delta misses (§3A); the FAKE dead-store closers are set aside
  (§5). Which sphere delta shares is free (each of the six alone reaches 0 while the ground depth shares,
  abl_d.log); the body shares the first one computed, the Y delta to focus 0.

### nbits / nbits2 — local-alloc combine_regs tie
- Landing: pseudo 359 (lz[0], then the shift) is set twice, so not a local quantity (global, pri 64285,
  $v1); the `& ~1` result is a separate local temp that takes $v0: `and $v0,$v1,$s6; subu $v1,$t9,$v0`.
- Per-value (v1pv_nbits): `lzcount = lz[0]` is written once and dies at the `& ~1` in one block: a local
  quantity (v2 chassis dump: local-alloc $v1), and combine_regs ties the `&` result to it: `and
  $v1,$v1,$s6; subu $v1,$t9,$v1`; 2 lines (4 for both).
- Record: every banked counting spelling for nbits / nbits2 misses (§3A).

""" + s[b:]

a = s.index("| v3 reviewer: every mix of the seven splits | 799 |")
b = s.index("No counting spelling reaches the target. The known closers all add FAKE constructs and are set aside (§5).")
s = s[:a] + """| v3 reviewer: every mix of the seven splits | 799 | 0 | 2 | rejected/v3_review/combo_res.txt |
| v3 reviewer: loop forms (goto / while / do-while) over 11 bases | 740 per-value (+ 74 `c7_*` reuse bodies, 8 of them 0) | 0 | 2 | loop_res.txt |
| v3 reviewer: statement orders | 72 per-value (+ 24 `c7_*` reuse bodies, 2 of them 0) | 0 | 21 | order_res.txt |
| v3 reviewer: early inits | 60 + 400 per-value (+ 1 reuse body) | 0 | 27 / 32 | init_res.txt, init2_res.txt |
| v1/v2 reviewers' FAKE-free probes (temp bytes at loop scope; nforce split) | 2 | 0 | 21 | r11/reviewer_probes/ (rv_tp4 42, rv_nf0 21) |
| v4 reviewer: types / register / initializer / ternary / compound respellings | 16 | 0 | 2 | r11/reviewer_probes/ (rv4_*) |
| v2 reviewer: single do-while(0) wraps (FAKE, set aside, for completeness) | 500 | 0 | 2 | rejected/v2_review/ |
| author v2: dead stores / self-assigns / chain-extenders (FAKE, set aside) | 209 | 0 | 2 | ds.log, ds2.log |

The v3 reviewer's FAKE-free one-variable-per-value spellings total 2,071 (the other 99 of its 2,170 bodies
are the reuse spelling with a loop or order respelled). """ + s[b:]

rep("rejected/ declare `s32 lz[6]` (the same declaration; the v1/v2 bodies carry the older comment text);",
    "rejected/ declare `s32 lz[6]` (the same declaration; the comment on it differs across versions, and the\nv1 variants carry none);")
rep("| work | per-value + `sq1 = 0;` dead store after temp's last use (end of the ellipsoid body / before `tot`) | 0 / 0 | tmp/func_800187F4/rv_wd_end.c, rv_wd_after2.c (v1 layer-2); v2 chassis c6 (variants_v2/c6.c) |",
    "| work | per-value + `sq1 = 0;` dead store after temp's last use (end of the ellipsoid body / before `tot`) | 0 / 0 | r11/reviewer_probes/rv_wd_end.c, rv_wd_after2.c (v1 layer-2); v2 chassis c6 (variants_v2/c6.c) |")
rep("| work + delta | both | 0 | rv_both.c; c6 |", "| work + delta | both | 0 | r11/reviewer_probes/rv_both.c; c6 |")
open(P, "w", encoding="utf-8", newline="\n").write(s)
print("ok")
