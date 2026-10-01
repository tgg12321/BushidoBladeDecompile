# func_800770B8 — evidence

## 2026-09-30 — ff-c, retro-audit-class fix-forward (laneC reviewer l2-759D0-r1 frontier, memory/grind/func_800759D0/evidence.md)

func_800768DC and func_800770B8 are fixed together: they share the D_800A35D0 declaration (TU-wide),
and so do func_800747D8 and func_80075670 (both completed, respelled byte-neutrally).

Object model:
- D_800A35D0 = `s16 D_800A35D0[2][2]`, one {s16, s16} pair per player. Evidence: data
  asm/data/91C98.data.s (two words 0x800A35D0..D7; D_800A35D8 is its own object); every consumer
  indexes it by player*4 (func_800768DC 0x80076948 `sll $v1,$s0,2` added to the base at 0x8007695C,
  the same index that forms SelWork f40[player] at +0x40, the parallel s16[2][2] passed as the other
  pointer to func_800692C0); func_800770B8 clears halves +2 and +0 of pair t0 (0x80077184-0x8007718C);
  func_800747D8 / func_80075670 pass pair 0 / pair arg1 to func_800692C0 (s16 *arg3). Replaces the
  `extern s16 D_800A35D0` scalar and its `(&D_800A35D0) + (player * 2)` index-past-scalar.
- D_8009BCE4 = `u8 D_8009BCE4[20]` (laneC ledger: 20 per-character flag bytes, no other symbol
  inside); `(&D_8009BCE4)[x]` -> `D_8009BCE4[x]`.
- D_8009BD21 is byte 1 of record 0 of `u8 D_8009BD20[][2]` (already declared in this TU and in
  text1b_tu1e.c; data 0x01 0x02 0x03 0x04 = two 2-byte records, asm/data/7D920.data.s:23805-23817).
  `(&D_8009BD21)[f67 * 2]` -> `D_8009BD20[f67][1]`; the C extern of D_8009BD21 is deleted (the
  dlabel / linker row stay for the INCLUDE_ASM users; no C handle remains).
- SelWork_800768DC 0x1C..0x23: two per-player s16 pairs (func_80075F80 lh/sh 0x1C/0x20 with a
  player-scaled base, asm/funcs/func_80075F80.s:107-156) that func_800770B8 clears with ONE word store
  each: 0x800772BC `sw $zero,0x20($a0)`, 0x800772C0 `sw $zero,0x1C($a0)`. Declared as Q33/Q46 union
  word views `union { s16 half[2]; s32 word; } f1C, f20;` in place of pad1C[0x18]'s first 8 bytes
  (pad24[0x10] keeps every later offset); the word members are named only at those two sites.

func_800770B8's `sym`: a typed row pointer to D_800A35D0 (pointer-alias-fake-exception typed re-view,
`s16 (*p)[2]`), needed by the existing FAKE same-value re-set (loop.c may_not_move; see that comment).
Exhaustion: no row pointer at all (direct subscripts, re-set dropped) 19/175; row pointer kept, re-set
dropped 18/175.

Scores (tmp/ffc/score_full.py = sandbox_score --disable all on full-file variants; diffs in
ff-c-2026-09-30/):

| variant | 800747D8 | 80075670 | 800768DC | 800770B8 |
|---|---|---|---|---|
| f1 declarations + consumers, 770B8 byte pointer kept | 0 | 0 | 0 | 0 |
| f2 + typed row pointer, D_8009BD20[][1], unions declared (word stores still cast) | 0 | 0 | 0 | 0 |
| **f2c + word stores through SELWORK_800768DC->f20.word / f1C.word (proposed)** | **0** | **0** | **0** | **0** |
| f3 f2c without the row pointer | — | — | — | 19 |
| f3b f2c, row pointer kept, re-set dropped | — | — | — | 18 |

### Applied (uncommitted, for layer-2), 2026-09-30

f4 = f2c + a FAKE pointer-alias annotation at `sym`'s declaration (text in src), regenerated on HEAD
1c47528d6 (after func_80074E08's reopen). Under the lock: rebuild build_sha1
62efab4f73f992798c43e8c730aa43baa10bb4fa; integrity OK; sandbox 0 for all four; layer-2 hashes
func_800747D8 d8db9fc860829384, func_80075670 ae836e096f26313c, func_800768DC 0d456e1392bb3646,
func_800770B8 120523d39b6ddfad. Exact bytes: tmp/ffc/backup_45/text1b_tu2.c.

## 2026-09-30 — REOPENED under owner Q37 (layer-2 rev-45, SelWork cluster)

Reopened together with the other SelWork consumers; the landed body is in
rejected/selwork-cluster-2026-09-30.c. Frontier (one landing for the whole cluster, rev-45's
findings, the Q33(5) size question, p_old, D_8009BD20, GaugeWork): 
pre-slim-2026-10-01:memory/grind/func_800768DC/selwork-cluster-2026-09-30.md (+ the reviewed patch beside it).

## 2026-09-30 — laneD (SelWork cluster, Q57)

Measurements, chosen variants and the one-type SelWork: pre-slim-2026-10-01:memory/grind/func_800768DC/laneD-2026-09-30.md;
Q57 (d) layout search: pre-slim-2026-10-01:memory/grind/func_800768DC/q57-layout-search-2026-09-30.md.

func_800770B8 status (laneD): all-member body byte-exact (candidate.c, 0/175) but it keeps the p_old
reuse + FAKE dead restore that rev-45 item 3 refused; owner question filed via the orchestrator
2026-09-30; not in this landing (INCLUDE_ASM).

## 2026-09-30 — owner Q66: "Refuse for now (Recommended)" — p_old reuse stays banned; INCLUDE_ASM

Frontier, measured by laneD on the all-member SelWork chassis. Details are in
pre-slim-2026-10-01:memory/grind/func_800768DC/laneD-2026-09-30.md; the bodies are in
pre-slim-2026-10-01:memory/grind/func_800768DC/laneD-2026-09-30/.

| form | score | notes |
|---|---|---|
| candidate.c: all-member body, p_old reuse + FAKE restore | 0/175 | refused (Ruling 11 (B)(1), Q66) |
| k0: no restore | 2/175 | see below |
| v7, m1-m3: separate list / work locals (rev-45's suggestion) | 20/175 (170 insns) | see below |
| r1/r2: Ruling 4 split of the list pointer | 20 | |
| r3: function-scope work pointer reused as the loop base | 34 | |

- **k0.** The only diff is the base register of the 0x30/0x34 clears: $s1 (p_old) where the target has
  $v0 (the func_8006E49C return). The cause is cse. It keeps the D_800A36A0 reload in
  SELWORK->f30/f34 equivalent to the pseudo stored into D_800A36A0, unless that pseudo is overwritten
  after the store. The restore was what broke that equivalence.
- **v7, m1-m3.** The list pointer arg0 + 0x58 is set once, so it is tied to arg0's register ($s0). The
  function then uses one fewer callee-saved register.

Re-measure after owner-adopted Q65 (the per-file gp model) lands. The residual involves D_800A36A0's
%gp_rel addressing, which Q65 changes.

## 2026-10-01 — laneA: re-measured on the regenerated Q65 series (scratch clone, tag step16 = b55913da8, base 6c73a2796)

SelWork f1C/f20 given the Q33/Q46 union word views (uncommitted, scratch only; text1b_b's other f1C/f20
element reads respelled `.half[...]`); f3C stays `s16 f3C[2]` as landed with func_80075F80. The function now
lives in text1b_b (M4 merge). `engine sandbox --disable all`:
- candidate.c (f3C.half -> f3C): 0/175 (only the masked D_8009BD21 reloc-symbol hunk at insn 163).
- k0 (candidate without the refused `p_old = prev;` restore): **2/175, unchanged by Q65** — the same
  operand-only hunk at insn 35: `sw zero,48 / sh zero,52` based on $s1 where the target uses $v0.
So Q66's "may shift after Q65" did not happen: the gp model does not touch this residual (it is the cse
equivalence of the D_800A36A0 reload, not its addressing). Frontier unchanged: an ordinary-C spelling that
breaks that equivalence without a dead write. Scripts: tmp/func_800770B8/{mk.py,scratch_sbx.sh} (not banked).
